package store

import (
	"context"
	"errors"
	"expvar"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
)

func contextBudget(t *testing.T, ctx context.Context, cancel context.CancelFunc, want time.Duration) {
	t.Helper()
	defer cancel()
	deadline, ok := ctx.Deadline()
	if !ok {
		t.Fatal("database context has no deadline")
	}
	remaining := time.Until(deadline)
	if remaining <= 0 || remaining > want || remaining < want-50*time.Millisecond {
		t.Fatalf("context budget=%s, want close to %s", remaining, want)
	}
}

func TestPostgresWriteAndReadContextsUseConfiguredBudgets(t *testing.T) {
	store := &PostgresStore{operationTimeout: 250 * time.Millisecond, readTimeout: 750 * time.Millisecond}
	writeCtx, cancelWrite := store.postgresContext()
	contextBudget(t, writeCtx, cancelWrite, 250*time.Millisecond)
	readCtx, cancelRead := store.postgresReadContext()
	contextBudget(t, readCtx, cancelRead, 750*time.Millisecond)

	readUsesWriteBudget := &PostgresStore{operationTimeout: 250 * time.Millisecond}
	ctx, cancel := readUsesWriteBudget.postgresReadContext()
	contextBudget(t, ctx, cancel, 250*time.Millisecond)

	defaults := &PostgresStore{}
	ctx, cancel = defaults.postgresReadContext()
	contextBudget(t, ctx, cancel, postgresOperationTimeout)
}

func TestObservePostgresReadCountsErrorsButNotEmptyResults(t *testing.T) {
	observePostgresRead(errors.New("initialize metric"))
	readMetric, ok := metricPostgresReadFailures.Get("error").(*expvar.Int)
	if !ok {
		t.Fatal("PostgreSQL read error metric is not initialized")
	}
	before := readMetric.Value()
	observePostgresRead(nil)
	observePostgresRead(pgx.ErrNoRows)
	if got := readMetric.Value(); got != before {
		t.Fatalf("successful or empty read changed error count from %d to %d", before, got)
	}
	observePostgresRead(errors.New("connection reset"))
	if got := readMetric.Value(); got != before+1 {
		t.Fatalf("read error count=%d, want %d", got, before+1)
	}
}

func TestPostgresUUIDValidatesAndDecodesCharacterIdentity(t *testing.T) {
	uid := "00112233445566778899aabbccddeeff"
	parsed, err := postgresUUID(uid)
	if err != nil || !parsed.Valid || parsed.Bytes != [16]byte{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
	} {
		t.Fatalf("postgres UUID=%+v err=%v", parsed, err)
	}
	for _, invalid := range []string{"", "not-hex", "0011"} {
		if value, err := postgresUUID(invalid); err == nil || value.Valid {
			t.Errorf("postgresUUID(%q)=%+v err=%v, want validation error", invalid, value, err)
		}
	}
}
