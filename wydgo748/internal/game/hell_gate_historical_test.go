package game

import (
	"encoding/json"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

// Exercise all twelve ordered draws in sub_578700, including adjacent rooms.
// This is a historical 7.59 rule adapted to the existing 7.48 wire contract.
func TestHellGateHistoricalPairProgression(t *testing.T) {
	for first := 0; first < 4; first++ {
		for offset := 1; offset <= 3; offset++ {
			second := (first + offset) % 4
			t.Run(fmt.Sprintf("rooms-%d-%d", first+1, second+1), func(t *testing.T) {
				w, inst, _ := hellGateVersion2Test(t)
				inst.Config.HellGate.Version = 3
				w.rng = &sequenceRNG{values: []int{first % 3, first, offset - 1}}
				killHellGateTestMob(t, w, inst, hellGatePopulationIDs(inst)[0])
				for id, corridor := range inst.MobQuadrants {
					if int(corridor) == inst.HellGateVariant {
						killHellGateTestMob(t, w, inst, id)
						break
					}
				}
				want := hellGateBit(uint8(first+1)) | hellGateBit(uint8(second+1))
				if inst.State != "lich_puzzle" || inst.HellGateValidLichMask != want || inst.Remaining != 144 {
					t.Fatalf("wrong random pair or room population: state=%s mask=%x want=%x remaining=%d",
						inst.State, inst.HellGateValidLichMask, want, inst.Remaining)
				}
				for _, id := range hellGatePopulationIDs(inst) {
					killHellGateTestMob(t, w, inst, id)
				}
				if inst.Remaining != 16 || len(inst.HellGateLichIDs) != 4 {
					t.Fatal("each cleared room must create one Lich and three escorts")
				}
				firstID := inst.HellGateLichIDs[uint8(first+1)]
				firstMob := w.mobsByID[firstID]
				killHellGateTestMob(t, w, inst, firstID)
				if inst.State != "lich_puzzle" || inst.HellGateWrongLich || len(inst.NPCIDs) != 0 {
					t.Fatal("one correct Lich unlocked Dionys or was incorrectly marked as a wrong Lich")
				}
				remaining := inst.Remaining
				w.onHellGateMobKilled(inst, firstMob, w.now())
				if inst.State != "lich_puzzle" || inst.Remaining != remaining {
					t.Fatal("replayed death counted as the second selected Lich")
				}
				for room, id := range inst.HellGateLichIDs {
					if hellGateBit(room)&want == 0 {
						killHellGateTestMob(t, w, inst, id)
					}
				}
				if inst.State != "lich_puzzle" || !inst.HellGateWrongLich {
					t.Fatal("unselected Liches advanced or permanently locked the puzzle")
				}
				killHellGateTestMob(t, w, inst, inst.HellGateLichIDs[uint8(second+1)])
				if inst.State != "protection" || !w.hellGateDionysAlive(inst) || inst.Remaining != 252 {
					t.Fatal("both selected Liches did not start the existing timed defense")
				}
			})
		}
	}
}

func TestHellGateHistoricalInstalledPortalReachesPopulatedArena(t *testing.T) {
	for selected := 1; selected <= 3; selected++ {
		for _, phase := range []string{"lich_puzzle", "protection"} {
			t.Run(fmt.Sprintf("selected-%d/%s", selected, phase), func(t *testing.T) {
				w, inst, _ := hellGateVersion2Test(t)
				inst.Config.HellGate.Version, inst.HellGateVariant = 3, selected
				if phase == "lich_puzzle" {
					if !w.spawnHellGateQuadrants(inst, w.now()) {
						t.Fatal("room creation failed")
					}
				} else if !w.spawnHellGateFinal(inst, w.now()) {
					t.Fatal("defense creation failed")
				}
				p := w.playersByID[inst.MemberIDs[0]]
				route := inst.Config.HellGate.Portals[selected-1]
				p.X, p.Y = route.SourceX, route.SourceY
				p.Char.X, p.Char.Y = p.X, p.Y
				w.updatePlayerSpatial(p)
				w.onReqTeleport(p.Session, make([]byte, 16))
				if p.X == route.SourceX || chebyshev(p.X, p.Y, route.DestX, route.DestY) > 28 ||
					w.playerRuntimeInstanceID(p.ID) != inst.RuntimeID || !w.terrain.Walkable(p.X, p.Y) {
					t.Fatal("selected installed portal failed to enter its populated arena")
				}
				reachable := hellGateReachable(w.terrain, p.X, p.Y)
				for id := range inst.MobIDs {
					mob := w.mobsByID[id]
					if mob == nil || !reachable[[2]uint16{mob.X, mob.Y}] {
						t.Fatal("selected portal stranded the player away from an encounter actor")
					}
				}
				for id := range inst.NPCIDs {
					mob := w.mobsByID[id]
					if mob == nil || !reachable[[2]uint16{mob.X, mob.Y}] {
						t.Fatal("selected portal cannot reach Dionys")
					}
				}
			})
		}
	}
}

func TestHellGateHistoricalSelectedPortal(t *testing.T) {
	for selected := 1; selected <= 3; selected++ {
		for source := 1; source <= 3; source++ {
			for _, state := range []string{"preparing", "entry", "corridors", "lich_puzzle", "protection", "failed"} {
				t.Run(fmt.Sprintf("selected-%d/source-%d/%s", selected, source, state), func(t *testing.T) {
					w, p, inst, store, _ := hellGatePortalTestWorld(t, state, 1756, 3616)
					inst.Config.HellGate.Version = 3
					inst.HellGateVariant = selected
					portal := inst.Config.HellGate.Portals[source-1]
					p.X, p.Y = portal.SourceX+3, portal.SourceY+3
					p.Char.X, p.Char.Y = p.X, p.Y
					w.updatePlayerSpatial(p)
					w.teleports = []model.Teleport{{SourceX: portal.SourceX, SourceY: portal.SourceY, DestX: 1900, DestY: 3600}}
					before := accountStateSnapshot(p.Account)
					w.onReqTeleport(p.Session, make([]byte, 16))
					allowed := source == selected && (state == "lich_puzzle" || state == "protection")
					if allowed {
						if p.X != portal.DestX || p.Y != portal.DestY || len(store.snapshots) != 1 {
							t.Fatalf("selected portal did not commit its route: position=%d,%d snapshots=%d", p.X, p.Y, len(store.snapshots))
						}
					} else if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || len(store.snapshots) != 0 {
						t.Fatal("closed or unselected portal changed the account or used a global fallback")
					}
					if w.playerRuntimeInstanceID(p.ID) != inst.RuntimeID || p.Char.Gold != 1000 {
						t.Fatal("portal changed membership or charged gold")
					}
				})
			}
		}
	}
	for _, selected := range []int{0, 4} {
		w, p, inst, store, _ := hellGatePortalTestWorld(t, "lich_puzzle", 1756, 3616)
		inst.Config.HellGate.Version, inst.HellGateVariant = 3, selected
		w.teleports = hellGateGlobalFallback()
		w.onReqTeleport(p.Session, make([]byte, 16))
		if p.X != 1756 || p.Y != 3616 || len(store.snapshots) != 0 {
			t.Fatal("invalid selected corridor escaped the instance gate")
		}
	}
}

func TestHellGateHistoricalSnapshotPreservesPairAndProgress(t *testing.T) {
	w, inst, clock := hellGateVersion2Test(t)
	inst.Config.HellGate.Version = 3
	inst.HellGateVariant = 2
	inst.HellGateValidLichMask = hellGateBit(1) | hellGateBit(2)
	if !w.spawnHellGateQuadrants(inst, clock.Now()) {
		t.Fatal("room spawn failed")
	}
	for _, id := range hellGatePopulationIDs(inst) {
		killHellGateTestMob(t, w, inst, id)
	}
	killHellGateTestMob(t, w, inst, inst.HellGateLichIDs[1])
	encoded, err := json.Marshal(w.instanceStateSnapshot())
	if err != nil {
		t.Fatal(err)
	}
	var snapshot model.InstanceStateSnapshot
	if err := json.Unmarshal(encoded, &snapshot); err != nil {
		t.Fatal(err)
	}
	if len(snapshot.Instances) != 1 || snapshot.Instances[0].HellGateVersion != 3 {
		t.Fatal("durable snapshot lost the encounter version")
	}
	restored, rule := installedHellGate(t)
	restored.clock = clock
	restored.volatiles.Items[3328] = rule
	restored.store = &gambleStore{load: &snapshot}
	if err := restored.restoreInstanceState(); err != nil {
		t.Fatal(err)
	}
	resumed := restored.itemInstances[inst.RuntimeID]
	if resumed == nil || resumed.Config.HellGate.Version != 3 || resumed.HellGateVariant != 2 ||
		resumed.HellGateValidLichMask != inst.HellGateValidLichMask || resumed.Remaining != 12 ||
		resumed.HellGateLichIDs[1] != 0 || resumed.HellGateLichIDs[2] == 0 {
		t.Fatal("restoration rerolled the pair, reopened a cleared room, or lost the portal selection")
	}
	killHellGateTestMob(t, restored, resumed, resumed.HellGateLichIDs[2])
	if resumed.State != "protection" || !restored.hellGateDionysAlive(resumed) {
		t.Fatal("restored first-Lich progress did not count toward the second Lich")
	}
}

func TestHellGateHistoricalRestoreVersionAndMaskValidation(t *testing.T) {
	for _, tc := range []struct {
		name        string
		version     int
		mask        uint8
		wantVersion int
		wantOK      bool
	}{
		{"unversioned existing run", 0, 1, 2, true},
		{"explicit existing run", 2, 1, 2, true},
		{"new adjacent pair", 3, 3, 3, true},
		{"new opposite pair", 3, 5, 3, true},
		{"single Lich in new run", 3, 1, 3, false},
		{"three Liches", 3, 7, 3, false},
		{"invalid room bit", 3, 17, 3, false},
		{"unknown version", 4, 3, 3, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, inst, _ := hellGateVersion2Test(t)
			w.removeHellGateMobs(inst)
			inst.Config.HellGate.Version = 3
			inst.State, inst.HellGateVariant = "lich_puzzle", 1
			inst.HellGateValidLichMask = tc.mask
			saved := model.InstanceRuntimeState{State: inst.State, HellGateVersion: tc.version}
			if got := w.restoreHellGateRuntime(inst, saved, w.now()); got != tc.wantOK {
				t.Fatalf("restore=%v want=%v", got, tc.wantOK)
			}
			if inst.Config.HellGate.Version != tc.wantVersion {
				t.Fatal("restoration upgraded an existing run or changed the shared configuration")
			}
		})
	}
}

