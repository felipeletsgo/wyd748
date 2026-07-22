param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Executavel 7.48 depois do Patch-WYD748.ps1 (mouse + cores de dano).
$supportedHash = '2AA1773AAC6A7EE186CD62B3B609004AD63D9A03E4B685CAE7E1B4AC2BB521EE'
$backup = Join-Path (Split-Path -Parent $Executable) 'WYD.pre-extended-stats.exe'

function Assert-Bytes([byte[]]$Data, [int]$Offset, [byte[]]$Expected, [string]$Name) {
    for ($i = 0; $i -lt $Expected.Length; $i++) {
        if ($Data[$Offset + $i] -ne $Expected[$i]) {
            throw ('{0}: byte inesperado em 0x{1:X}: esperado {2:X2}, encontrado {3:X2}' -f
                $Name, ($Offset + $i), $Expected[$i], $Data[$Offset + $i])
        }
    }
}

function Set-Bytes([byte[]]$Data, [int]$Offset, [byte[]]$Value) {
    [Array]::Copy($Value, 0, $Data, $Offset, $Value.Length)
}

function Set-WideRead([byte[]]$Data, [int]$Offset, [byte[]]$Expected,
    [byte[]]$Opcode, [uint32]$Address, [string]$Name) {
    Assert-Bytes $Data $Offset $Expected $Name
    $patch = New-Object 'System.Collections.Generic.List[byte]'
    $patch.AddRange($Opcode)
    $patch.AddRange([BitConverter]::GetBytes($Address))
    while ($patch.Count -lt $Expected.Length) {
        $patch.Add(0x90)
    }
    if ($patch.Count -ne $Expected.Length) {
        throw "${Name}: leitura wide nao cabe no trecho original."
    }
    Set-Bytes $Data $Offset $patch.ToArray()
}

function Add-Bytes($Code, [byte[]]$Bytes) {
    $Code.AddRange($Bytes)
}

function Add-U32($Code, [uint32]$Value) {
    $Code.AddRange([BitConverter]::GetBytes($Value))
}

function Add-Call($Code, [int]$BaseVA, [int]$TargetVA) {
    $instructionVA = $BaseVA + $Code.Count
    $Code.Add(0xE8)
    $Code.AddRange([BitConverter]::GetBytes([int]($TargetVA - ($instructionVA + 5))))
}

function Add-Jcc32($Code, [byte]$Condition) {
    Add-Bytes $Code ([byte[]](0x0F, $Condition))
    $displacementOffset = $Code.Count
    Add-U32 $Code 0
    return $displacementOffset
}

function Add-Jmp32($Code) {
    $Code.Add(0xE9)
    $displacementOffset = $Code.Count
    Add-U32 $Code 0
    return $displacementOffset
}

function Patch-Branch($Code, [int]$BaseVA, [int]$DisplacementOffset, [int]$TargetOffset) {
    $fromEndVA = $BaseVA + $DisplacementOffset + 4
    $targetVA = $BaseVA + $TargetOffset
    $bytes = [BitConverter]::GetBytes([int]($targetVA - $fromEndVA))
    for ($i = 0; $i -lt 4; $i++) {
        $Code[$DisplacementOffset + $i] = $bytes[$i]
    }
}

function Add-SetTextFromPacket($Code, [int]$BaseVA, [byte]$PacketOffset, [uint32]$SceneOffset) {
    # sprintf(buffer, "%d", *(uint32_t*)(packet + PacketOffset))
    Add-Bytes $Code ([byte[]](0x8B,0x45,0x08,0xFF,0xB0))
    Add-U32 $Code $PacketOffset
    Add-Bytes $Code ([byte[]](0x68))
    Add-U32 $Code 0x005BB828
    Add-Bytes $Code ([byte[]](0x8D,0x4D,0xBC,0x51))
    Add-Call $Code $BaseVA 0x0058F078
    Add-Bytes $Code ([byte[]](0x83,0xC4,0x0C))

    # scene->TextField->SetText(buffer, 0)
    Add-Bytes $Code ([byte[]](0xA1))
    Add-U32 $Code 0x0067CF38
    Add-Bytes $Code ([byte[]](0x8B,0x88))
    Add-U32 $Code $SceneOffset
    Add-Bytes $Code ([byte[]](0x85,0xC9))
    $skip = Add-Jcc32 $Code 0x84
    Add-Bytes $Code ([byte[]](0x6A,0x00,0x8D,0x55,0xBC,0x52,0x8B,0x01,0xFF,0x90))
    Add-U32 $Code 0x80
    Patch-Branch $Code $BaseVA $skip $Code.Count
}

$actualHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
if ($actualHash -ne $supportedHash) {
    # O patch e regeneravel: uma versao anterior com .xstat sempre volta para
    # a base pre-wide validada, sem empilhar secoes ou hooks sobre hooks.
    if ((Test-Path -LiteralPath $backup) -and
        ((Get-FileHash -LiteralPath $backup -Algorithm SHA256).Hash -eq $supportedHash)) {
        [IO.File]::WriteAllBytes($Executable, [IO.File]::ReadAllBytes($backup))
        $actualHash = $supportedHash
        Write-Host 'WYD.exe wide anterior substituido pela base pre-wide validada.'
    }
    else {
        throw "WYD.exe incompativel com o patch wide (SHA-256: $actualHash)."
    }
}

$data = [IO.File]::ReadAllBytes($Executable)

