package data

import (
	"fmt"
	"path/filepath"
	"reflect"
	"strings"
	"testing"

	"wydgo/internal/model"
)

// Dedicated Water definitions isolate leveling balance from every shared NPC.
func TestWaterDedicatedMonsterDefinitions(t *testing.T) {
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
		{"Meio_Orc_", "WTN_Orc", "normal"},
		{"Troll_Hunter", "WTN_Hunter", "normal"},
		{"Ghoul_Chief", "WTN_Ghoul", "normal"},
		{"Cyclops_Arc3", "WTN_CyclArc", "normal"},
		{"Skeleton", "WTN_Skeleton", "normal"},
		{"Grim_Lock_D", "WTN_Grim", "normal"},
		{"Cyclops_Anc", "WTN_CyclAnc", "normal"},
		{"Anf_Assasn2", "WTN_Assassin", "normal"},
		{"Troll_Caos__", "WTN_TrollB", "normal"},
		{"Dark_Elf2", "WTM_DarkElf", "mystic"},
		{"Troll_Ghoul_", "WTM_Troll", "mystic"},
		{"Anf_Ninja_", "WTM_Ninja", "mystic"},
		{"Tauron___", "WTM_Tauron", "mystic"},
		{"Grim_Lock_", "WTM_Grim", "mystic"},
		{"Elf_Witch2", "WTM_Witch", "mystic"},
		{"Gargoyle2", "WTM_Gargoyle", "mystic"},
		{"Aqua_Golem_", "WTM_Golem", "mystic"},
		{"Immort_Hydra", "WTM_HydraB", "mystic"},
		{"Hydra2", "WTM_Hydra", "mystic"},
		{"Gargoyle3", "WTA_Gargoyle", "arcane"},
		{"Fire_Golem", "WTA_Fire", "arcane"},
		{"Tauron_", "WTA_Tauron", "arcane"},
		{"Lich_Dragon2", "WTA_Dragon", "arcane"},
		{"Dark_Kn2", "WTA_DarkKn", "arcane"},
		{"Morlock____", "WTA_Morlock", "arcane"},
		{"Hell_Spider2", "WTA_Spider", "arcane"},
		{"LiggleSwarm", "WTA_Liggle", "arcane"},
		{"Lugefer_Maligno", "WTA_Lugefer", "arcane"},
	}
	dedicated := make(map[string]string)
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			source, monster := donors[tc.source], byName[tc.name]
			if source == nil || monster == nil || source.Score == nil || monster.Score == nil {
				t.Fatal("missing dedicated Water monster or source template")
			}
			if monster == source || monster.Score == source.Score || !monster.IsMonster() || len(monster.VisibleName()) > model.MaxNPCDisplayNameBytes {
				t.Fatal("Water definition is shared, non-hostile or exceeds the wire name limit")
			}
			want := map[string][2]uint32{"normal": {1000, 200000}, "mystic": {4000, 300000}, "arcane": {10000, 500000}}[tc.tier]
			expected, score := *source, *source.Score
			expected.Name, expected.ExpReward = tc.name, want[1]
			expected.Carry, expected.Gold = nil, 0
			score.MaxHP, score.CurHP = want[0], want[0]
			score.Attack, score.MagicAttack, score.Defense = 1, 1, 1
			expected.Score = &score
			if !reflect.DeepEqual(&expected, monster) {
				t.Fatalf("incorrect Water balance, drops or preserved appearance: %+v", monster)
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
	for _, inst := range configs {
		tier := ""
		if parts := strings.Split(inst.ID, "-"); len(parts) == 3 && parts[0] == "water" {
			tier = parts[1]
		}
		check := func(spawn model.VolatileInstanceSpawn) {
			monsterTier, exists := dedicated[spawn.NPC]
			if tier == "" {
				if exists {
					t.Errorf("%s borrows exclusive Water monster %s", inst.ID, spawn.NPC)
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
		for _, stage := range inst.Stages {
			for _, spawn := range append(append([]model.VolatileInstanceSpawn{}, stage.Spawns...), stage.CompletionSpawns...) {
				check(spawn)
			}
		}
		if inst.NightmareBoss != nil {
			check(inst.NightmareBoss.Spawn)
		}
	}
	for name := range dedicated {
		if !referenced[name] {
			t.Errorf("unused exclusive Water definition %s", name)
		}
	}
	// Every tier retains its eight progression rewards and reward-free boss room.
	for tier, firstItem := range map[string]uint16{"normal": 3173, "mystic": 777, "arcane": 3182} {
		for room := 1; room <= 9; room++ {
			id := fmt.Sprintf("water-%s-%d", tier, room)
			if room == 9 {
				id = "water-" + tier + "-boss"
			}
			inst, ok := configs[id]
			if !ok {
				t.Fatalf("missing Water room %s", id)
			}
			population := 0
			for _, spawn := range inst.Spawns {
				population += spawn.Count
			}
			wantPopulation := 15
			if room == 9 {
				wantPopulation = 11
			} else if tier == "arcane" && room == 4 {
				wantPopulation = 5
			}
			if population != wantPopulation {
				t.Errorf("%s population=%d, want=%d", id, population, wantPopulation)
			}
			if room < 9 && inst.RewardItem != firstItem+uint16(room) {
				t.Errorf("%s lost its next Water Scroll reward", id)
			}
			if room == 9 && (inst.RewardItem != 0 || inst.ChainNextItem != firstItem) {
				t.Errorf("%s lost its reward-free return to room one", id)
			}
		}
	}
	geners, err := LoadChannelNPCGener(filepath.Join(root, "spawns", "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	for _, gener := range geners {
		if dedicated[gener.Leader] != "" || dedicated[gener.Follower] != "" {
			t.Errorf("open-world generator %d references an exclusive Water monster", gener.Label)
		}
	}
}
