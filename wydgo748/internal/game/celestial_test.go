package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func celestialCharacter(evolution string, level uint32) *model.Char {
	natural := baseClassStats[0]
	return &model.Char{
		Name:      "Celestial",
		Evolution: evolution,
		Score: testScore(model.Score{
			Level: level, Str: uint32(natural[0]), Int: uint32(natural[1]),
			Dex: uint32(natural[2]), Con: uint32(natural[3]),
			MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100,
		}),
	}
}

func TestCelestialArchTierAndInitialCythera(t *testing.T) {
	tests := []struct {
		level   uint32
		tier    byte
		cythera uint16
		bonus   int
	}{
		{354, 0, 0, 0},
		{355, 1, 3500, 100},
		{369, 1, 3500, 100},
		{370, 2, 3500, 300},
		{379, 2, 3500, 300},
		{380, 3, 3501, 600},
		{397, 3, 3501, 600},
		{398, 4, 3501, 900},
		{399, 5, 3502, 1200},
	}
	for _, test := range tests {
		tier := celestialArchTier(test.level)
		if tier != test.tier || celestialCytheraForTier(tier) != test.cythera ||
			celestialArchTierBonus(tier) != test.bonus {
			t.Errorf("level=%d: tier/cythera/bonus=%d/%d/%d; want %d/%d/%d",
				test.level, tier, celestialCytheraForTier(tier),
				celestialArchTierBonus(tier), test.tier, test.cythera, test.bonus)
		}
	}
}

func TestCelestialCrossFormLevelBonusKeepsAllocationSeparate(t *testing.T) {
	ch := celestialCharacter("celestial", 190)
	ch.ArchCrystals = 4
	ch.CelestialArchTier = 5
	ch.AlternateCelestial = &model.CelestialForm{
		Evolution: "subcelestial", Class: 3, Face: model.Item{Index: 18},
		Score: testScore(model.Score{Level: 100, Str: 5, Int: 5, Dex: 5, Con: 5}),
	}
	// 1001 base + 400 crystals + 1200 Arch band + 1900 active form
	// + 600 alternate form + 290 on reaching internal level 190.
	if got, want := celestialStatusPointBudget(ch), 5391; got != want {
		t.Fatalf("Celestial budget=%d, want %d", got, want)
	}
	syncStatusPoints(ch)
	if ch.Score.StatusPts != 5391 {
		t.Fatalf("balance with natural stats=%d, want 5391", ch.Score.StatusPts)
	}
	ch.Score.Str += 391
	syncStatusPoints(ch)
	if ch.Score.StatusPts != 5000 {
		t.Fatalf("balance did not deduct distributed attributes: %d", ch.Score.StatusPts)
	}
}

func TestCelestialSkillAndMasteryBudgets(t *testing.T) {
	ch := celestialCharacter("subcelestial", 100)
	ch.SkillPointBonus = 9
	if got, want := skillPointBudget(ch), 1909; got != want {
		t.Fatalf("skill budget=%d, want %d", got, want)
	}
	ch.Score.Mastery = [4]uint32{100, 200, 50, 5}
	syncMasteryPoints(ch)
	if ch.Score.MasterPts != 500 {
		t.Fatalf("remaining mastery=%d, want 500", ch.Score.MasterPts)
	}
	if masteryPointLimit(ch, 1) != 200 {
		t.Fatalf("common Celestial branch was not capped at 200")
	}
	ch.LearnedSkill |= 1 << 7
	if masteryPointLimit(ch, 1) != 255 {
		t.Fatalf("eighth skill did not unlock mastery 255")
	}
	ch.Score.Level = 200
	if got, want := skillPointBudget(ch), 2210; got != want {
		t.Fatalf("skill budget above threshold=%d, want %d", got, want)
	}
}