# ItemList usa EF_GRID (33) como indice nestas duas tabelas globais. A primeira
# informa largura/altura para os SGrid de inventario, cargo, trade e lojas; a
# segunda e a mascara usada por BASE_CanCargo. Normalizar somente uma delas
# deixaria celulas invisiveis ocupadas ou permitiria sobreposicao no cliente.
$itemGridXYOffset = 0x1BEC0C
$itemGridXYExpected = New-Object 'System.Collections.Generic.List[byte]'
foreach ($dimension in @(1,1, 1,2, 1,3, 1,4, 2,1, 2,2, 2,3, 2,4)) {
    $itemGridXYExpected.AddRange([BitConverter]::GetBytes([int]$dimension))
}
$itemGridXYOneSlot = New-Object 'System.Collections.Generic.List[byte]'
for ($i = 0; $i -lt 16; $i++) {
    $itemGridXYOneSlot.AddRange([BitConverter]::GetBytes([int]1))
}
Assert-Bytes $data $itemGridXYOffset $itemGridXYExpected.ToArray() 'tabela EF_GRID largura/altura'
Set-Bytes $data $itemGridXYOffset $itemGridXYOneSlot.ToArray()

$itemGridMaskOffset = 0x1BEC4C
$itemGridMaskExpected = [byte[]](
    1,0,0,0,0,0,0,0, 1,0,1,0,0,0,0,0,
    1,0,1,0,1,0,0,0, 1,0,1,0,1,0,1,0,
    1,1,0,0,0,0,0,0, 1,1,1,1,0,0,0,0,
    1,1,1,1,1,1,0,0, 1,1,1,1,1,1,1,1
)
$itemGridMaskOneSlot = New-Object byte[] 64
for ($grid = 0; $grid -lt 8; $grid++) {
    $itemGridMaskOneSlot[$grid * 8] = 1
}
Assert-Bytes $data $itemGridMaskOffset $itemGridMaskExpected 'tabela EF_GRID mascara'
Set-Bytes $data $itemGridMaskOffset $itemGridMaskOneSlot

# Cria uma secao RWX dedicada. O PE original possui cinco secoes e espaco livre
# suficiente na tabela de secoes; o novo raw comeca exatamente no EOF alinhado.
$pe = [BitConverter]::ToInt32($data, 0x3C)
$numberOfSectionsOffset = $pe + 6
$optionalHeader = $pe + 24
$sizeOfOptionalHeader = [BitConverter]::ToUInt16($data, $pe + 20)
$sectionTable = $optionalHeader + $sizeOfOptionalHeader
$newSectionHeader = $sectionTable + 5 * 40
$newRaw = $data.Length
$newRVA = 0x00FC0000
$newVA = 0x013C0000
$newRawSize = 0x2000
$wideVA = $newVA + 0x1F00
$wide = @{
    Magic = $wideVA + 0x00; Attack = $wideVA + 0x04; Defense = $wideVA + 0x08
    MaxHP = $wideVA + 0x0C; MaxMP = $wideVA + 0x10
    CurHP = $wideVA + 0x14; CurMP = $wideVA + 0x18
    Str = $wideVA + 0x1C; Int = $wideVA + 0x20
    Dex = $wideVA + 0x24; Con = $wideVA + 0x28
    Accuracy = $wideVA + 0x2C; Evasion = $wideVA + 0x30
    StatusPts = $wideVA + 0x34; MasterPts = $wideVA + 0x38
    SkillPts = $wideVA + 0x3C
    Level = $wideVA + 0x40
    Mastery0 = $wideVA + 0x44; Mastery1 = $wideVA + 0x48
    Mastery2 = $wideVA + 0x4C; Mastery3 = $wideVA + 0x50
    Critical = $wideVA + 0x54; Parry = $wideVA + 0x58
    Range = $wideVA + 0x5C
    ResistFire = $wideVA + 0x60; ResistIce = $wideVA + 0x64
    ResistHoly = $wideVA + 0x68; ResistThunder = $wideVA + 0x6C
    SaveMana = $wideVA + 0x70; MagicAmp = $wideVA + 0x74
    RegenHP = $wideVA + 0x78; RegenMP = $wideVA + 0x7C
    AttackRun = $wideVA + 0x80; Merchant = $wideVA + 0x84
    Version = $wideVA + 0x88
}
$wideFlagVA = $wideVA + 0x8C

if ([BitConverter]::ToUInt16($data, $numberOfSectionsOffset) -ne 5) {
    throw 'Quantidade de secoes inesperada.'
}
Assert-Bytes $data $newSectionHeader ([byte[]](0) * 40) 'espaco do header .xstat'

$expanded = New-Object byte[] ($data.Length + $newRawSize)
[Array]::Copy($data, $expanded, $data.Length)
$data = $expanded

$header = New-Object byte[] 40
[Text.Encoding]::ASCII.GetBytes('.xstat') | ForEach-Object -Begin {$i=0} -Process {$header[$i++]=$_}
Set-Bytes $header 8 ([BitConverter]::GetBytes([uint32]$newRawSize))
Set-Bytes $header 12 ([BitConverter]::GetBytes([uint32]$newRVA))
Set-Bytes $header 16 ([BitConverter]::GetBytes([uint32]$newRawSize))
Set-Bytes $header 20 ([BitConverter]::GetBytes([uint32]$newRaw))
Set-Bytes $header 36 ([BitConverter]::GetBytes([uint32]0xE0000060L)) # CODE|INIT_DATA|R|W|X
Set-Bytes $data $newSectionHeader $header
Set-Bytes $data $numberOfSectionsOffset ([BitConverter]::GetBytes([uint16]6))
Set-Bytes $data ($optionalHeader + 56) ([BitConverter]::GetBytes([uint32]0x00FC2000))

