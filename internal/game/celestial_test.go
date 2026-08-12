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
		Extended: testExtended(model.ExtendedScore{
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
			t.Errorf("level=%d: tier/cythera/bonus=%d/%d/%d; quer %d/%d/%d",
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
		Extended: testExtended(model.ExtendedScore{Level: 100, Str: 5, Int: 5, Dex: 5, Con: 5}),
	}
	// 1001 base + 400 cristais + 1200 faixa Arch + 1900 forma ativa
	// + 600 forma alterna + 290 ao chegar no nivel interno 190.
	if got, want := celestialStatusPointBudget(ch), 5391; got != want {
		t.Fatalf("budget Celestial=%d, quer %d", got, want)
	}
	syncStatusPoints(ch)
	if ch.Extended.StatusPts != 5391 {
		t.Fatalf("saldo com stats naturais=%d, quer 5391", ch.Extended.StatusPts)
	}
	ch.Extended.Str += 391
	syncStatusPoints(ch)
	if ch.Extended.StatusPts != 5000 {
		t.Fatalf("saldo nao descontou atributos distribuidos: %d", ch.Extended.StatusPts)
	}
}

func TestCelestialSkillAndMasteryBudgets(t *testing.T) {
	ch := celestialCharacter("subcelestial", 100)
	ch.SkillPointBonus = 9
	if got, want := skillPointBudget(ch), 1909; got != want {
		t.Fatalf("skill budget=%d, quer %d", got, want)
	}
	ch.Extended.Mastery = [4]uint32{100, 200, 50, 5}
	syncMasteryPoints(ch)
	if ch.Extended.MasterPts != 500 {
		t.Fatalf("mastery restante=%d, quer 500", ch.Extended.MasterPts)
	}
	if masteryPointLimit(ch, 1) != 200 {
		t.Fatalf("ramo comum Celestial nao ficou limitado a 200")
	}
	ch.LearnedSkill |= 1 << 7
	if masteryPointLimit(ch, 1) != 255 {
		t.Fatalf("oitava skill nao liberou mastery 255")
	}
	ch.Extended.Level = 200
	if got, want := skillPointBudget(ch), 2210; got != want {
		t.Fatalf("skill budget acima do limiar=%d, quer %d", got, want)
	}
}

func TestCelestialClassHPMPGrowthUsesW2PPNonMortalColumns(t *testing.T) {
	w := &World{}
	for class := byte(0); class < 4; class++ {
		ch := &model.Char{
			Name:      "Growth",
			Class:     class,
			Evolution: "celestial",
			Extended:  newCelestialScore(class, nil),
		}
		ch.Extended.Level = 100
		w.recalcPlayer(ch)
		bonusHP, bonusMP := celestialHPMPBonus(int(class), 0)
		wantHP := uint32(baseClassHPMP[class][0]+archHPPerLevel[class]*100) + uint32(bonusHP)
		wantMP := uint32(baseClassHPMP[class][1]+archMPPerLevel[class]*100) + uint32(bonusMP)
		if playerMaxHP(ch) != wantHP || playerMaxMP(ch) != wantMP {
			t.Errorf("classe=%d HP/MP=%d/%d, quer %d/%d",
				class, playerMaxHP(ch), playerMaxMP(ch), wantHP, wantMP)
		}
	}
}

func TestEvolutionInitialCombatBasesMatchNormalW2PP(t *testing.T) {
	w := &World{}
	main := &model.Char{Class: 0, Evolution: "celestial", Extended: newCelestialScore(0, nil)}
	w.recalcPlayer(main)
	if main.Extended.Attack != 488 || main.Extended.Defense != 954 {
		t.Fatalf("base Celestial ATK/DEF=%d/%d, quer 488/954",
			main.Extended.Attack, main.Extended.Defense)
	}
	if playerAttack(main) != 890 || playerDefense(main) != 955 {
		t.Fatalf("score Celestial nivel 0 ATK/DEF=%d/%d, quer 890/955",
			playerAttack(main), playerDefense(main))
	}
	if playerMaxHP(main) != 1680 || playerMaxMP(main) != 345 {
		t.Fatalf("score Celestial nivel 0 HP/MP=%d/%d, quer 1680/345",
			playerMaxHP(main), playerMaxMP(main))
	}

	sub := &model.Char{Class: 0, Evolution: "subcelestial", Extended: newCelestialScore(0, nil)}
	w.recalcPlayer(sub)
	if sub.Extended.Attack != 488 || sub.Extended.Defense != 954 {
		t.Fatalf("base SubCelestial ATK/DEF=%d/%d, quer 488/954",
			sub.Extended.Attack, sub.Extended.Defense)
	}
	if playerAttack(sub) != 890 || playerDefense(sub) != 955 {
		t.Fatalf("score SubCelestial nivel 0 ATK/DEF=%d/%d, quer 890/955",
			playerAttack(sub), playerDefense(sub))
	}
}

func TestCelestialCrystalBasesMatchNormalW2PPBranch(t *testing.T) {
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
			t.Errorf("cristais=%d DEF=%d, quer %d", want.crystals, got, want.defense)
		}
		hp, mp := celestialHPMPBonus(0, want.crystals)
		if hp != want.hp || mp != want.mp {
			t.Errorf("cristais=%d HP/MP bonus=%d/%d, quer %d/%d",
				want.crystals, hp, mp, want.hp, want.mp)
		}
	}
}

