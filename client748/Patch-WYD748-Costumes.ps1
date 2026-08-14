[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Costumes-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Ports the current WYD KR costume selector and renderer data to the 7.48
# executable without importing any newer-version struct or offset. The two
# hooks below are native 7.48 call sites; the source-derived tables contain
# only costumes whose complete mesh/texture dependency set is present locally.
#
# Class/body contract:
#   EF_CLASS=5  -> TK/BM (male skeleton)
#   EF_CLASS=10 -> FM/HT (female skeleton)
#   EF_CLASS=15 -> renderer chooses the skeleton from the current body

$expectedInputHash = '9762B1AC6EFB4AB3C800877DE1DA048DD43EA407FCEEA945C755DF6986607F18'
$expectedOutputHash = '4A2AA37228A720ED389F5AC8A5978329855932B93E54FA0501B51A3A23316DEF'
$sectionRVA = [uint32]0x00FC2000
$sectionRaw = 0x001D5000
$sectionSize = 0x00010000
$sectionVA = [uint32]0x013C2000
$selectorOffset = 0x0000
$rendererOffset = 0x0100
$itemTableOffset = 0x0200
$admissionOffset = 0x0680
$rendererTableOffset = 0x0800
$selectiveCullOffset = 0x4100
$stringsOffset = 0x5000
$rendererTypeCount = 303
$partCount = 6

function Get-Sha([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

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

function Set-U16([byte[]]$Data, [int]$Offset, [uint16]$Value) {
    Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value))
}

function Set-U32([byte[]]$Data, [int]$Offset, [uint32]$Value) {
    Set-Bytes $Data $Offset ([BitConverter]::GetBytes($Value))
}

function Rel32([uint32]$FromNextInstruction, [uint32]$Target) {
    return ,([BitConverter]::GetBytes([int32]([int64]$Target - [int64]$FromNextInstruction)))
}

function New-Assembler([uint32]$BaseVA) {
    return [pscustomobject]@{
        BaseVA = $BaseVA
        Bytes = [Collections.Generic.List[byte]]::new()
        Labels = @{}
        Fixups = [Collections.Generic.List[object]]::new()
    }
}

function Emit($Assembler, [byte[]]$Bytes) {
    $Assembler.Bytes.AddRange($Bytes)
}

function Mark($Assembler, [string]$Name) {
    if ($Assembler.Labels.ContainsKey($Name)) { throw "rotulo duplicado: $Name" }
    $Assembler.Labels[$Name] = $Assembler.Bytes.Count
}

function Emit-Rel32($Assembler, [byte[]]$Opcode, [string]$Label) {
    Emit $Assembler $Opcode
    $offset = $Assembler.Bytes.Count
    Emit $Assembler ([byte[]](0, 0, 0, 0))
    $Assembler.Fixups.Add([pscustomobject]@{ Offset = $offset; Label = $Label })
}

function Complete($Assembler) {
    $result = $Assembler.Bytes.ToArray()
    foreach ($fixup in $Assembler.Fixups) {
        if (-not $Assembler.Labels.ContainsKey($fixup.Label)) { throw "rotulo ausente: $($fixup.Label)" }
        $from = [uint32]($Assembler.BaseVA + $fixup.Offset + 4)
        $target = [uint32]($Assembler.BaseVA + $Assembler.Labels[$fixup.Label])
        [Array]::Copy((Rel32 $from $target), 0, $result, $fixup.Offset, 4)
    }
    return ,$result
}

function Add-PESection([byte[]]$InputData) {
    if ($InputData.Length -ne $sectionRaw) {
        throw ('WYD.exe possui tamanho inesperado antes de .costkr: 0x{0:X}' -f $InputData.Length)
    }
    $pe = [BitConverter]::ToInt32($InputData, 0x3C)
    if ([Text.Encoding]::ASCII.GetString($InputData, $pe, 4) -ne "PE`0`0") { throw 'cabecalho PE invalido' }
    $sections = [BitConverter]::ToUInt16($InputData, $pe + 6)
    $optionalSize = [BitConverter]::ToUInt16($InputData, $pe + 20)
    if ($sections -ne 6) { throw "numero de secoes inesperado: $sections" }
    $sectionTable = $pe + 24 + $optionalSize
    $newHeader = $sectionTable + $sections * 40
    Assert-Bytes $InputData $newHeader ([byte[]](0) * 40) 'cabecalho livre da setima secao'

    $lastHeader = $sectionTable + ($sections - 1) * 40
    $lastName = [Text.Encoding]::ASCII.GetString($InputData, $lastHeader, 8).Trim([char]0)
    $lastRawSize = [BitConverter]::ToUInt32($InputData, $lastHeader + 16)
    $lastRaw = [BitConverter]::ToUInt32($InputData, $lastHeader + 20)
    if ($lastName -ne '.xstat' -or $lastRaw + $lastRawSize -ne $sectionRaw) {
        throw 'a ultima secao nao corresponde ao layout .xstat esperado'
    }

    $expanded = New-Object byte[] ($sectionRaw + $sectionSize)
    [Array]::Copy($InputData, $expanded, $InputData.Length)
    Set-U16 $expanded ($pe + 6) 7
    Set-U32 $expanded ($pe + 24 + 56) ([uint32]0x00FD2000)
    $name = [Text.Encoding]::ASCII.GetBytes(".costkr`0")
    Set-Bytes $expanded $newHeader $name
    Set-U32 $expanded ($newHeader + 8) ([uint32]$sectionSize)
    Set-U32 $expanded ($newHeader + 12) $sectionRVA
    Set-U32 $expanded ($newHeader + 16) ([uint32]$sectionSize)
    Set-U32 $expanded ($newHeader + 20) ([uint32]$sectionRaw)
    Set-U32 $expanded ($newHeader + 36) ([Convert]::ToUInt32('E0000060', 16))
    return ,$expanded
}

if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) { throw "WYD.exe ausente: $Executable" }
if (-not (Test-Path -LiteralPath $Manifest -PathType Leaf)) { throw "manifesto ausente: $Manifest" }
$definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
$items = @($definition.items | Where-Object available | Sort-Object item)
$renderers = @($definition.renderers | Sort-Object type)
if ($items.Count -ne 135 -or $renderers.Count -ne 129) {
    throw "manifesto inesperado: items=$($items.Count), renderers=$($renderers.Count)"
}

