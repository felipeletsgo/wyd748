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
		t.Fatalf("refined damage=%d, want 28", got)
	}
}

func TestItemAbilityUsesHighSanctificationTable(t *testing.T) {
	def := model.ItemDef{Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 100}}}
	item := model.Item{Index: 100, Eff: [6]byte{116, 250}} // +15 encoding
	if got := itemAbility(item, def, "EF_DAMAGE"); got != 370 {
		t.Fatalf("damage +15=%d, want 370", got)
	}
}

func TestItemMetadataIsNeverRefined(t *testing.T) {
	def := model.ItemDef{Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}}
	item := model.Item{Index: 100, Eff: [6]byte{116, 250}}
	if got := itemAbility(item, def, "EF_VOLATILE"); got != 2 {
		t.Fatalf("volatile metadata was refined to %d", got)
	}
}

func TestDualWieldPassivesFollow748Books(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, Unique: 7, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 100}}},
		101: {Index: 101, Unique: 7, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: 80}}},
	}}
	// 7.48 TransKnight bit 9 is Speed Up, not the server's Two-Hand Master,
	// so the same-type share stays 50%: 100 + 80*50/100.
	ch := &model.Char{Class: 0, LearnedSkill: 1 << 9}
	ch.Equip[6], ch.Equip[7] = model.Item{Index: 100}, model.Item{Index: 101}
	if got := w.equipmentDamage(ch); got != 140 {
		t.Fatalf("dual wield TK with Speed Up=%d, want 140", got)
	}
	// Two Weapon Fighting adds ten points to the second weapon share.
	ch.Class, ch.LearnedSkill = 3, 1<<(82-72)
	if got := w.equipmentDamage(ch); got != 148 {
		t.Fatalf("dual wield HT=%d, want 148", got)
	}
	ch.LearnedSkill = 1 << (90 - 72)
	if got := w.equipmentDamage(ch); got != 140 {
		t.Fatalf("Life Drain incorrectly changed the dual-weapon share: %d", got)
	}
}

func TestHuntressPassivesUseCorrectSkillBits(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, Unique: 42},
		101: {Index: 101, Unique: 7},
	}}
	ch := &model.Char{Class: 3, Score: testScore(model.Score{
		Attack: 100, Dex: 150, Critical: 2, Range: 1, Mastery: [4]uint32{0, 52, 100, 99},
	})}
	// Hunter's Vision affects drops, not critical, in 7.48.
	ch.RuntimeScore = testScore(*ch.Score)
	ch.LearnedSkill = 1 << (91 - 72)
	w.applyPassiveSkills(ch)
	if got := effectiveScore(ch).Critical; got != 2 {
		t.Fatalf("Hunter's Vision changed critical=%d, want 2", got)
	}
	if got := effectiveScore(ch).Attack; got != 100 {
		t.Fatalf("Hunter's Vision changed attack directly: %d", got)
	}

	ch.RuntimeScore = testScore(*ch.Score)
	ch.LearnedSkill = 1 << (90 - 72)
	w.applyPassiveSkills(ch)
	if got := effectiveScore(ch).Critical; got != 2 {
		t.Fatalf("Life Drain incorrectly received the Vision bonus: %d", got)
	}

	ch.RuntimeScore = testScore(*ch.Score)
	ch.LearnedSkill = 1 << (92 - 72)
	w.applyPassiveSkills(ch)
	if got := effectiveScore(ch).Range; got != 2 {
		t.Fatalf("Improved Range range=%d, want 2", got)
	}

	// The owner-selected bow/claw percentage uses Trade. Finishers do not stack it.
	ch.Equip[6] = model.Item{Index: 100}
	for _, tc := range []struct {
		learned uint32
		want    uint32
	}{
		{1 << (82 - 72), 115},
		{1<<(82-72) | 1<<(87-72), 115},
		{1<<(82-72) | 1<<(95-72), 115},
	} {
		ch.RuntimeScore = testScore(*ch.Score)
		ch.LearnedSkill = tc.learned
		w.applyPassiveSkills(ch)
		if got := effectiveScore(ch).Attack; got != tc.want {
			t.Fatalf("Two Weapon Fighting attack=%d, want %d", got, tc.want)
		}
	}
	ch.Equip[6] = model.Item{Index: 101}
	ch.RuntimeScore = testScore(*ch.Score)
	ch.LearnedSkill = 1 << (82 - 72)
	w.applyPassiveSkills(ch)
	if got := effectiveScore(ch).Attack; got != 100 {
		t.Fatalf("Two Weapon Fighting applied to Unique 7: %d", got)
	}
}

