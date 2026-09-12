# MSA common/static mesh loader — WYD 7.48

Status: `CONTRACT`

## Escopo

Contrato usado pelos objetos estáticos de `Field0813.dat` na cena de seleção
de servidor/login. O objetivo desta ficha é limitar o porte do TMProject ao que
foi confirmado no binário e nos assets oficiais 7.48.

Classificação do delta: `PARIDADE_NATIVA`.

## Fontes

- Binário/descompilação nativa 7.48: **UTILIZADA (primária)**.
  - `FUN_004bc7c7`: framing e expansão de `.msa`.
  - `FUN_004b93c4`: inicialização de `MeshTextureList`.
  - `FUN_004b9bb9`: lookup de textura de modelo em 2048 registros `0x108`.
- Assets oficiais 7.48: **UTILIZADA**.
  - `mesh/MeshTextureList.bin`, `mesh/MeshList.txt`, `mesh/kswa08.msa` e
    amostras correlatas.
- Go atual (`clientgo748`): **UTILIZADA** para integração e limites de parser.
- TMProject: **UTILIZADA apenas como comparação semântica secundária**.
  - `TMMesh::LoadMsa`, `TextureManager` e `MeshManager`.
- W2PP, Secrets e Micronics: **NÃO APLICÁVEL / excluídos da campanha**.

## Contrato 7.48 confirmado

`FUN_004bc7c7` lê, nesta ordem:

1. FVF (`uint32`), stride original (`uint32`) e quantidade de attributes
   (`uint32`);
2. `attributeCount` registros `D3DXATTRIBUTERANGE` de 20 bytes;
3. um nome de textura de exatamente 11 bytes por attribute;
4. tamanho em bytes do index buffer, seguido de índices 16-bit;
5. tamanho em bytes do vertex buffer, seguido dos vértices no stride original.

FVF `0x142` é mantido. Nos demais casos o nativo soma `0x100` ao FVF e 8 ao
stride. Quando o resultado é `0x212` (`0x112 -> 0x212`), copia os floats UV dos
offsets 24/28 para os novos offsets 32/36.

`MeshTextureList.bin` possui exatamente `0x800` registros compactos de `0x108`
bytes. `FUN_004b9bb9` percorre esses 2048 registros. Para nomes serializados
como `kswa01.tga`, `LoadMsa` forma primeiro `mesh\\kswa01.wyt`; durante cada
tentativa o lookup substitui somente os três caracteres finais da extensão
pelos da linha corrente. No corpus oficial isso resolve, por exemplo,
`kswa01.tga -> mesh\\kswa01.wys` no slot 194.

Esse ponto diverge da capacidade moderna do TMProject (`MAX_MODEL_TEXTURE`
3000): para a paridade desta fronteira, o Go usa o limite nativo 2048.

## Implementação Go

- `internal/assets/msa.go`: parser estrito, buffers próprios, limites de
  contagem/tamanho, ranges validados e expansão FVF/UV igual ao nativo.
- `internal/assets/model_texture_list.go`: parser da tabela compacta 7.48 e
  resolução dos nomes `.msa` com a mesma regra de extensão do lookup nativo.

Ainda não há claim `IMPLEMENTED` para a cena estática completa: faltam o parser
de `Field0813.dat`, resolução `dwObjType -> MeshList/.msa`, transformações dos
objetos e desenho por attribute/material no WGL. Esta ficha permanece
`CONTRACT` até esses consumidores serem integrados; `CLIENT_TESTED` exige
comparação visual no client real.