# Wrapper do 0x336: guarda a cauda wide em um sidecar permanente. A primeira
# sincronizacao local ainda chama o handler nativo; as seguintes retornam antes
# dele, pois ele reintroduz os WORDs truncados no TMHuman durante skills/regen.
$scoreVA = $newVA
$score = New-Object 'System.Collections.Generic.List[byte]'
Add-Bytes $score ([byte[]](0x55,0x8B,0xEC,0x83,0xEC,0x44,0x89,0x4D,0xFC))
Add-Bytes $score ([byte[]](0x8B,0x45,0x08,0x66,0x81,0x38,0xEC,0x00))
$scoreCallShort = Add-Jcc32 $score 0x82
Add-Bytes $score ([byte[]](0x81,0xB8)); Add-U32 $score 0xE8; Add-U32 $score 0x32435358
$scoreCallSignature = Add-Jcc32 $score 0x85
Add-Bytes $score ([byte[]](0x8B,0x15))
Add-U32 $score 0x0067CF38
Add-Bytes $score ([byte[]](0x85,0xD2))
$scoreCallNoScene = Add-Jcc32 $score 0x84
Add-Bytes $score ([byte[]](0x8B,0x4D,0xFC,0x3B,0x4A,0x4C))
$scoreCallNotLocal = Add-Jcc32 $score 0x85
Add-Bytes $score ([byte[]](0xC7,0x05))
Add-U32 $score ([uint32]$wideFlagVA)
Add-U32 $score 1
Add-Bytes $score ([byte[]](0x8B,0x45,0x08))
foreach ($copy in @(
    @(0x5C,$wide.Magic), @(0x60,$wide.Attack), @(0x64,$wide.Defense),
    @(0x68,$wide.MaxHP), @(0x6C,$wide.MaxMP),
    @(0x70,$wide.CurHP), @(0x74,$wide.CurMP),
    @(0x78,$wide.Str), @(0x7C,$wide.Int), @(0x80,$wide.Dex), @(0x84,$wide.Con),
    @(0x88,$wide.Accuracy), @(0x8C,$wide.Evasion),
    @(0x90,$wide.StatusPts), @(0x94,$wide.MasterPts), @(0x98,$wide.SkillPts),
    @(0x9C,$wide.Level),
    @(0xA0,$wide.Mastery0), @(0xA4,$wide.Mastery1),
    @(0xA8,$wide.Mastery2), @(0xAC,$wide.Mastery3),
    @(0xB0,$wide.Critical), @(0xB4,$wide.Parry), @(0xB8,$wide.Range),
    @(0xBC,$wide.ResistFire), @(0xC0,$wide.ResistIce),
    @(0xC4,$wide.ResistHoly), @(0xC8,$wide.ResistThunder),
    @(0xCC,$wide.SaveMana), @(0xD0,$wide.MagicAmp),
    @(0xD4,$wide.RegenHP), @(0xD8,$wide.RegenMP),
    @(0xDC,$wide.AttackRun), @(0xE0,$wide.Merchant),
    @(0xE4,$wide.Version)
)) {
    Add-Bytes $score ([byte[]](0x8B,0x90))
    Add-U32 $score ([uint32]$copy[0])
    Add-Bytes $score ([byte[]](0x89,0x15))
    Add-U32 $score ([uint32]$copy[1])
}
$scoreCallOffset = $score.Count
foreach ($branch in @($scoreCallShort,$scoreCallSignature,$scoreCallNoScene,$scoreCallNotLocal)) {
    Patch-Branch $score $scoreVA $branch $scoreCallOffset
}
Add-Bytes $score ([byte[]](0xFF,0x75,0x08,0x8B,0x4D,0xFC))
Add-Call $score $scoreVA 0x0052B97D
Add-Bytes $score ([byte[]](0xB8,0x01,0x00,0x00,0x00,0x8B,0xE5,0x5D,0xC2,0x04,0x00))

# Wrapper do 0x181: atualiza o sidecar local quando Size=36 possui a cauda
# uint32 e sempre chama o handler nativo com a projecao WORD.
$hpVA = $newVA + 0x800
$hp = New-Object 'System.Collections.Generic.List[byte]'
Add-Bytes $hp ([byte[]](0x55,0x8B,0xEC,0x51,0x89,0x4D,0xFC,0x8B,0x45,0x08))
Add-Bytes $hp ([byte[]](0x66,0x81,0x38,0x24,0x00))
$hpCall = Add-Jcc32 $hp 0x82
# O painel detalhado le o sidecar global, nao os Big* do TMHuman. Atualize-o
# somente quando este 0x181 pertence ao personagem local.
Add-Bytes $hp ([byte[]](0xA1)); Add-U32 $hp 0x0067CF38
Add-Bytes $hp ([byte[]](0x85,0xC0))
$hpNoScene = Add-Jcc32 $hp 0x84
Add-Bytes $hp ([byte[]](0x8B,0x4D,0xFC,0x3B,0x48,0x4C))
$hpNotLocal = Add-Jcc32 $hp 0x85
Add-Bytes $hp ([byte[]](0xC7,0x05)); Add-U32 $hp ([uint32]$wideFlagVA); Add-U32 $hp 1
Add-Bytes $hp ([byte[]](0x8B,0x45,0x08))
foreach ($copy in @(
    @(0x14,$wide.CurHP), @(0x18,$wide.CurMP),
    @(0x1C,$wide.MaxHP), @(0x20,$wide.MaxMP)
)) {
    Add-Bytes $hp ([byte[]](0x8B,0x50,[byte]$copy[0],0x89,0x15))
    Add-U32 $hp ([uint32]$copy[1])
}
$hpCallOffset = $hp.Count
foreach ($branch in @($hpCall,$hpNoScene,$hpNotLocal)) {
    Patch-Branch $hp $hpVA $branch $hpCallOffset
}
Add-Bytes $hp ([byte[]](0xFF,0x75,0x08,0x8B,0x4D,0xFC))
Add-Call $hp $hpVA 0x0052C5EB
Add-Bytes $hp ([byte[]](0xB8,0x01,0x00,0x00,0x00,0x8B,0xE5,0x5D,0xC2,0x04,0x00))