func TestHellGateHistoricalUpgradeKeepsExistingRunRules(t *testing.T) {
	for _, version := range []int{0, 2} {
		t.Run(fmt.Sprintf("saved-version-%d", version), func(t *testing.T) {
			w, inst, clock := hellGateVersion2Test(t)
			inst.HellGateVariant, inst.HellGateValidLichMask = 1, hellGateBit(2)
			if !w.spawnHellGateQuadrants(inst, clock.Now()) {
				t.Fatal("room spawn failed")
			}
			snapshot := w.instanceStateSnapshot()
			snapshot.Instances[0].HellGateVersion = version
			restored, rule := installedHellGate(t)
			if rule.Instance.HellGate.Version != 3 {
				t.Fatal("installed data did not activate the new historical puzzle")
			}
			restored.clock = clock
			restored.volatiles.Items[3328] = rule
			restored.store = &gambleStore{load: snapshot}
			if err := restored.restoreInstanceState(); err != nil {
				t.Fatal(err)
			}
			resumed := restored.itemInstances[inst.RuntimeID]
			if resumed == nil || resumed.Config.HellGate.Version != 2 || resumed.HellGateValidLichMask != hellGateBit(2) ||
				!resumed.HardDeadline.Equal(inst.HardDeadline) || rule.Instance.HellGate.Version != 3 {
				t.Fatal("upgrade changed an existing run or mutated the new-run template")
			}
			if got := restored.instanceStateSnapshot().Instances[0].HellGateVersion; got != 2 {
				t.Fatalf("existing run was not resaved with explicit version 2: %d", got)
			}
		})
	}
}