$actualHash = Get-Sha $Executable
if ($expectedOutputHash -and $actualHash -eq $expectedOutputHash) {
    Write-Host "Colecao KR ja instalada no WYD.exe ($actualHash)."
    return
}
if ($VerifyOnly) { throw "WYD.exe ainda nao contem a colecao KR (SHA-256: $actualHash)" }
if ($actualHash -ne $expectedInputHash) {
    throw "WYD.exe fora da entrada suportada para trajes KR (SHA-256: $actualHash)"
}

$backup = Join-Path (Split-Path -Parent $Executable) 'WYD.pre-costumes-kr.exe'
if (-not (Test-Path -LiteralPath $backup -PathType Leaf)) {
    Copy-Item -LiteralPath $Executable -Destination $backup
}

$inputData = [IO.File]::ReadAllBytes($Executable)
$selectHookOffset = 0xFB344
$selectHookVA = [uint32]0x004FB344
$admissionHookOffset = 0xFB228
$admissionHookVA = [uint32]0x004FB228
$renderHookOffset = 0xBF8BF
$renderHookVA = [uint32]0x004BF8BF
$cullHookOffset = 0xC51F7
$cullHookVA = [uint32]0x004C51F7
Assert-Bytes $inputData $selectHookOffset ([byte[]](0x66,0xC7,0x45,0xEC,0x01,0x00)) 'seletor nativo 7.48'
Assert-Bytes $inputData $admissionHookOffset ([byte[]](0x8B,0x8D,0x98,0xFC,0xFF,0xFF)) 'gate nativo de trajes 7.48'
Assert-Bytes $inputData $renderHookOffset ([byte[]](0x8B,0x8D,0x30,0xFF,0xFF,0xFF)) 'renderer nativo 7.48'
Assert-Bytes $inputData $cullHookOffset ([byte[]](0x8B,0x8D,0x58,0xFD,0xFF,0xFF)) 'culling nativo 7.48'
$data = Add-PESection $inputData

$itemTableVA = [uint32]($sectionVA + $itemTableOffset)
$rendererTableVA = [uint32]($sectionVA + $rendererTableOffset)
$stringsVA = [uint32]($sectionVA + $stringsOffset)

