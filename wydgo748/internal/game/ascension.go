package game

import (
	"fmt"
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// ascension.go implements the Mortal -> Arch chain. It was historically
// imported from a source that no longer exists; that provenance does not prove
// native 7.48 parity. Check current contracts against the tests.
//
// There are three chained steps, each at a different NPC:
//
//  1. Sefirot       (class skill master): 8 Sephira stones + 30M gold.
//  2. Eternal Stone (Black_Oracle): adjacent Unicon + Phoenix + 10 sapphires.
//  3. Arch          (king): Mortal 370+, Stone in slot 10 and Sefirot in 11.
//
// The Arch is born as a NEW character in the first free slot; the Mortal still
// exists and remains playable, exactly as natively.

const (
	// Chain items, all checked against data/itemlist.csv.
	unicornSpiritItem = 1740 // Unicon's_Spirit
	phoenixSpiritItem = 1741 // Phoenix's_Spirit
	eternalStoneItem  = 1742 // Eternal_Stone (the "Immortality Stone")
	// sefirotFirstItem+class: 1760 TK, 1761 FM, 1762 BM, 1763 HT.
	sefirotFirstItem = 1760
	// sephiraFirstStone..+7: Kocuna, Veean, Keced, Gebra, Tipalet, Nechark,
	// Yesord, Marcute.
	sephiraFirstStone = 1744
	sephiraStoneCount = 8

	sefirotGoldCost       = 30_000_000
	eternalStoneSapphires = 10

	// archMinLevel is the INTERNAL level the king requires. The WYD 7.48 source
	// uses 380, but community guides differ (355, 370, 300) because each server
	// calibrates its own gate; 370 is this server's rule and matches "when a
	// mortal reaches level 370-400 it can be reborn".
	archMinLevel = 370

	// Equipment slots the king uses for ascension.
	eternalStoneSlot = 10
	sefirotSlot      = 11

	// archFaceOffset ports `Equip[0] = MortalFace + 5 + cls`. With this
	// catalog's faces (TK 1, FM 11, BM 21, HT 31) it yields 6, 17, 28 and 39,
	// all existing and of the right class.
	archFaceOffset = 5

	// archEvolution is the value stored in Char.Evolution. Any non-empty value
	// already excludes the character from MortalOnly rules (quests, volatiles).
	archEvolution = "arch"
)

// sefirotClassForNPC resolves the class from the SKILL MASTER's NAME.
//
// The Sefirot compositor is the class skill master, not a dedicated NPC.
// Confirmed in game (the client opens the confirmation window at Foema_Ancian)
// and in the goldvivi source ("Skills Teacher (Armia town) - creates Sephiroth").
//
// Each master's class was identified by the SKILL RANGE it sells (index
// 5000 + skill), an objective criterion:
//
//	Knight_Leader  5000..5023 -> skills 0..23   -> TK
//	Foema_Ancian   5024..5047 -> skills 24..47  -> FM
//	Archi_Master   5048..5071 -> skills 48..71  -> BM
//	ForeLearner    5072..5095 -> skills 72..95  -> HT
//
// Natively the mob Exp field (994..997) distinguishes them; these JSON files
// do not keep that discriminator, so the name is the reliable source, the same
// criterion the kings use in kingdom.go.
func sefirotClassForNPC(def *model.NPCDef) (int, bool) {
	if def == nil {
		return 0, false
	}
	switch strings.ToLower(generName(def.Name)) {
	case "knight leader":
		return 0, true
	case "foema ancian":
		return 1, true
	case "archi master":
		return 2, true
	case "forelearner", "fore learner":
		return 3, true
	}
	return 0, false
}

// hasSephiraStones reports whether all eight stones are in the inventory. It
// GATES the click: without them, clicking the master is an ordinary skill-shop
// request and must not become a composition attempt.
func hasSephiraStones(ch *model.Char) bool {
	return findSephiraStones(ch) != nil
}

func isBlackOracle(def *model.NPCDef) bool {
	return def != nil && strings.EqualFold(generName(def.Name), "Black Oracle")
}

// handleAscensionNPC handles Black_Oracle (Eternal Stone). The Sefirot does
// NOT pass here: its compositor is a skill master whose click must be
// intercepted BEFORE the shop opens; see onNPCClick.
func (w *World) handleAscensionNPC(s *net.Session, p *Player, m *Mob) bool {
	if p == nil || p.Char == nil || m == nil || m.Def == nil {
		return false
	}
	if isBlackOracle(m.Def) {
		w.craftEternalStone(s, p)
		return true
	}
	return false
}

// findSephiraStones returns the slots of the 8 stones (one of each index), or
// nil when any is missing. One stone per type, as in the native loop.
func findSephiraStones(ch *model.Char) []int {
	slots := make([]int, 0, sephiraStoneCount)
	for offset := 0; offset < sephiraStoneCount; offset++ {
		wanted := uint16(sephiraFirstStone + offset)
		found := -1
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			if ch.Inv[slot].Index == wanted {
				found = slot
				break
			}
		}
		if found < 0 {
			return nil
		}
		slots = append(slots, found)
	}
	return slots
}

