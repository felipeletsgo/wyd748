package data

import (
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestCatalogNumericHelpersRejectMalformedValues(t *testing.T) {
	for _, tc := range []struct {
		name string
		raw  string
	}{
		{name: "integer text", raw: "abc"},
		{name: "empty dotted component", raw: "1..x"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if tc.name == "integer text" {
				if _, err := integer(tc.raw); err == nil {
					t.Fatal("invalid integer was accepted")
				}
				return
			}
			if _, err := dotted(tc.raw, 3); err == nil {
				t.Fatal("invalid dotted component was accepted")
			}
		})
	}
	values, err := dotted(" 1..3.4 ", 3)
	if err != nil || len(values) != 3 || values[0] != 1 || values[1] != 0 || values[2] != 3 {
		t.Fatalf("dotted fields = %v, err=%v", values, err)
	}
}

func TestItemEffectLoaderRejectsMalformedAndDuplicateDefinitions(t *testing.T) {
	for _, tc := range []struct {
		name string
		body string
		want string
	}{
		{name: "invalid value", body: "#define EF_A no\n", want: "invalid ID"},
		{name: "zero value", body: "#define EF_A 0\n", want: "invalid ID"},
		{name: "too large value", body: "#define EF_A 256\n", want: "invalid ID"},
		{name: "duplicate id", body: "#define EF_A 1\n#define EF_B 1\n", want: "duplicate ID"},
		{name: "duplicate name", body: "#define EF_A 1\n#define EF_A 2\n", want: "duplicate effect"},
		{name: "no definitions", body: "// no item effects\n", want: "no EF_* definitions"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			path := writeCatalogCoverageFile(t, "ItemEffect.h", tc.body)
			if _, err := loadItemEffects(path); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("loadItemEffects error = %v, want substring %q", err, tc.want)
			}
		})
	}
	if _, err := loadItemEffects(filepath.Join(t.TempDir(), "missing.h")); err == nil || !strings.Contains(err.Error(), "open ItemEffect") {
		t.Fatalf("missing ItemEffect error = %v", err)
	}
}

func TestRecordsAndItemListRejectInvalidInputs(t *testing.T) {
	if err := records(filepath.Join(t.TempDir(), "missing.csv"), func([]string) error { return nil }); err == nil {
		t.Fatal("records accepted a missing file")
	}
	path := writeCatalogCoverageFile(t, "records.csv", " # comment\n1,ok\n")
	visited := 0
	if err := records(path, func(row []string) error {
		visited++
		if len(row) != 2 || row[1] != "ok" {
			t.Fatalf("record = %v", row)
		}
		return nil
	}); err != nil || visited != 1 {
		t.Fatalf("records visited %d rows, err=%v", visited, err)
	}
	wantVisitErr := errors.New("rejected record")
	if err := records(path, func([]string) error { return wantVisitErr }); err == nil || !strings.Contains(err.Error(), wantVisitErr.Error()) {
		t.Fatalf("records callback error = %v", err)
	}

	valid := []string{"1", " Item ", "1.2", "3.4.5.6.7", "0", "10", "2", "3", "4"}
	for _, tc := range []struct {
		name   string
		mutate func([]string) []string
	}{
		{name: "too few columns", mutate: func(row []string) []string { return row[:8] }},
		{name: "non-numeric index", mutate: func(row []string) []string { row[0] = "x"; return row }},
		{name: "zero index", mutate: func(row []string) []string { row[0] = "0"; return row }},
		{name: "invalid mesh", mutate: func(row []string) []string { row[2] = "1.x"; return row }},
		{name: "invalid requirements", mutate: func(row []string) []string { row[3] = "1.2.x.4.5"; return row }},
		{name: "invalid unique", mutate: func(row []string) []string { row[4] = "x"; return row }},
		{name: "invalid price", mutate: func(row []string) []string { row[5] = "x"; return row }},
		{name: "negative price", mutate: func(row []string) []string { row[5] = "-1"; return row }},
		{name: "invalid position", mutate: func(row []string) []string { row[6] = "x"; return row }},
		{name: "invalid extra", mutate: func(row []string) []string { row[7] = "x"; return row }},
		{name: "invalid grade", mutate: func(row []string) []string { row[8] = "x"; return row }},
		{name: "invalid static effect", mutate: func(row []string) []string { return append(row, "EF_A", "x") }},
	} {
		t.Run(tc.name, func(t *testing.T) {
			row := append([]string(nil), valid...)
			path := writeCatalogCoverageFile(t, "itemlist.csv", strings.Join(tc.mutate(row), ",")+"\n")
			if _, err := loadItemList(path); err == nil {
				t.Fatalf("invalid itemlist row was accepted: %q", tc.name)
			}
		})
	}

	validPath := writeCatalogCoverageFile(t, "valid-item.csv", strings.Join(valid, ",")+"\n")
	items, err := loadItemList(validPath)
	if err != nil || items[1].Name != "Item" || items[1].Price != 10 {
		t.Fatalf("valid itemlist = %+v, err=%v", items, err)
	}
}

