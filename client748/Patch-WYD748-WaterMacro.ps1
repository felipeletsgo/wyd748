param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Independent sixth client-patch link.  The server remains authoritative: the
# code below only chooses a carry slot and calls the exact native UseItem
# routine used by a manual click.  It never builds a packet or decides whether
# a room may be entered.
#
#   ... -> Patch-WYD748-Lindy.ps1 -> Patch-WYD748-WaterMacro.ps1
#
# This script intentionally has no hard-coded Water item IDs or coordinates.
# tools/watermacrotable resolves data/volatiles.json through the same Go
# loaders used by the server and the resulting table is embedded in the cave.

$expectedHash = '9762B1AC6EFB4AB3C800877DE1DA048DD43EA407FCEEA945C755DF6986607F18'
$backupName = 'WYD.pre-water-macro.exe'
$caveOffset = 0x1D3243
$caveVA = [uint32]0x013C0243
$caveCapacity = 0x5BD # zero range ends at 0x1D37FF; macro cave starts earlier

function Assert-Bytes([byte[]]$Data, [int]$Offset, [byte[]]$Expected, [string]$Name) {
    if ($Offset -lt 0 -or $Offset + $Expected.Length -gt $Data.Length) {
        throw ('{0}: intervalo fora do executavel em 0x{1:X}' -f $Name, $Offset)
    }
    for ($i = 0; $i -lt $Expected.Length; $i++) {
        if ($Data[$Offset + $i] -ne $Expected[$i]) {
            throw ('{0}: byte inesperado em 0x{1:X}: esperado {2:X2}, encontrado {3:X2}' -f
                $Name, ($Offset + $i), $Expected[$i], $Data[$Offset + $i])
        }
    }
}

function Assert-AllZero([byte[]]$Data, [int]$Offset, [int]$Length, [string]$Name) {
    if ($Offset -lt 0 -or $Offset + $Length -gt $Data.Length) {
        throw ('{0}: intervalo fora do executavel em 0x{1:X}' -f $Name, $Offset)
    }
    for ($i = 0; $i -lt $Length; $i++) {
        if ($Data[$Offset + $i] -ne 0) {
            throw ('{0}: area nao esta livre em 0x{1:X}' -f $Name, ($Offset + $i))
        }
    }
}

function Set-Bytes([byte[]]$Data, [int]$Offset, [byte[]]$Value) {
    if ($Offset -lt 0 -or $Offset + $Value.Length -gt $Data.Length) {
        throw ('escrita fora do executavel em 0x{0:X}' -f $Offset)
    }
    [Array]::Copy($Value, 0, $Data, $Offset, $Value.Length)
}

function U16([int]$Value) {
    if ($Value -lt 0 -or $Value -gt 65535) { throw "valor uint16 invalido: $Value" }
    return [BitConverter]::GetBytes([uint16]$Value)
}

function U32([uint32]$Value) { return [BitConverter]::GetBytes($Value) }

function Rel32([uint32]$FromNextInstruction, [uint32]$Target) {
    return [BitConverter]::GetBytes([int32]([int64]$Target - [int64]$FromNextInstruction))
}

if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
    throw "Executavel nao encontrado: $Executable"
}

$beforeHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToUpperInvariant()
if ($beforeHash -ne $expectedHash) {
    throw "Entrada inesperada (SHA-256: $beforeHash). A Water Macro exige a saida pos-Lindy da cadeia."
}

$data = [IO.File]::ReadAllBytes($Executable)

# The two call sites are the only hooks.  Their original bytes are asserted so
# a client from another build/version can never be patched accidentally.
$chatOffset = 0x678E4
$chatVA = [uint32]0x004678E4
$chatOriginal = [byte[]](0xE8,0x8F,0x77,0x12,0x00)
$frameOffset = 0x779A7
$frameVA = [uint32]0x004779A7
$frameOriginal = [byte[]](0xE8,0x80,0xBC,0x01,0x00)
Assert-Bytes $data $chatOffset $chatOriginal 'hook de chat (formatter)'
Assert-Bytes $data $frameOffset $frameOriginal 'hook de tick (macro nativo)'
Assert-AllZero $data $caveOffset $caveCapacity 'cave .xstat Water'

