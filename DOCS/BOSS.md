# Sistema de boss — WYD-Go 7.48

Atualizado em 24/07/2026. Este documento descreve o subsistema **como foi
construído**. A visão de longo prazo, com o catálogo completo de eventos,
condições e ações, está em `DOCS/boss_inteligente.md`; o que segue é o
subconjunto que existe em código e testes.

## Princípio

Todo boss **é um `Mob` comum**. Ele participa do grid espacial, da visibilidade,
do combate, da morte e do envio de pacotes exatamente como qualquer monstro. O
comportamento extra vive num `BossRuntime` **paralelo**, indexado pelo ID do
mob (`World.bosses`).

Consequências práticas:

- a IA de mob comum não muda e não paga nada por existir boss no mundo;
- não há `if mob.IsBoss` espalhado pelo servidor — a lógica exclusiva fica em
  `internal/game/boss_*.go`;
- **nenhuma goroutine por boss**: o `World` continua sendo o único executor.

O teste `TestCommonMobIgnoresBossSubsystem` trava esse requisito.

## Fluxo autoritativo

```text
evento -> regras do evento -> intenção -> World VALIDA -> executa ou agenda
                                                       -> World REVALIDA
                                                       -> executa
```

Uma regra **nunca** muta estado: ela apenas propõe. Uma condição verdadeira
durante a avaliação não dispensa a validação antes de executar — entre as duas
coisas o boss pode ter morrido, mudado de fase ou perdido o alvo.

`Generation` invalida callback obsoleto: reset, cancelamento e substituição
incrementam o contador, e uma ação agendada só executa se a geração ainda bate.

## Escopo da v1

Deliberadamente pequeno. A especificação prevê 21 eventos, 19 condições e 15
ações; construir tudo isso antes do primeiro encontro existir seria adivinhar
requisito. Cada tipo novo deve entrar quando um encontro precisar.

| Categoria | Implementado |
|---|---|
| Eventos | `CombatStarted`, `Damaged`, `HealthThresholdCrossed`, `AddDied` |
| Condições | `PhaseIs`, `HPBelowOrEqual`, `ThresholdIs`, `AddsAliveEqual`, `NoPendingAction`, `ActionReady` |
| Ações | `ChangePhase`, `CastSkill`, `Summon`, `ApplyEffect`, `RemoveEffect` |

**Não existem** (removidos por não terem gerador nem caminho de configuração):
`CondPhaseIsNot`, `StopEvaluation`, `ActionSendMessage`.

## Os quatro tipos

Os tipos são **hardcoded em Go** (`boss_types.go`). O arquivo `.lua` só escolhe
o tipo pelo nome e fornece números — nenhuma regra nova pode nascer de conteúdo.

| Tipo | Comportamento | Exige |
|---|---|---|
| `chaser` | Persegue e ataca corpo a corpo, pela IA de mob comum. | — |
| `caster` | Lança as skills configuradas ao levar dano. | `skills` |
| `summoner` | Invoca grupos de adds durante a luta. | `summons` |
| `phased` | Troca de comportamento em limiares de HP. | `phases` |

O `type` do **topo** diz o que o boss PRECISA ter (validação de boot); o `type`
de cada **fase** diz o que ele USA naquele momento (restringe as reações
disponíveis). As mecânicas do roteiro — troca de fase, escudo, adds da fase —
ficam **sempre** habilitadas: elas contam a história do encontro e não podem
depender do comportamento vigente.

Os tipos são **geradores de regra**: compilam para a mesma estrutura de
regras/ações que o runtime executa. Não há um segundo motor paralelo.

## Prioridades

```text
95  troca de fase      -- nunca pode ser bloqueada
90  mecânica de fase   -- escudo sobe/cai, adds da fase
88  summon da fase     -- vira ação pendente, entra por último
50  reação de combate  -- skill e summon disparados por dano
```

Três bugs de ordenação apareceram durante a implementação e as prioridades acima
são a correção deles:

1. **`StopEvaluation` matava a cascata.** A regra de virada de fase o usava, o
   que abortava a avaliação e impedia escudo e summon — projetados para rodar
   *depois* dela, no mesmo evento.
2. **Summon pendente bloqueava a troca de fase.** Ação pendente
   não-interrompível trava as seguintes; uma mecânica de rotina travava a de
   maior prioridade do encontro. Toda ação gerada passou a ser interrompível.
3. **Summon de rotina bloqueava a queda do escudo** (mesma prioridade). Summon
   por dano foi reclassificado como *reação*; mecânicas de fase ficam acima.

