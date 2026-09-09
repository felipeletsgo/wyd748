[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$OutputDirectory = Join-Path $PSScriptRoot 'bin'
$OutputPath = Join-Path $OutputDirectory 'wydclient.exe'

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

Write-Host "Testing clientgo748 ($Configuration)..."
go test ./clientgo748/...
if ($LASTEXITCODE -ne 0) {
    throw "clientgo748 tests failed with exit code $LASTEXITCODE"
}

$BuildFlags = @('-trimpath')
if ($Configuration -eq 'Release') {
    $BuildFlags += '-ldflags'
    $BuildFlags += '-s -w'
}

Write-Host "Building $OutputPath..."
& go build @BuildFlags -o $OutputPath ./clientgo748/cmd/wydclient
if ($LASTEXITCODE -ne 0) {
    throw "clientgo748 build failed with exit code $LASTEXITCODE"
}

Write-Host "Built $OutputPath"
