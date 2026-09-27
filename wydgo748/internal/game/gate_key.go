package game

import (
	"encoding/binary"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// A key and its gate match by the item's EF_KEYID, not its index:
// First_Gate_Key (451) has EF_VOLATILE 3 and EF_KEYID 2; ground gate (458)
// has EF_GROUND 1 and the same EF_KEYID 2. The 7.48 catalog has 38 items
// with EF_KEYID, including keys and gates.
const (
	gateClosed byte = 0
	gateOpen   byte = 1
	// A gate must be within the same Chebyshev range as an item pickup.
	gateReach = pickupRange
)

// staticEffect reads an item-list effect without applying refinement.
// itemAbility would incorrectly multiply EF_KEYID on a refined item.
func staticEffect(def model.ItemDef, name string) int {
	for _, e := range def.StaticEffects {
		if e.Name == name {
			return e.Value
		}
	}
	return 0
}

// gateByKeyID finds a nearby closed gate matching the key ID. It returns nil
// if there is no matching gate or it is already open; the caller distinguishes
// those cases for its response.
func (w *World) gateByKeyID(p *Player, keyID int) (*GroundItem, bool) {
	var alreadyOpen bool
	for _, g := range w.nearbyGroundItems(p.X, p.Y, gateReach) {
		if !g.Permanent || chebyshev(p.X, p.Y, g.X, g.Y) > gateReach {
			continue
		}
		def, ok := w.items[g.Item.Index]
		if !ok || staticEffect(def, "EF_KEYID") != keyID {
			continue
		}
		if g.State == gateOpen {
			alreadyOpen = true
			continue
		}
		return g, false
	}
	return nil, alreadyOpen
}

// useGateKey opens the matching gate and consumes the key only after the
// account has been persisted, so a storage failure cannot lose the key.
func (w *World) useGateKey(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	def, ok := w.items[item.Index]
	if !ok {
		return
	}
	keyID := staticEffect(def, "EF_KEYID")
	if keyID == 0 {
		log.Printf("[#%d] key %d has no EF_KEYID in the catalog", s.ID, item.Index)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	gate, alreadyOpen := w.gateByKeyID(p, keyID)
	if gate == nil {
		if alreadyOpen {
			s.Send(wire.MessagePanel("This door is already open."))
		} else {
			s.Send(wire.MessagePanel("There is no door for this key here."))
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	w.openGateWithKey(s, p, gate, slot, rule.Consume, code)
}

// openGateWithKey handles both native flows: using the key via 0x373 and
// clicking the object via 0x374. The server finds and consumes the key in
// authoritative inventory; the gate changes only after a successful save.
func (w *World) openGateWithKey(s *net.Session, p *Player, gate *GroundItem, slot byte, consume bool, code int) bool {
	if s == nil || p == nil || p.Char == nil || p.Account == nil || gate == nil ||
		int(slot) >= model.PlayerCarrySlots || gate.State == gateOpen {
		return false
	}
	item := &p.Char.Inv[slot]
	keyDef, keyOK := w.items[item.Index]
	gateDef, gateOK := w.items[gate.Item.Index]
	keyID := staticEffect(keyDef, "EF_KEYID")
	if !keyOK || !gateOK || keyID == 0 || staticEffect(gateDef, "EF_KEYID") != keyID {
		return false
	}

	previous := *item
	if consume {
		consumeOne(item)
	}
	if err := w.saveAccount(p.Account); err != nil {
		*item = previous
		log.Printf("[#%d] failed to save use of key %d: %v", s.ID, previous.Index, err)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return false
	}

	gate.State = gateOpen
	w.applyGroundItemHeight(gate)
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	// The gate belongs to the world, not the player. Everyone seeing it must
	// receive the update; later arrivals receive its state in CreateItem.
	for _, observer := range w.nearbyWorldPlayers(gate.X, gate.Y, viewHalfX) {
		if observer.hasVisible(gate.ID) {
			observer.Session.Send(groundItemUpdatePacket(gate))
		}
	}
	log.Printf("[#%d] opened gate %d (item %d, keyid %d) with key %d volatile=%d",
		s.ID, gate.ID, gate.Item.Index, keyID, previous.Index, code)
	return true
}

func (w *World) gateKeySlot(p *Player, keyID int) int {
	if p == nil || p.Char == nil || keyID == 0 {
		return -1
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		item := p.Char.Inv[slot]
		if def, ok := w.items[item.Index]; ok && staticEffect(def, "EF_KEYID") == keyID {
			return slot
		}
	}
	return -1
}

// onUpdateGroundItem handles MSG_UpdateItem 0x374 (ItemID@12, State@16).
// The client only requests an opening. The world revalidates ID, proximity,
// visibility, gate state, EF_KEYID, and possession of the matching key.
func (w *World) onUpdateGroundItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 20 || playerCurHP(p.Char) == 0 {
		return
	}
	itemIDRaw := binary.LittleEndian.Uint32(pkt[12:16])
	state := int16(binary.LittleEndian.Uint16(pkt[16:18]))
	// The source accepts State 0..5 and transitions a validated gate to OPEN.
	// This field is client intent/visual state, never authoritative gate state.
	if itemIDRaw == 0 || itemIDRaw > uint32(^uint16(0)) || state < 0 || state > 5 {
		w.recordSecurityViolation(s, wire.OpUpdateItem, "invalid object ID or state")
		return
	}
	gate := w.groundItems[uint16(itemIDRaw)]
	if gate == nil || !gate.Permanent || !p.hasVisible(gate.ID) ||
		chebyshev(p.X, p.Y, gate.X, gate.Y) > gateReach {
		w.recordSecurityViolation(s, wire.OpUpdateItem, "object missing or out of reach")
		return
	}
	if gate.State == gateOpen {
		s.Send(groundItemUpdatePacket(gate))
		return
	}
	gateDef, ok := w.items[gate.Item.Index]
	keyID := staticEffect(gateDef, "EF_KEYID")
	if !ok || keyID == 0 {
		w.recordSecurityViolation(s, wire.OpUpdateItem, "object is not a keyed gate")
		return
	}
	slot := w.gateKeySlot(p, keyID)
	if slot < 0 {
		s.Send(wire.MessagePanel("You do not have the required key."))
		s.Send(groundItemUpdatePacket(gate))
		return
	}
	w.openGateWithKey(s, p, gate, byte(slot), true, 3)
}
