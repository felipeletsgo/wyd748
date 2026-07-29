package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestItemAbilityUsesStaticDynamicAndRefine(t *testing.T) {
	def := model.ItemDef{Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 10}},
		DynamicEffectNames: map[byte]string{2: "EF_DAMAGE", 43: "EF_SANC"}}
	item := model.Item{Index: 100, Eff: [6]byte{2, 5, 43, 9}}
	if got := itemAbility(item, def, "EF_DAMAGE"); got != 28 { // (10+5)*1.9
		t.Fatalf("damage refinado=%d, esperado 28", got)
	}
}

func TestItemAbilityUsesHighSanctificationTable(t *testing.T) {
	def := model.ItemDef{Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 100}}}
	item := model.Item{Index: 100, Eff: [6]byte{116, 250}} // codificacao +15
	if got := itemAbility(item, def, "EF_DAMAGE"); got != 370 {
		t.Fatalf("damage +15=%d, esperado 370", got)
	}
}

func TestItemMetadataIsNeverRefined(t *testing.T) {
	def := model.ItemDef{Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}}
	item := model.Item{Index: 100, Eff: [6]byte{116, 250}}
	if got := itemAbility(item, def, "EF_VOLATILE"); got != 2 {
		t.Fatalf("volatile foi refinado para %d", got)
	}
}

func TestDualWieldPassiveAddsClassPercentage(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, Unique: 7, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 100}}},
		101: {Index: 101, Unique: 7, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 80}}},
	}}
	ch := &model.Char{Class: 0, LearnedSkill: 1 << 9}
	ch.Equip[6], ch.Equip[7] = model.Item{Index: 100}, model.Item{Index: 101}
	if got := w.equipmentDamage(ch); got != 152 { // 100 + 65% de 80
		t.Fatalf("dual wield TK=%d, esperado 152", got)
	}
	ch.Class, ch.LearnedSkill = 3, 1<<11
	if got := w.equipmentDamage(ch); got != 148 { // 100 + 60% de 80
		t.Fatalf("dual wield HT=%d, esperado 148", got)
	}
}

func TestEquipmentCriticalUsesCurrentScoreScale(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_CRITICAL", Value: 40}}},
	}}
	ch := &model.Char{Class: 0, Extended: testExtended(model.ExtendedScore{Str: 8, Int: 4, Dex: 7, Con: 6})}
	ch.Equip[1] = model.Item{Index: 100}
	w.recalcPlayer(ch)
	if got := effectiveExtended(ch).Critical; got != 10 {
		t.Fatalf("critical=%d, esperado 10", got)
	}
}

func TestCanEquipIsServerAuthoritative(t *testing.T) {
	def := model.ItemDef{Index: 100, Pos: 1 << 6, ReqLevel: 10, ReqStr: 20,
		StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 1}}}
	w := &World{items: map[uint16]model.ItemDef{100: def}}
	ch := &model.Char{Class: 0, Extended: testExtended(model.ExtendedScore{Level: 10, Str: 20})}
	item := model.Item{Index: 100}
	if !w.canEquip(ch, item, 6) {
		t.Fatal("item valido foi rejeitado")
	}
	if w.canEquip(ch, item, 7) {
		t.Fatal("cliente conseguiu equipar em slot nao autorizado")
	}
	ch.Class = 1
	if w.canEquip(ch, item, 6) {
		t.Fatal("cliente conseguiu ignorar restricao de classe")
	}
}

func TestArchEquipUsesMortalBodyAndIgnoresRequirements(t *testing.T) {
	const (
		tkArmor  = 100
		htArmor  = 101
		fmWeapon = 102
	)
	w := &World{items: map[uint16]model.ItemDef{
		tkArmor: {
			Index: tkArmor, Pos: 1 << 2, ReqLevel: 400, ReqStr: 100_000,
			StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 1 << 0}},
		},
		htArmor: {
			Index: htArmor, Pos: 1 << 2,
			StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 1 << 3}},
		},
		fmWeapon: {
			Index: fmWeapon, Pos: 1 << 6, ReqLevel: 400, ReqInt: 100_000,
			StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 1 << 1}},
		},
	}}
	// Corpo TK (rosto Arch 9 => 9/10 = 0), Sephiroth HT (Class=3).
	ch := &model.Char{
		Class: 3, Evolution: archEvolution,
		Extended: testExtended(model.ExtendedScore{Level: 1, Str: 1, Int: 1, Dex: 1, Con: 1}),
	}
	ch.Equip[0] = model.Item{Index: 9}

	if !w.canEquip(ch, model.Item{Index: tkArmor}, 2) {
		t.Fatal("Arch TK/Sephiroth HT deveria equipar armadura de TK sem requisitos")
	}
	if w.canEquip(ch, model.Item{Index: htArmor}, 2) {
		t.Fatal("Sephiroth HT nao pode transformar o corpo TK em classe de armadura HT")
	}
	if !w.canEquip(ch, model.Item{Index: fmWeapon}, 6) {
		t.Fatal("Arch deveria equipar arma de qualquer classe sem requisitos")
	}
	if w.canEquip(ch, model.Item{Index: fmWeapon}, 7) {
		t.Fatal("isencao do Arch nao pode ignorar o slot permitido pelo item")
	}
}

