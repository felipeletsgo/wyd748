# Reprodução do mapeamento

Execute dentro de `clientgo748`. Os resultados são diagnósticos; não altere
binários históricos.

```powershell
python .\tools\research\query_corpus.py --corpus .\references\ghidra\corpus stats --repo .
python .\tools\research\triage_catalog.py --input .\references\catalog\functions.tsv --binary .\references\ghidra\input\WYD.exe --format summary
python .\tools\research\validate_research.py --flows .\references\research\flows
python -m unittest discover -s .\tools\research -p "test_*.py" -v
go test ./...
pwsh -NoProfile -File .\tools\Test-SelfContained.ps1
pwsh -NoProfile -File .\tools\Test-Manifest.ps1
pwsh -NoProfile -File .\Build-ClientGo.ps1 -Configuration Debug
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
