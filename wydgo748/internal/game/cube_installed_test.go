package game

import (
	"fmt"
	"testing"
	"time"

	"wydgo/internal/data"
)

// Exercise every shipped Cube room with real terrain and templates. Each
// activation owns its mobs; teardown must permit a later clean activation.
func TestCubeInstalledRoomsSpawnOnlyOnActivation(t *testing.T) {
	fixtures, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	for _, ticket := range []uint16{1772, 4000, 4001} {
		rule, _, ok := fixtures.volatiles.Rule(ticket)
		if !ok || rule.Instance == nil {
			t.Fatalf("missing Cube ticket %d", ticket)
		}
		for room := range rule.Instance.Stages {
			t.Run(fmt.Sprintf("%s/room%d", rule.Instance.ID, room+1), func(t *testing.T) {
				w, leader, observer, _, clock := instanceTestWorld()
				w.npcs, w.terrain, w.volatiles = npcs, fixtures.terrain, fixtures.volatiles
				leader.Party, observer.Party = nil, nil
				w.tickItemInstances(clock.Now())
				if len(w.mobsByID) != 0 {
					t.Fatal("loading instance data created mobs before activation")
				}
				inst := &ItemInstance{Config: cfgCopy(*rule.Instance), RuntimeID: rule.Instance.ID,
					MemberIDs: []uint16{leader.ID}, LeaderID: leader.ID}
				stage := inst.Config.Stages[room]
				for _, p := range []*Player{leader, observer} {
					p.X, p.Y, p.Char.X, p.Char.Y = stage.X, stage.Y, stage.X, stage.Y
					w.updatePlayerSpatial(p)
				}
				for run := 0; run < 2; run++ {
					w.itemInstances[inst.RuntimeID] = inst
					w.rebuildPlayerInstanceIndex()
					if ok, _ := w.spawnItemInstanceStage(inst, room, clock.Now(), false, true); !ok {
						t.Fatal("installed room could not spawn its complete wave")
					}
					want := map[string]int{}
					for _, spawn := range stage.Spawns {
						want[spawn.NPC] += spawn.Count
					}
					seen := map[string]int{}
					tiles := map[[2]uint16]bool{}
					for id := range inst.MobIDs {
						m := w.mobsByID[id]
						if m == nil || m.InstanceID != inst.RuntimeID || m.GenerIndex != -1 ||
							!w.terrain.Walkable(m.X, m.Y) || !instanceStageContains(stage, m.X, m.Y) {
							t.Fatal("room mob has invalid ownership, bounds or terrain")
						}
						if !leader.hasVisible(id) || observer.hasVisible(id) {
							t.Fatal("private room visibility crossed membership")
						}
						key := [2]uint16{m.X, m.Y}
						if tiles[key] {
							t.Fatal("room mobs overlap")
						}
						tiles[key] = true
						seen[m.Def.Name]++
					}
					if len(seen) != len(want) || len(inst.MobIDs) != stageMobCount(stage) {
						t.Fatalf("incomplete room composition: %+v, want %+v", seen, want)
					}
					for name, count := range want {
						if seen[name] != count {
							t.Fatalf("%s count=%d, want=%d", name, seen[name], count)
						}
					}
					w.retireItemInstanceEntities(inst)
					w.closeItemInstance(inst.RuntimeID)
					clock.Advance(time.Second)
					w.tickItemInstances(clock.Now())
					if len(w.mobsByID) != 0 || len(w.itemInstances) != 0 {
						t.Fatal("closed room leaked or respawned mobs")
					}
				}
			})
		}
	}
}

func TestCubeInstalledTicketCommitAndRejection(t *testing.T) {
	fixtures, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	for _, ticket := range []uint16{1772, 4000, 4001} {
		for _, outcome := range []string{"commit", "rejection"} {
			t.Run(fmt.Sprintf("%d/%s", ticket, outcome), func(t *testing.T) {
				w, leader, member, peer, _, base, queue, _, release := criticalInstanceEntryFixture(t, "water", outcome)
				leader.Party, member.Party, leader.Trade, peer.Trade = nil, nil, nil, nil
				w.terrain, w.npcs = fixtures.terrain, npcs
				rule, _, _ := fixtures.volatiles.Rule(ticket)
				leader.X, leader.Y, leader.Char.X, leader.Char.Y = 2390, 1735, 2390, 1735
				w.updatePlayerSpatial(leader)
				before := leader.Char.Inv[0]
				if err := w.queueCriticalInstanceTicket(leader, 0, rule, ""); err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, base.entered)
				// Uncommitted entities remain unpublished until persistence acknowledges entry.
				for id := range w.mobsByID {
					if leader.hasVisible(id) || member.hasVisible(id) {
						t.Fatal("uncommitted Cube mobs were published")
					}
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if outcome == "rejection" {
					if leader.Char.Inv[0] != before || len(w.mobsByID) != 0 || len(w.itemInstances) != 0 {
						t.Fatal("rejected Cube entry consumed a ticket or leaked population")
					}
					return
				}
				inst := w.itemInstanceForPlayer(leader.ID)
				if inst == nil || inst.Config.ID != rule.Instance.ID || leader.Char.Inv[0].Index != 0 ||
					len(inst.MobIDs) != stageMobCount(rule.Instance.Stages[0]) {
					t.Fatal("committed Cube entry did not activate its first wave")
				}
			})
		}
	}
}
