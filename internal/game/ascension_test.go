package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func newAscensionWorld(t *testing.T) (*World, *Player, *craftStore) {
	t.Helper()
	w := newZoneTestWorld()
	st := &craftStore{}
	w.store = st
	w.questZones = nil
	w.gameplay = model.DefaultGameplayConfig()

	// Templates das quatro classes: a ascensao monta o Arch a partir deles.
	for class := 0; class < 4; class++ {
		w.charTemplates[class] = model.CharacterTemplate{
			Name: "classe", Class: byte(class),
			ExtendedScore: model.ExtendedScore{
				Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100,
			},
		}
	}

	session := net.NewTestSession(1, 128)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{
		Name:     "Mortal",
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1000, CurHP: 1000},
	}}}
	p := &Player{ID: 1, Session: session, Account: acc, Char: &acc.Chars[0],
		InWorld: true, X: 2100, Y: 2100, Visible: map[uint16]struct{}{}}
	w.players[session] = p
	w.playersByID[1] = p
	return w, p, st
}

func giveSephiraStones(ch *model.Char) {
	for offset := 0; offset < sephiraStoneCount; offset++ {
		ch.Inv[offset] = model.Item{Index: uint16(sephiraFirstStone + offset)}
	}
}

// --- Sefirot -----------------------------------------------------------------

func TestSefirotRequiresAllEightStones(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	giveSephiraStones(p.Char)
	p.Char.Inv[3] = model.Item{} // falta uma
	p.Char.Gold = sefirotGoldCost

	w.craftSefirot(p.Session, p, 0)

	if st.saves != 0 {
		t.Fatal("nao deveria persistir sem as oito pedras")
	}
	if p.Char.Gold != sefirotGoldCost {
		t.Errorf("gold consumido indevidamente: %d", p.Char.Gold)
	}
}

func TestSefirotRequiresGold(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	giveSephiraStones(p.Char)
	p.Char.Gold = sefirotGoldCost - 1

	w.craftSefirot(p.Session, p, 0)

	if st.saves != 0 {
		t.Fatal("nao deveria persistir sem gold suficiente")
	}
	if p.Char.Inv[0].Index != sephiraFirstStone {
		t.Error("as pedras nao deveriam ser consumidas")
	}
}

func TestSefirotConsumesStonesAndGold(t *testing.T) {
	for class := 0; class < 4; class++ {
		w, p, _ := newAscensionWorld(t)
		giveSephiraStones(p.Char)
		p.Char.Gold = sefirotGoldCost + 500

		w.craftSefirot(p.Session, p, class)

		if p.Char.Gold != 500 {
			t.Errorf("classe %d: gold=%d, quer 500", class, p.Char.Gold)
		}
		wanted := uint16(sefirotFirstItem + class)
		if !inventoryHasItem(p.Char, wanted) {
			t.Errorf("classe %d: Sefirot %d nao foi entregue", class, wanted)
		}
		for offset := 0; offset < sephiraStoneCount; offset++ {
			if inventoryHasItem(p.Char, uint16(sephiraFirstStone+offset)) {
				t.Errorf("classe %d: pedra %d sobrou", class, sephiraFirstStone+offset)
			}
		}
	}
}

func TestSefirotRollbackOnSaveFailure(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	giveSephiraStones(p.Char)
	p.Char.Gold = sefirotGoldCost
	st.err = errors.New("disco indisponivel")
	before := cloneCharacterState(p.Char)

	w.craftSefirot(p.Session, p, 0)

	if p.Char.Gold != before.Gold {
		t.Errorf("gold=%d, quer %d apos falha de save", p.Char.Gold, before.Gold)
	}
	if !inventoryHasItem(p.Char, sephiraFirstStone) {
		t.Error("as pedras deveriam voltar apos falha de save")
	}
	if inventoryHasItem(p.Char, sefirotFirstItem) {
		t.Error("o Sefirot nao pode ficar apos falha de save")
	}
}