func TestCelestialClassHPMPGrowthUsesNative748NonMortalColumns(t *testing.T) {
	w := &World{}
	for class := byte(0); class < 4; class++ {
		ch := &model.Char{
			Name:      "Growth",
			Class:     class,
			Evolution: "celestial",
			Score:     newCelestialScore(class, nil),
		}
		ch.Score.Level = 100
		w.recalcPlayer(ch)
		bonusHP, bonusMP := celestialHPMPBonus(int(class), 0)
		wantHP := uint32(baseClassHPMP[class][0]+archHPPerLevel[class]*100) + uint32(bonusHP)
		wantMP := uint32(baseClassHPMP[class][1]+archMPPerLevel[class]*100) + uint32(bonusMP)
		if playerMaxHP(ch) != wantHP || playerMaxMP(ch) != wantMP {
			t.Errorf("class=%d HP/MP=%d/%d, want %d/%d",
				class, playerMaxHP(ch), playerMaxMP(ch), wantHP, wantMP)
		}
	}
}

func TestEvolutionInitialCombatBasesMatchNative748(t *testing.T) {
	w := &World{}
	main := &model.Char{Class: 0, Evolution: "celestial", Score: newCelestialScore(0, nil)}
	w.recalcPlayer(main)
	if main.Score.Attack != 488 || main.Score.Defense != 954 {
		t.Fatalf("base Celestial ATK/DEF=%d/%d, want 488/954",
			main.Score.Attack, main.Score.Defense)
	}
	// Derived values follow the 7.59 score (DEX/8 attack, no DEX defense);
	// only the 488/954 base is data.
	if playerAttack(main) != 889 || playerDefense(main) != 954 {
		t.Fatalf("Celestial level 0 score ATK/DEF=%d/%d, want 889/954",
			playerAttack(main), playerDefense(main))
	}
	if playerMaxHP(main) != 1680 || playerMaxMP(main) != 345 {
		t.Fatalf("Celestial level 0 score HP/MP=%d/%d, want 1680/345",
			playerMaxHP(main), playerMaxMP(main))
	}

	sub := &model.Char{Class: 0, Evolution: "subcelestial", Score: newCelestialScore(0, nil)}
	w.recalcPlayer(sub)
	if sub.Score.Attack != 488 || sub.Score.Defense != 954 {
		t.Fatalf("base SubCelestial ATK/DEF=%d/%d, want 488/954",
			sub.Score.Attack, sub.Score.Defense)
	}
	if playerAttack(sub) != 889 || playerDefense(sub) != 954 {
		t.Fatalf("SubCelestial level 0 score ATK/DEF=%d/%d, want 889/954",
			playerAttack(sub), playerDefense(sub))
	}
}

func TestCelestialCrystalBasesMatchNative748Branch(t *testing.T) {
	wants := []struct {
		crystals byte
		defense  uint32
		hp, mp   int64
	}{
		{0, 954, 1600, 300},
		{1, 954, 1600, 380},
		{2, 984, 1600, 380},
		{3, 984, 1680, 380},
		{4, 1004, 1740, 440},
	}
	for _, want := range wants {
		if got := celestialBaseDefense(want.crystals); got != want.defense {
			t.Errorf("crystals=%d DEF=%d, want %d", want.crystals, got, want.defense)
		}
		hp, mp := celestialHPMPBonus(0, want.crystals)
		if hp != want.hp || mp != want.mp {
			t.Errorf("crystals=%d HP/MP bonus=%d/%d, want %d/%d",
				want.crystals, hp, mp, want.hp, want.mp)
		}
	}
}

