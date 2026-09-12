param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Quinto e ultimo elo da cadeia do client 7.48.
# A Lindy usa uma constante compilada para o primeiro material da receita.
# O 7.54 e o servidor aceitam dois pacotes 3448 (EF_AMOUNT=10), mas o
# executavel 7.48 original compara o primeiro material com 4010.
#
# VA 0x00413FB7 / file offset 0x13FB7
# Antes: 81 FA AA 0F 00 00  (cmp edx,4010)
# Depois: 81 FA 78 0D 00 00 (cmp edx,3448)

$expectedHash = '4E916C1FD94D60D5EF7F8914B621BAB3787E7BF5460FB251C59F71BCC4D9BA2F'
$backupName = 'WYD.pre-lindy.exe'

function Assert-Bytes([byte[]]$Data, [int]$Offset, [byte[]]$Expected, [string]$Name) {
    if ($Offset -lt 0 -or $Offset + $Expected.Length -gt $Data.Length) {
        throw "${Name}: offset fora do executavel."
    }
    for ($i = 0; $i -lt $Expected.Length; $i++) {
        if ($Data[$Offset + $i] -ne $Expected[$i]) {
            throw ('{0}: byte inesperado em 0x{1:X}: esperado {2:X2}, encontrado {3:X2}. Patch cancelado.' -f
                $Name, ($Offset + $i), $Expected[$i], $Data[$Offset + $i])
        }
    }
}

function Set-Bytes([byte[]]$Data, [int]$Offset, [byte[]]$Value) {
    [Array]::Copy($Value, 0, $Data, $Offset, $Value.Length)
}

if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
    throw "Executavel nao encontrado: $Executable"
}

$beforeHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToUpperInvariant()
if ($beforeHash -ne $expectedHash) {
    throw "Entrada inesperada (SHA-256: $beforeHash). Restaure WYD.pre-lindy.exe e reaplique somente sobre a saida pre-Lindy da cadeia."
}

$data = [IO.File]::ReadAllBytes($Executable)
$offset = 0x13FB7
$before = [byte[]](0x81, 0xFA, 0xAA, 0x0F, 0x00, 0x00)
$after = [byte[]](0x81, 0xFA, 0x78, 0x0D, 0x00, 0x00)
Assert-Bytes $data $offset $before 'Lindy material (original)'

$backup = Join-Path (Split-Path -Parent $Executable) $backupName
if (Test-Path -LiteralPath $backup -PathType Leaf) {
    $backupHash = (Get-FileHash -LiteralPath $backup -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($backupHash -ne $expectedHash) {
        throw "Backup existente invalido: $backup (SHA-256: $backupHash). Nao sera sobrescrito."
    }
} else {
    Copy-Item -LiteralPath $Executable -Destination $backup
}

Set-Bytes $data $offset $after
[IO.File]::WriteAllBytes($Executable, $data)

$patched = [IO.File]::ReadAllBytes($Executable)
Assert-Bytes $patched $offset $after 'Lindy material (patched)'
$afterHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToUpperInvariant()

Write-Host 'Fix da Lindy aplicado com sucesso.'
Write-Host '4010 -> 3448'
Write-Host 'Offset: 0x13FB7 (VA 0x00413FB7)'
Write-Host "SHA-256 antes:  $beforeHash"
Write-Host "SHA-256 depois: $afterHash"
Write-Host "Backup: $backup"
