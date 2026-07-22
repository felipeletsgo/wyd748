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
# registrar o cast: DAT_0092eaf8[skillID*4] = agora. ECX ainda contem skillID
# (setado em 0x00493959, nao clobrado ate aqui; unico caminho de entrada e o JBE).
#
# Como a .text esta cheia (o Patch-WYD748.ps1 ja usou o padding estendido),
# encadeamos dois runs de padding livres existentes:
#   Cave A (0x0040C001, 15 bytes de 0xCC entre duas funcoes):
#       8B 45 F8               MOV EAX,[EBP-0x08]            ; EAX = agora
#       89 04 8D F8 EA 92 00   MOV [ECX*4+0x0092EAF8],EAX    ; cooldown[skillID]=agora
#       E9 rel32               JMP Cave B
#   Cave B (0x0059DD52, 14 bytes de 0x90):
#       A1 E8 71 3B 01         MOV EAX,[0x013B71E8]          ; instrucao deslocada
#       E9 rel32               JMP 0x00493975               ; volta
#   Hook (0x00493970): troca os 5 bytes MOV EAX,[0x013B71E8] por JMP Cave A.
# ============================================================================

# Estado esperado: WYD.exe com os patches principal + ExtendedStats ja aplicados
# (o macro fix vem por cima). Recusa em qualquer outro estado.
$expectedHash = 'F2B8CDEB9863192A5CDA4FCB0E244352A53C4C5E82465E7855B5477C5EC40C5B'

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

# --- offsets de arquivo (file offset = VA - 0x400000 nesta .text) ---
$hookOffset  = 0x93970    # VA 0x00493970  MOV EAX,[0x013B71E8]
$caveAOffset = 0xC001     # VA 0x0040C001  15 x 0xCC
$caveBOffset = 0x19DD52   # VA 0x0059DD52  14 x 0x90

# --- verificacoes do estado original ---
Assert-Bytes $data $hookOffset  ([byte[]](0xA1,0xE8,0x71,0x3B,0x01)) 'hook (MOV EAX,[013B71E8])'
Assert-Bytes $data $caveAOffset ([byte[]](0xCC) * 15) 'cave A (padding 0xCC)'
Assert-Bytes $data $caveBOffset ([byte[]](0x90) * 10) 'cave B (padding 0x90)'

# --- patches (rel32 pre-calculados; conferidos byte a byte) ---
# Hook -> Cave A:  E9 (0x40C001 - 0x493975) = E9 8C 86 F7 FF
Set-Bytes $data $hookOffset ([byte[]](0xE9,0x8C,0x86,0xF7,0xFF))

# Cave A: MOV EAX,[EBP-8]; MOV [ECX*4+0x92EAF8],EAX; JMP Cave B (0x59DD52-0x40C010)
Set-Bytes $data $caveAOffset ([byte[]](
    0x8B,0x45,0xF8,
    0x89,0x04,0x8D,0xF8,0xEA,0x92,0x00,
    0xE9,0x42,0x1D,0x19,0x00
))

# Cave B: MOV EAX,[0x013B71E8] (deslocada); JMP 0x00493975 (volta)
Set-Bytes $data $caveBOffset ([byte[]](
    0xA1,0xE8,0x71,0x3B,0x01,
    0xE9,0x19,0x9C,0xEF,0xFF
))

[IO.File]::WriteAllBytes($Executable, $data)

$newHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
Write-Host 'Macro fix aplicado: rotacao de skills + buffs (cooldown por skill agora e registrado).'
Write-Host "Novo SHA-256: $newHash"
Write-Host 'Se algo quebrar, restaure com: Copy-Item WYD.premacro.exe WYD.exe -Force'
