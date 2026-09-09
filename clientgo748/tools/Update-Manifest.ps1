[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = Split-Path -Parent $PSScriptRoot
$manifestPath = Join-Path $clientRoot "MANIFEST.sha256"
. (Join-Path $PSScriptRoot "Manifest-Policy.ps1")

function Get-PackageFiles {
    Get-ChildItem -LiteralPath $clientRoot -File -Recurse | Where-Object {
        $relative = [IO.Path]::GetRelativePath($clientRoot, $_.FullName)
        -not (Test-ManifestExcludedPath $relative)
    }
}

$records = foreach ($file in (Get-PackageFiles | Sort-Object FullName)) {
    $relative = [IO.Path]::GetRelativePath($clientRoot, $file.FullName).Replace('\', '/')
    $digest = Get-ManifestDigest $file.FullName $relative
    "$digest  $relative"
}

[IO.File]::WriteAllLines(
    $manifestPath,
    $records,
    [Text.UTF8Encoding]::new($false)
)

Write-Host "Manifest updated: $($records.Count) files -> $manifestPath"
