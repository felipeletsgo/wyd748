---
id: attack-frame-envelope
title: Envelope variável da família de ataque
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Envelope variável da família de ataque

## Pergunta

Quais comprimentos podem cruzar a família de ataque nos opcodes `0x39D`,
`0x39E` e `0x36C`, tanto nas intenções C->S do TMProject quanto nos resultados
S->C publicados pelo WYD-Go?

## Fronteira de evidência

- UTILIZADA: `flows/combat/skill-visual-dispatch.md`, que identifica
  `FUN_00489A3E @ 0x00489A3E` como o processamento nativo de ataque.
- UTILIZADA: `flows/transport/missing-entity-request.md`, que liga o dispatcher
  nativo ao mesmo consumidor e registra a leitura do atacante.
- UTILIZADA: `flows/ui/cc-auto-combat-state-sync.md`, que registra os prefixos
  nativos de um, dois e treze alvos.
- UTILIZADA: `TMFieldScene::OnPacketEvent`, `OnPacketAttack`, os três structs
  `MSG_Attack*` e o parser de dano amplo da source recompilável.
- UTILIZADA: `wire.AttackHitWideResult`, `wire.AttackHitWideFlankResult`,
  `wire.AttackHitsWideTwoResult`, `wire.SkillHits` e seus testes no WYD-Go.
- UTILIZADA: a allowlist C->S de `internal/game/security.go` e os emissores
  ativos de `MSG_Attack` em `SGrid`, `TMFieldScene` e `TMHuman`.
- NÃO APLICÁVEL: assets e recursos visuais; o corte só valida o envelope antes
  do handler já existente.
- INDISPONÍVEL NESTA RETOMADA: nova sessão do Ghidra; nenhuma claim nativa
  adicional foi criada além das fichas já versionadas.

## Fluxo nativo 7.48

### Entrada observável

`0x39D`, `0x39E` e `0x36C` descrevem ataques com capacidade nativa para um,
dois e treze alvos. Seus prefixos medem respectivamente 48, 52 e 96 bytes e
compartilham os campos até a lista de dano em `+44`.

### Callers

O dispatcher da Field entrega a família ao processamento nativo
`FUN_00489A3E @ 0x00489A3E`. Na source recompilável,
`TMFieldScene::OnPacketEvent` seleciona os mesmos três opcodes e chama
`OnPacketAttack`.

### Função principal

`OnPacketAttack` converte imediatamente o buffer para `MSG_Attack`, lê
`AttackerID`, posição, skill e a primeira entrada de dano, e então materializa
o ataque no ator ou pede a entidade ausente. Antes deste corte, o dispatcher
central exigia apenas o header de 12 bytes; `Header.Size` podia declarar um
frame maior que a view real.

### Callees

- O lookup do atacante pode emitir `0x369/16` quando a entidade não existe.
- O parser de dano amplo reconhece o DWORD físico de `0x39D/52` e a assinatura
  `DMGX` das extensões de skill.
- Os handlers visuais usam alvo, dano e skill sem possuir o comprimento real da
  view como uma segunda fronteira confiável.

### Saídas e erros

Somente os comprimentos publicados pelo contrato são entregues. Buffer nulo,
prefixo truncado, tamanho intermediário, excesso, `Header.Size` divergente ou
conflito entre `Header.Type` e opcode externo são rejeitados antes do callback.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| prefixo nativo | tamanho permitido para o opcode | `OnPacketAttack` | ataque visual enfileirado | animação, dano e MP visual | entrega única |
| dano amplo | cauda física ou `DMGX` completa | parser amplo existente | dano uint32 preservado | efeito visual equivalente | fallback permanece no parser |
| frame inválido | qualquer divergência de envelope | `ReceivedPacketDispatch` | estado preservado | nenhum callback | retorna `false` |

### Vtables, vptrs e receptores

O receptor continua sendo a `TMFieldScene` e os `TMHuman` localizados pelo
ObjectManager. Não há alteração de vtable, slot virtual ou tipo concreto.

### Ownership

O transporte empresta o buffer somente durante o callback. O gate inspeciona
uma cópia do header, não retém nem modifica o payload.

### Falha parcial

A validação termina antes da primeira leitura tipada. Assim, um header forjado
não pode iniciar lookup, animação, consumo visual de MP ou leitura da cauda
fora do buffer real.

### Cleanup e teardown

O contrato não cria alocação, timer ou objeto; efeitos já aceitos continuam sob
o cleanup existente da cena.

### Shutdown

N/A para o gate. Disconnect encerra a cena sem deixar buffer pendente criado
por esta validação.

### Logout e relogin

Cada FieldScene nova recebe ataques novos e aplica o mesmo contrato. Nenhum
frame é conservado pelo gate entre personagens ou sessões.

## Wire, ABI e recursos

Campos compartilhados pelos três prefixos:

| Campo | Offset | Largura | Semântica |
| --- | ---: | ---: | --- |
| `MSG_STANDARD` | 0 | 12 | header e discriminantes |
| `AttackerID/Progress` | 12 | 4 | ator e sequência do hit |
| `PosX/PosY` | 16 | 4 | origem |
| `TargetX/TargetY` | 20 | 4 | destino |
| `SkillIndex/CurrentMp` | 24 | 4 | skill e MP projetado |
| flags visuais | 28 | 4 | motion, parâmetro, local e crítico |
| `CurrentExp/ReqMp/Rsv/FakeExp` | 32 | 12 | estado projetado |
| `Dam[]` | 44 | 4 por alvo | alvo e dano nativo |

Matriz de resultados S->C aceitos:

| Opcode | Prefixo nativo | Extensões coordenadas |
| --- | ---: | --- |
| `0x39D` | 48 | 52 físico; 60 `DMGX` de um alvo |
| `0x39E` | 52 | 64 ou 68 `DMGX` para um/dois alvos |
| `0x36C` | 96 | 108..156, passo 4, para um a treze alvos |

`AttackFrameContract.h` centraliza opcodes, offsets, capacidades e a função de
validação. Não há packing, asset ou opcode novo.

Matriz de intenções C->S aceitas pelo peer ativo:

| Opcode | Comprimentos | Observação |
| --- | --- | --- |
| `0x39D` | 48 ou 96 | 48 é o prefixo de um alvo; 96 é o envelope legado observado e explicitamente permitido pelo servidor |
| `0x39E` | 52 | dois alvos |
| `0x36C` | 96 | até treze alvos |

`0x39D/72` não pertence ao contrato. O ramo de skill de alvo único em
`SGrid::OnRButtonDown` selecionava `nSize = sizeof(MSG_AttackOne)` (48), mas
ignorava essa decisão ao chamar `SendOneMessage` com o literal 72. O WYD-Go
rejeitava o frame antes do handler de combate.

## Mapeamento atual

### Source recompilável

`ReceivedPacketDispatch::CanDispatch` reconhece a família antes da tabela de
tamanho fixo e exige igualdade entre opcode externo, `Header.Type`, tamanho
declarado e tamanho real. Depois aplica `IsAttackPacketSize` ao par opcode e
comprimento.

Na saída C->S, os emissores enviam o tamanho correspondente ao opcode.
`SendOneMessage` aplica `IsClientToServerAttackPacketSize` antes do socket e
rejeita inclusive o antigo `0x39D/72`; o ramo de alvo único do `SGrid` agora
transmite o `nSize` já calculado.

### WYD-Go

`AttackHitWideResult` publica `0x39D/52`. `SkillHits` conserva os prefixos
nativos quando não há alvo e usa `DMGX`, contagem e DWORDs de dano nas caudas.
Os builders de dois alvos e flanco publicam `0x39E/64` ou `68`; o multi publica
`0x36C/108..156` alinhado por alvo.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| prefixos e offsets | 48/52/96, dano em `+44` | structs compatíveis | mesmos prefixos | manter |
| dano uint32 amplo | não alegado como nativo | parser `DMGX` existente | builders ativos | extensão coordenada |
| entrada S->C | consumidor tipado comprovado | só header mínimo | comprimentos fechados | validar antes do cast |
| saída C->S de alvo único | prefixo de 48 bytes | montava 48, enviava 72 | aceita 48 ou legado 96 | enviar 48 e rejeitar 72 |
| autoridade | client reproduz ataque | efeitos visuais | servidor calcula/publica | preservar |

## Decisões

- Classificar prefixos, offsets e capacidades como `PARIDADE_NATIVA` com
  evidência já versionada.
- Classificar as caudas de dano amplo como `EXTENSAO_COORDENADA`; não atribuir
  `DMGX` ao binário nativo.
- Classificar o novo guard como `MODERNIZACAO_COMPATIVEL`, sem alegar que
  reproduz o caller ainda não resolvido do gate nativo.
- Classificar a restauração de `0x39D/48` na saída C->S como
  `PARIDADE_NATIVA`; o guard geral da matriz C->S é
  `MODERNIZACAO_COMPATIVEL`.
- Não reduzir a família a um único tamanho nem aceitar comprimentos apenas por
  estarem entre mínimo e máximo.

## Lacunas

- Falta exercitar no `project.exe` real ataques físicos e skills com um, dois e
  treze alvos, incluindo dano acima de 65535.
- O caller/direção de `FUN_0055890A` permanece não resolvido e não é usado como
  prova do comportamento de rejeição deste gate.

## Validação

- Pesquisa: fichas nativas existentes e produtores/consumidores ativos foram
  cruzados; nenhuma nova inferência de Ghidra foi necessária.
- Automação: teste C++ percorre todos os comprimentos de 0 a 157 para cada
  opcode, além de nulo, discriminantes divergentes, mismatch real/declarado,
  entrega única, offsets e imutabilidade. A matriz C->S percorre 0..160 e
  comprova os conjuntos `0x39D/{48,96}`, `0x39E/52`, `0x36C/96` e a rejeição
  explícita de `0x39D/72`. Testes Go focados cobrem os builders.
- Build: Release executou 51.678 checks/asserts, instalou o candidato e
  verificou SHA-256
  `D58D3B9616D484B1E46592DB8B4D1B601C49E80935136FAFCC88DFF6A0807FDB`.
- Client real: pendente; build e testes não promovem o fluxo a `CLIENT_TESTED`.
