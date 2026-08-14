[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR-Native.csv')
)

$ErrorActionPreference='Stop'
$expectedExe='91EAB0CBDAC8E8957A2138B0D6060587D5F701F1178EF5CE40105A028E849209'
$sectionRaw=0x001E5000
$tableOffset=0x0400
$entrySize=24

function Get-Sha256([string]$Path){return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()}
function Get-GitBlobSha([string]$Path){
    $bytes=[IO.File]::ReadAllBytes($Path)
    $prefix=[Text.Encoding]::ASCII.GetBytes("blob $($bytes.Length)`0")
    $all=New-Object byte[] ($prefix.Length+$bytes.Length)
    [Array]::Copy($prefix,0,$all,0,$prefix.Length)
    [Array]::Copy($bytes,0,$all,$prefix.Length,$bytes.Length)
    $sha=[Security.Cryptography.SHA1]::Create()
    try{return ([BitConverter]::ToString($sha.ComputeHash($all))).Replace('-','').ToLowerInvariant()}
    finally{$sha.Dispose()}
}
function Assert-Eq($Actual,$Expected,[string]$Name){if($Actual-ne$Expected){throw "$Name`: esperado=$Expected encontrado=$Actual"}}
function Read-U16([byte[]]$d,[int]$o){return [BitConverter]::ToUInt16($d,$o)}

if(-not(Test-Path -LiteralPath $Executable -PathType Leaf)){throw "WYD.exe ausente: $Executable"}
if(-not(Test-Path -LiteralPath $Manifest -PathType Leaf)){throw "manifesto standalone ausente: $Manifest"}
Assert-Eq (Get-Sha256 $Executable) $expectedExe 'SHA do WYD.exe standalone'

$items=@(Import-Csv -LiteralPath $Manifest|Sort-Object {[int]$_.item})
Assert-Eq $items.Count 47 'quantidade de montarias'

$cases=@{
    4227=@{type=31;scale_bits='3F800000';mesh=@(17,17,0);skin=@(0,0,0);sanc=@(15,15,0)}
    4230=@{type=29;scale_bits='3FA00000';mesh=@(10,10,0);skin=@(0,0,0);sanc=@(13,13,0)}
    4232=@{type=49;scale_bits='3F333333';mesh=@(0,0,0);skin=@(10,0,0);sanc=@(13,0,0)}
    4233=@{type=31;scale_bits='3F800000';mesh=@(50,50,0);skin=@(0,0,0);sanc=@(0,0,0)}
    4234=@{type=25;scale_bits='3F800000';mesh=@(3,3,0);skin=@(0,0,0);sanc=@(12,12,0)}
    4241=@{type=49;scale_bits='3F333333';mesh=@(17,0,0);skin=@(0,0,0);sanc=@(12,0,0)}
}
foreach($id in $cases.Keys){
    $it=@($items|Where-Object { [int]$_.item -eq [int]$id })
    Assert-Eq $it.Count 1 "manifesto item $id"
    $want=$cases[$id]
    Assert-Eq ([int]$it[0].type) ([int]$want.type) "item $id type"
    Assert-Eq ([string]$it[0].scale_bits) ([string]$want.scale_bits) "item $id scale_bits"
    for($p=0;$p-lt 3;$p++){
        $meshField="mesh$p";$skinField="skin$p";$sancField="sanc$p"
        Assert-Eq ([int]$it[0].$meshField) ([int]$want.mesh[$p]) "item $id mesh[$p]"
        Assert-Eq ([int]$it[0].$skinField) ([int]$want.skin[$p]) "item $id skin[$p]"
        Assert-Eq ([int]$it[0].$sancField) ([int]$want.sanc[$p]) "item $id sanc[$p]"
    }
}

$data=[IO.File]::ReadAllBytes($Executable)
for($i=0;$i-lt$items.Count;$i++){
    $it=$items[$i];$off=$sectionRaw+$tableOffset+$i*$entrySize
    Assert-Eq (Read-U16 $data $off) ([int]$it.item) "table item $($it.item)"
    Assert-Eq (Read-U16 $data ($off+2)) ([int]$it.type) "table type $($it.item)"
    Assert-Eq ([BitConverter]::ToUInt32($data,$off+4)) ([Convert]::ToUInt32([string]$it.scale_bits,16)) "table scale_bits $($it.item)"
    for($p=0;$p-lt 3;$p++){
        $meshField="mesh$p";$skinField="skin$p";$sancField="sanc$p"
        Assert-Eq (Read-U16 $data ($off+8+$p*4)) ([int]$it.$meshField) "table mesh $($it.item)/$p"
        Assert-Eq (Read-U16 $data ($off+10+$p*4)) ([int]$it.$skinField) "table skin $($it.item)/$p"
        $wantSanc=if(([int]$it.$sancField) -eq 65535){0}else{[int]$it.$sancField}
        Assert-Eq ([int]$data[$off+20+$p]) $wantSanc "table sanc $($it.item)/$p"
    }
}

# Hook moderno deve sair de 0x51E3A4 para .mountkr+0x2000.
Assert-Eq ([int]$data[0x11E3A4]) 0xE9 'opcode do child animation hook'
$rel=[BitConverter]::ToInt32($data,0x11E3A5)
$target=[int64]0x0051E3A9+$rel
Assert-Eq $target ([int64]0x013D4000) 'destino do child animation hook'

# Protecao explicita dos trajes: os quatro artefatos permanecem exatamente nos
# blobs do main que precedeu esta correcao.
$costumeBlobs=@{
    'Costumes-KR.json'='b4db10ed8b9bdc1d7d3baeecf54e5c79445f8462'
    'Patch-WYD748-Costumes.ps1'='ffe21e9590d2199a7d20395b399ac07020547f04'
    'Patch-WYD748-CostumeItems.ps1'='b551a0feb2db7994f448fee6263a527f984c3e92'
    'Patch-WYD748-CostumeTextures.ps1'='17be87c3138fef5a6c624c01cf8b9bd7d075647c'
}
foreach($name in $costumeBlobs.Keys){
    $path=Join-Path $PSScriptRoot $name
    if(-not(Test-Path -LiteralPath $path -PathType Leaf)){throw "arquivo de traje ausente: $name"}
    Assert-Eq (Get-GitBlobSha $path) $costumeBlobs[$name] "regressao de traje $name"
}

$assetScript=Join-Path $PSScriptRoot 'Patch-WYD748-KRMountAssets.ps1'
$assetText=[IO.File]::ReadAllText($assetScript)
if($assetText.Contains('$compatibleAssetFallbacks')){throw 'fallback antigo ainda existe no patch de assets'}
if($assetText.Contains("'mesh\KK010112.msh' = 'mesh\KK010113.msh'")){throw 'substituicao KK010112->KK010113 ainda existe'}
if($assetText.Contains("'mesh\hs010117.wys' = 'mesh\hs010115.wys'")){throw 'substituicao hs010117->hs010115 ainda existe'}
if($assetText -notmatch 'for\(\$field=0;\$field-lt 186;\$field\+\+\)'){throw 'ValidIndex nao esta traduzido campo a campo'}

Write-Host 'Montarias KR standalone: regressao estatica aprovada.'
Write-Host 'Trajes KR: manifests/scripts permanecem byte-identical ao main de referencia.'
