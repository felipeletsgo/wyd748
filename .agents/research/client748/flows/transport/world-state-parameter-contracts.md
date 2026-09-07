---
id: world-state-parameter-contracts
title: Contratos de saldo do Cargo e remocao de entidade
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Saldo do Cargo e remocao de entidade

## Pergunta

Quais envelopes o client 7.48 consome para atualizar o gold armazenado e para
retirar ou matar uma entidade, e como impedir que frames incompletos alcancem
os handlers vivos usados pelo WYD-Go?

## Fronteira de evidência

- `UTILIZADA`: binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_00492E7D`,
  `FUN_00485664`, `FUN_0052EAA9`, `FUN_00529BF8` e `FUN_0055890A`.
- `UTILIZADA`: source atual `OnPacketUpdateCargoCoin`,
  `TMHuman::OnPacketRemoveMob`, `MSG_STANDARDPARM`, `MSG_RemoveMob` e
  `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.UpdateCargoGold`, `wire.RemoveMob`, seus emissores
  em Cargo, AutoTrade, visibilidade, morte e encerramento de sessao, e testes.
- `NAO APLICAVEL`: assets e guia KR nao definem estes envelopes. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D @ 0x00492E7D` encaminha `0x339` a
`FUN_00485664 @ 0x00485664`. O dispatch de entidades
`FUN_0052EAA9 @ 0x0052EAA9` encaminha `0x165` a
`FUN_00529BF8 @ 0x00529BF8`. `FUN_0055890A @ 0x0055890A` exige 16 bytes para
ambos os opcodes.

### Callees

`FUN_00485664` copia o DWORD de `+0x0C` para o saldo global do Cargo e chama
`FUN_004431E4` para atualizar a UI. `FUN_00529BF8` consome o tipo de remocao em
`+0x0C`, fecha estados contextuais ligados ao alvo e seleciona saida de visao,
morte, efeito ou remocao conforme o estado da entidade. O `Header.ID` escolhe
o `TMHuman` receptor antes desse consumo.

## Estado e lifecycle

| Evento | Receptor | Estado e efeitos | Falha |
| --- | --- | --- | --- |
| `0x339` valido | FieldScene | substitui saldo autoritativo e atualiza textos | frame invalido nao chama a cena |
| `0x165` valido | entidade indicada por `Header.ID` | executa lifecycle existente de remocao/morte | frame invalido nao percorre os filhos |
| logout/relogin | nova cena e novo snapshot | saldo e entidades sao rematerializados pelo servidor | nenhum frame e retido |

O transporte empresta o mesmo buffer somente durante o callback. Nenhum dos
dois packets transfere ownership. Este lote nao muda as transicoes dos
handlers; apenas fecha seu envelope antes dos casts.

## Wire, ABI e recursos

Ambos sao S->C e possuem 16 bytes:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`; `ID@6` seleciona cena ou entidade |
| `0x0C` | 4 | saldo u32 em `0x339`; `RemoveType` i32 em `0x165` |

Nao ha recurso novo. `WorldStateParameterContract.h` nomeia opcodes, tamanho e
offset. `Basedef.h` fixa `MSG_STANDARDPARM` e `MSG_RemoveMob` com asserts. O
gate exige tamanho real e declarado, alem de igualdade entre opcode externo e
`Header.Type`.

## Mapeamento atual

`ReceivedPacketDispatch::ExpectedSize` cobre os dois opcodes antes da travessia
legada. `TMFieldScene` usa o nome do contrato para `0x339` e `TMHuman` usa o
nome correspondente para `0x165`; os corpos dos handlers permanecem iguais.
No WYD-Go, os builders continuam sendo a unica origem dos frames e agora tem
fixtures byte a byte para os dois layouts.

## Matriz de delta

| Claim | Nativo 7.48 | Source antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| saldo Cargo | `0x339/16`, valor em `+12` | handler vivo sem tamanho no gate | contrato, asserts e fixture | `PARIDADE_NATIVA` |
| remover mob | `0x165/16`, tipo em `+12` | handler vivo sem tamanho no gate | contrato, asserts e fixture | `PARIDADE_NATIVA` |
| efeitos validos | handlers nativos distintos | implementacao existente | inalterados | manter |

## Decisões

- Rejeitar os dois envelopes invalidos antes da arvore de cena/entidades.
- Preservar `Header.ID`, o DWORD e todos os bytes para frames validos.
- Nao alterar regras de Cargo, visibilidade, morte, efeitos ou teardown.
- Manter fora deste lote opcodes nao emitidos pelo WYD-Go atual.

## Lacunas

- Executar deposito, saque e venda em AutoTrade no `project.exe`.
- Executar saida de visao, morte de mob/player e relogin com dois clients.
- Registrar separadamente cada fluxo real antes de classifica-lo
  `CLIENT_TESTED`.

## Validação

- Pesquisa: dispatch, consumidores, tamanhos e offsets nativos confirmados.
- Automacao: fixtures C++ cobrem truncamento, excesso, nulo, divergencias, ID,
  valor, entrega unica e preservacao para os dois opcodes. O teste Go confere
  o builder `0x339`; `go test -count=1 ./...` passou.
- Build Release passou pelo `Build-Client.ps1` com 24788 checks e asserts
  estaticos. XML, registro unico do header, `git diff --check` e validador de
  pesquisa passaram. Candidato instalado:
  `3D12125879994155078D5D0503339880C146F2D09CAE3553EA680FAC47808813`.
- Estado `AUTOMATED TESTED` / `STATICALLY VERIFIED`; client real ainda nao
  executado, portanto nao e `CLIENT_TESTED`.
