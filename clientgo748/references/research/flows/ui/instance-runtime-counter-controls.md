---
id: instance-runtime-counter-controls
title: Contadores runtime de tempo e objetivos de instancia
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-08
---

# Contadores runtime de tempo e objetivos de instancia

## Pergunta

Como o caminho compatível 7.48 deve materializar os textos atualizados pelos
packets de tempo e objetivos enviados pelo WYD-Go?

## Fronteira de evidência

- `UTILIZADA`: binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_00492E7D`,
  `FUN_00489192` e `FUN_00489260`.
- `UTILIZADA`: source atual `InitializeScene`, `InitializeCompatFieldScene`,
  `FrameMove`, handlers de contagem e ownership do `SControlContainer`.
- `UTILIZADA`: WYD-Go, que emite `OpInstanceTime=0x3A1` e
  `OpInstanceMobs=0x3B0` em Big Cube, Uxmal e item instances (incluindo
  Cube, Big Cube, Nightmare, Hell Gate e salas Water).
- `NAO APLICAVEL`: os textos sao objetos runtime e nao exigem novo asset.
- `NAO APLICAVEL`: guia KR nao define os controles internos. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D @ 0x00492E7D` encaminha `0x3A1` para
`FUN_00489192 @ 0x00489192` e `0x3B0` para
`FUN_00489260 @ 0x00489260`.

### Callees

`FUN_00489192` guarda o valor recebido, captura o tempo atual, formata o texto
e o torna visivel. `FUN_00489260` formata a quantidade restante, captura o
tempo atual e mostra o segundo texto. Ambos chamam `FUN_0058F078` para formatar
e depois metodos virtuais dos controles. A criacao nativa exata desses objetos
e seu teardown ainda nao foram localizados; por isso a ficha permanece
`LOCATED`.

## Estado e lifecycle

No caminho completo da source, tres `SText` sao alocados em runtime e entregues
ao `SControlContainer`: tempo, quantidade restante e tempo de quest. O caminho
compatível retornava antes dessas alocacoes, deixando os membros nulos. Um
`0x3A1` ou `0x3B0` emitido pelo WYD-Go causava acesso nulo; `FrameMove` tambem
podia acessar os membros sem packet quando o estado de dungeon mudava.

O helper compartilhado agora cria os mesmos objetos para ambos os caminhos,
mantem estado inicial, fontes, cores e coordenadas e transfere ownership ao
container. Falha parcial deixa cada ponteiro nulo; handlers, tick e quest
ignoram apenas o controle ausente e preservam o restante do estado.

## Correção do fluxo multi-instância

O contador e o cronômetro agora são dirigidos pelo contrato de packet, não por
coordenadas hardcoded do terreno. O `0x3A1` inicia o cronômetro para qualquer
instância e `Parm == 0` o oculta; o `0x3B0` mantém `Monsters N` visível até que
o servidor envie zero. O mesmo lifecycle é aplicado ao contador `0x3BB` de
objetivos com formato `current / max`. Isso cobre transições, quizzes,
respawn-until-timeout e salas alternativas que compartilham os emissores do
WYD-Go, sem depender de `RenderDevice::m_bDungeon` ou da lista histórica de
offsets.

## Wire, ABI e recursos

Esta mudanca nao altera wire. Os emissores Go continuam usando
`MSG_STANDARDPARM` para `0x3A1` e `0x3B0`. Os objetos sao runtime e usam
`ControlID=-2`; nenhum ID serializado ou novo recurso foi criado.

## Mapeamento atual

`InitializeRuntimeCounterTexts` e o unico construtor dos tres textos na source.
`InitializeScene` e `InitializeCompatFieldScene` chamam o mesmo helper.
`OnPacketStartTime`, `OnPacketRemainCount`, `OnPacketRemainNPCCount`, os dois
handlers Quest12, `SetQuestStatus`, `UpdateQuestTime` e `FrameMove` toleram
falha individual de alocacao. O container continua owner dos controles aceitos.

## Matriz de delta

| Claim | Nativo 7.48 | Source antes | Source atual | Decisao |
| --- | --- | --- | --- | --- |
| handlers `3A1/3B0` | atualizam textos | implementados | preservados | manter |
| criacao no compat | raiz exata pendente | pulada pelo retorno antecipado | helper compartilhado | `MODERNIZACAO_COMPATIVEL` |
| falha parcial | pendente | dereference nulo | consumidores tolerantes | proteger |
| assets/wire | sem novo recurso | existentes | inalterados | manter |

## Decisões

- Materializar os controles runtime necessários no caminho compatível.
- Compartilhar uma unica implementacao com o inicializador completo.
- Preservar os packets e todos os efeitos quando os controles existem.
- Nao atribuir a geometria atual ao nativo sem localizar seu construtor.
- Tratar `0x3A1`, `0x3B0` e `0x3BB` como sinais de estado do servidor; a
  coordenada do mapa não pode desligar uma UI já autorizada pelo packet.

## Lacunas

- Localizar no Ghidra a alocacao e o teardown exatos dos dois textos nativos.
- Testar Big Cube, Uxmal e item instance no `project.exe` em varias resolucoes.
- Confirmar esconder, expirar, trocar de regiao e relogar sem texto residual.

## Validação

- Pesquisa: dispatch e efeitos dos dois handlers confirmados; criador nativo
  permanece pendente, logo nao ha promocao para `TRACED`.
- Build Release passou pelo `Build-Client.ps1` com 26968 checks e asserts
  estaticos. O gate de transporte agora exige 16 bytes, Type/opcode
  correspondentes e valor em `+12` para `0x3A1`/`0x3B0`; os testes cobrem
  truncamento, excesso, nulo, divergencias, entrega unica e preservacao de
  bytes. `go test ./... -count=1` e `git diff --check` passaram. Candidato:
  `840D3E2B124B732640F9556F023CFC16920F8F86D7EDA6ACD5EE98A697BA17E2`.
- Estado `STATICALLY VERIFIED`; fluxo real ainda nao executado, portanto nao e
  `CLIENT_TESTED`.
