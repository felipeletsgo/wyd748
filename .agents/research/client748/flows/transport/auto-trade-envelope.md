---
id: auto-trade-envelope
title: Envelope de publicação e consulta de AutoTrade
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Envelope de publicação e consulta de AutoTrade

## Pergunta

Qual envelope deve ser aceito antes de `OnPacketAutoTrade` escrever na
descrição, copiar `m_stAutoTrade` e percorrer os doze itens e preços recebidos
em `0x397`?

## Fronteira de evidência

- UTILIZADA: inventário nativo versionado para `FUN_004656AF`, produtor
  estaticamente evidenciado chamado pelo dispatcher amplo `FUN_004662C5`.
- UTILIZADA: ficha `flows/ui/auto-trade-inventory-layout.md`, que resolve o
  lifecycle de abertura, fechamento e troca entre Cargo e Inventory.
- UTILIZADA: source atual `MSG_AutoTrade`, `TMFieldScene::OnPacketAutoTrade`,
  `SetVisibleAutoTrade` e os comentários de paridade dos doze slots.
- UTILIZADA: builder `wire.AutoTrade`, parser, autoridade e testes da Loja
  Fantasma no WYD-Go.
- NAO APLICAVEL: alteração de asset ou coordenada; este corte valida apenas o
  envelope antes do handler já existente.
- INDISPONIVEL NESTA RETOMADA: export externo do Ghidra; nenhuma claim nova foi
  criada além do inventário e das fichas já versionados.

## Fluxo nativo 7.48

### Entrada observável

`0x397` publica ou devolve a lista de uma AutoTrade. O frame de 196 bytes leva
descrição, doze itens, posições de origem, preços, taxa e o ID do vendedor. Ao
recebê-lo, a Field abre o painel, materializa as ofertas e associa o nome do
humano alvo.

### Callers

O inventário nativo registra `FUN_004656AF` como produtor chamado por
`FUN_004662C5`. A ficha de UI liga a entrega e os eventos subsequentes a
`FUN_0044AE38` e `FUN_004662C5`. Na source recompilável, o switch da Field
encaminha `MSG_AutoTrade_Opcode` diretamente para `OnPacketAutoTrade`.

### Função principal

`OnPacketAutoTrade` converte o header para `MSG_AutoTrade`, escreve zero nos
bytes finais de `Desc`, lê `TargetID`, copia todos os 196 bytes e percorre até
doze pares item/preço. Antes deste corte, `ReceivedPacketDispatch` exigia só o
header de 12 bytes para esse opcode.

### Callees

- `ObjectManager::GetHumanByID` resolve o nome do vendedor.
- `SetVisibleAutoTrade(1, 0)` compõe o painel do comprador com Inventory.
- Cada grid remove a oferta visual anterior e recebe um item temporário novo.

### Saídas e erros

Frame exato é entregue uma vez sem cópia pelo gate. Buffer nulo, prefixos
abaixo de 196 bytes, excesso e divergência de tamanho ou opcode são rejeitados
antes de qualquer escrita, alocação ou mudança de painel.

## Estado e lifecycle

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| abrir loja | clone/owner visível | `OnPacketAutoTrade` | ofertas locais substituídas | título, nome, itens e preços visíveis | frame inválido não chega ao handler |
| item vazio | slot sem `sIndex` ou preço | loop das ofertas | slot limpo | preço oculto | nenhuma alocação |
| fechar | painel ativo | `SetVisibleAutoTrade(0, ...)` | estado visual limpo | grids e superfícies liberados | ponteiros opcionais são tolerados |
| logout/troca de cena | Field encerrada | teardown da cena | nenhum painel sobrevive | objetos visuais destruídos | sem buffer pendente |
| relogin | nova Field | nova lista do servidor | snapshot novo | nenhum frame antigo reutilizado | mesmo gate de 196 bytes |

### Vtables, vptrs e receptores

O receptor é a `TMFieldScene`; controles e grids continuam resolvidos pelo
container e por seus métodos virtuais existentes. O contrato não altera tipo,
slot virtual ou ownership.

### Ownership

O transporte empresta o frame apenas durante o callback. `m_stAutoTrade` recebe
uma cópia própria; itens visuais são alocados pelo handler e pertencem aos
grids segundo o lifecycle já documentado.

### Falha parcial

A validação precede a escrita em `Desc`, o `memcpy` e qualquer alocação. Assim,
um truncamento não pode deixar descrição, oferta ou painel parcialmente
atualizados.

### Cleanup, shutdown, logout e relogin

O gate não cria recurso. Fechamento, disconnect e reconstrução da Field usam o
cleanup existente; nenhuma referência ao buffer recebido sobrevive à chamada.

## Wire, ABI e recursos

Direção C<->S, opcode `0x397`, tamanho fixo de 196 bytes:

| Campo | Offset | Largura | Semântica |
| --- | ---: | ---: | --- |
| `MSG_STANDARD` | 0 | 12 | size, key, checksum, type, id e tick |
| `Desc` | 12 | 24 | título terminado pelo client antes de exibir |
| `Item[12]` | 36 | 96 | doze `STRUCT_ITEM` de 8 bytes |
| `CarryPos[12]` | 132 | 12 | origem autoritativa de cada oferta |
| `TradeMoney[12]` | 144 | 48 | preços de 32 bits |
| `Tax` | 192 | 2 | taxa publicada |
| `TargetID` | 194 | 2 | vendedor ou clone da loja |

`AutoTradeContract.h` centraliza opcode, tamanho, offsets e cardinalidades. Os
asserts de `MSG_AutoTrade` prendem a fachada legada a esse contrato. Não há
mudança de packing, asset ou payload.

## Mapeamento atual

### Source recompilável

`ReceivedPacketDispatch::ExpectedSize` registra `0x397/196`. O gate compara
tamanho real/declarado e os discriminantes externo/interno antes do cast na
Field.

### WYD-Go

`wire.AutoTrade` já produz os mesmos 196 bytes. O servidor valida abertura,
Armia, origem dos itens, preço, persistência, compra e lifecycle da loja; o
client continua apenas apresentando o snapshot validado.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| opcode/cardinalidade | `0x397`, doze slots evidenciados | iguais | iguais | manter |
| layout | struct 196B já fixado | asserts parciais | builder nos offsets exatos | explicitar contrato |
| recepção S->C | consumidor conhecido | só header mínimo | sempre 196B | exigir frame exato |
| autoridade | apresentação local | handler visual | servidor valida e persiste | preservar |

## Decisões

- Classificar o ABI como `PARIDADE_NATIVA` com a evidência já versionada.
- Classificar o guard como `MODERNIZACAO_COMPATIVEL`: ele não muda o wire nem
  afirma equivalência com um gate nativo ainda não isolado.
- Não mover validação de negócio para o client nem alterar o lifecycle visual.

## Lacunas

- O fluxo precisa ser exercitado no `project.exe` como vendedor e comprador,
  incluindo loja vazia, doze ofertas, venda, fechamento e logout/relogin.
- O case nativo que rejeita frames S->C malformados não foi isolado; esta ficha
  não o usa como prova.

## Validação

- Pesquisa: produtor, consumidor, lifecycle e builder ativo foram cruzados com
  evidência versionada; não houve nova inferência a partir do corpus ausente.
- Automação: teste C++ cobre tamanho, nulo, todos os prefixos, excesso,
  discriminantes, entrega única, offsets extremos e imutabilidade. Testes Go
  existentes cobrem builder, parser e autoridade da Loja Fantasma.
- Client real: pendente; build e testes automatizados não promovem a
  `CLIENT_TESTED`.
