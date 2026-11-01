package store

import (
	"context"
	"errors"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
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
