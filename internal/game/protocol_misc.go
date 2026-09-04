package game

import (
	"encoding/binary"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const playerChallengeTTL = 30 * time.Second

// onPing trata o keepalive 0x3A0. O pacote nao pede data/hora e nao possui
// resposta: W2PP e Secrets apenas atualizam LastReceiveTime na camada de socket
// e retornam. Recebe-lo pelo game loop ja comprova atividade da sessao.
func (w *World) onPing(_ *net.Session, _ []byte) {}

// onPlayerChallenge trata o convite e a aceitacao do desafio entre jogadores.
// O mesmo opcode e reutilizado pelo client: Parm2 0..3 solicita o desafio e
// Parm2=4 aceita o convite recebido.
func (w *World) onPlayerChallenge(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 20 {
		return
	}
	header := wire.ParseHeader(pkt)
	if header.ID != p.ID {
		w.recordSecurityViolation(s, wire.OpPlayerChallenge, "id de sessao divergente")
		return
	}
	challengerID := binary.LittleEndian.Uint32(pkt[12:16])
	mode := binary.LittleEndian.Uint32(pkt[16:20])
	if mode == 4 {
		w.acceptPlayerChallenge(s, p, uint16(challengerID))
		return
	}
	if mode > 3 || challengerID == 0 || challengerID > uint32(^uint16(0)) {
		w.recordSecurityViolation(s, wire.OpPlayerChallenge, "desafio invalido")
		return
	}
	challenger := p
	target := w.playerByID(uint16(challengerID))
	if target == nil || target == challenger || !target.InWorld || target.Char == nil ||
		!w.playersShareGameplaySpace(challenger, target) || !challenger.hasVisible(target.ID) ||
		!inView(challenger.X, challenger.Y, target.X, target.Y) {
		s.Send(wire.MessagePanel("The selected player is no longer nearby."))
		return
	}
	target.ChallengeFrom = challenger.ID
	target.ChallengeMode = mode
	target.ChallengeUntil = w.now().Add(playerChallengeTTL)
	target.Session.Send(wire.StandardParm2(wire.OpPlayerChallenge, target.ID,
		uint32(challenger.ID), mode))
}

func (w *World) acceptPlayerChallenge(s *net.Session, target *Player, challengerID uint16) {
	if challengerID == 0 || target.ChallengeFrom != challengerID ||
		target.ChallengeUntil.IsZero() || w.now().After(target.ChallengeUntil) {
		target.ChallengeFrom = 0
		target.ChallengeMode = 0
		target.ChallengeUntil = time.Time{}
		s.Send(wire.MessagePanel("The player challenge expired."))
		return
	}
	challenger := w.playerByID(challengerID)
	valid := challenger != nil && challenger != target && challenger.InWorld && challenger.Char != nil &&
		w.playersShareGameplaySpace(challenger, target) && inView(challenger.X, challenger.Y, target.X, target.Y)
	mode := target.ChallengeMode
	// Consume before publishing either side so a duplicated acceptance cannot
	// replay the transition even if a client retransmits the packet.
	target.ChallengeFrom = 0
	target.ChallengeMode = 0
	target.ChallengeUntil = time.Time{}
	if !valid {
		s.Send(wire.MessagePanel("The player challenge is no longer valid."))
		return
	}
	message := "Player challenge accepted."
	if mode == 1 {
		message = "5 vs 5 challenge accepted."
	} else if mode == 2 {
		message = "10 vs 10 challenge accepted."
	} else if mode == 3 {
		message = "All vs all challenge accepted."
	}
	challenger.Session.Send(wire.MessagePanel(message))
	target.Session.Send(wire.MessagePanel(message))
}