# Wrapper do 0x337: guarda os contadores wide antes do handler original, para
# que o UpdateScoreUI chamado por ele ja leia o sidecar atualizado.
$etcVA = $newVA + 0xB00
$etc = New-Object 'System.Collections.Generic.List[byte]'
Add-Bytes $etc ([byte[]](0x55,0x8B,0xEC,0x83,0xEC,0x44,0x89,0x4D,0xFC))
Add-Bytes $etc ([byte[]](0x8B,0x45,0x08,0x66,0x81,0x38,0x30,0x00))
$etcCall = Add-Jcc32 $etc 0x82
foreach ($copy in @(
    @(0x24,$wide.StatusPts), @(0x28,$wide.MasterPts), @(0x2C,$wide.SkillPts)
)) {
    Add-Bytes $etc ([byte[]](0x8B,0x90))
    Add-U32 $etc ([uint32]$copy[0])
    Add-Bytes $etc ([byte[]](0x89,0x15))
    Add-U32 $etc ([uint32]$copy[1])
}
$etcCallOffset = $etc.Count
Patch-Branch $etc $etcVA $etcCall $etcCallOffset
Add-Bytes $etc ([byte[]](0xFF,0x75,0x08,0x8B,0x4D,0xFC))
Add-Call $etc $etcVA 0x0052D93D
Add-Bytes $etc ([byte[]](0xB8,0x01,0x00,0x00,0x00,0x8B,0xE5,0x5D,0xC2,0x04,0x00))

# Helpers dos dois caminhos que montam o texto flutuante. Alem do 0x39D fisico,
# leem a cauda DMGX das skills 0x39E/0x36C. O prefixo WORD continua alimentando
# animacao/efeitos; apenas o inteiro entregue ao sprintf passa a ser uint32.
$damage1VA = $newVA + 0x900 # entrada: ECX=packet, EAX=indice
$damage1 = New-Object 'System.Collections.Generic.List[byte]'
Add-Bytes $damage1 ([byte[]](0x66,0x81,0x79,0x04,0x9D,0x03)) # opcode 39D
$d1NotOne = Add-Jcc32 $damage1 0x85
Add-Bytes $damage1 ([byte[]](0x66,0x83,0x39,0x34)) # size >= 52
$d1OneLegacy = Add-Jcc32 $damage1 0x82
Add-Bytes $damage1 ([byte[]](0x85,0xC0))
$d1OneIndex = Add-Jcc32 $damage1 0x85
Add-Bytes $damage1 ([byte[]](0x66,0x83,0x39,0x34))
$d1Physical = Add-Jcc32 $damage1 0x84
Add-Bytes $damage1 ([byte[]](0x66,0x83,0x39,0x3C)) # skill: 48+8+DWORD
$d1SkillLegacy = Add-Jcc32 $damage1 0x82
Add-Bytes $damage1 ([byte[]](0x8B,0x41,0x38,0xC3)) # skill damage @56
$d1PhysicalOffset = $damage1.Count
Patch-Branch $damage1 $damage1VA $d1Physical $d1PhysicalOffset
Add-Bytes $damage1 ([byte[]](0x8B,0x41,0x30,0xC3)) # melee damage @48

$d1TwoOffset = $damage1.Count
Patch-Branch $damage1 $damage1VA $d1NotOne $d1TwoOffset
Add-Bytes $damage1 ([byte[]](0x66,0x81,0x79,0x04,0x9E,0x03)) # opcode 39E
$d1NotTwo = Add-Jcc32 $damage1 0x85
Add-Bytes $damage1 ([byte[]](0x66,0x83,0x39,0x40)) # base52+tail+1 DWORD
$d1TwoLegacy = Add-Jcc32 $damage1 0x82
Add-Bytes $damage1 ([byte[]](0x8B,0x44,0x81,0x3C,0xC3)) # [packet+60+index*4]

$d1MultiOffset = $damage1.Count
Patch-Branch $damage1 $damage1VA $d1NotTwo $d1MultiOffset
Add-Bytes $damage1 ([byte[]](0x66,0x81,0x79,0x04,0x6C,0x03)) # opcode 36C
$d1NotMulti = Add-Jcc32 $damage1 0x85
Add-Bytes $damage1 ([byte[]](0x66,0x83,0x39,0x6C)) # base96+tail+1 DWORD
$d1MultiLegacy = Add-Jcc32 $damage1 0x82
Add-Bytes $damage1 ([byte[]](0x8B,0x44,0x81,0x68,0xC3)) # [packet+104+index*4]

