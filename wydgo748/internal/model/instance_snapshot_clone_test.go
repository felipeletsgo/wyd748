package model

import "testing"

func TestInstanceStateSnapshotCloneDetachesMutableState(t *testing.T) {
	if (*InstanceStateSnapshot)(nil).Clone() != nil {
		t.Fatal("nil snapshot clone should remain nil")
	}
	original := &InstanceStateSnapshot{
		Instances:          []InstanceRuntimeState{{RuntimeID: "water:1", MemberCharacterUIDs: []string{"char-a", "char-b"}}},
		NightmarePartyRuns: map[string]int{"party-a": 2},
		Channels: map[byte]*InstanceStateSnapshot{
			2: {Instances: []InstanceRuntimeState{{RuntimeID: "nightmare:2", MemberCharacterUIDs: []string{"char-c"}}}},
		},
	}
	clone := original.Clone()
	clone.Instances[0].MemberCharacterUIDs[0] = "changed"
	clone.NightmarePartyRuns["party-a"] = 99
	clone.Channels[2].Instances[0].MemberCharacterUIDs[0] = "changed-channel"
	clone.Channels[3] = &InstanceStateSnapshot{}
	if original.Instances[0].MemberCharacterUIDs[0] != "char-a" || original.NightmarePartyRuns["party-a"] != 2 ||
		original.Channels[2].Instances[0].MemberCharacterUIDs[0] != "char-c" {
		t.Fatal("clone mutation changed the source snapshot")
	}
	if _, exists := original.Channels[3]; exists {
		t.Fatal("clone channel map shares ownership with source")
	}

	empty := (&InstanceStateSnapshot{}).Clone()
	if empty == nil || empty.NightmarePartyRuns == nil {
		t.Fatalf("empty clone should be initialized: %+v", empty)
	}
}
