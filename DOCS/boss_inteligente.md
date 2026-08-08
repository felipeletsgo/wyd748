# Especificação Técnica — Boss Inteligente Orientado a Eventos

> **NOTA DE ESTADO (24/07/2026).** Este documento é a **visão de longo prazo**.
> O que existe em código e testes é um subconjunto, descrito em `DOCS/BOSS.md`.
>
> **Implementado:** a arquitetura central (boss = `Mob` + `BossRuntime`
> paralelo), o fluxo evento → regra → intenção → validação → execução, a
> invalidação por `Generation`, prioridades/conflito, regras `Once`, fases por
> limiar de HP, escudo com mitigação real e adds. Quatro tipos hardcoded
> (`chaser`, `caster`, `summoner`, `phased`) configurados por `data/boss/*.lua`.
>
> **Deliberadamente fora da v1:** timers, seletores de alvo, arena, leash
> próprio, reset de encontro, métricas por boss e painel de debug. A própria
> spec (§35) recomenda começar pequeno; construir 21 eventos, 19 condições e 15
> ações antes do primeiro encontro existir seria adivinhar requisito.
>
> **Divergências conscientes:** (a) não há `StopEvaluation`, `CondPhaseIsNot`
> nem `ActionSendMessage` — nenhum gerador os produzia; (b) **não há reset de
> encontro nem recuperação de HP**: o dano permanece onde os jogadores o
> deixaram, por decisão de gameplay (chefe de HP altíssimo, luta de horas).

**Projeto:** WYD748
**Status:** Proposta para implementação
**Escopo:** Bosses inteligentes sem aumentar a complexidade dos mobs comuns
**Princípio central:** o `World` permanece o único proprietário e executor autoritativo do estado de gameplay.

---

## 1. Objetivo

Implementar um novo tipo de entidade de gameplay para bosses, capaz de:

- reagir a eventos de combate;
- executar skills conforme o contexto;
- possuir fases;
- agendar mecânicas;
- selecionar alvos de forma especializada;
- invocar adds;
- controlar elementos da arena;
- interromper ou substituir ações conforme prioridade;
- manter comportamento determinístico e testável;
- permitir evolução futura para configurações externas e Lua.

Mobs comuns continuarão utilizando a IA simples já existente:

- acordar quando um jogador entra no raio;
- perseguir;
- atacar;
- retornar à origem;
- dormir quando não houver jogadores próximos.

A inteligência adicional será paga apenas pelos bosses ativos.

---

## 2. Decisão arquitetural

### 2.1 Modelo recomendado

Todo boss também é um `Mob`, mas possui um runtime adicional:

```text
Mob
├── posição
├── atributos
├── efeitos
├── alvo atual
├── movimento
├── combate básico
└── representação espacial

BossRuntime
├── perfil do encontro
├── fase atual
├── memória
├── cooldowns
├── timers
├── ação pendente
├── regras consumidas
└── estado específico do encontro
```

O registro do `World` deve manter:

```go
type World struct {
    mobs   map[MobID]*Mob
    bosses map[MobID]*BossRuntime

    // Demais estruturas autoritativas.
}
```

Todo boss está presente em `mobs`, pois participa normalmente de:

- grid espacial;
- visibilidade;
- combate;
- efeitos;
- colisão;
- movimentação;
- morte;
- envio de pacotes.

Somente bosses estão presentes em `bosses`.

### 2.2 O que não fazer

Não espalhar condições como esta por todo o servidor:

```go
if mob.Type == MobTypeBoss {
    // comportamento especial
}
```

A lógica exclusiva deve ficar concentrada no subsistema de boss.

Também não criar uma goroutine por boss. O `World` continua responsável por:

- receber eventos;
- avaliar regras;
- validar intenções;
- agendar ações;
- aplicar resultados.

---

## 3. Fluxo autoritativo

O fluxo geral será:

```text
Evento de gameplay
        ↓
World atualiza o estado autoritativo
        ↓
World identifica que a entidade é um boss
        ↓
BossRuntime recebe um BossEvent
        ↓
Somente regras associadas ao evento são avaliadas
        ↓
A melhor regra válida produz uma BossIntent
        ↓
World valida novamente a intenção
        ↓
World inicia ou agenda a ação
        ↓
Scheduler entrega o vencimento
        ↓
World valida geração, estado, alvo e condições
        ↓
World executa o efeito
        ↓
Novos eventos podem ser emitidos
```

As regras nunca devem aplicar dano, teleportar ou alterar o estado diretamente.

Elas apenas propõem intenções.

---

## 4. Modelo de dados

### 4.1 Mob base

A estrutura existente pode ser adaptada para algo conceitualmente semelhante:

