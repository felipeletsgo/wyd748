# Ferramentas locais

Todas as ferramentas necessárias para consultar e validar o mapeamento ficam
neste diretório.

- `research/query_corpus.py`: busca funções, callers textuais e referências no
  corpus local;
- `research/triage_catalog.py`: ordena a fila sem promover maturidade;
- `research/validate_research.py`: valida as fichas locais;
- `research/ExportWyd*.java`: exports reproduzíveis para uso no Ghidra;
- `Update-Manifest.ps1`: recria o manifesto SHA-256 do pacote;
- `Test-Manifest.ps1`: verifica hashes, arquivos ausentes e arquivos não
  registrados.

Execute os scripts a partir da raiz `wyd-go`, conforme
`references/REPRODUCE.md`. As ferramentas Python descobrem o corpus e as
fichas dentro de `clientgo748`; elas não dependem mais de `.agents/`.

Os exports Java exigem uma instalação compatível do Ghidra apenas quando for
necessário atualizar evidência. Consultar o corpus, validar fichas, compilar o
client Go e conferir o manifesto não exige caminhos externos ao pacote.
