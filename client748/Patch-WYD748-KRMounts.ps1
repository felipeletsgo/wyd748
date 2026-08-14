[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Renderiza os costumes de montaria do client KR como itens Equip[14]
# independentes no 7.48. O hook usa exclusivamente offsets/ABI do executavel
# 7.48; a tabela importada contem apenas dados visuais do SetMountCostume KR.

$expectedInputHash = '4A2AA37228A720ED389F5AC8A5978329855932B93E54FA0501B51A3A23316DEF'
$legacyOutputHashes = @(
    '738FEBA396B4273721F63B17F044DA0AEFFAC20ECF9EFA195BE603E427775D53',
    '93418B98E42877596ACC4D52D518475059358B5954A7D7CBD38F97E5B9BEFE63',
    '63FFDF621631803BE4096730B0096F7617DB73D4DE517078A4EC2A49B17D96DF',
    'A5DCF57BD8C7D891E42796A7EBC028A3DD22DA7EF53C1A8DB0E023BAB3BCE8A3',
    '6D853FE2F69EDFFB4A1EC4793EE18CF651B5E5604E3009703D379C2B182B76CB',
    '78B27091ACF3B0DA0258E7F7510E55CA3A78C721C237F5B99F767CB780512005',
    'E1C34874E8BA5B4CF018F262D84A581DCA2242DD7F67410B4807B57BCC3691EA',
    '0648B586AF95D26FB0B0C27ED0F954FE5F8D291E4D3DD10B73BB816B3D5B1A75'
)
$expectedOutputHash = '556EC07005D17DCEDEF0CE15B8C8FDB13AE1E82975D992778ACDA846C108CD8F' # KR_MOUNT_OUTPUT_HASH
$sectionRVA = [uint32]0x00FD2000
$sectionRaw = 0x001E5000
$sectionSize = 0x00010000
$sectionVA = [uint32]0x013D2000
$codeOffset = 0x0000
$incrementalCodeOffset = 0x0200
$tableOffset = 0x0400
$poseCodeOffset = 0x1000
$poseConstantsOffset = 0x1200
$rotationCodeOffset = 0x1400
$runToggleCodeOffset = 0x1800
$runUICodeOffset = 0x1A00
$runSpeedCodeOffset = 0x1C00
$entrySize = 24

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant() }
function Assert-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Expected,[string]$Name) {
    for($i=0;$i -lt $Expected.Length;$i++) { if($Data[$Offset+$i] -ne $Expected[$i]) {
        throw ('{0}: byte inesperado em 0x{1:X}' -f $Name,($Offset+$i))
    }}
}
function Set-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Value) { [Array]::Copy($Value,0,$Data,$Offset,$Value.Length) }
function Set-U16([byte[]]$Data,[int]$Offset,[uint16]$Value) { Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value)) }
function Set-U32([byte[]]$Data,[int]$Offset,[uint32]$Value) { Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value)) }
function Rel32([uint32]$FromNextInstruction,[uint32]$Target) { return ,([BitConverter]::GetBytes([int32]([int64]$Target-[int64]$FromNextInstruction))) }
function New-Assembler([uint32]$BaseVA) { [pscustomobject]@{BaseVA=$BaseVA;Bytes=[Collections.Generic.List[byte]]::new();Labels=@{};Fixups=[Collections.Generic.List[object]]::new()} }
function Emit($a,[byte[]]$b) { $a.Bytes.AddRange($b) }
function Mark($a,[string]$n) { if($a.Labels.ContainsKey($n)){throw "rotulo duplicado: $n"};$a.Labels[$n]=$a.Bytes.Count }
function Emit-Rel32($a,[byte[]]$op,[string]$label) { Emit $a $op;$off=$a.Bytes.Count;Emit $a ([byte[]](0,0,0,0));$a.Fixups.Add([pscustomobject]@{Offset=$off;Label=$label}) }
function Complete($a) {
    $out=$a.Bytes.ToArray()
    foreach($f in $a.Fixups){if(-not $a.Labels.ContainsKey($f.Label)){throw "rotulo ausente: $($f.Label)"};$from=[uint32]($a.BaseVA+$f.Offset+4);$target=[uint32]($a.BaseVA+$a.Labels[$f.Label]);[Array]::Copy((Rel32 $from $target),0,$out,$f.Offset,4)}
    return ,$out
}
function Add-PESection([byte[]]$inputData) {
    if($inputData.Length -ne $sectionRaw){throw ('WYD.exe possui tamanho inesperado antes de .mountkr: 0x{0:X}' -f $inputData.Length)}
    $pe=[BitConverter]::ToInt32($inputData,0x3C)
    if([Text.Encoding]::ASCII.GetString($inputData,$pe,4) -ne "PE`0`0"){throw 'cabecalho PE invalido'}
    $sections=[BitConverter]::ToUInt16($inputData,$pe+6);$optionalSize=[BitConverter]::ToUInt16($inputData,$pe+20)
    if($sections -ne 7){throw "numero de secoes inesperado: $sections"}
    $sectionTable=$pe+24+$optionalSize;$newHeader=$sectionTable+$sections*40
    Assert-Bytes $inputData $newHeader ([byte[]](0)*40) 'cabecalho livre da oitava secao'
    $lastHeader=$sectionTable+($sections-1)*40
    $lastName=[Text.Encoding]::ASCII.GetString($inputData,$lastHeader,8).Trim([char]0)
    $lastRawSize=[BitConverter]::ToUInt32($inputData,$lastHeader+16);$lastRaw=[BitConverter]::ToUInt32($inputData,$lastHeader+20)
    if($lastName -ne '.costkr' -or $lastRaw+$lastRawSize -ne $sectionRaw){throw 'layout .costkr anterior inesperado'}
    $expanded=New-Object byte[] ($sectionRaw+$sectionSize);[Array]::Copy($inputData,$expanded,$inputData.Length)
    Set-U16 $expanded ($pe+6) 8;Set-U32 $expanded ($pe+24+56) ([uint32]0x00FE2000)
    Set-Bytes $expanded $newHeader ([Text.Encoding]::ASCII.GetBytes(".mountkr"))
    Set-U32 $expanded ($newHeader+8) ([uint32]$sectionSize);Set-U32 $expanded ($newHeader+12) $sectionRVA
    Set-U32 $expanded ($newHeader+16) ([uint32]$sectionSize);Set-U32 $expanded ($newHeader+20) ([uint32]$sectionRaw)
    Set-U32 $expanded ($newHeader+36) ([Convert]::ToUInt32('E0000060',16))
    return ,$expanded
}

