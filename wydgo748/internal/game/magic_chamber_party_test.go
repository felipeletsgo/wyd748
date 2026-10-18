package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func magicChamberPartyRule() model.VolatileRule {
	rule := instanceTestRule()
	rule.Instance.ID = "magic-chamber-normal"
	rule.Instance.ChainStart = false
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = []model.VolatileInstanceStage{
		{Name: "Room 1", X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202, AreaRadius: 16,
			DurationSeconds: 180, Spawns: rule.Instance.Spawns},
		{Name: "Room 2", X: 2300, Y: 2300, SpawnX: 2302, SpawnY: 2302, AreaRadius: 16,
			DurationSeconds: 180, Spawns: rule.Instance.Spawns},
	}
	return rule
}

func assertChamberPeers(t *testing.T, w *World, a, b *Player, together bool) {
	t.Helper()
	if w.playersShareGameplaySpace(a, b) != together || a.hasVisible(b.ID) != together || b.hasVisible(a.ID) != together {
		t.Fatalf("party visibility mismatch: players=%d/%d space=%q/%q visible=%v/%v want=%v",
			a.ID, b.ID, w.gameplaySpaceForPlayer(a), w.gameplaySpaceForPlayer(b), a.hasVisible(b.ID), b.hasVisible(a.ID), together)
	}
}

func TestMagicChamberConcurrentPartiesStayIsolatedAcrossRooms(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	rule := magicChamberPartyRule()
	w.volatiles.Items[100] = rule
	otherLeader, _ := networkedTestPlayer(3, "OtherLeader", 2102, 2100)
	otherMember, _ := networkedTestPlayer(4, "OtherMember", 2103, 2100)
	otherLeader.Party = &Party{Members: []*Player{otherLeader, otherMember}}
	otherMember.Party = otherLeader.Party
	for _, p := range []*Player{otherLeader, otherMember} {
		w.players[p.Session], w.playersByID[p.ID] = p, p
		w.updatePlayerSpatial(p)
	}
	otherLeader.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	w.onUseItem(otherLeader.Session, useItemPacket(0, 0))
	if len(w.itemInstances) != 2 || otherLeader.Char.Inv[0].Index != 0 {
		t.Fatalf("independent parties did not both enter: instances=%d ticket=%d", len(w.itemInstances), otherLeader.Char.Inv[0].Index)
	}
	assertChamberPeers(t, w, leader, member, true)
	assertChamberPeers(t, w, otherLeader, otherMember, true)
	assertChamberPeers(t, w, leader, otherLeader, false)
	for _, inst := range w.itemInstances {
		if len(inst.MemberIDs) != 2 {
			t.Fatalf("party membership split: %+v", inst.MemberIDs)
		}
		for _, id := range inst.MemberIDs {
			p := w.playerByID(id)
			p.MovePublished = true
			p.MoveAuthorityRoute = []byte{1, 1}
			p.AirMoveActive = true
		}
		if ok, _ := w.spawnItemInstanceStage(inst, 1, clock.Now(), true, true); !ok {
			t.Fatal("party room transition failed")
		}
	}
	assertChamberPeers(t, w, leader, member, true)
	assertChamberPeers(t, w, otherLeader, otherMember, true)
	assertChamberPeers(t, w, leader, otherLeader, false)
	for _, p := range []*Player{leader, member, otherLeader, otherMember} {
		if p.MovePublished || p.AirMoveActive || len(p.MoveAuthorityRoute) != 0 {
			t.Fatal("room transition retained movement from the previous room")
		}
		assertChamberPartyRefresh(t, p)
	}
	for _, inst := range w.itemInstances {
		for id := range inst.MobIDs {
			mob := w.mobsByID[id]
			for _, p := range []*Player{leader, member, otherLeader, otherMember} {
				owned := w.gameplaySpaceForPlayer(p) == inst.RuntimeID
				if p.hasVisible(id) != owned || w.playerCanInteractWithMob(p, mob) != owned {
					t.Fatalf("monster ownership/visibility mismatch: player=%d mob=%d runtime=%s", p.ID, id, inst.RuntimeID)
				}
			}
		}
	}
}

func assertChamberPartyRefresh(t *testing.T, p *Player) {
	t.Helper()
	seen := make(map[uint16]bool)
	for _, packet := range labelPackets(t, p.Session) {
		if binary.LittleEndian.Uint16(packet[4:6]) == wire.OpPartyAdd {
			seen[binary.LittleEndian.Uint16(packet[20:22])] = true
		}
	}
	for _, member := range p.Party.Members {
		if !seen[member.ID] {
			t.Fatalf("player %d did not receive party identification for %d", p.ID, member.ID)
		}
	}
}

func TestMagicChamberCriticalEntryPublishesPartyOnlyAfterCommit(t *testing.T) {
	for _, mode := range []string{"", "solo"} {
		t.Run("mode="+mode, func(t *testing.T) {
			w, leader, member, _, _, base, q, _, release := criticalInstanceEntryFixture(t, "water", "commit")
			rule := magicChamberPartyRule()
			rule.PartyMode = mode
			w.volatiles.Items[100] = rule
			// A separate execution must not become a shared-entry destination.
			existing := &ItemInstance{RuntimeID: rule.Instance.ID, Config: *rule.Instance}
			w.itemInstances[existing.RuntimeID] = existing
			w.onUseItem(leader.Session, useItemPacket(0, 0))
			awaitCriticalWrite(t, base.entered)
			if w.gameplaySpaceForPlayer(leader) != "" || leader.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("pending entry published membership or packets")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			runtimeID := w.gameplaySpaceForPlayer(leader)
			if runtimeID == "" || runtimeID == existing.RuntimeID || len(w.itemInstances) != 2 {
				t.Fatalf("entry reused another execution: runtime=%q instances=%d", runtimeID, len(w.itemInstances))
			}
			if mode == "solo" {
				if len(w.itemInstances[runtimeID].MemberIDs) != 1 || w.gameplaySpaceForPlayer(member) != "" {
					t.Fatal("individual critical entry pulled the party")
				}
				assertChamberPeers(t, w, leader, member, false)
			} else {
				assertChamberPeers(t, w, leader, member, true)
			}
			assertChamberPartyRefresh(t, leader)
		})
	}
}

func TestMagicChamberIndividualEntryDoesNotPullParty(t *testing.T) {
	w, leader, member, _, _ := instanceTestWorld()
	rule := magicChamberPartyRule()
	rule.PartyMode = "solo"
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(member.Session, useItemPacket(0, 0))
	if w.gameplaySpaceForPlayer(member) == "" || w.gameplaySpaceForPlayer(leader) != "" {
		t.Fatal("individual entry moved or required the party leader")
	}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if len(w.itemInstances) != 2 {
		t.Fatal("individual executions reused the same runtime")
	}
	for _, inst := range w.itemInstances {
		if len(inst.MemberIDs) != 1 {
			t.Fatalf("individual chamber admitted multiple members: %+v", inst.MemberIDs)
		}
	}
	assertChamberPeers(t, w, leader, member, false)
	if leader.Party != member.Party || leader.Party == nil {
		t.Fatal("individual execution changed social party membership")
	}
}
