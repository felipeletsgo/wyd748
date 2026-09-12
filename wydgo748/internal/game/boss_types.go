package game

import (
	"fmt"
	"time"

	"wydgo/internal/model"
)

// boss_types.go -- traducao de CONFIGURACAO em COMPORTAMENTO.
//
// Os quatro tipos sao hardcoded aqui. O arquivo .lua so escolhe o tipo pelo nome
// e fornece numeros; nenhuma regra nova pode nascer de conteudo.
//
// A traducao gera um *BossProfile -- a mesma estrutura de regras/acoes que o
// runtime (boss_events.go, boss_actions.go) ja executa. Ou seja: os tipos sao
// GERADORES DE REGRA, e nao um segundo motor paralelo.
//
//	chaser   -> persegue e bate. Sem regra extra: a IA de mob ja faz isso, o
//	            tipo apenas calibra aggro/leash/alcance.
//	caster   -> ganha uma regra por skill, disparada ao levar dano, com
//	            cooldown e porta de HP proprios.
//	summoner -> ganha uma regra por grupo de adds.
//	phased   -> ganha um limiar de HP por fase, com escudo e troca de
//	            comportamento.
//
// Um boss pode acumular: um "phased" com skills e summons recebe as regras dos
// tres. O tipo define o que e OBRIGATORIO, nao o que e permitido.

// IDs gerados a partir de 100, deixando a faixa baixa livre para perfis
// escritos a mao em Go, se um dia existirem.
const (
	bossActionBase = BossActionID(100)
	bossRuleBase   = BossRuleID(100)
)

// phaseIDFor mapeia o indice da fase configurada para o ID interno. A fase 1
// (inicial) e sempre o comportamento base; as configuradas vem depois.
func phaseIDFor(index int) BossPhaseID { return BossPhaseID(index + 2) }

const bossInitialPhase = BossPhaseID(1)

