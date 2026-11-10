package game

import (
	"time"

	"wydgo/internal/model"
)

// Native 7.59 runs recovery every 16 half-second timer passes.
const playerRegenInterval = 8 * time.Second

func naturalRecovery(attribute byte) uint32 {
	switch {
	case attribute&0x20 != 0:
		return 1000
	case attribute&0x01 != 0:
		return 500
	default:
		return 200
	}
}

func regenAmount(base, index uint32) uint32 {
	// The native score stores the index in units of four points.
	return base + index/4*4
}

func applyPlayerRegen(ch *model.Char, base uint32) bool {
	if ch == nil || playerCurHP(ch) == 0 {
		return false
	}
	oldHP, oldMP := playerCurHP(ch), playerCurMP(ch)
	e := effectiveScore(ch)
	if oldHP >= e.MaxHP && oldMP >= e.MaxMP {
		return false
	}
	heal := regenAmount(base, e.RegenHP)
	// Hide multiplies the normal HP recovery, not maximum HP. Expired
	// affects are removed by the World before its regeneration tick.
	if ch.Class == 3 {
		for _, affect := range ch.Affects {
			if affect.Type == 28 {
				heal = uint32(min(uint64(maxScoreValue), uint64(heal)*uint64(110+playerMastery(ch, 1)/10)/100))
				break
			}
		}
	}
	restorePlayerHP(ch, heal)
	restorePlayerMP(ch, regenAmount(base, e.RegenMP))
	return oldHP != playerCurHP(ch) || oldMP != playerCurMP(ch)
}

func (w *World) tickPlayerRegen(now time.Time) {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || w.criticalPlayerReserved(p) || now.Before(p.NextRegen) {
			continue
		}
		p.NextRegen = now.Add(playerRegenInterval)
		base := naturalRecovery(w.terrain.AttributeAt(p.X, p.Y))
		if applyPlayerRegen(p.Char, base) {
			w.syncPlayerVitals(p)
			w.updatePartyMember(p)
		}
	}
}
