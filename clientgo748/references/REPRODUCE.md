# Reprodução do mapeamento

Execute na raiz do repositório (`wyd-go`). Os resultados são diagnósticos; não
altere binários históricos.

```powershell
python .agents/skills/wyd-client748-research/scripts/query_corpus.py stats --repo .
python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
python .agents/skills/wyd-client748-research/scripts/validate_research.py --repo .
go test ./clientgo748/...
pwsh -NoProfile -File .\clientgo748\Build-ClientGo.ps1 -Configuration Debug
git diff --check
```

O triador só deve ser executado novamente quando o corpus, a fila ou uma nova
raiz mudarem. O validador de pesquisa só precisa ser executado quando uma ficha
ou seu schema mudar. O build do client Go não prova paridade visual nem
`CLIENT_TESTED`.

## Próximo lote recomendado

Começar pelas entradas `DOCUMENTED_FIRST` ligadas ao caminho crítico e fechar
uma transição por vez. Para cada lote, registrar no mesmo ciclo a ficha,
procedência, decisão, teste e o próximo símbolo. Não fazer uma tradução linear
dos 4.146 pseudocódigos.
