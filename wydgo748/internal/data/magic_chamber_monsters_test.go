package data

import (
	"path/filepath"
	"reflect"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestMagicChamberDedicatedMonsterDefinitions(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]*model.NPCDef)
	for i := range npcs {
		byName[npcs[i].Name] = &npcs[i]
	}
	donors := historicalInstanceDonors(t)
	cases := []struct{ source, name, tier string }{
		{"Meio_Orc__", "MCN_Orc", "normal"},
		{"Skull2", "MCN_Skull", "normal"},
		{"Skull_Lancr2", "MCN_Lancer", "normal"},
		{"Minotauro_", "MCN_Minotaur", "normal"},
		{"Troll_Caos__", "MCN_Troll", "normal"},
		{"Dark_Elf3", "MCM_DarkElf", "mystic"},
		{"Taurus_King3", "MCM_Taurus", "mystic"},
		{"Hydra2", "MCM_Hydra", "mystic"},
		{"Cruel_Cyclp3", "MCM_Cyclops", "mystic"},
		{"Zakum_", "MCM_Zakum", "mystic"},
		{"Dark_Elf_Ab2", "MCA_DarkElf", "arcane"},
		{"Tauron_____", "MCA_Tauron", "arcane"},
		{"Morlock_", "MCA_Morlock", "arcane"},
		{"Hell_Spider2", "MCA_Spider", "arcane"},
		{"Lugefer_", "MCA_Lugefer", "arcane"},
	}
	dedicated := make(map[string]string)
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			source, monster := donors[tc.source], byName[tc.name]
			if source == nil || monster == nil || source.Score == nil || monster.Score == nil {
				t.Fatal("missing dedicated Magic Chamber monster or source template")
			}
			if monster == source || monster.Score == source.Score || !monster.IsMonster() || len(monster.VisibleName()) > model.MaxNPCDisplayNameBytes {
				t.Fatal("dedicated definition is shared, non-hostile or exceeds the wire name limit")
			}
			want := map[string][2]uint32{"normal": {1000, 200000}, "mystic": {4000, 300000}, "arcane": {10000, 500000}}[tc.tier]
			expected, score := *source, *source.Score
			expected.Name, expected.ExpReward = tc.name, want[1]
			expected.Carry, expected.Gold = nil, 0
			score.MaxHP, score.CurHP = want[0], want[0]
			score.Attack, score.MagicAttack, score.Defense = 1, 1, 1
			expected.Score = &score
			if !reflect.DeepEqual(&expected, monster) {
				t.Fatalf("incorrect Magic Chamber balance, drops or preserved appearance: %+v", monster)
			}
		})
		dedicated[tc.name] = tc.tier
	}
	catalog, err := LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"), filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	configs := make(map[string]model.VolatileInstance)
	for _, inst := range volatiles.Instances {
		configs[inst.ID] = inst
	}
	for _, rule := range volatiles.Items {
		if rule.Instance != nil {
			configs[rule.Instance.ID] = *rule.Instance
		}
	}
	for _, rule := range volatiles.Rules {
		if rule.Instance != nil {
			configs[rule.Instance.ID] = *rule.Instance
		}
	}
	referenced := make(map[string]bool)
	chambers := 0
	for _, inst := range configs {
		tier := ""
		if strings.HasPrefix(inst.ID, "magic-chamber-") {
			tier = strings.TrimPrefix(inst.ID, "magic-chamber-")
			chambers++
			if len(inst.Stages) != 4 || inst.TransitionSeconds != 10 || inst.FinishPolicy != "advance_on_clear" {
				t.Fatalf("%s lost its four-room progression", inst.ID)
			}
		}
		check := func(spawn model.VolatileInstanceSpawn) {
			monsterTier, exists := dedicated[spawn.NPC]
			if tier == "" {
				if exists {
					t.Errorf("%s borrows exclusive Magic Chamber monster %s", inst.ID, spawn.NPC)
				}
				return
			}
			if !exists || monsterTier != tier {
				t.Errorf("%s uses shared or wrong-tier monster %s", inst.ID, spawn.NPC)
			}
			referenced[spawn.NPC] = true
		}
		for _, spawn := range inst.Spawns {
			check(spawn)
		}
		for i, stage := range inst.Stages {
			population := 0
			for _, spawn := range stage.Spawns {
				check(spawn)
				population += spawn.Count
			}
			for _, spawn := range stage.CompletionSpawns {
				check(spawn)
			}
			if tier != "" {
				want := 75
				if i == 3 {
					want = 24
				} else if tier == "normal" && i == 1 {
					want = 50
				} else if tier == "arcane" && i == 1 {
					want = 30
				}
				if population != want || stage.DurationSeconds != 180 ||
					(i == 3 && (len(stage.CompletionSpawns) != 1 || stage.CompletionSpawns[0].Count != 1)) ||
					(i != 3 && len(stage.CompletionSpawns) != 0) {
					t.Errorf("%s room %d changed population, timer or boss completion", inst.ID, i+1)
				}
			}
		}
		if inst.NightmareBoss != nil {
			check(inst.NightmareBoss.Spawn)
		}
	}
	if chambers != 3 || len(referenced) != len(dedicated) {
		t.Fatalf("chambers=%d, referenced exclusive definitions=%d, want 3 and %d", chambers, len(referenced), len(dedicated))
	}
	geners, err := LoadChannelNPCGener(filepath.Join(root, "spawns", "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	for _, gener := range geners {
		if dedicated[gener.Leader] != "" || dedicated[gener.Follower] != "" {
			t.Errorf("open-world generator %d references an exclusive Magic Chamber monster", gener.Label)
		}
	}
	for _, file := range []string{"CartaN.txt", "CartaM.txt", "CartaA.txt"} {
		geners, err := LoadNPCGener(filepath.Join(root, "spawns", "instances", file))
		if err != nil {
			t.Fatal(err)
		}
		for _, gener := range geners {
			if gener.MinuteGenerate != -1 || dedicated[gener.Leader] == "" || gener.Follower != gener.Leader {
				t.Errorf("%s generator %d lost its exclusive instance-only references", file, gener.Label)
			}
		}
	}
}
