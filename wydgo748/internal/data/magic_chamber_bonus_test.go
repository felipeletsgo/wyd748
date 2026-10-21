package data

import (
	"path/filepath"
	"testing"
)

// Each Magic Chamber's fourth room names a monster bonus boss at the 7.59
// position (806,3621), standing on a walkable cell.
func TestMagicChamberRuneBonusBossesAreConfigured(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	monsters := map[string]bool{}
	for _, npc := range npcs {
		monsters[npc.Name] = npc.IsMonster()
	}
	terrain, err := LoadTerrain(filepath.Join(root, "maps", "HeightMap.dat"), filepath.Join(root, "maps", "AttributeMap.dat"))
	if err != nil {
		t.Fatal(err)
	}
	want := map[string]string{"magic-chamber-normal": "MCN_CruelZak",
		"magic-chamber-mystic": "MCM_Gargoyle", "magic-chamber-arcane": "MCA_Rujeper"}
	found := map[string]bool{}
	for _, rule := range volatiles.Items {
		if rule.Instance == nil {
			continue
		}
		boss, ok := want[rule.Instance.ID]
		if !ok || len(rule.Instance.Stages) != 4 {
			continue
		}
		spawns := rule.Instance.Stages[3].RuneBonusSpawns
		if len(spawns) != 1 || spawns[0].NPC != boss || spawns[0].Count != 1 ||
			spawns[0].X != 806 || spawns[0].Y != 3621 || !monsters[boss] || !terrain.Walkable(806, 3621) {
			t.Fatalf("%s rune bonus = %+v; want one %s at walkable (806,3621)", rule.Instance.ID, spawns, boss)
		}
		found[rule.Instance.ID] = true
	}
	if len(found) != len(want) {
		t.Fatalf("configured chambers = %v", found)
	}
}