$d1LegacyOffset = $damage1.Count
foreach ($branch in @($d1OneLegacy,$d1OneIndex,$d1SkillLegacy,$d1TwoLegacy,$d1NotMulti,$d1MultiLegacy)) {
    Patch-Branch $damage1 $damage1VA $branch $d1LegacyOffset
}
Add-Bytes $damage1 ([byte[]](0x0F,0xBF,0x44,0x81,0x2E,0xC3))

$damage2VA = $newVA + 0xA00 # entrada: EDX=packet, ECX=indice
$damage2 = New-Object 'System.Collections.Generic.List[byte]'
Add-Bytes $damage2 ([byte[]](0x66,0x81,0x7A,0x04,0x9D,0x03))
$d2NotOne = Add-Jcc32 $damage2 0x85
Add-Bytes $damage2 ([byte[]](0x66,0x83,0x3A,0x34))
$d2OneLegacy = Add-Jcc32 $damage2 0x82
Add-Bytes $damage2 ([byte[]](0x85,0xC9))
$d2OneIndex = Add-Jcc32 $damage2 0x85
Add-Bytes $damage2 ([byte[]](0x66,0x83,0x3A,0x34))
$d2Physical = Add-Jcc32 $damage2 0x84
Add-Bytes $damage2 ([byte[]](0x66,0x83,0x3A,0x3C))
$d2SkillLegacy = Add-Jcc32 $damage2 0x82
Add-Bytes $damage2 ([byte[]](0x8B,0x42,0x38,0xC3))
$d2PhysicalOffset = $damage2.Count
Patch-Branch $damage2 $damage2VA $d2Physical $d2PhysicalOffset
Add-Bytes $damage2 ([byte[]](0x8B,0x42,0x30,0xC3))

$d2TwoOffset = $damage2.Count
Patch-Branch $damage2 $damage2VA $d2NotOne $d2TwoOffset
Add-Bytes $damage2 ([byte[]](0x66,0x81,0x7A,0x04,0x9E,0x03))
$d2NotTwo = Add-Jcc32 $damage2 0x85
Add-Bytes $damage2 ([byte[]](0x66,0x83,0x3A,0x40))
$d2TwoLegacy = Add-Jcc32 $damage2 0x82
Add-Bytes $damage2 ([byte[]](0x8B,0x44,0x8A,0x3C,0xC3))

$d2MultiOffset = $damage2.Count
Patch-Branch $damage2 $damage2VA $d2NotTwo $d2MultiOffset
Add-Bytes $damage2 ([byte[]](0x66,0x81,0x7A,0x04,0x6C,0x03))
$d2NotMulti = Add-Jcc32 $damage2 0x85
Add-Bytes $damage2 ([byte[]](0x66,0x83,0x3A,0x6C))
$d2MultiLegacy = Add-Jcc32 $damage2 0x82
Add-Bytes $damage2 ([byte[]](0x8B,0x44,0x8A,0x68,0xC3))

$d2LegacyOffset = $damage2.Count
foreach ($branch in @($d2OneLegacy,$d2OneIndex,$d2SkillLegacy,$d2TwoLegacy,$d2NotMulti,$d2MultiLegacy)) {
    Patch-Branch $damage2 $damage2VA $branch $d2LegacyOffset
}
Add-Bytes $damage2 ([byte[]](0x0F,0xBF,0x44,0x8A,0x2E,0xC3))

# UpdateScoreUI calcula m_pSkillDam com BASE_GetSkillDamage local (shorts).
# Para personagem wide, substitui somente o inteiro formatado pelo MATK
# autoritativo recebido no 0x336; os controles e cores originais permanecem.
$magicUIVA = $newVA + 0xC60
$magicUI = New-Object 'System.Collections.Generic.List[byte]'
Add-Bytes $magicUI ([byte[]](0x83,0x3D)); Add-U32 $magicUI ([uint32]$wideFlagVA)
Add-Bytes $magicUI ([byte[]](0x00,0x74,0x06,0xA1)); Add-U32 $magicUI ([uint32]$wide.Magic)
Add-Bytes $magicUI ([byte[]](0xC3,0x8B,0x85,0xA8,0xFB,0xFF,0xFF,0xC3))

if ($score.Count -gt 0x800) { throw "Wrapper score excedeu 0x800 bytes: $($score.Count)" }
if ($hp.Count -gt 0x100) { throw "Wrapper HP invadiu o helper de dano: $($hp.Count) bytes" }
if ($etc.Count -gt 0x400) { throw "Wrapper ETC excedeu 0x400 bytes: $($etc.Count)" }
Set-Bytes $data $newRaw $score.ToArray()
Set-Bytes $data ($newRaw + 0x800) $hp.ToArray()
Set-Bytes $data ($newRaw + 0xB00) $etc.ToArray()
if ($damage1.Count -gt 0x100 -or $damage2.Count -gt 0x100) {
    throw "Helper de dano excedeu a area reservada."
}
Set-Bytes $data ($newRaw + 0x900) $damage1.ToArray()
Set-Bytes $data ($newRaw + 0xA00) $damage2.ToArray()
Set-Bytes $data ($newRaw + 0xC60) $magicUI.ToArray()

