---
id: terrain-trn-loader
title: Native 7.48 TRN terrain loader
subsystem: render-assets
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Native 7.48 TRN terrain loader

## Pergunta

Qual contrato de framing do TRN e qual campo de cada célula o client nativo
consome para construir a malha de terreno?

## Fronteira de evidência

- Mode: `PARIDADE_NATIVA`
- Status: `CONTRACT`
- Scope: file framing and the one cell field directly consumed by the native
  terrain mesh builder. This does not claim collision, texture selection, or
  final rendering parity.

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
  record through `char` and converts it to a signed height sample. This is the
  only cell-field semantic promoted here.
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
| construir malha | grade carregada | `FUN_00534EBE` | altura consumida | leitura assinada de `Raw[0]` | demais bytes opacos |

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

O recurso é local e não atravessa o wire. Cada registro possui 12 bytes; o
primeiro é `char` assinado para a altura. Os outros 11 bytes permanecem crus.

## Go contract

`internal/assets.ParseTerrain` validates the length-prefixed name, two header
bytes, a square payload of 12-byte records, and bounded allocation. Each cell
owns its bytes and exposes `Height int8`, copied from `Raw[0]`. The other 11
bytes remain available through `Raw` and are intentionally opaque.

The parser does not use `Height` for collision or movement yet. A future
promotion of tile/material fields requires separate native xrefs and a focused
golden test.

## Mapeamento atual

### Source recompilável

Não há alteração da source nativa nesta unidade.

### WYD-Go

- Native 7.48 binary/Ghidra: `UTILIZADA`; primary evidence for framing and
  signed height access.
- Official 7.48 assets: `UTILIZADA`; `Env/Character.trn` validates the 64x64,
  4096-record instance.
- Current Go source/tests: `UTILIZADA`; implementation and regression tests.
- TMProject748: `UTILIZADA`; secondary comparison only for the 12-byte record
  shape, not for addresses or ABI.
- Guides and W2PP/Secrets/Micronics: `NÃO APLICÁVEL`/excluded; not used to
  establish this contract.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| framing TRN | confirmado | parser ausente | comparação secundária | parser validado | modernizar internamente |
| altura assinada | confirmado | não exposto | comparação secundária | `TerrainCell.Height` | portar |
| colisão/textura | não comprovado | não aplicável | não é prova | não ligado | proteger como pendente |

## Decisões

- Manter os 11 bytes desconhecidos em `Raw`.
- Usar a altura somente na superfície visual diagnóstica opcional.
- Não ligar altura a colisão ou movimento sem novos xrefs nativos.

## Lacunas

The meanings of bytes 1..11, the two header bytes, checksum table selection,
and the relationship between height and gameplay collision remain open. Do not
promote them from `Raw` without native evidence.

## Validação

- Pesquisa: Ghidra/corpus e binário nativo com hash registrado.
- Automação: `go test ./...`, `Verify-Fast.ps1`, `Verify-Contract.ps1`.
- Client real: superfície visual ainda pendente de execução interativa.
