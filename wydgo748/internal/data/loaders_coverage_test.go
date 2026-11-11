package data

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestLoadQuestZonesStrictFileContract(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "zones.json")
	missing, err := LoadQuestZones(filepath.Join(dir, "missing.json"))
	if err != nil || missing.Version != model.QuestZoneFileVersion {
		t.Fatalf("missing optional file=%+v err=%v", missing, err)
	}

	valid := model.QuestZoneFile{Version: model.QuestZoneFileVersion,
		Zones:      []model.QuestZone{{Name: "quest", NPC: "Aki", X1: 10, Y1: 20, X2: 30, Y2: 40}},
		ClearAreas: []model.QuestZone{{Name: "clear", X1: 1, Y1: 1, X2: 2, Y2: 2}},
	}
	encoded, err := json.Marshal(valid)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(path, encoded, 0o600); err != nil {
		t.Fatal(err)
	}
	loaded, err := LoadQuestZones(path)
	if err != nil || len(loaded.Zones) != 1 || len(loaded.ClearAreas) != 1 {
		t.Fatalf("loaded=%+v err=%v", loaded, err)
	}

	for _, body := range []string{
		"{",
		`{"version":1,"unknown":true}`,
		`{"version":1,"zones":[]} {}`,
		`{"version":2,"zones":[]}`,
		`{"version":1,"zones":[{"name":"same","x1":1,"y1":1,"x2":2,"y2":2}],"clearAreas":[{"name":"same","x1":1,"y1":1,"x2":2,"y2":2}]}`,
		`{"version":1,"zones":[{"name":"inverted","x1":3,"y1":1,"x2":2,"y2":2}]}`,
	} {
		if err := os.WriteFile(path, []byte(body), 0o600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadQuestZones(path); err == nil {
			t.Fatalf("invalid quest zone file accepted: %s", body)
		}
	}
}

func TestLoadColosseumRequiresExpectedEventGenerators(t *testing.T) {
	dir := t.TempDir()
	writeSet := func(t *testing.T, override string) {
		t.Helper()
		for tier, file := range colosseumFiles {
			content := fmt.Sprintf("#[%d]\nLeader: EventMob\nMinuteGenerate: -1\n", file.labels[0])
			if len(file.labels) > 1 {
				for _, label := range file.labels[1:] {
					content += fmt.Sprintf("#[%d]\nLeader: EventMob\nMinuteGenerate: -1\n", label)
				}
			}
			if tier == model.ColosseumTierN && override != "" {
				content = override
			}
			if err := os.WriteFile(filepath.Join(dir, file.name), []byte(content), 0o600); err != nil {
				t.Fatal(err)
			}
		}
	}
	writeSet(t, "")
	loaded, err := LoadColosseum(dir)
	if err != nil {
		t.Fatal(err)
	}
	for tier, file := range colosseumFiles {
		if len(loaded[tier]) != len(file.labels) {
			t.Fatalf("tier %d generators=%d want %d", tier, len(loaded[tier]), len(file.labels))
		}
	}

	for _, invalid := range []string{
		"#* [5]\nLeader: EventMob\nMinuteGenerate: -1\n",
		"#[5]\nLeader: EventMob\nMinuteGenerate: 0\n",
		"#[99]\nLeader: EventMob\nMinuteGenerate: -1\n",
	} {
		writeSet(t, invalid)
		if _, err := LoadColosseum(dir); err == nil {
			t.Fatalf("invalid event generator set accepted: %q", invalid)
		}
	}
	if err := os.Remove(filepath.Join(dir, colosseumFiles[model.ColosseumTierA].name)); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadColosseum(dir); err == nil {
		t.Fatal("missing tier file accepted")
	}
}

func TestValidateNPCDefinitionUsesNPCAndItemContracts(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs/Aki.json")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	validated, err := ValidateNPCDefinition(npcs[0], catalog.Items)
	if err != nil || validated.Name != npcs[0].Name {
		t.Fatalf("validated NPC=%+v err=%v", validated, err)
	}
	if _, err := ValidateNPCDefinition(npcs[0], map[uint16]model.ItemDef{}); err == nil {
		t.Fatal("NPC that references an unknown item was accepted")
	}
}

func TestSetGenerFieldUsesStrictFieldParsing(t *testing.T) {
	var generator model.NPCGener
	for _, field := range [][2]string{{"Leader:", "Wolf"}, {"Follower:", "Mage"}, {"StartX:", "2200"}, {"StartY:", "2100"}, {"StartRange:", "8"}, {"DestAction:", "return"}} {
		if err := setGenerField(&generator, field[0], field[1]); err != nil {
			t.Fatalf("set %s: %v", field[0], err)
		}
	}
	if generator.Leader != "Wolf" || generator.Follower != "Mage" || generator.Segments[0].X != 2200 || generator.Segments[4].Action != "return" {
		t.Fatalf("generator fields not retained: %+v", generator)
	}
	if err := setGenerField(&generator, "Unknown:", "not-an-integer"); err != nil {
		t.Fatalf("unknown field should be ignored: %v", err)
	}
	for _, field := range [][2]string{{"MaxNumMob:", "x"}, {"StartX:", "4096"}, {"StartRange:", "-1"}} {
		if err := setGenerField(&generator, field[0], field[1]); err == nil {
			t.Fatalf("invalid field %s=%s accepted", field[0], field[1])
		}
	}
}
