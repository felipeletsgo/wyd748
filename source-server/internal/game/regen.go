package game

import (
	"time"

	"wydgo/internal/model"
)

// RegenMob aplica MaxHP/MP * EF_REGEN / 120 e soma o piso natural de 10.
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
	restorePlayerHP(ch, regenAmount(playerMaxHP(ch), int(e.RegenHP)))
	restorePlayerMP(ch, regenAmount(playerMaxMP(ch), int(e.RegenMP)))
	return oldHP != playerCurHP(ch) || oldMP != playerCurMP(ch)
}

func (w *World) tickPlayerRegen(now time.Time) {
	for _, p := range w.players {
		if !p.InWorld || p.Char == nil || now.Before(p.NextRegen) {
			continue
		}
		p.NextRegen = now.Add(playerRegenInterval)
		if applyPlayerRegen(p.Char) {
			w.syncPlayerVitals(p)
			w.updatePartyMember(p)
		}
	}
}
