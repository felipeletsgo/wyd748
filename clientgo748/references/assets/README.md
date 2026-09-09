# Mapa operacional de assets

Este diretório é a camada orientada ao produto sobre os assets oficiais do
client Go. O censo não substitui `references/catalog/` nem a pesquisa Ghidra:
ele responde quais famílias existem, quanto ocupam e qual cadeia deve ser
fechada em seguida.

`asset-map.tsv` é gerado por:

```powershell
python .\clientgo748\tools\assets\build_asset_map.py
```

`asset-dependencies.tsv` só recebe relações com evidência registrada. Uma
família com arquivos presentes permanece `UNKNOWN` até que seu loader,
dependências e consumidor estejam rastreados. Não inferir `MSH → WYT`,
`ANI → BON` ou relações de efeitos apenas pela extensão; registrar a prova
nativa, do recurso ou do runtime antes de promover o estado.

O runtime não importa nada deste diretório. Ele recebe modelos canônicos de
`internal/assets` e os assets protegidos são empacotados somente depois que os
formatos e seus contratos estiverem estáveis.
