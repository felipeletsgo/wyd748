param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Patch version-locked para o WYD 7.48 de 23/04/2007.
$originalHash = 'B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C'
$backup = Join-Path (Split-Path -Parent $Executable) 'WYD.original.exe'

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

function Rel32([int]$FromVA, [int]$ToVA) {
    return [BitConverter]::GetBytes([int]($ToVA - ($FromVA + 5)))
}

$actualHash = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash
if ($actualHash -ne $originalHash) {
    throw "WYD.exe já alterado ou incompatível (SHA-256: $actualHash). Restaure WYD.original.exe antes de reaplicar."
}

$data = [IO.File]::ReadAllBytes($Executable)

# Expõe formalmente os 62 bytes finais já presentes no RawSize da seção .text.
# O helper fica nesse padding; as flags da seção já são CODE|EXECUTE|READ.
$textVirtualSizeOffset = 0x228
Assert-Bytes $data $textVirtualSizeOffset ([byte[]](0xC2,0x1F,0x1A,0x00)) 'VirtualSize .text'
$textVirtualSizePatch = [byte[]](0x00,0x20,0x1A,0x00)

# 1) Câmera: remove completamente o caminho Alt/botão direito. Se o botão
# central não estiver pressionado, salta a rotação; se estiver, entra nela.
$cameraOffset = 0xAEEB1
$cameraExpected = [byte[]](
    0x74,0x0E,0xA1,0xE0,0x71,0x3B,0x01,0x83,
    0xB8,0x0C,0x01,0x00,0x00,0x01,0x74,0x35
)
$cameraPatch = [byte[]](
    0x0F,0x84,0xB3,0x01,0x00,0x00, # JE  0x004AF06A: sem botão central
    0xE9,0x3A,0x00,0x00,0x00,      # JMP 0x004AEEF6: botão central pressionado
    0x90,0x90,0x90,0x90,0x90
)
Assert-Bytes $data $cameraOffset $cameraExpected 'camera'

# O campo de cena ainda usa CAMERAROTATE para decidir o destino do clique
# direito. Forca o ramo WM_RBUTTONDOWN a chamar SkillUse; a camera ja esta
# isolada no botao central pelo patch acima.
$rightClickOffset = 0x63179
$rightClickExpected = [byte[]](0x75,0x3E) # JNE 0x004631B9
$rightClickPatch = [byte[]](0xEB,0x3E)    # JMP 0x004631B9
Assert-Bytes $data $rightClickOffset $rightClickExpected 'botao direito/SkillUse'

# 2) Dano: os três locais abaixo originalmente fazem PUSH 3 (amarelo).
# Cada um chama o helper no padding executável final da seção .text.
$helperVA = 0x005A2FC2
$titleOffset = 0x1C5069
# A linha-base pre-ExtendedStats historicamente versionada traz 0x78 no
# tÃ­tulo interno, embora WYD.original.exe tenha 0x69. Registrar esse byte
# aqui torna a cadeia reproduzÃ­vel e elimina a ediÃ§Ã£o manual nÃ£o auditada.
Assert-Bytes $data $titleOffset ([byte[]](0x69)) 'tÃ­tulo da linha-base'
$titlePatch = [byte[]](0x78)
$damageSites = @(
    @{ Offset = 0x8FA08; VA = 0x0048FA08; Expected = [byte[]](0x6A,0x03,0x6A,0x00,0x68) },
    @{ Offset = 0x91AA5; VA = 0x00491AA5; Expected = [byte[]](0x6A,0x03,0x8B,0x95,0x68) },
    @{ Offset = 0x91DB4; VA = 0x00491DB4; Expected = [byte[]](0x6A,0x03,0x8B,0x85,0x68) }
)
foreach ($site in $damageSites) {
    Assert-Bytes $data $site.Offset $site.Expected ('dano 0x{0:X8}' -f $site.VA)
}

$helperOffset = 0x1A2FC2
Assert-Bytes $data $helperOffset ([byte[]](0) * 58) 'code cave principal'

$stubVA = 0x0058A7A0
$stubOffset = 0x18A7A0
Assert-Bytes $data $stubOffset ([byte[]](0xCC) * 23) 'code cave auxiliar'

# Helper comum:
# - retira da pilha o retorno do CALL;
# - SkillIndex -1/outside 0..95 => PUSH 3 (amarelo/físico);
# - SkillIndex 0..95 => PUSH 2 (azul/skill);
# - recompõe os bytes sobrescritos de cada um dos três call sites.
$ret1 = 0x0048FA0D
$ret2 = 0x00491AAA
$stub1VA = $stubVA
$stub2VA = $stubVA + 12

$helper = New-Object System.Collections.Generic.List[byte]
$helper.AddRange([byte[]](0x5A,0x8B,0x45,0xF0,0x0F,0xBF,0x40,0x18,0x85,0xC0,0x7C,0x09))
$helper.AddRange([byte[]](0x83,0xF8,0x60,0x7D,0x04,0x6A,0x02,0xEB,0x02,0x6A,0x03))
$helper.AddRange([byte[]](0x81,0xFA)); $helper.AddRange([BitConverter]::GetBytes([int]$ret1))
$helper.AddRange([byte[]](0x0F,0x84)); $helper.AddRange([BitConverter]::GetBytes([int]($stub1VA - ($helperVA + $helper.Count + 4))))
$helper.AddRange([byte[]](0x81,0xFA)); $helper.AddRange([BitConverter]::GetBytes([int]$ret2))
$helper.AddRange([byte[]](0x0F,0x84)); $helper.AddRange([BitConverter]::GetBytes([int]($stub2VA - ($helperVA + $helper.Count + 4))))
$helper.AddRange([byte[]](0x8B,0x85,0x68,0xF7,0xFF,0xFF,0x83,0xC2,0x03,0xFF,0xE2))
if ($helper.Count -gt 58) { throw "Helper excedeu o code cave: $($helper.Count) bytes" }

# Stub 1 recompõe PUSH 0 / PUSH 1500 do primeiro ponto.
$stub1 = [byte[]](0x6A,0x00,0x68,0xDC,0x05,0x00,0x00,0x83,0xC2,0x04,0xFF,0xE2)
# Stub 2 recompõe MOV EDX,[EBP-898] do segundo ponto.
$stub2 = [byte[]](0x8B,0x95,0x68,0xF7,0xFF,0xFF,0x83,0xC2,0x03,0xFF,0xE2)

if (-not (Test-Path -LiteralPath $backup)) {
    [IO.File]::WriteAllBytes($backup, $data)
}

Set-Bytes $data $cameraOffset $cameraPatch
Set-Bytes $data $rightClickOffset $rightClickPatch
Set-Bytes $data $textVirtualSizeOffset $textVirtualSizePatch
Set-Bytes $data $titleOffset $titlePatch
Set-Bytes $data $helperOffset $helper.ToArray()
Set-Bytes $data $stubOffset $stub1
Set-Bytes $data ($stubOffset + $stub1.Length) $stub2
foreach ($site in $damageSites) {
    $call = New-Object System.Collections.Generic.List[byte]
    $call.Add(0xE8)
    $call.AddRange([byte[]](Rel32 $site.VA $helperVA))
    Set-Bytes $data $site.Offset $call.ToArray()
}

[IO.File]::WriteAllBytes($Executable, $data)
Write-Host "WYD.exe corrigido. Backup: $backup"
Write-Host 'Mouse: esquerdo=físico, direito=skill, central=câmera.'
Write-Host 'Dano: físico=amarelo, skills=azul.'
