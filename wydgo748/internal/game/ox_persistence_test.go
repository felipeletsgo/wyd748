package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func oxCriticalStore(t *testing.T, w *World, outcome string) (*criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	if outcome == "rejection" {
		base.failure = criticalCertifiedRejection(t)
	} else if outcome == "unknown" {
		base.failure = errors.New("OX commit acknowledgement lost")
	}
	q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 4)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	return base, q, release
}

func TestOXCriticalTicketPersistsBeforeAdmission(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
			runOXEvent(w, "start", true)
			p := oxTestPlayer(w, 1, 1340, 1460)
			p.Char.Inv[2] = model.Item{Index: 1737, UID: "ox-ticket"}
			before := accountStateSnapshot(p.Account)
			base, q, release := oxCriticalStore(t, w, outcome)
			w.onUseItem(p.Session, useItemPacket(2, 0))
			proposal := awaitCriticalWrite(t, base.entered)
			if len(proposal.accounts) != 1 || proposal.accounts[0].Chars[0].Inv[2].Index != 0 || len(w.criticalTeleportTiles) != 1 {
				t.Fatal("ticket proposal omitted consumption or the retained destination")
			}
			for tile, owner := range w.criticalTeleportTiles {
				x, y := tile.xy()
				if owner != p || !oxArena.contains(x, y) {
					t.Fatal("ticket destination was not reserved inside the arena")
				}
			}
			if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || w.isOXMember(p) || !w.ox.admitting[p] {
				t.Fatal("unacknowledged admission consumed or published the ticket")
			}
			if r := runOXEvent(w, "mode_off", true); r.Code != "busy" {
				t.Fatal("operator changed the event during ticket persistence")
			}
			// An admitted write may outlive enrollment; no question starts before
			// the ticket's acknowledgement, and it still gets a full ten seconds.
			clock.Advance(70 * time.Second)
			w.tickOXEvent(clock.Now())
			if w.ox.phase != "enrollment" || p.Y != 1460 {
				t.Fatal("pending admission advanced the question or coordinates")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			if p.Char.Inv[2].Index != 1737 || w.isOXMember(p) {
				t.Fatal("worker completion published before acknowledgement")
			}
			w.processCriticalCompletions(1)
			switch outcome {
			case "commit":
				if !w.isOXMember(p) || p.Char.Inv[2].Index != 0 || len(w.criticalTeleportTiles) != 0 {
					t.Fatal("acknowledged ticket did not install admission")
				}
				w.tickOXEvent(clock.Now())
				if w.ox.phase != "question" || w.ox.deadline.Sub(clock.Now()) != 10*time.Second {
					t.Fatal("slow admission shortened the response window")
				}
			case "rejection":
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || w.isOXMember(p) ||
					w.ox.admitting[p] || len(w.criticalTeleportTiles) != 0 || w.shuttingDown ||
					!oxNoticeContains(p, "not consumed") {
					t.Fatal("rejected admission lost the ticket, retained a fence or omitted the notice")
				}
			case "unknown":
				if !w.shuttingDown || !w.criticalPlayerReserved(p) || !w.ox.admitting[p] ||
					!reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || w.isOXMember(p) {
					t.Fatal("unknown admission was not contained without speculative entry")
				}
			}
		})
	}
}

func TestOXCriticalRoundAtomicRewardsAndDepartures(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
			runOXEvent(w, "start", true)
			players := []*Player{oxTestPlayer(w, 1, 1340, 1460), oxTestPlayer(w, 2, 1338, 1460), oxTestPlayer(w, 3, 1336, 1460)}
			for i, p := range players {
				enterOXTest(t, w, p)
				placeOXTest(w, p, uint16(1315+i), 1500)
			}
			winner := players[0]
			placeOXTest(w, winner, 1366, 1500)
			clock.Advance(time.Minute)
			w.tickOXEvent(clock.Now())
			before := make(map[*Player]*model.Account)
			for _, p := range players {
				before[p] = accountStateSnapshot(p.Account)
				drainPlayerPackets(p)
			}
			base, q, release := oxCriticalStore(t, w, outcome)
			clock.Advance(10 * time.Second)
			w.tickOXEvent(clock.Now())
			proposal := awaitCriticalWrite(t, base.entered)
			if len(proposal.accounts) != 3 || proposal.instances == nil || !w.ox.settling || len(w.criticalTeleportTiles) != 2 {
				t.Fatal("round did not reserve one atomic batch and distinct loser destinations")
			}
			for tile, owner := range w.criticalTeleportTiles {
				x, y := tile.xy()
				if owner == winner || oxArena.contains(x, y) {
					t.Fatal("loser destination reservation retained an arena position")
				}
			}
			for _, account := range proposal.accounts {
				ch := account.Chars[0]
				if account.Name == winner.Account.Name {
					if ch.Exp != 8000 {
						t.Fatal("winner proposal omitted the configured EXP")
					}
				} else {
					if ch.Exp != 0 {
						t.Fatal("loser proposal included an EXP reward")
					}
				}
			}
			for _, p := range players {
				if !reflect.DeepEqual(before[p], accountStateSnapshot(p.Account)) || !w.criticalPlayerReserved(p) || !w.isOXMember(p) {
					t.Fatal("unacknowledged round changed visible state or lost its fence")
				}
				for packet, ok := p.Session.DequeuePacketForTest(); ok; packet, ok = p.Session.DequeuePacketForTest() {
					if wire.Decrypt(packet) && wire.ParseHeader(packet).Type == wire.OpInstanceQuestion {
						t.Fatal("round closed the question before acknowledgement")
					}
				}
			}
			clock.Advance(7 * time.Second)
			w.tickOXEvent(clock.Now())
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			switch outcome {
			case "commit":
				if winner.Char.Exp != 8000 || !w.isOXMember(winner) || w.ox.phase != "waiting" ||
					w.ox.deadline.Sub(clock.Now()) != 50*time.Second || len(w.criticalTeleportTiles) != 0 || w.ox.settling {
					t.Fatal("committed round lost the reward or shortened the next countdown")
				}
				for _, loser := range players[1:] {
					if loser.Char.Exp != 0 || w.isOXMember(loser) || oxArena.contains(loser.X, loser.Y) {
						t.Fatal("committed loser remained in the event or earned EXP")
					}
				}
				if players[1].X == players[2].X && players[1].Y == players[2].Y {
					t.Fatal("committed loser exits stacked players")
				}
			case "rejection":
				for _, p := range players {
					if !reflect.DeepEqual(before[p], accountStateSnapshot(p.Account)) || !w.isOXMember(p) || w.criticalPlayerReserved(p) {
						t.Fatal("certified rejection changed a participant or retained a fence")
					}
				}
				if w.ox.settling || w.ox.phase != "question" || w.ox.round != 0 || len(w.criticalTeleportTiles) != 0 || w.shuttingDown {
					t.Fatal("rejected round advanced or retained destination reservations")
				}
			case "unknown":
				if !w.shuttingDown || !w.ox.settling || !w.criticalPlayerReserved(winner) || winner.Char.Exp != 0 || w.ox.round != 0 {
					t.Fatal("unknown outcome published a reward or advanced without containment")
				}
			}
		})
	}
}
