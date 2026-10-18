package game

import (
	"testing"
	"time"
)

func hellGateDefense759(t *testing.T) (*World, *ItemInstance, *fakeClock, *Mob) {
	t.Helper()
	w, inst, clock := hellGateVersion2Test(t)
	inst.Config.HellGate.Version = 3
	if !w.spawnHellGateFinal(inst, clock.Now()) {
		t.Fatal("historical defense spawn failed")
	}
	for id := range inst.NPCIDs {
		return w, inst, clock, w.mobsByID[id]
	}
	t.Fatal("Dionys was not created")
	return nil, nil, nil, nil
}

func TestHellGateHistoricalAttackersMarchWithoutNearbyPlayers(t *testing.T) {
	w, inst, clock, guardian := hellGateDefense759(t)
	distances := w.hellGateRoute(inst, guardian)
	var attacker *Mob
	for id := range inst.MobIDs {
		mob := w.mobsByID[id]
		if _, found := distances[[2]uint16{mob.X, mob.Y}]; !found {
			t.Fatalf("historical spawn %s at %d,%d cannot march to Dionys", mob.Def.Name, mob.X, mob.Y)
		}
		if !mob.Awake || w.hellGateMarchTarget(mob) != guardian {
			t.Fatalf("distant attacker %d is asleep or lacks its event objective", id)
		}
		if attacker == nil || chebyshev(mob.X, mob.Y, guardian.X, guardian.Y) >
			chebyshev(attacker.X, attacker.Y, guardian.X, guardian.Y) {
			attacker = mob
		}
	}
	if chebyshev(attacker.X, attacker.Y, guardian.X, guardian.Y) <= mobActivationRange {
		t.Fatal("fixture does not exercise long-range acquisition")
	}
	// Isolate a complete route from group congestion. Every production spawn's
	// static route was checked above; dynamic occupancy is tested separately.
	for id := range inst.MobIDs {
		if id != attacker.ID {
			w.removeMobInstance(w.mobsByID[id])
			delete(inst.MobIDs, id)
		}
	}
	beforeHP := guardian.HP
	for step := 0; step < 1200 && guardian.HP == beforeHP; step++ {
		oldX, oldY := attacker.X, attacker.Y
		clock.Advance(250 * time.Millisecond)
		w.tickMobCombat(clock.Now(), 0, 1, true)
		w.tickActiveMobActions(clock.Now())
		if chebyshev(oldX, oldY, attacker.X, attacker.Y) > 1 ||
			!w.terrain.Walkable(attacker.X, attacker.Y) ||
			!w.terrain.RouteHeightCompatible(oldX, oldY, attacker.X, attacker.Y) ||
			attacker.InstanceID != inst.RuntimeID {
			t.Fatal("march crossed a blocked edge, skipped tiles, or escaped its runtime")
		}
	}
	if guardian.HP >= beforeHP || inst.HellGateDionysHP != guardian.HP || attacker.TargetID != guardian.ID {
		t.Fatalf("distant attacker never reached and damaged Dionys: at=%d,%d target=%d,%d HP=%d",
			attacker.X, attacker.Y, guardian.X, guardian.Y, guardian.HP)
	}
}

func TestHellGateHistoricalReplacementPreservesSurvivorsAndDeadline(t *testing.T) {
	w, inst, clock, guardian := hellGateDefense759(t)
	ids := hellGatePopulationIDs(inst)
	previous := make(map[uint16]bool, len(ids))
	for _, id := range ids {
		previous[id] = true
	}
	survivor := w.mobsByID[ids[1]]
	survivor.HP--
	guardian.HP--
	inst.HellGateDionysHP = guardian.HP
	survivorHP, guardianHP, deadline := survivor.HP, guardian.HP, inst.HardDeadline
	spawn := inst.HellGateDefenseSpawns[ids[0]]
	killHellGateTestMob(t, w, inst, ids[0])
	if inst.Remaining != 251 || len(inst.HellGateReplacements) != 1 {
		t.Fatal("a single casualty did not schedule its own generator")
	}
	w.tickHellGate(inst, clock.Now())
	if inst.Remaining != 252 || len(inst.HellGateReplacements) != 0 ||
		w.mobsByID[survivor.ID] != survivor || survivor.HP != survivorHP || guardian.HP != guardianHP ||
		!inst.HardDeadline.Equal(deadline) || len(inst.HellGateDefenseSpawns) != 252 {
		t.Fatal("replacement reset survivors, protector, population, or the absolute deadline")
	}
	found := false
	for id, replacement := range inst.HellGateDefenseSpawns {
		if !previous[id] && replacement == spawn && w.mobsByID[id].HP == w.mobsByID[id].Def.Score.MaxHP {
			found = true
		}
	}
	if !found {
		t.Fatal("replacement lost its original generator and NPC")
	}
	killHellGateTestMob(t, w, inst, survivor.ID)
	clock.Advance(deadline.Sub(clock.Now()))
	w.tickHellGate(inst, clock.Now())
	if inst.Remaining != 251 {
		t.Fatal("deadline spawned another attacker")
	}
}

