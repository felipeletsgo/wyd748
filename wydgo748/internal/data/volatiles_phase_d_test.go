package data

import (
	"testing"

	"wydgo/internal/model"
)

func TestPhaseDActionRegistry(t *testing.T) {
	for _, action := range []string{"magical_pill", "hunting_teleport", "summon_contract", "learn_special_skill"} {
		if !validVolatileActions[action] {
			t.Fatalf("acao %q ausente", action)
		}
	}
}

func TestPhaseDHuntingDestinationShape(t *testing.T) {
	r := model.VolatileRule{Action: "hunting_teleport", Destinations: make([]model.VolatileDestination, 10)}
	if len(r.Destinations) != 10 {
		t.Fatal("tabela nativa deve possuir 10 destinos")
	}
}

func TestGuardLettersUseSeparateItemRules(t *testing.T) {
	root := "../../data"
	catalog, err := LoadCatalog(root+"/catalog/itemlist.csv", root+"/catalog/Itemname.csv", root+"/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	rules := loadRealVolatiles(t, root, catalog)
	guard, guardCode, guardOK := rules.Rule(436)
	sophia, sophiaCode, sophiaOK := rules.Rule(438)
	if !guardOK || guardCode != 41 || guard.Summon == nil || !guard.Summon.Tank || guard.Summon.Refinement != 9 {
		t.Fatalf("invalid blue knight guard rule: %+v", guard)
	}
	if !sophiaOK || sophiaCode != 43 || sophia.Summon == nil || !sophia.Summon.Healer ||
		sophia.Summon.Shield != 1705 || sophia.Summon.Refinement != 9 {
		t.Fatalf("invalid Sophia rule: %+v", sophia)
	}
	for _, item := range []uint16{437, 439, 440, 441, 3972, 3973} {
		rule, _, ok := rules.Rule(item)
		if !ok || rule.Summon == nil || rule.Summon.Tank || rule.Summon.Healer {
			t.Fatalf("item %d lost its ordinary summon rule: %+v", item, rule)
		}
	}
}
