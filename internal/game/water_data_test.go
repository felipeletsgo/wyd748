package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
)

// TestRealWaterRoom1EntryAreas guards the three authoritative entry tables
// against the exact inversion that once made the external Room 1 entrance
// require a boss while leaving the boss platform open to a fresh character.
func TestRealWaterRoom1EntryAreas(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(
		filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"),
		filepath.Join(root, "SkillData.csv"),
	)
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.txt"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	cases := []struct {
		name      string
		item      uint16
		externalX uint16
		externalY uint16
		bossX     uint16
		bossY     uint16
	}{
		{name: "normal", item: 3173, externalX: 1965, externalY: 1773, bossX: 1080, bossY: 3510},
		{name: "mystic", item: 777, externalX: 1965, externalY: 1773, bossX: 1210, bossY: 3640},
		{name: "arcane", item: 3182, externalX: 1965, externalY: 1773, bossX: 1335, bossY: 3510},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			rule, _, ok := volatiles.Rule(tc.item)
			if !ok || rule.Instance == nil {
				t.Fatalf("item %d nao resolveu instancia Room 1", tc.item)
			}
			cfg := rule.Instance
			external, ok := instanceEntryAreaAt(cfg, tc.externalX, tc.externalY)
			if !ok || external.RequiresChain {
				t.Fatalf("entrada externa deveria aceitar Room 1: area=%+v ok=%v", external, ok)
			}
			boss, ok := instanceEntryAreaAt(cfg, tc.bossX, tc.bossY)
			if !ok || !boss.RequiresChain {
				t.Fatalf("plataforma do boss deveria exigir chain: area=%+v ok=%v", boss, ok)
			}
		})
	}
}
