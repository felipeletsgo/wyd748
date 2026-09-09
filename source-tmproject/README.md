# Client source

Esta arvore contem o trabalho de reconstrucao do client usado pelo WYD-Go.

```text
client748/                 assets ativos e project.exe candidato
client-source/tmproject/   source exclusiva do client 7.48
```

O client ativo deve ser produzido somente por esta source e instalado como
`client748/project.exe`. Os binarios e scripts em
`client748/wyd.exe nativo+patches/` sao referencias historicas read-only para
Ghidra; nao fazem parte do build, da instalacao ou da validacao.

## Primeiro marco

O baseline importado usa DirectX 9, Win32 e `STRUCT_SCORE` de 48 bytes. O
WYD-Go continuara persistindo e calculando exclusivamente com `Score`;
o score do client e apenas uma projecao de apresentacao e protocolo.

Para compilar no Windows:

```powershell
cd client-source/tmproject
.\Build-Client.ps1
```

O resultado transitorio fica em
`client-source/tmproject/build/Release/WYD.exe` e nao deve ser versionado.
Todo build bem-sucedido instala automaticamente esse resultado como
`client748/project.exe`, confere que os hashes coincidem e informa o hash do
candidato instalado. Falha na instalação também falha o fluxo de build.

Para um smoke test do `project.exe` instalado usando diretamente os assets do
`client748/`, sem tocar nos executáveis históricos:

```powershell
cd client-source/tmproject
.\Run-Client748.ps1 -Build
```

O runner serve apenas para smoke test local da mesma source; a aceitacao deve
usar o `client748/project.exe` instalado. Ele valida os formatos conhecidos e
define `WYD_ASSET_ROOT` para o novo executavel. A validacao tambem exige os shaders DirectX 9 trazidos do runtime
oficial do TMProject; sem eles o renderer termina com `Initialize Render
Failed`. Use `-VerifyOnly` para executar apenas a verificacao estatica.

Consulte `tmproject/WYD748_PORT.md` antes de alterar packets ou estruturas.
Toda semantica encontrada nos binarios historicos deve ser adaptada nesta
source para o contrato 7.48; nenhum patch pode ser aplicado ao candidato.
