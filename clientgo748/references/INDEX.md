# Índice das referências consolidadas

Este índice evita depender de documentação espalhada fora de `clientgo748`.
Os arquivos externos originais foram preservados; as cópias abaixo são o
snapshot usado para a pesquisa deste client.

## Evidência nativa

- `../DECOMP/` contém o export de descompilação/disassembly já colocado no
  projeto.
- `../CLIENT OFICIAL 7.48/` contém os recursos e o executável stock oficial.
- O binário Ghidra de referência e seus fingerprints são registrados em
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

## Método

As regras de maturidade, procedência, Ghidra, UI, assets e auditoria estão em
`skills/`. Os arquivos foram incluídos para que a próxima frente possa ser
reproduzida dentro do projeto sem consultar um caminho oculto.
