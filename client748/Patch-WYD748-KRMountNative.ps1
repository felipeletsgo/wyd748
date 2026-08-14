[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR-Native.csv'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Corrige somente o pipeline de montarias KR. O elo anterior materializava
# Equip[14] com tuples de SetMountCostume (overlay). Este elo substitui a tabela
# .mountkr pelos registros standalone de MountDataV do client KR e porta dois
# comportamentos do renderer que o 7.48 nao possui: offsets modernos de assento
# e o estado de animacao do TMSkinMesh filho. A secao .costkr nao e alterada.

$expectedInputHash  = 'F6F99CC0405654629D9867C84F6587B2064B30D58F67A2151E1ACD36F394E72D'
$expectedOutputHash = '91EAB0CBDAC8E8957A2138B0D6060587D5F701F1178EF5CE40105A028E849209'

$sectionRaw = 0x001E5000
$sectionVA  = [uint32]0x013D2000
$tableOffset = 0x0400
$poseCodeOffset = 0x1000
$poseConstantsOffset = 0x1200
$childAnimCodeOffset = 0x2000
$entrySize = 24

$costumeRaw = 0x001D5000
$costumeSize = 0x00010000

function Get-Sha([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}
function Get-RangeSha([byte[]]$Data,[int]$Offset,[int]$Length) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($sha.ComputeHash($Data,$Offset,$Length))).Replace('-','') }
    finally { $sha.Dispose() }
}
function Assert-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Expected,[string]$Name) {
    for($i=0;$i -lt $Expected.Length;$i++) {
        if($Data[$Offset+$i] -ne $Expected[$i]) {
            throw ('{0}: byte inesperado em 0x{1:X}' -f $Name,($Offset+$i))
        }
    }
}
function Set-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Value) {
    [Array]::Copy($Value,0,$Data,$Offset,$Value.Length)
}
function Set-U16([byte[]]$Data,[int]$Offset,[uint16]$Value) {
    Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value))
}
function Set-U32([byte[]]$Data,[int]$Offset,[uint32]$Value) {
    Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value))
}
function Set-F32([byte[]]$Data,[int]$Offset,[single]$Value) {
    Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value))
}
function Rel32([uint32]$FromNextInstruction,[uint32]$Target) {
    return ,([BitConverter]::GetBytes([int32]([int64]$Target-[int64]$FromNextInstruction)))
}
function New-Assembler([uint32]$BaseVA) {
    return [pscustomobject]@{
        BaseVA=$BaseVA
        Bytes=[Collections.Generic.List[byte]]::new()
        Labels=@{}
        Fixups=[Collections.Generic.List[object]]::new()
    }
}
function Emit($a,[byte[]]$b) { $a.Bytes.AddRange($b) }
function Mark($a,[string]$n) {
    if($a.Labels.ContainsKey($n)){throw "rotulo duplicado: $n"}
    $a.Labels[$n]=$a.Bytes.Count
}
function Emit-Rel32($a,[byte[]]$op,[string]$label) {
    Emit $a $op
    $off=$a.Bytes.Count
    Emit $a ([byte[]](0,0,0,0))
    $a.Fixups.Add([pscustomobject]@{Offset=$off;Label=$label})
}
function Complete($a) {
    $out=$a.Bytes.ToArray()
    foreach($f in $a.Fixups) {
        if(-not $a.Labels.ContainsKey($f.Label)){throw "rotulo ausente: $($f.Label)"}
        $from=[uint32]($a.BaseVA+$f.Offset+4)
        $target=[uint32]($a.BaseVA+$a.Labels[$f.Label])
        [Array]::Copy((Rel32 $from $target),0,$out,$f.Offset,4)
    }
    return ,$out
}

if(-not(Test-Path -LiteralPath $Executable -PathType Leaf)){throw "WYD.exe ausente: $Executable"}
if(-not(Test-Path -LiteralPath $Manifest -PathType Leaf)){throw "manifesto standalone ausente: $Manifest"}

