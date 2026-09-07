---
id: update-equip-visual-contract
title: Contrato do snapshot visual de equipamento
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Snapshot visual de equipamento

## Pergunta

Como o client 7.48 recebe `0x36B` e atualiza a aparencia sem transportar itens
autoritativos nem reintroduzir Necklace/Belt ausentes da UI do emulador?

## Fronteira de evidência

- `UTILIZADA`: binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_0052EAA9`,
  `FUN_0052B116` e `FUN_0055890A`.
- `UTILIZADA`: source atual `MSG_UpdateEquip`,
  `TMHuman::OnPacketUpdateEquip`, `SetPacketEquipItem`, `SetColorItem` e
  `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.SelfEquip`, `VisualEquip`, `bodyMesh`,
  `bodyAncient`, `clientEquipProjection`, emissores e testes.
- `NAO APLICAVEL`: o guia KR nao define o wire; nenhum asset novo participa.
  Sources 7.54, W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_0052EAA9 @ 0x0052EAA9` encaminha `0x36B` a
`FUN_0052B116 @ 0x0052B116` para o personagem indicado por `Header.ID`.
`FUN_0055890A @ 0x0055890A` exige `0x3C`/60 bytes.

### Callees

`FUN_0052B116` le os 16 WORDs desde `+0x0C`, atualiza rosto, armas, montaria,
mesh e ficha e usa o bloco de 16 bytes em `+0x2C` para cor/AnctCode. O handler
aciona efeitos de troca e reconstrucao do objeto, mas nao altera o inventario
nem transfere ownership de item.

## Estado e lifecycle

No source atual, o handler atualiza `TMHuman` do owner ou observador, aplica os
16 visuais, trata a montaria importada, aplica cores, recalcula altura/armas e
atualiza a ficha. O frame e emprestado apenas durante o callback. Efeitos
criados sao entregues ao container da cena; relogin ou reentrada na visao
rematerializam a aparencia pelo snapshot seguinte.

O servidor projeta a posicao 9 como zero em `bodyMesh`, `bodyAncient` e
`clientEquipProjection`; posicoes 16/17 nem cabem no frame de 16 entradas.
Assim este lote protege o envelope visual sem modificar a correcao ja publicada
que remove Necklace/Belt/NewSlot da UI e dos destinos equipaveis.

## Wire, ABI e recursos

`0x36B`, S->C, 60 bytes:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` seleciona o personagem |
| `0x0C` | 32 | `sEquip[16]`, WORD visual por posicao |
| `0x2C` | 16 | `Equip2[16]`, AnctCode/cor por posicao |

`UpdateEquipContract.h` nomeia opcode, tamanho, offsets e capacidades.
`Basedef.h` fixa os dois arrays por asserts. O gate valida tamanho real e
declarado e Type/opcode antes do cast. Nenhum recurso de UI ou mesh e criado por
esta alteracao.

## Mapeamento atual

`TMHuman::OnPacketUpdateEquip` usa a constante do contrato no dispatch. No
WYD-Go, `SelfEquip` deriva os arrays do equipamento e `VisualEquip` limita cada
bloco a 16 entradas. A projecao do emulador zera slots sem suporte antes do
builder; o client conserva os bytes recebidos sem reinterpretar o wire.

## Matriz de delta

| Claim | Nativo 7.48 | Source/Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| envelope | `0x36B/60` | 60B sem gate | gate e asserts | `PARIDADE_NATIVA` |
| visual | 16 WORD em `+12` | mesmo layout | preservado | manter |
| cor | 16 bytes em `+44` | mesmo layout | preservado | manter |
| Necklace/Belt | fora da projecao solicitada | zerados no servidor | inalterado | manter correcao publicada |

## Decisões

- Preservar o contrato nativo de 60 bytes e os dois arrays paralelos.
- Rejeitar frame invalido antes de efeitos e reconstrucao do avatar.
- Manter o servidor responsavel por projetar slots sem suporte como vazios.
- Nao reabrir a UI ou as regras de Necklace/Belt neste lote.

## Lacunas

- Executar equip/refino/cor/capa/montaria com owner e observador.
- Confirmar troca durante movimento, morte, reentrada na visao e relogin no
  `project.exe` antes de classificar como `CLIENT_TESTED`.

## Validação

- Pesquisa: dispatch, consumidor, tamanho, offsets, arrays e projecao do
  servidor confirmados.
- Automacao: a fixture C++ cobre 60 prefixos truncados, excesso, nulo,
  Type/Size divergentes, receptor, limites dos dois arrays, entrega unica e
  preservacao. O teste Go confere o primeiro item usado e a posicao 15 nos
  blocos visual/AnctCode; `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 25685 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `50AF940F6A1FBF8D7E1AE34603A94CAC8DCBB328F09C6FB8D9825CF7443A7655`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; o fluxo ainda nao e
  `CLIENT_TESTED`.
