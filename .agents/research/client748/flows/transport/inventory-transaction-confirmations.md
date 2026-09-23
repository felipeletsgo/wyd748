---
id: inventory-transaction-confirmations
title: Confirmações de movimentação e compra de item
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Confirmações de movimentação e compra de item

## Pergunta

Qual envelope e domínio de posições deve ser validado antes de `0x376` concluir
um drag entre Equip/Carry/Cargo e `0x379` materializar uma compra na UI 7.48?

## Fronteira de evidência

- UTILIZADA: inventário Ghidra versionado, que registra
  `FUN_00492E7D -> FUN_00486808 @ 0x00486808` para SwapItem e
  `FUN_00492E7D -> FUN_00487B92 @ 0x00487B92` para Buy.
- UTILIZADA: source atual `MSG_SwapItem`, `MSG_Buy`, `OnPacketSwapItem`,
  `OnPacketBuy`, criação dos requests em `SGrid` e topologia compatível.
- UTILIZADA: builders `wire.SwapItem`, `wire.BuyItem`, validação autoritativa em
  `onSwapItem`/`onBuyItem` e testes de layout do WYD-Go.
- UTILIZADA: fichas `shop-list-contract.md` e
  `equipment-slot-compatibility.md` para grade e slots ativos.
- NÃO APLICÁVEL: assets não alteram o wire nem os limites dos arrays.
- INDISPONÍVEL NESTA RETOMADA: corpus textual externo; os endereços/callers
  acima foram reutilizados do inventário Ghidra versionado, sem nova claim de
  pseudocódigo.

## Fluxo nativo 7.48

### Entrada observável

`0x376/20` devolve tipo e posição de origem/destino e conclui o estado do
cursor. `0x379/24` devolve mercador, célula da loja, slot Carry escolhido e o
saldo final, materializando a compra na grade.

### Callers

O dispatcher nativo `FUN_00492E7D @ 0x00492E7D` chama
`FUN_00486808 @ 0x00486808` e `FUN_00487B92 @ 0x00487B92`. Na source,
`TMFieldScene::OnPacketEvent` encaminha os mesmos opcodes aos handlers tipados.

### Função principal

`OnPacketSwapItem` usa os bytes recebidos como índices de arrays e grades,
troca os controles, atualiza os caches e solta o cursor. `OnPacketBuy` busca a
célula anunciada na loja, cria o visual no Carry e adota o `Coin` recebido.
Antes deste corte, ambos eram alcançáveis com apenas 12 bytes; SwapItem também
indexava posições sem validar o domínio completo do peer ativo.

### Callees

- As grades transferem ownership dos `SGridControlItem` aceitos.
- `UpdateMyHuman` reconstrói equipamento, montaria e costume após SwapItem.
- `UpdateScoreUI` projeta o saldo confirmado após Buy.
- O WYD-Go só publica sucesso depois de validar slots, mercador, item, preço,
  gold e persistência aplicáveis.

### Saídas e erros

Frames nulos, truncados, excedentes ou com Size/Type divergentes são rejeitados
antes dos handlers. Swap rejeita tipo desconhecido, Equip fora de `0..15` ou
slot 9, Carry fora de `0..62` e Cargo fora de `0..119`. Buy rejeita as regiões
mortas da posição esparsa de loja e Carry fora de `0..62`.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| swap confirmado | duas posições válidas | `OnPacketSwapItem` | caches/grades trocados | cursor solto e visual atualizado | inválido não toca estado |
| compra confirmada | loja e Carry válidos | `OnPacketBuy` | item materializado e Coin adotado | som e score atualizados | inválido não cria item |
| envelope inválido | qualquer | `ReceivedPacketDispatch` | estado preservado | nenhum callback | retorna `false` |

### Vtables, vptrs e receptores

O receptor continua sendo a `TMFieldScene`; controles e grids mantêm suas
vtable e regras de ownership. O contrato só antecipa validações.

