package game

import (
	"context"
	"os"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestGuildCityAreaPostgresRecallCommitsBeforePublication(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(context.Background(), url)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = conn.Close(context.Background()) })
	w, p, fallback := handlerTestWorld(t)
	w.terrain = guildCityTerrain()
	setGuildCityOwner(w, 0, 17)
	p.Account.Name = "guildareapg" + time.Now().UTC().Format("150405.000000000")
	p.Char.Name, p.Char.UID = "GuildAreaPG", ""
	p.Char.Gold, p.Char.Inv[0] = 12345, model.Item{Index: 900}
	p.X, p.Y, p.Char.X, p.Char.Y = 2092, 2147, 2092, 2147
	w.updatePlayerSpatial(p)
	t.Cleanup(func() {
		_, _ = conn.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(p.Account.Name))
	})
	if err := st.CreateAccount(p.Account); err != nil {
		t.Fatal(err)
	}
	w.indexPlayerCharacter(p)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("could not claim the isolated account")
	}
	gate, release := criticalGate(t)
	backend := &criticalHeldPostgresStore{PostgresStore: st, entered: make(chan criticalWrite, 2), release: gate}
	q := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	w.criticalAccountContainment = true
	before := accountStateSnapshot(p.Account)
	w.tickGuildCityAreas(w.now())
	awaitCriticalWrite(t, backend.entered)
	w.tickGuildCityAreas(w.now())
	if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || !w.criticalPlayerReserved(p) || len(backend.entered) != 0 {
		t.Fatal("the private area guard exposed or duplicated a pending database recall")
	}
	release()
	result := awaitCriticalResult(t, q)
	if result.err != nil {
		t.Fatal(result.err)
	}
	if p.X != 2092 || p.Y != 2147 {
		t.Fatal("database completion changed coordinates before actor acknowledgement")
	}
	w.applyCriticalCompletion(result)
	durable, err := st.LoadAccount(p.Account.Name)
	if err != nil || durable == nil {
		t.Fatalf("load committed recall: %v", err)
	}
	ch := &durable.Chars[p.CharSlot]
	// Session coordinates remain volatile. Durable entry uses the hometown.
	entry := &accountPersistenceSnapshot(p.Account).Chars[p.CharSlot]
	if ch.X != entry.X || ch.Y != entry.Y || w.terrain.GuildArea(p.X, p.Y) ||
		ch.Gold != before.Chars[p.CharSlot].Gold || ch.Inv != before.Chars[p.CharSlot].Inv ||
		playerHomeCity(ch) != playerHomeCity(&before.Chars[p.CharSlot]) ||
		w.criticalPlayerReserved(p) || p.PersistencePoisoned || p.Session.IsClosed() || fallback.saves != 0 {
		t.Fatalf("durable recall: position=%d,%d live=%d,%d private=%v gold=%d/%d inventoryEqual=%v hometown=%d/%d reserved=%v poisoned=%v closed=%v inlineSaves=%d",
			ch.X, ch.Y, p.X, p.Y, w.terrain.GuildArea(ch.X, ch.Y), ch.Gold, before.Chars[p.CharSlot].Gold,
			ch.Inv == before.Chars[p.CharSlot].Inv, playerHomeCity(ch), playerHomeCity(&before.Chars[p.CharSlot]),
			w.criticalPlayerReserved(p), p.PersistencePoisoned, p.Session.IsClosed(), fallback.saves)
	}
	w.tickGuildCityAreas(w.now())
	if len(backend.entered) != 0 || w.criticalPlayerReserved(p) {
		t.Fatal("the public return caused another database recall")
	}
}
