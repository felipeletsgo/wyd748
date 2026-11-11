package store

import (
	"context"
	"errors"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
)

// lostAckTx models a lost COMMIT acknowledgement. The real transaction either
// commits or rolls back on the server, and the caller only sees a transport
// error with no pgconn send certification.
type lostAckTx struct {
	pgx.Tx
	commit bool
}

func (tx *lostAckTx) Commit(ctx context.Context) error {
	if tx.commit {
		if err := tx.Tx.Commit(ctx); err != nil {
			return err
		}
	} else {
		_ = tx.Tx.Rollback(ctx)
	}
	return errors.New("read tcp: connection reset by peer")
}

// W4/D2: an unknown COMMIT is resolved by the attempt's exact transaction ID
// before the worker reports it. Only "in progress" stays unknown.
func TestUnknownCommitReconciledByTransactionStatus(t *testing.T) {
	for _, tc := range []struct {
		name               string
		commit             bool
		wantRow, rejection bool
	}{
		{"committed-ack-lost", true, true, false},
		{"aborted-ack-lost", false, false, true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			st, table := criticalFaultStore(t, 2*time.Second)
			begin := func(ctx context.Context, options pgx.TxOptions) (pgx.Tx, error) {
				tx, err := st.pool.BeginTx(ctx, options)
				if err != nil {
					return nil, err
				}
				return &lostAckTx{Tx: tx, commit: tc.commit}, nil
			}
			ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
			defer cancel()
			err := st.certifiedTxWithReconciliation(ctx, begin, func(ctx context.Context, tx pgx.Tx) error {
				_, err := tx.Exec(ctx, `INSERT INTO `+table+`(id, v) VALUES('reconcile', 1)`)
				return err
			}, time.Second)
			if tc.rejection != IsWriteRejected(err) || (!tc.rejection && err != nil) {
				t.Fatalf("reconciled outcome err=%v rejected=%t", err, IsWriteRejected(err))
			}
			if got := faultRowCount(t, st, table, "reconcile") == 1; got != tc.wantRow {
				t.Fatalf("durable row present=%t, want %t", got, tc.wantRow)
			}
		})
	}
}

func TestUnknownCommitStaysUnknownWhileInProgress(t *testing.T) {
	st, _ := criticalFaultStore(t, 2*time.Second)
	tx, err := st.pool.Begin(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	defer tx.Rollback(context.Background())
	var xid string
	if err := tx.QueryRow(context.Background(), `SELECT pg_current_xact_id()::text`).Scan(&xid); err != nil {
		t.Fatal(err)
	}
	unknown := indeterminateWrite(errors.New("commit acknowledgement lost"))
	err = st.reconcileUnknownCommit(xid, unknown, 200*time.Millisecond)
	if err == nil || IsWriteRejected(err) || !errors.Is(err, unknown) {
		t.Fatalf("an open transaction was settled: %v", err)
	}
}

func TestTransactionStatusReadsCommittedOutcome(t *testing.T) {
	st, _ := criticalFaultStore(t, 2*time.Second)
	tx, err := st.pool.Begin(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	var xid string
	if err := tx.QueryRow(context.Background(), `SELECT pg_current_xact_id()::text`).Scan(&xid); err != nil {
		_ = tx.Rollback(context.Background())
		t.Fatal(err)
	}
	if err := tx.Commit(context.Background()); err != nil {
		t.Fatal(err)
	}
	status, err := st.TransactionStatus(xid)
	if err != nil || status != "committed" {
		t.Fatalf("transaction status=%q err=%v", status, err)
	}
	if _, err := st.TransactionStatus("not-an-xid"); err == nil {
		t.Fatal("invalid transaction ID did not return a query error")
	}
	unknown := &UnresolvedCommitError{XID: xid, Cause: errors.New("acknowledgement lost")}
	if unknown.Error() != "acknowledgement lost" || !errors.Is(unknown, unknown.Cause) {
		t.Fatalf("unresolved error=%v", unknown)
	}
}

// closedAfterSendTx models pgconn returning its "conn closed" lock error
// after COMMIT reached PostgreSQL. That error claims SafeToRetry, but the
// outcome must still come from the exact transaction status.
type closedAfterSendTx struct {
	pgx.Tx
	commit bool
}

type closedAfterSendError struct{}

func (closedAfterSendError) Error() string     { return "conn closed" }
func (closedAfterSendError) SafeToRetry() bool { return true }
func (closedAfterSendError) Unwrap() error     { return pgconn.ErrConnClosed }

func (tx *closedAfterSendTx) Commit(ctx context.Context) error {
	if tx.commit {
		if err := tx.Tx.Commit(ctx); err != nil {
			return err
		}
	} else {
		_ = tx.Tx.Rollback(ctx)
	}
	return closedAfterSendError{}
}

func TestClosedConnectionAtCommitIsReconciledNotCertified(t *testing.T) {
	for _, tc := range []struct {
		name               string
		commit             bool
		wantRow, rejection bool
	}{
		{"committed", true, true, false},
		{"aborted", false, false, true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			st, table := criticalFaultStore(t, 2*time.Second)
			begin := func(ctx context.Context, options pgx.TxOptions) (pgx.Tx, error) {
				tx, err := st.pool.BeginTx(ctx, options)
				if err != nil {
					return nil, err
				}
				return &closedAfterSendTx{Tx: tx, commit: tc.commit}, nil
			}
			ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
			defer cancel()
			err := st.certifiedTxWithReconciliation(ctx, begin, func(ctx context.Context, tx pgx.Tx) error {
				_, err := tx.Exec(ctx, `INSERT INTO `+table+`(id, v) VALUES('closed', 1)`)
				return err
			}, time.Second)
			if tc.rejection != IsWriteRejected(err) || (!tc.rejection && err != nil) {
				t.Fatalf("closed-at-commit outcome err=%v rejected=%t", err, IsWriteRejected(err))
			}
			if got := faultRowCount(t, st, table, "closed") == 1; got != tc.wantRow {
				t.Fatalf("durable row present=%t, want %t", got, tc.wantRow)
			}
		})
	}
}