# --- resolve the authoritative table --------------------------------------
$repoRoot = Split-Path -Parent $PSScriptRoot
$go = Get-Command go -ErrorAction SilentlyContinue
if ($null -eq $go) { throw 'Go nao encontrado: tools/watermacrotable e obrigatorio.' }
$tablePath = Join-Path ([IO.Path]::GetTempPath()) ('wyd-water-table-' + [guid]::NewGuid().ToString('N') + '.json')
try {
    Push-Location $repoRoot
    & $go.Source run ./tools/watermacrotable -root . -out $tablePath | Out-Host
    if ($LASTEXITCODE -ne 0) { throw 'watermacrotable terminou com erro.' }
} finally {
    Pop-Location
}
try {
    $parsed = Get-Content -LiteralPath $tablePath -Raw | ConvertFrom-Json
    if ($parsed -is [Array]) {
        $waterEntries = [object[]]$parsed
    } else {
        $waterEntries = @($parsed)
    }
} finally {
    Remove-Item -LiteralPath $tablePath -Force -ErrorAction SilentlyContinue
}
if ($waterEntries.Count -le 0) { throw 'o catalogo nao possui Water Scrolls.' }
if ($waterEntries.Count -gt 255) { throw 'a tabela Water excede o contador de 8 bits.' }

# Variable-size records: uint16 item, uint16 rectangle count, then rectangles
# (minX,minY,maxX,maxY) as uint16 values.  This supports all resolved areas
# without padding the code cave with a second, duplicated table.
$tableBytes = [System.Collections.Generic.List[byte]]::new()
foreach ($entry in $waterEntries) {
    # `item` is also a PowerShell collection member; access JSON properties
    # explicitly so Windows PowerShell 5 does not bind PSParameterizedProperty.
    $entryItem = [int]$entry.PSObject.Properties['item'].Value
    $areas = @($entry.PSObject.Properties['areas'].Value)
    if ($areas.Count -lt 1 -or $areas.Count -gt 255) {
        throw ("item Water {0} possui {1} areas" -f $entryItem, $areas.Count)
    }
    foreach ($b in (U16 $entryItem)) { [void]$tableBytes.Add($b) }
    foreach ($b in (U16 $areas.Count)) { [void]$tableBytes.Add($b) }
    foreach ($area in $areas) {
        if ($area.minX -gt $area.maxX -or $area.minY -gt $area.maxY -or
            $area.minX -gt 65535 -or $area.minY -gt 65535 -or
            $area.maxX -gt 65535 -or $area.maxY -gt 65535) {
            throw ("area invalida para item Water {0}" -f $entryItem)
        }
        foreach ($value in @($area.minX,$area.minY,$area.maxX,$area.maxY)) {
            foreach ($b in (U16 $value)) { [void]$tableBytes.Add($b) }
        }
    }
}

# --- tiny fixed-width x86 assembler ---------------------------------------
$code = [System.Collections.Generic.List[byte]]::new()
$labels = @{}
$fixups = [System.Collections.Generic.List[object]]::new()

