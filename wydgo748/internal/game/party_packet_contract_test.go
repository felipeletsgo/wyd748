package game

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/wire"
)

func partyPacketWorld(t *testing.T, memberID uint16) (*World, []*Player, *Party, *batchGameStore) {
	t.Helper()
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(memberID, "Member", 2101, 2100)
	other, _ := networkedTestPlayer(3, "Other", 2102, 2100)
	players := []*Player{leader, member, other}
	w, st := economyWorld(players...)
	party := &Party{Members: append([]*Player(nil), players...)}
	for _, player := range players {
		player.Party = party
	}
	return w, players, party, st
}

func partyRemoveRequest(target uint32) []byte {
	pkt := inboundPacket(wire.OpPartyRemove, 16)
	binary.LittleEndian.PutUint32(pkt[12:16], target)
	return pkt
}

func TestPartyRemoveRequiresExactNativeEnvelope(t *testing.T) {
	for _, size := range []int{0, 12, 15, 17, 20} {
		t.Run(fmt.Sprintf("size_%d", size), func(t *testing.T) {
			w, players, party, st := partyPacketWorld(t, 2)
			beforeMembers := append([]*Player(nil), party.Members...)
			pkt := make([]byte, size)
			if size >= 16 {
				binary.LittleEndian.PutUint32(pkt[12:16], 2)
			}
			// Exercise the handler's own guard independently of ingress filtering.
			w.onPartyRemove(players[0].Session, pkt)
			if !reflect.DeepEqual(party.Members, beforeMembers) || st.saves != 0 || st.batchSaves != 0 {
				t.Fatal("non-native removal envelope changed membership or persistence")
			}
			for _, player := range players {
				if player.Party != party || player.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("non-native removal envelope detached a member or published an update")
				}
			}
		})
	}
}

func TestPartyRemoveRejectsFullWidthTargetAliases(t *testing.T) {
	for _, tt := range []struct {
		name      string
		memberID  uint16
		requester int
		target    uint32
	}{
		{"zero_alias", 2, 1, 0x10000},
		{"self_alias", 2, 1, 0x10002},
		{"leader_alias", 2, 0, 0x10001},
		{"member_alias", 2, 0, 0x10002},
		{"signed_member_alias", 2, 0, 0xFFFF0002},
		{"all_bits_set_alias", 0xFFFF, 0, 0xFFFFFFFF},
	} {
		t.Run(tt.name, func(t *testing.T) {
			w, players, party, st := partyPacketWorld(t, tt.memberID)
			beforeMembers := append([]*Player(nil), party.Members...)
			pkt := partyRemoveRequest(tt.target)
			beforePacket := append([]byte(nil), pkt...)
			for attempt := 0; attempt < 2; attempt++ {
				w.handle(command{s: players[tt.requester].Session, pkt: pkt})
				if !reflect.DeepEqual(party.Members, beforeMembers) || party.leader() != players[0] ||
					st.saves != 0 || st.batchSaves != 0 {
					t.Fatal("invalid full-width target changed membership, leadership, or persistence")
				}
				for _, player := range players {
					if player.Party != party || player.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("invalid target detached a member or published a party update")
					}
				}
				if !bytes.Equal(pkt, beforePacket) {
					t.Fatal("party removal modified borrowed request bytes")
				}
			}
		})
	}
}

func TestPartyRemoveCanonicalLeaveAndLeaderRemoval(t *testing.T) {
	for _, tt := range []struct {
		name      string
		requester int
		target    uint32
		leaving   int
	}{
		{"member_zero_leave", 1, 0, 1},
		{"member_self_leave", 1, 2, 1},
		{"leader_removes_member", 0, 2, 1},
		{"leader_zero_leave", 0, 0, 0},
		{"leader_self_leave", 0, 1, 0},
	} {
		t.Run(tt.name, func(t *testing.T) {
			w, players, party, st := partyPacketWorld(t, 2)
			pkt := partyRemoveRequest(tt.target)
			w.handle(command{s: players[tt.requester].Session, pkt: pkt})
			leaving := players[tt.leaving]
			if leaving.Party != nil || len(party.Members) != 2 || party.indexOf(leaving) != -1 ||
				st.saves != 0 || st.batchSaves != 0 {
				t.Fatal("canonical removal did not detach exactly the requested member")
			}
			wantLeader := players[0]
			if tt.leaving == 0 {
				wantLeader = players[1]
			}
			if party.leader() != wantLeader {
				t.Fatal("canonical leave changed the expected leadership order")
			}
			for _, player := range players {
				if player != leaving && player.Party != party {
					t.Fatal("canonical removal detached an unrelated member")
				}
				response, ok := player.Session.DequeuePacketForTest()
				if !ok || !wire.Decrypt(response) || len(response) != 16 ||
					wire.ParseHeader(response).Type != wire.OpPartyRemove {
					t.Fatal("removal did not publish the canonical encrypted PartyRemove frame")
				}
				wantTarget := uint32(leaving.ID)
				if player == leaving || tt.leaving == 0 {
					wantTarget = 0
				}
				if got := binary.LittleEndian.Uint32(response[12:16]); got != wantTarget {
					t.Fatalf("removal target=%d, want %d", got, wantTarget)
				}
				for player.Session.QueuedPacketsForTest() != 0 {
					player.Session.DequeuePacketForTest()
				}
			}
			beforeMembers := append([]*Player(nil), party.Members...)
			w.handle(command{s: players[tt.requester].Session, pkt: pkt})
			if !reflect.DeepEqual(party.Members, beforeMembers) {
				t.Fatal("repeated removal changed the surviving party")
			}
			for _, player := range players {
				if player.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("repeated removal published another party transition")
				}
			}
		})
	}
}

func TestPartyRequestTargetRequiresExactNativeEnvelope(t *testing.T) {
	for _, size := range []int{0, 12, 43, 45, 48, 52} {
		t.Run(fmt.Sprintf("size_%d", size), func(t *testing.T) {
			pkt := make([]byte, size)
			if size >= 44 {
				binary.LittleEndian.PutUint32(pkt[40:44], 2)
			}
			if target, ok := partyRequestTarget(pkt); ok || target != 0 {
				t.Fatalf("non-native envelope selected target=%d, ok=%v", target, ok)
			}
		})
	}
	for _, target := range []uint32{0, 0x10000, 0x10002, 0xFFFF0002, 0xFFFFFFFF} {
		pkt := make([]byte, 44)
		binary.LittleEndian.PutUint32(pkt[40:44], target)
		if got, ok := partyRequestTarget(pkt); ok || got != 0 {
			t.Fatalf("invalid invitation target=%#x selected target=%d, ok=%v", target, got, ok)
		}
	}
	for _, target := range []uint32{1, 2, 0xFFFF} {
		pkt := make([]byte, 44)
		binary.LittleEndian.PutUint32(pkt[40:44], target)
		if got, ok := partyRequestTarget(pkt); !ok || uint32(got) != target {
			t.Fatalf("canonical invitation target=%d selected target=%d, ok=%v", target, got, ok)
		}
	}
}
