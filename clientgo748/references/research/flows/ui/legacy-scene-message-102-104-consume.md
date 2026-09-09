---
id: legacy-scene-message-102-104-consume
title: Consumo opaco das mensagens de cena 0x102 e 0x104
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Consumo opaco das mensagens de cena 0x102 e 0x104

## Pergunta

O que o client 7.48 faz ao receber `0x102` e `0x104`, quais tamanhos aceita e
qual comportamento a source recompilável deve preservar sem inventar campos ou
emissores para payloads que o nativo não interpreta?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`, referência histórica
  somente leitura identificada pelo SHA-256 do frontmatter.
- Handler nativo: `FUN_0049889A @ 0x0049889A`, reconstruído no corpus
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Gate de tamanho: `FUN_0055890A @ 0x0055890A` no mesmo corpus.
- Callgraph focado:
  `.agents/research/client748/exports/field-scene-rebuild-server-move-focused.tsv`.
- Source atual: `TMScene::OnPacketEvent`, `CPSock::ReadMessage` e os contratos
  de packet em `Basedef.h`.
- Servidor atual: não há builder, handler nem emissor vivo de `0x102/0x104` no
  WYD-Go.
- Assets: nenhum recurso, texto ou controle é acessado por estes dois branches.

## Fluxo nativo 7.48

### Entrada observável

O transporte recebe um frame `S->C`. `FUN_0055890A` admite `0x102` somente
com 116 bytes (`0x74`) e `0x104` somente com 152 bytes (`0x98`). Quando o
dispatcher chama `FUN_0049889A`, a família de mensagens da cena é reconhecida
somente com `Header.ID == 0`.

### Callers

O export focado confirma quatro callers diretos de `FUN_0049889A`:

- `FUN_004339D8`;
- `FUN_00492E7D`;
- `FUN_004A626E`;
- `FUN_004AE3D6`.

Há também uma referência de dados em `0x005A4440`, correspondente à entrada do
receptor virtual. Esses caminhos convergem no mesmo handler de cena; nenhum
deles atribui semântica adicional ao payload de `0x102/0x104`.

### Função principal

`FUN_0049889A` compara `Header.ID` e `Header.Type`. Para `ID=0`, reconhece
`0x101`, `0x102` e `0x104`. Em seguida, qualquer tipo diferente de `0x101`
retorna `1` imediatamente. Assim, `0x102` e `0x104` são consumidos
deliberadamente sem ler bytes depois do header.

### Callees

Não há callee executado no branch específico de `0x102/0x104`: o retorno
ocorre antes do processamento textual de `0x101`, antes do encaminhamento ao
container de controles e antes do fallback para a árvore. Os demais callees
listados para `FUN_0049889A` pertencem a outros opcodes e não fazem parte desta
transição.

### Saídas e erros

- frame canônico, `ID=0`: retorna `1`, sem mutação observável;
- tamanho diferente do canônico: é recusado pelo gate antes do handler;
- `ID!=0`: não entra nesta família e segue o dispatch normal;
- não existe ACK, texto, controle, callback, persistência ou resposta de erro.

## Estado e lifecycle

### Matriz de transições

| Evento | Precondição | Função | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| receber `0x102` | `ID=0`, 116 bytes | gate -> `FUN_0049889A` | inalterado | nenhum | retorna `1` |
| receber `0x104` | `ID=0`, 152 bytes | gate -> `FUN_0049889A` | inalterado | nenhum | retorna `1` |
| tamanho inválido | opcode reconhecido | `FUN_0055890A` | inalterado | nenhum | não despacha |
| `ID!=0` | qualquer tamanho admitido pelo transporte | dispatcher | fora deste fluxo | depende de outro receptor | não aplicável |

### Vtables, vptrs e receptores

A referência `0x005A4440` liga o handler ao receptor virtual da cena. O branch
não resolve outro vptr, não encaminha ao control container e não seleciona
receptor por entidade.

### Ownership

O buffer pertence ao transporte durante a chamada. O handler não retém, copia,
aloca nem libera o frame e não assume ownership do payload.

### Falha parcial

Não há mutação antes do retorno; recusa por tamanho ou consumo válido deixam a
cena e o gameplay inalterados. Logo não existe rollback.

### Cleanup e teardown

Não aplicável: nenhum objeto, timer, controle, inscrição ou memória é criado.

### Shutdown

Não aplicável: não existe estado próprio a destruir nem ordem de shutdown a
preservar.

### Logout e relogin

Não aplicável: nenhum byte é persistido e nenhum estado sobrevive à chamada;
logout/relogin não requer reset nem reconstrução para estes opcodes.

## Wire, ABI e recursos

Ambos os frames usam `MSG_STANDARD` de 12 bytes, little-endian, com direção
observada `S->C` e `Header.ID=0` para esta transição:

| Opcode | Offset | Tamanho | Campo |
| --- | ---: | ---: | --- |
| `0x102` | `0x00` | 12 | `MSG_STANDARD`, `Size=116`, `Type=0x102`, `ID=0` |
| `0x102` | `0x0C` | 104 | payload opaco, nunca lido pelo handler |
| `0x104` | `0x00` | 12 | `MSG_STANDARD`, `Size=152`, `Type=0x104`, `ID=0` |
| `0x104` | `0x0C` | 140 | payload opaco, nunca lido pelo handler |

Packing e signedness internos do payload não são inferidos: como nenhum campo
é lido, atribuir nomes ou tipos seria hipótese sem consumidor observável. Não
há recurso ou asset associado.

## Mapeamento atual

### Source recompilável

`Basedef.h` nomeia os dois opcodes, representa os corpos como bytes opacos e
usa `static_assert` para congelar 116/152 bytes. `TMScene::OnPacketEvent`
mantém `ID=0`, valida os tamanhos canônicos e consome os dois tipos sem ler o
payload nem encaminhá-los a controles.

### WYD-Go

Não existe feature viva que emita estes frames. O servidor permanece sem
builder ou handler: conhecer um contrato nativo de consumo não justifica
inventar payload nem ativar tráfego sem efeito funcional.

## Matriz de delta

| Claim | Nativo 7.48 | Source antes | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| opcodes | `0x102/0x104` explícitos | números mágicos | nomes explícitos | `PARIDADE_NATIVA` |
| tamanhos | 116/152 exatos | não expressos | structs e asserts canônicos | `PARIDADE_NATIVA` |
| payload | não lido | não lido | opaco, não lido | manter |
| dispatch | `ID=0`, retorna `1` | `ID=0`, retorna `1` | preservado com gate local | manter |
| servidor | direção observada `S->C` | sem emissor | sem emissor | não implementar |

## Decisões

- Classificar o lote como `PARIDADE_NATIVA`: a source já possuía o consumo e
  agora expressa o contrato nativo sem alterar sua semântica.
- Preservar o retorno silencioso e não interpretar o payload por analogia com
  versões posteriores.
- Não criar builders, handlers ou emissão no WYD-Go até existir uma feature
  real com semântica comprovada ou um contrato coordenado novo.
- Consumir silenciosamente variantes malformadas no receptor recompilável para
  impedir fallthrough a controles não relacionados; o efeito continua sendo
  nenhuma mutação observável.

## Lacunas

- A semântica que um servidor histórico atribuía aos bytes opacos permanece
  desconhecida e desnecessária para o comportamento observado deste client.
- Capturar `0x102/0x104` reais permitiria confirmar origem e frequência, mas
  não é requisito para o contrato de consumo já fechado.
- Executar frames canônicos e malformados no `client748/project.exe` antes de
  qualquer claim `CLIENT_TESTED`.

## Validação

- Pesquisa: gate, callers, receptor virtual, branch sem callee, ownership e
  lifecycle foram fechados no hash registrado.
- ABI: `static_assert` torna os tamanhos 116/152 verificáveis no build Win32.
- Pesquisa automatizada: `validate_research.py --repo .` passou com
  `CONTRACT=14`, `TRACED=6`, `LOCATED=4` e `UNMAPPED=2`.
- Build: `Build-Client.ps1` passou em `Release|Win32` com zero erros e 31
  warnings preexistentes; instalou `client748/project.exe` com SHA-256
  `06598F434D36BC52CEDBEA63B8C2248DC8072B8857D4A08B0B92568A886940CB`.
- Higiene: `git diff --check` passou e não restou número mágico de opcode
  `0x102/0x104` no handler da cena.
- Client real: não executado; nenhuma alegação `CLIENT_TESTED` é feita.
