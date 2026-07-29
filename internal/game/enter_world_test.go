package game

import (
	"encoding/binary"
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
		Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion,
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
