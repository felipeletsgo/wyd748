# Guias locais de pesquisa e implementação

Os arquivos deste diretório são as cópias autocontidas dos guias usados para
mapear e reimplementar o client. Os três pontos de entrada ativos são:

- `wyd-client748-catalog-SKILL.md` para censo e priorização;
- `wyd-client748-research-SKILL.md` para evidência nativa e contratos;
- `wyd-go-feature-SKILL.md` para implementação incremental em Go.

Para conversão e consumo de assets, o ponto de entrada adicional é
`client-assets/SKILL.md`.

Referências como `ghidra-client748.md`, `client-ui-748.md`,
`evidence-record.md` e `subsystem-map.md` são lidas sob demanda. Os comandos
ativos usam `tools/research/`, e as fichas vivem em
`references/research/flows/`.

`repository-contracts.md`, `client748-AGENTS.md` e
`original-client748-skill-tree/` são snapshots de regras anteriores. Eles
podem mencionar caminhos externos antigos, mas não são instruções operacionais
do client Go. Consulte `../LOCAL_PATHS.md` para resolver toda citação histórica
para a cópia local.