func TestArchGrowthAndInitialPointBudgetsMatchW2PP(t *testing.T) {
	w := &World{}
	for class := byte(0); class < 4; class++ {
		natural := baseClassStats[class]
		ch := &model.Char{
			Class: class, Evolution: archEvolution,
			Extended: testExtended(model.ExtendedScore{
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
			t.Errorf("classe=%d Arch HP/MP=%d/%d, quer %d/%d",
				class, playerMaxHP(ch), playerMaxMP(ch), wantHP, wantMP)
		}
		if ch.Extended.SkillPts != uint32(mortalSkillPointBudget(100)+168) {
			t.Errorf("classe=%d Arch skill points=%d", class, ch.Extended.SkillPts)
		}
		if ch.Extended.MasterPts != 312 {
			t.Errorf("classe=%d Arch mastery points=%d, quer 312", class, ch.Extended.MasterPts)
		}
	}
}

func TestCelestialProgressionLocksAndExperienceCurve(t *testing.T) {
	ch := celestialCharacter("celestial", 0)
	levels, applied := grantExp(ch, ^uint32(0))
	if levels != 39 || ch.Extended.Level != 39 ||
		ch.Exp != celestialNextLevel[39] || applied != celestialNextLevel[39] {
		t.Fatalf("trava 40 atravessada: levels=%d level=%d exp=%d applied=%d",
			levels, ch.Extended.Level, ch.Exp, applied)
	}
	if _, applied = grantExp(ch, 1); applied != 0 {
		t.Fatalf("Celestial recebeu EXP ainda travado no 40: %d", applied)
	}

	ch.CelestialLevel40Unlocked = true
	_, _ = grantExp(ch, ^uint32(0))
	if ch.Extended.Level != 89 || ch.Exp != celestialNextLevel[89] {
		t.Fatalf("trava 90 atravessada: level=%d exp=%d", ch.Extended.Level, ch.Exp)
	}
	ch.CelestialLevel90Unlocked = true
	_, _ = grantExp(ch, ^uint32(0))
	if ch.Extended.Level != maxCelestialLevel ||
		ch.Exp != celestialNextLevel[len(celestialNextLevel)-1] {
		t.Fatalf("cap Celestial incorreto: level=%d exp=%d", ch.Extended.Level, ch.Exp)
	}

	sub := celestialCharacter("subcelestial", 0)
	_, _ = grantExp(sub, ^uint32(0))
	if sub.Extended.Level != maxCelestialLevel {
		t.Fatalf("SubCelestial sofreu travas exclusivas do Celestial: %d", sub.Extended.Level)
	}
}

func TestCelestialCombatExperienceUsesW2PPDivisors(t *testing.T) {
	tests := []struct {
		level uint32
		want  uint32
	}{
		// level interno + 1 e o nivel exibido usado em GetExpApply.
		{0, 450}, {39, 450}, {40, 360}, {79, 360},
		{80, 327}, {99, 327}, {100, 300}, {149, 300},
		{150, 276}, {189, 276}, {190, 257}, {199, 257},
	}
	for _, test := range tests {
		ch := celestialCharacter("celestial", test.level)
		if got := combatExperienceByEvolution(ch, 18_000); got != test.want {
			t.Errorf("level=%d: EXP=%d, quer %d", test.level, got, test.want)
		}
	}

	sub := celestialCharacter("subcelestial", 100)
	if got := combatExperienceByEvolution(sub, 18_000); got != 300 {
		t.Fatalf("SubCelestial nao compartilhou a curva Celestial: %d", got)
	}
}

func TestArchCombatExperienceUsesW2PPDivisors(t *testing.T) {
	tests := []struct {
		level uint32
		want  uint32
	}{
		{0, 1_200_000}, // exibido 1: 100%
		{4, 1_188_118}, // exibido 5: divisor 101
		{254, 794_701}, // exibido 255: divisor 151
		{255, 600_000}, // exibido 256: divisor 2
		{280, 300_000}, // exibido 281: divisor 4
		{300, 150_000}, // exibido 301: divisor 8
		{320, 100_000}, // exibido 321: divisor 12
		{340, 75_000},  // exibido 341: divisor 16
		{350, 60_000},  // exibido 351: divisor 20
		{360, 50_000},  // exibido 361: divisor 24
		{370, 42_857},  // exibido 371: divisor 28
		{380, 15_789},  // exibido 381: divisor 76
		{390, 13_333},  // exibido 391: divisor 90
		{395, 12_000},  // exibido 396: divisor 100
		{398, 10_000},  // exibido 399: divisor 120
	}
	for _, test := range tests {
		ch := celestialCharacter("arch", test.level)
		if got := combatExperienceByEvolution(ch, 1_200_000); got != test.want {
			t.Errorf("level interno=%d: EXP=%d, quer %d", test.level, got, test.want)
		}
	}
}

func TestMortalCombatExperienceRemainsUnreduced(t *testing.T) {
	ch := celestialCharacter("", 398)
	if got := combatExperienceByEvolution(ch, 1_200_000); got != 1_200_000 {
		t.Fatalf("curva de evolucao avancada atingiu Mortal: %d", got)
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
		t.Fatal("mesmo milestone foi aplicado duas vezes")
	}
	ch.Extended.Level = 198
	if !updateCelestialCythera(ch) ||
		ch.Equip[1].Eff != [6]byte{43, 10, 3, 50, 4, 70} ||
		ch.CelestialCytheraTier != 4 {
		t.Fatalf("milestone final nao substituiu o anterior: %+v", ch.Equip[1])
	}

	plain := celestialCharacter("subcelestial", 120)
	plain.Equip[1] = model.Item{Index: 3500}
	if !updateCelestialCythera(plain) ||
		plain.Equip[1].Eff != [6]byte{3, 30, 4, 40} {
		t.Fatalf("Cythera sem refino ganhou efeito artificial: %+v", plain.Equip[1])
	}

	tinted := celestialCharacter("subcelestial", 120)
	tinted.Equip[1] = model.Item{Index: 3500, Eff: [6]byte{121, 234, 8, 99}}
	if !updateCelestialCythera(tinted) ||
		tinted.Equip[1].Eff != [6]byte{43, 234, 3, 30, 4, 40} {
		t.Fatalf("Cythera nao normalizou/refez os pares nativos: %+v", tinted.Equip[1])
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
	p.Char.ArchCrystals = 0 // nao e requisito no Secrets 7.54
	p.Char.Extended = testExtended(model.ExtendedScore{
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
		t.Fatal("conta de teste sem personagem para reentrada")
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
		ch.Extended.Level != 0 || ch.Exp != 0 ||
		ch.Extended.Str != uint32(baseClassStats[2][0]) ||
		ch.Extended.Int != uint32(baseClassStats[2][1]) ||
		ch.Extended.Dex != uint32(baseClassStats[2][2]) ||
		ch.Extended.Con != uint32(baseClassStats[2][3]) {
		t.Fatalf("criacao Celestial incompleta: saves=%d char=%+v",
			st.saves, ch)
	}
	if ch.CelestialArchTier != 5 || ch.Equip[1].Index != 3502 ||
		ch.Equip[model.CapeSlot].Index != 3197 ||
		ch.Equip[model.CapeSlot].Eff != [6]byte{} ||
		ch.Equip[0].Eff[3] != celestialFaceType ||
		ch.LearnedSkill != celestialSoulBit ||
		ch.Extended.MasterPts != 855 || ch.Inv[0].Index != 0 {
		t.Fatalf("recompensas Celestial incorretas: tier=%d cythera=%d cape=%d learn=%X mastery=%d item=%d",
			ch.CelestialArchTier, ch.Equip[1].Index,
			ch.Equip[model.CapeSlot].Index, ch.LearnedSkill,
			ch.Extended.MasterPts, ch.Inv[0].Index)
	}
	if p.InWorld || p.Char != nil || p.ID != 0 ||
		p.Session.QueuedPacketsForTest() < packetsBefore+2 {
		t.Fatal("criacao Celestial nao retornou a selecao com a lista atualizada")
	}
	if observer.Session.QueuedPacketsForTest() != observerPackets+1 {
		t.Fatal("criacao Celestial nao enviou exatamente um anuncio global ao observador")
	}
}

func TestFailedCelestialCreationDoesNotBroadcastOrLeaveWorld(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	observer := addZonePlayer(w, 9, 1000, 1000, 500)
	before := observer.Session.QueuedPacketsForTest()
	st.err = errors.New("postgres unavailable")

	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)

	if observer.Session.QueuedPacketsForTest() != before {
		t.Fatal("criacao Celestial nao persistida foi anunciada")
	}
	if !p.InWorld || p.Char == nil || p.Char.Evolution != archEvolution {
		t.Fatal("falha de persistencia retirou o Arch do mundo ou manteve mutacao")
	}
}

func TestSubCelestialHasSeparateProgressionAndSharedBody(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	reenterCelestialTestCharacter(t, w, p)
	p.Char.Extended.Level = 120
	p.Char.Exp = celestialNextLevel[120]
	p.Char.Extended.Str = 111
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
		p.Char.AlternateCelestial.Extended.Level != 0 {
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
		t.Fatal("criacao da Sub nao entregou 10 Pedras Misteriosas")
	}
	if got := counterBalance(p, fameCounter); got != 0 {
		t.Fatalf("criacao da Sub nao consumiu 100 Fame: %d", got)
	}
	sharedCythera := p.Char.Equip[1]
	w.useMysteriousStone(
		p.Session, p, &p.Char.Inv[mysterySlot], byte(mysterySlot))

	if p.Char.Evolution != "subcelestial" || p.Char.Class != 3 ||
		p.Char.Extended.Level != 0 || p.Char.Extended.Str != uint32(baseClassStats[3][0]) ||
		p.Char.SoulInfo != 7 || p.Char.Gold != 123456 ||
		p.Char.Equip[1] != sharedCythera ||
		itemStackAmount(p.Char.Inv[mysterySlot]) != 9 {
		t.Fatalf("troca nao preservou campos compartilhados: %+v", p.Char)
	}
	if p.Char.Affects[0].Type != 0 ||
		p.Char.AlternateCelestial == nil ||
		p.Char.AlternateCelestial.Evolution != "celestial" ||
		p.Char.AlternateCelestial.Extended.Level != 120 ||
		p.Char.AlternateCelestial.Extended.Str != 111 ||
		p.Char.AlternateCelestial.Affects[0].Type != 2 {
		t.Fatalf("estado por forma vazou ou se perdeu: alt=%+v affects=%+v",
			p.Char.AlternateCelestial, p.Char.Affects)
	}
}

func TestMysteriousStoneOnlyWorksInsideNativeCity(t *testing.T) {
	w, p, st := newCelestialWorld(t, 399)
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	reenterCelestialTestCharacter(t, w, p)
	p.Char.Extended.Level = 120
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
		t.Fatal("Pedra Misteriosa funcionou fora da cidade")
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
		p.Char.Extended.Level = 89
		p.SpecialCoins = map[string]uint32{fameCounter: 500}
		p.Char.Inv[0] = model.Item{Index: furyStoneItem}
		w.useFuryStone(p.Session, p, &p.Char.Inv[0], 0)
		if !p.Char.CelestialLevel90Unlocked || counterBalance(p, fameCounter) != 0 ||
			p.Char.Inv[0].Index != 0 || st.saves != 1 {
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
		p.Char.Extended.Level = maxCelestialLevel
		p.SpecialCoins = map[string]uint32{fameCounter: 500}
		p.Char.Inv[0] = model.Item{Index: furyStoneItem}
		for i, stone := 1, uint16(5334); stone <= 5337; i, stone = i+1, stone+1 {
			p.Char.Inv[i] = model.Item{Index: stone}
		}
		p.Char.Equip[1] = model.Item{Index: 3502}
		if !setItemSanc(&p.Char.Equip[1], 11) {
			t.Fatal("cenario nao conseguiu refinar Cythera +11")
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
				t.Fatalf("Pedra Secreta inv[%d] nao foi consumida", i)
			}
		}
	})

	t.Run("rollback", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Rollback", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		st := &craftStore{err: errors.New("postgres indisponivel")}
		w.store = st
		p.Char.Evolution = "celestial"
		p.Char.Extended.Level = 89
		p.SpecialCoins = map[string]uint32{fameCounter: 500}
		p.Char.Inv[0] = model.Item{Index: furyStoneItem}
		w.useFuryStone(p.Session, p, &p.Char.Inv[0], 0)
		if p.Char.CelestialLevel90Unlocked || p.Char.Inv[0].Index != furyStoneItem ||
			counterBalance(p, fameCounter) != 500 {
			t.Fatal("falha de save nao restaurou Pedra da Furia/fama/flag")
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
		t.Fatal("rei nao reconheceu a receita da Pedra Ideal")
	}
	if p.Char.Equip[10].Index != 0 || p.Char.Equip[sefirotSlot].Index != 0 {
		t.Fatal("rei nao consumiu Imortalidade/Sephirot")
	}
	found := false
	for _, item := range p.Char.Inv {
		found = found || item.Index == idealStoneItem
	}
	if !found {
		t.Fatal("rei nao entregou a Pedra Ideal")
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
		t.Fatalf("Ehre nao criou pilha 10: saves=%d item=%+v", st.saves, p.Char.Inv[2])
	}
}

func TestCelestialSoulUsesSharedConfiguration(t *testing.T) {
	w := &World{}
	ch := celestialCharacter("celestial", 100)
	ch.SoulInfo = 5 // +80% STR, +40% CON
	ch.Extended.Str, ch.Extended.Con = 1000, 500
	ch.Affects[0] = model.Affect{
		Type: 29, Value: 102, ExpiresAt: time.Now().Add(time.Hour),
	}
	applyExtendedScore(ch)
	w.applyExtendedAffectStats(ch)
	e := effectiveExtended(ch)
	if e.Str != 1800 || e.Con != 700 ||
		e.Attack != ch.Extended.Attack+800/3 ||
		e.MaxHP != ch.Extended.MaxHP+400 {
		t.Fatalf("Soul Celestial incorreta: STR=%d CON=%d ATK=%d HP=%d",
			e.Str, e.Con, e.Attack, e.MaxHP)
	}
	if !specialSkillLearned(&model.Char{LearnedSkill: celestialSoulBit}, 102) {
		t.Fatal("bit 30 nao liberou a skill 102 Limite da Alma")
	}
}
