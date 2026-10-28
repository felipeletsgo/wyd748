package game

import (
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const aylinGoldCost = 50_000_000

// agathaBaseChance ports g_pAgathaBase (Basedef.cpp:156 in the WYD 7.48 source,
// default 20, configurable there through a text file). It is fixed here, like
// the other crafting costs in this file. The chance is base + grade*5 + level
// bonus (GetMatchCombineAgatha, GetFunc.cpp:564-628), typically 30-50%, not
// guaranteed success.
const agathaBaseChance = 20

// blockedCombineItem is Atila's_Crown. Native composition rejects it in any
// of the eight slots (GetFunc.cpp:55-59 and equivalent checks at lines 218,
// 263, 337, 400, 461, 505, 634, 669 and 740).
const blockedCombineItem = 747

func (w *World) beginCombine(s *net.Session, pkt []byte, npc string) (*Player, combineRequest, bool) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
		return nil, combineRequest{}, false
	}
	now := w.now()
	if !p.LastCraft.IsZero() && now.Sub(p.LastCraft) < 800*time.Millisecond {
		s.Send(wire.MessagePanel("Wait a second before trying again."))
		return p, combineRequest{}, false
	}
	p.LastCraft = now
	if _, err := w.combineNPC(p, npc); err != nil {
		w.sendCombineResult(p, 0)
		return p, combineRequest{}, false
	}
	req, err := parseCombineRequest(pkt, p.Char)
	if err != nil {
		w.sendCombineResult(p, 0)
		return p, combineRequest{}, false
	}
	for _, item := range req.Items {
		if item.Index == blockedCombineItem {
			w.sendCombineResult(p, 0)
			return p, combineRequest{}, false
		}
	}
	return p, req, true
}

func setItemSancRaw(item *model.Item, raw byte) bool {
	if item == nil {
		return false
	}
	for i := 0; i < 3; i++ {
		if effect := item.Eff[i*2]; effect == 43 || effect >= 116 && effect <= 125 {
			item.Eff[i*2+1] = raw
			return true
		}
	}
	return false
}

func (w *World) commitCombine(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{}, equipSlots map[int]struct{}, result uint32) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, nil, "")
}

func (w *World) commitCombineRoll(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, roll percentRoll) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, nil, roll.message())
}

func (w *World) commitCombineWithPlayerState(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccountAndCharStateResult(p) }, nil, "")
}

func (w *World) commitCombineWithRollback(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, rollback func()) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, rollback, "")
}

func (w *World) commitCombineWithRollbackRoll(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, rollback func(), roll percentRoll) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, rollback, roll.message())
}

func (w *World) commitCombineWithSave(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, persist func() error, rollback func(), resultMessage string) bool {
	if err := persist(); err != nil {
		p.Char.Inv, p.Char.Equip, p.Char.Gold = oldInv, oldEquip, oldGold
		if rollback != nil {
			rollback()
		}
		// Equipment compositions recalculate runtime stats before persistence.
		// Restoring only the array left authoritative stats derived from the
		// item whose database write failed.
		if len(equipSlots) != 0 && p.Char.Score != nil {
			w.recalcPlayer(p.Char)
		}
		for pos := range invSlots {
			p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
		}
		for pos := range equipSlots {
			p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(pos), p.Char.Equip[pos]))
		}
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.sendCombineResultMessage(p, 0, "Save failed. Reconnect to reload the authoritative state.")
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "composition", err)
		return false
	}
	for pos := range invSlots {
		p.Session.Send(wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
	}
	for pos := range equipSlots {
		p.Session.Send(wire.SendItem(p.ID, placeEquip, byte(pos), p.Char.Equip[pos]))
	}
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	if len(equipSlots) != 0 {
		// Equipment changes score and must be published incrementally; never
		// recreate an already visible character with CreateMob.
		w.syncPlayerScoreAndVitals(p)
	}
	w.sendCombineResultMessage(p, result, resultMessage)
	return true
}

func consumeCombineItems(ch *model.Char, req combineRequest, from, to int, changed map[int]struct{}) {
	for i := from; i <= to && i < combineSlots; i++ {
		if req.Items[i].Index == 0 {
			continue
		}
		pos := int(req.Pos[i])
		ch.Inv[pos] = model.Item{}
		changed[pos] = struct{}{}
	}
}

// compositorMaterials is the exact material count: neither three nor five.
const compositorMaterials = 4

