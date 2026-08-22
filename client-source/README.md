# Client source

Esta arvore contem o trabalho de reconstrucao do client usado pelo WYD-Go.

```text
client748/                 executavel 7.48 atual e sua cadeia de patches
client-source/tmproject/   client recompilavel usado na migracao gradual
```

O client recompilavel ainda nao substitui `client748/WYD.exe`. A migracao sera
feita por contratos byte-exatos, mantendo o servidor como unica autoridade de
gameplay.

## Primeiro marco

O baseline importado usa DirectX 9, Win32 e `STRUCT_SCORE` de 48 bytes. O
WYD-Go continuara persistindo e calculando exclusivamente com `ExtendedScore`;
o score do client e apenas uma projecao de apresentacao e protocolo.

Para compilar no Windows:

```powershell
cd client-source/tmproject
.\Build-Client.ps1
```

O resultado fica em `client-source/tmproject/build/Release/WYD.exe` e nao deve
ser versionado.

Para validar e executar o build usando diretamente os assets do `client748/`,
sem substituir o executavel distribuido:

```powershell
cd client-source/tmproject
.\Run-Client748.ps1 -Build
```

O runner valida os formatos conhecidos e define `WYD_ASSET_ROOT` para o novo
executavel. A validacao tambem exige os shaders DirectX 9 trazidos do runtime
oficial do TMProject; sem eles o renderer termina com `Initialize Render
Failed`. Use `-VerifyOnly` para executar apenas a verificacao estatica.

Consulte `tmproject/WYD748_PORT.md` antes de alterar packets ou estruturas.
