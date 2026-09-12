package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func clientIntegrityManifest(probes ...model.ClientIntegrityProbe) model.ClientIntegrityFile {
	return model.ClientIntegrityFile{Version: 1, TimeoutSeconds: 10, Probes: probes}
}

func clientIntegrityProbe(category, offset, value int32) model.ClientIntegrityProbe {
	return model.ClientIntegrityProbe{Category: category, ByteOffset: offset, ExpectedValue: value}
}

func clientIntegrityResponse(id uint16, category, offset, value int32) []byte {
	pkt := inboundPacket(wire.OpClientIntegrityResponse, wire.ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint16(pkt[6:8], id)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(category))
	binary.LittleEndian.PutUint32(pkt[16:20], uint32(offset))
	binary.LittleEndian.PutUint32(pkt[20:24], uint32(value))
	return pkt
}

func integrityWorld(p *Player, probes ...model.ClientIntegrityProbe) *World {
	w := worldWithNetworkedPlayers(p)
	w.clock = newFakeClock(time.Unix(100, 0))
	w.rng = fixedRNG{value: len(probes) - 1}
	WithClientIntegrity(clientIntegrityManifest(probes...))(w)
	return w
}

func TestClientIntegrityChallengeSelectionCopyAndSinglePending(t *testing.T) {
	p, session := networkedTestPlayer(7, "Integrity", 2100, 2100)
	probes := []model.ClientIntegrityProbe{
		clientIntegrityProbe(1, 10, 11),
		clientIntegrityProbe(2, 20, -12),
	}
	w := integrityWorld(p, probes...)
	probes[1].ExpectedValue = 99

	w.issueClientIntegrityChallenge(p)
	pending := p.clientIntegrityPending
	if pending == nil || pending.Category != 2 || pending.ByteOffset != 20 || pending.ExpectedValue != -12 {
		t.Fatalf("probe selecionado=%+v", pending)
	}
	if got := session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("challenge enviou %d pacotes", got)
	}
	w.issueClientIntegrityChallenge(p)
	if got := session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("segundo challenge foi enviado com pending ativo: %d", got)
	}
}

func TestClientIntegrityEmptyManifestIsDisabled(t *testing.T) {
	p, session := networkedTestPlayer(8, "Disabled", 2100, 2100)
	w := integrityWorld(p)
	w.issueClientIntegrityChallenge(p)
	if p.clientIntegrityPending != nil || session.QueuedPacketsForTest() != 0 {
		t.Fatal("manifesto vazio ativou o probe")
	}
}

func TestClientIntegritySuccessClearsPendingAndReplayCloses(t *testing.T) {
	p, session := networkedTestPlayer(9, "Success", 2100, 2100)
	probe := clientIntegrityProbe(3, 340, 45)
	w := integrityWorld(p, probe)
	w.issueClientIntegrityChallenge(p)
	response := clientIntegrityResponse(p.ID, probe.Category, probe.ByteOffset, probe.ExpectedValue)
	w.onClientIntegrityResponse(session, response)
	if p.clientIntegrityPending != nil || session.IsClosed() {
		t.Fatalf("sucesso nao limpou pending ou fechou sessao: pending=%+v closed=%v",
			p.clientIntegrityPending, session.IsClosed())
	}
	w.onClientIntegrityResponse(session, response)
	if !session.IsClosed() {
		t.Fatal("replay sem pending nao fechou a sessao")
	}
}

func TestClientIntegrityMismatchClosesImmediately(t *testing.T) {
	probe := clientIntegrityProbe(4, 341, -1)
	tests := map[string]func(*Player) []byte{
		"id":       func(p *Player) []byte { return clientIntegrityResponse(p.ID+1, 4, 341, -1) },
		"category": func(p *Player) []byte { return clientIntegrityResponse(p.ID, 5, 341, -1) },
		"offset":   func(p *Player) []byte { return clientIntegrityResponse(p.ID, 4, 342, -1) },
		"value":    func(p *Player) []byte { return clientIntegrityResponse(p.ID, 4, 341, 1) },
	}
	for name, response := range tests {
		t.Run(name, func(t *testing.T) {
			p, session := networkedTestPlayer(10, name, 2100, 2100)
			w := integrityWorld(p, probe)
			w.issueClientIntegrityChallenge(p)
			w.onClientIntegrityResponse(session, response(p))
			if !session.IsClosed() || p.clientIntegrityPending != nil {
				t.Fatalf("mismatch nao falhou fechado: closed=%v pending=%+v",
					session.IsClosed(), p.clientIntegrityPending)
			}
		})
	}
}

