---
id: ui-native-mix-inventory-layout
title: Composição dos ItemMix nativos com Inventory 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Composição dos ItemMix nativos com Inventory 7.48

## Pergunta

Como o client nativo posiciona cada uma das seis janelas de composição e o
Inventory compartilhado, e por que os painéis da source podiam abrir na
posição residual deixada por outra feature?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter no projeto
  `WYD748Native_20260821.gpr`.
- Initializer: `FUN_00435B13`.
- Toggles: `FUN_00449384`, `FUN_00449632`, `FUN_004498E0`, `FUN_00449B8E`,
  `FUN_00449E3C` e `FUN_0044A0FB`.
- Cleanups: `FUN_004487E2`, `FUN_004489C5`, `FUN_00448C38`, `FUN_00448E0B`,
  `FUN_00448FDE` e `FUN_004491B1`.
- Fechamento compartilhado: `FUN_00447691` e `FUN_004890F9`.
- Lifecycle da cena: vtable `PTR_FUN_005A4294`, deleting destructor
  `FUN_00493BC0`, teardown `FUN_004358DA` e construtor `FUN_004343A4`.
- Export focado: `exports/native-mix-inventory-layout-flow.tsv`, gerado em
  read-only/no-analysis sem `SCRIPT ERROR` para os 19 símbolos acima.
- Source: `TMFieldScene::PositionCompatNativeMixPanels()`,
  `TMFieldScene::InitializeCompatFieldScene()` e
  `TMFieldScene::SetVisibleNativeMix()`.
- Lifecycle base reutilizado: `flows/lifecycle/scene-transition.md`.

## Fluxo nativo 7.48

### Construção e bindings

`FUN_00435B13` resolve seis roots independentes e os mantém em membros
distintos da FieldScene:

| ItemMix | Root | Membro nativo | Toggle | Cleanup | Tipo do Carry |
| ---: | ---: | ---: | --- | --- | ---: |
| 1 | 1360 | `this+0x27B6C` | `FUN_00449384` | `FUN_004487E2` | `0x0D` |
| 2 | 6110 | `this+0x27B90` | `FUN_00449632` | `FUN_004489C5` | `0x0F` |
| 3 | 6145 | `this+0x27BB4` | `FUN_004498E0` | `FUN_00448C38` | `0x11` |
| 4 | 6432 | `this+0x27BE8` | `FUN_00449B8E` | `FUN_00448E0B` | `0x13` |
| 5 | 6481 | `this+0x27C10` | `FUN_00449E3C` | `FUN_00448FDE` | `0x15` |
| 6 | 6512 | `this+0x27C38` | `FUN_0044A0FB` | `FUN_004491B1` | `0x17` |

O Inventory root `257` permanece compartilhado com Character, Skill, Shop,
Trade e AutoTrade. A posição serializada não representa, sozinha, nenhuma
dessas composições de runtime.

### Fórmula de posição

Para viewport `(W,H)`, centro `(centerX,centerY)` e dimensões já
materializadas:

```text
ItemMix.x   = centerX - ItemMix.width*0.5
ItemMix.y   = centerY - ItemMix.height*0.5
Inventory.x = centerX + Inventory.width*0.5 + 10
Inventory.y = centerY - Inventory.height*0.5
```

Cada root ItemMix é centralizado independentemente; Inventory fica à direita
com o mesmo gap lógico de dez pixels usado nas demais composições 7.48. A
fórmula deve ser reaplicada na abertura porque os outros modos movem o root
`257` entre transições.

### Entrada observável e concorrência

Cada toggle é alcançado pelo dispatcher de interação com NPC em
`FUN_0045EE28`; `FUN_004890F9` também fecha os ItemMix 1 a 5 durante a
reconfiguração da cena. No branch de abertura, os toggles chamam a cascata
`FUN_00447691`, encerram painéis incompatíveis, limpam o staging do próprio
artesão, tornam Inventory e ItemMix visíveis e atribuem ao Carry o tipo ímpar
da tabela. AutoTrade é fechado por `FUN_0044AE38`.

Os seis toggles possuem o mesmo conjunto estrutural de callees: perfil de UI,
cascata de fechamento, modo de grid, cleanup específico, AutoTrade, controles
temporários e conversão de coordenadas. O export preserva callsites separados
quando o mesmo caller abre ou fecha um modo por branches distintos.

### Callers

Os seis toggles recebem chamadas do dispatcher `FUN_0045EE28`; Mix1 e Mix5
possuem callsites adicionais porque atendem mais de um branch/NPC. Mix1 a Mix5
também são chamados por `FUN_004890F9` durante o fechamento da configuração da
cena. A cascata compartilhada `FUN_00447691` recebe chamadas dos seis toggles,
de Shop, Trade, AutoTrade, ESC e outros caminhos de troca de painel registrados
no export. O initializer `FUN_00435B13` é alcançado pela entrada de construção
da FieldScene resolvida no projeto Ghidra.

### Callees

