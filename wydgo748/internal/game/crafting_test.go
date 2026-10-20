package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func TestCombineResultMessages(t *testing.T) {
	tests := map[uint32]string{
		0: "Wrong combination.",
		1: "Processing complete.",
		2: "The combination failed.",
		3: "",
	}
	for result, want := range tests {
		if got := combineResultMessage(result); got != want {
			t.Fatalf("result %d: message %q, expected %q", result, got, want)
		}
	}
}

func TestCombineNPCResolvesClientSideOpenedArtisan(t *testing.T) {
	w := &World{}
	artisan := &Mob{
		ID: 1500, X: 100, Y: 100,
		Def: &model.NPCDef{
			Name:  "Lindy",
			Tipo:  model.TipoNPC,
			Score: &model.Score{Merchant: craftingMerchant},
		},
	}
	w.registerMobSpatial(artisan)
	p := &Player{
		Char: &model.Char{}, X: 102, Y: 100,
		Visible: map[uint16]struct{}{artisan.ID: {}},
	}

	got, err := w.combineNPC(p, "Lindy")
	if err != nil {
		t.Fatalf("combineNPC rejected a locally opened artisan: %v", err)
	}
	if got != artisan || p.CraftNPC != artisan.ID {
		t.Fatalf("contexto incorreto: mob=%v CraftNPC=%d", got, p.CraftNPC)
	}
}

func TestCombineNPCReplacesStaleArtisanContext(t *testing.T) {
	w := &World{}
	old := &Mob{ID: 1500, X: 100, Y: 100, Def: &model.NPCDef{
		Name: "Tiny", Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: craftingMerchant},
	}}
	wanted := &Mob{ID: 1501, X: 101, Y: 100, Def: &model.NPCDef{
		Name: "Aylin", Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: craftingMerchant},
	}}
	w.registerMobSpatial(old)
	w.registerMobSpatial(wanted)
	p := &Player{
		Char: &model.Char{}, X: 100, Y: 100, CraftNPC: old.ID,
		Visible: map[uint16]struct{}{old.ID: {}, wanted.ID: {}},
	}

	got, err := w.combineNPC(p, "Aylin")
	if err != nil || got != wanted || p.CraftNPC != wanted.ID {
		t.Fatalf("did not replace stale artisan context: mob=%v CraftNPC=%d err=%v", got, p.CraftNPC, err)
	}
}

// craftStore is a minimal store.Store with injectable errors for testing the
// crafting handlers' persist-before-confirm and rollback paths.
type craftStore struct {
	saves int
	err   error
}

func (s *craftStore) LoadAccount(string) (*model.Account, error) {
	return nil, errors.New("unused")
}
func (s *craftStore) SaveAccount(acc *model.Account) error {
	s.saves++
	return s.err
}
func (s *craftStore) CharacterNameExists(string) (bool, error) { return false, nil }

// Implement the optional instance transaction for tests that exercise the
// same atomic account+room boundary as PostgreSQL. The fixture only counts the
// commit; it does not need a durable instance payload.
func (s *craftStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}, nil
}
func (s *craftStore) SaveInstanceState(*model.InstanceStateSnapshot) error { return s.err }
func (s *craftStore) SaveGameStateWithInstanceState(*model.GuildRegistry,
	*model.InstanceStateSnapshot, ...*model.Account) error {
	s.saves++
	return s.err
}

// newCraftWorld sets up a minimal world with a visible artisan in range and
// authoritative item definitions, ready to exercise an onCombine* handler.
func newCraftWorld(t *testing.T, npcName string, items map[uint16]model.ItemDef, gold uint32) (*World, *Player, *net.Session, *craftStore) {
	t.Helper()
	st := &craftStore{}
	w := &World{store: st, items: items}
	mob := &Mob{
		ID: 2000,
		Def: &model.NPCDef{
			Name:  npcName,
			Score: &model.Score{Merchant: craftingMerchant},
		},
		X: 100, Y: 100,
	}
	w.registerMobSpatial(mob)
	session := net.NewTestSession(1, 32)
	acc := &model.Account{Name: "felipe", Chars: []model.Char{{Name: "felipe", Gold: gold}}}
	p := &Player{
		ID: 1, Session: session, Account: acc, Char: &acc.Chars[0], InWorld: true,
		X: 100, Y: 100, CraftNPC: mob.ID, Visible: map[uint16]struct{}{mob.ID: {}},
	}
	w.players = map[*net.Session]*Player{session: p}
	return w, p, session, st
}

