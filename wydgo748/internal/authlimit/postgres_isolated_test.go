package authlimit_test

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/authlimit"
)

func TestPostgresLimiterReservesQuotasAtomicallyInIsolatedSchema(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	schema := fmt.Sprintf("authlimit_test_%d", time.Now().UnixNano())
	if _, err := conn.Exec(ctx, "CREATE SCHEMA "+schema); err != nil {
		conn.Close(context.Background())
		t.Fatal(err)
	}
	defer func() {
		_, _ = conn.Exec(context.Background(), "DROP SCHEMA "+schema+" CASCADE")
		_ = conn.Close(context.Background())
	}()
	if _, err := conn.Exec(ctx, "SET search_path TO "+schema); err != nil {
		t.Fatal(err)
	}
	if _, err := conn.Exec(ctx, `CREATE TABLE web_auth_limits(
		key text PRIMARY KEY, attempts integer NOT NULL, expires_at timestamptz NOT NULL)`); err != nil {
		t.Fatal(err)
	}
	cfg, err := pgxpool.ParseConfig(url)
	if err != nil {
		t.Fatal(err)
	}
	cfg.ConnConfig.RuntimeParams["search_path"] = schema
	pool, err := pgxpool.NewWithConfig(ctx, cfg)
	if err != nil {
		t.Fatal(err)
	}
	defer pool.Close()
	limiter := authlimit.Postgres{Pool: pool}
	quota := []authlimit.Limit{
		{Key: "global", Window: time.Hour, Maximum: 10},
		{Key: "account", Window: time.Hour, Maximum: 1},
	}
	if wait, err := limiter.Admit(ctx, quota); err != nil || wait != 0 {
		t.Fatalf("first reservation wait=%v err=%v", wait, err)
	}
	if wait, err := limiter.Admit(ctx, quota); err != nil || wait <= 0 {
		t.Fatalf("exhausted quota wait=%v err=%v", wait, err)
	}
	var globalAttempts, accountAttempts int
	if err := pool.QueryRow(ctx, `SELECT attempts FROM web_auth_limits WHERE key='global'`).Scan(&globalAttempts); err != nil {
		t.Fatal(err)
	}
	if err := pool.QueryRow(ctx, `SELECT attempts FROM web_auth_limits WHERE key='account'`).Scan(&accountAttempts); err != nil {
		t.Fatal(err)
	}
	if globalAttempts != 1 || accountAttempts != 1 {
		t.Fatalf("rejected reservation burned shared quota: global=%d account=%d", globalAttempts, accountAttempts)
	}
	if _, err := pool.Exec(ctx, `UPDATE web_auth_limits SET expires_at=now()-interval '1 second' WHERE key='account'`); err != nil {
		t.Fatal(err)
	}
	if _, err := pool.Exec(ctx, `INSERT INTO web_auth_limits(key,attempts,expires_at) VALUES('expired',1,now()-interval '1 second')`); err != nil {
		t.Fatal(err)
	}
	if wait, err := limiter.Admit(ctx, quota); err != nil || wait != 0 {
		t.Fatalf("expired quota did not recover: wait=%v err=%v", wait, err)
	}
	if err := pool.QueryRow(ctx, `SELECT attempts FROM web_auth_limits WHERE key='account'`).Scan(&accountAttempts); err != nil || accountAttempts != 1 {
		t.Fatalf("expired quota attempts=%d err=%v", accountAttempts, err)
	}
	var expiredRows int
	if err := pool.QueryRow(ctx, `SELECT count(*) FROM web_auth_limits WHERE key='expired'`).Scan(&expiredRows); err != nil || expiredRows != 0 {
		t.Fatalf("expired quota cleanup count=%d err=%v", expiredRows, err)
	}

	cancelled, stop := context.WithCancel(ctx)
	stop()
	if _, err := limiter.Admit(cancelled, quota); err == nil {
		t.Fatal("cancelled PostgreSQL admission succeeded")
	}
}
