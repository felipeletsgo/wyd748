package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestOwnerOfHandlesMissingCharacterAndParty(t *testing.T) {
	if got := ownerOf(nil); got != (groundOwner{}) {
		t.Fatalf("nil player owner = %+v", got)
	}
	if got := ownerOf(&Player{}); got != (groundOwner{}) {
		t.Fatalf("player without character owner = %+v", got)
	}

	party := &Party{}
	got := ownerOf(&Player{Char: &model.Char{UID: "character-uid"}, Party: party})
	if got.uid != "character-uid" || got.party != party {
		t.Fatalf("player owner = %+v, want UID and party", got)
	}
}

func TestResumableInstanceClassifiesPersistentAndTransientRooms(t *testing.T) {
	tests := []struct {
		name string
		inst *ItemInstance
		want bool
	}{
		{name: "nil"},
		{name: "water", inst: &ItemInstance{Config: model.VolatileInstance{ID: " Water-Room "}}, want: true},
		{name: "shared timed", inst: &ItemInstance{Config: model.VolatileInstance{Mode: " Shared_Timed_Zone "}}, want: true},
		{name: "hell gate", inst: &ItemInstance{Config: model.VolatileInstance{StateMachine: " Hell_Gate "}}, want: true},
		{name: "ordinary private", inst: &ItemInstance{Config: model.VolatileInstance{ID: "cube-room"}}},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := resumableInstance(tt.inst); got != tt.want {
				t.Fatalf("resumableInstance(%+v) = %t, want %t", tt.inst, got, tt.want)
			}
		})
	}
}

func TestCelestialEvolutionNormalizesKnownNames(t *testing.T) {
	for _, tt := range []struct {
		value string
		want  bool
	}{
		{value: " Celestial ", want: true},
		{value: "SUBCELESTIAL", want: true},
		{value: "mortal"},
		{value: ""},
	} {
		if got := celestialEvolution(tt.value); got != tt.want {
			t.Errorf("celestialEvolution(%q) = %t, want %t", tt.value, got, tt.want)
		}
	}
}

func TestFirstCargoSlotFindsFirstFreeAndReportsFull(t *testing.T) {
	if got := firstCargoSlot(nil); got != -1 {
		t.Fatalf("nil account slot = %d, want -1", got)
	}
	account := &model.Account{}
	if got := firstCargoSlot(account); got != 0 {
		t.Fatalf("empty cargo slot = %d, want 0", got)
	}
	account.Cargo[0].Index = 1
	if got := firstCargoSlot(account); got != 1 {
		t.Fatalf("next cargo slot = %d, want 1", got)
	}
	for i := 0; i < model.PlayerCargoSlots; i++ {
		account.Cargo[i].Index = 1
	}
	if got := firstCargoSlot(account); got != -1 {
		t.Fatalf("full cargo slot = %d, want -1", got)
	}
}

func TestItemInstanceKeyPrefersRuntimeIdentity(t *testing.T) {
	if got := itemInstanceKey(&ItemInstance{RuntimeID: "runtime:1", Config: model.VolatileInstance{ID: "base"}}); got != "runtime:1" {
		t.Fatalf("runtime instance key = %q", got)
	}
	if got := itemInstanceKey(&ItemInstance{Config: model.VolatileInstance{ID: "base"}}); got != "base" {
		t.Fatalf("base instance key = %q", got)
	}
}

func TestMinTimeSelectsEarlierDeadline(t *testing.T) {
	earlier := time.Date(2026, time.January, 1, 0, 0, 0, 0, time.UTC)
	later := earlier.Add(time.Minute)
	if got := minTime(earlier, later); !got.Equal(earlier) {
		t.Fatalf("minTime(earlier, later) = %v", got)
	}
	if got := minTime(later, earlier); !got.Equal(earlier) {
		t.Fatalf("minTime(later, earlier) = %v", got)
	}
}

func TestEvolutionPredicatesHandleNilAndBothCelestialForms(t *testing.T) {
	if isCelestial(nil) || isCelestialEvolution(nil) || advancedEvolution(nil, "celestial") {
		t.Fatal("nil character matched a celestial evolution")
	}
	for _, evolution := range []string{"CELESTIAL", "SubCelestial"} {
		character := &model.Char{Evolution: evolution}
		if !isCelestial(character) || !isCelestialEvolution(character) {
			t.Errorf("evolution %q was not recognized", evolution)
		}
	}
	if isCelestial(&model.Char{Evolution: "mortal"}) || isCelestialEvolution(&model.Char{Evolution: "mortal"}) ||
		advancedEvolution(&model.Char{Evolution: "mortal"}, "celestial", "subcelestial") ||
		advancedEvolution(&model.Char{Evolution: ""}) {
		t.Fatal("non-celestial evolution matched")
	}
}

func TestCharacterKingdomUsesCapeAndNeutralFallback(t *testing.T) {
	if got := characterKingdom(nil); got != model.KingdomNeutral {
		t.Fatalf("nil character kingdom = %d", got)
	}
	if got := characterKingdom(&model.Char{}); got != model.KingdomNeutral {
		t.Fatalf("character without a cape kingdom = %d", got)
	}
	for _, tt := range []struct {
		cape uint16
		want byte
	}{
		{cape: 545, want: model.KingdomHekalotia},
		{cape: 544, want: model.KingdomAkelonia},
	} {
		character := &model.Char{}
		character.Equip[model.CapeSlot].Index = tt.cape
		if got := characterKingdom(character); got != tt.want {
			t.Errorf("cape %d kingdom = %d, want %d", tt.cape, got, tt.want)
		}
	}
}

