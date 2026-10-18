package game

import (
	"testing"
	"time"
)

func TestHellGateInstalledSkeletonDamageAgainstDionys(t *testing.T) {
	w, rule := installedHellGate(t)
	start := time.Date(2026, 10, 17, 12, 2, 0, 0, time.UTC)
	w.clock, w.rng = newFakeClock(start), fixedRNG{value: 0}
	inst := &ItemInstance{
		Config:       cfgCopy(*rule.Instance),
		RuntimeID:    "hell-gate-defense-test",
		State:        "protection",
		MemberIDs:    []uint16{1},
		LeaderID:     1,
		HardDeadline: start.Add(18 * time.Minute),
		ScheduleEnd:  start,
	}
	// Preserve the published version-2 balance regression. Version-3 marching
	// and its historical mixed population have separate integration coverage.
	inst.Config.HellGate.Version = 2
	w.itemInstances[inst.RuntimeID] = inst
	if !w.spawnHellGateFinal(inst, start) {
		t.Fatal("installed Hell Gate protection population could not be created")
	}

	attackers := make([]*Mob, 0, len(inst.MobIDs))
	for id := range inst.MobIDs {
		mob := w.mobsByID[id]
		if mob == nil || mob.Def == nil || mob.Def.Score == nil || mob.Def.Name != "HG_Skeleton" {
			t.Fatalf("final protection attacker %d is not the installed HG_Skeleton: %+v", id, mob)
		}
		attackers = append(attackers, mob)
	}
	if len(attackers) != 30 {
		t.Fatalf("installed final protection loaded %d Skeletons, want 30", len(attackers))
	}

	var protector *Mob
	for id := range inst.NPCIDs {
		mob := w.mobsByID[id]
		if mob == nil || mob.Def == nil || mob.Def.Score == nil || mob.Def.Name != "HG_Dionys" {
			t.Fatalf("final protection NPC %d is not the installed HG_Dionys: %+v", id, mob)
		}
		if protector != nil {
			t.Fatal("installed protection population has more than one Dionys")
		}
		protector = mob
	}
	if protector == nil {
		t.Fatal("installed protection population has no Dionys")
	}
	if got := protector.Def.Score.Defense; got != 200 {
		t.Fatalf("installed HG_Dionys defense = %d, want calibrated 200", got)
	}

	// The production AI supplies this function only after movement, aggro, and
	// line-of-sight checks. This test intentionally bypasses those unknowns and
	// models full contact: every loaded Skeleton can attack each 1.5-second
	// cadence round until the protected NPC dies.
	var firstDamage uint32
	contactRounds := 0
	for now := start; now.Before(inst.HardDeadline) && protector.HP > 0; now = now.Add(mobAttackInterval) {
		contactRounds++
		for _, attacker := range attackers {
			attacker.TargetID = protector.ID
			before := protector.HP
			w.mobAttackNightmareProtector(attacker, protector, now)
			if firstDamage == 0 {
				firstDamage = before - protector.HP
			}
			if protector.HP == 0 {
				break
			}
		}
	}

	if firstDamage <= 1 {
		t.Fatalf("loaded Skeleton -> Dionys damage = %d, want above minimum damage 1", firstDamage)
	}
	if firstDamage != 114 {
		t.Fatalf("loaded Skeleton -> Dionys first damage = %d, want deterministic 114", firstDamage)
	}
	if protector.HP != 0 {
		t.Fatalf("full-contact protection simulation left Dionys at %d HP", protector.HP)
	}
	simulatedSurvival := time.Duration(contactRounds) * mobAttackInterval
	if simulatedSurvival >= 5*time.Minute {
		t.Fatalf("full-contact protection simulation survived %s, want meaningfully below 18m", simulatedSurvival)
	}
	t.Logf("loaded protection calibration: %d Skeletons, first hit %d, full-contact survival %s; movement/aggro remain unmodeled", len(attackers), firstDamage, simulatedSurvival)
}
