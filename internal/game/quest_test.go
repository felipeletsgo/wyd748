package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func questNPC(name string, merchant byte) model.NPCDef {
	return model.NPCDef{Name: name, Extended: &model.ExtendedScore{Merchant: merchant}}
}

func simpleQuest(id int, npc string) model.QuestDef {
	return model.QuestDef{
		ID:  id,
		NPC: npc,
		Messages: model.QuestMessages{
			Offer:   "oferta",
			Success: "sucesso",
		},
	}
}

// --- allowlist e validacao de boot -----------------------------------------

func TestIndexQuestsRecusaNPCInexistente(t *testing.T) {
	file := model.QuestFile{Version: model.QuestFileVersion,
		Quests: []model.QuestDef{simpleQuest(1, "NaoExiste")}}
	if _, err := indexQuests(file, []model.NPCDef{questNPC("Gravedigger", 43)}); err == nil {
		t.Fatal("quest apontando para npc inexistente deveria derrubar o boot")
	}
}

// A condicao mais importante da revisao: quest em cima de NPC que ja tem
// handler proprio nunca dispararia, porque o roteamento testa os tipos
// conhecidos antes. Precisa falhar no boot, nao ser ignorada.
func TestIndexQuestsRecusaNPCReservado(t *testing.T) {
	reserved := map[string]byte{
		"loja nativa":   1,  // merchant == 1
		"loja de gold":  3,  // nibble 3
		"mestre de hab": 19, // merchant == 19
		"cargo":         2,  // nibble 2
	}
	for kind, merchant := range reserved {
		file := model.QuestFile{Version: model.QuestFileVersion,
			Quests: []model.QuestDef{simpleQuest(1, "Alvo")}}
		_, err := indexQuests(file, []model.NPCDef{questNPC("Alvo", merchant)})
		if err == nil {
			t.Fatalf("quest em npc do tipo %q (merchant=%d) deveria falhar no boot", kind, merchant)
		}
	}
}

func TestIndexQuestsAceitaNPCDeQuest(t *testing.T) {
	// merchant 43 = 0x2B -> nibble 11, que e o tipo de quest da base Micronics.
	file := model.QuestFile{Version: model.QuestFileVersion,
		Quests: []model.QuestDef{simpleQuest(1, "Gravedigger")}}
	index, err := indexQuests(file, []model.NPCDef{questNPC("Gravedigger", 43)})
	if err != nil {
		t.Fatalf("npc de quest deveria ser aceito: %v", err)
	}
	if len(index) != 1 {
		t.Fatalf("indice deveria ter 1 quest, tem %d", len(index))
	}
}

// O NPCGener casa nomes com '_' e com espaco; a quest precisa aceitar os dois.
func TestQuestForNPCCasaNomeComUnderscoreEEspaco(t *testing.T) {
	file := model.QuestFile{Version: model.QuestFileVersion,
		Quests: []model.QuestDef{simpleQuest(1, "Drill Master")}}
	npcs := []model.NPCDef{questNPC("Drill_Master", 43)}
	index, err := indexQuests(file, npcs)
	if err != nil {
		t.Fatalf("indexQuests: %v", err)
	}
	w := &World{questsByNPC: index}
	if w.questForNPC(&npcs[0]) == nil {
		t.Fatal("quest deveria casar com o nome usando underscore")
	}
}

func TestQuestForNPCIgnoraNPCForaDaAllowlist(t *testing.T) {
	w := &World{questsByNPC: map[string]*model.QuestDef{}}
	npc := questNPC("Qualquer", 43)
	if w.questForNPC(&npc) != nil {
		t.Fatal("npc fora do quests.json nunca pode virar quest")
	}
}

func TestReservedNPCKind(t *testing.T) {
	cases := map[byte]bool{
		1: true, 3: true, 19: true, 2: true, // loja/mestre/cargo
		43: false, 8: true, 4: false, // quest, crafting reservado, ferreiro
	}
	for merchant, wantReserved := range cases {
		def := questNPC("X", merchant)
		if _, reserved := reservedNPCKind(&def); reserved != wantReserved {
			t.Fatalf("merchant=%d reservado=%v, quer %v", merchant, reserved, wantReserved)
		}
	}
}

