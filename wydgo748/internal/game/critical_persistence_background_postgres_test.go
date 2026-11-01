package game

import (
	"context"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// W7 on PostgreSQL: a row lock holds the autosave in its transaction. A newer
// write of the same account parks, starts after the snapshot commits, and the
// durable row ends with the newer state.
func TestCriticalBackgroundAutosaveOrderPostgres(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	ctx := context.Background()
	st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 4})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = conn.Close(ctx) })

	p, _ := networkedTestPlayer(1, "ParkedPG", 2100, 2100)
	suffix := time.Now().UTC().Format("150405000")
	p.Account.Name, p.Char.Name, p.Char.UID = "parkedpg"+suffix, "ParkedPG"+suffix[len(suffix)-4:], ""
	key := strings.ToLower(p.Account.Name)
	t.Cleanup(func() { _, _ = conn.Exec(ctx, `DELETE FROM accounts WHERE name_key=$1`, key) })
	if err := st.CreateAccount(p.Account); err != nil {
		t.Fatal(err)
	}
	w, err := NewWorld(st, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{}, WithCriticalPersistence(2, 16), WithCriticalAccountContainment())
	if err != nil {
		t.Fatal(err)
	}
	w.players[p.Session], w.playersByID[p.ID] = p, p
	w.updatePlayerSpatial(p)
	claimCriticalTestPlayers(t, w, p)
	queue := w.criticalWrites.queue

	lock, err := conn.Begin(ctx)
	if err != nil {
		t.Fatal(err)
	}
	defer func() { _ = lock.Rollback(ctx) }()
	if _, err := lock.Exec(ctx, `SELECT 1 FROM accounts WHERE name_key=$1 FOR UPDATE`, key); err != nil {
		t.Fatal(err)
	}
	p.Char.Gold = 111
	if err := w.queueAccountAutoSave(p, w.now()); err != nil {
		t.Fatal(err)
	}
	time.Sleep(150 * time.Millisecond) // let the snapshot reach the locked row
	if w.criticalPlayerReserved(p) {
		t.Fatal("autosave held gameplay input")
	}
	p.Char.Gold = 222
	parked, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{accountPersistenceSnapshot(p.Account)}},
		[]*Player{p}, func(result criticalPersistenceResult) error { return result.err })
	if err != nil {
		t.Fatalf("write behind autosave: %v", err)
	}
	if queue.pending[parked].parked == nil || len(queue.results) != 0 {
		t.Fatal("newer write was not parked behind the held autosave")
	}
	if err := lock.Rollback(ctx); err != nil {
		t.Fatal(err)
	}
	deadline := time.Now().Add(10 * time.Second)
	for len(queue.pending) > 0 && time.Now().Before(deadline) {
		w.processCriticalCompletions(criticalCompletionBatchLimit)
		time.Sleep(5 * time.Millisecond)
	}
	if len(queue.pending) != 0 || p.PersistencePoisoned || w.criticalPlayerWriteReserved(p) {
		t.Fatalf("writes did not settle: pending=%d poisoned=%t", len(queue.pending), p.PersistencePoisoned)
	}
	loaded, err := st.LoadAccount(p.Account.Name)
	if err != nil || loaded == nil {
		t.Fatalf("durable load: %v", err)
	}
	if loaded.Chars[0].Gold != 222 {
		t.Fatalf("durable gold %d, want the newer 222", loaded.Chars[0].Gold)
	}
}
