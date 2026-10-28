package npccontent

import (
	"fmt"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestReferencesUseTypedFieldsAndRejectMissingTemplates(t *testing.T) {
	spawn := func(name string) model.VolatileInstanceSpawn { return model.VolatileInstanceSpawn{NPC: name} }
	group := func(name string) model.NPCGener { return model.NPCGener{Enabled: true, Leader: name, Follower: name} }
	inst := model.VolatileInstance{ID: "metadata", Name: "metadata", Spawns: []model.VolatileInstanceSpawn{spawn("instance")},
		NightmareProtectors: []model.VolatileInstanceSpawn{spawn("protector")}, NightmareBoss: &model.VolatileNightmareBoss{Spawn: spawn("nightmare")},
		Stages: []model.VolatileInstanceStage{{Name: "metadata", Spawns: []model.VolatileInstanceSpawn{spawn("stage")},
			CompletionSpawns: []model.VolatileInstanceSpawn{spawn("completion")}, RuneBonusSpawns: []model.VolatileInstanceSpawn{spawn("bonus")}}},
		Uxmal: &model.VolatileUxmal{NPC: "uxmal", Generators: []model.NPCGener{group("rune")}},
		HellGate: &model.VolatileHellGate{ControllerNPC: "controller", Corridors: []model.VolatileInstanceSpawn{spawn("corridor")},
			FinalSpawns: []model.VolatileInstanceSpawn{spawn("final")}, FinalNPCs: []model.VolatileInstanceSpawn{spawn("exit")},
			Quadrants: []model.VolatileHellGateQuadrant{{Spawns: []model.VolatileInstanceSpawn{spawn("quadrant")}, Lich: spawn("lich")}},
			Population759: &model.VolatileHellGatePopulation{CorridorEscorts: []model.VolatileInstanceSpawn{spawn("escort")},
				Rooms: [][]model.VolatileInstanceSpawn{{spawn("room")}}, LichEscorts: [][]model.VolatileInstanceSpawn{{spawn("lichEscort")}},
				FinalSpawns: []model.VolatileInstanceSpawn{spawn("historicalFinal")}}}}
	sources := Sources{Generators: []model.NPCGener{group("leader"), {Leader: "disabled-missing"}},
		Quests:     model.QuestFile{Quests: []model.QuestDef{{ID: 71, NPC: "quest", Dialogue: []string{"metadata"}}}},
		QuestZones: []model.QuestZone{{Name: "metadata", NPC: "zone"}}, Bosses: model.BossCatalog{Bosses: []model.BossConfig{{ID: "metadata", Name: "metadata", NPC: "boss", Summons: []model.BossSummon{{NPC: "add"}}}}},
		Volatiles: model.VolatileCatalog{Default: model.VolatileRule{Instance: &inst}, Rules: map[int]model.VolatileRule{7: {Instance: &model.VolatileInstance{Spawns: []model.VolatileInstanceSpawn{spawn("rule")}}}},
			Items:     map[uint16]model.VolatileRule{42: {Summon: &model.VolatileSummon{Name: "metadata"}, Instance: &model.VolatileInstance{Spawns: []model.VolatileInstanceSpawn{spawn("item")}}}},
			Instances: map[string]model.VolatileInstance{"metadata": {Spawns: []model.VolatileInstanceSpawn{spawn("named")}}}}}
	sources.Colosseum[0] = []model.NPCGener{group("colosseum")}
	names := []string{"leader", "quest", "zone", "boss", "add", "instance", "protector", "nightmare", "stage", "completion", "bonus", "uxmal", "rune", "controller", "corridor", "final", "exit", "quadrant", "lich", "escort", "room", "lichEscort", "historicalFinal", "rule", "item", "named", "colosseum"}
	npcs := make([]model.NPCDef, 0, len(names)+1)
	for _, name := range names {
		npcs = append(npcs, model.NPCDef{Name: name})
	}
	npcs = append(npcs, model.NPCDef{Name: "metadata"})
	used, err := RequiredNames(npcs, sources)
	if err != nil || len(used) != len(names) || used["metadata"] {
		t.Fatalf("typed references: %v %v", used, err)
	}
	for i, name := range names {
		t.Run(name, func(t *testing.T) {
			incomplete := append([]model.NPCDef(nil), npcs[:i]...)
			incomplete = append(incomplete, npcs[i+1:]...)
			_, err := RequiredNames(incomplete, sources)
			if err == nil || !strings.Contains(err.Error(), fmt.Sprintf("missing template %q", name)) || !strings.Contains(err.Error(), ".") {
				t.Fatalf("missing contextual reference error: %v", err)
			}
		})
	}
}

func BenchmarkRequiredNames(b *testing.B) {
	npcs := make([]model.NPCDef, 1000)
	groups := make([]model.NPCGener, 5500)
	for i := range npcs {
		npcs[i].Name = fmt.Sprintf("NPC%d", i)
	}
	for i := range groups {
		groups[i] = model.NPCGener{Index: i, Enabled: true, Leader: npcs[i%len(npcs)].Name}
	}
	sources := Sources{Generators: groups}
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		if _, err := RequiredNames(npcs, sources); err != nil {
			b.Fatal(err)
		}
	}
}

func TestGeneratorAliasesPreserveExactIdentityAndRejectAmbiguity(t *testing.T) {
	for _, tc := range []struct {
		name, reference, want string
		npcs                  []model.NPCDef
	}{
		{"legacy spaces", "Test Mob", "Test_Mob", []model.NPCDef{{Name: "Test_Mob"}}},
		{"exact takes precedence", "A_B_C", "A_B_C", []model.NPCDef{{Name: "A_B_C"}, {Name: "A B_C"}}},
		{"ambiguous alias", "A_B C", "", []model.NPCDef{{Name: "A_B_C"}, {Name: "A B_C"}}},
		{"case is significant", "shop", "", []model.NPCDef{{Name: "Shop"}}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			used, err := RequiredNames(tc.npcs, Sources{Generators: []model.NPCGener{{Enabled: true, Leader: tc.reference}}})
			if tc.want == "" {
				if err == nil {
					t.Fatal("unresolved alias accepted")
				}
				return
			}
			if err != nil || len(used) != 1 || !used[tc.want] {
				t.Fatalf("wrong canonical template: %v %v", used, err)
			}
		})
	}
	if used, err := RequiredNames(nil, Sources{QuestZones: []model.QuestZone{{Name: "Area label"}}}); err != nil || len(used) != 0 {
		t.Fatalf("area metadata selected an NPC: %v %v", used, err)
	}
}
