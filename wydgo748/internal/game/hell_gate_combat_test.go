package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestHellGate759SkillSelection(t *testing.T) {
	counts := map[int]int{}
	for roll := 0; roll < 100; roll++ {
		counts[hellGateAttackSkill([4]byte{32, 33, 34, 255}, roll)]++
		if got := hellGateAttackSkill([4]byte{255, 255, 255, 255}, roll); got != -1 {
			t.Fatalf("empty bar selected skill %d", got)
		}
	}
	if counts[32] != 50 || counts[33] != 35 || counts[34] != 15 {
		t.Fatalf("historical selection windows: %v", counts)
	}
}

func TestHellGate759SkillDamageUsesAttackAndCompressedResistance(t *testing.T) {
	m := &Mob{Def: &model.NPCDef{Score: &model.Score{Attack: 3600, MagicAttack: 99999, Int: 9999}}}
	zero := func(int) int { return 0 }
	for _, tc := range []struct{ resist, want uint32 }{{0, 3465}, {200, 1732}, {210, 1697}, {300, 1386}, {320, 1351}} {
		got := hellGateSkillDamage(m, 200, model.ElementalResists{Fire: tc.resist}, model.SkillDef{InstanceType: 2, InstanceValue: 999}, zero, time.Time{})
		if got != tc.want {
			t.Fatalf("resistance %d: damage=%d want=%d", tc.resist, got, tc.want)
		}
	}
}

func TestHellGate759CasterOwnershipAndVersion(t *testing.T) {
	w, inst, _, _ := hellGateDefense759(t)
	w.skills = map[int]model.SkillDef{32: {Index: 32, InstanceType: 2}, 33: {Index: 33, InstanceType: 5}}
	var mage *Mob
	for id := range inst.MobIDs {
		if m := w.mobsByID[id]; m.Def.Name == "HG_SkelMage" {
			mage = m
			break
		}
	}
	if mage == nil {
		t.Fatal("historical mage missing")
	}
	if skill, ok := w.hellGateCombatSkill(mage); !ok || skill.Index != 32 {
		t.Fatal("owned historical mage did not select its imported skill")
	}
	delete(inst.MobIDs, mage.ID)
	if _, ok := w.hellGateCombatSkill(mage); ok {
		t.Fatal("unowned mage cast a spell")
	}
	inst.MobIDs[mage.ID] = struct{}{}
	inst.Config.HellGate.Version = 2
	if _, ok := w.hellGateCombatSkill(mage); ok {
		t.Fatal("restored version-2 AI changed")
	}
	inst.Config.HellGate.Version = 3
	w.skills = nil
	if _, ok := w.hellGateCombatSkill(mage); ok {
		t.Fatal("missing skill did not fall back to physical combat")
	}
	if _, ok := w.hellGateCombatSkill(nil); ok {
		t.Fatal("nil actor selected a spell")
	}
}

func TestHellGate759CasterTickDamagesProtectorOnlyAtAttackDeadline(t *testing.T) {
	w, inst, clock, guardian := hellGateDefense759(t)
	w.skills = map[int]model.SkillDef{32: {Index: 32, InstanceType: 2}}
	var mage *Mob
	for id := range inst.MobIDs {
		if m := w.mobsByID[id]; m.Def.Name == "HG_SkelMage" {
			mage = m
			break
		}
	}
	if mage == nil {
		t.Fatal("historical mage missing")
	}
	mage.X, mage.Y, mage.TargetID = guardian.X+1, guardian.Y, guardian.ID
	w.activeMobs = map[uint16]*Mob{mage.ID: mage}
	before := guardian.HP
	w.tickActiveMobActions(clock.Now())
	if got := before - guardian.HP; got != 3465 || inst.HellGateDionysHP != guardian.HP {
		t.Fatalf("historical mage tick damage=%d, want 3465", got)
	}
	w.tickActiveMobActions(clock.Now())
	if guardian.HP != before-3465 {
		t.Fatal("caster bypassed attack deadline")
	}
	clock.Advance(inst.HardDeadline.Sub(clock.Now()))
	w.tickActiveMobActions(clock.Now())
	if guardian.HP != before-3465 {
		t.Fatal("expired caster damaged protector")
	}
}

func TestHellGate759FormationAnchors(t *testing.T) {
	spawn := model.VolatileInstanceSpawn{X: 100, Y: 100, Formation759: 1}
	for i, want := range [][2]uint16{{100, 100}, {101, 101}, {99, 101}, {101, 99}, {99, 99}} {
		x, y, ok := hellGateFormationAnchor(spawn, i)
		if !ok || [2]uint16{x, y} != want {
			t.Fatalf("member %d: %d,%d valid=%v", i, x, y, ok)
		}
	}
	spawn.Formation759 = 2
	x, y, ok := hellGateFormationAnchor(spawn, 0)
	if !ok || x != 101 || y != 101 {
		t.Fatal("followers-only formation included a leader")
	}
	if _, _, ok := hellGateFormationAnchor(spawn, 12); ok {
		t.Fatal("oversized formation accepted")
	}
	spawn.X, spawn.Y = 65535, 65535
	if _, _, ok := hellGateFormationAnchor(spawn, 0); ok {
		t.Fatal("formation coordinate wrapped")
	}
	spawn.Formation759 = 0
	x, y, ok = hellGateFormationAnchor(spawn, 1)
	if !ok || x != 65535 || y != 65535 {
		t.Fatal("ordinary placement changed")
	}
}

func TestHellGate759CasterTickTargetsOnlyItsOwnMembers(t *testing.T) {
	w, p, _, merchant := hellStoreWorld(t)
	inst := attachDionysStore(w, p, merchant)
	inst.State = "lich_puzzle"
	mage := &Mob{ID: 1300, InstanceID: inst.RuntimeID, X: p.X + 1, Y: p.Y, HP: 32000, Awake: true, TargetID: p.ID,
		Def: &model.NPCDef{Name: "HG_SkelMage", Tipo: model.TipoMonstro, SkillBar: [4]byte{32, 33, 32, 255},
			Score: &model.Score{Level: 399, Attack: 3600, MaxHP: 32000}}}
	inst.MobIDs = map[uint16]struct{}{mage.ID: {}}
	w.registerMobSpatial(mage)
	w.activeMobs = map[uint16]*Mob{mage.ID: mage}
	w.skills = map[int]model.SkillDef{32: {Index: 32, InstanceType: 2}}
	w.rng = fixedRNG{value: 0}
	p.Char.RuntimeScore = nil
	p.Char.Score.MaxHP, p.Char.Score.CurHP = 50000, 50000
	p.show(mage.ID)
	w.tickActiveMobActions(w.now())
	if playerCurHP(p.Char) >= 50000 || p.LastAttackerID != mage.ID || mage.NextAttack.IsZero() {
		t.Fatal("historical caster did not execute its owned-member attack")
	}
	before := playerCurHP(p.Char)
	mage.NextAttack = time.Time{}
	inst.MemberIDs = nil
	w.rebuildPlayerInstanceIndex()
	w.tickActiveMobActions(w.now())
	if playerCurHP(p.Char) != before {
		t.Fatal("caster attacked a nonmember")
	}
}
