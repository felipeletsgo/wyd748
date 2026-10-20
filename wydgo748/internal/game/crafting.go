package game

import (
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	combinePacketSize = 84
	combineSlots      = 8
	tinyGoldCost      = 100_000_000
)

type combineRequest struct {
	Items [combineSlots]model.Item
	Pos   [combineSlots]int8
}

// parseCombineRequest validates the client snapshot against live inventory.
// A packet references a slot with matching eight-byte STRUCT_ITEM contents;
// it cannot select a different authoritative item to consume.
func parseCombineRequest(pkt []byte, ch *model.Char) (combineRequest, error) {
	var req combineRequest
	if len(pkt) != combinePacketSize || ch == nil {
		return req, fmt.Errorf("composition packet has %d bytes", len(pkt))
	}
	used := make(map[int]struct{}, combineSlots)
	for i := 0; i < combineSlots; i++ {
		packetItem := decodeTradeItem(pkt[12+i*8 : 20+i*8])
		req.Pos[i] = int8(pkt[76+i])
		if packetItem.Index == 0 {
			continue
		}
		pos := int(req.Pos[i])
		if pos < 0 || pos >= model.PlayerCarrySlots {
			return combineRequest{}, fmt.Errorf("ingredient %d references slot %d", i, pos)
		}
		if _, duplicate := used[pos]; duplicate {
			return combineRequest{}, fmt.Errorf("duplicate slot %d", pos)
		}
		used[pos] = struct{}{}
		if !ch.Inv[pos].WireEqual(packetItem) {
			return combineRequest{}, fmt.Errorf("ingredient %d differs from inventory", i)
		}
		// The packet proves only the native eight bytes. Retain the complete
		// authoritative instance, including its UID, in the recipe snapshot.
		req.Items[i] = ch.Inv[pos]
	}
	return req, nil
}

func (w *World) combineNPC(p *Player, expectedName string) (*Mob, error) {
	if p == nil || p.Char == nil {
		return nil, fmt.Errorf("player has no composition context")
	}
	// Some 7.48 windows open locally without 0x28B. CraftNPC may be empty or
	// refer to the previous artisan. Reuse only a valid context; otherwise
	// resolve the expected visible artisan in the local grid.
	if p.CraftNPC != 0 {
		if m, err := w.resolveNPCInteraction(p, p.CraftNPC); err == nil && craftingNPCMatches(m, expectedName) {
			return m, nil
		}
	}

	var selected *Mob
	bestDistance := npcInteractionRange + 1
	for _, m := range w.nearbyMobs(p.X, p.Y, npcInteractionRange) {
		if !p.hasVisible(m.ID) || !craftingNPCMatches(m, expectedName) {
			continue
		}
		distance := chebyshev(p.X, p.Y, m.X, m.Y)
		if distance < bestDistance {
			selected, bestDistance = m, distance
		}
	}
	if selected == nil {
		return nil, fmt.Errorf("artisan %s is not visible or within range", expectedName)
	}
	p.CraftNPC = selected.ID
	return selected, nil
}

func craftingNPCMatches(m *Mob, expectedName string) bool {
	return m != nil && !m.Dead && m.Def != nil && m.Def.Score != nil &&
		m.Def.Score.Merchant&0x0F == craftingMerchant &&
		strings.EqualFold(generName(m.Def.Name), expectedName)
}

// tinyDonorTier follows the donor-ID branches in the 7.59 TMSRVIN.exe
// TransWeaponUpgrade routine (0x4DB0C0). This later-server algorithm is a
// project choice for Tiny, not independent proof of native 7.48 parity.
func tinyDonorTier(index uint16) int {
	if index >= 3600 && index <= 3800 {
		return 2
	}
	for _, span := range [][2]uint16{
		{2723, 2726}, {2887, 2890}, {2483, 2486}, {2927, 2930},
		{2663, 2666}, {2843, 2846}, {2603, 2606}, {2543, 2546},
		{2783, 2786}, {2875, 2878},
	} {
		if index >= span[0] && index <= span[1] {
			return 0
		}
	}
	for _, span := range [][2]uint16{
		{2727, 2730}, {2891, 2894}, {2487, 2490}, {2931, 2934},
		{2667, 2670}, {2855, 2858}, {2847, 2850}, {2607, 2610},
		{2547, 2550}, {2787, 2790},
	} {
		if index >= span[0] && index <= span[1] {
			return 1
		}
	}
	return -1
}

func tinyChance(donorID uint16, donorSanc int) int {
	tier := tinyDonorTier(donorID)
	if tier < 0 || donorSanc < 9 {
		return 0
	}
	column := 3
	if donorSanc == 9 {
		column = 1
	} else if donorSanc == 10 {
		column = 2
	}
	// The native table contains thresholds 25/37/42, 29/41/47 and
	// 33/45/53. Its rand()%100 <= threshold adds one effective percent.
	return [3][4]int{{0, 26, 38, 43}, {0, 30, 42, 48}, {0, 34, 46, 54}}[tier][column]
}

