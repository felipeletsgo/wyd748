package game

import (
	"bytes"
	"errors"
	"math/rand"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// itemBonusSequenceRNG returns scripted rolls and records each bound.
type itemBonusSequenceRNG struct {
	t      *testing.T
	values []int
	bounds []int
}

func TestDropItemBonusGradeProbabilitiesMatchNativeRolls(t *testing.T) {
	// Literal counts among rand()%100 outcomes from FUN_005109B0.
	first := []map[int]int{
		{0: 45, 1: 31, 2: 18, 3: 4, 4: 2},
		{1: 35, 2: 41, 3: 20, 4: 4}, {2: 40, 3: 46, 4: 14},
		{3: 10, 4: 40, 5: 41, 6: 9}, {5: 75, 6: 25}, {6: 100},
	}
	second := []map[int]int{
		{0: 45, 1: 31, 2: 18, 3: 6},
		{1: 35, 2: 41, 3: 19, 4: 4, 5: 1}, {2: 40, 3: 44, 4: 14, 5: 2},
		{4: 55, 5: 36, 6: 9}, {4: 45, 5: 50, 6: 5}, {4: 35, 5: 65},
	}
	for level := range first {
		gotFirst, gotSecond := map[int]int{}, map[int]int{}
		for roll := 0; roll < 100; roll++ {
			gotFirst[itemBonusFirstGrade(level, roll)]++
			gotSecond[itemBonusSecondGrade(level, 2, roll)]++
			for previous := 0; previous < 2; previous++ {
				if level == 0 && itemBonusSecondGrade(level, previous, roll) != 0 {
					t.Fatal("native grade-zero coupling to the first roll was lost")
				}
			}
		}
		for grade := 0; grade <= 6; grade++ {
			if gotFirst[grade] != first[level][grade] || gotSecond[grade] != second[level][grade] {
				t.Fatalf("level %d grade %d: first=%d second=%d", level, grade, gotFirst[grade], gotSecond[grade])
			}
		}
	}
}

func TestDropItemBonusSlotZeroProbabilitiesMatchNativeRolls(t *testing.T) {
	for level := 0; level <= 4; level++ {
		counts := map[int]int{}
		for roll := 0; roll < 100; roll++ {
			rng := &itemBonusSequenceRNG{t: t, values: []int{0, 0, 2, 0, roll, 0, 0}}
			w := itemBonusWorld(rng, model.ItemDef{Index: 820, Pos: 64})
			item := w.applyDropItemBonus(model.Item{Index: 820, Eff: [6]byte{itemBonusGrade0 + byte(level)}}, 1, 0)
			kind := int(item.Eff[0])
			if item.Eff[0] == itemBonusSanc {
				kind = int(item.Eff[1])
			}
			counts[kind]++
		}
		want := map[int]int{2: 6, 1: 16, 0: 53, int(itemBonusHP): 15, int(itemBonusUnique): 10}
		if level >= 2 {
			want = map[int]int{2: 6, 1: 29, 0: 50, int(itemBonusHP): 15}
		}
		for _, kind := range []int{0, 1, 2, int(itemBonusHP), int(itemBonusUnique)} {
			if counts[kind] != want[kind] {
				t.Fatalf("level %d slot-zero outcome %d: got %d want %d", level, kind, counts[kind], want[kind])
			}
		}
	}
}

func TestDropItemBonusSurvivesKillCommitAndRollsBackOnFailure(t *testing.T) {
	for _, fail := range []bool{false, true} {
		var saveError error
		if fail {
			saveError = errors.New("injected option save failure")
		}
		w, p, mob, st := killRewardTestWorld(t, saveError)
		def := w.items[500]
		def.Pos = 64
		w.items[500] = def
		mob.Def.Carry[11].Eff = [6]byte{itemBonusGrade0 + 3, 0}
		before := p.Char.Inv
		w.killMobState(p, mob, 100, 100)
		if st.saves != 1 || st.queuedAtSave != 0 {
			t.Fatal("rolled drop was published before the kill save")
		}
		if mob.Def.Carry[11].Eff != [6]byte{itemBonusGrade0 + 3, 0} {
			t.Fatal("rolled drop mutated the reusable mob template")
		}
		if fail {
			if p.Char.Inv != before || mob.Dead {
				t.Fatal("failed save retained the rolled item or death")
			}
			continue
		}
		if p.Char.Inv[0].Eff != [6]byte{itemBonusSanc, 2, itemBonusAttSpeed, 21, itemBonusAttSpeed, 21} {
			t.Fatalf("committed item lost native options: %+v", p.Char.Inv[0])
		}
		found := false
		for packet, ok := p.Session.DequeuePacketForTest(); ok; packet, ok = p.Session.DequeuePacketForTest() {
			if !wire.Decrypt(packet) {
				t.Fatal("invalid kill publication")
			}
			if wire.ParseHeader(packet).Type == wire.OpSendItem {
				want := wire.SendItem(p.ID, placeInv, 0, p.Char.Inv[0])
				if bytes.Equal(packet[12:], want[12:]) {
					found = true
				}
			}
		}
		if !found {
			t.Fatal("committed options were not sent to the owner")
		}
	}
}

func TestDropItemBonusRetainsOptionsForFullInventoryGroundFallback(t *testing.T) {
	w, p, mob, _ := killRewardTestWorld(t, nil)
	def := w.items[500]
	def.Pos = 64
	w.items[500] = def
	for i := range p.Char.Inv {
		p.Char.Inv[i] = model.Item{Index: 1}
	}
	before := p.Char.Inv
	mob.Def.Carry[11].Eff = [6]byte{itemBonusGrade0 + 3, 0}
	drops := w.planMobDrops(p, mob)
	if len(drops) != 1 || drops[0].inventoryPos != -1 ||
		drops[0].item.Eff != [6]byte{itemBonusSanc, 2, itemBonusAttSpeed, 21, itemBonusAttSpeed, 21} || p.Char.Inv != before {
		t.Fatal("full inventory lost rolled options or changed its occupied items")
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("planning published a ground drop before commit")
	}
}

func (r *itemBonusSequenceRNG) Intn(n int) int {
	r.t.Helper()
	if len(r.bounds) >= len(r.values) {
		r.t.Fatalf("unexpected roll %d with bound %d", len(r.bounds)+1, n)
	}
	value := r.values[len(r.bounds)]
	r.bounds = append(r.bounds, n)
	if value < 0 || value >= n {
		r.t.Fatalf("scripted roll %d is outside [0,%d)", value, n)
	}
	return value
}

func itemBonusWorld(rng RNG, defs ...model.ItemDef) *World {
	w := &World{rng: rng, items: make(map[uint16]model.ItemDef, len(defs))}
	for _, def := range defs {
		w.items[def.Index] = def
	}
	return w
}

func TestDropItemBonusRollsNativeWeaponOptions(t *testing.T) {
	rng := &itemBonusSequenceRNG{t: t, values: []int{0, 1, 0, 0, 90, 1, 3}}
	w := itemBonusWorld(rng, model.ItemDef{Index: 820, Pos: 64})

	got := w.applyDropItemBonus(model.Item{Index: 820, Eff: [6]byte{itemBonusGrade0 + 2, 0}}, 50, 0)

	// Grade C: level difference 2. Options: ATTSPEED 3*(4+1), DAMAGE 9*(5-1),
	// slot 0 from the level-2 range table: RESISTALL 5+3.
	want := [6]byte{itemBonusResistAll, 8, itemBonusAttSpeed, 15, itemBonusDamage, 36}
	if got.Eff != want {
		t.Fatalf("eff=%v, want %v", got.Eff, want)
	}
	wantBounds := []int{101, 100, 100, 100, 100, 10, 6}
	for i, n := range wantBounds {
		if rng.bounds[i] != n {
			t.Fatalf("roll bounds=%v, want %v", rng.bounds, wantBounds)
		}
	}
}

func TestDropItemBonusUsesKillerDropBonus(t *testing.T) {
	gloves := model.ItemDef{Index: 1105, Pos: 16, ReqLevel: 10}
	carry := model.Item{Index: 1105, Eff: [6]byte{itemBonusGrade0, 0}}

	withBonus := itemBonusWorld(&itemBonusSequenceRNG{t: t, values: []int{0, 2, 60, 7, 90, 10}}, gloves)
	got := withBonus.applyDropItemBonus(carry, 10, 16)
	// A zero second grade becomes DropBonus/8 = 2: SPECIALALL 3*2.
	if want := [6]byte{itemBonusSanc, 1, itemBonusUnique, 7, itemBonusSpecialAll, 6}; got.Eff != want {
		t.Fatalf("with bonus eff=%v, want %v", got.Eff, want)
	}

	// Without bonus the same rolls leave a zero grade, so EF_UNIQUE is written.
	// The first choice also changes from 0%6 to 0%8; gloves ignore it.
	noBonus := itemBonusWorld(&itemBonusSequenceRNG{t: t, values: []int{0, 2, 60, 7, 90, 11, 10}}, gloves)
	got = noBonus.applyDropItemBonus(carry, 10, 0)
	if want := [6]byte{itemBonusSanc, 1, itemBonusUnique, 7, itemBonusUnique, 11}; got.Eff != want {
		t.Fatalf("without bonus eff=%v, want %v", got.Eff, want)
	}
}

func TestDropItemBonusSancUpgradeUsesCarryGradeValue(t *testing.T) {
	rng := &itemBonusSequenceRNG{t: t, values: []int{0, 0, 50, 50, 0, 3}}
	w := itemBonusWorld(rng, model.ItemDef{Index: 1300, Pos: 32})

	got := w.applyDropItemBonus(model.Item{Index: 1300, Eff: [6]byte{itemBonusGrade0 + 3, 7}}, 1, 0)

	// Grade D (level difference 3): both grades 4, adjusted to 3.
	// Grade value 7 upgrades the +2 refinement to +7.
	want := [6]byte{itemBonusSanc, 7, itemBonusDamage2, 6 * 3, itemBonusMagic, 2 * 3}
	if got.Eff != want {
		t.Fatalf("eff=%v, want %v", got.Eff, want)
	}
}

func TestDropItemBonusSkipsNonQualifyingItems(t *testing.T) {
	rng := &itemBonusSequenceRNG{t: t}
	w := itemBonusWorld(rng,
		model.ItemDef{Index: 551, Pos: 0},
		model.ItemDef{Index: 1500, Pos: 128},
		model.ItemDef{Index: 821, Pos: 64},
		model.ItemDef{Index: 412, Pos: 0, StaticEffects: []model.StaticEffect{{Name: "EF_AMOUNT", Value: 3}}},
	)
	for _, tc := range []struct {
		name string
		in   model.Item
		want [6]byte
	}{
		{"accessory consumes grade", model.Item{Index: 551, Eff: [6]byte{itemBonusGrade0 + 4, 5}}, [6]byte{}},
		{"shield position", model.Item{Index: 1500, Eff: [6]byte{itemBonusGrade0 + 1, 0}}, [6]byte{}},
		{"preset slot zero", model.Item{Index: 821, Eff: [6]byte{itemBonusSanc, 3}}, [6]byte{itemBonusSanc, 3}},
		{"static amount", model.Item{Index: 412}, [6]byte{itemBonusAmount, 3}},
		{"unknown item", model.Item{Index: 999, Eff: [6]byte{itemBonusGrade0, 0}}, [6]byte{itemBonusGrade0, 0}},
		{"outside native index range", model.Item{Index: itemBonusMaxIndex, Eff: [6]byte{itemBonusGrade0, 0}}, [6]byte{itemBonusGrade0, 0}},
	} {
		if got := w.applyDropItemBonus(tc.in, 80, 0); got.Eff != tc.want {
			t.Fatalf("%s: eff=%v, want %v", tc.name, got.Eff, tc.want)
		}
	}
	if len(rng.bounds) != 0 {
		t.Fatalf("non-qualifying items consumed rolls %v", rng.bounds)
	}
}

func TestDropItemBonusValuesStayInNativeRanges(t *testing.T) {
	defs := []model.ItemDef{
		{Index: 1001, Pos: 2}, {Index: 1002, Pos: 4}, {Index: 1003, Pos: 8},
		{Index: 1004, Pos: 16}, {Index: 1005, Pos: 32},
		{Index: 801, Pos: 64}, {Index: 802, Pos: 192}, {Index: 803, Pos: 64, Unique: 44},
	}
	w := itemBonusWorld(rand.New(rand.NewSource(7)), defs...)
	slot0Range := map[byte]int{}
	for i, effect := range itemBonusTypes {
		slot0Range[effect] = i
	}
	options := map[byte]bool{
		itemBonusDamage: true, itemBonusAC: true, itemBonusHP: true, itemBonusAttSpeed: true,
		itemBonusResistAll: true, itemBonusMagic: true, itemBonusCritical2: true,
		itemBonusACAdd2: true, itemBonusDamage2: true, itemBonusSpecialAll: true,
	}
	rolledOption := false
	for grade := byte(0); grade <= 5; grade++ {
		for _, def := range defs {
			for n := 0; n < 400; n++ {
				got := w.applyDropItemBonus(model.Item{Index: def.Index, Eff: [6]byte{itemBonusGrade0 + grade, 0}}, 1, 0)
				switch effect, value := got.Eff[0], int(got.Eff[1]); {
				case effect == 0:
					if grade != 5 {
						t.Fatalf("grade %d: empty slot 0", grade)
					}
				case effect == itemBonusSanc:
					if value > 2 {
						t.Fatalf("grade %d: sanc +%d without a grade value", grade, value)
					}
				case effect == itemBonusUnique:
					if grade >= 2 || value >= 128 {
						t.Fatalf("grade %d: unexpected slot 0 %v", grade, got.Eff)
					}
				default:
					pick, ok := slot0Range[effect]
					if !ok || grade == 5 {
						t.Fatalf("grade %d: invalid slot 0 %v", grade, got.Eff)
					}
					bounds := itemBonusRanges[grade][pick]
					if value < bounds[0] || value > bounds[1] {
						t.Fatalf("grade %d: slot 0 %v outside %v", grade, got.Eff, bounds)
					}
				}
				for slot := 2; slot < 6; slot += 2 {
					effect, value := got.Eff[slot], int(got.Eff[slot+1])
					if effect == itemBonusUnique {
						if value >= 128 {
							t.Fatalf("grade %d: unique value %d", grade, value)
						}
						continue
					}
					if !options[effect] || value > 10*7 {
						t.Fatalf("grade %d pos %d: invalid option %v", grade, def.Pos, got.Eff)
					}
					rolledOption = true
				}
			}
		}
	}
	if !rolledOption {
		t.Fatal("no option was rolled")
	}
}

func TestPlanMobDropsAppliesItemBonusDeterministically(t *testing.T) {
	carry := make([]model.Item, 12)
	carry[11] = model.Item{Index: 820, Eff: [6]byte{itemBonusGrade0 + 3, 4}}
	def := &model.NPCDef{Name: "Bonus", Tipo: model.TipoMonstro, Carry: carry,
		Score: &model.Score{Version: model.ScoreVersion, Level: 120}}

	roll := func() model.Item {
		p, _ := networkedTestPlayer(1, "Bonus", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		w.rng = rand.New(rand.NewSource(99))
		w.items = map[uint16]model.ItemDef{820: {Index: 820, Pos: 64}}
		drops := w.planMobDrops(p, &Mob{ID: 1000, X: 2100, Y: 2100, Def: def})
		if len(drops) != 1 || drops[0].inventoryPos != 0 {
			t.Fatalf("drops=%+v", drops)
		}
		return p.Char.Inv[0]
	}
	first, second := roll(), roll()
	if first.Eff != second.Eff {
		t.Fatalf("same RNG seed produced %v and %v", first.Eff, second.Eff)
	}
	if first.Eff[0] >= itemBonusGrade0 || first.Eff[2] == 0 || first.Eff[4] == 0 {
		t.Fatalf("drop kept the carry grade or missed options: %v", first.Eff)
	}
	if def.Carry[11].Eff != [6]byte{itemBonusGrade0 + 3, 4} {
		t.Fatalf("drop mutated the mob template: %v", def.Carry[11].Eff)
	}
}

// itemBonusListRNG replays a fixed roll list.
type itemBonusListRNG struct {
	values []int
	next   int
}

func (r *itemBonusListRNG) Intn(n int) int {
	value := r.values[r.next]
	r.next++
	return value % n
}

// TestDropItemBonusCombinationsMatchNativeTables enumerates every option
// choice roll. The expected sets are the literal FUN_005109B0 switches.
func TestDropItemBonusCombinationsMatchNativeTables(t *testing.T) {
	const none = itemBonusUnique
	type sets struct{ first, second []byte }
	// Expected effects by level-difference group: 0..2, 3, and 4..5.
	expected := map[string][3]sets{
		"helmet": {
			{[]byte{itemBonusAttSpeed, itemBonusMagic, none}, []byte{itemBonusHP, itemBonusAC, none}},
			{[]byte{itemBonusAttSpeed, itemBonusMagic, none}, []byte{itemBonusHP, itemBonusAC, none}},
			{[]byte{itemBonusAttSpeed, itemBonusMagic}, []byte{itemBonusHP, itemBonusAC, none}},
		},
		"armor": {
			{[]byte{itemBonusCritical2}, []byte{itemBonusMagic, itemBonusDamage, itemBonusAC, none}},
			{[]byte{itemBonusCritical2}, []byte{itemBonusMagic, itemBonusDamage, itemBonusAC, none}},
			{[]byte{itemBonusCritical2}, []byte{itemBonusMagic, itemBonusDamage, itemBonusAC}},
		},
		"gloves": {
			{[]byte{itemBonusACAdd2}, []byte{itemBonusMagic, itemBonusDamage, itemBonusSpecialAll, itemBonusResistAll, none}},
			{[]byte{itemBonusACAdd2}, []byte{itemBonusMagic, itemBonusDamage, itemBonusSpecialAll, itemBonusResistAll}},
			{[]byte{itemBonusACAdd2}, []byte{itemBonusMagic, itemBonusDamage, itemBonusSpecialAll}},
		},
		"boots": {
			{[]byte{itemBonusDamage2}, []byte{itemBonusMagic, itemBonusSpecialAll, none}},
			{[]byte{itemBonusDamage2}, []byte{itemBonusMagic, itemBonusSpecialAll, none}},
			{[]byte{itemBonusDamage2}, []byte{itemBonusMagic, itemBonusSpecialAll, none}},
		},
		"weapon": {
			{[]byte{itemBonusAttSpeed, itemBonusDamage, itemBonusSpecialAll, none}, []byte{itemBonusAttSpeed, itemBonusDamage, itemBonusSpecialAll, none}},
			{[]byte{itemBonusAttSpeed, itemBonusDamage, itemBonusSpecialAll, none}, []byte{itemBonusAttSpeed, itemBonusDamage, itemBonusSpecialAll, none}},
			{[]byte{itemBonusAttSpeed, itemBonusDamage}, []byte{itemBonusAttSpeed, itemBonusDamage, itemBonusSpecialAll}},
		},
		"magic weapon": {
			{[]byte{itemBonusMagic, itemBonusSpecialAll, none}, []byte{itemBonusMagic, itemBonusSpecialAll, none}},
			{[]byte{itemBonusMagic, itemBonusSpecialAll, none}, []byte{itemBonusMagic, itemBonusSpecialAll, none}},
			{[]byte{itemBonusMagic, itemBonusSpecialAll}, []byte{itemBonusMagic, itemBonusSpecialAll, none}},
		},
	}
	defs := map[string][]model.ItemDef{
		"helmet":       {{Index: 1001, Pos: 2}},
		"armor":        {{Index: 1002, Pos: 4}, {Index: 1003, Pos: 8}},
		"gloves":       {{Index: 1004, Pos: 16}},
		"boots":        {{Index: 1005, Pos: 32}},
		"weapon":       {{Index: 801, Pos: 64}, {Index: 802, Pos: 192}},
		"magic weapon": {{Index: 803, Pos: 64, Unique: 44}, {Index: 804, Pos: 192, Unique: 47}},
	}
	for name, group := range defs {
		for _, def := range group {
			for grade := 0; grade <= 5; grade++ {
				band := 0
				switch {
				case grade >= 4:
					band = 2
				case grade == 3:
					band = 1
				}
				want := map[[2]byte]bool{}
				for _, a := range expected[name][band].first {
					for _, b := range expected[name][band].second {
						want[[2]byte{a, b}] = true
					}
				}
				got := map[[2]byte]bool{}
				for choice := 0; choice < 101; choice++ {
					for second := 0; second < 100; second++ {
						// Grade rolls 2 and 0 give positive values at every level;
						// slot-0 roll 6 writes a refinement without more rolls.
						rng := &itemBonusListRNG{values: []int{choice, second, 2, 0, 6}}
						w := itemBonusWorld(rng, def)
						item := w.applyDropItemBonus(model.Item{Index: def.Index, Eff: [6]byte{itemBonusGrade0 + byte(grade)}}, 1, 0)
						got[[2]byte{item.Eff[2], item.Eff[4]}] = true
					}
				}
				if len(got) != len(want) {
					t.Fatalf("%s pos %d grade %d: got %d pairs %v, want %d %v", name, def.Pos, grade, len(got), got, len(want), want)
				}
				for pair := range want {
					if !got[pair] {
						t.Fatalf("%s pos %d grade %d: missing native pair %v; got %v", name, def.Pos, grade, pair, got)
					}
				}
			}
		}
	}
}