func TestHellGateBitValidatesQuadrants(t *testing.T) {
	for _, tt := range []struct {
		quadrant uint8
		want     uint8
	}{
		{quadrant: 0, want: 0},
		{quadrant: 1, want: 1},
		{quadrant: 4, want: 8},
		{quadrant: 8, want: 128},
		{quadrant: 9, want: 0},
	} {
		if got := hellGateBit(tt.quadrant); got != tt.want {
			t.Errorf("hellGateBit(%d) = %d, want %d", tt.quadrant, got, tt.want)
		}
	}
}

func TestAuthenticatedClientIPHelpersUseSocketOrigin(t *testing.T) {
	if _, ok := sessionRemoteIP(nil); ok {
		t.Fatal("nil session produced a remote IP")
	}
	if _, ok := authenticatedClientIPKey(nil); ok {
		t.Fatal("nil session produced an origin key")
	}
	valid := gameNet.NewTestSessionWithRemoteIP(1, 1, "192.0.2.7")
	if addr, ok := sessionRemoteIP(valid); !ok || addr.String() != "192.0.2.7" {
		t.Fatalf("remote IP = %v,%t", addr, ok)
	}
	if key, ok := authenticatedClientIPKey(valid); !ok || key != "192.0.2.7" {
		t.Fatalf("origin key = %q,%t", key, ok)
	}
	invalid := gameNet.NewTestSessionWithRemoteIP(2, 1, "not-an-ip")
	if _, ok := sessionRemoteIP(invalid); ok {
		t.Fatal("invalid remote IP was accepted")
	}
	if _, ok := authenticatedClientIPKey(invalid); ok {
		t.Fatal("invalid origin key was accepted")
	}
}

func TestPartyLeaderReturnsFirstMemberOrNil(t *testing.T) {
	var party *Party
	if party.leader() != nil || (&Party{}).leader() != nil {
		t.Fatal("empty party returned a leader")
	}
	leader, member := &Player{ID: 1}, &Player{ID: 2}
	if got := (&Party{Members: []*Player{leader, member}}).leader(); got != leader {
		t.Fatalf("party leader = %p, want first member %p", got, leader)
	}
}

func TestInstanceRowChannelDefaultsAndUsesClusterChannel(t *testing.T) {
	if got := (&World{}).instanceRowChannel(); got != 1 {
		t.Fatalf("standalone world row channel = %d, want 1", got)
	}
	cluster := &ChannelCluster{}
	if got := (&World{cluster: cluster}).instanceRowChannel(); got != 1 {
		t.Fatalf("cluster world without channel row = %d, want 1", got)
	}
	if got := (&World{cluster: cluster, channel: 2}).instanceRowChannel(); got != 2 {
		t.Fatalf("cluster channel row = %d, want 2", got)
	}
}

func TestHellGateDionysAliveRequiresOneLiveOwnedNPC(t *testing.T) {
	const runtimeID = "hell-gate-runtime"
	tests := []struct {
		name string
		inst *ItemInstance
		mobs map[uint16]*Mob
		want bool
	}{
		{name: "zero saved health", inst: &ItemInstance{RuntimeID: runtimeID, NPCIDs: map[uint16]struct{}{1: {}}}, mobs: map[uint16]*Mob{1: {ID: 1, HP: 1, InstanceID: runtimeID}}},
		{name: "no protector", inst: &ItemInstance{RuntimeID: runtimeID, HellGateDionysHP: 1}},
		{name: "multiple protectors", inst: &ItemInstance{RuntimeID: runtimeID, HellGateDionysHP: 1,
			NPCIDs: map[uint16]struct{}{1: {}, 2: {}}}},
		{name: "missing mob", inst: &ItemInstance{RuntimeID: runtimeID, HellGateDionysHP: 1,
			NPCIDs: map[uint16]struct{}{1: {}}}},
		{name: "dead mob", inst: &ItemInstance{RuntimeID: runtimeID, HellGateDionysHP: 1,
			NPCIDs: map[uint16]struct{}{1: {}}}, mobs: map[uint16]*Mob{1: {ID: 1, HP: 1, Dead: true, InstanceID: runtimeID}}},
		{name: "zero mob health", inst: &ItemInstance{RuntimeID: runtimeID, HellGateDionysHP: 1,
			NPCIDs: map[uint16]struct{}{1: {}}}, mobs: map[uint16]*Mob{1: {ID: 1, InstanceID: runtimeID}}},
		{name: "different runtime", inst: &ItemInstance{RuntimeID: runtimeID, HellGateDionysHP: 1,
			NPCIDs: map[uint16]struct{}{1: {}}}, mobs: map[uint16]*Mob{1: {ID: 1, HP: 1, InstanceID: "other-runtime"}}},
		{name: "alive", inst: &ItemInstance{RuntimeID: runtimeID, HellGateDionysHP: 1,
			NPCIDs: map[uint16]struct{}{1: {}}}, mobs: map[uint16]*Mob{1: {ID: 1, HP: 1, InstanceID: runtimeID}}, want: true},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w := &World{mobsByID: tt.mobs}
			if got := w.hellGateDionysAlive(tt.inst); got != tt.want {
				t.Fatalf("hellGateDionysAlive = %t, want %t", got, tt.want)
			}
		})
	}
}
