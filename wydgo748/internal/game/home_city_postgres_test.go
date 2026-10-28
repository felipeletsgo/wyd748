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

func TestCriticalHomeCityPostgresMovementAndRecall(t *testing.T) {
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
	p.Account.Name = "citypg" + time.Now().UTC().Format("150405.000000000")
	p.Char.Name, p.Char.UID = "CityPG", ""
	p.Char.Score.Merchant, p.Char.RuntimeScore.Merchant = 5, 5
	p.Char.Inv[0] = model.Item{Index: 900}
	w.items[900] = model.ItemDef{Index: 900}
	w.volatiles = model.VolatileCatalog{Rules: map[int]model.VolatileRule{11: {Action: "recall", Consume: true}},
		ItemCodes: map[uint16]int{900: 11}}
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
	p.X, p.Y, p.Char.X, p.Char.Y = 3499, 3105, 3499, 3105
	w.updatePlayerSpatial(p)
	now, route := w.now(), []byte("66")
	w.beginPlayerMovement(p, p.X, p.Y, 3501, 3105, route, route, 0, now)
	w.advancePlayerMovement(p, now.Add(2*time.Second))
	awaitCriticalWrite(t, backend.entered)
	if playerHomeCity(p.Char) != 0 || !w.criticalPlayerReserved(p) {
		t.Fatal("city binding became visible before database acknowledgement")
	}
	release()
	result := awaitCriticalResult(t, q)
	if result.err != nil {
		t.Fatal(result.err)
	}
	w.applyCriticalCompletion(result)
	durable, err := st.LoadAccount(p.Account.Name)
	if err != nil || durable == nil || playerHomeCity(&durable.Chars[0]) != 3 ||
		playerHomeCity(p.Char) != 3 || p.Session.IsClosed() || p.PersistencePoisoned {
		t.Fatalf("walking did not durably bind Nippleheim without disconnect: %v", err)
	}
	p.X, p.Y, p.Char.X, p.Char.Y = 2200, 2200, 2200, 2200
	w.updatePlayerSpatial(p)
	before := accountStateSnapshot(p.Account)
	w.onUseItem(p.Session, useItemPacket(0, 0))
	awaitCriticalWrite(t, backend.entered)
	result = awaitCriticalResult(t, q)
	if result.err != nil {
		t.Fatal(result.err)
	}
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.X != 2200 || p.Y != 2200 {
		t.Fatal("scroll consumption or movement bypassed actor acknowledgement")
	}
	w.applyCriticalCompletion(result)
	durable, err = st.LoadAccount(p.Account.Name)
	if err != nil || durable == nil || durable.Chars[0].Inv[0].Index != 0 ||
		playerHomeCity(&durable.Chars[0]) != 3 || !recallArea(3).contains(p.X, p.Y) ||
		p.Char.Inv[0].Index != 0 || p.PersistencePoisoned || p.Session.IsClosed() ||
		w.criticalPlayerReserved(p) || fallback.saves != 0 {
		t.Fatalf("scroll did not commit its consumption and return to the bound city: %v", err)
	}
}
