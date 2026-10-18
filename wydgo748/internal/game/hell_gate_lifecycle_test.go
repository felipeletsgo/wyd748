package game

import (
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
)

func hellGateVersion2Test(t *testing.T) (*World, *ItemInstance, *fakeClock) {
	t.Helper()
	w, rule := installedHellGate(t)
	clock := newFakeClock(time.Date(2026, 10, 17, 12, 2, 0, 0, time.UTC))
	w.clock, w.rng = clock, fixedRNG{value: 0}
	inst := &ItemInstance{Config: cfgCopy(*rule.Instance), RuntimeID: "hell-gate-v2-test", State: "entry",
		MemberIDs: []uint16{1}, LeaderID: 1, HardDeadline: clock.Now().Add(18 * time.Minute),
		ScheduleEnd: clock.Now()}
	// Keep explicit regression coverage for runs admitted before the v3 rollout.
	inst.Config.HellGate.Version = 2
	w.itemInstances[inst.RuntimeID] = inst
	w.rebuildPlayerInstanceIndex()
	setInstanceCombatDeadline(inst, inst.HardDeadline)
	if ok, _ := w.spawnItemInstanceStage(inst, 0, clock.Now(), false, false); !ok {
		t.Fatal("controller spawn failed")
	}
	return w, inst, clock
}

func killHellGateTestMob(t *testing.T, w *World, inst *ItemInstance, id uint16) {
	t.Helper()
	mob := w.mobsByID[id]
	if mob == nil {
		t.Fatalf("missing encounter actor %d", id)
	}
	mob.HP, mob.Dead = 0, true
	w.onItemInstanceMobKilled(mob, w.now())
	w.removeMobInstance(mob)
}

func hellGatePopulationIDs(inst *ItemInstance) []uint16 {
	ids := make([]uint16, 0, len(inst.MobIDs))
	for id := range inst.MobIDs {
		ids = append(ids, id)
	}
	return ids
}

func TestHellGateVersion2ProgressionAndContinuousProtection(t *testing.T) {
	w, inst, clock := hellGateVersion2Test(t)
	killHellGateTestMob(t, w, inst, hellGatePopulationIDs(inst)[0])
	if inst.State != "corridors" || inst.HellGateVariant != 1 || inst.Remaining != 3 {
		t.Fatalf("controller did not create three server-selected corridor bosses: %+v", inst)
	}
	bosses := make(map[uint8]uint16)
	for id, corridor := range inst.MobQuadrants {
		bosses[corridor] = id
	}
	killHellGateTestMob(t, w, inst, bosses[2])
	if inst.State != "corridors" {
		t.Fatal("an incorrect corridor boss unlocked the rooms")
	}
	killHellGateTestMob(t, w, inst, bosses[1])
	if inst.State != "lich_puzzle" || inst.Remaining != 36 || inst.HellGateValidLichMask != 1 {
		t.Fatalf("correct corridor did not unlock four rooms and one selected Lich: %+v", inst)
	}
	for _, id := range hellGatePopulationIDs(inst) {
		killHellGateTestMob(t, w, inst, id)
	}
	if len(inst.HellGateLichIDs) != 4 || inst.Remaining != 4 {
		t.Fatal("each cleared room must produce one Lich")
	}
	killHellGateTestMob(t, w, inst, inst.HellGateLichIDs[2])
	if inst.State != "lich_puzzle" || !inst.HellGateWrongLich {
		t.Fatal("wrong Lich prematurely unlocked the protector")
	}
	killHellGateTestMob(t, w, inst, inst.HellGateLichIDs[1])
	if inst.State != "protection" || !w.hellGateDionysAlive(inst) || inst.Remaining != 30 {
		t.Fatal("the single correct Lich did not start Dionys protection")
	}
	var protector *Mob
	for id := range inst.NPCIDs {
		protector = w.mobsByID[id]
	}
	protector.HP--
	inst.HellGateDionysHP = protector.HP
	retainedHP := protector.HP
	for _, id := range hellGatePopulationIDs(inst) {
		killHellGateTestMob(t, w, inst, id)
	}
	if inst.Remaining != 30 || inst.RewardGranted || !inst.ExitAt.IsZero() ||
		protector.HP != retainedHP || w.mobsByID[protector.ID] != protector {
		t.Fatal("clearing attackers must replenish pressure, not finish or heal Dionys")
	}
	clock.Advance(17 * time.Minute)
	w.tickHellGate(inst, clock.Now())
	if inst.RewardGranted || !inst.ExitAt.IsZero() {
		t.Fatal("completion reward was issued before the absolute event deadline")
	}
	killHellGateTestMob(t, w, inst, protector.ID)
	if inst.State != "failed" || inst.HellGateDionysHP != 0 || inst.RewardGranted {
		t.Fatal("Dionys death must fail the encounter without granting EXP")
	}
}