function Here { return $script:code.Count }
function Emit([byte[]]$Bytes) { foreach ($byte in $Bytes) { [void]$script:code.Add($byte) } }
function B([int]$Value) { Emit ([byte[]]([byte]($Value -band 0xFF))) }
function D([uint32]$Value) { Emit (U32 $Value) }
function Mark([string]$Name) { $script:labels[$Name] = Here }
function FixLabel([string]$Name, [int]$Offset, [string]$Kind) {
    [void]$script:fixups.Add([pscustomobject]@{ Name=$Name; Offset=$Offset; Kind=$Kind })
}
function Abs32([string]$Name) { $offset=Here; D 0; FixLabel $Name $offset 'abs' }
function CallLabel([string]$Name) { B 0xE8; $offset=Here; D 0; FixLabel $Name $offset 'rel' }
function JmpLabel([string]$Name) { B 0xE9; $offset=Here; D 0; FixLabel $Name $offset 'rel' }
function Jcc([int]$Condition, [string]$Name) {
    B 0x0F; B (0x80 + $Condition); $offset=Here; D 0; FixLabel $Name $offset 'rel'
}
function CallVA([uint32]$Address) {
    B 0xE8
    # PowerShell parses `Here + 4` as arguments to the Here command. The
    # explicit parentheses are required to calculate the address after the
    # four-byte displacement; without them every native call lands four bytes
    # past its function entry.
    $next = [int64]$caveVA + [int64]((Here) + 4)
    Emit ([BitConverter]::GetBytes([int32]([int64]$Address - $next)))
}
function JmpVA([uint32]$Address) {
    B 0xE9
    $next = [int64]$caveVA + [int64]((Here) + 4)
    Emit ([BitConverter]::GetBytes([int32]([int64]$Address - $next)))
}
function CallImport([uint32]$Address) { Emit ([byte[]](0xFF,0x15)); D $Address }
function Pushad { B 0x60 }
function Popad { B 0x61 }
function PushImm8([int]$Value) { Emit ([byte[]](0x6A,[byte]($Value -band 0xFF))) }
function PushImm32([uint32]$Value) { B 0x68; D $Value }
function PushLabel([string]$Name) { B 0x68; Abs32 $Name }
function MovRegImm([int]$Reg, [uint32]$Value) { B (0xB8 + $Reg); D $Value }
function MovRegImmLabel([int]$Reg, [string]$Name) {
    B (0xB8 + $Reg); $offset=Here; D 0; FixLabel $Name $offset 'abs'
}
function MovRegReg([int]$Dst, [int]$Src) {
    $modrm = 0xC0 + ($Dst -shl 3) + $Src
    Emit ([byte[]](0x8B,[byte]$modrm))
}
function MovEaxMem([string]$Name) { B 0xA1; Abs32 $Name }
function MovMemEax([string]$Name) { B 0xA3; Abs32 $Name }
function MovRegMem([int]$Reg, [string]$Name) {
    $rm = @(0x05,0x0D,0x15,0x1D,0x25,0x2D,0x35,0x3D)[$Reg]
    Emit ([byte[]](0x8B,[byte]$rm)); Abs32 $Name
}
function MovMemReg([int]$Reg, [string]$Name) {
    $rm = @(0x05,0x0D,0x15,0x1D,0x25,0x2D,0x35,0x3D)[$Reg]
    Emit ([byte[]](0x89,[byte]$rm)); Abs32 $Name
}
function PushMem([string]$Name) { Emit ([byte[]](0xFF,0x35)); Abs32 $Name }
function AddEsp([int]$Value) { Emit ([byte[]](0x83,0xC4,[byte]($Value -band 0xFF))) }
function TestEax { Emit ([byte[]](0x85,0xC0)) }
function XorEax { Emit ([byte[]](0x31,0xC0)) }
function IncReg([int]$Reg) { B (0x40 + $Reg) }

# Proven engine addresses in the 7.48 binary.
$timeGetTimeIAT = [uint32]0x005A3394
$floatToWordVA = [uint32]0x0058F1F0
$nativeUseItemVA = [uint32]0x00465F85
$scanIntervalMs = 3000 # W2PP ThreadMacro cadence; avoids stale UI/control reuse.
$nativeMacroVA = [uint32]0x0049362C
$messageVA = [uint32]0x00493AA1
$sprintfVA = [uint32]0x0058F078
$itemCategoryVA = [uint32]0x0054CD07

# Tick trampoline: scan first, then execute the original A/D macro unchanged.
Mark 'frame_entry'
CallLabel 'scan_water'
CallVA $nativeMacroVA
B 0xC3

# Chat trampoline. pushad keeps the original formatter arguments at fixed
# offsets: dest=[esp+24], format=[esp+28], raw=[esp+2C].  The native client
# normally delivers the command without its leading slash, but some input
# paths preserve it; the hook accepts both forms without changing fallback
# chat text.
Mark 'chat_entry'
Pushad
Emit ([byte[]](0x8B,0x74,0x24,0x2C)) # mov esi,[esp+2C]
# A few parser paths can reach the formatter with no command buffer.  Never
# pass a null pointer into the local comparator; preserve the original path.
Emit ([byte[]](0x85,0xF6)) # test esi,esi
Jcc 4 'chat_empty'
# Normalize an optional leading '/' only for comparison.  The original raw
# pointer remains on the stack, so non-local chat still reaches sprintf
# byte-for-byte unchanged.
Emit ([byte[]](0x80,0x3E,0x2F)) # cmp byte ptr [esi], '/'
Jcc 5 'chat_command'
IncReg 6 # ESI++
Mark 'chat_command'
MovRegImmLabel 7 'cmd_on'
CallLabel 'cmp_ci'
TestEax
Jcc 5 'chat_on'
MovRegImmLabel 7 'cmd_off'
CallLabel 'cmp_ci'
TestEax
Jcc 5 'chat_off'
# All non-local text must execute the original formatter byte-for-byte.
JmpLabel 'chat_fallback'

