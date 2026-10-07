package game

import (
	"context"
	"errors"
	"sync"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

func criticalRecoveryLoopFixture(t *testing.T, failure error) (*World, *Player, *Player, *criticalTestStore, *criticalPersistence, chan time.Time, func(), func()) {
	t.Helper()
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate, failure: failure}
	w, err := NewWorld(backend, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{})
	if err != nil {
		t.Fatal(err)
	}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	p, _ := networkedTestPlayer(1, "Recovering", 2100, 2100)
	unrelated, _ := networkedTestPlayer(2, "Unrelated", 2110, 2100)
	for _, player := range []*Player{p, unrelated} {
		w.players[player.Session], w.playersByID[player.ID] = player, player
		w.indexPlayerCharacter(player)
		w.updatePlayerSpatial(player)
		if !w.claimAccountSession(player.Session, player.Account.Name) {
			t.Fatal("fixture account admission failed")
		}
		player.NextCPRecovery = w.now().Add(time.Hour)
		player.NextRegen = w.now().Add(time.Hour)
	}
	p.Char.CP, p.NextCPRecovery = -2, w.now().Add(-time.Second)
	setPlayerCurHP(unrelated.Char, 500)
	unrelated.NextRegen = w.now().Add(-time.Second)
	// This gate exercises critical recovery, not the separately pending migration
	// of autosave/direct-store fallbacks. Avoid an unrelated synchronous autosave.
	w.nextAutoSave = w.now().Add(time.Hour)
	ticks, stop, stopped := make(chan time.Time), make(chan struct{}), make(chan struct{})
	go func() {
		defer close(stopped)
		w.run(ticks, stop)
	}()
	var once sync.Once
	stopLoop := func() {
		once.Do(func() { close(stop) })
		select {
		case <-stopped:
		case <-time.After(2 * time.Second):
			t.Error("World loop did not stop")
		}
	}
	t.Cleanup(stopLoop)
	return w, p, unrelated, backend, queue, ticks, release, stopLoop
}

func readCriticalLoopOverview(t *testing.T, w *World) control.Overview {
	t.Helper()
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	for {
		result, err := w.Overview(ctx, control.Query{})
		if err == nil {
			return result
		}
		if !errors.Is(err, control.ErrBusy) {
			t.Fatalf("World command blocked behind critical persistence: %v", err)
		}
		// A reply can arrive before the actor releases its one-read admission
		// gate. Retry only this transient rejection within the original deadline.
		select {
		case <-time.After(time.Millisecond):
		case <-ctx.Done():
			t.Fatalf("World overview remained busy: %v", ctx.Err())
		}
	}
}

func deliverCriticalLoopTick(t *testing.T, ticks chan<- time.Time) {
	t.Helper()
	select {
	case ticks <- time.Now():
	case <-time.After(time.Second):
		t.Fatal("World stopped accepting ticks while a critical save was held")
	}
}

func TestCriticalChaosRecoveryWorldLoopContinuesAndConfirmsAfterCommit(t *testing.T) {
	w, p, unrelated, backend, queue, ticks, release, stop := criticalRecoveryLoopFixture(t, nil)
	deliverCriticalLoopTick(t, ticks)
	write := awaitCriticalWrite(t, backend.entered)
	if write.accounts[0].Chars[p.CharSlot].CP != -1 {
		t.Fatal("actual tick did not submit proposed CP")
	}
	for range 3 {
		deliverCriticalLoopTick(t, ticks)
		overview := readCriticalLoopOverview(t, w)
		regenerated := false
		for _, player := range overview.Players {
			if player.UID == unrelated.Char.UID && player.HP > 500 {
				regenerated = true
			}
		}
		if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("held save blocked unrelated tick/read commands or confirmed CP before commit")
		}
	}
	select {
	case <-backend.entered:
		t.Fatal("repeated World ticks submitted recovery again")
	default:
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	for p.Session.QueuedPacketsForTest() == 0 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	if p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("World did not consume and publish the committed recovery")
	}
	readCriticalLoopOverview(t, w)
	stop()
	// No actor-owned state is read concurrently; the actor is now joined.
	if p.Char.CP != -1 || !p.NextCPRecovery.After(time.Now()) || len(queue.pending) != 0 ||
		w.criticalPlayerReserved(p) || w.mobTickCounter < 4 {
		t.Fatal("actual loop did not settle recovery, release the fence and advance ticks")
	}
}

func TestCriticalChaosRecoveryWorldLoopUnknownCommitEntersMaintenance(t *testing.T) {
	w, p, unrelated, backend, queue, ticks, release, stop := criticalRecoveryLoopFixture(t, errors.New("commit acknowledgement lost"))
	deliverCriticalLoopTick(t, ticks)
	awaitCriticalWrite(t, backend.entered)
	readCriticalLoopOverview(t, w)
	release()
	deadline := time.Now().Add(2 * time.Second)
	maintenance := false
	for time.Now().Before(deadline) {
		if readCriticalLoopOverview(t, w).State == "maintenance" {
			maintenance = true
			break
		}
	}
	if !maintenance {
		t.Fatal("actual loop failed to isolate the unknown outcome")
	}
	deliverCriticalLoopTick(t, ticks)
	readCriticalLoopOverview(t, w)
	stop()
	if p.Char.CP != -2 || !p.PersistencePoisoned || !unrelated.PersistencePoisoned ||
		!p.Session.IsClosed() || !unrelated.Session.IsClosed() || p.Session.QueuedPacketsForTest() != 0 ||
		len(queue.pending) != 1 || !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) || w.mobTickCounter != 1 {
		t.Fatal("unknown commit published, retried, released ownership or allowed further tick mutation")
	}
}

func TestCriticalDisconnectWorldLoopContinuesDuringFinalSave(t *testing.T) {
	w, p, unrelated, backend, queue, ticks, release, stop := criticalRecoveryLoopFixture(t, nil)
	select {
	case w.commands <- command{s: p.Session}:
	case <-time.After(time.Second):
		t.Fatal("World did not accept the physical disconnect")
	}
	write := awaitCriticalWrite(t, backend.entered)
	if write.accounts[0].Chars[p.CharSlot].CP != -2 {
		t.Fatal("final disconnect snapshot changed CP before commit")
	}
	for range 3 {
		deliverCriticalLoopTick(t, ticks)
		overview := readCriticalLoopOverview(t, w)
		regenerated := false
		for _, player := range overview.Players {
			if player.UID == unrelated.Char.UID && player.HP > 500 {
				regenerated = true
			}
		}
		if overview.State != "running" || !regenerated || !p.Session.IsClosed() || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("held final write blocked unrelated ticks/commands or published to closed transport")
		}
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	removed := false
	for time.Now().Before(deadline) {
		found := false
		for _, player := range readCriticalLoopOverview(t, w).Players {
			if player.UID == p.Char.UID {
				found = true
			}
		}
		if !found {
			removed = true
			break
		}
	}
	stop()
	// Join the actor before inspecting actor-owned maps and character state.
	if !removed || w.players[p.Session] != nil || len(queue.pending) != 0 || len(w.criticalDisconnects) != 0 ||
		w.accountSessions[accountSessionKey(p.Account.Name)] != nil || p.Char.CP != -2 || w.mobTickCounter < 3 ||
		len(backend.entered) != 0 || w.shuttingDown {
		t.Fatal("actual loop omitted final cleanup, retried persistence or failed to advance unrelated ticks")
	}
}
