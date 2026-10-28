package data

import (
	"encoding/json"
	"os"
	"testing"
	"wydgo/internal/model"
)

// These historical sources verify dedicated variants without restoring unused runtime NPCs.
func historicalInstanceDonors(t *testing.T) map[string]*model.NPCDef {
	t.Helper()
	raw, err := os.ReadFile("testdata/instance-donors.json")
	if err != nil {
		t.Fatal(err)
	}
	var definitions []model.NPCDef
	if err := json.Unmarshal(raw, &definitions); err != nil {
		t.Fatal(err)
	}
	donors := make(map[string]*model.NPCDef, len(definitions))
	for i := range definitions {
		donors[definitions[i].Name] = &definitions[i]
	}
	return donors
}