// compositorChance adds each material's bonus to the configured base. Each of
// the four materials must be equippable, from set D (item level 4) or E (5),
// and refined to +7..+9; refinement determines the bonus.
//
// Item Grade is not included. WYD 7.48 has two competing routines: the
// (level, grade, refinement) table in GetMatchCombine used by the binary, and
// the refinement-indexed g_pAnctChance[3] (Basedef.cpp:158), read from
// CompRate.txt. This server uses the second policy. In the legacy source it is
// unused twice over: no function reads the array, and the parser applies
// _strupr only to the first column, so "Item_+7" never matches "ITEM_+7".
// Values here come from this server's data/config/server.txt, not that legacy array.
//
// Return the chance, per-material log breakdown and recipe validity.
func (w *World) compositorChance(req combineRequest) (int, string, bool) {
	chance := int(w.gameplay.CompositorBaseChance)
	detail := make([]string, 0, compositorMaterials)
	// The 7.48 window has four material slots at 2..5. Accepting four items
	// scattered across 2..7 allowed layouts the legitimate client never sends.
	for i := 2; i < 2+compositorMaterials; i++ {
		item := req.Items[i]
		if item.Index == 0 {
			return 0, "", false
		}
		def, exists := w.items[item.Index]
		level, sanc := itemAbility(item, def, "EF_ITEMLEVEL"), itemSanc(item)
		refine := sanc - model.CompositorMinRefine
		if !exists || def.Pos == 0 || level < 4 || level > 5 ||
			refine < 0 || refine >= model.CompositorRefineLevels {
			return 0, "", false
		}
		bonus := int(w.gameplay.CompositorRefineChance[refine])
		chance += bonus
		detail = append(detail, fmt.Sprintf("%d(set%c+%d:%d)",
			item.Index, "DE"[level-4], sanc, bonus))
	}
	if req.Items[6].Index != 0 || req.Items[7].Index != 0 ||
		len(detail) != compositorMaterials {
		return 0, "", false
	}
	return chance, strings.Join(detail, " "), true
}

func (w *World) compositorRecipe(req combineRequest) (model.Item, int, string, bool) {
	target, jewel := req.Items[0], req.Items[1]
	targetDef, targetOK := w.items[target.Index]
	if !targetOK || targetDef.Unique < 41 || targetDef.Unique > 49 || targetDef.Extra <= 0 ||
		itemAbility(target, targetDef, "EF_MOBTYPE") == 3 || itemAbility(target, targetDef, "EF_ITEMLEVEL") <= 3 ||
		jewel.Index < 2441 || jewel.Index > 2444 || itemStackAmount(jewel) > 1 {
		return model.Item{}, 0, "", false
	}
	chance, breakdown, chanceOK := w.compositorChance(req)
	if !chanceOK {
		return model.Item{}, 0, "", false
	}
	resultIndex := uint16(targetDef.Extra + int(jewel.Index-2441))
	if _, resultExists := w.items[resultIndex]; !resultExists {
		return model.Item{}, 0, "", false
	}
	// Transform the base weapon while retaining the player's additional effects.
	resultItem := target
	resultItem.Index = resultIndex
	if !setItemSanc(&resultItem, 7) {
		return model.Item{}, 0, "", false
	}
	return resultItem, chance, breakdown, true
}

func (w *World) onCombineCompositor(s *net.Session, pkt []byte) {
	if w.criticalTradeReserved(w.players[s]) {
		return
	}
	p, req, ok := w.beginCombine(s, pkt, "Compositor")
	if !ok {
		return
	}
	resultItem, chance, breakdown, valid := w.compositorRecipe(req)
	if !valid {
		w.sendCombineResult(p, 0)
		return
	}
	if w.criticalWrites != nil {
		w.admitInventoryComposition(p, req, resultItem, w.rollPercent(chance), "Compositor")
		return
	}
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 0, 7, changed)
	roll := w.rollPercent(chance)
	success := roll.Success
	if success {
		pos := int(req.Pos[0])
		p.Char.Inv[pos] = resultItem
		changed[pos] = struct{}{}
	}
	code := uint32(2)
	if success {
		code = 1
	}
	if w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll) {
		log.Printf("[#%d] CRAFT Compositor success=%t roll=%d/%d (base %d + %s)",
			s.ID, success, roll.Roll, roll.Chance, w.gameplay.CompositorBaseChance, breakdown)
	}
}

