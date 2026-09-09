---
id: ui-auto-trade-inventory-layout
title: Posição compartilhada de AutoTrade, Cargo e Inventory 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Posição compartilhada de AutoTrade, Cargo e Inventory 7.48

## Pergunta

Quais roots o client nativo reposiciona ao abrir AutoTrade como vendedor ou
como cliente, e por que mover o Inventory no modo cliente deixa a janela em uma
coordenada residual quando ela é reaberta pelo toggle normal?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter; `FUN_0044AE38`,
  `FUN_00447691` e `FUN_00435B13` no projeto
  `WYD748Native_20260821.gpr`.
- Corpus auxiliar: `0044ae38_FUN_0044ae38.c` e
  `00447691_FUN_00447691.c`.
- Recurso: `client748/UI/FieldScene2.bin`, roots AutoTrade, Cargo e Inventory
  `257`.
- Source: `TMFieldScene::PositionCompatFeaturePanels()`,
  `TMFieldScene::SetVisibleInventory()` e
  `TMFieldScene::SetVisibleAutoTrade()`.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0044AE38(scene, show, cargo)` controla a abertura e o fechamento de
AutoTrade. `show=1,cargo=1` é a composição do vendedor, que mostra Cargo para
escolher itens. `show=1,cargo=0` é a composição do cliente, que mostra o
Inventory. `show=0` encerra a transição, limpa o estado temporário e oculta as
superfícies compartilhadas.

### Callers

O corpus possui chamadas diretas em `FUN_00447691`, `FUN_00447C73`,
`FUN_004481C5`, `FUN_004484F3`, `FUN_00449384`, `FUN_00449632`,
`FUN_004498E0`, `FUN_00449B8E`, `FUN_00449E3C`, `FUN_0044A0FB`,
`FUN_0044A3A9`, `FUN_0044A745`, `FUN_0044A9E9`, `FUN_0044B890`,
`FUN_0044C15C`, `FUN_0044C53F`, `FUN_0044DF53`, `FUN_004662C5`,
`FUN_004776C3`, `FUN_0047FCD7`, `FUN_004829F2`, `FUN_00486424`,
`FUN_00529BF8` e `FUN_0052E2F6`. Eles cobrem toggles de painéis concorrentes,
comandos, eventos de mundo e cleanup.

O caller decisivo para a regressão é `FUN_00447691`: ao alternar o Inventory,
ele fecha AutoTrade quando necessário e muda a visibilidade do root, mas não
grava uma nova posição no perfil clássico.

### Callees

#### Receptores e posição

`FUN_0044AE38` resolve controles pelo container da FieldScene e usa os slots
virtuais `+0x60`, `+0x70`, `+0x80`, `+0x8C`, `+0x98`, `+0xA8` e `+0xB8` para
visibilidade, posição, texto, seleção, detach do cursor e operações dos grids.
Os receptores são roots/controles pertencentes à árvore da cena; a função não
aloca nem transfere ownership.

No perfil clássico, o branch `cargo=1` grava `530.0f,0` somente no root Cargo
e oculta Inventory. O branch `cargo=0` oculta Cargo e mostra Inventory, mas não
executa o slot virtual de posição sobre Inventory. A única escrita de posição
no fechamento volta Cargo para `280.0f,0`; ambos os roots são então ocultos.

A posição normal do Inventory vem da composição inicial de `FUN_00435B13`.
Portanto, uma escrita `Inventory.SetPos(530,35)` introduzida apenas pela source
na abertura do cliente não tem contraparte nesse fluxo nativo e sobrevive ao
fechamento, pois o toggle normal deliberadamente não reposiciona o root.

## Estado e lifecycle

| Evento | Precondição | Transição | Resultado | Cleanup/falha |
| --- | --- | --- | --- | --- |
| construir FieldScene | recurso carregado | `FUN_00435B13` compõe Inventory | posição normal pronta | root ausente não recebe escrita |
| abrir AutoTrade vendedor | `show=1,cargo=1` | posiciona e mostra Cargo; oculta Inventory | escolha de itens ao lado do AutoTrade | grids mudam para modo de trade |
| abrir AutoTrade cliente | `show=1,cargo=0` | mostra Inventory sem `SetPos` | reutiliza composição normal | equipamento/grid ficam não negociáveis conforme o modo |
| fechar AutoTrade | `show=0` | limpa descrição, preços, itens e alvo; oculta roots | nenhum estado visual de trade ativo | grids/equipamento voltam ao modo normal quando aplicável |
| reabrir Inventory | AutoTrade fechado | `FUN_00447691` alterna visibilidade | mesma posição normal | não existe restauração tardia de coordenada |
| trocar cena/logout | FieldScene encerrada | árvore e estado local são destruídos | nenhum root compartilhado sobrevive | sem callback de layout pendente |
| relogin | nova FieldScene | recurso, bindings e composição são refeitos | layout limpo | mesmos guards de root ausente |

## Wire, ABI e recursos

Não há alteração de packet, opcode, struct, asset ou regra autoritativa. O
cliente apenas deixa de sobrescrever uma coordenada local. Cadastro da loja,
preços, disponibilidade, compra e venda continuam validados no servidor.

## Mapeamento atual

Classificação: `PARIDADE_NATIVA`.

No caminho compatível, `SetVisibleAutoTrade(1,0)` continua ancorando o root de
AutoTrade e alternando visibilidade/estado dos grids, mas não move mais
Inventory. `PositionCompatFeaturePanels()` permanece como dono da composição
normal do root `257`; o modo vendedor ainda posiciona explicitamente Cargo,
como faz o nativo.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| AutoTrade vendedor | move Cargo; oculta Inventory | equivalente | preservado | paridade |
| AutoTrade cliente | mostra Inventory sem mover | movia para `530x35` | não move | remover escrita residual |
| toggle posterior | apenas visibilidade | herdava `530x35` | reutiliza composição inicial | preservar owner do layout |
| wire/gameplay | inalterado | inalterado | inalterado | nenhuma extensão |

## Decisões

- Não restaurar Inventory no fechamento: Shop, Cargo ou outro fluxo pode
  aplicar imediatamente sua própria composição.
- Não editar `FieldScene2.bin`; a posição final é calculada em runtime.
- Manter o `SetPos` de Cargo no branch vendedor e remover somente o `SetPos`
  extra de Inventory no branch cliente.

## Lacunas

- `TRACED`: entrada, callers diretos, receptores virtuais, estado, cleanup,
  teardown e relogin foram correlacionados.
- `IMPLEMENTED`: a escrita não nativa foi removida da source.

## Validação

- Ainda falta executar `validate_research.py`, `git diff --check` e o build do
  candidato após este delta.
- Não é `CLIENT-TESTED`: testar AutoTrade cliente e vendedor, fechar e reabrir
  Inventory, Shop e Cargo em `800x600`, `1024x768` e `1280x960`, incluindo
  troca de cena e logout/relogin.