```go
type Mob struct {
    ID         MobID
    TemplateID MobTemplateID

    Position Position
    Home     Position

    Stats   CombatStats
    Effects EffectContainer

    TargetID EntityID
    State    MobState

    NextMoveAt   time.Time
    NextAttackAt time.Time
}
```

### 4.2 BossRuntime

```go
type BossRuntime struct {
    MobID MobID

    Profile *BossProfile

    Phase      BossPhaseID
    PhaseSince time.Time

    CombatStartedAt time.Time

    Memory BossMemory

    Cooldowns map[BossActionID]time.Time
    Timers    map[BossTimerID]BossTimerRuntime

    PendingAction *BossActionExecution

    ConsumedRules map[BossRuleID]struct{}

    Sequence uint64
}
```

`BossRuntime` armazena apenas o estado necessário para o comportamento do encontro.

O estado físico e de combate continua no `Mob`.

---

## 5. Eventos de boss

### 5.1 Tipos iniciais

```go
type BossEventType uint8

const (
    BossEventCombatStarted BossEventType = iota
    BossEventCombatEnded

    BossEventDamaged
    BossEventHealthThresholdCrossed
    BossEventTargetChanged

    BossEventPlayerEnteredArena
    BossEventPlayerLeftArena
    BossEventPlayerDied
    BossEventPlayerStartedCast
    BossEventPlayerFinishedCast

    BossEventEffectApplied
    BossEventEffectRemoved

    BossEventAddSpawned
    BossEventAddDied

    BossEventTimerElapsed
    BossEventSkillStarted
    BossEventSkillFinished
    BossEventSkillInterrupted

    BossEventPhaseEntered
    BossEventPhaseExited

    BossEventLeashExceeded
    BossEventBossStuck
)
```

### 5.2 Estrutura do evento

```go
type BossEvent struct {
    Type BossEventType
    At   time.Time

    SourceID EntityID
    TargetID EntityID

    SkillID  SkillID
    EffectID EffectID

    Damage int

    OldHPPercent int
    NewHPPercent int

    TimerID BossTimerID
    AddID   MobID

    Sequence uint64
}
```

Campos irrelevantes para um evento permanecem zerados.

### 5.3 Eventos devem nascer no ponto correto

Exemplo: dano recebido.

```go
func (w *World) applyDamage(
    target EntityID,
    source EntityID,
    amount int,
    now time.Time,
) {
    mob := w.mobByEntity(target)
    if mob == nil {
        return
    }

    oldHPPercent := mob.Stats.HPPercent()

    w.applyDamageToMob(mob, amount)

    newHPPercent := mob.Stats.HPPercent()

    boss := w.bosses[mob.ID]
    if boss == nil {
        return
    }

    w.handleBossEvent(boss, BossEvent{
        Type:         BossEventDamaged,
        At:           now,
        SourceID:     source,
        TargetID:     target,
        Damage:       amount,
        OldHPPercent: oldHPPercent,
        NewHPPercent: newHPPercent,
    })

    w.emitCrossedHealthThresholdEvents(
        boss,
        source,
        oldHPPercent,
        newHPPercent,
        now,
    )
}
```

A transição de HP deve ser detectada no momento da alteração, não por polling periódico.

---

## 6. Limiares de HP

O perfil deve declarar os limiares relevantes:

```go
type BossProfile struct {
    ID BossProfileID

    InitialPhase BossPhaseID

    HealthThresholds []int

    Phases map[BossPhaseID]BossPhaseDefinition

    RulesByEvent map[BossEventType][]BossRule

    Actions map[BossActionID]BossActionDefinition
}
```

Exemplo:

```go
HealthThresholds: []int{75, 50, 25},
```

Detecção:

```go
func crossedDownward(
    oldPercent int,
    newPercent int,
    threshold int,
) bool {
    return oldPercent > threshold &&
        newPercent <= threshold
}
```

O evento deve ser emitido apenas quando o limiar for atravessado.

Dano alto pode atravessar mais de um limiar. Nesse caso, todos os limiares atravessados devem ser emitidos em ordem descendente:

```text
HP: 80% → 20%

Eventos:
75%
50%
25%
```

As regras e transições de fase decidirão quais eventos ainda são relevantes após cada mudança.

---

## 7. Regras

### 7.1 Estrutura

```go
type BossRule struct {
    ID BossRuleID

    Event BossEventType

    Priority int

    Conditions []BossCondition
    Action     BossActionSpec

    Cooldown time.Duration

    Once bool

    StopEvaluation bool
}
```

### 7.2 Condições

```go
type BossCondition interface {
    Evaluate(*BossContext) bool
}
```

Condições iniciais:

