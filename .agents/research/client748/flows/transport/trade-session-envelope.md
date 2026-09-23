---
id: trade-session-envelope
title: Envelope de oferta e fechamento de trade
subsystem: transport
status: UNMAPPED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Envelope de oferta e fechamento de trade

## Pergunta

Quais frames o par ativo TMProject748/WYD-Go usa para publicar uma oferta de
trade e encerrar a janela, e como impedir que casts legados leiam um snapshot
truncado ou aceitem discriminantes divergentes?

## Fronteira de evidência

- UTILIZADA: source atual `TMHuman::OnPacketEvent`, `OnPacketTrade` e
  `OnPacketQuitTrade`.
- UTILIZADA: `MSG_Trade`, seus asserts de ABI e os emissores do client em
  `TMFieldScene`/`SGrid`.
- UTILIZADA: WYD-Go `wire.Trade`, `wire.CloseTrade`, `game.onTrade`,
  `game.cancelTrade` e os testes do wire/trade.
- UTILIZADA SEM PROMOCAO DO WIRE: a evidencia nativa de lifecycle visual em
  `flows/ui/trade-inventory-layout.md`.
- LACUNA: a busca nos exports Ghidra versionados nao isolou os cases `0x383`
  e `0x384` do dispatcher nativo; ocorrencias numericas sem callgraph foram
  descartadas como prova.
- NAO APLICAVEL: assets; nenhum ID, root ou arquivo de UI muda neste lote.

## Fluxo nativo 7.48

### Entrada observável

- Evento: convite ou atualizacao de oferta chega durante a FieldScene, ou o
  peer/servidor encerra a negociacao.
- Precondicoes: personagem local e container da cena materializados; para
  atualizar a oferta, a sessao de trade ja aponta para um oponente.
- Saida observavel: `0x383` abre/atualiza itens, gold e checks; `0x384` limpa o
  estado temporario e oculta a composicao Trade + Inventory.

### Callers

O dispatcher nativo ainda nao foi atribuido. Na source recompilavel,
`TMHuman::OnPacketEvent` seleciona os dois opcodes e encaminha o mesmo buffer.

### Função principal

As funcoes nativas do wire permanecem nao mapeadas. Os equivalentes atuais sao
`TMHuman::OnPacketTrade(MSG_Trade*)` e
`TMHuman::OnPacketQuitTrade(MSG_STANDARD*)`.

### Callees

Nao resolvidos no binario para este contrato. Na source, `OnPacketTrade`
percorre os 15 grids, compara/copia `STRUCT_ITEM`, atualiza gold/checks e pode
abrir a janela; `OnPacketQuitTrade` limpa o estado e fecha os paineis.

### Saídas e erros

Dependencias de cena ausentes fazem os handlers retornarem sem dereference. O
novo gate rejeita o envelope antes de qualquer callback e nao executa retry.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| `0x383/156B` convite | Field valida, sem trade local | `OnPacketTrade` | convite pendente | message box | retorna `1` |
| `0x383/156B` oferta | trade ativo | `OnPacketTrade` | snapshot remoto substituido | grids/gold/checks atualizados | retorna `1` |
| `0x384/12B` | janela ou estado temporario presente | `OnPacketQuitTrade` | trade zerado | paineis ocultos | retorna `1` |
| frame invalido | qualquer | `ReceivedPacketDispatch` | estado preservado | nenhum callback | rejeitado |

### Vtables, vptrs e receptores

N/A para a decisao do envelope. O gate atua antes dos objetos de UI e nao muda
dispatch virtual nem ownership de controles.

### Ownership

O transporte empresta o frame somente durante a chamada. Itens visuais validos
sao copiados para objetos proprios da grid; nenhum ponteiro para o frame fica
retido.

### Falha parcial

Um `0x383` parcial era capaz de alcancar leituras ate `OpponentID@154`. Agora
nulo, truncamento, excesso ou divergencia de Size/Type sao rejeitados antes da
primeira mutacao. O comportamento interno dos handlers validos nao mudou.

### Cleanup e teardown

`0x384` reutiliza o cleanup existente: zera o oponente/check local, solta estado
visual e fecha Trade/Inventory conforme o lifecycle da FieldScene.

### Shutdown

