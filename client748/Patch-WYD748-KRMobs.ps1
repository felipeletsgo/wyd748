[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Extends BASE_DefineSkinMeshType in the exact 7.48 executable ABI. W2PP/KR
# supplies only the semantic class->skin mapping; no foreign address, frame or
# structure offset is transplanted.

$expectedInputHash = '79B66BFF4E8D31D0788D857AD6AF3DE7F95DC7A07C7256D134A6DD5708EAA4AE' # KRMOB_EXE_INPUT_HASH
$expectedOutputHash = 'B3F385739C232275FE08FACAE0152ECDFD97D16D111C43D25E7277869FF5422B' # KRMOB_EXE_OUTPUT_HASH
$sectionRaw = 0x001F5000
$sectionSize = 0x00002000
$sectionRVA = [uint32]0x00FE2000
$sectionVA = [uint32]0x013E2000
$hookOffset = 0x000FAEF8
$hookVA = [uint32]0x004FAEF8
$resumeLogVA = [uint32]0x004FAEFF
$resumeReturnVA = [uint32]0x004FAF0C
$original = [byte[]](0xC7,0x45,0xFC,0x00,0x00,0x00,0x00)

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant() }
function Assert-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Expected,[string]$Name) {
    for($i=0;$i-lt$Expected.Length;$i++){if($Data[$Offset+$i]-ne$Expected[$i]){throw ('{0}: byte inesperado em 0x{1:X}' -f $Name,($Offset+$i))}}
}
function Set-Bytes([byte[]]$Data,[int]$Offset,[byte[]]$Value){[Array]::Copy($Value,0,$Data,$Offset,$Value.Length)}
function Set-U16([byte[]]$Data,[int]$Offset,[uint16]$Value){Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value))}
function Set-U32([byte[]]$Data,[int]$Offset,[uint32]$Value){Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value))}
function Rel32([uint32]$FromNext,[uint32]$Target){return ,([BitConverter]::GetBytes([int32]([int64]$Target-[int64]$FromNext)))}
function New-Assembler([uint32]$BaseVA){[pscustomobject]@{BaseVA=$BaseVA;Bytes=[Collections.Generic.List[byte]]::new();Labels=@{};Fixups=[Collections.Generic.List[object]]::new()}}
function Emit($a,[byte[]]$b){$a.Bytes.AddRange($b)}
function Mark($a,[string]$n){$a.Labels[$n]=$a.Bytes.Count}
function Emit-Rel32($a,[byte[]]$op,[string]$label){Emit $a $op;$off=$a.Bytes.Count;Emit $a ([byte[]](0,0,0,0));$a.Fixups.Add([pscustomobject]@{Offset=$off;Label=$label})}
function Complete($a){
    $out=$a.Bytes.ToArray()
    foreach($f in $a.Fixups){$from=[uint32]($a.BaseVA+$f.Offset+4);$target=[uint32]($a.BaseVA+$a.Labels[$f.Label]);[Array]::Copy((Rel32 $from $target),0,$out,$f.Offset,4)}
    return ,$out
}
function Add-PESection([byte[]]$InputData){
    if($InputData.Length-ne$sectionRaw){throw ('WYD.exe possui tamanho inesperado antes de .mobkr: 0x{0:X}'-f$InputData.Length)}
    $pe=[BitConverter]::ToInt32($InputData,0x3C)
    if([Text.Encoding]::ASCII.GetString($InputData,$pe,4)-ne"PE`0`0"){throw 'cabecalho PE invalido'}
    $sections=[BitConverter]::ToUInt16($InputData,$pe+6);$optionalSize=[BitConverter]::ToUInt16($InputData,$pe+20)
    if($sections-ne8){throw "numero de secoes inesperado: $sections"}
    $sectionTable=$pe+24+$optionalSize;$newHeader=$sectionTable+$sections*40
    Assert-Bytes $InputData $newHeader ([byte[]](0)*40) 'cabecalho livre da nona secao'
    $lastHeader=$sectionTable+($sections-1)*40
    $lastName=[Text.Encoding]::ASCII.GetString($InputData,$lastHeader,8).Trim([char]0)
    $lastRawSize=[BitConverter]::ToUInt32($InputData,$lastHeader+16);$lastRaw=[BitConverter]::ToUInt32($InputData,$lastHeader+20)
    if($lastName-ne'.mountkr'-or$lastRaw+$lastRawSize-ne$sectionRaw){throw 'layout .mountkr anterior inesperado'}
    $expanded=New-Object byte[] ($sectionRaw+$sectionSize);[Array]::Copy($InputData,$expanded,$InputData.Length)
    Set-U16 $expanded ($pe+6) 9
    Set-U32 $expanded ($pe+24+56) ([uint32]0x00FE4000)
    Set-Bytes $expanded $newHeader ([Text.Encoding]::ASCII.GetBytes('.mobkr'))
    Set-U32 $expanded ($newHeader+8) ([uint32]$sectionSize);Set-U32 $expanded ($newHeader+12) $sectionRVA
    Set-U32 $expanded ($newHeader+16) ([uint32]$sectionSize);Set-U32 $expanded ($newHeader+20) ([uint32]$sectionRaw)
    Set-U32 $expanded ($newHeader+36) ([Convert]::ToUInt32('E0000060',16))
    return ,$expanded
}

