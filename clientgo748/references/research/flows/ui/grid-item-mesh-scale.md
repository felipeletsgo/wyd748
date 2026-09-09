---
id: ui-grid-item-mesh-scale
title: Caixa visual e escala de itens em grids 7.48
subsystem: ui-grid
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Caixa visual e escala de itens em grids 7.48

## Pergunta

Como o client nativo 7.48 dimensiona sprites e meshes de itens dentro dos
slots de inventário, Cargo, loja e demais grids, e por que a contenção pela
diagonal do AABB e a exceção específica para `egg001..egg014` estavam erradas?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter, projeto
  `WYD748Native_20260821.gpr`; `FUN_0040D13E`, `FUN_0040CEA0`,
  `FUN_0040E6AA`, `FUN_0040E817`, `FUN_0040FC3E`, `FUN_0040DD00` e
  `FUN_004B14FD`.
- Xrefs: `exports/grid-item-mesh-scale-xrefs.tsv`,
  `exports/grid-item-mesh-scale-vtable-callers.tsv` e
  `exports/grid-item-mesh-scale-vtable-window.tsv`.
- Source: `client-source/tmproject/Projects/TMProject/SGrid.cpp`.
- Recursos: `client748/ItemList.bin`, texturas de item e atlas UI2 `199`.

Os offsets nativos abaixo descrevem somente o executável identificado pelo
hash; a adaptação recompilável usa membros tipados da source atual.

## Fluxo nativo 7.48

### Entrada observável

O fluxo começa quando um item do catálogo é materializado em um grid. O item
pode seguir o caminho de sprite, quando o índice de mesh é negativo, ou o
caminho 3D, depois que `FUN_0040CEA0` resolve a mesh real. A colocação em uma
célula chama o slot virtual de inserção correspondente ao tipo de grid e, no
caminho 3D, ajusta a escala antes do frame de render.

### Callers

`FUN_0040D13E` possui callers diretos em inicialização de FieldScene e nos
fluxos que materializam inventário, loja, trade e NPC, entre eles
`FUN_00435B13`, `FUN_004431E4`, `FUN_004470B9`, `FUN_004662C5`,
`FUN_004875C0`, `FUN_0052DC5D` e `FUN_0052E3C8`.

`FUN_0040E6AA` e `FUN_0040E817` não possuem caller `CALL rel32`: elas ocupam,
respectivamente, os slots `+0x8C` e `+0x90` da vtable iniciada em
`0x005A4024` (`0x005A40B0` e `0x005A40B4`). Os dispatches indiretos foram
resolvidos no projeto Ghidra; o export registra os sites em rotinas de
travessia/controle como `FUN_0040656C`, `FUN_00406BD7`, `FUN_00409563`,
`FUN_00410A91` e `FUN_004110F5`. Portanto, a ausência de xref textual direto
é explicada pelo despacho virtual e não indica função morta.

### Funções e branches decisivos

`FUN_0040D13E` separa os dois renderers:

- mesh ou UI legada: caixa lógica de `24 x 24` por célula ocupada;
- sprite UI2 com item `5000..5102`: atlas `199` e caixa `23 x 23` por célula;
- demais sprites UI2: caixa `32 x 32` por célula.

Depois da inserção, `FUN_0040E6AA` e `FUN_0040E817` resolvem a mesh por
`FUN_004B14FD(meshIndex, 0, 180000)`. As duas funções leem exclusivamente
`mesh+0x354` (`MaxZ`) e a altura lógica do item. A constante em
`0x005A40EC` é `0.3f`:

```text
targetHeight = itemCellHeight * 0.3
scale = MaxZ > targetHeight ? targetHeight / MaxZ : 1.0
```

Não há leitura de `MinX`, `MaxX`, `MinY`, `MaxY`, `MinZ`, diagonal, raio,
nome, tipo ou família do item. Logo, não existe branch nativo especial para
ovos. Itens pequenos nunca são ampliados acima de `1.0`; itens altos são
reduzidos uniformemente.

### Callees

- `FUN_0040D13E` chama `FUN_0040CEA0` para resolver o índice da mesh,
  `FUN_00401BDC` para ligá-la e os helpers de textura/sprite no caminho 2D.
- `FUN_0040E6AA` e `FUN_0040E817` chamam o slot `+0x94` do item para vincular
  o grid e `FUN_004B14FD` para obter a mesh antes de ler `MaxZ`.
- `FUN_0040FC3E` calcula a origem da célula; `FUN_0040DD00` aplica a meia
  largura/altura e atualiza o objeto renderizado. A centralização não faz
  parte da fórmula de escala.

### Saídas e falhas

