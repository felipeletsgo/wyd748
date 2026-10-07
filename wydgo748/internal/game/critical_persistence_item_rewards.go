package game

import (
	"errors"
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func criticalItemReward(action string) bool {
	return action == "loot_box" || action == "mount_revive"
}

// Random outcomes and new item identities are prepared once, before admission.
// Rejection never consumes the box, debits gold, or changes mount longevity.
func (w *World) queueItemReward(p *Player, slot byte, rule model.VolatileRule) error {
	if p == nil || p.Account == nil || p.Char == nil || !bindCriticalPlayer(p).current(w) ||
		int(slot) >= model.PlayerCarrySlots || p.Char.Inv[slot].Index == 0 || !criticalItemReward(rule.Action) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	reject := func(message string) error { return &nativeConsumableRejection{message: message} }
	rewardSlot, mountPos := -1, -1
	var rewardID uint16
	switch rule.Action {
	case "loot_box":
		if len(rule.RewardItems) == 0 {
			return reject("")
		}
		rewardID = rule.RewardItems[w.intn(len(rule.RewardItems))]
		reward, err := materializeItem(model.Item{Index: rewardID})
		if err != nil {
			return err
		}
		if model.IsMount(reward.Index) {
			w.initFreshMount(&reward)
		}
		if rule.Consume {
			consumeOne(&ch.Inv[slot])
		}
		rewardSlot = firstFreeVisibleInventorySlot(ch)
		if rewardSlot < 0 {
			return reject("Inventory full.")
		}
		ch.Inv[rewardSlot] = reward
	case "mount_revive":
		mount, pos := equippedMount(ch)
		mountPos = pos
		if mount == nil || mount.MountHP() > 0 || mount.MountLongev() <= 0 {
			return reject("Equip a wounded mount first.")
		}
		def, exists := w.items[mount.Index]
		if !exists || def.Price > ch.Gold {
			return reject("Not enough gold to revive the mount.")
		}
		ch.Gold -= def.Price
		longevity := mount.MountLongev() - w.intn(3)
		if longevity > 0 {
			mount.SetMountLongev(longevity)
			hp := 18_000
			if maximum := w.mountMaxHP(mount.Index); hp > maximum {
				hp = maximum
			}
			mount.SetMountHP(hp)
			if mount.MountFood() < 5 {
				mount.SetMountFood(5)
			}
		} else {
			*mount = model.Item{}
		}
		if rule.Consume {
			consumeOne(&ch.Inv[slot])
		}
		w.recalcPlayer(ch)
	}
	return w.queueCriticalAccountMutation(p, proposed, rule.Action,
		"Item use could not be saved. Please try again later.", func() {
			p.Char.Inv = ch.Inv
			if mountPos >= 0 {
				p.Char.Equip[mountPos], p.Char.Gold = ch.Equip[mountPos], ch.Gold
				w.recalcPlayer(p.Char)
			}
		}, func() {
			resyncUsedItem(p.Session, p, slot)
			if rewardSlot >= 0 {
				if rewardSlot != int(slot) {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(rewardSlot), p.Char.Inv[rewardSlot]))
				}
				p.Session.Send(wire.MessagePanel(fmt.Sprintf("You received %s.", w.items[rewardID].Name)))
			} else {
				mount := p.Char.Equip[mountPos]
				p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(mountPos), mount))
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				p.Session.Send(playerScorePacket(p))
				w.refreshAppearance(p)
				w.syncCriaPet(p)
				message := "Mount revived."
				if mount.Index == 0 {
					message = "The mount could not be revived and was lost."
				}
				p.Session.Send(wire.MessagePanel(message))
			}
		}, true)
}

func (w *World) admitItemReward(p *Player, slot byte, rule model.VolatileRule) {
	if err := w.queueItemReward(p, slot, rule); err != nil {
		resyncUsedItem(p.Session, p, slot)
		var rejection *nativeConsumableRejection
		if errors.As(err, &rejection) {
			if rejection.message != "" {
				p.Session.Send(wire.MessagePanel(rejection.message))
			}
			return
		}
		p.Session.Send(wire.MessagePanel("Item use is unavailable. Please try again later."))
		log.Printf("[#%d] %s admission rejected: %v", p.Session.ID, rule.Action, err)
	}
}
