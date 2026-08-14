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

# Instala as dependencias visuais das 59 montarias completas em Mounts-KR.json.
# Os 62 cases fonte continuam catalogados; tres permanecem indisponiveis porque
# os snapshots KR nao contem os arquivos que o proprio renderer referencia. Arquivos 7.48 existentes nunca sao sobrescritos: somente as
# variantes ausentes sao importadas; as tabelas de animacao recebem os cinco
# skeleton types inexistentes no client antigo.

$prefixes = @('dr01','be01','tw01','hs01','tg01','bo01','dr02','bd02','CP01','KK01','mc01','ct01','mo02')
$requiredImportedMeshes = @(
    'mesh\bo010104.msh','mesh\bo010104.wys','mesh\bo010204.msh',
    'mesh\hs010151.msh','mesh\hs010151.wys','mesh\hs010251.msh',
    'mesh\tg010102.msh','mesh\tg010102.wys','mesh\tg010202.msh',
    'mesh\tg010103.msh','mesh\tg010103.wys','mesh\tg010203.msh',
    'mesh\tg010105.msh','mesh\tg010105.wys','mesh\tg010205.msh',
    'mesh\KK010108.msh','mesh\KK010108.wys',
    'mesh\KK010109.msh','mesh\KK010109.wys',
    'mesh\be010111.msh','mesh\be010111.wys','mesh\be010211.msh',
    'mesh\KK010110.msh','mesh\KK010110.wys',
    'mesh\KK010101.msh','mesh\KK010111.wys',
    'mesh\KK010118.msh','mesh\KK010118.wys'
)
# Nao criar aliases para dependencias ausentes. Ausencia no client fonte
# torna a aparencia indisponivel no manifesto.
$boneRows = @(
    '48 5 2 mesh\CP01',
    '49 6 1 mesh\KK01',
    '50 5 1 mesh\mc01',
    '51 5 2 mesh\ct01',
    '59 4 3 mesh\mo02'
)
$expected = @{
    AniSound = @('3DCA7A996966ECF5C281F90445ED3A2C783A7203C739D3B779DFB09E0D02F47D','CAB1458EC3B9BD6B50060E43241A7C62DCFD4F0995D6831A8E6AB612DAF815BE')
    BoneAni = @('CAE8913A984CE10C7D2755DFFEEE6939EB24E02E5CB243BE42A8270C37BD314B','802BADEB2CE3B1A70EBD95C5F75C65513830AD58C128C98E77BBAA794B5E8B1A')
    ValidIndex = @('37A669140DDEEAB48435CAE6EFE9DCBBA0CCFEFAAAAB8D1F58701D28FCD8DC76','1E4DE8ECCCB31A1CE53178035E9158101B1ED4FF48439FCCFFB14411F7999E51')
}

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant() }
function Assert-Installed {
    $ani = Join-Path $ClientRoot 'AniSound4.txt'
    $bone = Join-Path $ClientRoot 'mesh\BoneAni4.txt'
    $valid = Join-Path $ClientRoot 'mesh\ValidIndex.bin'
    foreach ($path in @($ani,$bone,$valid)) { if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "asset ausente: $path" } }
    if ($expected.AniSound -notcontains (Get-Sha $ani) -or $expected.BoneAni -notcontains (Get-Sha $bone) -or $expected.ValidIndex -notcontains (Get-Sha $valid)) {
        throw 'tabelas de animacao das montarias KR divergiram do estado suportado'
    }
    foreach ($prefix in @('CP01','KK01','mc01','ct01','mo02')) {
        if (-not (Test-Path -LiteralPath (Join-Path $ClientRoot "mesh\$prefix.bon") -PathType Leaf)) {
            throw "skeleton KR ausente: $prefix.bon"
        }
    }
    foreach ($relative in $requiredImportedMeshes) {
        if (-not (Test-Path -LiteralPath (Join-Path $ClientRoot $relative) -PathType Leaf)) {
            throw "mesh de montaria KR ausente: $relative"
        }
    }
    if (-not (Test-Path -LiteralPath $Manifest -PathType Leaf)) { throw "manifesto ausente: $Manifest" }
    $definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
    $catalogTextures = @($definition.textures)
    $textures = @($catalogTextures | Where-Object { $_.available -ne $false })
    if ($catalogTextures.Count -ne 61 -or $textures.Count -ne 58) {
        throw "manifesto possui catalogo=$($catalogTextures.Count) texturas e disponiveis=$($textures.Count); esperado 61/58"
    }
    foreach ($texture in $textures) {
        $path = Join-Path $ClientRoot ([string]$texture.name)
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
            throw "textura de montaria ausente: $($texture.name)"
        }
    }
}

try { Assert-Installed; Write-Host 'Assets das 59 montarias KR completas ja estao instalados.'; return } catch { if ($VerifyOnly) { throw } }
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
$validCounts = @{}
foreach ($row in $boneRows) {
    $parts = @($row -split '\s+' | Where-Object { $_ })
    $validCounts[[int]$parts[0]] = [int]$parts[1]
}
foreach ($type in @(48,49,50,51,59)) {
    $count = [int]$validCounts[$type]
    $sourceOffset = $type * 744
    $targetOffset = $type * 744
    [Array]::Clear($valid, $targetOffset, 744)
    for ($index = 0; $index -lt $count; $index++) {
        [Array]::Copy($source, $sourceOffset + $index * 4, $valid, $targetOffset + $index * 4, 4)
    }
}
[IO.File]::WriteAllBytes($validPath, $valid)
Assert-Installed
Write-Host 'Assets e animacoes das 59 montarias KR completas instalados no client 7.48.'
