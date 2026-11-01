package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestCriticalInstanceMutationHidesProvisionalState(t *testing.T) {
	for _, outcome := range []string{"commit", "rejected", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, leader, member, _, _ := instanceTestWorld()
			leader.X, leader.Y = 2000, 2000
			leader.Char.X, leader.Char.Y = leader.X, leader.Y
			claimCriticalTestPlayers(t, w, leader, member)
			for _, p := range []*Player{leader, member} {
				w.indexPlayerCharacter(p)
				w.updatePlayerSpatial(p)
			}
			observer, _ := networkedTestPlayer(3, "Observer", member.X+1, member.Y)
			w.players[observer.Session], w.playersByID[observer.ID] = observer, observer
			w.indexPlayerCharacter(observer)
			w.updatePlayerSpatial(observer)
			inst := &ItemInstance{Config: *instanceTestRule().Instance, RuntimeID: "water-normal-1",
				LeaderID: member.ID, MemberIDs: []uint16{member.ID}}
			w.itemInstances[inst.RuntimeID] = inst
			before := accountStateSnapshot(member.Account)
			x, y := member.X, member.Y
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
			if outcome == "rejected" {
				base.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				base.failure = errors.New("instance commit acknowledgement lost")
			}
			q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			// A failed answer provisionally leaves the private runtime and grants
			// a changed avatar. An unrelated public observer must not see it.
			inst.MemberIDs = nil
			member.X, member.Y = observer.X, observer.Y
			member.Char.X, member.Char.Y = member.X, member.Y
			member.Char.Gold += 10
			published, rejected := 0, 0
			if !w.commitInstanceMutation("visibility test", []*model.Account{member.Account}, func() {
				inst.MemberIDs = []uint16{member.ID}
				installAccountInPlace(member.Account, before)
				member.X, member.Y = x, y
			}, func() {
				published++
				w.refreshPlayerVisibility(member)
			}, func() { rejected++ }) {
				t.Fatal("instance mutation was not admitted")
			}
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.accounts[0].Chars[0].Gold != before.Chars[0].Gold+10 {
				t.Fatal("detached proposal lost the account change")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, q, 1)
				}
				w.refreshPlayerVisibility(observer)
				if observer.hasVisible(member.ID) || observer.Session.QueuedPacketsForTest() != 0 ||
					!reflect.DeepEqual(accountStateSnapshot(member.Account), before) || member.X != x || member.Y != y ||
					!reflect.DeepEqual(inst.MemberIDs, []uint16{member.ID}) || published != 0 || rejected != 0 {
					t.Fatalf("%s exposed provisional membership, position, account or packets", phase)
				}
			}
			w.processCriticalCompletions(1)
			switch outcome {
			case "commit":
				if published != 1 || rejected != 0 || !observer.hasVisible(member.ID) ||
					member.Char.Gold != before.Chars[0].Gold+10 || len(inst.MemberIDs) != 0 || len(q.pending) != 0 {
					t.Fatal("commit did not install and publish the detached transition")
				}
			case "rejected":
				if published != 0 || rejected != 1 || len(q.pending) != 0 || w.shuttingDown ||
					!reflect.DeepEqual(accountStateSnapshot(member.Account), before) || len(inst.MemberIDs) != 1 {
					t.Fatal("certified rejection did not preserve the original visible state")
				}
			case "unknown":
				if published != 0 || rejected != 0 || !w.shuttingDown || !w.criticalInstanceReserved() ||
					len(q.pending) != 1 || !reflect.DeepEqual(accountStateSnapshot(member.Account), before) || len(inst.MemberIDs) != 1 {
					t.Fatal("unknown outcome installed, published or released provisional state")
				}
			}
		})
	}
}

