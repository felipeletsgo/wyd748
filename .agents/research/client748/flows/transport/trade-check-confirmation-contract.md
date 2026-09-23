---
id: trade-check-confirmation-contract
title: Confirmacao visual do primeiro check de trade
subsystem: transport
status: UNMAPPED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Confirmacao visual do primeiro check de trade

## Pergunta

Qual envelope o client ativo deve aceitar para `0x386` antes de marcar o
controle `MyCheck`, sem permitir que um frame truncado ou divergente alcance a
UI de trade?

## Fronteira de evidência

- UTILIZADA: source atual `TMHuman::OnPacketEvent` e
  `TMHuman::OnPacketCNFCheck`.
- UTILIZADA: WYD-Go `wire.CNFTradeCheck`, emissor em `game/trade.go` e testes
  unitarios/de integracao do trade.
- UTILIZADA: contrato de header compartilhado e gate incremental
  `ReceivedPacketDispatch`.
- NAO REABERTA: a evidencia nativa de UI ja registrada em
  `flows/ui/trade-inventory-layout.md`.
- LACUNA: os exports Ghidra versionados nao isolam ainda o case `0x386`, e o
  corpus textual externo documentado nao estava disponivel nesta retomada.
- NAO APLICAVEL: assets; este lote nao altera layout, IDs ou recursos.

## Fluxo nativo 7.48

### Entrada observável

- Evento: o peer envia a confirmacao do primeiro check durante um trade ativo.
- Precondicoes e estado inicial: janela de trade aberta e `MyCheck` ainda nao
  selecionado.
- Saida observavel: o botao local passa ao estado selecionado.

### Callers

Nao resolvido para o binario nativo neste lote. O dispatcher recompilavel
seleciona `0x386` em `TMHuman::OnPacketEvent`.

### Função principal

Nao atribuida no Ghidra. O equivalente recompilavel
`TMHuman::OnPacketCNFCheck` valida cena/control container e marca somente
`TMB_TRADE_MYCHECK`.

### Callees

Nao resolvido no nativo. Na source, `FindControl(TMB_TRADE_MYCHECK)` e a unica
consulta relevante; o resultado opcional recebe `m_bSelected=1`.

### Saídas e erros

Na source, cena ausente, container ausente, cena fora de Field ou controle nao
materializado nao causam dereference nem retry; o handler retorna `1`.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| `0x386/12B` valido | Field + container + controle | `OnPacketCNFCheck` | `MyCheck` selecionado | feedback visual local | retorna `1` |
| cena/controle ausente | qualquer | `OnPacketCNFCheck` | estado preservado | nenhum | retorna `1` |
| frame invalido | antes do callback | `ReceivedPacketDispatch` | estado preservado | nenhum | entrega rejeitada |

### Vtables, vptrs e receptores

N/A para a decisao deste lote: o gate atua antes do dispatcher legado e o
consumidor recompilavel acessa diretamente o container da cena corrente.

### Ownership

O frame e emprestado durante a chamada. O handler nao conserva ponteiro, nao
aloca recurso e somente altera um byte de estado pertencente ao controle.

### Falha parcial

O gate rejeita o frame antes de qualquer mutacao. Depois da entrega valida, a
ausencia do controle deixa a UI inalterada sem estado parcial adicional.

### Cleanup e teardown

O handler nao possui recurso proprio. Fechar/cancelar o trade segue o fluxo
existente `0x384`, que oculta e reinicializa a janela.

### Shutdown

N/A: nao ha callback, timer, alocacao ou ownership criado por `0x386`.

### Logout e relogin

O controle pertence a FieldScene e nao sobrevive a destruicao da cena. O teste
real de logout/relogin durante trade permanece pendente no programa de paridade.

## Wire, ABI e recursos

Direcao S->C, opcode `0x386`, packing do header Windows existente e tamanho
total de 12 bytes. Nao ha payload.

| Campo | Offset | Largura | Signedness/uso |
| --- | ---: | ---: | --- |
| `Header.Size` | 0 | 2 | u16, deve ser 12 |
| `Header.KeyWord` | 2 | 1 | u8, transporte |
| `Header.CheckSum` | 3 | 1 | u8, transporte |
| `Header.Type` | 4 | 2 | u16, deve ser `0x386` |
| `Header.ID` | 6 | 2 | u16, personagem destinatario |
| `Header.Tick` | 8 | 4 | u32 Windows, transporte |

`TradeCheckConfirmationContract.h` vincula o opcode a `sizeof(MSG_STANDARD)` e
falha em compilacao se o envelope deixar de ter 12 bytes. Nenhum asset participa.

## Mapeamento atual

### Source recompilável

`TMHuman::OnPacketEvent` usa a constante compartilhada. O gate central exige
tamanho real/declarado exato e coerencia entre metadata e `Header.Type` antes
de chamar `OnPacketCNFCheck`.

### WYD-Go

`wire.CNFTradeCheck(id)` chama `Build(OpCNFTradeCheck, id, 12)`. O servidor
emite a confirmacao ao primeiro check valido e conserva a decisao autoritativa
do trade; o segundo check efetiva a troca somente apos revalidacao e commit.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| callback visual | raiz ainda nao isolada | marca `MyCheck` | procedencia nao alegada | emite apos check valido | preservar |
| envelope | tamanho nativo nao promovido | consumidor usa so header | literal antigo `0x386` | builder 12B | `MODERNIZACAO_COMPATIVEL` |
| autoridade | nao reaberta | feedback visual | N/A | servidor decide e persiste | manter servidor autoritativo |

## Decisões

- Fixar `0x386` como frame header-only de 12 bytes para o unico par ativo
  TMProject748/WYD-Go.
- Rejeitar nulo, truncamento, excesso e discriminantes divergentes antes de
  qualquer alteracao visual.
- Nao promover a equivalencia nativa nem nomear funcao Ghidra sem o case
  reproduzivel.

## Lacunas

- Isolar `0x386` no dispatcher/consumidor do projeto Ghidra quando a proxima
  decisao depender de paridade nativa, sem bloquear este gate interno.
- Executar trade com dois clients, primeiro/segundo check, cancelamento,
  disconnect e logout/relogin; o fluxo ainda nao e `CLIENT_TESTED`.

## Validação

- Pesquisa: source, builder/emissor Go e documentacao vigente cruzados; busca
  nos exports versionados nao encontrou prova suficiente para promover o fluxo.
- Automacao: `ArchitectureTests` passou com 41.253 checks/asserts; build Release
  x86 e instalacao do candidato passaram; `go test -count=1 ./...`,
  `go vet ./...`, XML, layout, links e `git diff --check` passaram.
- Client real: nao executado; `CLIENT_TESTED` permanece pendente.
