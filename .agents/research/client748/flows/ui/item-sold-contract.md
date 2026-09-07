---
id: item-sold-contract
title: Contrato da notificacao de anuncio vendido
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Anuncio de AutoTrade vendido

## Pergunta

Como o client 7.48 recebe `0x39B` e remove um anuncio vendido da loja fantasma
sem deslocar os dois parametros ou executar o callback em frame incompleto?

## Fronteira de evidência

- `UTILIZADA`: binario nativo 7.48, projeto Ghidra e decompilacao de
  `FUN_00492E7D`, `FUN_004854ED` e `FUN_0055890A`.
- `UTILIZADA`: source atual `OnPacketItemSold`, `MSG_STANDARDPARM2`, grids de
  AutoTrade e `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.ItemSold`, `publishGhostShopItemSold` e testes.
- `NAO APLICAVEL`: assets e guia KR nao definem o envelope. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

`FUN_00492E7D` encaminha `0x39B` a `FUN_004854ED`, que exige o clone em
`Parm1@12`, a posicao do anuncio em `Parm2@16` e remove o item da grade quando
o clone coincide com a loja fantasma ativa. `FUN_0055890A` exige 20 bytes.

## Estado e lifecycle

O source atual valida painel, alvo e faixa da grade antes de retirar o visual;
cursor e aliases sao limpos e a loja permanece aberta para os anuncios
restantes. O frame nao transfere ownership. Reentrada ou relogin recria a lista
via `0x397/0x39A` e um novo snapshot.

## Wire, ABI e recursos

`0x39B`, S->C, 20 bytes: Header 12B, `TradeEntityID` u32 em `+12` e posicao
u32 em `+16`. `ItemSoldContract.h` nomeia opcode, tamanho e offsets; asserts e
o gate central validam o envelope antes do callback.

## Mapeamento atual

`TMFieldScene` usa a constante no dispatch. `wire.ItemSold` escreve os dois
DWORDs e usa `SceneField` no Header.ID, enquanto o clone permanece em Parm1.

## Decisões e lacunas

- Preservar o envelope nativo e a ordem de remocao da grade.
- Rejeitar frames curtos, excedentes ou com Type/Size divergentes.
- Executar venda de AutoTrade com dois clients, repeticao, fechamento e relogin
  no `project.exe`; ainda nao e `CLIENT_TESTED`.

## Validacao

- Pesquisa: caller, consumidor, offsets, tamanho e lifecycle confirmados.
- Automacao: fixture C++ e teste Go cobrem entidade/posicao, truncamento,
  excesso, nulo, divergencias, entrega unica e preservacao; `go test -count=1
  ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 26815 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `4929DED5CAEBCDFE1474E50C7CD775B39B0260965201D9374EE1DDA52C0B5570`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; ainda nao e
  `CLIENT_TESTED`.
