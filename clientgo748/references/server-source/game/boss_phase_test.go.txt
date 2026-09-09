package game

import (
	"testing"

	"wydgo/internal/model"
)

// --- comportamento por fase -------------------------------------------------

// TestPhaseTypeRestrictsReactions e a prova de que `phases[].type` faz algo: um
// boss que "vira summoner" precisa parar de lancar magia e passar a invocar.
func TestPhaseTypeRestrictsReactions(t *testing.T) {
	config := model.BossConfig{
		ID: "t", NPC: "X", Type: model.BossTypeCaster,
		Spawn:   model.BossSpawn{X: 1, Y: 1},
		Skills:  []model.BossSkill{{ID: 26}},
		Summons: []model.BossSummon{{NPC: "Skeleton", Count: 2}},
		Phases:  []model.BossPhase{{HPPercent: 50, Type: model.BossTypeSummoner}},
	}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}

	skill := bossActionOfKind(t, profile, ActionCastSkill)
	var summon BossActionID
	for id, action := range profile.Actions {
		if action.Kind == ActionSummon {
			summon = id
			break
		}
	}

	// Fase inicial = caster: magia sim, invocacao nao.
	if !profile.actionEnabled(bossInitialPhase, skill) {
		t.Error("caster deveria poder lancar magia na fase inicial")
	}
	if profile.actionEnabled(bossInitialPhase, summon) {
		t.Error("caster nao deveria invocar na fase inicial")
	}

	// Fase 2 = summoner: inverte.
	second := phaseIDFor(0)
	if profile.actionEnabled(second, skill) {
		t.Error("summoner nao deveria lancar magia")
	}
	if !profile.actionEnabled(second, summon) {
		t.Error("summoner deveria poder invocar")
	}
}

// TestPhaseMechanicsAlwaysEnabled: o roteiro do encontro (troca de fase,
// escudo, adds da fase) nao pode depender do comportamento vigente, senao um
// escudo subiria e nunca cairia.
func TestPhaseMechanicsAlwaysEnabled(t *testing.T) {
	config := model.BossConfig{
		ID: "t", NPC: "X", Type: model.BossTypeChaser, // chaser: nenhuma reacao
		Spawn:   model.BossSpawn{X: 1, Y: 1},
		Summons: []model.BossSummon{{NPC: "Skeleton", Count: 2}},
		Phases: []model.BossPhase{{
			HPPercent: 50, ShieldPercent: 100, ShieldUntilAddsDead: true,
		}},
	}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	for id, action := range profile.Actions {
		switch action.Kind {
		case ActionChangePhase, ActionApplyEffect, ActionRemoveEffect:
			for phaseID := range profile.Phases {
				if !profile.actionEnabled(phaseID, id) {
					t.Errorf("mecanica %v (acao %d) desabilitada na fase %d", action.Kind, id, phaseID)
				}
			}
		}
	}
}

// TestChaserPhaseHasNoReactions confirma o outro extremo: um chaser configurado
// com skills nao as usa -- o tipo da fase e que manda.
func TestChaserPhaseHasNoReactions(t *testing.T) {
	config := model.BossConfig{
		ID: "t", NPC: "X", Type: model.BossTypeChaser,
		Spawn:  model.BossSpawn{X: 1, Y: 1},
		Skills: []model.BossSkill{{ID: 26}},
	}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	skill := bossActionOfKind(t, profile, ActionCastSkill)
	if profile.actionEnabled(bossInitialPhase, skill) {
		t.Error("chaser nao deveria lancar magia")
	}
}

// TestPhasedUsesEverything: o phased e o tipo completo.
func TestPhasedUsesEverything(t *testing.T) {
	config := model.BossConfig{
		ID: "t", NPC: "X", Type: model.BossTypePhased,
		Spawn:   model.BossSpawn{X: 1, Y: 1},
		Skills:  []model.BossSkill{{ID: 26}},
		Summons: []model.BossSummon{{NPC: "Skeleton", Count: 2}},
		Phases:  []model.BossPhase{{HPPercent: 50}},
	}
	profile, err := compileBossProfile(config)
	if err != nil {
		t.Fatal(err)
	}
	skill := bossActionOfKind(t, profile, ActionCastSkill)
	if !profile.actionEnabled(bossInitialPhase, skill) {
		t.Error("phased deveria usar magia")
	}
	// A fase sem `type` herda o comportamento do topo.
	if !profile.actionEnabled(phaseIDFor(0), skill) {
		t.Error("fase sem type deveria herdar o comportamento do topo")
	}
}
