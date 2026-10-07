package game

import (
	"errors"
	"log"
	"maps"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Completion owns a detached grant and exit-grace transition. A ground fallback
// reserves an invisible runtime allocation; it is never a durable ground-item
// journal. The aggregate's reward flag prevents replay of a committed grant.
func (w *World) queueCriticalInstanceReward(inst *ItemInstance, now time.Time) error {
	if w.criticalWrites == nil || inst == nil || w.shuttingDown {
		return errCriticalPersistenceIdentity
	}
	if w.criticalInstanceReserved() {
		return errCriticalPersistenceBusy
	}
	id := inst.RuntimeID
	if id == "" {
		id = inst.Config.ID
	}
	if w.itemInstances[id] != inst || inst.RewardGranted || !inst.ExitAt.IsZero() {
		return errCriticalPersistenceIdentity
	}
	plan := &criticalInstanceDeparture{at: now, generation: w.criticalInstanceGeneration,
		before: cloneCriticalInstanceMembership(w), proposed: cloneCriticalInstanceMembership(w),
		originals: maps.Clone(w.itemInstances)}
	proposedInstance := plan.proposed.itemInstances[id]
	proposedInstance.RewardGranted = true
	proposedInstance.ExitAt = now.Add(10 * time.Second)
	proposedInstance.ExitDeadline = proposedInstance.ExitAt
	if err := plan.proposed.validateInstanceStateOwnership(); err != nil {
		return err
	}
	if inst.Config.RewardItem == 0 {
		// No player fields change. Membership and aggregate identity are still
		// retained by the singleton fence, including disconnected members.
		check := func() error {
			if w.shutdownErr != nil {
				return errCriticalPersistenceStopping
			}
			return plan.check(w)
		}
		if err := check(); err != nil {
			return err
		}
		_, err := w.criticalWrites.submitWithRejection(criticalWrite{instances: plan.snapshot()},
			func(result criticalPersistenceResult) error {
				if result.err != nil {
					return result.err
				}
				if err := check(); err != nil {
					return err
				}
				plan.install(w)
				return nil
			}, nil, func(criticalPersistenceResult) error { return check() })
		return err
	}
	leader := w.playersByID[inst.LeaderID]
	if leader == nil || leader.Char == nil || leader.Account == nil || leader.Session == nil ||
		!leader.InWorld || leader.Session.IsClosed() || !bindCriticalPlayer(leader).current(w) {
		return errCriticalPersistenceIdentity
	}
	if _, exists := w.items[inst.Config.RewardItem]; !exists {
		return errors.New("critical instance reward: unknown reward item")
	}
	members := []*Player{leader}
	for _, memberID := range inst.MemberIDs {
		if memberID == leader.ID {
			continue
		}
		member := w.playersByID[memberID]
		if member == nil || !member.InWorld {
			return errCriticalPersistenceIdentity
		}
		members = append(members, member)
	}
	reward, err := materializeItem(model.Item{Index: inst.Config.RewardItem})
	if err != nil {
		return err
	}
	account := accountStateSnapshot(leader.Account)
	slot := firstFreeVisibleInventorySlot(leader.Char)
	var ground *GroundItem
	var receipt itemInstanceRewardReceipt
	if slot >= 0 {
		account.Chars[leader.CharSlot].Inv[slot] = reward
		receipt = itemInstanceRewardReceipt{Slot: byte(slot), ItemIndex: reward.Index, ItemUID: reward.UID}
	} else {
		// Reuse the ground placement rules without publishing or borrowing
		// createGroundDropCell's live installation path.
		x, y := leader.X, leader.Y
		if reward.Index != 746 {
			var free bool
			if x, y, free = w.emptyGroundCell(x, y, id); !free {
				return errors.New("critical instance reward: no free ground cell")
			}
		}
		groundID, ok := w.allocGroundItemID(reward.Index)
		if !ok {
			return errors.New("critical instance reward: no free ground-item ID")
		}
		owner := ownerOf(leader)
		ground = &GroundItem{ID: groundID, Item: reward, X: x, Y: y,
			Expire: now.Add(groundItemLifetime), InstanceID: id, criticalDrop: 1,
			OwnerUID: owner.uid, ProtectedUntil: now.Add(groundItemProtection), ownerParty: owner.party}
		// Registration is solely an invisible reservation. Gameplay, expiry
		// and spatial replacement already honor criticalReserved().
		w.registerGroundItem(ground)
	}
	var groundContext GroundItem
	if ground != nil {
		groundContext = *ground
	}
	checkGround := func() error {
		if ground != nil && (w.groundItems[ground.ID] != ground || *ground != groundContext) {
			return errCriticalPersistenceIdentity
		}
		return nil
	}
	release := func() {
		if ground != nil {
			ground.criticalDrop = 0
			// The success installer has already set RewardGranted. A certified
			// rejection or admission failure has not, so release its allocation.
			if !inst.RewardGranted {
				w.unregisterGroundItem(ground)
			}
		}
	}
	err = w.queueCriticalInstanceAccounts(leader, members, map[*Player]*model.Account{leader: account}, plan,
		func() {
			if slot >= 0 {
				leader.Char.Inv[slot] = reward
			}
		}, func() {
			if ground != nil {
				w.publishItemSpawn(ground)
				if !leader.Session.IsClosed() {
					leader.Session.Send(wire.MessagePanel("Inventory full: the next Water Scroll was dropped."))
				}
			} else if !leader.Session.IsClosed() {
				leader.Session.Send(wire.SendItem(leader.ID, placeInv, byte(slot), reward))
				leader.Session.Send(wire.MessagePanel("You received the next Water Scroll."))
			}
			log.Printf("[#%d] committed instance reward instance=%q item=%d slot=%d", leader.Session.ID, id, reward.Index, slot)
			// A second admission is allowed only after the grant's account and
			// aggregate fences have been acknowledged and released.
			w.tryWaterAutoAdvance(inst, receipt, w.now())
		}, release, checkGround)
	if err != nil {
		release()
	}
	return err
}