func (w *World) agathaRecipe(req combineRequest) (model.Item, int, bool) {
	a, donor := req.Items[0], req.Items[1]
	defA, okA := w.items[a.Index]
	defB, okB := w.items[donor.Index]
	donorLevel := itemAbility(donor, defB, "EF_ITEMLEVEL")
	validPos := defA.Pos >= 2 && defA.Pos <= 32 && defA.Pos&(defA.Pos-1) == 0
	if !okA || !okB || itemAbility(a, defA, "EF_MOBTYPE") != 1 ||
		itemAbility(donor, defB, "EF_ITEMTYPE") != 0 || defA.Pos != defB.Pos || !validPos ||
		itemSanc(a) < 9 || itemSanc(donor) < 9 || donorLevel < 4 {
		return model.Item{}, 0, false
	}
	for i := 2; i <= 5; i++ {
		if req.Items[i].Index != 3140 || itemStackAmount(req.Items[i]) > 1 {
			return model.Item{}, 0, false
		}
	}
	if req.Items[6].Index != 0 || req.Items[7].Index != 0 {
		return model.Item{}, 0, false
	}
	result := a
	result.Eff = donor.Eff
	if !setItemSanc(&result, 7) {
		return model.Item{}, 0, false
	}
	// GetMatchCombineAgatha (GetFunc.cpp:620-627) uses the donor's grade, with
	// level 5 + grade 1 treated as grade 3; bonus +6 at level 5, otherwise +1.
	// Return the raw chance; the legacy source compares it against a 0..99 roll.
	grade := defB.Grade
	if donorLevel == 5 && grade == 1 {
		grade = 3
	}
	bonus := 1
	if donorLevel == 5 {
		bonus = 6
	}
	chance := agathaBaseChance + grade*5 + bonus
	return result, chance, true
}

func (w *World) onCombineAgatha(s *net.Session, pkt []byte) {
	if w.criticalTradeReserved(w.players[s]) {
		return
	}
	p, req, ok := w.beginCombine(s, pkt, "Agatha")
	if !ok {
		return
	}
	result, chance, valid := w.agathaRecipe(req)
	if !valid {
		w.sendCombineResult(p, 0)
		return
	}
	if w.criticalWrites != nil {
		w.admitInventoryComposition(p, req, result, w.rollPercent(chance), "Agatha")
		return
	}
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 2, 5, changed)
	mainPos := int(req.Pos[0])
	p.Char.Inv[mainPos] = model.Item{}
	changed[mainPos] = struct{}{}
	roll := w.rollPercent(chance)
	success := roll.Success
	if success {
		donorPos := int(req.Pos[1])
		p.Char.Inv[mainPos] = result
		p.Char.Inv[donorPos] = model.Item{}
		changed[donorPos] = struct{}{}
	}
	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll)
}

func (w *World) aylinRecipe(req combineRequest) (model.Item, int, bool) {
	result, reason := w.aylinRecipeResult(req)
	return result, 40, reason == ""
}

// aylinRecipeResult ports W2PP GetMatchCombineAilyn: two identical +9 items
// of the same grade, Light Stone 1774 and four gems. Ancient grades 5..8
// require their own gem (Diamond 2441, Emerald 2442, Coral 2443, Garnet
// 2444); any other item takes four equal gems. WYD-Go also requires a weapon
// to be Ancient, so two normal weapons cannot reach +10. A Cythera (circlet,
// slot 2) follows the armor rule. The reason is shown to the player.
func (w *World) aylinRecipeResult(req combineRequest) (model.Item, string) {
	a, b := req.Items[0], req.Items[1]
	defA, okA := w.items[a.Index]
	defB, okB := w.items[b.Index]
	validPos := defA.Pos == 2 || defA.Pos == 4 || defA.Pos == 8 || defA.Pos == 16 ||
		defA.Pos == 32 || defA.Pos == 64 || defA.Pos == 128 || defA.Pos == 192
	if !okA || !okB || a.Index != b.Index || defA.Grade != defB.Grade || !validPos {
		return model.Item{}, "Place two identical +9 items."
	}
	if itemSanc(a) != 9 || itemSanc(b) != 9 {
		return model.Item{}, "Both items must be refined to +9."
	}
	ancient := defA.Grade >= 5 && defA.Grade <= 8
	if (defA.Pos == 64 || defA.Pos == 192) && !ancient {
		return model.Item{}, "Only Ancient weapons can be composed to +10."
	}
	if req.Items[2].Index != 1774 || itemStackAmount(req.Items[2]) > 1 || req.Items[7].Index != 0 {
		return model.Item{}, "A Light Stone is required."
	}
	gem := req.Items[3].Index
	if ancient {
		gem = uint16(2441 + defA.Grade - 5)
	}
	for i := 3; i <= 6; i++ {
		if req.Items[i].Index < 2441 || req.Items[i].Index > 2444 || req.Items[i].Index != gem ||
			itemStackAmount(req.Items[i]) > 1 {
			if ancient {
				return model.Item{}, "This Ancient item requires four " + aylinGemNames[gem-2441] + " gems."
			}
			return model.Item{}, "Four equal gems are required."
		}
	}
	result := a
	if !setItemSancRaw(&result, byte(230+gem-2441)) {
		return model.Item{}, "This item cannot be composed."
	}
	return result, ""
}

