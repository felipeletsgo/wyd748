package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestUxmalTicketRoomAndSlotBoundaries(t *testing.T) {
	item := model.Item{Index: 5134}
	if uxmalTicketRoom(item) != 0 {
		t.Fatal("unrefined ticket did not select room zero")
	}
	if !setItemSanc(&item, 4) || uxmalTicketRoom(item) != 4 {
		t.Fatal("refined ticket did not select room four")
	}
	if !setItemSanc(&item, 15) || uxmalTicketRoom(item) != 6 {
		t.Fatal("ticket refinement above the room range was not capped")
	}
	w, leader, _, _, _ := uxmalTestWorld()
	if w.uxmalTicketSlot(nil, 0) != -1 {
		t.Fatal("ticket lookup accepted a missing character")
	}
	if w.uxmalTicketSlot(leader.Char, 2) != -1 {
		t.Fatal("ticket lookup accepted a different room")
	}
	setItemSanc(&leader.Char.Inv[0], 2)
	if got := w.uxmalTicketSlot(leader.Char, 2); got != 0 {
		t.Fatalf("ticket slot = %d, want 0", got)
	}
	if (&World{}).uxmalTicketSlot(leader.Char, 2) != -1 {
		t.Fatal("ticket lookup without Uxmal configuration succeeded")
	}
}

func TestUxmalMemberSelectionFiltersInvalidPartyMembers(t *testing.T) {
	w, leader, member, _, _ := uxmalTestWorld()
	if members, ok := w.uxmalMembers(nil); ok || len(members) != 0 {
		t.Fatal("nil leader was accepted")
	}
	if members, ok := w.uxmalMembers(leader); !ok || len(members) != 2 || members[0] != leader || members[1] != member {
		t.Fatalf("valid party membership = %v, %v", members, ok)
	}
	if members, ok := w.uxmalMembers(member); ok || len(members) != 0 {
		t.Fatal("non-leader party member was accepted")
	}

	member.InWorld = false
	if members, ok := w.uxmalMembers(leader); !ok || len(members) != 1 || members[0] != leader {
		t.Fatalf("offline party member was not filtered: %v, %v", members, ok)
	}
	member.InWorld = true
	member.X = 2100
	if members, ok := w.uxmalMembers(leader); !ok || len(members) != 1 {
		t.Fatalf("out-of-entry party member was not filtered: %v, %v", members, ok)
	}

	leader.Party = nil
	if members, ok := w.uxmalMembers(leader); !ok || len(members) != 1 || members[0] != leader {
		t.Fatalf("valid solo entry = %v, %v", members, ok)
	}
	leader.X = 2100
	if members, ok := w.uxmalMembers(leader); ok || len(members) != 0 {
		t.Fatal("leader outside entry area was accepted")
	}
}

func TestUxmalFreeSlotsAndStageLookupValidateBounds(t *testing.T) {
	w, _, _, _, _ := uxmalTestWorld()
	for _, room := range []int{-1, 7} {
		if w.uxmalFreeSlot(room) != -1 {
			t.Fatalf("invalid room %d returned a slot", room)
		}
	}
	if (&World{}).uxmalFreeSlot(0) != -1 {
		t.Fatal("free-slot lookup without Uxmal configuration succeeded")
	}
	if got := w.uxmalFreeSlot(0); got != 0 {
		t.Fatalf("initial free slot = %d, want zero", got)
	}
	w.itemInstances["uxmal:0:0"] = &ItemInstance{}
	if got := w.uxmalFreeSlot(0); got != 1 {
		t.Fatalf("next free slot = %d, want one", got)
	}
	w.itemInstances["uxmal:0:1"] = &ItemInstance{}
	if got := w.uxmalFreeSlot(0); got != -1 {
		t.Fatalf("full room free slot = %d, want -1", got)
	}
	for _, roomSlot := range [][2]int{{-1, 0}, {7, 0}, {0, -1}, {0, 3}} {
		if _, ok := w.uxmalStage(roomSlot[0], roomSlot[1]); ok {
			t.Errorf("invalid stage lookup room=%d slot=%d succeeded", roomSlot[0], roomSlot[1])
		}
	}
	if _, ok := (&World{}).uxmalStage(0, 0); ok {
		t.Fatal("stage lookup without configuration succeeded")
	}
	stage, ok := w.uxmalStage(2, 1)
	want := w.uxmal.Uxmal.RoomPositions[2][1]
	if !ok || stage.X != want.X || stage.Y != want.Y || stage.SpawnX != want.X || stage.SpawnY != want.Y {
		t.Fatalf("room stage position = %+v, ok=%v", stage, ok)
	}
}
