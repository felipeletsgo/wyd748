---
id: initial-server-logo-texture
title: Initial server-logo texture flow
subsystem: render-assets
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Initial server-logo texture flow

## Classification

- Mode: `PARIDADE_NATIVA`
- Status: `CONTRACT`
- Scope: read one official 7.48 `WT10` texture, convert it to owned RGBA8
  pixels, and make it available to the renderer without changing the source
  asset.

This contract is limited to the file boundary and pixel conversion. OpenGL
upload and scene presentation are separate lifecycle steps and are not claimed
as `CLIENT_TESTED` by this document.

## Native evidence

Executable under analysis:

```text
clientgo748/references/ghidra/input/WYD.exe
SHA-256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
```

Relevant native functions:

- `FUN_004B7BE5` initializes texture groups.
- `FUN_004B827F` loads `UI\\UITextureList.txt`.
- `FUN_004B8928` loads `UITextureSetList.txt`.
- `FUN_004B8333` opens a UI texture, checks DDS, removes the four-byte
  `WT10` prefix for the non-DDS path, appends the TGA signature bytes, and
  calls `FUN_00560EAB`.
- `FUN_00560EAB` wraps `FUN_00560838`, the generic device texture creation and
  upload path.
- `FUN_00560838` consumes the image buffer through the native graphics
  interface; its exported pseudocode does not expose the complete device
  structure layout.

The native loader behavior explains the eight bytes at the end of each
official file: after `WT10` is removed, the remaining image is a TGA image
with the eight-byte footer offsets. The loader supplies the final
`TRUEVISION-XFILE.` plus NUL signature before the graphics API consumes it.
The Go parser reads the payload directly and therefore does not need to create
that synthetic footer.

## Asset evidence

The official corpus contains 221 `.wyt` files. Every sampled file has:

```text
offset 0:  "WT10"
offset 4:  18-byte little-endian TGA true-color header
payload:   uncompressed pixels
trailer:   eight zero bytes
```

Observed source formats:

| Bits per pixel | Descriptor examples | Count | Conversion |
| ---: | --- | ---: | --- |
| 16 | `0x01` | 2 | B5G5R5A1 to RGBA8 |
| 24 | `0x00` | 180 | BGR to RGBA8 with opaque alpha |
| 32 | `0x08` | 39 | BGRA to RGBA8 |

The largest observed asset is `1024x512`; UI backgrounds include `800x600`.
The parser also accepts TGA origin bits and normalizes the result to top-left
origin for the renderer.

## Go contract

`internal/assets.ParseWYT`:

- requires the `WT10` signature;
- accepts only uncompressed true-color TGA (`image type 2`, no color map);
- accepts 16, 24, and 32 bits per pixel;
- validates non-zero dimensions, payload length, and the optional eight-byte
  zero trailer;
- bounds dimensions and total pixels before allocating;
- returns an owned `Texture` containing tightly packed RGBA8 pixels;
- rejects unsupported types, malformed signatures, truncation, unexpected
  trailing data, and unsafe dimensions.

`internal/assets.LoadWYTFile` reads the official file and delegates to the
parser. It never rewrites the source asset.

## Source matrix

| Source | Status | Use |
| --- | --- | --- |
| Native WYD 7.48 binary/Ghidra | `UTILIZADA` | Loader call chain and WT10/TGA handoff |
| Official 7.48 assets | `UTILIZADA` | Header, dimensions, formats, trailer |
| Current WYD-Go tests | `UTILIZADA` | Parser safety and pixel-order contract |
| Current TMProject source | `UTILIZADA` | Secondary comparison of texture-manager responsibilities only |
| Guides and other documented references | `NÃO APLICÁVEL` | No stronger format evidence than the native/assets pair |
| W2PP, Secrets, Micronics | `EXCLUÍDA` | Bugged sources excluded by project policy |

## Validation status

- Parser unit tests: `AUTOMATED TESTED`.
- Official `logo1.wyt` load: `AUTOMATED TESTED`.
- OpenGL upload and visual logo presentation: pending; do not claim
  `CLIENT_TESTED` until the built Windows client is executed.

## Pergunta

Como o client 7.48 localiza, valida, decodifica e entrega a primeira textura
oficial ao renderer, mantendo o asset original intacto e liberando os recursos
gráficos no encerramento?

## Fronteira de evidência

- Executável/hash: `references/ghidra/input/WYD.exe`, SHA-256 registrado no
  front matter.
- Projeto/corpus Ghidra: exports locais do loader de texturas e da criação de
  textura; o callgraph foi conferido para `FUN_004B8333` e seus callees.
- Assets: corpus oficial local com 221 arquivos `.wyt`, incluindo `logo1.wyt`.
- Source atual: `internal/assets`, `internal/graphics`, `internal/app` e
  `cmd/wydclient`.
- Servidor: não aplicável; esta unidade ainda não atravessa transporte ou
  estado autoritativo.

## Fluxo nativo 7.48

### Entrada observável

