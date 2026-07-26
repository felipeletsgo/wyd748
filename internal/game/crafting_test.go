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
			t.Fatalf("resultado %d: mensagem %q, esperado %q", result, got, want)
		}
	}
}

func TestCombineNPCResolvesClientSideOpenedArtisan(t *testing.T) {
	w := &World{}
	artisan := &Mob{
		ID: 1500, X: 100, Y: 100,
		Def: &model.NPCDef{
			Name:     "Lindy",
			Tipo:     model.TipoNPC,
			Extended: &model.ExtendedScore{Merchant: craftingMerchant},
		},
	}
	w.registerMobSpatial(artisan)
	p := &Player{
		Char: &model.Char{}, X: 102, Y: 100,
		Visible: map[uint16]struct{}{artisan.ID: {}},
	}

	got, err := w.combineNPC(p, "Lindy")
	if err != nil {
		t.Fatalf("combineNPC recusou artesao aberto localmente: %v", err)
	}
	if got != artisan || p.CraftNPC != artisan.ID {
		t.Fatalf("contexto incorreto: mob=%v CraftNPC=%d", got, p.CraftNPC)
	}
}

func TestCombineNPCReplacesStaleArtisanContext(t *testing.T) {
	w := &World{}
	old := &Mob{ID: 1500, X: 100, Y: 100, Def: &model.NPCDef{
		Name: "Tiny", Tipo: model.TipoNPC,
		Extended: &model.ExtendedScore{Merchant: craftingMerchant},
	}}
	wanted := &Mob{ID: 1501, X: 101, Y: 100, Def: &model.NPCDef{
		Name: "Aylin", Tipo: model.TipoNPC,
		Extended: &model.ExtendedScore{Merchant: craftingMerchant},
	}}
	w.registerMobSpatial(old)
	w.registerMobSpatial(wanted)
	p := &Player{
		Char: &model.Char{}, X: 100, Y: 100, CraftNPC: old.ID,
		Visible: map[uint16]struct{}{old.ID: {}, wanted.ID: {}},
	}

	got, err := w.combineNPC(p, "Aylin")
	if err != nil || got != wanted || p.CraftNPC != wanted.ID {
		t.Fatalf("nao substituiu contexto antigo: mob=%v CraftNPC=%d err=%v", got, p.CraftNPC, err)
	}
}

// craftStore e um store.Store minimo, injetavel com erro, para testar o
// caminho persist-before-confirm/rollback dos handlers de craft (mesmo padrao
// de autoSaveStore em autosave_test.go).
type craftStore struct {
	saves int
	err   error
}

func (s *craftStore) LoadAccount(string) (*model.Account, error) {
	return nil, errors.New("nao usado")
}
func (s *craftStore) SaveAccount(acc *model.Account) error {
	s.saves++
	return s.err
}
func (s *craftStore) CharacterNameExists(string) (bool, error) { return false, nil }