// buildCombinePacket builds the native 84-byte MSG_CombineItem: eight
// STRUCT_ITEM entries starting at offset 12 and eight positions at offset 76.
func buildCombinePacket(items [combineSlots]model.Item, pos [combineSlots]int8) []byte {
	pkt := make([]byte, combinePacketSize)
	for i, item := range items {
		binary.LittleEndian.PutUint16(pkt[12+i*8:], item.Index)
		copy(pkt[14+i*8:20+i*8], item.Eff[:])
		pkt[76+i] = byte(pos[i])
	}
	return pkt
}

func TestCombineLindyCreatesEliteCape(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Lindy",
		map[uint16]model.ItemDef{3193: {Index: 3193}}, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0], items[1] = model.Item{Index: 3448}, model.Item{Index: 3448}
	setItemAmount(&items[0], 10)
	setItemAmount(&items[1], 10)
	items[2] = model.Item{Index: 4127}
	for index := 3; index <= 6; index++ {
		items[index] = model.Item{Index: 413}
	}
	for index := range pos {
		pos[index] = int8(index)
	}
	placeItems(p.Char, items, pos)
	p.Char.Score = testScore(model.Score{
		Level: archLockLevel355, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100,
	})
	p.Char.Evolution = archEvolution
	// The level 355 unlock neither requires nor consumes Fame.
	p.SpecialCoins = map[string]uint32{}
	originalCape, err := materializeItem(model.Item{Index: 3193})
	if err != nil {
		t.Fatal(err)
	}
	p.Char.Equip[15] = originalCape

	w.onCombineLindy(session, buildCombinePacket(items, pos))
	if p.Char.Equip[15].Index != 3193 || p.Char.Equip[15].UID != originalCape.UID ||
		p.Char.Equip[15].Eff[0] != 54 || p.Char.Equip[15].Eff[1] != 16 ||
		st.saves != 1 || counterBalance(p, fameCounter) != 0 {
		t.Fatalf("Lindy: cape=%+v saves=%d", p.Char.Equip[15], st.saves)
	}
	for index := 0; index <= 6; index++ {
		if p.Char.Inv[index].Index != 0 {
			t.Fatalf("Lindy did not consume ingredient in slot %d: %+v",
				index, p.Char.Inv[index])
		}
	}
}

func TestLindyRequiresSevenFiveFourLaktoreriumPackage(t *testing.T) {
	for _, index := range []uint16{3448} {
		item := model.Item{Index: index}
		setItemAmount(&item, 10)
		if !lindyMaterialStack(item) {
			t.Fatalf("material %d with 10 units was rejected", index)
		}
	}
	for _, index := range []uint16{413, 3448} {
		item := model.Item{Index: index}
		setItemAmount(&item, 9)
		if lindyMaterialStack(item) {
			t.Fatalf("incomplete material %d was accepted", index)
		}
	}
	if item := (model.Item{Index: 413, Eff: [6]byte{effectAmount, 10}}); lindyMaterialStack(item) {
		t.Fatal("dust 413 was accepted as a V754 pack")
	}
}

