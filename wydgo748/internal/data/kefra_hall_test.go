package data

import "testing"

// The Kefra hall holds Kefra and his four guards (two Lich Crunt, two Lich
// Vatama) on walkable cells around him.
func TestKefraHallSpawnsKefraAndHisMagos(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	monsters := map[string]bool{}
	for _, npc := range npcs {
		monsters[npc.Name] = npc.IsMonster()
	}
	geners, err := LoadNPCGener("../../data/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	terrain, err := LoadTerrain("../../data/maps/HeightMap.dat", "../../data/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	counts := map[string]int{}
	for _, gener := range geners {
		x, y := gener.Segments[0].X, gener.Segments[0].Y
		if !gener.Enabled || x < 2335 || x >= 2395 || y < 3896 || y >= 3955 {
			continue
		}
		counts[gener.Leader]++
		if !monsters[gener.Leader] || !terrain.Walkable(x, y) {
			t.Fatalf("%s spawn (%d,%d) has no monster template or is not walkable", gener.Leader, x, y)
		}
	}
	if counts["Kefra"] != 1 || counts["Lich_Crunt"] != 2 || counts["Lich_Vatama"] != 2 || len(counts) != 3 {
		t.Fatalf("Kefra hall spawns = %v", counts)
	}
}