O bootstrap solicita uma textura por nome. O loader abre o `.wyt`, encontra
`WT10`, interpreta o cabeçalho TGA e entrega o buffer ao caminho nativo de
criação de textura. No caminho não-DDS, `FUN_004B8333` remove `WT10` e
completa a assinatura TGA esperada pelo consumidor gráfico.

### Callers

- `FUN_004B7BE5` inicializa grupos de textura.
- `FUN_004B827F` carrega `UI\\UITextureList.txt`.
- `FUN_004B8928` carrega `UITextureSetList.txt`.
- O bootstrap de UI alcança `FUN_004B8333` para materializar a textura.

### Função principal

`FUN_004B8333` é a fronteira nativa do recurso: valida o tipo de imagem,
normaliza o buffer WT10/TGA e chama a criação de textura.

### Callees

- `FUN_00560EAB` encapsula a criação/upload da textura.
- `FUN_00560838` é o caminho gráfico genérico consumido por
  `FUN_00560EAB`; seu layout interno não é portado.

### Saídas e erros

O caminho válido produz uma textura utilizável. Assinatura inválida, tipo TGA
não suportado, dimensão insegura, truncamento ou trailer inconsistente falham
antes da alocação/upload.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| bootstrap | janela/contexto em criação | `LoadWYTFile -> ParseWYT` | pixels RGBA8 próprios | leitura do asset | erro antes da alocação insegura |
| upload | contexto OpenGL/WGL atual | `TextureRenderer.UploadTexture` | handle de textura válido | cria recurso GPU | erro de upload aborta apresentação |
| frame inicial | textura válida | `DrawTexture` | logo apresentado | draw/blend | não usa handle inválido |
| encerramento | renderer possui textura | `Destroy` | handle liberado antes do contexto | elimina textura e janela | teardown idempotente |
| falha parcial | upload ou asset falha | retorno de erro do bootstrap | encerra sem recurso pendente | contexto destruído | erro propagado |

### Vtables, vptrs e receptores

Não há vtable nativa portada. `TextureRenderer` é uma interface Go local; o
renderer WGL é seu receptor concreto no Windows.

### Ownership

`ParseWYT` devolve pixels RGBA8 próprios. O renderer assume o handle OpenGL
depois do upload e o libera antes de destruir o contexto.

### Falha parcial

Falha de leitura, validação ou upload impede o uso da textura; o bootstrap não
continua com handle nulo.

### Cleanup e teardown

Textura, renderer, contexto e janela são destruídos em ordem inversa, na mesma
thread de UI. O asset em disco nunca é sobrescrito.

### Shutdown

`WM_CLOSE` e `Alt+F4` percorrem o encerramento normal; o smoke test confirma a
destruição da janela e do contexto.

### Logout e relogin

Não aplicável nesta unidade: ainda não existe sessão ou cena de mundo.

## Wire, ABI e recursos

Não há wire ou ABI de servidor nesta unidade. O recurso é `WT10` seguido de
TGA true-color não comprimido. Foram observados 16, 24 e 32 bits por pixel; a
saída interna é RGBA8 com orientação normalizada.

## Mapeamento atual

### Source recompilável

`ParseWYT` valida assinatura, dimensões, tipo TGA, bits por pixel, payload,
orientação e trailer. `LoadWYTFile` lê sem reescrever. O renderer WGL faz
upload, blending, desenho e liberação. O bootstrap resolve `logo1.wyt`.

### WYD-Go

Não aplicável: a textura inicial não é estado do servidor.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| prefixo/formato | `WT10` + TGA | parser seguro equivalente | comparação secundária | asset local próprio | modernizar internamente |
| pixels | device recebe TGA | RGBA8 próprio | ownership global | textura tipada | modernizar compatível |
| upload | `FUN_00560EAB -> FUN_00560838` | WGL/OpenGL separado | referência estrutural | renderer local | manter fronteira |
| teardown | recurso antes do device | textura antes do contexto | lifecycle implícito | ownership explícito | modernizar compatível |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT` no limite do recurso e
  `MODERNIZACAO_COMPATIVEL` para ownership e RGBA8.
- Não copiar código ou ABI do TMProject.
- Manter os `.wyt` oficiais somente leitura.
- Não declarar apresentação visual como `CLIENT_TESTED` sem inspeção manual.

## Lacunas

- Inspeção visual manual do logo ainda pendente.
- Texturas de cenas posteriores, reset de device e cache ainda não rastreados.
- Formato versionado, manifesto assinado e pacote criptografado pertencem à
  próxima unidade `assetc`.

## Validação

- Pesquisa: hash nativo, callgraph do loader e amostragem dos 221 assets
  oficiais registrados acima.
- Automação: testes sintéticos 16/24/32 bpp, orientação, truncamento e carga
  de `logo1.wyt`; build Windows e smoke test de janela/resize/fechamento.
- Client real: o smoke test cobre lifecycle, mas inspeção visual do conteúdo do
  logo permanece pendente.