func (w *World) tinyRecipe(req combineRequest) (model.Item, int, error) {
	for _, item := range req.Items {
		if item.Index == 747 {
			return model.Item{}, 0, fmt.Errorf("item 747 cannot participate in this composition")
		}
	}
	a, b, catalyst := req.Items[0], req.Items[1], req.Items[2]
	defA, okA := w.items[a.Index]
	defB, okB := w.items[b.Index]
	defC, okC := w.items[catalyst.Index]
	if a.Index == 0 || b.Index == 0 || catalyst.Index == 0 || !okA || !okB || !okC {
		return model.Item{}, 0, fmt.Errorf("three primary ingredients are required")
	}
	if itemAbility(a, defA, "EF_MOBTYPE") != 1 || defA.Grade < 5 || defA.Grade > 8 {
		return model.Item{}, 0, fmt.Errorf("main item is not a grade 5..8 Arch weapon")
	}
	if tinyDonorTier(b.Index) < 0 || defB.Grade < 5 || defB.Grade > 8 {
		return model.Item{}, 0, fmt.Errorf("invalid donor weapon")
	}
	if itemSanc(b) < 9 {
		return model.Item{}, 0, fmt.Errorf("donor weapon requires at least +9 refinement")
	}
	if itemAbility(catalyst, defC, "EF_ITEMTYPE") == 1 {
		return model.Item{}, 0, fmt.Errorf("prohibited catalyst type")
	}
	if defA.Pos != defB.Pos || defA.Pos != 64 && defA.Pos != 192 {
		return model.Item{}, 0, fmt.Errorf("weapons must occupy the same hand")
	}
	if itemSanc(a) != 9 || itemSanc(catalyst) != 9 {
		return model.Item{}, 0, fmt.Errorf("main item and catalyst require +9 refinement")
	}
	result := a
	result.Eff = b.Eff
	if !setItemSanc(&result, 7) {
		return model.Item{}, 0, fmt.Errorf("donor weapon effects cannot hold refinement")
	}
	return result, tinyChance(b.Index, itemSanc(b)), nil
}

func (w *World) sendCombineResult(p *Player, result uint32) {
	w.sendCombineResultMessage(p, result, "")
}

func (w *World) sendCombineResultMessage(p *Player, result uint32, message string) {
	if p != nil && p.Session != nil {
		// Send text before MSG_CombineComplete (0x3A7), which updates/closes
		// the native window. Display the persisted percentage decision;
		// never roll again merely to produce a message.
		if message == "" {
			message = combineResultMessage(result)
		}
		if message != "" {
			p.Session.Send(wire.MessagePanel(message))
		}
		p.Session.Send(wire.StandardParm(wire.OpCombineComplete, wire.SceneField, result))
	}
}

func combineResultMessage(result uint32) string {
	switch result {
	case 0:
		return "Wrong combination."
	case 1:
		return "Processing complete."
	case 2:
		return "The combination failed."
	default:
		return ""
	}
}

func (w *World) onCombineTiny(s *net.Session, pkt []byte) {
	if w.criticalTradeReserved(w.players[s]) {
		return
	}
	p, req, ok := w.beginCombine(s, pkt, "Tiny")
	if !ok {
		return
	}
	resultItem, chance, err := w.tinyRecipe(req)
	if err != nil || p.Char.Gold < tinyGoldCost {
		w.sendCombineResult(p, 0)
		return
	}

	if w.criticalWrites != nil {
		roll := w.rollPercent(chance)
		if err := w.queueTinyCraft(p, req, resultItem, roll); err != nil {
			log.Printf("[#%d] Tiny composition admission rejected: %v", s.ID, err)
			s.Send(wire.MessagePanel("Composition is unavailable. Please try again later."))
		}
		return // Rejected admission must never fall through to an inline save.
	}

	w.cancelTrade(p, "Tiny composition")
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	roll := w.rollPercent(chance)
	success := roll.Success
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 2, combineSlots-1, changed)
	if success {
		mainPos, donorPos := int(req.Pos[0]), int(req.Pos[1])
		p.Char.Inv[mainPos] = resultItem
		p.Char.Inv[donorPos] = model.Item{}
		changed[mainPos], changed[donorPos] = struct{}{}, struct{}{}
	}
	p.Char.Gold -= tinyGoldCost
	code := uint32(2)
	if success {
		code = 1
	}
	if w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll) {
		logCraftResult(s, roll, p.Char.Gold)
	}
}

func logCraftResult(s *net.Session, roll percentRoll, gold uint32) {
	if s == nil {
		return
	}
	status := "failure"
	if roll.Success {
		status = "success"
	}
	// Keep operational records concise; do not dump STRUCT_ITEM effect bytes.
	log.Printf("[#%d] CRAFT Tiny %s roll=%d/%d gold=%d",
		s.ID, status, roll.Roll, roll.Chance, gold)
}
