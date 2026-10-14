package game

import (
	"encoding/binary"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	effectAmount     = 61
	maxCharacterGold = uint32(2_000_000_000)
	potionCooldown   = 100 * time.Millisecond

	buffAlreadyActiveMessage = "This item cannot be used because the buff is already active."
	buffRejectedMessage      = "This buff cannot be applied right now."
)

// useItemRequest is native MSG_UseItem 0x373 (36 bytes). Validate DWORD fields
// before conversion so forged values cannot truncate to a valid slot.
type useItemRequest struct {
	srcType, srcPos uint32
	dstType, dstPos uint32
	gridX, gridY    uint16
	warpID          uint32
}

func parseUseItemRequest(pkt []byte) (useItemRequest, bool) {
	// MSG_UseItem/p373 is a fixed 36-byte layout. Accepting an oversized
	// packet here would let a forged trailer bypass the opcode size policy, and
	// reading only the low half of WarpID would truncate a malicious DWORD.
	if len(pkt) != 36 {
		return useItemRequest{}, false
	}
	r := useItemRequest{
		srcType: binary.LittleEndian.Uint32(pkt[12:16]),
		srcPos:  binary.LittleEndian.Uint32(pkt[16:20]),
		dstType: binary.LittleEndian.Uint32(pkt[20:24]),
		dstPos:  binary.LittleEndian.Uint32(pkt[24:28]),
		gridX:   binary.LittleEndian.Uint16(pkt[28:30]),
		gridY:   binary.LittleEndian.Uint16(pkt[30:32]),
		warpID:  binary.LittleEndian.Uint32(pkt[32:36]),
	}
	// The 7.54 plugin accepts use only from Carry/Inv. Validate the destination
	// even when potions and gold bars do not use it.
	if r.srcType != placeInv || r.srcPos >= model.PlayerCarrySlots ||
		r.dstType > placeStorage || r.dstPos >= model.PlayerCarrySlots {
		return useItemRequest{}, false
	}
	return r, true
}

// consumeOne ports AmountMinus: EF_AMOUNT (61) represents a stack. Without it,
// STRUCT_ITEM represents one unit and consumption must clear the slot.
func consumeOne(item *model.Item) {
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] != effectAmount {
			continue
		}
		if item.Eff[i*2+1] > 1 {
			item.Eff[i*2+1]--
		} else {
			*item = model.Item{}
		}
		return
	}
	*item = model.Item{}
}

func volatileGoldValue(rule model.VolatileRule, item model.Item, def model.ItemDef) uint32 {
	switch rule.ValueSource {
	case "price":
		return def.Price
	case "coin_effects":
		return uint32(itemAbility(item, def, "EF_HWORDCOIN")*256 + itemAbility(item, def, "EF_LWORDCOIN"))
	default:
		return rule.Gold
	}
}

func applyRestore(ch *model.Char, item model.Item, def model.ItemDef, rule model.VolatileRule) (hp, mp uint32) {
	var requestedHP, requestedMP int
	if rule.ValueSource == "item_effects" {
		requestedHP = itemAbility(item, def, "EF_HP") + itemAbility(item, def, "EF_HPADD") +
			itemAbility(item, def, "EF_HPADD2")
		requestedMP = itemAbility(item, def, "EF_MP") + itemAbility(item, def, "EF_MPADD") +
			itemAbility(item, def, "EF_MPADD2")
	} else {
		requestedHP, requestedMP = rule.HP, rule.MP
	}
	if requestedHP > 0 {
		hp = restorePlayerHP(ch, uint32(requestedHP))
	}
	if requestedMP > 0 {
		mp = restorePlayerMP(ch, uint32(requestedMP))
	}
	return hp, mp
}

// resyncUsedItem reverses the 7.48 client's optimistic MSG_UseItem decrement.
// Rejections and persistence failures must republish the authoritative slot,
// even when server-side state has already been restored.
func resyncUsedItem(s *net.Session, p *Player, slot byte) {
	s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
}

// resyncUsedItemTarget restores both ends of drag-and-drop item use; the client
// may have anticipated both source consumption and destination changes.
func resyncUsedItemTarget(s *net.Session, p *Player, slot, destType, destPos byte, dest model.Item) {
	resyncUsedItem(s, p, slot)
	s.Send(wire.SendItem(p.ID, destType, destPos, dest))
}