func TestBeastMasterShieldPassiveRaisesDefenseByFivePercent(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, Pos: 128, StaticEffects: []model.StaticEffect{{Name: "EF_AC", Value: 69}}},
		101: {Index: 101, Pos: 64, StaticEffects: []model.StaticEffect{{Name: "EF_AC", Value: 69}}},
	}}
	ch := &model.Char{Class: 2, Score: testScore(model.Score{Defense: 200})}
	ch.RuntimeScore = testScore(*ch.Score)
	ch.LearnedSkill = 1 << (67 - 48)
	ch.Equip[7] = model.Item{Index: 100}
	w.applyPassiveSkills(ch)
	if got := effectiveScore(ch).Defense; got != 210 {
		t.Fatalf("Torment Shield defense=%d, want 210", got)
	}

	ch.RuntimeScore = testScore(*ch.Score)
	ch.Equip[7] = model.Item{Index: 101}
	w.applyPassiveSkills(ch)
	if got := effectiveScore(ch).Defense; got != 200 {
		t.Fatalf("Torment Shield activated without a shield: %d", got)
	}
}

func TestEquipmentCriticalUsesCurrentScoreScale(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_CRITICAL", Value: 40}}},
	}}
	ch := &model.Char{Class: 0, Score: testScore(model.Score{Str: 8, Int: 4, Dex: 7, Con: 6})}
	ch.Equip[1] = model.Item{Index: 100}
	w.recalcPlayer(ch)
	// EF_CRITICAL 40 is shown as 4.0% on the item; Critical uses the same
	// tenths-of-a-percent unit, so the character gains exactly 4.0%.
	if got := effectiveScore(ch).Critical; got != 40 {
		t.Fatalf("critical=%d, want 40 (4.0%%)", got)
	}
}

func TestFourthMasteryAddsNoHPOrDefense(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{}}
	ch := &model.Char{Class: 3, Score: testScore(model.Score{Level: 100, Str: 8, Int: 4, Dex: 7, Con: 6})}
	w.recalcPlayer(ch)
	baseHP, baseDefense := effectiveScore(ch).MaxHP, effectiveScore(ch).Defense
	ch.Score.Mastery[3] = 200
	w.recalcPlayer(ch)
	if got := effectiveScore(ch).MaxHP; got != baseHP {
		t.Fatalf("fourth mastery changed MaxHP %d -> %d", baseHP, got)
	}
	if got := effectiveScore(ch).Defense; got != baseDefense {
		t.Fatalf("fourth mastery changed defense %d -> %d", baseDefense, got)
	}
}

func TestCanEquipIsServerAuthoritative(t *testing.T) {
	def := model.ItemDef{Index: 100, Pos: 1 << 6, ReqLevel: 10, ReqStr: 20,
		StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 1}}}
	w := &World{items: map[uint16]model.ItemDef{100: def}}
	ch := &model.Char{Class: 0, Score: testScore(model.Score{Level: 10, Str: 20})}
	item := model.Item{Index: 100}
	if !w.canEquip(ch, item, 6) {
		t.Fatal("valid item was rejected")
	}
	if w.canEquip(ch, item, 7) {
		t.Fatal("client equipped an item in an unauthorized slot")
	}
	necklace := model.Item{Index: 640}
	w.items[640] = model.ItemDef{Index: 640, Pos: 1 << 9}
	if !w.canEquip(ch, necklace, 9) {
		t.Fatal("native necklace slot rejected a valid necklace")
	}
	ch.Class = 1
	if w.canEquip(ch, item, 6) {
		t.Fatal("client bypassed the class restriction")
	}
}

