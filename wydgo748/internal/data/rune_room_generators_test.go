package data

import (
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func TestRuneTrackRoomGeneratorFiles(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(root, "catalog/itemlist.csv"), filepath.Join(root, "catalog/Itemname.csv"), filepath.Join(root, "catalog/SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	vc := loadRealVolatiles(t, root, catalog)
	cfg := vc.Instances["uxmal"].Uxmal
	if cfg == nil || cfg.Generator != "" || len(cfg.RoomGenerators) != 7 || len(cfg.Generators) != 327 {
		t.Fatal("Rune Track must load seven room files and all 327 generators")
	}
	if _, err := os.Stat(filepath.Join(root, "spawns/instances/RuneTrack759.txt")); !os.IsNotExist(err) {
		t.Fatal("obsolete combined Rune Track file still exists")
	}
	seen := map[int]model.NPCGener{}
	for room, count := range []int{53, 60, 60, 28, 50, 49, 27} {
		file := fmt.Sprintf("RuneTrack%d.txt", room)
		if cfg.RoomGenerators[room] != file {
			t.Fatalf("room %d uses wrong file", room)
		}
		generators, err := LoadNPCGener(filepath.Join(root, "spawns/instances", file))
		if err != nil || len(generators) != count {
			t.Fatalf("room %d count=%d error=%v", room, len(generators), err)
		}
		for _, g := range generators {
			if model.RuneTrackGeneratorRoom(g.Label) != room {
				t.Fatalf("room %d contains generator %d", room, g.Label)
			}
			if _, ok := seen[g.Label]; ok {
				t.Fatalf("duplicate generator %d", g.Label)
			}
			seen[g.Label] = g
		}
	}
	for _, g := range cfg.Generators {
		want := seen[g.Label]
		want.Index = g.Index // File-local ordinals become combined runtime ordinals.
		if !reflect.DeepEqual(g, want) {
			t.Fatalf("loader changed generator %d", g.Label)
		}
	}
}

func TestRuneTrackRoomGeneratorValidation(t *testing.T) {
	for _, scenario := range []string{"valid", "both-sources", "six-files", "empty-file-name", "duplicate-file", "missing-file", "wrong-room", "duplicate-label", "missing-label"} {
		t.Run(scenario, func(t *testing.T) {
			cache := &instanceGeneratorCache{dir: t.TempDir(), files: map[string][]model.NPCGener{}}
			cfg := &model.VolatileUxmal{Version: 759}
			for room := 0; room < 7; room++ {
				cfg.RoomGenerators = append(cfg.RoomGenerators, fmt.Sprintf("RuneTrack%d.txt", room))
			}
			for label := 5662; label <= 5988; label++ {
				file := cfg.RoomGenerators[model.RuneTrackGeneratorRoom(label)]
				cache.files[file] = append(cache.files[file], model.NPCGener{Label: label})
			}
			switch scenario {
			case "both-sources":
				cfg.Generator = "combined.txt"
			case "six-files":
				cfg.RoomGenerators = cfg.RoomGenerators[:6]
			case "empty-file-name":
				cfg.RoomGenerators[0] = ""
			case "duplicate-file":
				cfg.RoomGenerators[1] = cfg.RoomGenerators[0]
			case "missing-file":
				delete(cache.files, cfg.RoomGenerators[0])
			case "wrong-room":
				cache.files[cfg.RoomGenerators[0]][0].Label = 5715
			case "duplicate-label":
				cache.files[cfg.RoomGenerators[0]][1].Label = 5662
			case "missing-label":
				cache.files[cfg.RoomGenerators[0]] = cache.files[cfg.RoomGenerators[0]][1:]
			}
			inst := &model.VolatileInstance{Uxmal: cfg}
			err := cache.expand("Rune Track", inst)
			if (err == nil) != (scenario == "valid") {
				t.Fatalf("validation=%v", err)
			}
		})
	}
}
