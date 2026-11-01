package store

import (
	"context"
	"fmt"
	"os"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

// Serialization-conflict probe: concurrent certified writes of distinct
// accounts, as the critical workers issue them. Run with
// WYD_SERIALIZATION_PROBE=1 against the local PostgreSQL test database.
func TestSerializationConflictProbe(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" || os.Getenv("WYD_SERIALIZATION_PROBE") != "1" {
		t.Skip("set WYD_SERIALIZATION_PROBE=1 and WYD_TEST_POSTGRES_URL")
	}
	ctx := context.Background()
	st, err := NewPostgresStore(ctx, PostgresConfig{URL: url, MaxConns: 8, OperationTimeout: 500 * time.Millisecond})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = conn.Close(ctx) })
	prefix := "serial" + time.Now().UTC().Format("150405")
	t.Cleanup(func() { _, _ = conn.Exec(ctx, `DELETE FROM accounts WHERE name_key LIKE $1`, prefix+"%") })
	const accounts = 120
	list := make([]*model.Account, accounts)
	for i := range list {
		list[i] = postgresTestAccount(fmt.Sprintf("%s%03d", prefix, i), fmt.Sprintf("Se%s%03d", prefix[len(prefix)-4:], i),
			model.Item{Index: 4011})
		if err := st.CreateAccount(list[i]); err != nil {
			t.Fatal(err)
		}
		for slot := range 30 {
			list[i].Chars[0].Inv[slot] = model.Item{Index: uint16(400 + slot)}
		}
	}
	for _, workers := range []int{1, 2, 4} {
		retriesBefore := metricPostgresTxRetries.Value()
		var failed atomic.Int64
		var wg sync.WaitGroup
		started := time.Now()
		for worker := range workers {
			wg.Add(1)
			go func() {
				defer wg.Done()
				for i := worker; i < accounts; i += workers {
					account := list[i]
					account.Chars[0].Gold++
					state := &model.CharState{Version: model.CharStateVersion}
					if err := st.SavePlayerStateBatch(nil, account.Chars[0].UID, state, account); err != nil {
						failed.Add(1)
					}
				}
			}()
		}
		wg.Wait()
		t.Logf("workers=%d writes=%d failed=%d retries=%d elapsed=%s", workers, accounts, failed.Load(),
			metricPostgresTxRetries.Value()-retriesBefore, time.Since(started).Round(time.Millisecond))
	}
}
