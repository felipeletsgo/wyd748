[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = $PSScriptRoot
$python = Get-Command python -ErrorAction Stop

function Assert-SameText {
    param(
        [string]$Expected,
        [string]$Actual
    )

    $expectedText = [IO.File]::ReadAllText($Expected).Replace("`r`n", "`n")
    $actualText = [IO.File]::ReadAllText($Actual).Replace("`r`n", "`n")
    if ($expectedText -cne $actualText) {
        throw "Generated contract differs from the registered file: $Expected"
    }
}

& (Join-Path $clientRoot "Verify-Fast.ps1")
& (Join-Path $clientRoot "tools\Test-Manifest.ps1")
& (Join-Path $clientRoot "tools\Test-SelfContained.ps1")

$temporaryDirectory = Join-Path ([IO.Path]::GetTempPath()) (
    "clientgo748-contract-{0}" -f [Guid]::NewGuid().ToString("N")
)
$temporaryMap = Join-Path $temporaryDirectory "asset-map.tsv"
$temporaryDependencies = Join-Path $temporaryDirectory "asset-dependencies.tsv"

New-Item -ItemType Directory -Path $temporaryDirectory | Out-Null
try {
    & $python.Source (Join-Path $clientRoot "tools\assets\build_asset_map.py") `
        --root (Join-Path $clientRoot "assets\current") `
        --map-out $temporaryMap `
        --deps-out $temporaryDependencies
    if ($LASTEXITCODE -ne 0) {
        throw "Asset contract census failed with exit code $LASTEXITCODE"
    }

    Assert-SameText `
        (Join-Path $clientRoot "references\assets\asset-map.tsv") `
        $temporaryMap
    Assert-SameText `
        (Join-Path $clientRoot "references\assets\asset-dependencies.tsv") `
        $temporaryDependencies
}
finally {
    if (Test-Path -LiteralPath $temporaryDirectory) {
        Remove-Item -LiteralPath $temporaryDirectory -Recurse -Force
    }
}

Write-Host "Contract verification passed: fast gate, package manifest, self-contained inputs, and deterministic asset census."