if(-not(Test-Path -LiteralPath $Executable -PathType Leaf)){throw "WYD.exe ausente: $Executable"}
$actualHash=Get-Sha $Executable
if($expectedOutputHash-and$actualHash-eq$expectedOutputHash){Write-Host "Faces KR ja suportadas pelo renderer ($actualHash).";return}
if($VerifyOnly){throw "WYD.exe ainda nao contem o mapeamento das faces KR (SHA-256: $actualHash)"}
if($actualHash-ne$expectedInputHash){throw "WYD.exe fora da entrada suportada (SHA-256: $actualHash)"}

$input=[IO.File]::ReadAllBytes($Executable)
Assert-Bytes $input $hookOffset $original 'fallback de BASE_DefineSkinMeshType 7.48'
$backup=Join-Path (Split-Path -Parent $Executable) 'WYD.pre-mobs-kr.exe'
if(-not(Test-Path -LiteralPath $backup -PathType Leaf)){Copy-Item -LiteralPath $Executable -Destination $backup}
$data=Add-PESection $input

$mapping=[ordered]@{66=45;67=46;68=47;69=48;70=53;71=54;72=55;73=56;74=57}
$a=New-Assembler $sectionVA
Emit $a ([byte[]](0x8B,0x45,0x08))
foreach($entry in $mapping.GetEnumerator()){
    Emit $a ([byte[]](0x83,0xF8,[byte][int]$entry.Key))
    Emit-Rel32 $a ([byte[]](0x0F,0x84)) ("class$($entry.Key)")
}
Emit $a ([byte[]](0xC7,0x45,0xFC,0x00,0x00,0x00,0x00,0xE9))
Emit $a (Rel32 ([uint32]($sectionVA+$a.Bytes.Count+4)) $resumeLogVA)
foreach($entry in $mapping.GetEnumerator()){
    Mark $a ("class$($entry.Key)")
    Emit $a ([byte[]](0xC7,0x45,0xFC))
    Emit $a ([BitConverter]::GetBytes([int32]$entry.Value))
    Emit $a ([byte[]](0xE9))
    Emit $a (Rel32 ([uint32]($sectionVA+$a.Bytes.Count+4)) $resumeReturnVA)
}
$code=Complete $a
if($code.Length-gt$sectionSize){throw 'code cave .mobkr excedeu a secao'}
Set-Bytes $data $sectionRaw $code

$hook=[Collections.Generic.List[byte]]::new();$hook.Add(0xE9);$hook.AddRange((Rel32 ([uint32]($hookVA+5)) $sectionVA))
Set-Bytes $data $hookOffset $hook.ToArray()
[IO.File]::WriteAllBytes($Executable,$data)

$outputHash=Get-Sha $Executable
if($expectedOutputHash-and$outputHash-ne$expectedOutputHash){throw "saida divergente: $outputHash"}
Write-Host 'BASE_DefineSkinMeshType 7.48 estendido para classes KR 66..74.'
Write-Host "SHA-256: $outputHash"
