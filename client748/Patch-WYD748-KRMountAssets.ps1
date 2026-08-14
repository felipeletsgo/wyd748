[CmdletBinding()]
param(
    [string]$ClientRoot = $PSScriptRoot,
    [string[]]$SourceRoots = @(
        (Join-Path (Split-Path -Parent $PSScriptRoot) 'CLIENTS\WYD'),
        (Join-Path (Split-Path -Parent $PSScriptRoot) 'CLIENTS\wyd-test')
    ),
    [string]$NativeManifest = (Join-Path $PSScriptRoot 'Mounts-KR-Native.csv'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Instala somente assets reais das familias de montaria presentes no client KR.
# Nao cria substitutos por aproximacao. BoneAni/AniSound sao acrescidos com os
# tipos modernos e ValidIndex e traduzido campo a campo (186 u32 por tipo),
# depois de comprovar o mesmo layout semantico de 744 bytes nas duas versoes.

$prefixes = @('dr01','bo01','be01','hs01','dr02','bd02','CP01','KK01','mc01','ct01','mo02')
$boneRows = @(
    '48 5 2 mesh\CP01',
    '49 6 1 mesh\KK01',
    '50 5 1 mesh\mc01',
    '51 5 2 mesh\ct01',
    '59 4 3 mesh\mo02'
)

# Casos de regressao visual usados na auditoria: Tiger, Loki, Egg Chick e
# Bahamut Pogball, alem de alguns assets modernos que nao existem no 7.48 base.
$requiredNativeAssets = @(
    'mesh\be010111.msh','mesh\be010211.msh',
    'mesh\hs010118.msh','mesh\hs010218.msh',
    'mesh\KK010101.msh','mesh\KK010111.wys',
    'mesh\KK010118.msh','mesh\KK010101.wys',
    'mesh\CP010101.msh','mesh\CP010101.wys',
    'mesh\mc010101.msh','mesh\mc010101.wys',
    'mesh\ct010101.msh','mesh\ct010101.wys',
    'mesh\mo020101.msh','mesh\mo020101.wys'
)

# Arquivos criados pela implementacao anterior a partir de outra variante.
# Eles nao sao dependencias dos registros standalone de MountDataV.
$legacyFallbacks = @{
    'mesh\hs010117.wys' = 'mesh\hs010115.wys'
    'mesh\KK010112.msh' = 'mesh\KK010113.msh'
    'mesh\KK010112.wys' = 'mesh\KK010113.wys'
}

$expected = @{
    AniSound = '3DCA7A996966ECF5C281F90445ED3A2C783A7203C739D3B779DFB09E0D02F47D'
    BoneAni = 'CAE8913A984CE10C7D2755DFFEEE6939EB24E02E5CB243BE42A8270C37BD314B'
    ValidIndex = '37A669140DDEEAB48435CAE6EFE9DCBBA0CCFEFAAAAB8D1F58701D28FCD8DC76'
}

function Get-Sha([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}
function Set-U32([byte[]]$Data,[int]$Offset,[uint32]$Value) {
    [Array]::Copy([BitConverter]::GetBytes($Value),0,$Data,$Offset,4)
}
function Test-LfsPointer([string]$Path) {
    $info=Get-Item -LiteralPath $Path
    if($info.Length -gt 512){return $false}
    $bytes=[IO.File]::ReadAllBytes($Path)
    $prefix=[Text.Encoding]::ASCII.GetString($bytes,0,[Math]::Min($bytes.Length,64))
    return $prefix.StartsWith('version https://git-lfs.github.com/spec/v1',[StringComparison]::Ordinal)
}
function Find-RealFile([string]$Relative,[string[]]$Roots) {
    foreach($root in $Roots) {
        $candidate=Join-Path $root $Relative
        if((Test-Path -LiteralPath $candidate -PathType Leaf) -and -not(Test-LfsPointer $candidate)) { return $candidate }
    }
    return $null
}
function Test-LegacyFallbackResidue {
    foreach($targetRelative in $legacyFallbacks.Keys) {
        $target=Join-Path $ClientRoot $targetRelative
        $fallback=Join-Path $ClientRoot $legacyFallbacks[$targetRelative]
        if((Test-Path -LiteralPath $target -PathType Leaf) -and (Test-Path -LiteralPath $fallback -PathType Leaf)) {
            if((Get-Sha $target) -eq (Get-Sha $fallback)) { return $targetRelative }
        }
    }
    return $null
}
function Assert-Installed {
    $ani=Join-Path $ClientRoot 'AniSound4.txt'
    $bone=Join-Path $ClientRoot 'mesh\BoneAni4.txt'
    $valid=Join-Path $ClientRoot 'mesh\ValidIndex.bin'
    foreach($path in @($ani,$bone,$valid)) {
        if(-not(Test-Path -LiteralPath $path -PathType Leaf)){throw "asset ausente: $path"}
    }
    if((Get-Sha $ani)-ne$expected.AniSound -or (Get-Sha $bone)-ne$expected.BoneAni -or (Get-Sha $valid)-ne$expected.ValidIndex) {
        throw 'tabelas de animacao das montarias KR divergiram do estado suportado'
    }
    foreach($prefix in @('CP01','KK01','mc01','ct01','mo02')) {
        if(-not(Test-Path -LiteralPath (Join-Path $ClientRoot "mesh\$prefix.bon") -PathType Leaf)) {
            throw "skeleton KR ausente: $prefix.bon"
        }
    }
    foreach($relative in $requiredNativeAssets) {
        if(-not(Test-Path -LiteralPath (Join-Path $ClientRoot $relative) -PathType Leaf)) {
            throw "asset nativo de montaria ausente: $relative"
        }
    }
    $residue=Test-LegacyFallbackResidue
    if($residue){throw "fallback legado ainda materializado: $residue"}
}

if(-not(Test-Path -LiteralPath $NativeManifest -PathType Leaf)){throw "manifesto standalone ausente: $NativeManifest"}
$native=@(Import-Csv -LiteralPath $NativeManifest)
if($native.Count -ne 47){throw "manifesto standalone possui $($native.Count) montarias; esperado 47"}

$availableRoots=@($SourceRoots | Where-Object { Test-Path -LiteralPath $_ -PathType Container })
if($availableRoots.Count -eq 0){throw "nenhum client KR fonte foi encontrado: $($SourceRoots -join ', ')"}

if(-not $VerifyOnly) {
    # Remove apenas residuos byte-identicos aos substitutes antigos. Um asset
    # verdadeiro com conteudo diferente nunca e apagado.
    foreach($targetRelative in $legacyFallbacks.Keys) {
        $target=Join-Path $ClientRoot $targetRelative
        $fallback=Join-Path $ClientRoot $legacyFallbacks[$targetRelative]
        if((Test-Path -LiteralPath $target -PathType Leaf) -and (Test-Path -LiteralPath $fallback -PathType Leaf)) {
            if((Get-Sha $target) -eq (Get-Sha $fallback)) { Remove-Item -LiteralPath $target -Force }
        }
    }
}

try {
    Assert-Installed
    Write-Host 'Assets nativos das 47 montarias KR ja estao instalados.'
    return
} catch {
    if($VerifyOnly){throw}
}

$sourceMeshes=@($availableRoots | ForEach-Object { Join-Path $_ 'mesh' } | Where-Object { Test-Path -LiteralPath $_ -PathType Container })
if($sourceMeshes.Count -eq 0){throw "nenhuma pasta mesh dos clients KR foi encontrada: $($SourceRoots -join ', ')"}
$targetMesh=Join-Path $ClientRoot 'mesh'

# Importa somente arquivos reais. Arquivo 7.48 existente nao e sobrescrito.
foreach($prefix in $prefixes) {
    foreach($sourceMesh in $sourceMeshes) {
        Get-ChildItem -LiteralPath $sourceMesh -File | Where-Object {
            (-not(Test-LfsPointer $_.FullName)) -and $_.BaseName.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)
        } | ForEach-Object {
            $target=Join-Path $targetMesh $_.Name
            if(-not(Test-Path -LiteralPath $target -PathType Leaf)) {
                Copy-Item -LiteralPath $_.FullName -Destination $target
            }
        }
    }
}

$bonePath=Join-Path $targetMesh 'BoneAni4.txt'
$boneText=[IO.File]::ReadAllText($bonePath,[Text.Encoding]::Default)
foreach($row in $boneRows) {
    if($boneText -notmatch ('(?m)^'+[regex]::Escape($row)+'\s*$')) {
        $boneText=$boneText.TrimEnd()+"`r`n"+$row+"`r`n"
    }
}
[IO.File]::WriteAllText($bonePath,$boneText,[Text.Encoding]::Default)

$aniPath=Join-Path $ClientRoot 'AniSound4.txt'
$aniFragment=Join-Path $ClientRoot 'Mounts-KR-AniSound4.txt'
if(-not(Test-Path -LiteralPath $aniFragment -PathType Leaf)){throw "fragmento de animacao ausente: $aniFragment"}
$aniText=[IO.File]::ReadAllText($aniPath,[Text.Encoding]::Default)
if($aniText -notmatch '(?m)^\[KRMount48\]\s+48\s*$') {
    $aniText=$aniText.TrimEnd()+"`r`n`r`n"+[IO.File]::ReadAllText($aniFragment,[Text.Encoding]::ASCII)
    [IO.File]::WriteAllText($aniPath,$aniText,[Text.Encoding]::Default)
}

# ValidIndex tem 100 registros de 744 bytes. A engenharia reversa em ambos os
# clients confirmou 186 campos u32 por registro; a traducao e feita campo a
# campo para evitar memcpy de um registro estrangeiro opaco.
$validPath=Join-Path $targetMesh 'ValidIndex.bin'
$sourceValid=$null
foreach($sourceMesh in $sourceMeshes) {
    $candidate=Join-Path $sourceMesh 'ValidIndex.bin'
    if((Test-Path -LiteralPath $candidate -PathType Leaf) -and (Get-Item -LiteralPath $candidate).Length -eq 74400) {
        $sourceValid=$candidate
        break
    }
}
if(-not $sourceValid){throw 'ValidIndex.bin KR real de 74400 bytes nao encontrado'}
$valid=[IO.File]::ReadAllBytes($validPath)
$source=[IO.File]::ReadAllBytes($sourceValid)
if($valid.Length -ne 74400 -or $source.Length -ne 74400){throw 'ValidIndex.bin com tamanho inesperado'}
foreach($type in @(48,49,50,51,59)) {
    for($field=0;$field-lt 186;$field++) {
        $offset=$type*744+$field*4
        Set-U32 $valid $offset ([BitConverter]::ToUInt32($source,$offset))
    }
}
[IO.File]::WriteAllBytes($validPath,$valid)

Assert-Installed
Write-Host 'Assets e animacoes nativos das 47 montarias KR instalados no client 7.48.'
