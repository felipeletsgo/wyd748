package game

import (
	"testing"
)

func TestBossMitigationDoesNotOverflowWideDamage(t *testing.T) {
	w, mob, _, _ := newBossTestWorld(t, testBossProfile(t), ^uint32(0))
	for _, absorbed := range []int{0, 1, 50, 99, 100} {
		if !setMobOwnedMobAffectAt(mob, mob, bossShieldAffect, absorbed, 1, 60, w.now()) {
			t.Fatal("shield admission failed")
		}
		for _, damage := range []uint32{1, 100, 100000000, ^uint32(0)} {
			want := uint32(uint64(damage) * uint64(100-absorbed) / 100)
			if got := w.bossMitigateDamage(mob, damage); got != want {
				t.Fatalf("damage=%d absorbed=%d: got %d, want %d", damage, absorbed, got, want)
			}
		}
	}
}

func TestBossRegistrationPreservesExistingEncounter(t *testing.T) {
	profile := testBossProfile(t)
	w, mob, boss, _ := newBossTestWorld(t, profile, 1000)
	w.bossSummonAdds(boss, mob, BossActionDef{SummonNPC: "Skeleton", SummonCount: 1})
	if boss.addsAlive() != 1 {
		t.Fatal("add was not spawned")
	}
	if err := w.RegisterBoss(mob.ID, profile); err == nil || w.bossFor(mob.ID) != boss {
		t.Fatal("duplicate registration replaced the encounter")
	}
	w.UnregisterBoss(mob.ID)
	if len(boss.Adds) != 0 || len(w.mobsByID) != 1 {
		t.Fatal("duplicate registration orphaned an add")
	}
}