# Redireciona somente os CALLs dos handlers TMHuman.
Assert-Bytes $data 0x12ECD7 ([byte[]](0xE8,0xA1,0xCC,0xFF,0xFF)) 'call 0x336'
Assert-Bytes $data 0x12ECF9 ([byte[]](0xE8,0xED,0xD8,0xFF,0xFF)) 'call 0x181'
Assert-Bytes $data 0x12ED81 ([byte[]](0xE8,0xB7,0xEB,0xFF,0xFF)) 'call 0x337'
foreach ($site in @(
    @(0x12ECD7,0x0052ECD7,$scoreVA),
    @(0x12ECF9,0x0052ECF9,$hpVA),
    @(0x12ED81,0x0052ED81,$etcVA)
)) {
    $call = New-Object 'System.Collections.Generic.List[byte]'
    Add-Call $call ([int]$site[1]) ([int]$site[2])
    Set-Bytes $data ([int]$site[0]) $call.ToArray()
}

# O valor calculado da skill fica em [EBP-0x458] e e empilhado para sprintf
# neste ponto. O helper retorna MATK wide ou o valor original.
Assert-Bytes $data 0x45C73 ([byte[]](0x8B,0x85,0xA8,0xFB,0xFF,0xFF,0x50)) 'UI MATK'
$magicPatch = New-Object 'System.Collections.Generic.List[byte]'
Add-Call $magicPatch 0x00445C73 $magicUIVA
Add-Bytes $magicPatch ([byte[]](0x50,0x90))
Set-Bytes $data 0x45C73 $magicPatch.ToArray()

# O SListBox 7.48 anexa os itens na ordem correta, mas desenha uma lista ainda
# incompleta a partir da primeira linha (de cima para baixo). Para o chat
# principal, desloque o indice visual pelas linhas ainda vazias: a mensagem
# nova nasce embaixo e as anteriores sobem. Outras listas permanecem intactas.
$chatAlignVA = $newVA + 0xC80
$chatAlignRaw = $newRaw + 0xC80
$chatAlign = New-Object 'System.Collections.Generic.List[byte]'
Add-Bytes $chatAlign ([byte[]](0x51,0x52)) # preserva ecx/edx
Add-Bytes $chatAlign ([byte[]](0x8B,0x45,0xE4)) # eax = SListBox (caller [ebp-1C])
Add-Bytes $chatAlign ([byte[]](0x8B,0x15)); Add-U32 $chatAlign 0x0067CF38
Add-Bytes $chatAlign ([byte[]](0x85,0xD2))
$chatNoScene = Add-Jcc32 $chatAlign 0x84
Add-Bytes $chatAlign ([byte[]](0x3B,0x82,0xE8,0x7A,0x02,0x00)) # scene->m_pChatList
$chatNotMainList = Add-Jcc32 $chatAlign 0x85
Add-Bytes $chatAlign ([byte[]](0x8B,0x88,0xF4,0x01,0x00,0x00)) # visibleCount
Add-Bytes $chatAlign ([byte[]](0x2B,0x4D,0xF8)) # - currentVisibleCount
$chatAlreadyFull = Add-Jcc32 $chatAlign 0x8E # JLE
Add-Bytes $chatAlign ([byte[]](0x03,0x4D,0xF4)) # + nIndex
$chatLoadFloat = Add-Jmp32 $chatAlign
$chatLegacyOffset = $chatAlign.Count
foreach ($branch in @($chatNoScene,$chatNotMainList,$chatAlreadyFull)) {
    Patch-Branch $chatAlign $chatAlignVA $branch $chatLegacyOffset
}
Add-Bytes $chatAlign ([byte[]](0x8B,0x4D,0xF4)) # nIndex original
$chatLoadFloatOffset = $chatAlign.Count
Patch-Branch $chatAlign $chatAlignVA $chatLoadFloat $chatLoadFloatOffset
Add-Bytes $chatAlign ([byte[]](0x51,0xDB,0x04,0x24,0x83,0xC4,0x04)) # fild adjusted index
Add-Bytes $chatAlign ([byte[]](0x5A,0x59,0x8B,0x45,0xE4,0xC3))
Set-Bytes $data $chatAlignRaw $chatAlign.ToArray()

# SListBox::FrameMove2: substitui `fild [ebp-0C]; mov eax,[ebp-1C]` pelo helper.
$chatFrameOffset = 0x9930
$chatFrameVA = 0x00409930
Assert-Bytes $data $chatFrameOffset ([byte[]](0xDB,0x45,0xF4,0x8B,0x45,0xE4)) 'alinhamento inferior do chat'
$chatFramePatch = New-Object 'System.Collections.Generic.List[byte]'
Add-Call $chatFramePatch $chatFrameVA $chatAlignVA
$chatFramePatch.Add(0x90)
Set-Bytes $data $chatFrameOffset $chatFramePatch.ToArray()

# Os quatro sprintf do HUD leem diretamente o sidecar. Nao reutilize campos
# vizinhos do TMHuman: +0x220 e m_fMaxSpeed, +0x228 e um float de escala.
$numericSites = @(
    @(0x12C8C5, [byte[]](0x0F,0xBF,0x91,0x50,0x04,0x00,0x00), $wide.CurHP),
    @(0x12C91C, [byte[]](0x0F,0xBF,0x91,0x4C,0x04,0x00,0x00), $wide.MaxHP),
    @(0x12C973, [byte[]](0x0F,0xBF,0x91,0x52,0x04,0x00,0x00), $wide.CurMP),
    @(0x12C9CA, [byte[]](0x0F,0xBF,0x91,0x4E,0x04,0x00,0x00), $wide.MaxMP)
)
foreach ($site in $numericSites) {
    Assert-Bytes $data ([int]$site[0]) ([byte[]]$site[1]) ('HUD wide 0x{0:X}' -f $site[0])
    $replacement = New-Object 'System.Collections.Generic.List[byte]'
    Add-Bytes $replacement ([byte[]](0x8B,0x15)) # mov edx,[absolute]
    Add-U32 $replacement ([uint32]$site[2])
    $replacement.Add(0x90)
    Set-Bytes $data ([int]$site[0]) $replacement.ToArray()
}