# The selector always preserves the native type 1 fallback. Imported items are
# resolved by exact item index and only replace the local costume renderer type.
#
# TMHuman+0x5C is m_nSkinMeshType, not a male/female boolean. The native 7.48
# stores many body/skeleton variants there (0..43). Overwriting it with the
# manifest's 0/1 body category detached the head/hands/feet and could hide the
# torso. Keep the existing value intact; it is read only to choose the female
# renderer for the two classMask=15 costumes that actually have two variants.
# CreateMob/UpdateEquip transport only the low 12 bits of the visual item in
# the 7.48 ABI. Rebuild the 4xxx costume namespace before consulting the exact
# manifest so the same costume is selected at character select and in-world.
$selector = New-Assembler ([uint32]($sectionVA + $selectorOffset))
Emit $selector ([byte[]](0x66,0xC7,0x45,0xEC,0x01,0x00, 0x60,
    0x8B,0x85,0x98,0xFC,0xFF,0xFF, 0x8B,0x58,0x5C, 0x83,0xE3,0x01,
    0x0F,0xB7,0x90,0xAE,0x07,0x00,0x00,
    0x81,0xE2,0xFF,0x0F,0x00,0x00,
    0x81,0xC2,0x00,0x10,0x00,0x00,
    0xB9))
Emit $selector ([BitConverter]::GetBytes([uint32]$items.Count))
Emit $selector ([byte[]](0xBE))
Emit $selector ([BitConverter]::GetBytes($itemTableVA))
Mark $selector 'loop'
Emit $selector ([byte[]](0x66,0x3B,0x16))
Emit-Rel32 $selector ([byte[]](0x0F,0x84)) 'found'
Emit $selector ([byte[]](0x83,0xC6,0x08,0x49))
Emit-Rel32 $selector ([byte[]](0x0F,0x85)) 'loop'
Emit-Rel32 $selector ([byte[]](0xE9)) 'done'
Mark $selector 'found'
Emit $selector ([byte[]](0x0F,0xB7,0x7E,0x02, 0x80,0x7E,0x06,0xFF))
Emit-Rel32 $selector ([byte[]](0x0F,0x85)) 'selected'
Emit $selector ([byte[]](0x85,0xDB))
Emit-Rel32 $selector ([byte[]](0x0F,0x84)) 'selected'
Emit $selector ([byte[]](0x0F,0xB7,0x7E,0x04))
Mark $selector 'selected'
# O bit 0x4000 marca exclusivamente uma instancia de traje importado. Os
# numeros de m_nCosType tambem aparecem em skins nativas de personagens/NPCs;
# sem esta marca o hook de RestoreDeviceObjects nao consegue distinguir as
# duas origens com seguranca, sobretudo na parte 0.
Emit $selector ([byte[]](0x66,0x81,0xCF,0x00,0x40, 0x66,0x89,0x7D,0xEC))
Mark $selector 'done'
Emit $selector ([byte[]](0x61,0xE9))
$selectorBytes = Complete $selector
$selectorBytes += [byte[]](Rel32 ([uint32]($sectionVA + $selectorOffset + $selectorBytes.Length + 4)) ([uint32]0x004FB34A))
if ($selectorBytes.Length -gt ($rendererOffset - $selectorOffset)) { throw 'seletor excedeu a area reservada' }
Set-Bytes $data ($sectionRaw + $selectorOffset) $selectorBytes

# Build one stable string pool and a direct [type][part] pointer table. Empty
# mesh names are intentional in the KR data and skip only that body part.
$stringData = [Collections.Generic.List[byte]]::new()
$stringPointers = @{}
$stringData.Add(0)
$emptyStringVA = $stringsVA
function Add-String([string]$Value) {
    if (-not $Value) { return $emptyStringVA }
    $key = $Value.ToLowerInvariant()
    if ($stringPointers.ContainsKey($key)) { return [uint32]$stringPointers[$key] }
    $pointer = [uint32]($stringsVA + $stringData.Count)
    $bytes = [Text.Encoding]::ASCII.GetBytes($Value)
    $stringData.AddRange($bytes)
    $stringData.Add(0)
    $stringPointers[$key] = $pointer
    return $pointer
}

$rendererByType = @{}
foreach ($renderer in $renderers) { $rendererByType[[int]$renderer.type] = $renderer }
for ($type = 0; $type -lt $rendererTypeCount; $type++) {
    for ($part = 0; $part -lt $partCount; $part++) {
        $entryOffset = $sectionRaw + $rendererTableOffset + (($type * $partCount + $part) * 8)
        if (-not $rendererByType.ContainsKey($type)) { continue }
        $partDef = $rendererByType[$type].parts[$part]
        $texturePointer = Add-String ([string]$partDef.texture)
        $meshPointer = if ([string]$partDef.mesh) { Add-String ([string]$partDef.mesh) } else { [uint32]0 }
        Set-U32 $data $entryOffset $texturePointer
        Set-U32 $data ($entryOffset + 4) $meshPointer
    }
}
if ($stringsOffset + $stringData.Count -gt $sectionSize) { throw 'strings de traje excederam .costkr' }
Set-Bytes $data ($sectionRaw + $stringsOffset) $stringData.ToArray()