func TestMortalStillChecksClassAndRequirements(t *testing.T) {
	def := model.ItemDef{
		Index: 100, Pos: 1 << 6, ReqLevel: 10, ReqStr: 20,
		StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 1 << 0}},
	}
	w := &World{items: map[uint16]model.ItemDef{100: def}}
	item := model.Item{Index: 100}

	lowTK := &model.Char{Class: 0, Extended: testExtended(model.ExtendedScore{Level: 1, Str: 1})}
	if w.canEquip(lowTK, item, 6) {
		t.Fatal("Mortal nao pode receber a isencao de requisitos do Arch")
	}
	strongHT := &model.Char{Class: 3, Extended: testExtended(model.ExtendedScore{Level: 100, Str: 100})}
	if w.canEquip(strongHT, item, 6) {
		t.Fatal("Mortal nao pode usar arma de outra classe")
	}
}

func TestPhysicalAttackRangeUsesOnlyMainWeaponRange(t *testing.T) {
	bow := model.ItemDef{Index: 816, StaticEffects: []model.StaticEffect{{Name: "EF_RANGE", Value: 5}}}
	accessory := model.ItemDef{Index: 900, StaticEffects: []model.StaticEffect{{Name: "EF_RANGE", Value: 20}}}
	w := &World{items: map[uint16]model.ItemDef{816: bow, 900: accessory}}
	ch := &model.Char{}
	ch.Equip[6] = model.Item{Index: 816}
	ch.Equip[9] = model.Item{Index: 900}
	if got := w.physicalAttackRange(ch); got != 10 {
		t.Fatalf("alcance do arco=%d, quer 10", got)
	}
	ch.Equip[6] = model.Item{}
	if got := w.physicalAttackRange(ch); got != attackRange {
		t.Fatalf("ataque sem arma de alcance=%d, quer %d", got, attackRange)
	}
}

func TestAttackSkillClassificationKeepsRangedNormalAttackPhysical(t *testing.T) {
	ch := &model.Char{Class: 3} // Huntress: indices globais 72..95.
	if isLearnedClassSkill(ch, 0) {
		t.Fatal("campo residual de skill em ataque de arco nao pode virar skill")
	}

	ch.LearnedSkill = 1 << 2
	if !isLearnedClassSkill(ch, 74) {
		t.Fatal("skill aprendida da propria classe deve seguir para onSkillAttack")
	}
	if isLearnedClassSkill(ch, 50) {
		t.Fatal("skill de outra classe nao pode ser executada")
	}
}

func TestSkillPointsUseServerSkillDataCost(t *testing.T) {
	w := &World{
		items:  map[uint16]model.ItemDef{1: {Index: 1}},
		skills: map[int]model.SkillDef{0: {Index: 0, SkillPoint: 24}},
	}
	ch := &model.Char{Class: 0, LearnedSkill: 1, Extended: testExtended(model.ExtendedScore{
		Level: 10, MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100,
		Str: 8, Int: 4, Dex: 7, Con: 6,
	})}
	w.recalcPlayer(ch)
	if ch.Extended.SkillPts != 6 { // level*3 (30) - SkillData[0].SkillPoint (24)
		t.Fatalf("skill points=%d, esperado 6", ch.Extended.SkillPts)
	}
}

func TestParseAttackSkillCompactAndMulti(t *testing.T) {
	compact := make([]byte, 48)
	binary.LittleEndian.PutUint16(compact[24:26], 7)
	compact[28] = 9
	binary.LittleEndian.PutUint16(compact[44:46], 1001)
	if req := parseAttackSkill(compact); req.Skill != 7 || req.Motion != 9 || req.TargetID != 1001 {
		t.Fatalf("compact skill=%d motion=%d target=%d", req.Skill, req.Motion, req.TargetID)
	}
	multi := make([]byte, 120)
	binary.LittleEndian.PutUint16(multi[24:26], 35)
	multi[28] = 11
	if req := parseAttackSkill(multi); req.Skill != 35 || req.Motion != 11 {
		t.Fatalf("multi skill=%d motion=%d", req.Skill, req.Motion)
	}
}

func TestSkillManaCostUsesMasteryAndSaveMana(t *testing.T) {
	skill := model.SkillDef{ManaSpent: 100}
	if got := skillManaCost(skill, 40, 25); got != 90 { // 100*120%*75%
		t.Fatalf("mana=%d, esperado 90", got)
	}
}

