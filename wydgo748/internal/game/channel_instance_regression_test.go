package game

import (
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

type channelSnapshotStore struct {
	craftStore
	state *model.InstanceStateSnapshot
	err   error
}

func (s *channelSnapshotStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return s.state, s.err
}

func channelInstanceSnapshot(channel byte) *model.InstanceStateSnapshot {
	return &model.InstanceStateSnapshot{
		Version: model.InstanceStateVersion,
		Instances: []model.InstanceRuntimeState{{
			// Runtime IDs can collide between channels; membership must not.
			RuntimeID: "same-runtime", ConfigID: "nightmare",
			MemberCharacterUIDs: []string{fmt.Sprintf("member-%d", channel)},
			LeaderCharacterUID:  fmt.Sprintf("member-%d", channel),
			CurrentStage:        int(channel), NightmareBossKilled: channel == 2,
		}},
		NightmarePartyRuns: map[string]int{"same-window": int(channel)},
		GamblePool:         uint64(channel) * 100, GambleJackpot: uint32(channel) * 10,
	}
}

func TestChannelInstancesRestartRestoresEachWorld(t *testing.T) {
	for _, legacy := range []bool{false, true} {
		for _, order := range [][2]byte{{1, 2}, {2, 1}} {
			t.Run(fmt.Sprintf("legacy-%t/order-%d-%d", legacy, order[0], order[1]), func(t *testing.T) {
				one, two := channelInstanceSnapshot(1), channelInstanceSnapshot(2)
				root := one.Clone()
				if !legacy {
					root.Channels = map[byte]*model.InstanceStateSnapshot{2: two.Clone()}
				}
				encoded, err := json.Marshal(root)
				if err != nil {
					t.Fatal(err)
				}
				var disk model.InstanceStateSnapshot
				if err := json.Unmarshal(encoded, &disk); err != nil {
					t.Fatal(err)
				}
				backend := &channelSnapshotStore{state: &disk}
				c, err := NewChannelCluster(backend)
				if err != nil {
					t.Fatal(err)
				}
				// Changing a store-owned buffer after loading cannot alter the actor.
				disk.Instances[0].MemberCharacterUIDs[0] = "store-reused-buffer"
				for _, channel := range order {
					w := newZoneTestWorld()
					w.store, w.channel = backend, channel
					if err := c.attach(w); err != nil {
						t.Fatal(err)
					}
					got := w.restoredChannelInstances(c.instances)
					want := one
					if channel == 2 {
						want = two
						if legacy {
							want = nil
						}
					}
					if !reflect.DeepEqual(got, want) {
						t.Fatalf("channel %d restored another world's state: got=%+v want=%+v", channel, got, want)
					}
					if got != nil {
						got.Instances[0].MemberCharacterUIDs[0] = "world-local-change"
						got.NightmarePartyRuns["same-window"] = 99
						if !reflect.DeepEqual(w.restoredChannelInstances(c.instances), want) {
							t.Fatal("world mutation changed the restart snapshot")
						}
					}
				}
			})
		}
	}
}

func TestChannelInstancesLoadFailureDoesNotBootEmpty(t *testing.T) {
	injected := errors.New("injected instance load failure")
	if c, err := NewChannelCluster(&channelSnapshotStore{err: injected}); c != nil || !errors.Is(err, injected) {
		t.Fatalf("failed load silently discarded restart state: cluster=%v error=%v", c, err)
	}
	if c, err := NewChannelCluster(&channelSnapshotStore{}); err != nil || c.instances != nil {
		t.Fatalf("first boot rejected a missing snapshot: cluster=%v error=%v", c, err)
	}
}

func TestChannelInstancesQueuedWritesPreserveBothWorlds(t *testing.T) {
	for _, first := range []byte{1, 2} {
		t.Run(fmt.Sprintf("first-channel-%d", first), func(t *testing.T) {
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			backend := &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}
			q := newCriticalTestQueue(t, backend, 2, 4)
			t.Cleanup(release)
			c, err := NewChannelCluster(backend)
			if err != nil {
				t.Fatal(err)
			}
			c.owner = newCriticalPersistenceOwner(q)
			for _, channel := range []byte{1, 2} {
				w := newZoneTestWorld()
				w.store, w.channel, w.criticalWrites = backend, channel, c.owner
				if err := c.attach(w); err != nil {
					t.Fatal(err)
				}
			}
			a, b := c.world(first), c.world(3-first)
			one, two := channelInstanceSnapshot(first), channelInstanceSnapshot(3-first)
			c.enter(a)
			if _, err := c.owner.submit(criticalWrite{instances: one}, func(criticalPersistenceResult) error { return nil }); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			// Hold the aggregate through worker completion and actor acknowledgement.
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, q, 1)
				}
				c.enter(b)
				if _, err := c.owner.submit(criticalWrite{instances: two}, func(criticalPersistenceResult) error { return nil }); !errors.Is(err, errCriticalPersistenceBusy) {
					t.Fatalf("%s competing channel write was admitted: %v", phase, err)
				}
				if c.instances != nil {
					t.Fatalf("%s proposal replaced committed state early", phase)
				}
			}
			b.processCriticalCompletions(1)
			if c.active != b || !reflect.DeepEqual(a.restoredChannelInstances(c.instances), one) {
				t.Fatal("completion lost its source channel or changed the active actor")
			}
			proposal.instances.Instances = nil
			if !reflect.DeepEqual(a.restoredChannelInstances(c.instances), one) {
				t.Fatal("worker payload aliases the acknowledged snapshot")
			}
			if _, err := c.owner.submit(criticalWrite{instances: two}, func(criticalPersistenceResult) error { return nil }); err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, base.entered)
			awaitReadyCriticalResults(t, q, 1)
			c.enter(a)
			a.processCriticalCompletions(1)
			if !reflect.DeepEqual(a.restoredChannelInstances(c.instances), one) || !reflect.DeepEqual(b.restoredChannelInstances(c.instances), two) || len(q.pending) != 0 {
				t.Fatal("alternating writes overwrote another channel or leaked a reservation")
			}
			// Clearing one channel's rooms must preserve the other channel's rooms.
			empty := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, NightmarePartyRuns: map[string]int{}}
			if _, err := c.owner.submit(criticalWrite{instances: empty}, func(criticalPersistenceResult) error { return nil }); err != nil {
				t.Fatal(err)
			}
			awaitCriticalWrite(t, base.entered)
			awaitReadyCriticalResults(t, q, 1)
			c.enter(b)
			b.processCriticalCompletions(1)
			if !reflect.DeepEqual(a.restoredChannelInstances(c.instances), empty) || !reflect.DeepEqual(b.restoredChannelInstances(c.instances), two) {
				t.Fatal("instance cleanup cleared another channel's restart state")
			}
		})
	}
}