Cada toggle chama `FUN_00447691`, seu cleanup específico da tabela,
`FUN_00447F6F` para o modo do grid, `FUN_0044AE38` para AutoTrade e os helpers
de controles/som/conversão registrados no export. Os seis cleanups chamam
`FUN_00447F6F` e `_memset`. `FUN_00447691` chama todos os seis cleanups, além
dos toggles de Shop, AutoTrade e Trade. O deleting destructor
`FUN_00493BC0` chama `FUN_004358DA`, que executa o teardown da cena.

### Cleanup, teardown e relogin

Os cleanups específicos restauram o estado visual dos itens staged, esvaziam
os grids temporários, zeram o packet local e devolvem o Carry ao modo normal.
`FUN_00447691` chama os seis cleanups na cascata geral de fechamento/ESC.
`FUN_004890F9` chama os cinco primeiros toggles para fechamento; o sexto é
controlado pelo fluxo próprio do NPC correspondente.

No fim da FieldScene, o slot da vtable `PTR_FUN_005A4294` aponta para o
deleting destructor `FUN_00493BC0`, que chama `FUN_004358DA`. Esse teardown
libera a árvore e o estado da cena; não há root ou posição de ItemMix que deva
atravessar logout. `FUN_004343A4` é o construtor, não o destrutor. No relogin,
uma nova FieldScene executa novamente os bindings e fórmulas do initializer.

## Estado e lifecycle

| Evento | Precondição | Transição | Resultado | Falha/saída |
| --- | --- | --- | --- | --- |
| construir FieldScene | recurso carregado | resolve 6 roots e calcula posições | layouts prontos e ocultos | root ausente não deve derrubar a cena |
| abrir artesão N | NPC/serviço válido | fecha concorrentes, limpa N, reposiciona e mostra dupla | ItemMix N central + Inventory à direita | abertura parcial denuncia binding/asset ausente |
| trocar de artesão | outro ItemMix ativo | fecha e limpa anterior antes do próximo | um único staging ativo | item visual volta ao Carry |
| fechar/ESC | ItemMix ativo | cleanup específico + modo normal | dupla oculta e estado temporário zerado | cursor é destacado na source |
| logout/troca de cena | FieldScene ativa | deleting destructor chama teardown | árvore liberada | nenhum callback retém painel |
| relogin | nova FieldScene | bindings e fórmulas são refeitos | posição limpa | mesmos guards de root/dispositivo |

## Wire, ABI e recursos

Esta mudança não altera packet, opcode, struct, receita nem asset. Os tipos
`0x0D..0x17` pertencem ao estado local do Carry e não autorizam combinação.
O servidor continua validando NPC, alcance, receita, itens, gold, mutação,
persistência e resposta. Os offsets nativos acima documentam correlação; não
são copiados para a classe recompilável.

## Mapeamento atual

Classificação: `PARIDADE_NATIVA`.

Antes desta correção, `InitializeCompatFieldScene()` chamava
`PositionCompatFeaturePanels()` antes de vincular os seis membros ItemMix, e
`SetVisibleNativeMix()` apenas tornava Inventory e o artesão visíveis. Portanto,
ambos podiam conservar coordenadas serializadas ou deixadas por Shop, Trade e
AutoTrade.

`PositionCompatNativeMixPanels()` centraliza todos os roots existentes e
posiciona Inventory à direita com guards para perfil 7.48, dispositivo e roots
opcionais. A chamada ocorre depois dos seis bindings e novamente no branch de
abertura de `SetVisibleNativeMix()`. Nenhum caminho moderno ou protocolo foi
alterado.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| posição dos ItemMix | cada root centralizado | posição residual | fórmula por root | reproduzir initializer |
| posição do Inventory | direita, gap 10 | compartilhada sem recomposição | fórmula reaplicada no open | impedir herança |
| concorrência | um artesão por vez | já fechava os demais | preservada | nenhuma extensão |
| cleanup/teardown | específico + cascata + cena | já implementado | preservado | não misturar com layout |
| autoridade | servidor valida combinação | inalterada | inalterada | nenhuma mudança wire |

## Decisões

- Não alterar posições fixas em `FieldScene2.bin`; o nativo recompõe em runtime.
- Reaplicar a dupla na abertura porque Inventory é um root compartilhado.
- Centralizar os seis roots, mesmo ocultos, para que qualquer entrada comece de
  um estado determinístico.
- Preservar recipes, opcodes, tipos locais, staging e cleanup já implementados.

## Lacunas

Falta validar visualmente os seis NPCs, alternar entre eles e Shop/Trade/
AutoTrade, fechar por botão e ESC e repetir após logout/relogin em `800x600`,
`1024x768` e `1280x960`.

## Validação

- `TRACED`: initializer, roots, membros, toggles, cleanups, fechamento
  compartilhado, vtable, teardown e relogin foram correlacionados.
- `IMPLEMENTED`: helper, chamada pós-binding e reentrada no open estão na
  source.
- `STATICALLY VERIFIED`: `validate_research.py` e `git diff --check` passaram;
  `Build-Client.ps1` concluiu `Release|Win32` com zero erros e 21 warnings e
  instalou `client748/project.exe` com SHA-256
  `D01DF6971578BA828AF674772C52A506AD0D654B23910185F63892177EE577AD`.
- Não é `CLIENT-TESTED`: falta execução real do candidato.