O gate nao cria callback, thread, timer ou recurso. O teardown continua sob a
FieldScene e o ObjectManager existentes.

### Logout e relogin

O servidor cancela trade em desconexao/logout e publica o fechamento ao peer.
O teste real com dois clients continua pendente; nao ha claim `CLIENT_TESTED`.

## Wire, ABI e recursos

`0x383` e bidirecional e possui 156 bytes. `0x384` e bidirecional/header-only e
possui 12 bytes no par ativo.

| Campo | Offset | Largura | Signedness/uso |
| --- | ---: | ---: | --- |
| `Header` | 0 | 12 | `Size`, `Type`, `ID` e transporte |
| `Item[15]` | 12 | 120 | quinze `STRUCT_ITEM` de 8 bytes |
| `CarryPos[15]` | 132 | 15 | i8; `-1` representa slot vazio |
| padding | 147 | 1 | alinhamento do `int` Windows x86 |
| `TradeMoney` | 148 | 4 | i32 na source; validado como gold nao negativo no servidor |
| `MyCheck` | 152 | 1 | dominio 0/1 |
| padding | 153 | 1 | alinhamento de `OpponentID` |
| `OpponentID` | 154 | 2 | u16 |

`TradeSessionContract.h` centraliza opcodes, tamanhos e offsets. Os asserts de
`Basedef.h` vinculam esses valores a `MSG_Trade`; o projeto permanece x86.
Nenhum asset participa.

## Mapeamento atual

### Source recompilável

`TMHuman::OnPacketEvent` usa constantes compartilhadas. O gate central exige
tamanho real/declarado exato e igualdade entre metadata e `Header.Type` antes
dos casts para `MSG_Trade*` e `MSG_STANDARD*`.

### WYD-Go

`wire.Trade` usa `Build(OpTrade, id, 156)` e escreve exatamente os offsets da
tabela. `wire.CloseTrade` usa `Build(OpCloseTrade, id, 12)`. `onTrade` valida o
snapshot contra inventario/gold autoritativos; cancelamento, commit e outros
teardowns usam o fechamento sem transferir autoridade ao client.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| lifecycle visual | composicao/cleanup ja `TRACED` | abre, atualiza e fecha | herdado/adaptado | emite nos estados vivos | preservar |
| envelope `0x383` | case nativo nao isolado | cast usa ate byte 155 | struct/assert 156B | builder e parser 156B | `MODERNIZACAO_COMPATIVEL` |
| envelope `0x384` | case nativo nao isolado | consumidor header-only | literal legado | builder 12B | `MODERNIZACAO_COMPATIVEL` |
| autoridade | nao reaberta | projecao visual/intencao | N/A | valida e efetiva | manter servidor autoritativo |

## Decisões

- Fixar `0x383/156B` e `0x384/12B` para o unico par ativo sem alterar payload,
  efeitos visuais ou regra de negocio.
- Rejeitar nulo, todo prefixo truncado, excesso e discriminantes divergentes
  antes dos handlers legados.
- Nao promover o wire a paridade nativa enquanto os cases Ghidra nao forem
  reproduziveis.

## Lacunas

- Isolar `0x383` e `0x384` no dispatcher nativo quando uma decisao futura
  depender de paridade, sem reabrir o gate interno ja testado.
- Executar convite, aceite, mudanca de item/gold, dois checks, cancelamento,
  disconnect e logout/relogin com dois clients reais.

## Validação

- Pesquisa: consumidor/emissores C++, builders/parser/emissores Go, docs e
  exports versionados foram cruzados; nao houve evidencia suficiente para
  promover o contrato wire nativo.
- Automacao focada: `ArchitectureTests` passou com 41.441 checks/asserts,
  cobrindo todos os prefixos, excesso, nulo, Size/Type divergentes, entrega
  unica, offsets observados e imutabilidade dos dois frames.
- Integracao: `go test -count=1 ./...`, `go vet ./...`, XML dos projetos,
  pesquisa e layout/links passaram.
- Build: `Build-Client.ps1` passou com os mesmos 41.441 checks/asserts e
  instalou `tmproject/client748/project.exe` com SHA256
  `3E50847668AAE39DDFC09C1068ACBBD02FB62AB2C47B508FD6265D529BE453D4`.
- Client real: nao executado; o fluxo permanece nao `CLIENT_TESTED`.
