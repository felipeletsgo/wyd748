---
id: ui-trade-inventory-layout
title: Abertura lado a lado de Trade e Inventory 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Abertura lado a lado de Trade e Inventory 7.48

## Pergunta

Como o client nativo compõe o painel de troca (`576`) com o Inventory (`257`)
e encerra o estado temporário da negociação sem deixar a posição do Inventory
contaminar as próximas janelas?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter; initializer `FUN_00435B13` e
  toggle `FUN_0044B890` no projeto `WYD748Native_20260821.gpr`.
- Export focado: `exports/trade-inventory-layout-flow.tsv`.
- Callers diretos: `FUN_00447691`, `FUN_00447C73`, `FUN_004481C5`,
  `FUN_004484F3`, `FUN_0044AE38`, `FUN_0044C15C`, `FUN_0044C53F`,
  `FUN_0044DF53`, `FUN_004640E5`, `FUN_0047FCD7`, `FUN_0052DC5D`,
  `FUN_0052E2F6` e `FUN_0052E3C8`.
- Recurso: `client748/UI/FieldScene2.bin`, roots `576` e `257`.
- Source: `TMFieldScene::PositionCompatTradePanels()` e
  `TMFieldScene::SetVisibleTrade()`.
- Capturas fornecidas em 2026-09-02 mostram várias janelas herdando a mesma
  posição; a referência original mantém composições lado a lado.

## Fluxo nativo 7.48

### Entrada observável

`FUN_00435B13` resolve root `576` em `this+0x285F0` e Inventory `257` em
`this+0x27B34`, calcula a composição pelo viewport e inicia Trade oculto.
Packets de convite/aceite de troca chegam a callers como `FUN_0052DC5D` e
`FUN_004640E5`, que preparam nomes/estado e chamam `FUN_0044B890(1)`.

### Callers

O export registra treze callers únicos e dezessete callsites. Além das duas
entradas de abertura, os demais callers fecham Trade ao alternar Inventory,
Shop, Cargo, AutoTrade, NPCs e durante cleanup de sessão/cena. A presença de
vários encerramentos é parte do contrato de exclusividade, não duplicação.

### Funções e fórmula

No branch UI2 de `FUN_00435B13`, para viewport `(W,H)`:

```text
Trade.x     = W*0.5 - Trade.width*0.5
Trade.y     = H*0.5 - Trade.height*0.5
Inventory.x = W*0.5 + Inventory.width*0.5 + 10
Inventory.y = H*0.5 - Inventory.height*0.5
```

`FUN_0044B890(show=1)` solta o item do cursor, fecha AutoTrade e painéis
concorrentes, mostra Trade + Inventory, oculta roots incompatíveis e muda o
modo do grid para `7`. A função não recalcula a posição na abertura: ela usa a
composição criada pelo initializer. Como a source adaptada compartilha o mesmo
Inventory entre features que o reposicionam, ela reaplica exatamente essa
fórmula antes de mostrar Trade.

No fechamento, a função percorre a grade nativa `9x7`, remove highlight
vermelho, zera os textos de gold, oculta Trade/Inventory, limpa os quinze slots
temporários de cada lado, encerra o estado de troca e restaura o modo do grid.

### Callees

Os callees diretos são `FUN_0040C0F0`, `FUN_00429A6D`, `FUN_0042AD2B`,
`FUN_00447691`, `FUN_00447F6F`, `FUN_0044AE38`, `FUN_0055F2DD`, `_memset` e
`__ftol`. As chamadas virtuais do container fazem lookup de controles; as dos
roots controlam visibilidade e percorrem os itens da grade.

### Saídas e falhas

A abertura produz Trade + Inventory visíveis, campos de gold inicializados e
modo de grid temporário. O fechamento remove toda seleção e estado transitório;
se uma troca ainda estiver ativa, o caminho nativo também envia o cancelamento
correspondente. Na source, controles opcionais ausentes no asset 7.48 precisam
ser tolerados sem fabricar widgets de versões posteriores.

## Estado e lifecycle

| Evento | Precondição | Transição | Resultado | Falha/saída |
| --- | --- | --- | --- | --- |
| construir FieldScene | recurso carregado | resolve 576/257, posiciona e oculta | composição pronta | root ausente impede a dupla |
| aceitar/iniciar troca | outro jogador e sessão válidos | fecha concorrentes, mostra dupla, ativa grid 7 | negociação visível | servidor pode recusar/cancelar |
| fechar/cancelar | Trade ativo | limpa highlights, gold, slots e estado | UI normal | cancelamento é enviado se necessário |
| abrir outra feature | painel concorrente solicitado | caller fecha Trade primeiro | exclusividade preservada | sem posição residual |
| logout/troca de cena | FieldScene encerrada | controles e buffers locais são liberados | nenhum Trade retido | rede encerra sessão pendente |
| relogin | nova FieldScene | initializer refaz bindings e posições | layout limpo | mesmos guards de construção |

## Wire, ABI e recursos

Não há mudança de packet, ABI, struct ou asset. A adaptação usa IDs de roots,
dimensões e viewport da source; não copia offsets do objeto nativo. Validação
de convite, participantes, itens e conclusão da troca permanece autoritativa
no servidor.

## Mapeamento atual

### Source recompilável

Classificação: `PARIDADE_NATIVA`.

`PositionCompatTradePanels()` reproduz a composição de `FUN_00435B13` e é
chamado depois dos bindings e antes de `SetVisibleTrade(1)` expor os roots.
`SetVisibleTrade()` também fecha AutoTrade visível antes do reposicionamento.
O restante do lifecycle existente foi preservado.

### WYD-Go

Nenhuma alteração server-side é necessária para esta geometria. O servidor
continua autoritativo para pedido, aceite, conteúdo e commit/rollback da troca.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| layout Trade/Inventory | dupla responsiva com gap 10 | posição compartilhada podia ficar stale | fórmula nativa reaplicada | paridade |
| AutoTrade concorrente | fechado antes de Trade | condição dependia da visibilidade de Trade | fecha AutoTrade visível | corrigir gate |
| cleanup | grade 9x7 e quinze slots por lado | já presente com adaptação parcial | preservado | sem mudança de wire |
| servidor | troca autoritativa | inalterado | inalterado | nenhuma extensão |

## Decisões

- Reaplicar a fórmula no open porque o Inventory é compartilhado e móvel.
- Não gravar posições fixas no asset nem usar coordenadas posteriores.
- Preservar cleanup, cancelamento e autoridade do servidor.
- Não materializar controles opcionais ausentes no recurso 7.48.

## Lacunas

Falta testar em jogo convite, aceite, troca, cancelamento e alternância com
Shop/AutoTrade, inclusive depois de logout/relogin, em `800x600`, `1024x768` e
`1280x960`.

## Validação

- `TRACED`: initializer, treze callers, nove callees, fórmula, cleanup,
  teardown e relogin foram correlacionados.
- `IMPLEMENTED`: helper e reentrada em `SetVisibleTrade(1)` estão na source.
- Validação estática e build do lote atual permanecem pendentes.
- Não é `CLIENT-TESTED`: falta executar a negociação no candidato recompilado.
