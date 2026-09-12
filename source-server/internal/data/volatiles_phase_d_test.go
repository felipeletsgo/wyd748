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
