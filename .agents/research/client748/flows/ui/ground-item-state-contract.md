---
id: ground-item-state-contract
title: Contrato de atualizacao e remocao de item do chao
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Atualizacao e remocao de item do chao

## Pergunta

Como o client 7.48 atualiza o estado de um objeto com `0x374` e encerra sua
presenca com `0x16F`, e quais bytes o WYD-Go deve emitir?

## Fronteira de evidência

- `UTILIZADA`: binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e decompilacao de `FUN_00492E7D`,
  `FUN_004862B6`, `FUN_004863E2` e `FUN_0055890A`.
- `UTILIZADA`: source atual `MSG_UpdateItem`, `MSG_STANDARDPARM`,
  `OnPacketUpdateItem`, `OnPacketRemoveItem` e `ReceivedPacketDispatch`.
- `UTILIZADA`: WYD-Go `wire.UpdateItem` e `wire.RemoveItem`, testes e fluxo de
  portoes e visibilidade.
- `NAO APLICAVEL`: assets e guia KR nao definem os envelopes. Sources 7.54,
  W2PP, Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D @ 0x00492E7D` encaminha `0x374` a
`FUN_004862B6 @ 0x004862B6` e `0x16F` a `FUN_004863E2 @ 0x004863E2`.
`FUN_0055890A @ 0x0055890A` exige respectivamente 20 e 16 bytes.

### Callees

No `0x374`, `FUN_004862B6` encontra o objeto por `ItemID@0x0C`, interpreta
`State` como short em `+0x10` e `Height` como char em `+0x12`, atualiza o
heightmap e o estado por `FUN_005554CC` e chama `FUN_004431E4` para a ficha.

No `0x16F`, `FUN_004863E2` limpa o item sob o mouse e chama virtualmente o
ObjectManager para destruir o objeto identificado pelo valor de 32 bits em
`+0x0C`. Nao existe corpo adicional nem resposta de rede.

## Estado e lifecycle

| Evento | Precondicao | Estado resultante | Side effects | Erro ou saida |
| --- | --- | --- | --- | --- |
| `0x374` | frame 20B, objeto existente e gate | estado e height atualizados | ficha atualizada | objeto ausente apenas atualiza ficha |
| `0x16F` | frame 16B | objeto removido do manager | mouse-over limpo | ID ausente e idempotente no manager |
| frame invalido | Size ou Type divergente | estado intacto | nenhum callback | gate rejeita |
| cena incompleta | manager ausente | estado intacto | nenhum | handler retorna antes do acesso |
| relogin | snapshot de visibilidade posterior | objetos rematerializados | owners normais da cena | nenhum frame retido |

O transporte empresta o frame apenas durante a chamada. `0x374` nao transfere
ownership. `0x16F` delega a destruicao ao ObjectManager, owner do objeto.

## Wire, ABI e recursos

`0x374`, S->C, 20 bytes:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD` |
| `0x0C` | 4 | `ItemID` |
| `0x10` | 2 | `State`, short |
| `0x12` | 1 | `Height`, char |
| `0x13` | 1 | reservado |

`0x16F`, S->C, 16 bytes: `MSG_STANDARD` seguido por `ItemID` de 32 bits em
`0x0C`. Nenhum recurso visual novo participa.

## Mapeamento atual

`GroundItemStateContract.h` nomeia os dois opcodes, tamanhos e offsets.
`Basedef.h` fixa o layout de `MSG_UpdateItem`. O gate central valida tamanho
real e declarado e Type igual ao opcode antes dos casts. Os dois handlers
validam mensagem e ObjectManager. O encoder Go de `UpdateItem` agora grava
somente `State` u16 em `+0x10`, mantendo Height e reserva zerados; antes usava
`putU32`, permitindo que bits altos contaminassem esses bytes. `RemoveItem` ja
emitia os 16 bytes corretos.

## Matriz de delta

| Claim | Nativo 7.48 | Source e Go antes | Estado atual | Decisao |
| --- | --- | --- | --- | --- |
| update | `0x374/20`, state i16, height i8 | sem gate; Go escrevia u32 | gate, asserts e u16 | `PARIDADE_NATIVA` |
| remove | `0x16F/16`, ID em 12 | sem gate exato | gate e fixture | `PARIDADE_NATIVA` |
| manager nulo | lifecycle pressuposto | acesso direto | guarda local | `MODERNIZACAO_COMPATIVEL` |

## Decisões

- Preservar efeitos e ownership dos handlers para frames validos.
- Corrigir o encoder Go para o tail realmente consumido pelo 7.48.
- Rejeitar envelopes invalidos antes da cena e manager ausente no handler.
- Nao alterar estado de gameplay, recurso, vtable ou semantica de portao.

## Lacunas

- Executar abertura e fechamento de portao e remocao por visibilidade no client.
- Confirmar troca de regiao e relogin com objetos atualizados.
- O comportamento para State fora do enum ativo continua definido pelo servidor.

## Validação

- Pesquisa: consumidores e tamanhos nativos confirmados nas quatro funcoes.
- Automacao: fixtures cobrem truncamento, excesso, Size e Type, offsets e
  preservacao dos frames. O teste Go confirma State u16 e Height/reserva zero.
  `go test -count=1 ./...` e `go vet ./...` passaram.
- Build Release passou pelo `Build-Client.ps1` com 24682 checks e asserts
  estaticos. XML e registro unico do header passaram. Candidato instalado:
  `A04E28EC82F087891A4FCA1BE97BFE0AC6652FDA896764B943485366154D31E9`.
- Estado `AUTOMATED TESTED`; client real ainda nao executado, portanto nao e
  `CLIENT_TESTED`.
