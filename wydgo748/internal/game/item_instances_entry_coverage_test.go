package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func sharedTimedEntryCoverageFixture(t *testing.T) (*World, *Player, *Player, *guildFlowStore, *fakeClock, model.VolatileRule, *ItemInstance) {
	t.Helper()
	w, leader, member, st, clock := instanceTestWorld()
	rule := nightmareTestRule("normal")
	rule.PartyMode = "solo"
	rule.Instance.ID = "nightmare-entry-coverage"
	rule.Instance.Mode = "shared_timed_zone"
	rule.Instance.SharedGroup = "nightmare-entry-coverage"
	rule.Instance.FinishPolicy = "respawn_until_timeout"
	rule.Instance.MaxPlayers = 13
	rule.Instance.RewardItem = 0
	rule.Instance.EntryAreas = []model.VolatileInstanceEntryArea{{MinX: 2000, MinY: 2000, MaxX: 2300, MaxY: 2300}}
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100, UID: "shared-zone-ticket"}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["shared:nightmare-entry-coverage"]
	if inst == nil {
		t.Fatal("shared timed instance fixture failed to create its room")
	}
	if member.Char.Inv[0].Index != 100 {
		t.Fatal("shared timed instance creation consumed the non-participating member ticket")
	}
	return w, leader, member, st, clock, rule, inst
}

func TestJoinSharedTimedInstanceRejectionsPreserveTicket(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*World, *Player, *Player, *fakeClock, *model.VolatileRule, *ItemInstance)
	}{
		{name: "wrong map", mutate: func(_ *World, _ *Player, member *Player, _ *fakeClock, _ *model.VolatileRule, _ *ItemInstance) {
			member.X, member.Y = 1000, 1000
		}},
		{name: "closed schedule", mutate: func(_ *World, _ *Player, _ *Player, clock *fakeClock, _ *model.VolatileRule, inst *ItemInstance) {
			inst.ScheduleEnd = clock.Now().Add(-time.Second)
		}},
		{name: "party run limit", mutate: func(w *World, _ *Player, _ *Player, clock *fakeClock, rule *model.VolatileRule, _ *ItemInstance) {
			rule.PartyMode = "party"
			rule.Instance.PartyRunLimit = 1
			if w.nightmarePartyRuns == nil {
				w.nightmarePartyRuns = make(map[string]int)
			}
			w.nightmarePartyRuns[rule.Instance.SharedGroup] = 1
			_ = clock
		}},
		{name: "invalid entry mode", mutate: func(_ *World, _ *Player, _ *Player, _ *fakeClock, rule *model.VolatileRule, _ *ItemInstance) {
			rule.PartyMode = "unconfigured"
		}},
		{name: "nonleader party entry", mutate: func(_ *World, _ *Player, _ *Player, _ *fakeClock, rule *model.VolatileRule, _ *ItemInstance) {
			rule.PartyMode = "party"
		}},
		{name: "ineligible evolution", mutate: func(_ *World, _ *Player, member *Player, _ *fakeClock, _ *model.VolatileRule, _ *ItemInstance) {
			member.Char.Evolution = "celestial"
			member.Char.Score.Level = 180
			applyScore(member.Char)
		}},
		{name: "already in target instance", mutate: func(w *World, _ *Player, member *Player, _ *fakeClock, _ *model.VolatileRule, inst *ItemInstance) {
			inst.MemberIDs = append(inst.MemberIDs, member.ID)
			w.indexInstanceMembers(inst)
		}},
		{name: "already in another instance", mutate: func(w *World, _ *Player, member *Player, _ *fakeClock, _ *model.VolatileRule, _ *ItemInstance) {
			other := &ItemInstance{RuntimeID: "other-active-room", MemberIDs: []uint16{member.ID}}
			w.itemInstances[other.RuntimeID] = other
			w.setPlayerInstanceIndex(member.ID, other.RuntimeID)
		}},
		{name: "full instance", mutate: func(_ *World, _ *Player, _ *Player, _ *fakeClock, rule *model.VolatileRule, inst *ItemInstance) {
			rule.Instance.MaxPlayers, inst.Config.MaxPlayers = 1, 1
		}},
		{name: "no active room", mutate: func(_ *World, _ *Player, _ *Player, _ *fakeClock, _ *model.VolatileRule, inst *ItemInstance) {
			inst.CurrentStage = len(instanceStages(&inst.Config))
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, _, member, st, clock, rule, inst := sharedTimedEntryCoverageFixture(t)
			if test.mutate != nil {
				test.mutate(w, nil, member, clock, &rule, inst)
			}
			oldItem, oldX, oldY := member.Char.Inv[0], member.X, member.Y
			oldMembers := append([]uint16(nil), inst.MemberIDs...)
			saves := st.gameSaves
			w.joinSharedTimedItemInstance(member.Session, member, &member.Char.Inv[0], 0, rule, inst)
			if member.Char.Inv[0] != oldItem || member.X != oldX || member.Y != oldY ||
				!reflect.DeepEqual(inst.MemberIDs, oldMembers) || st.gameSaves != saves ||
				member.Session.QueuedPacketsForTest() < 2 {
				t.Fatalf("rejected shared entry changed state: item=%+v pos=(%d,%d) members=%v saves=%d",
					member.Char.Inv[0], member.X, member.Y, inst.MemberIDs, st.gameSaves)
			}
		})
	}
}

