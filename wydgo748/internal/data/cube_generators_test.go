package data

import (
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestCubeDedicatedGeneratorsCoverAllRooms(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	vc := loadRealVolatiles(t, root, catalog)
	terrain, err := LoadTerrain(filepath.Join(root, "world/maps/HeightMap.dat"), filepath.Join(root, "world/maps/AttributeMap.dat"))
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		item           uint16
		file           string
		first          int
		boss, follower string
		finalCount     int
	}{{1772, "CubeN.txt", 195, "OrcTrooper@", "Carbunkle@", 6},
		{4000, "CubeM.txt", 220, "OrcGuardian", "StrengthGolem", 5},
		{4001, "CubeA.txt", 245, "OrcLeader", "Orc_Medico__", 5}} {
		t.Run(tc.file, func(t *testing.T) {
			rule, _, ok := vc.Rule(tc.item)
			if !ok || rule.Instance == nil || len(rule.Instance.Stages) != 25 {
				t.Fatal("Cube must resolve 25 instance rooms")
			}
			generators, err := LoadNPCGener(filepath.Join(root, "spawns/instances", tc.file))
			if err != nil || len(generators) != 25 {
				t.Fatalf("Cube generator file: count=%d error=%v", len(generators), err)
			}
			rooms := map[[2]uint16]bool{}
			for i, stage := range rule.Instance.Stages {
				g := generators[i]
				point := g.Segments[0]
				key := [2]uint16{stage.X, stage.Y}
				if rooms[key] || stage.X != point.X || stage.Y != point.Y ||
					stage.SpawnX != point.X || stage.SpawnY != point.Y || !terrain.Walkable(stage.X, stage.Y) {
					t.Errorf("room %d is repeated, blocked or detached from its source: %+v", i+1, key)
				}
				rooms[key] = true
				if stage.Generator != tc.file || stage.GeneratorMode != "initial_group" ||
					len(stage.GeneratorIDs) != 1 || stage.GeneratorIDs[0] != tc.first+i || g.Label != tc.first+i {
					t.Errorf("room %d lost its dedicated generator selector", i+1)
				}
				count := 0
				for _, spawn := range stage.Spawns {
					count += spawn.Count
					if spawn.X != point.X || spawn.Y != point.Y || spawn.Range != int(point.Range) {
						t.Errorf("room %d lost native placement", i+1)
					}
				}
				if count != min(1+g.MinGroup, g.MaxNumMob) {
					t.Errorf("room %d uses capacity instead of its initial group: %d", i+1, count)
				}
				if i < 24 {
					q := stage.Quiz
					if q == nil || !terrain.Walkable(q.TrueX, q.TrueY) || !terrain.Walkable(q.FalseX, q.FalseY) {
						t.Errorf("room %d lost its O/X platforms", i+1)
					}
				}
			}
			last := rule.Instance.Stages[24].Spawns
			if len(last) != 2 || last[0].NPC != tc.boss || last[0].Count != 1 ||
				last[1].NPC != tc.follower || last[1].Count != tc.finalCount-1 {
				t.Fatalf("final wave lost boss/follower composition: %+v", last)
			}
			if tc.item == 1772 {
				for i, want := range map[int][2]uint16{5: {1770, 4054}, 11: {1702, 3986}} {
					s := rule.Instance.Stages[i]
					if [2]uint16{s.X, s.Y} != want {
						t.Errorf("restored Normal room %d has wrong coordinates", i+1)
					}
				}
			}
		})
	}
}

func TestInstanceGeneratorInitialGroupValidation(t *testing.T) {
	for _, scenario := range []string{"valid", "same-template", "capacity", "zero-followers", "variable", "disabled", "unknown-mode", "missing-generator", "missing-block", "missing-follower"} {
		t.Run(scenario, func(t *testing.T) {
			g := model.NPCGener{Label: 1, Enabled: true, Leader: "Boss", Follower: "Helper", MaxNumMob: 100, MinGroup: 14, MaxGroup: 14}
			g.Segments[0] = model.GenerSegment{X: 100, Y: 200, Range: 3}
			stage := model.VolatileInstanceStage{Generator: "wave.txt", GeneratorIDs: []int{1}, GeneratorMode: "initial_group"}
			want, valid := 15, true
			switch scenario {
			case "same-template":
				g.Follower = g.Leader
			case "capacity":
				g.MaxNumMob, want = 5, 5
			case "zero-followers":
				g.MinGroup, g.MaxGroup, want = 0, 0, 1
			case "variable":
				g.MaxGroup, valid = 15, false
			case "disabled":
				g.Enabled, valid = false, false
			case "unknown-mode":
				stage.GeneratorMode, valid = "invalid", false
			case "missing-generator":
				stage.Generator, valid = "", false
			case "missing-block":
				stage.GeneratorIDs, valid = []int{2}, false
			case "missing-follower":
				g.Follower, valid = "", false
			}
			cache := &instanceGeneratorCache{files: map[string][]model.NPCGener{"wave.txt": {g}}}
			inst := &model.VolatileInstance{Stages: []model.VolatileInstanceStage{stage}}
			err := cache.expand("initial group", inst)
			if (err == nil) != valid {
				t.Fatalf("validation = %v, valid=%v", err, valid)
			}
			if !valid {
				return
			}
			count := 0
			for _, spawn := range inst.Stages[0].Spawns {
				count += spawn.Count
			}
			if count != want {
				t.Fatalf("population=%d, want=%d", count, want)
			}
		})
	}
}
