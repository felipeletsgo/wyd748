[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [switch]$VerifyOnly,
    [switch]$ResetFromOriginal
)

$ErrorActionPreference = 'Stop'

# PONTO UNICO DE ENTRADA — client 7.48
#
# Este orquestrador e o unico script que deve ser executado normalmente. Os
# cinco scripts Patch-WYD748-*.ps1 abaixo continuam separados de proposito:
# cada um implementa um elo pequeno, valida os bytes originais e possui um
# SHA de entrada/saida proprio. Isso preserva rollback e impede que uma
# alteracao em um patch esconda uma falha em outro. Nao executar os elos fora
# desta ordem.
#
# 1. Patch-WYD748.ps1              base: camera, mouse e cores de dano
# 2. Patch-WYD748-ExtendedStats.ps1 score/HP/MP/dano 32-bit (.xstat)
# 3. Patch-WYD748-Bypass.ps1       versao + checksums de SkillData/ItemList
# 4. Patch-WYD748-Macro.ps1        rotacao de skills e renovacao de buffs
# 5. Patch-WYD748-Lindy.ps1        receita 3448 da janela da Lindy
# D. Patch-WYD748-ClientItemUse.ps1 marcador de clique do Warrior's Seal
#    (ItemList.bin; nao altera a cadeia SHA do executavel)
#
# O progresso e retomavel: se o processo for interrompido, a proxima execucao
# identifica o SHA atual e continua do elo correspondente. O modo
# -VerifyOnly apenas confere a cadeia e nunca escreve o executavel.