func TestArchGrowthAndInitialPointBudgetsMatchNative748(t *testing.T) {
	w := &World{}
	for class := byte(0); class < 4; class++ {
		natural := baseClassStats[class]
		ch := &model.Char{
			Class: class, Evolution: archEvolution,
			Score: testScore(model.Score{
				Level: 100, Attack: [...]uint32{5, 6, 5, 9}[class], Defense: 4,
				MaxHP: uint32(baseClassHPMP[class][0]), MaxMP: uint32(baseClassHPMP[class][1]),
				Str: uint32(natural[0]), Int: uint32(natural[1]),
				Dex: uint32(natural[2]), Con: uint32(natural[3]),
			}),
		}
		w.recalcPlayer(ch)
		wantHP := uint32(baseClassHPMP[class][0] + archHPPerLevel[class]*100)
		wantMP := uint32(baseClassHPMP[class][1] + archMPPerLevel[class]*100)
		if playerMaxHP(ch) != wantHP || playerMaxMP(ch) != wantMP {
			t.Errorf("class=%d Arch HP/MP=%d/%d, want %d/%d",
				class, playerMaxHP(ch), playerMaxMP(ch), wantHP, wantMP)
		}
		if ch.Score.SkillPts != uint32(mortalSkillPointBudget(100)+168) {
			t.Errorf("class=%d Arch skill points=%d", class, ch.Score.SkillPts)
		}
		if ch.Score.MasterPts != 312 {
			t.Errorf("class=%d Arch mastery points=%d, want 312", class, ch.Score.MasterPts)
		}
	}
}

func TestCelestialProgressionLocksAndExperienceCurve(t *testing.T) {
	ch := celestialCharacter("celestial", 0)
	levels, applied := grantExp(ch, ^uint32(0))
	if levels != 39 || ch.Score.Level != 39 ||
		ch.Exp != celestialNextLevel[39] || applied != celestialNextLevel[39] {
		t.Fatalf("trava 40 atravessada: levels=%d level=%d exp=%d applied=%d",
			levels, ch.Score.Level, ch.Exp, applied)
	}
	if _, applied = grantExp(ch, 1); applied != 0 {
		t.Fatalf("Celestial received EXP while still locked at 40: %d", applied)
	}

	ch.CelestialLevel40Unlocked = true
	_, _ = grantExp(ch, ^uint32(0))
	if ch.Score.Level != 89 || ch.Exp != celestialNextLevel[89] {
		t.Fatalf("trava 90 atravessada: level=%d exp=%d", ch.Score.Level, ch.Exp)
	}
	ch.CelestialLevel90Unlocked = true
	_, _ = grantExp(ch, ^uint32(0))
	if ch.Score.Level != maxCelestialLevel ||
		ch.Exp != celestialNextLevel[len(celestialNextLevel)-1] {
		t.Fatalf("cap Celestial incorreto: level=%d exp=%d", ch.Score.Level, ch.Exp)
	}

	sub := celestialCharacter("subcelestial", 0)
	_, _ = grantExp(sub, ^uint32(0))
	if sub.Score.Level != maxCelestialLevel {
		t.Fatalf("SubCelestial received Celestial-only locks: %d", sub.Score.Level)
	}
}

func TestCelestialCombatExperienceUsesNative748Divisors(t *testing.T) {
	tests := []struct {
		level uint32
		want  uint32
	}{
		// internal level + 1 is the displayed level used by GetExpApply.
		{0, 450}, {39, 450}, {40, 360}, {79, 360},
		{80, 327}, {99, 327}, {100, 300}, {149, 300},
		{150, 276}, {189, 276}, {190, 257}, {199, 257},
	}
	for _, test := range tests {
		ch := celestialCharacter("celestial", test.level)
		if got := combatExperienceByEvolution(ch, 18_000); got != test.want {
			t.Errorf("level=%d: EXP=%d, want %d", test.level, got, test.want)
		}
	}

	sub := celestialCharacter("subcelestial", 100)
	if got := combatExperienceByEvolution(sub, 18_000); got != 300 {
		t.Fatalf("SubCelestial did not share the Celestial curve: %d", got)
	}
}

