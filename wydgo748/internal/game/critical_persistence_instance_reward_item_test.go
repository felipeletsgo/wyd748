package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func criticalWaterItemRewardFixture(t *testing.T) (*World, *Player, *Player, *ItemInstance, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, leader, member, _, clock := instanceTestWorld()
	claimCriticalTestPlayers(t, w, leader, member)
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil {
		t.Fatal("Water fixture did not create its instance")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	store := &criticalInstanceRowsTestStore{&criticalInstanceAtomicTestStore{
		&criticalInstanceTestStore{backend},
	}}
	queue := newCriticalTestQueue(t, store, 1, 3)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	w.clock = clock
	return w, leader, member, inst, backend, queue, release
}

func TestCriticalWaterRewardCommitsItemAndExitGraceTogether(t *testing.T) {
	w, leader, member, inst, backend, queue, release := criticalWaterItemRewardFixture(t)
	now := w.now()
	beforeLeader, beforeMember := accountStateSnapshot(leader.Account), accountStateSnapshot(member.Account)
	slot := firstFreeVisibleInventorySlot(leader.Char)
	if slot < 0 {
		t.Fatal("Water fixture inventory unexpectedly full")
	}

	w.completeItemInstance(inst, now)
	proposal := awaitCriticalWrite(t, backend.entered)
	if len(proposal.accounts) != 2 || len(proposal.instanceKeys) != 1 ||
		proposal.instanceKeys[0].RuntimeID != inst.RuntimeID || !proposal.instances.Instances[0].RewardGranted ||
		!proposal.instances.Instances[0].ExitAt.Equal(now.Add(10*time.Second)) ||
		proposal.accounts[0].Chars[leader.CharSlot].Inv[slot].Index != inst.Config.RewardItem ||
		!reflect.DeepEqual(accountStateSnapshot(leader.Account), beforeLeader) ||
		!reflect.DeepEqual(accountStateSnapshot(member.Account), beforeMember) || inst.RewardGranted || !inst.ExitAt.IsZero() {
		t.Fatal("pending Water reward did not isolate its detached item and instance changes")
	}
	uid := proposal.accounts[0].Chars[leader.CharSlot].Inv[slot].UID
	if uid == "" {
		t.Fatal("detached Water reward has no materialized item UID")
	}

	settleCriticalCelestialWrite(t, w, queue, release)
	if leader.Char.Inv[slot].UID != uid || leader.Char.Inv[slot].Index != inst.Config.RewardItem ||
		!inst.RewardGranted || !inst.ExitAt.Equal(now.Add(10*time.Second)) ||
		w.criticalInstanceRuntimeReserved(inst.RuntimeID) {
		t.Fatalf("acknowledged Water reward failed to settle: item=%+v instance=%+v", leader.Char.Inv[slot], inst)
	}
	if err := w.queueCriticalInstanceReward(inst, now); !errors.Is(err, errCriticalPersistenceIdentity) {
		t.Fatalf("duplicate Water reward admission = %v, want identity rejection", err)
	}
}

func TestCriticalWaterRewardCertifiedRejectionPreservesItemAndInstance(t *testing.T) {
	w, leader, member, inst, backend, queue, release := criticalWaterItemRewardFixture(t)
	backend.failure = criticalCertifiedRejection(t)
	beforeLeader, beforeMember := accountStateSnapshot(leader.Account), accountStateSnapshot(member.Account)
	now := w.now()

	if err := w.queueCriticalInstanceReward(inst, now); err != nil {
		t.Fatalf("queue Water reward: %v", err)
	}
	awaitCriticalWrite(t, backend.entered)
	settleCriticalCelestialWrite(t, w, queue, release)
	if !reflect.DeepEqual(accountStateSnapshot(leader.Account), beforeLeader) ||
		!reflect.DeepEqual(accountStateSnapshot(member.Account), beforeMember) || inst.RewardGranted ||
		!inst.ExitAt.IsZero() || !inst.ExitDeadline.IsZero() || w.criticalInstanceRuntimeReserved(inst.RuntimeID) {
		t.Fatal("certified rejection consumed the Water reward or changed exit grace")
	}
}

func TestCriticalWaterRewardUsesInvisibleGroundReservationWhenInventoryIsFull(t *testing.T) {
	w, leader, _, inst, backend, queue, release := criticalWaterItemRewardFixture(t)
	for slot := range leader.Char.Inv {
		leader.Char.Inv[slot] = model.Item{Index: uint16(500 + slot), UID: "occupied"}
	}
	before := accountStateSnapshot(leader.Account)
	now := w.now()

	if err := w.queueCriticalInstanceReward(inst, now); err != nil {
		t.Fatalf("queue ground Water reward: %v", err)
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	var reserved *GroundItem
	for _, item := range w.groundItems {
		if item.criticalReserved() {
			reserved = item
		}
	}
	if reserved == nil || proposal.accounts[0].Chars[leader.CharSlot].Inv != before.Chars[leader.CharSlot].Inv ||
		!reflect.DeepEqual(accountStateSnapshot(leader.Account), before) || inst.RewardGranted {
		t.Fatal("full inventory published a ground reward or mutated live state before acknowledgement")
	}

	settleCriticalCelestialWrite(t, w, queue, release)
	if w.groundItems[reserved.ID] != reserved || reserved.criticalReserved() || !inst.RewardGranted ||
		!reflect.DeepEqual(accountStateSnapshot(leader.Account), before) {
		t.Fatal("committed ground fallback did not preserve the account and release its invisible reservation")
	}
}

func TestCriticalWaterGroundRewardRejectionReleasesAllocation(t *testing.T) {
	w, leader, _, inst, backend, queue, release := criticalWaterItemRewardFixture(t)
	backend.failure = criticalCertifiedRejection(t)
	for slot := range leader.Char.Inv {
		leader.Char.Inv[slot] = model.Item{Index: uint16(500 + slot), UID: "occupied"}
	}
	before := accountStateSnapshot(leader.Account)
	now := w.now()

	if err := w.queueCriticalInstanceReward(inst, now); err != nil {
		t.Fatalf("queue ground Water reward: %v", err)
	}
	awaitCriticalWrite(t, backend.entered)
	var reserved *GroundItem
	for _, item := range w.groundItems {
		if item.criticalReserved() {
			reserved = item
		}
	}
	if reserved == nil {
		t.Fatal("pending ground fallback did not reserve its ID")
	}
	settleCriticalCelestialWrite(t, w, queue, release)
	if w.groundItems[reserved.ID] != nil || !reflect.DeepEqual(accountStateSnapshot(leader.Account), before) ||
		inst.RewardGranted || !inst.ExitAt.IsZero() || w.criticalInstanceRuntimeReserved(inst.RuntimeID) {
		t.Fatal("rejected ground fallback retained its allocation or reward marker")
	}
}

func TestCriticalWaterRewardRejectsUnknownItemBeforePersistence(t *testing.T) {
	w, leader, _, inst, backend, queue, _ := criticalWaterItemRewardFixture(t)
	delete(w.items, inst.Config.RewardItem)
	before := accountStateSnapshot(leader.Account)
	err := w.queueCriticalInstanceReward(inst, w.now())
	if err == nil || len(queue.pending) != 0 || len(backend.entered) != 0 ||
		!reflect.DeepEqual(accountStateSnapshot(leader.Account), before) || inst.RewardGranted {
		t.Fatalf("unknown reward item admission error=%v pending=%d", err, len(queue.pending))
	}
}
