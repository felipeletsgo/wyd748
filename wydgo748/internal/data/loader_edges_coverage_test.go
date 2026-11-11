package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestValidateInstanceSpawnShapeRejectsIncompleteSpawns(t *testing.T) {
	for _, test := range []struct {
		name   string
		spawns []model.VolatileInstanceSpawn
	}{
		{name: "empty", spawns: nil},
		{name: "blank NPC", spawns: []model.VolatileInstanceSpawn{{NPC: "  ", Count: 1}}},
		{name: "zero count", spawns: []model.VolatileInstanceSpawn{{NPC: "Boss", Count: 0}}},
		{name: "negative count", spawns: []model.VolatileInstanceSpawn{{NPC: "Boss", Count: -1}}},
		{name: "only X coordinate", spawns: []model.VolatileInstanceSpawn{{NPC: "Boss", Count: 1, X: 10}}},
		{name: "only Y coordinate", spawns: []model.VolatileInstanceSpawn{{NPC: "Boss", Count: 1, Y: 10}}},
	} {
		t.Run(test.name, func(t *testing.T) {
			if err := validateInstanceSpawnShape("fixture", "wave", test.spawns); err == nil {
				t.Fatal("invalid spawn shape was accepted")
			}
		})
	}
}

func TestValidateInstanceSpawnShapeAcceptsPairedCoordinates(t *testing.T) {
	for _, spawn := range []model.VolatileInstanceSpawn{
		{NPC: "Boss", Count: 1},
		{NPC: "Boss", Count: 2, X: 10, Y: 20},
	} {
		if err := validateInstanceSpawnShape("fixture", "wave", []model.VolatileInstanceSpawn{spawn}); err != nil {
			t.Fatalf("valid spawn %+v rejected: %v", spawn, err)
		}
	}
}

func TestLoadTeleportsRejectsMalformedInput(t *testing.T) {
	cases := []struct {
		name    string
		content string
		missing bool
	}{
		{name: "missing file", missing: true},
		{name: "empty file", content: ""},
		{name: "wrong column count", content: "1,2,3\n"},
		{name: "too many columns", content: "1,2,3,4,5,6,7\n"},
		{name: "malformed CSV", content: "\"1,2,3,4,5\n"},
		{name: "invalid number", content: "one,2,3,4,5\n"},
		{name: "number exceeds uint32", content: "4294967296,2,3,4,5\n"},
		{name: "coordinate outside map", content: "4096,2,3,4,5\n"},
		{name: "scanner token too long", content: strings.Repeat("x", 70_000) + "\n"},
	}
	for _, test := range cases {
		t.Run(test.name, func(t *testing.T) {
			path := filepath.Join(t.TempDir(), "teleports.ini")
			if !test.missing {
				if err := os.WriteFile(path, []byte(test.content), 0o600); err != nil {
					t.Fatal(err)
				}
			}
			if _, err := LoadTeleports(path); err == nil {
				t.Fatal("malformed teleport input was accepted")
			}
		})
	}
}

func TestLoadMountsHandlesMissingAndInvalidFiles(t *testing.T) {
	missing := filepath.Join(t.TempDir(), "missing-mounts.json")
	catalog, err := LoadMounts(missing)
	if err != nil || catalog.Types == nil || len(catalog.Types) != 0 {
		t.Fatalf("missing mount file result=%+v err=%v", catalog, err)
	}

	for _, test := range []struct {
		name    string
		content string
	}{
		{name: "malformed JSON", content: "{"},
		{name: "mount type outside table", content: `{"types":{"30":{}}}`},
		{name: "premium horse outside range", content: `{"premium":{"3979":{"speed":1,"durationDays":1}}}`},
	} {
		t.Run(test.name, func(t *testing.T) {
			path := filepath.Join(t.TempDir(), "mounts.json")
			if err := os.WriteFile(path, []byte(test.content), 0o600); err != nil {
				t.Fatal(err)
			}
			if _, err := LoadMounts(path); err == nil {
				t.Fatal("invalid mount file was accepted")
			}
		})
	}
}