func TestArchCombatExperienceUsesNative748Divisors(t *testing.T) {
	tests := []struct {
		level uint32
		want  uint32
	}{
		{0, 1_200_000}, // displayed 1: 100%
		{4, 1_188_118}, // displayed 5: divisor 101
		{254, 794_701}, // displayed 255: divisor 151
		{255, 600_000}, // displayed 256: divisor 2
		{280, 300_000}, // displayed 281: divisor 4
		{300, 150_000}, // displayed 301: divisor 8
		{320, 100_000}, // displayed 321: divisor 12
		{340, 75_000},  // displayed 341: divisor 16
		{350, 60_000},  // displayed 351: divisor 20
		{360, 50_000},  // displayed 361: divisor 24
		{370, 42_857},  // displayed 371: divisor 28
		{380, 15_789},  // displayed 381: divisor 76
		{390, 13_333},  // displayed 391: divisor 90
		{395, 12_000},  // displayed 396: divisor 100
		{398, 10_000},  // displayed 399: divisor 120
	}
	for _, test := range tests {
		ch := celestialCharacter("arch", test.level)
		if got := combatExperienceByEvolution(ch, 1_200_000); got != test.want {
			t.Errorf("internal level=%d: EXP=%d, want %d", test.level, got, test.want)
		}
	}
}

func TestMortalCombatExperienceRemainsUnreduced(t *testing.T) {
	ch := celestialCharacter("", 398)
	if got := combatExperienceByEvolution(ch, 1_200_000); got != 1_200_000 {
		t.Fatalf("advanced evolution curve reached Mortal: %d", got)
	}
}

func TestCelestialCytheraMilestonesReplaceInsteadOfStack(t *testing.T) {
	ch := celestialCharacter("subcelestial", 120)
	ch.Equip[1] = model.Item{Index: 3500, Eff: [6]byte{43, 10}}
	if !updateCelestialCythera(ch) ||
		ch.Equip[1].Eff != [6]byte{43, 10, 3, 30, 4, 40} ||
		ch.CelestialCytheraTier != 1 {
		t.Fatalf("milestone 121 incorreto: tier=%d item=%+v",
			ch.CelestialCytheraTier, ch.Equip[1])
	}
	if updateCelestialCythera(ch) {
		t.Fatal("same milestone was applied twice")
	}
	ch.Score.Level = 198
	if !updateCelestialCythera(ch) ||
		ch.Equip[1].Eff != [6]byte{43, 10, 3, 50, 4, 70} ||
		ch.CelestialCytheraTier != 4 {
		t.Fatalf("final milestone did not replace the previous one: %+v", ch.Equip[1])
	}

	plain := celestialCharacter("subcelestial", 120)
	plain.Equip[1] = model.Item{Index: 3500}
	if !updateCelestialCythera(plain) ||
		plain.Equip[1].Eff != [6]byte{3, 30, 4, 40} {
		t.Fatalf("unrefined Cythera gained an artificial effect: %+v", plain.Equip[1])
	}

	tinted := celestialCharacter("subcelestial", 120)
	tinted.Equip[1] = model.Item{Index: 3500, Eff: [6]byte{121, 234, 8, 99}}
	if !updateCelestialCythera(tinted) ||
		tinted.Equip[1].Eff != [6]byte{43, 234, 3, 30, 4, 40} {
		t.Fatalf("Cythera did not normalize/rebuild the native pairs: %+v", tinted.Equip[1])
	}
}

func newCelestialWorld(t *testing.T, level uint32) (*World, *Player, *craftStore) {
	t.Helper()
	p, _ := networkedTestPlayer(1, "Celestial", 2100, 2100)
	st := &craftStore{}
	w := worldWithNetworkedPlayers(p)
	w.store = st
	p.Char.Class = 2
	p.Char.Evolution = archEvolution
	p.Char.ArchMortalLevel = maxMortalLevel
	p.Char.ArchCrystals = 0 // not a requirement in WYD 7.48
	p.Char.Score = testScore(model.Score{
		Level: level, Str: 500, Int: 600, Dex: 700, Con: 800,
		MaxHP: 5000, CurHP: 4000, MaxMP: 3000, CurMP: 2000,
	})
	p.Char.Equip[0] = model.Item{Index: 17}
	p.Char.Equip[model.CapeSlot] = model.Item{
		Index: 3191, UID: "11111111111141118111111111111111",
		Eff: [6]byte{43, 9, 3, 20},
	}
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}
	return w, p, st
}

