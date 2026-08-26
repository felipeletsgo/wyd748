[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Os skeletons KR importados para mobs usam o sistema de eixos moderno do
# renderer posterior. O TMSkinMesh do 7.48 aplica a rotacao legada a qualquer
# tipo, deixando Anubis/Berial/tteok deitados. Este elo altera somente a
# decisao de orientacao no executavel que ja contem .mountkr/.mobkr:
# tipos de skeleton 45..57 usam yaw+90/pitch/roll; os demais permanecem no
# caminho nativo. O tipo 47 recebe ainda a escala 1.5 observada na source.
#
# O patch e separado dos elos de montaria e de classe visual para que cada
# mudanca continue reversivel, com SHA de entrada/saida e asserts de bytes.

$expectedInputHash = 'B3F385739C232275FE08FACAE0152ECDFD97D16D111C43D25E7277869FF5422B'
$expectedOutputHash = '8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593' # KRMOB_POSE_OUTPUT_HASH
$sectionRaw = 0x001E5000
$sectionSize = 0x00010000
$sectionVA = [uint32]0x013D2000
$renderOrientationCodeOffset = 0x3000
$renderOrientationHookOffset = 0x0BE395
$renderOrientationHookVA = [uint32]0x004BE395

function Get-Sha([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
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
function Rel32([uint32]$FromNextInstruction,[uint32]$Target) {
    return ,([BitConverter]::GetBytes([int32]([int64]$Target-[int64]$FromNextInstruction)))
}
function New-Assembler([uint32]$BaseVA) {
    [pscustomobject]@{
        BaseVA = $BaseVA
        Bytes = [Collections.Generic.List[byte]]::new()
        Labels = @{}
        Fixups = [Collections.Generic.List[object]]::new()
    }
}
function Emit($a,[byte[]]$b) { $a.Bytes.AddRange($b) }
function Mark($a,[string]$Name) {
    if($a.Labels.ContainsKey($Name)){ throw "rotulo duplicado: $Name" }
    $a.Labels[$Name] = $a.Bytes.Count
}
function Emit-Rel32($a,[byte[]]$Opcode,[string]$Label) {
    Emit $a $Opcode
    $offset = $a.Bytes.Count
    Emit $a ([byte[]](0,0,0,0))
    $a.Fixups.Add([pscustomobject]@{ Offset=$offset; Label=$Label })
}
function Complete($a) {
    $out = $a.Bytes.ToArray()
    foreach($fixup in $a.Fixups) {
        if(-not $a.Labels.ContainsKey($fixup.Label)){ throw "rotulo ausente: $($fixup.Label)" }
        $from = [uint32]($a.BaseVA+$fixup.Offset+4)
        $target = [uint32]($a.BaseVA+$a.Labels[$fixup.Label])
        [Array]::Copy((Rel32 $from $target),0,$out,$fixup.Offset,4)
    }
    return ,$out
}

if(-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
    throw "WYD.exe ausente: $Executable"
}
$actualHash = Get-Sha $Executable
if($expectedOutputHash -and $actualHash -eq $expectedOutputHash) {
    Write-Host "Orientacao das faces KR ja instalada ($actualHash)."
    return
}
if($VerifyOnly) {
    throw "WYD.exe ainda nao contem a orientacao moderna das faces KR (SHA-256: $actualHash)"
}
if($actualHash -ne $expectedInputHash) {
    throw "WYD.exe fora da entrada suportada (SHA-256: $actualHash)"
}

$inputData = [IO.File]::ReadAllBytes($Executable)
if($inputData.Length -lt ($sectionRaw+$sectionSize)) {
    throw 'WYD.exe nao possui a secao .mountkr esperada'
}
# O elo anterior deixou o selector 48..51 neste ponto. A assinatura protege
# contra aplicar este patch em um executavel diferente ou em uma versao antiga.
Assert-Bytes $inputData ($sectionRaw+$renderOrientationCodeOffset) ([byte[]](
    0x8B,0x85,0xEC,0xFD,0xFF,0xFF,
    0x8B,0x08,0x83,0xF9,0x30)) 'selector de orientacao anterior'
Assert-Bytes $inputData $renderOrientationHookOffset ([byte[]](0xE9)) 'hook de orientacao anterior'

$codeBaseVA = [uint32]($sectionVA+$renderOrientationCodeOffset)
$o = New-Assembler $codeBaseVA

# TMSkinMesh object = [ebp-0x214], m_nBoneAniIndex = [object+0].
Emit $o ([byte[]](
    0x8B,0x85,0xEC,0xFD,0xFF,0xFF,
    0x8B,0x08,
    0x83,0xF9,0x2D))
Emit-Rel32 $o ([byte[]](0x0F,0x8C)) 'legacy'
Emit $o ([byte[]](0x83,0xF9,0x39))
Emit-Rel32 $o ([byte[]](0x0F,0x8F)) 'legacy'

# Skeleton 45 preserva a matriz identidade em tres animacoes nativas; a rotina
# original ja inicializou [ebp-0x140] antes do hook, portanto basta saltar o
# construtor de rotacao nesses indices.
Emit $o ([byte[]](0x83,0xF9,0x2D))
Emit-Rel32 $o ([byte[]](0x0F,0x85)) 'check47'
Emit $o ([byte[]](0x8B,0x90,0xD0,0x02,0x00,0x00,0x83,0xFA,0x07))
Emit-Rel32 $o ([byte[]](0x0F,0x8C)) 'check47'
Emit $o ([byte[]](0x83,0xFA,0x09))
Emit-Rel32 $o ([byte[]](0x0F,0x8E)) 'return'

Mark $o 'check47'
# Skeleton 47 (tteok) usa escala 1.5 no renderer moderno.
Emit $o ([byte[]](0x83,0xF9,0x2F))
Emit-Rel32 $o ([byte[]](0x0F,0x85)) 'modern'
Emit $o ([byte[]](
    0xC7,0x40,0x24,0x00,0x00,0xC0,0x3F,
    0xC7,0x40,0x28,0x00,0x00,0xC0,0x3F,
    0xC7,0x40,0x2C,0x00,0x00,0xC0,0x3F))

Mark $o 'modern'
# Ramo moderno: roll=angle.z, pitch=angle.x, yaw=angle.y+90 graus.
Emit $o ([byte[]](
    0x8B,0x50,0x20,0x52,
    0x8B,0x50,0x18,0x52,
    0xD9,0x40,0x1C,
    0xD8,0x05,0x80,0x43,0x5A,0x00,
    0x51,0xD9,0x1C,0x24,
    0x8D,0x85,0xC0,0xFE,0xFF,0xFF,0x50,
    0xE8))
$modernCallNext = [uint32]($o.BaseVA+$o.Bytes.Count+4)
Emit $o (Rel32 $modernCallNext ([uint32]0x0056284F))
Emit-Rel32 $o ([byte[]](0xE9)) 'return'

Mark $o 'legacy'
# Ramo original 7.48: roll=angle.z, pitch=angle.x-90, yaw=angle.y-90.
Emit $o ([byte[]](
    0x8B,0x85,0xEC,0xFD,0xFF,0xFF,
    0x8B,0x50,0x20,0x52,
    0xD9,0x40,0x18,
    0xD8,0x25,0x80,0x43,0x5A,0x00,
    0x51,0xD9,0x1C,0x24,
    0xD9,0x40,0x1C,
    0xD8,0x25,0x80,0x43,0x5A,0x00,
    0x51,0xD9,0x1C,0x24,
    0x8D,0x85,0xC0,0xFE,0xFF,0xFF,0x50,
    0xE8))
$legacyCallNext = [uint32]($o.BaseVA+$o.Bytes.Count+4)
Emit $o (Rel32 $legacyCallNext ([uint32]0x0056284F))

Mark $o 'return'
Emit $o ([byte[]](0xE9))
$returnNext = [uint32]($o.BaseVA+$o.Bytes.Count+4)
Emit $o (Rel32 $returnNext ([uint32]0x004BE3CB))

$code = Complete $o
if($code.Length -gt 0x1000) {
    throw ('adapter de orientacao das faces excedeu a area reservada: 0x{0:X} bytes' -f $code.Length)
}
Set-Bytes $inputData ($sectionRaw+$renderOrientationCodeOffset) $code
$hook = [byte[]](0xE9)+[byte[]](Rel32 ($renderOrientationHookVA+5) ([uint32]$codeBaseVA))+([byte[]](0x90)*2)
Set-Bytes $inputData $renderOrientationHookOffset $hook

$backup = Join-Path (Split-Path -Parent $Executable) 'WYD.pre-mob-poses.exe'
if(-not (Test-Path -LiteralPath $backup -PathType Leaf)) {
    Copy-Item -LiteralPath $Executable -Destination $backup
} elseif((Get-Sha $backup) -ne $expectedInputHash) {
    throw "backup de orientacao das faces divergente: $backup"
}
[IO.File]::WriteAllBytes($Executable,$inputData)
$finalHash = Get-Sha $Executable
Write-Host "Orientacao moderna das faces KR aplicada. SHA-256: $finalHash"
if($expectedOutputHash -and $finalHash -ne $expectedOutputHash) {
    throw "SHA-256 final inesperado: $finalHash"
}
