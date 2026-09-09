[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = Split-Path -Parent $PSScriptRoot
$manifestPath = Join-Path $clientRoot "MANIFEST.sha256"

if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
    throw "Package manifest is missing: $manifestPath"
}

$expected = [Collections.Generic.Dictionary[string,string]]::new(
    [StringComparer]::OrdinalIgnoreCase
)

foreach ($line in [IO.File]::ReadAllLines($manifestPath)) {
    if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') {
        throw "Invalid manifest record: $line"
    }

    $relative = $Matches[2].Replace('/', [IO.Path]::DirectorySeparatorChar)
    if ([IO.Path]::IsPathRooted($relative) -or $relative -split '[\\/]' -contains '..') {
        throw "Unsafe manifest path: $relative"
    }

    $fullPath = [IO.Path]::GetFullPath((Join-Path $clientRoot $relative))
    $rootPrefix = $clientRoot + [IO.Path]::DirectorySeparatorChar
    if (-not $fullPath.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Manifest path escapes the package: $relative"
    }
    if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
        throw "Manifest file is missing: $relative"
    }
    if ($expected.ContainsKey($relative)) {
        throw "Duplicate manifest path: $relative"
    }

    $expected.Add($relative, $Matches[1].ToUpperInvariant())
}

foreach ($record in $expected.GetEnumerator()) {
    $fullPath = Join-Path $clientRoot $record.Key
    $actual = (Get-FileHash -LiteralPath $fullPath -Algorithm SHA256).Hash
    if ($actual -ne $record.Value) {
        throw "Manifest hash mismatch: $($record.Key)"
    }
}

$actualFiles = Get-ChildItem -LiteralPath $clientRoot -File -Recurse | Where-Object {
    $relative = [IO.Path]::GetRelativePath($clientRoot, $_.FullName)
    $relative -ne "MANIFEST.sha256" -and
        $relative -notmatch '^bin[\\/]'
}
foreach ($file in $actualFiles) {
    $relative = [IO.Path]::GetRelativePath($clientRoot, $file.FullName)
    if (-not $expected.ContainsKey($relative)) {
        throw "File is not registered in the manifest: $relative"
    }
}

Write-Host "Manifest verified: $($expected.Count) files."
