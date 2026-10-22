package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func colosseumCriticalWorld(t *testing.T, rejection bool) (*World, *Player, *criticalTestStore, func()) {
	t.Helper()
	w, p, _ := handlerTestWorld(t)
	w.clock = newFakeClock(time.Date(2026, 10, 21, 20, 5, 0, 0, time.UTC))
	p.Char.UID = "colosseum-owner"
	w.playersByCharacterUID[p.Char.UID] = p
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("account admission failed")
	}
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	if rejection {
		base.failure = criticalCertifiedRejection(t)
	}
	q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 4)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	w.store = &guildFlowStore{}
	w.colosseum = &colosseumRuntime{unpaid: make(map[*Player]bool)}
	return w, p, base, func() {
		release()
		awaitReadyCriticalResults(t, q, 1)
		w.processCriticalCompletions(1)
	}
}

// Under critical persistence the arena exit revives and moves the player only
// after the write commits, never with a synchronous save on the World.
func TestColosseumCriticalExitRevivesThroughTheCoordinator(t *testing.T) {
	w, p, base, commit := colosseumCriticalWorld(t, false)
	setPlayerCurHP(p.Char, 0)
	p.DeadAt = w.now().Add(-time.Second)
	oldX, oldY := p.X, p.Y
	if !w.moveOutOfArena(p, colosseumExitX, colosseumExitY) {
		t.Fatal("the arena exit was not admitted")
	}
	proposal := awaitCriticalWrite(t, base.entered)
	if playerCurHP(&proposal.accounts[0].Chars[p.CharSlot]) != 1 || p.X != oldX || p.Y != oldY {
		t.Fatal("the exit omitted the revival or moved before the commit")
	}
	// A pending write fences the class filter: no duplicate recall is queued.
	p.Char.Evolution = archEvolution
	p.X, p.Y = 2630, 1726
	w.updatePlayerSpatial(p)
	w.enforceColosseumClass(model.ColosseumTierN, colosseumArea)
	if len(base.entered) != 0 {
		t.Fatal("the class filter queued a write while another was pending")
	}
	p.Char.Evolution, p.X, p.Y = "", oldX, oldY
	w.updatePlayerSpatial(p)
	commit()
	if playerCurHP(p.Char) != 1 || !p.DeadAt.IsZero() || chebyshev(p.X, p.Y, colosseumExitX, colosseumExitY) > 3 {
		t.Fatalf("committed exit hp=%d at (%d,%d)", playerCurHP(p.Char), p.X, p.Y)
	}
}

// A rejected elixir write keeps the item and marks the player unpaid, so the
// next tick charges or recalls them instead of granting a free entry.
func TestColosseumCriticalElixirRejectionMarksUnpaid(t *testing.T) {
	w, p, base, commit := colosseumCriticalWorld(t, true)
	p.Char.Inv[2] = model.Item{Index: colosseumElixirItem}
	w.chargeColosseumElixir(p)
	awaitCriticalWrite(t, base.entered)
	if !w.colosseum.unpaid[p] {
		t.Fatal("a pending charge granted entry before its outcome")
	}
	commit()
	if p.Char.Inv[2].Index != colosseumElixirItem || !w.colosseum.unpaid[p] {
		t.Fatal("a rejected charge lost the elixir or left the player paid")
	}
}

func TestColosseumWaitsForElixirCommitBeforeStartingWaves(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		w, p, base, commit := colosseumCriticalWorld(t, rejected)
		w.warConfig.Timezone = "UTC"
		w.colosseum.config = model.DefaultColosseumConfig()
		w.colosseum.stage = 1
		w.colosseum.tier = model.ColosseumTierA
		w.colosseum.startedAt = w.now()
		w.colosseum.endsAt = w.now().Add(12 * time.Minute)
		p.X, p.Y = 2630, 1726
		w.updatePlayerSpatial(p)
		p.Char.Inv[2] = model.Item{Index: colosseumElixirItem}
		w.chargeColosseumElixir(p)
		awaitCriticalWrite(t, base.entered)
		start := w.now().Add(time.Minute)
		w.tickColosseum(start)
		if w.colosseum.stage != 1 {
			t.Fatal("waves started before the entry write committed")
		}
		commit()
		w.tickColosseum(start.Add(time.Second))
		if rejected && w.colosseum.stage != 1 || !rejected && w.colosseum.stage != 2 {
			t.Fatalf("rejected=%v stage=%d", rejected, w.colosseum.stage)
		}
	}
}