// --- Pedra Eterna ------------------------------------------------------------

// TestEternalStoneRequiresAdjacentSpirits protege a exigencia nativa: os dois
// espiritos precisam estar lado a lado, nessa ordem.
func TestEternalStoneRequiresAdjacentSpirits(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	p.Char.Inv[0] = model.Item{Index: unicornSpiritItem}
	p.Char.Inv[5] = model.Item{Index: phoenixSpiritItem} // separados
	p.Char.Inv[10] = model.Item{Index: model.SapphirePack}

	w.craftEternalStone(p.Session, p)

	if st.saves != 0 {
		t.Fatal("espiritos separados nao deveriam forjar a pedra")
	}

	// Invertidos tambem nao servem: a ordem e Unicon depois Phoenix.
	p.Char.Inv[0] = model.Item{Index: phoenixSpiritItem}
	p.Char.Inv[1] = model.Item{Index: unicornSpiritItem}
	w.craftEternalStone(p.Session, p)
	if st.saves != 0 {
		t.Fatal("ordem invertida nao deveria forjar a pedra")
	}
}

func TestEternalStoneRequiresTenSapphires(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	p.Char.Inv[0] = model.Item{Index: unicornSpiritItem}
	p.Char.Inv[1] = model.Item{Index: phoenixSpiritItem}
	for slot := 2; slot < 11; slot++ { // 9 safiras: uma a menos
		p.Char.Inv[slot] = model.Item{Index: model.SapphireItem}
	}

	w.craftEternalStone(p.Session, p)

	if st.saves != 0 {
		t.Fatal("nove safiras nao deveriam bastar")
	}
	if !inventoryHasItem(p.Char, unicornSpiritItem) {
		t.Error("os espiritos nao deveriam ser consumidos")
	}
}

func TestEternalStoneConsumesEverything(t *testing.T) {
	w, p, _ := newAscensionWorld(t)
	p.Char.Inv[0] = model.Item{Index: unicornSpiritItem}
	p.Char.Inv[1] = model.Item{Index: phoenixSpiritItem}
	p.Char.Inv[2] = model.Item{Index: model.SapphirePack} // vale dez

	w.craftEternalStone(p.Session, p)

	if !inventoryHasItem(p.Char, eternalStoneItem) {
		t.Fatal("a Pedra Eterna nao foi entregue")
	}
	if inventoryHasItem(p.Char, unicornSpiritItem) || inventoryHasItem(p.Char, phoenixSpiritItem) {
		t.Error("os espiritos deveriam ser consumidos")
	}
	if inventoryHasItem(p.Char, model.SapphirePack) {
		t.Error("o pacote de safiras deveria ser consumido")
	}
}

// --- Arch --------------------------------------------------------------------

func prepareArchCandidate(p *Player, class int) {
	p.Char.Extended.Level = archMinLevel
	p.Char.Equip[0] = model.Item{Index: uint16(1 + 10*class)} // rosto Mortal
	p.Char.Equip[eternalStoneSlot] = model.Item{Index: eternalStoneItem}
	p.Char.Equip[sefirotSlot] = model.Item{Index: uint16(sefirotFirstItem + class)}
}

// TestArchIgnoredWithoutItems garante que o clique no rei sem os itens continua
// sendo pedido de CAPA, e nao de ascensao.
func TestArchIgnoredWithoutItems(t *testing.T) {
	w, p, _ := newAscensionWorld(t)
	p.Char.Extended.Level = archMinLevel

	if w.createArch(p.Session, p) {
		t.Fatal("sem os itens equipados o rei nao deveria tratar como ascensao")
	}
}

func TestArchRequiresLevel(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	prepareArchCandidate(p, 0)
	p.Char.Extended.Level = archMinLevel - 1

	if !w.createArch(p.Session, p) {
		t.Fatal("com os itens equipados o rei deveria tratar como ascensao")
	}
	if st.saves != 0 {
		t.Error("nao deveria persistir abaixo do nivel exigido")
	}
	if len(p.Account.Chars) != 1 {
		t.Errorf("nenhum personagem deveria ser criado: %d", len(p.Account.Chars))
	}
}

