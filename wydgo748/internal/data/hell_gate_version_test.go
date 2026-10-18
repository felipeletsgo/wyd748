package data

import (
	"testing"

	"wydgo/internal/model"
)

func TestHellGateVersion2RejectsInvalidEncounterConfiguration(t *testing.T) {
	valid := func() *model.VolatileHellGate {
		return &model.VolatileHellGate{Version: 2, RewardEXP: 10_000_000,
			Corridors: []model.VolatileInstanceSpawn{
				{NPC: "Boss", Count: 1, X: 1, Y: 1},
				{NPC: "Boss", Count: 1, X: 1, Y: 5},
				{NPC: "Boss", Count: 1, X: 1, Y: 9}},
			Quadrants:   make([]model.VolatileHellGateQuadrant, 4),
			FinalSpawns: []model.VolatileInstanceSpawn{{NPC: "Attacker", Count: 1}},
			FinalNPCs:   []model.VolatileInstanceSpawn{{NPC: "Dionys", Count: 1}},
			Portals: []model.Teleport{
				{SourceX: 4, SourceY: 4, DestX: 40, DestY: 40},
				{SourceX: 4, SourceY: 8, DestX: 40, DestY: 40},
				{SourceX: 4, SourceY: 12, DestX: 40, DestY: 40}}}
	}
	if err := validateHellGateVersion("valid", valid()); err != nil {
		t.Fatal(err)
	}
	current := valid()
	current.Version = 3
	if err := validateHellGateVersion("historical puzzle", current); err != nil {
		t.Fatal(err)
	}
	for name, mutate := range map[string]func(*model.VolatileHellGate){
		"unsupported version":          func(g *model.VolatileHellGate) { g.Version = 4 },
		"missing EXP":                  func(g *model.VolatileHellGate) { g.RewardEXP = 0 },
		"missing corridors":            func(g *model.VolatileHellGate) { g.Corridors = nil },
		"multiple bosses per corridor": func(g *model.VolatileHellGate) { g.Corridors[0].Count = 2 },
		"no attackers":                 func(g *model.VolatileHellGate) { g.FinalSpawns = nil },
		"two protectors":               func(g *model.VolatileHellGate) { g.FinalNPCs[0].Count = 2 },
		"same portal block":            func(g *model.VolatileHellGate) { g.Portals[1].SourceY = 5 },
		"missing destination":          func(g *model.VolatileHellGate) { g.Portals[1].DestX = 0 },
		"priced portal":                func(g *model.VolatileHellGate) { g.Portals[1].Price = 1 },
	} {
		t.Run(name, func(t *testing.T) {
			gate := valid()
			mutate(gate)
			if validateHellGateVersion(name, gate) == nil {
				t.Fatal("invalid encounter was accepted")
			}
		})
	}
}
