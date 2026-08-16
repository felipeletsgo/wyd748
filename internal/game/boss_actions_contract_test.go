package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func bossContractProfile() *BossProfile {
	return &BossProfile{
		ID: "contract", NPCName: "Boss", InitialPhase: 1,
		Phases: map[BossPhaseID]BossPhase{
			1: {ID: 1, Name: "one"},
			2: {ID: 2, Name: "two"},
		},
	}
}

func bossContractMob(id uint16) *Mob {
	return &Mob{ID: id, X: 100, Y: 100, HP: 100, Def: &model.NPCDef{
		Name: "Boss", Tipo: model.TipoMonstro,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, Level: 10, MaxHP: 100, CurHP: 100, MaxMP: 50, CurMP: 50, Attack: 30, MagicAttack: 40, Int: 20},
	}}
}

func TestBossChangePhaseCoversMissingSamePendingAndSuccessfulTransition(t *testing.T) {
	w := testSpatialWorld(nil)
	mob := bossContractMob(1000)
	profile := bossContractProfile()
	boss := newBossRuntime(mob.ID, profile)

	w.bossChangePhase(boss, mob, BossActionDef{Phase: 99}, 0)
	if boss.Phase != 1 {
		t.Fatal("fase inexistente alterou runtime")
	}
	w.bossChangePhase(boss, mob, BossActionDef{Phase: 1}, 0)
	if boss.Phase != 1 {
		t.Fatal("transicao para a propria fase alterou runtime")
	}

	boss.Pending = &BossPendingAction{Generation: 3}
	boss.Generation = 3
	w.bossChangePhase(boss, mob, BossActionDef{Phase: 2}, 0)
	if boss.Phase != 2 || boss.Pending != nil || boss.Generation != 4 {
		t.Fatalf("transicao nao cancelou pendencia: phase=%d pending=%+v gen=%d", boss.Phase, boss.Pending, boss.Generation)
	}
}

func TestExecuteBossActionDispatchesAllKindsWithoutInventingUnknownBehavior(t *testing.T) {
	w := testSpatialWorld(nil)
	mob := bossContractMob(1000)
	profile := bossContractProfile()
	boss := newBossRuntime(mob.ID, profile)
	w.bosses = map[uint16]*BossRuntime{mob.ID: boss}
	w.mobsByID[mob.ID] = mob
	w.appendMobInstance(mob)

	w.executeBossAction(boss, mob, BossActionDef{Kind: ActionChangePhase, Phase: 2}, BossEvent{}, 0)
	if boss.Phase != 2 {
		t.Fatal("dispatch de change phase falhou")
	}
	w.executeBossAction(boss, mob, BossActionDef{Kind: ActionCastSkill, SkillID: 999}, BossEvent{SourceID: 999}, 0)
	w.executeBossAction(boss, mob, BossActionDef{Kind: ActionSummon, SummonNPC: "missing", SummonCount: 1}, BossEvent{}, 0)
	w.executeBossAction(boss, mob, BossActionDef{Kind: ActionApplyEffect, AffectType: 20, AffectDuration: 1}, BossEvent{}, 0)
	if activeMobAffectAt(mob, 20, w.now()) == nil {
		t.Fatal("dispatch de apply effect falhou")
	}
	w.executeBossAction(boss, mob, BossActionDef{Kind: ActionRemoveEffect, AffectType: 20}, BossEvent{}, 0)
	if activeMobAffectAt(mob, 20, w.now()) != nil {
		t.Fatal("dispatch de remove effect falhou")
	}
	// Kind fora do enum e deliberadamente no-op.
	w.executeBossAction(boss, mob, BossActionDef{Kind: BossActionKind(255)}, BossEvent{}, 0)
}

func TestBossSkillDamageCoversNilPhysicalFallbackScalingResistanceAndClamp(t *testing.T) {
	now := time.Unix(2_400_000_000, 0)
	zeroRNG := func(int) int { return 0 }
	if bossSkillDamageWithRNG(nil, nil, model.SkillDef{}, zeroRNG, now) != 0 {
		t.Fatal("skill damage nil deveria ser zero")
	}

	target := &Player{Char: &model.Char{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100, MaxMP: 50, CurMP: 50,
		Defense: 10, ResistFire: 50,
	}}}
	mob := bossContractMob(1000)
	skill := model.SkillDef{Index: 7, InstanceValue: 50, InstanceType: 1}
	damage := bossSkillDamageWithRNG(mob, target, skill, zeroRNG, now)
	if damage == 0 || damage > maxExtendedStat {
		t.Fatalf("dano magico escalado invalido: %d", damage)
	}

	// Sem MagicAttack/INT/Level o caminho cai no ataque fisico efetivo.
	fallback := bossContractMob(1001)
	fallback.Def.Extended.MagicAttack = 0
	fallback.Def.Extended.Int = 0
	fallback.Def.Extended.Level = 0
	fallback.Def.Extended.Attack = 50
	if got := bossSkillDamageWithRNG(fallback, target, model.SkillDef{}, zeroRNG, now); got == 0 {
		t.Fatal("fallback fisico produziu zero")
	}

	immuneTarget := &Player{Char: &model.Char{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100, MaxMP: 1, CurMP: 1,
		ResistFire: 100,
	}}}
	if got := bossSkillDamageWithRNG(mob, immuneTarget, model.SkillDef{InstanceType: 1}, zeroRNG, now); got != 1 {
		t.Fatalf("resistencia total deveria cair no clamp minimo 1, got=%d", got)
	}
}

