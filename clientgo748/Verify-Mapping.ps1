[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = $PSScriptRoot
$repoRoot = Split-Path -Parent $clientRoot
$python = Get-Command python -ErrorAction Stop
$expectedNativeHash = "8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593"

function Invoke-Checked {
    param([string]$Program, [string[]]$Arguments)

    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE: $Program $($Arguments -join ' ')"
    }
}

$corpus = Join-Path $clientRoot "references\ghidra\corpus"
$catalog = Join-Path $clientRoot "references\catalog\functions.tsv"
$native = Join-Path $clientRoot "references\ghidra\input\WYD.exe"
$flows = Join-Path $clientRoot "references\research\flows"

if (-not (Test-Path -LiteralPath $native)) {
    throw "Native 7.48 executable is missing: $native"
}
$actualNativeHash = (Get-FileHash -LiteralPath $native -Algorithm SHA256).Hash
if ($actualNativeHash -ne $expectedNativeHash) {
    throw "Native executable fingerprint mismatch. Expected $expectedNativeHash, found $actualNativeHash"
}

$functionCount = (Import-Csv -LiteralPath $catalog -Delimiter "`t").Count
if ($functionCount -ne 4146) {
    throw "Function catalog mismatch. Expected 4146, found $functionCount"
}

Invoke-Checked $python.Source @(
    (Join-Path $clientRoot "tools\research\query_corpus.py"),
    "--corpus", $corpus,
    "stats", "--repo", $clientRoot
)
Invoke-Checked $python.Source @(
    (Join-Path $clientRoot "tools\research\triage_catalog.py"),
    "--input", $catalog,
    "--binary", $native,
    "--format", "summary"
)
Invoke-Checked $python.Source @(
    (Join-Path $clientRoot "tools\research\validate_research.py"),
    "--flows", $flows
)

Push-Location $repoRoot
try {
    Invoke-Checked "go" @("test", "./clientgo748/...")
}
finally {
    Pop-Location
}

Write-Host "Mapping package verified: 4,146 functions, native fingerprint, research records, and Go tests."
