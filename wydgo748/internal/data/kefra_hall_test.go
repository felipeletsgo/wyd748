package data

import "testing"

// The shared hall retains the approved native population and imported project
// groups. Both populations must resolve to hostile templates on walkable cells.
func TestKefraHallSpawnsKefraAndHisMagos(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	monsters := map[string]bool{}
	for _, npc := range npcs {
		monsters[npc.Name] = npc.IsMonster()
	}
	geners, err := LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	terrain, err := LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
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
	if counts["Kefra"] != 2 || counts["Lich_Crunt"] != 2 || counts["Lich_Vatama"] != 2 ||
		counts["CepraGuard@"] != 17 || counts["DarkMage"] != 5 || len(counts) != 5 {
		t.Fatalf("Kefra hall spawns = %v", counts)
	}
}
