package game

import (
	"errors"
	"log"
	"sort"
	"time"
)

// boss_events.go -- entrada de eventos, avaliacao de regras e submissao de
// intencoes.
//
// FLUXO AUTORITATIVO (spec §3):
//
//	evento -> regras do evento -> intencao -> World VALIDA -> inicia/agenda
//	                                                       -> World REVALIDA
//	                                                       -> executa
//
// A regra NUNCA muta estado. Ela apenas propoe. Uma condicao verdadeira durante
// a avaliacao nao dispensa a validacao antes de executar: entre as duas coisas o
// boss pode ter morrido, mudado de fase ou perdido o alvo.

var (
	errBossProfileNil = errors.New("perfil de boss nulo")
	errBossMobMissing = errors.New("mob do boss nao existe")
)

// maxBossEventDepth limita eventos encadeados (spec §21). Uma acao emite evento
// que dispara outra regra -- desejavel, mas precisa de freio contra ciclo.
const maxBossEventDepth = 16

// mobHPPercent devolve o HP do mob em porcentagem (0-100).
func mobHPPercent(m *Mob) int {
	if m == nil || m.Def == nil || m.Def.Score.MaxHP == 0 {
		return 0
	}
	if m.HP >= m.Def.Score.MaxHP {
		return 100
	}
	return int(uint64(m.HP) * 100 / uint64(m.Def.Score.MaxHP))
}

// notifyMobDamaged e o gancho chamado pelos caminhos de dano DEPOIS de alterar
// o HP do mob. Para um mob comum custa uma consulta de mapa e retorna.
//
// Emite, nesta ordem: CombatStarted (uma vez), Damaged e os limiares de HP
// atravessados. A ordem importa -- uma regra de fase precisa ver o combate ja
// iniciado.
func (w *World) notifyMobDamaged(m *Mob, oldHP uint32, sourceID uint16, damage uint32) {
	boss := w.bossFor(m.ID)
	if boss == nil {
		return
	}
	now := w.now()
	oldPercent := hpPercentOf(oldHP, m.Def.Score.MaxHP)
	newPercent := mobHPPercent(m)

	if !boss.InCombat {
		boss.InCombat = true
		w.handleBossEvent(boss, BossEvent{
			Type: BossEventCombatStarted, At: now, SourceID: sourceID,
			OldHPPercent: oldPercent, NewHPPercent: newPercent,
		}, 0)
	}

	w.handleBossEvent(boss, BossEvent{
		Type: BossEventDamaged, At: now, SourceID: sourceID, Damage: damage,
		OldHPPercent: oldPercent, NewHPPercent: newPercent,
	}, 0)

	w.emitCrossedThresholds(boss, sourceID, oldPercent, newPercent, now)
}

// hpPercentOf converte HP absoluto em percentual.
func hpPercentOf(hp, maxHP uint32) int {
	if maxHP == 0 {
		return 0
	}
	if hp >= maxHP {
		return 100
	}
	return int(uint64(hp) * 100 / uint64(maxHP))
}

// emitCrossedThresholds dispara UM evento por limiar atravessado, do maior para
// o menor. Um golpe grande (80% -> 20%) atravessa varios de uma vez e todos
// precisam ser emitidos, senao uma mecanica de fase seria pulada.
//
// A deteccao e por TRANSICAO, nunca por polling: so ha evento quando o HP
// realmente cruzou o limiar, e crossedThresholds impede repetir.
func (w *World) emitCrossedThresholds(boss *BossRuntime, sourceID uint16, oldPercent, newPercent int, now time.Time) {
	if newPercent >= oldPercent {
		return // cura ou sem mudanca: nada a atravessar
	}
	// HealthThresholds ja esta em ordem descendente (Compile).
	for _, threshold := range boss.Profile.HealthThresholds {
		if oldPercent <= threshold || newPercent > threshold {
			continue // nao atravessou este limiar agora
		}
		if _, already := boss.crossedThresholds[threshold]; already {
			continue
		}
		event := BossEvent{
			Type: BossEventHealthThresholdCrossed, At: now, SourceID: sourceID,
			OldHPPercent: oldPercent, NewHPPercent: newPercent, Threshold: threshold,
		}
		matched, accepted := w.handleBossEvent(boss, event, 0)
		if !matched || accepted {
			boss.crossedThresholds[threshold] = struct{}{}
			delete(boss.pendingThresholds, threshold)
			continue
		}
		// A regra existia e suas condicoes passaram, mas o World recusou a
		// acao por conflito. Nao consumir o limiar: a virada obrigatoria sera
		// reavaliada assim que a acao atual terminar.
		boss.pendingThresholds[threshold] = event
		log.Printf("BOSS %q: limiar %d pendente; acao bloqueada por conflito",
			boss.Profile.ID, threshold)
	}
}