for ($i = 0; $i -lt $items.Count; $i++) {
    $item = $items[$i]
    $offset = $sectionRaw + $itemTableOffset + $i * 8
    Set-U16 $data $offset ([uint16]$item.item)
    Set-U16 $data ($offset + 2) ([uint16]$item.maleType)
    Set-U16 $data ($offset + 4) ([uint16]$item.femaleType)
    $skin = if ([int]$item.skin -lt 0) { [byte]0xFF } else { [byte]$item.skin }
    $data[$offset + 6] = $skin
}

# O 7.48 rejeita Equip[13] fora da faixa nativa 4151..4200 antes de chegar ao
# seletor de traje. A maior parte da colecao KR usa IDs superiores a 4200.
# Este gate adicional admite somente os IDs exatos do manifesto e os envia ao
# caminho nativo que prepara o look; qualquer outro item conserva integralmente
# as comparacoes originais do executavel.
$admission = New-Assembler ([uint32]($sectionVA + $admissionOffset))
Emit $admission ([byte[]](0x60,
    0x8B,0x85,0x98,0xFC,0xFF,0xFF,
    0x0F,0xB7,0x90,0xAE,0x07,0x00,0x00,
    0x81,0xE2,0xFF,0x0F,0x00,0x00,
    0x81,0xC2,0x00,0x10,0x00,0x00,
    0xB9))
Emit $admission ([BitConverter]::GetBytes([uint32]$items.Count))
Emit $admission ([byte[]](0xBE))
Emit $admission ([BitConverter]::GetBytes($itemTableVA))
Mark $admission 'loop'
Emit $admission ([byte[]](0x66,0x3B,0x16))
Emit-Rel32 $admission ([byte[]](0x0F,0x84)) 'found'
Emit $admission ([byte[]](0x83,0xC6,0x08,0x49))
Emit-Rel32 $admission ([byte[]](0x0F,0x85)) 'loop'
Emit $admission ([byte[]](0x61,0x8B,0x8D,0x98,0xFC,0xFF,0xFF,0xE9))
$admissionFallbackNext = [uint32]($admission.BaseVA + $admission.Bytes.Count + 4)
Emit $admission (Rel32 $admissionFallbackNext ([uint32]0x004FB22E))
Mark $admission 'found'
Emit $admission ([byte[]](0x61,0xE9))
$admissionFoundNext = [uint32]($admission.BaseVA + $admission.Bytes.Count + 4)
Emit $admission (Rel32 $admissionFoundNext ([uint32]0x004FB25A))
$admissionBytes = Complete $admission
if (($itemTableOffset + $items.Count * 8) -gt $admissionOffset) { throw 'itemTable invadiu o gate de admissao' }
if ($admissionBytes.Length -gt ($rendererTableOffset - $admissionOffset)) { throw 'gate de admissao excedeu a area reservada' }
Set-Bytes $data ($sectionRaw + $admissionOffset) $admissionBytes

$renderer = New-Assembler ([uint32]($sectionVA + $rendererOffset))
Emit $renderer ([byte[]](0x60,
    0x8B,0x85,0x30,0xFF,0xFF,0xFF,
    0x0F,0xB7,0x80,0x56,0x03,0x00,0x00,
    0xA9,0x00,0x40,0x00,0x00))
