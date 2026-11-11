package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestPartyRequestTarget748(t *testing.T) {
	pkt := make([]byte, 44)
	binary.LittleEndian.PutUint32(pkt[40:44], 321)
	target, ok := partyRequestTarget(pkt)
	if !ok || target != 321 {
		t.Fatalf("incorrect 7.48 target: target=%d ok=%v", target, ok)
	}
}

func TestPartyRequestTargetRejectsLaterLayout(t *testing.T) {
	pkt := make([]byte, 48)
	binary.LittleEndian.PutUint32(pkt[40:44], 321)
	binary.LittleEndian.PutUint16(pkt[44:46], 654)
	target, ok := partyRequestTarget(pkt)
	if ok || target != 0 {
		t.Fatalf("later layout must not select either target: target=%d ok=%v", target, ok)
	}
}

func TestPartyRequestRejectsInvalidTargetsAndPartyState(t *testing.T) {
	tests := []struct {
		name       string
		targetID   uint32
		edit       func(*World, *Player, *Player)
		wantPacket int
	}{
		{name: "missing inviter session", targetID: 2, edit: func(w *World, inviter, _ *Player) {
			delete(w.players, inviter.Session)
		}},
		{name: "inviter outside world", targetID: 2, edit: func(_ *World, inviter, _ *Player) { inviter.InWorld = false }},
		{name: "inviter missing character", targetID: 2, edit: func(_ *World, inviter, _ *Player) { inviter.Char = nil }},
		{name: "missing target", targetID: 99, wantPacket: 1},
		{name: "self invite", targetID: 1, wantPacket: 1},
		{name: "target outside view", targetID: 2, edit: func(_ *World, _ *Player, target *Player) { target.X += viewHalfX + 1 }, wantPacket: 1},
		{name: "offline target", targetID: 2, edit: func(_ *World, _ *Player, target *Player) { target.InWorld = false }, wantPacket: 1},
		{name: "target already in party", targetID: 2, edit: func(_ *World, _ *Player, target *Player) {
			party := &Party{Members: []*Player{target}}
			target.Party = party
		}, wantPacket: 1},
		{name: "reserved inviter", targetID: 2, edit: func(w *World, inviter, _ *Player) {
			w.criticalDisconnects = map[*gameNet.Session]*criticalDisconnect{
				inviter.Session: {player: inviter},
			}
		}},
		{name: "reserved target", targetID: 2, edit: func(w *World, _, target *Player) {
			w.criticalDisconnects = map[*gameNet.Session]*criticalDisconnect{
				target.Session: {player: target},
			}
		}},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			inviter, inviterSession := networkedTestPlayer(1, "Leader", 2100, 2100)
			target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
			w := worldWithNetworkedPlayers(inviter, target)
			if test.edit != nil {
				test.edit(w, inviter, target)
			}
			request := make([]byte, 44)
			binary.LittleEndian.PutUint32(request[40:44], test.targetID)
			w.onPartyRequest(inviterSession, request)
			if target.InviteFrom != 0 || !target.InviteUntil.IsZero() ||
				inviterSession.QueuedPacketsForTest() != test.wantPacket {
				t.Fatalf("invalid invitation changed state: invite=%d until=%v packets=%d",
					target.InviteFrom, target.InviteUntil, inviterSession.QueuedPacketsForTest())
			}
		})
	}

	for _, targetID := range []uint32{0, 65536} {
		inviter, inviterSession := networkedTestPlayer(1, "Leader", 2100, 2100)
		target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
		w := worldWithNetworkedPlayers(inviter, target)
		request := make([]byte, 44)
		binary.LittleEndian.PutUint32(request[40:44], targetID)
		w.onPartyRequest(inviterSession, request)
		if target.InviteFrom != 0 || inviterSession.QueuedPacketsForTest() != 0 {
			t.Fatalf("invalid target ID %d reached a live player", targetID)
		}
	}
}

func TestPartyRequestRequiresLeaderAndAvailableCapacity(t *testing.T) {
	t.Run("only leader invites", func(t *testing.T) {
		captain, _ := networkedTestPlayer(3, "Captain", 2100, 2100)
		inviter, inviterSession := networkedTestPlayer(1, "Member", 2100, 2100)
		target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
		w := worldWithNetworkedPlayers(captain, inviter, target)
		party := &Party{Members: []*Player{captain, inviter}}
		captain.Party, inviter.Party = party, party
		request := make([]byte, 44)
		binary.LittleEndian.PutUint32(request[40:44], uint32(target.ID))
		w.onPartyRequest(inviterSession, request)
		if target.InviteFrom != 0 || inviterSession.QueuedPacketsForTest() != 1 {
			t.Fatal("nonleader invited a player", target.InviteFrom)
		}
	})

	t.Run("full party cannot invite", func(t *testing.T) {
		members := make([]*Player, 0, maxPartyMembers+1)
		var inviterSession *gameNet.Session
		for i := 0; i < maxPartyMembers+1; i++ {
			player, session := networkedTestPlayer(uint16(i+1), "PartyMember", 2100, 2100)
			members = append(members, player)
			if i == 0 {
				inviterSession = session
			}
		}
		w := worldWithNetworkedPlayers(members...)
		party := &Party{Members: append([]*Player(nil), members[:maxPartyMembers]...)}
		for _, member := range party.Members {
			member.Party = party
		}
		target := members[maxPartyMembers]
		request := make([]byte, 44)
		binary.LittleEndian.PutUint32(request[40:44], uint32(target.ID))
		w.onPartyRequest(inviterSession, request)
		if target.InviteFrom != 0 || inviterSession.QueuedPacketsForTest() != 1 {
			t.Fatal("full party admitted another member", target.InviteFrom)
		}
	})
}