func TestColosseumManualStartKeepsCriticalElixirAdmission(t *testing.T) {
	w, p, base, commit := colosseumCriticalWorld(t, false)
	w.clock = newFakeClock(time.Date(2026, 10, 21, 17, 0, 0, 0, time.UTC))
	w.warConfig.Timezone = "UTC"
	w.colosseum.config = model.DefaultColosseumConfig()
	p.X, p.Y = 2630, 1726
	w.updatePlayerSpatial(p)
	p.Char.Inv[2] = model.Item{Index: colosseumElixirItem}
	r := runColosseumCommand(w, colosseumStartCommand(w, "A"), true)
	if r.Code != "ok" || !w.colosseum.unpaid[p] {
		t.Fatalf("manual admission skipped the critical write: %+v", r)
	}
	awaitCriticalWrite(t, base.entered)
	w.tickColosseum(w.now().Add(time.Minute))
	if w.colosseum.stage != 1 || p.Char.Inv[2].Index != colosseumElixirItem {
		t.Fatal("manual start advanced or consumed the item before commit")
	}
	commit()
	w.tickColosseum(w.now().Add(time.Minute + time.Second))
	if w.colosseum.stage != 2 || p.Char.Inv[2].Index != 0 || w.colosseum.unpaid[p] {
		t.Fatal("manual start did not resume after the durable elixir charge")
	}
}

func TestColosseumRetriesRejectedExitAfterEventHour(t *testing.T) {
	w, p, base, commit := colosseumCriticalWorld(t, true)
	p.X, p.Y = 2630, 1726
	w.updatePlayerSpatial(p)
	w.finishColosseum()
	awaitCriticalWrite(t, base.entered)
	w.retryColosseumExits()
	if len(base.entered) != 0 {
		t.Fatal("a pending exit was duplicated")
	}
	commit()
	if !w.colosseum.exits[p] || p.X != 2630 {
		t.Fatal("a rejected exit was forgotten or published")
	}
	base.failure = nil
	w.colosseum.config = model.DefaultColosseumConfig()
	w.warConfig.Timezone = "UTC"
	w.tickColosseum(time.Date(2026, 10, 22, 0, 1, 0, 0, time.UTC))
	awaitCriticalWrite(t, base.entered)
	commit()
	if w.colosseum.exits[p] || colosseumExitArea.contains(p.X, p.Y) {
		t.Fatal("the committed retry did not complete the arena exit")
	}
}

func TestColosseumFinalizesUnpaidEntryAfterClockSkipsTheEventHour(t *testing.T) {
	w, p, base, commit := colosseumCriticalWorld(t, false)
	w.colosseum.config = model.DefaultColosseumConfig()
	w.warConfig.Timezone = "UTC"
	w.colosseum.stage = 1
	w.colosseum.endsAt = w.now().Add(12 * time.Minute)
	p.X, p.Y = 2630, 1726
	w.updatePlayerSpatial(p)
	w.colosseum.unpaid[p] = true
	w.tickColosseum(time.Date(2026, 10, 22, 0, 1, 0, 0, time.UTC))
	awaitCriticalWrite(t, base.entered)
	if w.colosseum.stage != 8 || w.colosseum.unpaid != nil || !w.colosseum.exits[p] {
		t.Fatal("a skipped event hour left unpaid entrants or the arena active")
	}
	commit()
	if w.colosseum.exits[p] || colosseumExitArea.contains(p.X, p.Y) {
		t.Fatal("the overdue event exit did not commit")
	}
}