if(-not(Test-Path -LiteralPath $Executable -PathType Leaf)){throw "WYD.exe ausente: $Executable"}
if(-not(Test-Path -LiteralPath $Manifest -PathType Leaf)){throw "manifesto ausente: $Manifest"}
$definition=Get-Content -LiteralPath $Manifest -Raw|ConvertFrom-Json
$catalogItems=@($definition.items|Sort-Object item)
$items=@($catalogItems|Where-Object { $_.available -ne $false })
$unavailableItems=@($catalogItems|Where-Object { $_.available -eq $false })
if($catalogItems.Count -ne 47 -or $items.Count -ne 45 -or $unavailableItems.Count -ne 2){
    throw "manifesto inesperado: catalogo=$($catalogItems.Count) disponiveis=$($items.Count) indisponiveis=$($unavailableItems.Count)"
}
$actualHash=Get-Sha $Executable
if($expectedOutputHash -and $actualHash -eq $expectedOutputHash){Write-Host "Montarias KR ja instaladas ($actualHash).";return}
if($VerifyOnly){throw "WYD.exe ainda nao contem montarias KR (SHA-256: $actualHash)"}
$backup=Join-Path (Split-Path -Parent $Executable) 'WYD.pre-mounts-kr.exe'
if($legacyOutputHashes -contains $actualHash){
    $versionedBackup=Join-Path (Split-Path -Parent $Executable) ("WYD.pre-mounts-kr.{0}.exe" -f $expectedInputHash.Substring(0,12))
    if((Test-Path -LiteralPath $backup -PathType Leaf) -and (Get-Sha $backup) -eq $expectedInputHash){
        $sourceBackup=$backup
    }elseif((Test-Path -LiteralPath $versionedBackup -PathType Leaf) -and (Get-Sha $versionedBackup) -eq $expectedInputHash){
        $sourceBackup=$versionedBackup
    }else{
        throw "montarias KR legadas detectadas, mas nenhum backup pre-montarias possui SHA-256 $expectedInputHash"
    }
    Copy-Item -LiteralPath $sourceBackup -Destination $Executable -Force
    $actualHash=Get-Sha $Executable
    Write-Host 'Montarias KR legadas detectadas; reconstruindo o elo com refresh imediato.'
}
if($actualHash -ne $expectedInputHash){throw "WYD.exe fora da entrada suportada (SHA-256: $actualHash)"}