Se não houver mesh, o caminho de sprite continua válido. Se a mesh não for
resolvida ou `MaxZ` não for positivo/finito, a source preserva escala `1.0`
em vez de dividir por zero. Uma caixa de sprite ausente continua sendo um
problema de asset/catálogo e não autoriza inventar AABB ou footprint.

## Estado e lifecycle

| Evento | Precondição | Transição | Resultado | Falha/saída |
| --- | --- | --- | --- | --- |
| materializar item | índice válido no catálogo | resolve sprite ou mesh e define caixa lógica | item pronto para o grid | recurso ausente mantém guard existente |
| inserir no grid | célula e footprint válidos | registra ocupação e aplica escala por `MaxZ` | mesh cabe na altura lógica | mesh inválida fica em `1.0` |
| render/frame | item ligado ao grid | calcula origem e centro, renderiza sprite/mesh | apresentação centralizada | item nulo é ignorado |
| arrastar | item anexado ao cursor | copia o `GeomControl`, inclusive escala | drag preserva a apresentação | cancelamento devolve o item ao grid |
| remover/fechar | grid ou cena encerrado | item é desligado/destruído pelo owner | sem objeto retido | nenhuma tarefa assíncrona |
| logout/relogin | FieldScene reconstruída | controles e itens são recriados | fórmula é reaplicada | nenhum cache atravessa cenas |

## Wire, ABI e recursos

Não há mudança de packet ou autoridade. Índice, slot e footprint continuam os
do contrato 7.48; o servidor não recebe escala, caixa de sprite ou posição de
render. Os valores `23/24/32`, o atlas `199`, a constante `0.3f`, os slots de
vtable e `mesh+0x354` são evidência nativa, não offsets copiados para a ABI da
source recompilável.

## Mapeamento atual

### Source recompilável

Classificação: `PARIDADE_NATIVA`.

`WYD748_GetGridMeshScale()` em `SGrid.cpp` implementa a fórmula por `MaxZ` e
altura lógica. `WYD748_ApplyGridMeshScale()` a compartilha entre grids e
cursor, enquanto equipamento mantém escala explícita `1.0` por ser um painel
de receptáculos irregulares.

As caixas lógicas `23/24/32` são multiplicadas por
`RenderDevice::m_fWidthRatio/m_fHeightRatio`, porque os slots do controle já
são materializados no viewport atual. `m_GCObj.m_fWidth/m_fHeight` é
sincronizado com a mesma geometria, inclusive em `AddSkillItem()`, para que
slot e ícone cresçam juntos em resoluções acima de `800x600`.

### WYD-Go

Não aplicável: escala e caixa são apresentação local. Validação de compra,
movimento e uso do item continua server-authoritative e não foi alterada.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| escala de mesh | somente `MaxZ` e altura `0.3` por linha | diagonal AABB `0.27` | fórmula nativa | remover adaptação não comprovada |
| ovos | sem branch por família | exceção por ranges de mesh | fluxo comum | eliminar exceção |
| sprites UI2 | `23` em `5000..5102`, senão `32` | caixa não acompanhava slot escalado | caixa e `GeomControl` escalados juntos | portar geometria lógica |
| mesh/legado | `24` por célula | tamanho visual pequeno em viewport maior | `24` lógico com ratio | preservar proporção do slot |
| equipamento | receptáculo irregular | escala própria | escala `1.0` | não aplicar fit de grid |

## Decisões

- Usar somente `MaxZ`, altura lógica e teto `1.0`.
- Remover AABB diagonal, margem arbitrária e exceções por família de item.
- Preservar as três caixas lógicas nativas e escalá-las junto do controle.
- Não alterar wire, catálogo ou servidor para corrigir apresentação.

## Lacunas

A correlação estática está fechada. Falta validar visualmente meshes e sprites
em inventário, Cargo, loja, trade/mix, Skill Master e drag, incluindo itens
pequenos, altos e largos, em `800x600`, `1024x768` e `1280x960`.

## Validação

- `TRACED`: construtor, slots virtuais, dispatches indiretos, callee de mesh,
  fórmula, centralização e lifecycle foram correlacionados.
- `IMPLEMENTED`: a source ativa usa `MaxZ` e caixas lógicas `23/24/32` com a
  geometria renderizada sincronizada.
- `validate_research.py --repo .` e `git diff --check` passaram em 2026-09-02;
  este último emitiu somente avisos LF/CRLF da worktree existente.
- `Build-Client.ps1` passou em `Release|Win32` v145 com zero erros e 21
  warnings preexistentes. Output e `client748/project.exe` têm SHA-256
  `AD624D96A9F6DE7024D383594E9778586B20A113A9683150D9EDB1584CECCFEC`.
- Não é `CLIENT-TESTED`: nenhuma captura do candidato deste lote foi aprovada.