$actualHash=Get-Sha $Executable
if($actualHash -eq $expectedOutputHash) {
    Write-Host "Montarias KR standalone ja instaladas ($actualHash)."
    return
}
if($VerifyOnly) {
    throw "WYD.exe ainda nao contem a correcao standalone (SHA-256: $actualHash)"
}
if($actualHash -ne $expectedInputHash) {
    throw "WYD.exe fora da entrada suportada para o elo standalone (SHA-256: $actualHash)"
}

$items=@(Import-Csv -LiteralPath $Manifest | Sort-Object {[int]$_.item})
if($items.Count -ne 47){throw "manifesto standalone inesperado: $($items.Count) montarias"}
if(($items | Select-Object -ExpandProperty item -Unique).Count -ne 47){throw 'manifesto possui item duplicado'}
if(($items | Select-Object -ExpandProperty mountdata_index -Unique).Count -ne 47){throw 'manifesto possui mountdata_index duplicado'}

$backup=Join-Path (Split-Path -Parent $Executable) 'WYD.pre-mount-native.exe'
if(-not(Test-Path -LiteralPath $backup -PathType Leaf)) { Copy-Item -LiteralPath $Executable -Destination $backup }
elseif((Get-Sha $backup) -ne $expectedInputHash) { throw "backup pre-mount-native divergente: $backup" }

$data=[IO.File]::ReadAllBytes($Executable)
$costumeBefore=Get-RangeSha $data $costumeRaw $costumeSize

# Tabela de 24 bytes consumida pelos adapters 7.48 existentes. A origem agora
# e MountDataV standalone. EF_SANC 0xFFFF significa "nao sobrescrever" no
# client KR; como o adapter parte de zero, sua projecao compativel e byte 0.
for($i=0;$i -lt $items.Count;$i++) {
    $item=$items[$i]
    if([int]$item.extra0 -ne 0 -or [int]$item.extra1 -ne 1){throw "item $($item.item): extras MountDataV nao confirmados para este adapter"}
    $off=$sectionRaw+$tableOffset+$i*$entrySize
    Set-U16 $data $off ([uint16][int]$item.item)
    Set-U16 $data ($off+2) ([uint16][int]$item.type)
    Set-U32 $data ($off+4) ([Convert]::ToUInt32([string]$item.scale_bits,16))
    for($p=0;$p-lt 3;$p++) {
        $meshField="mesh$p";$skinField="skin$p";$sancField="sanc$p"
        Set-U16 $data ($off+8+$p*4) ([uint16][int]$item.$meshField)
        Set-U16 $data ($off+10+$p*4) ([uint16][int]$item.$skinField)
        $sanc=[int]$item.$sancField
        if($sanc -eq 65535) { $wireSanc=0 }
        elseif($sanc -ge 0 -and $sanc -le 255) { $wireSanc=$sanc }
        else { throw "item $($item.item): sanc fora do contrato: $sanc" }
        $data[$off+20+$p]=[byte]$wireSanc
    }
    $data[$off+23]=0
}

# Reescreve somente o payload do hook de pose ja instalado pelo elo anterior.
# Mapeamento de locais 7.48: [ebp-44]=axis1, [ebp-40]=axis2,
# [ebp-3c]=axis3. Os novos valores abaixo sao os branches equivalentes do
# client KR; os adapters 48/49/50 ja existentes sao preservados.
$const02VA=[uint32]($sectionVA+$poseConstantsOffset)
$const05VA=[uint32]($const02VA+4)
$const08VA=[uint32]($const02VA+8)
Set-F32 $data ($sectionRaw+$poseConstantsOffset) ([single]0.2)
Set-F32 $data ($sectionRaw+$poseConstantsOffset+4) ([single]0.5)
Set-F32 $data ($sectionRaw+$poseConstantsOffset+8) ([single]0.8)

