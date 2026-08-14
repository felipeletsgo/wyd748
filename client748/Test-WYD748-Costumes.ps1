[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Costumes-KR.json')
)

$ErrorActionPreference = 'Stop'
$supportedHashes = @(
    '4A2AA37228A720ED389F5AC8A5978329855932B93E54FA0501B51A3A23316DEF',
    '0648B586AF95D26FB0B0C27ED0F954FE5F8D291E4D3DD10B73BB816B3D5B1A75',
    '556EC07005D17DCEDEF0CE15B8C8FDB13AE1E82975D992778ACDA846C108CD8F',
    'D1CC8C8CCE860968D6C8E9839B8D2B95276B11FA2CDE1B0CED5BD08EE2DF01DF',
    '2ABF628994A6FD61687CB33425672A37443A4D1F56E4E473901A1E64C4C6CB98',
    '2E7315FAE3F549899A295CB40950391A05B1B9A5D967CA2CBBBB93EB1812C3E3',
    '7536512E0F298CD182A44307E7D9C7D34A4E60D93B4C2650DAD0D59A4A8A1646',
    '3D3270952B10D3BFDDD4B07A3F19D0FD2E211EE4733C9F0369F377B30A2F0B15',
    '3AC3296B82D19F19A8CF5D621C3914C40A220994294BC5E7EBCF300796F16302',
    '63935115018E39179C254C70AE4062B2449E523CCED852B2E8A4B3ACBBD70CB8',
    '79B66BFF4E8D31D0788D857AD6AF3DE7F95DC7A07C7256D134A6DD5708EAA4AE',
    'B3F385739C232275FE08FACAE0152ECDFD97D16D111C43D25E7277869FF5422B'
)
$sectionRaw = 0x001D5000
$sectionVA = [uint32]0x013C2000
$selectorOffset = 0x0000
$rendererOffset = 0x0100
$admissionOffset = 0x0680
$itemTableOffset = 0x0200
$rendererTableOffset = 0x0800
$selectiveCullOffset = 0x4100
$partCount = 6

function Assert-Equal($Actual, $Expected, [string]$Name) {
    if ($Actual -ne $Expected) { throw "$Name divergente: esperado '$Expected', encontrado '$Actual'" }
}

function Read-CString([byte[]]$Data, [uint32]$Pointer) {
    if ($Pointer -eq 0) { return $null }
    $offset = [int]($sectionRaw + ($Pointer - $sectionVA))
    if ($offset -lt $sectionRaw -or $offset -ge $Data.Length) { throw ('ponteiro fora de .costkr: 0x{0:X8}' -f $Pointer) }
    $end = $offset
    while ($end -lt $Data.Length -and $Data[$end] -ne 0) { $end++ }
    if ($end -eq $Data.Length) { throw ('string sem terminador em 0x{0:X8}' -f $Pointer) }
    return [Text.Encoding]::ASCII.GetString($Data, $offset, $end - $offset)
}