func TestHellGateHistoricalMarchWaitsForOccupiedRoute(t *testing.T) {
	w, inst, clock, guardian := hellGateDefense759(t)
	attacker := w.mobsByID[hellGatePopulationIDs(inst)[0]]
	for id := range inst.MobIDs {
		if id != attacker.ID {
			w.removeMobInstance(w.mobsByID[id])
			delete(inst.MobIDs, id)
		}
	}
	distances := w.hellGateRoute(inst, guardian)
	current := [2]uint16{attacker.X, attacker.Y}
	var blockers []*Mob
	for dy := -1; dy <= 1; dy++ {
		for dx := -1; dx <= 1; dx++ {
			tile := [2]uint16{uint16(int(attacker.X) + dx), uint16(int(attacker.Y) + dy)}
			if distance, ok := distances[tile]; ok && distance < distances[current] {
				blocker := &Mob{ID: uint16(5000 + len(blockers)), X: tile[0], Y: tile[1], HP: 1, Def: attacker.Def, InstanceID: inst.RuntimeID}
				w.registerMobSpatial(blocker)
				blockers = append(blockers, blocker)
			}
		}
	}
	if len(blockers) == 0 {
		t.Fatal("fixture lacks a descending route")
	}
	w.marchHellGateAttacker(attacker, guardian, clock.Now())
	if current != [2]uint16{attacker.X, attacker.Y} || !attacker.NextMove.After(clock.Now()) {
		t.Fatal("attacker crossed occupied tiles or failed to pace its retry")
	}
	for _, blocker := range blockers {
		w.removeMobInstance(blocker)
	}
	clock.Advance(time.Second)
	w.marchHellGateAttacker(attacker, guardian, clock.Now())
	if current == [2]uint16{attacker.X, attacker.Y} {
		t.Fatal("attacker failed to resume after congestion cleared")
	}
}

func TestHellGateHistoricalMarchObjectiveBoundaries(t *testing.T) {
	w, inst, clock, guardian := hellGateDefense759(t)
	attacker := w.mobsByID[hellGatePopulationIDs(inst)[0]]
	cases := []struct {
		name   string
		change func() func()
	}{
		{"old version", func() func() { inst.Config.HellGate.Version = 2; return func() { inst.Config.HellGate.Version = 3 } }},
		{"wrong phase", func() func() { inst.State = "lich_puzzle"; return func() { inst.State = "protection" } }},
		{"summon", func() func() { attacker.SummonerID = 1; return func() { attacker.SummonerID = 0 } }},
		{"unowned", func() func() {
			delete(inst.MobIDs, attacker.ID)
			return func() { inst.MobIDs[attacker.ID] = struct{}{} }
		}},
		{"other runtime", func() func() { guardian.InstanceID = "other"; return func() { guardian.InstanceID = inst.RuntimeID } }},
		{"dead protector", func() func() { hp := guardian.HP; guardian.HP = 0; return func() { guardian.HP = hp } }},
		{"deadline", func() func() {
			deadline := inst.HardDeadline
			inst.HardDeadline = clock.Now()
			return func() { inst.HardDeadline = deadline }
		}},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			restore := tc.change()
			defer restore()
			if w.hellGateMarchTarget(attacker) != nil {
				t.Fatal("invalid actor acquired the Hell Gate objective")
			}
		})
	}
}
