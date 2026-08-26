---
name: build-deploy
description: >-
  Build the FunctionsV02.dll plugin and deploy it to the running TMSRV server for
  this WYD 7.54 project. Use whenever you've changed anything under
  `Sourcer Star Micronics/` and need to test in-game, or when a rebuild "isn't
  taking effect." Encodes the exact MSBuild invocation for VS2026 and the
  kill-before-copy dance that prevents the stale/locked-DLL trap that has bitten
  this project repeatedly.
---

# Build & deploy the WYD plugin

This workflow is exclusively for `Sourcer Star Micronics/`. For the Go emulator
under `wyd-go/`, use `$wyd-go-feature` and build `./cmd/server`; never copy a DLL
or stop the native Micronics server as part of a WYD-Go-only change.

Goal: turn edited source under `Sourcer Star Micronics/` into a running change
on the server without confundir build, deploy e teste. O checkout Go fica em
`.../SERVER 7.54/wyd-go`; os dois projetos legados ficam ao lado dele. A falha
recorrente é testar uma DLL antiga porque `TMSRV.exe` ou um launcher de restart
continuou ativo.

## 0. Resolver alvos e preservar estado

Quando o diretório atual for `wyd-go`, resolver primeiro:

```powershell
$wydLegacyRoot = Resolve-Path -LiteralPath '..'
$pluginSolution = Join-Path $wydLegacyRoot 'Sourcer Star Micronics\FunctionsV02.sln'
$builtDll = Join-Path $wydLegacyRoot 'Sourcer Star Micronics\Build\FunctionsV02.dll'
$deployedDll = Join-Path $wydLegacyRoot 'Server Star Micronics\TMSRV\run\Plugin\FunctionsV02.dll'

@($pluginSolution, (Split-Path $deployedDll -Parent)) | ForEach-Object {
    if (-not (Test-Path -LiteralPath $_)) { throw "Artefato legado ausente: $_" }
}
```

Se o layout for diferente, localizar a solution e a pasta do servidor e então
definir caminhos absolutos equivalentes. Não procurar indefinidamente, não
copiar para um destino aproximado e não prosseguir se houver mais de um alvo
plausível. Antes de mutar runtime, registrar branch/status das sources, hash da
DLL atualmente implantada e processos ativos.

## 1. Build

Use the VS2026 MSBuild directly (there is no VS2017 on this machine; the project is retargeted to toolset v141 + Windows SDK 10.0.26100.0):

```powershell
$msbuild = 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe'
if (-not (Test-Path -LiteralPath $msbuild)) { throw "MSBuild ausente: $msbuild" }

& $msbuild $pluginSolution /t:Rebuild /p:Configuration=Build /p:Platform=Win32
if ($LASTEXITCODE -ne 0) { throw "Build falhou com exit code $LASTEXITCODE" }
if (-not (Test-Path -LiteralPath $builtDll)) { throw "DLL não produzida: $builtDll" }
```

- Exigir `0 Error(s)` e exit code zero. Warnings conhecidos podem ser
  inventariados, mas não declarar todo warning irrelevante por contagem.
- **x86 / Win32 only** — never x64; the DLL loads into a 32-bit process.
- Usar `Rebuild` para eliminar a ambiguidade do incremental quando a finalidade
  é deploy; confirmar timestamp e SHA-256 do output novo.
- If link fails with **LNK1201 (can't write FunctionsV02.pdb)**, the PDB is locked — OllyDbg is attached or TMSRV is running. Close them, then rebuild.

Output DLL: `Sourcer Star Micronics/Build/FunctionsV02.dll`.

Build verde significa apenas `STATICALLY VERIFIED`; ainda não prova que a DLL
foi implantada nem que o comportamento funciona.

## 2. Encerrar TMSRV antes de copiar

`TMSRV.exe` holds an open handle to `Plugin/FunctionsV02.dll` while running. If it's up (or crash-looping), the copy fails silently and you test the OLD dll. Kill every instance first — check for and close the `.bat` window too, or it will relaunch:

```powershell
Get-Process TMSRV -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500
if (Get-Process TMSRV -ErrorAction SilentlyContinue) {
    throw 'TMSRV ainda está ativo; interrompa o launcher de restart antes do deploy.'
}
```

Identificar e interromper também o launcher que o relança. Não copiar enquanto
o processo reaparecer. A parada do servidor é uma mutação operacional: só
executá-la quando build/deploy estiverem no escopo pedido pelo usuário.

## 3. Copy the DLL into the plugin folder

```powershell
Copy-Item -LiteralPath $builtDll -Destination $deployedDll -Force
```

Se a cópia falhar, não alterar ACL nem matar processos não relacionados por
tentativa. Resolver o handle/launcher que mantém o destino aberto.

## 4. Verificar o deploy por SHA-256

Do not trust that the copy worked — confirm the deployed bytes equal the freshly built bytes:

```powershell
$buildHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $builtDll).Hash
$deployHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $deployedDll).Hash
if ($buildHash -ne $deployHash) {
    throw "Deploy divergente: build=$buildHash deploy=$deployHash"
}
```

Registrar ambos os hashes. Igualdade prova somente que os bytes copiados são os
bytes compilados.

## 5. Restart & test

Start `DBSRV` first, then `TMSRV`, usando os launchers reais do checkout
resolvido. Confirmar que cada processo permanece ativo, inspecionar o log de
startup/plugin e verificar que não entrou em crash-loop. Não iniciar uma segunda
instância se o serviço já estiver saudável.

Depois reproduzir no client:

1. caminho que deve acionar a mudança;
2. caminho de rejeição/erro;
3. caminho adjacente que não deve acionar o hook;
4. repetição ou relogin quando houver estado persistente/global.

Classificar separadamente `build`, `deploy`, `startup` e `teste real`. Somente o
último pode sustentar uma alegação comportamental.

## Recuperação

Antes de testar hook de alto risco, preservar de forma recuperável o hash e uma
cópia conhecida da DLL implantada. Se houver crash-loop, interromper o launcher,
guardar dump/log/hash, restaurar a DLL conhecida, comparar SHA-256 e confirmar o
startup antes de investigar. Não apagar dumps nem sobrescrever a única cópia
funcional.

## Note when hand-driving the user

Quando uma mudança não aparecer, verificar primeiro evidências baratas: hash de
build/deploy, processo/launcher, log de carregamento e timestamp. Se os hashes
coincidem e o plugin carregou, tratar então como possível erro de código ou
gatilho. Não presumir automaticamente DLL stale nem declarar sucesso apenas
porque a cópia terminou.
