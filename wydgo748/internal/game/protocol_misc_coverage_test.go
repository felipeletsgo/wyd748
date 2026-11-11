package game

import (
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestPlayerChallengeIgnoresUnavailableSessionAndMalformedPacket(t *testing.T) {
	tests := []struct {
		name   string
		packet func([]byte) []byte
		setup  func(*World, *Player)
	}{
		{name: "unknown session", setup: func(w *World, p *Player) { delete(w.players, p.Session) }},
		{name: "player outside world", setup: func(_ *World, p *Player) { p.InWorld = false }},
		{name: "missing character", setup: func(_ *World, p *Player) { p.Char = nil }},
		{name: "truncated packet", packet: func(pkt []byte) []byte { return pkt[:len(pkt)-1] }},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			player, _ := networkedTestPlayer(1, "Requester", 2100, 2100)
			w := worldWithNetworkedPlayers(player)
			pkt := wire.StandardParm2(wire.OpPlayerChallenge, player.ID, 2, 0)
			if test.packet != nil {
				pkt = test.packet(pkt)
			}
			if test.setup != nil {
				test.setup(w, player)
			}
			w.onPlayerChallenge(player.Session, pkt)
			if got := player.Session.QueuedPacketsForTest(); got != 0 {
				t.Fatalf("ignored request queued %d packets", got)
			}
			if len(w.security) != 0 {
				t.Fatalf("ignored request recorded a security violation: %+v", w.security)
			}
		})
	}
}

func TestPlayerChallengeRejectsUnavailableTargets(t *testing.T) {
	tests := []struct {
		name      string
		targetID  uint32
		configure func(*Player, *Player)
	}{
		{name: "unknown target", targetID: 999},
		{name: "self challenge", targetID: 1},
		{name: "target not visible", targetID: 2},
		{
			name: "target outside view", targetID: 2,
			configure: func(requester, target *Player) {
				requester.show(target.ID)
				target.X = 2200
			},
		},
		{
			name: "target offline", targetID: 2,
			configure: func(requester, target *Player) {
				requester.show(target.ID)
				target.InWorld = false
			},
		},
		{
			name: "target has no character", targetID: 2,
			configure: func(requester, target *Player) {
				requester.show(target.ID)
				target.Char = nil
			},
		},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			requester, _ := networkedTestPlayer(1, "Requester", 2100, 2100)
			target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
			w := worldWithNetworkedPlayers(requester, target)
			if test.configure != nil {
				test.configure(requester, target)
			}
			beforeRequester := requester.Session.QueuedPacketsForTest()
			beforeTarget := target.Session.QueuedPacketsForTest()
			w.onPlayerChallenge(requester.Session,
				wire.StandardParm2(wire.OpPlayerChallenge, requester.ID, test.targetID, 0))
			if got := requester.Session.QueuedPacketsForTest(); got != beforeRequester+1 {
				t.Fatalf("unavailable target response count=%d, want %d", got, beforeRequester+1)
			}
			if got := target.Session.QueuedPacketsForTest(); got != beforeTarget {
				t.Fatalf("unavailable target received %d packets", got-beforeTarget)
			}
			if target.ChallengeFrom != 0 || target.ChallengeMode != 0 || !target.ChallengeUntil.IsZero() {
				t.Fatalf("unavailable target received an invitation: from=%d mode=%d until=%v",
					target.ChallengeFrom, target.ChallengeMode, target.ChallengeUntil)
			}
		})
	}
}

func TestPlayerChallengeClearsInviteWhenChallengerLeaves(t *testing.T) {
	challenger, _ := networkedTestPlayer(1, "Challenger", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(challenger, target)
	target.ChallengeFrom = challenger.ID
	target.ChallengeMode = 3
	target.ChallengeUntil = time.Now().Add(time.Minute)
	delete(w.playersByID, challenger.ID)
	challengerBefore := challenger.Session.QueuedPacketsForTest()
	targetBefore := target.Session.QueuedPacketsForTest()

	w.onPlayerChallenge(target.Session,
		wire.StandardParm2(wire.OpPlayerChallenge, target.ID, uint32(challenger.ID), 4))

	if target.ChallengeFrom != 0 || target.ChallengeMode != 0 || !target.ChallengeUntil.IsZero() {
		t.Fatal("acceptance did not consume an invitation whose challenger left")
	}
	if challenger.Session.QueuedPacketsForTest() != challengerBefore ||
		target.Session.QueuedPacketsForTest() != targetBefore+1 {
		t.Fatal("invalid acceptance notified the challenger or omitted the target response")
	}
}

func TestPlayerChallengeAcceptsDefaultAndAllVsAllModes(t *testing.T) {
	for _, mode := range []uint32{0, 2, 3} {
		t.Run(map[uint32]string{0: "default", 2: "10 vs 10", 3: "all vs all"}[mode], func(t *testing.T) {
			challenger, _ := networkedTestPlayer(1, "Challenger", 2100, 2100)
			target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
			w := worldWithNetworkedPlayers(challenger, target)
			challenger.show(target.ID)
			w.onPlayerChallenge(challenger.Session,
				wire.StandardParm2(wire.OpPlayerChallenge, challenger.ID, uint32(target.ID), mode))
			challengerBefore := challenger.Session.QueuedPacketsForTest()
			targetBefore := target.Session.QueuedPacketsForTest()

			w.onPlayerChallenge(target.Session,
				wire.StandardParm2(wire.OpPlayerChallenge, target.ID, uint32(challenger.ID), 4))

			if challenger.Session.QueuedPacketsForTest() != challengerBefore+1 ||
				target.Session.QueuedPacketsForTest() != targetBefore+1 {
				t.Fatalf("mode %d acceptance did not notify both players", mode)
			}
		})
	}
}

func TestPlayerChallengeRejectsInvalidTargetIDs(t *testing.T) {
	for _, id := range []uint32{0, uint32(^uint16(0)) + 1} {
		t.Run(map[uint32]string{0: "zero", uint32(^uint16(0)) + 1: "overflow"}[id], func(t *testing.T) {
			requester, _ := networkedTestPlayer(1, "Requester", 2100, 2100)
			w := worldWithNetworkedPlayers(requester)
			w.onPlayerChallenge(requester.Session,
				wire.StandardParm2(wire.OpPlayerChallenge, requester.ID, id, 0))
			if state := w.security[requester.Session]; state == nil || state.violations != 1 {
				t.Fatalf("invalid target ID %d did not record one violation: %+v", id, state)
			}
		})
	}
}
