<#
.SYNOPSIS
Audit static TMProject748 asset references against client748.
.DESCRIPTION
Read JSON manifests and static path literals in C++ sources without changing
assets. Dependencies declared unavailable in Costumes-KR.json are separate
from unclassified missing references. By default, report existing debt;
-FailOnMissing rejects unclassified missing references and casing mismatches.
SourceReferenceClassifications.json records exact reviewed source/path pairs;
conditional missing assets remain diagnostic and do not establish gameplay acceptance.
Source literals come from product code only; tests/ names deliberately missing
fixture files. A source .wyt or .wys model texture that is absent but has a
shipped sibling with the other extension is resolved, because the runtime
lookup compares stems (model_texture_748::SameName in ModelTextureDirectory.h).
#>
[CmdletBinding()]
param(
    [switch]$Details,
    [switch]$FailOnMissing,
    [string]$ClassificationManifestPath = (Join-Path $PSScriptRoot 'inputs/SourceReferenceClassifications.json')
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$assetRoot = Join-Path $repoRoot 'tmproject/client748'
$sourceRoot = Join-Path $repoRoot 'tmproject/TMProject748'
$assetPattern = '(?i)\.(?:msh|wys|msa|wyt|trn|ani|bon)$'
$sourceLiteralPattern = '(?i)"(?<path>(?:mesh|effect|env|ui)[/\\][^"\r\n%]+\.(?:msh|wys|msa|wyt|trn|ani|bon))"'

if (-not (Test-Path -LiteralPath $assetRoot -PathType Container)) {
    throw "Asset root missing: $assetRoot"
}
if (-not (Test-Path -LiteralPath $sourceRoot -PathType Container)) {
    throw "Source root missing: $sourceRoot"
}

function Normalize-AssetPath([string]$Path) {
    return (($Path -replace '[/\\]+', '/').TrimStart('./'))
}

# Model textures resolve by stem at runtime: a .wyt request finds a registered
# .wys file and the reverse (ModelTextureDirectory.h, model_texture_748::SameName).
function Get-TextureAlias([string]$Path) {
    if ($Path -match '(?i)^(?<stem>.+)\.(?<ext>wyt|wys)$') {
        $other = if ($Matches.ext -ieq 'wyt') { 'wys' } else { 'wyt' }
        return "$($Matches.stem).$other"
    }
    return $null
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
$manifestRoot = Join-Path $PSScriptRoot 'inputs'
$costumeManifestPath = Join-Path $manifestRoot $costumeManifestName
if (-not (Test-Path -LiteralPath $costumeManifestPath -PathType Leaf)) {
    throw "Manifest missing: $costumeManifestName"
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
$mountManifestPath = Join-Path $manifestRoot $mountManifestName
if (-not (Test-Path -LiteralPath $mountManifestPath -PathType Leaf)) {
    throw "Manifest missing: $mountManifestName"
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

$testRoot = (Join-Path $sourceRoot 'tests') + [IO.Path]::DirectorySeparatorChar
foreach ($file in Get-ChildItem -LiteralPath $sourceRoot -Recurse -File -Include '*.cpp', '*.h') {
    # Test fixtures name deliberately missing files; they are not runtime references.
    if ($file.FullName.StartsWith($testRoot, [StringComparison]::OrdinalIgnoreCase)) {
        continue
    }
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
$resolvedByAlias = [Collections.Generic.List[object]]::new()
foreach ($reference in $uniqueReferences) {
    $expected = $reference.Path
    if (-not $actualByKey.ContainsKey($expected)) {
        $alias = if ($reference.Kind -eq 'source') { Get-TextureAlias $expected } else { $null }
        if ($null -eq $alias -or -not $actualByKey.ContainsKey($alias)) {
            $missing.Add($reference)
            continue
        }
        $resolvedByAlias.Add([pscustomobject]@{
            Kind = $reference.Kind
            Source = $reference.Source
            Path = $reference.Path
            Actual = $actualByKey[$alias]
        })
        $expected = $alias
    }
    $actual = $actualByKey[$expected]
    if ($actual -cne $expected) {
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

# Classifications apply to exact source/path pairs, never whole directories.
# Source hashes force review when the consumer or its selection rules change.
$classificationManifest = [IO.File]::ReadAllText($ClassificationManifestPath) | ConvertFrom-Json
if ($classificationManifest.schemaVersion -ne 1) { throw 'Unsupported source classification schema.' }
$sourceKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($reference in $sourceMissingUnclassified) {
    $null = $sourceKeys.Add("$($reference.Source)|$($reference.Path)")
}
$classifiedKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$classifiedMissing = @($classificationManifest.references)
foreach ($property in $classificationManifest.sourceSha256.PSObject.Properties) {
    $source = Join-Path $sourceRoot $property.Name
    if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -cne $property.Value) {
        throw "Source classification needs review: $($property.Name)"
    }
}
foreach ($property in $classificationManifest.evidenceSha256.PSObject.Properties) {
    if ((Get-FileHash -LiteralPath (Join-Path $repoRoot $property.Name) -Algorithm SHA256).Hash -cne $property.Value) {
        throw "Source classification evidence needs review: $($property.Name)"
    }
}
foreach ($entry in $classifiedMissing) {
    $key = "$($entry.source)|$($entry.path)"
    if (-not $sourceKeys.Contains($key) -or -not $classifiedKeys.Add($key) -or
        $entry.classification -notin @('inactive-catalog-branch', 'remapped-name', 'conditional-unavailable') -or
        [string]::IsNullOrWhiteSpace($entry.reason) -or
        $entry.source -notin $classificationManifest.sourceSha256.PSObject.Properties.Name) {
        throw "Invalid or stale source classification: $key"
    }
    if ($entry.classification -eq 'remapped-name' -and -not $actualByKey.ContainsKey($entry.replacement)) {
        throw "Missing remap target: $($entry.replacement)"
    }
}
$sourceMissingUnclassified = @($sourceMissingUnclassified | Where-Object {
    -not $classifiedKeys.Contains("$($_.Source)|$($_.Path)")
})

[pscustomobject]@{
    AssetFiles = $actualByKey.Count
    ManifestRequiredReferences = $manifestReferences.Count
    ManifestRequiredMissing = $manifestMissing.Count
    DeclaredUnavailableReferences = $uniqueDeclaredUnavailable.Count
    SourceLiteralReferences = $sourceReferences.Count
    SourceResolvedByTextureAlias = $resolvedByAlias.Count
    SourceLiteralMissing = $sourceMissing.Count
    SourceMissingDeclaredUnavailable = $sourceMissingDeclaredUnavailable.Count
    SourceMissingUnclassified = $sourceMissingUnclassified.Count
    SourceMissingClassified = $classifiedMissing.Count
    SourceMissingInactive = @($classifiedMissing | Where-Object classification -eq 'inactive-catalog-branch').Count
    SourceMissingRemapped = @($classifiedMissing | Where-Object classification -eq 'remapped-name').Count
    SourceMissingConditional = @($classifiedMissing | Where-Object classification -eq 'conditional-unavailable').Count
    CaseMismatches = $caseMismatch.Count
}

if ($Details) {
    Write-Output 'Reviewed missing source references (conditional assets remain unavailable):'
    Write-Output (($classifiedMissing | Group-Object classification | Select-Object Count, Name |
        Format-Table -AutoSize | Out-String).TrimEnd())
    if ($manifestMissing.Count) {
        Write-Output 'Missing required manifest references:'
        Write-Output (($manifestMissing | Sort-Object Path, Source |
            Format-Table Kind, Source, Path -AutoSize | Out-String).TrimEnd())
    }
    if ($resolvedByAlias.Count) {
        Write-Output 'Source texture references resolved by the .wyt/.wys stem alias:'
        Write-Output (($resolvedByAlias | Sort-Object Path, Source |
            Format-Table Kind, Source, Path, Actual -AutoSize | Out-String).TrimEnd())
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