if(Test-Path -LiteralPath $backup -PathType Leaf){
    $backupHash=Get-Sha $backup
    if($backupHash -ne $actualHash){
        $backup=Join-Path (Split-Path -Parent $Executable) ("WYD.pre-mounts-kr.{0}.exe" -f $actualHash.Substring(0,12))
        if(Test-Path -LiteralPath $backup -PathType Leaf){
            if((Get-Sha $backup) -ne $actualHash){throw "backup versionado de montarias divergente: $backup"}
        }else{Copy-Item -LiteralPath $Executable -Destination $backup}
    }
}else{Copy-Item -LiteralPath $Executable -Destination $backup}
$inputData=[IO.File]::ReadAllBytes($Executable)
$hookOffset=0x12538B;$hookVA=[uint32]0x0052538B
$original=[byte[]](0x8B,0x4D,0x08,0x0F,0xBF,0x91,0xCC,0x00,0x00,0x00)
Assert-Bytes $inputData $hookOffset $original 'seletor nativo de montaria 7.48'
$incrementalHookOffset=0x1266DB;$incrementalHookVA=[uint32]0x005266DB
$incrementalOriginal=[byte[]](0x8B,0x4D,0x08,0x33,0xD2)
Assert-Bytes $inputData $incrementalHookOffset $incrementalOriginal 'refresh incremental de Equip[14] 7.48'
$mountConstructHookOffset=0x12799E;$mountConstructHookVA=[uint32]0x0052799E
$mountConstructOriginal=[byte[]](0x89,0x45,0xC8,0xEB,0x07)
Assert-Bytes $inputData $mountConstructHookOffset $mountConstructOriginal 'retorno do construtor TMSkinMesh de montaria 7.48'
$runToggleHookOffset=0x5096F;$runToggleHookVA=[uint32]0x0045096F
$runToggleOriginal=[byte[]](0x0F,0xBE,0x91,0x9C,0x07,0x00,0x00)
Assert-Bytes $inputData $runToggleHookOffset $runToggleOriginal 'gate nativo da tecla R de montaria 7.48'
$runUIHookOffset=0x46F8A;$runUIHookVA=[uint32]0x00446F8A
$runUIOriginal=[byte[]](0x0F,0xBE,0x91,0x9C,0x07,0x00,0x00)
Assert-Bytes $inputData $runUIHookOffset $runUIOriginal 'gate nativo do botao de corrida de montaria 7.48'
$runSpeedHookOffset=0xFA8D9;$runSpeedHookVA=[uint32]0x004FA8D9
$runSpeedOriginal=[byte[]](0x8B,0x4D,0xFC,0x83,0xB9,0xA0,0x07,0x00,0x00,0x1F)
Assert-Bytes $inputData $runSpeedHookOffset $runSpeedOriginal 'gate nativo da velocidade de corrida de montaria 7.48'
$data=Add-PESection $inputData
$tableVA=[uint32]($sectionVA+$tableOffset)

for($i=0;$i -lt $items.Count;$i++){
    $item=$items[$i];$off=$sectionRaw+$tableOffset+$i*$entrySize
    if(@($item.mesh).Count-ne 3 -or @($item.skin).Count-ne 3 -or @($item.sanc).Count-ne 3){throw "item $($item.item): visual incompleto"}
    Set-U16 $data $off ([uint16]$item.item);Set-U16 $data ($off+2) ([uint16]$item.type)
    Set-U32 $data ($off+4) ([BitConverter]::ToUInt32([BitConverter]::GetBytes([single]$item.scale),0))
    for($p=0;$p-lt 3;$p++){Set-U16 $data ($off+8+$p*4) ([uint16]$item.mesh[$p]);Set-U16 $data ($off+10+$p*4) ([uint16]$item.skin[$p]);$data[$off+20+$p]=[byte]$item.sanc[$p]}
    $data[$off+23]=[byte]$item.special
}

$a=New-Assembler ([uint32]($sectionVA+$codeOffset))
Emit $a ([byte[]](
    0x60,0x8B,0x45,0x08,0x0F,0xB7,0x90,0xCC,0x00,0x00,0x00,
    0x81,0xE2,0xFF,0x0F,0x00,0x00,
    0x81,0xC2,0x00,0x10,0x00,0x00,
    0xB9))