```text
PhaseIs
PhaseIsNot
BossHPBelowOrEqual
BossHPAbove
EventCrossedThreshold
ActionReady
NoPendingAction
TargetExists
TargetInRange
TargetOutsideRange
SourceIsCurrentTarget
SourceIsCasting
SourceHasEffect
NearbyPlayerCountAtLeast
NearbyPlayerCountAtMost
AddsAliveAtLeast
AddsAliveEqual
ElapsedSinceCombatStart
TimerIs
RuleNotConsumed
```

### 7.3 Ordenação

As regras associadas a um evento devem ser pré-ordenadas por prioridade durante o carregamento do perfil.

```go
sort.Slice(rules, func(i, j int) bool {
    if rules[i].Priority != rules[j].Priority {
        return rules[i].Priority > rules[j].Priority
    }

    return rules[i].ID < rules[j].ID
})
```

O desempate por ID garante determinismo.

### 7.4 Avaliação

```go
func (w *World) evaluateBossRules(
    boss *BossRuntime,
    event BossEvent,
    now time.Time,
) {
    rules := boss.Profile.RulesByEvent[event.Type]
    if len(rules) == 0 {
        return
    }

    ctx := NewBossContext(w, boss, event, now)

    for i := range rules {
        rule := &rules[i]

        if !w.canEvaluateBossRule(boss, rule, now) {
            continue
        }

        if !allBossConditionsMatch(ctx, rule.Conditions) {
            continue
        }

        intent, ok := buildBossIntent(boss, event, *rule)
        if !ok {
            continue
        }

        accepted := w.submitBossIntent(boss, intent, now)
        if !accepted {
            continue
        }

        w.markBossRuleAccepted(boss, rule, now)

        if rule.StopEvaluation {
            return
        }
    }
}
```

---

## 8. BossContext e consultas sob demanda

Algumas regras exigem consultas espaciais ou agregações.

Esses dados devem ser carregados somente quando usados.

```go
type BossContext struct {
    World *World
    Boss  *BossRuntime
    Event BossEvent
    Now   time.Time

    bossMob *Mob

    nearbyPlayersLoaded bool
    nearbyPlayersRadius int
    nearbyPlayers       []EntityID

    aliveAddsLoaded bool
    aliveAdds       []MobID
}
```

Regras que só verificam fase ou cooldown não devem pagar o custo de consultar a grid.

---

## 9. Intenções

### 9.1 Estrutura

```go
type BossIntent struct {
    BossID MobID

    RuleID BossRuleID

    Action BossActionSpec

    TriggerEventType     BossEventType
    TriggerEventSequence uint64

    Priority int

    CreatedAt time.Time
}
```

### 9.2 Tipos de ação

```go
type BossActionType uint8

const (
    BossActionCastSkill BossActionType = iota
    BossActionChangePhase
    BossActionSelectTarget
    BossActionMoveTo
    BossActionMoveAway
    BossActionTeleport
    BossActionSummon
    BossActionApplyEffect
    BossActionRemoveEffect
    BossActionStartTimer
    BossActionCancelTimer
    BossActionSendMessage
    BossActionLockArena
    BossActionUnlockArena
    BossActionResetEncounter
)
```

### 9.3 Especificação de ação

```go
type BossActionSpec struct {
    Type BossActionType

    ActionID BossActionID
    SkillID  SkillID
    PhaseID  BossPhaseID
    TimerID  BossTimerID
    EffectID EffectID

    TargetSelector BossTargetSelector

    Position Position

    Duration time.Duration
    Count    int
}
```

A intenção descreve o que o boss deseja fazer, mas ainda não garante que será executado.

---

## 10. Validação autoritativa

Antes de aceitar uma intenção, o `World` deve validar:

- boss existe;
- boss está vivo;
- encontro está ativo quando necessário;
- ação está habilitada na fase;
- cooldown terminou;
- boss não está silenciado;
- boss não está stunado;
- recurso necessário existe;
- alvo existe;
- alvo está vivo;
- alvo continua dentro das regras de alcance;
- posição é válida;
- arena permite a ação;
- ação pendente permite substituição;
- geração do encontro continua válida.

```go
func (w *World) submitBossIntent(
    boss *BossRuntime,
    intent BossIntent,
    now time.Time,
) bool {
    if !w.validateBossIntent(boss, intent, now) {
        return false
    }

    if !w.resolveBossActionConflict(boss, intent, now) {
        return false
    }

    return w.beginBossIntent(boss, intent, now)
}
```

Uma condição ser verdadeira durante a avaliação não elimina a necessidade de validar novamente antes da execução.

---

## 11. Ações pendentes e cast

### 11.1 Execução agendada

