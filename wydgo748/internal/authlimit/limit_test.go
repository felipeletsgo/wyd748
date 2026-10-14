package authlimit_test

import (
	"context"
	"fmt"
	"os"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/authlimit"
	"wydgo/internal/store"
)

func TestPolicyGroupsAndRedactsIdentifiers(t *testing.T) {
	a := authlimit.Policy("player_login", "2001:db8::1", "Player1")
	b := authlimit.Policy("player_login", "2001:db8::ffff", "PLAYER1")
	for i := range a {
		if a[i] != b[i] || strings.Contains(a[i].Key, "Player1") || strings.Contains(a[i].Key, "2001:db8") {
			t.Fatal("quota identity leaked or rotation bypassed")
		}
	}
	c := authlimit.Policy("staff_login", "2001:db8::1", "Player1")
	if a[0].Key == c[0].Key {
		t.Fatal("staff and player quotas collide")
	}
	limits := authlimit.Policy("registration", "192.0.2.1", "")
	if limits[3].Maximum != 5 || limits[3].Window != time.Hour {
		t.Fatal("registration quota changed")
	}
	if _, err := (authlimit.Postgres{}).Admit(context.Background(), a); err == nil {
		t.Fatal("missing storage failed open")
	}
}

func TestDurableQuotaIntegration(t *testing.T) {
	raw := os.Getenv("WYD_TEST_PLAYER_OWNER_URL")
	if raw == "" {
		t.Skip("dedicated authentication database fixture is not configured")
	}
	cfg, err := pgxpool.ParseConfig(raw)
	if err != nil || cfg.ConnConfig.Database != "wyd_portal_validation" {
		t.Fatal("integration requires the dedicated wyd_portal_validation database")
	}
	ctx := context.Background()
	writer, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: raw, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	writer.Close()
	pool, err := pgxpool.New(ctx, raw)
	if err != nil {
		t.Fatal(err)
	}
	defer pool.Close()
	prefix := fmt.Sprintf("validation:%d:", time.Now().UnixNano())
	defer pool.Exec(ctx, `DELETE FROM web_auth_limits WHERE key LIKE $1`, prefix+"%")
	quota := []authlimit.Limit{{Key: prefix + "0global", Window: time.Hour, Maximum: 100}, {Key: prefix + "1account", Window: time.Hour, Maximum: 5}}
	var admitted atomic.Int32
	var wg sync.WaitGroup
	for i := 0; i < 20; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			wait, err := (authlimit.Postgres{Pool: pool}).Admit(ctx, quota)
			if err != nil {
				t.Error(err)
			} else if wait == 0 {
				admitted.Add(1)
			}
		}()
	}
	wg.Wait()
	if admitted.Load() != 5 {
		t.Fatalf("concurrent quota admitted %d", admitted.Load())
	}
	var count int
	if err := pool.QueryRow(ctx, `SELECT attempts FROM web_auth_limits WHERE key=$1`, quota[0].Key).Scan(&count); err != nil || count != 5 {
		t.Fatalf("rejected attempts burned global budget: %d %v", count, err)
	}
	// A new connection pool models a process restart without resetting quotas.
	restarted, err := pgxpool.New(ctx, raw)
	if err != nil {
		t.Fatal(err)
	}
	defer restarted.Close()
	if wait, err := (authlimit.Postgres{Pool: restarted}).Admit(ctx, quota); err != nil || wait <= 0 {
		t.Fatalf("restart reset quota: %v %v", wait, err)
	}
	if _, err := pool.Exec(ctx, `UPDATE web_auth_limits SET expires_at=now()-interval '1 second' WHERE key LIKE $1`, prefix+"%"); err != nil {
		t.Fatal(err)
	}
	if wait, err := (authlimit.Postgres{Pool: restarted}).Admit(ctx, quota); err != nil || wait != 0 {
		t.Fatalf("expired quota did not recover: %v %v", wait, err)
	}
	cancelled, cancel := context.WithCancel(ctx)
	cancel()
	if _, err := (authlimit.Postgres{Pool: pool}).Admit(cancelled, quota); err == nil {
		t.Fatal("cancelled database call failed open")
	}
}
