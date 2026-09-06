package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/wire"
)

func motionIntent(motion, parm uint16) []byte {
	pkt := inboundPacket(wire.OpMotion, 20)
	binary.LittleEndian.PutUint16(pkt[6:8], 999) // ID declarado nao e autoridade.
	binary.LittleEndian.PutUint16(pkt[12:14], motion)
	binary.LittleEndian.PutUint16(pkt[14:16], parm)
	binary.LittleEndian.PutUint32(pkt[16:20], 0x7FC00000) // NaN deve ser descartado.
	return pkt
}

func TestMotionRoundtripPublishesAuthoritativePlayerEmote(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Emoter", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	outsider, _ := networkedTestPlayer(3, "Outsider", 2200, 2200)
	observer.show(owner.ID)
	w := worldWithNetworkedPlayers(owner, observer, outsider)

	w.onMotion(owner.Session, motionIntent(21, 0))

	if got := owner.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("emissor recebeu %d retornos, quer 1", got)
	}
	if got := observer.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("observer recebeu %d motions, quer 1", got)
	}
	if got := outsider.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("jogador fora da visao recebeu %d motions", got)
	}
}

func TestMotionRejectsClientOwnedEffects(t *testing.T) {
	for _, tc := range []struct {
		name   string
		motion uint16
		parm   uint16
	}{
		{name: "motion de sistema", motion: 100},
		{name: "motion server-side", motion: 14},
		{name: "efeito de personagem", motion: 21, parm: 1},
		{name: "limpa morte", motion: 21, parm: 2},
		{name: "level up", motion: 21, parm: 3},
	} {
		t.Run(tc.name, func(t *testing.T) {
			owner, _ := networkedTestPlayer(1, "Emoter", 2100, 2100)
			observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
			observer.show(owner.ID)
			w := worldWithNetworkedPlayers(owner, observer)

			w.onMotion(owner.Session, motionIntent(tc.motion, tc.parm))

			if owner.Session.QueuedPacketsForTest() != 0 ||
				observer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("efeito controlado pelo client foi publicado")
			}
		})
	}
}
