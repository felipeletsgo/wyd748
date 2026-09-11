---
id: bon-ani-loader
title: BON/ANI asset loading for the Go 7.48 client
subsystem: render-assets
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-10
---

# BON/ANI asset loading for the Go 7.48 client

## Pergunta

Como materializar o catálogo `BoneAni4.txt`, os buffers `.bon`, o
`ValidIndex.bin` e as matrizes `.ani` no `clientgo748` sem copiar ABI, ponteiros
ou ownership do TMProject?

## Fronteira de evidência

- Mode: `PARIDADE_NATIVA` somente para a fronteira nativa já localizada;
  `MODERNIZACAO_COMPATIVEL` para ownership, validação e modelo Go.
- Status nativo: `LOCATED`. `FUN_004B00E4` está identificada como loader do
  catálogo de animação, mas callers/callees e a transição completa do loader
  ainda não foram fechados nesta ficha. A implementação Go não depende de
  promover artificialmente essa maturidade.
- A interpretação byte a byte dos formatos BON/ANI usa diretamente a semântica
  do parser do TMProject748, aceita nesta campanha porque esse parser já foi
  exercitado com os assets/client 7.48 e apresentou compatibilidade.
- Scope: parsing e materialização em memória, mais a continuação local já
  implementada para hierarquia BON, palette, CPU skinning e amostragem básica
  de clip ANI. Seleção de motion/weapon/class, transição especial entre clips,
  quaternion/slerp e render da pose no personagem permanecem fora do claim
  `CLIENT_TESTED`.

## Procedência

- Native 7.48/Ghidra: `UTILIZADA`; `FUN_004B00E4` e a ficha
  `transport/bone-animation-array-probe.md` comprovam bootstrap, ownership do
  buffer BON, tamanho bruto, uso pelo protocolo e teardown do catálogo.
- Assets oficiais 7.48: `UTILIZADA`; `BoneAni4.txt`, `ValidIndex.bin`, `.bon` e
  `.ani` reais são consumidos pelos golden/full-catalog tests.
- TMProject748: `UTILIZADA`; `MeshManager::InitBoneAnimation` é a fonte direta
  para a tradução/interpretação dos formatos e para a tolerância a ANI ausente;
  `TMSkinMesh::FrameMove`, `RestoreDeviceObjects`, `CFrame::UpdateFrames` e
  `CMesh` fornecem a semântica já validada no 7.48 para hierarquia, ordem das
  matrizes, palette, quatro substeps e interpolação básica dentro do clip.
- `clientgo748` e testes: `UTILIZADA`; o parser Go adiciona validação de limites,
  overflow, truncamento e ownership próprio sem alterar o conteúdo aceito.
- W2PP, Secrets e Micronics: `NÃO APLICÁVEL`/excluídas.

## Fluxo nativo 7.48

`FUN_004B00E4 @ 0x004B00E4` é a raiz nativa já localizada para a
materialização do catálogo de animações. A ficha
`transport/bone-animation-array-probe.md` comprova que esse bootstrap conserva
os buffers `.bon` brutos no manager e que eles permanecem disponíveis para o
fluxo `0x1C1/0x2C2`.

Esta ficha não promove o loader para `TRACED`: o caller de bootstrap e o
callgraph completo específico de `FUN_004B00E4` não foram resolvidos aqui.
Também não se usa a ausência de caller textual no catálogo como prova de código
morto ou de entrada direta.

## Estado e lifecycle

O estado nativo confirmado e reutilizado é estreito: o catálogo/BON é
materializado durante bootstrap, permanece sob ownership do `MeshManager` e é
consumido como recurso global durante a vida do processo. A ficha do probe
registra teardown por `NewApp::Finalize`, incluindo os buffers BON e matrizes de
animação.

O lifecycle completo do loader — entrada de bootstrap, falha parcial por asset,
ordem de inicialização dos consumidores e todos os caminhos de teardown — ainda
não foi rastreado nesta ficha. Por isso a maturidade permanece `LOCATED`.

## Wire, ABI e recursos

Não há wire novo nesta unidade do `clientgo748`, nem ABI C++ copiada. O contrato
relevante é o formato dos recursos e a semântica de materialização em memória.

### Contrato de arquivos

`BoneAni4.txt` fornece `index`, quantidade de entradas de animação, partes e o
basename do conjunto. `ValidIndex.bin` contém `100 * 186` inteiros little-endian
de 32 bits e é opcional, como no TMProject.