### Ownership

O buffer é emprestado durante o callback. Os handlers materializam ou movem
controles próprios; o gate não retém nem altera bytes.

### Falha parcial

O domínio é validado antes de indexar `pGridSrc`, `pGridDest`, `Equip`, `Carry`
ou `Cargo`. Buy valida as duas posições antes de alocar o item visual.

### Cleanup e teardown

Nenhuma alocação é criada pelo gate. Cleanup de controles continua no handler
e no fechamento da FieldScene.

### Shutdown

N/A. Disconnect impede novas entregas e não deixa transação pendente criada por
esta validação.

### Logout e relogin

O servidor persiste o resultado antes da confirmação quando necessário; novo
login recebe snapshots autoritativos e não reutiliza estes buffers.

## Wire, ABI e recursos

`0x376`, C<->S, 20 bytes:

| Campo | Offset | Largura |
| --- | ---: | ---: |
| header | 0 | 12 |
| `SourType/SourPos/DestType/DestPos` | 12 | 4 |
| `TargetID` | 16 | 2 |
| reservado | 18 | 2 |

`0x379`, C<->S, 24 bytes:

| Campo | Offset | Largura |
| --- | ---: | ---: |
| header | 0 | 12 |
| `TargetID` | 12 | 2 |
| `TargetCarryPos` | 14 | 2 |
| `MyCarryPos` | 16 | 2 |
| alinhamento | 18 | 2 |
| `Coin` | 20 | 4 |

A loja usa três blocos de 27 posições, mas somente as primeiras nove células
de cada bloco são endereçáveis: `0..8`, `27..35` e `54..62`.

## Mapeamento atual

### Source recompilável

`InventoryTransactionContract.h` centraliza opcodes, tamanhos, offsets e
domínios. `ReceivedPacketDispatch` exige 20/24 bytes exatos; asserts fixam os
structs e os handlers consultam os mesmos predicados semânticos.

### WYD-Go

`onSwapItem` só confirma posições resolvidas por `playerSlotOf`; sucesso usa
`wire.SwapItem`, enquanto rejeições ressincronizam células com `SendItem`.
`onBuyItem` exige o mercador atualmente aberto, resolve a posição esparsa,
persiste item/gold e publica `wire.BuyItem` com o slot Carry escolhido.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| SwapItem | `0x376/20`, handler localizado | struct 20B, sem gate/domínio total | confirma 20B validado | fixar envelope e domínio |
| Buy | `0x379/24`, handler localizado | struct 24B, sem gate, aceitava buracos | confirma 24B validado | fixar envelope e células |
| autoridade | confirmação visual | mutava caches/UI | valida e persiste | preservar servidor autoritativo |

## Decisões

- Classificar layouts e lifecycle dos handlers como `PARIDADE_NATIVA` com a
  evidência Ghidra já versionada.
- Classificar o gate e os predicados antecipados como
  `MODERNIZACAO_COMPATIVEL`.
- Tratar slots 9, 16 e 17 fora da projeção equipável do peer WYD-Go 7.48 sem
  redimensionar o array estrutural legado.
- Não substituir confirmação de sucesso por `SendItem`; ele permanece o
  mecanismo de ressincronização de célula em falhas.

## Lacunas

- Falta validar no client real swaps entre Equip/Carry/Cargo e compra normal,
  incluindo bordas 62/119 e rejeições do servidor.
- O corpus externo precisa ser restaurado antes de qualquer nova afirmação
  sobre instruções internas dos dois handlers nativos.

## Validação

- Automação C++ cobre todos os prefixos, excesso, nulo, discriminantes,
  imutabilidade e todos os valores byte dos três domínios de Swap; Buy cobre o
  mapa esparso completo e Carry.
- Testes Go focados cobrem builders e handlers autoritativos.
- Client real: pendente; build/testes não promovem a `CLIENT_TESTED`.
