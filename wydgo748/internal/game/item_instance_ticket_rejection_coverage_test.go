package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func blockInstanceTicketTerrain(w *World, x, y uint16) {
	height := make([]byte, model.TerrainCells)
	attribute := make([]byte, model.AttributeCells)
	height[int(y)*model.TerrainWidth+int(x)] = model.TerrainBlockedByte
	w.terrain = model.TerrainMap{Height: height, Attribute: attribute}
}

func TestUseInstanceTicketRejectsInvalidAdmissionWithoutMutation(t *testing.T) {
	for _, tc := range []struct {
		name   string
		actor  string
		mutate func(*World, *Player, *Player, *model.VolatileRule, time.Time)
	}{
		{name: "missing template", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance = nil
		}},
		{name: "exclusive group already active", mutate: func(w *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance.ExclusiveGroup = "exclusive"
			w.itemInstances["other-exclusive"] = &ItemInstance{RuntimeID: "other-exclusive",
				Config: model.VolatileInstance{ExclusiveGroup: "exclusive"}}
		}},
		{name: "non shared target already active", mutate: func(w *World, leader, _ *Player, rule *model.VolatileRule, _ time.Time) {
			w.itemInstances[rule.Instance.ID] = &ItemInstance{RuntimeID: rule.Instance.ID,
				Config: *rule.Instance, MemberIDs: []uint16{leader.ID}}
		}},
		{name: "ticket used outside entry area", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance.EntryAreas = []model.VolatileInstanceEntryArea{{MinX: 2200, MinY: 2200, MaxX: 2210, MaxY: 2210}}
		}},
		{name: "schedule closed", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, now time.Time) {
			rule.Instance.Schedule = []model.VolatileInstanceWindow{{StartMinute: (now.Minute() + 1) % 60, DurationSeconds: 10}}
		}},
		{name: "party required", mutate: func(_ *World, leader, _ *Player, rule *model.VolatileRule, _ time.Time) {
			leader.Party = nil
			rule.PartyMode = "party"
		}},
		{name: "only party leader can enter", actor: "member", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.PartyMode = "party"
		}},
		{name: "no eligible party member", mutate: func(_ *World, leader, member *Player, rule *model.VolatileRule, _ time.Time) {
			leader.Party = &Party{Members: []*Player{leader, member}}
			leader.InWorld, member.InWorld = false, false
			rule.PartyMode = "raid"
		}},
		{name: "party member already in another instance", mutate: func(w *World, leader, _ *Player, _ *model.VolatileRule, _ time.Time) {
			active := &ItemInstance{RuntimeID: "other-active", MemberIDs: []uint16{leader.ID}}
			w.itemInstances[active.RuntimeID] = active
		}},
		{name: "no configured monsters", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance.Spawns = nil
		}},
		{name: "party exceeds capacity", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.PartyMode, rule.Instance.MaxPlayers = "party", 1
		}},
		{name: "evolution not allowed", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance.AllowedEvolutions = []string{"celestial"}
		}},
		{name: "entrance spawn not walkable", mutate: func(w *World, _ *Player, _ *Player, _ *model.VolatileRule, _ time.Time) {
			blockInstanceTicketTerrain(w, 2202, 2202)
		}},
		{name: "quiz platform not walkable", mutate: func(w *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance.Stages = []model.VolatileInstanceStage{{
				X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202, AreaRadius: 8,
				Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
				Quiz:   &model.VolatileInstanceQuiz{TrueX: 2205, TrueY: 2205, FalseX: 2206, FalseY: 2205},
			}}
			blockInstanceTicketTerrain(w, 2205, 2205)
		}},
		{name: "spawn area not walkable", mutate: func(w *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance.Spawns[0].X, rule.Instance.Spawns[0].Y = 2204, 2204
			blockInstanceTicketTerrain(w, 2204, 2204)
		}},
		{name: "monster missing", mutate: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule, _ time.Time) {
			rule.Instance.Spawns[0].NPC = "MissingRoomMonster"
		}},
		{name: "entrance has no free positions", mutate: func(w *World, _ *Player, leader *Player, rule *model.VolatileRule, _ time.Time) {
			w.criticalTeleportTiles = make(map[criticalTeleportTile]*Player)
			for y := uint16(2196); y <= 2204; y++ {
				for x := uint16(2196); x <= 2204; x++ {
					w.criticalTeleportTiles[criticalTile(x, y, rule.Instance.ID)] = leader
				}
			}
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, leader, member, st, clock := instanceTestWorld()
			rule := instanceTestRule()
			rule.PartyMode = "solo"
			rule.Instance.ID = "ticket-coverage"
			rule.Instance.ChainStart = false
			rule.Instance.AllowChainDuringExitGrace = false
			item := model.Item{Index: 100, UID: "ticket-uid"}
			actor := leader
			if tc.actor == "member" {
				actor = member
			}
			actor.Char.Inv[0] = item
			if tc.mutate != nil {
				tc.mutate(w, leader, member, &rule, clock.Now())
			}
			beforeInstances, beforeSaves := len(w.itemInstances), st.gameSaves
			beforePackets := actor.Session.QueuedPacketsForTest()
			w.useInstanceTicket(actor.Session, actor, &actor.Char.Inv[0], 0, rule, useItemRequest{})
			if actor.Char.Inv[0] != item || len(w.itemInstances) != beforeInstances || st.gameSaves != beforeSaves ||
				actor.Session.QueuedPacketsForTest() < beforePackets+2 {
				t.Fatalf("rejected ticket changed state: item=%+v instances=%d/%d saves=%d/%d packets=%d/%d",
					actor.Char.Inv[0], len(w.itemInstances), beforeInstances, st.gameSaves, beforeSaves,
					actor.Session.QueuedPacketsForTest(), beforePackets)
			}
		})
	}
}
