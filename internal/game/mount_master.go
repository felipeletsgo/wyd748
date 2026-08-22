package game

import (
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	// Merchant 58 e convertido pelo score legado no tipo 10 que faz o client
	// 7.48 abrir a confirmacao nativa. Ao aceitar, ele envia 0x28B/ClickOk=1.
	mountMasterMerchant = 58
	mountMasterReviveHP = 20
	npcKindMountMaster  = "mestre de montaria"
)

func isMountMasterNPC(def *model.NPCDef) bool {
	return def != nil && def.Extended != nil && def.Extended.Merchant == mountMasterMerchant
}

// speakMountMaster reproduz SendSay: a fala sai do ID do NPC e e recebida por
// todos os jogadores proximos, sem transformar texto do client em autoridade.
func (w *World) speakMountMaster(m *Mob, message string) {
	if m == nil || message == "" {
		return
	}
	for _, listener := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if listener != nil && listener.InWorld && listener.Session != nil {
			listener.Session.Send(wire.MessageChatForProtocol(listener.Session.ClientProtocol(), m.ID, message))
		}
	}
}

// handleMountMasterNPC porta o ramo MOUNT_MASTER de _MSG_Quest.cpp. O client
// apenas pede a operacao; montaria, HP, preco, gold e resultado sao sempre
// resolvidos do estado autoritativo do World.
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
		log.Printf("[#%d] Mount_Master recusado: montaria %d ausente do catalogo", s.ID, mount.Index)
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

	snapshot := cloneCharacterState(p.Char)
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
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		w.recalcPlayer(p.Char)
		log.Printf("[#%d] salvar revival no Mount_Master: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. The mount was not changed."))
		return true
	}

	s.Send(wire.SendItem(p.ID, placeEquip, mountSlot, p.Char.Equip[mountSlot]))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(playerScorePacket(p))
	w.refreshAppearance(p)
	w.syncCriaPet(p)
	if p.Char.Equip[mountSlot].Index == 0 {
		w.speakMountMaster(m, "The treatment failed and the mount was lost.")
	} else {
		w.speakMountMaster(m, "Mount revived.")
	}
	return true
}