func TestAdvancedEvolutionEquipUsesMortalBodyAndIgnoresRequirements(t *testing.T) {
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
	for _, evolution := range []string{archEvolution, "celestial", "subcelestial"} {
		t.Run(evolution, func(t *testing.T) {
			// TK body (face 9 => 9/10 = 0), HT skill class.
			ch := &model.Char{
				Class: 3, Evolution: evolution,
				Score: testScore(model.Score{
					Level: 1, Str: 1, Int: 1, Dex: 1, Con: 1,
				}),
			}
			ch.Equip[0] = model.Item{Index: 9}

			if !w.canEquip(ch, model.Item{Index: tkArmor}, 2) {
				t.Fatal("advanced TK should equip TK armor without stat requirements")
			}
			if w.canEquip(ch, model.Item{Index: htArmor}, 2) {
				t.Fatal("HT skill class cannot make a TK body wear HT armor")
			}
			if !w.canEquip(ch, model.Item{Index: fmWeapon}, 6) {
				t.Fatal("advanced evolution should equip any class weapon without stat requirements")
			}
			if w.canEquip(ch, model.Item{Index: fmWeapon}, 7) {
				t.Fatal("requirement exemption must not bypass the item's position mask")
			}
		})
	}
}

func TestMortalStillChecksClassAndRequirements(t *testing.T) {
	def := model.ItemDef{
		Index: 100, Pos: 1 << 6, ReqLevel: 10, ReqStr: 20,
		StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 1 << 0}},
	}
	w := &World{items: map[uint16]model.ItemDef{100: def}}
	item := model.Item{Index: 100}

	lowTK := &model.Char{Class: 0, Score: testScore(model.Score{Level: 1, Str: 1})}
	if w.canEquip(lowTK, item, 6) {
		t.Fatal("Mortal must not receive the Arch requirement exemption")
	}
	strongHT := &model.Char{Class: 3, Score: testScore(model.Score{Level: 100, Str: 100})}
	if w.canEquip(strongHT, item, 6) {
		t.Fatal("Mortal must not use another class's weapon")
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
		t.Fatalf("bow range=%d, want 10", got)
	}
	ch.Equip[6] = model.Item{}
	if got := w.physicalAttackRange(ch); got != attackRange {
		t.Fatalf("attack without a ranged weapon=%d, want %d", got, attackRange)
	}

	// Spectral Force (101 / LearnedSkill bit 29) adds one tile to the
	// calculated range, including melee attacks without EF_RANGE.
	ch.LearnedSkill = 1 << 29
	if got := w.physicalAttackRange(ch); got != attackRange+1 {
		t.Fatalf("melee with Spectral Force=%d, want %d", got, attackRange+1)
	}
	ch.Equip[6] = model.Item{Index: 816}
	if got := w.physicalAttackRange(ch); got != 11 {
		t.Fatalf("bow with Spectral Force=%d, want 11", got)
	}
}

func TestAttackSkillClassificationKeepsRangedNormalAttackPhysical(t *testing.T) {
	ch := &model.Char{Class: 3} // Huntress: global indices 72..95.
	if isLearnedClassSkill(ch, 0) {
		t.Fatal("residual skill field in a bow attack must not become a skill")
	}

	ch.LearnedSkill = 1 << 2
	if !isLearnedClassSkill(ch, 74) {
		t.Fatal("learned skill from the character's class should reach onSkillAttack")
	}
	if isLearnedClassSkill(ch, 50) {
		t.Fatal("another class's skill must not execute")
	}
}