Emit $a ([BitConverter]::GetBytes([uint32]$items.Count));Emit $a ([byte[]](0xBE));Emit $a ([BitConverter]::GetBytes($tableVA))
Mark $a 'loop';Emit $a ([byte[]](0x66,0x3B,0x16));Emit-Rel32 $a ([byte[]](0x0F,0x84)) 'found'
Emit $a ([byte[]](0x83,0xC6,$entrySize,0x49));Emit-Rel32 $a ([byte[]](0x0F,0x85)) 'loop'
Emit $a ([byte[]](0x61));Emit $a $original;Emit $a ([byte[]](0xE9))
$fallbackNext=[uint32]($a.BaseVA+$a.Bytes.Count+4);Emit $a (Rel32 $fallbackNext ([uint32]0x00525395))
Mark $a 'found'
Emit $a ([byte[]](
    0x8B,0x7D,0x80,
    0x8A,0x87,0x9C,0x07,0x00,0x00,0x88,0x87,0x9D,0x07,0x00,0x00,0xC6,0x87,0x9C,0x07,0x00,0x00,0x01,
    0xC7,0x45,0xEC,0x01,0x00,0x00,0x00,0xC7,0x45,0xD8,0x56,0x01,0x00,0x00,0xC7,0x45,0xE4,0x00,0x00,0x00,0x00,
    0x66,0xC7,0x87,0xA0,0x01,0x00,0x00,0x1B,0x00,
    0x0F,0xB7,0x46,0x02,0x89,0x87,0xA0,0x07,0x00,0x00,
    0x8B,0x46,0x04,0x89,0x87,0xA4,0x07,0x00,0x00,
    0x8B,0x46,0x08,0x89,0x87,0xA2,0x01,0x00,0x00,
    0x8B,0x46,0x0C,0x89,0x87,0xA6,0x01,0x00,0x00,
    0x8B,0x46,0x10,0x89,0x87,0xAA,0x01,0x00,0x00,
    0x8A,0x46,0x14,0x88,0x87,0xC2,0x01,0x00,0x00,
    0x8A,0x46,0x15,0x88,0x87,0xC3,0x01,0x00,0x00,
    0x8A,0x46,0x16,0x88,0x87,0xC4,0x01,0x00,0x00,
    0x61,0xE9))
$foundNext=[uint32]($a.BaseVA+$a.Bytes.Count+4);Emit $a (Rel32 $foundNext ([uint32]0x00525954))
$code=Complete $a
if($code.Length -gt $incrementalCodeOffset){throw 'hook de materializacao completa excedeu a area reservada'}
Set-Bytes $data ($sectionRaw+$codeOffset) $code
$hook=[byte[]](0xE9)+[byte[]](Rel32 ($hookVA+5) ([uint32]($sectionVA+$codeOffset)))+([byte[]](0x90)*5)
Set-Bytes $data $hookOffset $hook

# O pacote 0x36B atualiza somente o equipamento visual. O client 7.48 mascara
# Equip[14] com 0x0FFF e, por isso, interpreta os IDs 4190+ como 94+ e remove a
# montaria. Este adapter resolve a mesma tabela visual antes de InitObject(),
# fazendo equipar/desequipar refletir imediatamente para owner e observadores.
$b=New-Assembler ([uint32]($sectionVA+$incrementalCodeOffset))
Emit $b ([byte[]](
    0x60,
    0x8B,0x45,0x08,
    0x0F,0xB7,0x50,0x1C,
    0x81,0xE2,0xFF,0x0F,0x00,0x00,
    0x81,0xC2,0x00,0x10,0x00,0x00,
    0xB9))
