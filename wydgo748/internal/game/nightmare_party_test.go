package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func assertNightmarePartyAdmission(t *testing.T, w *World, inst *ItemInstance, members ...*Player) {
	t.Helper()
	if inst == nil {
		t.Fatal("Nightmare admission did not create a runtime")
	}
	for _, p := range members {
		if !itemInstanceHasMember(inst, p.ID) || w.gameplaySpaceForPlayer(p) != inst.RuntimeID ||
			chebyshev(p.X, p.Y, inst.Config.X, inst.Config.Y) > 4 {
			t.Fatalf("party member %d was not admitted to runtime %q", p.ID, inst.RuntimeID)
		}
		if p.MovePublished || p.AirMoveActive || len(p.MoveAuthorityRoute) != 0 {
			t.Fatalf("party member %d retained movement from outside Nightmare", p.ID)
		}
		seenParty := make(map[uint16]bool)
		for _, packet := range labelPackets(t, p.Session) {
			if wire.ParseHeader(packet).Type == wire.OpPartyAdd {
				seenParty[binary.LittleEndian.Uint16(packet[20:22])] = true
			}
		}
		for _, peer := range members {
			if !seenParty[peer.ID] {
				t.Fatalf("member %d received no party identity for %d", p.ID, peer.ID)
			}
			if p != peer && (!w.playersShareGameplaySpace(p, peer) || !p.hasVisible(peer.ID)) {
				t.Fatalf("member %d cannot see party peer %d after admission", p.ID, peer.ID)
			}
		}
	}
}

func TestNightmarePartyAdmissionToNewAndExistingZone(t *testing.T) {
	for _, tier := range []string{"normal", "mystic", "arcane"} {
		t.Run(tier, func(t *testing.T) {
			w, leader, member, _, clock := instanceTestWorld()
			clock.current = time.Date(2026, 10, 17, 12, 0, 0, 0, time.UTC)
			rule := scheduledNightmareTestRule(tier, 0)
			w.volatiles.Items[100] = rule
			w.onUseItem(leader.Session, useItemPacket(0, 0))
			inst := w.itemInstances[instanceRuntimeKey(rule.Instance)]
			assertNightmarePartyAdmission(t, w, inst, leader, member)
			deadline := inst.HardDeadline

			otherLeader, _ := networkedTestPlayer(3, "OtherLeader", 2102, 2100)
			otherMember, _ := networkedTestPlayer(4, "OtherMember", 2103, 2100)
			otherLeader.Party = &Party{Members: []*Player{otherLeader, otherMember}}
			otherMember.Party = otherLeader.Party
			for _, p := range []*Player{otherLeader, otherMember} {
				w.players[p.Session], w.playersByID[p.ID] = p, p
				w.updatePlayerSpatial(p)
				p.MovePublished, p.AirMoveActive = true, true
				p.MoveAuthorityRoute = []byte{1, 1}
			}
			otherLeader.Char.Inv[0] = model.Item{Index: 100}
			clock.Advance(time.Minute)
			w.onUseItem(otherLeader.Session, useItemPacket(0, 0))
			assertNightmarePartyAdmission(t, w, inst, otherLeader, otherMember)
			if len(w.itemInstances) != 1 || len(inst.MemberIDs) != 4 || !inst.HardDeadline.Equal(deadline) ||
				otherLeader.Char.Inv[0].Index != 0 || otherMember.Party != otherLeader.Party {
				t.Fatal("joining split the party, reset the encounter or charged the wrong ticket")
			}
		})
	}
}

