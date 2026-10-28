package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	mysticLandStartMerchant  = 10
	mysticLandRewardMerchant = 11
	mysticLandHuntMessage    = 440
	mysticLandReadyMessage   = 441
	mysticLandStartMessage   = 442
	mysticLandClaimedMessage = 443
	mysticLandRewardMessage  = 444
)

// The recovered 7.59 server uses merchants 10/11 and quest bits 2/4/8.
// The existing 7.48 client confirms these NPCs locally, then sends 0x28B.
func (w *World) handleMysticLandNPC(s *net.Session, p *Player, m *Mob, clickOK int32) bool {
	if m == nil || m.Def == nil || m.Def.Score == nil {
		return false
	}
	merchant := m.Def.Score.Merchant
	if merchant != mysticLandStartMerchant && merchant != mysticLandRewardMerchant {
		return false
	}
	if s == nil || s.IsClosed() || p == nil || p.Session != s || !p.InWorld ||
		p.Account == nil || p.Char == nil || p.Char.Score == nil ||
		p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) ||
		p.Char != &p.Account.Chars[p.CharSlot] || playerCurHP(p.Char) == 0 ||
		p.PersistencePoisoned || w.shuttingDown || w.criticalTradeReserved(p) {
		return true
	}
	if p.Char.MysticLandRewardClaimed {
		circleServiceMessage(p, mysticLandClaimedMessage)
		return true
	}
	if merchant == mysticLandStartMerchant {
		if p.Char.MysticLandStarted || clickOK != 1 {
			message := int16(mysticLandHuntMessage)
			if p.Char.MysticLandEligible {
				message = mysticLandReadyMessage
			}
			circleServiceMessage(p, message)
			return true
		}
		images := criticalPreimages(p)
		p.Char.MysticLandStarted = true
		w.commitMysticLand(p, images, "Mystic Land start", func() {
			circleServiceMessage(p, mysticLandHuntMessage)
		})
		return true
	}
	if !p.Char.MysticLandEligible {
		message := int16(mysticLandStartMessage)
		if p.Char.MysticLandStarted {
			message = mysticLandHuntMessage
		}
		circleServiceMessage(p, message)
		return true
	}
	w.claimMysticLandReward(p)
	return true
}

// Called only while constructing the unpublished kill reward plan. Its full
// character snapshot restores this flag if the shared kill transaction fails.
// Credit belongs to the killer; the native branch has no party or area check.
func (w *World) advanceMysticLand(p *Player, m *Mob) bool {
	if p == nil || p.Char == nil || !p.Char.MysticLandStarted ||
		p.Char.MysticLandEligible || p.Char.MysticLandRewardClaimed ||
		m == nil || m.Def == nil || m.Def.Tipo != model.TipoMonstro {
		return false
	}
	face := m.Def.Equip.Rosto.Index
	if (face != 239 && face != 241) || w.intn(20) != 0 {
		return false
	}
	p.Char.MysticLandEligible = true
	return true
}

// Score.Level is zero-based. Preserve the native raw thresholds 50 and 80,
// rather than silently moving them to displayed levels 50 and 80.
func (w *World) rollMysticLandAmulet(level uint32) model.Item {
	first := uint16(551)
	if level >= 80 {
		first = 559
	} else if level >= 50 {
		first = 555
	}
	item := model.Item{Index: first + uint16(w.intn(4)), Eff: [6]byte{43, 0}}
	additions := [...]struct {
		effect, minimum, count byte
	}{{4, 20, 41}, {2, 5, 21}, {26, 5, 11}, {5, 20, 51},
		{60, 2, 7}, {7, 5, 16}, {8, 5, 16}, {9, 5, 16}}
	for pair := 1; pair < 3; pair++ {
		addition := additions[w.intn(len(additions))]
		item.Eff[2*pair] = addition.effect
		item.Eff[2*pair+1] = addition.minimum + byte(w.intn(int(addition.count)))
	}
	return item
}

func (w *World) claimMysticLandReward(p *Player) {
	slot := firstFreeVisibleInventorySlot(p.Char)
	if slot < 0 {
		circleServiceMessage(p, 1)
		return
	}
	item := w.rollMysticLandAmulet(p.Char.Score.Level)
	if _, ok := w.items[item.Index]; !ok {
		circleServiceMessage(p, 190)
		return
	}
	item, err := materializeItem(item)
	if err != nil {
		log.Printf("[#%d] Mystic Land item identity: %v", p.ID, err)
		circleServiceMessage(p, 188)
		return
	}
	images := criticalPreimages(p)
	p.Char.Inv[slot] = item
	p.Char.MysticLandRewardClaimed = true
	w.commitMysticLand(p, images, "Mystic Land reward", func() {
		p.Session.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
		circleServiceMessage(p, mysticLandRewardMessage)
	})
}

func (w *World) commitMysticLand(p *Player, images []criticalPreimage, operation string, publish func()) {
	rejected := func() { circleServiceMessage(p, 188) }
	committed := func() {
		if p.Session != nil && !p.Session.IsClosed() {
			publish()
		}
	}
	if w.criticalWrites != nil {
		if err := w.queueCriticalLiveMutation(operation, images, committed,
			criticalLiveMutationOptions{reject: rejected}); err != nil {
			log.Printf("[#%d] %s admission: %v", p.ID, operation, err)
			rejected()
		}
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		w.restoreCriticalPreimages(images)
		log.Printf("[#%d] %s save: %v", p.ID, operation, err)
		rejected()
		return
	}
	committed()
}