Emit $b ([BitConverter]::GetBytes([uint32]$items.Count));Emit $b ([byte[]](0xBE));Emit $b ([BitConverter]::GetBytes($tableVA))
Mark $b 'loop';Emit $b ([byte[]](0x66,0x3B,0x16));Emit-Rel32 $b ([byte[]](0x0F,0x84)) 'found'
Emit $b ([byte[]](0x83,0xC6,$entrySize,0x49));Emit-Rel32 $b ([byte[]](0x0F,0x85)) 'loop'
Emit $b ([byte[]](0x61));Emit $b $incrementalOriginal;Emit $b ([byte[]](0xE9))
$incrementalFallbackNext=[uint32]($b.BaseVA+$b.Bytes.Count+4);Emit $b (Rel32 $incrementalFallbackNext ([uint32]0x005266E0))
Mark $b 'found'
Emit $b ([byte[]](
    0x8B,0x7D,0xD8,
    0x8A,0x87,0x9C,0x07,0x00,0x00,0x88,0x87,0x9D,0x07,0x00,0x00,0xC6,0x87,0x9C,0x07,0x00,0x00,0x01,
    0x66,0xC7,0x87,0xA0,0x01,0x00,0x00,0x1B,0x00,
    0x0F,0xB7,0x46,0x02,0x89,0x87,0xA0,0x07,0x00,0x00,
    0x8B,0x46,0x04,0x89,0x87,0xA4,0x07,0x00,0x00,
    0x8B,0x46,0x08,0x89,0x87,0xA2,0x01,0x00,0x00,
    0x8B,0x46,0x0C,0x89,0x87,0xA6,0x01,0x00,0x00,
    0x8B,0x46,0x10,0x89,0x87,0xAA,0x01,0x00,0x00,
    0x8A,0x46,0x14,0x88,0x87,0xC2,0x01,0x00,0x00,
    0x8A,0x46,0x15,0x88,0x87,0xC3,0x01,0x00,0x00,
    0x8A,0x46,0x16,0x88,0x87,0xC4,0x01,0x00,0x00,
    0x61,0xE9))
$incrementalFoundNext=[uint32]($b.BaseVA+$b.Bytes.Count+4);Emit $b (Rel32 $incrementalFoundNext ([uint32]0x00526BAE))
$incrementalCode=Complete $b
if($incrementalCode.Length -gt ($tableOffset-$incrementalCodeOffset)){throw 'hook incremental excedeu a area reservada'}
Set-Bytes $data ($sectionRaw+$incrementalCodeOffset) $incrementalCode
$incrementalHook=[byte[]](0xE9)+[byte[]](Rel32 ($incrementalHookVA+5) ([uint32]($sectionVA+$incrementalCodeOffset)))
Set-Bytes $data $incrementalHookOffset $incrementalHook

# O renderer 7.48 conhece somente os offsets de assento das montarias antigas.
# Este adapter roda depois dos ajustes corporais nativos e porta somente as
# formulas observadas no WYD.exe KR de referencia. Sao offsets aditivos/diretos;
# multiplicar os vetores (como fazia o patch anterior) desloca o cavaleiro.
$poseHookOffset=0x1042C0;$poseHookVA=[uint32]0x005042C0
$poseOriginal=[byte[]](0x8B,0x4D,0xC4,0x51,0x8B,0x55,0xC0)
Assert-Bytes $inputData $poseHookOffset $poseOriginal 'offset de assento de montaria 7.48'
$const02VA=[uint32]($sectionVA+$poseConstantsOffset)
$const05VA=[uint32]($const02VA+4)
$const08VA=[uint32]($const02VA+8)
Set-U32 $data ($sectionRaw+$poseConstantsOffset) ([BitConverter]::ToUInt32([BitConverter]::GetBytes([single]0.2),0))
Set-U32 $data ($sectionRaw+$poseConstantsOffset+4) ([BitConverter]::ToUInt32([BitConverter]::GetBytes([single]0.5),0))
Set-U32 $data ($sectionRaw+$poseConstantsOffset+8) ([BitConverter]::ToUInt32([BitConverter]::GetBytes([single]0.8),0))