func TestNightmarePartyCriticalAdmissionWaitsForCommit(t *testing.T) {
	for _, existing := range []bool{false, true} {
		for _, outcome := range []string{"commit", "rejection"} {
			name := outcome + "/new"
			if existing {
				name = outcome + "/existing"
			}
			t.Run(name, func(t *testing.T) {
				w, leader, member, peer, _, base, q, _, release := criticalInstanceEntryFixture(t, "water", outcome)
				leader.Trade, peer.Trade = nil, nil
				w.clock = newFakeClock(time.Date(2026, 10, 17, 12, 0, 0, 0, time.UTC))
				rule := scheduledNightmareTestRule("mystic", 0)
				w.volatiles.Items[100] = rule
				runtimeID := instanceRuntimeKey(rule.Instance)
				if existing {
					inst := &ItemInstance{Config: cfgCopy(*rule.Instance), RuntimeID: runtimeID,
						MemberIDs: []uint16{peer.ID}, LeaderID: peer.ID,
						ScheduleEnd: w.now().Add(4 * time.Minute), HardDeadline: w.now().Add(19 * time.Minute)}
					prepareNightmareEntry(inst)
					w.itemInstances[runtimeID] = inst
					peer.X, peer.Y = rule.Instance.X+3, rule.Instance.Y
					peer.Char.X, peer.Char.Y = peer.X, peer.Y
					w.indexInstanceMembers(inst)
					w.updatePlayerSpatial(peer)
				}
				for _, p := range []*Player{leader, member} {
					p.MovePublished, p.AirMoveActive = true, true
					p.MoveAuthorityRoute = []byte{1, 1}
				}
				if err := w.queueCriticalInstanceTicket(leader, 0, rule, ""); err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, base.entered)
				for _, p := range []*Player{leader, member} {
					if w.gameplaySpaceForPlayer(p) != "" || p.X < 2100 || p.X > 2101 ||
						p.Session.QueuedPacketsForTest() != 0 || !p.MovePublished {
						t.Fatal("pending entry moved or published a party member")
					}
				}
				release()
				awaitReadyCriticalResults(t, q, 1)
				w.processCriticalCompletions(1)
				if outcome == "rejection" {
					for _, p := range []*Player{leader, member} {
						if w.gameplaySpaceForPlayer(p) != "" || !p.MovePublished || !p.AirMoveActive {
							t.Fatal("rejected entry changed party location or movement")
						}
					}
					if leader.Char.Inv[0].Index != 100 {
						t.Fatal("rejected entry consumed the ticket")
					}
					return
				}
				assertNightmarePartyAdmission(t, w, w.itemInstances[runtimeID], leader, member)
				if existing && (!peer.hasVisible(leader.ID) || !peer.hasVisible(member.ID)) {
					t.Fatal("existing participant cannot see the entering party")
				}
			})
		}
	}
}

func TestNightmareSoloTicketDoesNotTransportParty(t *testing.T) {
	for _, tier := range []string{"normal", "mystic", "arcane"} {
		t.Run(tier, func(t *testing.T) {
			w, leader, member, _, clock := instanceTestWorld()
			clock.current = time.Date(2026, 10, 17, 12, 0, 0, 0, time.UTC)
			rule := scheduledNightmareTestRule(tier, 0)
			rule.PartyMode = "solo"
			w.volatiles.Items[100] = rule
			// Solo entry is permitted for a non-leader but must not pull the leader.
			member.Char.Inv[0] = model.Item{Index: 100}
			w.onUseItem(member.Session, useItemPacket(0, 0))
			inst := w.itemInstances[instanceRuntimeKey(rule.Instance)]
			if inst == nil || len(inst.MemberIDs) != 1 || !itemInstanceHasMember(inst, member.ID) ||
				w.gameplaySpaceForPlayer(leader) != "" || leader.Char.Inv[0].Index != 100 {
				t.Fatal("solo ticket transported or charged another party member")
			}
			w.onUseItem(leader.Session, useItemPacket(0, 0))
			assertNightmarePartyAdmission(t, w, inst, leader, member)
			if len(w.itemInstances) != 1 || len(inst.MemberIDs) != 2 {
				t.Fatal("separate solo admissions changed the shared Nightmare runtime")
			}
		})
	}
}