Ordem dentro do evento de limiar: **troca de fase → escudo → summon da fase**.
O escudo é instantâneo e precisa ser aceito antes de qualquer ação pendente
bloqueá-lo; o summon tem `CastTime` e vira pendente, então vem por último.

## Escudo

`bossShieldAffect = 19` (a "Imunidade" nativa: já tem visual no client e não é
consumida por nenhuma regra de mob, que usam 1, 3, 10 e 12).

**A mitigação é real** (`bossMitigateDamage`), aplicada antes de subtrair o HP
nos dois caminhos de dano do jogador. Nenhum affect de mob concedia mitigação —
os quatro honrados são debuffs. Um escudo apenas decorativo faria o boss morrer
durante a fase em que deveria estar protegido.

`shield_until_adds_dead` exige que o boss tenha `summons`: o validador recusa a
combinação impossível, porque o escudo nunca cairia.

## HP não se recupera

**Decisão de gameplay (felipe):** o dano permanece onde os jogadores o
deixaram. Um chefe é feito para ter HP altíssimo e cair ao longo de horas,
possivelmente em várias sessões. Nada restaura vida — só a morte, que faz nascer
uma instância nova depois do `respawn_seconds`.

Não existe reset de encontro por abandono. Uma versão anterior tinha
`resetBossEncounter` com restauração de HP; foi removida por inviabilizar o
combate em várias sessões.

## Spawn e respawn

Bosses **não passam pelo NPCGener**. Cada um tem posição e respawn próprios,
declarados no seu `.lua`. Nenhum mob existente do mundo vira boss.

Do NPC do catálogo vem apenas os **assets** (rosto, equipamento, atributos
base), que o `.lua` pode sobrescrever em `stats`. O `carry` e o `vende` do NPC
base são zerados: os drops do boss saem da tabela própria (`drops`), e boss é
sempre hostil.

O `NPCDef` do boss é uma **cópia** do NPC base — alterar o boss não pode alterar
todos os mobs daquele NPC.

## Perseguição

O boss usa os **mesmos alcances** de qualquer mob (aquisição 4, retenção 7,
leash 16, ataque 1, intervalo 1,5 s). Não há o que configurar.

O leash de 16 é proposital: mais que isso e o chefe se afastaria demais do ponto
de origem. Uma versão anterior permitia sobrepor esses valores por instância
(`MobChase`); foi removida por adicionar regra própria sobre o que é portado do
TMSrv.

## Configuração

Formato completo, sandbox e campos em `data/boss/README.md`. Resumo:

- um `.lua` por encontro, lidos em ordem alfabética no boot;
- **erro derruba o boot** de propósito, com a mensagem apontando arquivo e
  campo;
- **campo desconhecido é erro**, não aviso — `atack = 500` nasceria um boss
  fraco sem ninguém entender por quê;
- sandbox sem `os`, `io`, `dofile`, `loadfile`, `load`, `require`, com limite de
  5 s por arquivo; `string`/`table`/`math` ficam, porque tornam o config
  expressivo (`max_hp = 9200 * 40` é avaliado de verdade).

Os quatro `data/boss/exemplo_*.lua` estão **ativos**, um de cada tipo.

## Arquivos

```text
internal/game/boss.go            runtime, registro e limpeza de adds
internal/game/boss_profile.go    tipos do perfil e Compile (ordenação/validação)
internal/game/boss_events.go     eventos, regras, intents e ação pendente
internal/game/boss_actions.go    execução (única parte que muta o mundo)
internal/game/boss_types.go      os quatro tipos -> geradores de regra
internal/game/boss_spawn.go      spawn, respawn e drops especiais
internal/data/boss.go            loader Lua + sandbox
internal/data/boss_lua.go        conversão Lua -> Go (estrita)
internal/model/boss.go           configuração declarativa + validação
```

## Dependência externa

O loader usa **gopher-lua** (`github.com/yuin/gopher-lua`, MIT — compatível com
a GPLv3 do projeto). É a **primeira e única** dependência externa do módulo:
antes dela o `go.mod` declarava apenas `go 1.26`. `go version -m` confirma que
nenhuma transitiva é linkada.

## Pendências

- Validação in-game no client 7.48: escudo, adds materializando e mensagens de
  fase.
- Os itens de drop dos exemplos foram conferidos no `itemlist.csv`, mas o
  balanceamento não foi avaliado.
- Eventos previstos na spec e ainda sem uso: timers, seletores de alvo, arena,
  leash próprio e reset de encontro.