// newCraftWorld monta um World minimo com um artesao `npcName` (Merchant tipo
// craftingMerchant) visivel e ao alcance do jogador, pronto para exercitar um
// handler onCombine*. items entra no catalogo autoritativo (w.items).
func newCraftWorld(t *testing.T, npcName string, items map[uint16]model.ItemDef, gold uint32) (*World, *Player, *net.Session, *craftStore) {
	t.Helper()
	st := &craftStore{}
	w := &World{store: st, items: items}
	mob := &Mob{
		ID: 2000,
		Def: &model.NPCDef{
			Name:     npcName,
			Extended: &model.ExtendedScore{Merchant: craftingMerchant},
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

// buildCombinePacket monta os 84 bytes do MSG_CombineItem nativo: 8 STRUCT_ITEM
// (Index LE + Eff[6]) a partir de 12, seguidos de 8 bytes de posicao a partir
// de 76. Espelha o parsing de parseCombineRequest (crafting.go).
func buildCombinePacket(items [combineSlots]model.Item, pos [combineSlots]int8) []byte {
	pkt := make([]byte, combinePacketSize)
	for i, item := range items {
		binary.LittleEndian.PutUint16(pkt[12+i*8:], item.Index)
		copy(pkt[14+i*8:20+i*8], item.Eff[:])
		pkt[76+i] = byte(pos[i])
	}
	return pkt
}

// placeItems copia os itens de req para o inventario do personagem nas
// posicoes dadas, para que parseCombineRequest valide o snapshot autoritativo.
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

// TestCombineAgathaChanceMatchesPortedFormula prova a correcao do bug: antes,
// chance := 100+grade*5+1 (sempre >=106, clamped a 100) fazia a Agatha NUNCA
// falhar. A formula real (GetMatchCombineAgatha, W2PP GetFunc.cpp:564-628) e
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

	// Resultado esperado no sucesso: Index do item ARCH original + Eff do
	// doador, mas com o SANC forcado a +7 (onCombineAgatha faz
	// result.Eff=donor.Eff seguido de setItemSanc(&result,7) -- o doador foi
	// setado a +9, entao o byte de valor muda de 9 para 7 no resultado).
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
			t.Fatalf("tentativa %d: saves=%d, quer 1 (persist-before-confirm)", i, st.saves)
		}
		result := p.Char.Inv[0]
		switch {
		case result == expectedSuccess:
			successes++
		case result == (model.Item{}):
			// Fiel a fonte: _MSG_CombineItemAgatha.cpp limpa o item alvo ANTES
			// do roll e so o reescreve em caso de sucesso -- falha destroi o
			// alvo, nao so os catalisadores. Confirmado com o codigo-fonte.
			failures++
		default:
			t.Fatalf("tentativa %d: slot 0 inesperado: %+v", i, result)
		}
	}
	if successes == 0 || failures == 0 {
		t.Fatalf("chance=46%% deveria produzir sucesso E falha em %d tentativas; sucessos=%d falhas=%d",
			attempts, successes, failures)
	}
	t.Logf("Agatha: %d/%d sucessos (~%.0f%%, esperado ~46%%)", successes, attempts, 100*float64(successes)/attempts)
}

// TestCombineAgathaRejectsWrongPosition prova que a validacao de posicao (o
// item Arch e o doador precisam ocupar o MESMO slot de equip) continua ativa
// apos a mudanca de formula -- nao consome nada, nao salva.
func TestCombineAgathaRejectsWrongPosition(t *testing.T) {
	arch, donor, catalyst, defs := agathaScenario()
	donor.Index = 601
	d := defs[600]
	d.Index = 601
	d.Pos = 8 // slot diferente do arch (4)
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
		t.Fatalf("posicoes divergentes nao deveriam salvar, saves=%d", st.saves)
	}
	if p.Char.Inv[0] != arch {
		t.Fatalf("item alvo foi alterado apesar da validacao falhar: %+v", p.Char.Inv[0])
	}
}

// TestCombineTinySucceedsAndChargesGoldOnce prova que o refactor do Tiny (para
// reusar beginCombine/commitCombine em vez de reimplementar a sequencia) nao
// mudou o comportamento: tinyBaseChance=100 torna o roll sempre >=100%
// (chance nunca cai abaixo de 100 para nivel>=0), entao o sucesso e
// deterministico -- serve de regressao ao refactor do Passo 2.
func TestCombineTinySucceedsAndChargesGoldOnce(t *testing.T) {
	main := model.Item{Index: 700}
	setItemSanc(&main, 9)
	donor := model.Item{Index: 701}
	catalyst := model.Item{Index: 702}
	setItemSanc(&catalyst, 9) // tinyRecipe exige itemSanc(catalyst) >= 9
	defs := map[uint16]model.ItemDef{
		700: {Index: 700, Pos: 64, Grade: 5, StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}},
		701: {Index: 701, Pos: 64, Grade: 5},
		702: {Index: 702, Pos: 1},
	}
	var items [combineSlots]model.Item
	items[0], items[1], items[2] = main, donor, catalyst
	var pos [combineSlots]int8
	pos[0], pos[1], pos[2] = 0, 1, 2
	pkt := buildCombinePacket(items, pos)

	w, p, session, st := newCraftWorld(t, "Tiny", defs, tinyGoldCost+1)
	placeItems(p.Char, items, pos)
	w.onCombineTiny(session, pkt)

	if st.saves != 1 {
		t.Fatalf("saves=%d, quer 1", st.saves)
	}
	if p.Char.Gold != 1 {
		t.Fatalf("gold=%d, quer 1 (custo cobrado exatamente uma vez)", p.Char.Gold)
	}
	if p.Char.Inv[0].Index != main.Index || p.Char.Inv[1] != (model.Item{}) {
		t.Fatalf("resultado inesperado: slot0=%+v slot1=%+v", p.Char.Inv[0], p.Char.Inv[1])
	}
}

