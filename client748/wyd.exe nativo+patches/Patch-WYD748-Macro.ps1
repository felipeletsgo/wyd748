param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# ============================================================================
# Fix do macro de rotacao/buff do WYD.exe 7.48 (macro improvisado, incompleto).
#
# O macro (FUN_0049362c) decide a proxima skill da rotacao por um cooldown por
# skill, lido de DAT_0092eaf8[skillID*4] ("ultimo instante de cast"). O array e
# LIDO na checagem (0x00493959) mas NUNCA e escrito -> fica sempre 0 -> o 1o slot
# esta sempre "pronto" e dispara todo frame; a rotacao nunca avanca e os buffs
# (slots seguintes) nunca sao alcancados. Sintoma: "so 1 skill, sem rotacao,
# sem buff".
#
# Fix: no ponto do cast (0x00493970, apos o JBE confirmar que a skill vai sair),
# registrar o cast em DAT_0092eaf8[skillID*4]. Para ataques, grava "agora".
# Para buffs reconhecidos pelo macro, grava:
#
#   agora + 150000 ms - cooldown_calculado
#
# Como a checagem seguinte soma novamente o cooldown, o proximo pedido do macro
# ocorre exatamente em 150 s (180 s - janela de renovacao de 30 s). O clique
# manual usa outro fluxo do client e nao passa por este hook.
#
# A code cave fica na area livre executavel da secao .xstat, criada pelo patch
# ExtendedStats. Uma tabela de 96 bits identifica somente os buffs 0..95.
#
# O macro NAO possui um segundo construtor de ataque: depois do hook ele chama
# a rotina nativa 0x004595EC, a mesma familia usada pelo cast manual. Os dois
# ramos dessa rotina selecionam o framing pelo SkillData.MaxTarget:
#
#   MaxTarget == 1 -> 0x39D / 48 bytes
#   MaxTarget == 2 -> 0x39E / 52 bytes
#   demais         -> 0x36C / 96 bytes
#
# As assertions abaixo travam tanto a CALL quanto os dois builders. Assim uma
# linha-base divergente nunca recebe silenciosamente um patch que reintroduza o
# 0x39D/96 observado em executaveis antigos. O servidor conserva essa variante
# apenas como compatibilidade temporaria; ela nao e o formato canonico.
# ============================================================================

# Estado esperado: WYD.exe com os patches principal + ExtendedStats ja aplicados
# (o macro fix vem por cima). Recusa em qualquer outro estado.
$expectedHash = 'E704DA0A185892EBE394324ED3A1001FC0DFB832334546186E61948DED15E20D'

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
function Rel32([uint32]$FromNextInstruction, [uint32]$Target) {
    return [BitConverter]::GetBytes([int32]([int64]$Target - [int64]$FromNextInstruction))
}

$actualHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
if ($actualHash -ne $expectedHash) {
    throw "WYD.exe nao esta no estado esperado (SHA-256: $actualHash). O macro fix so aplica sobre o exe com os patches principal + ExtendedStats ja feitos, antes deste. Se ja aplicou o macro fix, restaure WYD.premacro.exe."
}

# Backup de seguranca (nao existe WYD.original.exe; este exe e o unico).
$backup = Join-Path (Split-Path -Parent $Executable) 'WYD.premacro.exe'
if (-not (Test-Path $backup)) {
    Copy-Item -LiteralPath $Executable -Destination $backup
    Write-Host "Backup criado: $backup"
}

$data = [IO.File]::ReadAllBytes($Executable)

# --- enderecos/offsets ---
$hookOffset = 0x93970       # VA 0x00493970 MOV EAX,[0x013B71E8]
$hookVA = [uint32]0x00493970
$caveOffset = 0x1D3207      # .xstat RVA 0xFC0207
$caveVA = [uint32]0x013C0207
$buffBitsOffset = $caveOffset + 0x30
$buffBitsVA = [uint32]($caveVA + 0x30)