func TestCombineLindyRejectsCharacterOutsideArchLock(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Lindy", nil, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0], items[1] = model.Item{Index: 3448}, model.Item{Index: 3448}
	setItemAmount(&items[0], 10)
	setItemAmount(&items[1], 10)
	items[2] = model.Item{Index: 4127}
	for index := 3; index <= 6; index++ {
		items[index] = model.Item{Index: 413}
	}
	for index := range pos {
		pos[index] = int8(index)
	}
	placeItems(p.Char, items, pos)
	p.Char.Score = testScore(model.Score{Level: 360, MaxHP: 100, CurHP: 100})
	p.Char.Evolution = archEvolution
	before := p.Char.Inv

	w.onCombineLindy(session, buildCombinePacket(items, pos))

	if st.saves != 0 || p.Char.Inv != before {
		t.Fatalf("Lindy accepted Arch outside the locks: saves=%d inventoryChanged=%t",
			st.saves, p.Char.Inv != before)
	}
}

func TestCombineLindyLevel370KeepsEquippedCape(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Lindy", nil, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0], items[1] = model.Item{Index: 3448}, model.Item{Index: 3448}
	setItemAmount(&items[0], 10)
	setItemAmount(&items[1], 10)
	items[2] = model.Item{Index: 4127}
	for index := 3; index <= 6; index++ {
		items[index] = model.Item{Index: 413}
	}
	for index := range pos {
		pos[index] = int8(index)
	}
	placeItems(p.Char, items, pos)
	p.Char.Score = testScore(model.Score{Level: archLockLevel370, MaxHP: 100, CurHP: 100})
	p.Char.Evolution = archEvolution
	p.Char.ArchLevel355 = true
	p.Char.Equip[15] = model.Item{Index: 3197, UID: "11111111111141118111111111111111", Eff: [6]byte{43, 7}}
	p.SpecialCoins = map[string]uint32{fameCounter: 1}
	beforeCape := p.Char.Equip[15]

	w.onCombineLindy(session, buildCombinePacket(items, pos))

	if st.saves != 1 || !p.Char.ArchLevel370 || counterBalance(p, fameCounter) != 0 {
		t.Fatalf("incomplete level 370 unlock: saves=%d flag=%t fame=%d",
			st.saves, p.Char.ArchLevel370, counterBalance(p, fameCounter))
	}
	if p.Char.Equip[15] != beforeCape {
		t.Fatalf("Lindy substituiu a capa no destrave 370: got=%+v want=%+v",
			p.Char.Equip[15], beforeCape)
	}
}

func TestCombineLindyLevel370RequiresOneFameWithoutConsumingRecipe(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Lindy", nil, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0], items[1] = model.Item{Index: 3448}, model.Item{Index: 3448}
	setItemAmount(&items[0], 10)
	setItemAmount(&items[1], 10)
	items[2] = model.Item{Index: 4127}
	for index := 3; index <= 6; index++ {
		items[index] = model.Item{Index: 413}
	}
	for index := range pos {
		pos[index] = int8(index)
	}
	placeItems(p.Char, items, pos)
	p.Char.Score = testScore(model.Score{Level: archLockLevel370, MaxHP: 100, CurHP: 100})
	p.Char.Evolution = archEvolution
	p.Char.ArchLevel355 = true
	before := p.Char.Inv

	w.onCombineLindy(session, buildCombinePacket(items, pos))

	if st.saves != 0 || p.Char.ArchLevel370 || p.Char.Inv != before {
		t.Fatalf("level 370 without Fame changed state: saves=%d flag=%t inventoryChanged=%t",
			st.saves, p.Char.ArchLevel370, p.Char.Inv != before)
	}
}