func TestBossSummonAddsCoversMissingCapacityAndCreation(t *testing.T) {
	w := testSpatialWorld(nil)
	bossMob := bossContractMob(1000)
	w.mobsByID[bossMob.ID] = bossMob
	w.appendMobInstance(bossMob)
	profile := bossContractProfile()
	boss := newBossRuntime(bossMob.ID, profile)

	w.bossSummonAdds(boss, bossMob, BossActionDef{SummonNPC: "Missing", SummonCount: 2})
	if len(boss.Adds) != 0 {
		t.Fatal("NPC inexistente gerou adds")
	}

	addDef := model.NPCDef{Name: "Add_Name", Tipo: model.TipoMonstro,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 25, CurHP: 25}}
	w.npcs = append(w.npcs, addDef)
	boss.Adds[9000] = struct{}{}
	w.bossSummonAdds(boss, bossMob, BossActionDef{SummonNPC: "Add Name", SummonCount: 2, SummonMax: 1})
	if len(boss.Adds) != 1 {
		t.Fatal("SummonMax cheio ainda criou add")
	}
	delete(boss.Adds, 9000)

	w.bossSummonAdds(boss, bossMob, BossActionDef{SummonNPC: "Add Name", SummonCount: 2, SummonMax: 3})
	if len(boss.Adds) != 2 {
		t.Fatalf("adds criados=%d, esperado 2", len(boss.Adds))
	}
	for id := range boss.Adds {
		add := w.mobsByID[id]
		if add == nil || add.LeaderID != bossMob.ID || add.Def.Name != "Add_Name" || add.HP != 25 {
			t.Fatalf("add invalido id=%d mob=%+v", id, add)
		}
	}
}

func TestBossEffectMitigationClearAnnouncementAndNPCLookup(t *testing.T) {
	now := time.Unix(2_400_000_000, 0)
	w := testSpatialWorld(nil)
	w.clock = newFakeClock(now)
	mob := bossContractMob(1000)
	boss := newBossRuntime(mob.ID, bossContractProfile())
	w.bosses = map[uint16]*BossRuntime{mob.ID: boss}

	if w.bossMitigateDamage(mob, 0) != 0 {
		t.Fatal("dano zero foi alterado")
	}
	delete(w.bosses, mob.ID)
	if w.bossMitigateDamage(mob, 100) != 100 {
		t.Fatal("mob comum teve mitigacao de boss")
	}
	w.bosses[mob.ID] = boss
	if w.bossMitigateDamage(mob, 100) != 100 {
		t.Fatal("boss sem escudo teve mitigacao")
	}

	if !setMobOwnedMobAffectAt(mob, mob, bossShieldAffect, 25, 0, 10, now) {
		t.Fatal("nao aplicou escudo")
	}
	if got := w.bossMitigateDamage(mob, 100); got != 75 {
		t.Fatalf("mitigacao 25%%=%d", got)
	}
	for i := range mob.Affects {
		if mob.Affects[i].Type == bossShieldAffect {
			mob.Affects[i].Value = 150
		}
	}
	if got := w.bossMitigateDamage(mob, 100); got != 0 {
		t.Fatalf("mitigacao saturada=%d", got)
	}

	if clearMobAffect(nil, 1) || clearMobAffect(mob, 0) || clearMobAffect(mob, 99) {
		t.Fatal("clearMobAffect reportou remocao inexistente")
	}
	if !clearMobAffect(mob, bossShieldAffect) || activeMobAffectAt(mob, bossShieldAffect, now) != nil {
		t.Fatal("escudo nao foi removido")
	}

	w.announceToMobView(mob, "")
	w.npcs = []model.NPCDef{{Name: "Ancient_Guard"}, {Name: "Other"}}
	if got := w.npcDefByName("Ancient Guard"); got == nil || got.Name != "Ancient_Guard" {
		t.Fatalf("lookup por generName falhou: %+v", got)
	}
	if w.npcDefByName("Missing") != nil {
		t.Fatal("NPC inexistente foi resolvido")
	}
}
