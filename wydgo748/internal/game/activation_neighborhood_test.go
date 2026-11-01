package game

import (
	"math/rand"
	"testing"
)

// W6 item 5 oracle: a pass neighborhood answers exactly as the per-mob cell
// scan, for positions inside its window and for stale positions outside it.
func TestActivationNeighborhoodMatchesCellScan(t *testing.T) {
	for seed := int64(1); seed <= 20; seed++ {
		w, _ := oracleWorld(seed)
		rng := rand.New(rand.NewSource(seed * 13))
		for range 50 {
			center := spatialKey(uint16(1990+rng.Intn(140)), uint16(1990+rng.Intn(140)))
			cx, cy := spatialCellXY(center)
			cr := mobActivationRange/spatialCellSize + 1
			n := &w.activationScratch
			n.reset(cx, cy, cr+mobActivationRange/spatialCellSize+1, w.now())
			for range 40 {
				// Mostly mobs of the recomputed cells; some far away (stale index).
				x := uint16((cx-cr+rng.Intn(2*cr+1))*spatialCellSize + rng.Intn(spatialCellSize))
				y := uint16((cy-cr+rng.Intn(2*cr+1))*spatialCellSize + rng.Intn(spatialCellSize))
				if rng.Intn(5) == 0 {
					x, y = uint16(1960+rng.Intn(200)), uint16(1960+rng.Intn(200))
				}
				got := w.hasNearbyPublicPlayer(n, x, y, mobActivationRange)
				if want := w.hasNearbyPlayerInGameplaySpace(x, y, mobActivationRange, ""); got != want {
					t.Fatalf("seed %d: (%d,%d) near cell (%d,%d) = %t, want %t", seed, x, y, cx, cy, got, want)
				}
			}
		}
	}
}

// After random player moves, activation computed through pass neighborhoods
// equals a full recomputation of every mob.
func TestActivationNeighborhoodPassMatchesFullRecompute(t *testing.T) {
	for seed := int64(1); seed <= 10; seed++ {
		w, _ := oracleWorld(seed)
		rng := rand.New(rand.NewSource(seed * 17))
		for i := range 80 {
			m := combatMob(uint16(1000+i), uint16(1990+rng.Intn(140)), uint16(1990+rng.Intn(140)))
			w.mobs = append(w.mobs, m)
			w.registerMobSpatial(m)
		}
		players := make([]*Player, 0, len(w.players))
		for _, p := range w.players {
			players = append(players, p)
		}
		for range 200 {
			p := players[rng.Intn(len(players))]
			p.X, p.Y = uint16(int(p.X)+rng.Intn(21)-10), uint16(int(p.Y)+rng.Intn(21)-10)
			p.Char.X, p.Char.Y = p.X, p.Y
			w.updatePlayerSpatial(p)
		}
		incremental := make(map[uint16]bool, len(w.mobs))
		for _, m := range w.mobs {
			incremental[m.ID] = m.Awake
		}
		for _, m := range w.mobs {
			w.recomputeMobActive(m)
			if m.Awake != incremental[m.ID] {
				t.Fatalf("seed %d: mob %d at (%d,%d) incremental=%t full=%t", seed, m.ID, m.X, m.Y, incremental[m.ID], m.Awake)
			}
		}
	}
}
