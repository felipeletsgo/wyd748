[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [ValidateSet('TMProject748', 'TMProject')]
    [string]$ProjectVariant = 'TMProject748',

    [string]$PlatformToolset,

    [switch]$Rebuild
)

$ErrorActionPreference = 'Stop'

$root = $PSScriptRoot
# TMProject748 e o caminho ativo da modernizacao. TMProject permanece disponivel
# apenas como referencia compilavel para comparacoes controladas.
$projectFileName = "$ProjectVariant.vcxproj"
$project = Join-Path $root "Projects\$ProjectVariant\$projectFileName"
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$repositoryRoot = Split-Path (Split-Path $root -Parent) -Parent
$clientDirectory = Join-Path $repositoryRoot 'client748'
$installedExecutable = Join-Path $clientDirectory 'project.exe'

if (-not (Test-Path -LiteralPath $project)) {
    throw "$projectFileName not found: $project"
}
if (-not (Test-Path -LiteralPath $clientDirectory -PathType Container)) {
    throw "Client installation directory not found: $clientDirectory"
}
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Visual Studio Installer (vswhere.exe) was not found.'
}

$instances = @(& $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -format json | ConvertFrom-Json)
if ($instances.Count -eq 0) {
    throw 'A Visual Studio installation with MSBuild was not found.'
}

$instance = $instances[0]
$msbuild = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' |
    Select-Object -First 1
if (-not $msbuild) {
    throw 'MSBuild.exe was not found.'
}

if (-not $PlatformToolset) {
    $PlatformToolset = switch ([string]$instance.catalog.productLineVersion) {
        '18' { 'v145' }
        '17' { 'v143' }
        default { throw "Unsupported Visual Studio product line: $($_)" }
    }
}

# Cada variante possui saidas proprias para impedir que objetos ou executaveis
# do projeto de referencia contaminem o projeto 7.48 reestruturado.
$output = Join-Path $root "build\$ProjectVariant\$Configuration"
$intermediate = Join-Path $root "build\obj\$ProjectVariant\$Configuration"
$target = if ($Rebuild) { 'Rebuild' } else { 'Build' }
# MSBuild compatibility: a quoted property ending in one backslash escapes its closing quote; two preserve the required directory separator.
$solutionDir = $root.TrimEnd('\') + '\\'
$outputDir = $output.TrimEnd('\') + '\\'
$intermediateDir = $intermediate.TrimEnd('\') + '\\'

New-Item -ItemType Directory -Path $output, $intermediate -Force | Out-Null

Write-Host "Building $ProjectVariant ($Configuration|Win32, $PlatformToolset)"
& $msbuild $project `
    '/m' `
    "/t:$target" `
    "/p:Configuration=$Configuration" `
    '/p:Platform=Win32' `
    "/p:PlatformToolset=$PlatformToolset" `
    "/p:SolutionDir=$solutionDir" `
    "/p:OutDir=$outputDir" `
    "/p:IntDir=$intermediateDir"

if ($LASTEXITCODE -ne 0) {
    throw "$ProjectVariant build failed with exit code $LASTEXITCODE."
}

$fileName = if ($Configuration -eq 'Release') { 'WYD.exe' } else { 'WYDestiny.exe' }
$executable = Join-Path $output $fileName
if (-not (Test-Path -LiteralPath $executable)) {
    throw "Build completed without the expected executable: $executable"
}

$buildHash = Get-FileHash -Algorithm SHA256 -LiteralPath $executable

if ($ProjectVariant -eq 'TMProject748') {
    # Apenas a arquitetura nova instala o client ativo. Compilar TMProject
    # nunca substitui project.exe por acidente; seu artefato fica em build/.
    Copy-Item -LiteralPath $executable -Destination $installedExecutable -Force

    $installedHash = Get-FileHash -Algorithm SHA256 -LiteralPath $installedExecutable
    if ($buildHash.Hash -ne $installedHash.Hash) {
        throw "Installed project.exe hash does not match the source build."
    }

    Write-Host "Installed client: $installedExecutable"
    $installedHash
}
else {
    Write-Host "Reference build preserved without installing over project.exe: $executable"
    $buildHash
}
