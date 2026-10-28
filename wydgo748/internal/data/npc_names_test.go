package data

import (
	"regexp"
	"testing"

	"wydgo/internal/model"
)

func TestNPCNamesFitWire(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	byID := make(map[string]string, len(npcs))
	suffixed := regexp.MustCompile(`_[0-9a-f]{4}$`)
	for _, npc := range npcs {
		byID[npc.Name] = npc.VisibleName()
		if suffixed.MatchString(npc.Name) {
			t.Errorf("artificial template identifier retained: %q", npc.Name)
		}
		if name := npc.VisibleName(); len(name) > model.MaxNPCDisplayNameBytes {
			t.Errorf("NPC %q visible name %q exceeds %d bytes", npc.Name, name, model.MaxNPCDisplayNameBytes)
		}
	}
	for id, want := range map[string]string{
		"ActKerdanChief":  "ActKerdanChief",
		"Aranha_Dourada":  "Aranha_Dourada",
		"Berserker_Titan": "Berserker_Titan",
	} {
		if got := byID[id]; got != want {
			t.Errorf("shared NPC %q visible name = %q, want %q", id, got, want)
		}
	}
}

func TestNPCNamesHaveSupportedEncoding(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	for _, npc := range npcs {
		if npc.Name == "" {
			t.Error("empty NPC identifier")
		}
		// These shared identifiers may contain English or Portuguese words.
		// The unchanged byte-copy wire path supports their ASCII spelling;
		// localized display names need a separate encoding contract.
		for _, b := range []byte(npc.Name) {
			if b < 0x20 || b > 0x7e {
				t.Errorf("NPC identifier %q contains unsupported name byte 0x%02x", npc.Name, b)
				break
			}
		}
	}
}

// NPCGener, quest bindings and npcDefByName all use the same identifier.
func TestNPCNamesAreUnique(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	seen := make(map[string]bool, len(npcs))
	for _, npc := range npcs {
		if seen[npc.Name] {
			t.Errorf("duplicate NPC identifier: %q", npc.Name)
		}
		seen[npc.Name] = true
	}
}

func TestBaseGeneratorsResolveTemplateNames(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	groups, err := LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	names := make(map[string]bool, len(npcs))
	for _, npc := range npcs {
		names[npc.Name] = true
	}
	count := 0
	for _, group := range groups {
		if !group.Enabled {
			continue
		}
		count++
		for _, name := range []string{group.Leader, group.Follower} {
			if !names[name] {
				t.Errorf("generator %d has no direct template %q", group.Index, name)
			}
		}
	}
	if count == 0 {
		t.Fatal("no enabled generators exercised")
	}
}
