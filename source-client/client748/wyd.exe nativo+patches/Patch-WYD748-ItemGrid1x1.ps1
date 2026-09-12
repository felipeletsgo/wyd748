[CmdletBinding()]
param(
    [string]$ItemList = (Join-Path $PSScriptRoot 'ItemList.bin'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# ItemList.bin stores the 7.48 inventory footprint as EF_GRID (33).  Index 0
# is the native 1x1 entry, so normalizing this metadata prevents every grid
# consumer from selecting one of the legacy multi-cell layouts.
$recordCount = 6500
$recordSize = 140
$payloadLength = $recordCount * $recordSize
$expectedLength = $payloadLength + 4
$xorKey = 0x5A
$effectStart = 0x50
$effectCount = 12
$gridEffect = 33
$oneByOneGrid = 0

function Get-Sha([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-DecodedUInt16([byte[]]$Data, [int]$Offset) {
    $lo = [int]$Data[$Offset] -bxor $xorKey
    $hi = [int]$Data[$Offset + 1] -bxor $xorKey
    return $lo + ($hi -shl 8)
}

function Set-EncodedUInt16([byte[]]$Data, [int]$Offset, [uint16]$Value) {
    # Encode only the selected little-endian field; the rest of the record and
    # the four-byte trailing stamp must remain byte-identical.
    $Data[$Offset] = [byte](([int]$Value -band 0xFF) -bxor $xorKey)
    $Data[$Offset + 1] = [byte]((([int]$Value -shr 8) -band 0xFF) -bxor $xorKey)
}

function Assert-ItemListShape([byte[]]$Data) {
    if ($Data.Length -ne $expectedLength) {
        throw "ItemList.bin com tamanho inesperado: $($Data.Length) bytes; esperado $expectedLength (6500 x 140 + stamp)."
    }
}

function Measure-GridMetadata([byte[]]$Data) {
    $pairs = 0
    $nonZeroPairs = 0
    $records = 0

    for ($itemID = 0; $itemID -lt $recordCount; $itemID++) {
        $recordHasGrid = $false
        for ($effectSlot = 0; $effectSlot -lt $effectCount; $effectSlot++) {
            $typeOffset = $itemID * $recordSize + $effectStart + $effectSlot * 4
            if ((Get-DecodedUInt16 $Data $typeOffset) -ne $gridEffect) { continue }

            $recordHasGrid = $true
            $pairs++
            $value = Get-DecodedUInt16 $Data ($typeOffset + 2)
            if ($value -gt 7) {
                throw "item $itemID, efeito $effectSlot`: EF_GRID invalido ($value); esperado indice entre 0 e 7."
            }
            if ($value -ne $oneByOneGrid) { $nonZeroPairs++ }
        }
        if ($recordHasGrid) { $records++ }
    }

    if ($pairs -eq 0) {
        throw 'Nenhum EF_GRID foi encontrado; o layout do ItemList.bin nao corresponde ao catalogo 7.48 esperado.'
    }
    return [pscustomobject]@{ Pairs = $pairs; NonZeroPairs = $nonZeroPairs; Records = $records }
}

if (-not (Test-Path -LiteralPath $ItemList -PathType Leaf)) {
    throw "ItemList.bin nao encontrado: $ItemList"
}

$resolvedItemList = (Resolve-Path -LiteralPath $ItemList).Path
$before = [IO.File]::ReadAllBytes($resolvedItemList)
Assert-ItemListShape $before
$beforeMeasure = Measure-GridMetadata $before
$beforeHash = Get-Sha $resolvedItemList

if ($beforeMeasure.NonZeroPairs -eq 0) {
    Write-Host ("ItemList ja normalizado: {0} registros e {1} pares EF_GRID usam 1x1." -f
        $beforeMeasure.Records, $beforeMeasure.Pairs)
    Write-Host ("SHA-256: {0}" -f $beforeHash)
    return
}

if ($VerifyOnly) {
    throw ("ItemList ainda possui {0} pares EF_GRID fora de 1x1, em {1} registros com EF_GRID." -f
        $beforeMeasure.NonZeroPairs, $beforeMeasure.Records)
}

# A backup content-addressed preserves every distinct pre-patch catalog without
# conflating it with backups from earlier item-import stages.
$backupName = 'ItemList.pre-grid1x1.{0}.bin' -f $beforeHash.Substring(0, 12)
$backup = Join-Path (Split-Path -Parent $resolvedItemList) $backupName
if (Test-Path -LiteralPath $backup -PathType Leaf) {
    $backupHash = Get-Sha $backup
    if ($backupHash -ne $beforeHash) {
        throw "Backup existente diverge do catalogo atual: $backup (SHA-256: $backupHash)."
    }
} else {
    Copy-Item -LiteralPath $resolvedItemList -Destination $backup
    if ((Get-Sha $backup) -ne $beforeHash) {
        throw "Falha ao validar o backup antes do patch: $backup"
    }
}

$candidate = New-Object byte[] $before.Length
[Array]::Copy($before, $candidate, $before.Length)
$allowedOffsets = [Collections.Generic.HashSet[int]]::new()
$changedPairs = 0
for ($itemID = 0; $itemID -lt $recordCount; $itemID++) {
    for ($effectSlot = 0; $effectSlot -lt $effectCount; $effectSlot++) {
        $typeOffset = $itemID * $recordSize + $effectStart + $effectSlot * 4
        if ((Get-DecodedUInt16 $candidate $typeOffset) -ne $gridEffect) { continue }

        $valueOffset = $typeOffset + 2
        if ((Get-DecodedUInt16 $candidate $valueOffset) -eq $oneByOneGrid) { continue }
        [void]$allowedOffsets.Add($valueOffset)
        [void]$allowedOffsets.Add($valueOffset + 1)
        Set-EncodedUInt16 $candidate $valueOffset $oneByOneGrid
        $changedPairs++
    }
}

if ($changedPairs -ne $beforeMeasure.NonZeroPairs) {
    throw "Contagem interna divergente: medidos $($beforeMeasure.NonZeroPairs), preparados $changedPairs."
}

$candidateMeasure = Measure-GridMetadata $candidate
if ($candidateMeasure.NonZeroPairs -ne 0 -or
    $candidateMeasure.Pairs -ne $beforeMeasure.Pairs -or
    $candidateMeasure.Records -ne $beforeMeasure.Records) {
    throw 'Validacao do candidato falhou: os pares EF_GRID nao foram preservados e normalizados integralmente.'
}

$changedBytes = 0
for ($offset = 0; $offset -lt $candidate.Length; $offset++) {
    if ($candidate[$offset] -eq $before[$offset]) { continue }
    if (-not $allowedOffsets.Contains($offset)) {
        throw ('Candidato alterou byte fora de um valor EF_GRID: offset 0x{0:X}.' -f $offset)
    }
    $changedBytes++
}

$temporary = "$resolvedItemList.grid1x1.$PID.tmp"
try {
    [IO.File]::WriteAllBytes($temporary, $candidate)
    $temporaryData = [IO.File]::ReadAllBytes($temporary)
    Assert-ItemListShape $temporaryData
    $temporaryMeasure = Measure-GridMetadata $temporaryData
    if ($temporaryMeasure.NonZeroPairs -ne 0) {
        throw "Validacao do arquivo temporario falhou: $($temporaryMeasure.NonZeroPairs) EF_GRID nao normalizados."
    }

    Copy-Item -LiteralPath $temporary -Destination $resolvedItemList -Force
} finally {
    if (Test-Path -LiteralPath $temporary -PathType Leaf) {
        Remove-Item -LiteralPath $temporary -Force
    }
}

$after = [IO.File]::ReadAllBytes($resolvedItemList)
Assert-ItemListShape $after
$afterMeasure = Measure-GridMetadata $after
if ($afterMeasure.NonZeroPairs -ne 0 -or $afterMeasure.Pairs -ne $beforeMeasure.Pairs) {
    throw 'Validacao pos-patch falhou: o ItemList gravado nao esta integralmente em 1x1.'
}

for ($offset = 0; $offset -lt $after.Length; $offset++) {
    if ($after[$offset] -eq $before[$offset]) { continue }
    if (-not $allowedOffsets.Contains($offset)) {
        throw ('Arquivo gravado alterou byte fora de um valor EF_GRID: offset 0x{0:X}.' -f $offset)
    }
}

$afterHash = Get-Sha $resolvedItemList
Write-Host ("EF_GRID normalizado para 1x1 em {0} pares; {1} registros com EF_GRID verificados." -f
    $changedPairs, $afterMeasure.Records)
Write-Host ("Bytes efetivamente alterados: {0}; stamp final preservado." -f $changedBytes)
Write-Host ("SHA-256 antes:  {0}" -f $beforeHash)
Write-Host ("SHA-256 depois: {0}" -f $afterHash)
Write-Host ("Backup: {0}" -f $backup)