// craftSefirot ports COMP_SEPHI: eight Sephira stones and 30 million gold
// become the Sefirot of the NPC's class.
func (w *World) craftSefirot(s *net.Session, p *Player, class int) {
	stones := findSephiraStones(p.Char)
	if stones == nil {
		s.Send(wire.MessagePanel("Bring the eight Sephira stones."))
		return
	}
	if p.Char.Gold < sefirotGoldCost {
		s.Send(wire.MessagePanel("Eight Sephira stones and 30,000,000 gold are required."))
		return
	}
	sefirot := model.Item{Index: uint16(sefirotFirstItem + class)}
	// Check room BEFORE consuming: the stones free slots, so room is assured,
	// but the check keeps the "validate everything, then mutate" order.
	if freeInventorySlots(p.Char) == 0 && len(stones) == 0 {
		s.Send(wire.MessagePanel("Inventory is full."))
		return
	}

	images := criticalPreimages(p)
	for _, slot := range stones {
		p.Char.Inv[slot] = model.Item{}
	}
	p.Char.Gold -= sefirotGoldCost
	target := addToInv(p.Char, sefirot)
	if target < 0 {
		w.restoreCriticalPreimages(images)
		s.Send(wire.MessagePanel("Inventory is full."))
		return
	}
	w.commitPlayerMutation(p, images, "Sefirot", "Save failed. Nothing was consumed.", false, func() {
		if !s.IsClosed() {
			for _, slot := range stones {
				s.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
			}
			s.Send(wire.SendItem(p.ID, placeInv, byte(target), p.Char.Inv[target]))
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			s.Send(wire.MessagePanel("The Sefirot has been created."))
		}
		log.Printf("[#%d] Sefirot created class=%d item=%d", s.ID, class, sefirot.Index)
	})
}

// findAdjacentSpirits finds Unicon (1740) followed by Phoenix (1741) in the
// IMMEDIATELY following slot. Adjacency is a native requirement, not an
// implementation detail: it is how the player assembles the pair.
func findAdjacentSpirits(ch *model.Char) (int, bool) {
	for slot := 0; slot+1 < model.PlayerCarrySlots; slot++ {
		if ch.Inv[slot].Index == unicornSpiritItem && ch.Inv[slot+1].Index == phoenixSpiritItem {
			return slot, true
		}
	}
	return 0, false
}

// craftEternalStone ports BLACKORACLE: the two adjacent spirits plus ten
// sapphires become the Eternal Stone.
func (w *World) craftEternalStone(s *net.Session, p *Player) {
	slot, ok := findAdjacentSpirits(p.Char)
	if !ok {
		s.Send(wire.MessagePanel("Place Unicon's Spirit and Phoenix's Spirit side by side, in that order."))
		return
	}
	if sapphireCount(p.Char) < eternalStoneSapphires {
		s.Send(wire.MessagePanel("10 Sapphires are required."))
		return
	}

	images := criticalPreimages(p)
	if !consumeSapphires(p.Char, eternalStoneSapphires) {
		w.restoreCriticalPreimages(images)
		s.Send(wire.MessagePanel("The Sapphires could not be consumed."))
		return
	}
	p.Char.Inv[slot] = model.Item{}
	p.Char.Inv[slot+1] = model.Item{}
	target := addToInv(p.Char, model.Item{Index: eternalStoneItem})
	if target < 0 {
		w.restoreCriticalPreimages(images)
		s.Send(wire.MessagePanel("Inventory is full."))
		return
	}
	w.commitPlayerMutation(p, images, "Eternal Stone", "Save failed. Nothing was consumed.", false, func() {
		if !s.IsClosed() {
			w.resendInventory(p)
			s.Send(wire.MessagePanel("The Eternal Stone has been forged."))
		}
		log.Printf("[#%d] Eternal Stone forged slot=%d", s.ID, target)
	})
}

