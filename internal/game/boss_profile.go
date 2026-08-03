package game

import (
	"fmt"
	"sort"
	"time"
)

// boss_profile.go -- tipos que DESCREVEM um encontro de boss. Sao dados puros:
// nao tocam no mundo, nao mutam estado e podem ser validados no boot.
//
// A v1 e deliberadamente pequena (4 eventos, 5 condicoes, 5 acoes). A
// especificacao completa em DOCS/boss_inteligente.md preve muito mais, mas
// construir o framework inteiro antes do primeiro encontro existir seria
// adivinhar requisito. Cada tipo novo deve entrar quando um encontro precisar.
//
// Os perfis nascem em Go (tipados, compilados). Migrar para JSON e um passo
// posterior: a estrutura ja e serializavel de proposito.

type (
	BossPhaseID  int
	BossRuleID   int
	BossActionID int
)

// Prioridades de acao (spec §12). Transicao de fase acima de mecanica, que fica
// acima de reacao comum: um contra-ataque nunca cancela a virada de fase.
const (
	priorityPhaseChange = 95
	priorityMechanic    = 90
	priorityReaction    = 50
)

// BossEventType e o que aconteceu com o boss. Regras se registram por evento,
// entao so as regras daquele evento sao avaliadas -- o custo acompanha a
// atividade real, nao o tamanho do catalogo.
type BossEventType uint8

const (
	BossEventCombatStarted BossEventType = iota
	BossEventDamaged
	BossEventHealthThresholdCrossed
	BossEventAddDied
)

func (e BossEventType) String() string {
	switch e {
	case BossEventCombatStarted:
		return "combat_started"
	case BossEventDamaged:
		return "damaged"
	case BossEventHealthThresholdCrossed:
		return "threshold_crossed"
	case BossEventAddDied:
		return "add_died"
	}
	return "desconhecido"
}

// BossEvent carrega o contexto do que aconteceu. Campos irrelevantes para um
// tipo de evento ficam zerados.
type BossEvent struct {
	Type     BossEventType
	At       time.Time
	SourceID uint16 // quem causou (jogador, normalmente)
	Damage   uint32
	// OldHPPercent/NewHPPercent descrevem a transicao que gerou o evento.
	OldHPPercent int
	NewHPPercent int
	// Threshold e o limiar efetivamente atravessado (evento de threshold).
	Threshold int
	AddID     uint16
}

// BossConditionKind enumera as perguntas que uma regra pode fazer. Sao todas
// baratas: nenhuma varre o mundo. Consultas espaciais so entram quando um
// encontro exigir, e ai com carga sob demanda (§8 da spec).
type BossConditionKind uint8

const (
	CondPhaseIs         BossConditionKind = iota
	CondHPBelowOrEqual                    // percentual de HP do boss
	CondThresholdIs                       // o limiar do evento e exatamente Value
	CondAddsAliveEqual                    // quantos adds do encontro seguem vivos
	CondNoPendingAction                   // nao ha acao em curso
	CondActionReady                       // acao fora de cooldown
)

// BossCondition e uma condicao declarativa. Struct (e nao interface) para que o
// perfil seja um literal simples e, mais adiante, JSON.
type BossCondition struct {
	Kind     BossConditionKind
	Value    int
	ActionID BossActionID // usado por CondActionReady
	Phase    BossPhaseID  // usado por CondPhaseIs
}

// BossActionKind enumera o que o boss pode FAZER. Toda acao e executada pelo
// World -- a regra apenas propoe.
type BossActionKind uint8

const (
	ActionChangePhase BossActionKind = iota
	ActionCastSkill
	ActionSummon
	ActionApplyEffect
	ActionRemoveEffect
)

// BossActionDef descreve uma acao do encontro: custo de tempo, cooldown e
// parametros. Uma acao com CastTime > 0 vira acao PENDENTE, agendada e
// revalidada no vencimento.
type BossActionDef struct {
	ID       BossActionID
	Kind     BossActionKind
	Priority int
	CastTime time.Duration
	Cooldown time.Duration
	// Interruptible: uma acao de prioridade maior pode cancelar esta.
	Interruptible bool
	// CanInterrupt: esta acao pode cancelar uma acao pendente interrompivel.
	CanInterrupt bool

	Phase   BossPhaseID // ActionChangePhase
	SkillID int         // ActionCastSkill
	// CastRange e o alcance da skill. Zero cai no alcance de corpo a corpo.
	CastRange int
	// ActionSummon: nome do NPC (resolvido no catalogo) e quantidade.
	SummonNPC   string
	SummonCount int
	// SummonMax limita o total simultaneo deste grupo. Zero = sem limite.
	SummonMax int
	// ActionApplyEffect/ActionRemoveEffect: affect nativo.
	AffectType     byte
	AffectValue    int
	AffectLevel    int
	AffectDuration int // unidades de 8 s, como o affect nativo
	// Mensagem anunciada aos jogadores proximos.
	Message string
}

// BossPhase agrupa quais acoes existem em cada estagio do encontro.
type BossPhase struct {
	ID   BossPhaseID
	Name string
	// EnabledActions NULO significa "sem restricao"; qualquer mapa atribuido,
	// mesmo vazio, restringe a fase ao que estiver nele.
	//
	// A distincao entre nulo e vazio importa: uma fase de chaser restringe a
	// NENHUMA reacao, e com a regra "vazio libera tudo" ela acabaria liberando
	// justamente as magias que o tipo deveria proibir.
	EnabledActions map[BossActionID]struct{}
	// EnterMessage e anunciada ao entrar na fase, se preenchida.
	EnterMessage string
}