// --- defesa server-side ----------------------------------------------------
//
// Nao existe estado pendente: a confirmacao do 7.48 e uma janela CLIENT-side, e
// o servidor recebe um unico pacote por interacao (ClickOk=0 em npc comum,
// ClickOk=1 em npc com o gatilho de confirmacao). Por isso toda a defesa mora
// em questRequirementsMet + flag de concluida, testados aqui.

func questTestPlayer(level uint32, gold uint32) *Player {
	return &Player{
		ID: 1,
		Char: &model.Char{
			Name:     "Felipe",
			Gold:     gold,
			Extended: &model.ExtendedScore{Level: level},
		},
	}
}

func TestQuestRequirementsMet(t *testing.T) {
	w := &World{}

	t.Run("nivel minimo", func(t *testing.T) {
		quest := simpleQuest(1, "X")
		quest.Requires.MinLevel = 50
		if _, ok := w.questRequirementsMet(questTestPlayer(49, 0), &quest); ok {
			t.Fatal("abaixo do nivel minimo deveria recusar")
		}
		if _, ok := w.questRequirementsMet(questTestPlayer(50, 0), &quest); !ok {
			t.Fatal("no nivel minimo deveria passar")
		}
	})

	t.Run("nivel maximo", func(t *testing.T) {
		quest := simpleQuest(1, "X")
		quest.Requires.MaxLevel = 30
		if _, ok := w.questRequirementsMet(questTestPlayer(31, 0), &quest); ok {
			t.Fatal("acima do nivel maximo deveria recusar")
		}
	})

	t.Run("somente mortal", func(t *testing.T) {
		quest := simpleQuest(1, "X")
		quest.Requires.MortalOnly = true
		mortal := questTestPlayer(1, 0)
		if _, ok := w.questRequirementsMet(mortal, &quest); !ok {
			t.Fatal("mortal deveria passar")
		}
		mortal.Char.Evolution = "arch"
		if _, ok := w.questRequirementsMet(mortal, &quest); ok {
			t.Fatal("evolucao avancada deveria ser recusada")
		}
	})

	t.Run("gold", func(t *testing.T) {
		quest := simpleQuest(1, "X")
		quest.Requires.Gold = 1000
		if _, ok := w.questRequirementsMet(questTestPlayer(1, 999), &quest); ok {
			t.Fatal("gold insuficiente deveria recusar")
		}
		if _, ok := w.questRequirementsMet(questTestPlayer(1, 1000), &quest); !ok {
			t.Fatal("gold exato deveria passar")
		}
	})

	t.Run("quest anterior", func(t *testing.T) {
		quest := simpleQuest(2, "X")
		quest.Requires.AfterQuest = 1
		p := questTestPlayer(1, 0)
		if _, ok := w.questRequirementsMet(p, &quest); ok {
			t.Fatal("sem a quest anterior deveria recusar")
		}
		markQuestCompleted(p.Char, 1)
		if _, ok := w.questRequirementsMet(p, &quest); !ok {
			t.Fatal("com a quest anterior deveria passar")
		}
	})

	t.Run("itens exigidos e consumidos", func(t *testing.T) {
		quest := simpleQuest(1, "X")
		quest.Consumes = []model.QuestItem{{Index: 700, Amount: 3}}
		p := questTestPlayer(1, 0)
		if _, ok := w.questRequirementsMet(p, &quest); ok {
			t.Fatal("sem o item de consumo deveria recusar")
		}
		p.Char.Inv[0] = model.Item{Index: 700, Eff: [6]byte{effectAmount, 3}}
		if _, ok := w.questRequirementsMet(p, &quest); !ok {
			t.Fatal("com o item de consumo deveria passar")
		}
	})

	t.Run("inventario cheio recusa recompensa", func(t *testing.T) {
		quest := simpleQuest(1, "X")
		quest.Rewards.Items = []model.QuestItem{{Index: 55}}
		p := questTestPlayer(1, 0)
		for i := 0; i < model.PlayerCarrySlots; i++ {
			p.Char.Inv[i] = model.Item{Index: 1}
		}
		if _, ok := w.questRequirementsMet(p, &quest); ok {
			t.Fatal("sem espaco no inventario deveria recusar antes de consumir")
		}
	})
}