func partyTestPlayer(id uint16, x, y uint16) *Player {
	return &Player{
		ID: id, X: x, Y: y, InWorld: true,
		Char: &model.Char{Name: "Player", Score: testScore(model.Score{MaxHP: 100, CurHP: 100})},
	}
}

func TestPartyExpGivesFullRewardWithTwoPercentPerMember(t *testing.T) {
	leader := partyTestPlayer(1, 2200, 2100)
	memberA := partyTestPlayer(2, 2201, 2101)
	memberB := partyTestPlayer(3, 2202, 2102)
	party := &Party{Members: []*Player{leader, memberA, memberB}}
	leader.Party, memberA.Party, memberB.Party = party, party, party

	shares := partyExpShares(leader, 1000, 2)
	if len(shares) != 3 {
		t.Fatalf("shares=%d, want 3", len(shares))
	}
	got := map[uint16]uint32{}
	for _, share := range shares {
		got[share.player.ID] = share.reward
	}
	if got[1] != 1060 || got[2] != 1060 || got[3] != 1060 {
		t.Fatalf("three-member rewards=%v, want 1060 each", got)
	}
}

func TestPartyExpExcludesDeadOfflineAndOtherSector(t *testing.T) {
	killer := partyTestPlayer(1, 2200, 2100)
	near := partyTestPlayer(2, 2201, 2101)
	dead := partyTestPlayer(3, 2202, 2102)
	setPlayerCurHP(dead.Char, 0)
	offline := partyTestPlayer(4, 2203, 2103)
	offline.InWorld = false
	far := partyTestPlayer(5, 2304, 2100) // X sector 18; the killer is in sector 17.
	party := &Party{Members: []*Player{killer, near, dead, offline, far}}
	for _, member := range party.Members {
		member.Party = party
	}

	shares := partyExpShares(killer, 999, 2)
	if len(shares) != 2 || shares[0].player != killer || shares[1].player != near {
		t.Fatalf("incorrect eligible members: %+v", shares)
	}
	if shares[0].reward != 1038 || shares[1].reward != 1038 { // floor(999 * 104%)
		t.Fatalf("incorrect bonus: %+v", shares)
	}
}

func TestPartyExpBonusFromOneToThirteenMembers(t *testing.T) {
	for members := 1; members <= maxPartyMembers; members++ {
		players := make([]*Player, 0, members)
		for i := 0; i < members; i++ {
			players = append(players, partyTestPlayer(uint16(i+1), 2200+uint16(i), 2100))
		}
		party := &Party{Members: players}
		for _, player := range players {
			player.Party = party
		}
		shares := partyExpShares(players[0], 1000, 2)
		want := uint32(1000 + members*20)
		if len(shares) != members {
			t.Fatalf("members=%d shares=%d", members, len(shares))
		}
		for _, share := range shares {
			if share.reward != want {
				t.Fatalf("members=%d reward=%d, want %d", members, share.reward, want)
			}
		}
	}
}

func TestPartyExpUsesConfiguredBonus(t *testing.T) {
	leader := partyTestPlayer(1, 2200, 2100)
	member := partyTestPlayer(2, 2201, 2101)
	party := &Party{Members: []*Player{leader, member}}
	leader.Party, member.Party = party, party

	shares := partyExpShares(leader, 10_000, 5)
	if len(shares) != 2 || shares[0].reward != 11_000 || shares[1].reward != 11_000 {
		t.Fatalf("configured bonus was not applied: %+v", shares)
	}
}

func TestDefaultGlobalExperienceProducesTenThousandTwoHundredSolo(t *testing.T) {
	config := model.DefaultGameplayConfig()
	player := partyTestPlayer(1, 2200, 2100)
	base := scaledMobExperience(1, config)
	shares := partyExpShares(player, base, config.PartyEXPBonusPercent)
	if len(shares) != 1 || shares[0].reward != 10_200 {
		t.Fatalf("incorrect default solo reward: base=%d shares=%+v", base, shares)
	}
}

func TestPartyExperienceExcludesInternalLevel399AtFinalEXPCap(t *testing.T) {
	maxed := partyTestPlayer(1, 2200, 2100)
	maxed.Char.Score.Level = maxMortalLevel
	maxed.Char.Exp = mortalNextLevel[400]
	if shares := partyExpShares(maxed, 10_000, 2); len(shares) != 0 {
		t.Fatalf("character at the level cap received EXP: %+v", shares)
	}
}

func TestPartyCapacityMatchesLeaderPlusTwelveMembers(t *testing.T) {
	if maxPartyMembers != 13 {
		t.Fatalf("capacity=%d, want leader + 12", maxPartyMembers)
	}
}