# --- verificacoes do estado original ---
Assert-Bytes $data $hookOffset  ([byte[]](0xA1,0xE8,0x71,0x3B,0x01)) 'hook (MOV EAX,[013B71E8])'
Assert-Bytes $data $caveOffset ([byte[]](0x00) * 64) 'macro cave (.xstat livre)'
Assert-Bytes $data 0x939B3 ([byte[]](0xE8,0x34,0x5C,0xFC,0xFF)) 'macro -> cast nativo 0x004595EC'

# Primeiro builder dentro de 0x004595EC.
Assert-Bytes $data 0x5CB53 ([byte[]](0xC7,0x85,0x20,0xFD,0xFF,0xFF,0x60,0x00,0x00,0x00)) 'builder A tamanho multi 96'
Assert-Bytes $data 0x5CB64 ([byte[]](0x83,0xB9,0x44,0xFF,0x92,0x00,0x01)) 'builder A MaxTarget == 1'
Assert-Bytes $data 0x5CB6D ([byte[]](0x66,0xC7,0x85,0x68,0xFF,0xFF,0xFF,0x9D,0x03)) 'builder A opcode 0x39D'
Assert-Bytes $data 0x5CB76 ([byte[]](0xC7,0x85,0x20,0xFD,0xFF,0xFF,0x30,0x00,0x00,0x00)) 'builder A tamanho single 48'

# Segundo builder dentro de 0x004595EC.
Assert-Bytes $data 0x5E26E ([byte[]](0xC7,0x85,0x70,0xFC,0xFF,0xFF,0x60,0x00,0x00,0x00)) 'builder B tamanho multi 96'
Assert-Bytes $data 0x5E27F ([byte[]](0x83,0xBA,0x44,0xFF,0x92,0x00,0x01)) 'builder B MaxTarget == 1'
Assert-Bytes $data 0x5E288 ([byte[]](0x66,0xC7,0x85,0xAC,0xFB,0xFF,0xFF,0x9D,0x03)) 'builder B opcode 0x39D'
Assert-Bytes $data 0x5E291 ([byte[]](0xC7,0x85,0x70,0xFC,0xFF,0xFF,0x30,0x00,0x00,0x00)) 'builder B tamanho single 48'

# Hook -> macro cave.
$hook = [byte[]](0xE9) + [byte[]](Rel32 ($hookVA + 5) $caveVA)
Set-Bytes $data $hookOffset $hook

# IDs de buff reconhecidos pela rotina 0x0049362C:
# TK 5,11,13,15; Foema 37,41,43-46; BM 53,54,64,66,68,70,71;
# Huntress 76,77,87,89.
$buffBits = [byte[]](0x00,0x28,0xA0,0x00,0x00,0x2A,0x70,0x60,0x15,0xC0,0x30,0x82)
Set-Bytes $data $buffBitsOffset $buffBits

# MOV EAX,[EBP-8]                  ; agora
# BT  DWORD PTR [buffBits],ECX     ; CF=1 para buff
# JNC normal
# SUB EAX,[EBP-40h]                ; remove cooldown que a checagem somara
# ADD EAX,150000                   ; proximo pedido em 150 s
# normal:
# MOV [ECX*4+0092EAF8h],EAX
# MOV EAX,[013B71E8h]              ; instrucao deslocada
# JMP 00493975h
$code = [byte[]](
    0x8B,0x45,0xF8,
    0x0F,0xA3,0x0D
) + [BitConverter]::GetBytes($buffBitsVA) + [byte[]](
    0x73,0x08,
    0x2B,0x45,0xC0,
    0x05,0xF0,0x49,0x02,0x00,
    0x89,0x04,0x8D,0xF8,0xEA,0x92,0x00,
    0xA1,0xE8,0x71,0x3B,0x01,
    0xE9
)
$returnFrom = [uint32]($caveVA + $code.Length + 4)
$code += [byte[]](Rel32 $returnFrom ([uint32]0x00493975))
Set-Bytes $data $caveOffset $code

[IO.File]::WriteAllBytes($Executable, $data)

$newHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
Write-Host 'Macro fix aplicado: rotacao de skills + buffs (cooldown por skill agora e registrado).'
Write-Host "Novo SHA-256: $newHash"
Write-Host 'Se algo quebrar, restaure com: Copy-Item WYD.premacro.exe WYD.exe -Force'