$c=New-Assembler ([uint32]($sectionVA+$poseCodeOffset))
Emit $c ([byte[]](0x60,0x8B,0x45,0x98,0x8B,0x88,0xA0,0x07,0x00,0x00,0x83,0xF9,0x1D))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type31'
# type 29, meshes 5 e 10: X=-0.2 e escala vertical inversa, como no KR.
Emit $c ([byte[]](0x0F,0xB7,0x90,0xA6,0x01,0x00,0x00,0x83,0xFA,0x05))
Emit-Rel32 $c ([byte[]](0x0F,0x84)) 'type29special'
Emit $c ([byte[]](0x83,0xFA,0x0A))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'done'
Mark $c 'type29special'
Emit $c ([byte[]](0xC7,0x45,0xBC,0xCD,0xCC,0x4C,0xBE,0xD9,0xE8,0xD8,0xB0,0xA4,0x07,0x00,0x00,0xD9,0x5D,0xC0))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type31'
Emit $c ([byte[]](0x83,0xF9,0x1F))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type48'
# Loki/type31 mesh17: o KR fixa X em -0.6.
Emit $c ([byte[]](0x0F,0xB7,0x90,0xA6,0x01,0x00,0x00,0x83,0xFA,0x11))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'done'
Emit $c ([byte[]](0xC7,0x45,0xBC,0x9A,0x99,0x19,0xBF))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type48'
Emit $c ([byte[]](0x83,0xF9,0x30))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type49'
# type48: X -= 0.8; Z += 0.2.
Emit $c ([byte[]](0xD9,0x45,0xBC,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const08VA));Emit $c ([byte[]](0xD9,0x5D,0xBC))
Emit $c ([byte[]](0xD9,0x45,0xC4,0xD8,0x05));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xC4))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type49'
Emit $c ([byte[]](0x83,0xF9,0x31))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type50'
# type49: X -= 0.2; Z -= 0.2. Mesh24 nao recebe o ajuste de Z.
Emit $c ([byte[]](0xD9,0x45,0xBC,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xBC))
Emit $c ([byte[]](0x0F,0xB7,0x90,0xA6,0x01,0x00,0x00,0x83,0xFA,0x18))
Emit-Rel32 $c ([byte[]](0x0F,0x84)) 'done'
Emit $c ([byte[]](0xD9,0x45,0xC4,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xC4))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type50'
Emit $c ([byte[]](0x83,0xF9,0x32))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type51'
# As montarias type50 habilitadas usam meshes 0/1: ramo padrao KR.
Emit $c ([byte[]](0xD9,0x45,0xBC,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const05VA));Emit $c ([byte[]](0xD9,0x5D,0xBC))
Emit $c ([byte[]](0xD9,0x45,0xC4,0xD8,0x25));Emit $c ([BitConverter]::GetBytes($const02VA));Emit $c ([byte[]](0xD9,0x5D,0xC4))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type51'
Emit $c ([byte[]](0x83,0xF9,0x33))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'type59'
# type51: X=-0.38; Z=+0.30.
Emit $c ([byte[]](0xC7,0x45,0xBC,0x5C,0x8F,0xC2,0xBE,0xC7,0x45,0xC4,0x9A,0x99,0x99,0x3E))
Emit-Rel32 $c ([byte[]](0xE9)) 'done'

Mark $c 'type59'
Emit $c ([byte[]](0x83,0xF9,0x3B))
Emit-Rel32 $c ([byte[]](0x0F,0x85)) 'done'
# type59: X=-0.18; Z=-0.20; Y=1.0.
Emit $c ([byte[]](0xC7,0x45,0xBC,0xEC,0x51,0x38,0xBE,0xC7,0x45,0xC4,0xCD,0xCC,0x4C,0xBE,0xC7,0x45,0xC0,0x00,0x00,0x80,0x3F))

Mark $c 'done'
Emit $c ([byte[]](0x61));Emit $c $poseOriginal;Emit $c ([byte[]](0xE9))
$poseReturnNext=[uint32]($c.BaseVA+$c.Bytes.Count+4);Emit $c (Rel32 $poseReturnNext ([uint32]0x005042C7))
$poseCode=Complete $c
if($poseCode.Length -gt ($poseConstantsOffset-$poseCodeOffset)){throw 'hook de assento excedeu a area reservada'}
Set-Bytes $data ($sectionRaw+$poseCodeOffset) $poseCode
$poseHook=[byte[]](0xE9)+[byte[]](Rel32 ($poseHookVA+5) ([uint32]($sectionVA+$poseCodeOffset)))+([byte[]](0x90)*2)
Set-Bytes $data $poseHookOffset $poseHook

# O renderer antigo elimina faces CW/CCW, enquanto as meshes esqueletais KR
# sao two-sided no client atual. Depois de construir o TMSkinMesh da montaria,
# reconhecemos a assinatura visual exata (tipo, escala e tres pares mesh/skin)
# da tabela importada e gravamos a mesma sentinela usada pelo hook seletivo de
# CULL_NONE instalado no elo de trajes. Nenhuma montaria nativa e alterada.
$d=New-Assembler ([uint32]($sectionVA+$rotationCodeOffset))
Emit $d ([byte[]](0x89,0x45,0xC8,0x60,0x85,0xC0))
Emit-Rel32 $d ([byte[]](0x0F,0x84)) 'done'
Emit $d ([byte[]](
    0x8B,0xF8,
    0x8B,0x5D,0xD0,
    0xB9))
