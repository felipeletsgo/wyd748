[CmdletBinding()]
param(
    [string]$ItemList = (Join-Path $PSScriptRoot 'ItemList.bin'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Client compatibility metadata (not a gameplay rule).
#
# The 7.48 right-click gate only recognises the old volatile allow-list.  It
# does not contain W2PP's explicit item-ID exception for Warrior's Seal
# (item 4146), so a correctly encoded volatile 199 is rejected before the
# native 0x373 UseItem packet is built.  The server remains authoritative and
# still resolves item 4146 to volatile 199.  This patch changes only the
# client-side clickability marker from volatile 199 to the known clickable
# marker 1; it does not change the item ID, name, price, visual effects, or
# any server-side volatile data.
#
# ItemList.bin format used by this client:
#   6500 records x 140 bytes, XOR 0x5A, followed by a four-byte random stamp.
#   The native reader computes BASE_GetSum2 over the encoded payload, then
#   reads that stamp but never compares it. The executable chain already
#   bypasses both embedded BASE_GetSum2 constants, so this script preserves the
#   stamp and changes no unrelated payload bytes.
#   item 4146, decoded EF_VOLATILE pair: type at record+0x58 and value at
#   record+0x5A (each field is a little-endian uint16).

$recordCount = 6500
$recordSize = 140
$payloadLength = $recordCount * $recordSize
$expectedLength = $payloadLength + 4
$xorKey = 0x5A
$itemID = 4146
$effectTypeOffset = 0x58
$effectValueOffset = 0x5A
$volatileEffect = 38       # EF_VOLATILE
$serverVolatile = 199
$clientClickableMarker = 1
$backupName = 'ItemList.pre-warrior-seal.bin'

function Get-DecodedUInt16([byte[]]$Data, [int]$Offset, [int]$Xor) {
    $lo = [int]$Data[$Offset] -bxor $Xor
    $hi = [int]$Data[$Offset + 1] -bxor $Xor
    return $lo + ($hi -shl 8)
}

function Assert-ItemListShape([byte[]]$Data) {
    if ($Data.Length -ne $expectedLength) {
        throw "ItemList.bin com tamanho inesperado: $($Data.Length) bytes; esperado $expectedLength (6500 x 140 + checksum)."
    }
}

if (-not (Test-Path -LiteralPath $ItemList -PathType Leaf)) {
    throw "ItemList.bin nao encontrado: $ItemList"
}

$data = [IO.File]::ReadAllBytes($ItemList)
Assert-ItemListShape $data

$recordOffset = $itemID * $recordSize
$effectType = Get-DecodedUInt16 $data ($recordOffset + $effectTypeOffset) $xorKey
$effectValue = Get-DecodedUInt16 $data ($recordOffset + $effectValueOffset) $xorKey
if ($effectType -ne $volatileEffect) {
    throw ('item {0}: efeito em 0x{1:X} nao e EF_VOLATILE (esperado {2}, encontrado {3}).' -f
        $itemID, $effectTypeOffset, $volatileEffect, $effectType)
}

$currentHash = (Get-FileHash -LiteralPath $ItemList -Algorithm SHA256).Hash.ToUpperInvariant()
if ($effectValue -eq $clientClickableMarker) {
    Write-Host ("ItemList ja possui marcador de clique para item {0}; volatile server-side continua {1}." -f
        $itemID, $serverVolatile)
    Write-Host ("SHA-256: {0}" -f $currentHash)
    return
}
if ($effectValue -ne $serverVolatile) {
    throw ('item {0}: EF_VOLATILE inesperado no client: {1}. Esperado {2} (fonte) ou {3} (marcador ja aplicado). Nenhum byte sera sobrescrito.' -f
        $itemID, $effectValue, $serverVolatile, $clientClickableMarker)
}

if ($VerifyOnly) {
    throw ('marcador de clique ainda nao aplicado ao item {0}: o client ve volatile {1}, mas precisa do marcador {2}.' -f
        $itemID, $serverVolatile, $clientClickableMarker)
}

$backup = Join-Path (Split-Path -Parent $ItemList) $backupName
if (Test-Path -LiteralPath $backup -PathType Leaf) {
    $backupHash = (Get-FileHash -LiteralPath $backup -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($backupHash -ne $currentHash) {
        throw "Backup existente nao corresponde ao ItemList.bin antes do patch: $backup (SHA-256: $backupHash). Nao sera sobrescrito."
    }
} else {
    Copy-Item -LiteralPath $ItemList -Destination $backup
}

$data[$recordOffset + $effectValueOffset] = [byte]($clientClickableMarker -bxor $xorKey)
[IO.File]::WriteAllBytes($ItemList, $data)

$patched = [IO.File]::ReadAllBytes($ItemList)
Assert-ItemListShape $patched
$patchedType = Get-DecodedUInt16 $patched ($recordOffset + $effectTypeOffset) $xorKey
$patchedValue = Get-DecodedUInt16 $patched ($recordOffset + $effectValueOffset) $xorKey
if ($patchedType -ne $volatileEffect -or $patchedValue -ne $clientClickableMarker) {
    throw ('validacao pos-patch falhou para item {0}: type={1}, value={2}.' -f
        $itemID, $patchedType, $patchedValue)
}

$afterHash = (Get-FileHash -LiteralPath $ItemList -Algorithm SHA256).Hash.ToUpperInvariant()
Write-Host ("Marcador de clique aplicado ao item {0}: client EF_VOLATILE {1} -> {2}." -f
    $itemID, $serverVolatile, $clientClickableMarker)
Write-Host ("Servidor continua usando volatile {0} para item {1}." -f $serverVolatile, $itemID)
Write-Host 'Stamp final do ItemList preservado; o client usa o bypass da cadeia para BASE_GetSum2.'
Write-Host ("SHA-256 antes:  {0}" -f $currentHash)
Write-Host ("SHA-256 depois: {0}" -f $afterHash)
Write-Host ("Backup: {0}" -f $backup)
