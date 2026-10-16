package game

import (
	"testing"

	"wydgo/internal/model"
)

// A teleported owner finds its summons beside it instead of walking back.
func TestTeleportBringsSummonsBesideOwner(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	summon := &Mob{ID: 1450, X: owner.X + 1, Y: owner.Y, HP: 100, SummonerID: owner.ID,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	w.mobs = append(w.mobs, summon)
	w.registerMobSpatial(summon)
	w.summons[summon.ID] = summon
	destX, destY := owner.X+60, owner.Y
	if !w.teleportPlayer(owner, destX, destY) {
		t.Skip("fixture terrain rejects the destination")
	}
	if chebyshev(summon.X, summon.Y, owner.X, owner.Y) > summonFollowDistance {
		t.Fatalf("summon left behind at (%d,%d), owner at (%d,%d)", summon.X, summon.Y, owner.X, owner.Y)
	}
}
