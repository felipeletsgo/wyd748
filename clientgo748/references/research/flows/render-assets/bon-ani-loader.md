---
id: bon-ani-loader
title: BON/ANI asset loading for the Go 7.48 client
subsystem: render-assets
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-10
---

# BON/ANI asset loading for the Go 7.48 client

## Pergunta

Como materializar o catálogo `BoneAni4.txt`, os buffers `.bon`, o
`ValidIndex.bin` e as matrizes `.ani` no `clientgo748` sem copiar ABI, ponteiros
ou ownership do TMProject?

## Fronteira de evidência

- Mode: `PARIDADE_NATIVA` para a fronteira BON/lifecycle já comprovada no
  client 7.48; `MODERNIZACAO_COMPATIVEL` para ownership, validação e modelo Go.
- Status: `CONTRACT` para carga do catálogo/BON e integração do recurso. A
  interpretação byte a byte dos formatos BON/ANI usa diretamente a semântica
  do parser do TMProject, aceita nesta campanha porque esse parser já foi
  exercitado com os assets/client 7.48 e apresentou compatibilidade.
- Scope: parsing e materialização em memória. Seleção de motion, lookup de
  weapon/class, interpolação, quaternion, skinning e render da pose são a
  próxima unidade e não são `CLIENT_TESTED` aqui.

## Procedência

- Native 7.48/Ghidra: `UTILIZADA`; `FUN_004B00E4` e a ficha
  `transport/bone-animation-array-probe.md` comprovam bootstrap, ownership do
  buffer BON, tamanho bruto, uso pelo protocolo e teardown do catálogo.
- Assets oficiais 7.48: `UTILIZADA`; `BoneAni4.txt`, `ValidIndex.bin`, `.bon` e
  `.ani` reais são consumidos pelos golden/full-catalog tests.
- TMProject748: `UTILIZADA`; `MeshManager::InitBoneAnimation` é a fonte direta
  para a tradução/interpretação dos formatos e para a tolerância a ANI ausente.
- `clientgo748` e testes: `UTILIZADA`; o parser Go adiciona validação de limites,
  overflow, truncamento e ownership próprio sem alterar o conteúdo aceito.
- W2PP, Secrets e Micronics: `NÃO APLICÁVEL`/excluídas.

## Contrato de arquivos

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

## Modelo Go

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
| quaternion/skinning | fora desta ficha | existe em `TMSkinMesh`/manager | pendente | próxima unidade |

## Validação

- `go test ./internal/assets ./internal/graphics/...` passa.
- O teste de catálogo completo carrega os assets ativos até o índice esparso 87.
- `ch01.bon` preserva 376 bytes e expõe 47 pares; a ANI de referência possui
  `frameCount` coerente com esses 47 frames/bones.
- O gate de integração desta unidade inclui `Verify-Fast.ps1`,
  `Verify-Contract.ps1`, `Verify-Mapping.ps1`, build Windows e
  `git diff --check`.
- `CLIENT_TESTED` permanece pendente até o renderer executar pose/skinning e a
  animação ser observada no executável Go.
