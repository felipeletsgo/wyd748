package game

import (
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	apprenticeLeaderName = "Lider_Aprendiz"
	// TMSRV Quest/24_350Quest_TS_Leader.c REWARD-4: ITEM 411.
	apprenticeRecallReward uint16 = 411
)

// MODERNIZACAO_COMPATIVEL: restore the supplied Apprentice Emblem exchange.
func (w *World) handleApprenticeCape(s *net.Session, p *Player, m *Mob, clickOK int32) bool {
	if m == nil || m.Def == nil || m.Def.Name != apprenticeLeaderName {
		return false
	}
	if p == nil || p.Account == nil || p.Char == nil || p.Char.Score == nil || !p.InWorld || s == nil ||
		s.IsClosed() || p.Session != s || playerCurHP(p.Char) == 0 {
		return true
	}
	if w.shuttingDown || w.criticalTradeReserved(p) {
		s.Send(wire.MessageIndexed(453 - 1000))
		return true
	}
	if !matchesEvolution(p.Char, "mortal") || playerLevel(p.Char) < 100 || playerLevel(p.Char) > 149 {
		s.Send(wire.MessageIndexed(455 - 1000))
		return true
	}
	if clickOK != 1 || p.Char.Equip[13].Index != 4080 {
		s.Send(wire.MessageIndexed(454 - 1000))
		return true
	}
	if p.Char.Equip[model.CapeSlot].Index != 0 {
		s.Send(wire.MessageIndexed(456 - 1000))
		return true
	}
	rewardSlot := firstFreeVisibleInventorySlot(p.Char)
	if rewardSlot < 0 {
		s.Send(wire.MessageIndexed(1 - 1000))
		return true
	}
	if _, ok := w.items[apprenticeRecallReward]; !ok {
		s.Send(wire.MessageIndexed(449 - 1000))
		return true
	}
	if _, ok := w.items[4006]; !ok {
		s.Send(wire.MessageIndexed(449 - 1000))
		return true
	}
	cape, err := materializeItem(model.Item{Index: 4006})
	if err != nil {
		return true
	}
	scrolls, err := materializeItem(model.Item{Index: apprenticeRecallReward})
	if err != nil {
		return true
	}
	images := criticalPreimages(p)
	p.Char.Equip[13], p.Char.Equip[model.CapeSlot] = model.Item{}, cape
	p.Char.Inv[rewardSlot] = scrolls
	w.recalcPlayer(p.Char)
	resync := func() {
		if !s.IsClosed() {
			s.Send(wire.SendItem(p.ID, placeEquip, 13, p.Char.Equip[13]))
			s.Send(wire.SendItem(p.ID, placeEquip, model.CapeSlot, p.Char.Equip[model.CapeSlot]))
			s.Send(wire.SendItem(p.ID, placeInv, byte(rewardSlot), p.Char.Inv[rewardSlot]))
		}
	}
	failed := func() { resync(); s.Send(wire.MessageIndexed(458 - 1000)) }
	publish := func() {
		resync()
		s.Send(playerScorePacket(p))
		w.refreshAppearance(p)
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		s.Send(wire.MessageIndexed(457 - 1000))
	}
	if w.criticalWrites != nil {
		if err := w.queueCriticalLiveMutation("apprentice cape", images, publish, criticalLiveMutationOptions{reject: failed}); err != nil {
			failed()
		}
	} else if err := w.saveAccount(p.Account); err != nil {
		w.restoreCriticalPreimages(images)
		failed()
	} else {
		publish()
	}
	return true
}