// compileBossProfile traduz a configuracao no perfil executavel.
func compileBossProfile(config model.BossConfig) (*BossProfile, error) {
	profile := &BossProfile{
		ID:           config.ID,
		NPCName:      config.NPC,
		InitialPhase: bossInitialPhase,
		Phases:       map[BossPhaseID]BossPhase{},
		Actions:      map[BossActionID]BossActionDef{},
	}

	// Fase inicial: o comportamento declarado no topo do arquivo.
	profile.Phases[bossInitialPhase] = BossPhase{ID: bossInitialPhase, Name: string(config.Type)}

	// As fases configuradas viram fases numeradas, ordenadas por HP decrescente
	// (a de maior HP e atravessada primeiro).
	phaseOrder := sortedPhaseIndexes(config.Phases)
	for position, configIndex := range phaseOrder {
		phase := config.Phases[configIndex]
		id := phaseIDFor(position)
		name := string(phase.Type)
		if name == "" {
			name = string(config.Type)
		}
		profile.Phases[id] = BossPhase{ID: id, Name: name, EnterMessage: phase.Message}
		profile.HealthThresholds = append(profile.HealthThresholds, phase.HPPercent)
	}

	nextAction := bossActionBase
	nextRule := bossRuleBase
	// As acoes sao separadas por PAPEL para que o tipo de cada fase possa
	// habilitar so as suas. Mecanicas de fase (troca, escudo, adds da fase)
	// valem sempre: sao o roteiro do encontro, nao reacao de combate.
	var skillActions, summonActions, phaseActions []BossActionID
	allocAction := func(def BossActionDef) BossActionID {
		def.ID = nextAction
		profile.Actions[def.ID] = def
		nextAction++
		return def.ID
	}
	allocRule := func(rule BossRule) {
		rule.ID = nextRule
		profile.Rules = append(profile.Rules, rule)
		nextRule++
	}

	// --- skills: uma regra por skill, reagindo ao dano recebido.
	for _, skill := range config.Skills {
		actionID := allocAction(BossActionDef{
			Kind:     ActionCastSkill,
			Priority: priorityReaction,
			SkillID:  skill.ID,
			Cooldown: time.Duration(skill.CooldownSeconds) * time.Second,
			// Interruptivel: uma virada de fase tem precedencia sobre uma magia.
			Interruptible: true,
			Message:       skill.Message,
			CastRange:     skill.Range,
		})
		skillActions = append(skillActions, actionID)
		conditions := []BossCondition{
			{Kind: CondNoPendingAction},
			{Kind: CondActionReady, ActionID: actionID},
		}
		// max_hp_percent = 100 (ou 0) significa "sempre"; qualquer outro valor
		// vira uma porta de HP.
		if skill.MaxHPPercent > 0 && skill.MaxHPPercent < 100 {
			conditions = append(conditions, BossCondition{
				Kind: CondHPBelowOrEqual, Value: skill.MaxHPPercent,
			})
		}
		allocRule(BossRule{
			Event: BossEventDamaged, Priority: priorityReaction,
			Conditions: conditions, ActionID: actionID,
		})
	}

	// --- summons: uma regra por grupo.
	for _, summon := range config.Summons {
		actionID := allocAction(BossActionDef{
			Kind: ActionSummon,
			// Reacao de combate, NAO mecanica de fase: precisa ficar abaixo das
			// mecanicas para nao bloquea-las quando estiver pendente (o escudo
			// caindo, por exemplo).
			Priority:    priorityReaction,
			SummonNPC:   summon.NPC,
			SummonCount: summon.Count,
			SummonMax:   summon.MaxAlive,
			Cooldown:    time.Duration(summon.CooldownSeconds) * time.Second,
			Message:     summon.Message,
			// Meio segundo de conjuracao: o client processa a animacao antes de
			// varios mobs aparecerem de uma vez.
			CastTime:     500 * time.Millisecond,
			CanInterrupt: true,
			// Interrompivel de proposito: o summon vira acao PENDENTE, e uma
			// pendente nao-interrompivel BLOQUEARIA a troca de fase (que tem
			// prioridade maior). Uma mecanica de rotina nunca pode travar a
			// virada de fase do encontro.
			Interruptible: true,
		})
		summonActions = append(summonActions, actionID)
		allocRule(BossRule{
			Event: BossEventDamaged, Priority: priorityReaction,
			Conditions: []BossCondition{
				{Kind: CondNoPendingAction},
				{Kind: CondActionReady, ActionID: actionID},
			},
			ActionID: actionID,
		})
	}

	// --- fases: cada limiar troca a fase e, se pedido, ergue escudo.
	for position, configIndex := range phaseOrder {
		phase := config.Phases[configIndex]
		targetPhase := phaseIDFor(position)

		changeID := allocAction(BossActionDef{
			Kind: ActionChangePhase, Priority: priorityPhaseChange,
			Phase: targetPhase, CanInterrupt: true,
		})
		phaseActions = append(phaseActions, changeID)
		allocRule(BossRule{
			Event: BossEventHealthThresholdCrossed, Priority: priorityPhaseChange,
			Conditions: []BossCondition{
				{Kind: CondThresholdIs, Value: phase.HPPercent},
			},
			ActionID: changeID, Once: true,
			// SEM StopEvaluation: as regras de escudo abaixo tem prioridade
			// menor e precisam ser avaliadas DEPOIS, ja na fase nova.
		})

		if phase.ShieldPercent <= 0 {
			continue
		}
		shieldID := allocAction(BossActionDef{
			Kind: ActionApplyEffect, Priority: priorityMechanic,
			AffectType: bossShieldAffect, AffectValue: phase.ShieldPercent,
			AffectLevel: 1, AffectDuration: 30,
			Message: "",
		})
		phaseActions = append(phaseActions, shieldID)
		allocRule(BossRule{
			Event: BossEventHealthThresholdCrossed, Priority: priorityMechanic,
			Conditions: []BossCondition{
				{Kind: CondThresholdIs, Value: phase.HPPercent},
				{Kind: CondPhaseIs, Phase: targetPhase},
			},
			ActionID: shieldID, Once: true,
		})

		if !phase.ShieldUntilAddsDead {
			continue
		}

		// A fase precisa dos SEUS adds. Nao da para depender do summon
		// disparado por dano: ele vira acao pendente e a propria troca de fase
		// (prioridade maior) o cancelaria, deixando um escudo que nunca cairia
		// porque nunca houve guarda para matar.
		//
		// Prioridade abaixo do escudo de proposito: o escudo e instantaneo e
		// precisa ser aceito ANTES deste summon virar acao pendente.
		for _, summon := range config.Summons {
			phaseSummonID := allocAction(BossActionDef{
				Kind: ActionSummon, Priority: priorityMechanic - 2,
				SummonNPC: summon.NPC, SummonCount: summon.Count,
				SummonMax: summon.MaxAlive, Message: summon.Message,
				CastTime: 500 * time.Millisecond, CanInterrupt: true, Interruptible: true,
			})
			phaseActions = append(phaseActions, phaseSummonID)
			allocRule(BossRule{
				Event: BossEventHealthThresholdCrossed, Priority: priorityMechanic - 2,
				Conditions: []BossCondition{
					{Kind: CondThresholdIs, Value: phase.HPPercent},
					{Kind: CondPhaseIs, Phase: targetPhase},
				},
				ActionID: phaseSummonID, Once: true,
			})
		}

		dropID := allocAction(BossActionDef{
			Kind: ActionRemoveEffect, Priority: priorityMechanic,
			AffectType: bossShieldAffect,
			// Pode preemptar uma reacao pendente: a queda do escudo e a virada
			// do encontro e nao pode esperar um summon de rotina terminar.
			CanInterrupt: true,
		})
		phaseActions = append(phaseActions, dropID)
		allocRule(BossRule{
			Event: BossEventAddDied, Priority: priorityMechanic,
			Conditions: []BossCondition{
				{Kind: CondPhaseIs, Phase: targetPhase},
				{Kind: CondAddsAliveEqual, Value: 0},
			},
			ActionID: dropID, Once: true,
		})
	}

	applyPhaseBehaviour(profile, config, phaseOrder, skillActions, summonActions, phaseActions)

	if err := profile.Compile(); err != nil {
		return nil, fmt.Errorf("boss %q: %w", config.ID, err)
	}
	return profile, nil
}