```go
type BossActionExecution struct {
    Generation uint64

    RuleID   BossRuleID
    ActionID BossActionID
    Type     BossActionType

    SkillID SkillID

    TargetID EntityID
    Position Position

    Priority int

    StartedAt time.Time
    ExecuteAt time.Time

    Interruptible bool
}
```

### 11.2 Início

```go
func (w *World) beginBossCast(
    boss *BossRuntime,
    intent BossIntent,
    targetID EntityID,
    now time.Time,
) bool {
    def := boss.Profile.Actions[intent.Action.ActionID]

    boss.Sequence++

    execution := &BossActionExecution{
        Generation: boss.Sequence,

        RuleID:   intent.RuleID,
        ActionID: def.ID,
        Type:     BossActionCastSkill,

        SkillID: def.SkillID,

        TargetID: targetID,
        Priority: intent.Priority,

        StartedAt: now,
        ExecuteAt: now.Add(def.CastTime),

        Interruptible: def.Interruptible,
    }

    boss.PendingAction = execution

    w.broadcastBossCastStarted(boss, execution)

    w.scheduleBossActionExecution(
        boss.MobID,
        execution.Generation,
        execution.ExecuteAt,
    )

    return true
}
```

### 11.3 Vencimento

```go
func (w *World) executeScheduledBossAction(
    bossID MobID,
    generation uint64,
    now time.Time,
) {
    boss := w.bosses[bossID]
    if boss == nil {
        return
    }

    pending := boss.PendingAction
    if pending == nil {
        return
    }

    if pending.Generation != generation {
        return
    }

    if !w.validatePendingBossAction(
        boss,
        pending,
        now,
    ) {
        w.cancelBossAction(
            boss,
            BossCancelInvalidated,
            now,
        )
        return
    }

    w.resolveBossAction(boss, pending, now)

    boss.PendingAction = nil

    w.handleBossEvent(boss, BossEvent{
        Type:     BossEventSkillFinished,
        At:       now,
        SkillID:  pending.SkillID,
        TargetID: pending.TargetID,
        Sequence: boss.Sequence,
    })
}
```

A comparação de `Generation` invalida callbacks antigos após cancelamento, reset ou substituição.

---

## 12. Prioridades e conflitos

Bosses podem receber vários eventos em sequência.

Exemplo:

```text
HP cruza 50%
timer da arena vence
healer inicia cast
add morre
```

As ações devem possuir prioridade.

Sugestão inicial:

```text
100 — reset ou encerramento do encontro
95  — transição obrigatória de fase
90  — mecânica crítica do encontro
80  — reação especial
70  — interrupção ou controle
50  — skill contextual
30  — skill ofensiva comum
10  — ataque básico
```

### 12.1 Política de conflito

```go
type BossActionDefinition struct {
    ID BossActionID

    Type BossActionType

    Priority int

    CastTime       time.Duration
    Cooldown       time.Duration
    GlobalCooldown time.Duration

    Interruptible bool
    CanInterrupt  bool

    RevalidateTargetOnExecute bool
}
```

Regras sugeridas:

1. Sem ação pendente: a nova ação pode começar.
2. Ação pendente não interrompível: nova ação é rejeitada.
3. Nova ação sem `CanInterrupt`: rejeitada.
4. Nova prioridade menor ou igual à atual: rejeitada.
5. Nova prioridade maior: ação atual é cancelada e a nova começa.
6. Transição de fase pode cancelar ações comuns.
7. Ataque comum nunca cancela uma mecânica de fase.

---

## 13. Cooldowns

Bosses podem possuir:

- cooldown por ação;
- cooldown global;
- cooldown por grupo;
- cooldown de regra.

Versão inicial recomendada:

```go
type BossRuntime struct {
    // ...

    Cooldowns map[BossActionID]time.Time

    GlobalCooldownUntil time.Time
}
```

```go
func (b *BossRuntime) ActionReady(
    actionID BossActionID,
    now time.Time,
) bool {
    if now.Before(b.GlobalCooldownUntil) {
        return false
    }

    return !now.Before(b.Cooldowns[actionID])
}
```

Cooldown deve ser iniciado quando a ação for aceita, executada ou finalizada conforme a definição da ação.

```go
type CooldownStartPolicy uint8

const (
    CooldownOnAccepted CooldownStartPolicy = iota
    CooldownOnExecuted
    CooldownOnFinished
)
```

Para evitar spam após interrupção, skills importantes geralmente devem iniciar cooldown quando aceitas.

---

## 14. Regras `Once`

Regras de uma única execução devem ser registradas no runtime.

```go
func (w *World) markBossRuleAccepted(
    boss *BossRuntime,
    rule *BossRule,
    now time.Time,
) {
    if rule.Once {
        boss.ConsumedRules[rule.ID] = struct{}{}
    }

    if rule.Cooldown > 0 {
        // Pode existir um mapa separado de cooldowns de regra.
    }
}
```

