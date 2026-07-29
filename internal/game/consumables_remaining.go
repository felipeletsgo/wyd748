package game

import (
	"fmt"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const accessCounterPrefix = "access_expires:"

func (w *World) useNoDirectItem(s *net.Session, p *Player, item *model.Item, slot byte,
	message string) {
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	if message == "" {
		message = "This item is used by its corresponding NPC or command."
	}
	s.Send(wire.MessagePanel(message))
}

func (w *World) useLootBox(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	if len(rule.RewardItems) == 0 {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	rewardID := rule.RewardItems[w.intn(len(rule.RewardItems))]
	reward, err := materializeItem(model.Item{Index: rewardID})
	if err != nil {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if model.IsMount(reward.Index) {
		w.initFreshMount(&reward)
	}

	snapshot := cloneCharacterState(p.Char)
	if rule.Consume {
		consumeOne(item)
	}
	rewardSlot := firstFreeVisibleInventorySlot(p.Char)
	if rewardSlot < 0 {
		*p.Char = snapshot
		s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
		s.Send(wire.MessagePanel("Inventory full."))
		return
	}
	p.Char.Inv[rewardSlot] = reward
	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
	if rewardSlot != int(slot) {
		s.Send(wire.SendItem(p.ID, placeInv, byte(rewardSlot), reward))
	}
	s.Send(wire.MessagePanel(fmt.Sprintf("You received %s.", w.items[rewardID].Name)))
}

func (w *World) useMountRevive(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	mount, mountPos := equippedMount(p.Char)
	if mount == nil || mount.MountHP() > 0 || mount.MountLongev() <= 0 {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Equip a wounded mount first."))
		return
	}
	def, exists := w.items[mount.Index]
	if !exists || def.Price > p.Char.Gold {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Not enough gold to revive the mount."))
		return
	}

	oldItem, oldMount, oldGold := *item, *mount, p.Char.Gold
	p.Char.Gold -= def.Price
	longevity := mount.MountLongev() - w.intn(3)
	if longevity > 0 {
		mount.SetMountLongev(longevity)
		hp := 18_000
		if maxHP := w.mountMaxHP(mount.Index); hp > maxHP {
			hp = maxHP
		}
		mount.SetMountHP(hp)
		if mount.MountFood() < 5 {
			mount.SetMountFood(5)
		}
	} else {
		*mount = model.Item{}
	}
	if rule.Consume {
		consumeOne(item)
	}
	w.recalcPlayer(p.Char)
	if err := w.saveAccount(p.Account); err != nil {
		*item, *mount, p.Char.Gold = oldItem, oldMount, oldGold
		w.recalcPlayer(p.Char)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	s.Send(wire.SendItem(p.ID, placeEquip, byte(mountPos), *mount))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	w.refreshAppearance(p)
	w.syncCriaPet(p)
	if mount.Index == 0 {
		s.Send(wire.MessagePanel("The mount could not be revived and was lost."))
	} else {
		s.Send(wire.MessagePanel("Mount revived."))
	}
}

func (w *World) useTimedAccess(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	now := w.now()
	key := accessCounterPrefix + rule.AccessKey
	current := counterBalance(p, key)
	if current > uint32(now.Unix()) {
		remaining := time.Unix(int64(current), 0).Sub(now).Round(time.Minute)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel(fmt.Sprintf("This access is already active for %s.", remaining)))
		return
	}
	expires := now.Add(time.Duration(rule.DurationSeconds) * time.Second)
	if expires.Unix() <= 0 || expires.Unix() > int64(^uint32(0)) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	oldItem, oldCounters := *item, copyCounters(p)
	if p.SpecialCoins == nil {
		p.SpecialCoins = make(map[string]uint32)
	}
	p.SpecialCoins[key] = uint32(expires.Unix())
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*item, p.SpecialCoins = oldItem, oldCounters
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	s.Send(wire.MessagePanel(fmt.Sprintf("Access enabled until %s.",
		expires.Local().Format("2006-01-02 15:04"))))
}

func (w *World) useDungeonTeleport(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	members, ok := instancePartyMembers(p, rule.PartyMode)
	if !ok {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Only the party leader can use this ticket."))
		return
	}
	positions, positionsOK := w.planInstancePositions(members, rule.X, rule.Y)
	if !positionsOK {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	type oldPosition struct {
		player *Player
		x, y   uint16
	}
	oldItem := *item
	oldPositions := make([]oldPosition, 0, len(members))
	accounts := make([]*model.Account, 0, len(members))
	seen := make(map[*model.Account]struct{})
	for index, member := range members {
		oldPositions = append(oldPositions, oldPosition{member, member.X, member.Y})
		member.X, member.Y = positions[index][0], positions[index][1]
		member.Char.X, member.Char.Y = member.X, member.Y
		if _, duplicate := seen[member.Account]; !duplicate {
			seen[member.Account] = struct{}{}
			accounts = append(accounts, member.Account)
		}
	}
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		*item = oldItem
		for _, old := range oldPositions {
			old.player.X, old.player.Y = old.x, old.y
			old.player.Char.X, old.player.Char.Y = old.x, old.y
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	exitAt := w.now().Add(time.Duration(rule.DurationSeconds) * time.Second)
	for _, member := range members {
		member.DungeonExitAt = exitAt
		member.DungeonExitX, member.DungeonExitY = playerEntryX, playerEntryY
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte {
			return wire.ActionStop(member.ID, member.X, member.Y)
		})
		member.Session.Send(wire.StandardParm(
			wire.OpInstanceTime, instanceSignalID, uint32(rule.DurationSeconds)))
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
}

func (w *World) tickDungeonTeleports(now time.Time) {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.DungeonExitAt.IsZero() || now.Before(p.DungeonExitAt) {
			continue
		}
		x, y := p.DungeonExitX, p.DungeonExitY
		if !w.teleportPlayer(p, x, y) {
			// Mantem o deadline vencido para repetir no proximo tick. Apagar
			// antes do commit deixaria o personagem preso se o banco falhasse.
			continue
		}
		p.DungeonExitAt = time.Time{}
		p.DungeonExitX, p.DungeonExitY = 0, 0
		p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 0))
	}
}
