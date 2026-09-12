[CmdletBinding()]
param(
    [string]$ClientRoot = $PSScriptRoot,
    [string]$SourceRoot = (Join-Path (Split-Path -Parent $PSScriptRoot) 'CLIENTS\WYD'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'MobFaces-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Installs the complete skeleton families used by the imported KR creatures.
# LB/LK deliberately share texture files in the modern renderer. The two
# legacy aliases below reproduce that proven mapping instead of inventing an
# absent texture or modifying the meshes.

$familyPrefixes = @('LB01','LK01','DD01','sp02','mm01','dw01','an01','an02')
$boneRows = @(
    '45 8 3 mesh\LB01',
    '46 7 3 mesh\LK01',
    '47 6 1 mesh\DD01',
    '53 6 3 mesh\sp02',
    '54 6 3 mesh\mm01',
    '55 5 2 mesh\dw01',
    '56 9 4 mesh\an01',
    '57 9 5 mesh\an02'
)
$animationTypes = @(45,46,47,53,54,55,56,57)
$specificFiles = @(
    'gg010103.msh','gg010103.wys','gg010203.msh',
    'sq010104.msh','sq010104.wys','sq010204.msh','sq010204.wys',
    'et010103.wys','rc010103.wys','rc010104.wys'
)

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant() }
function Test-ByteRangeEqual([byte[]]$A, [byte[]]$B, [int]$Offset, [int]$Count) {
    for ($i = 0; $i -lt $Count; $i++) { if ($A[$Offset + $i] -ne $B[$Offset + $i]) { return $false } }
    return $true
}
function Get-AniSection([string]$Text, [int]$Type) {
    $match = [regex]::Match($Text, "(?ms)^\[[^\r\n]+\]\s+$Type\s*\r?\n.*?(?=^\[|\z)")
    if (-not $match.Success) { throw "AniSound4 fonte nao possui secao $Type" }
    # The KR text carries tabs/spaces after several values. They are not part
    # of the format and would make git diff --check reject a reproducible
    # installation, so normalize only the imported section.
    return ([regex]::Replace($match.Value, '[ \t]+(?=\r?$)', '', 'Multiline')).TrimEnd()
}
function Assert-Installed {
    $sourceMesh = Join-Path $SourceRoot 'mesh'
    $targetMesh = Join-Path $ClientRoot 'mesh'
    foreach ($prefix in $familyPrefixes) {
        $sourceFiles = @(Get-ChildItem -LiteralPath $sourceMesh -File | Where-Object { $_.BaseName.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase) })
        if ($sourceFiles.Count -eq 0) { throw "familia KR ausente: $prefix" }
        foreach ($source in $sourceFiles) {
            $target = Join-Path $targetMesh $source.Name
            if (-not (Test-Path -LiteralPath $target -PathType Leaf)) { throw "asset KR ausente: $($source.Name)" }
        }
    }
    foreach ($name in $specificFiles) {
        if (-not (Test-Path -LiteralPath (Join-Path $targetMesh $name) -PathType Leaf)) { throw "asset variante KR ausente: $name" }
    }
    foreach ($name in @('LB010301.wys','LK010301.wys')) {
        if (-not (Test-Path -LiteralPath (Join-Path $targetMesh $name) -PathType Leaf)) { throw "alias de textura KR ausente: $name" }
    }
    $boneText = [IO.File]::ReadAllText((Join-Path $targetMesh 'BoneAni4.txt'), [Text.Encoding]::Default)
    foreach ($row in $boneRows) {
        if ($boneText -notmatch ('(?m)^' + [regex]::Escape($row) + '\s*$')) { throw "BoneAni4 nao possui: $row" }
    }
    $aniText = [IO.File]::ReadAllText((Join-Path $ClientRoot 'AniSound4.txt'), [Text.Encoding]::Default)
    foreach ($type in $animationTypes) {
        if ($aniText -notmatch "(?m)^\[[^\r\n]+\]\s+$type\s*$") { throw "AniSound4 nao possui tipo $type" }
    }
    $valid = [IO.File]::ReadAllBytes((Join-Path $targetMesh 'ValidIndex.bin'))
    $sourceValid = [IO.File]::ReadAllBytes((Join-Path $sourceMesh 'ValidIndex.bin'))
    if ($valid.Length -ne 74400 -or $sourceValid.Length -ne 74400) { throw 'ValidIndex.bin com tamanho inesperado' }
    foreach ($type in $animationTypes) {
        if (-not (Test-ByteRangeEqual $valid $sourceValid ($type * 744) 744)) { throw "ValidIndex tipo $type divergente" }
    }
}