Emit $d ([BitConverter]::GetBytes([uint32]$items.Count))
Emit $d ([byte[]](0xBE));Emit $d ([BitConverter]::GetBytes($tableVA))
Mark $d 'loop'
Emit $d ([byte[]](
    0x0F,0xB7,0x46,0x02,
    0x3B,0x83,0xA0,0x07,0x00,0x00))
Emit-Rel32 $d ([byte[]](0x0F,0x85)) 'next'
Emit $d ([byte[]](
    0x8B,0x46,0x04,0x3B,0x83,0xA4,0x07,0x00,0x00))
Emit-Rel32 $d ([byte[]](0x0F,0x85)) 'next'
Emit $d ([byte[]](
    0x8B,0x46,0x08,0x3B,0x83,0xA2,0x01,0x00,0x00))
Emit-Rel32 $d ([byte[]](0x0F,0x85)) 'next'
Emit $d ([byte[]](
    0x8B,0x46,0x0C,0x3B,0x83,0xA6,0x01,0x00,0x00))
Emit-Rel32 $d ([byte[]](0x0F,0x85)) 'next'
Emit $d ([byte[]](
    0x8B,0x46,0x10,0x3B,0x83,0xAA,0x01,0x00,0x00))
Emit-Rel32 $d ([byte[]](0x0F,0x85)) 'next'
Emit $d ([byte[]](0xC6,0x87,0x51,0x03,0x00,0x00,0x7F))
Emit-Rel32 $d ([byte[]](0xE9)) 'done'
Mark $d 'next'
Emit $d ([byte[]](0x83,0xC6,$entrySize,0x49))
Emit-Rel32 $d ([byte[]](0x0F,0x85)) 'loop'
Mark $d 'done'
Emit $d ([byte[]](0x61,0xE9))
$mountConstructReturnNext=[uint32]($d.BaseVA+$d.Bytes.Count+4)
Emit $d (Rel32 $mountConstructReturnNext ([uint32]0x005279AA))
$mountConstructCode=Complete $d
if($rotationCodeOffset+$mountConstructCode.Length -gt $sectionSize){throw 'hook de marcacao das montarias excedeu .mountkr'}
Set-Bytes $data ($sectionRaw+$rotationCodeOffset) $mountConstructCode
$mountConstructHook=[byte[]](0xE9)+[byte[]](Rel32 ($mountConstructHookVA+5) ([uint32]($sectionVA+$rotationCodeOffset)))
Set-Bytes $data $mountConstructHookOffset $mountConstructHook

# O 7.48 admite corrida somente para quatro tipos visuais nativos
# (31/40/20/39). As montarias KR materializadas possuem outros tipos, embora
# sejam Equip[14] validas. Cada adapter abaixo conserva o predicado nativo e
# amplia somente as instancias cuja assinatura visual completa aparece na
# tabela .mountkr. Assim um NPC/objeto que reutilize o mesmo numero de tipo nao
# ganha a funcao de corrida.
function Build-MountRunAdapter(
    [uint32]$BaseVA,
    [byte[]]$HumanLoad,
    [byte[]]$OriginalBytes,
    [uint32]$NativeContinue,
    [uint32]$EligibleTarget
) {
    $r = New-Assembler $BaseVA
    Emit $r ([byte[]](0x60))
    Emit $r $HumanLoad
    Emit $r ([byte[]](0x80,0xBF,0x9C,0x07,0x00,0x00,0x01))
    Emit-Rel32 $r ([byte[]](0x0F,0x85)) 'native'
    Emit $r ([byte[]](0xB9));Emit $r ([BitConverter]::GetBytes([uint32]$items.Count))
    Emit $r ([byte[]](0xBE));Emit $r ([BitConverter]::GetBytes($tableVA))
    Mark $r 'loop'
    Emit $r ([byte[]](
        0x0F,0xB7,0x46,0x02,0x3B,0x87,0xA0,0x07,0x00,0x00))
    Emit-Rel32 $r ([byte[]](0x0F,0x85)) 'next'
    Emit $r ([byte[]](0x8B,0x46,0x04,0x3B,0x87,0xA4,0x07,0x00,0x00))
    Emit-Rel32 $r ([byte[]](0x0F,0x85)) 'next'
    Emit $r ([byte[]](0x8B,0x46,0x08,0x3B,0x87,0xA2,0x01,0x00,0x00))
    Emit-Rel32 $r ([byte[]](0x0F,0x85)) 'next'
    Emit $r ([byte[]](0x8B,0x46,0x0C,0x3B,0x87,0xA6,0x01,0x00,0x00))
    Emit-Rel32 $r ([byte[]](0x0F,0x85)) 'next'
    Emit $r ([byte[]](0x8B,0x46,0x10,0x3B,0x87,0xAA,0x01,0x00,0x00))
    Emit-Rel32 $r ([byte[]](0x0F,0x85)) 'next'
    Emit $r ([byte[]](0x61,0xE9))
    $eligibleNext=[uint32]($r.BaseVA+$r.Bytes.Count+4)
    Emit $r (Rel32 $eligibleNext $EligibleTarget)
    Mark $r 'next'
    Emit $r ([byte[]](0x83,0xC6,$entrySize,0x49))
    Emit-Rel32 $r ([byte[]](0x0F,0x85)) 'loop'
    Mark $r 'native'
    Emit $r ([byte[]](0x61));Emit $r $OriginalBytes;Emit $r ([byte[]](0xE9))
    $nativeNext=[uint32]($r.BaseVA+$r.Bytes.Count+4)
    Emit $r (Rel32 $nativeNext $NativeContinue)
    return ,(Complete $r)
}

