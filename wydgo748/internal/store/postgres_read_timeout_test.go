package store

import (
	"context"
	"errors"
	"expvar"
	"os"
	"testing"
	"time"
)

func readFailures(label string) int64 {
	if v, ok := metricPostgresReadFailures.Get(label).(*expvar.Int); ok {
		return v.Value()
	}
	return 0
}

// W2/D8: reads use their own budget, which includes pool acquisition. A pool
// held past the short write deadline must not fail an account read.
func TestPostgresReadBudgetIsIndependentFromCriticalDeadline(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	for _, tc := range []struct {
		name        string
		read, hold  time.Duration
		wantTimeout bool
	}{
		{"pool-wait-within-read-budget", 2 * time.Second, 400 * time.Millisecond, false},
		{"pool-exhausted-past-read-budget", 200 * time.Millisecond, 700 * time.Millisecond, true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 1,
				OperationTimeout: 100 * time.Millisecond, ReadTimeout: tc.read})
			if err != nil {
				t.Fatal(err)
			}
			defer st.Close()
			held, err := st.pool.Acquire(context.Background())
			if err != nil {
				t.Fatal(err)
			}
			time.AfterFunc(tc.hold, held.Release)
			before := readFailures("timeout")
			started := time.Now()
			_, err = st.LoadAccount("missing-read-budget-account")
			waited := time.Since(started)
			if tc.wantTimeout {
				if !errors.Is(err, context.DeadlineExceeded) || readFailures("timeout") != before+1 {
					t.Fatalf("exhausted pool err=%v timeouts=%d", err, readFailures("timeout")-before)
				}
				time.Sleep(tc.hold) // let the held connection return before Close
				return
			}
			if !errors.Is(err, os.ErrNotExist) || waited < 300*time.Millisecond {
				t.Fatalf("read inherited the write deadline: err=%v waited=%s", err, waited)
			}
		})
	}
}
