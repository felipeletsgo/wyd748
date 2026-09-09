---
id: drop-confirmation-contract
title: Confirmacao de remocao ao jogar item no chao
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Confirmacao de remocao ao jogar item no chao

## Pergunta

Qual frame `0x175` remove o item local depois do drop e quais origens/indices
podem ser aplicados sem ultrapassar Equip, Carry ou Cargo?

## Fronteira de evidência

- `UTILIZADA` — binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e descompilacao estudada de `FUN_00492E7D`,
  `FUN_00485D43` e `FUN_0055890A`.
- `UTILIZADA` — source atual `MSG_CNFDropItem`, `OnPacketCNFDropItem`, helpers
  de Carry/Cargo e `ReceivedPacketDispatch`.
- `UTILIZADA` — WYD-Go `wire.CNFDropItem`, teste byte a byte e fluxo
  autoritativo `onDropItem`.
- `NAO APLICAVEL` — assets: nenhuma grade ou imagem nova e carregada.
- `NAO APLICAVEL` — guia KR nao define o wire. Sources 7.54, W2PP, Secrets e
  Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D @ 0x00492E7D` compara o opcode recebido com `0x175` e chama
diretamente `FUN_00485D43 @ 0x00485D43`. O validador `FUN_0055890A` associa
`0x175` ao tamanho `0x1C`.

### Callees

`FUN_00485D43` escolhe Equip quando `SourType=0`, Carry quando `1` e Cargo
quando `2`. Retira o controle da celula, zera oito bytes do armazenamento,
solta o cursor, destroi o visual retirado e atualiza familiar, ficha e humano.
Os callees diretos relevantes sao `FUN_004431E4` para a ficha e
`FUN_00480A83` para atualizar o humano; as chamadas virtuais resolvem as grids.

O nativo calcula os enderecos com `SourPos@0x10` sem validar o indice. A source
mantem a semantica para entradas validas e rejeita valores fora das capacidades
do produto 7.48 antes da primeira consulta ou escrita.

## Estado e lifecycle

| Evento | Precondicao | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- |
| confirmar Equip | tipo 0, slot valido e suportado | Equip zerado | visual/familiar/ficha atualizados | Necklace/NewSlot rejeitados no modo compatível |
| confirmar Carry | tipo 1, slot `0..62` | Carry zerado | visual destacado e destruido | slot 63 ou maior rejeitado |
| confirmar Cargo | tipo 2, slot `0..119` | Cargo zerado | visual destacado e destruido | reserva `120..127` rejeitada |
| origem desconhecida | tipo fora de `0..2` | estado intacto | nenhum | retorno imediato |
| fechar/relogar | confirmacao consumida | snapshot seguinte rematerializa grids | teardown normal | buffer nao e retido |

O frame e emprestado somente durante o callback. `PickupItem/PickupAtItem`
devolve ownership do visual ao handler, que o destroi depois de soltar o cursor.
Uma confirmacao invalida nao adquire nem libera ownership.

## Wire, ABI e recursos

Contrato S->C little-endian:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`, `Type=0x175`, ID zero no emissor Go |
| `0x0C` | 4 | `SourType` |
| `0x10` | 4 | `SourPos` |
| `0x14` | 4 | `Rotate` |
| `0x18` | 2 | `GridX` |
| `0x1A` | 2 | `GridY` |
| total | 28 | tamanho exigido por `FUN_0055890A` |

Carry possui 63 celulas visiveis. Cargo possui 128 entradas estruturais, das
quais 120 sao utilizaveis pelo protocolo e UI ativos. O array Equip permanece
com 18 entradas na source, mas os slots 9, 16 e 17 nao possuem alvo no modo
compatível do emulador.

## Mapeamento atual

`DropConfirmationContract.h` nomeia opcode, tamanho, offsets e limites.
`Basedef.h` mantem a struct e fixa cada offset por `static_assert`.
`ReceivedPacketDispatch` exige frame real/declarado de 28 bytes e Type igual ao
opcode externo. `OnPacketCNFDropItem` valida tipo/slot antes de indexar a grid ou
o armazenamento. O WYD-Go valida a intencao, persiste a remocao, publica o item
no chao e envia `0x175` seguido de `0x182` com o slot autoritativo vazio.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisao |
| --- | --- | --- | --- | --- |
| opcode/tamanho | `0x175/28` | literal, sem gate exato | contrato e gate | `PARIDADE_NATIVA` |
| offsets | `12/16/20/24/26` | struct sem asserts locais | asserts e fixture | `PARIDADE_NATIVA` |
| indices | usados sem bounds no nativo | usados antes de validar | limites por origem | `MODERNIZACAO_COMPATIVEL` |
| slots ausentes | n/a no produto alvo | podiam ser indexados | bloqueados no compat | manter decisao do ecossistema |

## Decisões

- Validar envelope antes do cast e origem/slot antes da mutacao.
- Preservar os tres ramos nativos para frames validos e o ownership existente.
- Manter o servidor autoritativo e a ordem `0x175 -> 0x182`.
- Nao alterar recurso, packing, vtable ou packet do servidor.

## Lacunas

- Executar drops de Carry e Cargo, rejeicao e relogin no candidato.
- Confirmar no jogo o detach do cursor e a atualizacao do familiar.
- O drop de Equip nao e emitido pelo WYD-Go e permanece apenas compatibilidade
  do consumidor legado para slots suportados.

## Validação

- Pesquisa: `FUN_00492E7D -> FUN_00485D43` fecha o receptor e
  `FUN_0055890A` confirma `0x175/0x1C`.
- Automacao: fixture cobre todos os prefixos, excesso, nulo, divergencias de
  Size/Type, offsets, preservacao dos bytes e cada slot utilizavel de
  Carry/Cargo. `go test -count=1 ./internal/wire` e `go vet ./internal/wire`
  passaram para o builder Go.
- Build Release passou pelo `Build-Client.ps1` com 24581 checks e asserts
  estaticos. XML e registro unico do header passaram. Candidato instalado:
  `595C4722DB4DAD8C2A0EB5A6A0ECA2621A159F85C79822BA6E9B13295B21EE7C`.
- Estado `AUTOMATED TESTED`; client real ainda nao executado, portanto nao e
  `CLIENT_TESTED`.