func reenterCelestialTestCharacter(t *testing.T, w *World, p *Player) {
	t.Helper()
	if p == nil || p.Account == nil || len(p.Account.Chars) == 0 {
		t.Fatal("test account has no character for re-entry")
	}
	p.CharSlot = 0
	p.Char = &p.Account.Chars[0]
	p.ID = 1
	p.InWorld = true
	p.X, p.Y = 2100, 2100
	p.Visible = make(map[uint16]struct{})
	w.playersByID[p.ID] = p
	w.indexPlayerCharacter(p)
	w.updatePlayerSpatial(p)
}

func TestIdealCreatesCelestialWithoutCrystalPrerequisite(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	observer := addZonePlayer(w, 9, 1000, 1000, 500)
	observerPackets := observer.Session.QueuedPacketsForTest()
	packetsBefore := p.Session.QueuedPacketsForTest()
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	ch := &p.Account.Chars[0]

	if st.saves != 1 || ch.Evolution != "celestial" ||
		ch.Score.Level != 0 || ch.Exp != 0 ||
		ch.Score.Str != uint32(baseClassStats[2][0]) ||
		ch.Score.Int != uint32(baseClassStats[2][1]) ||
		ch.Score.Dex != uint32(baseClassStats[2][2]) ||
		ch.Score.Con != uint32(baseClassStats[2][3]) {
		t.Fatalf("incomplete Celestial creation: saves=%d char=%+v",
			st.saves, ch)
	}
	if ch.CelestialArchTier != 5 || ch.Equip[1].Index != 3502 ||
		ch.Equip[model.CapeSlot].Index != 3197 ||
		ch.Equip[model.CapeSlot].Eff != [6]byte{} ||
		ch.Equip[0].Eff[3] != celestialFaceType ||
		ch.LearnedSkill != celestialSoulBit ||
		ch.Score.MasterPts != 855 || ch.Inv[0].Index != 0 {
		t.Fatalf("recompensas Celestial incorretas: tier=%d cythera=%d cape=%d learn=%X mastery=%d item=%d",
			ch.CelestialArchTier, ch.Equip[1].Index,
			ch.Equip[model.CapeSlot].Index, ch.LearnedSkill,
			ch.Score.MasterPts, ch.Inv[0].Index)
	}
	if p.InWorld || p.Char != nil || p.ID != 0 ||
		p.Session.QueuedPacketsForTest() < packetsBefore+2 {
		t.Fatal("Celestial creation did not return to selection with the updated list")
	}
	if observer.Session.QueuedPacketsForTest() != observerPackets+1 {
		t.Fatal("Celestial creation did not send exactly one global announcement to the observer")
	}
}

func TestFailedCelestialCreationDoesNotBroadcastOrLeaveWorld(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	observer := addZonePlayer(w, 9, 1000, 1000, 500)
	before := observer.Session.QueuedPacketsForTest()
	st.err = errors.New("postgres unavailable")

	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)

	if observer.Session.QueuedPacketsForTest() != before {
		t.Fatal("unpersisted Celestial creation was announced")
	}
	if !p.InWorld || p.Char == nil || p.Char.Evolution != archEvolution {
		t.Fatal("persistence failure removed the Arch from the world or kept the mutation")
	}
}

