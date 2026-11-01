package game

import (
	"fmt"
	"math/rand"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

// W6/P7 oracle: the distance-first predicates must select exactly what the
// previous filter order selected, for dead, hidden, instanced and far players.

func referenceHasNearby(w *World, x, y uint16, radius int, space string) bool {
	space = strings.TrimSpace(space)
	now := w.now()
	for _, p := range w.players {
		if validMobTargetAt(p, now) && w.gameplaySpaceForPlayer(p) == space && chebyshev(x, y, p.X, p.Y) <= radius {
			return true
		}
	}
	return false
}

func referenceNearest(w *World, x, y uint16, maxDistance int, space string, accept func(*Player) bool) *Player {
	now := w.now()
	var best *Player
	bestDistance := maxDistance + 1
	for _, p := range w.players {
		if !validMobTargetAt(p, now) || w.gameplaySpaceForPlayer(p) != space || accept != nil && !accept(p) {
			continue
		}
		distance := chebyshev(x, y, p.X, p.Y)
		if distance <= maxDistance && (distance < bestDistance ||
			distance == bestDistance && (best == nil || p.ID < best.ID)) {
			best, bestDistance = p, distance
		}
	}
	return best
}

func oracleWorld(seed int64) (*World, []string) {
	rng := rand.New(rand.NewSource(seed))
	w := testSpatialWorld(nil)
	w.clock = newFakeClock(time.Date(2026, 10, 31, 12, 0, 0, 0, time.UTC))
	w.itemInstances = map[string]*ItemInstance{}
	w.playerInstance = map[uint16]string{}
	spaces := []string{"", "room-a", "room-b"}
	for _, room := range spaces[1:] {
		w.itemInstances[room] = &ItemInstance{}
	}
	for i := range 60 {
		p, session := networkedTestPlayer(uint16(i+1), fmt.Sprintf("Oracle%02d", i), uint16(2000+rng.Intn(120)), uint16(2000+rng.Intn(120)))
		switch rng.Intn(6) {
		case 0:
			setPlayerCurHP(p.Char, 0) // dead
		case 1:
			p.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: w.now().Add(time.Hour)} // hidden
		case 2, 3:
			room := spaces[1+rng.Intn(2)]
			w.itemInstances[room].MemberIDs = append(w.itemInstances[room].MemberIDs, p.ID)
		}
		w.players[session], w.playersByID[p.ID] = p, p
		w.updatePlayerSpatial(p)
	}
	return w, spaces
}

func TestDistanceFirstActivationMatchesReferenceOrder(t *testing.T) {
	for seed := int64(1); seed <= 20; seed++ {
		w, spaces := oracleWorld(seed)
		rng := rand.New(rand.NewSource(seed * 7))
		evenID := func(p *Player) bool { return p.ID%2 == 0 }
		for range 200 {
			x, y := uint16(1980+rng.Intn(160)), uint16(1980+rng.Intn(160))
			radius := rng.Intn(40)
			space := spaces[rng.Intn(len(spaces))]
			if got, want := w.hasNearbyPlayerInGameplaySpace(x, y, radius, space), referenceHasNearby(w, x, y, radius, space); got != want {
				t.Fatalf("seed %d: activation at (%d,%d) r=%d space=%q = %t, want %t", seed, x, y, radius, space, got, want)
			}
			for _, accept := range []func(*Player) bool{nil, evenID} {
				got := w.nearestPlayerMatching(x, y, radius, space, accept)
				if want := referenceNearest(w, x, y, radius, space, accept); got != want {
					t.Fatalf("seed %d: nearest at (%d,%d) r=%d space=%q differs", seed, x, y, radius, space)
				}
			}
		}
	}
}
