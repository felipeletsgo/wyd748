package model

import (
	"bytes"
	"encoding/json"
	"testing"
)

func TestMarshalCP(t *testing.T) {
	c := Char{
		Name:     "Test",
		CP:       -25,
		Gold:     1000,
		Extended: &ExtendedScore{Version: ExtendedScoreVersion},
	}

	b, err := json.Marshal(c)
	if err != nil {
		t.Fatal(err)
	}

	t.Logf("Marshaled: %s", string(b))

	var c2 Char
	if err := json.Unmarshal(b, &c2); err != nil {
		t.Fatal(err)
	}

	if c2.CP != -25 {
		t.Fatalf("Expected CP -25, got %d", c2.CP)
	}
}

func TestUnmarshalMigratesLegacyChaosByteToSignedCP(t *testing.T) {
	c := Char{
		Name: "Test", Extended: &ExtendedScore{Version: ExtendedScoreVersion},
	}
	b, err := json.Marshal(c)
	if err != nil {
		t.Fatal(err)
	}
	var raw map[string]any
	if err := json.Unmarshal(b, &raw); err != nil {
		t.Fatal(err)
	}
	delete(raw, "cp")
	raw["chaos"] = 150
	b, err = json.Marshal(raw)
	if err != nil {
		t.Fatal(err)
	}
	var migrated Char
	if err := json.Unmarshal(b, &migrated); err != nil {
		t.Fatal(err)
	}
	if migrated.CP != 75 {
		t.Fatalf("chaos bruto 150 deveria migrar para CP +75, veio %d", migrated.CP)
	}
	out, err := json.Marshal(migrated)
	if err != nil {
		t.Fatal(err)
	}
	if string(out) == "" || bytes.Contains(out, []byte(`"chaos"`)) {
		t.Fatalf("formato legado continuou persistido: %s", out)
	}
}

func TestUnmarshalMigratesMissingLegacyMechanicToCleanCP(t *testing.T) {
	c := Char{Name: "Test", Extended: &ExtendedScore{Version: ExtendedScoreVersion}}
	b, err := json.Marshal(c)
	if err != nil {
		t.Fatal(err)
	}
	var raw map[string]any
	if err := json.Unmarshal(b, &raw); err != nil {
		t.Fatal(err)
	}
	delete(raw, "cp")
	raw["chaos"] = 0
	b, err = json.Marshal(raw)
	if err != nil {
		t.Fatal(err)
	}
	var migrated Char
	if err := json.Unmarshal(b, &migrated); err != nil {
		t.Fatal(err)
	}
	if migrated.CP != MaxCP {
		t.Fatalf("chaos legado zero deveria preservar personagem limpo em CP +75, veio %d", migrated.CP)
	}
}
