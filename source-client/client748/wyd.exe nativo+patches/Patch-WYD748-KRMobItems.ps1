[CmdletBinding()]
param(
    [string]$ItemList = (Join-Path $PSScriptRoot 'ItemList.bin'),
    [string]$SourceItemList = (Join-Path (Split-Path -Parent $PSScriptRoot) 'CLIENTS\WYD\ItemList.bin'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'MobFaces-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Translates the modern 172-byte KR face records into the 140-byte 7.48 ABI.
# Source IDs above 4095 are intentionally remapped: compact CreateMob refreshes
# preserve only twelve face bits. Effects and legacy tail fields are rebuilt,
# never copied across layouts.

$targetRecordSize = 140
$targetRecordCount = 6500
$sourceRecordSize = 172
$sourceRecordCount = 14500
$xorKey = 0x5A
$supportedInputs = @(
    '2FD8E4DB869257307ECA91BF99B47F408C36BB9C4A58AC025E48657F3EB007FE',
    '678D5A500B70F8DC83B26387C4884B05336D09B0BBB3190AD34FF73DB6D977B1'
)
$expectedOutputHash = '2C9323E0374FCE22D33CD9C29F633D2650559364B298D1FD73CD2836A18F0DC5' # KRMOB_ITEM_OUTPUT_HASH
$grid1x1OutputHash = '112C6CFAC316232F009CB7250538F4337E9A35FDBF084E6A2742D444AE8627E3'

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant() }
function Decode-Record([byte[]]$Data, [int]$Size, [int]$ID) {
    $record = New-Object byte[] $Size
    for ($i = 0; $i -lt $Size; $i++) { $record[$i] = [byte]($Data[$ID * $Size + $i] -bxor $xorKey) }
    return $record
}
function Encode-Record([byte[]]$Data, [int]$ID, [byte[]]$Record) {
    for ($i = 0; $i -lt $targetRecordSize; $i++) { $Data[$ID * $targetRecordSize + $i] = [byte]($Record[$i] -bxor $xorKey) }
}
function Set-U16([byte[]]$Data, [int]$Offset, [uint16]$Value) {
    [Array]::Copy([BitConverter]::GetBytes($Value), 0, $Data, $Offset, 2)
}
function Record-Name([byte[]]$Record) {
    $length = 0
    while ($length -lt 64 -and $Record[$length] -ne 0 -and $Record[$length] -ne 0x3F) { $length++ }
    return [Text.Encoding]::ASCII.GetString($Record, 0, $length)
}
function Assert-Record([byte[]]$Record, $Item) {
    if ((Record-Name $Record) -ne [string]$Item.name) { throw "face $($Item.targetItem): nome divergente" }
    if ([BitConverter]::ToUInt16($Record, 0x40) -ne [int]$Item.mesh -or
        [BitConverter]::ToUInt16($Record, 0x42) -ne [int]$Item.texture) {
        throw "face $($Item.targetItem): mesh/texture divergentes"
    }
    if ([BitConverter]::ToUInt16($Record, 0x50) -ne 18 -or
        [BitConverter]::ToUInt16($Record, 0x52) -ne [int]$Item.class -or
        [BitConverter]::ToUInt16($Record, 0x54) -ne 27 -or
        [BitConverter]::ToUInt16($Record, 0x56) -ne [int]$Item.range -or
        [BitConverter]::ToUInt16($Record, 0x86) -ne 1) {
        throw "face $($Item.targetItem): contrato visual 7.48 divergente"
    }
}

foreach ($path in @($ItemList, $SourceItemList, $Manifest)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "arquivo ausente: $path" }
}
$definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
$items = @($definition.items | Sort-Object targetItem)
if ($items.Count -ne 19) { throw "manifesto possui $($items.Count) faces; esperado 19" }

$data = [IO.File]::ReadAllBytes($ItemList)
$sourceData = [IO.File]::ReadAllBytes($SourceItemList)
if ($data.Length -ne $targetRecordCount * $targetRecordSize + 4) { throw 'ItemList 7.48 com tamanho inesperado' }
if ($sourceData.Length -ne $sourceRecordCount * $sourceRecordSize + 4) { throw 'ItemList KR com tamanho inesperado' }

$installed = $true
foreach ($item in $items) {
    try { Assert-Record (Decode-Record $data $targetRecordSize $item.targetItem) $item } catch { $installed = $false; break }
}
if ($installed) {
    $hash = Get-Sha $ItemList
    # The final grid stage changes only EF_GRID values outside these translated
    # face records, so both deterministic chain states are valid here.
    if ($hash -notin @($expectedOutputHash, $grid1x1OutputHash)) {
        throw "ItemList KR mobs divergiu dos SHAs esperados: $hash"
    }
    Write-Host 'As 19 faces KR de monstros ja estao materializadas no ItemList 7.48.'
    Write-Host "SHA-256: $hash"
    return
}
if ($VerifyOnly) { throw 'ItemList.bin ainda nao contem as 19 faces KR de monstros.' }
$inputHash = Get-Sha $ItemList
if ($supportedInputs -notcontains $inputHash) { throw "ItemList.bin fora da entrada suportada (SHA-256: $inputHash)" }

$backup = Join-Path (Split-Path -Parent $ItemList) 'ItemList.pre-mobs-kr.bin'
if (-not (Test-Path -LiteralPath $backup -PathType Leaf)) { Copy-Item -LiteralPath $ItemList -Destination $backup }

foreach ($item in $items) {
    $source = Decode-Record $sourceData $sourceRecordSize $item.sourceItem
    if (-not (Record-Name $source)) { throw "face fonte KR $($Item.sourceItem) esta vazia" }
    if ([BitConverter]::ToUInt16($source, 0x52) -ne [int]$item.class) {
        throw "face fonte KR $($item.sourceItem) possui EF_CLASS divergente"
    }
    $record = New-Object byte[] $targetRecordSize
    $name = [Text.Encoding]::ASCII.GetBytes([string]$item.name)
    if ($name.Length -gt 12) { throw "face $($item.targetItem): nome excede o wire de 12 bytes" }
    [Array]::Copy($name, 0, $record, 0, $name.Length)
    Set-U16 $record 0x40 ([uint16]$item.mesh)
    Set-U16 $record 0x42 ([uint16]$item.texture)
    Set-U16 $record 0x50 18
    Set-U16 $record 0x52 ([uint16]$item.class)
    Set-U16 $record 0x54 27
    Set-U16 $record 0x56 ([uint16]$item.range)
    Set-U16 $record 0x86 1
    Assert-Record $record $item
    Encode-Record $data $item.targetItem $record
}

[IO.File]::WriteAllBytes($ItemList, $data)
$written = [IO.File]::ReadAllBytes($ItemList)
foreach ($item in $items) { Assert-Record (Decode-Record $written $targetRecordSize $item.targetItem) $item }
$outputHash = Get-Sha $ItemList
if ($expectedOutputHash -and $outputHash -ne $expectedOutputHash) { throw "saida divergente: $outputHash" }
Write-Host '19 faces KR de monstros materializadas no ItemList.bin 7.48.'
Write-Host "SHA-256: $outputHash"