func TestQuestBlockedMessagePrefereTextoDoAutor(t *testing.T) {
	quest := simpleQuest(1, "X")
	if got := questBlockedMessage(&quest, "motivo padrao"); got != "motivo padrao" {
		t.Fatalf("sem texto do autor deveria usar o motivo: %q", got)
	}
	quest.Messages.Denied = "Volte quando for mais forte."
	if got := questBlockedMessage(&quest, "motivo padrao"); got != quest.Messages.Denied {
		t.Fatalf("texto do autor deveria vencer: %q", got)
	}
}

// --- conclusao e pilhas ----------------------------------------------------

func TestQuestCompletedEMark(t *testing.T) {
	ch := &model.Char{}
	if questCompleted(ch, 3) {
		t.Fatal("quest nao concluida nao pode aparecer como concluida")
	}
	markQuestCompleted(ch, 3)
	markQuestCompleted(ch, 3) // idempotente
	if !questCompleted(ch, 3) || len(ch.QuestsDone) != 1 {
		t.Fatalf("marcacao incorreta: %v", ch.QuestsDone)
	}
}

func TestQuestRepetivelNaoMarcaConclusao(t *testing.T) {
	w := newZoneTestWorld()
	p := addZonePlayer(w, 1, 2100, 2100, 500)
	p.Char.Extended.Level = 39
	quest := simpleQuest(1, "Gravedigger")
	quest.Repeatable = true
	quest.Requires = model.QuestRequirements{MinLevel: 39, MaxLevel: 115, MortalOnly: true}
	quest.Rewards.Teleport = &model.QuestTeleport{X: 2398, Y: 2105}

	w.executeQuest(p.Session, p, &Mob{}, &quest)
	if len(p.Char.QuestsDone) != 0 {
		t.Fatalf("portal repetivel nao deve sujar QuestsDone: %v", p.Char.QuestsDone)
	}
	if p.X != 2398 || p.Y != 2105 {
		t.Fatalf("portal nao teleportou: (%d,%d)", p.X, p.Y)
	}
}

func TestContagemEConsumoRespeitamPilhaDeEfAmount(t *testing.T) {
	ch := &model.Char{}
	// duas pilhas do mesmo item: 5 e 3 unidades
	ch.Inv[0] = model.Item{Index: 100, Eff: [6]byte{effectAmount, 5}}
	ch.Inv[1] = model.Item{Index: 100, Eff: [6]byte{effectAmount, 3}}
	// e uma unidade avulsa, sem EF_AMOUNT
	ch.Inv[2] = model.Item{Index: 100}

	if got := countInventoryItem(ch, 100); got != 9 {
		t.Fatalf("contagem deveria somar pilhas e avulso: %d", got)
	}
	if !consumeInventoryItem(ch, 100, 7) {
		t.Fatal("consumo de 7 deveria caber em 9")
	}
	if got := countInventoryItem(ch, 100); got != 2 {
		t.Fatalf("apos consumir 7 de 9 deveria sobrar 2: %d", got)
	}
	if consumeInventoryItem(ch, 100, 5) {
		t.Fatal("consumo acima do disponivel deveria falhar")
	}
}

func TestConsumoInsuficienteNaoAlteraInventario(t *testing.T) {
	ch := &model.Char{}
	ch.Inv[0] = model.Item{Index: 100, Eff: [6]byte{effectAmount, 2}}
	before := ch.Inv
	if consumeInventoryItem(ch, 100, 5) {
		t.Fatal("consumo maior que o estoque deveria falhar")
	}
	if ch.Inv != before {
		t.Fatal("falha de consumo nao pode mexer no inventario")
	}
}

func TestGrantInventoryItemUsaOPrimeiroSlotLivre(t *testing.T) {
	ch := &model.Char{}
	ch.Inv[0] = model.Item{Index: 1}
	if !grantInventoryItem(ch, model.QuestItem{Index: 55, Amount: 4}) {
		t.Fatal("deveria caber")
	}
	if ch.Inv[1].Index != 55 || ch.Inv[1].Eff[0] != effectAmount || ch.Inv[1].Eff[1] != 4 {
		t.Fatalf("recompensa gravada incorretamente: %+v", ch.Inv[1])
	}
}

