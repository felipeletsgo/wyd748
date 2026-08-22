package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func putoutSealPacket(sourceType, sourcePos uint32, x, y uint16, name string) []byte {
	pkt := make([]byte, 52)
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpPutoutSeal)
	binary.LittleEndian.PutUint32(pkt[12:16], sourceType)
	binary.LittleEndian.PutUint32(pkt[16:20], sourcePos)
	binary.LittleEndian.PutUint32(pkt[20:24], placeInv)
	binary.LittleEndian.PutUint32(pkt[24:28], 0)
	binary.LittleEndian.PutUint16(pkt[28:30], x)
	binary.LittleEndian.PutUint16(pkt[30:32], y)
	copy(pkt[34:50], name)
	return pkt
}

func TestCelestialSealRoundTripUsesCargoAndCreatesNewCharacter(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Celestial", 2100, 2100)
	uid, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	p.Char.UID = uid
	p.Char.Evolution = "celestial"
	p.Char.Score.Level = 90
	p.Char.Inv[0] = model.Item{Index: model.CelestialSealItem}
	w := worldWithNetworkedPlayers(p)
	w.store = &craftStore{}
	w.charSpawn = model.CharacterSpawn{X: 2100, Y: 2100}
	w.items = map[uint16]model.ItemDef{model.CelestialSealItem: {Index: model.CelestialSealItem}}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{model.CelestialSealItem: {Action: "celestial_capsule", Consume: true}},
		ItemCodes: map[uint16]int{model.CelestialSealItem: 206},
	}

	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.InWorld || p.Char != nil || len(p.Account.CelestialCapsules) != 1 ||
		p.Account.Cargo[0].Index != model.CelestialSealItem ||
		p.Account.Chars[0].Name != "" {
		t.Fatalf("encapsulamento incompleto: inWorld=%v char=%v capsules=%d cargo=%+v chars=%+v",
			p.InWorld, p.Char, len(p.Account.CelestialCapsules), p.Account.Cargo[0], p.Account.Chars)
	}
	capsuleID, ok := model.CelestialSealID(p.Account.Cargo[0])
	if !ok || capsuleID != p.Account.CelestialCapsules[0].ID {
		t.Fatalf("selo nao referencia a capsula: id=%d ok=%v capsule=%d", capsuleID, ok,
			p.Account.CelestialCapsules[0].ID)
	}

	// Um personagem ativo diferente pode extrair o selo. O slot vazio deixado
	// pelo Celestial é reutilizado pela nova ficha, como no DBSrv nativo.
	active, _ := networkedTestPlayer(2, "Carrier", 2100, 2100)
	activeUID, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	active.Char.UID = activeUID
	p.Account.Chars = []model.Char{{}, *active.Char}
	p.Account.Chars[1].UID = activeUID
	p.Account.Chars[1].Name = "Carrier"
	p.Account.Chars[1].Score = active.Char.Score
	p.Account.Chars[1].Inv[0] = p.Account.Cargo[0]
	p.Account.Cargo[0] = model.Item{}
	p.Char = &p.Account.Chars[1]
	p.CharSlot, p.ID, p.InWorld, p.X, p.Y = 1, 2, true, 2100, 2100
	p.Visible = map[uint16]struct{}{}
	w.players[p.Session] = p
	w.playersByID[p.ID] = p
	w.charNames = map[string]struct{}{"carrier": {}}

	w.onPutoutSeal(p.Session, putoutSealPacket(placeInv, 0, 2100, 2100, "Restored"))
	if p.InWorld || p.Char != nil || len(p.Account.CelestialCapsules) != 0 ||
		p.Account.Chars[1].Inv[0].Index != 0 || p.Account.Chars[0].Name != "Restored" {
		t.Fatalf("extracao incompleta: inWorld=%v char=%v capsules=%d cargo=%+v chars=%+v",
			p.InWorld, p.Char, len(p.Account.CelestialCapsules), p.Account.Cargo[0], p.Account.Chars)
	}
	created := p.Account.Chars[0]
	if created.Evolution != "celestial" || created.Score.Level != 90 ||
		created.Gold != 0 || created.Equip[1].Index != 0 || created.Inv[0].Index != 0 {
		t.Fatalf("ficha extraida perdeu semantica nativa: %+v", created)
	}
}

