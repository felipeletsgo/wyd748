package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestIndexInstanceMembersAndLookupRepairEdges(t *testing.T) {
	var nilWorld *World
	inst := &ItemInstance{RuntimeID: "room", MemberIDs: []uint16{0, 7}}
	nilWorld.indexInstanceMembers(inst)

	w := &World{
		clock:          newFakeClock(time.Unix(2_000_000_000, 0)),
		itemInstances:  map[string]*ItemInstance{"room": inst},
		playerInstance: map[uint16]string{7: "stale-room"},
	}
	w.indexInstanceMembers(nil)
	if got := w.itemInstanceForPlayer(0); got != nil {
		t.Fatalf("zero player ID resolved to instance %+v", got)
	}
	if got := nilWorld.itemInstanceForPlayer(7); got != nil {
		t.Fatalf("nil world resolved to instance %+v", got)
	}
	if got := w.itemInstanceForPlayer(7); got != inst || w.playerInstance[7] != "room" {
		t.Fatalf("stale membership index was not repaired: got=%p index=%q", got, w.playerInstance[7])
	}
	if got := w.itemInstanceForPlayer(8); got != nil {
		t.Fatalf("non-member resolved to instance %+v", got)
	}

	grace := &ItemInstance{
		RuntimeID: "grace", MemberIDs: []uint16{9},
		Config:        model.VolatileInstance{ChainNextItem: 100, AllowChainDuringExitGrace: true},
		RewardGranted: true, ExitAt: w.now().Add(time.Minute),
	}
	w.itemInstances[grace.RuntimeID] = grace
	w.setPlayerInstanceIndex(9, grace.RuntimeID)
	if got := w.itemInstanceForPlayer(9); got != nil {
		t.Fatalf("exit-grace member resolved as active: %+v", got)
	}
}

func TestRemoveUnpublishedItemInstanceMobsAndRollback(t *testing.T) {
	w, leader, _, _, _ := instanceTestWorld()
	first := &Mob{ID: 40, X: leader.X, Y: leader.Y}
	second := &Mob{ID: 41, X: leader.X, Y: leader.Y}
	w.mobs = []*Mob{first, second}
	w.mobListIndex = map[uint16]int{first.ID: 0, second.ID: 1}
	w.mobsByID = map[uint16]*Mob{first.ID: first, second.ID: second}
	inst := &ItemInstance{
		MobIDs:    map[uint16]struct{}{first.ID: {}, 99: {}},
		NPCIDs:    map[uint16]struct{}{second.ID: {}, 98: {}},
		Remaining: 2,
	}
	w.removeUnpublishedItemInstanceMobs(nil)
	w.removeUnpublishedItemInstanceMobs(inst)
	if len(w.mobs) != 0 || len(inst.MobIDs) != 0 || len(inst.NPCIDs) != 0 || inst.Remaining != 0 {
		t.Fatalf("unpublished entities remained: mobs=%v mobIDs=%v npcIDs=%v remaining=%d",
			w.mobs, inst.MobIDs, inst.NPCIDs, inst.Remaining)
	}

	third := &Mob{ID: 42, X: leader.X, Y: leader.Y}
	w.mobs = []*Mob{third}
	w.mobListIndex = map[uint16]int{third.ID: 0}
	w.rollbackItemInstanceMobs([]*Mob{nil, third}, false)
	if len(w.mobs) != 0 {
		t.Fatalf("unpublished rollback retained mob: %v", w.mobs)
	}

	fourth := &Mob{ID: 43, X: leader.X, Y: leader.Y}
	w.mobs = []*Mob{fourth}
	w.mobListIndex = map[uint16]int{fourth.ID: 0}
	w.rollbackItemInstanceMobs([]*Mob{fourth}, true)
	if len(w.mobs) != 0 {
		t.Fatalf("published rollback retained mob: %v", w.mobs)
	}
}

func TestCloseItemInstanceClearsMembershipAndReleasesGroundItems(t *testing.T) {
	w, leader, member, _, _ := instanceTestWorld()
	inst := &ItemInstance{RuntimeID: "closing-room", MemberIDs: []uint16{leader.ID, member.ID}}
	w.itemInstances[inst.RuntimeID] = inst
	w.indexInstanceMembers(inst)
	w.pendingInstanceMembers = map[string]map[string]struct{}{"closing-room": {leader.Char.UID: {}}}
	w.pendingInstanceLeaders = map[string]string{"closing-room": leader.Char.UID}
	w.magicChamberRunes = map[string]magicChamberRun{"closing-room": {BonusSpawned: true}}
	ground := &GroundItem{ID: 77, Item: model.Item{Index: 100}, X: leader.X, Y: leader.Y, InstanceID: inst.RuntimeID}
	w.groundItems = map[uint16]*GroundItem{77: ground, 78: {ID: 78, InstanceID: "other-room"}, 79: nil}
	before := leader.Session.QueuedPacketsForTest()

	w.closeItemInstance(inst.RuntimeID)

	if w.itemInstances[inst.RuntimeID] != nil || w.playerRuntimeInstanceID(leader.ID) != "" ||
		w.playerRuntimeInstanceID(member.ID) != "" || w.pendingInstanceMembers[inst.RuntimeID] != nil ||
		w.pendingInstanceLeaders[inst.RuntimeID] != "" || len(w.magicChamberRunes) != 0 ||
		ground.InstanceID != "" || !leader.hasVisible(ground.ID) ||
		leader.Session.QueuedPacketsForTest() != before+1 {
		t.Fatalf("instance close retained derived state: room=%v indexes=%v ground=%q packets=%d/%d",
			w.itemInstances[inst.RuntimeID], w.playerInstance, ground.InstanceID,
			leader.Session.QueuedPacketsForTest(), before)
	}
	if w.instanceDirtyRuntimes[inst.RuntimeID] == 0 {
		t.Fatal("closing a runtime did not record its durable deletion")
	}
	w.closeItemInstance("missing-room")
}
