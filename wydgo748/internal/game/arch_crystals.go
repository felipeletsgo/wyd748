package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The four Kefra elemental crystals originate from volatile 187 in the
// historical source (_MSG_UseItem.cpp:7184). Each Arch oath adds 100 points
// to the later Celestial budget. They are not an ascension prerequisite;
// progress is stored in QuestInfo.Arch.Cristal.
//
// Historical-source divergence: its condition tests ClassMaster CELESTIAL.
// This project gates on ARCH because that branch also requires level 355,
// beyond the 7.54 Celestial progression's internal cap of 199. Its Arch field
// and later point-budget use support treating this as Arch progress rather
// than an ascension gate; this is not independent native 7.48 parity proof.
const (
	firstArchCrystal = uint16(4106) // Elime, Sylphid, Salion, Nohas
	archCrystalCount = 4
	// Each crystal costs experience. Recalculate the level from the reduced
	// experience; the resulting level decrease is intentional.
	archCrystalExpCost = uint32(100_000_000)
	// Native 7.59 checks displayed level 355 and EXP before deducting the cost.
	archCrystalMinLevel = uint32(354)
	archCrystalMinExp   = uint32(1_950_000_000)
)

// archCrystalStep returns the crystal's position in the sequence (1..4).
func archCrystalStep(index uint16) (byte, bool) {
	if index < firstArchCrystal || index >= firstArchCrystal+archCrystalCount {
		return 0, false
	}
	return byte(index-firstArchCrystal) + 1, true
}

// levelForExp finds the level for an experience total. It supports level
// decreases after a deduction, unlike grantExp, which only increases levels.
func levelForExp(exp uint32) uint32 {
	level := uint32(0)
	for i := 1; i < len(mortalNextLevel); i++ {
		if exp < mortalNextLevel[i] {
			break
		}
		level = uint32(i)
	}
	if level > maxMortalLevel {
		level = maxMortalLevel
	}
	return level
}

// useArchCrystal consumes an elemental crystal in sequence: crystal N
// requires all N-1 preceding oaths.
func (w *World) useArchCrystal(s *net.Session, p *Player, item *model.Item, slot byte, rule model.VolatileRule, code int) {
	ch := p.Char
	passo, ok := archCrystalStep(item.Index)
	if !ok {
		log.Printf("[#%d] crystal %d outside range %d..%d", s.ID, item.Index,
			firstArchCrystal, firstArchCrystal+archCrystalCount-1)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if !isArch(ch) {
		s.Send(wire.MessagePanel(w.msg("_WG_Only_God_Can_Use_Crystal")))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.Score == nil || ch.Score.Level < archCrystalMinLevel {
		s.Send(wire.MessagePanel(w.msg("_WG_Must_Reach_Level_Take_Oath")))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.ArchCrystals >= passo {
		s.Send(wire.MessagePanel(w.msg("_NN_Youve_Done_It_Already")))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.ArchCrystals != passo-1 {
		s.Send(wire.MessagePanel(w.msg("_WG_Must_Take_Oaths_Order")))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.Exp < archCrystalMinExp {
		s.Send(wire.MessagePanel(w.msg("_WG_Need_Million_Experience_Oath")))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	snapshot := cloneCharacterState(ch)

	ch.ArchCrystals = passo
	ch.Exp -= archCrystalExpCost
	ch.Score.Level = levelForExp(ch.Exp)
	syncProgression(ch)
	if rule.Consume {
		consumeOne(item)
	}
	w.recalcPlayer(ch)

	// Persist before confirming this costly oath; restore all state on failure.
	if err := w.saveAccount(p.Account); err != nil {
		*ch = snapshot
		s.Send(wire.MessagePanel(w.msg("_WG_Save_Failed_Oath_Was_Not_Taken")))
		log.Printf("[#%d] ERROR saving crystal %d: %v", s.ID, item.Index, err)
		return
	}

	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	// Score publication follows the source/stock ABI negotiated at login.
	s.Send(playerScorePacketForChar(p, ch))
	s.Send(wire.UpdateEtc(p.ID, *ch))
	w.syncPlayerVitalsToObservers(p)
	if ch.ArchCrystals == archCrystalCount {
		s.Send(wire.MessagePanel(w.msg("_WG_Four_Oaths_Complete")))
	}
	log.Printf("[#%d] CRYSTAL %d (%d/%d) item=%d volatile=%d exp=%d->%d level=%d->%d",
		s.ID, passo, ch.ArchCrystals, archCrystalCount, snapshot.Inv[slot].Index, code,
		snapshot.Exp, ch.Exp, snapshot.Score.Level, ch.Score.Level)
}
