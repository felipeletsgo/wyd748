param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# ============================================================================
# Fix da janela de combinacao da Lindy no WYD.exe 7.48.
#
# A rotina do client compara o primeiro material com o item 4010 em
# VA 0x00413FB7 (file offset 0x13FB7):
#
#   81 FA AA 0F 00 00    cmp edx,4010
#
# O servidor e a receita-alvo 7.54 usam dois pacotes do item 3448, cada um com
# EF_AMOUNT=10. Como a validacao da janela e client-side, a receita correta era
# recusada antes do pacote de combine chegar ao servidor.
#
# Este patch muda somente a constante 4010 -> 3448:
#
#   81 FA 78 0D 00 00    cmp edx,3448
#
# Os compares proximos com 4127, 413 e 4141 pertencem a outros ramos da mesma
# rotina e NAO sao alterados: nao ha evidencia de que sejam defeitos.
# ============================================================================

# Estado esperado: WYD.exe atual depois de principal + ExtendedStats + Bypass +
# Macro. Esta e a SHA-256 documentada em PATCHES.md antes do fix da Lindy.
$expectedHash = 'B2678AB927F03BF0F3114F36AE682025A9C732D2A59B55FB5B26DECEE07F2F94'

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
    throw "WYD.exe nao esta no estado esperado para o fix da Lindy (SHA-256: $actualHash). Reaplique a cadeia ate Patch-WYD748-Macro.ps1 antes deste patch."
}

$data = [IO.File]::ReadAllBytes($Executable)

$lindyOffset = 0x13FB7
$before = [byte[]](0x81,0xFA,0xAA,0x0F,0x00,0x00) # cmp edx,4010
$after  = [byte[]](0x81,0xFA,0x78,0x0D,0x00,0x00) # cmp edx,3448

Assert-Bytes $data $lindyOffset $before 'Lindy material check'

$backup = Join-Path (Split-Path -Parent $Executable) 'WYD.pre-lindy.exe'
if (-not (Test-Path $backup)) {
    Copy-Item -LiteralPath $Executable -Destination $backup
    Write-Host "Backup criado: $backup"
}

Set-Bytes $data $lindyOffset $after
[IO.File]::WriteAllBytes($Executable, $data)

# Verificacao pos-escrita: garante que o patch alterou exatamente a constante
# esperada antes de reportar sucesso.
$patched = [IO.File]::ReadAllBytes($Executable)
Assert-Bytes $patched $lindyOffset $after 'Lindy material check (patched)'

$newHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
Write-Host 'Fix da Lindy aplicado: material client-side 4010 -> 3448.'
Write-Host 'Offset: 0x13FB7 (VA 0x00413FB7)'
Write-Host "Novo SHA-256: $newHash"
Write-Host 'Se algo quebrar, restaure com: Copy-Item WYD.pre-lindy.exe WYD.exe -Force'
