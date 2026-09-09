[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipTests
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$OutputDirectory = Join-Path $PSScriptRoot 'bin'
$OutputPath = Join-Path $OutputDirectory 'wydclient.exe'

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

Push-Location $PSScriptRoot
try {
    if ($SkipTests) {
        Write-Host "Skipping clientgo748 tests; run Verify-Fast.ps1 first."
    }
    else {
        Write-Host "Testing clientgo748 ($Configuration)..."
        & go test ./...
        if ($LASTEXITCODE -ne 0) {
            throw "clientgo748 tests failed with exit code $LASTEXITCODE"
        }
    }

    $BuildFlags = @('-trimpath')
    if ($Configuration -eq 'Release') {
        $BuildFlags += '-ldflags'
        $BuildFlags += '-s -w'
    }

    Write-Host "Building $OutputPath..."
    & go build @BuildFlags -o $OutputPath ./cmd/wydclient
    if ($LASTEXITCODE -ne 0) {
        throw "clientgo748 build failed with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}

Write-Host "Built $OutputPath"