$runToggleCode=Build-MountRunAdapter ([uint32]($sectionVA+$runToggleCodeOffset)) `
    ([byte[]](0x8B,0xF9)) $runToggleOriginal ([uint32]0x00450976) ([uint32]0x004509CB)
if($runToggleCode.Length-gt($runUICodeOffset-$runToggleCodeOffset)){throw 'adapter da tecla R excedeu a area reservada'}
Set-Bytes $data ($sectionRaw+$runToggleCodeOffset) $runToggleCode
$runToggleHook=[byte[]](0xE9)+[byte[]](Rel32 ($runToggleHookVA+5) ([uint32]($sectionVA+$runToggleCodeOffset)))+([byte[]](0x90)*2)
Set-Bytes $data $runToggleHookOffset $runToggleHook

$runUICode=Build-MountRunAdapter ([uint32]($sectionVA+$runUICodeOffset)) `
    ([byte[]](0x8B,0xF9)) $runUIOriginal ([uint32]0x00446F91) ([uint32]0x00446FF9)
if($runUICode.Length-gt($runSpeedCodeOffset-$runUICodeOffset)){throw 'adapter da UI de corrida excedeu a area reservada'}
Set-Bytes $data ($sectionRaw+$runUICodeOffset) $runUICode
$runUIHook=[byte[]](0xE9)+[byte[]](Rel32 ($runUIHookVA+5) ([uint32]($sectionVA+$runUICodeOffset)))+([byte[]](0x90)*2)
Set-Bytes $data $runUIHookOffset $runUIHook

$runSpeedCode=Build-MountRunAdapter ([uint32]($sectionVA+$runSpeedCodeOffset)) `
    ([byte[]](0x8B,0x7D,0xFC)) $runSpeedOriginal ([uint32]0x004FA8E3) ([uint32]0x004FA91A)
if($runSpeedCodeOffset+$runSpeedCode.Length-gt$sectionSize){throw 'adapter da velocidade de corrida excedeu .mountkr'}
Set-Bytes $data ($sectionRaw+$runSpeedCodeOffset) $runSpeedCode
$runSpeedHook=[byte[]](0xE9)+[byte[]](Rel32 ($runSpeedHookVA+5) ([uint32]($sectionVA+$runSpeedCodeOffset)))+([byte[]](0x90)*5)
Set-Bytes $data $runSpeedHookOffset $runSpeedHook

[IO.File]::WriteAllBytes($Executable,$data)
$newHash=Get-Sha $Executable
if($expectedOutputHash -and $newHash-ne$expectedOutputHash){throw "saida divergente: $newHash"}
Write-Host "$($items.Count) montarias KR instaladas no renderer 7.48."
Write-Host "SHA-256 antes:  $actualHash";Write-Host "SHA-256 depois: $newHash";Write-Host "Backup: $backup"
