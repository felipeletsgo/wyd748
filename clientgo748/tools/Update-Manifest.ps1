[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = Split-Path -Parent $PSScriptRoot
$manifestPath = Join-Path $clientRoot "MANIFEST.sha256"

function Test-ManifestExcludedPath([string]$relative) {
    $normalized = $relative.Replace('\', '/')
    return $normalized -eq "MANIFEST.sha256" -or
        $normalized -match '^bin/' -or
        $normalized -match '(^|/)__pycache__(/|$)' -or
        $normalized -match '\.py[cod]$' -or
        $normalized -match '\.log$' -or
        $normalized -match '(^|/)[^/]+\.tmp(?:-[^/]*)?$' -or
        $normalized -match '~$'
}

function Get-PackageFiles {
    Get-ChildItem -LiteralPath $clientRoot -File -Recurse | Where-Object {
        $relative = [IO.Path]::GetRelativePath($clientRoot, $_.FullName)
        -not (Test-ManifestExcludedPath $relative)
    }
}

$records = foreach ($file in (Get-PackageFiles | Sort-Object FullName)) {
    $relative = [IO.Path]::GetRelativePath($clientRoot, $file.FullName).Replace('\', '/')
    $digest = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    "$digest  $relative"
}

[IO.File]::WriteAllLines(
    $manifestPath,
    $records,
    [Text.UTF8Encoding]::new($false)
)

Write-Host "Manifest updated: $($records.Count) files -> $manifestPath"