// onUseItem handles MSG_UseItem 0x373 using authoritative items and catalog
// definitions. Never trust the item ID or effect claimed by the client.
func (w *World) onUseItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 || w.criticalTradeReserved(p) {
		return
	}
	req, ok := parseUseItemRequest(pkt)
	if !ok {
		log.Printf("[#%d] invalid item-use packet", s.ID)
		return
	}
	slot := byte(req.srcPos)
	item := &p.Char.Inv[slot]
	if rule, _, registered := w.volatiles.Rule(item.Index); registered && isDonateCoinRule(rule) {
		if _, exists := w.items[item.Index]; exists {
			w.admitDonateCoin(p, slot, rule)
		} else {
			resyncUsedItem(s, p, slot)
		}
		return
	}
	// Migrated native consumables reserve both trade participants and cancel
	// only after durable acknowledgement. All other paths retain legacy ordering.
	if w.criticalWrites != nil {
		if rule, _, registered := w.volatiles.Rule(item.Index); registered && criticalTeleportConsumable(rule.Action) {
			if _, exists := w.items[item.Index]; exists {
				w.admitTeleportConsumable(p, slot, rule, req)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
		if rule, code, registered := w.volatiles.Rule(item.Index); registered &&
			(rule.Action == "gate_key" || rule.Action == "summon_contract") {
			if _, exists := w.items[item.Index]; !exists {
				resyncUsedItem(s, p, slot)
			} else if rule.Action == "gate_key" {
				w.useGateKey(s, p, item, slot, rule, code)
			} else {
				w.admitContractSummon(p, slot, rule)
			}
			return
		}
		if rule, _, registered := w.volatiles.Rule(item.Index); registered && criticalCelestialConsumable(rule.Action) {
			if _, exists := w.items[item.Index]; exists {
				w.admitCelestialConsumable(p, slot, rule.Action)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
		if rule, code, registered := w.volatiles.Rule(item.Index); registered && rule.Action == "mount" {
			if _, exists := w.items[item.Index]; exists {
				w.applyMountItem(p, s, item, slot, rule, code, req)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
		if rule, code, registered := w.volatiles.Rule(item.Index); registered && criticalEquipmentConsumable(rule.Action) {
			if _, exists := w.items[item.Index]; !exists {
				resyncUsedItem(s, p, slot)
			} else if egg, eggType, eggSlot := w.destEggTarget(p, req); rule.Action == "refine" && egg != nil {
				w.incubateEgg(p, s, item, slot, egg, eggType, eggSlot, code)
			} else {
				w.admitEquipmentConsumable(p, slot, req, rule)
			}
			return
		}
		if rule, _, registered := w.volatiles.Rule(item.Index); registered && criticalItemReward(rule.Action) {
			if _, exists := w.items[item.Index]; exists {
				w.admitItemReward(p, slot, rule)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
		if rule, _, registered := w.volatiles.Rule(item.Index); registered && criticalSidecarConsumable(rule.Action) {
			if _, exists := w.items[item.Index]; exists {
				w.admitSidecarConsumable(p, slot, rule)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
		if rule, _, registered := w.volatiles.Rule(item.Index); registered && criticalBasicConsumable(rule.Action) {
			if def, exists := w.items[item.Index]; exists {
				w.admitBasicConsumable(p, slot, rule, def)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
		if rule, code, registered := w.volatiles.Rule(item.Index); registered && criticalItemExperience(rule.Action) {
			if _, exists := w.items[item.Index]; exists {
				w.admitItemExperience(p, slot, rule, code)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
		if rule, _, registered := w.volatiles.Rule(item.Index); registered &&
			(rule.Action == "equipment_gem" || rule.Action == "ore_upgrade") {
			if _, exists := w.items[item.Index]; !exists {
				resyncUsedItem(s, p, slot)
				return
			}
			if rule.Action == "equipment_gem" {
				w.useEquipmentGem(s, p, item, slot, rule, req)
			} else {
				w.useOreUpgrade(s, p, item, slot, rule, req)
			}
			return
		}
		if rule, code, registered := w.volatiles.Rule(item.Index); registered && criticalNativeConsumable(rule.Action) {
			if _, exists := w.items[item.Index]; exists {
				w.admitNativeConsumable(p, item, slot, rule, code)
			} else {
				resyncUsedItem(s, p, slot)
			}
			return
		}
	}
	if w.criticalWrites != nil {
		if rule, _, registered := w.volatiles.Rule(item.Index); registered && rule.Action == "instance_ticket" {
			w.admitCriticalInstanceTicket(p, slot, rule, "")
			return
		}
	}
	w.cancelTrade(p, "item consumed")
	if item.Index == 0 {
		return
	}
	def, ok := w.items[item.Index]
	if !ok {
		log.Printf("[#%d] use rejected: item %d missing from server-side catalog", s.ID, item.Index)
		resyncUsedItem(s, p, slot)
		return
	}
	rule, code, registered := w.volatiles.Rule(item.Index)
	if !registered {
		log.Printf("[#%d] use rejected: item %d has no server-side EF_VOLATILE", s.ID, item.Index)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	switch rule.Action {
	case "equipment_gem":
		w.useEquipmentGem(s, p, item, slot, rule, req)

	case "ore_upgrade":
		w.useOreUpgrade(s, p, item, slot, rule, req)

	case "instance_ticket":
		if rule.Instance != nil && isDurablePrivateWaterConfig(*rule.Instance) {
			w.useWaterScroll(s, p, item, slot, rule, req, "")
		} else {
			w.useInstanceTicket(s, p, item, slot, rule, req)
		}

	case "loot_box":
		w.useLootBox(s, p, item, slot, rule)

	case "mount_revive":
		w.useMountRevive(s, p, item, slot, rule)

	case "timed_access":
		w.useTimedAccess(s, p, item, slot, rule)

	case "no_direct_use":
		w.useNoDirectItem(s, p, item, slot, "")

	case "celestial_capsule":
		w.useCelestialCapsule(s, p, item, slot)

	case "celestial_ideal":
		w.useCelestialIdeal(s, p, item, slot)

	case "celestial_fury":
		w.useFuryStone(s, p, item, slot)

	case "celestial_switch":
		w.useMysteriousStone(s, p, item, slot)

	case "nightmare_ticket":
		w.useNightmareTicket(s, p, item, slot, rule)

	case "refine_equipped":
		w.useEquippedRefine(s, p, item, slot, rule)

	case "territory_pass":
		w.useTerritoryPass(s, p, item, slot, rule)

	case "mastery_reset":
		w.useMasteryReset(s, p, item, slot, rule, code)

	case "firework":
		// Premium FireCracker uses MSG_UseItem2 0x3C9 from the client's 10x10
		// pattern window. Accepting MSG_UseItem here would discard the pattern
		// and turn the premium item into an ordinary firework.
		if rule.CustomPattern {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		parm := uint16(w.intn(6))
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 100, parm) })

	case "chaos_remission":
		// Native SetPKPoint(150) uses the raw CP+75 byte, so the forgiveness
		// letter sets authoritative CP to its maximum of +75. Confirm only
		// after persistence to prevent duplication or memory-only recovery.
		oldItem, oldCP := *item, p.Char.CP
		p.Char.CP = model.MaxCP
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.CP = oldItem, oldCP
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.syncPlayerChaos(p)

	case "grant_next_level":
		// Native Fairy Dust sets EXP exactly to the next threshold, granting
		// one level rather than an arbitrary fixed amount.
		if !matchesEvolution(p.Char, "mortal") ||
			p.Char.Score == nil || p.Char.Score.Level >= maxMortalLevel {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		next := mortalNextLevel[int(p.Char.Score.Level)+1]
		if p.Char.Exp >= next {
			syncProgression(p.Char)
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldIndex := item.Index
		res := w.grantItemExpReward(s, p, item, slot, next-p.Char.Exp, 0, rule.Consume)
		if !res.OK && res.Err != nil {
			log.Printf("[#%d] failed to save Fairy Dust item=%d: %v", s.ID, oldIndex, res.Err)
		}

	case "quest_reward":
		w.useQuestReward(s, p, item, slot, rule, code)

	case "arch_crystal":
		w.useArchCrystal(s, p, item, slot, rule, code)

	case "grant_counter":
		w.useCounterGrant(s, p, item, slot, rule, code)

	case "grant_counter_once":
		w.useCounterGrantOnce(s, p, item, slot, rule, code)

	case "gate_key":
		w.useGateKey(s, p, item, slot, rule, code)

	case "magical_pill":
		if p.Char.MagicalPillUsed {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldItem, oldBonus := *item, p.Char.SkillPointBonus
		p.Char.MagicalPillUsed, p.Char.SkillPointBonus = true, oldBonus+9
		if rule.Consume {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.SkillPointBonus, p.Char.MagicalPillUsed = oldItem, oldBonus, false
			w.recalcPlayer(p.Char)
			resyncUsedItem(s, p, slot)
			return
		}
		s.Send(playerScorePacket(p))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "hunting_teleport":
		if req.warpID < 1 || req.warpID > 10 || len(rule.Destinations) != 10 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		dest, oldItem := rule.Destinations[int(req.warpID)-1], *item
		if rule.Consume {
			consumeOne(item)
		}
		if !w.teleportPlayer(p, dest.X, dest.Y) {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "learn_special_skill":
		bit := uint(rule.LearnedBit)
		if bit < 25 || bit > 29 || p.Char.LearnedSkill&(uint32(1)<<bit) != 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldItem, oldLearned := *item, p.Char.LearnedSkill
		p.Char.LearnedSkill |= uint32(1) << bit
		if rule.Consume {
			consumeOne(item)
		}
		filterShortSkills(p.Char)
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.LearnedSkill = oldItem, oldLearned
			resyncUsedItem(s, p, slot)
			return
		}
		s.Send(playerScorePacket(p))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "summon_contract":
		oldItem := *item
		if rule.Summon == nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		plan, ok := w.planContractSummon(p, rule.Summon)
		if !ok {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			resyncUsedItem(s, p, slot)
			return
		}
		w.commitContractSummon(plan)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "restore":
		now := w.now()
		if !p.LastPotion.IsZero() && now.Sub(p.LastPotion) < potionCooldown {
			// The client anticipates use visually; resend the authoritative slot.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		snapshot := cloneCharacterState(p.Char)
		oldLastPotion := p.LastPotion
		hp, mp := applyRestore(p.Char, *item, def, rule)
		if hp == 0 && mp == 0 {
			// The client may automatically repeat stack use. When both resources
			// are full, restore the slot without consuming a unit or recording
			// a heal that never occurred.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		p.LastPotion = now
		// A potion changes both the stack and the authoritative current
		// resource. Persist before acknowledging the use; otherwise a forced
		// disconnect between SendItem and autosave can duplicate the unit or
		// roll the resource back independently of the inventory.
		if err := w.saveAccount(p.Account); err != nil {
			*p.Char = snapshot
			p.LastPotion = oldLastPotion
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			log.Printf("[#%d] failed to save potion item=%d: %v", s.ID, item.Index, err)
			return
		}
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	case "gold":
		value := volatileGoldValue(rule, *item, def)
		if value == 0 || value > maxCharacterGold {
			log.Printf("[#%d] invalid volatile gold item=%d value=%d", s.ID, item.Index, value)
			resyncUsedItem(s, p, slot)
			return
		}
		if p.Char.Gold > maxCharacterGold || value > maxCharacterGold-p.Char.Gold {
			// Native behavior does not consume a bar if the total would exceed two billion.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			log.Printf("[#%d] bar item=%d rejected: gold=%d value=%d limit=%d",
				s.ID, item.Index, p.Char.Gold, value, maxCharacterGold)
			return
		}
		oldItem, oldGold := *item, p.Char.Gold
		if rule.Consume {
			consumeOne(item)
		}
		p.Char.Gold += value
		// A bar may be worth up to two billion: persist before confirmation to
		// prevent duplication by disconnecting between use and autosave.
		if err := w.saveAccount(p.Account); err != nil {
			*item, p.Char.Gold = oldItem, oldGold
			resyncUsedItem(s, p, slot)
			log.Printf("[#%d] failed to save bar use item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] used bar item=%d gold=+%d total=%d",
			s.ID, oldItem.Index, value, p.Char.Gold)

	case "teleport":
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		oldX, oldY := p.X, p.Y
		if !w.teleportPlayer(p, rule.X, rule.Y) {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] used teleport item=%d volatile=%d @(%d,%d)->(%d,%d)",
			s.ID, def.Index, code, oldX, oldY, p.X, p.Y)

	case "refine":
		// Ori/Lac refinement targets DestPos in 0x373. Validation and consumption
		// are centralized in refineItem to prevent infinite-refinement exploits.
		w.refineItem(p, s, item, slot, req, rule, code)

	case "refine_set":
		// Gargoyle Molar deterministically refines equipped armor slots 1-5,
		// without a roll or a drag-and-drop target.
		w.refineSet(p, s, item, slot, rule, code)

	case "save_position":
		// Star Gem (volatile 12) records the current position for later Warp use.
		// Persist before confirmation to prevent item duplication.
		oldSX, oldSY, oldItem := p.Char.SavedX, p.Char.SavedY, *item
		p.Char.SavedX, p.Char.SavedY = p.X, p.Y
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			p.Char.SavedX, p.Char.SavedY, *item = oldSX, oldSY, oldItem
			resyncUsedItem(s, p, slot)
			log.Printf("[#%d] failed to save save_position item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Position saved."))
		log.Printf("[#%d] Star Gem saved (%d,%d) item=%d volatile=%d",
			s.ID, p.X, p.Y, oldItem.Index, code)

	case "warp_saved":
		// Warp (volatile 13) teleports to the Gem's saved position. Without one,
		// do not consume. Otherwise use the fixed-teleport flow.
		if p.Char.SavedX == 0 || p.Char.SavedY == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("No position saved."))
			return
		}
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		if !w.teleportPlayer(p, p.Char.SavedX, p.Char.SavedY) {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] warped to saved position (%d,%d) item=%d volatile=%d",
			s.ID, p.X, p.Y, oldItem.Index, code)

	case "buff":
		// Damage potions, food and timed buffs select AffectType from data.
		// applyExtendedAffectStats supplies formulas; volatiles.json supplies
		// balancing values (percentage or level) and duration.
		snapshot := cloneCharacterState(p.Char)
		// Consuming the last unit may clear the slot. Retain its index and UID
		// for both character-state persistence and auditing.
		usedItem := *item
		affects := rule.Affects
		if len(affects) == 0 && rule.AffectType > 0 {
			affects = []model.VolatileAffect{{
				Type: rule.AffectType, Value: rule.AffectValue,
				Level: rule.AffectLevel, DurationUnits: rule.DurationUnits,
			}}
		}
		if len(affects) == 0 {
			log.Printf("[#%d] volatile buff has no affect item=%d code=%d", s.ID, item.Index, code)
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		buffResult := w.applyVolatileBuff(p.Char, rule, usedItem)
		if buffResult != volatileBuffApplied {
			// Native behavior rejects another use when an equal/stronger buff is
			// active or duration is capped. Resynchronize, explain the rejection,
			// and neither consume nor persist the unit.
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			if buffResult == volatileBuffAlreadyActive {
				s.Send(wire.MessagePanel(buffAlreadyActiveMessage))
			} else {
				s.Send(wire.MessagePanel(buffRejectedMessage))
				log.Printf("[#%d] buff item=%d rejected: invalid configuration or affect slots full",
					s.ID, item.Index)
			}
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		w.recalcPlayer(p.Char)
		// Timed affects are economic state: commit consumption and duration in
		// the same save. Otherwise disconnecting before autosave restored the
		// item and allowed repeating Bigger Potion, food or EXP chests.
		if err := w.saveAccountAndCharStateResult(p); err != nil {
			*p.Char = snapshot
			s.Send(wire.SendItem(p.ID, placeInv, slot, p.Char.Inv[slot]))
			log.Printf("[#%d] failed to save buff item=%d: %v", s.ID, item.Index, err)
			return
		}
		w.publishPlayerAffects(p)
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		// Log actual active types and expiration times, not just configured
		// amounts, to distinguish items sharing EF_VOLATILE.
		appliedTypes := make([]byte, 0, len(affects))
		expiresUnix := make([]int64, 0, len(affects))
		for _, configured := range affects {
			affectType := configured.Type
			if configured.SkillID > 0 {
				affectType = w.skills[configured.SkillID].AffectType
			}
			if active := activePlayerAffectAt(p.Char, byte(affectType), w.now()); active != nil {
				appliedTypes = append(appliedTypes, active.Type)
				expiresUnix = append(expiresUnix, active.ExpiresAt.Unix())
			}
		}
		log.Printf("[#%d] used buff item=%d uid=%q volatile=%d affects=%v expires_unix=%v",
			s.ID, usedItem.Index, usedItem.UID, code, appliedTypes, expiresUnix)

	case "grant_exp":
		// EXP chests and powders use the shared EXP transaction (full snapshot
		// and persistence before confirmation), with raw EXP and no rate, buff or gold.
		if rule.Exp == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		oldIndex := item.Index
		res := w.grantItemExpReward(s, p, item, slot, rule.Exp, 0, rule.Consume)
		if !res.OK {
			if res.Err != nil {
				log.Printf("[#%d] failed to save grant_exp item=%d: %v", s.ID, oldIndex, res.Err)
			}
			return
		}
		log.Printf("[#%d] used grant_exp item=%d volatile=%d exp=+%d levels=+%d",
			s.ID, oldIndex, code, res.Exp, res.Levels)

	case "disabled":
		// Explicitly disabled volatile (e.g. code 9): consume without an effect
		// to remove the confusing item. Unlike generic, this action consumes it.
		oldItem := *item
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*item = oldItem
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			log.Printf("[#%d] failed to save disabled volatile item=%d: %v", s.ID, oldItem.Index, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] disabled volatile item=%d code=%d (consumed=%v)", s.ID, item.Index, code, rule.Consume)

	case "face_transform":
		// Cosmetic monster-face transformation (70-77): store the mesh in the
		// affect Value; bodyMesh applies it and observers receive the appearance.
		snapshot := cloneCharacterState(p.Char)
		if !setFaceAffectAt(p.Char, rule.FaceMesh, rule.DurationUnits, w.now()) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccountAndCharStateResult(p); err != nil {
			*p.Char = snapshot
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			log.Printf("[#%d] failed to save face transformation item=%d: %v", s.ID, item.Index, err)
			return
		}
		w.recalcPlayer(p.Char)
		w.publishPlayerAffects(p)
		w.refreshAppearance(p) // Refresh the owner's SelfEquip and observers.
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] transformed face item=%d volatile=%d mesh=%d duration=%d",
			s.ID, item.Index, code, rule.FaceMesh, rule.DurationUnits)

	case "face_restore":
		// Restore the original face (volatile 89); consume only if transformed.
		snapshot := cloneCharacterState(p.Char)
		restored := removeFaceAffect(p.Char)
		if rule.Consume && restored {
			consumeOne(item)
		}
		if restored {
			if err := w.saveAccountAndCharStateResult(p); err != nil {
				*p.Char = snapshot
				s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
				log.Printf("[#%d] failed to save face restoration item=%d: %v", s.ID, item.Index, err)
				return
			}
		}
		w.recalcPlayer(p.Char)
		w.publishPlayerAffects(p)
		w.refreshAppearance(p) // Restore appearance for the owner and observers.
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		log.Printf("[#%d] restored face item=%d volatile=%d (was_transformed=%v)", s.ID, item.Index, code, restored)

	case "tint":
		// Tint (186) replaces the target's sanc effect code with color (116-125).
		// Refinement value is unchanged, so no stat recalculation is needed.
		// Persist before confirming; notify observers if the target is equipped.
		dest, destType, destPos := w.destItemTarget(p, req)
		if dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Drag the dye onto an item."))
			return
		}
		if def, ok := w.items[dest.Index]; !ok || !isArmorSetPiece(def) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel(tintSetOnlyMessage))
			return
		}
		oldDest, oldItem := *dest, *item
		if !tintItem(dest, rule.Color) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Dye a refined item (one with a + glow)."))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*dest, *item = oldDest, oldItem
			resyncUsedItemTarget(s, p, slot, byte(destType), byte(destPos), *dest)
			if destType == placeEquip {
				w.refreshAppearance(p)
			}
			log.Printf("[#%d] failed to save tint: %v", s.ID, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		if destType == placeEquip {
			w.refreshAppearance(p)
		}
		log.Printf("[#%d] tinted item=%d target=%d color=%d volatile=%d",
			s.ID, item.Index, dest.Index, rule.Color, code)

	case "untint":
		// Remover_Enamel (3417) converts color back to EF_SANC (43).
		// Consume only when a tint was removed.
		dest, destType, destPos := w.destItemTarget(p, req)
		if dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Drag it onto a colored item."))
			return
		}
		oldDest, oldItem := *dest, *item
		if !untintItem(dest) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Use it on colored items."))
			return
		}
		if rule.Consume {
			consumeOne(item)
		}
		if err := w.saveAccount(p.Account); err != nil {
			*dest, *item = oldDest, oldItem
			resyncUsedItemTarget(s, p, slot, byte(destType), byte(destPos), *dest)
			if destType == placeEquip {
				w.refreshAppearance(p)
			}
			log.Printf("[#%d] failed to save untint: %v", s.ID, err)
			return
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		if destType == placeEquip {
			w.refreshAppearance(p)
		}
		log.Printf("[#%d] removed tint item=%d target=%d volatile=%d",
			s.ID, item.Index, dest.Index, code)

	case "repliction":
		// Repliction (190, normal/premium A-E) ports SetItemBonus2: roll a full
		// bonus pair for the armor part and replace both adds. Reserve the third
		// logical pair for refinement/tint.
		dest, destType, destPos := w.destItemTarget(p, req)
		if destType != placeEquip || destPos >= 16 || dest == nil || dest.Index == 0 {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Repliction can only be used on equipped defensive equipment."))
			return
		}
		def, exists := w.items[dest.Index]
		if !exists {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			return
		}

		oldDest, oldItem := *dest, *item
		bonus, err := w.applyRepliction(dest, def, item.Index)
		if err != nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel(err.Error()))
			return
		}

		if rule.Consume {
			consumeOne(item)
		}

		if err := w.saveAccount(p.Account); err != nil {
			*dest, *item = oldDest, oldItem
			resyncUsedItemTarget(s, p, slot, byte(destType), byte(destPos), *dest)
			// Equipment bonuses changed: republish score, equipment and vitals
			// to remove optimistic previews for the owner and observers.
			w.recalcPlayer(p.Char)
			equip := clientEquipProjection(p.Char)
			s.Send(wire.SelfEquip(p.ID, equip[:]))
			s.Send(playerScorePacket(p))
			w.syncPlayerVitalsToObservers(p)
			log.Printf("[#%d] failed to save repliction: %v", s.ID, err)
			return
		}

		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.SendItem(
			p.ID,
			byte(destType),
			byte(destPos),
			*dest,
		))

		if destType == placeEquip {
			w.recalcPlayer(p.Char)
			equip := clientEquipProjection(p.Char)
			s.Send(wire.SelfEquip(p.ID, equip[:]))
			s.Send(playerScorePacket(p))
			w.syncPlayerVitalsToObservers(p)
		}

		s.Send(wire.MessagePanel("The item gained a new bonus!"))

		log.Printf(
			"[#%d] repliction item=%d target=%d adds=ef%d/v%d+ef%d/v%d volatile=%d",
			s.ID,
			item.Index,
			dest.Index,
			bonus.Effect1, bonus.Value1,
			bonus.Effect2, bonus.Value2,
			code,
		)

	case "mount":
		// Mount consumables: essence 16, feed 15, LP/catalysts 90-92, longevity 93,
		// growth 94 and hatch 196. MountAction selects the effect in mount.go.
		w.applyMountItem(p, s, item, slot, rule, code, req)

	case "generic":
		// Unconfigured volatile fallback: retain state and item. Log all fields
		// needed to identify behavior when the rule is configured.
		log.Printf("[#%d] GENERIC VOLATILE code=%d item=%d src=%d/%d dst=%d/%d grid=(%d,%d) packetItem=%d",
			s.ID, code, item.Index, req.srcType, req.srcPos, req.dstType, req.dstPos,
			req.gridX, req.gridY, req.warpID)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))

	default:
		log.Printf("[#%d] item %d volatile=%d action=%s has no active handler yet",
			s.ID, item.Index, code, rule.Action)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	}
}

func (w *World) applyRepliction(dest *model.Item, def model.ItemDef, sourceID uint16) (model.ReplictionBonus, error) {
	rule, ok := w.volatiles.Repliction.Items[sourceID]
	if !ok {
		return model.ReplictionBonus{}, clientError("This repliction is not configured.")
	}
	pool := w.volatiles.Repliction.Pools[def.Pos]
	if len(pool) == 0 {
		return model.ReplictionBonus{}, clientError("Repliction works only on defensive equipment.")
	}
	if dest.Index >= 3500 && dest.Index <= 3507 {
		return model.ReplictionBonus{}, clientError("Repliction cannot be used on Cythera.")
	}
	if sanc := itemSanc(*dest); sanc > rule.MaxSanc {
		return model.ReplictionBonus{}, clientError("The refinement level is too high for this repliction.")
	}
	mobType := itemAbility(*dest, def, "EF_MOBTYPE")
	if mobType != 0 && mobType != 2 {
		return model.ReplictionBonus{}, clientError("This item type cannot receive Repliction.")
	}
	if itemAbility(*dest, def, "EF_ITEMLEVEL") != rule.ItemLevel {
		return model.ReplictionBonus{}, clientError("The repliction grade does not match the item.")
	}

	// Preserve refinement or tint regardless of its old effect slot.
	// An unrefined item receives EF_SANC +0, as in SetItemBonus2.
	sancEffect, sancValue := byte(0), byte(0)
	for id, name := range def.DynamicEffectNames {
		if name == "EF_SANC" {
			sancEffect = id
			break
		}
	}
	if sancEffect == 0 {
		return model.ReplictionBonus{}, clientError("EF_SANC is missing from ItemEffect.")
	}
	for i := 0; i < 3; i++ {
		effect := dest.Eff[i*2]
		if effect == sancEffect || (effect >= 116 && effect <= 125) {
			sancEffect, sancValue = effect, dest.Eff[i*2+1]
			break
		}
	}
	bonus := pool[w.intn(len(pool))]
	dest.Eff = [6]byte{
		sancEffect, sancValue,
		byte(bonus.Effect1), byte(bonus.Value1),
		byte(bonus.Effect2), byte(bonus.Value2),
	}
	return bonus, nil
}

// refineChance returns the configured chance for the current level.
// World.rollPercent uses a uniform 1..100 domain and success when roll <= chance.
// refinePowderChance decides whether a "refine" powder may act on an item at
// the given level. Below RefineMax it uses the native rate table. A +10 item
// can additionally reach +11 at RefineTenChance. Ordinary +9 equipment still
// requires Aylin; equipmentPowderChance handles the equipped-earring exception.
func refinePowderChance(level int, rule model.VolatileRule) (int, bool) {
	limit := rule.RefineMax
	if limit <= 0 || limit > 15 {
		limit = 15
	}
	if level < limit {
		return refineChance(level), true
	}
	if level == 10 && rule.RefineTenChance > 0 {
		return clampInt(rule.RefineTenChance, 0, 100), true
	}
	return 0, false
}

// The requested +9 earring exception is limited to Lac (volatile 5), slot 8
// and the earring catalog mask. W2PP _MSG_UseItem accepts rand()%100 <= 15,
// i.e. 16 outcomes. Keep this server's existing non-destructive failure policy;
// do not import W2PP's destruction or its unrelated +11..+15 extension.
func equipmentPowderChance(item model.Item, def model.ItemDef, destType, destPos uint32,
	powderCode int, rule model.VolatileRule) (int, bool) {
	level := itemSanc(item)
	if level == 9 && powderCode == 5 && rule.RefineMax == 9 &&
		destType == placeEquip && destPos == 8 && def.Pos == 1<<8 {
		return 16, true
	}
	return refinePowderChance(level, rule)
}

func refineChance(sanc int) int {
	if sanc < 0 || sanc >= len(celestialRate) {
		return 0
	}
	return clampInt(celestialRate[sanc], 0, 100)
}

// refineSet ports MolarGargula: deterministic refinement of equipped armor
// slots 1-5, capped at RefineMax=6 without downgrades. Consume only if something
// changed and persist before confirmation, as in Ori/Lac refinement.
func (w *World) refineSet(p *Player, s *net.Session, powder *model.Item, powderSlot byte,
	rule model.VolatileRule, code int) {
	if w.criticalTradeReserved(p) {
		return
	}
	if w.criticalWrites != nil {
		w.admitEquipmentConsumable(p, powderSlot, useItemRequest{}, rule)
		return
	}
	if p == nil || p.Char == nil || p.Char.Score == nil {
		return
	}
	resend := func() {
		if int(powderSlot) < len(p.Char.Inv) {
			s.Send(wire.SendItem(p.ID, placeInv, powderSlot, p.Char.Inv[powderSlot]))
		}
	}
	// Molar is a Mortal quest, not a generic refinement consumable. Internal
	// levels 200..255 display as 201..256 in the 7.48 client. Its negative flag
	// is separate from data-driven quests to prevent reward reuse.
	if rule.MortalOnly && !matchesEvolution(p.Char, "mortal") {
		resend()
		s.Send(wire.MessagePanel("This item is exclusive to Mortals."))
		return
	}
	if rule.RequiredEvolution != "" && !matchesEvolution(p.Char, rule.RequiredEvolution) {
		resend()
		s.Send(wire.MessagePanel("This item cannot be used by this evolution."))
		return
	}
	playerLevel := p.Char.Score.Level
	if (rule.MinLevel != 0 && playerLevel < rule.MinLevel) ||
		(rule.MaxLevelExclusive != 0 && playerLevel >= rule.MaxLevelExclusive) {
		resend()
		s.Send(wire.MessagePanel("This item is only available between levels 201 and 256."))
		return
	}
	if rule.OnceQuestID != 0 && questCompleted(p.Char, int(rule.OnceQuestID)) {
		resend()
		s.Send(wire.MessagePanel("You have already completed this quest."))
		return
	}
	// A Cythera occupies the first armor slot but is not a piece of the mortal
	// armor set. The native 7.54 handler rejects the whole use when it is
	// equipped, rather than silently refining another slot.
	if cythera := p.Char.Equip[1].Index; cythera >= 3500 && cythera <= 3507 {
		resend()
		s.Send(wire.MessagePanel("Cythera cannot be refined by this quest."))
		return
	}

	level := rule.RefineMax
	if level <= 0 || level > 15 {
		level = 6
	}
	snapshot := cloneCharacterState(p.Char)
	oldPowder := snapshot.Inv[powderSlot]
	changed := 0
	for slot := 1; slot <= 5; slot++ {
		it := &p.Char.Equip[slot]
		if it.Index == 0 || itemSanc(*it) >= level {
			continue
		}
		if setItemSanc(it, level) {
			changed++
		}
	}
	if changed == 0 {
		resend()
		s.Send(wire.MessagePanel("No armor piece to refine."))
		return
	}
	if rule.OnceQuestID != 0 {
		markQuestCompleted(p.Char, int(rule.OnceQuestID))
	}
	if rule.Consume {
		consumeOne(powder)
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		*p.Char = snapshot
		log.Printf("[#%d] failed to save refine_set: %v", s.ID, err)
		resend()
		return
	}
	w.recalcPlayer(p.Char)
	resend()
	// Resend each equipped armor slot individually. Per-slot SendItem, rather
	// than full-array SelfEquip, redraws refinement glow without re-equipping.
	for slot := 1; slot <= 5; slot++ {
		if p.Char.Equip[slot].Index != 0 {
			s.Send(wire.SendItem(p.ID, placeEquip, byte(slot), p.Char.Equip[slot]))
		}
	}
	s.Send(playerScorePacket(p))
	w.syncPlayerVitalsToObservers(p)
	// Incremental UpdateEquip carries the refinement glow/mesh.
	w.refreshAppearance(p)
	s.Send(wire.MessagePanel("Set refined!"))
	log.Printf("[#%d] refine_set code=%d powder=%d pieces=%d level=+%d",
		s.ID, code, oldPowder.Index, changed, level)
}

// destRefineTarget resolves the 0x373 DestType/DestPos refinement target.
// Only equipped items are allowed: the 7.48 client's SGrid:2324 permits Ori/Lac
// dragging only onto equipment. Inventory/cargo returns nil; refineItem asks
// the player to equip the target first.
func (w *World) destRefineTarget(p *Player, req useItemRequest) (*model.Item, uint32, uint32) {
	if req.dstType == placeEquip && req.dstPos < model.MaxEquipSlots &&
		clientEquipSlotSupported(byte(req.dstPos)) {
		return &p.Char.Equip[req.dstPos], req.dstType, req.dstPos
	}
	return nil, 0, 0
}

// destItemTarget accepts equipment or inventory for tinting. Exclude cargo:
// the client does not drag a consumable onto storage.
func (w *World) destItemTarget(p *Player, req useItemRequest) (*model.Item, uint32, uint32) {
	switch req.dstType {
	case placeEquip:
		if req.dstPos < model.MaxEquipSlots && clientEquipSlotSupported(byte(req.dstPos)) {
			return &p.Char.Equip[req.dstPos], req.dstType, req.dstPos
		}
	case placeInv:
		if int(req.dstPos) < len(p.Char.Inv) {
			return &p.Char.Inv[req.dstPos], req.dstType, req.dstPos
		}
	}
	return nil, 0, 0
}

// refineItem ports WYD 7.48 Ori/Lac refinement. Accept only equipment; reject
// consumables/EF_NOSANC; respect each powder's cap. Consume on success and
// failure, and persist before confirmation to prevent duplication exploits.
func (w *World) refineItem(p *Player, s *net.Session, powder *model.Item, powderSlot byte,
	req useItemRequest, rule model.VolatileRule, code int) {
	if w.criticalTradeReserved(p) {
		return
	}
	if w.criticalWrites != nil {
		if egg, eggType, eggSlot := w.destEggTarget(p, req); egg != nil {
			w.incubateEgg(p, s, powder, powderSlot, egg, eggType, eggSlot, code)
		} else {
			w.admitEquipmentConsumable(p, powderSlot, req, rule)
		}
		return
	}
	resend := func() { s.Send(wire.SendItem(p.ID, placeInv, powderSlot, *powder)) }

	// Ori/Lac on an equipped egg starts/advances incubation instead of refining.
	// Slot 14 is required and also controls the online-hour counter.
	if egg, eggType, eggSlot := w.destEggTarget(p, req); egg != nil {
		w.incubateEgg(p, s, powder, powderSlot, egg, eggType, eggSlot, code)
		return
	}

	dest, destType, destPos := w.destRefineTarget(p, req)
	if dest == nil {
		// Reject non-equipment destinations: refinement requires an equipped item.
		resend()
		s.Send(wire.MessagePanel("Equip the item to refine it."))
		return
	}
	if dest.Index == 0 {
		resend()
		return
	}
	def, ok := w.items[dest.Index]
	if !ok {
		resend()
		return
	}
	// Require EF_POS equipment without EF_VOLATILE or EF_NOSANC.
	// itemAbility combines definition and instance effects.
	if def.Pos == 0 || refinementLocked(def) || itemAbility(*dest, def, "EF_VOLATILE") != 0 ||
		itemAbility(*dest, def, "EF_NOSANC") != 0 {
		resend()
		s.Send(wire.MessagePanel("That item cannot be refined."))
		return
	}
	sanc := itemSanc(*dest)
	chance, allowed := equipmentPowderChance(*dest, def, destType, destPos, code, rule)
	if !allowed {
		resend()
		s.Send(wire.MessagePanel("That item cannot be refined any further."))
		return
	}

	oldPowder, oldDest := *powder, *dest
	roll := w.rollPercent(chance)
	success := roll.Success
	if success {
		if !setItemSanc(dest, sanc+1) {
			// Without EF_SANC or a free effect pair, reject without consuming
			// powder: do not penalize the player for an invalid target.
			*dest = oldDest
			resend()
			s.Send(wire.MessagePanel("That item cannot be refined."))
			return
		}
	}
	if rule.Consume {
		consumeOne(powder)
	}
	// Persist before confirming. Roll back on failure; never confirm refinement
	// that storage did not retain.
	if err := w.saveAccount(p.Account); err != nil {
		*powder, *dest = oldPowder, oldDest
		log.Printf("[#%d] failed to save refinement powder=%d powder_uid=%q target=%d target_uid=%q consumed=false: %v",
			s.ID, oldPowder.Index, oldPowder.UID, oldDest.Index, oldDest.UID, err)
		resend()
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "refinement", err)
		return
	}
	resend()
	s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
	if destType == placeEquip {
		w.recalcPlayer(p.Char)
		s.Send(playerScorePacket(p))
		w.syncPlayerVitals(p)
		// Incremental UpdateEquip carries the refinement glow/mesh.
		w.refreshAppearance(p)
	}
	s.Send(wire.MessagePanel(roll.message()))
	result := "FAILED"
	if success {
		result = "OK"
	}
	// UID distinguishes otherwise identical units/stacks. Remaining quantity
	// proves whether a valid attempt consumed powder on success or failure.
	remaining := uint32(0)
	if powder.Index != 0 {
		// itemStackAmount returns 1 for unstacked items. Log an empty slot as zero
		// to retain the actual economic outcome in the audit.
		remaining = itemStackAmount(*powder)
	}
	log.Printf("[#%d] refinement code=%d powder=%d powder_uid=%q target=%d target_uid=%q +%d->%d %s consumed=%t remaining=%d roll=%d/%d",
		s.ID, code, oldPowder.Index, oldPowder.UID, oldDest.Index, oldDest.UID,
		sanc, sanc+1, result, rule.Consume, remaining, roll.Roll, roll.Chance)
}
