package store

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

func TestApplyPostgresSchemaUpgradesVersion14(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated test database")
	}
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()

	schema := fmt.Sprintf("wyd_upgrade_%x", time.Now().UnixNano())
	adminConfig, err := pgx.ParseConfig(url)
	if err != nil {
		t.Fatal("could not parse isolated test database configuration")
	}
	admin, err := pgx.ConnectConfig(ctx, adminConfig)
	if err != nil {
		t.Fatal("could not connect to isolated test database")
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
		t.Fatal("could not parse isolated pool configuration")
	}
	poolConfig.ConnConfig.RuntimeParams["search_path"] = schema
	poolConfig.MaxConns = 2
	pool, err := pgxpool.NewWithConfig(ctx, poolConfig)
	if err != nil {
		t.Fatal("could not open isolated migration schema")
	}
	defer pool.Close()

	_, err = pool.Exec(ctx, `
		CREATE TABLE schema_migrations (version integer PRIMARY KEY);
		INSERT INTO schema_migrations(version) VALUES (14);
		CREATE TABLE instance_state (singleton boolean PRIMARY KEY, payload jsonb NOT NULL);
		CREATE TABLE guild_state (singleton boolean PRIMARY KEY, payload jsonb NOT NULL);`)
	if err != nil {
		t.Fatalf("prepare version-14 migration fixtures: %v", err)
	}
	if err := applyPostgresSchema(ctx, pool); err != nil {
		t.Fatalf("upgrade schema v14: %v", err)
	}

	var version, instances, channels, guildRows, guildHeaders int
	if err := pool.QueryRow(ctx, `SELECT MAX(version) FROM schema_migrations`).Scan(&version); err != nil {
		t.Fatal(err)
	}
	if err := pool.QueryRow(ctx, `SELECT (SELECT count(*) FROM instance_runtime_state),
		(SELECT count(*) FROM instance_channel_state), (SELECT count(*) FROM guild_rows),
		(SELECT count(*) FROM guild_registry_state)`).Scan(&instances, &channels, &guildRows, &guildHeaders); err != nil {
		t.Fatal(err)
	}
	if version != postgresSchemaVersion || instances != 0 || channels != 0 || guildRows != 0 || guildHeaders != 0 {
		t.Fatalf("upgrade result: version=%d runtime=%d channels=%d guilds=%d headers=%d",
			version, instances, channels, guildRows, guildHeaders)
	}
}
