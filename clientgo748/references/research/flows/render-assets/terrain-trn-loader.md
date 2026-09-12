---
id: terrain-trn-loader
title: Native 7.48 TRN terrain loader
subsystem: render-assets
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-12
---

# Native 7.48 TRN terrain loader

## Pergunta

Qual contrato de framing do TRN, quais campos visuais de cada célula o client
nativo consome e como esses campos chegam às texturas/UVs usadas pelo terreno?

## Fronteira de evidência

- Mode: `PARIDADE_NATIVA`
- Status: `CONTRACT`
- Scope: file framing, signed height, primary/secondary material indices,
  primary/secondary UV selectors, `EnvTextureList3.bin` slot mapping and the
  WYS wrapper used by the active login terrain path, including the proven
  stage-1 `MODULATE` combine and texture-coordinate set 1. The Go/WGL path now
  implements that two-stage contract. Real-client Server and Login scenes now
  render the native terrain/materials, but this does not claim `CLIENT_TESTED`
  until the full Server -> Login -> Character Select path is exercised.

## Fluxo nativo 7.48

### Entrada observável

O loader recebe um caminho `.trn` e materializa uma grade de 4.096 células.

### Callers

The analyzed executable is `references/ghidra/input/WYD.exe` with SHA-256
`8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.

- `FUN_00533DD7` opens the path, reads a one-byte name length, copies that
  many bytes, reads two one-byte dimensions, then reads `0x1000` records of
  `0x0c` bytes into the terrain object.
- The same function validates a checksum over all `0xc000` payload bytes and
  builds border/normal data from the grid.
- `FUN_00534EBE`, called while building the mesh, reads the first byte of each
  record through `char` and converts it to a signed height sample.
- `FUN_00535298` is the native terrain renderer root used to promote the visual
  cell fields. It consumes `Raw[1]` as the primary material index,
  `Raw[2]` as the primary UV selector, `Raw[3]` as the secondary material
  index and `Raw[4]` as the secondary UV selector.
- `FUN_00535298` resolves the primary texture through slot `Raw[1] + 10`. For
  environment categories 0, 3 and 4 it also binds secondary slot
  `Raw[3] + 0x100` on texture stage 1.
- The same renderer copies primary UVs from the table rooted at
  `DAT_005bb918`/`DAT_005bb91c` and secondary UVs from
  `DAT_005bba18`/`DAT_005bba1c`. The recovered layouts contain exactly 8
  primary selectors and 32 secondary selectors.
- `FUN_004b9ce5` loads `EnvTextureList3.bin` as 512 fixed records of `0x108`
  bytes; the record starts with the zero-terminated filename for that texture
  slot.
- `FUN_004b9d99` decodes environment WYS assets by skipping the first byte,
  restoring the DDS prefix and restoring the FourCC at reconstructed DDS
  offset `0x54`. Native discriminator `'2'` maps to DXT1; the other native
  branch maps to DXT3.
- `FUN_00532390` constructs the terrain object; callers such as
  `FUN_0049F0E7`, `FUN_004A8F14`, and `FUN_00432181` select `.trn` resources for
  character and field scenes.

### Função principal

`FUN_00533DD7` lê o cabeçalho e os registros; `FUN_00534EBE` consome a altura.

### Callees

`FUN_00532390` constrói o objeto de terreno; os callers documentados acima
selecionam os recursos `.trn`.

### Saídas e erros

Framing inválido ou checksum inválido rejeita o asset; registros válidos
produzem a grade materializada.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| carregar TRN | caminho e framing válidos | `FUN_00533DD7` | grade 64x64 | células materializadas | rejeita framing/checksum |
| construir malha | grade carregada | `FUN_00534EBE` | altura consumida | leitura assinada de `Raw[0]` | bytes não promovidos permanecem crus |
| selecionar material primário | célula válida | `FUN_00535298` | stage 0 preparado | slot `Raw[1]+10`; UV selector `Raw[2]` | selector fora da tabela não possui contrato Go válido |
| selecionar material secundário | categoria 0/3/4 | `FUN_00535298` | stage 1 modula o resultado anterior | slot `Raw[3]+0x100`; UV selector `Raw[4]`; `D3DTSS_COLOROP=MODULATE`; `D3DTSS_TEXCOORDINDEX=1` | WGL usa multitextura real; Server/Login confirmados no client real; Character Select pendente |
| resolver filename por slot | `EnvTextureList3.bin` válido | `FUN_004b9ce5` | filename do slot disponível | 512 x `0x108` | framing inválido rejeitado no Go |
| decodificar WYS | wrapper válido | `FUN_004b9d99` | DDS reconstruído | `'2'` -> DXT1; outro branch nativo -> DXT3 | implementação ativa suporta DXT1 confirmado |

### Vtables, vptrs e receptores

Não há vtable adicional comprovada nesta fronteira; o objeto de terreno recebe
os registros durante o loader.

### Ownership

O parser Go devolve cópias próprias dos bytes de cada célula; o renderer recebe
somente uma referência de leitura ao terreno opcional.

### Falha parcial

O diagnóstico não é criado quando o asset está ausente; asset presente e
malformado produz erro explícito.

### Cleanup e teardown

O diagnóstico não mantém callbacks nem recursos além da cena mundial.

### Shutdown

O owner da aplicação libera a cena e o renderer no fechamento global.

### Logout e relogin

A superfície diagnóstica é reconstruída quando a cena mundial é criada; não há
estado de terreno persistido entre logout e relogin.

## Wire, ABI e recursos

O recurso é local e não atravessa o wire. Cada registro possui 12 bytes. Os
campos visuais comprovados são:

- `Raw[0]`: altura assinada (`int8`);
- `Raw[1]`: índice de material primário, resolvido como slot `+10`;
- `Raw[2]`: selector das 8 disposições UV primárias;
- `Raw[3]`: índice de material secundário, resolvido como slot `+0x100` nas
  categorias nativas 0/3/4;
- `Raw[4]`: selector das 32 disposições UV secundárias.

`Raw[5..11]` continuam sem semântica promovida nesta ficha.

## Go contract

`internal/assets.ParseTerrain` validates the length-prefixed name, two header
bytes, a square payload of 12-byte records, and bounded allocation. Each cell
owns its raw record and promotes `Height`, `PrimaryMaterialIndex`,
`PrimaryUVSelector`, `SecondaryMaterialIndex` and `SecondaryUVSelector` from
`Raw[0..4]`. Helpers expose the proven native primary/secondary slot formulas.

`internal/assets/terrain_uv.go` contains the exact 8 primary and 32 secondary
UV layouts recovered from native data referenced by `FUN_00535298`.

`internal/assets/env_texture_list.go` validates the exact 512 x `0x108`
`EnvTextureList3.bin` shape and promotes only the zero-terminated filename at
the start of each record. Unknown bytes in each record are not assigned
semantics.

`internal/assets/wys.go` implements the active native WS10/DXT1 path used by
the official Env corpus: it validates the wrapper/DDS header, accepts the
native `'2'` discriminator and expands the top DXT1 mip to RGBA8. The native
DXT3 branch is documented but intentionally not implemented until an active
7.48 asset requires it.

The parser still does not use terrain height/material data for gameplay
collision or movement.

## Mapeamento atual

### Source recompilável

Não há alteração da source nativa nesta unidade.

### WYD-Go

- Native 7.48 binary/Ghidra: `UTILIZADA`; primary evidence for framing,
  `Raw[0..4]`, texture slot formulas, UV tables, `EnvTextureList3.bin`, WYS
  reconstruction and the existence of the secondary stage.
- Studied decompilation corpus: `UTILIZADA`; `FUN_00533DD7`, `FUN_00534EBE`,
  `FUN_00535298`, `FUN_004b9ce5` and `FUN_004b9d99` are the direct anchors.
- Official 7.48 assets: `UTILIZADA`; `Field0813.trn` validates the 64x64,
  4096-record login terrain and exercises 30 distinct primary material slots.
  `EnvTextureList3.bin` maps those slots to official WYS assets; the scanned
  active Env WYS corpus is WS10/DXT1, 128x128 with four mip levels.
- Current Go source/tests: `UTILIZADA`; parser, UV tables, texture-list parser,
  WYS decoder, login material grouping and WGL stage-0 rendering have focused
  regression coverage.
- TMProject748: `NÃO APLICÁVEL` to the primary claims in this update; no native
  address, slot, UV or WYS semantic is being derived from TMProject.
- Guides and W2PP/Secrets/Micronics: `NÃO APLICÁVEL`/excluded; not used to
  establish this contract.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| framing TRN | confirmado | parser ausente | comparação secundária | parser validado | modernizar internamente |
| altura assinada | confirmado | não exposto | comparação secundária | `TerrainCell.Height` | portar |
| material primário/UV | confirmado em `FUN_00535298` | não aplicável | não é prova | slot + UV implementados | portar stage 0 |
| `EnvTextureList3.bin` | confirmado em `FUN_004b9ce5` | não aplicável | não é prova | parser 512 x `0x108` | portar |
| WYS WS10/DXT1 | confirmado em `FUN_004b9d99` + assets | não aplicável | não é prova | decoder RGBA8 | portar caminho ativo |
| material secundário/UV | slots/UV/combine confirmados | não aplicável | comparação secundária | dois UVs + par de slots + WGL multitextura implementados | portar contrato comprovado |
| colisão/movimento | não comprovado | não aplicável | não é prova | não ligado | proteger como pendente |

## Decisões

- Manter todos os 12 bytes em `Raw` mesmo quando `Raw[0..4]` possuem projeções
  semânticas, preservando provenance e pesquisa futura.
- Renderizar stage 0 por célula/material para preservar selectors UV que podem
  variar entre células adjacentes; não compartilhar vértices quando isso
  destruiria coordenadas UV por célula.
- Separar texturas de terreno do estado/cache de texturas de UI.
- Carregar somente os slots primários e secundários realmente usados pelo
  `Field0813.trn` no caminho ativo de login, deduplicando slots e resolvendo os
  filenames nativos sob o runtime asset root.
- `FUN_00535298` chama `(1,1,4)` e `(1,0xb,1)` para categorias 0/3/4. A source
  TMProject748 7.48 corrobora secundariamente esses valores como
  `SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE)` e
  `SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1)`. O reset `(1,1,1)` é
  `D3DTSS_COLOROP=D3DTOP_DISABLE`; stage 2 também é desabilitado ao final.
- O backend WGL carrega `glActiveTexture` e `glMultiTexCoord2f` via
  `wglGetProcAddress` após ativar o contexto, usa unidades 0/1 com `MODULATE` e
  desabilita/resetta a unidade 1 ao terminar cada lote de terreno.
- Não ligar altura/material a colisão ou movimento sem novos xrefs nativos.

## Lacunas

`Raw[5..11]`, the semantic meaning of the two TRN header bytes, checksum table
selection, and the relationship between terrain height/materials and gameplay
collision remain open.

The secondary texture slot, its 32 UV layouts and the fixed-function combine
state are proven: native stage 1 uses `COLOROP=MODULATE`, takes texture
coordinates from set 1 (`TEXCOORDINDEX=1`) and is reset with
`COLOROP=DISABLE`. That contract is now represented in the Go geometry and WGL
backend. Remaining gaps in this unit are semantic fields outside `Raw[0..4]`,
collision/gameplay linkage and completion of the real-client visual path through
Character Select.

## Validação

- Pesquisa: Ghidra/corpus e binário nativo com hash registrado; primary stage,
  secondary slot/UV contract, Env texture list and WYS wrapper traced.
- Automação desta unidade: focused Go tests pass for assets, graphics,
  loginflow and `cmd/wydclient`; Windows WGL compiles, including the native
  multitexture entry-point path. `Verify-Fast.ps1`, `Verify-Contract.ps1`,
  build/package smoke, bootstrap smoke and manifest regeneration all passed for
  this tree before the runtime visual check.
- Client real: the native terrain/materials render in Server Selection and the
  local Server -> Login transition was exercised successfully in the built
  `bin/Client Limpo/wydclient.exe`. The unit remains `CONTRACT`, not
  `CLIENT_TESTED`, because Login -> Character Select still requires an
  authenticated server session and remains pending.
