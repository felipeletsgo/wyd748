package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func prepareOdinRecipe(t *testing.T, recipe []uint16, definitions map[uint16]model.ItemDef) (*World, *Player, *net.Session, [combineSlots]model.Item, [combineSlots]int8) {
	t.Helper()
	w, p, session, _ := newCraftWorld(t, "Odin", definitions, 0)
	var items [combineSlots]model.Item
	var positions [combineSlots]int8
	for i, index := range recipe {
		items[i] = model.Item{Index: index}
		positions[i] = int8(i)
	}
	placeItems(p.Char, items, positions)
	return w, p, session, items, positions
}

func TestOdinCrystalRecipeAndCapeUpgradeCommit(t *testing.T) {
	t.Run("crystal recipe", func(t *testing.T) {
		w, p, session, items, positions := prepareOdinRecipe(t,
			[]uint16{421, 422, 423, 424, 425, 426, 427}, map[uint16]model.ItemDef{4032: {Index: 4032}})
		w.onCombineOdin(session, buildCombinePacket(items, positions))
		if p.Char.Inv[0].Index != 4032 || w.store.(*craftStore).saves != 1 {
			t.Fatalf("crystal output=%d saves=%d", p.Char.Inv[0].Index, w.store.(*craftStore).saves)
		}
		for i := 1; i < 7; i++ {
			if p.Char.Inv[i].Index != 0 {
				t.Fatalf("crystal recipe kept ingredient slot %d", i)
			}
		}
	})

	t.Run("celestial cape refinement", func(t *testing.T) {
		w, p, session, items, positions := prepareOdinRecipe(t,
			[]uint16{4127, 4127, 5135, 413, 413, 413, 413}, nil)
		p.Char.Evolution = "celestial"
		p.Char.Score = testScore(model.Score{Level: 120, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100})
		p.Char.Equip[15] = model.Item{Index: 3197, Eff: [6]byte{43, 0}}
		w.onCombineOdin(session, buildCombinePacket(items, positions))
		if itemSanc(p.Char.Equip[15]) != 1 || w.store.(*craftStore).saves != 1 {
			t.Fatalf("cape refinement=%d saves=%d", itemSanc(p.Char.Equip[15]), w.store.(*craftStore).saves)
		}
		for i := 0; i < 7; i++ {
			if p.Char.Inv[i].Index != 0 {
				t.Fatalf("cape recipe kept ingredient slot %d", i)
			}
		}
	})
}

func TestOdinCelestialLevel40RecipeRollsBackOnSaveFailure(t *testing.T) {
	recipe := []uint16{4127, 4127, 5135, 5113, 5129, 5112, 5110}
	w, p, session, items, positions := prepareOdinRecipe(t, recipe, nil)
	p.Char.Evolution = "celestial"
	p.Char.Score = testScore(model.Score{Level: 39, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100})
	st := w.store.(*craftStore)
	st.err = errors.New("database unavailable")

	w.onCombineOdin(session, buildCombinePacket(items, positions))

	if p.Char.CelestialLevel40Unlocked || st.saves != 1 {
		t.Fatalf("failed unlock changed flag or save count: unlocked=%v saves=%d", p.Char.CelestialLevel40Unlocked, st.saves)
	}
	for i, index := range recipe {
		if p.Char.Inv[i].Index != index {
			t.Fatalf("failed unlock changed ingredient slot %d: got=%d want=%d", i, p.Char.Inv[i].Index, index)
		}
	}
}

func TestOdinBlessedRefinementSuccessFailureAndInvalidCatalyst(t *testing.T) {
	for _, tc := range []struct {
		name      string
		rng       RNG
		catalyst  uint16
		wantLevel int
		wantSaves int
	}{
		{name: "success with catalyst", rng: fixedRNG{value: 0}, catalyst: 3338, wantLevel: 12, wantSaves: 1},
		{name: "failed roll", rng: fixedRNG{value: 99}, wantLevel: 10, wantSaves: 1},
		{name: "invalid catalyst", rng: fixedRNG{value: 0}, catalyst: 999, wantLevel: 11},
	} {
		t.Run(tc.name, func(t *testing.T) {
			recipe := []uint16{4043, 4043, 500, tc.catalyst}
			w, p, session, items, positions := prepareOdinRecipe(t, recipe,
				map[uint16]model.ItemDef{500: {Index: 500}})
			w.rng = tc.rng
			items[2].Eff = [6]byte{43, 234} // +11
			if tc.catalyst == 3338 {
				items[3].Eff = [6]byte{43, 3}
			}
			p.Char.Inv[2] = items[2]
			p.Char.Inv[3] = items[3]
			packet := buildCombinePacket(items, positions)
			w.onCombineOdin(session, packet)
			if got := itemSanc(p.Char.Inv[2]); got != tc.wantLevel || w.store.(*craftStore).saves != tc.wantSaves {
				t.Fatalf("refine level=%d saves=%d, want level=%d saves=%d", got, w.store.(*craftStore).saves, tc.wantLevel, tc.wantSaves)
			}
			if tc.wantSaves == 1 && (p.Char.Inv[0].Index != 0 || p.Char.Inv[1].Index != 0) {
				t.Fatal("committed refinement kept blessed scrolls")
			}
			if tc.wantSaves == 0 && (p.Char.Inv[0].Index != 4043 || p.Char.Inv[1].Index != 4043 || p.Char.Inv[3].Index != 999) {
				t.Fatal("invalid catalyst changed ingredients")
			}
		})
	}
}