func TestCombineLindyUnlocksRollBackFlagsFameAndRecipeOnSaveFailure(t *testing.T) {
	for _, test := range []struct {
		name  string
		level uint32
		fame  uint32
	}{
		{name: "level 355", level: archLockLevel355, fame: 7},
		{name: "level 370", level: archLockLevel370, fame: 2},
	} {
		t.Run(test.name, func(t *testing.T) {
			defs := map[uint16]model.ItemDef{}
			if test.level == archLockLevel355 {
				// The resulting cape depends on the realm inferred from the current cape.
				// Include all three variants to reach persistence regardless of that choice.
				defs[3191] = model.ItemDef{Index: 3191}
				defs[3192] = model.ItemDef{Index: 3192}
				defs[3193] = model.ItemDef{Index: 3193}
			}
			w, p, session, st := newCraftWorld(t, "Lindy", defs, 0)
			var items [combineSlots]model.Item
			var pos [combineSlots]int8
			items[0], items[1] = model.Item{Index: 3448}, model.Item{Index: 3448}
			setItemAmount(&items[0], 10)
			setItemAmount(&items[1], 10)
			items[2] = model.Item{Index: 4127}
			for index := 3; index <= 6; index++ {
				items[index] = model.Item{Index: 413}
			}
			for index := range pos {
				pos[index] = int8(index)
			}
			placeItems(p.Char, items, pos)
			p.Char.Score = testScore(model.Score{Level: test.level, MaxHP: 100, CurHP: 100})
			p.Char.Evolution = archEvolution
			p.Char.ArchLevel355 = test.level == archLockLevel370
			p.Char.Equip[15] = model.Item{Index: 3197, UID: "11111111111141118111111111111111", Eff: [6]byte{43, 7}}
			p.SpecialCoins = map[string]uint32{fameCounter: test.fame}
			beforeInv, beforeCape := p.Char.Inv, p.Char.Equip[15]
			st.err = errors.New("database unavailable")

			w.onCombineLindy(session, buildCombinePacket(items, pos))

			if st.saves != 1 || p.Char.Inv != beforeInv || p.Char.Equip[15] != beforeCape ||
				p.Char.ArchLevel370 || p.Char.ArchLevel355 != (test.level == archLockLevel370) ||
				counterBalance(p, fameCounter) != test.fame {
				t.Fatalf("incomplete Lindy rollback: saves=%d 355=%t 370=%t fame=%d inv=%t cape=%+v",
					st.saves, p.Char.ArchLevel355, p.Char.ArchLevel370,
					counterBalance(p, fameCounter), p.Char.Inv != beforeInv, p.Char.Equip[15])
			}
		})
	}
}

// placeItems copies request items into the character's inventory so that
// parseCombineRequest can validate the authoritative snapshot.
func placeItems(ch *model.Char, items [combineSlots]model.Item, pos [combineSlots]int8) {
	for i, item := range items {
		if item.Index != 0 {
			ch.Inv[pos[i]] = item
		}
	}
}

func agathaScenario() (arch, donor, catalyst model.Item, defs map[uint16]model.ItemDef) {
	arch = model.Item{Index: 500}
	setItemSanc(&arch, 9)
	donor = model.Item{Index: 600}
	setItemSanc(&donor, 9)
	catalyst = model.Item{Index: 3140}
	defs = map[uint16]model.ItemDef{
		500: {Index: 500, Pos: 4, StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}},
		600: {Index: 600, Pos: 4, Grade: 5, StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 4}}},
	}
	return
}