foreach ($path in @($ClientRoot, $SourceRoot, $Manifest)) {
    if (-not (Test-Path -LiteralPath $path)) { throw "caminho ausente: $path" }
}
try {
    Assert-Installed
    Write-Host 'Assets, skeletons e animacoes das 19 faces KR ja estao instalados.'
    return
} catch {
    if ($VerifyOnly) { throw }
}

$sourceMesh = Join-Path $SourceRoot 'mesh'
$targetMesh = Join-Path $ClientRoot 'mesh'
foreach ($prefix in $familyPrefixes) {
    Get-ChildItem -LiteralPath $sourceMesh -File | Where-Object {
        $_.BaseName.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)
    } | ForEach-Object {
        $target = Join-Path $targetMesh $_.Name
        if (-not (Test-Path -LiteralPath $target -PathType Leaf)) { Copy-Item -LiteralPath $_.FullName -Destination $target }
    }
}
foreach ($name in $specificFiles) {
    $source = Join-Path $sourceMesh $name
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "asset fonte ausente: $name" }
    $target = Join-Path $targetMesh $name
    if (-not (Test-Path -LiteralPath $target -PathType Leaf)) { Copy-Item -LiteralPath $source -Destination $target }
}

# W2PP maps LB/LK parts 1+2 to texture 01 and part 3 to texture 02. The 7.48
# renderer resolves one pathname per part, so materialize the same mapping.
Copy-Item -LiteralPath (Join-Path $sourceMesh 'LB010101.wys') -Destination (Join-Path $targetMesh 'LB010201.wys') -Force
Copy-Item -LiteralPath (Join-Path $sourceMesh 'LB010201.wys') -Destination (Join-Path $targetMesh 'LB010301.wys') -Force
Copy-Item -LiteralPath (Join-Path $sourceMesh 'LK010101.wys') -Destination (Join-Path $targetMesh 'LK010201.wys') -Force
Copy-Item -LiteralPath (Join-Path $sourceMesh 'LK010201.wys') -Destination (Join-Path $targetMesh 'LK010301.wys') -Force

$bonePath = Join-Path $targetMesh 'BoneAni4.txt'
$boneText = [IO.File]::ReadAllText($bonePath, [Text.Encoding]::Default)
foreach ($row in $boneRows) {
    if ($boneText -notmatch ('(?m)^' + [regex]::Escape($row) + '\s*$')) { $boneText = $boneText.TrimEnd() + "`r`n$row`r`n" }
}
[IO.File]::WriteAllText($bonePath, $boneText, [Text.Encoding]::Default)

$aniPath = Join-Path $ClientRoot 'AniSound4.txt'
$sourceAniPath = Join-Path $SourceRoot 'AniSound4.txt'
$aniText = [IO.File]::ReadAllText($aniPath, [Text.Encoding]::Default)
$sourceAniText = [IO.File]::ReadAllText($sourceAniPath, [Text.Encoding]::Default)
foreach ($type in $animationTypes) {
    if ($aniText -notmatch "(?m)^\[[^\r\n]+\]\s+$type\s*$") {
        $aniText = $aniText.TrimEnd() + "`r`n`r`n" + (Get-AniSection $sourceAniText $type) + "`r`n"
    }
}
[IO.File]::WriteAllText($aniPath, $aniText, [Text.Encoding]::Default)

$validPath = Join-Path $targetMesh 'ValidIndex.bin'
$sourceValidPath = Join-Path $sourceMesh 'ValidIndex.bin'
$valid = [IO.File]::ReadAllBytes($validPath)
$sourceValid = [IO.File]::ReadAllBytes($sourceValidPath)
if ($valid.Length -ne 74400 -or $sourceValid.Length -ne 74400) { throw 'ValidIndex.bin com tamanho inesperado' }
foreach ($type in $animationTypes) { [Array]::Copy($sourceValid, $type * 744, $valid, $type * 744, 744) }
[IO.File]::WriteAllBytes($validPath, $valid)

Assert-Installed
Write-Host 'Assets, skeletons e animacoes das 19 faces KR instalados no client 7.48.'
Write-Host "AniSound4 SHA-256: $(Get-Sha $aniPath)"
Write-Host "BoneAni4 SHA-256: $(Get-Sha $bonePath)"
Write-Host "ValidIndex SHA-256: $(Get-Sha $validPath)"
