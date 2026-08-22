package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// enterWorldPacket monta o 0x213 com o slot pedido.
func enterWorldPacket(slot int) []byte {
	pkt := make([]byte, characterLoginPacketSize)
	binary.LittleEndian.PutUint16(pkt[0:2], characterLoginPacketSize)
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpCharacterLogin)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(slot))
	return pkt
}

func TestEnterWorldFailsClosedWhenCharStateLoadFails(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	w.store = &charStateMemoryStore{loadErr: errors.New("postgres unavailable")}
	w.onEnterWorld(s, enterWorldPacket(0))
	if p.InWorld || p.Char != nil || p.CharSlot != -1 || p.ID != 0 {
		t.Fatalf("personagem entrou sem estado autoritativo: %+v", p)
	}
	if s.QueuedPacketsForTest() != 1 {
		t.Fatalf("falha deveria enviar somente o aviso, pacotes=%d", s.QueuedPacketsForTest())
	}
}

func newEnterWorldPlayer(t *testing.T, curHP, maxHP uint32) (*World, *Player, *net.Session) {
	t.Helper()
	w := newZoneTestWorld()
	w.store = &craftStore{}
	w.questZones = nil
	w.gameplay = model.DefaultGameplayConfig()

	session := net.NewTestSession(1, 256)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{
		Name:  "Morto",
		Class: 0,
		Score: &model.Score{
			Version: model.ScoreVersion,
			Level:   50, MaxHP: maxHP, CurHP: curHP, MaxMP: 100, CurMP: 100,
			Str: 10, Int: 10, Dex: 10, Con: 10,
		},
	}}}
	p := &Player{Session: session, Account: acc, CharSlot: -1, Visible: map[uint16]struct{}{}}
	w.players[session] = p
	return w, p, session
}

// TestDeadCharacterEntersWithOneHP protege uma regressão que já apareceu
// in-game: quem morre e sai tem CurHP=0 persistido, e entrar assim TRAVA o
// jogador -- o client desenha a pose de morte e não responde a nada, nem ao
// /restart.
func TestDeadCharacterEntersWithOneHP(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 0, 1000)

	w.onEnterWorld(s, enterWorldPacket(0))

	if !p.InWorld {
		t.Fatal("o personagem deveria ter entrado no mundo")
	}
	if got := playerCurHP(p.Char); got != 1 {
		t.Fatalf("HP na entrada=%d, quer 1", got)
	}
	if !p.DeadAt.IsZero() {
		t.Error("DeadAt deveria ser limpo ao reviver na entrada")
	}
}

// TestLiveCharacterKeepsHPOnEnter garante que a correção não mexe em quem
// entrou vivo.
func TestLiveCharacterKeepsHPOnEnter(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)

	w.onEnterWorld(s, enterWorldPacket(0))

	if got := playerCurHP(p.Char); got != 742 {
		t.Fatalf("HP na entrada=%d, quer preservar 742", got)
	}
}

func TestLoadtestAccountUsesConfiguredNoatumSpawn(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	p.Account.Name = "bot0001"
	w.loadtestAccountPrefix = "bot"
	w.loadtestSpawn = model.CharacterSpawn{X: 1162, Y: 1700}

	w.onEnterWorld(s, enterWorldPacket(0))

	if p.X != 1162 || p.Y != 1700 || p.Char.X != 1162 || p.Char.Y != 1700 {
		t.Fatalf("spawn loadtest=(%d,%d) char=(%d,%d)", p.X, p.Y, p.Char.X, p.Char.Y)
	}
}

func TestOnlyProvisionedLoadtestRangeUsesNoatumSpawn(t *testing.T) {
	for _, tc := range []struct {
		name string
		want bool
	}{
		{"bot0001", true}, {"bot0990", true}, {"bot0991", false},
		{"bot9999", false}, {"botabcd", false}, {"felipe", false},
	} {
		if got := isLoadtestAccountName(tc.name, "bot"); got != tc.want {
			t.Errorf("%s: isLoadtest=%v, want %v", tc.name, got, tc.want)
		}
	}
}

func TestEnterWorldRejectsWhenAllPlayerIDsAreOccupied(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	for id := uint16(1); id < 1000; id++ {
		ownerSession := net.NewTestSession(int64(1000+id), 1)
		owner := &Player{Session: ownerSession, ID: id, InWorld: true,
			X: playerEntryX, Y: playerEntryY}
		w.players[ownerSession] = owner
		w.playersByID[id] = owner
	}
	w.onEnterWorld(s, enterWorldPacket(0))
	if p.InWorld || p.ID != 0 || w.playersByID[999] == nil {
		t.Fatal("entrada deveria ser recusada sem sobrescrever o ultimo jogador")
	}
	if s.QueuedPacketsForTest() == 0 {
		t.Fatal("recusa de mundo cheio deveria avisar o cliente")
	}
}

// TestDeadCharacterRevivedBeforePackets: o revive precisa acontecer ANTES dos
// pacotes de entrada, senão o client recebe HP 0 no EnterWorld/CreateMob e
// desenha a pose de morte mesmo com o servidor já corrigido.
func TestDeadCharacterRevivedBeforePackets(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 0, 1000)
	// DeadAt sujo de uma morte anterior na mesma sessão.
	p.DeadAt = time.Now()

	w.onEnterWorld(s, enterWorldPacket(0))

	if s.QueuedPacketsForTest() == 0 {
		t.Fatal("a sequência de entrada deveria ter sido enviada")
	}
	// O HP autoritativo já está corrigido no momento em que os pacotes saíram.
	if got := playerCurHP(p.Char); got != 1 {
		t.Fatalf("HP=%d ao montar os pacotes, quer 1", got)
	}
}