// TestCombineAgathaChanceMatchesPortedFormula verifies the corrected chance:
// chance := 100+grade*5+1 (sempre >=106, clamped a 100) fazia a Agatha NUNCA
// falhar. A formula real (GetMatchCombineAgatha, WYD 7.48 GetFunc.cpp:564-628) e
// agathaBaseChance(20) + grade*5 + bonus-de-nivel = 20+25+1 = 46% neste
// cenario (grade 5, nivel 4) -- precisa produzir os DOIS desfechos.
func TestCombineAgathaChanceMatchesPortedFormula(t *testing.T) {
	arch, donor, catalyst, defs := agathaScenario()
	var items [combineSlots]model.Item
	items[0], items[1] = arch, donor
	items[2], items[3], items[4], items[5] = catalyst, catalyst, catalyst, catalyst
	var pos [combineSlots]int8
	pos[0], pos[1], pos[2], pos[3], pos[4], pos[5] = 0, 1, 2, 3, 4, 5
	pkt := buildCombinePacket(items, pos)

	// On success, retain the original Arch item ID, copy the donor effects,
	// and set refinement to +7. The donor's refinement byte changes from 9 to 7.
	expectedSuccess := arch
	expectedSuccess.Eff = donor.Eff
	setItemSanc(&expectedSuccess, 7)

	successes, failures := 0, 0
	const attempts = 200
	for i := 0; i < attempts; i++ {
		w, p, session, st := newCraftWorld(t, "Agatha", defs, 0)
		placeItems(p.Char, items, pos)
		w.onCombineAgatha(session, pkt)
		if st.saves != 1 {
			t.Fatalf("attempt %d: saves=%d, want 1 (persist-before-confirm)", i, st.saves)
		}
		result := p.Char.Inv[0]
		switch {
		case result == expectedSuccess:
			successes++
		case result == (model.Item{}):
			// The historical handler clears the target before the roll and only
			// restores it on success: failure destroys the target as well as catalysts.
			failures++
		default:
			t.Fatalf("attempt %d: unexpected slot 0: %+v", i, result)
		}
	}
	if successes == 0 || failures == 0 {
		t.Fatalf("46%% chance should yield successes and failures in %d attempts; successes=%d failures=%d",
			attempts, successes, failures)
	}
	t.Logf("Agatha: %d/%d successes (~%.0f%%, expected ~46%%)", successes, attempts, 100*float64(successes)/attempts)
}

// TestCombineAgathaRejectsWrongPosition verifies that the Arch item and donor
// must occupy the same equipment slot; rejection consumes and saves nothing.
func TestCombineAgathaRejectsWrongPosition(t *testing.T) {
	arch, donor, catalyst, defs := agathaScenario()
	donor.Index = 601
	d := defs[600]
	d.Index = 601
	d.Pos = 8 // A different slot from the Arch item (4).
	defs[601] = d
	delete(defs, 600)

	var items [combineSlots]model.Item
	items[0], items[1] = arch, donor
	items[2], items[3], items[4], items[5] = catalyst, catalyst, catalyst, catalyst
	var pos [combineSlots]int8
	pos[0], pos[1], pos[2], pos[3], pos[4], pos[5] = 0, 1, 2, 3, 4, 5
	pkt := buildCombinePacket(items, pos)

	w, p, session, st := newCraftWorld(t, "Agatha", defs, 0)
	placeItems(p.Char, items, pos)
	w.onCombineAgatha(session, pkt)
	if st.saves != 0 {
		t.Fatalf("different positions should not save, saves=%d", st.saves)
	}
	if p.Char.Inv[0] != arch {
		t.Fatalf("target item changed despite failed validation: %+v", p.Char.Inv[0])
	}
}

// TestCombineTinySucceedsAndChargesGoldOnce verifies a successful roll and
// charges the composition fee exactly once.
func TestCombineTinySucceedsAndChargesGoldOnce(t *testing.T) {
	main := model.Item{Index: 2723}
	setItemSanc(&main, 9)
	donor := model.Item{Index: 2727}
	setItemSanc(&donor, 9)
	catalyst := model.Item{Index: 702}
	setItemSanc(&catalyst, 9)
	defs := map[uint16]model.ItemDef{
		2723: {Index: 2723, Pos: 64, Grade: 5, StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}},
		2727: {Index: 2727, Pos: 64, Grade: 5},
		702:  {Index: 702, Pos: 1},
	}
	var items [combineSlots]model.Item
	items[0], items[1], items[2] = main, donor, catalyst
	var pos [combineSlots]int8
	pos[0], pos[1], pos[2] = 0, 1, 2
	pkt := buildCombinePacket(items, pos)

	w, p, session, st := newCraftWorld(t, "Tiny", defs, tinyGoldCost+1)
	w.rng = fixedRNG{value: 0}
	placeItems(p.Char, items, pos)
	w.onCombineTiny(session, pkt)

	if st.saves != 1 {
		t.Fatalf("saves=%d, want 1", st.saves)
	}
	if p.Char.Gold != 1 {
		t.Fatalf("gold=%d, want 1 (fee charged exactly once)", p.Char.Gold)
	}
	if p.Char.Inv[0].Index != main.Index || p.Char.Inv[1] != (model.Item{}) {
		t.Fatalf("unexpected result: slot0=%+v slot1=%+v", p.Char.Inv[0], p.Char.Inv[1])
	}
}

