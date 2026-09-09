# Índice das referências consolidadas

Este índice evita depender de documentação espalhada fora de `clientgo748`.
Os arquivos externos originais foram preservados; as cópias abaixo são o
snapshot usado para a pesquisa deste client.

Os caminhos originais preservados em fichas e snapshots são traduzidos em
[`LOCAL_PATHS.md`](LOCAL_PATHS.md); nenhum deles é uma dependência operacional.

## Evidência nativa

- `../DECOMP/` contém o export de descompilação/disassembly já colocado no
  projeto.
- `../CLIENT OFICIAL 7.48/` contém os recursos e o executável stock oficial.
- `ghidra/input/WYD.exe` é o binário analisado, e `ghidra/project/` contém o
  projeto Ghidra completo. Seus fingerprints são registrados em
  [`../MAPPING.md`](../MAPPING.md) e em `skills/ghidra-client748.md`.

## Catálogo e callgraph

- `catalog/functions.tsv`: censo de 4.146 entradas e estado atual.
- `catalog/ghidra-functions.tsv`: catálogo produzido a partir do Ghidra.
- `catalog/research-queue.tsv`: triagem priorizada por documentação, fan-out,
  callbacks e corpus.

## Pesquisa por transição

- `research/flows/`: fichas nativas, agrupadas em `lifecycle`, `transport`,
  `ui` e `combat`.
- `research/exports/`: xrefs, vtables, contratos e tabelas auxiliares.
- `research/inventory/`: README, fila e registros de inventário.

## Comparações secundárias

- `tmproject/`: documentação e snapshot read-only do TMProject748. Serve para
  nomes, arquitetura e alternativas de implementação; não é fonte de ABI,
  opcode, offset, ID de UI ou comportamento nativo.
- `server-docs/`: documentação do emulador e guias de gameplay.
- `server-source/`: cópia somente de `wire`, `game` e `model` usados para
  conferir contratos server-authoritative. Os arquivos Go estão com sufixo
  `.go.txt` de propósito: são snapshot de consulta e nunca pacotes compiláveis
  do client.
- `server-data/`: tabelas de skills, itens e NPCs relevantes.
- `legacy-tools-data/`: tabelas históricas úteis copiadas sem executáveis,
  DLLs, compactados ou patchers.
- `project-rules/`: snapshot das regras que governaram a consolidação.

## Assets ativos

- `../assets/current/`: cópia autocontida dos assets usados pela preparação do
  client Go.
- `../assets/README.md`: escopo, exclusões e regra de futura conversão.

## Método

As regras de maturidade, procedência, Ghidra, UI, assets e auditoria estão em
`skills/`. As ferramentas executáveis ficam em `../tools/`. Os arquivos foram
incluídos para que a próxima frente possa ser reproduzida dentro do projeto
sem consultar um caminho oculto.
