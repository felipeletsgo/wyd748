package control

import (
	"encoding/json"
	"testing"
)

func TestNPCDefinitionTargetsRequireBothExplicitVersions(t *testing.T) {
	base := NPCDefinitionCommand{Version: 1, Channel: 1, ExpectedVersion: 7, Name: "Shop", Reason: "Update stock", Definition: json.RawMessage(`{"name":"Shop"}`)}
	if err := base.Validate(); err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		name    string
		targets []NPCDefinitionTarget
		valid   bool
	}{
		{"both", []NPCDefinitionTarget{{1, 7}, {2, 3}}, true},
		{"explicit empty", []NPCDefinitionTarget{}, false},
		{"reverse order", []NPCDefinitionTarget{{2, 3}, {1, 7}}, true},
		{"only one", []NPCDefinitionTarget{{1, 7}}, false},
		{"duplicate", []NPCDefinitionTarget{{1, 7}, {1, 7}}, false},
		{"primary mismatch", []NPCDefinitionTarget{{1, 6}, {2, 3}}, false},
		{"missing version", []NPCDefinitionTarget{{1, 7}, {2, 0}}, false},
		{"overflow", []NPCDefinitionTarget{{1, 7}, {2, ^uint32(0)}}, false},
		{"unsupported channel", []NPCDefinitionTarget{{1, 7}, {3, 3}}, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			cmd := base
			cmd.Targets = tc.targets
			if (cmd.Validate() == nil) != tc.valid {
				t.Fatal("invalid target acceptance")
			}
		})
	}
}
