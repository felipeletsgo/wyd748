[CmdletBinding()]
param(
    [string]$ItemList,
    [string]$ReferenceItemList,
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if ([string]::IsNullOrWhiteSpace($ItemList)) {
    $ItemList = Join-Path $repositoryRoot 'client748\ItemList.bin'
}

# This is the active source/asset workflow for WYD 7.48. EF_GRID value zero is
# the canonical 1x1 footprint; no executable or historical patch is involved.
$recordCount = 6500
$recordSize = 140
$payloadLength = $recordCount * $recordSize
$expectedLength = $payloadLength + 4
$xorKey = 0x5A
$effectOffset = 0x50
$effectCount = 12
$effectSize = 4
$gridEffect = 33
$expectedGridEffects = 1980

function Get-DecodedUInt16([byte[]]$Data, [int]$Offset) {
    $low = ([int]$Data[$Offset]) -bxor $xorKey
    $high = ([int]$Data[$Offset + 1]) -bxor $xorKey
    return $low -bor ($high -shl 8)
}

function Set-EncodedUInt16([byte[]]$Data, [int]$Offset, [uint16]$Value) {
    # Only this little-endian value field is writable. Type bytes, all other
    # record fields and the trailing four-byte stamp remain byte-identical.
    $Data[$Offset] = [byte]((([int]$Value) -band 0xFF) -bxor $xorKey)
    $Data[$Offset + 1] = [byte](((([int]$Value) -shr 8) -band 0xFF) -bxor $xorKey)
}

function Assert-ItemListShape([byte[]]$Data, [string]$Label) {
    if ($Data.Length -ne $expectedLength) {
        throw "Invalid $Label length: $($Data.Length); expected $expectedLength (6500 x 140 + 4-byte stamp)."
    }
}

function Measure-ItemGrid([byte[]]$Data, [string]$Label) {
    Assert-ItemListShape $Data $Label
    $pairs = 0
    $nonZeroPairs = 0
    $records = 0

    for ($itemIndex = 0; $itemIndex -lt $recordCount; $itemIndex++) {
        $recordHasGrid = $false
        for ($effectIndex = 0; $effectIndex -lt $effectCount; $effectIndex++) {
            $typeOffset = ($itemIndex * $recordSize) + $effectOffset + ($effectIndex * $effectSize)
            if ((Get-DecodedUInt16 $Data $typeOffset) -ne $gridEffect) {
                continue
            }

            $recordHasGrid = $true
            $pairs++
            $value = Get-DecodedUInt16 $Data ($typeOffset + 2)
            if ($value -gt 7) {
                throw "Invalid $Label EF_GRID value $value at item $itemIndex, effect $effectIndex; expected 0..7 after XOR 0x5A decoding."
            }
            if ($value -ne 0) {
                $nonZeroPairs++
            }
        }
        if ($recordHasGrid) {
            $records++
        }
    }

    if ($pairs -ne $expectedGridEffects) {
        throw "Invalid $Label EF_GRID count: $pairs; expected $expectedGridEffects after XOR 0x5A decoding."
    }

    return [pscustomobject]@{
        Pairs = $pairs
        NonZeroPairs = $nonZeroPairs
        Records = $records
    }
}

function Get-StampHex([byte[]]$Data) {
    $stamp = [byte[]]::new(4)
    [Array]::Copy($Data, $payloadLength, $stamp, 0, 4)
    return [Convert]::ToHexString($stamp)
}

function Get-ByteHash([byte[]]$Data) {
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try {
        return [Convert]::ToHexString($sha256.ComputeHash($Data))
    } finally {
        $sha256.Dispose()
    }
}

function Measure-ByteDifferences([byte[]]$Expected, [byte[]]$Actual) {
    if ($Expected.Length -ne $Actual.Length) {
        throw "Cannot compare buffers with different lengths: $($Expected.Length) and $($Actual.Length)."
    }

    $count = 0
    $firstOffset = -1
    for ($offset = 0; $offset -lt $Expected.Length; $offset++) {
        if ($Expected[$offset] -eq $Actual[$offset]) {
            continue
        }
        if ($firstOffset -lt 0) {
            $firstOffset = $offset
        }
        $count++
    }

    return [pscustomobject]@{
        Count = $count
        FirstOffset = $firstOffset
    }
}

if (-not (Test-Path -LiteralPath $ItemList -PathType Leaf)) {
    throw "ItemList.bin not found: $ItemList"
}
$resolvedItemList = (Resolve-Path -LiteralPath $ItemList).Path

if ([string]::IsNullOrWhiteSpace($ReferenceItemList)) {
    # Search only beside the active asset. Historical patch directories are
    # deliberately outside this reproducible source/asset workflow.
    $referenceCandidates = @(Get-ChildItem -LiteralPath (Split-Path $resolvedItemList -Parent) -Filter 'ItemList.pre-grid1x1*.bin' -File)
    if ($referenceCandidates.Count -eq 0) {
        throw 'No ItemList.pre-grid1x1*.bin reference was found beside the active asset. Pass -ReferenceItemList explicitly.'
    }
    if ($referenceCandidates.Count -gt 1) {
        $candidateNames = ($referenceCandidates.Name -join ', ')
        throw "More than one ItemList reference was found ($candidateNames). Pass -ReferenceItemList explicitly."
    }
    $ReferenceItemList = $referenceCandidates[0].FullName
}

if (-not (Test-Path -LiteralPath $ReferenceItemList -PathType Leaf)) {
    throw "Reference ItemList.bin not found: $ReferenceItemList"
}
$resolvedReference = (Resolve-Path -LiteralPath $ReferenceItemList).Path
if ($resolvedReference -eq $resolvedItemList) {
    throw 'The reference ItemList.bin must be different from the active asset.'
}

$reference = [IO.File]::ReadAllBytes($resolvedReference)
$referenceMeasure = Measure-ItemGrid $reference 'reference ItemList.bin'
$referenceHash = Get-ByteHash $reference
$referenceStamp = Get-StampHex $reference

# The expected canonical file is always regenerated from the unmodified
# reference. This proves that every byte outside EF_GRID values is preserved.
$candidate = [byte[]]::new($reference.Length)
[Array]::Copy($reference, $candidate, $reference.Length)
$allowedOffsets = [Collections.Generic.HashSet[int]]::new()
$changedPairs = 0

for ($itemIndex = 0; $itemIndex -lt $recordCount; $itemIndex++) {
    for ($effectIndex = 0; $effectIndex -lt $effectCount; $effectIndex++) {
        $typeOffset = ($itemIndex * $recordSize) + $effectOffset + ($effectIndex * $effectSize)
        if ((Get-DecodedUInt16 $candidate $typeOffset) -ne $gridEffect) {
            continue
        }

        $valueOffset = $typeOffset + 2
        [void]$allowedOffsets.Add($valueOffset)
        [void]$allowedOffsets.Add($valueOffset + 1)
        if ((Get-DecodedUInt16 $candidate $valueOffset) -ne 0) {
            $changedPairs++
        }
        Set-EncodedUInt16 $candidate $valueOffset 0
    }
}

if ($changedPairs -ne $referenceMeasure.NonZeroPairs) {
    throw "Internal EF_GRID count mismatch: measured $($referenceMeasure.NonZeroPairs), changed $changedPairs."
}

$candidateMeasure = Measure-ItemGrid $candidate 'canonical candidate'
if ($candidateMeasure.NonZeroPairs -ne 0 -or
    $candidateMeasure.Pairs -ne $referenceMeasure.Pairs -or
    $candidateMeasure.Records -ne $referenceMeasure.Records) {
    throw 'Candidate validation failed after EF_GRID normalization.'
}

$changedBytes = 0
for ($offset = 0; $offset -lt $candidate.Length; $offset++) {
    if ($candidate[$offset] -eq $reference[$offset]) {
        continue
    }
    if (-not $allowedOffsets.Contains($offset)) {
        throw ('Candidate changed a byte outside an EF_GRID value at offset 0x{0:X}.' -f $offset)
    }
    $changedBytes++
}

$candidateStamp = Get-StampHex $candidate
if ($candidateStamp -ne $referenceStamp) {
    throw 'The canonical candidate changed the trailing four-byte stamp.'
}
$candidateHash = Get-ByteHash $candidate

$activeBefore = [IO.File]::ReadAllBytes($resolvedItemList)
$activeBeforeMeasure = Measure-ItemGrid $activeBefore 'active ItemList.bin'
$activeBeforeHash = Get-ByteHash $activeBefore
$activeBeforeStamp = Get-StampHex $activeBefore

if ($VerifyOnly) {
    $difference = Measure-ByteDifferences $candidate $activeBefore
    if ($difference.Count -ne 0) {
        throw ('Active ItemList.bin differs from the canonical reference-derived result in {0} bytes; first mismatch at 0x{1:X}. Expected SHA-256 {2}, found {3}.' -f
            $difference.Count, $difference.FirstOffset, $candidateHash, $activeBeforeHash)
    }
    if ($activeBeforeStamp -ne $referenceStamp) {
        throw "Active ItemList.bin stamp $activeBeforeStamp differs from reference stamp $referenceStamp."
    }

    [pscustomobject]@{
        ItemList = $resolvedItemList
        ReferenceItemList = $resolvedReference
        Records = $recordCount
        GridEffects = $activeBeforeMeasure.Pairs
        NonZeroGridEffects = $activeBeforeMeasure.NonZeroPairs
        ReferenceNonZeroGridEffects = $referenceMeasure.NonZeroPairs
        ChangedGridEffects = $changedPairs
        ChangedBytes = $changedBytes
        XOR = 'VALID (0x5A)'
        Stamp = $activeBeforeStamp
        StampMatchesReference = $true
        ReferenceSHA256 = $referenceHash
        SHA256 = $activeBeforeHash
        Status = 'VERIFIED AGAINST REFERENCE'
    }
    return
}

# Write beside the destination and atomically replace it only after the complete
# candidate passes shape, XOR, effect-count, diff-scope and stamp validation.
$temporary = Join-Path (Split-Path $resolvedItemList -Parent) ('.{0}.update.{1}.tmp' -f (Split-Path $resolvedItemList -Leaf), $PID)
$backup = Join-Path (Split-Path $resolvedItemList -Parent) ('.{0}.backup.{1}.tmp' -f (Split-Path $resolvedItemList -Leaf), $PID)
try {
    [IO.File]::WriteAllBytes($temporary, $candidate)
    $temporaryData = [IO.File]::ReadAllBytes($temporary)
    $temporaryMeasure = Measure-ItemGrid $temporaryData 'temporary ItemList.bin'
    $temporaryDifference = Measure-ByteDifferences $candidate $temporaryData
    if ($temporaryMeasure.NonZeroPairs -ne 0 -or $temporaryDifference.Count -ne 0) {
        throw 'Temporary ItemList.bin verification failed.'
    }

    # Windows performs Replace atomically when source, destination and backup
    # share a volume. The backup also keeps the old asset recoverable until the
    # post-write verification below has reopened the destination successfully.
    [IO.File]::Replace($temporary, $resolvedItemList, $backup, $true)
} finally {
    if (Test-Path -LiteralPath $temporary -PathType Leaf) {
        Remove-Item -LiteralPath $temporary -Force
    }
}

$activeAfter = [IO.File]::ReadAllBytes($resolvedItemList)
$activeAfterMeasure = Measure-ItemGrid $activeAfter 'written ItemList.bin'
$writtenDifference = Measure-ByteDifferences $candidate $activeAfter
$activeAfterStamp = Get-StampHex $activeAfter
$activeAfterHash = Get-ByteHash $activeAfter
if ($writtenDifference.Count -ne 0 -or $activeAfterMeasure.NonZeroPairs -ne 0) {
    throw 'Written ItemList.bin does not match the canonical reference-derived result.'
}
if ($activeAfterStamp -ne $referenceStamp) {
    throw 'The trailing ItemList.bin stamp changed during the update.'
}
if (Test-Path -LiteralPath $backup -PathType Leaf) {
    Remove-Item -LiteralPath $backup -Force
}

[pscustomobject]@{
    ItemList = $resolvedItemList
    ReferenceItemList = $resolvedReference
    Records = $recordCount
    GridEffects = $activeAfterMeasure.Pairs
    NonZeroGridEffects = $activeAfterMeasure.NonZeroPairs
    ReferenceNonZeroGridEffects = $referenceMeasure.NonZeroPairs
    ChangedGridEffects = $changedPairs
    ChangedBytes = $changedBytes
    XOR = 'VALID (0x5A)'
    Stamp = $activeAfterStamp
    StampMatchesReference = $true
    ReferenceSHA256 = $referenceHash
    SHA256Before = $activeBeforeHash
    SHA256After = $activeAfterHash
    Status = 'UPDATED AND VERIFIED AGAINST REFERENCE'
}