Mark 'chat_empty'
# Avoid passing a null %s argument to the CRT formatter on malformed input.
# This path is unreachable for normal client chat, but makes the hook safe
# when the input widget is torn down during logout/zone changes.
MovRegImmLabel 0 'empty'
Emit ([byte[]](0x89,0x44,0x24,0x2C)) # replace raw argument in saved stack
JmpLabel 'chat_fallback'

Mark 'chat_fallback'
# Original formatter call: push raw, format, destination. After pushad the
# original values are dest=+24, format=+28 and raw=+2C; because ESP changes
# after each push, +2C resolves to raw, then format, then destination.
Emit ([byte[]](0xFF,0x74,0x24,0x2C))
Emit ([byte[]](0xFF,0x74,0x24,0x2C))
Emit ([byte[]](0xFF,0x74,0x24,0x2C))
CallVA $sprintfVA
AddEsp 12
Popad
B 0xC3

Mark 'chat_on'
MovRegImm 0 1
MovMemEax 'macro_state'
Emit ([byte[]](0x8B,0x8D,0x2C,0xE5,0xFF,0xFF)) # mov ecx,[ebp-1AD4]
PushLabel 'msg_on'
CallVA $messageVA
Popad
MovRegImm 0 1
# The parser's handled-return path performs the normal epilogue and returns
# with the correct status.  Jumping there is important: returning to the
# formatter call site would continue the original chat-send path and leak the
# local command to the server.
JmpVA ([uint32]0x00470D31)

Mark 'chat_off'
MovRegImm 0 0
MovMemEax 'macro_state'
Emit ([byte[]](0x8B,0x8D,0x2C,0xE5,0xFF,0xFF))
PushLabel 'msg_off'
CallVA $messageVA
Popad
MovRegImm 0 1
JmpVA ([uint32]0x00470D31)

# Exact ASCII, case-insensitive comparison. ESI/EDI are caller-owned by the
# trampoline and are restored by pushad; EBX is preserved locally as required.
Mark 'cmp_ci'
B 0x53 # push ebx
Mark 'cmp_loop'
Emit ([byte[]](0x8A,0x06,0x8A,0x1F))
B 0x3C; B 0x41; Jcc 2 'cmp_a_done'; B 0x3C; B 0x5A; Jcc 7 'cmp_a_done'; B 0x0C; B 0x20
Mark 'cmp_a_done'
B 0x80; B 0xFB; B 0x41; Jcc 2 'cmp_b_done'; B 0x80; B 0xFB; B 0x5A; Jcc 7 'cmp_b_done'; B 0x80; B 0xCB; B 0x20
Mark 'cmp_b_done'
B 0x38; B 0xD8; Jcc 5 'cmp_no'
B 0x84; B 0xC0; Jcc 4 'cmp_yes'
IncReg 6; IncReg 7; JmpLabel 'cmp_loop'
Mark 'cmp_no'
XorEax; B 0x5B; B 0xC3
Mark 'cmp_yes'
MovRegImm 0 1; B 0x5B; B 0xC3

# Scanner called in the original client thread. This 7.48 client exposes one
# carry grid with 63 visible slots (9 columns x 7 rows). The structural slot
# 63 is intentionally excluded from automation.  The native grid ABI is
# GetItem(x,y), so each call pushes y first and x second.
Mark 'scan_water'
Pushad
MovRegReg 6 1 # ESI = scene (ECX at the frame call)
Emit ([byte[]](0x83,0x3D)); Abs32 'macro_state'; B 0
Jcc 4 'scan_done'
CallImport $timeGetTimeIAT
MovRegReg 3 0 # EBX = now
MovRegReg 2 0 # EDX = now
Emit ([byte[]](0x2B,0x15)); Abs32 'last_scan'
Emit ([byte[]](0x81,0xFA)); D $scanIntervalMs
Jcc 2 'scan_done'
MovMemReg 3 'last_scan'
MovMemReg 6 'scan_scene'
Emit ([byte[]](0x8B,0x86,0x3C,0x87,0x02,0x00,0x85,0xC0)) # eax = active carry grid; test
Jcc 4 'scan_done'
MovMemEax 'scan_grid'
Emit ([byte[]](0x8B,0x46,0x4C,0x85,0xC0)) # eax = scene+4c; test
Jcc 4 'scan_done'
Emit ([byte[]](0xD9,0x40,0x28)); CallVA $floatToWordVA
Emit ([byte[]](0x0F,0xB7,0xD0)); MovMemReg 2 'scan_x'
Emit ([byte[]](0x8B,0x46,0x4C,0xD9,0x40,0x2C)); CallVA $floatToWordVA
Emit ([byte[]](0x0F,0xB7,0xC8)); MovMemReg 1 'scan_y'
Emit ([byte[]](0x31,0xFF)); MovMemReg 7 'scan_slot_value'

