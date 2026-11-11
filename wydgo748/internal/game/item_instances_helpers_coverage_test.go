package game

import (
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestItemInstanceIdentityAndDeadlineHelperEdges(t *testing.T) {
	if itemInstanceGameplaySpace(nil) != "" || instanceRuntimeKey(nil) != "" || expectedChainItem(nil) != 0 {
		t.Fatal("nil instance helpers returned values")
	}
	inst := &ItemInstance{RuntimeID: "  runtime  ", Config: model.VolatileInstance{ID: " config "}}
	if got := itemInstanceGameplaySpace(inst); got != "runtime" {
		t.Fatalf("runtime space=%q", got)
	}
	inst.RuntimeID = "  "
	if got := itemInstanceGameplaySpace(inst); got != "config" {
		t.Fatalf("fallback space=%q", got)
	}
	cfg := model.VolatileInstance{ID: "room", SharedGroup: " group ", RewardItem: 12}
	if got := instanceRuntimeKey(&cfg); got != "shared:group" {
		t.Fatalf("shared runtime key=%q", got)
	}
	cfg.SharedGroup = ""
	if got := instanceRuntimeKey(&cfg); got != "room" {
		t.Fatalf("private runtime key=%q", got)
	}
	if expectedChainItem(&cfg) != 12 {
		t.Fatal("chain item did not fall back to reward item")
	}
	cfg.ChainNextItem = 15
	if expectedChainItem(&cfg) != 15 {
		t.Fatal("configured chain item was ignored")
	}
	if got := (*World)(nil).nextItemInstanceRuntimeID(&cfg); got != "room" {
		t.Fatalf("nil world runtime key=%q", got)
	}
	w := &World{itemInstances: map[string]*ItemInstance{"room": {}, "room:1": {}}}
	if got := w.nextItemInstanceRuntimeID(&cfg); got != "room:2" {
		t.Fatalf("collision-free runtime key=%q", got)
	}

	now := time.Unix(2_000_000_000, 0)
	if remainingInstanceSeconds(time.Time{}, now) != 0 || remainingInstanceSeconds(now, now) != 0 ||
		remainingInstanceSeconds(now.Add(500*time.Millisecond), now) != 1 {
		t.Fatal("remaining seconds did not clamp zero and fractional deadlines")
	}
	if instanceCombatDeadline(nil) != (time.Time{}) {
		t.Fatal("nil combat deadline was not zero")
	}
	deadline, earlier := now.Add(time.Minute), now.Add(30*time.Second)
	inst = &ItemInstance{Deadline: deadline, CombatDeadline: deadline}
	if got := instanceCombatDeadline(inst); !got.Equal(deadline) {
		t.Fatalf("combat deadline=%v", got)
	}
	inst.Deadline = earlier
	if got := instanceCombatDeadline(inst); !got.Equal(earlier) {
		t.Fatalf("earlier administrative deadline=%v", got)
	}
	setInstanceCombatDeadline(nil, now)
	setInstanceCombatDeadline(inst, now)
	if !inst.Deadline.Equal(now) || !inst.CombatDeadline.Equal(now) {
		t.Fatal("combat deadline setter did not update both fields")
	}
}

func TestInstanceParticipantIdentityReservationAndLeaderRepair(t *testing.T) {
	if instanceParticipantKey(nil) != "" || instanceParticipantKey(&Player{ID: 7}) != "" {
		t.Fatal("invalid participant had an identity")
	}
	p := &Player{ID: 7, Char: &model.Char{UID: "  uid-7 "}}
	if got := instanceParticipantKey(p); got != "uid:uid-7" {
		t.Fatalf("character identity=%q", got)
	}
	p.Char.UID = " "
	if got := instanceParticipantKey(p); got != "id:7" {
		t.Fatalf("session identity=%q", got)
	}

	inst := &ItemInstance{RuntimeID: "r", MemberIDs: []uint16{7, 8},
		MemberCharacterUIDs: []string{" uid-7 ", " "}}
	w := &World{playersByID: map[uint16]*Player{7: p}, pendingInstanceMembers: map[string]map[string]struct{}{
		"r": {"pending": {}, " ": {}},
	}}
	if got := w.instanceReservedParticipantKeys(nil); len(got) != 0 {
		t.Fatalf("nil instance reserved keys=%v", got)
	}
	if got := (*World)(nil).instanceReservedParticipantKeys(inst); len(got) != 0 {
		t.Fatalf("nil world reserved keys=%v", got)
	}
	keys := w.instanceReservedParticipantKeys(inst)
	for _, key := range []string{"id:7", "id:8", "uid:uid-7", "uid:pending"} {
		if _, ok := keys[key]; !ok {
			t.Errorf("reserved key %q missing from %v", key, keys)
		}
	}
	if got := w.instanceReservedParticipantCountWith(inst, []*Player{p, nil}); got != len(keys) {
		t.Fatalf("deduplicated reserved count=%d keys=%v", got, keys)
	}
	ensureItemInstanceLeader(nil)
	inst.LeaderID = 9
	ensureItemInstanceLeader(inst)
	if inst.LeaderID != 7 {
		t.Fatalf("leader was not reassigned: %d", inst.LeaderID)
	}
	inst.MemberIDs = nil
	ensureItemInstanceLeader(inst)
	if inst.LeaderID != 0 {
		t.Fatalf("empty instance retained leader %d", inst.LeaderID)
	}
}

func TestInstanceMembershipDetachSnapshotAndRestore(t *testing.T) {
	w, leader, member, _, _ := instanceTestWorld()
	inst := &ItemInstance{RuntimeID: "old", MemberIDs: []uint16{leader.ID, member.ID}, LeaderID: leader.ID,
		MemberCharacterUIDs: []string{leader.Char.UID, member.Char.UID, "offline-uid"},
		LeaderCharacterUID:  leader.Char.UID}
	w.itemInstances[inst.RuntimeID] = inst
	w.indexInstanceMembers(inst)
	w.pendingInstanceMembers = map[string]map[string]struct{}{"old": {leader.Char.UID: {}, "offline-uid": {}}}
	w.pendingInstanceLeaders = map[string]string{"old": leader.Char.UID}
	snapshot := w.snapshotInstanceMembership(inst)
	if got := w.snapshotInstanceMembership(nil); !reflect.DeepEqual(got, instanceMembershipSnapshot{}) {
		t.Fatalf("nil snapshot=%+v", got)
	}
	w.detachInstanceMembersAfterCommit(inst, map[uint16]struct{}{leader.ID: {}})
	if itemInstanceHasMember(inst, leader.ID) || !itemInstanceHasMember(inst, member.ID) ||
		inst.LeaderID != member.ID || inst.LeaderCharacterUID != "" {
		t.Fatalf("detach state=%+v", inst)
	}
	if len(inst.MemberCharacterUIDs) != 2 || inst.MemberCharacterUIDs[0] != member.Char.UID ||
		len(w.pendingInstanceMembers["old"]) != 1 || w.pendingInstanceLeaders["old"] != "" {
		t.Fatalf("detached UID state=%+v pending=%v leaders=%v", inst, w.pendingInstanceMembers, w.pendingInstanceLeaders)
	}
	w.restoreInstanceMembership(inst, snapshot)
	if !reflect.DeepEqual(inst.MemberIDs, snapshot.memberIDs) || inst.LeaderID != leader.ID ||
		!reflect.DeepEqual(w.pendingInstanceMembers["old"], snapshot.pendingMembers) ||
		w.pendingInstanceLeaders["old"] != leader.Char.UID || w.playerRuntimeInstanceID(leader.ID) != "old" {
		t.Fatalf("membership restore failed: inst=%+v pending=%v leaders=%v index=%v",
			inst, w.pendingInstanceMembers, w.pendingInstanceLeaders, w.playerInstance)
	}
	(*World)(nil).restoreInstanceMembership(inst, snapshot)
	w.restoreInstanceMembership(nil, snapshot)
}

func TestInstanceMobAreaAndScheduleInvalidEntries(t *testing.T) {
	if _, _, _, ok := instanceMobArea(nil, nil); ok {
		t.Fatal("nil mob area was accepted")
	}
	mob := &Mob{ID: 77}
	inst := &ItemInstance{Config: model.VolatileInstance{StateMachine: "hell_gate", HellGate: &model.VolatileHellGate{
		Quadrants: []model.VolatileHellGateQuadrant{{ID: 2, X: 200, Y: 300, AreaRadius: 4}},
	}}, MobQuadrants: map[uint16]uint8{77: 2}}
	if x, y, radius, ok := instanceMobArea(inst, mob); !ok || x != 200 || y != 300 || radius != 4 {
		t.Fatalf("Hell Gate quadrant area=%d,%d r=%d ok=%v", x, y, radius, ok)
	}
	inst.MobQuadrants[mob.ID] = 9
	inst.Config.X, inst.Config.Y, inst.Config.AreaRadius = 400, 500, 8
	if x, y, radius, ok := instanceMobArea(inst, mob); !ok || x != 400 || y != 500 || radius != 8 {
		t.Fatalf("fallback stage area=%d,%d r=%d ok=%v", x, y, radius, ok)
	}
	inst.Config.AreaRadius = 0
	if _, _, _, ok := instanceMobArea(inst, mob); ok {
		t.Fatal("zero-radius stage was accepted")
	}

	now := time.Unix(2_000_000_000, 0)
	if _, ok := instanceScheduleEnd(nil, now); !ok {
		t.Fatal("nil schedule should be open")
	}
	cfg := &model.VolatileInstance{Schedule: []model.VolatileInstanceWindow{
		{StartMinute: -1, DurationSeconds: 10}, {StartMinute: 60, DurationSeconds: 10},
		{StartMinute: 1, DurationSeconds: 0},
	}}
	if _, ok := instanceScheduleEnd(cfg, now); ok {
		t.Fatal("all-invalid schedule should have no open window")
	}
}