func TestCelestialCapsuleRejectsNonEmptySealAndBadPutoutName(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Mortal", 2100, 2100)
	p.Char.Evolution = "mortal"
	p.Char.Inv[0] = model.Item{Index: model.CelestialSealItem, Eff: [6]byte{1, 2, 3, 4, 0, 0}}
	w := worldWithNetworkedPlayers(p)
	w.store = &craftStore{}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{model.CelestialSealItem: {Action: "celestial_capsule", Consume: true}},
		ItemCodes: map[uint16]int{model.CelestialSealItem: 206},
	}
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != model.CelestialSealItem || len(p.Account.CelestialCapsules) != 0 {
		t.Fatal("selo preenchido foi consumido fora do fluxo de extracao")
	}
	if req, ok := parsePutoutSealRequest(putoutSealPacket(placeStorage, 0, 0, 0, "ab")); !ok || req.name != "ab" {
		t.Fatal("parser do putout perdeu o nome curto")
	}
}

func TestCelestialSealRollbackOnPersistenceFailure(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Celestial", 2100, 2100)
	uid, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	p.Char.UID = uid
	p.Char.Evolution = "celestial"
	p.Char.Score.Level = 90
	p.Char.Inv[0] = model.Item{Index: model.CelestialSealItem}
	w := worldWithNetworkedPlayers(p)
	w.store = &atomicCharStateMemoryStore{atomicErr: errors.New("database unavailable")}
	w.charSpawn = model.CharacterSpawn{X: 2100, Y: 2100}
	w.items = map[uint16]model.ItemDef{model.CelestialSealItem: {Index: model.CelestialSealItem}}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{model.CelestialSealItem: {Action: "celestial_capsule", Consume: true}},
		ItemCodes: map[uint16]int{model.CelestialSealItem: 206},
	}

	w.onUseItem(p.Session, useItemPacket(0, 0))
	if !p.InWorld || p.Char == nil || p.Char.Inv[0].Index != model.CelestialSealItem ||
		len(p.Account.CelestialCapsules) != 0 || p.Account.Cargo[0].Index != 0 {
		t.Fatalf("falha de persistencia nao restaurou o estado: inWorld=%v char=%v inv=%+v capsules=%d cargo=%+v",
			p.InWorld, p.Char, p.Char.Inv[0], len(p.Account.CelestialCapsules), p.Account.Cargo[0])
	}
}

func celestialExtractionFixture(t *testing.T) (*World, *Player, model.Item) {
	t.Helper()
	p, _ := networkedTestPlayer(1, "Carrier", 2100, 2100)
	carrierUID, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	sourceUID, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	itemUID, err := model.NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	p.Char.UID = carrierUID
	p.CharSlot = 0
	// len==cap força append/realloc ao criar o segundo slot e protege contra o
	// ponteiro antigo de Player.Char voltar a ser usado.
	p.Account.Chars = append([]model.Char(nil), *p.Char)
	p.Char = &p.Account.Chars[0]
	seal := model.NewCelestialSeal(itemUID, 1)
	p.Char.Inv[0] = seal
	sealed := model.Char{
		UID: sourceUID, Name: "Original", Class: 1, Evolution: "celestial",
		Score: testExtended(model.Score{Level: 90, Str: 100}),
	}
	p.Account.CelestialCapsules = []model.CelestialCapsule{{
		ID: 1, ItemUID: itemUID, SourceUID: sourceUID, Character: sealed,
	}}
	w := worldWithNetworkedPlayers(p)
	w.store = &craftStore{}
	w.charSpawn = model.CharacterSpawn{X: 2100, Y: 2100}
	w.charNames = map[string]struct{}{"carrier": {}}
	return w, p, seal
}

func TestCelestialExtractionRebindsActiveCharacterAfterSliceGrowth(t *testing.T) {
	w, p, _ := celestialExtractionFixture(t)
	w.onPutoutSeal(p.Session, putoutSealPacket(placeInv, 0, 2100, 2100, "Restored"))
	if p.InWorld || p.Char != nil || len(p.Account.Chars) != 2 ||
		p.Account.Chars[0].Inv[0].Index != 0 || p.Account.Chars[1].Name != "Restored" ||
		len(p.Account.CelestialCapsules) != 0 {
		t.Fatalf("extracao apos realloc inconsistente: inWorld=%v chars=%+v capsules=%d",
			p.InWorld, p.Account.Chars, len(p.Account.CelestialCapsules))
	}
}