Emit-Rel32 $renderer ([byte[]](0x0F,0x84)) 'fallback'
Emit $renderer ([byte[]](0x25,0xFF,0x3F,0x00,0x00, 0x3D))
Emit $renderer ([BitConverter]::GetBytes([uint32]$rendererTypeCount))
Emit-Rel32 $renderer ([byte[]](0x0F,0x83)) 'fallback'
Emit $renderer ([byte[]](0x6B,0xC0,0x30, 0x8B,0x55,0xC4, 0x83,0xFA,0x05))
Emit-Rel32 $renderer ([byte[]](0x0F,0x87)) 'fallback'
Emit $renderer ([byte[]](0xC1,0xE2,0x03, 0x05))
Emit $renderer ([BitConverter]::GetBytes($rendererTableVA))
Emit $renderer ([byte[]](0x03,0xC2, 0x8B,0x30, 0x85,0xF6))
Emit-Rel32 $renderer ([byte[]](0x0F,0x84)) 'fallback'
Emit $renderer ([byte[]](0x8B,0x78,0x04, 0x85,0xFF))
Emit-Rel32 $renderer ([byte[]](0x0F,0x85)) 'loadPart'
# Uma entrada vazia em part0 significa preservar a face nativa; nas demais
# partes significa omitir somente aquela geometria. Part0 real nunca mais cai
# no renderer legado.
Emit $renderer ([byte[]](0x83,0x7D,0xC4,0x00))
Emit-Rel32 $renderer ([byte[]](0x0F,0x84)) 'fallback'
Emit-Rel32 $renderer ([byte[]](0xE9)) 'skipPart'
Mark $renderer 'loadPart'
Emit $renderer ([byte[]](0x56,0x8D,0x4D,0x80,0x51,0xE8))
$textureCallNext = [uint32]($renderer.BaseVA + $renderer.Bytes.Count + 4)
Emit $renderer (Rel32 $textureCallNext ([uint32]0x0058F078))
Emit $renderer ([byte[]](0x83,0xC4,0x08, 0x57,0x8D,0x55,0xD0,0x52,0xE8))
$meshCallNext = [uint32]($renderer.BaseVA + $renderer.Bytes.Count + 4)
Emit $renderer (Rel32 $meshCallNext ([uint32]0x0058F078))
Emit $renderer ([byte[]](0x83,0xC4,0x08,0x61,0xE9))
$renderSuccessNext = [uint32]($renderer.BaseVA + $renderer.Bytes.Count + 4)
Emit $renderer (Rel32 $renderSuccessNext ([uint32]0x004C0052))
Mark $renderer 'skipPart'
Emit $renderer ([byte[]](0x61,0xE9))
$skipNext = [uint32]($renderer.BaseVA + $renderer.Bytes.Count + 4)
Emit $renderer (Rel32 $skipNext ([uint32]0x004BEAD1))
Mark $renderer 'fallback'
Emit $renderer ([byte[]](0x61,0x8B,0x8D,0x30,0xFF,0xFF,0xFF,0xE9))
$fallbackNext = [uint32]($renderer.BaseVA + $renderer.Bytes.Count + 4)
Emit $renderer (Rel32 $fallbackNext ([uint32]0x004BF8C5))
$rendererBytes = Complete $renderer
if ($rendererBytes.Length -gt ($itemTableOffset - $rendererOffset)) { throw 'renderer excedeu a area reservada' }
Set-Bytes $data ($sectionRaw + $rendererOffset) $rendererBytes

# As meshes esqueletais KR foram produzidas para o renderer atual, que usa
# D3DCULL_NONE. O 7.48 escolhe CW/CCW para todo TMSkinMesh tipo 1 e elimina
# faces validas dessas meshes, produzindo torsos invisiveis e superficies
# vistas pelo lado interno. A excecao abaixo e deliberadamente seletiva: so a
# renderer existente na tabela KR recebe CULL_NONE; todo asset nativo continua no caminho
# original byte a byte. Nao usamos m_cRotate como marcador de traje porque o
# client o reescreve sempre que recalcula o tipo das armas.
$selectiveCull = New-Assembler ([uint32]($sectionVA + $selectiveCullOffset))
Emit $selectiveCull ([byte[]](
    0x60,
    0x8B,0x85,0x58,0xFD,0xFF,0xFF,
    0x8B,0x80,0xC0,0x00,0x00,0x00,
    0x80,0xB8,0x51,0x03,0x00,0x00,0x7F))
Emit-Rel32 $selectiveCull ([byte[]](0x0F,0x84)) 'imported'
Emit $selectiveCull ([byte[]](
    0x0F,0xB7,0x80,0x56,0x03,0x00,0x00,
    0xA9,0x00,0x40,0x00,0x00))
