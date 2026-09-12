[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipTests
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$OutputDirectory = Join-Path $PSScriptRoot 'bin'
$RuntimeDirectory = Join-Path $OutputDirectory 'Client Limpo'
$OutputPath = Join-Path $RuntimeDirectory 'wydclient.exe'
$AssetSourceDirectory = Join-Path $PSScriptRoot 'assets\current'
$RuntimeAssetDirectory = Join-Path $RuntimeDirectory 'assets\current'
$LegacyOutputPath = Join-Path $OutputDirectory 'wydclient.exe'

New-Item -ItemType Directory -Path $RuntimeDirectory -Force | Out-Null
New-Item -ItemType Directory -Path $RuntimeAssetDirectory -Force | Out-Null

if (-not (Test-Path -LiteralPath $AssetSourceDirectory -PathType Container)) {
    throw "clientgo748 active asset directory was not found: $AssetSourceDirectory"
}

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

    Write-Host "Staging runtime assets in $RuntimeAssetDirectory..."
    Copy-Item -Path (Join-Path $AssetSourceDirectory '*') -Destination $RuntimeAssetDirectory -Recurse -Force

    if (Test-Path -LiteralPath $LegacyOutputPath -PathType Leaf) {
        Remove-Item -LiteralPath $LegacyOutputPath -Force
    }
}
finally {
    Pop-Location
}

Write-Host "Built $OutputPath"
