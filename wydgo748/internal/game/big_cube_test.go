package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func bigCubeTestConfig() model.VolatileInstance {
	return model.VolatileInstance{
		ID: "big-cube-test", Name: "Big Cube", StateMachine: "big_cube",
		X: 1342, Y: 1447, SpawnX: 1342, SpawnY: 1447, AreaRadius: 20,
		DurationSeconds: 300, NoCombatTimeout: true, ExitX: 2100, ExitY: 2100,
		Stages: []model.VolatileInstanceStage{{
			Name: "Big Cube", X: 1342, Y: 1447, SpawnX: 1342, SpawnY: 1447,
			AreaRadius: 20, Spawns: []model.VolatileInstanceSpawn{},
			Quiz: &model.VolatileInstanceQuiz{
				Question: "Test O or X", Answer: true,
				TrueX: 1316, TrueY: 1500, FalseX: 1366, FalseY: 1500,
				DurationSeconds: 10, RewardExp: 100,
			},
		}},
	}
}

func TestBigCubeQuestionIsPrivateAndCorrectPlatformReturnsToArmia(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	inst := &ItemInstance{
		Config: bigCubeTestConfig(), RuntimeID: "big-cube-test", LeaderID: leader.ID,
		MemberIDs: []uint16{leader.ID}, MobIDs: make(map[uint16]struct{}),
		CurrentStage: 0,
	}
	w.itemInstances[inst.RuntimeID] = inst
	if !w.startBigCubeQuestion(inst, clock.Now()) {
		t.Fatal("Big Cube did not open the question")
	}
	if inst.QuizAt.IsZero() || inst.QuizDeadline.IsZero() {
		t.Fatalf("question deadline was not recorded: %+v", inst)
	}
	leader.X, leader.Y = 1316, 1500 // O = Yes, the correct answer to this question.
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	clock.Advance(10 * time.Second)
	if !w.resolveBigCubeQuestion(inst, clock.Now()) {
		t.Fatal("Big Cube correct answer failed")
	}
	if len(inst.MemberIDs) != 1 || inst.ExitAt.IsZero() {
		t.Fatalf("correct player was not marked for return: %+v", inst)
	}
	w.tickItemInstances(clock.Now())
	if leader.X != 2100 || leader.Y != 2100 || w.itemInstances[inst.RuntimeID] != nil {
		t.Fatalf("correct player did not return to Armia: pos=(%d,%d) inst=%v",
			leader.X, leader.Y, w.itemInstances[inst.RuntimeID])
	}
}

func TestBigCubeWrongPlatformIsRemoved(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	inst := &ItemInstance{
		Config: bigCubeTestConfig(), RuntimeID: "big-cube-wrong", LeaderID: leader.ID,
		MemberIDs: []uint16{leader.ID}, MobIDs: make(map[uint16]struct{}),
		CurrentStage: 0,
	}
	w.itemInstances[inst.RuntimeID] = inst
	if !w.startBigCubeQuestion(inst, clock.Now()) {
		t.Fatal("Big Cube did not open the question")
	}
	leader.X, leader.Y = 1366, 1500 // X = No, but the test statement is true.
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	clock.Advance(10 * time.Second)
	if !w.resolveBigCubeQuestion(inst, clock.Now()) {
		t.Fatal("wrong answer should be processed")
	}
	w.tickItemInstances(clock.Now())
	if leader.X != 2100 || leader.Y != 2100 || len(inst.MemberIDs) != 0 {
		t.Fatalf("player on wrong platform was not removed: pos=(%d,%d) members=%v",
			leader.X, leader.Y, inst.MemberIDs)
	}
}

func TestBigCubeQuestionPrivatePacketAndFullAnswerWindow(t *testing.T) {
	w, leader, outsider, _, clock := instanceTestWorld()
	inst := &ItemInstance{Config: bigCubeTestConfig(), RuntimeID: "question-window", LeaderID: leader.ID,
		MemberIDs: []uint16{leader.ID}, MobIDs: make(map[uint16]struct{})}
	w.itemInstances[inst.RuntimeID] = inst
	drainPlayerPackets(leader)
	drainPlayerPackets(outsider)
	if !w.startBigCubeQuestion(inst, clock.Now()) || !inst.QuizDeadline.Equal(clock.Now().Add(10*time.Second)) {
		t.Fatal("question did not start a full ten-second window")
	}
	for _, opcode := range []uint16{wire.OpInstanceTime, wire.OpInstanceQuestion} {
		packet, ok := leader.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != opcode {
			t.Fatalf("missing private packet %#x", opcode)
		}
		if opcode == wire.OpInstanceQuestion && (len(packet) != 116 || wire.ParseHeader(packet).ID != leader.ID ||
			binary.LittleEndian.Uint32(packet[16:20]) != 10000 || string(packet[20:31]) != "Test O or X") {
			t.Fatal("question packet lost its recipient, text or lifetime")
		}
	}
	if outsider.Session.QueuedPacketsForTest() != 0 || leader.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("question leaked to a nonmember or used the shared notice panel")
	}
	leader.X, leader.Y = 1316, 1500
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	clock.Advance(9999 * time.Millisecond)
	w.tickItemInstances(clock.Now())
	if inst.QuizAt.IsZero() || leader.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("question resolved before the ten-second boundary")
	}
	clock.Advance(time.Millisecond)
	w.tickItemInstances(clock.Now())
	packet, ok := leader.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpInstanceQuestion ||
		binary.LittleEndian.Uint32(packet[16:20]) != 0 {
		t.Fatal("answer resolution did not close the question before publishing results")
	}
}