func TestCombineTinyRejectsBelowGoldCost(t *testing.T) {
	main := model.Item{Index: 700}
	setItemSanc(&main, 9)
	donor := model.Item{Index: 701}
	catalyst := model.Item{Index: 702}
	setItemSanc(&catalyst, 9) // tinyRecipe exige itemSanc(catalyst) >= 9
	defs := map[uint16]model.ItemDef{
		700: {Index: 700, Pos: 64, Grade: 5, StaticEffects: []model.StaticEffect{{Name: "EF_MOBTYPE", Value: 1}}},
		701: {Index: 701, Pos: 64, Grade: 5},
		702: {Index: 702, Pos: 1},
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
		t.Fatalf("gold insuficiente nao deveria salvar, saves=%d", st.saves)
	}
	if p.Char.Inv[0] != main || p.Char.Inv[1] != donor {
		t.Fatalf("itens nao deveriam mudar: slot0=%+v slot1=%+v", p.Char.Inv[0], p.Char.Inv[1])
	}
}

// TestCombineAylinRollsBackGoldAndInventoryOnSaveFailure cobre o caminho de
// persist-before-confirm com falha de disco: commitCombine precisa restaurar
// TODO o estado (inventario E gold) e nao confirmar sucesso ao client quando
// o save falha, mesmo quando o craft em si teria sido valido.
func TestCombineAylinRollsBackGoldAndInventoryOnSaveFailure(t *testing.T) {
	a := model.Item{Index: 800}
	setItemSanc(&a, 9)
	b := a                              // mesmo Index/grade/sanc exigido pela Aylin
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
		t.Fatalf("deveria tentar salvar exatamente 1 vez, saves=%d", st.saves)
	}
	if p.Char.Gold != oldGold {
		t.Fatalf("gold nao foi revertido: got=%d quer=%d", p.Char.Gold, oldGold)
	}
	if p.Char.Inv != oldInv {
		t.Fatalf("inventario nao foi revertido apos falha de save")
	}
}

// TestCombineRejectsBlockedItem: Atila's_Crown (747) em QUALQUER um dos oito
// slots recusa a composicao. O nativo repete essa varredura em 10 das 11
// GetMatchCombineXxx (GetFunc.cpp:57, 218, 263, 337, 400, 461, 505, 634, 669 e
// 740); a Agatha (564) e a UNICA sem ela, e essa excecao e deliberada.
func TestCombineRejectsBlockedItem(t *testing.T) {
	defs := map[uint16]model.ItemDef{
		blockedCombineItem: {Index: blockedCombineItem, Name: "Atila's_Crown"},
		500:                {Index: 500},
	}
	for _, npc := range []string{"Compositor", "Aylin", "Lindy", "Ehre", "Odin", "Tiny"} {
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
				t.Errorf("%s: item %d no slot %d foi aceito", npc, blockedCombineItem, slot)
			}
		}
	}
}

// TestAgathaAcceptsBlockedItem preserva a excecao do nativo: a Agatha nao varre
// os slots atras do 747. Se um dia essa trava virar global, este teste quebra e
// obriga a decisao a ser consciente.
func TestAgathaAcceptsBlockedItem(t *testing.T) {
	defs := map[uint16]model.ItemDef{blockedCombineItem: {Index: blockedCombineItem}}
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0] = model.Item{Index: blockedCombineItem}
	for i := range pos {
		pos[i] = int8(i)
	}
	w, p, session, _ := newCraftWorld(t, agathaCombineNPC, defs, 0)
	placeItems(p.Char, items, pos)

	if _, _, ok := w.beginCombine(session, buildCombinePacket(items, pos), agathaCombineNPC); !ok {
		t.Error("a Agatha recusou o 747; o nativo (GetFunc.cpp:564) nao tem essa trava")
	}
}