// Follow the real 7.48 terrain rather than treating walkable but disconnected
// anchors as proof that players can reach the encounter.
func hellGateReachable(terrain model.TerrainMap, x, y uint16) map[[2]uint16]bool {
	start := [2]uint16{x, y}
	seen := map[[2]uint16]bool{start: true}
	queue := [][2]uint16{start}
	for cursor := 0; cursor < len(queue); cursor++ {
		from := queue[cursor]
		for dy := -1; dy <= 1; dy++ {
			for dx := -1; dx <= 1; dx++ {
				nx, ny := int(from[0])+dx, int(from[1])+dy
				if nx < 1700 || nx > 1949 || ny < 3530 || ny > 3849 {
					continue
				}
				to := [2]uint16{uint16(nx), uint16(ny)}
				if seen[to] || !terrain.RouteHeightCompatible(from[0], from[1], to[0], to[1]) {
					continue
				}
				if dx != 0 && dy != 0 && !terrain.Walkable(to[0], from[1]) && !terrain.Walkable(from[0], to[1]) {
					continue
				}
				seen[to] = true
				queue = append(queue, to)
			}
		}
	}
	return seen
}

func TestHellGateInstalledRouteReachesEveryPopulation(t *testing.T) {
	w, inst, _ := hellGateVersion2Test(t)
	stage := instanceStages(&inst.Config)[0]
	entry := hellGateReachable(w.terrain, stage.X, stage.Y)
	killHellGateTestMob(t, w, inst, hellGatePopulationIDs(inst)[0])
	assertReachable := func(reachable map[[2]uint16]bool) {
		t.Helper()
		for _, ids := range []map[uint16]struct{}{inst.MobIDs, inst.NPCIDs} {
			for id := range ids {
				mob := w.mobsByID[id]
				if mob == nil || !reachable[[2]uint16{mob.X, mob.Y}] {
					if mob == nil {
						t.Fatalf("encounter actor %d is missing", id)
					}
					t.Fatalf("actor %d at %d,%d is unreachable (%d reachable tiles)", id, mob.X, mob.Y, len(reachable))
				}
			}
		}
	}
	assertReachable(entry)
	for _, portal := range inst.Config.HellGate.Portals {
		if !entry[[2]uint16{portal.SourceX, portal.SourceY}] {
			t.Fatal("an entrance portal is unreachable from the corridor")
		}
		attribute := w.terrain.Attribute[(int(portal.SourceY)/4)*model.AttributeWidth+int(portal.SourceX)/4]
		if attribute&0x10 == 0 {
			t.Fatal("entrance route has no native teleport attribute")
		}
	}
	portal := inst.Config.HellGate.Portals[0]
	arena := hellGateReachable(w.terrain, portal.DestX, portal.DestY)
	if entry[[2]uint16{portal.DestX, portal.DestY}] {
		t.Fatal("fixture no longer covers the disconnected entrance that caused the missing mobs")
	}
	for _, route := range inst.Config.HellGate.Portals {
		if !arena[[2]uint16{route.DestX, route.DestY}] {
			t.Fatalf("portal destination %d,%d cannot reach the other rooms or Dionys", route.DestX, route.DestY)
		}
	}
	if !w.spawnHellGateQuadrants(inst, w.now()) {
		t.Fatal("room creation failed")
	}
	assertReachable(arena)
	for _, room := range inst.Config.HellGate.Quadrants {
		if !w.spawnHellGateLich(inst, uint8(room.ID)) {
			t.Fatal("Lich creation failed")
		}
	}
	assertReachable(arena)
	if !w.spawnHellGateFinal(inst, w.now()) {
		t.Fatal("defense creation failed")
	}
	assertReachable(arena)
}

