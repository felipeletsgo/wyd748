[CmdletBinding()]
param(
    [string]$ClientRoot = $PSScriptRoot,
    [string[]]$SourceRoots = @(
        (Join-Path (Split-Path -Parent $PSScriptRoot) 'CLIENTS\WYD'),
        (Join-Path (Split-Path -Parent $PSScriptRoot) 'CLIENTS\wyd-test')
    ),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Mounts-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Instala as dependencias visuais usadas pelas 47 montarias presentes em
# Mounts-KR.json. Arquivos 7.48 existentes nunca sao sobrescritos: somente as
# variantes ausentes sao importadas; as tabelas de animacao recebem os cinco
# skeleton types inexistentes no client antigo.
#
# Regra de fidelidade: asset ausente permanece ausente e interrompe a instalacao.
# Nunca sintetize uma variante copiando mesh/textura vizinha. O nome numerico faz
# parte do contrato entre LOOK_INFO e TMSkinMesh; um arquivo de outro indice pode
# ter UV, material ou geometria incompatíveis mesmo dentro da mesma familia.

$prefixes = @('dr01','be01','tw01','hs01','tg01','dr02','bd02','CP01','KK01','mc01','ct01','mo02')
$requiredImportedMeshes = @(
    'mesh\KK010108.msh','mesh\KK010108.wys',
    'mesh\KK010109.msh','mesh\KK010109.wys',
    'mesh\be010111.msh','mesh\be010111.wys','mesh\be010211.msh',
    'mesh\KK010110.msh','mesh\KK010110.wys',
    'mesh\KK010101.msh','mesh\KK010111.wys',
    'mesh\KK010112.msh','mesh\KK010112.wys',
    'mesh\KK010118.msh','mesh\KK010118.wys'
)
$boneRows = @(
    '48 5 2 mesh\CP01',
    '49 6 1 mesh\KK01',
    '50 5 1 mesh\mc01',
    '51 5 2 mesh\ct01',
    '59 4 3 mesh\mo02'
)
$expected = @{
    AniSound = '3DCA7A996966ECF5C281F90445ED3A2C783A7203C739D3B779DFB09E0D02F47D'
    BoneAni = 'CAE8913A984CE10C7D2755DFFEEE6939EB24E02E5CB243BE42A8270C37BD314B'
    ValidIndex = '37A669140DDEEAB48435CAE6EFE9DCBBA0CCFEFAAAAB8D1F58701D28FCD8DC76'
}

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant() }
function Assert-Installed {
    $ani = Join-Path $ClientRoot 'AniSound4.txt'
    $bone = Join-Path $ClientRoot 'mesh\BoneAni4.txt'
    $valid = Join-Path $ClientRoot 'mesh\ValidIndex.bin'
    foreach ($path in @($ani,$bone,$valid)) { if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "asset ausente: $path" } }
    if ((Get-Sha $ani) -ne $expected.AniSound -or (Get-Sha $bone) -ne $expected.BoneAni -or (Get-Sha $valid) -ne $expected.ValidIndex) {
        throw 'tabelas de animacao das montarias KR divergiram do estado suportado'
    }
    foreach ($prefix in @('CP01','KK01','mc01','ct01','mo02')) {
        if (-not (Test-Path -LiteralPath (Join-Path $ClientRoot "mesh\$prefix.bon") -PathType Leaf)) {
            throw "skeleton KR ausente: $prefix.bon"
        }
    }
    foreach ($relative in $requiredImportedMeshes) {
        if (-not (Test-Path -LiteralPath (Join-Path $ClientRoot $relative) -PathType Leaf)) {
            throw "asset exato de montaria KR ausente: $relative (nao use variante vizinha como fallback)"
        }
    }
    if (-not (Test-Path -LiteralPath $Manifest -PathType Leaf)) { throw "manifesto ausente: $Manifest" }
    $definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
    $textures = @($definition.textures)
    if ($textures.Count -ne 50) { throw "manifesto possui $($textures.Count) texturas de montaria; esperado 50" }
    foreach ($texture in $textures) {
        $path = Join-Path $ClientRoot ([string]$texture.name)
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "textura de montaria ausente: $($texture.name)"
        }
    }
}

try { Assert-Installed; Write-Host 'Assets das 47 montarias KR ja estao instalados.'; return } catch { if ($VerifyOnly) { throw } }
$availableRoots = @($SourceRoots | Where-Object { Test-Path -LiteralPath $_ -PathType Container })
if ($availableRoots.Count -eq 0) { throw "nenhum client KR fonte foi encontrado: $($SourceRoots -join ', ')" }
$sourceMeshes = @($availableRoots | ForEach-Object { Join-Path $_ 'mesh' } | Where-Object { Test-Path -LiteralPath $_ -PathType Container })
if ($sourceMeshes.Count -eq 0) { throw "nenhuma pasta mesh dos clients KR foi encontrada: $($SourceRoots -join ', ')" }
$targetMesh = Join-Path $ClientRoot 'mesh'
foreach ($prefix in $prefixes) {
    foreach ($sourceMesh in $sourceMeshes) {
        Get-ChildItem -LiteralPath $sourceMesh -File | Where-Object {
            $_.BaseName.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)
        } | ForEach-Object {
            $target = Join-Path $targetMesh $_.Name
            if (-not (Test-Path -LiteralPath $target -PathType Leaf)) { Copy-Item -LiteralPath $_.FullName -Destination $target }
        }
    }
}

$bonePath = Join-Path $targetMesh 'BoneAni4.txt'
$boneText = [IO.File]::ReadAllText($bonePath, [Text.Encoding]::Default)
foreach ($row in $boneRows) {
    if ($boneText -notmatch ('(?m)^' + [regex]::Escape($row) + '\s*$')) { $boneText = $boneText.TrimEnd() + "`r`n" + $row + "`r`n" }
}
[IO.File]::WriteAllText($bonePath, $boneText, [Text.Encoding]::Default)

$aniPath = Join-Path $ClientRoot 'AniSound4.txt'
$aniFragment = Join-Path $ClientRoot 'Mounts-KR-AniSound4.txt'
if (-not (Test-Path -LiteralPath $aniFragment -PathType Leaf)) { throw "fragmento de animacao ausente: $aniFragment" }
$aniText = [IO.File]::ReadAllText($aniPath, [Text.Encoding]::Default)
if ($aniText -notmatch '(?m)^\[KRMount48\]\s+48\s*$') {
    $aniText = $aniText.TrimEnd() + "`r`n`r`n" + [IO.File]::ReadAllText($aniFragment, [Text.Encoding]::ASCII)
    [IO.File]::WriteAllText($aniPath, $aniText, [Text.Encoding]::Default)
}

$validPath = Join-Path $targetMesh 'ValidIndex.bin'
$sourceValid = Join-Path $sourceMeshes[0] 'ValidIndex.bin'
$valid = [IO.File]::ReadAllBytes($validPath)
$source = [IO.File]::ReadAllBytes($sourceValid)
if ($valid.Length -ne 74400 -or $source.Length -ne 74400) { throw 'ValidIndex.bin com tamanho inesperado' }
foreach ($type in @(48,49,50,51,59)) { [Array]::Copy($source, $type * 744, $valid, $type * 744, 744) }
[IO.File]::WriteAllBytes($validPath, $valid)
Assert-Installed
Write-Host 'Assets e animacoes das 47 montarias KR instalados no client 7.48.'