// resendInventory resends the whole inventory. Compositions that touch many
// slots at once (loose sapphires and packs) use it, where listing every
// changed slot would cost more and be more fragile than a refresh.
func (w *World) resendInventory(p *Player) {
	if p == nil || p.Char == nil || p.Session == nil {
		return
	}
	p.Session.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
}

// archClassFromSefirot returns the Arch class from the equipped Sefirot.
func archClassFromSefirot(item model.Item) (int, bool) {
	class := int(item.Index) - sefirotFirstItem
	if class < 0 || class > 3 {
		return 0, false
	}
	return class, true
}

// freeCharacterSlot returns the account's first empty slot, like the native
// DBSrv loop, or -1 when there is no room.
func freeCharacterSlot(account *model.Account) int {
	for slot := 0; slot < maxCharactersPerAccount; slot++ {
		if slot >= len(account.Chars) || account.Chars[slot].Name == "" {
			return slot
		}
	}
	return -1
}

// growCharacterSlots grows Account.Chars to contain the requested slot and
// REPOINTS p.Char.
//
// append may REALLOCATE the array. Since p.Char is `&p.Account.Chars[CharSlot]`
// (handlers.go), the pointer would be orphaned in the old array: every later
// write (item consumption, rollback and then the whole session) would go to
// memory nobody persists while saveAccount writes the new array. During
// ascension that left the Stone and Sefirot INTACT on disk, making ascension
// repeatable.
func (w *World) growCharacterSlots(p *Player, slot int) {
	if p == nil || p.Account == nil {
		return
	}
	for len(p.Account.Chars) <= slot {
		p.Account.Chars = append(p.Account.Chars, model.Char{})
	}
	// Repoint even without reallocation: it is cheap and does not depend on
	// guessing whether append grew the array.
	if p.CharSlot >= 0 && p.CharSlot < len(p.Account.Chars) {
		p.Char = &p.Account.Chars[p.CharSlot]
	}
}

