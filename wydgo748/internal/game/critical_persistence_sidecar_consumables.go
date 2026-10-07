package game

import (
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func criticalSidecarConsumable(action string) bool {
	switch action {
	case "buff", "face_transform", "face_restore", "grant_counter", "grant_counter_once", "timed_access":
		return true
	}
	return false
}

// Account consumption, reward identities, currencies and absolute affect
// expiry are one aggregate. A held write must not extend a buff on completion.
func (w *World) queueSidecarConsumable(p *Player, slot byte, rule model.VolatileRule) error {
	if p == nil || p.Account == nil || p.Char == nil || !bindCriticalPlayer(p).current(w) ||
		int(slot) >= model.PlayerCarrySlots || p.Char.Inv[slot].Index == 0 || !criticalSidecarConsumable(rule.Action) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	detached := &Player{Char: ch, SpecialCoins: copyCounters(p)}
	reject := func(message string) error { return &nativeConsumableRejection{message: message} }
	now := w.now()
	var accessExpiry time.Time
	switch rule.Action {
	case "buff":
		result := w.applyVolatileBuff(ch, rule, ch.Inv[slot])
		if result == volatileBuffAlreadyActive {
			return reject(buffAlreadyActiveMessage)
		}
		if result != volatileBuffApplied {
			return reject(buffRejectedMessage)
		}
	case "face_transform":
		if !setFaceAffectAt(ch, rule.FaceMesh, rule.DurationUnits, now) {
			return reject("")
		}
	case "face_restore":
		if !removeFaceAffect(ch) {
			return reject("")
		}
	case "grant_counter", "grant_counter_once":
		if len(rule.Counters) == 0 {
			return reject("")
		}
		for name, amount := range rule.Counters {
			if amount > model.MaxQuestCounter {
				return reject("Invalid counter reward.")
			}
			if rule.Action == "grant_counter_once" && counterBalance(detached, name) >= amount {
				return reject("This unlock is already active.")
			}
		}
	case "timed_access":
		key := accessCounterPrefix + rule.AccessKey
		current := counterBalance(detached, key)
		if current > uint32(now.Unix()) {
			remaining := time.Unix(int64(current), 0).Sub(now).Round(time.Minute)
			return reject(fmt.Sprintf("This access is already active for %s.", remaining))
		}
		accessExpiry = now.Add(time.Duration(rule.DurationSeconds) * time.Second)
		if accessExpiry.Unix() <= 0 || accessExpiry.Unix() > int64(^uint32(0)) {
			return reject("")
		}
		if detached.SpecialCoins == nil {
			detached.SpecialCoins = make(map[string]uint32)
		}
		detached.SpecialCoins[key] = uint32(accessExpiry.Unix())
	}
	if rule.Consume {
		consumeOne(&ch.Inv[slot])
	}
	var rewardSlots []int
	if rule.Action == "grant_counter_once" {
		for _, id := range rule.RewardItems {
			reward, err := materializeItem(model.Item{Index: id})
			if err != nil {
				return err
			}
			pos := firstFreeVisibleInventorySlot(ch)
			if pos < 0 {
				return reject("Inventory full.")
			}
			ch.Inv[pos] = reward
			rewardSlots = append(rewardSlots, pos)
		}
	}
	if rule.Action == "grant_counter" || rule.Action == "grant_counter_once" {
		grantCounters(detached, rule.Counters)
	}
	changesAffects := rule.Action == "buff" || rule.Action == "face_transform" || rule.Action == "face_restore"
	if changesAffects {
		w.recalcPlayer(ch)
	}
	write := criticalWrite{character: &criticalCharacterState{uid: ch.UID, state: buildCharState(detached, now)}}
	return w.queueCriticalAccountWrite(p, proposed, write, rule.Action,
		"Item use could not be saved. Please try again later.", func() {
			p.Char.Inv = ch.Inv
			p.SpecialCoins = detached.SpecialCoins
			if changesAffects {
				p.Char.Affects = ch.Affects
				w.recalcPlayer(p.Char)
			}
		}, func() {
			resyncUsedItem(p.Session, p, slot)
			for _, pos := range rewardSlots {
				if pos != int(slot) {
					p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
				}
			}
			if changesAffects {
				w.publishPlayerAffects(p)
				if rule.Action == "buff" {
					w.syncPlayerVitals(p)
					w.updatePartyMember(p)
				} else {
					w.refreshAppearance(p)
				}
			}
			if rule.Action == "grant_counter_once" {
				p.Session.Send(wire.MessagePanel("Event access enabled."))
			}
			if rule.Action == "timed_access" {
				p.Session.Send(wire.MessagePanel(fmt.Sprintf("Access enabled until %s.", accessExpiry.Local().Format("2006-01-02 15:04"))))
			}
		}, true, criticalAccountMutationHooks{})
}

func (w *World) admitSidecarConsumable(p *Player, slot byte, rule model.VolatileRule) {
	if err := w.queueSidecarConsumable(p, slot, rule); err != nil {
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
