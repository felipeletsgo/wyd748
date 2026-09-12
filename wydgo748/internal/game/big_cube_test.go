package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
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
				Question: "Teste O ou X", Answer: true,
				TrueX: 1316, TrueY: 1500, FalseX: 1366, FalseY: 1500,
				DurationSeconds: 14, RewardExp: 100,
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
		t.Fatal("Big Cube nao abriu a pergunta")
	}
	if inst.QuizAt.IsZero() || inst.QuizDeadline.IsZero() {
		t.Fatalf("prazo da pergunta nao foi gravado: %+v", inst)
	}
	leader.X, leader.Y = 1316, 1500 // O = Sim, resposta correta desta pergunta.
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	clock.Advance(14 * time.Second)
	if !w.resolveBigCubeQuestion(inst, clock.Now()) {
		t.Fatal("resposta correta do Big Cube falhou")
	}
	if len(inst.MemberIDs) != 1 || inst.ExitAt.IsZero() {
		t.Fatalf("jogador correto nao foi marcado para retorno: %+v", inst)
	}
	w.tickItemInstances(clock.Now())
	if leader.X != 2100 || leader.Y != 2100 || w.itemInstances[inst.RuntimeID] != nil {
		t.Fatalf("jogador correto nao retornou a Armia: pos=(%d,%d) inst=%v",
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
		t.Fatal("Big Cube nao abriu a pergunta")
	}
	leader.X, leader.Y = 1366, 1500 // X = Nao, mas a afirmacao de teste e verdadeira.
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	clock.Advance(14 * time.Second)
	if !w.resolveBigCubeQuestion(inst, clock.Now()) {
		t.Fatal("resposta errada deveria ser processada")
	}
	w.tickItemInstances(clock.Now())
	if leader.X != 2100 || leader.Y != 2100 || len(inst.MemberIDs) != 0 {
		t.Fatalf("jogador na plataforma errada nao foi retirado: pos=(%d,%d) membros=%v",
			leader.X, leader.Y, inst.MemberIDs)
	}
}
