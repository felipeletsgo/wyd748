package store

import (
	"context"
	"errors"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
	"github.com/jackc/pgx/v5/pgxpool"
)

func TestPostgresSerializableTxRetriesSerializationConflicts(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated test database")
	}

	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()
	st, err := NewPostgresStore(ctx, PostgresConfig{URL: url, MaxConns: 1, OperationTimeout: time.Second})
	if err != nil {
		t.Fatal(err)
	}
	defer st.Close()

	serializationErr := &pgconn.PgError{Code: "40001", Message: "injected serialization failure"}
	attempts := 0
	err = st.withSerializableTx(func(context.Context, pgx.Tx) error {
		attempts++
		return serializationErr
	})
	if err == nil || attempts != 3 || !errors.Is(err, serializationErr) ||
		!strings.Contains(err.Error(), "exceeded retries") {
		t.Fatalf("retry result: attempts=%d error=%v", attempts, err)
	}

	deadlineStore := newPostgresStore(st.pool, PostgresConfig{OperationTimeout: 50 * time.Millisecond})
	deadlineAttempts := 0
	err = deadlineStore.withSerializableTx(func(ctx context.Context, _ pgx.Tx) error {
		deadlineAttempts++
		<-ctx.Done()
		return serializationErr
	})
	if deadlineAttempts != 1 || !errors.Is(err, context.DeadlineExceeded) ||
		!strings.Contains(err.Error(), "total deadline") {
		t.Fatalf("deadline result: attempts=%d error=%v", deadlineAttempts, err)
	}

	closedPool, err := pgxpool.New(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	closedPool.Close()
	closedStore := newPostgresStore(closedPool, PostgresConfig{})
	callbackRan := false
	err = closedStore.withSerializableTx(func(context.Context, pgx.Tx) error {
		callbackRan = true
		return nil
	})
	if err == nil || callbackRan {
		t.Fatalf("closed pool did not fail before the transaction callback: err=%v callbackRan=%t", err, callbackRan)
	}
}
