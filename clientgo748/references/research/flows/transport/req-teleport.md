---
id: req-teleport
title: Pedido de portal 0x290
subsystem: transport-session
status: CONTRACT
native_sha256: B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C
updated: 2026-09-08
---

# Pedido de portal 0x290

## Pergunta

Qual frame o client 7.48 envia ao confirmar um portal e qual estado local
permite o envio?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYDoriginal.exe`, SHA-256
  do frontmatter; o executável stock é a referência primária.
- Projeto/corpus Ghidra: `WYD748Stock_20260908`; export focado
  `C:\Users\felipe\Tools\GhidraAnalysis\20260908-stock-teleport\stock-candidate.tsv`.
- Assets: `NÃO APLICÁVEL`; o frame contém somente header e um DWORD reservado.
- Source atual: `TMFieldScene.cpp`, `Basedef.h` e
  `internal/wire/ReqTeleportPacket.h`.
- Servidor: `internal/game/teleports.go`, `security.go`, `world.go` e testes
  de teleporte.
- TMProject posterior/guias: `UTILIZADA` somente como implementação candidata
  secundária; não define endereço, ABI ou comportamento nativo. W2PP, Secrets
  e Micronics foram excluídos da decisão.

## Fluxo nativo 7.48

O Ghidra confirmou no stock `FUN_004776C3` o caminho de entrada no frame de
campo: leitura do atributo por `FUN_00433A60`, tratamento do bit `0x10` e
chamadas de UI/envio próximas a `FUN_0055F2DD`. A source compatível estava
retornando antes desse slice; `UpdateTeleportPrompt` agora o executa antes do
retorno compacto e o caminho completo reutiliza o mesmo helper.

### Entrada observável

Ao confirmar a caixa de portal (`message == 0x10`), `FUN_004640E5` consulta a
atribuição da posição atual por `FUN_00433A60`. Se o bit `0x10` está presente,
zera 16 bytes, grava o ID local, `Type=0x290`, deixa o DWORD em `+12` zerado e
envia por `FUN_0055F2DD`. Sem o atributo de portal, mostra a mensagem de erro e
não transmite frame.

### Callers

`FUN_004640E5` é a entrada nativa do evento de confirmação da caixa de
mensagem; o argumento `0x10` corresponde ao `case 16` de
`TMFieldScene::OnMsgBoxEvent` na source atual. A rotina participa do despacho
de controles da FieldScene e retorna após ocultar a caixa.

### Callees

- `FUN_00433A60` lê o atributo da célula atual e decide se o portal é válido.
- `FUN_0055F2DD` aplica o throttle e envia o buffer de 16 bytes.
- `FUN_0055890A` aceita `0x290` somente com tamanho `0x10`.

## Estado e lifecycle

| Evento/estado | Pré-condição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| confirmar portal | caixa `0x10`, atributo `0x10` | `FUN_004640E5 -> FUN_0055F2DD` | servidor recebe intenção | caixa fecha; timer local segue o fluxo existente | sem atributo não envia |
| servidor recebe | sessão autenticada, frame 16B | `World.onReqTeleport` | rota autoritativa é resolvida | valida posição, gold, alcance e persistência | rejeição mostra aviso, sem mutação parcial |
| desconexão | transporte falha | teardown geral | sessão encerra | nenhum estado próprio do packet | sem retry do frame |
| logout/relogin | nova FieldScene | lifecycle geral | cooldown e posição são reconstruídos | request antigo não é repetido | servidor permanece autoridade |

O buffer nativo vive na pilha e é emprestado ao transporte. O frame não leva
destino, preço, gold ou item; `teleports.ini`, terrain, cobrança, persistência e
publicação continuam no servidor.

## Wire, ABI e recursos

Direção C->S, opcode `0x290`, exatamente 16 bytes:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | envelope WYD 7.48 |
| `+12` | `Reserved` | `int32` | zero no emissor nativo; não é destino |

`Header.ID` identifica o personagem local. Não existe resposta específica do
client; o servidor envia apenas os efeitos normais do teleporte, como estado
monetário e aviso, depois de persistir com sucesso.

## Mapeamento atual

`TMFieldScene::OnMsgBoxEvent` já reproduzia o branch nativo, mas usava o literal
decimal `656` e a fachada `MSG_STANDARDPARM`. Agora usa
`MSG_ReqTeleport_Opcode` e `MSG_ReqTeleport`, preservando a consulta de
`BASE_GetAttr`, o erro, o fechamento da caixa e o valor reservado zero.
O WYD-Go já exige 16 bytes e resolve a rota a partir da posição autoritativa;
nenhum destino fornecido pelo client é aceito.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire | `0x290/16`, reservado zero em `+12` | equivalente, antes literal decimal | gate/handler equivalentes | extrair tipo dedicado |
| precondição | atributo `0x10` | `BASE_GetAttr` e `case 16` preservados | portal por bloco 4x4 e terrain | manter |
| autoridade | client envia intenção | não envia destino | servidor resolve/cobra/persiste | preservar |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT`.
- Criar `ReqTeleportPacket.h` com opcode, frame de 16 bytes e asserts.
- Substituir apenas o tipo/literal do `case 16`, sem alterar o fluxo de
  mensagem, timers ou servidor.
- Não adicionar rota ao `ReceivedPacketDispatch`, pois `0x290` é C->S.

## Lacunas

- Confirmação real do portal, recusa fora do atributo, cobrança, troca de cena
  e relogin ainda não foram executados no `client748/project.exe`; não é
  `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra read-only usou hash nativo esperado; export registra branch,
  precondição, campos, envio e gate de tamanho.
- Automação: fixture C++ verifica opcode, ID, tamanho, offset e reservado zero;
  `go test -count=1 ./...`, XML/caminhos e `git diff --check` passaram. Debug e
  Release via `Build-Client.ps1 -Rebuild` passaram com 1934 checks/asserts; o
  Release instalado tem SHA-256
  `20E080DC44D40F5620B9A649D77976F29A85BD3357F7A9B459DC4FFBE33192BD`.
- Client real: ainda não executado.

## Hunting Scroll / AirMove control dispatch

The 7.48 resources define `TMB_POTAL_BTN=12546` as Confirm and
`TMB_POTAL_BTN2=12547` as Cancel.  `SGrid` opens this list for volatile item
types 195 and records the selected destination in `MSG_UseItem.ItemID`; the
server consumes opcode `0x373` only after validating the 36-byte frame and a
destination index from 1 through 10.

The source dispatch had the confirmation branch guarded by
`idwControlID != 12546`.  Consequently the Confirm control skipped the branch,
while the Cancel path was unreachable whenever the list existed.  The guard is
now `idwControlID == 12546 && m_pPotalList`; Cancel remains the explicit
`12547` branch.  This is a source control-flow correction, not a new wire
contract: the existing `0x373`/36-byte packet and destination offset are
unchanged.

Evidence provenance: native 7.48/Ghidra remains primary for the FieldScene
control lifecycle; resource IDs and the current source establish the local
control contract.  TMProject is used only as a secondary semantic comparison.
W2PP, Secrets and Micronics are excluded.

Validation status: static/source correction complete; client build and manual
Confirm/Cancel plus destination execution remain pending, so this is not yet
`CLIENT_TESTED`.