# Uma segunda rotina de atualizacao do painel (0x0052317F..0x00523266)
# reformata os mesmos quatro valores depois de eventos de combate/skill. Sem
# este redirecionamento ela sobrescreve os textos wide com TMHuman+44C..452,
# mesmo que o 0x181 e seu sidecar estejam corretos.
$secondaryNumericSites = @(
    @(0x12317F, [byte[]](0x0F,0xBF,0x88,0x50,0x04,0x00,0x00), [byte[]](0x8B,0x0D), $wide.CurHP, 'painel secundario HP'),
    @(0x1231CA, [byte[]](0x0F,0xBF,0x91,0x4C,0x04,0x00,0x00), [byte[]](0x8B,0x15), $wide.MaxHP, 'painel secundario MaxHP'),
    @(0x123215, [byte[]](0x0F,0xBF,0x82,0x52,0x04,0x00,0x00), [byte[]](0xA1),      $wide.CurMP, 'painel secundario MP'),
    @(0x123266, [byte[]](0x0F,0xBF,0x91,0x4E,0x04,0x00,0x00), [byte[]](0x8B,0x15), $wide.MaxMP, 'painel secundario MaxMP')
)
foreach ($site in $secondaryNumericSites) {
    Assert-Bytes $data ([int]$site[0]) ([byte[]]$site[1]) ([string]$site[4])
    $replacement = New-Object 'System.Collections.Generic.List[byte]'
    Add-Bytes $replacement ([byte[]]$site[2])
    Add-U32 $replacement ([uint32]$site[3])
    while ($replacement.Count -lt ([byte[]]$site[1]).Length) {
        $replacement.Add(0x90)
    }
    Set-Bytes $data ([int]$site[0]) $replacement.ToArray()
}

