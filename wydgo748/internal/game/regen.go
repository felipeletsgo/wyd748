package game

import (
	"time"

	"wydgo/internal/model"
)

// RegenMob applies MaxHP/MP * EF_REGEN / 120 plus the natural floor of 10.
const playerRegenInterval = 3 * time.Second

func regenAmount(max uint32, rate int) uint32 {
	value := uint64(max)*uint64(clampInt(rate, 0, 255))/120 + 10
	if value > uint64(maxScoreValue) {
		return maxScoreValue
	}
	return uint32(value)
}

func applyPlayerRegen(ch *model.Char) bool {
	if ch == nil || playerCurHP(ch) == 0 {
		return false
	}
	oldHP, oldMP := playerCurHP(ch), playerCurMP(ch)
	e := effectiveScore(ch)
	heal := regenAmount(playerMaxHP(ch), int(e.RegenHP))
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
	restorePlayerMP(ch, regenAmount(playerMaxMP(ch), int(e.RegenMP)))
	return oldHP != playerCurHP(ch) || oldMP != playerCurMP(ch)
}

func (w *World) tickPlayerRegen(now time.Time) {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || w.criticalPlayerReserved(p) || now.Before(p.NextRegen) {
			continue
		}
		p.NextRegen = now.Add(playerRegenInterval)
		if applyPlayerRegen(p.Char) {
			w.syncPlayerVitals(p)
			w.updatePartyMember(p)
		}
	}
}
