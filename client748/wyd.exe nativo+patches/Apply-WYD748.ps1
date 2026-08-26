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
# nove scripts Patch-WYD748-*.ps1 abaixo continuam separados de proposito:
# cada um implementa um elo pequeno, valida os bytes originais e possui um
# SHA de entrada/saida proprio. Isso preserva rollback e impede que uma
# alteracao em um patch esconda uma falha em outro. Nao executar os elos fora
# desta ordem.
#
# 1. Patch-WYD748.ps1              base: camera, mouse e cores de dano
# 2.  score/HP/MP/dano 32-bit (.xstat)
# 3. Patch-WYD748-Bypass.ps1       versao + checksums de SkillData/ItemList
# 4. Patch-WYD748-Macro.ps1        rotacao de skills e renovacao de buffs
# 5. Patch-WYD748-Lindy.ps1        receita 3448 da janela da Lindy
# 6. Patch-WYD748-Costumes.ps1     colecao de trajes do client KR
# 7. Patch-WYD748-KRMounts.ps1      montarias visuais do client KR (.mountkr)
# 8. Patch-WYD748-KRMobs.ps1        classes visuais 66..74 dos monstros KR
# 9. Patch-WYD748-KRMobPoses.ps1    orientacao dos skeletons modernos KR
# D. Patch-WYD748-ClientItemUse.ps1 marcador de clique do Warrior's Seal
# D. Patch-WYD748-CostumeItems.ps1 registros/icon dos trajes KR completos
# D. Patch-WYD748-CostumeTextures.ps1 registros das texturas KR
# D. Patch-WYD748-KRMountItems.ps1  clones da Shire para Equip[14]
# D. Patch-WYD748-KRMountAssets.ps1 meshes e animacoes das montarias KR
# D. Patch-WYD748-KRMobItems.ps1     faces KR traduzidas para o ItemList 7.48
# D. Patch-WYD748-KRMobAssets.ps1    meshes, texturas, bones e animacoes KR
# D. Patch-WYD748-ItemGrid1x1.ps1    normaliza todo EF_GRID para uma celula
#    (ItemList/MeshTextureList; nao alteram a cadeia SHA do executavel)
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
                $resetBackup = Join-Path (Split-Path -Parent $Executable) ("WYD.before-reset.{0}.exe" -f $beforeReset.Substring(0,12))
                if (Test-Path -LiteralPath $resetBackup -PathType Leaf) {
                    if ((Get-Sha $resetBackup) -ne $beforeReset) {
                        throw "Backup versionado de reset divergente: $resetBackup"
                    }
                } else {
                    Copy-Item -LiteralPath $Executable -Destination $resetBackup
                }
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
        Number = 2; Name = 'extended stats (.xstat)'; Script = ''
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
    },
    # ETAPA 6 - seletor e renderer da colecao de trajes importada do client KR.
    [pscustomobject]@{
        Number = 6; Name = 'colecao de trajes KR'; Script = 'Patch-WYD748-Costumes.ps1'
        Input = '9762B1AC6EFB4AB3C800877DE1DA048DD43EA407FCEEA945C755DF6986607F18'
        Output = '4A2AA37228A720ED389F5AC8A5978329855932B93E54FA0501B51A3A23316DEF'
    },
    # ETAPA 7 - costumes de montaria KR materializados como montarias Shire.
    [pscustomobject]@{
        Number = 7; Name = 'colecao de montarias KR'; Script = 'Patch-WYD748-KRMounts.ps1'
        Input = '4A2AA37228A720ED389F5AC8A5978329855932B93E54FA0501B51A3A23316DEF'
        Output = '79B66BFF4E8D31D0788D857AD6AF3DE7F95DC7A07C7256D134A6DD5708EAA4AE'
    },
    # ETAPA 8 - classes visuais e skeleton types das criaturas KR.
    [pscustomobject]@{
        Number = 8; Name = 'faces de monstros KR'; Script = 'Patch-WYD748-KRMobs.ps1'
        Input = '79B66BFF4E8D31D0788D857AD6AF3DE7F95DC7A07C7256D134A6DD5708EAA4AE'
        Output = 'B3F385739C232275FE08FACAE0152ECDFD97D16D111C43D25E7277869FF5422B' # KRMOB_CHAIN_OUTPUT_HASH
    },
    # ETAPA 9 - os skeletons modernos 45..57 usam eixo diferente no 7.48.
    [pscustomobject]@{
        Number = 9; Name = 'orientacao das faces KR'; Script = 'Patch-WYD748-KRMobPoses.ps1'
        Input = 'B3F385739C232275FE08FACAE0152ECDFD97D16D111C43D25E7277869FF5422B'
        Output = '8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593' # KRMOB_POSE_OUTPUT_HASH
    }
)

