package store

import (
	"context"
	"errors"
	"fmt"
	"testing"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
)

// Embedding deliberately leaves unrelated database methods unavailable. These
// tests exercise transaction outcome and retry decisions without a database.
type criticalOutcomeTx struct {
	pgx.Tx
	commitErr   error
	rollbackErr error
	commits     int
	rollbacks   int
}

func (tx *criticalOutcomeTx) Commit(context.Context) error {
	tx.commits++
	return tx.commitErr
}

func (tx *criticalOutcomeTx) Rollback(context.Context) error {
	tx.rollbacks++
	return tx.rollbackErr
}

// notSentErr models pgconn's certification that a statement never reached the server.
type notSentErr struct{}

func (notSentErr) Error() string     { return "context deadline exceeded before send" }
func (notSentErr) SafeToRetry() bool { return true }

func TestCertifiedTransactionAttemptOutcomes(t *testing.T) {
	callbackErr := errors.New("ledger conflict")
	transportErr := errors.New("connection lost")
	for _, tc := range []struct {
		name     string
		runErr   error
		commit   error
		rollback error
		rejected bool
		commits  int
	}{
		{"committed", nil, nil, pgx.ErrTxClosed, false, 1},
		{"rolled-back", callbackErr, nil, nil, true, 0},
		// COMMIT was never sent: a failed rollback discards the connection only.
		{"rollback-failed-before-commit", callbackErr, nil, transportErr, true, 0},
		{"nested-rejection-rollback-failed", rejectWrite(callbackErr), nil, transportErr, true, 0},
		{"commit-not-sent", nil, notSentErr{}, nil, true, 1},
		{"commit-unknown", nil, transportErr, nil, false, 1},
		{"aborted-commit", nil, pgx.ErrTxCommitRollback, pgx.ErrTxClosed, true, 1},
		{"serialization-abort", nil, &pgconn.PgError{Code: "40001"}, pgx.ErrTxClosed, true, 1},
		{"deadlock-abort", nil, &pgconn.PgError{Code: "40P01"}, pgx.ErrTxClosed, true, 1},
	} {
		t.Run(tc.name, func(t *testing.T) {
			tx := &criticalOutcomeTx{commitErr: tc.commit, rollbackErr: tc.rollback}
			err := certifiedTransactionAttempt(context.Background(), tx, func(context.Context, pgx.Tx) error {
				return tc.runErr
			})
			if IsWriteRejected(err) != tc.rejected || tx.commits != tc.commits || tx.rollbacks == 0 {
				t.Fatalf("outcome=%v rejection=%t commits=%d rollbacks=%d", err, IsWriteRejected(err), tx.commits, tx.rollbacks)
			}
			if tc.runErr == nil && tc.commit == nil {
				if err != nil {
					t.Fatal(err)
				}
			} else if err == nil {
				t.Fatal("failed transaction reported success")
			}
			if tc.runErr != nil && !errors.Is(err, tc.runErr) {
				t.Fatal("callback cause was discarded")
			}
		})
	}
}

func TestCertifiedSerializableTxRetriesOnlyCertifiedAborts(t *testing.T) {
	for _, mode := range []string{"rollback", "commit", "wrapped-abort", "failed-rollback", "unknown-commit"} {
		t.Run(mode, func(t *testing.T) {
			attempts, callbacks := 0, 0
			serializationErr := &pgconn.PgError{Code: "40001"}
			var transactions []*criticalOutcomeTx
			err := certifiedSerializableTx(context.Background(), func(_ context.Context, options pgx.TxOptions) (pgx.Tx, error) {
				if options.IsoLevel != pgx.Serializable {
					t.Fatal("critical write lost serializable isolation")
				}
				attempts++
				tx := &criticalOutcomeTx{}
				if attempts == 1 {
					switch mode {
					case "commit":
						tx.commitErr = serializationErr
					case "failed-rollback":
						tx.rollbackErr = errors.New("rollback connection lost")
					case "unknown-commit":
						tx.commitErr = errors.New("commit acknowledgement lost")
					case "wrapped-abort":
						tx.commitErr = fmt.Errorf("commit aborted: %w", serializationErr)
					}
				}
				transactions = append(transactions, tx)
				return tx, nil
			}, func(context.Context, pgx.Tx) error {
				callbacks++
				if callbacks == 1 && (mode == "rollback" || mode == "failed-rollback") {
					return rejectWrite(serializationErr)
				}
				return nil
			})
			if mode == "unknown-commit" {
				if err == nil || IsWriteRejected(err) || attempts != 1 || callbacks != 1 || transactions[0].commits != 1 {
					t.Fatalf("uncertain outcome authorized retry: err=%v attempts=%d callbacks=%d", err, attempts, callbacks)
				}
			} else if err != nil || attempts != 2 || callbacks != 2 {
				t.Fatalf("certified abort did not retry once: err=%v attempts=%d callbacks=%d", err, attempts, callbacks)
			}
		})
	}
}

func TestWriteOutcomeUnknownDominatesNestedRejection(t *testing.T) {
	rejected := rejectWrite(errors.New("rejected"))
	unknown := indeterminateWrite(errors.Join(rejected, errors.New("rollback lost")))
	for _, err := range []error{unknown, fmt.Errorf("adapter: %w", unknown), errors.Join(rejected, unknown), errors.Join(unknown, rejected)} {
		if IsWriteRejected(err) || !errors.Is(err, rejected) {
			t.Fatalf("unknown outcome lost precedence or diagnostics: %v", err)
		}
	}
}

func TestCertifiedSerializableTxAdmissionAndRetryLimit(t *testing.T) {
	for _, mode := range []string{"cancelled", "begin-failure", "retry-limit"} {
		t.Run(mode, func(t *testing.T) {
			ctx, cancel := context.WithCancel(context.Background())
			defer cancel()
			if mode == "cancelled" {
				cancel()
			}
			attempts, callbacks := 0, 0
			err := certifiedSerializableTx(ctx, func(context.Context, pgx.TxOptions) (pgx.Tx, error) {
				attempts++
				if mode == "begin-failure" {
					return nil, errors.New("connection unavailable before callback")
				}
				return &criticalOutcomeTx{commitErr: &pgconn.PgError{Code: "40001"}}, nil
			}, func(context.Context, pgx.Tx) error {
				callbacks++
				return nil
			})
			wantAttempts, wantCallbacks := 0, 0
			if mode == "begin-failure" {
				wantAttempts = 1
			} else if mode == "retry-limit" {
				wantAttempts, wantCallbacks = 3, 3
			}
			if !IsWriteRejected(err) || attempts != wantAttempts || callbacks != wantCallbacks {
				t.Fatalf("rejection=%v attempts=%d callbacks=%d", err, attempts, callbacks)
			}
		})
	}
}

func TestCertifiedTransactionPanicRollsBackAndPropagates(t *testing.T) {
	tx := &criticalOutcomeTx{}
	defer func() {
		if failure := recover(); failure != "callback panic" || tx.commits != 0 || tx.rollbacks != 1 {
			t.Fatalf("panic=%v commits=%d rollbacks=%d", failure, tx.commits, tx.rollbacks)
		}
	}()
	_ = certifiedTransactionAttempt(context.Background(), tx, func(context.Context, pgx.Tx) error {
		panic("callback panic")
	})
}
