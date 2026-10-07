package game

import (
	"encoding/binary"
	"log"
	"sort"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// Hell Gate store, 0x2BD (24 bytes): TargetID@12 (u16), option@14 (i16),
// MyCarryPos@16 (i16), Coin@20 (ignored). The native 7.48 client opens the
// store when an NPC with head 51 is clicked inside the Hell Gate
// (FUN_0045ee28, FUN_0044a3a9) and sends the request after the confirmation
// box (FUN_004640e5). UI/hellStoredesc.txt states the rules: a random item of
// the chosen category, random options, no refinement, unlimited purchases.
// The item pools and the option rules below are the server's reconstruction
// (owner decision, 2026-10-03); see DOCS/protocol/hell-gate-store.md.
const (
	hellStorePacketSize = 24
	hellStoreNPCName    = "Hell_Store"

	hellStoreWeapon = 1
	hellStoreArmor  = 2
	hellStoreAmulet = 3
	hellStoreRandom = 4

	hellStoreSancEffect   = 43 // EF_SANC
	hellStoreUniqueEffect = 59 // EF_UNIQUE
)

var hellStorePrices = map[int]uint64{
	hellStoreWeapon: 9_000_000,
	hellStoreArmor:  6_000_000,
	hellStoreAmulet: 5_000_000,
	hellStoreRandom: 7_000_000,
}

func isHellStoreNPC(m *Mob) bool {
	return m != nil && m.Def != nil && m.Def.Name == hellStoreNPCName
}

// hellStorePool lists the catalog items of one category in ascending order.
// Weapons tier 9-10 and armor tier 7-8 are the base-line items (below index
// 1800) with EF_ITEMLEVEL 4 that have an ancient version (Extra); ancients
// themselves are excluded. Amulets tier 1-3 are Silver, Gold and Mystic
// (551-562).
func hellStorePool(items map[uint16]model.ItemDef, category int) []uint16 {
	pool := make([]uint16, 0, 128)
	for index, def := range items {
		if hellStoreCategoryAccepts(def, category) {
			pool = append(pool, index)
		}
	}
	sort.Slice(pool, func(i, j int) bool { return pool[i] < pool[j] })
	return pool
}

func hellStoreCategoryAccepts(def model.ItemDef, category int) bool {
	baseLine := def.Extra != 0 && staticAbility(def, "EF_ITEMLEVEL") == 4
	switch category {
	case hellStoreWeapon:
		return baseLine && def.Index >= 800 && def.Index < 950 && (def.Pos == 64 || def.Pos == 192)
	case hellStoreArmor:
		switch def.Pos {
		case 2, 4, 8, 16, 32:
			return baseLine && def.Index >= 1000 && def.Index < 1800 &&
				staticAbility(def, "EF_MOBTYPE") == 0
		}
		return false
	case hellStoreAmulet:
		return def.Index >= 551 && def.Index <= 562
	default:
		return false
	}
}

// hellStoreWeaponOption is one entry of the weapon branch of the native drop
// generator SetItemBonus (positions 64/192) at level difference 0.
type hellStoreWeaponOption struct {
	effect     byte
	multiplier int
	adjust     int
}

func hellStoreWeaponChoice(roll int, magic, second bool) hellStoreWeaponOption {
	if magic {
		switch roll {
		case 0:
			return hellStoreWeaponOption{60, 4, -1} // EF_MAGIC
		case 1:
			return hellStoreWeaponOption{74, 3, 1} // EF_SPECIALALL
		}
		return hellStoreWeaponOption{hellStoreUniqueEffect, 0, 0}
	}
	switch roll {
	case 0:
		return hellStoreWeaponOption{26, 3, 1} // EF_ATTSPEED
	case 1:
		return hellStoreWeaponOption{2, 9, -1} // EF_DAMAGE
	case 2:
		if second {
			return hellStoreWeaponOption{74, 3, 1}
		}
		return hellStoreWeaponOption{74, 3, 0}
	}
	return hellStoreWeaponOption{hellStoreUniqueEffect, 0, 0}
}

// hellStoreWeaponGrade is the level-difference-0 grade table of SetItemBonus.
func hellStoreWeaponGrade(roll int) int {
	switch {
	case roll < 2:
		return 4
	case roll < 6:
		return 3
	case roll < 24:
		return 2
	case roll < 55:
		return 1
	default:
		return 0
	}
}

func (w *World) hellStoreWeaponSlot(def model.ItemDef, second bool) (byte, byte) {
	magic := def.Unique == 44 || def.Unique == 47
	choiceRoll := w.intn(101) % 8
	if second {
		choiceRoll = w.intn(100) % 8
	}
	option := hellStoreWeaponChoice(choiceRoll, magic, second)
	grade := hellStoreWeaponGrade(w.intn(100)) + option.adjust
	if grade <= 0 {
		return hellStoreUniqueEffect, byte(w.intn(128))
	}
	return option.effect, byte(option.multiplier * grade)
}

// hellStoreOptions rolls the instance effects. Every item keeps refinement at
// +0. Armor draws from the native SetItemBonus2 pools also used by Repliction;
// weapons use the SetItemBonus weapon branch; amulets get no added options, as
// SetItemBonus skips accessories.
func (w *World) hellStoreOptions(def model.ItemDef, category int) [6]byte {
	switch category {
	case hellStoreWeapon:
		e1, v1 := w.hellStoreWeaponSlot(def, false)
		e2, v2 := w.hellStoreWeaponSlot(def, true)
		return [6]byte{hellStoreSancEffect, 0, e1, v1, e2, v2}
	case hellStoreArmor:
		pool := w.volatiles.Repliction.Pools[def.Pos]
		if len(pool) == 0 {
			return [6]byte{hellStoreSancEffect, 0}
		}
		bonus := pool[w.intn(len(pool))]
		return [6]byte{hellStoreSancEffect, 0, byte(bonus.Effect1), byte(bonus.Value1),
			byte(bonus.Effect2), byte(bonus.Value2)}
	default:
		return [6]byte{}
	}
}

func (w *World) rollHellStoreItem(option int) (model.Item, bool) {
	category := option
	if option == hellStoreRandom {
		category = hellStoreWeapon + w.intn(3)
	}
	pool := hellStorePool(w.items, category)
	if len(pool) == 0 {
		return model.Item{}, false
	}
	def := w.items[pool[w.intn(len(pool))]]
	return model.Item{Index: def.Index, Eff: w.hellStoreOptions(def, category)}, true
}

func (w *World) onHellStoreBuy(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld || w.shuttingDown ||
		p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) ||
		playerCurHP(p.Char) == 0 || len(pkt) != hellStorePacketSize {
		return
	}
	header := wire.ParseHeader(pkt)
	if header.ID != p.ID {
		w.recordSecurityViolation(s, header.Type, "session id mismatch")
		return
	}
	if p.Trade != nil && w.criticalPlayerReserved(w.playerByID(p.Trade.OpponentID)) {
		return
	}
	w.cancelTrade(p, "Hell Gate store purchase")
	targetID := binary.LittleEndian.Uint16(pkt[12:14])
	option := int(int16(binary.LittleEndian.Uint16(pkt[14:16])))
	dst := int(int16(binary.LittleEndian.Uint16(pkt[16:18])))

	m, err := w.resolveNPCInteraction(p, targetID)
	if err != nil {
		s.Send(wire.MessagePanel(npcInteractionMessage(err)))
		return
	}
	if !isHellStoreNPC(m) {
		s.Send(wire.MessagePanel("That character is not the Hell Gate merchant."))
		return
	}
	price, ok := hellStorePrices[option]
	if !ok {
		s.Send(wire.MessagePanel("Choose an item category first."))
		return
	}
	if dst < 0 || dst >= model.PlayerCarrySlots || p.Char.Inv[dst].Index != 0 {
		s.Send(wire.MessagePanel("Choose an empty inventory slot."))
		return
	}
	if uint64(p.Char.Gold) < price {
		s.Send(wire.MessagePanel("You do not have enough gold."))
		return
	}
	rolled, ok := w.rollHellStoreItem(option)
	if !ok {
		s.Send(wire.MessagePanel("The Hell Gate store has nothing to sell."))
		return
	}
	item, err := materializeItem(rolled)
	if err != nil {
		log.Printf("[#%d] Hell Gate store item %d could not be created: %v", s.ID, rolled.Index, err)
		s.Send(wire.MessagePanel("The purchase could not be completed."))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueHellStorePurchase(p, item, dst, price, option); err != nil {
			log.Printf("[#%d] submit Hell Gate store purchase of item %d: %v", s.ID, item.Index, err)
			s.Send(wire.MessagePanel("The purchase could not be submitted. Please try again."))
		}
		return
	}
	if err := w.commitTicketPurchase(p, item, dst, price, 0, 0); err != nil {
		log.Printf("[#%d] Hell Gate store purchase of item %d failed: %v", s.ID, item.Index, err)
		s.Send(wire.MessagePanel("The purchase could not be completed."))
		return
	}
	publishHellStorePurchase(p, option, item, price, dst)
}

func publishHellStorePurchase(p *Player, option int, item model.Item, price uint64, dst int) {
	if p.Session.IsClosed() {
		return
	}
	p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	log.Printf("[#%d] Hell Gate store option %d: item %d eff=%v for %d gold -> inv[%d]",
		p.Session.ID, option, item.Index, item.Eff, price, dst)
}