func TestCelestialExtractionRollbackRestoresSealSnapshotAndPointer(t *testing.T) {
	w, p, seal := celestialExtractionFixture(t)
	w.store = &atomicCharStateMemoryStore{atomicErr: errors.New("database unavailable")}
	w.onPutoutSeal(p.Session, putoutSealPacket(placeInv, 0, 2100, 2100, "Restored"))
	if !p.InWorld || p.Char == nil || p.Char != &p.Account.Chars[0] ||
		len(p.Account.Chars) != 1 || p.Char.Inv[0] != seal ||
		len(p.Account.CelestialCapsules) != 1 {
		t.Fatalf("rollback de extracao inconsistente: inWorld=%v char=%p accountChar=%p chars=%d seal=%+v capsules=%d",
			p.InWorld, p.Char, &p.Account.Chars[0], len(p.Account.Chars), p.Account.Chars[0].Inv[0],
			len(p.Account.CelestialCapsules))
	}
}

func TestCapsuleInfoUses748Projection(t *testing.T) {
	learned := uint32(0)
	for bit := 0; bit < 24; bit++ {
		learned |= uint32(1) << bit
	}
	capsule := &model.CelestialCapsule{Character: model.Char{
		Class: 1, Evolution: "celestial", LearnedSkill: learned, ArchCrystals: 4,
		Score: testExtended(model.Score{
			Level: 90, Str: 100_000, Int: 20, Dex: 30, Con: 40,
			Mastery: [4]uint32{50, 60, 70, 80},
		}),
		AlternateCelestial: &model.CelestialForm{Class: 3, Evolution: "subcelestial",
			Score: testExtended(model.Score{Level: 80})},
	}}
	info := capsuleInfoFor(capsule)
	if info.Class != 19 || info.Level != 90 || info.Str != 32767 ||
		info.Mastery != [2]uint16{50, 60} || info.Quest != 0x1000 {
		t.Fatalf("projecao 7.48 incorreta: %+v", info)
	}
	wantSkills := [9]uint16{5, 6, 7, 13, 14, 15, 21, 22, 23}
	for i := range wantSkills {
		wantSkills[i] += 24 // classe Foema: skill global com base 24
	}
	if info.Skill != wantSkills {
		t.Fatalf("resumo de skills=%v, esperado=%v", info.Skill, wantSkills)
	}
}

func TestCelestialExtractionCancelsTradeBeforeChangingAggregate(t *testing.T) {
	w, p, seal := celestialExtractionFixture(t)
	p.Trade = &TradeState{OpponentID: 999}
	w.onPutoutSeal(p.Session, putoutSealPacket(placeInv, 0, 2100, 2100, "Restored"))
	if p.Trade != nil || !p.InWorld || p.Char == nil || p.Char.Inv[0] != seal ||
		len(p.Account.CelestialCapsules) != 1 || len(p.Account.Chars) != 1 {
		t.Fatalf("cancelamento de trade alterou a capsula: trade=%v inWorld=%v seal=%+v capsules=%d chars=%d",
			p.Trade, p.InWorld, p.Char.Inv[0], len(p.Account.CelestialCapsules), len(p.Account.Chars))
	}
}

func TestFilledCelestialSealOwnershipRequiresSingleItem(t *testing.T) {
	w, p, seal := celestialExtractionFixture(t)
	w.items = map[uint16]model.ItemDef{model.CelestialSealItem: {Index: model.CelestialSealItem}}
	if err := w.validateTradableItem(seal); err != nil {
		t.Fatalf("selo preenchido deveria ser negociavel: %v", err)
	}
	if !accountOwnsCapsuleSeal(p.Account, &p.Account.CelestialCapsules[0]) {
		t.Fatal("selo valido no Carry nao foi associado ao snapshot")
	}
	p.Char.Inv[1] = seal
	if accountOwnsCapsuleSeal(p.Account, &p.Account.CelestialCapsules[0]) {
		t.Fatal("selo duplicado foi aceito como propriedade valida")
	}
}

func tradeCapsuleFixture(t *testing.T, owner *Player, id uint16, slot int) model.Item {
	t.Helper()
	itemUID, err := model.NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	sourceUID, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	seal := model.NewCelestialSeal(itemUID, id)
	owner.Char.Inv[slot] = seal
	owner.Account.CelestialCapsules = append(owner.Account.CelestialCapsules, model.CelestialCapsule{
		ID: id, ItemUID: itemUID, SourceUID: sourceUID,
		Character: model.Char{
			UID: sourceUID, Name: "ReadyCelestial", Class: 1, Evolution: "celestial",
			Score: testExtended(model.Score{Level: 190, Str: 1_000}),
		},
	})
	return seal
}

