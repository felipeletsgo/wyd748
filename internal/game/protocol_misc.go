package game

import (
	"encoding/binary"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// onPing trata o keepalive 0x3A0. O pacote nao pede data/hora e nao possui
// resposta: W2PP e Secrets apenas atualizam LastReceiveTime na camada de socket
// e retornam. Recebe-lo pelo game loop ja comprova atividade da sessao.
func (w *World) onPing(_ *net.Session, _ []byte) {}

// onReqRanking trata o uso efetivo do 0x39F nas sources mais novas: Parm1 e o
// ID do jogador consultado. A janela 0x324 do 7.59 nao existe no client 7.48,
// portanto devolvemos o mesmo resumo autoritativo pelo painel nativo.
func (w *World) onReqRanking(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 20 {
		return
	}
	targetIDRaw := binary.LittleEndian.Uint32(pkt[12:16])
	if targetIDRaw == 0 || targetIDRaw > uint32(^uint16(0)) {
		w.recordSecurityViolation(s, wire.OpReqRanking, "id de jogador invalido")
		return
	}
	target := w.playerByID(uint16(targetIDRaw))
	if target == nil || !target.InWorld || target.Char == nil ||
		!w.playersShareGameplaySpace(p, target) || !p.hasVisible(target.ID) ||
		!inView(p.X, p.Y, target.X, target.Y) {
		s.Send(wire.MessagePanel("The selected player is no longer nearby."))
		return
	}
	s.Send(wire.MessagePanel(w.characterInfoLine(target.Char)))
}