var aylinGemNames = [4]string{"Diamond", "Emerald", "Coral", "Garnet"}

func (w *World) onCombineAylin(s *net.Session, pkt []byte) {
	if w.criticalTradeReserved(w.players[s]) {
		return
	}
	p, req, ok := w.beginCombine(s, pkt, "Aylin")
	if !ok {
		return
	}
	result, chance, valid := w.aylinRecipe(req)
	if !valid {
		w.sendCombineResult(p, 0)
		_, reason := w.aylinRecipeResult(req)
		s.Send(wire.MessagePanel(reason))
		return
	}
	if p.Char.Gold < aylinGoldCost {
		w.sendCombineResult(p, 0)
		s.Send(wire.MessagePanel("Aylin charges 50,000,000 gold."))
		return
	}
	if w.criticalWrites != nil {
		w.admitInventoryComposition(p, req, result, w.rollPercent(chance), "Aylin")
		return
	}
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 2, 6, changed)
	// Server policy uses rolls 1..100 and succeeds when roll <= chance.
	// Aylin therefore has exactly 40%, without the legacy 41st successful value.
	roll := w.rollPercent(40)
	success := roll.Success
	if success {
		mainPos, donorPos := int(req.Pos[0]), int(req.Pos[1])
		p.Char.Inv[mainPos] = result
		p.Char.Inv[donorPos] = model.Item{}
		changed[mainPos], changed[donorPos] = struct{}{}, struct{}{}
	}
	p.Char.Gold -= aylinGoldCost
	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll)
}

func (w *World) admitInventoryComposition(p *Player, req combineRequest, result model.Item, roll percentRoll, artisan string) {
	if err := w.queueInventoryComposition(p, req, result, roll, artisan); err != nil {
		log.Printf("[#%d] %s composition admission rejected: %v", p.Session.ID, artisan, err)
		p.Session.Send(wire.MessagePanel("Composition is unavailable. Please try again later."))
	}
}

