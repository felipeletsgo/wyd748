package data

import (
	"path/filepath"
	"strings"
	"testing"
)

// Magic Chamber (Carta) rooms and Nightmare waves spawn from the dedicated
// generator files in data/NPCGener, at each block's native start point.
func TestInstanceWavesComeFromGeneratorFiles(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	terrain, err := LoadTerrain(filepath.Join(root, "maps", "HeightMap.dat"), filepath.Join(root, "maps", "AttributeMap.dat"))
	if err != nil {
		t.Fatal(err)
	}

	normal := volatiles.Items[3172].Instance
	if normal == nil || len(normal.Stages) != 4 {
		t.Fatal("Carta Normal lost its four rooms")
	}
	first := normal.Stages[0].Spawns
	if len(first) != 3 || first[0].NPC != "Meio_Orc__" || first[0].Count != 25 ||
		first[0].X != 801 || first[0].Y != 3684 || first[0].Range != 3 {
		t.Fatalf("Carta Normal room 1 = %+v", first)
	}
	if boss := normal.Stages[3].CompletionSpawns; len(boss) != 1 || boss[0].NPC != "Troll_Caos__" {
		t.Fatalf("Carta Normal boss = %+v", boss)
	}

	for item, tier := range map[uint16]string{3172: "Normal", 3171: "Mystic", 1731: "Arcane",
		3324: "Nightmare N", 3325: "Nightmare M", 3326: "Nightmare A"} {
		inst := volatiles.Items[item].Instance
		if inst == nil {
			t.Fatalf("%s has no instance", tier)
		}
		spawns := 0
		check := func(where string, npc string, count int, x, y uint16) {
			if npc == "" || count <= 0 || x == 0 || y == 0 || !terrain.Walkable(x, y) {
				t.Errorf("%s %s: %s x%d at (%d,%d) is not a walkable generator spawn", tier, where, npc, count, x, y)
			}
		}
		for _, spawn := range inst.Spawns {
			check("wave", spawn.NPC, spawn.Count, spawn.X, spawn.Y)
			spawns++
		}
		for i, stage := range inst.Stages {
			for _, spawn := range append(stage.Spawns, stage.CompletionSpawns...) {
				check(stage.Name, spawn.NPC, spawn.Count, spawn.X, spawn.Y)
				spawns++
			}
			if stage.Generator == "" {
				t.Errorf("%s stage %d is not backed by a generator file", tier, i)
			}
		}
		if spawns == 0 {
			t.Errorf("%s has no generator spawns", tier)
		}
	}

	// The open world never spawns the instance-only Carta/Nightmare groups.
	geners, err := LoadNPCGener(filepath.Join(root, "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	instanceOnly := map[string]bool{"Meio_Orc__": true, "Tauron_____": true, "Tauron__": true,
		"Troll_Caos__": true, "Lugefer_": true, "Dark_Shadow_": true, "Dark_Shadow__": true}
	for _, gener := range geners {
		if gener.Enabled && gener.MinuteGenerate >= 0 && instanceOnly[gener.Leader] {
			t.Errorf("open-world generator %d spawns instance monster %s", gener.Label, gener.Leader)
		}
		if strings.Contains(gener.Leader, "Orc_N") {
			t.Errorf("placeholder generator leader %s left in NPCGener.txt", gener.Leader)
		}
	}
}
