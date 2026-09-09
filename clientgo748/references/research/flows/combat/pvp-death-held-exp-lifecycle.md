---
id: pvp-death-held-exp-lifecycle
title: Divida Held EXP criada por morte PvP e paga por combate
subsystem: combat-progression
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-03
---

# Divida Held EXP criada por morte PvP e paga por combate

## Pergunta

Como a morte PvP cria e publica a divida Held EXP, como a EXP de combate a
amortiza e como o estado sobrevive a rollback, logout e relogin sem se
confundir com CP/Chaos?

## Fronteira de evidência

- Executavel/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; o consumo visual de
  Hold/FakeExp e seus controles esta fechado em
  `flows/ui/character-stat-fields-update.md`.
- Source atual: `TMHuman.cpp`, `TMFieldScene.cpp` e `Basedef.h`.
- Servidor: `internal/model/model.go`, `internal/game/cp.go`,
  `internal/game/progression.go`, `internal/game/kill_rewards.go`,
  `internal/game/visibility.go`, `internal/game/equipment.go`,
  `internal/wire/codec.go` e `internal/wire/source_client.go`.
- Referencia server-side complementar: curvas W2PP/7.5x para divisores de
  morte e amortizacao. Elas nao substituem o contrato wire recuperado do 7.48.

## Fluxo nativo 7.48

### Entrada observável

- Evento: um jogador morre para outro jogador.
- Precondicoes: atacante e vitima validos, ataque PvP autorizado e HP da
  vitima reduzido a zero pelo servidor.
- Saida observavel: a vitima morre sem perder imediatamente EXP ou gold; Hold
  aumenta, aparece no painel Character e a EXP de combates posteriores paga a
  divida antes de voltar a progredir.

### Callers

O client recebe o snapshot inicial por `MSG_CNFCharacterLogin`, atualizacoes
incrementais por `MSG_UpdateEtc` e confirmacoes de morte por `MSG_CNFMobKill`.
Os caminhos que convergem em `FUN_004431E4` atualizam os controles Character
depois de Score, equipamento e skill; os callers estao registrados na ficha de
UI citada acima.

No WYD-Go, o fechamento de uma morte PvP converge em `applyPvPKills`, que
adiciona a divida antes da persistencia. Mortes de mob convergem em
`planMobKill`, que chama `grantCombatExp` para cada recebedor da partilha.

### Funcao principal

No client, `TMHuman::OnPacketUpdateEtc` copia `MSG_UpdateEtc::Hold` para
`ObjectManager::m_nFakeExp`; o login copia `Ext1.Data[0]`; e a confirmacao de
morte transporta o mesmo Hold/FakeExp em byte 12. `TMFieldScene` apresenta o
valor, o percentual e o limiar visual sem tratar esse campo como CP.

No servidor, `addHeldExperienceDeathDebt` cria a divida e
`grantCombatExp` a paga. O limite e 10% do intervalo cumulativo do nivel atual.
A parcela de cada morte usa o divisor da faixa de nivel e nunca excede a EXP
atual nem o limite. Ao atingir 80% do limite, somente o `RuntimeScore.MaxHP`
efetivo e dividido por dois.

### Callees

- `currentExperienceInterval` recupera `next[level+1] - next[level]`.
- `heldExperienceDeathDivisor` seleciona o divisor por faixa de nivel.
- `heldExperienceDeathDebt` limita a nova parcela pela EXP atual.
- `heldExperienceLimit` e `saturatingAddU32` impedem overflow e excesso.
- `heldExperiencePenaltyActive` decide o limiar de 80%.
- `recalcPlayer` reaplica ou remove a reducao de MaxHP.
- `wire.UpdateEtc`, `wire.CNFMobKill` e `wire.EnterWorld` publicam o estado.

### Saidas e erros

