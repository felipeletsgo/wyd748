---
id: ui-shop-inventory-layout
title: Abertura lado a lado de Shop e Inventory 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Abertura lado a lado de Shop e Inventory 7.48

## Pergunta

Como o client nativo posiciona e abre a loja de NPC (`1793`) junto do
Inventory (`257`), e por que reaplicar apenas a posição serializada ou a
posição deixada por AutoTrade faz as janelas aparecerem deslocadas?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter; `FUN_00435B13` e
  `FUN_004481C5` no projeto `WYD748Native_20260821.gpr`.
- Export focado: `exports/shop-inventory-layout-flow.tsv` (225 instruções,
  nove callers únicos e sete callees).
- Callers diretos de abertura/fechamento: `FUN_00410A91`, `FUN_0044A745`,
  `FUN_0044A9E9`, `FUN_0044DF53`, `FUN_00472966`, `FUN_0047FCD7`,
  `FUN_004875C0`, `FUN_00488FFF` e `FUN_00529BF8`.
- Recurso: `client748/UI/FieldScene2.bin`, roots `1793` e `257`.
- Source: `TMFieldScene::PositionCompatShopPanels()` e
  `TMFieldScene::SetVisibleShop()`.
- Capturas fornecidas em 2026-09-02: loja e inventário deslocados no candidato;
  client original com a dupla centralizada lado a lado.

## Fluxo nativo 7.48

### Entrada observável

Na construção da FieldScene, `FUN_00435B13` encontra Shop `1793` e Inventory
`257`, calcula posições responsivas e inicia os roots ocultos. Ao interagir
com um NPC comerciante, um dos callers de `FUN_004481C5` abre o modo Shop; a
função fecha painéis concorrentes, troca o estado local do grid de inventário
e torna Shop e Inventory visíveis como uma única composição.

### Callers

`FUN_004481C5` possui os nove callers únicos listados na fronteira; o export
registra dez callsites porque `FUN_0047FCD7` chama a função em dois branches.
Eles cobrem abertura por NPC, fechamento, mudança entre lojas/serviços e
cleanup de UI. `FUN_00435B13` pertence à construção da FieldScene; seu caller
não é um `CALL rel32` no corpus textual, mas foi resolvido no projeto Ghidra
como a entrada de inicialização da cena.

### Funções e fórmula

No branch UI2 de `FUN_00435B13`, para viewport `(W,H)` e dimensões dos roots:

```text
Shop.x      = W*0.5 - Shop.width*0.5
Shop.y      = H*0.5 - Shop.height*0.5
Inventory.x = W*0.5 + Inventory.width*0.5 + 10
Inventory.y = H*0.5 - Inventory.height*0.5
```

O root `1793` fica centrado e `257` é colocado à direita com dez pixels
lógicos de separação. A composição precisa ser reaplicada ao abrir a loja,
porque AutoTrade e outras features reutilizam e movem o mesmo Inventory.

`FUN_004481C5(show=1)` fecha Cargo e painéis concorrentes, oculta AutoTrade por
`FUN_0044AE38`, fecha Trade por `FUN_0044B890`, apresenta Shop e Inventory e
coloca o grid no modo de venda. No perfil de UI 2, a abertura também reaplica
a posição de Inventory a partir do viewport; Shop permanece na posição
central calculada pelo initializer. O branch de fechamento restaura o modo
normal, limpa seleção/estado temporário e oculta a dupla.

### Callees

Os sete callees diretos de `FUN_004481C5` são `FUN_0040C0F0` para o perfil de
UI, `FUN_0044AE38` para fechar AutoTrade, `FUN_0044B890` para fechar Trade,
`FUN_00447F6F` para o modo do grid, `FUN_00429A6D`/`FUN_0042AD2B` para o helper
local de som e `__ftol` para a conversão da posição. Visibilidade e posição dos
roots são chamadas virtuais dos controles.

### Saídas e falhas

