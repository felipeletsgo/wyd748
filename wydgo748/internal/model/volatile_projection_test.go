package model

import "testing"

func TestVolatileHellGatePopulationSpawnsOrderAndCopy(t *testing.T) {
	if (*VolatileHellGatePopulation)(nil).Spawns() != nil {
		t.Fatal("nil population should have no spawns")
	}
	population := &VolatileHellGatePopulation{
		CorridorEscorts: []VolatileInstanceSpawn{{NPC: "corridor"}},
		FinalSpawns:     []VolatileInstanceSpawn{{NPC: "final"}},
		Rooms:           [][]VolatileInstanceSpawn{{{NPC: "room-a"}, {NPC: "room-b"}}},
		LichEscorts:     [][]VolatileInstanceSpawn{{{NPC: "lich"}}},
	}
	got := population.Spawns()
	want := []string{"corridor", "final", "room-a", "room-b", "lich"}
	if len(got) != len(want) {
		t.Fatalf("spawns length = %d, want %d", len(got), len(want))
	}
	for i := range want {
		if got[i].NPC != want[i] {
			t.Errorf("spawn[%d] = %q, want %q", i, got[i].NPC, want[i])
		}
	}
	got[0].NPC = "changed"
	if population.CorridorEscorts[0].NPC != "corridor" {
		t.Fatal("returned spawn slice aliases the stored corridor list")
	}
}

func TestVolatileRuleLookupPrecedence(t *testing.T) {
	defaultRule := VolatileRule{Action: "default"}
	codeRule := VolatileRule{Action: "code"}
	itemRule := VolatileRule{Action: "item"}
	catalog := VolatileCatalog{
		Default:   defaultRule,
		ItemCodes: map[uint16]int{1: 10, 2: 20, 3: 30},
		Rules:     map[int]VolatileRule{20: codeRule},
		Items:     map[uint16]VolatileRule{1: itemRule},
	}
	if rule, code, ok := catalog.Rule(1); !ok || code != 10 || rule.Action != itemRule.Action {
		t.Fatalf("item rule = (%+v, %d, %t), want item override", rule, code, ok)
	}
	if rule, code, ok := catalog.Rule(2); !ok || code != 20 || rule.Action != codeRule.Action {
		t.Fatalf("code rule = (%+v, %d, %t), want code override", rule, code, ok)
	}
	if rule, code, ok := catalog.Rule(3); !ok || code != 30 || rule.Action != defaultRule.Action {
		t.Fatalf("default rule = (%+v, %d, %t), want catalog default", rule, code, ok)
	}
	if rule, code, ok := catalog.Rule(4); ok || code != 0 || rule.Action != "" {
		t.Fatalf("unknown item rule = (%+v, %d, %t), want missing", rule, code, ok)
	}
}
