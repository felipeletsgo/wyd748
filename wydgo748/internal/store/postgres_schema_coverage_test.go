package store

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"

	"wydgo/internal/model"
)

func TestApplyPostgresSchemaFreshAndCurrentVersion(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated test database")
	}
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()

	schema := fmt.Sprintf("wyd_coverage_%x", time.Now().UnixNano())
	adminConfig, err := pgx.ParseConfig(url)
	if err != nil {
		t.Fatal("could not parse test database configuration")
	}
	admin, err := pgx.ConnectConfig(ctx, adminConfig)
	if err != nil {
		t.Fatal("could not connect to test database")
	}
	defer admin.Close(context.Background())
	identifier := pgx.Identifier{schema}.Sanitize()
	if _, err := admin.Exec(ctx, "CREATE SCHEMA "+identifier); err != nil {
		t.Fatalf("create isolated migration schema: %v", err)
	}
	t.Cleanup(func() {
		_, _ = admin.Exec(context.Background(), "DROP SCHEMA "+identifier+" CASCADE")
	})

	poolConfig, err := pgxpool.ParseConfig(url)
	if err != nil {
		t.Fatal("could not parse pool configuration")
	}
	poolConfig.ConnConfig.RuntimeParams["search_path"] = schema
	poolConfig.MaxConns = 2
	pool, err := pgxpool.NewWithConfig(ctx, poolConfig)
	if err != nil {
		t.Fatal("could not open isolated schema pool")
	}
	defer pool.Close()
	if err := applyPostgresSchema(ctx, pool); err != nil {
		t.Fatalf("apply fresh schema migrations: %v", err)
	}
	var version int
	if err := pool.QueryRow(ctx, "SELECT COALESCE(MAX(version),0) FROM schema_migrations").Scan(&version); err != nil {
		t.Fatal(err)
	}
	if version != postgresSchemaVersion {
		t.Fatalf("schema version=%d, want %d", version, postgresSchemaVersion)
	}
	st := newPostgresStore(pool, PostgresConfig{})
	state := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
		NightmarePartyRuns: map[string]int{"coverage-window": 2}, GambleJackpot: 7}
	if err := st.SaveInstanceState(state); err != nil {
		t.Fatalf("save instance state: %v", err)
	}
	loaded, err := st.LoadInstanceState()
	if err != nil || loaded.NightmarePartyRuns["coverage-window"] != 2 || loaded.GambleJackpot != 7 {
		t.Fatalf("loaded instance state=%+v err=%v", loaded, err)
	}
	state.GambleJackpot = 9
	if err := st.SaveGameStateWithInstanceState(nil, state); err != nil {
		t.Fatalf("save combined game state: %v", err)
	}
	if err := applyPostgresSchema(ctx, pool); err != nil {
		t.Fatalf("reapplying current schema failed: %v", err)
	}
	if _, err := pool.Exec(ctx, "INSERT INTO schema_migrations(version) VALUES ($1)", postgresSchemaVersion+1); err != nil {
		t.Fatal(err)
	}
	if err := applyPostgresSchema(ctx, pool); err == nil {
		t.Fatal("newer schema version was accepted")
	}
}