func TestSubCelestialHasSeparateProgressionAndSharedBody(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	reenterCelestialTestCharacter(t, w, p)
	p.Char.Score.Level = 120
	p.Char.Exp = celestialNextLevel[120]
	p.Char.Score.Str = 111
	p.Char.SoulInfo = 7
	p.Char.Gold = 123456
	p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}
	p.Char.Affects[0] = model.Affect{
		Type: 2, Value: 1, ExpiresAt: time.Now().Add(time.Hour),
	}
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1763}
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}

	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	if st.saves != 2 || p.Char.Evolution != "celestial" ||
		p.Char.AlternateCelestial == nil ||
		p.Char.AlternateCelestial.Evolution != "subcelestial" ||
		p.Char.AlternateCelestial.Class != 3 ||
		p.Char.AlternateCelestial.Face.Index != 18 ||
		p.Char.AlternateCelestial.Face.Eff[3] != subCelestialFaceType ||
		p.Char.AlternateCelestial.Score.Level != 0 {
		t.Fatalf("SubCelestial incorreta: saves=%d alt=%+v", st.saves, p.Char.AlternateCelestial)
	}

	mysterySlot := -1
	for i := range p.Char.Inv {
		if p.Char.Inv[i].Index == mysteriousStoneItem {
			mysterySlot = i
			break
		}
	}
	if mysterySlot < 0 || itemStackAmount(p.Char.Inv[mysterySlot]) != 10 {
		t.Fatal("Sub creation did not deliver 10 Mysterious Stones")
	}
	if got := counterBalance(p, fameCounter); got != 0 {
		t.Fatalf("Sub creation did not consume 100 Fame: %d", got)
	}
	sharedCythera := p.Char.Equip[1]
	w.useMysteriousStone(
		p.Session, p, &p.Char.Inv[mysterySlot], byte(mysterySlot))

	if p.Char.Evolution != "subcelestial" || p.Char.Class != 3 ||
		p.Char.Score.Level != 0 || p.Char.Score.Str != uint32(baseClassStats[3][0]) ||
		p.Char.SoulInfo != 7 || p.Char.Gold != 123456 ||
		p.Char.Equip[1] != sharedCythera ||
		itemStackAmount(p.Char.Inv[mysterySlot]) != 9 {
		t.Fatalf("swap did not preserve shared fields: %+v", p.Char)
	}
	if p.Char.Affects[0].Type != 0 ||
		p.Char.AlternateCelestial == nil ||
		p.Char.AlternateCelestial.Evolution != "celestial" ||
		p.Char.AlternateCelestial.Score.Level != 120 ||
		p.Char.AlternateCelestial.Score.Str != 111 ||
		p.Char.AlternateCelestial.Affects[0].Type != 2 {
		t.Fatalf("per-form state leaked or was lost: alt=%+v affects=%+v",
			p.Char.AlternateCelestial, p.Char.Affects)
	}
}

func TestMysteriousStoneOnlyWorksInsideNativeCity(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	reenterCelestialTestCharacter(t, w, p)
	p.Char.Score.Level = 120
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1760}
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}
	p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	p.X, p.Y = 100, 100
	before := *p.Char
	w.useMysteriousStone(p.Session, p, &p.Char.Inv[0], 0)
	if p.Char.Evolution != before.Evolution ||
		itemStackAmount(p.Char.Inv[0]) != itemStackAmount(before.Inv[0]) ||
		st.saves != 2 {
		t.Fatal("Mysterious Stone worked outside the city")
	}
}

