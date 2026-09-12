[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR.json')
)

$ErrorActionPreference = 'Stop'
$expectedHashes = @(
    '79B66BFF4E8D31D0788D857AD6AF3DE7F95DC7A07C7256D134A6DD5708EAA4AE', # KR_MOUNT_TEST_HASH
    'B3F385739C232275FE08FACAE0152ECDFD97D16D111C43D25E7277869FF5422B', # mobs KR sobre as montarias
    '8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593'  # orientacao moderna das faces KR
)
$sectionRaw = 0x001E5000
$tableOffset = 0x0400
$entrySize = 24

function Assert-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Expected,[string]$Name) {
    for($i=0;$i-lt$Expected.Length;$i++){if($Data[$Offset+$i]-ne$Expected[$i]){throw "$Name divergiu em 0x$('{0:X}'-f($Offset+$i))"}}
}
$currentHash=(Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToUpperInvariant()
if($expectedHashes -notcontains $currentHash){throw 'SHA do WYD.exe com montarias KR divergiu'}
$definition=Get-Content -LiteralPath $Manifest -Raw|ConvertFrom-Json
$catalogItems=@($definition.items|Sort-Object item)
$items=@($catalogItems|Where-Object { $_.available -ne $false })
$unavailableItems=@($catalogItems|Where-Object { $_.available -eq $false })
if($catalogItems.Count-ne 62-or$items.Count-ne 59-or$unavailableItems.Count-ne 3){throw "manifesto possui catalogo=$($catalogItems.Count) disponiveis=$($items.Count) indisponiveis=$($unavailableItems.Count)"}
if((@($unavailableItems|ForEach-Object{[int]$_.item}|Sort-Object)-join ',')-ne'4211,4235,6003'){throw 'casos indisponiveis divergiram de 4211/4235/6003'}
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
    6000 = @(30,1.0,0,0,0,0,0,0,0,0,0)
    6001 = @(31,1.0,0,0,4,1,1,0,0,0,0)
    6002 = @(31,1.0,0,0,5,1,1,0,0,0,0)
    6003 = @(49,0.6,25,25,0,0,0,0,13,13,13)
    6004 = @(25,1.0,3,3,0,0,0,0,12,12,12)
    6005 = @(31,1.0,50,50,0,0,0,0,0,0,0)
    6006 = @(49,0.6,9,9,0,0,0,0,12,12,12)
    6007 = @(49,0.7,0,0,0,10,0,0,0,0,0)
    6008 = @(38,1.0,4,4,0,0,0,0,8,8,8)
    6009 = @(29,1.0,0,0,0,0,0,0,0,0,0)
    6010 = @(38,1.0,1,1,0,0,0,0,8,8,8)
    6011 = @(38,1.0,2,2,0,0,0,0,8,8,8)
    6012 = @(49,0.8,7,7,0,0,0,0,13,13,13)
    6013 = @(49,0.8,8,8,0,0,0,0,13,13,13)
    6014 = @(29,1.25,10,10,0,0,0,0,13,13,13)
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
if($catalogTextures.Count-ne 61-or$textures.Count-ne 58-or$unavailableTextures.Count-ne 3){throw "manifesto possui catalogo=$($catalogTextures.Count) texturas, disponiveis=$($textures.Count), indisponiveis=$($unavailableTextures.Count)"}
$textureByName=@{};foreach($texture in $textures){$textureByName[[string]$texture.name]=[int]$texture.alpha}
$unavailableTextureNames=@($unavailableTextures|ForEach-Object{[string]$_.name}|Sort-Object)
if(($unavailableTextureNames-join ',')-ne'mesh\hs010117.wys,mesh\KK010112.wys,mesh\KK010126.wys'){throw 'texturas indisponiveis divergiram da fonte KR'}
if($textureByName['mesh\dr010149.wys']-ne[byte][char]'C'){throw 'material KR da montaria dr010149 divergiu'}
if($textureByName['mesh\KK010101.wys']-ne[byte][char]'A'){throw 'alpha da montaria KK010101 divergiu'}
foreach($required in @('mesh\tw010101.wys','mesh\hs010102.wys','mesh\hs010305.wys','mesh\hs010306.wys','mesh\bo010104.wys','mesh\hs010151.wys','mesh\tg010102.wys','mesh\tg010103.wys','mesh\tg010105.wys','mesh\be010101.wys','mesh\KK010108.wys','mesh\KK010109.wys','mesh\KK010110.wys','mesh\KK010111.wys','mesh\KK010118.wys','mesh\be010111.wys')){
    if(-not$textureByName.ContainsKey($required)){throw "textura runtime ausente do manifesto: $required"}
}
$data=[IO.File]::ReadAllBytes($Executable)
if($data.Length-ne 0x001F5000 -and $data.Length-ne 0x001F7000){throw ('tamanho final inesperado: 0x{0:X}'-f$data.Length)}
$pe=[BitConverter]::ToInt32($data,0x3C);$sections=[BitConverter]::ToUInt16($data,$pe+6);$optional=[BitConverter]::ToUInt16($data,$pe+20)
if($sections-ne 8 -and $sections-ne 9){throw "numero de secoes inesperado: $sections"}
$header=$pe+24+$optional+7*40
if([Text.Encoding]::ASCII.GetString($data,$header,8).Trim([char]0)-ne'.mountkr'){throw 'secao .mountkr ausente'}
Assert-Bytes $data 0x12538B ([byte[]](0xE9)) 'hook de montaria'
Assert-Bytes $data 0x1266DB ([byte[]](0xE9)) 'hook incremental de Equip[14]'
Assert-Bytes $data 0x1042C0 ([byte[]](0xE9)) 'hook de assento das montarias modernas'
Assert-Bytes $data 0x12799E ([byte[]](0x89,0x45,0xC8,0xEB,0x07)) 'culling nativo preservado nas montarias KR'
Assert-Bytes $data 0x5096F ([byte[]](0xE9)) 'hook da tecla R das montarias KR'
Assert-Bytes $data 0x46F8A ([byte[]](0xE9)) 'hook da UI de corrida das montarias KR'
Assert-Bytes $data 0xFA8D9 ([byte[]](0xE9)) 'hook da velocidade de corrida das montarias KR'
Assert-Bytes $data 0xC3607 ([byte[]](0xE9)) 'hook das matrizes CFrame das montarias KR'
Assert-Bytes $data 0xBE395 ([byte[]](0xE9)) 'hook da orientacao TMSkinMesh das montarias KR'
Assert-Bytes $data ($sectionRaw+0x0200) ([byte[]](0x60,0x8B,0x45,0x08,0x0F,0xB7,0x50,0x1C)) 'adapter incremental de Equip[14]'
# Marcadores do adapter de assento extraidos do WYD.exe KR: Loki/type31, type51 e type59.
$pose=[byte[]]$data[($sectionRaw+0x1000)..($sectionRaw+0x11FF)]
$poseHex=($pose|ForEach-Object{$_.ToString('X2')})-join''
foreach($signature in @('C745BC9A9919BF','C745BC5C8FC2BEC745C49A99993E','C745BCEC5138BEC745C4CDCC4CBE')){if(-not$poseHex.Contains($signature)){throw "adapter de assento nao contem assinatura nativa $signature"}}
Assert-Bytes $data ($sectionRaw+0x1800) ([byte[]](0x60,0x8B,0xF9,0x80,0xBF,0x9C,0x07,0x00,0x00,0x01)) 'adapter da tecla R'
Assert-Bytes $data ($sectionRaw+0x1A00) ([byte[]](0x60,0x8B,0xF9,0x80,0xBF,0x9C,0x07,0x00,0x00,0x01)) 'adapter da UI de corrida'
Assert-Bytes $data ($sectionRaw+0x1C00) ([byte[]](0x60,0x8B,0x7D,0xFC,0x80,0xBF,0x9C,0x07,0x00,0x00,0x01)) 'adapter da velocidade de corrida'
# CFrame::UpdateFrames: os tipos 48..51 e 59 usam ossos e matrizes proprios no
# executavel KR. Sem estes ramos, o rider fica suspenso/invertido embora os
# assets e a animacao estejam corretos.
Assert-Bytes $data ($sectionRaw+0x2000) ([byte[]](0x60,0x8B,0x9D,0x00,0xFE,0xFF,0xFF,0x8B,0x93,0x94,0x00,0x00,0x00,0x8B,0x02)) 'adapter das matrizes CFrame'
$frameMatrix=[byte[]]$data[($sectionRaw+0x2000)..($sectionRaw+0x2FFF)]
$frameMatrixHex=($frameMatrix|ForEach-Object{$_.ToString('X2')})-join''
foreach($signature in @(
    '83F830','83F831','83F832','83F833','83F83B',
    '8DB3580000008DBA74000000',
    '8DB3680000008DBA84000000',
    '8DB3580000008DBA94000000'
)){if(-not$frameMatrixHex.Contains($signature)){throw "adapter CFrame nao contem assinatura KR $signature"}}
# O type50 usa row2 no segundo eixo, conforme o executavel KR. A source W2PP
# publicada aponta row3; esse erro injeta translacao na orientacao e remove o
# cavaleiro do Wooden Horse. Há quatro usos corretos de row2 -> OutMatrix.row1:
# types 48, 49, 50 e 51.
$row2ToOutRow1='8DB3680000008DBA84000000'
if(([regex]::Matches($frameMatrixHex,$row2ToOutRow1)).Count-ne4){throw 'adapter CFrame nao preservou os quatro ramos row2 -> OutMatrix.row1'}
if($frameMatrixHex.Contains('8DB3780000008DBA84000000')){throw 'adapter CFrame ainda usa row3 incorretamente no assento type50'}
# O executavel KR inverte o terceiro eixo de todos os skeletons 48..51. A
# source W2PP publicada omite esse sinal; conferir apenas os offsets deixava o
# teste verde enquanto o cavaleiro permanecia suspenso/invertido.
foreach($signature in @('81B29400000000000080','81B29800000000000080','81B29C00000000000080','81B2A000000000000080')){
    $count=([regex]::Matches($frameMatrixHex,$signature)).Count
    if($count-ne4){throw "adapter CFrame deveria inverter o terceiro eixo 4 vezes; assinatura $signature apareceu $count"}
}
# type59: constantes exatas de RotationZ(3.351032f): cos, sin e -sin.
Assert-Bytes $data ($sectionRaw+0x120C) ([byte[]](0xE1,0x67,0x7A,0xBF,0xD4,0xE6,0x54,0xBE,0xD4,0xE6,0x54,0x3E)) 'constantes CFrame type59'
# TMSkinMesh::Render: na cadeia final os skeletons 45..57 usam yaw+90/pitch
# original; os demais continuam no ramo legado yaw-90/pitch-90. O hash de
# montaria intermediario ainda pode ser validado para diagnostico.
$orientationPrefixLegacy=[byte[]](0x8B,0x85,0xEC,0xFD,0xFF,0xFF,0x8B,0x08,0x83,0xF9,0x30)
$orientationPrefixModern=[byte[]](0x8B,0x85,0xEC,0xFD,0xFF,0xFF,0x8B,0x08,0x83,0xF9,0x2D)
if($currentHash -eq '8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593'){
    Assert-Bytes $data ($sectionRaw+0x3000) $orientationPrefixModern 'seletor moderno de orientacao TMSkinMesh'
}else{
    Assert-Bytes $data ($sectionRaw+0x3000) $orientationPrefixLegacy 'seletor de orientacao TMSkinMesh'
}
$orientation=[byte[]]$data[($sectionRaw+0x3000)..($sectionRaw+0x30FF)]
$orientationHex=($orientation|ForEach-Object{$_.ToString('X2')})-join''
if($currentHash -eq '8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593'){
    foreach($signature in @(
        '83F92D0F8C','8B90D002000083FA07','83FA090F8E','83F9390F8F','C740240000C03FC740280000C03FC7402C0000C03F',
        '8B5020528B501852D9401CD80580435A00',
        'D94018D82580435A00','D9401CD82580435A00'
    )){if(-not$orientationHex.Contains($signature)){throw "adapter moderno de orientacao nao contem assinatura $signature"}}
}else{
    foreach($signature in @(
        '83F9300F8C','83F9330F8F',
        '8B5020528B501852D9401CD80580435A00',
        'D94018D82580435A00','D9401CD82580435A00',
        'E956930EFF'
    )){if(-not$orientationHex.Contains($signature)){throw "adapter de orientacao nao contem assinatura $signature"}}
}
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
Write-Host 'OK: 59 montarias KR completas, 3 cases indisponiveis sem fallback, refresh imediato, renderer, ItemList, skeletons, animacoes e assets conferidos.'