// notifyBossAddDied avisa o encontro que um add morreu.
func (w *World) notifyBossAddDied(addID uint16) {
	if len(w.bosses) == 0 {
		return
	}
	for _, boss := range w.bosses {
		if _, mine := boss.Adds[addID]; !mine {
			continue
		}
		delete(boss.Adds, addID)
		w.handleBossEvent(boss, BossEvent{
			Type: BossEventAddDied, At: w.now(), AddID: addID,
		}, 0)
		return
	}
}

// handleBossEvent avalia as regras registradas para o evento. depth protege
// contra ciclo de eventos encadeados.
func (w *World) handleBossEvent(boss *BossRuntime, event BossEvent, depth int) (matched, accepted bool) {
	if depth > maxBossEventDepth {
		log.Printf("BOSS %q: limite de eventos encadeados atingido em %s; avaliacao interrompida",
			boss.Profile.ID, event.Type)
		return false, false
	}
	mob := w.mobsByID[boss.MobID]
	if mob == nil || mob.Dead {
		return false, false
	}
	for _, rule := range boss.Profile.rulesFor(event.Type) {
		if _, consumed := boss.ConsumedRules[rule.ID]; consumed {
			continue
		}
		if !w.bossConditionsMatch(boss, mob, rule.Conditions, event) {
			continue
		}
		matched = true
		if !w.submitBossIntent(boss, mob, rule, event, depth) {
			continue // rejeitada: a regra Once NAO e consumida
		}
		accepted = true
		if rule.Once {
			// Consumida somente quando ACEITA, para que uma rejeicao nao queime
			// a unica chance da regra.
			boss.ConsumedRules[rule.ID] = struct{}{}
		}
	}
	return matched, accepted
}

// retryBossThresholds reavalia limiares que cruzaram enquanto outra acao
// bloqueava a transicao. A ordem descendente preserva a semantica de um golpe
// que atravessa varias fases, e a operacao continua confinada ao World.
func (w *World) retryBossThresholds(boss *BossRuntime, now time.Time) {
	if boss == nil || len(boss.pendingThresholds) == 0 || boss.Pending != nil {
		return
	}
	thresholds := make([]int, 0, len(boss.pendingThresholds))
	for threshold := range boss.pendingThresholds {
		thresholds = append(thresholds, threshold)
	}
	sort.Sort(sort.Reverse(sort.IntSlice(thresholds)))
	for _, threshold := range thresholds {
		event := boss.pendingThresholds[threshold]
		event.At = now
		matched, accepted := w.handleBossEvent(boss, event, 0)
		if !matched || accepted {
			boss.crossedThresholds[threshold] = struct{}{}
			delete(boss.pendingThresholds, threshold)
		}
		if boss.Pending != nil {
			break
		}
	}
}

// bossConditionsMatch avalia as condicoes de uma regra. Todas precisam passar.
func (w *World) bossConditionsMatch(boss *BossRuntime, mob *Mob, conditions []BossCondition, event BossEvent) bool {
	for _, condition := range conditions {
		if !w.bossConditionMatches(boss, mob, condition, event) {
			return false
		}
	}
	return true
}

func (w *World) bossConditionMatches(boss *BossRuntime, mob *Mob, condition BossCondition, event BossEvent) bool {
	switch condition.Kind {
	case CondPhaseIs:
		return boss.Phase == condition.Phase
	case CondHPBelowOrEqual:
		return mobHPPercent(mob) <= condition.Value
	case CondThresholdIs:
		return event.Threshold == condition.Value
	case CondAddsAliveEqual:
		return boss.addsAlive() == condition.Value
	case CondNoPendingAction:
		return boss.Pending == nil
	case CondActionReady:
		return boss.actionReady(condition.ActionID, event.At)
	}
	return false
}