if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) { throw "WYD.exe ausente: $Executable" }
if ($supportedHashes -notcontains (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToUpperInvariant()) {
    throw 'WYD.exe nao possui o build reproduzivel da colecao KR'
}
$definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
$items = @($definition.items | Where-Object available | Sort-Object item)
$renderers = @($definition.renderers | Sort-Object type)
Assert-Equal $items.Count 135 'quantidade de trajes completos'
Assert-Equal @($items | Where-Object { [int]$_.item -ge 4201 }).Count 103 'trajes fora da faixa nativa admitidos pelo hook'
Assert-Equal $renderers.Count 129 'quantidade de renderers completos'
$textureByName = @{}
foreach ($texture in @($definition.textures)) { $textureByName[[string]$texture.name] = [int]$texture.alpha }
Assert-Equal $textureByName['mesh\WhitePolice.wys'] ([byte][char]'A') 'alpha WhitePolice'
Assert-Equal $textureByName['mesh\ch0201108.wys'] ([byte][char]'C') 'material KR ChristmasCostume'
Assert-Equal $textureByName['mesh\ch0201109.wys'] ([byte][char]'C') 'material KR 2020CostumeF'
Assert-Equal $textureByName['mesh\ch0202104.wys'] ([byte][char]'C') 'material KR hallow_costumes_FM'

# Esses renderers chegaram inicialmente com as seis partes deslocadas e foram
# mantidos com a primeira entrada vazia no manifesto. O adapter deve usar o
# caminho nativo somente para essa entrada vazia, sem deslocar as partes 1..5.
foreach ($type in @(131,132,133,134,135,136,137,138,271,272,273,274)) {
    $renderer = @($renderers | Where-Object { [int]$_.type -eq $type })
    Assert-Equal $renderer.Count 1 "renderer corrigido $type"
    Assert-Equal ([string]$renderer[0].parts[0].texture) '' "renderer $type parte 0 textura"
    Assert-Equal ([string]$renderer[0].parts[0].mesh) '' "renderer $type parte 0 mesh"
    for ($part = 1; $part -lt $partCount; $part++) {
        if (-not [string]$renderer[0].parts[$part].texture -or -not [string]$renderer[0].parts[$part].mesh) {
            throw "renderer $type parte $part ainda esta deslocado/incompleto"
        }
    }
}

# Student_Set e Dancing_Girl mantem a primeira entrada vazia no manifesto. O
# fallback seletivo preserva a parte nativa somente nesses casos comprovados.
foreach ($type in @(25,33)) {
    $renderer = @($renderers | Where-Object { [int]$_.type -eq $type })
    Assert-Equal $renderer.Count 1 "renderer sem cabeca $type"
    Assert-Equal ([string]$renderer[0].parts[0].texture) '' "renderer $type preserva textura da face"
    Assert-Equal ([string]$renderer[0].parts[0].mesh) '' "renderer $type preserva mesh da face"
    for ($part = 1; $part -lt $partCount; $part++) {
        if (-not [string]$renderer[0].parts[$part].texture -or -not [string]$renderer[0].parts[$part].mesh) {
            throw "renderer $type parte $part incompleto"
        }
    }
}

# No fluxo nativo RestoreDeviceObjects, i=0 carrega ch0101<tipo>.msh. Portanto
# a parte 0 dos trajes modernos abaixo e geometria real e nunca pode ser
# substituida incondicionalmente pela armadura nativa.
foreach ($type in @(200,219,220,248,249,250,251,253,254)) {
    $renderer = @($renderers | Where-Object { [int]$_.type -eq $type })
    Assert-Equal $renderer.Count 1 "renderer com geometria part0 $type"
    if (-not [string]$renderer[0].parts[0].texture -or -not [string]$renderer[0].parts[0].mesh) {
        throw "renderer $type perdeu a parte 0 real"
    }
    $part0Path = Join-Path $PSScriptRoot ([string]$renderer[0].parts[0].mesh)
    if (-not (Test-Path -LiteralPath $part0Path -PathType Leaf)) {
        throw "renderer $type sem asset da parte 0: $part0Path"
    }
    if ((Get-Item -LiteralPath $part0Path).Length -le 552) {
        throw "renderer $type parte 0 parece placeholder: $part0Path"
    }
}

$data = [IO.File]::ReadAllBytes($Executable)
$expectedLength = if ($data.Length -eq 0x001F7000) { 0x001F7000 } elseif ($data.Length -eq 0x001F5000) { 0x001F5000 } else { 0x001E5000 }
Assert-Equal $data.Length $expectedLength 'tamanho do executavel'
$pe = [BitConverter]::ToInt32($data, 0x3C)
$sectionCount = [BitConverter]::ToUInt16($data, $pe + 6)
if ($sectionCount -notin @(7, 8, 9)) { throw "numero de secoes PE divergente: $sectionCount" }
$optionalSize = [BitConverter]::ToUInt16($data, $pe + 20)
$sectionHeader = $pe + 24 + $optionalSize + 6 * 40
Assert-Equal ([Text.Encoding]::ASCII.GetString($data, $sectionHeader, 8).Trim([char]0)) '.costkr' 'nome da secao'
Assert-Equal ([BitConverter]::ToUInt32($data, $sectionHeader + 12)) 0x00FC2000 'RVA da secao'
Assert-Equal ([BitConverter]::ToUInt32($data, $sectionHeader + 20)) $sectionRaw 'raw da secao'

# O hook de admissao precisa existir antes do gate nativo 4151..4200. Sem ele,
# os trajes modernos continuam equipaveis, mas nunca chegam ao seletor visual.
Assert-Equal $data[0xFB228] 0xE9 'opcode do hook de admissao'
$admissionRel = [BitConverter]::ToInt32($data, 0xFB229)
$admissionTarget = [uint32](0x004FB22D + $admissionRel)
Assert-Equal $admissionTarget ([uint32]($sectionVA + $admissionOffset)) 'destino do hook de admissao'
Assert-Equal $data[0xFB22D] 0x90 'padding do hook de admissao'

# O culling two-sided e estritamente opt-in pela tabela de renderers de trajes
# KR. As montarias preservam o culling nativo do renderer 7.48.
Assert-Equal $data[0xC51F7] 0xE9 'opcode do hook seletivo de culling'
$cullRel = [BitConverter]::ToInt32($data, 0xC51F8)
$cullTarget = [uint32](0x004C51FC + $cullRel)
Assert-Equal $cullTarget ([uint32]($sectionVA + $selectiveCullOffset)) 'destino do hook seletivo de culling'
Assert-Equal $data[$sectionRaw+$selectiveCullOffset] 0x60 'inicio preservando registradores do culling'

# O bit 0x4000 distingue a instancia importada de skins nativas que reutilizam
# o mesmo numero de tipo. Somente depois dessa prova part0 pode usar a tabela.
$rendererPrefix = [byte[]](
    0x60,0x8B,0x85,0x30,0xFF,0xFF,0xFF,
    0x0F,0xB7,0x80,0x56,0x03,0x00,0x00,
    0xA9,0x00,0x40,0x00,0x00,0x0F,0x84)
for ($i = 0; $i -lt $rendererPrefix.Length; $i++) {
    Assert-Equal $data[$sectionRaw+0x0100+$i] $rendererPrefix[$i] "renderer marcado byte $i"
}
$selectorMarker = [byte[]](0x66,0x81,0xCF,0x00,0x40,0x66,0x89,0x7D,0xEC)
$selectorBytesForMarker = $data[($sectionRaw+$selectorOffset)..($sectionRaw+$rendererOffset-1)]
$markerFound = $false
for ($i = 0; $i -le $selectorBytesForMarker.Length-$selectorMarker.Length; $i++) {
    $matches = $true
    for ($j = 0; $j -lt $selectorMarker.Length; $j++) {
        if ($selectorBytesForMarker[$i+$j] -ne $selectorMarker[$j]) { $matches = $false; break }
    }
    if ($matches) { $markerFound = $true; break }
}
if (-not $markerFound) { throw 'seletor nao marca exclusivamente os trajes KR' }

# O teste confirma que o modo comprovado no registro KR @510 foi preservado no
# registro legado.
$textureList = [IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'mesh\MeshTextureList.bin'))
$textureRecordSize = 264
foreach ($name in @('mesh\ch0201108.wys','mesh\ch0201109.wys','mesh\ch0202104.wys')) {
    $found = $false
    for ($row = 0; $row -lt 2048; $row++) {
        $offset = $row * $textureRecordSize
        $length = 0
        while ($length -lt 255 -and $textureList[$offset+$length] -notin @(0,0xCD)) { $length++ }
        if ([Text.Encoding]::ASCII.GetString($textureList,$offset,$length) -ieq $name) {
            Assert-Equal $textureList[$offset+255] ([byte][char]'C') "material legado $name"
            $found = $true
            break
        }
    }
    if (-not $found) { throw "textura nao registrada: $name" }
}
# Tanto CreateMob quanto UpdateEquip carregam somente os 12 bits baixos do
# item visual. Os dois hooks devem reconstruir o namespace 4xxx antes da busca.
$normalizeItem = [byte[]](
    0x0F,0xB7,0x90,0xAE,0x07,0x00,0x00,
    0x81,0xE2,0xFF,0x0F,0x00,0x00,
    0x81,0xC2,0x00,0x10,0x00,0x00
)
for ($i = 0; $i -lt $normalizeItem.Length; $i++) {
    Assert-Equal $data[$sectionRaw+$selectorOffset+19+$i] $normalizeItem[$i] "normalizacao do seletor byte $i"
    Assert-Equal $data[$sectionRaw+$admissionOffset+7+$i] $normalizeItem[$i] "normalizacao da admissao byte $i"
}