func TestSkillPointsUseServerSkillDataCost(t *testing.T) {
	w := &World{
		items:  map[uint16]model.ItemDef{1: {Index: 1}},
		skills: map[int]model.SkillDef{0: {Index: 0, SkillPoint: 24}},
	}
	ch := &model.Char{Class: 0, LearnedSkill: 1, Score: testScore(model.Score{
		Level: 10, MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100,
		Str: 8, Int: 4, Dex: 7, Con: 6,
	})}
	w.recalcPlayer(ch)
	if ch.Score.SkillPts != 6 { // level*3 (30) - SkillData[0].SkillPoint (24)
		t.Fatalf("skill points=%d, want 6", ch.Score.SkillPts)
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
	binary.LittleEndian.PutUint16(multi[44:46], 1001)
	binary.LittleEndian.PutUint16(multi[48:50], 1002)
	if req := parseAttackSkill(multi); req.Skill != 35 || req.Motion != 11 ||
		req.TargetID != 1001 || req.SecondaryTargetID != 1002 {
		t.Fatalf("multi request=%+v", req)
	}
}

func TestSkillManaCostUsesMasteryAndSaveMana(t *testing.T) {
	skill := model.SkillDef{ManaSpent: 100}
	if got := skillManaCost(skill, 40, 25); got != 90 { // 100*120%*75%
		t.Fatalf("mana=%d, want 90", got)
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
		Score: testScore(model.Score{
			Level: 1, MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100,
			Str: 8, Int: 4, Dex: 7, Con: 6,
		}),
	}
	ch.Score.ResistFire, ch.Score.ResistIce = 5, 6
	ch.Score.ResistHoly, ch.Score.ResistThunder = 7, 60
	ch.Equip[1] = model.Item{Index: 100}
	w.recalcPlayer(ch)
	if got := effectiveScore(ch).ResistFire; got != 35 {
		t.Fatalf("fire=%d, want 35", got)
	}
	if got := effectiveScore(ch).ResistIce; got != 46 {
		t.Fatalf("ice=%d, want 46", got)
	}
	if got := effectiveScore(ch).ResistHoly; got != 57 {
		t.Fatalf("holy=%d, want 57", got)
	}
	if got := effectiveScore(ch).ResistThunder; got != 100 {
		t.Fatalf("thunder=%d, want cap 100", got)
	}
}

func TestRecalcPlayerIncludesNativeNecklaceSlot(t *testing.T) {
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
	ch := &model.Char{Class: 0, Score: testScore(model.Score{
		Level: 1, MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100,
		Str: 8, Int: 4, Dex: 7, Con: 6,
	})}
	ch.Equip[0] = model.Item{Index: 1}
	ch.Equip[9] = model.Item{Index: 100, Eff: [6]byte{47, 3, 48, 4}}
	w.recalcPlayer(ch)
	if effectiveScore(ch).RegenHP != 5 || effectiveScore(ch).RegenMP != 6 {
		t.Fatalf("necklace regen HP/MP=%d/%d, want 5/6",
			effectiveScore(ch).RegenHP, effectiveScore(ch).RegenMP)
	}
	if bodyMesh(ch)[9] == 0 || clientEquipProjection(ch)[9] != ch.Equip[9] {
		t.Fatal("native necklace was omitted from the client projection")
	}
}

func TestRecalcPlayerRemovesEquipmentBonuses(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{
			{Name: "EF_AC", Value: 80}, {Name: "EF_STR", Value: 20},
		}},
	}}
	ch := &model.Char{Score: testScore(model.Score{
		Level: 10, Defense: 10, Attack: 10, MaxHP: 100, MaxMP: 100,
		CurHP: 100, CurMP: 100, Str: 8, Int: 4, Dex: 7, Con: 6,
	})}
	ch.Equip[2] = model.Item{Index: 100}
	w.recalcPlayer(ch)
	equippedDefense, equippedStr := playerDefense(ch), playerStr(ch)
	ch.Equip[2] = model.Item{}
	w.recalcPlayer(ch)
	if playerDefense(ch) >= equippedDefense || playerStr(ch) >= equippedStr {
		t.Fatalf("bonus remained: equipped def/str=%d/%d removed=%d/%d",
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
	ch := &model.Char{Class: 1, Score: &model.Score{
		Version: model.ScoreVersion, Level: 400,
		MaxHP: 60, CurHP: 60, MaxMP: 65, CurMP: 65,
		Str: 5, Int: 308, Dex: 5, Con: 5,
	}}

	ch.Equip[6] = model.Item{Index: 100, Eff: [6]byte{43, 234}} // +11
	w.recalcPlayer(ch)
	plus11, amp11 := playerMagicAttack(ch), effectiveScore(ch).MagicAmp

	ch.Equip[6] = model.Item{Index: 100, Eff: [6]byte{43, 250}} // +15
	w.recalcPlayer(ch)
	plus15, amp15 := playerMagicAttack(ch), effectiveScore(ch).MagicAmp

	if amp11 != 55 || amp15 != 92 {
		t.Fatalf("incorrect refined amplification: +11=%d +15=%d", amp11, amp15)
	}
	if plus11 != 1_247 || plus15 != 1_825 || plus15 <= plus11 {
		t.Fatalf("MATK did not scale with refinement: +11=%d +15=%d", plus11, plus15)
	}
}
