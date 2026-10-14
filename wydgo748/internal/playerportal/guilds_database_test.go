package playerportal

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Invoked only inside the dedicated production-refusing database fixture.
func testSavedPublicGuildsDatabase(t *testing.T, ctx context.Context, writer *store.PostgresStore, owner *pgxpool.Pool, db *Database, ownerURL string) {
	t.Helper()
	empty, err := db.PublicGuilds(ctx)
	if err != nil || len(empty.Guilds) != 0 {
		t.Fatalf("absent registry: %+v %v", empty, err)
	}
	fixture := publicGuildFixture()
	registry := model.GuildRegistry{Version: 1}
	for _, saved := range fixture.Guilds {
		guild := model.Guild{ID: saved.ID, Name: saved.Name, Kingdom: saved.Kingdom, Fame: saved.Fame, Ally: saved.Ally, WarTarget: saved.WarTarget}
		for _, member := range saved.Members {
			guild.Members = append(guild.Members, model.GuildMember{Character: member.Name, Rank: member.Rank, Account: "PrivateOwner", JoinedAt: time.Now().UTC()})
		}
		registry.Guilds = append(registry.Guilds, guild)
	}
	if err := writer.SaveGameState(&registry); err != nil {
		t.Fatal(err)
	}
	snapshot, err := db.PublicGuilds(ctx)
	if err != nil || len(snapshot.Guilds) != 2 || snapshot.Guilds[0].Fame != 4294967295 {
		t.Fatalf("canonical snapshot: %+v %v", snapshot, err)
	}
	var savedTime time.Time
	if err := owner.QueryRow(ctx, `SELECT updated_at FROM guild_state`).Scan(&savedTime); err != nil || !snapshot.AsOf.Equal(savedTime) {
		t.Fatal("snapshot timestamp mismatch", err)
	}
	var publicJSON string
	if err := db.pool.QueryRow(ctx, `SELECT projection::text FROM player_public_guilds`).Scan(&publicJSON); err != nil {
		t.Fatal(err)
	}
	for _, key := range []string{"PrivateOwner", "account", "joinedAt", "wars"} {
		if strings.Contains(publicJSON, key) {
			t.Fatal("SQL projection exposed private data", key)
		}
	}
	for _, query := range []string{`SELECT payload FROM guild_state`, `UPDATE player_public_guilds SET projection='{}'`, `DELETE FROM guild_state`} {
		if _, err := db.pool.Exec(ctx, query); err == nil {
			t.Fatal("portal role escaped projection", query)
		}
	}

	// Reconstruct v7. Tripwires forbid writes to either authoritative table.
	var before, after string
	const stateSQL = `SELECT jsonb_build_object('accounts',(SELECT jsonb_agg(to_jsonb(a) ORDER BY name_key) FROM accounts a),'guild',(SELECT to_jsonb(g) FROM guild_state g))::text`
	if err := owner.QueryRow(ctx, stateSQL).Scan(&before); err != nil {
		t.Fatal(err)
	}
	if _, err := owner.Exec(ctx, `DROP VIEW public.player_public_guilds; DELETE FROM schema_migrations WHERE version=8;
 CREATE FUNCTION validation_no_guild_upgrade_write() RETURNS trigger LANGUAGE plpgsql AS $$ BEGIN RAISE EXCEPTION 'Upgrade must not rewrite authoritative data'; END $$;
 CREATE TRIGGER validation_no_guild_upgrade_write BEFORE INSERT OR UPDATE OR DELETE ON guild_state FOR EACH ROW EXECUTE FUNCTION validation_no_guild_upgrade_write();
 CREATE TRIGGER validation_no_guild_upgrade_write BEFORE INSERT OR UPDATE OR DELETE ON accounts FOR EACH ROW EXECUTE FUNCTION validation_no_guild_upgrade_write();`); err != nil {
		t.Fatal(err)
	}
	for i := 0; i < 2; i++ {
		upgraded, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: ownerURL, MaxConns: 2})
		if err != nil {
			t.Fatal(err)
		}
		upgraded.Close()
	}
	if err := owner.QueryRow(ctx, stateSQL).Scan(&after); err != nil || before != after {
		t.Fatal("v7 migration rewrote authoritative state", err)
	}
	if _, err := owner.Exec(ctx, `DROP TRIGGER validation_no_guild_upgrade_write ON guild_state; DROP TRIGGER validation_no_guild_upgrade_write ON accounts; DROP FUNCTION validation_no_guild_upgrade_write(); GRANT SELECT ON public.player_public_guilds TO wyd_player;`); err != nil {
		t.Fatal(err)
	}
	raw, err := json.Marshal(registry)
	if err != nil {
		t.Fatal(err)
	}
	for name, expression := range map[string]string{
		"version":         `jsonb_set($1::jsonb,'{version}','2')`,
		"fractional fame": `jsonb_set($1::jsonb,'{guilds,0,fame}','1.5')`,
		"overflow fame":   `jsonb_set($1::jsonb,'{guilds,0,fame}','4294967296')`,
		"invalid rank":    `jsonb_set($1::jsonb,'{guilds,0,members,0,rank}','5')`,
		"missing ally":    `jsonb_set($1::jsonb,'{guilds,0,ally}','4095')`,
		"scalar members":  `jsonb_set($1::jsonb,'{guilds,0,members}','true')`,
		"oversized":       `jsonb_set($1::jsonb,'{private}',to_jsonb(repeat('x',8388609)))`,
	} {
		t.Run("public guild "+name, func(t *testing.T) {
			if _, err := owner.Exec(ctx, `UPDATE guild_state SET payload=`+expression, string(raw)); err != nil {
				t.Fatal(err)
			}
			if _, err := db.PublicGuilds(ctx); err == nil {
				t.Fatal("corrupt public guild snapshot accepted")
			}
		})
	}
	if err := writer.SaveGameState(&registry); err != nil {
		t.Fatal(err)
	}
	if _, err := db.PublicGuilds(ctx); err != nil {
		t.Fatal("valid snapshot did not recover", err)
	}
	cancelled, cancel := context.WithCancel(ctx)
	cancel()
	if _, err := db.PublicGuilds(cancelled); err == nil {
		t.Fatal("cancelled query succeeded")
	}
}
