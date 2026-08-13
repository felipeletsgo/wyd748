[CmdletBinding()]
param(
    [string]$ItemList = (Join-Path $PSScriptRoot 'ItemList.bin'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Materializa as aparencias de montaria do client KR como montarias simples do
# 7.48. O registro 342 (Shire) e a unica base: mesmo icone, Equip[14], classe
# e alcance. Nao importamos o sistema moderno de costume/timer de montaria.

$recordCount = 6500
$recordSize = 140
$xorKey = 0x5A
$effectStart = 0x50
$sourceID = 342
$supportedInputs = @(
    '9FDCCFAE6D074744CE8BCAEDDBCBF3DC90136EEC34F879EF545D50ABA45AB903'
)
$expectedOutputHash = '24534F712879BE18F8A3E89DC48FEA35783E047721EEA3FEF1CACFE746C24EA5'

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
function Assert-Record([byte[]]$Record, $Item) {
    if ((Record-Name $Record) -ne [string]$Item.name) { throw "item $($Item.item): nome divergente" }
    if ([BitConverter]::ToUInt16($Record, 0x40) -ne 7) { throw "item $($Item.item): icone nao e o da Shire" }
    if ([BitConverter]::ToUInt16($Record, 0x86) -ne 16384) { throw "item $($Item.item): slot nao e Equip[14]" }
    $expected = @(@(18,43), @(27,2))
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
$items = @($definition.items | Sort-Object item)
if ($items.Count -ne 47) { throw "manifesto inesperado: $($items.Count) montarias; esperado 47" }

$data = [IO.File]::ReadAllBytes($ItemList)
if ($data.Length -ne $recordCount * $recordSize + 4) { throw "ItemList.bin com tamanho inesperado: $($data.Length)" }
$alreadyInstalled = $true
foreach ($item in $items) {
    try { Assert-Record (Decode-Record $data $item.item) $item } catch { $alreadyInstalled = $false; break }
}
if ($alreadyInstalled) {
    $hash = Get-Sha $ItemList
    if ($expectedOutputHash -and $hash -ne $expectedOutputHash) { throw "ItemList montado divergiu do SHA esperado: $hash" }
    Write-Host "Todas as $($items.Count) montarias KR ja estao no ItemList.bin."
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
foreach ($item in $items) {
    $record = New-Object byte[] $recordSize
    [Array]::Copy($source, $record, $recordSize)
    [Array]::Clear($record, 0, 64)
    $name = [Text.Encoding]::ASCII.GetBytes([string]$item.name)
    if ($name.Length -ge 64) { throw "item $($item.item): nome excede 63 bytes" }
    [Array]::Copy($name, 0, $record, 0, $name.Length)
    Encode-Record $data $item.item $record
}
[IO.File]::WriteAllBytes($ItemList, $data)
$written = [IO.File]::ReadAllBytes($ItemList)
foreach ($item in $items) { Assert-Record (Decode-Record $written $item.item) $item }
$newHash = Get-Sha $ItemList
if ($expectedOutputHash -and $newHash -ne $expectedOutputHash) { throw "saida divergente: $newHash" }
Write-Host "$($items.Count) montarias KR instaladas no ItemList.bin 7.48."
Write-Host "SHA-256: $newHash"