- Hold abaixo do limite: acumula normalmente.
- Mortes repetidas: saturam em 10% do intervalo, sem overflow.
- EXP menor que a parcela: a divida criada nao supera a EXP existente.
- Recompensa de combate menor que Hold: reduz somente Hold.
- Recompensa de combate maior que Hold: zera Hold e aplica apenas o restante
  em EXP.
- Quest e item: chamam `grantExp` e nao amortizam Hold.
- Falha de persistencia PvP: restaura `Char`, `Score` e `RuntimeScore`; nenhum
  estado parcial e publicado como commit.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondicao | Funcao/call | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- | --- |
| morte PvP | vitima viva e ataque autorizado | `applyPvPKills -> addHeldExperienceDeathDebt -> recalcPlayer` | Hold acrescido e limitado | CP e atualizado por sua regra separada | rollback integral se o save falhar |
| Hold cruza 80% | limite do nivel maior que zero | `recalcPlayer -> heldExperiencePenaltyActive` | `RuntimeScore.MaxHP / 2` | HP corrente e limitado ao novo teto | Score persistente nao e mutilado |
| kill de mob | recebedor elegivel | `planMobKill -> grantCombatExp` | Hold pago primeiro | restante aumenta EXP/level | snapshot por recebedor evita EXP/Hold alheio |
| Hold cai abaixo de 80% | recompensa suficiente | `grantCombatExp -> recalcPlayer` | MaxHP normal restaurado | `UpdateScore` acompanha a troca do limiar | sem bonus retroativo de HP |
| quest/item | recompensa nao-combate | `grantExp` | EXP aumenta; Hold permanece | fluxo original da recompensa | nao paga divida por engano |
| entrada/relogin | Char persistido | `EnterWorld` | `Ext1.Data[0]` restaura Hold | UI recalcula valor e visibilidade | zero significa ausencia de divida |

### Vtables, vptrs e receptores

O receptor client-side e o `TMHuman` local para `UpdateEtc`; a FieldScene le o
estado do `ObjectManager`. Esta adaptacao nao importa vptrs ou offsets de
objetos nativos. Os offsets abaixo pertencem somente aos frames wire
confirmados.

### Ownership

`model.Char.Hold` e o estado duravel e autoritativo. `RuntimeScore` e uma
projecao recalculavel, nao uma segunda fonte de verdade. No client,
`m_nFakeExp` e a copia de apresentacao. CP permanece em `Char.CP` e no byte
legado de Chaos; nenhum caminho copia CP para Hold.

### Falha parcial

A mutacao PvP captura snapshots antes de alterar CP, Hold e score. Se a
persistencia atomica falhar, os personagens sao restaurados antes de qualquer
publicacao de sucesso. No ganho por mob, os snapshots do plano restauram
progressao e score caso o commit falhe; o pacote final usa o estado efetivamente
confirmado.

### Cleanup e teardown

Hold nao possui timer, goroutine, callback nem heap proprio. O teardown da
FieldScene libera apenas os controles que exibem a copia client-side. Encerrar
a cena nao apaga o valor persistido.

### Shutdown

O shutdown usa a persistencia normal da conta/personagem. Nenhuma fila
especial de Hold permanece ativa e nenhuma referencia de UI sobrevive a
destruicao da cena.

### Logout e relogin

No logout, o `Char.Hold` ja persistido permanece na conta. No relogin,
`EnterWorld` escreve o valor em `Ext1.Data[0]@1264`; o client o copia para
`m_nFakeExp` e recalcula `1168/1376`. Hold zero oculta o ornamento; Hold
positivo o exibe. CP e reconstruido pela fronteira propria e nao interfere.

## Wire, ABI e recursos

| Frame | Tamanho | Campo Hold | Demais campos relevantes |
| --- | ---: | --- | --- |
| `0x337 UpdateEtc` | 36 | `uint32 @12` | EXP `@16`, LearnedSkill `@20`, pontos `@24..30`, gold `@32` |
| `0x338 CNFMobKill` | 24 | Hold/FakeExp `uint32 @12` | Killed/Killer `@16/@18`, EXP `uint32 @20` |
| `0x114 EnterWorld` source-built | 2104 | `Ext1.Data[0] uint32 @1264` | snapshot completo do personagem |

