package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestLearnSkillAtMasterPersistsAndRollsBack(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Class = 0
	p.Char.Extended.Level = 20
	p.Char.Extended.SkillPts = 100
	applyExtendedScore(p.Char)
	w.skills = map[int]model.SkillDef{
		0: {Index: 0, Name: "Skill0", SkillPoint: 1},
		1: {Index: 1, Name: "Skill1", SkillPoint: 1},
	}
	w.items[5000] = model.ItemDef{Index: 5000}
	w.items[5001] = model.ItemDef{Index: 5001}
	master := &Mob{
		ID: 1200, X: p.X + 1, Y: p.Y,
		Def: &model.NPCDef{
			Name: "Master", Tipo: model.TipoNPC,
			Extended: &model.ExtendedScore{
				Version: model.ExtendedScoreVersion, Merchant: skillMasterMerchant,
			},
			Vende: []model.Item{{Index: 5000}, {Index: 5001}},
		},
	}
	w.registerMobSpatial(master)
	p.ShopNPC = master.ID

	w.onLearnSkill(p.Session, p, 5000)
	if p.Char.LearnedSkill&1 == 0 || st.saves != 1 {
		t.Fatalf("skill nao foi aprendida/persistida: mask=%X saves=%d", p.Char.LearnedSkill, st.saves)
	}

	st.err = errors.New("save failed")
	w.onLearnSkillAtMaster(p.Session, p, 5001, master.ID)
	if p.Char.LearnedSkill&(1<<1) != 0 {
		t.Fatal("falha de persistencia deixou a segunda skill aprendida")
	}
}

func TestSkillAttackPvEAppliesDamageManaAndDebuff(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 1
	p.Char.LearnedSkill = 1
	p.Char.Extended.Int = 500
	p.Char.Extended.MagicAttack = 1000
	p.Char.Extended.Mastery[1] = 40
	p.Char.Extended.CurMP = 800
	p.Char.Extended.MaxMP = 800
	applyExtendedScore(p.Char)
	skill := model.SkillDef{
		Index: 24, Name: "Magic", InstanceType: 2, InstanceValue: 100,
		ManaSpent: 20, Delay: 1, Range: 6, MaxTarget: 1, Aggressive: 1,
		AffectType: 3, AffectValue: 10, AffectTime: 5,
	}
	w.skills = map[int]model.SkillDef{24: skill}
	mob := &Mob{
		ID: 1300, X: p.X + 2, Y: p.Y, HP: 100_000, GenerIndex: -1,
		Def: testNPCDef(model.ExtendedScore{
			Level: 20, MaxHP: 100_000, CurHP: 100_000, MaxMP: 100,
			Defense: 100, ResistFire: 20,
		}),
	}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	p.show(mob.ID)
	oldMP := playerCurMP(p.Char)

	w.onSkillAttack(p, skillCastRequest{Skill: 24, Motion: 5, TargetID: mob.ID})
	if mob.HP >= 100_000 || mob.TargetID != p.ID || p.CombatTargetID != mob.ID {
		t.Fatalf("ataque PvE nao alterou combate: hp=%d mobTarget=%d playerTarget=%d",
			mob.HP, mob.TargetID, p.CombatTargetID)
	}
	if playerCurMP(p.Char) >= oldMP {
		t.Fatalf("skill nao consumiu mana: %d -> %d", oldMP, playerCurMP(p.Char))
	}
	if activeMobAffect(mob, 3) == nil {
		t.Fatal("debuff da skill nao foi aplicado ao mob")
	}
	if p.SkillReady[24].Before(time.Now()) {
		t.Fatal("cooldown nao foi registrado")
	}
}

func TestSkillAttackSupportHealsAndRejectsCooldown(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 1
	p.Char.LearnedSkill = 1 << 3 // skill global 27, local 3
	p.Char.Extended.Mastery[1] = 40
	p.Char.Extended.CurHP = 100
	p.Char.Extended.MaxHP = 1000
	p.Char.Extended.CurMP = 500
	p.Char.Extended.MaxMP = 500
	applyExtendedScore(p.Char)
	skill := model.SkillDef{
		Index: 27, Name: "Heal", InstanceValue: 100, ManaSpent: 10,
		Delay: 4, Range: 6, MaxTarget: 1,
	}
	w.skills = map[int]model.SkillDef{27: skill}

	w.onSkillAttack(p, skillCastRequest{Skill: 27})
	hpAfterFirst, mpAfterFirst := playerCurHP(p.Char), playerCurMP(p.Char)
	if hpAfterFirst <= 100 || mpAfterFirst >= 500 {
		t.Fatalf("cura nao aplicou HP/MP: hp=%d mp=%d", hpAfterFirst, mpAfterFirst)
	}

	w.onSkillAttack(p, skillCastRequest{Skill: 27})
	if playerCurHP(p.Char) != hpAfterFirst || playerCurMP(p.Char) != mpAfterFirst {
		t.Fatal("recast durante cooldown alterou recursos")
	}
}

