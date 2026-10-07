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
	"wydgo/internal/wire"
)

// The production configuration end to end: NewWorld over the real PostgreSQL
// adapters, the actual actor loop, a durable tick producer, packet deferral,
// logout and shutdown. No gameplay write may run inline on World.
func TestCriticalActivatedWorldPostgresEndToEnd(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{URL: url, MaxConns: 4})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(context.Background(), url)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = conn.Close(context.Background()) })

	p, _ := networkedTestPlayer(1, "ActivePG", 2100, 2100)
	suffix := time.Now().UTC().Format("150405000")
	p.Account.Name, p.Char.Name, p.Char.UID = "activepg"+suffix, "ActivePG"+suffix[len(suffix)-4:], ""
	t.Cleanup(func() {
		_, _ = conn.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(p.Account.Name))
	})
	if err := st.CreateAccount(p.Account); err != nil {
		t.Fatal(err)
	}
	if p.Char.UID == "" {
		t.Fatal("PostgreSQL did not assign the character identity")
	}

	w, err := NewWorld(st, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{}, WithCriticalPersistence(2, 16), WithCriticalAccountContainment())
	if err != nil {
		t.Fatal(err)
	}
	rejectedBefore := metricSyncPersistenceRejected.Value()
	w.players[p.Session], w.playersByID[p.ID] = p, p
	w.updatePlayerSpatial(p)
	claimCriticalTestPlayers(t, w, p)
	p.NextRegen = w.now().Add(time.Hour)
	p.Char.CP, p.NextCPRecovery = -3, w.now().Add(-time.Second)
	w.nextAutoSave = w.now().Add(time.Hour)

	ticks, stop, stopped := make(chan time.Time), make(chan struct{}), make(chan struct{})
	go func() {
		defer close(stopped)
		w.run(ticks, stop)
	}()
	send := func(cmd command) {
		select {
		case w.commands <- cmd:
		case <-time.After(time.Second):
			t.Fatal("World did not accept a command")
		}
	}
	durableCP := func() int16 {
		loaded, err := st.LoadAccount(p.Account.Name)
		if err != nil || loaded == nil {
			t.Fatalf("durable load failed: %v", err)
		}
		return loaded.Chars[0].CP
	}
	deliverCriticalLoopTick(t, ticks)
	send(command{s: p.Session, pkt: criticalPKModePacket(true)})
	deadline := time.Now().Add(5 * time.Second)
	for durableCP() != -2 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	if durableCP() != -2 {
		t.Fatal("tick producer did not commit CP recovery through PostgreSQL")
	}
	send(command{s: p.Session, pkt: inboundPacket(wire.OpCharacterLogout, 12)})
	for time.Now().Before(deadline) {
		if overview := readCriticalLoopOverview(t, w); len(overview.Players) == 0 {
			break
		}
	}
	if !w.Shutdown(3 * time.Second) {
		t.Fatal("activated PostgreSQL shutdown did not complete")
	}
	close(stop)
	<-stopped
	if delta := metricSyncPersistenceRejected.Value() - rejectedBefore; delta != 0 || len(w.syncPersistenceRejected) != 0 {
		t.Fatalf("activated gameplay attempted %d unmigrated inline write(s): %v", delta, w.syncPersistenceRejected)
	}
	if p.InWorld || durableCP() != -2 || !w.shuttingDown {
		t.Fatal("activated PostgreSQL flow lost logout or durable state")
	}
	// Logout resets runtime PK state; the deferred packet's reply proves it ran.
	pkReplayed := false
	for {
		pkt, ok := p.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if wire.Decrypt(pkt) && wire.ParseHeader(pkt).Type == wire.OpMessagePanel &&
			strings.HasPrefix(string(pkt[wire.HeaderSize:]), "PK mode enabled.") {
			pkReplayed = true
		}
	}
	if !pkReplayed {
		t.Fatal("packet deferred behind the durable write was never replayed")
	}
}