Emit-Rel32 $selectiveCull ([byte[]](0x0F,0x84)) 'nativePop'
Emit $selectiveCull ([byte[]](0x25,0xFF,0x3F,0x00,0x00, 0x3D))
Emit $selectiveCull ([BitConverter]::GetBytes([uint32]$rendererTypeCount))
Emit-Rel32 $selectiveCull ([byte[]](0x0F,0x83)) 'nativePop'
Emit $selectiveCull ([byte[]](0x6B,0xC0,0x30,0x05))
Emit $selectiveCull ([BitConverter]::GetBytes($rendererTableVA))
Emit $selectiveCull ([byte[]](0x8B,0xF0,0xB9,0x0C,0x00,0x00,0x00,0x33,0xD2))
Mark $selectiveCull 'scan'
Emit $selectiveCull ([byte[]](0x0B,0x16,0x83,0xC6,0x04,0x49))
Emit-Rel32 $selectiveCull ([byte[]](0x0F,0x85)) 'scan'
Emit $selectiveCull ([byte[]](0x85,0xD2))
Emit-Rel32 $selectiveCull ([byte[]](0x0F,0x84)) 'nativePop'
Mark $selectiveCull 'imported'
Emit $selectiveCull ([byte[]](0x61))
# Mantem o local usado depois pelo renderer, inclusive no antigo caso de
# escudo, antes de selecionar D3DCULL_NONE (1).
Emit $selectiveCull ([byte[]](
    0x8B,0x85,0x58,0xFD,0xFF,0xFF,
    0x8B,0x80,0xC0,0x00,0x00,0x00,
    0x8B,0x80,0xF4,0x02,0x00,0x00,
    0x89,0x85,0x48,0xFF,0xFF,0xFF,
    0x6A,0x01,0x6A,0x16,
    0xA1,0x9C,0xCF,0x5C,0x00,
    0x8B,0x10,
    0x8B,0x0D,0x9C,0xCF,0x5C,0x00,
    0xFF,0x52,0x30,
    0xE9))
$selectiveReturnNext = [uint32]($selectiveCull.BaseVA + $selectiveCull.Bytes.Count + 4)
Emit $selectiveCull (Rel32 $selectiveReturnNext ([uint32]0x004C528D))
Mark $selectiveCull 'native'
Emit $selectiveCull ([byte[]](0x8B,0x8D,0x58,0xFD,0xFF,0xFF,0xE9))
$selectiveNativeNext = [uint32]($selectiveCull.BaseVA + $selectiveCull.Bytes.Count + 4)
Emit $selectiveCull (Rel32 $selectiveNativeNext ([uint32]0x004C51FD))
Mark $selectiveCull 'nativePop'
Emit $selectiveCull ([byte[]](0x61))
Emit-Rel32 $selectiveCull ([byte[]](0xE9)) 'native'
$selectiveCullBytes = Complete $selectiveCull
if ($rendererTableOffset + ($rendererTypeCount * $partCount * 8) -gt $selectiveCullOffset) { throw 'tabela de renderers invadiu o hook de culling' }
if ($selectiveCullOffset + $selectiveCullBytes.Length -gt $stringsOffset) { throw 'hook de culling excedeu a area reservada' }
Set-Bytes $data ($sectionRaw + $selectiveCullOffset) $selectiveCullBytes

$selectHook = [byte[]](0xE9) + [byte[]](Rel32 ($selectHookVA + 5) ([uint32]($sectionVA + $selectorOffset))) + [byte[]](0x90)
$admissionHook = [byte[]](0xE9) + [byte[]](Rel32 ($admissionHookVA + 5) ([uint32]($sectionVA + $admissionOffset))) + [byte[]](0x90)
$renderHook = [byte[]](0xE9) + [byte[]](Rel32 ($renderHookVA + 5) ([uint32]($sectionVA + $rendererOffset))) + [byte[]](0x90)
$cullHook = [byte[]](0xE9) + [byte[]](Rel32 ($cullHookVA + 5) ([uint32]($sectionVA + $selectiveCullOffset))) + [byte[]](0x90)
Set-Bytes $data $selectHookOffset $selectHook
Set-Bytes $data $admissionHookOffset $admissionHook
Set-Bytes $data $renderHookOffset $renderHook
Set-Bytes $data $cullHookOffset $cullHook

[IO.File]::WriteAllBytes($Executable, $data)
$newHash = Get-Sha $Executable
if ($expectedOutputHash -and $newHash -ne $expectedOutputHash) {
    throw "saida de trajes KR divergente: esperado $expectedOutputHash, encontrado $newHash"
}
Write-Host "$($items.Count) trajes KR e $($renderers.Count) renderers instalados no WYD 7.48."
Write-Host "SHA-256 antes:  $actualHash"
Write-Host "SHA-256 depois: $newHash"
Write-Host "Backup: $backup"