function Get-Sha([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

$original = Join-Path (Split-Path -Parent $Executable) 'WYD.original.exe'
if ($ResetFromOriginal) {
    if (-not (Test-Path -LiteralPath $original -PathType Leaf)) {
        throw "Nao existe WYD.original.exe para reiniciar a cadeia: $original"
    }
    if (Test-Path -LiteralPath $Executable -PathType Leaf) {
        $beforeReset = Get-Sha $Executable
        $resetBackup = Join-Path (Split-Path -Parent $Executable) 'WYD.before-reset.exe'
        if (Test-Path -LiteralPath $resetBackup -PathType Leaf) {
            $backupHash = Get-Sha $resetBackup
            if ($backupHash -ne $beforeReset) {
                throw "Backup de reset existente nao corresponde ao WYD.exe atual: $resetBackup"
            }
        } else {
            Copy-Item -LiteralPath $Executable -Destination $resetBackup
        }
    }
    Copy-Item -LiteralPath $original -Destination $Executable -Force
} elseif (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
    throw "Executavel nao encontrado: $Executable. Use -ResetFromOriginal para iniciar da copia WYD.original.exe."
}

$steps = @(
    # ETAPA 1 — base de compatibilidade do client original.
    [pscustomobject]@{
        Number = 1; Name = 'base (camera/mouse/dano)'; Script = 'Patch-WYD748.ps1'
        Input = 'B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C'
        Output = '2AA1773AAC6A7EE186CD62B3B609004AD63D9A03E4B685CAE7E1B4AC2BB521EE'
    },
    # ETAPA 2 — campos estendidos usados pelo servidor autoritativo.
    [pscustomobject]@{
        Number = 2; Name = 'extended stats (.xstat)'; Script = 'Patch-WYD748-ExtendedStats.ps1'
        Input = '2AA1773AAC6A7EE186CD62B3B609004AD63D9A03E4B685CAE7E1B4AC2BB521EE'
        Output = '4724B76F19702645D6052A9D7437971135376DDCF55E94FC934F44C349216234'
    },
    # ETAPA 3 — inclui a verificacao inicial do ItemList em 0x14FAD0.
    [pscustomobject]@{
        Number = 3; Name = 'bypass de versao/checksums'; Script = 'Patch-WYD748-Bypass.ps1'
        Input = '4724B76F19702645D6052A9D7437971135376DDCF55E94FC934F44C349216234'
        Output = 'E704DA0A185892EBE394324ED3A1001FC0DFB832334546186E61948DED15E20D'
    },
    # ETAPA 4 — registro de cooldown por skill no macro nativo.
    [pscustomobject]@{
        Number = 4; Name = 'macro de skills/buffs'; Script = 'Patch-WYD748-Macro.ps1'
        Input = 'E704DA0A185892EBE394324ED3A1001FC0DFB832334546186E61948DED15E20D'
        Output = '4E916C1FD94D60D5EF7F8914B621BAB3787E7BF5460FB251C59F71BCC4D9BA2F'
    },
    # ETAPA 5 — constante 4010 -> 3448 da receita nativa da Lindy.
[pscustomobject]@{
    Number = 5; Name = 'receita da Lindy'; Script = 'Patch-WYD748-Lindy.ps1'
    Input = '4E916C1FD94D60D5EF7F8914B621BAB3787E7BF5460FB251C59F71BCC4D9BA2F'
    Output = '9762B1AC6EFB4AB3C800877DE1DA048DD43EA407FCEEA945C755DF6986607F18'
}
)

$current = Get-Sha $Executable
$legacyWaterMacroHashes = @(
    '65486F2A4ED791BA977C00D1478BFA6450783DA37BC54A8039F196B8A73E0A0E',
    'F76D9D8CEDFFBD3E046F10C5282CF0139E6D94BFC7DF30BCCA549324B0D1107E'
)
if ($legacyWaterMacroHashes -contains $current) {
    if ($VerifyOnly) {
        Write-Host "WaterMacro legado detectado ($current). Execute sem -VerifyOnly para reconstruir o client ate o elo Lindy."
        return
    }
    if (-not (Test-Path -LiteralPath $original -PathType Leaf)) {
        throw "WaterMacro legado detectado, mas WYD.original.exe nao existe para reconstruir a cadeia."
    }
    $legacyBackup = Join-Path (Split-Path -Parent $Executable) 'WYD.pre-server-water.exe'
    if (-not (Test-Path -LiteralPath $legacyBackup -PathType Leaf)) {
        Copy-Item -LiteralPath $Executable -Destination $legacyBackup
    }
    Copy-Item -LiteralPath $original -Destination $Executable -Force
    $current = Get-Sha $Executable
    Write-Host 'WaterMacro client-side removido; reconstruindo os cinco elos suportados.'
}
$final = $steps[-1].Output
if ($current -eq $final) {
    $itemUseScript = Join-Path $PSScriptRoot 'Patch-WYD748-ClientItemUse.ps1'
    if (-not (Test-Path -LiteralPath $itemUseScript -PathType Leaf)) {
        throw "Elo de compatibilidade do ItemList ausente: $itemUseScript"
    }
    if ($VerifyOnly) {
        & $itemUseScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin') -VerifyOnly
    } else {
        & $itemUseScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
    }
    Write-Host "Cadeia WYD 7.48 ja concluida: $current"
    return
}

$index = -1
for ($i = 0; $i -lt $steps.Count; $i++) {
    if ($steps[$i].Input -eq $current) { $index = $i; break }
}
if ($index -lt 0) {
    $known = ($steps | ForEach-Object { "etapa $($_.Number): entrada=$($_.Input.Substring(0,12)) saida=$($_.Output.Substring(0,12))" }) -join '; '
    throw "WYD.exe esta fora da cadeia suportada (SHA-256: $current). $known"
}

if ($VerifyOnly) {
    Write-Host "Estado valido para continuar na etapa $($steps[$index].Number): $($steps[$index].Name)."
    return
}

for ($i = $index; $i -lt $steps.Count; $i++) {
    $step = $steps[$i]
    $scriptPath = Join-Path $PSScriptRoot $step.Script
    if (-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
        throw "Elo ausente: $scriptPath"
    }
    $before = Get-Sha $Executable
    if ($before -ne $step.Input) {
        throw "Estado mudou antes da etapa $($step.Number): esperado $($step.Input), encontrado $before"
    }
    Write-Host "[$($step.Number)/$($steps.Count)] $($step.Name)"
    & $scriptPath -Executable $Executable
    $after = Get-Sha $Executable
    if ($after -ne $step.Output) {
        throw "Saida inesperada na etapa $($step.Number): esperado $($step.Output), encontrado $after"
    }
}

Write-Host "Cadeia WYD 7.48 concluida: $(Get-Sha $Executable)"

# ItemList.bin nao participa da cadeia SHA do executavel: ele e um artefato de
# dados e possui checksum proprio. Aplicar depois dos elos permite repetir o
# orquestrador sem reescrever o binario ja concluido.
$itemUseScript = Join-Path $PSScriptRoot 'Patch-WYD748-ClientItemUse.ps1'
if (-not (Test-Path -LiteralPath $itemUseScript -PathType Leaf)) {
    throw "Elo de compatibilidade do ItemList ausente: $itemUseScript"
}
& $itemUseScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
