package game

import (
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func nightmareReconnectFixture(t *testing.T, partySize int) (*World, []*Player, *ItemInstance, *fakeClock) {
	t.Helper()
	w, leader, member, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 17, 12, 0, 0, 0, time.UTC)
	rule := scheduledNightmareTestRule("normal", 0)
	nightmareTestProtectors(w, &rule)
	w.volatiles.Items[100] = rule
	members := []*Player{leader, member}
	for i := 2; i < partySize; i++ {
		p, _ := networkedTestPlayer(uint16(i+1), fmt.Sprintf("Member%d", i), 2101, 2101)
		p.Party = leader.Party
		leader.Party.Members = append(leader.Party.Members, p)
		w.players[p.Session], w.playersByID[p.ID] = p, p
		members = append(members, p)
	}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[instanceRuntimeKey(rule.Instance)]
	if inst == nil || len(inst.MemberIDs) != partySize {
		t.Fatal("group ticket did not admit the entire party")
	}
	for _, p := range members {
		labelPackets(t, p.Session)
	}
	return w, members, inst, clock
}

func reconnectNightmareCharacter(t *testing.T, w *World, old *Player) *Player {
	t.Helper()
	p, _ := networkedTestPlayer(99, old.Char.Name, old.X, old.Y)
	character := cloneCharacterState(old.Char)
	p.Account.Name = old.Account.Name
	p.Account.Chars[0] = character
	p.Char = &p.Account.Chars[0]
	p.ID, p.InWorld = 0, false
	w.players[p.Session] = p
	w.publishCharacterEntry(p, 0)
	return p
}

func TestNightmareReconnectRestoresHUDPartyAndVisibility(t *testing.T) {
	for _, size := range []int{2, 3} {
		for _, disconnected := range []int{0, 1} {
			for _, combat := range []bool{false, true} {
				t.Run(fmt.Sprintf("size%d/member%d/combat%v", size, disconnected, combat), func(t *testing.T) {
					w, members, inst, clock := nightmareReconnectFixture(t, size)
					party := members[0].Party
					if combat {
						clock.Advance(4 * time.Minute)
						w.tickItemInstances(clock.Now())
					}
					inst.NightmareLostMask = 3
					deadline := instanceCombatDeadline(inst)
					old := members[disconnected]
					w.finishDisconnect(old.Session, old)
					clock.Advance(time.Minute)
					for _, p := range members {
						labelPackets(t, p.Session)
					}
					p := reconnectNightmareCharacter(t, w, old)
					if p.Party != party || len(party.Members) != size ||
						w.itemInstanceForPlayer(p.ID) != inst || !instanceCombatDeadline(inst).Equal(deadline) {
						t.Fatal("reconnect lost the original party/runtime or reset its deadline")
					}
					var entered, timer, protectors, roster bool
					for _, packet := range labelPackets(t, p.Session) {
						switch wire.ParseHeader(packet).Type {
						case wire.OpEnterWorld:
							entered = true
						case wire.OpInstanceTime:
							timer = entered && binary.LittleEndian.Uint32(packet[12:16]) == uint32(remainingInstanceSeconds(deadline, clock.Now()))
						case wire.OpInstanceMobs:
							protectors = entered && binary.LittleEndian.Uint32(packet[12:16]) == 6
						case wire.OpPartyAdd:
							roster = entered
						}
					}
					if !timer || !protectors || !roster {
						t.Fatalf("missing post-entry snapshot: timer=%v protectors=%v roster=%v", timer, protectors, roster)
					}
					for _, peer := range party.Members {
						if peer == p {
							continue
						}
						if !p.hasVisible(peer.ID) || !peer.hasVisible(p.ID) {
							t.Fatal("reconnected party peers are not mutually visible")
						}
						seen := false
						for _, packet := range labelPackets(t, peer.Session) {
							if wire.ParseHeader(packet).Type == wire.OpPartyAdd && binary.LittleEndian.Uint16(packet[20:22]) == p.ID {
								seen = true
							}
						}
						if !seen {
							t.Fatal("connected peer did not receive the returning member's party identity")
						}
					}
				})
			}
		}
	}
}

func TestNightmareReconnectDoesNotJoinFullOrDissolvedParty(t *testing.T) {
	for _, scenario := range []string{"full", "dissolved", "expired", "replaced-runtime", "recycled-id"} {
		t.Run(scenario, func(t *testing.T) {
			w, members, inst, clock := nightmareReconnectFixture(t, 2)
			leader, old := members[0], members[1]
			party := leader.Party
			w.finishDisconnect(old.Session, old)
			switch scenario {
			case "full":
				for i := 1; i < maxPartyMembers; i++ {
					p, _ := networkedTestPlayer(uint16(20+i), fmt.Sprintf("Replacement%d", i), 2200, 2200)
					p.Party = party
					party.Members = append(party.Members, p)
					w.players[p.Session], w.playersByID[p.ID] = p, p
				}
			case "dissolved":
				w.removePartyPlayer(leader)
			case "expired":
				clock.Advance(20 * time.Minute)
			case "replaced-runtime":
				replacement := *inst
				w.itemInstances[inst.RuntimeID] = &replacement
			case "recycled-id":
				stranger, _ := networkedTestPlayer(old.ID, "Unrelated", 2100, 2100)
				w.players[stranger.Session], w.playersByID[stranger.ID] = stranger, stranger
				w.restoreNightmareSession(stranger)
				if stranger.Party != nil {
					t.Fatal("recycled actor ID inherited another character's party")
				}
				return
			}
			p := reconnectNightmareCharacter(t, w, old)
			if p.Party != nil {
				t.Fatal("reconnect joined a full, dissolved, expired or replaced party")
			}
			if scenario == "full" && (len(party.Members) != maxPartyMembers || w.itemInstanceForPlayer(p.ID) != inst) {
				t.Fatal("full party evicted a member or blocked encounter recovery")
			}
		})
	}
}

