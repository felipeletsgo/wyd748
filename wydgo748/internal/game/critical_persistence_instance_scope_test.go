package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestInstallScopedInitializesMapsAndInstallsNewParticipants(t *testing.T) {
	world := &World{nightmarePartyRuns: map[string]int{"old": 1}, gambleJackpot: 10, gamblePool: 20,
		criticalInstanceGeneration: &criticalInstanceGeneration{}}
	previousGeneration := world.criticalInstanceGeneration
	plan := &criticalInstanceDeparture{
		before: &World{},
		proposed: &World{
			itemInstances:          map[string]*ItemInstance{"new": {RuntimeID: "new", State: "active"}},
			pendingInstanceMembers: map[string]map[string]struct{}{"new": {"member": {}}},
			pendingInstanceLeaders: map[string]string{"new": "leader"},
			playerInstance:         map[uint16]string{1: "new"},
			nightmarePartyRuns:     map[string]int{"new": 2},
			gambleJackpot:          30,
			gamblePool:             40,
		},
		scope: &criticalInstanceScope{runtimes: map[string]bool{"new": true}, globals: true},
	}

	plan.installScoped(world)

	if world.itemInstances["new"] == nil || world.itemInstances["new"].State != "active" ||
		len(world.pendingInstanceMembers["new"]) != 1 || world.pendingInstanceLeaders["new"] != "leader" ||
		world.playerInstance[1] != "new" {
		t.Fatal("scoped install did not initialize instance membership maps")
	}
	if world.gambleJackpot != 30 || world.gamblePool != 40 || world.nightmarePartyRuns["new"] != 2 {
		t.Fatal("scoped global participants were not installed")
	}
	if world.criticalInstanceGeneration == previousGeneration {
		t.Fatal("scoped install did not advance the instance generation")
	}
}

func TestInstallScopedUpdatesAndRemovesOnlyItsParticipants(t *testing.T) {
	updated := &ItemInstance{RuntimeID: "updated", State: "old"}
	removed := &ItemInstance{RuntimeID: "removed", State: "active"}
	untouched := &ItemInstance{RuntimeID: "untouched", State: "active"}
	world := &World{
		itemInstances: map[string]*ItemInstance{"updated": updated, "removed": removed, "untouched": untouched},
		pendingInstanceMembers: map[string]map[string]struct{}{
			"updated": {"old": {}}, "removed": {"old": {}}, "delete": {"old": {}},
		},
		pendingInstanceLeaders: map[string]string{"updated": "old", "removed": "old", "delete": "old"},
		playerInstance:         map[uint16]string{1: "updated", 2: "removed", 3: "untouched"},
		gambleJackpot:          10,
		gamblePool:             20,
	}
	plan := &criticalInstanceDeparture{
		before: &World{
			itemInstances:          map[string]*ItemInstance{"updated": updated, "removed": removed},
			pendingInstanceMembers: map[string]map[string]struct{}{"updated": {"old": {}}, "removed": {"old": {}}, "delete": {"old": {}}},
			pendingInstanceLeaders: map[string]string{"updated": "old", "removed": "old", "delete": "old"},
			playerInstance:         map[uint16]string{1: "updated", 2: "removed", 3: "untouched"},
		},
		proposed: &World{
			itemInstances:          map[string]*ItemInstance{"updated": {RuntimeID: "updated", State: "new"}, "added": {RuntimeID: "added", State: "active"}},
			pendingInstanceMembers: map[string]map[string]struct{}{"updated": {"new": {}}, "added": {"member": {}}},
			pendingInstanceLeaders: map[string]string{"updated": "new", "added": "leader"},
			playerInstance:         map[uint16]string{1: "added", 3: "untouched"},
		},
		scope: &criticalInstanceScope{runtimes: map[string]bool{
			"updated": true, "removed": true, "added": true, "delete": true,
		}},
	}

	plan.installScoped(world)

	if world.itemInstances["updated"] != updated || updated.State != "new" || world.itemInstances["removed"] != nil ||
		world.itemInstances["added"] == nil || world.itemInstances["untouched"] != untouched || untouched.State != "active" {
		t.Fatal("scoped install replaced a held pointer or changed an unrelated runtime")
	}
	_, updatedPending := world.pendingInstanceMembers["updated"]["new"]
	_, addedPending := world.pendingInstanceMembers["added"]["member"]
	if len(world.pendingInstanceMembers["updated"]) != 1 || !updatedPending ||
		world.pendingInstanceMembers["removed"] != nil || world.pendingInstanceMembers["delete"] != nil || !addedPending {
		t.Fatal("scoped install did not replace and remove pending member rows")
	}
	if world.pendingInstanceLeaders["updated"] != "new" || world.pendingInstanceLeaders["removed"] != "" ||
		world.pendingInstanceLeaders["delete"] != "" || world.pendingInstanceLeaders["added"] != "leader" {
		t.Fatal("scoped install did not replace and remove pending leaders")
	}
	if world.playerInstance[1] != "added" || world.playerInstance[2] != "" || world.playerInstance[3] != "untouched" {
		t.Fatalf("scoped player indexes = %v", world.playerInstance)
	}
	if world.gambleJackpot != 10 || world.gamblePool != 20 {
		t.Fatal("non-global scoped install changed unrelated global values")
	}
}

