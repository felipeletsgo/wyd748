---
id: msh-loader
title: Native 7.48 mesh manager and MSH loading boundary
subsystem: render-assets
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-10
---

# Native 7.48 mesh manager and MSH loading boundary

## Pergunta

Qual é o contrato observável do inicializador de meshes e qual framing interno
o client nativo usa para ler um arquivo `.msh`?

## Fronteira de evidência

- Mode: `PARIDADE_NATIVA` para framing/recurso; `MODERNIZACAO_COMPATIVEL` para
  validação e ownership do modelo Go.
- Status: `CONTRACT` para framing, buffers de geometria e teardown do cache.
- Scope: inicialização do gerenciador, leitura binária `.msh`, cache e
  lifecycle dos buffers. Material/textura, normals/UV, BON/ANI, pose, câmera e
  aparência final ainda não estão fechados.

O executável analisado é `references/ghidra/input/WYD.exe`, SHA-256
`8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0042c51b` cria o objeto do Mesh Manager (`operator_new(0x69a68)`), chama
`FUN_004afbc6` para inicializá-lo e, em sucesso, invoca o método virtual no
slot `+8`. Falha na inicialização emite `Initialize Mesh Manager Failed` e
retorna zero.

`FUN_004afbc6` chama primeiro o inicializador de Bone Animation pelo vtable
`+4`, abre `MeshList.txt` com `FUN_0058f716`, lê linhas no formato
`%d %s`, grava cada nome em uma tabela com stride `0x58` e finaliza com
`FUN_004afca2`.

`FUN_004afca2` abre `object.bin`, decodifica exatamente `0x80000` bytes com uma
chave repetida (`DAT_005b98d8`), calcula o checksum sobre blocos de 16 bytes e
compara o valor final armazenado no arquivo. Divergência mostra
`Object_bin_File_Crashed`/`CheckSum_Error` e rejeita a inicialização.

O leitor binário real de `.msh` é `FUN_004c097c`. Ele abre o recurso com
`FUN_0058fb07`, lê por `FUN_0058f8c9` (que alcança `ReadFile` em
`FUN_0058f92e`) e fecha com `FUN_0058f75f` no caminho de sucesso. Falha de
abertura marca a entrada cacheada como inválida e retorna zero.

O loader faz oito leituras sequenciais de 4 bytes antes das regiões variáveis.
Os usos nativos comprovados são:

| Campo | Destino nativo | Uso comprovado |
| --- | --- | --- |
| 0 | local temporário | semântica ainda não promovida |
| 1 | `param_1+0x08` | preservado no cache do manager |
| 2 | objeto `+0x10` | passado à criação do vertex buffer; nome semântico segue secundário |
| 3 | objeto `+0x0C` | stride do vértice |
| 4 | `param_1+0xB8` | preservado no cache; semântica detalhada ainda não promovida |
| 5 | `param_1+0x0C` | quantidade de entradas de paleta/bone IDs |
| 6 | objeto `+0x28` | quantidade de vértices |
| 7 | objeto `+0x20` | quantidade de índices; face count = `indexCount/3` |

Após o header, o nativo lê `paletteCount*64` bytes para a paleta,
`paletteCount*4` bytes para IDs de bone, `vertexStride*vertexCount` bytes para
o vertex buffer e `indexCount*2` bytes para o index buffer de 16 bits. O loop
de bounds lê `float` nos offsets 0, 4 e 8 de cada vértice, confirmando XYZ como
o primeiro subconjunto de layout utilizável sem inferência.

### Callers

- `FUN_004c0052` chama diretamente `FUN_004c097c`; no sucesso continua a
  resolução do parent node e associa o objeto carregado ao nó correspondente.
- `FUN_004bf8c5` chama diretamente `FUN_004c097c` no fluxo equivalente de
  materialização e também só prossegue com a associação quando o loader
  retorna sucesso.

O corpus atual contém esses dois callers textuais diretos de `FUN_004c097c`.
Chamadas indiretas adicionais não são inferidas por ausência de token textual.

### Função principal

`FUN_004c097c` é o loader/cache boundary desta ficha: resolve ou cria o slot do
mesh, abre o `.msh`, materializa as regiões variáveis, cria/abastece os buffers
gráficos, calcula bounds XYZ e publica os ponteiros/contagens no cache.

### Callees

- `FUN_004b15cc` consulta o slot de cache e `FUN_004b163b` obtém um slot livre.
- `FUN_004baf10` constrói o objeto de mesh quando o slot ainda não possui um.
- `FUN_0058f078` registra/copia o nome do recurso na entrada do cache.
- `FUN_0058fb07`, `FUN_0058f8c9` e `FUN_0058f75f` formam o caminho
  open/read/close comprovado do arquivo.
- As chamadas virtuais do dispositivo nos slots `+0x5c`/`+0x60` criam os
  vertex/index buffers; os slots `+0x2c`/`+0x30` fazem lock/unlock. Esta ficha
  registra os papéis observáveis sem atribuir nomes nativos não demonstrados
  aos destinos indiretos.

### Funções e relações