func TestCriticalInstanceMutationRejectsChangedPreimage(t *testing.T) {
	for _, changed := range []string{"account", "position", "runtime", "runtime-pointer"} {
		for _, outcome := range []string{"commit", "rejected"} {
			t.Run(changed+"/"+outcome, func(t *testing.T) {
				w, _, p, _, _ := instanceTestWorld()
				claimCriticalTestPlayers(t, w, p)
				w.indexPlayerCharacter(p)
				inst := &ItemInstance{Config: *instanceTestRule().Instance, RuntimeID: "water-normal-1",
					LeaderID: p.ID, MemberIDs: []uint16{p.ID}}
				w.itemInstances[inst.RuntimeID] = inst
				before := accountStateSnapshot(p.Account)
				gate, release := criticalGate(t)
				base := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
				if outcome == "rejected" {
					base.failure = criticalCertifiedRejection(t)
				}
				q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(q)
				p.Char.Gold += 10
				// The transition changes this runtime, so the runtime is in the
				// write's scope and later changes to it must be detected (W8).
				inst.CurrentStage++
				published, rejected := 0, 0
				if !w.commitInstanceMutation("changed preimage test", []*model.Account{p.Account},
					func() { installAccountInPlace(p.Account, before); inst.CurrentStage-- },
					func() { published++ }, func() { rejected++ }) {
					t.Fatal("transition was not admitted")
				}
				awaitCriticalWrite(t, base.entered)
				switch changed {
				case "account":
					p.Char.Gold += 7
				case "position":
					p.X++
				case "runtime":
					inst.Remaining++
				case "runtime-pointer":
					copy := *inst
					w.itemInstances[inst.RuntimeID] = &copy
				}
				changedAccount := accountStateSnapshot(p.Account)
				changedX, changedY := p.X, p.Y
				changedInstance := cloneCriticalInstanceMembership(w).itemInstances[inst.RuntimeID]
				release()
				awaitReadyCriticalResults(t, q, 1)
				w.processCriticalCompletions(1)
				if !w.shuttingDown || !w.criticalInstanceReserved() || len(q.pending) != 1 || published != 0 || rejected != 0 ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), changedAccount) || p.X != changedX || p.Y != changedY ||
					!reflect.DeepEqual(w.itemInstances[inst.RuntimeID], changedInstance) {
					t.Fatal("changed retained state was overwritten, published or unfenced")
				}
			})
		}
	}
}

// W8: a transition of one runtime does not reserve or check another runtime.
// The unrelated runtime may change while the write is pending.
func TestCriticalInstanceMutationIgnoresUnrelatedRuntime(t *testing.T) {
	w, _, p, _, _ := instanceTestWorld()
	claimCriticalTestPlayers(t, w, p)
	w.indexPlayerCharacter(p)
	inst := &ItemInstance{Config: *instanceTestRule().Instance, RuntimeID: "water-normal-1",
		LeaderID: p.ID, MemberIDs: []uint16{p.ID}}
	other := &ItemInstance{Config: *instanceTestRule().Instance, RuntimeID: "water-normal-2"}
	w.itemInstances[inst.RuntimeID], w.itemInstances[other.RuntimeID] = inst, other
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	q := newCriticalTestQueue(t, &criticalInstanceRowsTestStore{&criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}}, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	inst.CurrentStage++
	published := 0
	if !w.commitInstanceMutation("unrelated runtime test", nil, func() { inst.CurrentStage-- },
		func() { published++ }, nil) {
		t.Fatal("transition was not admitted")
	}
	write := awaitCriticalWrite(t, base.entered)
	if len(write.instanceKeys) != 1 || write.instanceKeys[0].RuntimeID != inst.RuntimeID {
		t.Fatalf("write keys %+v, want only %s", write.instanceKeys, inst.RuntimeID)
	}
	if !w.criticalInstanceRuntimeReserved(inst.RuntimeID) || w.criticalInstanceRuntimeReserved(other.RuntimeID) {
		t.Fatal("reservation is not scoped to the changed runtime")
	}
	other.Remaining++ // unrelated runtime progresses during the write
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if w.shuttingDown || published != 1 || inst.CurrentStage != 1 || other.Remaining != 1 || len(q.pending) != 0 {
		t.Fatalf("scoped transition failed: maintenance=%t published=%d stage=%d", w.shuttingDown, published, inst.CurrentStage)
	}
}

// criticalInstanceRowsTestStore adds the keyed instance adapter (W8).
type criticalInstanceRowsTestStore struct {
	*criticalInstanceAtomicTestStore
}

func (s *criticalInstanceRowsTestStore) SaveCriticalInstanceRows(state *model.InstanceStateSnapshot,
	keys []store.InstanceParticipant, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, instances: state, instanceKeys: keys})
}