func TestJoinSharedTimedInstanceRestoresPartyAndWindowAfterSaveFailure(t *testing.T) {
	w, _, member, st, _, rule, inst := sharedTimedEntryCoverageFixture(t)
	oldItem, oldX, oldY := member.Char.Inv[0], member.X, member.Y
	oldMembers, oldSchedule := append([]uint16(nil), inst.MemberIDs...), inst.ScheduleEnd
	st.err = errors.New("instance transaction failed")
	w.joinSharedTimedItemInstance(member.Session, member, &member.Char.Inv[0], 0, rule, inst)
	if member.Char.Inv[0] != oldItem || member.X != oldX || member.Y != oldY ||
		member.Char.X != oldX || member.Char.Y != oldY || !reflect.DeepEqual(inst.MemberIDs, oldMembers) ||
		!inst.ScheduleEnd.Equal(oldSchedule) || w.itemInstanceForPlayer(member.ID) != nil ||
		member.Session.QueuedPacketsForTest() < 2 {
		t.Fatal("failed shared-zone save left a consumed ticket, moved player, or stale membership")
	}
}

func TestJoinSharedCubeRestoresTicketPositionAndMembershipAfterSaveFailure(t *testing.T) {
	w, leader, member, st, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "cube-shared-failure"
	rule.Instance.SharedEntry = true
	rule.Instance.MaxPlayers = 6
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100, UID: "cube-ticket"}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["cube-shared-failure"]
	if inst == nil {
		t.Fatal("Cube fixture failed to create the shared instance")
	}
	oldItem, oldX, oldY := member.Char.Inv[0], member.X, member.Y
	oldMembers := append([]uint16(nil), inst.MemberIDs...)
	saves := st.gameSaves
	st.err = errors.New("instance transaction failed")
	w.joinSharedItemInstance(member.Session, member, &member.Char.Inv[0], 0, rule, inst, useItemRequest{})
	if member.Char.Inv[0] != oldItem || member.X != oldX || member.Y != oldY ||
		member.Char.X != oldX || member.Char.Y != oldY || !reflect.DeepEqual(inst.MemberIDs, oldMembers) ||
		w.itemInstanceForPlayer(member.ID) != nil || st.gameSaves != saves+1 {
		t.Fatal("failed Cube entry left a consumed ticket, moved player, or stale membership")
	}
}

func TestJoinSharedCubeCommitsIndividualEntry(t *testing.T) {
	w, _, member, st, rule, inst := sharedCubeEntryCoverageFixture(t)
	oldX, oldY := member.X, member.Y
	saves := st.gameSaves
	w.joinSharedItemInstance(member.Session, member, &member.Char.Inv[0], 0, rule, inst, useItemRequest{})
	if member.Char.Inv[0].Index != 0 || !itemInstanceHasMember(inst, member.ID) ||
		w.itemInstanceForPlayer(member.ID) != inst || (member.X == oldX && member.Y == oldY) ||
		st.gameSaves != saves+1 || member.Session.QueuedPacketsForTest() < 4 {
		t.Fatalf("shared entry not committed: item=%+v members=%v position=(%d,%d) saves=%d",
			member.Char.Inv[0], inst.MemberIDs, member.X, member.Y, st.gameSaves)
	}
}