func TestCombineTinyRejectsBelowGoldCost(t *testing.T) {
	main := model.Item{Index: 2723}
	setItemSanc(&main, 9)
	donor := model.Item{Index: 2727}
	setItemSanc(&donor, 9)
	catalyst := model.Item{Index: 702}
	setItemSanc(&catalyst, 9)
	defs := map[uint16]model.ItemDef{
		2723: {Index: 2723, Pos: 64, Grade: 5, StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}},
		2727: {Index: 2727, Pos: 64, Grade: 5},
		702:  {Index: 702, Pos: 1},
	}
	var items [combineSlots]model.Item
	items[0], items[1], items[2] = main, donor, catalyst
	var pos [combineSlots]int8
	pos[0], pos[1], pos[2] = 0, 1, 2
	pkt := buildCombinePacket(items, pos)

	w, p, session, st := newCraftWorld(t, "Tiny", defs, tinyGoldCost-1)
	placeItems(p.Char, items, pos)
	w.onCombineTiny(session, pkt)

	if st.saves != 0 {
		t.Fatalf("insufficient gold should not save, saves=%d", st.saves)
	}
	if p.Char.Inv[0] != main || p.Char.Inv[1] != donor {
		t.Fatalf("items should not change: slot0=%+v slot1=%+v", p.Char.Inv[0], p.Char.Inv[1])
	}
}

// TestCombineAylinRollsBackGoldAndInventoryOnSaveFailure covers a storage
// failure: commitCombine must restore both inventory and gold and must not
// confirm success to the client even when the recipe itself was valid.
func TestCombineAylinRollsBackGoldAndInventoryOnSaveFailure(t *testing.T) {
	a := model.Item{Index: 800}
	setItemSanc(&a, 9)
	b := a                              // Aylin requires the same item ID, grade, and refinement.
	catalyst := model.Item{Index: 1774} // catalisador fixo, distinto das joias
	jewel := model.Item{Index: 2441}
	defs := map[uint16]model.ItemDef{800: {Index: 800, Pos: 2, Grade: 3}}

	var items [combineSlots]model.Item
	items[0], items[1] = a, b
	items[2], items[3], items[4], items[5], items[6] = catalyst, jewel, jewel, jewel, jewel
	var pos [combineSlots]int8
	pos[0], pos[1], pos[2], pos[3], pos[4], pos[5], pos[6] = 0, 1, 2, 3, 4, 5, 6
	pkt := buildCombinePacket(items, pos)

	w, p, session, st := newCraftWorld(t, "Aylin", defs, aylinGoldCost+1000)
	placeItems(p.Char, items, pos)
	st.err = errors.New("disco cheio")
	oldGold := p.Char.Gold
	oldInv := p.Char.Inv

	w.onCombineAylin(session, pkt)

	if st.saves != 1 {
		t.Fatalf("should attempt exactly one save, saves=%d", st.saves)
	}
	if p.Char.Gold != oldGold {
		t.Fatalf("gold was not rolled back: got=%d want=%d", p.Char.Gold, oldGold)
	}
	if p.Char.Inv != oldInv {
		t.Fatalf("inventory was not rolled back after save failure")
	}
}

func TestAylinChanceUsesOneToHundredBoundary(t *testing.T) {
	w := &World{rng: fixedRNG{value: 39}} // visible roll 40
	if got := w.rollPercent(40); !got.Success || got.Roll != 40 {
		t.Fatalf("roll 40/40 should succeed: %+v", got)
	}
	w.rng = fixedRNG{value: 40} // visible roll 41
	if got := w.rollPercent(40); got.Success || got.Roll != 41 {
		t.Fatalf("roll 41/40 should fail: %+v", got)
	}
}

