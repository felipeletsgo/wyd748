[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = Split-Path -Parent $PSScriptRoot
$requiredPaths = @(
    "go.mod",
    "AGENTS.md",
    "assets\current",
    "references\catalog\functions.tsv",
    "references\ghidra\input\WYD.exe",
    "references\ghidra\project\WYD748Native_20260821.gpr",
    "references\ghidra\project\WYD748Native_20260821.rep",
    "references\ghidra\corpus",
    "references\research\flows",
    "references\server-source",
    "references\server-data",
    "references\tmproject\TMProject748",
    "tools\research"
)

foreach ($relative in $requiredPaths) {
    $path = Join-Path $clientRoot $relative
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required local package path is missing: $relative"
    }
}

$links = Get-ChildItem -LiteralPath $clientRoot -Force -Recurse | Where-Object {
    $_.Attributes -band [IO.FileAttributes]::ReparsePoint
}
if ($links) {
    $relativeLinks = $links | ForEach-Object {
        [IO.Path]::GetRelativePath($clientRoot, $_.FullName)
    }
    throw "Package contains filesystem links: $($relativeLinks -join ', ')"
}

$activeFiles = @(
    Get-Item -LiteralPath (Join-Path $clientRoot "go.mod")
    Get-Item -LiteralPath (Join-Path $clientRoot "Build-ClientGo.ps1")
    Get-Item -LiteralPath (Join-Path $clientRoot "Verify-Mapping.ps1")
    Get-ChildItem -LiteralPath (Join-Path $clientRoot "cmd") -File -Recurse
    Get-ChildItem -LiteralPath (Join-Path $clientRoot "internal") -File -Recurse
    Get-ChildItem -LiteralPath (Join-Path $clientRoot "tools") -File -Recurse |
        Where-Object {
            $_.Name -ne "Test-SelfContained.ps1" -and
            $_.Name -notlike "test_*.py" -and
            $_.Extension -in ".ps1", ".py"
        }
)

$forbidden = [regex]'(?i)(%USERPROFILE%|C:[\\/]+Users[\\/]|\.agents[\\/]|client-source[\\/]+tmproject)'
foreach ($file in $activeFiles) {
    $lineNumber = 0
    foreach ($line in [IO.File]::ReadLines($file.FullName)) {
        $lineNumber++
        if ($forbidden.IsMatch($line)) {
            $relative = [IO.Path]::GetRelativePath($clientRoot, $file.FullName)
            throw "Active file contains an external operational path: ${relative}:${lineNumber}"
        }
    }
}

$moduleText = [IO.File]::ReadAllText((Join-Path $clientRoot "go.mod"))
if ($moduleText -match '(?m)^\s*replace\s+') {
    throw "The independent client module must not use Go replace directives"
}

Write-Host "Self-contained package verified: required inputs are local and no filesystem links or external operational paths were found."
