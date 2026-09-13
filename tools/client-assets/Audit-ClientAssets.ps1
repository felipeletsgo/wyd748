<#
.SYNOPSIS
Audita referencias estaticas de assets do TMProject748 contra client748.
.DESCRIPTION
Le os manifests JSON e literais estaticos de paths nas sources C++ sem alterar
assets. Dependencias declaradas como ausentes em Costumes-KR.json sao separadas
de faltas ainda nao classificadas. Por padrao reporta a divida existente;
-FailOnMissing transforma faltas nao classificadas ou divergencias de casing em
erro para uso futuro como gate.
#>
[CmdletBinding()]
param(
    [switch]$Details,
    [switch]$FailOnMissing
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$assetRoot = Join-Path $repoRoot 'tmproject/client748'
$sourceRoot = Join-Path $repoRoot 'tmproject/TMProject748'
$assetPattern = '(?i)\.(?:msh|wys|msa|wyt|trn|ani|bon)$'
$sourceLiteralPattern = '(?i)"(?<path>(?:mesh|effect|env|ui)[/\\][^"\r\n%]+\.(?:msh|wys|msa|wyt|trn|ani|bon))"'

if (-not (Test-Path -LiteralPath $assetRoot -PathType Container)) {
    throw "Asset root ausente: $assetRoot"
}
if (-not (Test-Path -LiteralPath $sourceRoot -PathType Container)) {
    throw "Source root ausente: $sourceRoot"
}

function Normalize-AssetPath([string]$Path) {
    return (($Path -replace '[/\\]+', '/').TrimStart('./'))
}

$actualByKey = [Collections.Generic.Dictionary[string,string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($file in Get-ChildItem -LiteralPath $assetRoot -Recurse -File) {
    $relative = Normalize-AssetPath ([IO.Path]::GetRelativePath($assetRoot, $file.FullName))
    if ($relative -match $assetPattern -and -not $actualByKey.ContainsKey($relative)) {
        $actualByKey.Add($relative, $relative)
    }
}

$references = [Collections.Generic.List[object]]::new()
$declaredUnavailable = [Collections.Generic.List[object]]::new()

$costumeManifestName = 'Costumes-KR.json'
$costumeManifestPath = Join-Path $assetRoot $costumeManifestName
if (-not (Test-Path -LiteralPath $costumeManifestPath -PathType Leaf)) {
    throw "Manifest ausente: $costumeManifestName"
}
$costumeManifest = [IO.File]::ReadAllText($costumeManifestPath) | ConvertFrom-Json
foreach ($item in @($costumeManifest.items)) {
    foreach ($path in @($item.missingAssets)) {
        if ([string]::IsNullOrWhiteSpace($path)) {
            continue
        }
        $declaredUnavailable.Add([pscustomobject]@{
            Kind = 'declared-unavailable'
            Source = "$costumeManifestName item=$($item.item)"
            Path = Normalize-AssetPath $path
        })
    }
}

$mountManifestName = 'Mounts-KR.json'
$mountManifestPath = Join-Path $assetRoot $mountManifestName
if (-not (Test-Path -LiteralPath $mountManifestPath -PathType Leaf)) {
    throw "Manifest ausente: $mountManifestName"
}
$mountManifest = [IO.File]::ReadAllText($mountManifestPath) | ConvertFrom-Json
foreach ($texture in @($mountManifest.textures)) {
    if ([string]::IsNullOrWhiteSpace($texture.name)) {
        continue
    }

    $normalizedPath = Normalize-AssetPath $texture.name
    if ($null -ne $texture.PSObject.Properties['available'] -and $texture.available -eq $false) {
        $declaredUnavailable.Add([pscustomobject]@{
            Kind = 'declared-unavailable'
            Source = "$mountManifestName textures"
            Path = $normalizedPath
        })
        continue
    }

    $references.Add([pscustomobject]@{
        Kind = 'manifest-required'
        Source = "$mountManifestName textures"
        Path = $normalizedPath
    })
}

foreach ($file in Get-ChildItem -LiteralPath $sourceRoot -Recurse -File -Include '*.cpp', '*.h') {
    $raw = [IO.File]::ReadAllText($file.FullName)
    foreach ($match in [regex]::Matches($raw, $sourceLiteralPattern)) {
        $references.Add([pscustomobject]@{
            Kind = 'source'
            Source = Normalize-AssetPath ([IO.Path]::GetRelativePath($sourceRoot, $file.FullName))
            Path = Normalize-AssetPath $match.Groups['path'].Value
        })
    }
}

$uniqueReferences = @($references | Sort-Object Kind, Source, Path -Unique)
$uniqueDeclaredUnavailable = @($declaredUnavailable | Sort-Object Path -Unique)
$declaredUnavailablePaths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($reference in $uniqueDeclaredUnavailable) {
    $null = $declaredUnavailablePaths.Add($reference.Path)
}
$missing = [Collections.Generic.List[object]]::new()
$caseMismatch = [Collections.Generic.List[object]]::new()
foreach ($reference in $uniqueReferences) {
    if (-not $actualByKey.ContainsKey($reference.Path)) {
        $missing.Add($reference)
        continue
    }
    $actual = $actualByKey[$reference.Path]
    if ($actual -cne $reference.Path) {
        $caseMismatch.Add([pscustomobject]@{
            Kind = $reference.Kind
            Source = $reference.Source
            Path = $reference.Path
            Actual = $actual
        })
    }
}

$manifestReferences = @($uniqueReferences | Where-Object Kind -eq 'manifest-required')
$sourceReferences = @($uniqueReferences | Where-Object Kind -eq 'source')
$manifestMissing = @($missing | Where-Object Kind -eq 'manifest-required')
$sourceMissing = @($missing | Where-Object Kind -eq 'source')
$sourceMissingDeclaredUnavailable = @($sourceMissing | Where-Object { $declaredUnavailablePaths.Contains($_.Path) })
$sourceMissingUnclassified = @($sourceMissing | Where-Object { -not $declaredUnavailablePaths.Contains($_.Path) })

[pscustomobject]@{
    AssetFiles = $actualByKey.Count
    ManifestRequiredReferences = $manifestReferences.Count
    ManifestRequiredMissing = $manifestMissing.Count
    DeclaredUnavailableReferences = $uniqueDeclaredUnavailable.Count
    SourceLiteralReferences = $sourceReferences.Count
    SourceLiteralMissing = $sourceMissing.Count
    SourceMissingDeclaredUnavailable = $sourceMissingDeclaredUnavailable.Count
    SourceMissingUnclassified = $sourceMissingUnclassified.Count
    CaseMismatches = $caseMismatch.Count
}

if ($Details) {
    if ($manifestMissing.Count) {
        Write-Output 'Missing required manifest references:'
        Write-Output (($manifestMissing | Sort-Object Path, Source |
            Format-Table Kind, Source, Path -AutoSize | Out-String).TrimEnd())
    }
    if ($sourceMissingDeclaredUnavailable.Count) {
        Write-Output 'Source references already declared unavailable by costume manifest:'
        Write-Output (($sourceMissingDeclaredUnavailable | Sort-Object Path, Source |
            Format-Table Kind, Source, Path -AutoSize | Out-String).TrimEnd())
    }
    if ($sourceMissingUnclassified.Count) {
        Write-Output 'Unclassified missing source references:'
        Write-Output (($sourceMissingUnclassified | Sort-Object Path, Source |
            Format-Table Kind, Source, Path -AutoSize | Out-String).TrimEnd())
    }
    if ($caseMismatch.Count) {
        Write-Output 'Casing mismatches:'
        Write-Output (($caseMismatch | Sort-Object Kind, Path, Source |
            Format-Table Kind, Source, Path, Actual -AutoSize | Out-String).TrimEnd())
    }
}

if ($FailOnMissing -and ($manifestMissing.Count -gt 0 -or $sourceMissingUnclassified.Count -gt 0 -or $caseMismatch.Count -gt 0)) {
    throw "Asset audit failed: $($manifestMissing.Count) required manifest missing, $($sourceMissingUnclassified.Count) unclassified source missing, $($caseMismatch.Count) casing mismatches."
}
