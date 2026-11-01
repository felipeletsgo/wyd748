package store

import (
	"context"
	"encoding/json"
	"testing"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

// W8 guild rows. Each test runs in a rolled-back transaction with guild IDs
// that other fixtures do not use.

func guildRowsTx(t *testing.T) (pgx.Tx, context.Context) {
	t.Helper()
	_, conn := instanceRowsStore(t) // applies migrations and skips without a database
	ctx := context.Background()
	tx, err := conn.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.ReadCommitted})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = tx.Rollback(ctx) })
	return tx, ctx
}

func guildRowFame(t *testing.T, tx pgx.Tx, ctx context.Context, id uint16) (int, bool) {
	t.Helper()
	var payload []byte
	err := tx.QueryRow(ctx, `SELECT payload FROM guild_rows WHERE guild_id=$1`, int32(id)).Scan(&payload)
	if err == pgx.ErrNoRows {
		return 0, false
	}
	if err != nil {
		t.Fatal(err)
	}
	var guild model.Guild
	if err := json.Unmarshal(payload, &guild); err != nil {
		t.Fatal(err)
	}
	return int(guild.Fame), true
}

func TestGuildRowsKeyedWriteTouchesOnlyDeclaredRows(t *testing.T) {
	tx, ctx := guildRowsTx(t)
	const a, b = 60001, 60002
	registry := func(fameA, fameB uint32, withB bool) *model.GuildRegistry {
		r := &model.GuildRegistry{Version: model.GuildRegistryVersion,
			Guilds: []model.Guild{{ID: a, Name: "W8A", Fame: fameA}}}
		if withB {
			r.Guilds = append(r.Guilds, model.Guild{ID: b, Name: "W8B", Fame: fameB})
		}
		return r
	}
	if err := writeGuildRows(ctx, tx, registry(1, 1, true), []GuildParticipant{{GuildID: a}, {GuildID: b}}); err != nil {
		t.Fatal(err)
	}
	// A stale copy of guild B travels with a write that declares only A.
	if err := writeGuildRows(ctx, tx, registry(2, 9, true), []GuildParticipant{{GuildID: a}}); err != nil {
		t.Fatal(err)
	}
	if fame, _ := guildRowFame(t, tx, ctx, a); fame != 2 {
		t.Fatalf("declared guild fame %d", fame)
	}
	if fame, _ := guildRowFame(t, tx, ctx, b); fame != 1 {
		t.Fatalf("undeclared guild was overwritten: fame %d", fame)
	}
	if err := writeGuildRows(ctx, tx, registry(2, 0, false), []GuildParticipant{{GuildID: b}}); err != nil {
		t.Fatal(err)
	}
	if _, present := guildRowFame(t, tx, ctx, b); present {
		t.Fatal("declared removal kept the guild row")
	}
}

func TestGuildRowsMigrationRebuildsSingleton(t *testing.T) {
	tx, ctx := guildRowsTx(t)
	original := &model.GuildRegistry{Version: model.GuildRegistryVersion, FameResetDay: "2026-11-01",
		Guilds: []model.Guild{{ID: 7, Name: "Seven", Fame: 70, Ally: 3}, {ID: 3, Name: "Three", Fame: 30, Ally: 7}}}
	payload, err := json.Marshal(original)
	if err != nil {
		t.Fatal(err)
	}
	for _, statement := range []string{`DELETE FROM guild_rows`, `DELETE FROM guild_registry_state`} {
		if _, err := tx.Exec(ctx, statement); err != nil {
			t.Fatal(err)
		}
	}
	if _, err := tx.Exec(ctx, `INSERT INTO guild_state(singleton,payload) VALUES(true,$1)
		ON CONFLICT(singleton) DO UPDATE SET payload=EXCLUDED.payload`, payload); err != nil {
		t.Fatal(err)
	}
	if err := migrateGuildRows(ctx, tx); err != nil {
		t.Fatalf("migration: %v", err)
	}
	rebuilt, ok, err := loadGuildRows(ctx, tx)
	if err != nil || !ok || !equalGuildRegistries(original, rebuilt) || len(rebuilt.Guilds) != 2 || rebuilt.Guilds[0].ID != 3 {
		t.Fatalf("rebuilt registry differs: %+v ok=%t err=%v", rebuilt, ok, err)
	}
	if err := migrateGuildRows(ctx, tx); err == nil {
		t.Fatal("a second migration merged into existing rows")
	}
}