// submitBossIntent e a VALIDACAO AUTORITATIVA (spec §10). Roda antes de aceitar
// qualquer intencao, mesmo que as condicoes da regra tenham passado.
func (w *World) submitBossIntent(boss *BossRuntime, mob *Mob, rule BossRule, event BossEvent, depth int) bool {
	action, ok := boss.Profile.Actions[rule.ActionID]
	if !ok {
		return false
	}
	if mob.Dead {
		return false
	}
	if !boss.Profile.actionEnabled(boss.Phase, action.ID) {
		return false // acao nao pertence a esta fase
	}
	if !boss.actionReady(action.ID, event.At) {
		return false
	}
	if !w.resolveBossActionConflict(boss, action, event.At) {
		return false
	}

	// Cooldown comeca quando a acao e ACEITA. Se so comecasse na execucao, uma
	// interrupcao permitiria repetir a mecanica imediatamente (spec §13).
	if action.Cooldown > 0 {
		boss.Cooldowns[action.ID] = event.At.Add(action.Cooldown)
	}

	if action.CastTime <= 0 {
		w.executeBossAction(boss, mob, action, event, depth)
		return true
	}

	boss.Generation++
	boss.Pending = &BossPendingAction{
		Generation:    boss.Generation,
		ActionID:      action.ID,
		TargetID:      event.SourceID,
		ExecuteAt:     event.At.Add(action.CastTime),
		Priority:      action.Priority,
		Interruptible: action.Interruptible,
	}
	log.Printf("BOSS %q: acao %d agendada (regra %d, geracao %d)",
		boss.Profile.ID, action.ID, rule.ID, boss.Generation)
	return true
}

// resolveBossActionConflict decide se a nova acao pode comecar diante de uma
// acao pendente (spec §12.1). Regra: so prioridade ESTRITAMENTE maior
// interrompe, e apenas se ambos os lados permitirem. Prioridade igual nao
// substitui -- senao duas mecanicas de mesmo peso ficariam se cancelando.
func (w *World) resolveBossActionConflict(boss *BossRuntime, action BossActionDef, now time.Time) bool {
	pending := boss.Pending
	if pending == nil {
		return true
	}
	if !pending.Interruptible || !action.CanInterrupt {
		return false
	}
	if action.Priority <= pending.Priority {
		return false
	}
	// A acao antiga e descartada: incrementar a geracao invalida o callback
	// dela, que pode ja estar agendado.
	boss.Generation++
	boss.Pending = nil
	log.Printf("BOSS %q: acao %d cancelada por prioridade (%d > %d)",
		boss.Profile.ID, pending.ActionID, action.Priority, pending.Priority)
	return true
}

// tickBossActions vence as acoes pendentes. Chamado pelo tick do World -- nao
// existe goroutine nem ticker por boss.
func (w *World) tickBossActions(now time.Time) {
	if len(w.bosses) == 0 {
		return
	}
	for _, boss := range w.bosses {
		pending := boss.Pending
		if pending == nil {
			w.retryBossThresholds(boss, now)
			continue
		}
		if now.Before(pending.ExecuteAt) {
			continue
		}
		// A geracao invalida callbacks obsoletos: se o encontro resetou ou a
		// acao foi substituida, esta execucao nao vale mais.
		if pending.Generation != boss.Generation {
			boss.Pending = nil
			continue
		}
		mob := w.mobsByID[boss.MobID]
		// REVALIDACAO no vencimento: o mundo mudou desde a aceitacao.
		if mob == nil || mob.Dead || !boss.Profile.actionEnabled(boss.Phase, pending.ActionID) {
			boss.Pending = nil
			continue
		}
		action, ok := boss.Profile.Actions[pending.ActionID]
		if !ok {
			boss.Pending = nil
			continue
		}
		boss.Pending = nil
		w.executeBossAction(boss, mob, action, BossEvent{
			Type: BossEventDamaged, At: now, SourceID: pending.TargetID,
		}, 0)
		w.retryBossThresholds(boss, now)
	}
}
