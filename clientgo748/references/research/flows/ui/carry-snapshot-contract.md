---
id: carry-snapshot-contract
title: Contrato do snapshot completo do Carry
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Snapshot completo do Carry

## Pergunta

Como o client 7.48 recebe `0x185`, substitui o inventario interativo e preserva
o array estrutural de 64 itens quando somente 63 slots possuem celula visual?

## Fronteira de evidência

- `UTILIZADA`: binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_0052EAA9`,
  `FUN_0052E3C8`, `FUN_0040D13E` e `FUN_0055890A`.
- `UTILIZADA`: source atual `MSG_Carry`, `TMHuman::OnPacketCarry`, grade 9x7,
  ownership de `SGridControlItem` e `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.UpdateCarry`, `model.MaxCarry`, emissores de
  login, comandos, compra, trade, AutoTrade, quest e composicoes, e testes.
- `NAO APLICAVEL`: o guia KR nao define o wire; nenhum asset novo e necessario.
  Sources 7.54, W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_0052EAA9 @ 0x0052EAA9` encaminha `0x185` a
`FUN_0052E3C8 @ 0x0052E3C8` para o personagem cujo ID casa com `Header.ID`.
`FUN_0055890A @ 0x0055890A` exige `0x210`/528 bytes.

### Callees

`FUN_0052E3C8` esvazia as 63 celulas da grade, copia `0x200`/512 bytes de itens
desde `+0x0C`, materializa indices `>40` por `FUN_0040D13E` em
`x=slot%9, y=slot/9`, copia Coin de `+0x20C` e atualiza ficha/estado da UI. A
iteracao visual termina em 63; a copia logica conserva os 64 `STRUCT_ITEM`.

## Estado e lifecycle

O source atual aceita o snapshot somente no `TMHuman` local e em FieldScene
com grade de inventario materializada. `Empty()` elimina as representacoes
anteriores e seus aliases de cursor; depois os 512 bytes substituem o Carry
autoritativo. Cada item visivel alocado transfere ownership apenas quando
`AddItem` aceita a celula. Falha de alocacao deixa o snapshot logico intacto e
omite somente o visual correspondente. Coin e ficha sao atualizados no fim.

O slot 63 atravessa wire, cache e persistencia, mas nao e desenhado nem vira
destino de input. O frame e emprestado somente durante o callback. Relogin e
troca de mapa recebem outro snapshot e repetem a mesma reconstrucao.

## Wire, ABI e recursos

`0x185`, S->C, 528 bytes:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` seleciona o personagem |
| `0x0C` | 512 | `STRUCT_ITEM Carry[64]`, oito bytes cada |
| `0x20C` | 4 | Coin u32 |

`CarrySnapshotContract.h` nomeia opcode, tamanho, offsets e capacidades.
`Basedef.h` fixa tamanho, array e Coin por asserts. O gate valida tamanho real
e declarado e Type/opcode antes da travessia de entidades. O recurso existente
continua sendo a grade 9x7; nenhum controle e criado.

## Mapeamento atual

`TMHuman::OnPacketCarry` preserva a formula nativa de slot e o limite visual
0..62. `wire.UpdateCarry` grava no maximo `model.MaxCarry` itens, mantendo Coin
intacto mesmo se um caller fornecer uma slice maior; slices menores deixam o
restante zerado. Todos os emissores ativos continuam usando o mesmo builder.

## Matriz de delta

| Claim | Nativo 7.48 | Source/Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| envelope | `0x185/528` | 528B sem gate | gate e asserts | `PARIDADE_NATIVA` |
| array | 64 itens copiados | 64 estruturais | preservado | manter |
| UI | slots 0..62, 9x7 | mesma projecao | preservada | manter |
| slice Go maior | N/A | podia ultrapassar buffer/panicar | limitada a 64 | `MODERNIZACAO_COMPATIVEL` |

## Decisões

- Preservar exatamente o envelope nativo de 528 bytes e seus 64 itens.
- Manter o slot estrutural 63 fora da grade visual.
- Rejeitar frames invalidos antes do rebuild destrutivo da grade.
- Limitar o builder Go sem reduzir o array nem alterar callers validos.

## Lacunas

- Executar login, compra, trade, AutoTrade, quest e composicao no candidato.
- Testar snapshot durante drag e inventario cheio, incluindo troca de mapa e
  relogin, antes de classificar o fluxo como `CLIENT_TESTED`.

## Validação

- Pesquisa: dispatch, consumidor, tamanho, offsets, capacidade e formula visual
  confirmados.
- Automacao: a fixture C++ cobre 528 prefixos truncados, excesso, nulo,
  Type/Size divergentes, receptor, slots 0/62/63, Coin, entrega unica e
  preservacao. O teste Go usa 65 entradas e confirma limite estrutural sem
  sobrescrever Coin; `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 25614 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `39117672AAA8DD939CFB2B503344932195E4B179F5812AB9D28AE8F2E990FA6D`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; o fluxo ainda nao e
  `CLIENT_TESTED`.
