package game

import (
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	// The legacy score converts Merchant 58 into type 10, which makes the 7.48
	// client open its native confirmation. Accepting sends 0x28B/ClickOk=1.
	mountMasterMerchant = 58
	mountMasterReviveHP = 20
	npcKindMountMaster  = "mount master"
)

func isMountMasterNPC(def *model.NPCDef) bool {
	return def != nil && def.Score != nil && def.Score.Merchant == mountMasterMerchant
}

// speakMountMaster reproduces SendSay: the line comes from the NPC ID and
// reaches every nearby player, without trusting client text as authority.
func (w *World) speakMountMaster(m *Mob, message string) {
	if m == nil || message == "" {
		return
	}
	for _, listener := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if listener != nil && listener.InWorld && listener.Session != nil {
			listener.Session.Send(wire.MessageChat(m.ID, message))
		}
	}
}

// handleMountMasterNPC ports the MOUNT_MASTER branch of _MSG_Quest.cpp. The
// client only requests the operation; mount, HP, price, gold and outcome are
// always resolved from authoritative World state.
func (w *World) handleMountMasterNPC(s *net.Session, p *Player, m *Mob, clickOk int32) bool {
	if m == nil || !isMountMasterNPC(m.Def) {
		return false
	}
	if s == nil || p == nil || p.Char == nil || !p.InWorld {
		return true
	}

	mount := &p.Char.Equip[mountSlot]
	if !model.IsMount(mount.Index) || mount.MountHP() > 0 {
		w.speakMountMaster(m, "Bring me an injured mount and I can heal it.")
		return true
	}
	def, exists := w.items[mount.Index]
	if !exists {
		log.Printf("[#%d] Mount_Master rejected: mount %d is missing from the catalog", s.ID, mount.Index)
		s.Send(wire.MessagePanel("This mount is not registered by the server."))
		return true
	}
	price := def.Price
	if clickOk != 1 {
		name := def.Name
		if name == "" {
			name = fmt.Sprintf("mount %d", mount.Index)
		}
		w.speakMountMaster(m, fmt.Sprintf("Reviving %s costs %d Gold.", name, price))
		return true
	}
	if p.Char.Gold < price {
		s.Send(wire.MessagePanel("Not enough gold to revive the mount."))
		return true
	}

	images := criticalPreimages(p)
	p.Char.Gold -= price
	longevity := mount.MountLongev() - w.intn(3)
	if longevity > 0 {
		mount.SetMountLongev(longevity)
		mount.SetMountHP(mountMasterReviveHP)
		mount.SetMountFood(5)
	} else {
		*mount = model.Item{}
	}
	w.recalcPlayer(p.Char)
	w.commitPlayerMutation(p, images, "Mount_Master revival", "Save failed. The mount was not changed.", false, func() {
		if !s.IsClosed() {
			s.Send(wire.SendItem(p.ID, placeEquip, mountSlot, p.Char.Equip[mountSlot]))
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			s.Send(playerScorePacket(p))
		}
		w.refreshAppearance(p)
		w.syncCriaPet(p)
		if p.Char.Equip[mountSlot].Index == 0 {
			w.speakMountMaster(m, "The treatment failed and the mount was lost.")
		} else {
			w.speakMountMaster(m, "Mount revived.")
		}
	})
	return true
}