# O seletor pode consultar m_nSkinMeshType para decidir entre os dois únicos
# renderers duplos, mas nunca pode reescrevê-lo. Esse campo não é um booleano:
# a escrita `mov [eax+5C], ecx` desmontava o corpo do personagem no mundo.
$selectorBytes = $data[($sectionRaw+$selectorOffset)..($sectionRaw+$itemTableOffset-1)]
$forbiddenSkinWrite = [byte[]](0x89,0x48,0x5C)
for ($i = 0; $i -le $selectorBytes.Length-$forbiddenSkinWrite.Length; $i++) {
    $matches = $true
    for ($j = 0; $j -lt $forbiddenSkinWrite.Length; $j++) {
        if ($selectorBytes[$i+$j] -ne $forbiddenSkinWrite[$j]) { $matches = $false; break }
    }
    if ($matches) { throw 'seletor de trajes ainda reescreve TMHuman.m_nSkinMeshType' }
}

for ($i = 0; $i -lt $items.Count; $i++) {
    $item = $items[$i]
    $offset = $sectionRaw + $itemTableOffset + $i * 8
    Assert-Equal ([BitConverter]::ToUInt16($data, $offset)) ([int]$item.item) "itemTable[$i].item"
    Assert-Equal ([BitConverter]::ToUInt16($data, $offset + 2)) ([int]$item.maleType) "itemTable[$i].maleType"
    Assert-Equal ([BitConverter]::ToUInt16($data, $offset + 4)) ([int]$item.femaleType) "itemTable[$i].femaleType"
    $skin = if ([int]$item.skin -lt 0) { 255 } else { [int]$item.skin }
    Assert-Equal $data[$offset + 6] $skin "itemTable[$i].skin"
}