func (w *World) onCombineLindy(s *net.Session, pkt []byte) {
	if w.criticalTradeReserved(w.players[s]) {
		return
	}
	p, req, ok := w.beginCombine(s, pkt, "Lindy")
	if !ok {
		return
	}
	// The implemented contract uses two item-3448 packages with EF_AMOUNT=10.
	// Do not accept item-413 stacks as a variant: keep the recipe coordinated
	// with the client. Historical origin in another version is not native
	// 7.48 parity evidence.
	if !lindyMaterialStack(req.Items[0]) || !lindyMaterialStack(req.Items[1]) ||
		req.Items[2].Index != 4127 {
		w.sendCombineResult(p, 0)
		s.Send(wire.MessagePanel("Two Laktorerium packages (10 units each) and one Sealed Scroll are required."))
		return
	}
	for i := 3; i <= 6; i++ {
		if req.Items[i].Index != 413 || itemStackAmount(req.Items[i]) > 1 {
			w.sendCombineResult(p, 0)
			s.Send(wire.MessagePanel("Four Laktorerium powders are required."))
			return
		}
	}
	// Only an Arch at one of the two locked levels can use this recipe. The
	// WYD 7.48 handler returns before consumption for any other evolution,
	// level or already unlocked limit.
	trava, destrava := lindyLevelUnlock(p.Char)
	if !destrava {
		w.sendCombineResult(p, 0)
		return
	}
	// _MSG_CombineItemLindy: unlocking 355 neither checks nor consumes Fame.
	// Only unlocking 370 costs Fame, exactly one point.
	const lindyLevel370FameCost = uint32(1)
	if trava == archLockLevel370 && counterBalance(p, fameCounter) < lindyLevel370FameCost {
		s.Send(wire.MessagePanel("You need 1 fame point."))
		w.sendCombineResult(p, 0)
		return
	}

	var cape model.Item
	if trava == archLockLevel355 {
		// _MSG_CombineItemLindy selects the Elite cape by current Clan: 3191
		// Hekalotia, 3192 Akelonia, 3193 neutral. Kingdom still comes from the
		// old cape here, so capture it before replacement.
		capeIndex := lindyCapeIndex(p.Char)
		cape = model.Item{Index: capeIndex, UID: p.Char.Equip[15].UID, Eff: [6]byte{54, 16}}
		var err error
		cape, err = materializeItem(cape)
		if err != nil {
			w.sendCombineResult(p, 0)
			return
		}
		if _, exists := w.items[cape.Index]; !exists {
			w.sendCombineResult(p, 0)
			return
		}
	}

	if w.criticalWrites != nil {
		if err := w.queueLindyCraft(p, req, cape); err != nil {
			log.Printf("[#%d] Lindy composition admission rejected: %v", s.ID, err)
			s.Send(wire.MessagePanel("Composition is unavailable. Please try again later."))
		}
		return
	}

	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	oldFame := copyCounters(p)
	old355, old370 := p.Char.ArchLevel355, p.Char.ArchLevel370
	changedInv := make(map[int]struct{}, 7)
	changedEquip := make(map[int]struct{}, 1)
	consumeCombineItems(p.Char, req, 0, 6, changedInv)
	// Only unlocking 355 creates the Elite cape. At 370, native behavior
	// changes the flag/Fame while leaving the equipped cape intact.
	if trava == archLockLevel355 {
		p.Char.Equip[15] = cape
		changedEquip[15] = struct{}{}
	}
	if trava == archLockLevel355 {
		p.Char.ArchLevel355 = true
	} else {
		p.Char.ArchLevel370 = true
	}
	if trava == archLockLevel370 {
		spendCounters(p, map[string]uint32{fameCounter: lindyLevel370FameCost})
	}
	if len(changedEquip) != 0 {
		w.recalcPlayer(p.Char)
	}
	// Unlocking 355 changes only account/inventory/equipment. Unlocking 370
	// also debits the Fame sidecar and needs the joint PostgreSQL transaction.
	persisted := false
	if trava == archLockLevel370 {
		persisted = w.commitCombineWithPlayerState(
			p, oldInv, oldEquip, oldGold, changedInv, changedEquip, 1)
	} else {
		persisted = w.commitCombine(
			p, oldInv, oldEquip, oldGold, changedInv, changedEquip, 1)
	}
	if !persisted {
		// commitCombine restored inventory/equipment; restore the other fields.
		p.SpecialCoins = oldFame
		p.Char.ArchLevel355, p.Char.ArchLevel370 = old355, old370
		w.recalcPlayer(p.Char)
		return
	}
	s.Send(wire.MessagePanel("Your level limit has been lifted."))
	log.Printf("[#%d] ARCH unlocked level %d (fame=%d)", s.ID, trava+1, counterBalance(p, fameCounter))
}

func lindyMaterialStack(item model.Item) bool {
	return item.Index == 3448 && itemStackAmount(item) == 10
}

// lindyLevelUnlock returns the character's locked limit, if any. Only an Arch
// exactly at a still-locked limit qualifies; consuming the recipe elsewhere
// must not set an unlock flag.
func lindyLevelUnlock(ch *model.Char) (uint32, bool) {
	if ch == nil || ch.Score == nil || !isArch(ch) {
		return 0, false
	}
	switch ch.Score.Level {
	case archLockLevel355:
		return archLockLevel355, !ch.ArchLevel355
	case archLockLevel370:
		return archLockLevel370, !ch.ArchLevel370
	}
	return 0, false
}

func lindyCapeIndex(ch *model.Char) uint16 {
	cape, _ := model.KingdomCapeAtTier(model.CapeTierElite, characterKingdom(ch))
	return cape
}
