package data

import (
	"path/filepath"
	"testing"
)

func TestConfiguredWorldBossesHaveLargeCON(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadBossCatalog(filepath.Join(root, "content", "boss"))
	if err != nil {
		t.Fatal(err)
	}
	want := map[string]string{
		"arquimago_negro":      "DeathWorm",
		"guardiao_das_ruinas":  "LegendBerial",
		"senhor_demoniaco":     "Murmy",
		"necromante_ancestral": "Anubis",
	}
	if len(catalog.Bosses) != len(want) {
		t.Fatalf("configured boss count=%d, want %d", len(catalog.Bosses), len(want))
	}
	for _, boss := range catalog.Bosses {
		npc, ok := want[boss.ID]
		if !ok || boss.NPC != npc {
			t.Errorf("unexpected boss identity: id=%q NPC=%q", boss.ID, boss.NPC)
			continue
		}
		if boss.Stats.Con == nil || *boss.Stats.Con != 5000 {
			t.Errorf("boss %q CON=%v, want 5000", boss.ID, boss.Stats.Con)
		}
	}

	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	templateCON := map[string]uint32{"Anubis": 1000, "DeathWorm": 1000, "LegendBerial": 1, "Murmy": 1}
	for _, npc := range npcs {
		for _, name := range want {
			if npc.Name == name && (npc.Score == nil || npc.Score.Con != templateCON[name]) {
				t.Errorf("native template %q must retain base CON %d", name, templateCON[name])
			}
		}
	}
}