func TestNightmareReconnectAfterEntirePartyDisconnects(t *testing.T) {
	w, members, inst, _ := nightmareReconnectFixture(t, 2)
	party := members[0].Party
	for _, old := range members {
		w.finishDisconnect(old.Session, old)
	}
	first := reconnectNightmareCharacter(t, w, members[1])
	second := reconnectNightmareCharacter(t, w, members[0])
	if first.Party != party || second.Party != party || len(party.Members) != 2 ||
		w.itemInstanceForPlayer(first.ID) != inst || w.itemInstanceForPlayer(second.ID) != inst {
		t.Fatal("entire disconnected party did not recover into its original encounter")
	}
}

func TestNightmareGroupTicketOnlyChargesLeader(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 17, 12, 0, 0, 0, time.UTC)
	rule := scheduledNightmareTestRule("normal", 0)
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(member.Session, useItemPacket(0, 0))
	if len(w.itemInstances) != 0 || member.Char.Inv[0].Index != 100 {
		t.Fatal("nonleader used a group ticket")
	}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 0 || member.Char.Inv[0].Index != 100 ||
		w.itemInstanceForPlayer(leader.ID) != w.itemInstanceForPlayer(member.ID) {
		t.Fatal("group admission charged a member or split the party")
	}
}

func TestNightmareReconnectSoloRestoresHUDWithoutCreatingParty(t *testing.T) {
	w, p, _, _, clock := instanceTestWorld()
	w.removePartyPlayer(p)
	clock.current = time.Date(2026, 10, 17, 12, 0, 0, 0, time.UTC)
	rule := scheduledNightmareTestRule("normal", 0)
	rule.PartyMode = "solo"
	nightmareTestProtectors(w, &rule)
	w.volatiles.Items[100] = rule
	w.onUseItem(p.Session, useItemPacket(0, 0))
	inst := w.itemInstanceForPlayer(p.ID)
	if inst == nil {
		t.Fatal("solo ticket did not admit the player")
	}
	w.finishDisconnect(p.Session, p)
	clock.Advance(time.Minute)
	returning := reconnectNightmareCharacter(t, w, p)
	if returning.Party != nil || w.itemInstanceForPlayer(returning.ID) != inst {
		t.Fatal("solo reconnect created a party or lost its encounter")
	}
	assertNightmareHUD(t, returning, 180, 8)
}

func TestNightmareReconnectPartyReturnWaitsForDisconnectCommit(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	backend := &criticalDisconnectInstanceStore{criticalInstanceTestStore: &criticalInstanceTestStore{base}, instanceRelease: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w, p, _ := criticalLifecycleTestWorld(t, queue)
	room := addCriticalWaterRoom(w, p)
	room.Config.Mode, room.Config.NightmareTier = "shared_timed_zone", "normal"
	room.HardDeadline = w.now().Add(10 * time.Minute)
	peer, _ := networkedTestPlayer(2, "ConnectedPeer", p.X+1, p.Y)
	w.players[peer.Session], w.playersByID[peer.ID] = peer, peer
	w.indexPlayerCharacter(peer)
	room.MemberIDs = append(room.MemberIDs, peer.ID)
	w.setPlayerInstanceIndex(peer.ID, room.RuntimeID)
	party := &Party{Members: []*Player{p, peer}}
	p.Party, peer.Party = party, party
	w.handle(command{s: p.Session})
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	awaitCriticalWrite(t, base.entered)
	if len(w.nightmarePartyReturns) != 0 || p.Party != party || len(party.Members) != 2 {
		t.Fatal("held instance persistence changed party membership or published a return hint")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.players[p.Session] != nil || len(party.Members) != 1 || len(w.nightmarePartyReturns) != 1 {
		t.Fatal("disconnect acknowledgement did not retain the original party return")
	}
	returning := reconnectNightmareCharacter(t, w, p)
	if returning.Party != party || peer.Party != party || len(party.Members) != 2 {
		t.Fatal("committed disconnect could not recover the original party")
	}
}

func TestNightmareReconnectDefersPartyUntilReservationSettles(t *testing.T) {
	w, members, _, _ := nightmareReconnectFixture(t, 2)
	leader, old := members[0], members[1]
	party := leader.Party
	w.finishDisconnect(old.Session, old)
	w.indexPlayerCharacter(leader)
	if !w.claimAccountSession(leader.Session, leader.Account.Name) {
		t.Fatal("could not bind the remaining party leader")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{leader.Account}},
		[]*Player{leader}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	p := reconnectNightmareCharacter(t, w, old)
	w.tickNightmarePartyReturns()
	if p.Party != nil || len(party.Members) != 1 || len(w.nightmarePartyReturns) != 1 {
		t.Fatal("reconnect mutated a reserved party or discarded its deferred return")
	}
	assertNightmareHUD(t, p, 240, 8)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	w.tickNightmarePartyReturns()
	if p.Party != party || len(party.Members) != 2 || len(w.nightmarePartyReturns) != 0 {
		t.Fatal("party return was not retried after its reservation settled")
	}
}

func TestNightmareReconnectExpiryRemovesReturnShell(t *testing.T) {
	w, members, _, clock := nightmareReconnectFixture(t, 2)
	party := members[0].Party
	w.finishDisconnect(members[1].Session, members[1])
	clock.Advance(20 * time.Minute)
	w.tickNightmarePartyReturns()
	if members[0].Party != nil || len(w.nightmarePartyReturns) != 0 || !party.closed {
		t.Fatal("expired encounter retained an orphan party shell or return identity")
	}
}