// applyPhaseBehaviour faz o `type` de cada fase valer de verdade, restringindo
// quais REACOES existem nela.
//
// Sem isso o campo seria decorativo: as regras seriam avaliadas igual em toda
// fase e um "aos 60% vira summoner" nao mudaria nada. As mecanicas do roteiro
// (troca de fase, escudo, adds da fase) ficam sempre habilitadas -- elas contam
// a historia do encontro e nao podem depender do comportamento vigente.
//
// O `type` do topo diz o que o boss PRECISA ter (validacao); o `type` da fase
// diz o que ele USA naquele momento.
func applyPhaseBehaviour(profile *BossProfile, config model.BossConfig, phaseOrder []int,
	skillActions, summonActions, phaseActions []BossActionID) {
	enable := func(id BossPhaseID, behaviour model.BossType) {
		phase := profile.Phases[id]
		allowed := make(map[BossActionID]struct{}, len(phaseActions)+len(skillActions)+len(summonActions))
		for _, actionID := range phaseActions {
			allowed[actionID] = struct{}{}
		}
		switch behaviour {
		case model.BossTypeCaster:
			for _, actionID := range skillActions {
				allowed[actionID] = struct{}{}
			}
		case model.BossTypeSummoner:
			for _, actionID := range summonActions {
				allowed[actionID] = struct{}{}
			}
		case model.BossTypePhased:
			// O phased e o tipo "completo": usa tudo que estiver configurado.
			for _, actionID := range append(append([]BossActionID{}, skillActions...), summonActions...) {
				allowed[actionID] = struct{}{}
			}
		case model.BossTypeChaser:
			// So corpo a corpo: nenhuma reacao alem do ataque comum da IA.
		}
		phase.EnabledActions = allowed
		profile.Phases[id] = phase
	}

	enable(bossInitialPhase, config.Type)
	for position, configIndex := range phaseOrder {
		behaviour := config.Phases[configIndex].Type
		if behaviour == "" {
			behaviour = config.Type
		}
		enable(phaseIDFor(position), behaviour)
	}
}

// sortedPhaseIndexes devolve os indices das fases em ordem de HP DECRESCENTE.
// A fase de 60% e atravessada antes da de 30%, entao ela precisa vir antes.
func sortedPhaseIndexes(phases []model.BossPhase) []int {
	order := make([]int, len(phases))
	for i := range phases {
		order[i] = i
	}
	// Insercao simples: sao poucas fases e a estabilidade importa mais que a
	// complexidade assintotica.
	for i := 1; i < len(order); i++ {
		for j := i; j > 0 && phases[order[j]].HPPercent > phases[order[j-1]].HPPercent; j-- {
			order[j], order[j-1] = order[j-1], order[j]
		}
	}
	return order
}
