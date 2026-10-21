package data

import (
	"path/filepath"
	"reflect"
	"strings"
	"testing"

	"wydgo/internal/model"
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
		t.Fatal("Magic Chamber Normal lost its four rooms")
	}
	first := normal.Stages[0].Spawns
	if len(first) != 3 || first[0].NPC != "MCN_Orc" || first[0].Count != 25 ||
		first[0].X != 801 || first[0].Y != 3684 || first[0].Range != 3 {
		t.Fatalf("Magic Chamber Normal room 1 = %+v", first)
	}
	if boss := normal.Stages[3].CompletionSpawns; len(boss) != 1 || boss[0].NPC != "MCN_Troll" {
		t.Fatalf("Magic Chamber Normal boss = %+v", boss)
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

// Nightmare balance belongs to dedicated static definitions, never shared-map
// templates or a runtime HP/EXP rewrite. Visuals retain their source data; loot is empty.
func TestNightmareDedicatedMonsterDefinitions(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]*model.NPCDef)
	for i := range npcs {
		byName[npcs[i].Name] = &npcs[i]
	}
	cases := []struct{ source, name, tier string }{
		{"Morlock__", "NMN_Morlock", "normal"},
		{"Dark_Kn1", "NMN_DarkKn", "normal"},
		{"Elf_ABJ2", "NMN_Elf", "normal"},
		{"Aqua_Golem__", "NMN_Aqua", "normal"},
		{"Gargoyle3", "NMN_Gargoyle", "normal"},
		{"Dark_Shadow_", "NMN_Shadow", "normal"},
		{"Tauron__", "NMM_Tauron", "mystic"},
		{"Hell_Spider3", "NMM_Spider", "mystic"},
		{"GrubSwarm_", "NMM_Grub", "mystic"},
		{"Wise_Gargo3", "NMM_Gargoyle", "mystic"},
		{"Xeno_Cropper_", "NMM_Cropper", "mystic"},
		{"Dark_Shadow__", "NMM_Shadow", "mystic"},
		{"Adamant_Tauron", "NMA_ArmTaur", "arcane"},
		{"Tauron_____", "NMA_Tauron", "arcane"},
		{"Manticora_", "NMA_Manticor", "arcane"},
		{"Hell_Spider2", "NMA_Spider", "arcane"},
		{"Lugefer_", "NMA_Lugefer", "arcane"},
	}
	dedicated := make(map[string]string)
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			source, monster := byName[tc.source], byName[tc.name]
			if source == nil || monster == nil || source.Score == nil || monster.Score == nil {
				t.Fatal("missing dedicated monster or source template")
			}
			if monster == source || monster.Score == source.Score || !monster.IsMonster() || len(monster.Name) > wireNameBytes {
				t.Fatal("dedicated definition is shared, non-hostile or exceeds the wire name limit")
			}
			want := map[string][2]uint32{"normal": {2000, 200000}, "mystic": {5000, 400000}, "arcane": {10000, 600000}}[tc.tier]
			score := monster.Score
			if score.MaxHP != want[0] || score.CurHP != want[0] || monster.ExpReward != want[1] ||
				score.Attack != 1 || score.MagicAttack != 1 || score.Defense != 1 {
				t.Fatalf("incorrect dedicated balance: score=%+v EXP=%d", score, monster.ExpReward)
			}
			if len(monster.Carry) != 0 || monster.Gold != 0 {
				t.Fatal("Nightmare monsters must not contain item or gold drops")
			}
			// Preserve the template except for identity, leveling balance and removed loot.
			expected, expectedScore := *source, *source.Score
			expected.Name, expected.ExpReward = monster.Name, want[1]
			expected.Carry, expected.Gold = nil, 0
			expectedScore.MaxHP, expectedScore.CurHP = want[0], want[0]
			expectedScore.Attack, expectedScore.MagicAttack, expectedScore.Defense = 1, 1, 1
			expected.Score = &expectedScore
			if !reflect.DeepEqual(&expected, monster) {
				t.Fatal("dedicated monster unexpectedly changed appearance or unrelated statistics")
			}
			if source.Score.MaxHP == want[0] || source.Score.Attack == 1 || source.ExpReward == want[1] {
				t.Fatal("original template no longer retains its independent balance")
			}
		})
		dedicated[tc.name] = tc.tier
	}
	catalog, err := LoadCatalog(filepath.Join(root, "itemlist.csv"), filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, root, catalog)
	referenced := make(map[string]bool)
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
	nightmares := 0
	for _, inst := range configs {
		if inst.NightmareTier != "" {
			nightmares++
			if inst.RewardItem != 0 {
				t.Errorf("%s contains item reward %d in an EXP-only encounter", inst.ID, inst.RewardItem)
			}
		}
		check := func(spawn model.VolatileInstanceSpawn) {
			tier, exists := dedicated[spawn.NPC]
			if inst.NightmareTier == "" {
				if exists {
					t.Errorf("%s uses exclusive Nightmare monster %s", inst.ID, spawn.NPC)
				}
				return
			}
			if !exists || tier != inst.NightmareTier {
				t.Errorf("%s references shared or wrong-tier monster %s", inst.ID, spawn.NPC)
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
	if nightmares != 6 {
		t.Fatalf("validated %d Nightmare configurations, want all six party/solo tiers", nightmares)
	}
	for name := range dedicated {
		if !referenced[name] {
			t.Errorf("dedicated definition %s is not used by Nightmare", name)
		}
	}
	geners, err := LoadNPCGener(filepath.Join(root, "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	for _, gener := range geners {
		if dedicated[gener.Leader] != "" || dedicated[gener.Follower] != "" {
			t.Errorf("open-world generator %d references an exclusive Nightmare monster", gener.Label)
		}
	}
}

func TestNightmareBossConfigurationValidation(t *testing.T) {
	for _, scenario := range []string{"valid", "wrong-tier", "zero-time", "before-combat", "multiple-bosses", "ordinary-respawn"} {
		t.Run(scenario, func(t *testing.T) {
			inst := &model.VolatileInstance{NightmareTier: "arcane",
				NightmareBoss: &model.VolatileNightmareBoss{SecondsRemaining: 300,
					Spawn: model.VolatileInstanceSpawn{NPC: "Lugefer_", Count: 1, X: 1208, Y: 173}}}
			switch scenario {
			case "wrong-tier":
				inst.NightmareTier = "normal"
			case "zero-time":
				inst.NightmareBoss.SecondsRemaining = 0
			case "before-combat":
				inst.NightmareBoss.SecondsRemaining = 900
			case "multiple-bosses":
				inst.NightmareBoss.Spawn.Count = 2
			case "ordinary-respawn":
				inst.Spawns = []model.VolatileInstanceSpawn{inst.NightmareBoss.Spawn}
			}
			cache := &instanceGeneratorCache{}
			if err := cache.expand("Nightmare A", inst); (err == nil) != (scenario == "valid") {
				t.Fatalf("configuration validation returned %v", err)
			}
		})
	}
}

func TestInstanceGeneratorRejectsInlineCompletionSpawns(t *testing.T) {
	gener := model.NPCGener{Label: 1, MaxNumMob: 1, Leader: "Mob"}
	gener.Segments[0] = model.GenerSegment{X: 100, Y: 200}
	cache := &instanceGeneratorCache{files: map[string][]model.NPCGener{
		"waves.txt": {gener},
	}}
	inst := &model.VolatileInstance{Stages: []model.VolatileInstanceStage{{
		Generator:        "waves.txt",
		GeneratorIDs:     []int{1},
		CompletionSpawns: []model.VolatileInstanceSpawn{{NPC: "Boss", Count: 1, X: 100, Y: 200}},
	}}}
	if err := cache.expand("inline completion", inst); err == nil {
		t.Fatal("inline completion spawns were accepted alongside a generator")
	}
}