func TestHellGateInstalledPortalsRelocateWithinPopulatedInstance(t *testing.T) {
	for _, phase := range []string{"lich_puzzle", "protection"} {
		t.Run(phase, func(t *testing.T) {
			w, inst, _ := hellGateVersion2Test(t)
			if phase == "lich_puzzle" {
				if !w.spawnHellGateQuadrants(inst, w.now()) {
					t.Fatal("room creation failed")
				}
			} else if !w.spawnHellGateFinal(inst, w.now()) {
				t.Fatal("defense creation failed")
			}
			player := w.playersByID[inst.MemberIDs[0]]
			for index, route := range inst.Config.HellGate.Portals {
				t.Run(fmt.Sprintf("portal_%d", index+1), func(t *testing.T) {
					player.X, player.Y = route.SourceX, route.SourceY
					player.Char.X, player.Char.Y = player.X, player.Y
					w.updatePlayerSpatial(player)
					beforeGold := player.Char.Gold
					w.onReqTeleport(player.Session, make([]byte, 16))
					reachable := hellGateReachable(w.terrain, route.DestX, route.DestY)
					if !reachable[[2]uint16{player.X, player.Y}] ||
						!hellGatePortalArenaFor(inst.Config.HellGate, route.DestX, route.DestY).contains(player.X, player.Y) ||
						w.positionOccupiedInGameplaySpace(player.X, player.Y, itemInstanceGameplaySpace(inst), nil, player, nil) {
						t.Fatalf("portal %d failed to select an unoccupied reachable room tile: %d,%d", index+1, player.X, player.Y)
					}
					if w.playerRuntimeInstanceID(player.ID) != inst.RuntimeID ||
						!itemInstanceHasMember(inst, player.ID) || player.Char.Gold != beforeGold || inst.State != phase {
						t.Fatal("portal changed membership, price or encounter phase")
					}
					for _, ids := range []map[uint16]struct{}{inst.MobIDs, inst.NPCIDs} {
						for id := range ids {
							mob := w.mobsByID[id]
							if mob == nil || !reachable[[2]uint16{mob.X, mob.Y}] || mob.InstanceID != inst.RuntimeID {
								t.Fatalf("portal %d cannot reach encounter actor %d in the same instance", index+1, id)
							}
						}
					}
				})
			}
		})
	}
}

func TestHellGateVersion2RestoreRetainsProtectorHealth(t *testing.T) {
	w, inst, _ := hellGateVersion2Test(t)
	w.removeHellGateMobs(inst)
	inst.State, inst.HellGateDionysHP = "protection", 12345
	saved := model.InstanceRuntimeState{State: "protection", HellGateDionysHP: 12345}
	if !w.restoreHellGateVersion2(inst, saved, w.now()) || !w.hellGateDionysAlive(inst) {
		t.Fatal("Dionys defense could not be restored")
	}
	for id := range inst.NPCIDs {
		if w.mobsByID[id].HP != 12345 {
			t.Fatal("restoration healed the protected actor")
		}
	}
	w.removeHellGateMobs(inst)
	saved.HellGateDionysHP = 0
	if w.restoreHellGateVersion2(inst, saved, w.now()) || len(inst.NPCIDs) != 0 {
		t.Fatal("restoration resurrected a dead protected actor")
	}
}

func TestHellGateProtectorAnchorOccupiedByMember(t *testing.T) {
	w, inst, _ := hellGateVersion2Test(t)
	w.removeHellGateMobs(inst)
	anchor := inst.Config.HellGate.FinalNPCs[0]
	player := w.playersByID[inst.MemberIDs[0]]
	player.X, player.Y = anchor.X, anchor.Y
	player.Char.X, player.Char.Y = anchor.X, anchor.Y
	w.updatePlayerSpatial(player)
	if !w.spawnHellGateFinal(inst, w.now()) || !w.hellGateDionysAlive(inst) {
		t.Fatal("a member occupying the protector anchor cancelled the encounter")
	}
	for id := range inst.NPCIDs {
		protector := w.mobsByID[id]
		if protector.X == player.X && protector.Y == player.Y ||
			!w.terrain.LineOfSight(anchor.X, anchor.Y, protector.X, protector.Y) {
			t.Fatal("protector relocation overlapped a member or crossed a terrain barrier")
		}
	}
}
