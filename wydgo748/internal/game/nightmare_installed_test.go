package game

import (
	"path/filepath"
	"testing"
	"time"

	"wydgo/internal/data"
)

// TestNightmareSoloOpensWithInstalledData drives ticket 3390 through the
// production critical-persistence entry with the shipped terrain and monsters.
// Critical tile reservations once used 16x16 spatial cells, so a radius-55
// room could hold only about 49 monsters and the 101-monster first wave was
// rejected with "The instance monster area is blocked."
func TestNightmareSoloOpensWithInstalledData(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	terrain, err := data.LoadTerrain(filepath.Join(root, "maps", "HeightMap.dat"),
		filepath.Join(root, "maps", "AttributeMap.dat"))
	if err != nil {
		t.Skipf("installed maps unavailable: %v", err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	items, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), items.Items, items.Skills)
	if err != nil {
		t.Fatal(err)
	}
	rule, ok := catalog.Items[3390]
	if !ok || rule.Instance == nil {
		t.Fatal("ticket 3390 is not an instance ticket")
	}

	w, leader, member, peer, _, base, q, _, release := criticalInstanceEntryFixture(t, "water", "commit")
	leader.Party, member.Party = nil, nil
	leader.Trade, peer.Trade = nil, nil
	w.terrain = terrain
	w.npcs = npcs
	w.volatiles.Items[100] = rule
	// Inside the minute-40 window and the Erion entry area.
	w.clock = newFakeClock(time.Date(2026, 10, 13, 20, 41, 0, 0, time.UTC))
	leader.X, leader.Y, leader.Char.X, leader.Char.Y = 2457, 2016, 2457, 2016
	w.updatePlayerSpatial(leader)
	leader.Char.Score.Level = 300
	leader.Char.RuntimeScore = nil
	applyScore(leader.Char)

	if err := w.queueCriticalInstanceTicket(leader, 0, rule, ""); err != nil {
		t.Fatalf("Nightmare Normal entry rejected: %v", err)
	}
	awaitCriticalWrite(t, base.entered)
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)

	var inst *ItemInstance
	for _, candidate := range w.itemInstances {
		if candidate != nil && candidate.Config.ID == rule.Instance.ID {
			inst = candidate
		}
	}
	want := 0
	for _, spawn := range rule.Instance.Spawns {
		want += spawn.Count
	}
	if inst == nil || !itemInstanceHasMember(inst, leader.ID) || len(inst.MobIDs) != want {
		t.Fatalf("Nightmare Normal did not open with %d monsters: %+v", want, inst)
	}
	seen := make(map[uint32]struct{}, want)
	for id := range inst.MobIDs {
		mob := w.mobsByID[id]
		if mob == nil || !w.terrain.Walkable(mob.X, mob.Y) {
			t.Fatalf("monster %d was not placed on walkable terrain", id)
		}
		tile := uint32(mob.X)<<16 | uint32(mob.Y)
		if _, duplicate := seen[tile]; duplicate {
			t.Fatalf("two monsters share tile (%d,%d)", mob.X, mob.Y)
		}
		seen[tile] = struct{}{}
	}
}
