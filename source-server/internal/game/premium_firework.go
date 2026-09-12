package game

import (
	"encoding/binary"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	premiumFireworkUsePacketSize = 52
	premiumFireworkPatternBytes  = 16
	// TMFieldScene::UseFireWork bloqueia novas tentativas por 200 ms. Repetir o
	// mesmo limite no servidor protege o save sincrono contra replay de pacote.
	premiumFireworkCooldown = 200 * time.Millisecond
)

type premiumFireworkRequest struct {
	sourcePos byte
	gridX     uint16
	gridY     uint16
	pattern   [premiumFireworkPatternBytes]byte
}

// parsePremiumFireworkRequest porta o MSG_UseItem2 que o client 7.48 monta em
// TMFieldScene::UseFireWork. Somente os primeiros 100 bits sao pixels; os 28
// bits finais e os dois bytes de padding da struct devem continuar zerados.
func parsePremiumFireworkRequest(pkt []byte) (premiumFireworkRequest, bool) {
	if len(pkt) != premiumFireworkUsePacketSize ||
		binary.LittleEndian.Uint32(pkt[12:16]) != placeInv ||
		binary.LittleEndian.Uint32(pkt[16:20]) >= model.PlayerCarrySlots ||
		binary.LittleEndian.Uint32(pkt[20:24]) != 0 ||
		binary.LittleEndian.Uint32(pkt[24:28]) != 0 ||
		binary.LittleEndian.Uint16(pkt[32:34]) != 0 ||
		pkt[50] != 0 || pkt[51] != 0 {
		return premiumFireworkRequest{}, false
	}

	var req premiumFireworkRequest
	req.sourcePos = byte(binary.LittleEndian.Uint32(pkt[16:20]))
	req.gridX = binary.LittleEndian.Uint16(pkt[28:30])
	req.gridY = binary.LittleEndian.Uint16(pkt[30:32])
	copy(req.pattern[:], pkt[34:50])
	if req.pattern[12]&0xF0 != 0 || req.pattern[13] != 0 ||
		req.pattern[14] != 0 || req.pattern[15] != 0 {
		return premiumFireworkRequest{}, false
	}
	return req, true
}

func (w *World) resyncPremiumFireworkSlot(s *net.Session, p *Player, pkt []byte) {
	if s == nil || p == nil || p.Char == nil || len(pkt) < 20 {
		return
	}
	pos := binary.LittleEndian.Uint32(pkt[16:20])
	if pos >= model.PlayerCarrySlots {
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
}

// onUsePremiumFirework trata o fluxo desenhavel separado dos fogos comuns.
// O client remove a unidade localmente ao enviar 0x3C9, portanto toda recusa
// ressincroniza o slot; o desenho so e publicado depois do commit da conta.
func (w *World) onUsePremiumFirework(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	req, ok := parsePremiumFireworkRequest(pkt)
	if !ok {
		w.resyncPremiumFireworkSlot(s, p, pkt)
		w.recordSecurityViolation(s, wire.OpUsePremiumFirework,
			"layout ou bitmap do Premium FireCracker invalido")
		return
	}
	if absDiff(req.gridX, p.X) > maxStopPositionDrift ||
		absDiff(req.gridY, p.Y) > maxStopPositionDrift {
		w.resyncPremiumFireworkSlot(s, p, pkt)
		w.recordSecurityViolation(s, wire.OpUsePremiumFirework,
			"coordenada do Premium FireCracker diverge da posicao autoritativa")
		return
	}

	item := &p.Char.Inv[req.sourcePos]
	rule, _, registered := w.volatiles.Rule(item.Index)
	if item.Index == 0 || !registered || rule.Action != "firework" ||
		!rule.CustomPattern || !rule.Consume {
		w.resyncPremiumFireworkSlot(s, p, pkt)
		w.recordSecurityViolation(s, wire.OpUsePremiumFirework,
			fmt.Sprintf("slot %d nao contem Premium FireCracker", req.sourcePos))
		return
	}
	if _, exists := w.items[item.Index]; !exists {
		w.resyncPremiumFireworkSlot(s, p, pkt)
		log.Printf("[#%d] Premium FireCracker %d ausente do catalogo server-side",
			s.ID, item.Index)
		return
	}

	now := w.now()
	if !p.LastPremiumFirework.IsZero() &&
		now.Sub(p.LastPremiumFirework) < premiumFireworkCooldown {
		w.resyncPremiumFireworkSlot(s, p, pkt)
		return
	}

	w.cancelTrade(p, "item consumido")
	oldItem := *item
	consumeOne(item)
	if err := w.saveAccount(p.Account); err != nil {
		*item = oldItem
		w.resyncPremiumFireworkSlot(s, p, pkt)
		log.Printf("[#%d] Premium FireCracker nao persistido: %v", s.ID, err)
		return
	}

	p.LastPremiumFirework = now
	s.Send(wire.SendItem(p.ID, placeInv, req.sourcePos, *item))
	w.sendToPlayerView(p, func() []byte {
		return wire.PremiumFirework(p.ID, req.pattern)
	})
}
