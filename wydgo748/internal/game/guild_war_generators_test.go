package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestNoatumTowersCannotSpawnThroughOrdinaryGenerators(t *testing.T) {
	// Keep the import guard covered without retaining obsolete general spawns.
	for i, start := range []model.GenerSegment{{X: 1051, Y: 1690}, {X: 1046, Y: 1690}, {X: 1124, Y: 1708}} {
		def := model.NPCGener{
			Index: i, Leader: "Tower_of_Thor", MinuteGenerate: -1, MaxNumMob: 1,
			Segments: [model.MaxGenerSegments]model.GenerSegment{start},
		}
		if !generatorReservedForGuildWar(def) {
			t.Fatalf("Noatum tower at (%d,%d) was not reserved", start.X, start.Y)
		}
		for _, day := range []int{14, 18} { // Wednesday and Sunday: neither is a castle-war lifecycle.
			w := newZoneTestWorld()
			g := &generState{def: def}
			w.scheduleGenerator(g, time.Date(2026, 10, day, 21, 6, 0, 0, time.UTC))
			w.spawnGroup(g) // A reserved generator must not even dereference its template.
			if g.current != 0 || !g.nextGenerate.IsZero() || len(w.mobs) != 0 {
				t.Fatalf("ordinary generator %d created/scheduled a Noatum war tower", def.Index)
			}
		}
	}
}

func TestGuildWarReservationPreservesOtherGenerators(t *testing.T) {
	for _, def := range []model.NPCGener{
		{Leader: "Tower_of_Thor", Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 2507, Y: 1873}}},
		{Leader: "Merchant", MinuteGenerate: -1, MaxNumMob: 1, Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 1051, Y: 1690}}},
	} {
		if generatorReservedForGuildWar(def) {
			t.Fatalf("unrelated generator %q was reserved", def.Leader)
		}
		w := newZoneTestWorld()
		g := &generState{def: def, leader: &model.NPCDef{Name: def.Leader, Tipo: model.TipoNPC, Score: &model.Score{MaxHP: 100}}}
		g.def.MaxNumMob = 1
		w.spawnGroup(g)
		if g.current != 1 || len(w.mobs) != 1 {
			t.Fatalf("ordinary generator %q stopped spawning", def.Leader)
		}
	}
}

func TestTowerWarUsesReferenceAnchor(t *testing.T) {
	if towerWarSpawnX != 2507 || towerWarSpawnY != 1873 {
		t.Fatal("daily tower anchor differs from the reference generator")
	}
	w, _, _, _, _ := scheduledWarWorld(t, "2026-10-14 21:06")
	if err := w.spawnWarTower(); err != nil {
		t.Fatal(err)
	}
	m := w.warTower()
	if m == nil || m.X != towerWarSpawnX || m.Y != towerWarSpawnY || !towerWarArena.contains(m.X, m.Y) {
		t.Fatal("daily war did not spawn its dedicated tower at the reference anchor")
	}
}