func TestCombineEhreRollsBackSoulOnSaveFailure(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Ehre", nil, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	for index, itemIndex := range soulRecipes[0] {
		items[index] = model.Item{Index: itemIndex}
		pos[index] = int8(index)
	}
	placeItems(p.Char, items, pos)
	p.Char.SoulInfo = 7
	before := p.Char.Inv
	st.err = errors.New("postgres indisponivel")

	w.onCombineEhre(session, buildCombinePacket(items, pos))

	if st.saves != 1 || p.Char.SoulInfo != 7 || p.Char.Inv != before {
		t.Fatalf("incomplete Ehre soul rollback: saves=%d soul=%d inventoryChanged=%t",
			st.saves, p.Char.SoulInfo, p.Char.Inv != before)
	}
}

func TestCombineEhreRollsBackExperienceOnSaveFailure(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Ehre", nil, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0], items[1] = model.Item{Index: 697}, model.Item{Index: 697}
	items[2] = model.Item{Index: 3338, Eff: [6]byte{43, 8}}
	pos[0], pos[1], pos[2] = 0, 1, 2
	placeItems(p.Char, items, pos)
	p.Char.Evolution = "celestial"
	p.Char.Exp = 10_000_000
	p.Char.Score = testScore(model.Score{Level: 190, MaxHP: 100, CurHP: 100})
	beforeInv, beforeExp := p.Char.Inv, p.Char.Exp
	st.err = errors.New("postgres indisponivel")

	w.onCombineEhre(session, buildCombinePacket(items, pos))

	if st.saves != 1 || p.Char.Exp != beforeExp || p.Char.Inv != beforeInv {
		t.Fatalf("incomplete Ehre experience rollback: saves=%d exp=%d/%d inventoryChanged=%t",
			st.saves, p.Char.Exp, beforeExp, p.Char.Inv != beforeInv)
	}
}

func TestCombineEhreRejectsRefinementWithoutEffectSlot(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Ehre", nil, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0], items[1] = model.Item{Index: 697}, model.Item{Index: 697}
	items[2] = model.Item{
		Index: 3338,
		UID:   "11111111111141118111111111111111",
		Eff:   [6]byte{1, 10, 2, 20, 3, 30},
	}
	pos[0], pos[1], pos[2] = 0, 1, 2
	placeItems(p.Char, items, pos)
	p.Char.Evolution = "celestial"
	p.Char.Exp = 10_000_000
	p.Char.Score = testScore(model.Score{
		Level: 190, MaxHP: 100, CurHP: 100,
	})
	beforeInv, beforeExp := p.Char.Inv, p.Char.Exp

	w.onCombineEhre(session, buildCombinePacket(items, pos))

	if st.saves != 0 || p.Char.Inv != beforeInv || p.Char.Exp != beforeExp {
		t.Fatalf("Ehre consumed items without being able to store EF_SANC: saves=%d inv=%t exp=%d/%d",
			st.saves, p.Char.Inv != beforeInv, p.Char.Exp, beforeExp)
	}
}

func TestCombineOdinCapeRejectsWhenRefinementHasNoEffectSlot(t *testing.T) {
	w, p, session, st := newCraftWorld(t, "Odin", nil, 0)
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	recipe := []uint16{4127, 4127, 5135, 413, 413, 413, 413}
	for index, itemIndex := range recipe {
		items[index] = model.Item{Index: itemIndex}
		pos[index] = int8(index)
	}
	placeItems(p.Char, items, pos)
	p.Char.Evolution = "celestial"
	p.Char.Score = testScore(model.Score{Level: 39, MaxHP: 100, CurHP: 100})
	p.Char.Equip[15] = model.Item{
		Index: 3199,
		UID:   "11111111111141118111111111111111",
		Eff:   [6]byte{1, 10, 2, 20, 3, 30},
	}
	beforeInv, beforeCape := p.Char.Inv, p.Char.Equip[15]

	w.onCombineOdin(session, buildCombinePacket(items, pos))

	if st.saves != 0 || p.Char.Inv != beforeInv || p.Char.Equip[15] != beforeCape {
		t.Fatalf("Odin consumed recipe without being able to store EF_SANC: saves=%d inv=%t cape=%+v",
			st.saves, p.Char.Inv != beforeInv, p.Char.Equip[15])
	}
}