func TestArchRequiresMortal(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	prepareArchCandidate(p, 0)
	p.Char.Evolution = archEvolution // ja e Arch

	w.createArch(p.Session, p)

	if st.saves != 0 {
		t.Error("um Arch nao pode ascender de novo")
	}
}

func TestArchRequiresFreeSlot(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	prepareArchCandidate(p, 0)
	for len(p.Account.Chars) < maxCharactersPerAccount {
		p.Account.Chars = append(p.Account.Chars, model.Char{Name: "ocupado"})
	}
	p.Char = &p.Account.Chars[0]
	prepareArchCandidate(p, 0)

	w.createArch(p.Session, p)

	if st.saves != 0 {
		t.Error("conta cheia nao deveria criar o Arch")
	}
}

// TestArchCreatedKeepsMortalAndName cobre as duas decisoes centrais: o Mortal
// PERMANECE e o Arch herda o nome dele (fiel ao nativo).
func TestArchCreatedKeepsMortalAndName(t *testing.T) {
	w, p, _ := newAscensionWorld(t)
	prepareArchCandidate(p, 1) // Foema
	mortalName := p.Char.Name

	if !w.createArch(p.Session, p) {
		t.Fatal("a ascensao deveria ser tratada")
	}

	if p.Account.Chars[0].Name != mortalName || p.Account.Chars[0].Evolution != "" {
		t.Errorf("o Mortal deveria permanecer intacto: %+v", p.Account.Chars[0])
	}
	if len(p.Account.Chars) < 2 {
		t.Fatalf("o Arch nao foi criado: %d personagens", len(p.Account.Chars))
	}
	arch := p.Account.Chars[1]
	if arch.Name != mortalName {
		t.Errorf("nome do Arch=%q, quer %q (mesmo do Mortal)", arch.Name, mortalName)
	}
	if arch.Evolution != archEvolution {
		t.Errorf("evolucao=%q, quer %q", arch.Evolution, archEvolution)
	}
	// Rosto: Mortal Foema 11 + 5 + classe 1 = 17.
	if arch.Equip[0].Index != 17 {
		t.Errorf("rosto do Arch=%d, quer 17", arch.Equip[0].Index)
	}
	// Os itens da ascensao sao consumidos.
	if p.Char.Equip[eternalStoneSlot].Index != 0 || p.Char.Equip[sefirotSlot].Index != 0 {
		t.Error("Pedra e Sefirot deveriam ser consumidos")
	}
}

// TestArchFaceMatchesClassBlock confirma a formula em todas as classes: cada
// rosto resultante pertence ao bloco de dez da propria classe.
func TestArchFaceMatchesClassBlock(t *testing.T) {
	want := []uint16{6, 17, 28, 39}
	for class := 0; class < 4; class++ {
		w, p, _ := newAscensionWorld(t)
		prepareArchCandidate(p, class)

		w.createArch(p.Session, p)

		arch := p.Account.Chars[1]
		if arch.Equip[0].Index != want[class] {
			t.Errorf("classe %d: rosto=%d, quer %d", class, arch.Equip[0].Index, want[class])
		}
	}
}

func TestArchRollbackOnSaveFailure(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	prepareArchCandidate(p, 0)
	st.err = errors.New("disco indisponivel")

	w.createArch(p.Session, p)

	if len(p.Account.Chars) != 1 {
		t.Errorf("o Arch nao pode sobreviver a falha de save: %d personagens", len(p.Account.Chars))
	}
	if p.Char.Equip[eternalStoneSlot].Index != eternalStoneItem {
		t.Error("a Pedra deveria voltar apos falha de save")
	}
	if p.Char.Equip[sefirotSlot].Index != sefirotFirstItem {
		t.Error("o Sefirot deveria voltar apos falha de save")
	}
}