func TestFuryUnlockAndArcanaComposition(t *testing.T) {
	t.Run("level 90", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Fury", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		st := &craftStore{}
		w.store = st
		p.Char.Evolution = "celestial"
		p.Char.CelestialLevel40Unlocked = true
		p.Char.Score.Level = 89
		p.SpecialCoins = map[string]uint32{fameCounter: 500}
		p.Char.Inv[0] = model.Item{Index: furyStoneItem}
		p.Char.Equip[1] = model.Item{Index: 3501, Eff: [6]byte{43, 9}}
		w.useFuryStone(p.Session, p, &p.Char.Inv[0], 0)
		// The second Cythera matches the equipped one, unrefined, in the slot
		// the Fury Stone left.
		if !p.Char.CelestialLevel90Unlocked || counterBalance(p, fameCounter) != 0 ||
			p.Char.Inv[0].Index != 3501 || itemSanc(p.Char.Inv[0]) != 0 || st.saves != 1 {
			t.Fatalf("destrave 90 falhou: flag=%t fame=%d item=%d saves=%d",
				p.Char.CelestialLevel90Unlocked, counterBalance(p, fameCounter),
				p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("Arcana sucesso e consumo atomico", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Arcana", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		st := &craftStore{}
		w.store, w.rng = st, fixedRNG{value: 1}
		p.Char.Evolution = "celestial"
		p.Char.Score.Level = maxCelestialLevel
		p.SpecialCoins = map[string]uint32{fameCounter: 500}
		p.Char.Inv[0] = model.Item{Index: furyStoneItem}
		for i, stone := 1, uint16(5334); stone <= 5337; i, stone = i+1, stone+1 {
			p.Char.Inv[i] = model.Item{Index: stone}
		}
		p.Char.Equip[1] = model.Item{Index: 3502}
		if !setItemSanc(&p.Char.Equip[1], 11) {
			t.Fatal("scenario could not refine Cythera +11")
		}
		w.useFuryStone(p.Session, p, &p.Char.Inv[0], 0)
		if !p.Char.CelestialArcana || p.Char.Equip[1].Index != arcanaCytheraItem ||
			p.Char.Inv[0].Index != 0 || counterBalance(p, fameCounter) != 0 ||
			st.saves != 1 {
			t.Fatalf("Arcana falhou: flag=%t index=%d fame=%d saves=%d",
				p.Char.CelestialArcana, p.Char.Equip[1].Index,
				counterBalance(p, fameCounter), st.saves)
		}
		for i := 1; i <= 4; i++ {
			if p.Char.Inv[i].Index != 0 {
				t.Fatalf("Secret Stone inv[%d] was not consumed", i)
			}
		}
	})

	t.Run("level 90 needs a slot for the second Cythera", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "FuryFull", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		st := &craftStore{}
		w.store = st
		p.Char.Evolution = "celestial"
		p.Char.CelestialLevel40Unlocked = true
		p.Char.Score.Level = 89
		p.SpecialCoins = map[string]uint32{fameCounter: 500}
		p.Char.Equip[1] = model.Item{Index: 3500}
		for i := 0; i < model.PlayerCarrySlots; i++ {
			p.Char.Inv[i] = model.Item{Index: 413, Eff: [6]byte{61, 2}}
		}
		p.Char.Inv[0] = model.Item{Index: furyStoneItem, Eff: [6]byte{61, 2}}
		w.useFuryStone(p.Session, p, &p.Char.Inv[0], 0)
		if p.Char.CelestialLevel90Unlocked || counterBalance(p, fameCounter) != 500 ||
			itemStackAmount(p.Char.Inv[0]) != 2 || st.saves != 0 {
			t.Fatal("level 90 unlocked without room for the second Cythera")
		}
	})

	t.Run("rollback", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Rollback", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		st := &craftStore{err: errors.New("postgres indisponivel")}
		w.store = st
		p.Char.Evolution = "celestial"
		p.Char.Score.Level = 89
		p.SpecialCoins = map[string]uint32{fameCounter: 500}
		p.Char.Inv[0] = model.Item{Index: furyStoneItem}
		w.useFuryStone(p.Session, p, &p.Char.Inv[0], 0)
		if p.Char.CelestialLevel90Unlocked || p.Char.Inv[0].Index != furyStoneItem ||
			counterBalance(p, fameCounter) != 500 {
			t.Fatal("save failure did not restore Fury Stone/fame/flag")
		}
	})
}

func TestKingCraftsIdealStone(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Ideal", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	st := &craftStore{}
	w.store = st
	p.Char.Evolution = archEvolution
	p.Char.Equip[10] = model.Item{Index: 1742}
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1761}
	for i, stone := 0, uint16(5334); stone <= 5337; i, stone = i+1, stone+1 {
		p.Char.Inv[i] = model.Item{Index: stone}
	}
	if !w.craftIdealStoneAtKing(p.Session, p) || st.saves != 1 {
		t.Fatal("king did not recognize the Ideal Stone recipe")
	}
	if p.Char.Equip[10].Index != 0 || p.Char.Equip[sefirotSlot].Index != 0 {
		t.Fatal("king did not consume Immortality/Sephirot")
	}
	found := false
	for _, item := range p.Char.Inv {
		found = found || item.Index == idealStoneItem
	}
	if !found {
		t.Fatal("king did not deliver the Ideal Stone")
	}
}