A regra deve ser consumida quando a intenção for aceita, e não apenas quando for avaliada.

---

## 15. Fases

### 15.1 Estrutura

```go
type BossPhaseDefinition struct {
    ID BossPhaseID

    Name string

    EnterActions []BossActionSpec
    ExitActions  []BossActionSpec

    EnabledActions map[BossActionID]struct{}

    EnabledRules map[BossRuleID]struct{}
}
```

### 15.2 Cuidados na transição

- impedir transição para fase inexistente;
- impedir ciclos acidentais durante `EnterActions`;
- limitar profundidade de eventos encadeados;
- definir se a ação pendente deve ser cancelada;
- limpar timers exclusivos da fase anterior;
- registrar duração da fase anterior;
- emitir `PhaseExited` e `PhaseEntered` em ordem determinística.

---

## 16. Timers

Timers devem gerar eventos; não devem executar a mecânica diretamente.

### 16.1 Definição

```go
type BossTimerRuntime struct {
    ID BossTimerID

    DueAt time.Time

    Generation uint64

    RepeatInterval time.Duration

    PhaseScoped bool
    PhaseID     BossPhaseID
}
```

### 16.2 Regras

- um timer pode ser único ou repetitivo;
- um timer de fase deve ser invalidado ao sair da fase;
- reiniciar um timer deve incrementar sua geração;
- callbacks com geração antiga devem ser ignorados;
- reset do encontro deve cancelar todos os timers;
- o vencimento deve emitir `BossEventTimerElapsed`;
- a regra acionada decide qual ação será executada.

---

## 17. Seletores de alvo

A seleção de alvo deve ser separada da execução da skill.

```go
type BossTargetSelectorType uint8

const (
    BossTargetCurrent BossTargetSelectorType = iota
    BossTargetEventSource
    BossTargetEventTarget
    BossTargetHighestThreat
    BossTargetLowestHP
    BossTargetHighestHP
    BossTargetNearest
    BossTargetFarthest
    BossTargetRandomPlayer
    BossTargetRandomNonTank
    BossTargetCastingPlayer
    BossTargetPositionOfEventSource
)
```

```go
type BossTargetSelector struct {
    Type BossTargetSelectorType

    Radius int

    ExcludeCurrentTarget bool
    ExcludeRecentTargets int
}
```

A seleção deve:

- usar apenas jogadores válidos;
- respeitar arena;
- respeitar alcance máximo da consulta;
- utilizar RNG controlado pelo `World`;
- possuir critério determinístico de desempate;
- retornar falha quando não houver candidato.

---

## 18. Summons e adds

Adds continuam sendo mobs normais, podendo ou não possuir comportamento especializado.

O boss mantém apenas referências aos adds do encontro:

```go
type BossMemory struct {
    LastTargetID EntityID

    SpawnedAdds map[MobID]BossAddMetadata

    AddsSpawned int
    AddsDied    int

    PlayersKilled int

    LastDamageAt time.Time
}
```

```go
type BossAddMetadata struct {
    GroupID BossAddGroupID
    PhaseID BossPhaseID

    SpawnedAt time.Time
}
```

No reset do encontro, todos os adds pertencentes à instância devem ser removidos de forma autoritativa.

---

## 19. Arena

Arena deve ser um subsistema opcional do encontro.

```go
type BossArenaDefinition struct {
    Bounds Area

    LockOnCombat bool

    ResetWhenEmpty bool

    EmptyResetDelay time.Duration

    MaxPlayers int
}
```

Ações possíveis:

- bloquear entrada;
- desbloquear entrada;
- fechar portões;
- teleportar jogadores para pontos permitidos;
- detectar arena vazia;
- resetar após abandono;
- impedir que o boss seja levado para fora;
- detectar leash.

A arena não deve confiar apenas no cliente.

---

## 20. Reset do encontro

Reset é uma operação crítica.

Deve:

1. invalidar ações agendadas;
2. cancelar timers;
3. remover efeitos temporários do encontro;
4. remover adds;
5. limpar regras consumidas;
6. limpar cooldowns;
7. restaurar HP e recursos;
8. restaurar posição;
9. restaurar fase inicial;
10. limpar ameaça e alvo;
11. restaurar arena;
12. incrementar geração;
13. retornar ao estado ocioso;
14. permitir dormir caso não existam jogadores próximos.