Cada `<basename>.bon` é preservado integralmente. Os pares completos são lidos
de oito em oito bytes como `ParentID`/`BoneID`, mas `Raw` mantém inclusive bytes
residuais. Isso preserva o contrato já observado no 7.48 em que o tamanho bruto
do BON é relevante e não pode ser reconstruído por `numBone*8`.

Cada ANI começa por `tickCount` e `frameCount` (`uint32` little-endian), seguido
por `tickCount*frameCount` matrizes 4x4 de `float32`. O loader acumula as
matrizes dos clips existentes e registra o offset inicial de cada clip. ANI
ausente é ignorado; catálogo/BON ausente interrompe a carga.

## Mapeamento atual

### Modelo Go

O modelo não porta `malloc`, ponteiros `D3DXMATRIX`, COM nem arrays globais do
TMProject. `BoneFile` possui o payload bruto e uma visão de pares; `AnimationFile`
possui header e matrizes; `BoneAnimation` agrega clips e matriz contínua; e
`BoneAnimationSet` mantém os cem slots do catálogo e os índices válidos.

As validações locais rejeitam catálogo fora dos limites, duplicidade de índice,
`ValidIndex.bin` com tamanho divergente, ANI truncado, bytes residuais em ANI,
dimensões que excedam a capacidade endereçável e mudança incoerente de
`frameCount` entre clips do mesmo conjunto.

## Matriz de delta

| Claim | 7.48/Ghidra | TMProject748 | Go | Decisão |
| --- | --- | --- | --- | --- |
| catálogo/BON e lifecycle | comprovado | implementação compatível | implementado | preservar contrato 7.48 |
| tamanho bruto BON | comprovado | preservado por `numBoneBytes` | `BoneFile.Raw` | `MODERNIZACAO_COMPATIVEL` |
| framing/parse ANI | integração nativa delimitada | parser validado no 7.48 | parser seguro | portar sem ABI C++ |
| ANI ausente | não muda contrato externo | skip | skip | manter semântica do parser |
| hierarquia/palette/skinning | integração nativa ainda não promovida nesta ficha | semântica validada no 7.48 | implementado e testado | `MODERNIZACAO_COMPATIVEL`; sem claim nativo novo |
| playback ANI dentro do clip | integração nativa ainda não promovida nesta ficha | 30 ms, 4 substeps, wrap e blend linear | implementado e testado | manter separado da transição especial |
| transição entre clips/quaternion | fora desta ficha | existe em `TMSkinMesh::FrameMove` | pendente | próxima unidade |

## Decisões

- Manter a maturidade nativa desta ficha em `LOCATED` até o callgraph/lifecycle
  específico de `FUN_004B00E4` ser fechado; não inventar callers para satisfazer
  o schema.
- Reutilizar diretamente no Go a tradução/interpretação dos formatos MSH/BON/ANI
  do TMProject748 quando já validada com assets/client 7.48 pelo usuário,
  acrescentando validações e ownership idiomáticos de Go.
- Não copiar ABI, ponteiros, COM, estado global ou ownership C++ do TMProject.
- Manter parsing/playback automatizado separado da alegação de paridade nativa e
  de `CLIENT_TESTED`.

## Lacunas

- Resolver no Ghidra os callers/callees e o lifecycle completo de
  `FUN_004B00E4` antes de promover esta ficha a `TRACED`/`CONTRACT`.
- Portar separadamente a transição especial entre clips, incluindo estado da
  animação anterior, janela de transição e quaternion/slerp quando aplicável.
- Integrar pose/skinning/playback a um personagem real no renderer Go e validar
  visualmente antes de `CLIENT_TESTED`.

## Validação

- `go test ./internal/assets ./internal/graphics/...` passa.
- O teste de catálogo completo carrega os assets ativos até o índice esparso 87.
- `ch01.bon` preserva 376 bytes e expõe 47 pares; a ANI de referência possui
  `frameCount` coerente com esses 47 frames/bones.
- Testes de graphics cobrem raiz sintética versus BON 0, ordem D3DX
  `local * parent`, palette `bind * combined`, pesos de 1..4 influências,
  seleção de tick, substeps/interpolação, wrap e estados ANI inválidos.
- O gate de integração desta unidade inclui `Verify-Fast.ps1`,
  `Verify-Contract.ps1`, `Verify-Mapping.ps1`, build Windows e
  `git diff --check`.
- `CLIENT_TESTED` permanece pendente até um personagem real consumir
  pose/skinning/playback e a animação ser observada no executável Go.
