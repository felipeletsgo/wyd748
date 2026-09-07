---
id: combine-complete-contract
title: Contrato do sinal de conclusao de ItemMix
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Conclusao de ItemMix

## Pergunta

Como o client recebe `0x3A7` e fecha o painel ItemMix ativo sem aceitar frame
incompleto ou misturar os seis roots nativos?

## Fronteira de evidência

- `UTILIZADA`: binario nativo 7.48, projeto Ghidra e decompilacao de
  `FUN_00492E7D` e `FUN_004890F9`.
- `UTILIZADA`: source atual `OnPacketCombineComplete`, os seis helpers
  `SetVisibleNativeMix` e `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.StandardParm(OpCombineComplete,...)`,
  `sendCombineResult` e testes.
- `NAO APLICAVEL`: assets e guia KR nao definem envelope. Sources 7.54, W2PP,
  Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

`FUN_00492E7D` encaminha `0x3A7` a `FUN_004890F9`, que oculta os cinco roots
ItemMix comuns. O tamanho do frame nao aparece como literal em
`FUN_0055890A`; o contrato source/WYD-Go usa `MSG_STANDARDPARM` de 16 bytes,
com resultado DWORD em `+12`, e o gate e classificado como modernizacao
compativel baseada no consumidor e no encoder.

## Estado e lifecycle

O source atual fecha somente o painel nativo compatível visível, preservando os
demais roots e evitando que um reconhecimento de um artesão afete outro. A
mensagem textual de resultado chega antes do sinal e permanece independente do
fechamento. Nenhum ownership de payload e retido; reabertura cria novo estado.

## Wire, ABI e recursos

`0x3A7`, S->C, 16 bytes no contrato coordenado: Header 12B e resultado u32 em
`+12`. `CombineCompleteContract.h`, asserts e `ReceivedPacketDispatch` fixam o
envelope e exigem Type/opcode coerentes. Nenhum recurso novo participa.

## Mapeamento atual

`TMFieldScene` usa a constante no dispatch; `wire.StandardParm` monta o frame e
o servidor envia resultado 0/1/2 conforme a decisao persistida. Os seis roots
continuam compartilhados pelos helpers existentes.

## Matriz de delta

| Claim | Nativo 7.48 | Source/Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| callback | `FUN_004890F9` oculta ItemMix | handler existente | preservado | manter |
| envelope | tamanho nao literal no validador | 16B StandardParm | gate e asserts | `MODERNIZACAO_COMPATIVEL` |
| resultado | DWORD convencional | mesmo builder | preservado | manter |

## Decisões e lacunas

- Rejeitar frames curtos, longos ou divergentes antes de fechar qualquer root.
- Preservar o resultado e o fechamento seletivo do painel ativo.
- Executar os seis artesãos, resultado 0/1/2, Cancel/Esc e relogin no
  `project.exe`; ainda nao e `CLIENT_TESTED`.

## Validação

- Pesquisa: caller, consumidor e lifecycle dos roots confirmados.
- Automacao: fixture C++ e teste Go cobrem truncamento, excesso, nulo,
  divergencias, receptor, resultado, entrega unica e preservacao; `go test
  -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 26841 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `549FDF4F6A46A8E0C537C4F429E37A71747A4CF58741C3D5BC366026AF09403D`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; ainda nao e
  `CLIENT_TESTED`.
