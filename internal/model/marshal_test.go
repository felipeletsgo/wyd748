package model

import (
	"bytes"
	"encoding/json"
	"testing"
)

func TestMarshalCP(t *testing.T) {
	c := Char{
		Name:  "Test",
		CP:    -25,
		Gold:  1000,
		Score: &Score{Version: ScoreVersion},
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
		Name: "Test", Score: &Score{Version: ScoreVersion},
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

func TestUnmarshalMigratesMissingLegacyMechanicToNeutralCP(t *testing.T) {
	c := Char{Name: "Test", Score: &Score{Version: ScoreVersion}}
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
	if migrated.CP != 0 {
		t.Fatalf("chaos legado zero deveria preservar personagem neutro em CP 0, veio %d", migrated.CP)
	}
}

func TestUnmarshalMigratesOldExplicitNeutralCP(t *testing.T) {
	seed := Char{
		Name:  "Old",
		CP:    MaxCP,
		Score: &Score{Version: ScoreVersion},
	}
	raw, err := json.Marshal(seed)
	if err != nil {
		t.Fatal(err)
	}
	var legacy map[string]any
	if err := json.Unmarshal(raw, &legacy); err != nil {
		t.Fatal(err)
	}
	delete(legacy, "chaosVersion")
	raw, err = json.Marshal(legacy)
	if err != nil {
		t.Fatal(err)
	}
	var migrated Char
	if err := json.Unmarshal(raw, &migrated); err != nil {
		t.Fatal(err)
	}
	if migrated.CP != 0 || migrated.ChaosVersion != ChaosFormatVersion {
		t.Fatalf("cp antigo 75 nao migrou para neutro: cp=%d versao=%d",
			migrated.CP, migrated.ChaosVersion)
	}
}

func TestSignedCPIsPreservedAfterChaosFormatMarker(t *testing.T) {
	seed := Char{
		Name:         "Current",
		CP:           MaxCP,
		ChaosVersion: ChaosFormatVersion,
		Score:        &Score{Version: ScoreVersion},
	}
	raw, err := json.Marshal(seed)
	if err != nil {
		t.Fatal(err)
	}
	var current Char
	if err := json.Unmarshal(raw, &current); err != nil {
		t.Fatal(err)
	}
	if current.CP != 75 || current.ChaosVersion != ChaosFormatVersion {
		t.Fatalf("cp assinado marcado foi alterado: cp=%d versao=%d",
			current.CP, current.ChaosVersion)
	}
}
