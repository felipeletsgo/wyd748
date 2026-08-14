[CmdletBinding()]
param(
    [string]$ItemList = (Join-Path $PSScriptRoot 'ItemList.bin'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Costumes-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Materializes only costumes whose complete mesh/texture set is present in the
# supplied Korean clients. All reuse the native Uniform Set icon (36), occupy
# Equip[13] (Pos=8192), and retain the 7.48 costume effects. Class masks follow
# the renderer skeleton: 5=TK/BM, 10=FM/HT, 15=dynamic/unisex.

$recordCount = 6500
$recordSize = 140
$xorKey = 0x5A
$effectStart = 0x50
$sourceID = 4152
$supportedInputs = @(
    '57BAA28EFE62F6CE5323608E5881C209638C0CC6F6A15F3D879FAC39FFF5D9C2',
    '42293947CADF4F89DAB4A457E400AA901049DEF817403E7D206F4E37BE090622',
    # Current complete client before UID-bound premium lifetime was added.
    '1FC562F273E506972517B39B3D60C7107786D25D80D48C753B569853AAD542AD',
    # Imported collection updated; the seven native costumes still lack WDAY.
    '2FD8E4DB869257307ECA91BF99B47F408C36BB9C4A58AC025E48657F3EB007FE'
)
$nativeCostumeIDs = 4150..4156

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant() }
function Decode-Record([byte[]]$Data, [int]$ID) {
    $record = New-Object byte[] $recordSize
    for ($i = 0; $i -lt $recordSize; $i++) { $record[$i] = [byte]($Data[$ID * $recordSize + $i] -bxor $xorKey) }
    return $record
}
function Encode-Record([byte[]]$Data, [int]$ID, [byte[]]$Record) {
    for ($i = 0; $i -lt $recordSize; $i++) { $Data[$ID * $recordSize + $i] = [byte]($Record[$i] -bxor $xorKey) }
}
function Record-Name([byte[]]$Record) { return [Text.Encoding]::ASCII.GetString($Record, 0, 64).Trim([char]0) }
function Set-U16([byte[]]$Record, [int]$Offset, [uint16]$Value) {
    [Array]::Copy([BitConverter]::GetBytes($Value), 0, $Record, $Offset, 2)
}
function Assert-Record([byte[]]$Record, $Item) {
    if ((Record-Name $Record) -ne [string]$Item.name) { throw "item $($Item.item): nome divergente" }
    if ([BitConverter]::ToUInt16($Record, 0x40) -ne 36) { throw "item $($Item.item): icone nao e 36" }
    if ([BitConverter]::ToUInt16($Record, 0x86) -ne 8192) { throw "item $($Item.item): slot nao e Equip[13]" }
    $expected = @(
        @(18, [int]$Item.classMask),
        @(111, 1),
        @(3, 80),
        @(44, 10),
        @(106, 30)
    )
    for ($i = 0; $i -lt $expected.Count; $i++) {
        $offset = $effectStart + $i * 4
        if ([BitConverter]::ToUInt16($Record, $offset) -ne $expected[$i][0] -or
            [BitConverter]::ToUInt16($Record, $offset + 2) -ne $expected[$i][1]) {
            throw "item $($Item.item): efeito $i divergente"
        }
    }
}
function Assert-NativePremium([byte[]]$Record, [int]$ItemID) {
    if ([BitConverter]::ToUInt16($Record, 0x40) -ne 36 -or
        [BitConverter]::ToUInt16($Record, 0x86) -ne 8192 -or
        [BitConverter]::ToUInt16($Record, ($effectStart + 16)) -ne 106 -or
        [BitConverter]::ToUInt16($Record, ($effectStart + 18)) -ne 30) {
        throw "item nativo $ItemID nao possui o contrato premium de 30 dias"
    }
}

if (-not (Test-Path -LiteralPath $ItemList -PathType Leaf)) { throw "ItemList.bin ausente: $ItemList" }
if (-not (Test-Path -LiteralPath $Manifest -PathType Leaf)) { throw "manifesto ausente: $Manifest" }
$definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
$items = @($definition.items | Where-Object available | Sort-Object item)
if ($items.Count -ne 135) { throw "manifesto inesperado: $($items.Count) trajes completos; esperado 135" }

$data = [IO.File]::ReadAllBytes($ItemList)
if ($data.Length -ne $recordCount * $recordSize + 4) { throw "ItemList.bin com tamanho inesperado: $($data.Length)" }
$alreadyInstalled = $true
foreach ($item in $items) {
    try { Assert-Record (Decode-Record $data $item.item) $item } catch { $alreadyInstalled = $false; break }
}
if ($alreadyInstalled) {
    foreach ($itemID in $nativeCostumeIDs) {
        try { Assert-NativePremium (Decode-Record $data $itemID) $itemID } catch { $alreadyInstalled = $false; break }
    }
}
if ($alreadyInstalled) {
    Write-Host "Todos os $($items.Count) trajes KR ja estao instalados no ItemList.bin."
    Write-Host "SHA-256: $(Get-Sha $ItemList)"
    return
}
if ($VerifyOnly) { throw 'ItemList.bin ainda nao contem toda a colecao de trajes KR.' }
$hash = Get-Sha $ItemList
if ($supportedInputs -notcontains $hash) { throw "ItemList.bin fora da cadeia suportada (SHA-256: $hash)" }

$source = Decode-Record $data $sourceID
if ((Record-Name $source) -notin @('Uniform_Set', 'Uniform_Set(30days)') -or
    [BitConverter]::ToUInt16($source, 0x40) -ne 36 -or
    [BitConverter]::ToUInt16($source, 0x86) -ne 8192) {
    throw 'registro-base 4152 nao corresponde ao Uniform Set 7.48 esperado'
}

$backup = Join-Path (Split-Path -Parent $ItemList) 'ItemList.pre-costumes-kr.bin'
if (-not (Test-Path -LiteralPath $backup -PathType Leaf)) { Copy-Item -LiteralPath $ItemList -Destination $backup }

foreach ($item in $items) {
    $record = New-Object byte[] $recordSize
    [Array]::Copy($source, $record, $recordSize)
    [Array]::Clear($record, 0, 64)
    $name = [Text.Encoding]::ASCII.GetBytes([string]$item.name)
    if ($name.Length -ge 64) { throw "item $($item.item): nome excede 63 bytes" }
    [Array]::Copy($name, 0, $record, 0, $name.Length)
    Set-U16 $record 0x40 36
    [Array]::Clear($record, $effectStart, 48)
    Set-U16 $record $effectStart 18
    Set-U16 $record ($effectStart + 2) ([uint16]$item.classMask)
    Set-U16 $record ($effectStart + 4) 111
    Set-U16 $record ($effectStart + 6) 1
    Set-U16 $record ($effectStart + 8) 3
    Set-U16 $record ($effectStart + 10) 80
    Set-U16 $record ($effectStart + 12) 44
    Set-U16 $record ($effectStart + 14) 10
    Set-U16 $record ($effectStart + 16) 106
    Set-U16 $record ($effectStart + 18) 30
    Set-U16 $record 0x86 8192
    Assert-Record $record $item
    Encode-Record $data $item.item $record
}
foreach ($itemID in $nativeCostumeIDs) {
    $record = Decode-Record $data $itemID
    Set-U16 $record ($effectStart + 16) 106
    Set-U16 $record ($effectStart + 18) 30
    Assert-NativePremium $record $itemID
    Encode-Record $data $itemID $record
}
[IO.File]::WriteAllBytes($ItemList, $data)
$written = [IO.File]::ReadAllBytes($ItemList)
foreach ($item in $items) { Assert-Record (Decode-Record $written $item.item) $item }
foreach ($itemID in $nativeCostumeIDs) { Assert-NativePremium (Decode-Record $written $itemID) $itemID }
Write-Host "$($items.Count) trajes KR instalados no ItemList.bin 7.48."
Write-Host "SHA-256: $(Get-Sha $ItemList)"