func TestJoinSharedCubeRejectsInvalidEntryState(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*World, *Player, *model.VolatileRule, *ItemInstance)
	}{
		{name: "no configured template", mutate: func(_ *World, _ *Player, rule *model.VolatileRule, _ *ItemInstance) {
			rule.Instance = nil
		}},
		{name: "outside entry area", mutate: func(_ *World, member *Player, rule *model.VolatileRule, _ *ItemInstance) {
			rule.Instance.EntryAreas = []model.VolatileInstanceEntryArea{{MinX: member.X + 10, MinY: member.Y + 10, MaxX: member.X + 20, MaxY: member.Y + 20}}
		}},
		{name: "not individually shared", mutate: func(_ *World, _ *Player, rule *model.VolatileRule, _ *ItemInstance) {
			rule.Instance.SharedEntry = false
		}},
		{name: "already in target room", mutate: func(w *World, member *Player, rule *model.VolatileRule, inst *ItemInstance) {
			inst.MemberIDs = append(inst.MemberIDs, member.ID)
			inst.Config.ChainNextItem = 100
			inst.Config.AllowChainDuringExitGrace = true
			inst.RewardGranted = true
			inst.ExitAt = w.now().Add(time.Minute)
			rule.Instance.ChainNextItem = 100
			rule.Instance.AllowChainDuringExitGrace = true
		}},
		{name: "already in another room", mutate: func(w *World, member *Player, _ *model.VolatileRule, _ *ItemInstance) {
			grace := &ItemInstance{RuntimeID: "grace-room", MemberIDs: []uint16{member.ID},
				Config:        model.VolatileInstance{ChainNextItem: 100, AllowChainDuringExitGrace: true},
				RewardGranted: true, ExitAt: w.now().Add(time.Minute)}
			active := &ItemInstance{RuntimeID: "other-room", MemberIDs: []uint16{member.ID}}
			w.itemInstances[grace.RuntimeID], w.itemInstances[active.RuntimeID] = grace, active
			w.setPlayerInstanceIndex(member.ID, active.RuntimeID)
		}},
		{name: "room already started", mutate: func(_ *World, _ *Player, _ *model.VolatileRule, inst *ItemInstance) {
			inst.CurrentStage = 1
		}},
		{name: "room no longer accepting players", mutate: func(_ *World, _ *Player, _ *model.VolatileRule, inst *ItemInstance) {
			inst.Remaining = 0
		}},
		{name: "room at capacity", mutate: func(_ *World, _ *Player, rule *model.VolatileRule, inst *ItemInstance) {
			rule.Instance.MaxPlayers, inst.Config.MaxPlayers = 1, 1
		}},
		{name: "evolution not eligible", mutate: func(_ *World, member *Player, rule *model.VolatileRule, _ *ItemInstance) {
			member.Char.Evolution = "mortal"
			rule.Instance.AllowedEvolutions = []string{"celestial"}
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, _, member, st, rule, inst := sharedCubeEntryCoverageFixture(t)
			test.mutate(w, member, &rule, inst)
			oldItem, oldX, oldY := member.Char.Inv[0], member.X, member.Y
			oldMembers, saves := append([]uint16(nil), inst.MemberIDs...), st.gameSaves
			w.joinSharedItemInstance(member.Session, member, &member.Char.Inv[0], 0, rule, inst, useItemRequest{})
			if member.Char.Inv[0] != oldItem || member.X != oldX || member.Y != oldY ||
				!reflect.DeepEqual(inst.MemberIDs, oldMembers) || st.gameSaves != saves ||
				member.Session.QueuedPacketsForTest() < 2 {
				t.Fatalf("rejected entry changed state: item=%+v pos=(%d,%d) members=%v saves=%d",
					member.Char.Inv[0], member.X, member.Y, inst.MemberIDs, st.gameSaves)
			}
		})
	}
}

func sharedCubeEntryCoverageFixture(t *testing.T) (*World, *Player, *Player, *guildFlowStore, model.VolatileRule, *ItemInstance) {
	t.Helper()
	w, leader, member, st, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.SharedEntry = true
	rule.Instance.MaxPlayers = 6
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100, UID: "cube-entry-ticket"}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil || !itemInstanceHasMember(inst, leader.ID) || itemInstanceHasMember(inst, member.ID) {
		t.Fatal("shared Cube entry fixture failed to create a leader-only instance")
	}
	return w, leader, member, st, rule, inst
}
