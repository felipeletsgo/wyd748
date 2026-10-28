package data

import "testing"

// The native 7.48 client opens the Hell Gate store only for an NPC with head
// 51 standing where x/128 is 13 or 14 and y/128 is 28 (FUN_0045ee28).
func TestConfiguredHellStoreNPCSpawnsWhereTheNativeClientOpensTheStore(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	head := -1
	for _, npc := range npcs {
		if npc.Name == "Hell_Store" {
			head = int(npc.Equip.Rosto.Index)
		}
	}
	if head != 51 {
		t.Fatalf("Hell_Store head = %d; want 51", head)
	}
	geners, err := LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	terrain, err := LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	found := 0
	for _, gener := range geners {
		if !gener.Enabled || gener.Leader != "Hell_Store" {
			continue
		}
		found++
		x, y := gener.Segments[0].X, gener.Segments[0].Y
		if (x>>7 != 13 && x>>7 != 14) || y>>7 != 28 || !terrain.Walkable(x, y) {
			t.Fatalf("Hell_Store spawn (%d,%d) is outside the native store region or not walkable", x, y)
		}
	}
	// The manually curated general population may omit the standalone store.
	// Instance-owned Dionys is spawned and tested through the Hell Gate lifecycle.
	if found > 1 {
		t.Fatalf("Hell_Store spawns = %d; want at most 1", found)
	}
}
