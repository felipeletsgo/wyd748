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
	// Minimum internal level, using the king's archMinLevel convention.
	// Historical _MSG_UseItem.cpp:6362 requires BaseScore.Level >= 355;
	// the client text also specifies an ascension requirement of level 355.
	//
	// Check the minimum before deducting experience. Checking it afterwards
	// would reject a valid oath because its own cost lowers the level.
	archCrystalMinLevel = uint32(355)
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
		s.Send(wire.MessagePanel("Only a God can use this crystal."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.Score.Level < archCrystalMinLevel {
		s.Send(wire.MessagePanel("You must reach level 355 to take this oath."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.ArchCrystals >= passo {
		s.Send(wire.MessagePanel("You have already done it."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if ch.ArchCrystals != passo-1 {
		s.Send(wire.MessagePanel("You must take the oaths in order."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	// Guard unsigned underflow even if edited state or a changed level table
	// violates the usual minimum-level experience assumptions. Wrapping a
	// uint32 subtraction would reward experience instead of charging it.
	if ch.Exp < archCrystalExpCost {
		s.Send(wire.MessagePanel("You need 100 million experience for this oath."))
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	anteriorItem, anteriorExp := *item, ch.Exp
	anteriorNivel, anteriorCristais := ch.Score.Level, ch.ArchCrystals

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
		*item, ch.Exp = anteriorItem, anteriorExp
		ch.Score.Level, ch.ArchCrystals = anteriorNivel, anteriorCristais
		syncProgression(ch)
		w.recalcPlayer(ch)
		s.Send(wire.MessagePanel("Save failed. The oath was not taken."))
		log.Printf("[#%d] ERROR saving crystal %d: %v", s.ID, anteriorItem.Index, err)
		return
	}

	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	// Score publication follows the source/stock ABI negotiated at login.
	s.Send(playerScorePacketForChar(p, ch))
	s.Send(wire.UpdateEtc(p.ID, *ch))
	w.syncPlayerVitalsToObservers(p)
	if ch.ArchCrystals == archCrystalCount {
		s.Send(wire.MessagePanel("The four oaths are complete."))
	}
	log.Printf("[#%d] CRYSTAL %d (%d/%d) item=%d volatile=%d exp=%d->%d level=%d->%d",
		s.ID, passo, ch.ArchCrystals, archCrystalCount, anteriorItem.Index, code,
		anteriorExp, ch.Exp, anteriorNivel, ch.Score.Level)
}
