[CmdletBinding()]
param(
    [string]$ItemList = (Join-Path $PSScriptRoot 'ItemList.bin'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Materializa as aparencias de montaria do client KR como montarias simples do
# 7.48. O registro 342 (Shire) fornece icone e Equip[14]. Cada montaria recebe
# o contrato premium exibido pelo client; prazo e bonus continuam autoritativos
# no servidor e vinculados ao UID da instancia.

$recordCount = 6500
$recordSize = 140
$xorKey = 0x5A
$effectStart = 0x50
$sourceID = 342
$supportedInputs = @(
    '9FDCCFAE6D074744CE8BCAEDDBCBF3DC90136EEC34F879EF545D50ABA45AB903',
    # Build legado que materializava 47 aparencias, incluindo dois fallbacks sem fonte.
    '24534F712879BE18F8A3E89DC48FEA35783E047721EEA3FEF1CACFE746C24EA5',
    # Saida anterior das 45 montarias completas, antes das faces KR.
    '2FD8E4DB869257307ECA91BF99B47F408C36BB9C4A58AC025E48657F3EB007FE',
    # Costumes premium ja atualizados; montarias ainda no contrato Shire antigo.
    'A2941E51F451F4DAAC4E2603C06196171645179D8736108C9E39913F6BEAFF2C',
    # ItemList atual com a colecao de mobs KR e as 45 montarias anteriores.
    '613308B643C80B777C173D0854C0702837A7CEDED2C25F15482CB1407F53E6FA'
)
$expectedOutputHash = '678D5A500B70F8DC83B26387C4884B05336D09B0BBB3190AD34FF73DB6D977B1' # KR_MOUNT_ITEM_OUTPUT_HASH

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
function Assert-EmptyRecord([byte[]]$Record, $Item) {
    for ($i = 0; $i -lt $Record.Length; $i++) { if ($Record[$i] -ne 0) { throw "item $($Item.item): registro indisponivel nao esta vazio" } }
}
function Assert-Record([byte[]]$Record, $Item) {
    if ((Record-Name $Record) -ne [string]$Item.name) { throw "item $($Item.item): nome divergente" }
    if ([BitConverter]::ToUInt16($Record, 0x40) -ne 7) { throw "item $($Item.item): icone nao e o da Shire" }
    if ([BitConverter]::ToUInt16($Record, 0x86) -ne 16384) { throw "item $($Item.item): slot nao e Equip[14]" }
    $expected = @(
        @(18,43),
        @(27,2),
        @(111,1),
        @(106,30),
        @(29,6),
        @(2,520),
        @(60,65)
    )
    for ($i = 0; $i -lt $expected.Count; $i++) {
        $offset = $effectStart + $i * 4
        if ([BitConverter]::ToUInt16($Record, $offset) -ne $expected[$i][0] -or
            [BitConverter]::ToUInt16($Record, $offset + 2) -ne $expected[$i][1]) {
            throw "item $($Item.item): contrato Shire divergente no efeito $i"
        }
    }
}

if (-not (Test-Path -LiteralPath $ItemList -PathType Leaf)) { throw "ItemList.bin ausente: $ItemList" }
if (-not (Test-Path -LiteralPath $Manifest -PathType Leaf)) { throw "manifesto ausente: $Manifest" }
$definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
$catalogItems = @($definition.items | Sort-Object item)
$items = @($catalogItems | Where-Object { $_.available -ne $false })
$unavailableItems = @($catalogItems | Where-Object { $_.available -eq $false })
if ($catalogItems.Count -ne 62 -or $items.Count -ne 59 -or $unavailableItems.Count -ne 3) {
    throw "manifesto inesperado: catalogo=$($catalogItems.Count) disponiveis=$($items.Count) indisponiveis=$($unavailableItems.Count)"
}

$data = [IO.File]::ReadAllBytes($ItemList)
if ($data.Length -ne $recordCount * $recordSize + 4) { throw "ItemList.bin com tamanho inesperado: $($data.Length)" }
$alreadyInstalled = $true
foreach ($item in $items) {
    try { Assert-Record (Decode-Record $data $item.item) $item } catch { $alreadyInstalled = $false; break }
}
if ($alreadyInstalled) {
    foreach ($item in $unavailableItems) {
        try { Assert-EmptyRecord (Decode-Record $data $item.item) $item } catch { $alreadyInstalled = $false; break }
    }
}
if ($alreadyInstalled) {
    $hash = Get-Sha $ItemList
    Write-Host "Todas as $($items.Count) montarias KR disponiveis ja estao no ItemList.bin; 3 casos incompletos permanecem vazios."
    Write-Host "SHA-256: $hash"
    return
}
if ($VerifyOnly) { throw 'ItemList.bin ainda nao contem toda a colecao de montarias KR.' }
$hash = Get-Sha $ItemList
if ($supportedInputs -notcontains $hash) { throw "ItemList.bin fora da cadeia suportada (SHA-256: $hash)" }

$source = Decode-Record $data $sourceID
if ((Record-Name $source) -ne 'Shire_Horse' -or
    [BitConverter]::ToUInt16($source, 0x40) -ne 7 -or
    [BitConverter]::ToUInt16($source, 0x86) -ne 16384) {
    throw 'registro-base 342 nao corresponde a Shire 7.48 esperada'
}

$backup = Join-Path (Split-Path -Parent $ItemList) 'ItemList.pre-mounts-kr.bin'
if (-not (Test-Path -LiteralPath $backup -PathType Leaf)) { Copy-Item -LiteralPath $ItemList -Destination $backup }
foreach ($item in $unavailableItems) {
    $empty = New-Object byte[] $recordSize
    Encode-Record $data $item.item $empty
}
foreach ($item in $items) {
    $record = New-Object byte[] $recordSize
    [Array]::Copy($source, $record, $recordSize)
    [Array]::Clear($record, 0, 64)
    $name = [Text.Encoding]::ASCII.GetBytes([string]$item.name)
    if ($name.Length -ge 64) { throw "item $($item.item): nome excede 63 bytes" }
    [Array]::Copy($name, 0, $record, 0, $name.Length)
    [Array]::Clear($record, $effectStart, 48)
    $effects = @(
        @(18,43),
        @(27,2),
        @(111,1),
        @(106,30),
        @(29,6),
        @(2,520),
        @(60,65)
    )
    for ($i = 0; $i -lt $effects.Count; $i++) {
        Set-U16 $record ($effectStart + $i * 4) ([uint16]$effects[$i][0])
        Set-U16 $record ($effectStart + $i * 4 + 2) ([uint16]$effects[$i][1])
    }
    Encode-Record $data $item.item $record
}
[IO.File]::WriteAllBytes($ItemList, $data)
$written = [IO.File]::ReadAllBytes($ItemList)
foreach ($item in $items) { Assert-Record (Decode-Record $written $item.item) $item }
foreach ($item in $unavailableItems) { Assert-EmptyRecord (Decode-Record $written $item.item) $item }
$newHash = Get-Sha $ItemList
if ($expectedOutputHash -and $newHash -ne $expectedOutputHash) { throw "saida divergente: $newHash" }
Write-Host "$($items.Count) montarias KR disponiveis instaladas no ItemList.bin 7.48; 3 casos incompletos desabilitados."
Write-Host "SHA-256: $newHash"