Mark 'scan_slot_loop'
MovEaxMem 'scan_slot_value'
# Stop at 63: slots 0..62 are visible and slot 63 is structural/internal.
Emit ([byte[]](0x83,0xF8,0x3F)); Jcc 3 'scan_done' # unsigned >=63
MovEaxMem 'scan_slot_value'
MovRegImm 2 0
MovRegImm 5 9
Emit ([byte[]](0xF7,0xF5))
MovMemEax 'scan_row'
MovMemReg 2 'scan_cell'
PushMem 'scan_row'; PushMem 'scan_cell'
MovRegMem 1 'scan_grid'
Emit ([byte[]](0x8B,0x01,0xFF,0x90,0xB4,0x00,0x00,0x00))
MovMemEax 'scan_item_control'; TestEax; Jcc 4 'scan_next_slot'
Emit ([byte[]](0x8B,0x88,0x70,0x06,0x00,0x00,0x89,0x0D)); Abs32 'scan_item_data'
Emit ([byte[]](0x85,0xC9)); Jcc 4 'scan_next_slot'
Emit ([byte[]](0x0F,0xB7,0x01)); MovMemEax 'scan_item_id'

# EBP walks the variable-size records; ECX is the record index.
Emit ([byte[]](0x31,0xC9)); MovRegMem 5 'water_table'
Mark 'scan_record'
Emit ([byte[]](0x83,0xF9)); B $waterEntries.Count; Jcc 3 'scan_next_slot'
Emit ([byte[]](0x0F,0xB7,0x55,0x00,0x3B,0x15)); Abs32 'scan_item_id'; Jcc 5 'scan_next_record'
Emit ([byte[]](0x0F,0xB7,0x45,0x02)); TestEax; Jcc 4 'scan_next_record'
XorEax
Mark 'scan_area'
Emit ([byte[]](0x66,0x3B,0x45,0x02)); Jcc 3 'scan_next_record'
Emit ([byte[]](0x8D,0x54,0xC5,0x04)) # EDX = EBP + EAX*8 + 4
# The instruction is `cmp bound, [current]`.  For the lower bound, JA means
# bound > current (current is below the rectangle); for the upper bound, JB
# means bound < current (current is above it).
Emit ([byte[]](0x0F,0xB7,0x1A,0x3B,0x1D)); Abs32 'scan_x'; Jcc 7 'scan_area_no'
Emit ([byte[]](0x0F,0xB7,0x5A,0x04,0x3B,0x1D)); Abs32 'scan_x'; Jcc 2 'scan_area_no'
Emit ([byte[]](0x0F,0xB7,0x5A,0x02,0x3B,0x1D)); Abs32 'scan_y'; Jcc 7 'scan_area_no'
Emit ([byte[]](0x0F,0xB7,0x5A,0x06,0x3B,0x1D)); Abs32 'scan_y'; Jcc 2 'scan_area_no'
JmpLabel 'scan_area_yes'
Mark 'scan_area_no'
IncReg 0; JmpLabel 'scan_area'

Mark 'scan_area_yes'
PushImm8 0x26; PushMem 'scan_item_data'; CallVA $itemCategoryVA; AddEsp 8
PushMem 'scan_row'; PushMem 'scan_cell'; PushMem 'scan_item_id'; B 0x50
PushMem 'scan_item_control'; MovRegMem 1 'scan_scene'; CallVA $nativeUseItemVA
JmpLabel 'scan_done'

