package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestNightmareTeardownClearsPreviousSessionFromEveryViewer(t *testing.T) {
	for _, tier := range []string{"normal", "mystic", "arcane"} {
		for _, outcome := range []string{"inline", "commit", "rejection"} {
			t.Run(tier+"/"+outcome, func(t *testing.T) {
				w, leader, member, peer, _, base, queue, _, release := criticalInstanceEntryFixture(t, "water", outcome)
				leader.Trade, peer.Trade = nil, nil
				coordinator := w.criticalWrites
				w.criticalWrites = nil
				clock := newFakeClock(time.Date(2026, 10, 16, 12, 0, 0, 0, time.UTC))
				w.clock = clock
				rule := nightmareBossTestRule(w)
				rule.Instance.ID = "nightmare-" + tier
				rule.Instance.NightmareTier = tier
				wantEntities := 10
				if tier != "arcane" {
					rule.Instance.NightmareBoss = nil
					wantEntities = 9
				}
				w.volatiles.Items[100] = rule
				w.onUseItem(leader.Session, useItemPacket(0, 0))
				inst := w.itemInstances[rule.Instance.ID]
				if inst == nil {
					t.Fatal("Nightmare admission failed")
				}
				clock.Advance(14 * time.Minute)
				w.tickItemInstances(clock.Now())
				oldMobs := make(map[uint16]*Mob)
				for id := range inst.MobIDs {
					oldMobs[id] = w.mobsByID[id]
				}
				for id := range inst.NPCIDs {
					oldMobs[id] = w.mobsByID[id]
				}
				if len(oldMobs) != wantEntities || tier == "arcane" && !inst.NightmareBossSpawned {
					t.Fatalf("expected the complete tier population (%d entities): %+v", wantEntities, inst)
				}
				for _, p := range []*Player{leader, member} {
					for id := range oldMobs {
						if !p.hasVisible(id) {
							t.Fatalf("player %d never saw entity %d", p.ID, id)
						}
					}
					drainPlayerPackets(p)
				}
				if outcome != "inline" {
					w.criticalWrites = coordinator
				}
				clock.current = inst.HardDeadline
				w.tickItemInstances(clock.Now())
				if outcome != "inline" {
					awaitCriticalWrite(t, base.entered)
					for _, phase := range []string{"pending", "unacknowledged"} {
						if phase == "unacknowledged" {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						if w.itemInstances[rule.Instance.ID] != inst || len(w.mobsByID) != len(oldMobs) {
							t.Fatalf("%s exit retired authoritative entities early", phase)
						}
						for _, p := range []*Player{leader, member} {
							if !instanceMemberInStage(inst, p) || p.Session.QueuedPacketsForTest() != 0 {
								t.Fatalf("%s exit moved or published player %d early", phase, p.ID)
							}
						}
						w.tickItemInstances(clock.Now())
					}
					w.processCriticalCompletions(1)
					if outcome == "rejection" {
						if w.itemInstances[rule.Instance.ID] != inst || len(w.mobsByID) != len(oldMobs) || w.criticalInstanceReserved() {
							t.Fatal("rejected exit lost the encounter or retained its reservation")
						}
						for _, p := range []*Player{leader, member} {
							if !instanceMemberInStage(inst, p) || p.Session.QueuedPacketsForTest() != 0 {
								t.Fatal("rejected exit published a teleport or removal")
							}
						}
						base.failure = nil
						w.tickItemInstances(clock.Now())
						awaitCriticalWrite(t, base.entered)
						awaitReadyCriticalResults(t, queue, 1)
						w.processCriticalCompletions(1)
					}
				}
				if w.itemInstances[rule.Instance.ID] != nil {
					t.Fatal("expired Nightmare remained registered")
				}
				for id, mob := range oldMobs {
					if w.mobsByID[id] != nil || w.activeMobs[id] != nil {
						t.Fatalf("retired entity %d remained in a canonical index", id)
					}
					if _, present := w.mobCell[id]; present {
						t.Fatalf("retired entity %d retained a spatial cell", id)
					}
					for _, live := range w.mobs {
						if live == mob {
							t.Fatalf("retired entity %d remained in the simulation list", id)
						}
					}
				}
				for _, p := range []*Player{leader, member} {
					removed := make(map[uint16]bool)
					for {
						packet, ok := p.Session.DequeuePacketForTest()
						if !ok {
							break
						}
						if !wire.Decrypt(packet) {
							t.Fatal("invalid teardown packet")
						}
						header := wire.ParseHeader(packet)
						if header.Type == wire.OpRemoveMob && binary.LittleEndian.Uint32(packet[12:16]) == 3 {
							removed[header.ID] = true
						}
					}
					for id := range oldMobs {
						if p.hasVisible(id) || !removed[id] {
							t.Fatalf("player %d retained previous-session entity %d: visible=%v removal=%v", p.ID, id, p.hasVisible(id), removed[id])
						}
					}
				}
				// The next admission window must publish only its new protectors.
				clock.current = time.Date(2026, 10, 16, 13, 0, 0, 0, time.UTC)
				leader.Char.Inv[0] = model.Item{Index: 100}
				w.onUseItem(leader.Session, useItemPacket(0, 0))
				if outcome != "inline" {
					awaitCriticalWrite(t, base.entered)
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
				}
				next := w.itemInstances[rule.Instance.ID]
				if next == nil || next == inst || !nightmareWaiting(next) || len(next.MobIDs) != 0 || len(w.mobsByID) != 8 {
					t.Fatal("new admission reused the previous encounter's population")
				}
				for _, mob := range w.mobs {
					if oldMobs[mob.ID] == mob {
						t.Fatal("previous-session entity survived room reuse")
					}
				}
			})
		}
	}
}
