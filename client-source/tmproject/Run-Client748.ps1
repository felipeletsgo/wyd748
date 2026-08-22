[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [string]$AssetRoot = (Join-Path (Split-Path $PSScriptRoot -Parent) '..\client748'),

    [switch]$Build,

    [switch]$VerifyOnly,

    [switch]$Wait
)

$ErrorActionPreference = 'Stop'
$AssetRoot = (Resolve-Path -LiteralPath $AssetRoot).Path

# Fail before DirectX initialization so a malformed table cannot become a
# renderer crash whose original asset cause is difficult to diagnose.
& (Join-Path $PSScriptRoot 'Test-Client748Assets.ps1') -AssetRoot $AssetRoot

if ($VerifyOnly) {
    return
}

if ($Build) {
    & (Join-Path $PSScriptRoot 'Build-Client.ps1') -Configuration $Configuration
}

$fileName = if ($Configuration -eq 'Release') { 'WYD.exe' } else { 'WYDestiny.exe' }
$executable = Join-Path $PSScriptRoot "build\$Configuration\$fileName"
if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw "Source-built client was not found: $executable. Use -Build first."
}

# The executable reads relative paths. The environment variable is consumed by
# WYD748_InitializeAssetRoot and avoids copying over client748/WYD.exe.
$previousRoot = $env:WYD_ASSET_ROOT
$env:WYD_ASSET_ROOT = $AssetRoot
try {
    $arguments = @{}
    if ($Wait) {
        $arguments.Wait = $true
    }

    Start-Process -FilePath $executable -WorkingDirectory $AssetRoot @arguments
}
finally {
    $env:WYD_ASSET_ROOT = $previousRoot
}
