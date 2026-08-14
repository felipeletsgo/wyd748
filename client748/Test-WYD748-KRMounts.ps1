[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR.json')
)

$ErrorActionPreference = 'Stop'
$expectedHash = '556EC07005D17DCEDEF0CE15B8C8FDB13AE1E82975D992778ACDA846C108CD8F' # KR_MOUNT_TEST_HASH
$sectionRaw = 0x001E5000
$tableOffset = 0x0400
$entrySize = 24

function Assert-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Expected,[string]$Name) {
    for($i=0;$i-lt$Expected.Length;$i++){if($Data[$Offset+$i]-ne$Expected[$i]){throw "$Name divergiu em 0x$('{0:X}'-f($Offset+$i))"}}
}
if((Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToUpperInvariant()-ne$expectedHash){throw 'SHA do WYD.exe com montarias KR divergiu'}
$definition=Get-Content -LiteralPath $Manifest -Raw|ConvertFrom-Json
$catalogItems=@($definition.items|Sort-Object item)
$items=@($catalogItems|Where-Object { $_.available -ne $false })
$unavailableItems=@($catalogItems|Where-Object { $_.available -eq $false })
if($catalogItems.Count-ne 47-or$items.Count-ne 45-or$unavailableItems.Count-ne 2){throw "manifesto possui catalogo=$($catalogItems.Count) disponiveis=$($items.Count) indisponiveis=$($unavailableItems.Count)"}
if((@($unavailableItems|ForEach-Object{[int]$_.item}|Sort-Object)-join ',')-ne'4211,4235'){throw 'casos indisponiveis divergiram de 4211/4235'}
$nativeCases = @{
    4211 = @(31,1.0,14,14,0,2,0,0,10,10,10)
    4214 = @(40,1.0,46,46,46,3,3,0,6,6,6)
    4223 = @(59,1.0,1,1,1,0,0,0,7,7,7)
    4228 = @(49,0.8,7,7,0,0,0,0,13,13,13)
    4229 = @(49,0.8,8,8,0,0,0,0,13,13,13)
    4230 = @(29,1.25,10,10,0,0,0,0,13,13,13)
    4231 = @(49,0.6,9,9,0,0,0,0,12,12,12)
    4232 = @(49,0.7,0,0,0,10,0,0,0,0,0)
    4233 = @(49,0.7,0,0,0,10,0,0,0,0,0)
    4234 = @(49,0.7,0,0,0,10,0,0,0,0,0)
    4235 = @(49,0.6,11,11,0,0,0,0,9,9,9)
    4241 = @(49,0.7,17,17,0,0,0,0,12,12,12)
}
$itemByID=@{};foreach($item in $catalogItems){$itemByID[[int]$item.item]=$item}
foreach($id in $nativeCases.Keys){
    $expected=$nativeCases[$id];$item=$itemByID[[int]$id]
    if(-not$item){throw "caso nativo $id ausente"}
    $actual=@([int]$item.type,[double]$item.scale)+@($item.mesh|ForEach-Object{[int]$_})+@($item.skin|ForEach-Object{[int]$_})+@($item.sanc|ForEach-Object{[int]$_})
    if(($actual-join ',')-ne($expected-join ',')){throw "assinatura nativa do item $id divergiu"}
}
$catalogTextures=@($definition.textures)
$textures=@($catalogTextures|Where-Object { $_.available -ne $false })
$unavailableTextures=@($catalogTextures|Where-Object { $_.available -eq $false })
if($catalogTextures.Count-ne 50-or$textures.Count-ne 48-or$unavailableTextures.Count-ne 2){throw "manifesto possui catalogo=$($catalogTextures.Count) texturas, disponiveis=$($textures.Count), indisponiveis=$($unavailableTextures.Count)"}
$textureByName=@{};foreach($texture in $textures){$textureByName[[string]$texture.name]=[int]$texture.alpha}
$unavailableTextureNames=@($unavailableTextures|ForEach-Object{[string]$_.name}|Sort-Object)
if(($unavailableTextureNames-join ',')-ne'mesh\hs010117.wys,mesh\KK010112.wys'){throw 'texturas indisponiveis divergiram da fonte KR'}
if($textureByName['mesh\dr010149.wys']-ne[byte][char]'C'){throw 'material KR da montaria dr010149 divergiu'}
if($textureByName['mesh\KK010101.wys']-ne[byte][char]'A'){throw 'alpha da montaria KK010101 divergiu'}
foreach($required in @('mesh\KK010108.wys','mesh\KK010109.wys','mesh\KK010110.wys','mesh\KK010111.wys','mesh\KK010118.wys','mesh\be010111.wys')){
    if(-not$textureByName.ContainsKey($required)){throw "textura runtime ausente do manifesto: $required"}
}
$data=[IO.File]::ReadAllBytes($Executable)
if($data.Length-ne 0x001F5000){throw ('tamanho final inesperado: 0x{0:X}'-f$data.Length)}
$pe=[BitConverter]::ToInt32($data,0x3C);$sections=[BitConverter]::ToUInt16($data,$pe+6);$optional=[BitConverter]::ToUInt16($data,$pe+20)
if($sections-ne 8){throw "numero de secoes inesperado: $sections"}
$header=$pe+24+$optional+7*40
if([Text.Encoding]::ASCII.GetString($data,$header,8).Trim([char]0)-ne'.mountkr'){throw 'secao .mountkr ausente'}
Assert-Bytes $data 0x12538B ([byte[]](0xE9)) 'hook de montaria'
Assert-Bytes $data 0x1266DB ([byte[]](0xE9)) 'hook incremental de Equip[14]'
Assert-Bytes $data 0x1042C0 ([byte[]](0xE9)) 'hook de assento das montarias modernas'
Assert-Bytes $data 0x12799E ([byte[]](0xE9)) 'hook de culling seletivo das montarias KR'
Assert-Bytes $data 0x5096F ([byte[]](0xE9)) 'hook da tecla R das montarias KR'
Assert-Bytes $data 0x46F8A ([byte[]](0xE9)) 'hook da UI de corrida das montarias KR'
Assert-Bytes $data 0xFA8D9 ([byte[]](0xE9)) 'hook da velocidade de corrida das montarias KR'
Assert-Bytes $data ($sectionRaw+0x0200) ([byte[]](0x60,0x8B,0x45,0x08,0x0F,0xB7,0x50,0x1C)) 'adapter incremental de Equip[14]'
Assert-Bytes $data ($sectionRaw+0x1400) ([byte[]](0x89,0x45,0xC8,0x60,0x85,0xC0)) 'marcador pos-construcao das montarias KR'
# Marcadores do adapter de assento extraidos do WYD.exe KR: Loki/type31, type51 e type59.
$pose=[byte[]]$data[($sectionRaw+0x1000)..($sectionRaw+0x11FF)]
$poseHex=($pose|ForEach-Object{$_.ToString('X2')})-join''
foreach($signature in @('C745BC9A9919BF','C745BC5C8FC2BEC745C49A99993E','C745BCEC5138BEC745C4CDCC4CBE')){if(-not$poseHex.Contains($signature)){throw "adapter de assento nao contem assinatura nativa $signature"}}
Assert-Bytes $data ($sectionRaw+0x1800) ([byte[]](0x60,0x8B,0xF9,0x80,0xBF,0x9C,0x07,0x00,0x00,0x01)) 'adapter da tecla R'
Assert-Bytes $data ($sectionRaw+0x1A00) ([byte[]](0x60,0x8B,0xF9,0x80,0xBF,0x9C,0x07,0x00,0x00,0x01)) 'adapter da UI de corrida'
Assert-Bytes $data ($sectionRaw+0x1C00) ([byte[]](0x60,0x8B,0x7D,0xFC,0x80,0xBF,0x9C,0x07,0x00,0x00,0x01)) 'adapter da velocidade de corrida'
for($i=0;$i-lt$items.Count;$i++){
    $item=$items[$i];$off=$sectionRaw+$tableOffset+$i*$entrySize
    if([BitConverter]::ToUInt16($data,$off)-ne[int]$item.item-or[BitConverter]::ToUInt16($data,$off+2)-ne[int]$item.type){throw "entrada visual $i divergiu"}
    $scale=[BitConverter]::ToSingle($data,$off+4)
    if([Math]::Abs($scale-[single]$item.scale)-gt 0.0001){throw "escala do item $($item.item) divergiu"}
    for($part=0;$part-lt 3;$part++){
        if([BitConverter]::ToUInt16($data,$off+8+$part*4)-ne[int]$item.mesh[$part]){throw "mesh $part do item $($item.item) divergiu"}
        if([BitConverter]::ToUInt16($data,$off+10+$part*4)-ne[int]$item.skin[$part]){throw "skin $part do item $($item.item) divergiu"}
        if($data[$off+20+$part]-ne[int]$item.sanc[$part]){throw "sanc $part do item $($item.item) divergiu"}
    }
    if($data[$off+23]-ne[int]$item.special){throw "flag especial do item $($item.item) divergiu"}
}
& (Join-Path $PSScriptRoot 'Patch-WYD748-KRMountItems.ps1') -ItemList (Join-Path $PSScriptRoot 'ItemList.bin') -VerifyOnly
& (Join-Path $PSScriptRoot 'Patch-WYD748-KRMountAssets.ps1') -ClientRoot $PSScriptRoot -VerifyOnly
& (Join-Path $PSScriptRoot 'Patch-WYD748-CostumeTextures.ps1') -VerifyOnly
Write-Host 'OK: 45 montarias KR completas, 2 cases indisponiveis sem fallback, refresh imediato, renderer, ItemList, skeletons, animacoes e assets conferidos.'