func TestGrantInventoryItemFalhaComInventarioCheio(t *testing.T) {
	ch := &model.Char{}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		ch.Inv[i] = model.Item{Index: 1}
	}
	if grantInventoryItem(ch, model.QuestItem{Index: 55}) {
		t.Fatal("inventario cheio deveria recusar a recompensa")
	}
}

// --- interacao com NPC -----------------------------------------------------

func TestResolveNPCInteractionValidaAlcanceEVisibilidade(t *testing.T) {
	def := questNPC("Gravedigger", 43)
	mob := &Mob{ID: 1000, X: 2200, Y: 2200, Def: &def}
	w := testSpatialWorld([]*Mob{mob})

	p := &Player{ID: 1, X: 2200, Y: 2200, Char: &model.Char{Name: "A"},
		Visible: map[uint16]struct{}{1000: {}}}
	if _, err := w.resolveNPCInteraction(p, 1000); err != nil {
		t.Fatalf("npc visivel e adjacente deveria passar: %v", err)
	}

	// Fora de alcance: a loja tambem passa por aqui, entao isso fecha o buraco
	// de abrir loja de longe.
	far := &Player{ID: 2, X: 2200 + npcInteractionRange + 1, Y: 2200,
		Char: &model.Char{Name: "B"}, Visible: map[uint16]struct{}{1000: {}}}
	if _, err := w.resolveNPCInteraction(far, 1000); err == nil {
		t.Fatal("npc fora do alcance deveria ser recusado")
	}

	blind := &Player{ID: 3, X: 2200, Y: 2200, Char: &model.Char{Name: "C"},
		Visible: map[uint16]struct{}{}}
	if _, err := w.resolveNPCInteraction(blind, 1000); err == nil {
		t.Fatal("npc nao materializado para o jogador deveria ser recusado")
	}

	if _, err := w.resolveNPCInteraction(p, 9999); err == nil {
		t.Fatal("npc inexistente deveria ser recusado")
	}
}

// --- lista de loja -----------------------------------------------------------
//
// O client 7.48 percorre `for (i = 0; i < 27; ++i)` no OnPacketShopList e monta
// a grade com (i%5, i/5). Item alem da 27a posicao do PACOTE nunca e desenhado.
// Como os NPCs do Micronics espalham o estoque pelas 64 posicoes originais, sem
// compactar a maior parte da loja fica invisivel.

func shopItems(indexes ...uint16) []model.Item {
	out := make([]model.Item, len(indexes))
	for i, index := range indexes {
		out[i] = model.Item{Index: index}
	}
	return out
}

func TestShopDisplayListCompactaBuracos(t *testing.T) {
	// Padrao real do Aki: itens em 1..7, buraco longo, e mais itens la no fim.
	vende := shopItems(0, 685, 691, 415, 0, 411, 697, 787)
	for len(vende) < 27 {
		vende = append(vende, model.Item{})
	}
	vende = append(vende, shopItems(4038, 4039, 4040, 0, 700)...)

	got := shopDisplayList(vende, wire.ShopNormal)
	want := []uint16{685, 691, 415, 411, 697, 787, 4038, 4039, 4040, 700}
	if len(got) != len(want) {
		t.Fatalf("esperava %d itens compactados, veio %d", len(want), len(got))
	}
	for i, index := range want {
		if got[i].Index != index {
			t.Fatalf("posicao %d: veio %d, quer %d", i, got[i].Index, index)
		}
	}
}

func TestShopDisplayListRespeitaOLimiteDoClient(t *testing.T) {
	vende := make([]model.Item, 0, 40)
	for i := 0; i < 40; i++ {
		vende = append(vende, model.Item{Index: uint16(100 + i)})
	}
	got := shopDisplayList(vende, wire.ShopNormal)
	if len(got) != clientShopSlots {
		t.Fatalf("lista deveria parar em %d, veio %d", clientShopSlots, len(got))
	}
}

// Na loja de skill os vazios em 8, 17 e 26 separam os tres ramos: compactar
// embaralharia a grade do mestre.
func TestShopDisplayListNaoCompactaLojaDeSkill(t *testing.T) {
	vende := shopItems(5000, 5001, 0, 5002)
	got := shopDisplayList(vende, wire.ShopSkill)
	if len(got) != len(vende) || got[2].Index != 0 {
		t.Fatalf("loja de skill deveria ser preservada intacta: %+v", got)
	}
}