func TestSupportBuffManualRecastIsNotBlockedByAutomaticWindow(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 0
	p.Char.LearnedSkill = 1 << 5
	p.Char.Extended.Mastery[1] = 40
	p.Char.Extended.CurMP = 2_000
	p.Char.Extended.MaxMP = 2_000
	for i := range p.Char.ShortSkill {
		p.Char.ShortSkill[i] = 0xFF
	}
	p.Char.ShortSkill[0] = 5
	applyExtendedScore(p.Char)
	skill := model.SkillDef{
		Index: 5, Name: "Aura", ManaSpent: 10, AffectTime: 99,
		TickType: 17, TickValue: 75,
	}
	w.skills = map[int]model.SkillDef{5: skill}

	// O recast manual deve funcionar mesmo muito antes da janela automatica.
	p.Char.Affects[0] = model.Affect{
		Type: 17, Value: 75, Level: 40,
		ExpiresAt: time.Now().Add(2 * time.Minute),
	}
	oldExpiry := p.Char.Affects[0].ExpiresAt
	oldMP := playerCurMP(p.Char)
	w.onSkillAttack(p, skillCastRequest{Skill: 5, Motion: 5, TargetID: p.ID})
	if !p.Char.Affects[0].ExpiresAt.After(oldExpiry) {
		t.Fatal("recast manual acima de 30 segundos foi ignorado")
	}
	if playerCurMP(p.Char) >= oldMP {
		t.Fatal("recast manual nao consumiu mana")
	}

}

func TestSkillAttackPvPBreaksHideAndPreservesOverkillState(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Caster", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(caster, target)
	w.store = &craftStore{}
	w.items = make(map[uint16]model.ItemDef)
	w.ghostShops = make(map[uint16]*GhostShop)
	caster.Char.Class = 0
	caster.Char.LearnedSkill = 1
	caster.Char.Extended.Attack = 50_000
	caster.Char.Extended.MagicAttack = 50_000
	caster.Char.Extended.Int = 10_000
	caster.Char.Extended.CurMP = 1000
	caster.Char.Extended.MaxMP = 1000
	target.Char.Extended.CurHP = 100
	target.Char.Extended.MaxHP = 100
	caster.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: time.Now().Add(time.Minute)}
	applyExtendedScore(caster.Char)
	applyExtendedScore(target.Char)
	w.skills = map[int]model.SkillDef{0: {
		Index: 0, Name: "PvP", InstanceType: 2, InstanceValue: 5000,
		ManaSpent: 10, Delay: 1, Range: 6, MaxTarget: 1, Aggressive: 1,
	}}

	w.onSkillAttack(caster, skillCastRequest{Skill: 0, TargetID: target.ID, Motion: 5})
	if hasActiveAffect(caster.Char, 28) {
		t.Fatal("atacar nao removeu Hide in Shadows")
	}
	if playerCurHP(target.Char) != 0 || target.DeadAt.IsZero() || target.LastAttackerID != caster.ID {
		t.Fatalf("morte PvP incompleta: hp=%d dead=%v attacker=%d",
			playerCurHP(target.Char), target.DeadAt, target.LastAttackerID)
	}
}

func TestBMSummonCastReplacesCreatureFamily(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 2
	p.Char.Extended.Int = 1000
	p.Char.Extended.Con = 500
	p.Char.Extended.Mastery[2] = 255
	applyExtendedScore(p.Char)

	if !w.castSummon(p, model.SkillDef{Index: 56, InstanceValue: 1}, 255) {
		t.Fatal("Condor nao foi invocado")
	}
	if len(w.summons) != summonTemplates[0].maxSummons {
		t.Fatalf("quantidade de Condor=%d, quer %d", len(w.summons), summonTemplates[0].maxSummons)
	}
	for _, summon := range w.summons {
		if summon.SummonerID != p.ID || summon.SummonKind != summonKindBM ||
			summon.Def.Equip.Rosto.Index != summonTemplates[0].face {
			t.Fatalf("Condor invalido: %+v", summon)
		}
	}

	if !w.castSummon(p, model.SkillDef{Index: 57, InstanceValue: 2}, 0) {
		t.Fatal("Javali nao substituiu a familia anterior")
	}
	live := 0
	for _, summon := range w.summons {
		if !summon.Dead {
			live++
			if summon.Def.Equip.Rosto.Index != summonTemplates[1].face {
				t.Fatalf("familia obsoleta permaneceu viva: face=%d", summon.Def.Equip.Rosto.Index)
			}
		}
	}
	if live != 1 {
		t.Fatalf("Javali mastery zero deveria deixar uma evocacao viva: %d", live)
	}
}

func TestContractSummonReplacesPreviousAndFollowsOwnerTarget(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	template := &model.VolatileSummon{
		Name: "Knight", Face: 300, HP: 500, Attack: 300, Defense: 100,
		MoveSpeed: 4, AttackRange: 1,
	}
	if !w.replaceContractSummon(owner, template) {
		t.Fatal("contrato valido nao criou summon")
	}
	var first *Mob
	for _, summon := range w.summons {
		first = summon
	}
	template.Name, template.Face = "Archer", 301
	if !w.replaceContractSummon(owner, template) {
		t.Fatal("segundo contrato nao substituiu o primeiro")
	}
	if !first.Dead {
		t.Fatal("summon de contrato anterior permaneceu vivo")
	}
	live := 0
	for _, summon := range w.summons {
		if !summon.Dead && summon.SummonKind == summonKindContract {
			live++
		}
	}
	if live != 1 {
		t.Fatalf("limite de um contrato ativo falhou: %d", live)
	}
}