A saída é estritamente visual/local: dois roots visíveis, Inventory em modo
de venda e painéis incompatíveis fechados. Root ausente é tolerado na source
para preservar inicialização parcial; a janela ausente continua sendo defeito
de asset/binding. Nenhum valor de preço ou item é aceito como autoritativo por
esta transição.

## Estado e lifecycle

| Evento | Precondição | Transição | Resultado | Falha/saída |
| --- | --- | --- | --- | --- |
| construir FieldScene | recurso carregado | resolve 1793/257, calcula posições e oculta | composição pronta | root ausente é ignorado |
| abrir NPC Shop | alvo/loja válidos | fecha AutoTrade, reaplica layout e mostra a dupla | Shop + Inventory lado a lado | abertura parcial se root faltar |
| fechar Shop | dupla ativa | oculta roots e restaura modo do grid | UI normal | seleção temporária é limpa |
| abrir AutoTrade antes/depois | Inventory compartilhado | cada feature reaplica sua composição | sem posição herdada indevida | painel concorrente é fechado |
| logout/troca de cena | FieldScene destruída | árvore e estado local são liberados | nenhum root retido | sem callback assíncrono |
| relogin | nova FieldScene | bindings e fórmulas são refeitos | layout limpo | mesmos guards de construção |

## Wire, ABI e recursos

Não há mudança de wire, opcode, struct ou asset. `FieldScene2.bin` continua a
fornecer roots e dimensões; a posição final é recomposta em runtime. A compra
e venda continuam validadas pelo servidor. A adaptação não copia offsets de
objeto do executável nativo.

## Mapeamento atual

### Source recompilável

Classificação: `PARIDADE_NATIVA`.

`PositionCompatShopPanels()` usa o viewport atual e as dimensões já escaladas
dos roots para reproduzir a fórmula. Ele roda após os bindings iniciais e
novamente em `SetVisibleShop(1)`. Antes de mostrar a dupla, o caminho compatível
fecha AutoTrade visível; em seguida define `GRID_SELL` e apresenta Shop e
Inventory. No fechamento, ambos são ocultados e o grid volta a
`GRID_DEFAULT`.

### WYD-Go

Não há alteração server-side. O servidor continua validando NPC, distância,
catálogo, preço, gold, slot e persistência independentemente da posição da UI.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| layout Shop/Inventory | dupla responsiva com gap 10 | posição herdada/overlap | fórmula nativa | reaplicar no open |
| AutoTrade concorrente | fechado ao abrir Shop | podia deixar Inventory movido | fechamento explícito | preservar exclusividade |
| fechamento | dupla e modo temporário encerrados | caminho compatível incompleto | simétrico | restaurar grid local |
| servidor | autoridade de compra/venda | inalterado | inalterado | nenhuma extensão |

## Decisões

- Não editar posições fixas no asset para reproduzir uma fórmula de runtime.
- Reaplicar a composição sempre que Shop abrir, pois Inventory é compartilhado.
- Fechar AutoTrade concorrente antes de mover Inventory.
- Não alterar protocolo ou regras econômicas.

## Lacunas

Falta testar em jogo lojas de NPC com tamanhos/conteúdos diferentes, fechar e
reabrir, alternar com AutoTrade e repetir depois de logout/relogin nas três
resoluções-alvo.

## Validação

- `TRACED`: initializer, nove callers únicos, sete callees, fórmula,
  concorrência, teardown e relogin foram correlacionados.
- `IMPLEMENTED`: helper e reentrada de `SetVisibleShop(1)` estão na source.
- `validate_research.py --repo .` e `git diff --check` passaram em 2026-09-02;
  este último emitiu somente avisos LF/CRLF da worktree existente.
- `Build-Client.ps1` passou em `Release|Win32` v145 com zero erros e 21
  warnings preexistentes. Output e `client748/project.exe` têm SHA-256
  `AD624D96A9F6DE7024D383594E9778586B20A113A9683150D9EDB1584CECCFEC`.
- Não é `CLIENT-TESTED`: ainda falta aprovação visual do candidato deste lote.
