package data

import (
	"encoding/json"
	"path/filepath"
	"reflect"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestHellGateHistoricalPopulationValidation(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	items, err := LoadCatalog(filepath.Join(root, "itemlist.csv"), filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	catalog := loadRealVolatiles(t, root, items)
	gate := catalog.Items[3328].Instance.HellGate
	encoded, err := json.Marshal(gate)
	if err != nil {
		t.Fatal(err)
	}
	for _, reason := range []string{"valid", "old-version", "missing-room", "empty-room", "empty-escorts", "missing-npc", "oversized-group", "unsafe-range", "actor-budget"} {
		t.Run(reason, func(t *testing.T) {
			var candidate model.VolatileHellGate
			if err := json.Unmarshal(encoded, &candidate); err != nil {
				t.Fatal(err)
			}
			p := candidate.Population759
			if p == nil {
				t.Fatal("installed historical population missing")
			}
			switch reason {
			case "old-version":
				candidate.Version = 2
			case "missing-room":
				p.Rooms = p.Rooms[:3]
			case "empty-room":
				p.Rooms[0] = nil
			case "empty-escorts":
				p.LichEscorts[1] = nil
			case "missing-npc":
				p.FinalSpawns[0].NPC = ""
			case "oversized-group":
				p.FinalSpawns[0].Count = 65
			case "unsafe-range":
				p.FinalSpawns[0].Range = 9
			case "actor-budget":
				for i := range p.FinalSpawns {
					p.FinalSpawns[i].Count = 64
				}
			}
			err := validateHellGateVersion(reason, &candidate)
			if (err == nil) != (reason == "valid") {
				t.Fatalf("population %s: %v", reason, err)
			}
		})
	}
}

func TestHellGateDedicatedGeneratorDefinitions(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	items, err := LoadCatalog(filepath.Join(root, "itemlist.csv"), filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	catalog := loadRealVolatiles(t, root, items)
	inst := catalog.Items[3328].Instance
	if inst == nil || inst.HellGate == nil || inst.HellGate.Generator != "NPCGener/HellGate.txt" {
		t.Fatal("Hell Gate has no dedicated generator file")
	}
	geners, err := LoadNPCGener(filepath.Join(root, "NPCGener", "HellGate.txt"))
	if err != nil {
		t.Fatal(err)
	}
	if len(geners) != 15 {
		t.Fatalf("Hell Gate generator block count = %d, want 15", len(geners))
	}
	byID := make(map[int]model.NPCGener)
	for _, g := range geners {
		if g.MinuteGenerate != -1 || !strings.HasPrefix(g.Leader, "HG_") || g.Follower != g.Leader {
			t.Fatalf("Hell Gate generator is not instance-only and dedicated: %+v", g)
		}
		byID[g.Label] = g
	}
	check := func(s model.VolatileInstanceSpawn) {
		t.Helper()
		g, ok := byID[s.GeneratorID]
		if !ok || s.NPC != g.Leader || s.Count != g.MaxNumMob || s.X != g.Segments[0].X ||
			s.Y != g.Segments[0].Y || s.Range != int(g.Segments[0].Range) {
			t.Fatalf("spawn does not match its selected generator: %+v", s)
		}
		delete(byID, s.GeneratorID)
	}
	// Stage selectors are expanded by the existing Nightmare/Carta loader.
	entry := inst.Stages[0]
	if entry.Generator != inst.HellGate.Generator || len(entry.Spawns) != 1 || entry.Spawns[0].NPC != inst.HellGate.ControllerNPC {
		t.Fatal("entry and controller do not use the Hell Gate generator")
	}
	delete(byID, entry.GeneratorIDs[0])
	for _, s := range inst.HellGate.Corridors {
		check(s)
	}
	for _, q := range inst.HellGate.Quadrants {
		for _, s := range q.Spawns {
			check(s)
		}
		check(q.Lich)
	}
	for _, s := range inst.HellGate.FinalSpawns {
		check(s)
	}
	if len(byID) != 0 {
		t.Fatalf("unused Hell Gate generators: %v", byID)
	}
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]model.NPCDef)
	for _, npc := range npcs {
		byName[npc.Name] = npc
	}
	if got, want := byName["HG_Lich"].Equip.Rosto, byName["Sand_Lich"].Equip.Rosto; got != want || got.Index != 4088 {
		t.Fatalf("Hell Gate Lich face = %+v, want the existing KR Sand Lich face %+v", got, want)
	}
	for dedicated, source := range map[string]string{"HG_Spider": "Hell_Spider", "HG_Skeleton": "Skeleton", "HG_Lich": "Lich_Dragon"} {
		got, want := byName[dedicated], byName[source]
		if got.Name == "" || !got.IsMonster() || got.Score == nil || got.Score == want.Score {
			t.Fatalf("%s is missing or reuses a shared score", dedicated)
		}
		want.Name = dedicated
		if dedicated == "HG_Lich" {
			// The user-selected KR Sand Lich replaces only the Dragon Lich face.
			want.Equip.Rosto = model.Item{Index: 4088, Eff: [6]byte{43, 7, 0, 0, 0, 0}}
		}
		if dedicated == "HG_Spider" || dedicated == "HG_Lich" {
			calibrated := *want.Score
			calibrated.MaxHP = got.Score.MaxHP
			calibrated.CurHP = got.Score.CurHP
			want.Score = &calibrated
		}
		if !reflect.DeepEqual(got, want) {
			t.Errorf("%s changed existing balance, visuals or loot while separating templates", dedicated)
		}
	}
	for _, tc := range []struct {
		name    string
		monster bool
		maxHP   uint32
	}{
		{name: "HG_Spider", monster: true, maxHP: 400000},
		{name: "HG_Corridor", monster: true, maxHP: 400000},
		{name: "HG_Lich", monster: true, maxHP: 400000},
		{name: "HG_Dionys", monster: false, maxHP: 150000},
	} {
		npc, ok := byName[tc.name]
		if !ok || npc.Score == nil || npc.IsMonster() != tc.monster ||
			npc.Score.MaxHP != tc.maxHP || npc.Score.CurHP != tc.maxHP {
			t.Errorf("%s calibration = %+v, want monster=%t maxHP=%d", tc.name, npc.Score, tc.monster, tc.maxHP)
		}
	}
}

func TestHellGateGeneratorRejectsAmbiguousSpawns(t *testing.T) {
	for _, tc := range []struct {
		name string
		gate model.VolatileHellGate
	}{
		{"missing file", model.VolatileHellGate{FinalSpawns: []model.VolatileInstanceSpawn{{GeneratorID: 1}}}},
		{"missing selector", model.VolatileHellGate{Generator: "NPCGener/HellGate.txt", FinalSpawns: []model.VolatileInstanceSpawn{{NPC: "HG_Skeleton", Count: 1}}}},
		{"inline conflict", model.VolatileHellGate{Generator: "NPCGener/HellGate.txt", FinalSpawns: []model.VolatileInstanceSpawn{{NPC: "HG_Skeleton", GeneratorID: 10}}}},
		{"unknown block", model.VolatileHellGate{Generator: "NPCGener/HellGate.txt", FinalSpawns: []model.VolatileInstanceSpawn{{GeneratorID: 999}}}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			cache := &instanceGeneratorCache{dir: filepath.Join("..", "..", "data"), files: make(map[string][]model.NPCGener)}
			if err := cache.expandHellGate(tc.name, &tc.gate); err == nil {
				t.Fatal("invalid Hell Gate generator configuration was accepted")
			}
		})
	}
}
