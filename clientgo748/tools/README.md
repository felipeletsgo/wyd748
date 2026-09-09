# Ferramentas locais

Todas as ferramentas necessárias para consultar e validar o mapeamento ficam
neste diretório.

- `research/query_corpus.py`: busca funções, callers textuais e referências no
  corpus local;
- `research/triage_catalog.py`: ordena a fila sem promover maturidade;
- `research/validate_research.py`: valida as fichas locais;
- `research/inventory_corpus.py`: reconstrói o censo combinado usando somente
  o corpus, o catálogo Ghidra, as fichas e o snapshot TMProject locais;
- `research/build_research_queue.py`: reconstrói a fila de pesquisa local;
- `research/ExportWyd*.java`: exports reproduzíveis para uso no Ghidra;
- `Update-Manifest.ps1`: recria o manifesto SHA-256 do pacote, ignorando
  artefatos transitórios de execução e build e normalizando apenas quebras de
  linha de arquivos textuais;
- `Test-Manifest.ps1`: verifica hashes, arquivos ausentes e arquivos não
  registrados, aplicando a mesma exclusão de artefatos transitórios;
- `Test-SelfContained.ps1`: rejeita links de filesystem, inputs essenciais
  ausentes, módulo Go acoplado e caminhos externos nos pontos de entrada ativos.

Execute os scripts dentro de `clientgo748`, conforme
`references/REPRODUCE.md`. As ferramentas Python recebem caminhos locais do
corpus, catálogo e fichas; elas não dependem de `.agents/` nem da raiz
`wyd-go`.

Os exports Java exigem uma instalação compatível do Ghidra apenas quando for
necessário atualizar evidência. Consultar o corpus, validar fichas, compilar o
client Go e conferir o manifesto não exige caminhos externos ao pacote.
