package data

import "testing"

// Uxmal registers the Rune Track, so it stands inside the entrance area like
// the 7.59 generator (3297,1693), not at the old Field0813 spot (1057,1717).
func TestUxmalStandsAtTheRuneTrackEntrance(t *testing.T) {
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
		if !gener.Enabled || gener.Leader != "Uxmal" {
			continue
		}
		found++
		for _, segment := range gener.Segments {
			if segment.X == 0 && segment.Y == 0 {
				continue // unused route slot
			}
			if segment.X != 3297 || segment.Y != 1693 {
				t.Fatalf("Uxmal segment at (%d,%d), want (3297,1693)", segment.X, segment.Y)
			}
		}
		if !terrain.Walkable(3297, 1693) {
			t.Fatal("Uxmal stands on an unwalkable cell")
		}
	}
	if found != 1 {
		t.Fatalf("enabled Uxmal generators = %d, want 1", found)
	}
}