$c=New-Assembler ([uint32]($sectionVA+$poseCodeOffset))
Emit $c ([byte[]](0x9C,0x60,0x8B,0x45,0x98,0x8B,0x88,0xA0,0x07,0x00,0x00,0x83,0xF9,0x1D))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type48'
Emit $c ([byte[]](0x0F,0xB7,0x90,0xA6,0x01,0x00,0x00,0x83,0xFA,0x05))
Emit-Rel32 $c ([byte[]](0x0F,0x84)) 'type29apply'
Emit $c ([byte[]](0x83,0xFA,0x0A))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'done'
Mark $c 'type29apply'
Emit $c ([byte[]](0xC7,0x45,0xBC,0xCD,0xCC,0x4C,0xBE,0xD9,0xE8,0xD8,0xB0,0xA4,0x07,0x00,0x00,0xD9,0x5D,0xC0))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type48'
Emit $c ([byte[]](0x83,0xF9,0x30))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type49'
Emit $c ([byte[]](0xD9,0x45,0xBC,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const08VA));Emit $c ([byte[]](0xD9,0x5D,0xBC,0xD9,0x45,0xC4,0xD8,0x05));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xC4))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type49'
Emit $c ([byte[]](0x83,0xF9,0x31))
Emit-Rel32 $c ([byte[]](0x0F,0x84)) 'type49or52'
Emit $c ([byte[]](0x83,0xF9,0x34))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type50'
Mark $c 'type49or52'
Emit $c ([byte[]](0xD9,0x45,0xBC,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xBC,0xD9,0x45,0xC4,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xC4))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type50'
Emit $c ([byte[]](0x83,0xF9,0x32))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type31'
Emit $c ([byte[]](0xD9,0x45,0xBC,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const05VA));Emit $c ([byte[]](0xD9,0x5D,0xBC,0xD9,0x45,0xC4,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xC4))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

# Loki: type 31 + mesh1 17 -> axis1 = -0.6.
Mark $c 'type31'
Emit $c ([byte[]](0x83,0xF9,0x1F))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type51'
Emit $c ([byte[]](0x0F,0xB7,0x90,0xA6,0x01,0x00,0x00,0x83,0xFA,0x11))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'done'
Emit $c ([byte[]](0xC7,0x45,0xBC,0x9A,0x99,0x19,0xBF))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

# type 51 -> axis1=-0.38, axis3=+0.30.
Mark $c 'type51'
Emit $c ([byte[]](0x83,0xF9,0x33))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type59'
Emit $c ([byte[]](0xC7,0x45,0xBC,0x5C,0x8F,0xC2,0xBE,0xC7,0x45,0xC4,0x9A,0x99,0x99,0x3E))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

# type 59 -> axis1=-0.18, axis2=1.0, axis3=-0.20.
Mark $c 'type59'
Emit $c ([byte[]](0x83,0xF9,0x3B))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'done'
Emit $c ([byte[]](0xC7,0x45,0xBC,0xEC,0x51,0x38,0xBE,0xC7,0x45,0xC0,0x00,0x00,0x80,0x3F,0xC7,0x45,0xC4,0xCD,0xCC,0x4C,0xBE))

Mark $c 'done'
Emit $c ([byte[]](0x61,0x9D,0x8B,0x4D,0xC4,0x51,0x8B,0x55,0xC0,0xE9))
$poseReturnNext=[uint32]($c.BaseVA+$c.Bytes.Count+4)
Emit $c (Rel32 $poseReturnNext ([uint32]0x005042C7))
$poseCode=Complete $c
if($poseCode.Length -gt 0x200){throw 'hook de assento standalone excedeu a area reservada'}
for($i=0;$i-lt 0x200;$i++){$data[$sectionRaw+$poseCodeOffset+$i]=0}
Set-Bytes $data ($sectionRaw+$poseCodeOffset) $poseCode