foreach ($renderer in $renderers) {
    for ($part = 0; $part -lt $partCount; $part++) {
        $offset = $sectionRaw + $rendererTableOffset + (([int]$renderer.type * $partCount + $part) * 8)
        $texture = Read-CString $data ([BitConverter]::ToUInt32($data, $offset))
        $mesh = Read-CString $data ([BitConverter]::ToUInt32($data, $offset + 4))
        Assert-Equal $texture ([string]$renderer.parts[$part].texture) "renderer $($renderer.type) parte $part textura"
        $expectedMesh = [string]$renderer.parts[$part].mesh
        if (-not $expectedMesh) { $expectedMesh = $null }
        Assert-Equal $mesh $expectedMesh "renderer $($renderer.type) parte $part mesh"
        foreach ($asset in @($texture, $mesh)) {
            if (-not $asset) { continue }
            $path = Join-Path $PSScriptRoot $asset
            if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "asset ausente: $asset" }
        }
    }
}

& (Join-Path $PSScriptRoot 'Patch-WYD748-CostumeItems.ps1') -VerifyOnly
& (Join-Path $PSScriptRoot 'Patch-WYD748-CostumeTextures.ps1') -VerifyOnly
Write-Host "Colecao KR validada: 135 trajes, 129 renderers e $($definition.textures.Count) texturas de traje."