func TestMergeCommittedChannelInstancesAppliesOnlyCommittedKeys(t *testing.T) {
	world := &World{}
	world.mergeCommittedChannelInstances(&model.InstanceStateSnapshot{}, nil)
	world.cluster = &ChannelCluster{}
	world.mergeCommittedChannelInstances(nil, nil)
	if world.cluster.instances != nil {
		t.Fatal("nil saved state changed the cluster snapshot")
	}

	// The first keyed write initializes an empty root and a non-primary channel.
	seed := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
		Channels: map[byte]*model.InstanceStateSnapshot{2: {Version: model.InstanceStateVersion,
			Instances: []model.InstanceRuntimeState{{RuntimeID: "seed", State: "committed"}}}},
	}
	world.mergeCommittedChannelInstances(seed, []store.InstanceParticipant{{Channel: 2, RuntimeID: "seed"}})
	root := world.cluster.instances
	if root == nil || root.Channels[2] == nil || len(root.Channels[2].Instances) != 1 ||
		root.Channels[2].Instances[0].State != "committed" {
		t.Fatal("first keyed commit did not initialize the shared root and channel")
	}

	root.Instances = []model.InstanceRuntimeState{
		{RuntimeID: "replace", State: "old"},
		{RuntimeID: "remove", State: "stale"},
		{RuntimeID: "keep", State: "current"},
	}
	root.NightmarePartyRuns = map[string]int{"old": 1}
	root.GambleJackpot, root.GamblePool = 5, 6
	root.Channels[3] = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
		Instances:          []model.InstanceRuntimeState{{RuntimeID: "gone", State: "stale"}},
		NightmarePartyRuns: map[string]int{"stale": 2}, GambleJackpot: 7, GamblePool: 8,
	}
	saved := &model.InstanceStateSnapshot{
		Version:            model.InstanceStateVersion,
		Instances:          []model.InstanceRuntimeState{{RuntimeID: "replace", State: "new"}, {RuntimeID: "add", State: "added"}, {RuntimeID: "keep", State: "saved"}},
		NightmarePartyRuns: map[string]int{"today": 3},
		GambleJackpot:      9,
		GamblePool:         10,
		Channels: map[byte]*model.InstanceStateSnapshot{2: {
			Version:   model.InstanceStateVersion,
			Instances: []model.InstanceRuntimeState{{RuntimeID: "seed", State: "updated"}},
		}},
	}
	world.mergeCommittedChannelInstances(saved, []store.InstanceParticipant{
		{Channel: 1, RuntimeID: "replace"},
		{Channel: 1, RuntimeID: "remove"},
		{Channel: 1, RuntimeID: "add"},
		{Channel: 1},
		{Channel: 2, RuntimeID: "seed"},
		{Channel: 3, RuntimeID: "gone"},
		{Channel: 3},
	})

	if got := root.Instances; len(got) != 3 || got[0].RuntimeID != "add" || got[0].State != "added" ||
		got[1].RuntimeID != "keep" || got[1].State != "current" ||
		got[2].RuntimeID != "replace" || got[2].State != "new" {
		t.Fatalf("primary channel merge = %+v", got)
	}
	if root.NightmarePartyRuns["today"] != 3 || root.GambleJackpot != 9 || root.GamblePool != 10 {
		t.Fatal("primary channel globals were not replaced from the committed snapshot")
	}
	if got := root.Channels[2].Instances; len(got) != 1 || got[0].RuntimeID != "seed" || got[0].State != "updated" {
		t.Fatalf("secondary channel merge = %+v", got)
	}
	if len(root.Channels[3].Instances) != 0 || root.Channels[3].NightmarePartyRuns != nil ||
		root.Channels[3].GambleJackpot != 0 || root.Channels[3].GamblePool != 0 {
		t.Fatal("missing committed rows did not remove only the selected channel participants")
	}
}