| Função | Papel confirmado |
| --- | --- |
| `FUN_0042c51b` | criação do manager, erro e chamada do método virtual de inicialização |
| `FUN_004afac0` | construtor; zera tabelas e instala vtable |
| `FUN_004afbc6` | inicializador, `MeshList.txt`, criação da tabela de nomes |
| `FUN_004afca2` | validação/decodificação de `object.bin` |
| `FUN_004c097c` | leitor binário real de `.msh`, criação dos buffers e cache/reuse |
| `FUN_0058fb07` / `FUN_0058f8c9` / `FUN_0058f75f` | open/read/close do arquivo |
| `FUN_004c151c` | criação de entradas de recurso de mesh com ownership do manager |
| `FUN_004be7b1` / `FUN_004bead1` | seleção de nomes `.msh` e `.wyt` por personagem/parte |
| `FUN_004b93c4` | carregamento da lista `MeshTextureList.txt` |
| `FUN_004afe59` | teardown de até `0x400` slots, buffers auxiliares e objetos de mesh |
| `FUN_004afba0` / `FUN_0042d66e` | destruição do manager e shutdown global de recursos |

## Estado e lifecycle

| Evento | Estado resultante | Erro/saída |
| --- | --- | --- |
| criar manager | objeto alocado e tabelas zeradas | alocação nula retorna falha |
| inicializar manager | tabela de meshes preenchida e `object.bin` validado | mensagem de erro e retorno zero |
| selecionar mesh de personagem | nome `.msh` derivado do estado do personagem | regras especiais ainda não generalizadas |
| destruir/shutdown | `FUN_004afe59` libera objeto, paleta e IDs e zera ponteiros | chamado pelo destrutor do manager e shutdown global |

## Lacunas

O framing, tamanhos das regiões, XYZ, índices e teardown estão fechados nesta
fronteira. A interpretação dos layouts de vértice usada pelo Go segue o parser
do TMProject748 já validado com assets 7.48 e possui validação automatizada
própria; isso não promove um novo claim nativo desta ficha. Material/textura,
câmera, integração do personagem e aparência final ainda exigem suas fronteiras
específicas antes de `CLIENT_TESTED`.

## Procedência

- Native 7.48/Ghidra: `UTILIZADA`; fonte primária para a fronteira acima.
- Assets oficiais 7.48 (`MeshList.txt`, `MeshTextureList.txt`, `.msh`):
  `UTILIZADA`; confirma nomes e disponibilidade, não o framing binário.
- Source/testes atuais do `clientgo748`: `UTILIZADA`; `ParseMSH` e o golden
  asset foram conferidos contra o framing nativo.
- TMProject748: `UTILIZADA` como referência direta da interpretação dos layouts
  de vértice/pesos/índices de palette já exercitada com 7.48; continua não sendo
  usado para inventar offsets, framing ou lifecycle nativos desta ficha.
- Guias: `NÃO APLICÁVEL` para o framing não comprovado.
- W2PP, Secrets e Micronics: `NÃO APLICÁVEL`/excluídas.

## Wire, ABI e recursos

O recurso é local e não atravessa o wire. O loader nativo confirma oito campos
DWORD seguidos por paleta, IDs de bone, vértices de stride variável e índices
de 16 bits. O modelo Go usa `uint32`/little-endian para o framing e ownership
próprio de slices; não replica ponteiros, COM ou lifetime do ABI nativo.

## Mapeamento atual

### Source recompilável

Não há alteração na source nativa nesta unidade.

### WYD-Go

O parser seguro preserva o framing nativo, mas usa ownership Go. A fronteira
gráfica decodifica os cinco layouts observados no catálogo atual, incluindo
posição, normal, UV, pesos e índices de palette, após validar FVF, stride,
influências, tamanho dos buffers, pesos e faixa de índices.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| inicialização do manager | confirmado | não aplicável | comparação secundária | não portada | manter pendente |
| tabela `MeshList.txt` | confirmado | não aplicável | comparação secundária | não portada | manter pendente |
| framing `.msh` | `FUN_004c097c` confirmado | parser ativo | comparação secundária | parser seguro com golden asset | `PARIDADE_NATIVA` de framing + ownership Go |
| geometria/layout | XYZ + índices confirmados | extração canônica | interpretação dos cinco layouts validada no 7.48 | atributos tipados e validações próprias | `MODERNIZACAO_COMPATIVEL` sobre framing comprovado |
| palette/skinning | fora do claim desta ficha | não aplicável | `LinkBones`/`UpdateFrames`/`CMesh` usados como semântica | hierarchy + CPU skinning testados | implementar sem promover claim nativo novo |
| teardown | cache libera objeto + 2 blocos auxiliares | GC + recursos gráficos próprios | comparação secundária | sem ponteiros nativos | `MODERNIZACAO_COMPATIVEL` |

## Decisão

Manter `ParseMSH` como parser canônico do framing e usar a interpretação dos
layouts do TMProject748 já validada com 7.48 para a visão tipada do renderer.
Combinações FVF/stride/influência não observadas continuam rejeitadas. O WGL
preserva o caminho estático e possui caminho de CPU skinning; câmera,
material/textura, integração de personagem e aparência final permanecem
pendentes.

## Decisões

- O framing agora é claim nativo `CONTRACT`; os nomes semânticos não diretamente
  provados continuam tratados como nomenclatura secundária.
- O parser Go rejeita truncamento, contagens absurdas, stride menor que XYZ,
  faces incompletas e bytes residuais; ele não assume ownership/ABI nativo.
- `graphics.ExtractMeshGeometry` rejeita tamanho inconsistente e índice fora da
  faixa antes de qualquer draw.
- `CLIENT_TESTED` não foi alcançado: ainda não existe observação real do mesh
  com câmera/material decididos no executável Go.

## Validação

Gate esperado desta unidade: parser/golden e testes focados de geometria verdes,
`Verify-Contract.ps1`, `Verify-Mapping.ps1`, build Windows e `git diff --check`.
Isso permite `CONTRACT`/`AUTOMATED_TESTED`, mas não `CLIENT_TESTED`.