func TestHellGateHistoricalRejectsCorruptProgressAndDowngrade(t *testing.T) {
	for _, tc := range []struct {
		name       string
		configured int
		variant    int
		cleared    uint8
		spawned    uint8
	}{
		{"configuration rollback", 2, 1, 0, 0},
		{"missing corridor", 3, 0, 0, 0},
		{"out of range corridor", 3, 4, 0, 0},
		{"invalid cleared room", 3, 1, 16, 16},
		{"invalid spawned room", 3, 1, 0, 16},
		{"cleared Lich never spawned", 3, 1, 1, 0},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, inst, _ := hellGateVersion2Test(t)
			w.removeHellGateMobs(inst)
			inst.Config.HellGate.Version = tc.configured
			inst.HellGateVariant = tc.variant
			inst.HellGateValidLichMask = 3
			inst.HellGateClearedMask, inst.HellGateLichSpawnedMask = tc.cleared, tc.spawned
			saved := model.InstanceRuntimeState{
				State: "lich_puzzle", HellGateVersion: 3,
				HellGateClearedMask: tc.cleared, HellGateLichSpawned: tc.spawned,
			}
			if w.restoreHellGateRuntime(inst, saved, w.now()) || len(inst.MobIDs) != 0 {
				t.Fatal("invalid progress or a downgraded configuration recreated a live encounter")
			}
		})
	}
}
