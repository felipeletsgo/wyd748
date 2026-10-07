package game

import (
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func (w *World) queueMountConsumable(p *Player, slot byte, rule model.VolatileRule, req useItemRequest, incubation bool) error {
	if p == nil || p.Account == nil || p.Char == nil || !bindCriticalPlayer(p).current(w) ||
		int(slot) >= model.PlayerCarrySlots || p.Char.Inv[slot].Index == 0 {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	item, mount := &ch.Inv[slot], &ch.Equip[mountSlot]
	reject := func(message string) error { return &nativeConsumableRejection{message: message} }
	message := ""
	var rolls []percentRoll
	hatched := false
	if incubation || rule.MountAction == "hatch" {
		if req.dstType != placeEquip || req.dstPos != mountSlot || !model.IsMountEgg(mount.Index) {
			return reject("Use this item on an equipped egg.")
		}
		if incubation {
			if delay := eggDelay(*mount); delay > 0 {
				return reject(fmt.Sprintf("The egg is still incubating (%d hour(s) remaining).", delay))
			}
			def, exists := w.items[mount.Index]
			threshold := staticAbility(def, "EF_INCUBATE")
			if !exists || threshold < 0 || threshold > 15 {
				return reject("")
			}
			progress := eggProgress(*mount)
			roll := w.rollPercent(refineChance(progress))
			rolls = []percentRoll{roll}
			hatched = roll.Success && progress >= threshold
			if !hatched {
				if roll.Success && !setEggProgress(mount, progress+1) {
					return reject("The egg has no free effect slot for incubation.")
				}
				delay := eggHatchDelayMinHours + w.intn(eggHatchDelayMaxHours-eggHatchDelayMinHours+1)
				if !setEggDelay(mount, delay) {
					return reject("The egg has no free effect slot for incubation.")
				}
				if roll.Success {
					message = fmt.Sprintf("Egg incubation advanced (+%d). Wait %d hour(s).", eggProgress(*mount), delay)
				} else {
					message = fmt.Sprintf("Refinement failed. Wait %d hour(s) before trying again.", delay)
				}
			}
		} else {
			hatched = true
		}
		if hatched {
			mount.Index += model.MountTypeCount
			mount.Eff = [6]byte{}
			w.initFreshMount(mount)
			message = "The egg hatched! A hatchling was born."
		}
		consumeOne(item)
	} else {
		if !model.IsMount(mount.Index) {
			return reject("Equip the mount to use this on it.")
		}
		var ok bool
		switch rule.MountAction {
		case "essence":
			outcome := w.mountEssence(mount, item.Index)
			ok, message, rolls = outcome.OK, outcome.Message, outcome.Rolls
		case "feed":
			ok, message = mountFeed(mount, item.Index)
		case "longevity":
			ok, message = mountLongevityRecoverWithRNG(mount, w.intn)
		case "longevity_restore":
			ok, message = mountLongevityRestore(mount, rule.Amount)
		case "level_set":
			ok, message = mountSetLevel(mount, rule.MountMinLevel, rule.Amount)
		case "growth":
			ok, message = mountGrowthWithRNG(mount, item.Index, w.intn)
		default:
			return reject("")
		}
		if !ok {
			return reject(message)
		}
		if rule.Consume {
			consumeOne(item)
		}
	}
	w.recalcPlayer(ch)
	eggUID, eggTick := p.EggIncubationUID, p.NextEggIncubationTick
	return w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, "mount consumable",
		"Mount item use could not be saved. Please try again later.", func() {
			p.Char.Inv[slot], p.Char.Equip[mountSlot] = *item, *mount
			if incubation || rule.MountAction == "hatch" {
				p.EggIncubationUID, p.NextEggIncubationTick = "", time.Time{}
				if !hatched {
					p.EggIncubationUID = eggIncubationClockKey(mount)
					// Unacknowledged time must not earn online incubation hours.
					p.NextEggIncubationTick = w.now().Add(eggIncubationHour)
				}
			}
			w.recalcPlayer(p.Char)
		}, func() {
			resyncUsedItem(p.Session, p, slot)
			p.Session.Send(wire.SendItem(p.ID, placeEquip, mountSlot, p.Char.Equip[mountSlot]))
			p.Session.Send(playerScorePacket(p))
			w.syncPlayerVitals(p)
			w.refreshAppearance(p)
			w.syncCriaPet(p)
			for i, roll := range rolls {
				text := roll.message()
				if i > 0 {
					text = roll.namedMessage("Penalty")
				}
				p.Session.Send(wire.MessagePanel(text))
			}
			if message != "" {
				p.Session.Send(wire.MessagePanel(message))
			}
		}, true, criticalAccountMutationHooks{check: func() error {
			if p.EggIncubationUID != eggUID || p.NextEggIncubationTick != eggTick {
				return errCriticalPersistenceIdentity
			}
			return nil
		}, reject: func() {
			if !p.Session.IsClosed() {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, mountSlot, p.Char.Equip[mountSlot]))
			}
		}})
}

func (w *World) admitMountConsumable(p *Player, slot byte, rule model.VolatileRule, req useItemRequest, incubation bool) {
	if err := w.queueMountConsumable(p, slot, rule, req, incubation); err != nil {
		resyncUsedItem(p.Session, p, slot)
		var rejection *nativeConsumableRejection
		if errors.As(err, &rejection) {
			if rejection.message != "" {
				p.Session.Send(wire.MessagePanel(rejection.message))
			}
			return
		}
		p.Session.Send(wire.MessagePanel("Mount item use is unavailable. Please try again later."))
		log.Printf("[#%d] mount consumable admission rejected: %v", p.Session.ID, err)
	}
}