func TestRecalcPlayerCombinesBaseAndEquipmentResistances(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {
			Index: 100,
			StaticEffects: []model.StaticEffect{
				{Name: "EF_RESIST1", Value: 20},
				{Name: "EF_RESIST2", Value: 30},
				{Name: "EF_RESIST3", Value: 40},
				{Name: "EF_RESIST4", Value: 50},
				{Name: "EF_RESISTALL", Value: 10},
			},
		},
	}}
	ch := &model.Char{
		Class: 0,
		Extended: testExtended(model.ExtendedScore{
			Level: 1, MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100,
			Str: 8, Int: 4, Dex: 7, Con: 6,
		}),
	}
	ch.Extended.ResistFire, ch.Extended.ResistIce = 5, 6
	ch.Extended.ResistHoly, ch.Extended.ResistThunder = 7, 60
	ch.Equip[1] = model.Item{Index: 100}
	w.recalcPlayer(ch)
	if got := effectiveExtended(ch).ResistFire; got != 35 {
		t.Fatalf("fogo=%d, quer 35", got)
	}
	if got := effectiveExtended(ch).ResistIce; got != 46 {
		t.Fatalf("gelo=%d, quer 46", got)
	}
	if got := effectiveExtended(ch).ResistHoly; got != 57 {
		t.Fatalf("sagrado=%d, quer 57", got)
	}
	if got := effectiveExtended(ch).ResistThunder; got != 100 {
		t.Fatalf("trovao=%d, quer limite 100", got)
	}
}

func TestRecalcPlayerUsesBaseClassAndItemRegeneration(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		1: {
			Index: 1,
			StaticEffects: []model.StaticEffect{
				{Name: "EF_REGENHP", Value: 2},
				{Name: "EF_REGENMP", Value: 2},
			},
		},
		100: {Index: 100, DynamicEffectNames: map[byte]string{
			47: "EF_REGENHP", 48: "EF_REGENMP",
		}},
	}}
	ch := &model.Char{Class: 0, Extended: testExtended(model.ExtendedScore{
		Level: 1, MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100,
		Str: 8, Int: 4, Dex: 7, Con: 6,
	})}
	ch.Equip[0] = model.Item{Index: 1}
	ch.Equip[9] = model.Item{Index: 100, Eff: [6]byte{47, 3, 48, 4}}
	w.recalcPlayer(ch)
	if effectiveExtended(ch).RegenHP != 5 || effectiveExtended(ch).RegenMP != 6 {
		t.Fatalf("regen HP/MP=%d/%d, quer 5/6", effectiveExtended(ch).RegenHP, effectiveExtended(ch).RegenMP)
	}
}

func TestRecalcPlayerRemovesEquipmentBonuses(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{
			{Name: "EF_AC", Value: 80}, {Name: "EF_STR", Value: 20},
		}},
	}}
	ch := &model.Char{Extended: testExtended(model.ExtendedScore{
		Level: 10, Defense: 10, Attack: 10, MaxHP: 100, MaxMP: 100,
		CurHP: 100, CurMP: 100, Str: 8, Int: 4, Dex: 7, Con: 6,
	})}
	ch.Equip[2] = model.Item{Index: 100}
	w.recalcPlayer(ch)
	equippedDefense, equippedStr := playerDefense(ch), playerStr(ch)
	ch.Equip[2] = model.Item{}
	w.recalcPlayer(ch)
	if playerDefense(ch) >= equippedDefense || playerStr(ch) >= equippedStr {
		t.Fatalf("bonus permaneceu: equipado def/str=%d/%d removido=%d/%d",
			equippedDefense, equippedStr, playerDefense(ch), playerStr(ch))
	}
}

func TestExtendedMagicRecalcUsesRefinedMagicAmplification(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {
			Index: 100,
			StaticEffects: []model.StaticEffect{
				{Name: "EF_MAGIC", Value: 100},
			},
		},
	}}
	ch := &model.Char{Class: 1, Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion, Level: 400,
		MaxHP: 60, CurHP: 60, MaxMP: 65, CurMP: 65,
		Str: 5, Int: 308, Dex: 5, Con: 5,
	}}

	ch.Equip[6] = model.Item{Index: 100, Eff: [6]byte{43, 234}} // +11
	w.recalcPlayer(ch)
	plus11, amp11 := playerMagicAttack(ch), effectiveExtended(ch).MagicAmp

	ch.Equip[6] = model.Item{Index: 100, Eff: [6]byte{43, 250}} // +15
	w.recalcPlayer(ch)
	plus15, amp15 := playerMagicAttack(ch), effectiveExtended(ch).MagicAmp

	if amp11 != 55 || amp15 != 92 {
		t.Fatalf("amplificacao refinada incorreta: +11=%d +15=%d", amp11, amp15)
	}
	if plus11 != 1_247 || plus15 != 1_825 || plus15 <= plus11 {
		t.Fatalf("MATK nao escalou com refinacao: +11=%d +15=%d", plus11, plus15)
	}
}
