---
id: pickup-confirmation-contract
title: Confirmacao de coleta para Carry
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Confirmacao de coleta para Carry

## Pergunta

Qual frame confirma a coleta no client 7.48 e quais limites precisam ser
validados antes de atualizar o Carry e sua grade visual?

## Fronteira de evidência

- `UTILIZADA` — executavel historico `client748/wyd.exe nativo+patches/WYD.exe`
  identificado pelo hash do frontmatter e projeto Ghidra
  `WYD748Native_20260821.gpr`.
- `UTILIZADA` — descompilacao estudada de `FUN_00492E7D`, `FUN_00486009` e
  `FUN_0055890A`; o export `packet-size-gate-bodyrefs.tsv` confirma o mesmo
  programa e a estrutura do validador.
- `UTILIZADA` — source atual `MSG_CNFGetItem`, `TMFieldScene::OnPacketEvent`,
  `TMFieldScene::OnPacketCNFGetItem` e `ReceivedPacketDispatch`.
- `UTILIZADA` — WYD-Go `wire.CNFGetItem`, seu teste byte a byte e o emissor
  autoritativo de coleta em `internal/game/handlers.go`.
- `NAO APLICAVEL` — assets: o fluxo usa a grade Carry 9x7 ja materializada e
  nao cria ou altera recurso.
- `NAO APLICAVEL` — guia KR nao especifica este wire. Sources 7.54, W2PP,
  Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

O dispatcher de Field `FUN_00492E7D @ 0x00492E7D` compara `Header.Type` com
`0x171` e chama diretamente `FUN_00486009 @ 0x00486009`. A tabela de
`FUN_0055890A @ 0x0055890A` associa `0x171` somente ao tamanho `0x1C`.

### Callees

`FUN_00486009` copia oito bytes de `packet+0x14`. Se a habilidade 38 identifica
moeda, soma o valor ao Coin e atualiza os tres textos de dinheiro. Nos demais
itens, cria um controle de grade, usa `DestPos@0x10` nas coordenadas
`slot%9,slot/9` e copia o item ao Carry. Os callees relevantes sao
`FUN_0040D13E` para o controle, `FUN_0054CD07` para habilidades,
`FUN_0058F220` para as copias e `FUN_004431E4` para atualizar a ficha.

### Saidas e erros

- item comum: Carry e grade recebem o snapshot; som 45 e ficha sao atualizados;
- moeda: Coin e textos sao atualizados, sem ocupar celula;
- falha de alocacao: nenhuma grade/cache de item e materializada;
- frame com tamanho diferente: o contrato nativo o classifica como invalido;
- `DestPos` fora de `0..62`: o nativo nao protege a escrita, por isso a source
  compatível rejeita o indice antes de enderecar o array.

## Estado e lifecycle

| Evento | Precondicao | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- |
| receber `0x171` comum | frame 28B, Field ativa, slot `0..62` | snapshot aplicado ao Carry | visual, som e ficha | frame/slot invalido nao despacha/aplica |
| receber moeda | habilidade 38 igual a 2 | Coin incrementado | textos e som | controles opcionais podem estar ausentes |
| receber `0x182` seguinte | coleta confirmada pelo WYD-Go | slot recebe item autoritativo | substitui snapshot zerado do `0x171` | segue contrato SendItem |
| fechar/relogar | cena ou sessao termina | cache e controles pertencem aos owners existentes | teardown normal da cena | nenhum buffer e retido |

O transporte empresta o frame durante o callback. O gate copia apenas o header;
o handler cria sua propria copia de `STRUCT_ITEM`. A grade assume o controle
somente quando `AddItem` aceita; rejeicao libera controle e payload.

## Wire, ABI e recursos

Contrato S->C little-endian:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`, `Type=0x171` |
| `0x0C` | 4 | `DestType`, inteiro de 32 bits |
| `0x10` | 4 | `DestPos`, inteiro de 32 bits |
| `0x14` | 8 | `STRUCT_ITEM` |
| total | 28 | tamanho exigido por `FUN_0055890A` |

O recurso visual e a grade Carry nativa continuam 9x7, com 63 celulas. O slot
estrutural 63 do array de 64 entradas nao possui destino visual no 7.48.

## Mapeamento atual

`PickupConfirmationContract.h` nomeia opcode, tamanho, offsets e capacidade
visivel. `Basedef.h` mantem a struct concreta e seus asserts. O gate central
exige tamanho real/declarado igual a 28 e igualdade entre o opcode externo e
`Header.Type` antes do cast da cena. O handler rejeita item comum com slot fora
de `0..62`. O WYD-Go emite 28 bytes, com `Item` zerado, e envia logo depois o
`0x182` que materializa o item autoritativo no mesmo slot.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisao |
| --- | --- | --- | --- | --- |
| opcode/tamanho | `0x171/28` | literal sem gate exato | contrato nomeado e gate | `PARIDADE_NATIVA` |
| offsets | `12/16/20` | struct sem asserts locais | asserts e fixture | `PARIDADE_NATIVA` |
| slot visivel | grade 9x7 | indice usado antes da protecao | aceita somente `0..62` | `MODERNIZACAO_COMPATIVEL` |
| emissao Go | 28B + `0x182` | ja autoritativa | preservada | manter |

## Decisões

- Validar o frame na fronteira comum antes de qualquer callback legado.
- Preservar layout, ordem `0x171 -> 0x182`, efeitos e ownership existentes.
- Rejeitar apenas indices sem celula no Carry 7.48; nao alterar slots validos.
- Nao criar fallback, packet moderno ou recurso visual novo.

## Lacunas

- Exercitar coleta de item comum e inventario cheio no `project.exe`.
- Confirmar visualmente a sequencia `0x171/0x182`, som e relogin.
- A alcancabilidade historica completa de `FUN_0055890A` permanece na ficha
  separada do gate; o par opcode/tamanho e o consumidor estao decididos aqui.

## Validação

- Pesquisa: `FUN_00492E7D -> FUN_00486009` fecha o receptor; `FUN_0055890A`
  confirma `0x171/0x1C`; source e WYD-Go concordam nos offsets.
- Automacao: fixture cobre todos os prefixos, excesso, nulo, divergencias de
  Size/Type, entrega unica, preservacao dos bytes e cada slot `0..62`; o slot
  estrutural 63 e rejeitado. `go test -count=1 ./internal/wire` e
  `go vet ./internal/wire` passaram para o builder do servidor.
- Build Release passou pelo `Build-Client.ps1` com 24356 checks e asserts
  estaticos. XML e registro unico do header passaram. Candidato instalado:
  `5C14838866A943809B70D9D791D9A6FEBDCDC5575ECE83E9E02E981D5207F671`.
- Estado `AUTOMATED TESTED`; client real ainda nao executado, portanto nao e
  `CLIENT_TESTED`.
