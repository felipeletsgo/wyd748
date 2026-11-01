package store

import (
	"context"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
)

// W8 and W9 migration dry run on a real database. Set WYD_MIGRATION_DRYRUN_ENV to
// the name of an environment variable that holds the database URL. The
// migrations run in one transaction that is always rolled back.
func TestParticipantMigrationDryRun(t *testing.T) {
	name := os.Getenv("WYD_MIGRATION_DRYRUN_ENV")
	url := ""
	if name != "" {
		url = os.Getenv(name)
	}
	if url == "" {
		t.Skip("set WYD_MIGRATION_DRYRUN_ENV to the variable holding the database URL")
	}
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal("connect failed") // never print the URL
	}
	defer conn.Close(context.Background())
	var version int
	if err := conn.QueryRow(ctx, `SELECT COALESCE(MAX(version),0) FROM schema_migrations`).Scan(&version); err != nil {
		t.Fatal(err)
	}
	tx, err := conn.Begin(ctx)
	if err != nil {
		t.Fatal(err)
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	started := time.Now()
	if version < 15 {
		if err := migrateInstanceRows(ctx, tx); err != nil {
			t.Fatalf("instance migration: %v", err)
		}
	}
	if version < 16 {
		if err := migrateGuildRows(ctx, tx); err != nil {
			t.Fatalf("guild migration: %v", err)
		}
	}
	// W9: lease, ticket and message tables, and the unique guild name index,
	// which fails here if two stored guilds share a name.
	for _, step := range []struct {
		version int
		ddl     string
	}{{17, postgresAccountLeasesSchema}, {18, postgresChannelTicketsSchema},
		{19, postgresChannelMessagesSchema}, {20, postgresGuildNamesSchema}} {
		if version < step.version {
			if _, err := tx.Exec(ctx, step.ddl); err != nil {
				t.Fatalf("v%d migration: %v", step.version, err)
			}
		}
	}
	var runtimes, channels, guilds, headers int
	_ = tx.QueryRow(ctx, `SELECT (SELECT count(*) FROM instance_runtime_state), (SELECT count(*) FROM instance_channel_state),
		(SELECT count(*) FROM guild_rows), (SELECT count(*) FROM guild_registry_state)`).Scan(&runtimes, &channels, &guilds, &headers)
	t.Logf("dry run from schema v%d: %d runtime rows, %d channel rows, %d guild rows, %d registry rows, %s; rolled back",
		version, runtimes, channels, guilds, headers, time.Since(started).Round(time.Millisecond))
}
