package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func cubeQuestionTestInstance(w *World, leader, member *Player, now time.Time) (*ItemInstance, *Mob) {
	inst := &ItemInstance{
		Config: model.VolatileInstance{
			ID: "cube-question", ExitX: 2100, ExitY: 2100, TransitionSeconds: 1,
			Stages: []model.VolatileInstanceStage{
				{X: 2200, Y: 2200, AreaRadius: 8, Quiz: &model.VolatileInstanceQuiz{
					Question: "True_or_false?", Answer: true, TrueX: 2200, TrueY: 2200,
					FalseX: 2210, FalseY: 2210, DurationSeconds: 10, RewardExp: 8000,
				}},
				{X: 2250, Y: 2250, AreaRadius: 8},
			},
		},
		RuntimeID: "cube-question", LeaderID: leader.ID, MemberIDs: []uint16{leader.ID, member.ID},
		MobIDs: map[uint16]struct{}{1000: {}}, Remaining: 1,
		CombatDeadline: now.Add(time.Minute),
	}
	w.itemInstances[inst.RuntimeID] = inst
	leader.X, leader.Y = 2200, 2200
	member.X, member.Y = 2210, 2210
	for _, p := range []*Player{leader, member} {
		p.Char.X, p.Char.Y = p.X, p.Y
		drainPlayerPackets(p)
	}
	return inst, &Mob{ID: 1000, InstanceID: inst.RuntimeID}
}

func nextCubeQuestionPacket(t *testing.T, p *Player, opcode uint16) []byte {
	t.Helper()
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != opcode {
		t.Fatalf("player %d: expected packet %#x", p.ID, opcode)
	}
	return packet
}

func TestCubeQuestionPrivatePanelAndExactTenSecondWindow(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	outsider, _ := networkedTestPlayer(3, "Outside", 2201, 2201)
	w.playersByID[outsider.ID] = outsider
	inst, mob := cubeQuestionTestInstance(w, leader, member, clock.Now())
	oldExp := leader.Char.Exp
	w.onItemInstanceMobKilled(mob, clock.Now())
	if !inst.QuizDeadline.Equal(clock.Now().Add(10*time.Second)) || inst.QuizAt != inst.QuizDeadline {
		t.Fatal("room clear did not start a full ten-second answer window")
	}
	for _, p := range []*Player{leader, member} {
		nextCubeQuestionPacket(t, p, wire.OpInstanceMobs)
		timer := nextCubeQuestionPacket(t, p, wire.OpInstanceTime)
		question := nextCubeQuestionPacket(t, p, wire.OpInstanceQuestion)
		if binary.LittleEndian.Uint32(timer[12:16]) != 10 || len(question) != 116 ||
			wire.ParseHeader(question).ID != p.ID || binary.LittleEndian.Uint32(question[16:20]) != 10000 ||
			string(question[20:34]) != "True or false?" || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("question lost its text, private recipient or lifetime, or used the shared notice panel")
		}
	}
	if outsider.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("question leaked to a bystander")
	}
	clock.Advance(9999 * time.Millisecond)
	w.tickItemInstances(clock.Now())
	if inst.QuizAt.IsZero() || leader.Char.Exp != oldExp || len(inst.MemberIDs) != 2 {
		t.Fatal("quiz evaluated before the ten-second boundary")
	}
	for _, p := range []*Player{leader, member} {
		if p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("quiz was unexpectedly replaced or closed before its deadline")
		}
	}
	clock.Advance(time.Millisecond)
	w.tickItemInstances(clock.Now())
	for _, p := range []*Player{leader, member} {
		packet := nextCubeQuestionPacket(t, p, wire.OpInstanceQuestion)
		if binary.LittleEndian.Uint32(packet[16:20]) != 0 {
			t.Fatal("resolution must close the question before publishing EXP or departure")
		}
	}
	if !inst.QuizAt.IsZero() || len(inst.MemberIDs) != 1 || inst.MemberIDs[0] != leader.ID ||
		leader.Char.Exp <= oldExp || chebyshev(member.X, member.Y, 2100, 2100) > 6 {
		t.Fatal("answer resolution changed the existing reward or wrong-answer departure contract")
	}
}

func TestCubeQuestionRefreshesStaleKillTimestamp(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	inst, mob := cubeQuestionTestInstance(w, leader, member, clock.Now())
	killedAt := clock.Now()
	clock.Advance(3 * time.Second)
	w.onItemInstanceMobKilled(mob, killedAt)
	if !inst.QuizDeadline.Equal(clock.Now().Add(10 * time.Second)) {
		t.Fatal("kill persistence latency shortened the question window")
	}
}

func TestCubeQuestionSaveFailureDoesNotPublishCloseOrReward(t *testing.T) {
	w, leader, member, store, clock := instanceTestWorld()
	inst, mob := cubeQuestionTestInstance(w, leader, member, clock.Now())
	w.onItemInstanceMobKilled(mob, clock.Now())
	drainPlayerPackets(leader)
	drainPlayerPackets(member)
	oldExp, deadline := leader.Char.Exp, inst.QuizDeadline
	oldX, oldY := member.X, member.Y
	store.err = errors.New("quiz commit unavailable")
	clock.Advance(10 * time.Second)
	if w.resolveItemInstanceQuiz(inst, clock.Now()) {
		t.Fatal("quiz unexpectedly committed")
	}
	if leader.Char.Exp != oldExp || member.X != oldX || member.Y != oldY ||
		inst.QuizDeadline != deadline || len(inst.MemberIDs) != 2 {
		t.Fatal("failed commit exposed speculative quiz state")
	}
	for _, p := range []*Player{leader, member} {
		if p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("failed commit published a question close or result")
		}
	}
}