$current = Get-Sha $Executable
$legacyKRVisualHashes = @(
    # Build anterior a importacao das montarias premium reais do catalogo KR.
    '63935115018E39179C254C70AE4062B2449E523CCED852B2E8A4B3ACBBD70CB8',
    # Build de 47 montarias anterior a disponibilidade estrita e offsets KR nativos.
    'F6F99CC0405654629D9867C84F6587B2064B30D58F67A2151E1ACD36F394E72D',
    # Build anterior ao marcador exclusivo de trajes e ao suporte de R para
    # todas as montarias KR.
    '738FEBA396B4273721F63B17F044DA0AEFFAC20ECF9EFA195BE603E427775D53',
    # Colecao anterior ao culling seletivo das meshes esqueletais KR.
    '93418B98E42877596ACC4D52D518475059358B5954A7D7CBD38F97E5B9BEFE63',
    # Trajes sem o gate de admissao para IDs modernos.
    '273540C8755E75D2412CB513AB8A2D26C169E90EF37342587AE8B37B8476D8F5',
    # Montarias antes do refresh incremental de Equip[14].
    '63FFDF621631803BE4096730B0096F7617DB73D4DE517078A4EC2A49B17D96DF',
    # Montarias com refresh, ainda sobre o patch antigo de trajes.
    '517B8C2F4438B8A5DF1A3C4CDE1D645F69373A9B9FEF8CA04DCD554312B768FE',
    # Colecao anterior ao rebuild do ID compacto de trajes no mundo.
    '677D2D209076708539F174657498D119FB9860A0A8BD39940103C4F1BAE6905B',
    'A5DCF57BD8C7D891E42796A7EBC028A3DD22DA7EF53C1A8DB0E023BAB3BCE8A3',
    # Colecao com IDs compactos corrigidos, antes de preservar a face nativa.
    '03EC8FA4DFEF7994A079BFEB491CDD4812A39FB7F29B44A71D964728BE3AB3E9',
    '6D853FE2F69EDFFB4A1EC4793EE18CF651B5E5604E3009703D379C2B182B76CB',
    # Colecao com materiais/rotacao corrigidos, antes do assento type 59.
    'E1C34874E8BA5B4CF018F262D84A581DCA2242DD7F67410B4807B57BCC3691EA',
    # Colecao que sobrescrevia m_nSkinMeshType com 0/1 e desmontava o corpo.
    '78B27091ACF3B0DA0258E7F7510E55CA3A78C721C237F5B99F767CB780512005',
    # Montarias com pose corrigida, ainda marcadas globalmente como two-sided.
    '3D3270952B10D3BFDDD4B07A3F19D0FD2E211EE4733C9F0369F377B30A2F0B15'
)
if ($legacyKRVisualHashes -contains $current) {
    if ($VerifyOnly) {
        Write-Host "Colecao visual KR antiga detectada ($current). Execute sem -VerifyOnly depois de fechar o WYD.exe."
        return
    }
    if (-not (Test-Path -LiteralPath $original -PathType Leaf)) {
        throw 'Colecao visual KR antiga detectada, mas WYD.original.exe nao existe para reconstruir a cadeia.'
    }
    Copy-Item -LiteralPath $original -Destination $Executable -Force
    $current = Get-Sha $Executable
    Write-Host 'Reconstruindo os nove elos para aplicar a admissao dos trajes modernos.'
}
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
    Write-Host 'WaterMacro client-side removido; reconstruindo os nove elos suportados.'
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
    $costumeItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-CostumeItems.ps1'
    if (-not (Test-Path -LiteralPath $costumeItemScript -PathType Leaf)) {
        throw "Elo de dados dos trajes KR ausente: $costumeItemScript"
    }
    if ($VerifyOnly) {
        & $costumeItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin') -VerifyOnly
    } else {
        & $costumeItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
    }
    $costumeTextureScript = Join-Path $PSScriptRoot 'Patch-WYD748-CostumeTextures.ps1'
    if (-not (Test-Path -LiteralPath $costumeTextureScript -PathType Leaf)) {
        throw "Elo de texturas dos trajes KR ausente: $costumeTextureScript"
    }
    $meshTextureList = Join-Path $PSScriptRoot 'mesh\MeshTextureList.bin'
    if ($VerifyOnly) {
        & $costumeTextureScript -MeshTextureList $meshTextureList -VerifyOnly
    } else {
        & $costumeTextureScript -MeshTextureList $meshTextureList
    }
    $mountItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMountItems.ps1'
    $mountAssetScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMountAssets.ps1'
    $mobItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMobItems.ps1'
    $mobAssetScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMobAssets.ps1'
    $gridItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-ItemGrid1x1.ps1'
    foreach ($script in @($mountItemScript, $mountAssetScript, $mobItemScript, $mobAssetScript)) {
        if (-not (Test-Path -LiteralPath $script -PathType Leaf)) { throw "Elo de montarias KR ausente: $script" }
    }
    if (-not (Test-Path -LiteralPath $gridItemScript -PathType Leaf)) {
        throw "Elo de normalizacao 1x1 do ItemList ausente: $gridItemScript"
    }
    if ($VerifyOnly) {
        & $mountItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin') -VerifyOnly
        & $mountAssetScript -ClientRoot $PSScriptRoot -VerifyOnly
        & $mobItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin') -VerifyOnly
        & $mobAssetScript -ClientRoot $PSScriptRoot -VerifyOnly
        # Run after every ItemList writer so imported records cannot reintroduce
        # a legacy multi-cell EF_GRID value.
        & $gridItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin') -VerifyOnly
    } else {
        & $mountItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
        & $mountAssetScript -ClientRoot $PSScriptRoot
        & $mobItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
        & $mobAssetScript -ClientRoot $PSScriptRoot
        & $gridItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
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
$costumeItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-CostumeItems.ps1'
if (-not (Test-Path -LiteralPath $costumeItemScript -PathType Leaf)) {
    throw "Elo de dados dos trajes KR ausente: $costumeItemScript"
}
& $costumeItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
$costumeTextureScript = Join-Path $PSScriptRoot 'Patch-WYD748-CostumeTextures.ps1'
if (-not (Test-Path -LiteralPath $costumeTextureScript -PathType Leaf)) {
    throw "Elo de texturas dos trajes KR ausente: $costumeTextureScript"
}
& $costumeTextureScript -MeshTextureList (Join-Path $PSScriptRoot 'mesh\MeshTextureList.bin')
$mountItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMountItems.ps1'
$mountAssetScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMountAssets.ps1'
$mobItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMobItems.ps1'
$mobAssetScript = Join-Path $PSScriptRoot 'Patch-WYD748-KRMobAssets.ps1'
$gridItemScript = Join-Path $PSScriptRoot 'Patch-WYD748-ItemGrid1x1.ps1'
foreach ($script in @($mountItemScript, $mountAssetScript, $mobItemScript, $mobAssetScript)) {
    if (-not (Test-Path -LiteralPath $script -PathType Leaf)) { throw "Elo de montarias KR ausente: $script" }
}
if (-not (Test-Path -LiteralPath $gridItemScript -PathType Leaf)) {
    throw "Elo de normalizacao 1x1 do ItemList ausente: $gridItemScript"
}
& $mountItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
& $mountAssetScript -ClientRoot $PSScriptRoot
& $mobItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
& $mobAssetScript -ClientRoot $PSScriptRoot
# This is deliberately the last ItemList stage: costume, mount and mob clones
# may carry their source record's original EF_GRID metadata.
& $gridItemScript -ItemList (Join-Path $PSScriptRoot 'ItemList.bin')