func confirmedCapsuleTrade(a, b *Player, aItem model.Item, aSlot int) {
	aPositions := emptyTradePositions()
	aPositions[0] = int8(aSlot)
	a.Trade = &TradeState{
		OpponentID: b.ID, Checked: true, CarryPos: aPositions,
		Items: [maxTradeItems]model.Item{aItem},
	}
	b.Trade = &TradeState{OpponentID: a.ID, Checked: true, CarryPos: emptyTradePositions()}
}

func TestFilledCelestialSealTradeTransfersSnapshotAtomically(t *testing.T) {
	a, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	b, _ := networkedTestPlayer(2, "Buyer", 2101, 2100)
	w, st := economyWorld(a, b)
	w.items[model.CelestialSealItem] = model.ItemDef{Index: model.CelestialSealItem}
	seal := tradeCapsuleFixture(t, a, 7, 0)
	confirmedCapsuleTrade(a, b, seal, 0)

	w.commitTrade(a, b)
	if a.Trade != nil || b.Trade != nil || a.Char.Inv[0].Index != 0 || b.Char.Inv[0].UID != seal.UID {
		t.Fatalf("item nao foi transferido: a=%+v b=%+v", a.Char.Inv[0], b.Char.Inv[0])
	}
	if len(a.Account.CelestialCapsules) != 0 || len(b.Account.CelestialCapsules) != 1 ||
		b.Account.CelestialCapsules[0].ItemUID != seal.UID {
		t.Fatalf("snapshot nao acompanhou o selo: a=%+v b=%+v",
			a.Account.CelestialCapsules, b.Account.CelestialCapsules)
	}
	if st.batchSaves != 1 {
		t.Fatalf("trade deveria salvar duas contas em um lote: %d", st.batchSaves)
	}
}

func TestFilledCelestialSealTradeRemapsDestinationIDCollision(t *testing.T) {
	a, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	b, _ := networkedTestPlayer(2, "Buyer", 2101, 2100)
	w, _ := economyWorld(a, b)
	w.items[model.CelestialSealItem] = model.ItemDef{Index: model.CelestialSealItem}
	seal := tradeCapsuleFixture(t, a, 1, 0)
	_ = tradeCapsuleFixture(t, b, 1, 5)
	confirmedCapsuleTrade(a, b, seal, 0)

	w.commitTrade(a, b)
	receivedID, ok := model.CelestialSealID(b.Char.Inv[0])
	if !ok || receivedID == 1 {
		t.Fatalf("ID recebido nao foi remapeado: id=%d ok=%v item=%+v", receivedID, ok, b.Char.Inv[0])
	}
	capsule, _ := w.capsuleByID(b.Account, receivedID)
	if capsule == nil || capsule.ItemUID != seal.UID || len(b.Account.CelestialCapsules) != 2 {
		t.Fatalf("snapshot remapeado inconsistente: id=%d capsules=%+v", receivedID,
			b.Account.CelestialCapsules)
	}
}

func TestFilledCelestialSealTradeRollsBackItemAndSnapshot(t *testing.T) {
	a, _ := networkedTestPlayer(1, "Seller", 2100, 2100)
	b, _ := networkedTestPlayer(2, "Buyer", 2101, 2100)
	w, st := economyWorld(a, b)
	w.items[model.CelestialSealItem] = model.ItemDef{Index: model.CelestialSealItem}
	seal := tradeCapsuleFixture(t, a, 3, 0)
	confirmedCapsuleTrade(a, b, seal, 0)
	st.err = errors.New("database unavailable")

	w.commitTrade(a, b)
	if a.Char.Inv[0] != seal || b.Char.Inv[0].Index != 0 ||
		len(a.Account.CelestialCapsules) != 1 || len(b.Account.CelestialCapsules) != 0 {
		t.Fatalf("rollback separou selo e snapshot: aItem=%+v bItem=%+v aCaps=%+v bCaps=%+v",
			a.Char.Inv[0], b.Char.Inv[0], a.Account.CelestialCapsules, b.Account.CelestialCapsules)
	}
}
