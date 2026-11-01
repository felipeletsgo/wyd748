package store

import (
	"context"
	"errors"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
)

// Real PostgreSQL faults for the certified transaction path (W1/D2). Each case
// fails before COMMIT and must leave no durable row, no held lock and a pool
// that serves the next operation.
func criticalFaultStore(t *testing.T, timeout time.Duration) (*PostgresStore, string) {
	t.Helper()
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 3, OperationTimeout: timeout})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	table := fmt.Sprintf("critical_fault_%d", time.Now().UnixNano())
	if _, err := st.pool.Exec(context.Background(), `CREATE TABLE `+table+` (id text PRIMARY KEY, v int NOT NULL)`); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _, _ = st.pool.Exec(context.Background(), `DROP TABLE IF EXISTS `+table) })
	return st, table
}

func faultRowCount(t *testing.T, st *PostgresStore, table, id string) int {
	t.Helper()
	var count int
	if err := st.pool.QueryRow(context.Background(), `SELECT count(*) FROM `+table+` WHERE id=$1`, id).Scan(&count); err != nil {
		t.Fatal(err)
	}
	return count
}

// assertRecovered proves no row, no held lock on id and a usable pool.
func assertRecovered(t *testing.T, st *PostgresStore, table, id string) {
	t.Helper()
	if n := faultRowCount(t, st, table, id); n != 0 {
		t.Fatalf("rejected attempt left %d durable row(s)", n)
	}
	err := st.withCertifiedSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		if _, err := tx.Exec(ctx, `SET LOCAL lock_timeout = '200ms'`); err != nil {
			return err
		}
		_, err := tx.Exec(ctx, `INSERT INTO `+table+`(id, v) VALUES($1, 2)`, id)
		return err
	})
	if err != nil {
		t.Fatalf("next operation on the same key failed: %v", err)
	}
	if acquired := st.pool.Stat().AcquiredConns(); acquired != 0 {
		t.Fatalf("pool still holds %d connection(s)", acquired)
	}
}

func TestCertifiedTransactionPostgresFaultsBeforeCommit(t *testing.T) {
	for _, tc := range []struct {
		name string
		run  func(table, id string) func(context.Context, pgx.Tx) error
	}{
		{"callback-error-after-write", func(table, id string) func(context.Context, pgx.Tx) error {
			return func(ctx context.Context, tx pgx.Tx) error {
				if _, err := tx.Exec(ctx, `INSERT INTO `+table+`(id, v) VALUES($1, 1)`, id); err != nil {
					return err
				}
				return errors.New("ledger validation failed")
			}
		}},
		{"deadline-during-sql", func(table, id string) func(context.Context, pgx.Tx) error {
			return func(ctx context.Context, tx pgx.Tx) error {
				if _, err := tx.Exec(ctx, `INSERT INTO `+table+`(id, v) VALUES($1, 1)`, id); err != nil {
					return err
				}
				_, err := tx.Exec(ctx, `SELECT pg_sleep(0.8)`)
				return err
			}
		}},
		{"connection-lost-during-sql", func(table, id string) func(context.Context, pgx.Tx) error {
			return func(ctx context.Context, tx pgx.Tx) error {
				if _, err := tx.Exec(ctx, `INSERT INTO `+table+`(id, v) VALUES($1, 1)`, id); err != nil {
					return err
				}
				_, err := tx.Exec(ctx, `SELECT pg_terminate_backend(pg_backend_pid())`)
				return err
			}
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			st, table := criticalFaultStore(t, 300*time.Millisecond)
			id := tc.name
			err := st.withCertifiedSerializableTx(tc.run(table, id))
			if err == nil || !IsWriteRejected(err) {
				t.Fatalf("pre-COMMIT failure was not a certified rejection: %v", err)
			}
			if tc.name == "deadline-during-sql" {
				time.Sleep(time.Second) // let the abandoned backend end its sleep and abort
			}
			assertRecovered(t, st, table, id)
		})
	}
}

func TestCertifiedTransactionPostgresDeadlineBeforeBegin(t *testing.T) {
	st, table := criticalFaultStore(t, time.Second)
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	calls := 0
	err := certifiedSerializableTx(ctx, st.pool.BeginTx, func(context.Context, pgx.Tx) error {
		calls++
		return nil
	})
	if !IsWriteRejected(err) || calls != 0 {
		t.Fatalf("expired admission ran the callback or lost certification: calls=%d err=%v", calls, err)
	}
	assertRecovered(t, st, table, "before-begin")
}

func TestCertifiedTransactionPostgresExhaustedSerializationRetries(t *testing.T) {
	st, table := criticalFaultStore(t, 2*time.Second)
	attempts := 0
	err := st.withCertifiedSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		attempts++
		if _, err := tx.Exec(ctx, `INSERT INTO `+table+`(id, v) VALUES('retry', 1)`); err != nil {
			return err
		}
		_, err := tx.Exec(ctx, `DO $$ BEGIN RAISE EXCEPTION 'injected conflict' USING ERRCODE = '40001'; END $$`)
		return err
	})
	if !IsWriteRejected(err) || attempts != 3 {
		t.Fatalf("serialization retries: attempts=%d err=%v", attempts, err)
	}
	assertRecovered(t, st, table, "retry")
}
