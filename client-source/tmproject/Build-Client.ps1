[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [string]$PlatformToolset,

    [switch]$Rebuild
)

$ErrorActionPreference = 'Stop'

$root = $PSScriptRoot
$project = Join-Path $root 'Projects\TMProject\TMProject.vcxproj'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $project)) {
    throw "TMProject.vcxproj not found: $project"
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

$output = Join-Path $root "build\$Configuration"
$intermediate = Join-Path $root "build\obj\$Configuration"
$target = if ($Rebuild) { 'Rebuild' } else { 'Build' }
# MSBuild compatibility: a quoted property ending in one backslash escapes its closing quote; two preserve the required directory separator.
$solutionDir = $root.TrimEnd('\') + '\\'
$outputDir = $output.TrimEnd('\') + '\\'
$intermediateDir = $intermediate.TrimEnd('\') + '\\'

New-Item -ItemType Directory -Path $output, $intermediate -Force | Out-Null

Write-Host "Building TMProject ($Configuration|Win32, $PlatformToolset)"
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
    throw "TMProject build failed with exit code $LASTEXITCODE."
}

$fileName = if ($Configuration -eq 'Release') { 'WYD.exe' } else { 'WYDestiny.exe' }
$executable = Join-Path $output $fileName
if (-not (Test-Path -LiteralPath $executable)) {
    throw "Build completed without the expected executable: $executable"
}

Get-FileHash -Algorithm SHA256 -LiteralPath $executable