// createArch ports Arch creation at the king. It returns true when this path
// handled the NPC, including rejections, so the king does not continue and
// try to grant a kingdom cape on the same click.
//
// The Mortal STAYS on the account: the Arch is a NEW character with the same
// name in the first free slot. That is the native behavior.
func (w *World) createArch(s *net.Session, p *Player) bool {
	stone := p.Char.Equip[eternalStoneSlot]
	sefirot := p.Char.Equip[sefirotSlot]
	// Without both items equipped the player is not asking for ascension: let
	// the click continue to the kingdom logic.
	if stone.Index != eternalStoneItem {
		return false
	}
	class, ok := archClassFromSefirot(sefirot)
	if !ok {
		return false
	}

	if strings.TrimSpace(p.Char.Evolution) != "" {
		s.Send(wire.MessagePanel("Only a Mortal can ascend."))
		return true
	}
	if p.Char.Score == nil || p.Char.Score.Level < archMinLevel {
		s.Send(wire.MessagePanel("You must be level 371 to ascend."))
		return true
	}
	slot := freeCharacterSlot(p.Account)
	if slot < 0 {
		s.Send(wire.MessagePanel("There is no room for a new character on this account."))
		return true
	}

	// The Arch inherits the Mortal's NAME (as natively), so it SKIPS the
	// uniqueness check: the name already belongs to this account.
	arch, built := w.newCharacterFromTemplate(p.Char.Name, class)
	if !built {
		log.Printf("[#%d] missing class layout %d for ascension", s.ID, class)
		s.Send(wire.MessagePanel("The Arch could not be created."))
		return true
	}
	arch.Evolution = archEvolution
	// Record the origin by stable identity. The slot may be deleted or reused
	// and must never make an Arch inherit another character's level.
	arch.ArchMortalUID = p.Char.UID
	arch.ArchMortalLevel = p.Char.Score.Level
	// Arch face = Mortal face + 5 + class (CFileDB.cpp:1993).
	arch.Equip[0].Index = p.Char.Equip[0].Index + archFaceOffset + uint16(class)

	if w.criticalWrites != nil {
		// Grow empty slots before the pre-image: growCharacterSlots repoints
		// p.Char, and the proposal must match the pre-image length.
		w.growCharacterSlots(p, slot)
		images := criticalPreimages(p)
		p.Account.Chars[slot] = arch
		p.Char.Equip[eternalStoneSlot] = model.Item{}
		p.Char.Equip[sefirotSlot] = model.Item{}
		w.commitPlayerMutation(p, images, "Arch ascension", "Save failed. The ascension was not completed.", false, func() {
			if !s.IsClosed() {
				s.Send(wire.SendItem(p.ID, placeEquip, eternalStoneSlot, p.Char.Equip[eternalStoneSlot]))
				s.Send(wire.SendItem(p.ID, placeEquip, sefirotSlot, p.Char.Equip[sefirotSlot]))
				s.Send(wire.MessagePanel("Your Arch has been created. It awaits at character selection."))
			}
			announcement := fmt.Sprintf("The character %s has become a God!", arch.Name)
			w.broadcast(func() []byte { return wire.MessageWhisper(0, "[SERVER]", announcement, 7) })
			log.Printf("[#%d] Arch created name=%q class=%d slot=%d face=%d (mortal level=%d)",
				s.ID, arch.Name, class, slot, arch.Equip[0].Index, p.Char.Score.Level)
			w.returnToCharacterSelectionAfterCommittedChange(p, "Arch creation")
		})
		return true
	}
	previousChars := append([]model.Char(nil), p.Account.Chars...)
	previousEquip := p.Char.Equip
	// Grow the slice BEFORE any write through p.Char: growCharacterSlots
	// repoints the pointer that append could orphan.
	w.growCharacterSlots(p, slot)
	p.Account.Chars[slot] = arch
	// Ascension consumes the Stone and the Sefirot.
	p.Char.Equip[eternalStoneSlot] = model.Item{}
	p.Char.Equip[sefirotSlot] = model.Item{}

	if err := w.saveAccount(p.Account); err != nil {
		// Replacing the slice also invalidates p.Char: repoint BEFORE restoring
		// equipment, or the rollback writes to the discarded array.
		p.Account.Chars = previousChars
		if p.CharSlot >= 0 && p.CharSlot < len(p.Account.Chars) {
			p.Char = &p.Account.Chars[p.CharSlot]
		}
		p.Char.Equip = previousEquip
		log.Printf("[#%d] ERROR saving ascension of %q: %v", s.ID, p.Char.Name, err)
		s.Send(wire.MessagePanel("Save failed. The ascension was not completed."))
		return true
	}

	s.Send(wire.SendItem(p.ID, placeEquip, eternalStoneSlot, p.Char.Equip[eternalStoneSlot]))
	s.Send(wire.SendItem(p.ID, placeEquip, sefirotSlot, p.Char.Equip[sefirotSlot]))
	s.Send(wire.MessagePanel("Your Arch has been created. It awaits at character selection."))
	// World announcement AFTER the save: a failed ascension is never announced.
	// Same channel and color as /spk ([SERVER], color 7). "God" is the English
	// term for the Arch used by the global client and sources.
	announcement := fmt.Sprintf("The character %s has become a God!", arch.Name)
	w.broadcast(func() []byte { return wire.MessageWhisper(0, "[SERVER]", announcement, 7) })
	log.Printf("[#%d] Arch created name=%q class=%d slot=%d face=%d (mortal level=%d)",
		s.ID, arch.Name, class, slot, arch.Equip[0].Index, p.Char.Score.Level)
	w.returnToCharacterSelectionAfterCommittedChange(p, "Arch creation")
	return true
}

// isArch reports whether the character already ascended.
func isArch(ch *model.Char) bool {
	return ch != nil && strings.EqualFold(ch.Evolution, archEvolution)
}

// refreshArchMortalLevel updates, at login, the origin Mortal level of each
// Arch on the account.
//
// Natively this value is recalculated on every entry (ProcessDBMessage.cpp:1002),
// which lets the Arch keep gaining points as the Mortal levels after
// ascension. A deleted Mortal keeps the last known value instead of resetting
// points already distributed.
func refreshArchMortalLevel(account *model.Account) bool {
	if account == nil {
		return false
	}
	changed := false
	for i := range account.Chars {
		arch := &account.Chars[i]
		if arch.Name == "" || !isArch(arch) {
			continue
		}
		if arch.ArchMortalUID == "" || arch.ArchMortalUID == arch.UID {
			continue
		}
		var mortal *model.Char
		for j := range account.Chars {
			candidate := &account.Chars[j]
			if j != i && candidate.UID == arch.ArchMortalUID {
				mortal = candidate
				break
			}
		}
		if mortal == nil || mortal.Name == "" || mortal.Score == nil {
			continue
		}
		if arch.ArchMortalLevel != mortal.Score.Level {
			arch.ArchMortalLevel = mortal.Score.Level
			changed = true
		}
	}
	return changed
}