func TestCountShopItemsIgnoraBuracos(t *testing.T) {
	if got := countShopItems(shopItems(0, 5, 0, 7)); got != 2 {
		t.Fatalf("contagem deveria ignorar vazios: %d", got)
	}
}

// O client remapeia a posicao antes de mandar no 0x379 (SGrid.cpp, BuyItem):
//
//	SourPos = cellX + 5*cellY            // indice da lista exibida
//	SourPos = SourPos % 9 + 27 * (SourPos / 9)
//
// Os valores 27, 28, 29, 30, 32, 34 e 55 sao exatamente os que apareceram no
// log como "compra invalida" -- eles vem dos indices 9..19. Os indices 0..8
// mapeiam para si mesmos, o que fazia as primeiras compras funcionarem e
// escondia o problema.
func TestShopSlotFromClientInverteORemapeamentoNativo(t *testing.T) {
	// forward: como o client calcula, para cada indice de exibicao
	forward := func(i int) uint16 { return uint16(i%9 + 27*(i/9)) }

	for i := 0; i < 27; i++ {
		got, ok := shopSlotFromClient(forward(i))
		if !ok || got != i {
			t.Fatalf("indice %d -> client %d -> voltou %d (ok=%v)", i, forward(i), got, ok)
		}
	}

	// casos reais colhidos do log
	for sourPos, want := range map[uint16]int{
		27: 9, 28: 10, 29: 11, 30: 12, 32: 14, 34: 16, 55: 19,
	} {
		got, ok := shopSlotFromClient(sourPos)
		if !ok || got != want {
			t.Fatalf("slot %d deveria virar indice %d, veio %d (ok=%v)", sourPos, want, got, ok)
		}
	}
}

// A regiao entre os blocos (9..26, 36..53) nao corresponde a celula nenhuma.
func TestShopSlotFromClientRecusaRegiaoMorta(t *testing.T) {
	for _, sourPos := range []uint16{9, 15, 26, 36, 44, 53} {
		if _, ok := shopSlotFromClient(sourPos); ok {
			t.Fatalf("slot %d esta na regiao morta e deveria ser recusado", sourPos)
		}
	}
}

// --- cidadania ---------------------------------------------------------------

// A taxa nativa e invertida (rand()%rate == 0 dropa), entao bonus precisa
// DIVIDIR o rate. Somar deixaria o drop mais raro -- o oposto do pretendido.
func TestApplyDropBonusDivideATaxa(t *testing.T) {
	if got := applyDropBonus(100, 0); got != 100 {
		t.Fatalf("sem bonus a taxa nao muda: %d", got)
	}
	if got := applyDropBonus(110, 10); got != 100 {
		t.Fatalf("+10%% em 110 deveria dar 100, deu %d", got)
	}
	if got := applyDropBonus(180, 80); got != 100 {
		t.Fatalf("+80%% em 180 deveria dar 100, deu %d", got)
	}
	// Taxa nunca pode zerar (rand.Intn(0) entra em panico).
	if got := applyDropBonus(2, 1000); got < 1 {
		t.Fatalf("taxa ajustada precisa ser >= 1, deu %d", got)
	}
	// Drop garantido (rate <= 1) nao e afetado.
	if got := applyDropBonus(1, 50); got != 1 {
		t.Fatalf("drop garantido nao deveria mudar: %d", got)
	}
}

func TestDropBonusForUsaCidadaniaDoCanal(t *testing.T) {
	w := &World{channel: 1}
	cidadao := &Player{Char: &model.Char{Citizenship: 1}}
	if got := w.dropBonusFor(cidadao); got != model.CitizenChannelDropBonus {
		t.Fatalf("cidadao do canal deveria ganhar %d, ganhou %d",
			model.CitizenChannelDropBonus, got)
	}
	outroCanal := &Player{Char: &model.Char{Citizenship: 2}}
	if got := w.dropBonusFor(outroCanal); got != 0 {
		t.Fatalf("cidadao de outro canal nao ganha o bonus de canal: %d", got)
	}
	if got := w.dropBonusFor(&Player{}); got != 0 {
		t.Fatalf("jogador sem char nao pode quebrar nem bonificar: %d", got)
	}
}