func TestClientIntegrityTimeoutClosesAndClears(t *testing.T) {
	p, session := networkedTestPlayer(11, "Timeout", 2100, 2100)
	w := integrityWorld(p, clientIntegrityProbe(1, 1, 1))
	w.issueClientIntegrityChallenge(p)
	clock := w.clock.(*fakeClock)
	clock.Advance(10 * time.Second)
	w.tickClientIntegrity(clock.Now())
	if !session.IsClosed() || p.clientIntegrityPending != nil {
		t.Fatalf("timeout nao falhou fechado: closed=%v pending=%+v",
			session.IsClosed(), p.clientIntegrityPending)
	}
}

func TestClientIntegrityResponseAtDeadlineClosesBeforeTick(t *testing.T) {
	p, session := networkedTestPlayer(14, "LateResponse", 2100, 2100)
	probe := clientIntegrityProbe(1, 1, 1)
	w := integrityWorld(p, probe)
	w.issueClientIntegrityChallenge(p)
	w.clock.(*fakeClock).Advance(10 * time.Second)
	w.onClientIntegrityResponse(session, clientIntegrityResponse(
		p.ID, probe.Category, probe.ByteOffset, probe.ExpectedValue))
	if !session.IsClosed() || p.clientIntegrityPending != nil {
		t.Fatalf("resposta vencida foi aceita antes do tick: closed=%v pending=%+v",
			session.IsClosed(), p.clientIntegrityPending)
	}
}

func TestClientIntegrityIdentifiableMalformedPacketsCloseAtGate(t *testing.T) {
	tests := map[string]func(*Player) []byte{
		"short": func(_ *Player) []byte {
			pkt := make([]byte, 6)
			binary.LittleEndian.PutUint16(pkt[0:2], 6)
			binary.LittleEndian.PutUint16(pkt[4:6], wire.OpClientIntegrityResponse)
			return pkt
		},
		"header size": func(p *Player) []byte {
			pkt := clientIntegrityResponse(p.ID, 1, 1, 1)
			binary.LittleEndian.PutUint16(pkt[0:2], wire.ClientIntegrityPacketSize-1)
			return pkt
		},
		"packet size": func(p *Player) []byte {
			pkt := inboundPacket(wire.OpClientIntegrityResponse, wire.ClientIntegrityPacketSize+1)
			binary.LittleEndian.PutUint16(pkt[6:8], p.ID)
			return pkt
		},
	}
	for name, packet := range tests {
		t.Run(name, func(t *testing.T) {
			p, session := networkedTestPlayer(12, name, 2100, 2100)
			w := integrityWorld(p, clientIntegrityProbe(1, 1, 1))
			if w.validateInboundCommand(session, packet(p)) {
				t.Fatal("pacote malformado foi aceito")
			}
			if !session.IsClosed() {
				t.Fatal("0x2C2 identificavel nao fechou imediatamente")
			}
		})
	}
}

func TestClientIntegrityWrongPhaseClosesAtGate(t *testing.T) {
	session := gameNet.NewTestSession(13, 8)
	w := worldWithNetworkedPlayers()
	if w.validateInboundCommand(session, clientIntegrityResponse(13, 1, 1, 1)) {
		t.Fatal("0x2C2 fora do mundo foi aceito")
	}
	if !session.IsClosed() {
		t.Fatal("0x2C2 fora de fase nao fechou a sessao")
	}
}

func TestResetCharacterRuntimeClearsClientIntegrityPending(t *testing.T) {
	p := &Player{clientIntegrityPending: &clientIntegrityPending{Category: 1}}
	resetCharacterRuntime(p)
	if p.clientIntegrityPending != nil {
		t.Fatal("reset preservou o pending de integridade")
	}
}