```go
func (w *World) resetBossEncounter(
    boss *BossRuntime,
    reason BossResetReason,
    now time.Time,
) {
    boss.Sequence++

    boss.PendingAction = nil
    clear(boss.Timers)
    clear(boss.Cooldowns)
    clear(boss.ConsumedRules)

    w.removeBossAdds(boss)
    w.clearBossEncounterEffects(boss)
    w.restoreBossMob(boss)
    w.restoreBossArena(boss)

    boss.Phase = boss.Profile.InitialPhase
    boss.PhaseSince = now
    boss.CombatStartedAt = time.Time{}

    boss.Memory = NewBossMemory()

    w.recordBossReset(boss, reason)
}
```

Qualquer callback antigo falhará pela geração ou pela ausência do timer ou ação correspondente.

---

## 21. Limite de eventos encadeados

Uma ação pode emitir um evento que ativa outra regra.

Exemplo:

```text
HealthThresholdCrossed
→ ChangePhase
→ PhaseEntered
→ StartTimer
```

Isso é desejável, mas deve haver proteção contra ciclos.

Sugestão:

```go
const maxBossEventsPerWorldCommand = 64
```

Preferir uma fila local de eventos processada iterativamente, evitando recursão profunda.

Ao ultrapassar o limite:

- interromper a avaliação;
- registrar erro estruturado;
- incrementar métrica;
- colocar o encontro em estado seguro;
- opcionalmente resetar o boss.

---

## 22. Exemplo de encontro

### 22.1 Fase 1

- inicia combate ao receber o primeiro dano;
- usa ataque comum;
- a cada 15 segundos marca um jogador distante;
- quando três ou mais jogadores ficam próximos, usa ataque em área.

### 22.2 Transição aos 60%

- cancela ação comum;
- entra na fase 2;
- aplica invulnerabilidade por 3 segundos;
- invoca quatro adds;
- inicia timer de 20 segundos para a mecânica da arena.

### 22.3 Fase 2

- cada morte de add aumenta o contador;
- ao morrerem os quatro adds, remove invulnerabilidade;
- a cada 20 segundos lança uma skill em uma região da arena;
- aos 20% entra em enrage.

### 22.4 Regras conceituais

```go
BossRule{
    ID:       RuleEnterPhase2,
    Event:    BossEventHealthThresholdCrossed,
    Priority: 95,
    Conditions: []BossCondition{
        EventThresholdIs(60),
        PhaseIs(Phase1),
    },
    Action:         ChangePhase(Phase2),
    Once:           true,
    StopEvaluation: true,
}
```

```go
BossRule{
    ID:       RuleRemoveShield,
    Event:    BossEventAddDied,
    Priority: 90,
    Conditions: []BossCondition{
        PhaseIs(Phase2),
        AddsAliveEqual(0),
    },
    Action: RemoveEffect(EffectInvulnerable),
    Once:   true,
}
```

```go
BossRule{
    ID:       RuleEnterEnrage,
    Event:    BossEventHealthThresholdCrossed,
    Priority: 95,
    Conditions: []BossCondition{
        EventThresholdIs(20),
    },
    Action:         ChangePhase(PhaseEnrage),
    Once:           true,
    StopEvaluation: true,
}
```

---

## 23. Definição em Go, JSON e Lua

### 23.1 Primeira versão

A primeira implementação deve usar definições em Go.

Vantagens:

- compilação;
- tipos fortes;
- refatoração segura;
- depuração simples;
- menor superfície de erro.

### 23.2 Segunda versão

Após o runtime estar estável, mover perfis para JSON, YAML ou outro formato de dados.

O carregador deve validar:

- IDs duplicados;
- fases inexistentes;
- ações inexistentes;
- timers inexistentes;
- referências circulares óbvias;
- thresholds fora de 0–100;
- cooldown negativo;
- cast time negativo;
- seletores incompatíveis;
- regras `Once` sem ID estável;
- fases sem caminho de saída quando isso for inválido.

### 23.3 Lua futura

Lua deve ser usada para definir condições ou decisões especiais, não para alterar diretamente o `World`.

```text
Go monta contexto limitado
        ↓
Lua recebe valores somente de leitura
        ↓
Lua retorna uma descrição de intenção
        ↓
Go valida o resultado
        ↓
World executa
```

Lua não pode:

- acessar ponteiros internos;
- percorrer o `World` arbitrariamente;
- modificar inventários;
- criar itens diretamente;
- aplicar dano sem validação;
- abrir arquivos;
- executar processos;
- acessar rede;
- controlar goroutines.

---

## 24. Scheduler

Bosses devem reutilizar o scheduler autoritativo do `World`.

Tipos de itens agendados:

```go
type ScheduledKind uint8

const (
    ScheduledBossAction ScheduledKind = iota
    ScheduledBossTimer
)
```

Não criar ticker individual para cada boss ou timer.

Exigências:

- entregar ações vencidas;
- medir atraso;
- invalidar callbacks antigos;
- não bloquear o `World`;
- manter ordem determinística para itens com o mesmo vencimento.

---

## 25. Métricas Prometheus

Métricas recomendadas:

```text
wyd_boss_events_total{boss,event}
wyd_boss_rules_evaluated_total{boss,event}
wyd_boss_rules_matched_total{boss,rule}
wyd_boss_intents_total{boss,action,result}
wyd_boss_actions_total{boss,action,result}
wyd_boss_action_lateness_seconds{boss,action}
wyd_boss_cast_duration_seconds{boss,action}
wyd_boss_cast_interruptions_total{boss,action,reason}
wyd_boss_phase_transitions_total{boss,from,to}
wyd_boss_phase_duration_seconds{boss,phase}
wyd_boss_resets_total{boss,reason}
wyd_boss_active
wyd_boss_pending_actions
wyd_boss_active_timers
```

Labels devem utilizar IDs de template ou nomes de perfil, nunca o ID de cada instância dinâmica.

Não usar:

```text
boss_instance_id
player_id
mob_id
operation_id
```

---

## 26. Logs e rastreabilidade

Eventos importantes devem produzir logs estruturados:

```text
boss_spawned
boss_combat_started
boss_rule_matched
boss_intent_rejected
boss_action_started
boss_action_cancelled
boss_action_executed
boss_phase_changed
boss_timer_started
boss_timer_elapsed
boss_reset
boss_event_limit_exceeded
```

Campos úteis:

```text
boss_profile
boss_instance
phase
rule
action
event
target
generation
priority
reason
scheduled_at
executed_at
lateness_ms
```

IDs individuais podem existir em logs; a restrição de cardinalidade se aplica às métricas.

---

## 27. Testabilidade

O subsistema deve utilizar um relógio injetável.

```go
type Clock interface {
    Now() time.Time
}
```

O RNG também deve ser injetável:

```go
type RNG interface {
    Intn(n int) int
}
```

Isso permite testes determinísticos de:

- timers;
- cooldowns;
- seleção aleatória;
- transições de fase;
- casts;
- interrupções;
- reset;
- atraso do scheduler.

---

## 28. Testes unitários obrigatórios

### Eventos

- emite `Damaged` após dano;
- emite threshold apenas ao atravessar;
- dano que atravessa vários thresholds emite todos em ordem;
- não repete threshold sem nova travessia válida;
- evento de add morto referencia o boss correto.

### Regras

- regras são avaliadas por prioridade;
- desempate é determinístico;
- condição falsa impede ação;
- `Once` é consumida somente quando aceita;
- cooldown de regra impede repetição;
- `StopEvaluation` interrompe regras inferiores.

### Intenções

- intenção inválida é rejeitada;
- alvo morto invalida cast;
- alvo fora de alcance invalida quando exigido;
- ação desabilitada na fase é rejeitada;
- skill em cooldown é rejeitada.

### Ações pendentes

- callback com geração antiga é ignorado;
- ação não interrompível não é substituída;
- prioridade superior substitui ação interrompível;
- prioridade igual não substitui;
- cancelamento emite evento correto.

### Fases

- transição executa saída e entrada;
- regra `Once` não repete fase;
- timers de fase anterior são invalidados;
- ação comum é cancelada quando definido;
- fase inexistente é rejeitada.

### Timers

- timer dispara no vencimento;
- timer cancelado não dispara;
- timer repetitivo agenda próximo vencimento;
- timer antigo não revive após reset;
- timer de fase é removido após transição.

### Reset

- remove adds;
- limpa timers;
- limpa cooldowns;
- limpa ação pendente;
- restaura fase;
- restaura HP;
- invalida callbacks antigos;
- restaura arena;
- não mantém regras consumidas.

---

## 29. Testes de integração

Criar ao menos um encontro completo executado em memória:

```text
spawn
→ jogador entra
→ combate inicia
→ timer dispara
→ boss usa skill
→ HP cruza 60%
→ fase muda
→ adds são criados
→ adds morrem
→ shield é removido
→ HP cruza 20%
→ enrage
→ boss morre
→ encontro encerra
```

O teste deve validar:

- ordem dos eventos;
- ações iniciadas;
- efeitos aplicados;
- timers;
- pacotes produzidos;
- métricas;
- ausência de callbacks após morte ou reset.

---

## 30. Testes de carga

Cenários:

```text
10 bosses ativos
50 bosses ativos
100 bosses ativos
```

Cada boss deve receber:

- dano frequente;
- timers;
- eventos de jogador;
- transições;
- casts;
- interrupções.

Medir:

- duração total de avaliação;
- regras avaliadas por evento;
- consultas espaciais;
- atraso p95 e p99;
- alocações;
- tamanho da fila do `World`;
- tempo de GC;
- ações invalidadas.

O teste deve provar que o custo cresce com bosses ativos e eventos reais, não com o total de mobs comuns.

---

## 31. Critérios de aceitação

A primeira versão estará pronta quando:

1. Um boss puder ser registrado sem alterar a IA dos mobs comuns.
2. O `World` continuar sendo o único mutador do estado.
3. Eventos acionarem apenas as regras correspondentes.
4. Regras produzirem intenções, sem alterar o estado diretamente.
5. Intenções forem validadas antes de iniciar e antes de executar.
6. Casts forem agendados com geração.
7. Timers gerarem eventos.
8. Fases puderem habilitar e desabilitar ações.
9. Regras `Once` não forem repetidas.
10. Prioridades resolverem conflitos de ação.
11. Reset invalidar todos os callbacks anteriores.
12. O comportamento for reproduzível em testes com relógio e RNG controlados.
13. Métricas permitirem medir atraso e volume.
14. Um encontro de duas fases funcionar de ponta a ponta.
15. Nenhuma goroutine for criada por boss.

---

## 32. Roadmap de implementação

### Etapa 1 — Fundação

- `BossRuntime`;
- registro `bosses`;
- `BossProfile`;
- `BossEvent`;
- `BossRule`;
- `BossCondition`;
- `BossIntent`;
- evento `CombatStarted`;
- evento `Damaged`;
- thresholds;
- regras `Once`.

### Etapa 2 — Execução

- definições de ação;
- validação autoritativa;
- ação pendente;
- cast time;
- geração;
- cooldown;
- interrupção;
- prioridade.

### Etapa 3 — Fases e timers

- `BossPhaseDefinition`;
- entrada e saída de fase;
- timers;
- eventos de timer;
- timers repetitivos;
- invalidação por fase.

### Etapa 4 — Mecânicas

- seletores de alvo;
- summons;
- adds;
- arena;
- efeitos;
- mensagens;
- leash;
- reset completo.

### Etapa 5 — Operação

- métricas;
- logs;
- painel de debug;
- comando administrativo para inspecionar boss;
- testes de carga;
- profiling.

### Etapa 6 — Conteúdo externo

- validação de perfis;
- carregamento por JSON;
- hot reload somente em desenvolvimento;
- Lua limitada para encontros especiais.

---

## 33. Estrutura de arquivos sugerida

```text
internal/game/
  mob.go
  mob_ai.go

  boss.go
  boss_profile.go
  boss_runtime.go

  boss_event.go
  boss_event_queue.go

  boss_rule.go
  boss_condition.go
  boss_context.go

  boss_intent.go
  boss_action.go
  boss_action_execution.go

  boss_phase.go
  boss_timer.go
  boss_target.go
  boss_arena.go
  boss_add.go

  boss_scheduler.go
  boss_metrics.go
  boss_debug.go

  boss_test.go
  boss_rules_test.go
  boss_scheduler_test.go
  boss_encounter_integration_test.go
```

A divisão exata pode ser ajustada ao tamanho real dos arquivos. O objetivo é impedir que toda a implementação termine em um único `boss.go`.

---

## 34. Interface mínima recomendada

```go
func (w *World) RegisterBoss(
    mobID MobID,
    profile *BossProfile,
    now time.Time,
) error

func (w *World) UnregisterBoss(
    mobID MobID,
    now time.Time,
)

func (w *World) HandleBossEvent(
    bossID MobID,
    event BossEvent,
)

func (w *World) SubmitBossIntent(
    bossID MobID,
    intent BossIntent,
    now time.Time,
) bool

func (w *World) ResetBossEncounter(
    bossID MobID,
    reason BossResetReason,
    now time.Time,
)
```

Internamente, os métodos podem receber ponteiros já resolvidos para evitar buscas repetidas.

---

## 35. Resumo executivo

A implementação deve seguir este modelo:

```text
Mob comum
    → IA simples existente
    → baixo custo

Boss
    → reutiliza Mob
    → possui BossRuntime
    → reage a eventos
    → avalia regras específicas
    → produz BossIntent
    → World valida
    → scheduler agenda
    → World revalida
    → ação é executada
```

Essa arquitetura oferece:

- bosses contextuais;
- fases complexas;
- custo proporcional à atividade;
- fácil criação de novos encontros;
- testabilidade determinística;
- isolamento da IA comum;
- segurança autoritativa;
- base adequada para Lua no futuro.

O primeiro boss implementado deve ser pequeno, com duas fases, um timer, uma skill contextual e um grupo de adds. Ele servirá como prova do runtime antes da criação de encontros mais complexos.
