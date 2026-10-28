package data

import (
	"testing"

	"wydgo/internal/model"
)

func TestMolarEntryGeneratorsAndDrop(t *testing.T) {
	base, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	terrain, err := LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	for _, file := range []string{"NPCGener.txt"} {
		t.Run(file, func(t *testing.T) {
			groups, err := LoadChannelNPCGener("../../data/spawns/" + file)
			if err != nil {
				t.Fatal(err)
			}
			npcs := base

			byName := make(map[string]model.NPCDef, len(npcs))
			for _, npc := range npcs {
				byName[npc.Name] = npc
			}
			entries, dropGroups := 0, 0
			for _, group := range groups {
				if !group.Enabled || group.MinuteGenerate < 0 || group.MaxNumMob <= 0 {
					continue
				}
				npc := byName[group.Leader]
				if npc.InteractionName() == "Gargoyle_Watcher" {
					entries++
					if npc.IsMonster() || npc.Score == nil || npc.Score.Merchant != 43 ||
						group.MaxNumMob != 1 || group.MinGroup != 0 || group.MaxGroup != 0 {
						t.Fatal("Molar entrance is hostile, has the wrong handler type or spawns duplicates")
					}
					if start := group.Segments[0]; start.X != 787 || start.Y != 4058 || !terrain.Walkable(start.X, start.Y) {
						t.Fatalf("unusable Molar entrance placement: %+v", start)
					}
				}
				start := group.Segments[0]
				if !npc.IsMonster() || start.X < 780 || start.X > 840 || start.Y < 4030 || start.Y > 4095 {
					continue
				}
				for _, item := range npc.Carry {
					if item.Index == 4122 {
						dropGroups++
						break
					}
				}
			}
			if entries != 1 || dropGroups == 0 || !terrain.Walkable(817, 4062) {
				t.Fatalf("unusable Molar area: entrances=%d drop groups=%d destination walkable=%v",
					entries, dropGroups, terrain.Walkable(817, 4062))
			}
			t.Logf("one friendly entrance and %d enabled Molar drop groups", dropGroups)
		})
	}
}
