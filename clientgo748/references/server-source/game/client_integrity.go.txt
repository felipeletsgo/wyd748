package game

import (
	"fmt"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

type clientIntegrityPending struct {
	Category      int32
	ByteOffset    int32
	ExpectedValue int32
	Deadline      time.Time
}

// WithClientIntegrity entrega ao World uma copia privada do manifesto. Um
// manifesto sem probes desabilita o mecanismo sem mudar o fluxo de login.
func WithClientIntegrity(file model.ClientIntegrityFile) WorldOption {
	return func(w *World) {
		file.Probes = append([]model.ClientIntegrityProbe(nil), file.Probes...)
		w.clientIntegrity = file
	}
}

func (w *World) issueClientIntegrityChallenge(p *Player) {
	if p == nil || p.Session == nil || !p.InWorld || p.Char == nil ||
		len(w.clientIntegrity.Probes) == 0 || p.clientIntegrityPending != nil {
		return
	}
	probe := w.clientIntegrity.Probes[w.intn(len(w.clientIntegrity.Probes))]
	p.clientIntegrityPending = &clientIntegrityPending{
		Category:      probe.Category,
		ByteOffset:    probe.ByteOffset,
		ExpectedValue: probe.ExpectedValue,
		Deadline:      w.now().Add(time.Duration(w.clientIntegrity.TimeoutSeconds) * time.Second),
	}
	p.Session.Send(wire.ClientIntegrityChallenge(p.ID, probe.Category, probe.ByteOffset))
}

func (w *World) rejectClientIntegrity(s *net.Session, reason string) {
	w.recordSecurityViolation(s, wire.OpClientIntegrityResponse, reason)
	if s != nil {
		s.Close()
	}
}

func (w *World) onClientIntegrityResponse(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil {
		w.rejectClientIntegrity(s, "resposta sem personagem ativo")
		return
	}
	pending := p.clientIntegrityPending
	if pending == nil {
		w.rejectClientIntegrity(s, "resposta ausente ou repetida")
		return
	}
	if !w.now().Before(pending.Deadline) {
		p.clientIntegrityPending = nil
		w.rejectClientIntegrity(s, "timeout da resposta")
		return
	}
	response, err := wire.ParseClientIntegrityResponse(pkt)
	if err != nil {
		p.clientIntegrityPending = nil
		w.rejectClientIntegrity(s, err.Error())
		return
	}
	if response.ID != p.ID || response.Category != pending.Category ||
		response.ByteOffset != pending.ByteOffset || response.Value != pending.ExpectedValue {
		p.clientIntegrityPending = nil
		w.rejectClientIntegrity(s, fmt.Sprintf(
			"resposta divergente id=%d categoria=%d offset=%d valor=%d",
			response.ID, response.Category, response.ByteOffset, response.Value))
		return
	}
	p.clientIntegrityPending = nil
}

func (w *World) tickClientIntegrity(now time.Time) {
	if len(w.clientIntegrity.Probes) == 0 {
		return
	}
	for _, p := range w.players {
		pending := p.clientIntegrityPending
		if pending == nil || now.Before(pending.Deadline) {
			continue
		}
		p.clientIntegrityPending = nil
		w.rejectClientIntegrity(p.Session, "timeout da resposta")
	}
}
