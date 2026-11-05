<#
.SYNOPSIS
Package the operational native launcher and client. Do not publish or install.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)] [ValidatePattern('^[A-Za-z0-9._-]{1,64}$')] [string]$Release,
    [Parameter(Mandatory)] [ValidateRange(48001,49999)] [int]$Version,
    [Parameter(Mandatory)] [string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$output = [IO.Path]::GetFullPath($OutputDirectory)
$buildRoot = [IO.Path]::GetFullPath((Join-Path $repo 'build')) + [IO.Path]::DirectorySeparatorChar
if (-not $output.StartsWith($buildRoot, [StringComparison]::OrdinalIgnoreCase) -or (Test-Path -LiteralPath $output)) {
    throw 'Use a new output directory under root build/.'
}
& (Join-Path $PSScriptRoot 'Stage-ClientRuntime.ps1') -OutputDirectory "$output-runtime" | Out-Null
& python -B (Join-Path $PSScriptRoot 'build_native_client_release.py') build --release $Release --version $Version `
    --output $output --runtime-receipt (Join-Path "$output-runtime" 'runtime-receipt.json')
if ($LASTEXITCODE -ne 0) { throw 'Native client packaging failed.' }