func TestItemNameAndSkillLoadersRejectInvalidRows(t *testing.T) {
	items := map[uint16]model.ItemDef{1: {Index: 1, Name: "old"}}
	for _, row := range []string{"1", "bad,name", "0,name"} {
		path := writeCatalogCoverageFile(t, "names.csv", row+"\n")
		if err := loadItemNames(path, items); err == nil {
			t.Fatalf("invalid Itemname row %q was accepted", row)
		}
	}
	path := writeCatalogCoverageFile(t, "names-valid.csv", "1, New Name \n2,ignored\n")
	if err := loadItemNames(path, items); err != nil || items[1].Name != "New Name" {
		t.Fatalf("valid Itemname update = %+v, err=%v", items[1], err)
	}
	if err := loadItemNames(filepath.Join(t.TempDir(), "missing.csv"), items); err == nil {
		t.Fatal("Itemname accepted a missing file")
	}

	validSkill := make([]string, 23)
	for i := range validSkill {
		validSkill[i] = "0"
	}
	validSkill[22] = "Skill"
	for _, tc := range []struct {
		name   string
		mutate func([]string) []string
	}{
		{name: "wrong number of columns", mutate: func(row []string) []string { return row[:22] }},
		{name: "invalid parameter", mutate: func(row []string) []string { row[3] = "x"; return row }},
		{name: "fractional animation", mutate: func(row []string) []string { row[13] = "1.x"; return row }},
		{name: "animation byte overflow", mutate: func(row []string) []string { row[13] = "256.0.0.0.0.0.0.0"; return row }},
		{name: "alternate animation byte overflow", mutate: func(row []string) []string { row[14] = "0.0.0.0.0.0.0.256"; return row }},
		{name: "invalid tail value", mutate: func(row []string) []string { row[15] = "x"; return row }},
	} {
		t.Run(tc.name, func(t *testing.T) {
			row := tc.mutate(append([]string(nil), validSkill...))
			path := writeCatalogCoverageFile(t, "skills.csv", strings.Join(row, ",")+"\n")
			if _, err := loadSkills(path); err == nil {
				t.Fatalf("invalid SkillData row was accepted: %q", tc.name)
			}
		})
	}
	validPath := writeCatalogCoverageFile(t, "valid-skills.csv", strings.Join(validSkill, ",")+"\n")
	if skills, err := loadSkills(validPath); err != nil || skills[0].Name != "Skill" {
		t.Fatalf("valid SkillData = %v, err=%v", skills, err)
	}
}

func TestCatalogRejectsInvalidStaticEffectsAndArgumentShapes(t *testing.T) {
	for _, name := range []string{"unknown", "256", "0"} {
		items := map[uint16]model.ItemDef{1: {Index: 1, StaticEffects: []model.StaticEffect{{Name: name}}}}
		if err := validateStaticEffects(items, map[byte]string{1: "EF_ONE"}); err == nil {
			t.Fatalf("unknown static effect %q was accepted", name)
		}
	}
	if _, err := LoadCatalog("itemlist.csv", "Itemname.csv"); err == nil {
		t.Fatal("LoadCatalog accepted an unsupported argument count")
	}
}

func writeCatalogCoverageFile(t *testing.T, name, body string) string {
	t.Helper()
	path := filepath.Join(t.TempDir(), name)
	if err := os.WriteFile(path, []byte(body), 0o600); err != nil {
		t.Fatal(err)
	}
	return path
}
