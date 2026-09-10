---
id: msh-loader
title: Native 7.48 mesh manager and MSH loading boundary
subsystem: render-assets
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Native 7.48 mesh manager and MSH loading boundary

## Pergunta

Qual é o contrato observável do inicializador de meshes e qual framing interno
o client nativo usa para ler um arquivo `.msh`?

## Fronteira de evidência

- Mode: `PARIDADE_NATIVA`
- Status: `LOCATED`
- Scope: inicialização do gerenciador, tabelas de nomes e dependência de
  `object.bin`. O framing dos registros `.msh`, seus vértices, índices,
  materiais e lifecycle gráfico ainda não está fechado.

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

### Funções e relações

| Função | Papel confirmado |
| --- | --- |
| `FUN_0042c51b` | criação do manager, erro e chamada do método virtual de inicialização |
| `FUN_004afac0` | construtor; zera tabelas e instala vtable |
| `FUN_004afbc6` | inicializador, `MeshList.txt`, criação da tabela de nomes |
| `FUN_004afca2` | validação/decodificação de `object.bin` |
| `FUN_004c151c` | criação de entradas de recurso de mesh com ownership do manager |
| `FUN_004be7b1` / `FUN_004bead1` | seleção de nomes `.msh` e `.wyt` por personagem/parte |
| `FUN_004b93c4` | carregamento da lista `MeshTextureList.txt` |

## Estado e lifecycle

| Evento | Estado resultante | Erro/saída |
| --- | --- | --- |
| criar manager | objeto alocado e tabelas zeradas | alocação nula retorna falha |
| inicializar manager | tabela de meshes preenchida e `object.bin` validado | mensagem de erro e retorno zero |
| selecionar mesh de personagem | nome `.msh` derivado do estado do personagem | regras especiais ainda não generalizadas |
| destruir/relogar | não comprovado nesta ficha | teardown de vtable ainda pendente |

## Lacunas

As funções extraídas mostram nomes e a fronteira de inicialização, mas não
mostram a leitura binária de um `.msh`. Não há, nesta evidência, tamanho de
header, contagem de vértices/índices, packing, signedness, offsets, materiais,
texturas ou ownership dos buffers. O tamanho observado de `ag010101.msh` não é
um contrato. Um parser baseado somente no TMProject, em inspeção superficial
ou em um único asset seria uma hipótese e poderia corromper o renderer.

## Procedência

- Native 7.48/Ghidra: `UTILIZADA`; fonte primária para a fronteira acima.
- Assets oficiais 7.48 (`MeshList.txt`, `MeshTextureList.txt`, `.msh`):
  `UTILIZADA`; confirma nomes e disponibilidade, não o framing binário.
- Source/testes atuais do `clientgo748`: `UTILIZADA`; não existe parser MSH
  ativo e a lacuna está preservada.
- TMProject 7.69+: `UTILIZADA` somente como comparação de nomenclatura;
  não fornece ABI ou framing 7.48.
- Guias: `NÃO APLICÁVEL` para o framing não comprovado.
- W2PP, Secrets e Micronics: `NÃO APLICÁVEL`/excluídas.

## Wire, ABI e recursos

O recurso é local e não atravessa o wire. O framing observado no loader do
TMProject é compatível com os assets oficiais conferidos, mas continua sendo
uma comparação secundária até que o leitor nativo equivalente seja localizado.
Os oito primeiros campos são little-endian `uint32`; após eles vêm paleta,
nomes, vértices de stride variável e índices `uint16`.

## Mapeamento atual

### Source recompilável

Não há alteração na source nativa nesta unidade.

### WYD-Go

O parser seguro é uma modernização interna baseada no framing observado no
TMProject e validada contra assets 7.48; ele não reivindica identidade de ABI
ou de ownership com o executável nativo.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| inicialização do manager | confirmado | não aplicável | comparação secundária | não portada | manter pendente |
| tabela `MeshList.txt` | confirmado | não aplicável | comparação secundária | não portada | manter pendente |
| framing `.msh` | não localizado no corpus | parser ausente | confirmado no código candidato | parser seguro com golden asset | modernizar internamente, sem claim de paridade |

## Decisão

`proteger como pendente`: manter os assets MSH disponíveis e não criar um
parser/paridade falsa até localizar o método nativo que lê o arquivo. A próxima
pesquisa deve seguir os callers de `FUN_004c15fc`, `FUN_004c167b` e os slots da
vtable de `FUN_004c151c`, além das referências de leitura binária associadas ao
manager.

## Decisões

- O framing observado no TMProject é usado apenas como candidato de
  modernização interna, porque fecha exatamente o asset oficial conferido.
- O parser Go rejeita truncamento, contagens absurdas, faces incompletas e
  bytes residuais; ele não assume ownership ou ABI do executável nativo.
- A maturidade nativa permanece `LOCATED` até que o leitor equivalente seja
  localizado e seu fluxo de recursos seja rastreado no Ghidra.

## Validação

Esta ficha é pesquisa/documentação; não altera o executável nem o contrato do
servidor. O validador de fichas deve aceitar `LOCATED` e manter a unidade fora
de `CONTRACT`/`CLIENT_TESTED` até o framing ser comprovado.
