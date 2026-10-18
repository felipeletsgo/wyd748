package game

import (
	"fmt"
	"testing"

	"wydgo/internal/model"
)

func odinLevel40Attempt(t *testing.T, recipe [7]uint16, fame uint32) (*Player, *craftStore) {
	t.Helper()
	w, p, session, st := newCraftWorld(t, "Odin", nil, 0)
	p.Char.Evolution = "celestial"
	p.Char.Score = testScore(model.Score{
		Level: 39, Str: 5, Int: 5, Dex: 5, Con: 5,
		MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100,
	})
	p.SpecialCoins = map[string]uint32{fameCounter: fame}

	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	for i, index := range recipe {
		items[i] = model.Item{Index: index}
		pos[i] = int8(i)
	}
	placeItems(p.Char, items, pos)
	w.onCombineOdin(session, buildCombinePacket(items, pos))
	return p, st
}

func TestOdinCelestialLevel40DoesNotRequireOrConsumeFame(t *testing.T) {
	recipe := [7]uint16{4127, 4127, 5135, 5113, 5129, 5112, 5110}
	for _, fame := range []uint32{0, 1, 199, 200, 500} {
		t.Run(fmt.Sprintf("fame-%d", fame), func(t *testing.T) {
			p, st := odinLevel40Attempt(t, recipe, fame)
			if st.saves != 1 || !p.Char.CelestialLevel40Unlocked {
				t.Fatalf("level-40 recipe saves=%d unlocked=%t", st.saves, p.Char.CelestialLevel40Unlocked)
			}
			if got := counterBalance(p, fameCounter); got != fame {
				t.Fatalf("level-40 recipe changed Fame: got=%d want=%d", got, fame)
			}
			for i := 0; i < 7; i++ {
				if p.Char.Inv[i].Index != 0 {
					t.Fatalf("level-40 recipe did not consume slot %d: %+v", i, p.Char.Inv[i])
				}
			}
		})
	}
}

func TestOdinCelestialLevel40RejectsReorderedIngredients(t *testing.T) {
	// This was the accidental server-only ordering before the regression fix.
	reordered := [7]uint16{5113, 5129, 5112, 5110, 4127, 4127, 5135}
	p, st := odinLevel40Attempt(t, reordered, 200)

	if st.saves != 0 {
		t.Fatalf("reordered level-40 recipe persisted: saves=%d", st.saves)
	}
	if p.Char.CelestialLevel40Unlocked {
		t.Fatal("reordered level-40 recipe unlocked Celestial level 40")
	}
	if got := counterBalance(p, fameCounter); got != 200 {
		t.Fatalf("reordered level-40 recipe consumed Fame: %d", got)
	}
	for i, index := range reordered {
		if p.Char.Inv[i].Index != index {
			t.Fatalf("reordered level-40 recipe consumed slot %d: got=%d want=%d", i, p.Char.Inv[i].Index, index)
		}
	}
}