// BossRule liga um evento a uma acao, sob condicoes.
type BossRule struct {
	ID         BossRuleID
	Event      BossEventType
	Priority   int
	Conditions []BossCondition
	ActionID   BossActionID
	// Once: a regra e consumida quando a intencao e ACEITA (nao quando apenas
	// avaliada), para que uma rejeicao nao queime a unica chance.
	Once bool
}

// BossProfile e o encontro completo.
type BossProfile struct {
	ID string
	// NPCName casa com o nome do NPC no catalogo: e assim que um mob spawnado
	// vira boss, sem marcar tipo novo no arquivo de dados.
	NPCName      string
	InitialPhase BossPhaseID
	// HealthThresholds em percentual, ordem DESCENDENTE apos Compile.
	HealthThresholds []int
	Phases           map[BossPhaseID]BossPhase
	Actions          map[BossActionID]BossActionDef
	Rules            []BossRule

	// rulesByEvent e o indice pre-ordenado criado por Compile.
	rulesByEvent map[BossEventType][]BossRule
}

// Compile valida o perfil e monta o indice de regras por evento, JA ORDENADO
// por prioridade (desc) com desempate por ID (asc). Ordenar aqui, e nao a cada
// evento, mantem a avaliacao barata e -- mais importante -- DETERMINISTICA:
// duas regras de mesma prioridade sempre disparam na mesma ordem.
func (p *BossProfile) Compile() error {
	if p.ID == "" {
		return fmt.Errorf("perfil de boss sem ID")
	}
	if p.NPCName == "" {
		return fmt.Errorf("perfil %q: NPCName vazio", p.ID)
	}
	if _, ok := p.Phases[p.InitialPhase]; !ok {
		return fmt.Errorf("perfil %q: fase inicial %d inexistente", p.ID, p.InitialPhase)
	}
	for _, threshold := range p.HealthThresholds {
		if threshold <= 0 || threshold >= 100 {
			return fmt.Errorf("perfil %q: limiar de HP %d fora de (0,100)", p.ID, threshold)
		}
	}
	// Descendente: 75, 50, 25. A travessia de varios limiares de uma vez emite
	// na mesma ordem, do maior para o menor.
	sort.Sort(sort.Reverse(sort.IntSlice(p.HealthThresholds)))

	for id, action := range p.Actions {
		if action.CastTime < 0 || action.Cooldown < 0 {
			return fmt.Errorf("perfil %q: acao %d com tempo negativo", p.ID, id)
		}
		if action.Kind == ActionChangePhase {
			if _, ok := p.Phases[action.Phase]; !ok {
				return fmt.Errorf("perfil %q: acao %d muda para fase %d inexistente",
					p.ID, id, action.Phase)
			}
		}
		if action.Kind == ActionSummon && (action.SummonNPC == "" || action.SummonCount <= 0) {
			return fmt.Errorf("perfil %q: acao %d de summon sem NPC ou quantidade", p.ID, id)
		}
	}
	for phaseID, phase := range p.Phases {
		for actionID := range phase.EnabledActions {
			if _, ok := p.Actions[actionID]; !ok {
				return fmt.Errorf("perfil %q: fase %d habilita acao %d inexistente",
					p.ID, phaseID, actionID)
			}
		}
	}

	seen := make(map[BossRuleID]struct{}, len(p.Rules))
	index := make(map[BossEventType][]BossRule, 4)
	for _, rule := range p.Rules {
		if _, repeated := seen[rule.ID]; repeated {
			return fmt.Errorf("perfil %q: regra %d duplicada", p.ID, rule.ID)
		}
		seen[rule.ID] = struct{}{}
		if _, ok := p.Actions[rule.ActionID]; !ok {
			return fmt.Errorf("perfil %q: regra %d aponta para acao %d inexistente",
				p.ID, rule.ID, rule.ActionID)
		}
		for _, condition := range rule.Conditions {
			switch condition.Kind {
			case CondPhaseIs:
				if _, ok := p.Phases[condition.Phase]; !ok {
					return fmt.Errorf("perfil %q: regra %d referencia fase %d inexistente",
						p.ID, rule.ID, condition.Phase)
				}
			case CondActionReady:
				if _, ok := p.Actions[condition.ActionID]; !ok {
					return fmt.Errorf("perfil %q: regra %d referencia acao %d inexistente",
						p.ID, rule.ID, condition.ActionID)
				}
			}
		}
		index[rule.Event] = append(index[rule.Event], rule)
	}
	for event := range index {
		rules := index[event]
		sort.SliceStable(rules, func(i, j int) bool {
			if rules[i].Priority != rules[j].Priority {
				return rules[i].Priority > rules[j].Priority
			}
			return rules[i].ID < rules[j].ID
		})
		index[event] = rules
	}
	p.rulesByEvent = index
	return nil
}

// rulesFor devolve as regras ja ordenadas de um evento.
func (p *BossProfile) rulesFor(event BossEventType) []BossRule {
	return p.rulesByEvent[event]
}

// actionEnabled informa se a acao pode ocorrer na fase. Fase sem restricao
// explicita habilita todas as acoes do perfil.
func (p *BossProfile) actionEnabled(phaseID BossPhaseID, actionID BossActionID) bool {
	phase, ok := p.Phases[phaseID]
	if !ok {
		return false
	}
	if phase.EnabledActions == nil {
		return true // fase sem restricao declarada
	}
	_, enabled := phase.EnabledActions[actionID]
	return enabled
}