// TestCombineRejectsBlockedItem rejects Atila's_Crown (747) in any of the
// eight slots. The historical Agatha handler also checks this restriction.
func TestCombineRejectsBlockedItem(t *testing.T) {
	defs := map[uint16]model.ItemDef{
		blockedCombineItem: {Index: blockedCombineItem, Name: "Atila's_Crown"},
		500:                {Index: 500},
	}
	for _, npc := range []string{"Compositor", "Agatha", "Aylin", "Lindy", "Ehre", "Odin", "Tiny"} {
		for slot := 0; slot < combineSlots; slot++ {
			var items [combineSlots]model.Item
			var pos [combineSlots]int8
			items[slot] = model.Item{Index: blockedCombineItem}
			for i := range pos {
				pos[i] = int8(i)
			}
			w, p, session, _ := newCraftWorld(t, npc, defs, 0)
			placeItems(p.Char, items, pos)

			if _, _, ok := w.beginCombine(session, buildCombinePacket(items, pos), npc); ok {
				t.Errorf("%s: item %d in slot %d was accepted", npc, blockedCombineItem, slot)
			}
		}
	}
}

// W2PP GetMatchCombineAilyn plus the WYD-Go Ancient-weapon rule.
func TestAylinRecipeRules(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		811:  {Index: 811, Pos: 192},              // normal weapon
		2491: {Index: 2491, Pos: 192, Grade: 5},   // Diamond Ancient
		2494: {Index: 2494, Pos: 192, Grade: 8},   // Garnet Ancient
		1209: {Index: 1209, Pos: 4, Grade: 1},     // armor
		3502: {Index: 3502, Pos: 2, Grade: 26},    // Mystic Cythera
		1711: {Index: 1711, Pos: 128, Unique: 51}, // shield
	}}
	request := func(index uint16, gems ...uint16) combineRequest {
		var req combineRequest
		req.Items[0], req.Items[1] = model.Item{Index: index}, model.Item{Index: index}
		setItemSanc(&req.Items[0], 9)
		setItemSanc(&req.Items[1], 9)
		req.Items[2] = model.Item{Index: 1774}
		for i, gem := range gems {
			req.Items[3+i] = model.Item{Index: gem}
		}
		return req
	}
	for _, tc := range []struct {
		name    string
		req     combineRequest
		wantRaw byte
	}{
		{"normal-weapon", request(811, 2441, 2441, 2441, 2441), 0},
		{"ancient-diamond", request(2491, 2441, 2441, 2441, 2441), 230},
		{"ancient-wrong-gem", request(2491, 2443, 2443, 2443, 2443), 0},
		{"ancient-garnet", request(2494, 2444, 2444, 2444, 2444), 233},
		{"armor-coral", request(1209, 2443, 2443, 2443, 2443), 232},
		{"armor-mixed-gems", request(1209, 2441, 2442, 2443, 2444), 0},
		{"cythera-emerald", request(3502, 2442, 2442, 2442, 2442), 231},
		{"shield-garnet", request(1711, 2444, 2444, 2444, 2444), 233},
	} {
		t.Run(tc.name, func(t *testing.T) {
			result, _, valid := w.aylinRecipe(tc.req)
			raw, _ := sancRaw(result)
			if valid != (tc.wantRaw != 0) || valid && raw != tc.wantRaw {
				t.Fatalf("valid=%t raw=%d", valid, raw)
			}
		})
	}
}
