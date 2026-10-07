package game

import (
	"errors"
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func criticalCelestialConsumable(action string) bool {
	return action == "celestial_ideal" || action == "celestial_switch" || action == "celestial_fury"
}

// Evolution is prepared entirely on the actor's detached account. In particular,
// form scores cannot alias either the active score or its alternate form.
func (w *World) queueCelestialConsumable(p *Player, slot byte, action string) error {
	if p == nil || p.Account == nil || p.Char == nil || p.Char.Score == nil ||
		!bindCriticalPlayer(p).current(w) || int(slot) >= model.PlayerCarrySlots || !criticalCelestialConsumable(action) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalTradeReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	detached := &Player{Char: ch, SpecialCoins: copyCounters(p)}
	item := &ch.Inv[slot]
	reject := func(message string) error { return &nativeConsumableRejection{message: message} }
	message, selection := "", false
	oldX, oldY := p.X, p.Y
	switch action {
	case "celestial_ideal":
		if item.Index != idealStoneItem {
			return reject("")
		}
		if celestialTransformationActive(ch, w.now()) {
			return reject("The Ideal Stone cannot be used while transformed.")
		}
		switch strings.ToLower(strings.TrimSpace(ch.Evolution)) {
		case archEvolution:
			if ch.Score.Level < 355 {
				return reject("The Arch must be level 356 or higher.")
			}
			if ch.ArchMortalLevel < maxMortalLevel {
				return reject("The respective Mortal must be level 400.")
			}
			tier := celestialArchTier(ch.Score.Level)
			cytheraIndex := celestialCytheraForTier(tier)
			if tier == 0 || cytheraIndex == 0 {
				return reject("")
			}
			cythera, err := materializeItem(model.Item{Index: cytheraIndex})
			if err != nil {
				return reject("The Cythera could not be created.")
			}
			ch.CelestialArchTier, ch.CelestialCytheraTier = tier, 0
			ch.CelestialLevel40Unlocked, ch.CelestialLevel90Unlocked, ch.CelestialArcana = false, false, false
			ch.AlternateCelestial = nil
			ch.Evolution, ch.Exp = "celestial", 0
			markCelestialFace(&ch.Equip[0], ch.Evolution)
			ch.Score = newCelestialScore(ch.Class, ch.Score)
			ch.Score.Defense = celestialBaseDefense(ch.ArchCrystals)
			ch.RuntimeScore = nil
			ch.LearnedSkill, ch.SecondaryLearnedSkill = celestialSoulBit, 0
			ch.ShortSkill, ch.Affects = [20]byte{}, [16]model.Affect{}
			ch.Equip[1] = cythera
			kingdom := model.KingdomFromCape(ch.Equip[model.CapeSlot].Index)
			cape, _ := model.KingdomCapeAtTier(model.CapeTierMaster, kingdom)
			uid := ch.Equip[model.CapeSlot].UID
			ch.Equip[model.CapeSlot] = model.Item{Index: cape, UID: uid}
			if uid == "" {
				ch.Equip[model.CapeSlot], err = materializeItem(ch.Equip[model.CapeSlot])
				if err != nil {
					return err
				}
			}
			consumeOne(item)
			syncProgression(ch)
			w.recalcPlayer(ch)
			setPlayerCurHP(ch, playerMaxHP(ch))
			setPlayerCurMP(ch, playerMaxMP(ch))
			selection = true
			message = "Your character is now Celestial. Re-enter it from character selection."
		case "celestial":
			if ch.AlternateCelestial != nil {
				return reject("The SubCelestial has already been created.")
			}
			if ch.Score.Level < 120 {
				return reject("The Celestial must be level 121 or higher.")
			}
			if counterBalance(detached, fameCounter) < subCelestialFameCost {
				return reject("100 Fame is required to create a SubCelestial.")
			}
			subClass, ok := archClassFromSefirot(ch.Equip[sefirotSlot])
			if !ok {
				return reject("Equip the SubCelestial class Sefirot.")
			}
			bodyBase := int(ch.Equip[0].Index) - int(ch.Class)
			if bodyBase < 0 {
				return reject("The Celestial face is invalid.")
			}
			mystery := model.Item{Index: mysteriousStoneItem}
			setItemAmount(&mystery, 10)
			mystery, err := materializeItem(mystery)
			if err != nil {
				return err
			}
			subFace := ch.Equip[0]
			subFace.Index = uint16(bodyBase + subClass)
			markCelestialFace(&subFace, "subcelestial")
			ch.AlternateCelestial = &model.CelestialForm{
				Evolution: "subcelestial", Class: byte(subClass), Face: subFace,
				Score: newCelestialScore(byte(subClass), ch.Score), LearnedSkill: celestialSoulBit,
			}
			ch.AlternateCelestial.Score.Defense = celestialBaseDefense(ch.ArchCrystals)
			consumeOne(item)
			ch.Equip[sefirotSlot] = model.Item{}
			if addToInv(ch, mystery) < 0 {
				return reject("Inventory is full.")
			}
			if !spendCounters(detached, map[string]uint32{fameCounter: subCelestialFameCost}) {
				return reject("100 Fame is required to create a SubCelestial.")
			}
			syncProgression(ch)
			w.recalcPlayer(ch)
			message = "Your SubCelestial has been created. Use the Mysterious Stone to switch."
		default:
			return reject("There is no Celestial quest available for this character.")
		}
	case "celestial_switch":
		if item.Index != mysteriousStoneItem || ch.AlternateCelestial == nil || !isCelestialEvolution(ch) {
			return reject("This character has no alternate Celestial form.")
		}
		if !inNativeCity(p.X, p.Y) {
			return reject("The Mysterious Stone can only be used inside a city.")
		}
		current, next := activeCelestialForm(ch), *ch.AlternateCelestial
		loadCelestialForm(ch, next)
		ch.AlternateCelestial = &current
		consumeOne(item)
		syncProgression(ch)
		w.recalcPlayer(ch)
		filterShortSkills(ch)
		if playerCurHP(ch) == 0 {
			setPlayerCurHP(ch, playerMaxHP(ch))
		}
		message = "Celestial form changed."
	case "celestial_fury":
		if item.Index != furyStoneItem || !isCelestialEvolution(ch) {
			return reject("")
		}
		if counterBalance(detached, fameCounter) < 500 {
			return reject("500 Fame is required.")
		}
		if ch.Score.Level < maxCelestialLevel {
			if ch.Score.Level != 89 || ch.CelestialLevel90Unlocked || !advancedEvolution(ch, "celestial") {
				return reject("This unlock is only available at Celestial level 90.")
			}
			ch.CelestialLevel90Unlocked = true
			message = "Celestial level 90 has been unlocked."
		} else {
			if ch.CelestialArcana || ch.Equip[1].Index != 3502 || itemSanc(ch.Equip[1]) < 11 {
				return reject("Equip a Mystic Cythera +11 or higher.")
			}
			for stone := uint16(5334); stone <= 5337; stone++ {
				if !consumeInventoryItem(ch, stone, 1) {
					return reject("The four Secret Stones are required.")
				}
			}
			if w.intn(2) != 0 {
				ch.Equip[1].Index, ch.CelestialArcana = arcanaCytheraItem, true
				message = "The Arcana Cythera was created."
			} else {
				message = "The Arcana composition failed."
			}
			w.recalcPlayer(ch)
		}
		if !spendCounters(detached, map[string]uint32{fameCounter: 500}) {
			return reject("500 Fame is required.")
		}
		consumeOne(item)
	}
	write := criticalWrite{character: &criticalCharacterState{uid: ch.UID, state: buildCharState(detached, w.now())}}
	hooks := criticalAccountMutationHooks{check: func() error {
		if p.X != oldX || p.Y != oldY {
			return errCriticalPersistenceIdentity
		}
		return nil
	}, reject: func() {
		if !p.Session.IsClosed() {
			equip := clientEquipProjection(p.Char)
			p.Session.Send(wire.SelfEquip(p.ID, equip[:]))
		}
	}}
	publish := func() {
		w.resendInventory(p)
		w.syncCelestialPlayer(p)
		p.Session.Send(wire.MessagePanel(message))
	}
	if selection {
		publish = nil
		hooks.afterAck = func() {
			if !p.Session.IsClosed() {
				p.Session.Send(wire.MessagePanel(message))
			}
			announcement := fmt.Sprintf("The character %s has become Celestial!", p.Char.Name)
			w.broadcast(func() []byte { return wire.MessageWhisper(0, "[SERVER]", announcement, 7) })
			w.returnToCharacterSelectionAfterCommittedChange(p, "Celestial creation")
		}
	}
	return w.queueCriticalAccountWrite(p, proposed, write, action,
		"Save failed. Nothing was consumed.", func() {
			// Keep actor-owned Char and Score identities stable until lifecycle teardown.
			score := p.Char.Score
			installed := cloneCharacterState(ch)
			*score = *installed.Score
			installed.Score = score
			*p.Char = installed
			p.SpecialCoins = copyCounters(detached)
			log.Printf("[#%d] celestial action=%s evolution=%s class=%d level=%d",
				p.Session.ID, action, p.Char.Evolution, p.Char.Class, p.Char.Score.Level)
		}, publish, true, hooks)
}

func (w *World) admitCelestialConsumable(p *Player, slot byte, action string) {
	if err := w.queueCelestialConsumable(p, slot, action); err != nil {
		resyncUsedItem(p.Session, p, slot)
		var rejection *nativeConsumableRejection
		if errors.As(err, &rejection) {
			if rejection.message != "" {
				p.Session.Send(wire.MessagePanel(rejection.message))
			}
			return
		}
		p.Session.Send(wire.MessagePanel("Celestial item use is unavailable. Please try again later."))
		log.Printf("[#%d] celestial admission rejected: %v", p.Session.ID, err)
	}
}

func (w *World) admitIdealStoneCraft(p *Player) {
	if w.criticalTradeReserved(p) {
		return
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	for stone := uint16(5334); stone <= 5337; stone++ {
		if !consumeInventoryItem(ch, stone, 1) {
			return
		}
	}
	consumeOne(&ch.Equip[10])
	consumeOne(&ch.Equip[sefirotSlot])
	ideal, err := materializeItem(model.Item{Index: idealStoneItem})
	if err != nil {
		p.Session.Send(wire.MessagePanel("The Ideal Stone could not be created."))
		return
	}
	if addToInv(ch, ideal) < 0 {
		p.Session.Send(wire.MessagePanel("Inventory is full. Nothing was consumed."))
		return
	}
	err = w.queueCriticalAccountMutation(p, proposed, "Ideal Stone composition",
		"Save failed. Nothing was consumed.", func() {
			p.Char.Inv, p.Char.Equip = ch.Inv, ch.Equip
		}, func() {
			w.resendInventory(p)
			p.Session.Send(wire.SendItem(p.ID, placeEquip, 10, p.Char.Equip[10]))
			p.Session.Send(wire.SendItem(p.ID, placeEquip, sefirotSlot, p.Char.Equip[sefirotSlot]))
			p.Session.Send(wire.MessagePanel("The Ideal Stone was created successfully."))
		}, true)
	if err != nil {
		p.Session.Send(wire.MessagePanel("Ideal Stone composition is unavailable. Please try again later."))
		log.Printf("[#%d] Ideal Stone composition admission rejected: %v", p.Session.ID, err)
	}
}
