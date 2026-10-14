package playerportal

import (
	"context"
	"testing"

	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/store"
)

// Called only by the production-refusing disposable database integration gate.
func testProgressionSchemaUpgrade(t *testing.T, owner *pgxpool.Pool, ownerURL string) {
	t.Helper()
	ctx := context.Background()
	var before, after string
	const snapshotSQL = `SELECT jsonb_agg(to_jsonb(a) ORDER BY name_key)::text FROM accounts a`
	if err := owner.QueryRow(ctx, snapshotSQL).Scan(&before); err != nil {
		t.Fatal(err)
	}
	// Reconstruct the deployed v6 state. A tripwire forbids any historical
	// account rewrite while the additive progression migration executes.
	_, err := owner.Exec(ctx, `DROP VIEW public.player_character_progression;
DROP VIEW public.player_public_guilds;
DELETE FROM schema_migrations WHERE version>=7;
CREATE FUNCTION public.validation_no_account_rewrite() RETURNS trigger LANGUAGE plpgsql AS $$
BEGIN RAISE EXCEPTION 'Upgrade must not rewrite accounts'; END $$;
CREATE TRIGGER validation_no_account_rewrite BEFORE INSERT OR UPDATE OR DELETE ON accounts
FOR EACH ROW EXECUTE FUNCTION public.validation_no_account_rewrite();`)
	if err != nil {
		t.Fatal(err)
	}
	for attempt := 0; attempt < 2; attempt++ {
		writer, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: ownerURL, MaxConns: 2})
		if err != nil {
			t.Fatalf("v6 upgrade or v8 repeated startup: %v", err)
		}
		writer.Close()
		var version int
		var viewExists bool
		if err := owner.QueryRow(ctx, `SELECT max(version), to_regclass('public.player_character_progression') IS NOT NULL AND to_regclass('public.player_public_guilds') IS NOT NULL FROM schema_migrations`).Scan(&version, &viewExists); err != nil || version != 8 || !viewExists {
			t.Fatalf("progression migration not installed: version=%d view=%v error=%v", version, viewExists, err)
		}
	}
	if err := owner.QueryRow(ctx, snapshotSQL).Scan(&after); err != nil || before != after {
		t.Fatalf("migration changed account payload, version or timestamps: %v", err)
	}
	if _, err := owner.Exec(ctx, `INSERT INTO schema_migrations(version) VALUES(9)`); err != nil {
		t.Fatal(err)
	}
	future, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: ownerURL, MaxConns: 2})
	if err == nil {
		future.Close()
		t.Fatal("future schema was accepted")
	}
	var version int
	if err := owner.QueryRow(ctx, `SELECT max(version) FROM schema_migrations`).Scan(&version); err != nil || version != 9 {
		t.Fatalf("future schema was altered: version=%d error=%v", version, err)
	}
	if _, err := owner.Exec(ctx, `DELETE FROM schema_migrations WHERE version=9;
DROP TRIGGER validation_no_account_rewrite ON accounts;
DROP FUNCTION public.validation_no_account_rewrite();
GRANT SELECT ON public.player_character_progression, public.player_public_guilds TO wyd_player;`); err != nil {
		t.Fatal(err)
	}
}
