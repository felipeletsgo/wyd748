package game

import (
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestPingIsNativeNoResponseKeepalive(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	before := p.Session.QueuedPacketsForTest()

	w.onPing(p.Session, make([]byte, 12))

	if got := p.Session.QueuedPacketsForTest(); got != before {
		t.Fatalf("ping gerou resposta: fila %d -> %d", before, got)
	}
}

func TestPlayerChallengeRequestStoresInviteAndUsesNativePacket(t *testing.T) {
	requester, _ := networkedTestPlayer(1, "Requester", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(requester, target)
	requester.show(target.ID)
	pkt := wire.StandardParm2(wire.OpPlayerChallenge, requester.ID, uint32(target.ID), 2)
	beforeTarget := target.Session.QueuedPacketsForTest()

	w.onPlayerChallenge(requester.Session, pkt)

	if target.ChallengeFrom != requester.ID || target.ChallengeMode != 2 || target.ChallengeUntil.Before(time.Now()) {
		t.Fatalf("convite nao registrado: from=%d mode=%d until=%v", target.ChallengeFrom, target.ChallengeMode, target.ChallengeUntil)
	}
	if got := target.Session.QueuedPacketsForTest(); got != beforeTarget+1 {
		t.Fatalf("convite nao foi enviado ao alvo: %d -> %d", beforeTarget, got)
	}
}

func TestPlayerChallengeRejectsInvalidModeAndForgedIdentity(t *testing.T) {
	requester, _ := networkedTestPlayer(1, "Requester", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(requester, target)
	requester.show(target.ID)

	w.onPlayerChallenge(requester.Session, wire.StandardParm2(wire.OpPlayerChallenge, requester.ID, uint32(target.ID), 5))
	if target.ChallengeFrom != 0 || target.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("aceitacao sem convite foi tratada como solicitacao")
	}
	w.onPlayerChallenge(requester.Session, wire.StandardParm2(wire.OpPlayerChallenge, target.ID, uint32(target.ID), 0))
	if target.ChallengeFrom != 0 || target.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("identidade forjada criou convite")
	}
}

func TestPlayerChallengeAcceptsOnceAndRejectsExpiredInvite(t *testing.T) {
	challenger, _ := networkedTestPlayer(1, "Challenger", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(challenger, target)
	challenger.show(target.ID)
	w.onPlayerChallenge(challenger.Session, wire.StandardParm2(wire.OpPlayerChallenge, challenger.ID, uint32(target.ID), 1))
	challengerBefore := challenger.Session.QueuedPacketsForTest()
	targetBefore := target.Session.QueuedPacketsForTest()

	w.onPlayerChallenge(target.Session, wire.StandardParm2(wire.OpPlayerChallenge, target.ID, uint32(challenger.ID), 4))
	if target.ChallengeFrom != 0 || target.ChallengeMode != 0 || !target.ChallengeUntil.IsZero() {
		t.Fatal("convite nao foi consumido na aceitacao")
	}
	if challenger.Session.QueuedPacketsForTest() != challengerBefore+1 || target.Session.QueuedPacketsForTest() != targetBefore+1 {
		t.Fatal("aceitacao nao notificou os dois jogadores")
	}
	w.onPlayerChallenge(target.Session, wire.StandardParm2(wire.OpPlayerChallenge, target.ID, uint32(challenger.ID), 4))
	if challenger.Session.QueuedPacketsForTest() != challengerBefore+1 || target.Session.QueuedPacketsForTest() != targetBefore+2 {
		t.Fatal("convite aceito novamente")
	}

	target.ChallengeFrom = challenger.ID
	target.ChallengeMode = 0
	target.ChallengeUntil = time.Now().Add(-time.Second)
	before := target.Session.QueuedPacketsForTest()
	w.onPlayerChallenge(target.Session, wire.StandardParm2(wire.OpPlayerChallenge, target.ID, uint32(challenger.ID), 4))
	if target.ChallengeFrom != 0 || !target.ChallengeUntil.IsZero() || target.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("convite expirado nao foi rejeitado e limpo")
	}
}