O recurso 7.48 usa `1168` para o ornamento HOLD e `1376` para o texto
`valor / percentual`. O denominador e 10% do intervalo do nivel atual. O texto
fica vermelho a partir de 80%, o mesmo limiar da penalidade de MaxHP.

## Mapeamento atual

### Source recompilavel

- `TMHuman.cpp` trata `Hold` como divida de EXP, sem alias com Chaos.
- `TMFieldScene.cpp` calcula o limite como `(next - current) / 10`, exibe o
  percentual e controla a visibilidade de `1168`.
- O login source-built consome `Ext1.Data[0]` no mesmo estado local.

### WYD-Go

- `Char.Hold uint32` e persistente em JSON/PostgreSQL por meio do modelo.
- Morte PvP cria divida sem retirar EXP ou gold.
- EXP de combate paga Hold antes da progressao; quest/item nao paga.
- O cruzamento do limiar recalcula e publica MaxHP.
- Pacotes de morte carregam EXP/Hold proprios de cada destinatario.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go anterior | Decisao |
| --- | --- | --- | --- | --- |
| significado de Hold | divida EXP/FakeExp | UI existia, comentario confundia o campo | campo zerado/ausente | `PARIDADE_NATIVA` |
| CP/Chaos | estado PK separado | podia ser interpretado como C.POINT | vazava conceitualmente para Hold | separar os estados |
| morte PvP | publica Hold sem perda imediata de EXP/gold | consumidor presente | sem divida | criar e persistir Held EXP |
| ganho de combate | paga Hold antes da EXP | UI recebe novo snapshot | toda EXP ia para progressao | amortizar primeiro |
| limiar 80% | texto vermelho e penalidade MaxHP | apresentacao parcial | sem penalidade | reduzir somente MaxHP efetivo |

## Decisões

- Classificacao: `PARIDADE_NATIVA` na fronteira wire/client e adaptacao
  server-authoritative da regra correspondente.
- Manter CP/Chaos e Held EXP em campos, persistencia e publicacoes distintos.
- Nao retirar EXP nem gold no instante da morte PvP.
- Nao permitir que recompensas de quest/item paguem a divida de combate.
- Aplicar a penalidade somente na projecao `RuntimeScore.MaxHP` para que o
  recalc restaure o valor normal assim que o limiar deixar de valer.

## Lacunas

- Validar com dois clients reais: morte PvP, Hold crescente, ausencia de perda
  imediata de EXP/gold e CP independente.
- Matar mobs ate pagar parcialmente e totalmente a divida, confirmando texto,
  cor, EXP e restauracao de MaxHP ao cruzar o limiar.
- Repetir depois de logout/relogin e de uma falha de persistencia controlada.
- Ate esses testes, nao promover para `CLIENT_TESTED`.

## Validação

- Pesquisa: contrato wire, controles, ownership, erro, teardown, shutdown e
  relogin registrados nesta ficha e na ficha Character.
- Automacao: testes cobrem divisores/limite, saturacao, pagamento parcial e
  total, isolamento de quest/item, penalidade/restauracao de MaxHP,
  persistencia, rollback e frames wire. `go test -count=1 ./...`,
  `validate_research.py` e `git diff --check` passaram.
- Client source: `Build-Client.ps1` concluiu `Release|Win32` com zero erros e
  13 warnings C4018 e instalou `client748/project.exe` com SHA-256
  `ADFA0B99C17F96367F05E6718E61512039AB50CA54CBDDB104333EF663210795`.
- Client real: nao executado; estado maximo `IMPLEMENTED / STATICALLY VERIFIED`,
  sem claim `CLIENT_TESTED`.