// inventoryHasItem e um auxiliar de leitura para os testes.
func inventoryHasItem(ch *model.Char, index uint16) bool {
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		if ch.Inv[slot].Index == index {
			return true
		}
	}
	return false
}

// TestSefirotComposerIsTheClassSkillMaster trava a identificacao dos NPCs. O
// compositor do Sefirot e o mestre de skill da classe -- a primeira versao usou
// os NPCs Sephirot_*, que so batiam no nome, e a composicao nunca acontecia.
//
// A classe de cada mestre vem da faixa de skills que ele vende (5000+skill),
// conferida em data/npcs/*.json.
func TestSefirotComposerIsTheClassSkillMaster(t *testing.T) {
	for _, tc := range []struct {
		npc   string
		class int
	}{
		{"Knight_Leader", 0}, // skills 0..23   TK
		{"Foema_Ancian", 1},  // skills 24..47  FM
		{"Mestre_Archi", 2},  // skills 48..71  BM
		{"ForeLearner", 3},   // skills 72..95  HT
	} {
		got, ok := sefirotClassForNPC(&model.NPCDef{Name: tc.npc})
		if !ok {
			t.Errorf("%s deveria ser reconhecido como compositor", tc.npc)
			continue
		}
		if got != tc.class {
			t.Errorf("%s -> classe %d, quer %d", tc.npc, got, tc.class)
		}
	}
	// Os NPCs Sephirot_* NAO sao os compositores.
	for _, name := range []string{"Sephirot_TK", "Sephirot_FM", "LojaTeste"} {
		if _, ok := sefirotClassForNPC(&model.NPCDef{Name: name}); ok {
			t.Errorf("%s nao deveria ser tratado como compositor", name)
		}
	}
}

// TestSephiraStoneGateProtectsSkillShop: sem as oito pedras o clique no mestre
// continua sendo pedido de loja, e nao tentativa de composicao.
func TestSephiraStoneGateProtectsSkillShop(t *testing.T) {
	_, p, _ := newAscensionWorld(t)
	if hasSephiraStones(p.Char) {
		t.Fatal("inventario vazio nao deveria passar no gate")
	}
	giveSephiraStones(p.Char)
	if !hasSephiraStones(p.Char) {
		t.Fatal("com as oito pedras o gate deveria abrir")
	}
	// Faltando uma, o gate fecha de novo.
	p.Char.Inv[4] = model.Item{}
	if hasSephiraStones(p.Char) {
		t.Fatal("faltando uma pedra o gate deveria fechar")
	}
}

// TestArchCreationAnnouncesToWorld cobre o aviso global. O ponto delicado e o
// MOMENTO: o anuncio sai depois da persistencia, para uma ascensao que falhou
// nao virar noticia.
func TestArchCreationAnnouncesToWorld(t *testing.T) {
	w, p, _ := newAscensionWorld(t)
	prepareArchCandidate(p, 0)

	// Um segundo jogador, para provar que o aviso e do MUNDO e nao so de quem
	// ascendeu.
	outro := addZonePlayer(w, 9, 2100, 2100, 500)
	antes := outro.Session.QueuedPacketsForTest()

	w.createArch(p.Session, p)

	if depois := outro.Session.QueuedPacketsForTest(); depois <= antes {
		t.Fatalf("os demais jogadores nao receberam o aviso (%d -> %d)", antes, depois)
	}
}

func TestFailedAscensionDoesNotAnnounce(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	prepareArchCandidate(p, 0)
	st.err = errors.New("disco indisponivel")

	outro := addZonePlayer(w, 9, 2100, 2100, 500)
	antes := outro.Session.QueuedPacketsForTest()

	w.createArch(p.Session, p)

	if depois := outro.Session.QueuedPacketsForTest(); depois != antes {
		t.Fatalf("ascensao que falhou nao pode anunciar (%d -> %d)", antes, depois)
	}
}