# O source KR define estados explicitos do child TMSkinMesh antes de aplicar a
# escala: 48->3, 49/52/50->6 e 59->4. O 7.48 possui o mesmo ponto sem esses
# branches. Hook version-local em 0x51E3A4, antes da multiplicacao por scale.
$childHookOffset=0x0011E3A4
$childHookVA=[uint32]0x0051E3A4
$childOriginal=[byte[]](0x8B,0x45,0xB8,0x8B,0x88,0x9C,0x01,0x00,0x00)
Assert-Bytes $data $childHookOffset $childOriginal 'estado filho da montaria 7.48'

$d=New-Assembler ([uint32]($sectionVA+$childAnimCodeOffset))
Emit $d ([byte[]](0x9C,0x60,0x8B,0x45,0xB8,0x8B,0x88,0x9C,0x01,0x00,0x00,0x85,0xC9))
Emit-Rel32 $d ([byte[]](0x0F,0x84)) 'done'
Emit $d ([byte[]](0x8B,0x90,0xA0,0x07,0x00,0x00))
Emit $d ([byte[]](0x83,0xFA,0x30));Emit-Rel32 $d ([byte[]](0x0F,0x84)) 'set3'
Emit $d ([byte[]](0x83,0xFA,0x31));Emit-Rel32 $d ([byte[]](0x0F,0x84)) 'set6'
Emit $d ([byte[]](0x83,0xFA,0x34));Emit-Rel32 $d ([byte[]](0x0F,0x84)) 'set6'
Emit $d ([byte[]](0x83,0xFA,0x32));Emit-Rel32 $d ([byte[]](0x0F,0x84)) 'set6'
Emit $d ([byte[]](0x83,0xFA,0x3B));Emit-Rel32 $d ([byte[]](0x0F,0x84)) 'set4'
Emit-Rel32 $d ([byte[]](0xE9)) 'done'
Mark $d 'set3';Emit $d ([byte[]](0xC7,0x81,0xE4,0x02,0x00,0x00,0x03,0x00,0x00,0x00));Emit-Rel32 $d ([byte[]](0xE9)) 'done'
Mark $d 'set6';Emit $d ([byte[]](0xC7,0x81,0xE4,0x02,0x00,0x00,0x06,0x00,0x00,0x00));Emit-Rel32 $d ([byte[]](0xE9)) 'done'
Mark $d 'set4';Emit $d ([byte[]](0xC7,0x81,0xE4,0x02,0x00,0x00,0x04,0x00,0x00,0x00));Emit-Rel32 $d ([byte[]](0xE9)) 'done'
Mark $d 'done'
Emit $d ([byte[]](0x61,0x9D));Emit $d $childOriginal;Emit $d ([byte[]](0xE9))
$childReturnNext=[uint32]($d.BaseVA+$d.Bytes.Count+4)
Emit $d (Rel32 $childReturnNext ([uint32]0x0051E3AD))
$childCode=Complete $d
if($childCode.Length -gt 0x200){throw 'hook de animacao filha excedeu a area reservada'}
Set-Bytes $data ($sectionRaw+$childAnimCodeOffset) $childCode
$childHook=[byte[]](0xE9)+[byte[]](Rel32 ($childHookVA+5) ([uint32]($sectionVA+$childAnimCodeOffset)))+([byte[]](0x90)*4)
Set-Bytes $data $childHookOffset $childHook

$costumeAfter=Get-RangeSha $data $costumeRaw $costumeSize
if($costumeAfter -ne $costumeBefore){throw 'regressao: secao .costkr foi alterada pelo patch de montarias'}

[IO.File]::WriteAllBytes($Executable,$data)
$newHash=Get-Sha $Executable
if($newHash -ne $expectedOutputHash){throw "saida standalone divergente: $newHash"}
Write-Host "$($items.Count) montarias KR rematerializadas a partir de MountDataV standalone."
Write-Host "SHA-256 antes:  $actualHash"
Write-Host "SHA-256 depois: $newHash"
Write-Host "Backup: $backup"
