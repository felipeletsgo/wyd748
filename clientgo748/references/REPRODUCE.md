# Reprodução do mapeamento

Execute dentro de `clientgo748`. Os resultados são diagnósticos; não altere
binários históricos. O ciclo comum não precisa reconstruir toda a evidência:

```powershell
# Código ativo: testes, vet, build e whitespace.
pwsh -NoProfile -File .\Verify-Fast.ps1

# Protocolo/assets: nível rápido + manifesto, pacote e census de assets.
pwsh -NoProfile -File .\Verify-Contract.ps1

# Evidência/pesquisa: contrato + corpus, catálogo, fichas e tooling.
pwsh -NoProfile -File .\Verify-Mapping.ps1
```

Os comandos abaixo são a decomposição reproduzível da auditoria completa:

```powershell
python .\tools\research\query_corpus.py --corpus .\references\ghidra\corpus stats --repo .
python .\tools\research\triage_catalog.py --input .\references\catalog\functions.tsv --binary .\references\ghidra\input\WYD.exe --format summary
python .\tools\research\validate_research.py --flows .\references\research\flows
python -m unittest discover -s .\tools\research -p "test_*.py" -v
go test ./...
pwsh -NoProfile -File .\tools\Test-SelfContained.ps1
pwsh -NoProfile -File .\tools\Test-Manifest.ps1
pwsh -NoProfile -File .\Build-ClientGo.ps1 -Configuration Debug
pwsh -NoProfile -File .\tools\Test-ClientBootstrap.ps1
git diff --check
```

O triador só deve ser executado novamente quando o corpus, a fila ou uma nova
raiz mudarem. O validador de pesquisa só precisa ser executado quando uma ficha
ou seu schema mudar. O build do client Go não prova paridade visual nem
`CLIENT_TESTED`.

`MANIFEST.sha256` é um artefato derivado e não decide sozinho o nível do CI:
qualquer arquivo versionado do pacote pode alterar o manifesto. O nível é
escolhido pelos arquivos-fonte afetados; contrato e auditoria completa sempre
validam o manifesto integral.

`Test-ClientBootstrap.ps1` é um gate Windows do executável produzido. Ele
localiza a janela pela classe `WYDClientGo748Window` e pelo PID, confirma a
área cliente inicial, redimensiona a janela e exercita `WM_CLOSE` e Alt+F4.
Ele não substitui validação visual de cenas ou assets.

## Próximo lote recomendado

Começar pelas entradas `DOCUMENTED_FIRST` ligadas ao caminho crítico e fechar
uma transição por vez. Para cada lote, registrar no mesmo ciclo a ficha,
procedência, decisão, teste e o próximo símbolo. Não fazer uma tradução linear
dos 4.146 pseudocódigos.
