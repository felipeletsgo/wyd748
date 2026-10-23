<#
.SYNOPSIS
Builds and locally installs the language extension for the production native launcher.
.DESCRIPTION
Preserves the native update.htm/ZIP protocol and site integration. Does not
publish, restart services, start the game, or execute the native update flow.
The source-built extension and disposable bootstrap fixture are tested first.
.PARAMETER BaseLauncher
Optional pinned native executable. The default uses a preserved local build
input, initializing it from client748/WYDLauncher.exe only when its hash matches.
.PARAMETER NoDeploy
Validate candidates without installing them into client748.
#>
[CmdletBinding()]
param([string]$BaseLauncher, [switch]$NoDeploy)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$client = Join-Path $repo 'tmproject/client748'
$build = Join-Path $repo 'tmproject/build/native-launcher'
$expected = 'A60DCA35B68DBE36ECE57541917A433F48282518561E2F7E89282C50F3E2E901'
$preserved = @{}
foreach ($name in @('WYD.exe', 'config.txt', 'lang.txt')) {
    $path = Join-Path $client $name
    $preserved[$name] = if (Test-Path -LiteralPath $path -PathType Leaf) { (Get-FileHash -LiteralPath $path).Hash } else { $null }
}
if (-not $BaseLauncher) {
    $BaseLauncher = Join-Path $build 'base/WYDLauncher.exe'
    if (-not (Test-Path -LiteralPath $BaseLauncher -PathType Leaf)) {
        $installed = Join-Path $client 'WYDLauncher.exe'
        if ((Get-FileHash -LiteralPath $installed -Algorithm SHA256).Hash -ne $expected) {
            throw 'Installed launcher is not the pinned native baseline. Supply -BaseLauncher from a verified backup.'
        }
        New-Item -ItemType Directory -Path (Split-Path $BaseLauncher -Parent) -Force | Out-Null
        Copy-Item -LiteralPath $installed -Destination $BaseLauncher
    }
}
$BaseLauncher = (Resolve-Path -LiteralPath $BaseLauncher).Path
if ((Get-FileHash -LiteralPath $BaseLauncher -Algorithm SHA256).Hash -ne $expected) {
    throw 'Base launcher SHA-256 differs from the verified operational native executable.'
}
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$matches = @(& $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe')
if ($LASTEXITCODE -ne 0 -or $matches.Count -eq 0) { throw 'MSBuild discovery failed.' }
$msbuild = $matches[0]
foreach ($project in @('LanguageExtension', 'LanguageExtensionTests')) {
    & $msbuild (Join-Path $repo "tmproject/WYDLauncher/native/$project.vcxproj") /p:Configuration=Release /p:Platform=Win32 /nologo /v:minimal
    if ($LASTEXITCODE -ne 0) { throw "$project build failed; runtime was not changed." }
}
$candidate = Join-Path $build 'Release/WYDLauncher.exe'
$library = Join-Path $build 'Release/WYDLauncherLanguage.dll'
& (Join-Path $build 'Release/NativeLauncherLanguageTests.exe') $library
if ($LASTEXITCODE -ne 0) { throw 'Native language control tests failed; runtime was not changed.' }
& python -B (Join-Path $PSScriptRoot 'build_native_launcher_language.py') --source $BaseLauncher --output $candidate
if ($LASTEXITCODE -ne 0) { throw 'Native launcher adaptation failed; runtime was not changed.' }
& python -B (Join-Path $PSScriptRoot 'test_native_launcher_language.py') --source $BaseLauncher --candidate $candidate --dll $library
if ($LASTEXITCODE -ne 0) { throw 'Native PE/bootstrap validation failed; runtime was not changed.' }
if (-not $NoDeploy) {
    $identical = $true
    foreach ($name in @('WYDLauncher.exe', 'WYDLauncherLanguage.dll')) {
        $installed = Join-Path $client $name
        if (-not (Test-Path -LiteralPath $installed -PathType Leaf) -or
            (Get-FileHash -LiteralPath $installed).Hash -ne (Get-FileHash -LiteralPath (Join-Path $build "Release/$name")).Hash) {
            $identical = $false
        }
    }
    $rollback = Split-Path $BaseLauncher -Parent
    if (-not $identical) {
        if (Get-Process -Name WYDLauncher -ErrorAction SilentlyContinue) { throw 'Close WYDLauncher before installing the candidate.' }
        $rollback = Join-Path $build ("rollback-" + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ'))
        New-Item -ItemType Directory -Path $rollback -Force | Out-Null
        foreach ($name in @('WYDLauncher.exe', 'WYDLauncherLanguage.dll')) {
            $installed = Join-Path $client $name
            if (Test-Path -LiteralPath $installed -PathType Leaf) { Copy-Item -LiteralPath $installed -Destination (Join-Path $rollback $name) }
        }
        # Install the dependency first, then the executable which imports it.
        Copy-Item -LiteralPath $library -Destination (Join-Path $client 'WYDLauncherLanguage.dll') -Force
        Copy-Item -LiteralPath $candidate -Destination (Join-Path $client 'WYDLauncher.exe') -Force
    }
    foreach ($name in @('WYDLauncher.exe', 'WYDLauncherLanguage.dll')) {
        if ((Get-FileHash -LiteralPath (Join-Path $client $name)).Hash -ne (Get-FileHash -LiteralPath (Join-Path $build "Release/$name")).Hash) {
            throw "Installed file verification failed: $name. Rollback: $rollback"
        }
    }
    foreach ($name in $preserved.Keys) {
        $path = Join-Path $client $name
        $hash = if (Test-Path -LiteralPath $path -PathType Leaf) { (Get-FileHash -LiteralPath $path).Hash } else { $null }
        if ($hash -ne $preserved[$name]) { throw "Unexpected change to preserved runtime file: $name" }
    }
    $receiptDirectory = Join-Path $repo 'build/client-locales'
    New-Item -ItemType Directory -Path $receiptDirectory -Force | Out-Null
    @{
        installed_at = [DateTime]::UtcNow.ToString('o')
        baseline_sha256 = $expected
        rollback_directory = $rollback
        source_head = (& git -C $repo rev-parse HEAD)
        preserved_files = $preserved
        files = @('WYDLauncher.exe', 'WYDLauncherLanguage.dll') | ForEach-Object {
            $path = Join-Path $client $_
            @{ path = $path; sha256 = (Get-FileHash -LiteralPath $path).Hash; size = (Get-Item -LiteralPath $path).Length }
        }
        native_sections_preserved = @('.text', '.rdata', '.data')
        validation = 'BUILD_AND_AUTOMATED_TESTS_VERIFIED; actual native update UI and gameplay pending'
    } | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $receiptDirectory 'native-launcher-install.json') -Encoding utf8
}
[pscustomobject]@{
    Executable = $candidate
    SHA256 = (Get-FileHash -LiteralPath $candidate).Hash
    LibrarySHA256 = (Get-FileHash -LiteralPath $library).Hash
    Installed = -not $NoDeploy
    Validation = 'BUILD_AND_AUTOMATED_TESTS_VERIFIED; native update UI and gameplay pending'
}