Mark 'scan_next_record'
Emit ([byte[]](0x0F,0xB7,0x55,0x02,0x6B,0xD2,0x08,0x83,0xC2,0x04,0x01,0xD5))
IncReg 1; JmpLabel 'scan_record'
Mark 'scan_next_slot'
IncReg 7; MovMemReg 7 'scan_slot_value'; JmpLabel 'scan_slot_loop'
Mark 'scan_done'
Popad; B 0xC3

# --- data and fixups -------------------------------------------------------
Mark 'water_table'
Emit ([byte[]]$tableBytes.ToArray())
Mark 'macro_state'; D 0
Mark 'last_scan'; D 0
Mark 'scan_scene'; D 0
Mark 'scan_grid'; D 0
Mark 'scan_x'; D 0
Mark 'scan_y'; D 0
Mark 'scan_slot_value'; D 0
Mark 'scan_row'; D 0
Mark 'scan_cell'; D 0
Mark 'scan_item_id'; D 0
Mark 'scan_item_control'; D 0
Mark 'scan_item_data'; D 0
# The native parser normally removes the leading slash before this formatter
# hook (W2PP ComandosChat receives "autowater", not "/autowater"). The hook
# also normalizes a slash at runtime so both client input paths are accepted.
Mark 'cmd_on'; Emit ([Text.Encoding]::ASCII.GetBytes('macropergaon' + [char]0))
Mark 'cmd_off'; Emit ([Text.Encoding]::ASCII.GetBytes('macropergaoff' + [char]0))
Mark 'empty'; Emit ([byte]0)
Mark 'msg_on'; Emit ([Text.Encoding]::ASCII.GetBytes('Water scroll macro enabled.' + [char]0))
Mark 'msg_off'; Emit ([Text.Encoding]::ASCII.GetBytes('Water scroll macro disabled.' + [char]0))

if ($code.Count -gt $caveCapacity) {
    throw ("cave Water excedeu a area livre: 0x{0:X} > 0x{1:X}" -f $code.Count, $caveCapacity)
}

foreach ($fix in $fixups) {
    if (-not $labels.ContainsKey($fix.Name)) { throw "label ausente: $($fix.Name)" }
    $target = [uint32]($caveVA + $labels[$fix.Name])
    if ($fix.Kind -eq 'abs') {
        $bytes = U32 $target
    } else {
        $fromNext = [uint32]($caveVA + $fix.Offset + 4)
        $bytes = Rel32 $fromNext $target
    }
    for ($i = 0; $i -lt 4; $i++) { $code[$fix.Offset + $i] = $bytes[$i] }
}

$codeBytes = [byte[]]$code.ToArray()
Set-Bytes $data $caveOffset $codeBytes
$chatTargetVA = [uint32]($caveVA + $labels['chat_entry'])
$frameTargetVA = [uint32]($caveVA + $labels['frame_entry'])
Set-Bytes $data $chatOffset ([byte[]](0xE8) + (Rel32 ($chatVA + 5) $chatTargetVA))
Set-Bytes $data $frameOffset ([byte[]](0xE8) + (Rel32 ($frameVA + 5) $frameTargetVA))

$backup = Join-Path (Split-Path -Parent $Executable) $backupName
if (Test-Path -LiteralPath $backup -PathType Leaf) {
    $backupHash = (Get-FileHash -LiteralPath $backup -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($backupHash -ne $expectedHash) { throw "Backup existente invalido: $backup (SHA-256: $backupHash)." }
} else {
    Copy-Item -LiteralPath $Executable -Destination $backup
}

[IO.File]::WriteAllBytes($Executable, $data)
$patched = [IO.File]::ReadAllBytes($Executable)
Assert-Bytes $patched $chatOffset ([byte[]](0xE8) + (Rel32 ($chatVA + 5) $chatTargetVA)) 'hook de chat (patch)'
Assert-Bytes $patched $frameOffset ([byte[]](0xE8) + (Rel32 ($frameVA + 5) $frameTargetVA)) 'hook de tick (patch)'
$afterHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToUpperInvariant()

Write-Host 'Water Scroll Macro aplicado: comandos locais + scanner carry 0..62 + UseItem nativo.'
Write-Host ("Registros Water: {0}; bytes cave: 0x{1:X}; cave VA: 0x{2:X8}" -f $waterEntries.Count, $code.Count, $caveVA)
Write-Host "SHA-256 antes:  $beforeHash"
Write-Host "SHA-256 depois: $afterHash"
Write-Host "Backup: $backup"