# UpdateScoreUI roda continuamente e possui dois controles de barra para cada
# recurso. O espelho local apontado por [ebp-1F8] usa +48/+4A para MaxHP/MaxMP
# e +4C/+4E para CurHP/CurMP; os valores interpolados ficam no objeto da cena
# em +26E80/+26E82. Todos precisam usar o mesmo sidecar, ou o HUD mistura um
# valor wide com outro legado. Cada helper conserva a leitura WORD original
# para personagens que nao usam o protocolo estendido.
$uiWideSites = @(
    @(0x1001FE,[byte[]](0x8B,0x85,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0x50,0x04,0x00,0x00),[byte[]](0x8B,0x0D),$wide.CurHP,'painel local texto HP atual'),
    @(0x100237,[byte[]](0x8B,0x85,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0x4C,0x04,0x00,0x00),[byte[]](0x8B,0x0D),$wide.MaxHP,'painel local texto HP max'),
    @(0x100270,[byte[]](0x8B,0x85,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0x52,0x04,0x00,0x00),[byte[]](0x8B,0x0D),$wide.CurMP,'painel local texto MP atual'),
    @(0x1002AF,[byte[]](0x8B,0x85,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0x4E,0x04,0x00,0x00),[byte[]](0x8B,0x0D),$wide.MaxMP,'painel local texto MP max'),
    @(0x1002EE,[byte[]](0x8B,0x85,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0x4C,0x04,0x00,0x00),[byte[]](0x8B,0x0D),$wide.MaxHP,'painel local barra HP max'),
    @(0x10030A,[byte[]](0x8B,0x8D,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x91,0x50,0x04,0x00,0x00),[byte[]](0x8B,0x15),$wide.CurHP,'painel local barra HP atual'),
    @(0x10032C,[byte[]](0x8B,0x85,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0x4E,0x04,0x00,0x00),[byte[]](0x8B,0x0D),$wide.MaxMP,'painel local barra MP max'),
    @(0x100348,[byte[]](0x8B,0x8D,0x4C,0xFE,0xFF,0xFF,0x0F,0xBF,0x91,0x52,0x04,0x00,0x00),[byte[]](0x8B,0x15),$wide.CurMP,'painel local barra MP atual'),
    @(0x43255,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x48,0x48),[byte[]](0x8B,0x0D),$wide.MaxHP,'barra HP max primaria'),
    @(0x4328F,[byte[]](0x8B,0x95,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x42,0x4C),[byte[]](0xA1),$wide.CurHP,'barra HP atual primaria'),
    @(0x432C9,[byte[]](0x8B,0x8D,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x51,0x48),[byte[]](0x8B,0x15),$wide.MaxHP,'barra HP max interpolada'),
    @(0x43303,[byte[]](0x8B,0x8D,0x50,0xFA,0xFF,0xFF,0x0F,0xBF,0x91,0x80,0x6E,0x02,0x00),[byte[]](0x8B,0x15),$wide.CurHP,'barra HP atual interpolada'),
    @(0x43331,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x48,0x4C),[byte[]](0x8B,0x0D),$wide.CurHP,'UI HP'),
    @(0x433CF,[byte[]](0x8B,0x8D,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x51,0x4A),[byte[]](0x8B,0x15),$wide.MaxMP,'barra MP max primaria'),
    @(0x43409,[byte[]](0x8B,0x8D,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x51,0x4E),[byte[]](0x8B,0x15),$wide.CurMP,'barra MP atual primaria'),
    @(0x43443,[byte[]](0x8B,0x8D,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x51,0x4A),[byte[]](0x8B,0x15),$wide.MaxMP,'barra MP max interpolada'),
    @(0x4347D,[byte[]](0x8B,0x8D,0x50,0xFA,0xFF,0xFF,0x0F,0xBF,0x91,0x82,0x6E,0x02,0x00),[byte[]](0x8B,0x15),$wide.CurMP,'barra MP atual interpolada'),
    @(0x434AB,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x48,0x4E),[byte[]](0x8B,0x0D),$wide.CurMP,'UI MP'),
    @(0x43FAE,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0xE0,0x02,0x00,0x00),[byte[]](0x8B,0x0D),$wide.StatusPts,'UI status points'),
    @(0x44008,[byte[]](0x8B,0x8D,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x91,0xE2,0x02,0x00,0x00),[byte[]](0x8B,0x15),$wide.MasterPts,'UI mastery points'),
    @(0x44062,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x88,0xE4,0x02,0x00,0x00),[byte[]](0x8B,0x0D),$wide.SkillPts,'UI skill points'),
    @(0x444B3,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x48,0x50),[byte[]](0x8B,0x0D),$wide.Str,'UI STR'),
    @(0x4450A,[byte[]](0x8B,0x8D,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x51,0x52),[byte[]](0x8B,0x15),$wide.Int,'UI INT'),
    @(0x44561,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x48,0x54),[byte[]](0x8B,0x0D),$wide.Dex,'UI DEX'),
    @(0x445B8,[byte[]](0x8B,0x8D,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x51,0x56),[byte[]](0x8B,0x15),$wide.Con,'UI CON'),
    @(0x45A5F,[byte[]](0x8B,0x95,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x42,0x44),[byte[]](0xA1),$wide.Attack,'UI ataque'),
    @(0x45F81,[byte[]](0x8B,0x85,0x08,0xFE,0xFF,0xFF,0x0F,0xBF,0x48,0x42),[byte[]](0x8B,0x0D),$wide.Defense,'UI defesa')
)
$uiHelperVA = $chatAlignVA + $chatAlign.Count
$uiHelperRaw = $chatAlignRaw + $chatAlign.Count
foreach ($site in $uiWideSites) {
    $offset = [int]$site[0]
    $expected = [byte[]]$site[1]
    $wideOpcode = [byte[]]$site[2]
    $wideAddress = [uint32]$site[3]
    $name = [string]$site[4]
    Assert-Bytes $data $offset $expected $name

    $helper = New-Object 'System.Collections.Generic.List[byte]'
    Add-Bytes $helper ([byte[]](0x83,0x3D))
    Add-U32 $helper ([uint32]$wideFlagVA)
    $helper.Add(0x00)
    Add-Bytes $helper ([byte[]](0x74,0x00))
    $shortJumpByte = $helper.Count - 1
    Add-Bytes $helper $wideOpcode
    Add-U32 $helper $wideAddress
    $helper.Add(0xC3)
    $legacyOffset = $helper.Count
    $shortDistance = $legacyOffset - ($shortJumpByte + 1)
    if ($shortDistance -lt -128 -or $shortDistance -gt 127) {
        throw "${name}: salto curto fora do alcance."
    }
    $helper[$shortJumpByte] = [byte]$shortDistance

    Add-Bytes $helper $expected
    $helper.Add(0xC3)

    Set-Bytes $data $uiHelperRaw $helper.ToArray()
    $patch = New-Object 'System.Collections.Generic.List[byte]'
    Add-Call $patch (0x00400000 + $offset) $uiHelperVA
    while ($patch.Count -lt $expected.Length) {
        $patch.Add(0x90)
    }
    Set-Bytes $data $offset $patch.ToArray()
    $uiHelperVA += $helper.Count
    $uiHelperRaw += $helper.Count
}
if ($uiHelperVA -gt $wideVA) {
    throw ('Helpers de UI invadiram o sidecar: fim=0x{0:X}, wide=0x{1:X}' -f $uiHelperVA,$wideVA)
}

# Substitui somente os MOVSX que alimentam o texto de dano. O restante dos dois
# caminhos permanece byte a byte igual ao client original.
foreach ($site in @(
    @(0x8F5EB,0x0048F5EB,$damage1VA,[byte[]](0x0F,0xBF,0x44,0x81,0x2E)),
    @(0x9158A,0x0049158A,$damage2VA,[byte[]](0x0F,0xBF,0x44,0x8A,0x2E))
)) {
    Assert-Bytes $data ([int]$site[0]) ([byte[]]$site[3]) ('dano wide 0x{0:X}' -f $site[1])
    $call = New-Object 'System.Collections.Generic.List[byte]'
    Add-Call $call ([int]$site[1]) ([int]$site[2])
    Set-Bytes $data ([int]$site[0]) $call.ToArray()
}

if (-not (Test-Path -LiteralPath $backup)) {
    [IO.File]::WriteAllBytes($backup, [IO.File]::ReadAllBytes($Executable))
}
[IO.File]::WriteAllBytes($Executable, $data)
Write-Host "WYD.exe com extensao de stats 32-bit."
Write-Host "Backup anterior: $backup"
Write-Host "EF_GRID: todos os itens ocupam uma unica celula no inventario e cargo."
Write-Host "Chat: mensagens novas alinhadas embaixo; historico sobe com novas mensagens."
Write-Host "Secao: .xstat RVA 0x00FC0000; score=$($score.Count) bytes; hp=$($hp.Count) bytes; etc=$($etc.Count) bytes; dano uint32 ativo."
