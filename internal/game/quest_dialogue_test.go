package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/net"
)

// A fala do NPC sai como 0x333 com o ID DELE, entao o client mostra como fala
// do NPC e quem esta por perto tambem le -- e o mesmo efeito do SendSay nativo.

// TestTodaFalaCabeNoPacote e a guarda que importa: o 0x333 carrega String[96] e
// o builder copia 95 bytes. Fala maior nao da erro, ela chega CORTADA no meio
// da frase, e isso so apareceria lendo o chat in-game.
func TestTodaFalaCabeNoPacote(t *testing.T) {
	quests, err := data.LoadQuests(filepath.Join("..", "..", "data", "quests.json"))
	if err != nil {
		t.Fatal(err)
	}
	comFala := 0
	for _, quest := range quests.Quests {
		if len(quest.Dialogue) > 0 {
			comFala++
		}
		for i, linha := range quest.Dialogue {
			if len(linha) > model.MaxQuestLine {
				t.Errorf("quest %d (%s) fala %d tem %d caracteres, o teto e %d: %q",
					quest.ID, quest.NPC, i, len(linha), model.MaxQuestLine, linha)
			}
			// Uma fala so nao e "mais de uma frase", que foi o pedido.
			if linha == "" {
				t.Errorf("quest %d fala %d vazia", quest.ID, i)
			}
		}
		if n := len(quest.Dialogue); n == 1 {
			t.Errorf("quest %d (%s) tem so uma fala; o NPC deve ter variedade",
				quest.ID, quest.NPC)
		}
	}
	if comFala != len(quests.Quests) {
		t.Errorf("%d de %d quests tem fala; todas deveriam ter", comFala, len(quests.Quests))
	}
}

// TestFalaChegaAQuemEstaPerto: a fala e publica, como no nativo. Quem esta ao
// alcance ouve, mesmo sem ter clicado no NPC.
func TestFalaChegaAQuemEstaPerto(t *testing.T) {
	w := newZoneTestWorld()
	w.players = map[*net.Session]*Player{}
	mob := &Mob{ID: 2000, X: 2100, Y: 2100, Def: &model.NPCDef{Name: "Gravedigger"}}

	perto := net.NewTestSession(1, 64)
	longe := net.NewTestSession(2, 64)
	for _, c := range []struct {
		s    *net.Session
		x, y uint16
	}{{perto, 2100, 2100}, {longe, 2100 + viewHalfX + 20, 2100}} {
		id := uint16(c.s.ID)
		acc := &model.Account{Name: "c", Chars: []model.Char{{Name: "n",
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 10, CurHP: 10}}}}
		p := &Player{ID: id, Session: c.s, Account: acc, Char: &acc.Chars[0],
			InWorld: true, X: c.x, Y: c.y}
		w.players[c.s] = p
		w.updatePlayerSpatial(p)
	}

	quest := &model.QuestDef{ID: 1, NPC: "Gravedigger", Dialogue: []string{"The dead do not rest here."}}
	w.speakQuestNPC(w.players[perto], mob, quest)

	if perto.QueuedPacketsForTest() == 0 {
		t.Error("quem estava ao lado do NPC nao ouviu a fala")
	}
	if longe.QueuedPacketsForTest() != 0 {
		t.Error("a fala vazou para quem estava fora do alcance")
	}
}

// TestNPCSemFalaNaoQuebra: dialogue e opcional.
func TestNPCSemFalaNaoQuebra(t *testing.T) {
	w := newZoneTestWorld()
	w.players = map[*net.Session]*Player{}
	mob := &Mob{ID: 2000, X: 2100, Y: 2100, Def: &model.NPCDef{Name: "X"}}
	w.speakQuestNPC(nil, mob, &model.QuestDef{ID: 1})
	w.speakQuestNPC(nil, nil, &model.QuestDef{ID: 1, Dialogue: []string{"oi"}})
}

// TestValidacaoRecusaFalaLonga: o boot precisa recusar, nao truncar.
func TestValidacaoRecusaFalaLonga(t *testing.T) {
	longa := ""
	for len(longa) <= model.MaxQuestLine {
		longa += "x"
	}
	q := model.QuestDef{
		ID: 1, NPC: "Gravedigger",
		Messages: model.QuestMessages{Offer: "o", Success: "s"},
		Dialogue: []string{longa},
	}
	if err := q.Validate(); err == nil {
		t.Fatalf("fala de %d caracteres foi aceita; o teto e %d", len(longa), model.MaxQuestLine)
	}
	q.Dialogue = []string{"   "}
	if err := q.Validate(); err == nil {
		t.Fatal("fala em branco foi aceita")
	}
}

// --- recompensa em refinacao (os guardas do Training Camp) -------------------

func charComEquip(slots ...int) *model.Char {
	ch := &model.Char{Name: "R",
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 10, CurHP: 10}}
	for _, s := range slots {
		ch.Equip[s] = model.Item{Index: uint16(100 + s), Eff: [6]byte{43, 0}}
	}
	return ch
}

func TestRefinaSoOSlotPedido(t *testing.T) {
	ch := charComEquip(6, 7)
	if !refineQuestReward(ch, &model.QuestRefine{Slot: 6, Sanc: 7}) {
		t.Fatal("nao refinou a arma")
	}
	if got := itemSanc(ch.Equip[6]); got != 7 {
		t.Errorf("arma ficou +%d, quer +7", got)
	}
	if got := itemSanc(ch.Equip[7]); got != 0 {
		t.Errorf("refinou o slot 7 sem ter sido pedido: +%d", got)
	}
}

func TestRefinaTodoEquipamentoComSlotNegativo(t *testing.T) {
	ch := charComEquip(0, 6, 9)
	if !refineQuestReward(ch, &model.QuestRefine{Slot: -1, Sanc: 6}) {
		t.Fatal("nao refinou nada")
	}
	for _, s := range []int{0, 6, 9} {
		if got := itemSanc(ch.Equip[s]); got != 6 {
			t.Errorf("slot %d ficou +%d, quer +6", s, got)
		}
	}
}

// TestRefinarSemEquipamentoFalha: sem isso a quest gastaria a chave e nao
// entregaria nada -- e ela so pode ser feita uma vez.
func TestRefinarSemEquipamentoFalha(t *testing.T) {
	if refineQuestReward(charComEquip(), &model.QuestRefine{Slot: -1, Sanc: 6}) {
		t.Error("aceitou refinar um personagem sem equipamento")
	}
	if refineQuestReward(charComEquip(7), &model.QuestRefine{Slot: 6, Sanc: 7}) {
		t.Error("aceitou refinar um slot vazio")
	}
}
