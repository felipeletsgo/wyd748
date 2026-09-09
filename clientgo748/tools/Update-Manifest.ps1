[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = Split-Path -Parent $PSScriptRoot
$manifestPath = Join-Path $clientRoot "MANIFEST.sha256"

function Get-PackageFiles {
    Get-ChildItem -LiteralPath $clientRoot -File -Recurse | Where-Object {
        $relative = [IO.Path]::GetRelativePath($clientRoot, $_.FullName)
        $relative -ne "MANIFEST.sha256" -and
            $relative -notmatch '^bin[\\/]'
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
