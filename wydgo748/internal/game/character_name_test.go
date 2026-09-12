package game

import (
	"encoding/binary"
	"strings"
	"testing"

	"wydgo/internal/account"
	"wydgo/internal/model"
	"wydgo/internal/net"
)

// A ascensao e o UNICO caminho que produz homonimos: o Arch herda o nome do
// Mortal, fiel ao nativo. A criacao normal (0x20F) exige nome globalmente unico.
//
// Consequencia: uma entrada de charNames pode pertencer a DOIS personagens, e
// apagar um deles nao pode liberar o nome enquanto o outro existir.

const deleteTestPassword = "segredo1"

func newDeleteWorld(t *testing.T, chars ...model.Char) (*World, *Player, *net.Session) {
	t.Helper()
	hash, err := account.HashPassword(deleteTestPassword)
	if err != nil {
		t.Fatal(err)
	}
	acc := &model.Account{Name: "felipe", PasswordHash: hash, Chars: chars}
	session := net.NewTestSession(1, 64)
	p := &Player{Session: session, Account: acc, CharSlot: -1}
	w := &World{
		store:     &craftStore{},
		players:   map[*net.Session]*Player{session: p},
		charNames: map[string]struct{}{},
	}
	for _, ch := range chars {
		if ch.Name != "" {
			w.charNames[strings.ToLower(ch.Name)] = struct{}{}
		}
	}
	return w, p, session
}

// deleteCharacterPacket monta os 44 bytes do 0x211: Slot@12, MobName@16 e
// Password@32.
func deleteCharacterPacket(slot int, name, password string) []byte {
	pkt := make([]byte, 44)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(slot))
	copy(pkt[16:32], name)
	copy(pkt[32:44], password)
	return pkt
}

func nameIsTaken(w *World, name string) bool {
	_, taken := w.charNames[strings.ToLower(name)]
	return taken
}

// TestDeletingOneTwinKeepsTheNameReserved: com Mortal e Arch homonimos, apagar
// qualquer um dos dois tem que MANTER o nome reservado -- o gemeo ainda o usa.
// Sem isso, outra conta conseguiria criar um personagem com o mesmo nome.
func TestDeletingOneTwinKeepsTheNameReserved(t *testing.T) {
	tests := []struct {
		name    string
		slot    int
		remains string
	}{
		{"apaga o Arch, sobra o Mortal", 1, "Mortal"},
		{"apaga o Mortal, sobra o Arch", 0, "Arch"},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w, p, session := newDeleteWorld(t,
				model.Char{Name: "Felipe"}, // Mortal
				model.Char{Name: "Felipe"}, // Arch, mesmo nome
			)

			w.onDeleteCharacter(session, deleteCharacterPacket(tt.slot, "Felipe", deleteTestPassword))

			if p.Account.Chars[tt.slot].Name != "" {
				t.Fatalf("o slot %d nao foi apagado", tt.slot)
			}
			if !nameIsTaken(w, "Felipe") {
				t.Errorf("o nome foi liberado, mas o %s homonimo ainda existe", tt.remains)
			}
		})
	}
}

// TestDeletingLastCharacterFreesTheName: sem gemeo, o nome tem que voltar a
// ficar disponivel -- senao a correcao acima vazaria nomes para sempre.
func TestDeletingLastCharacterFreesTheName(t *testing.T) {
	w, _, session := newDeleteWorld(t, model.Char{Name: "Felipe"})

	w.onDeleteCharacter(session, deleteCharacterPacket(0, "Felipe", deleteTestPassword))

	if nameIsTaken(w, "Felipe") {
		t.Error("o nome continuou reservado depois de apagar o unico personagem que o usava")
	}
}

// TestDeletingCharacterKeepsOtherNames: apagar um personagem nao pode mexer no
// nome dos outros da conta.
func TestDeletingCharacterKeepsOtherNames(t *testing.T) {
	w, _, session := newDeleteWorld(t,
		model.Char{Name: "Felipe"},
		model.Char{Name: "Outro"},
	)

	w.onDeleteCharacter(session, deleteCharacterPacket(0, "Felipe", deleteTestPassword))

	if nameIsTaken(w, "Felipe") {
		t.Error("o nome apagado continuou reservado")
	}
	if !nameIsTaken(w, "Outro") {
		t.Error("o nome de outro personagem foi liberado junto")
	}
}

// TestFailedDeletionKeepsTheNameReserved: senha errada nao apaga nada, entao o
// nome nao pode ser liberado.
func TestFailedDeletionKeepsTheNameReserved(t *testing.T) {
	w, p, session := newDeleteWorld(t, model.Char{Name: "Felipe"})

	w.onDeleteCharacter(session, deleteCharacterPacket(0, "Felipe", "senhaerrada"))

	if p.Account.Chars[0].Name != "Felipe" {
		t.Fatal("o personagem foi apagado com a senha errada")
	}
	if !nameIsTaken(w, "Felipe") {
		t.Error("o nome foi liberado numa exclusao que falhou")
	}
}

func TestAccountUsesName(t *testing.T) {
	acc := &model.Account{Chars: []model.Char{
		{Name: "Felipe"}, {}, {Name: "Outro"},
	}}
	if !accountUsesName(acc, "felipe") {
		t.Error("nao encontrou o nome, que difere so na caixa")
	}
	if accountUsesName(acc, "Ninguem") {
		t.Error("encontrou um nome que a conta nao usa")
	}
	if accountUsesName(acc, "") {
		t.Error("slot vazio nao pode casar com nome vazio")
	}
	if accountUsesName(nil, "Felipe") {
		t.Error("conta nil nao pode usar nome nenhum")
	}
}