func TestEhreCraftsTenMysteriousStones(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Ehre",
		map[uint16]model.ItemDef{mysteriousStoneItem: {Index: mysteriousStoneItem}}, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0], items[1] = model.Item{Index: 5110}, model.Item{Index: 5120}
	items[2] = model.Item{Index: 413}
	setItemAmount(&items[2], 10)
	pos[0], pos[1], pos[2] = 0, 1, 2
	placeItems(p.Char, items, pos)

	w.onCombineEhre(session, buildCombinePacket(items, pos))

	if st.saves != 1 || p.Char.Inv[2].Index != mysteriousStoneItem ||
		itemStackAmount(p.Char.Inv[2]) != 10 {
		t.Fatalf("Ehre did not create a stack of 10: saves=%d item=%+v", st.saves, p.Char.Inv[2])
	}
}

func TestCelestialSoulUsesSharedConfiguration(t *testing.T) {
	w := &World{}
	ch := celestialCharacter("celestial", 100)
	ch.SoulInfo = 5 // +80% STR, +40% CON
	ch.Score.Str, ch.Score.Con = 1000, 500
	ch.Affects[0] = model.Affect{
		Type: 29, Value: 102, ExpiresAt: time.Now().Add(time.Hour),
	}
	applyScore(ch)
	w.applyExtendedAffectStats(ch)
	e := effectiveScore(ch)
	if e.Str != 1800 || e.Con != 700 ||
		e.Attack != ch.Score.Attack+800/3 ||
		e.MaxHP != ch.Score.MaxHP+400 {
		t.Fatalf("Soul Celestial incorreta: STR=%d CON=%d ATK=%d HP=%d",
			e.Str, e.Con, e.Attack, e.MaxHP)
	}
	if !specialSkillLearned(&model.Char{LearnedSkill: celestialSoulBit}, 102) {
		t.Fatal("bit 30 did not unlock skill 102 Soul Limit")
	}
}

// The Soul Limit attribute bonus feeds the derived values like base points:
// INT raises magic attack and MP; DEX raises physical attack (DEX/8) only.
func TestCelestialSoulFeedsDerivedStats(t *testing.T) {
	w := &World{}
	ch := celestialCharacter("celestial", 100)
	ch.Class = 1
	ch.SoulInfo = 2 // +120% INT
	ch.Score.Int = 1000
	ch.Affects[0] = model.Affect{Type: 29, ExpiresAt: time.Now().Add(time.Hour)}
	applyScore(ch)
	base := *ch.RuntimeScore
	w.applyExtendedAffectStats(ch)
	e := effectiveScore(ch)
	wantMagic := base.MagicAttack + uint32((1200/3+1200/30)*(100+4*int64(base.MagicAmp))/100*5/4)
	if e.Int != base.Int+1200 || e.MagicAttack != wantMagic || e.MaxMP != base.MaxMP+2400 {
		t.Fatalf("INT soul: INT=%d magic=%d (want %d) MP=%d", e.Int, e.MagicAttack, wantMagic, e.MaxMP)
	}

	dexChar := celestialCharacter("celestial", 100)
	dexChar.Class = 3
	dexChar.SoulInfo = 3 // +120% DEX
	dexChar.Score.Dex = 1000
	dexChar.Affects[0] = model.Affect{Type: 29, ExpiresAt: time.Now().Add(time.Hour)}
	applyScore(dexChar)
	dexBase := *dexChar.RuntimeScore
	w.applyExtendedAffectStats(dexChar)
	d := effectiveScore(dexChar)
	if d.Dex != dexBase.Dex+1200 || d.Attack != dexBase.Attack+1200/8 || d.Defense != dexBase.Defense {
		t.Fatalf("DEX soul: DEX=%d ATK=%d DEF=%d", d.Dex, d.Attack, d.Defense)
	}
}
