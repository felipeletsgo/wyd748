[CmdletBinding()]
param(
    [string]$MeshTextureList = (Join-Path $PSScriptRoot 'mesh\MeshTextureList.bin'),
    [string]$Manifest = (Join-Path $PSScriptRoot 'Costumes-KR.json'),
    [string]$MountManifest = (Join-Path $PSScriptRoot 'Mounts-KR.json'),
    [string]$MobManifest = (Join-Path $PSScriptRoot 'MobFaces-KR.json'),
    [switch]$VerifyOnly
)

$ErrorActionPreference = 'Stop'

# Registers every texture used by the costume and mount assets imported from
# the current Korean client. KR uses a newer 528-byte texture record, while
# WYD 7.48 reads exactly 2,048 legacy 264-byte stTextureListInfo rows. The
# manifests carry the material byte translated from KR offset 510. Preserve
# that value in the legacy row. A global C-to-A projection was tested and made
# previously visible costumes disappear, so material adaptation must remain a
# per-renderer change backed by an in-game comparison.

$recordSize = 264
$recordCount = 2048
$originalHash = '7FE770F97E419E7A080689423B024B788CA6C4761FC35FE68888154F9CA65395'
$pilotHash = '2BD35F88C159FB17C4FC6831BAA0F3173CF7521A5F8E3DF53E1B109E4C5B5D67'

function Get-Sha([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Get-RecordName([byte[]]$Data, [int]$Index) {
    $offset = $Index * $recordSize
    $length = 0
    while ($length -lt 255 -and $Data[$offset + $length] -ne 0 -and $Data[$offset + $length] -ne 0xCD) {
        $length++
    }
    return [Text.Encoding]::ASCII.GetString($Data, $offset, $length)
}

function Set-EmptyRecord([byte[]]$Data, [int]$Index) {
    $offset = $Index * $recordSize
    for ($i = 0; $i -lt $recordSize; $i++) { $Data[$offset + $i] = 0xCD }
    $Data[$offset] = 0
    $Data[$offset + 255] = [byte][char]'N'
}

function Set-TextureRecord([byte[]]$Data, [int]$Index, [string]$Name, [byte]$Alpha) {
    if ($Name.Length -ge 255) { throw "nome de textura excede 254 bytes: $Name" }
    $offset = $Index * $recordSize
    for ($i = 0; $i -lt $recordSize; $i++) { $Data[$offset + $i] = 0xCD }
    $bytes = [Text.Encoding]::ASCII.GetBytes($Name)
    [Array]::Copy($bytes, 0, $Data, $offset, $bytes.Length)
    $Data[$offset + $bytes.Length] = 0
    $Data[$offset + 255] = $Alpha
}

function Get-TextureIndex([byte[]]$Data) {
    $byName = @{}
    $empty = [Collections.Generic.List[int]]::new()
    for ($index = 0; $index -lt $recordCount; $index++) {
        $name = Get-RecordName $Data $index
        if ($name) { $byName[$name.ToLowerInvariant()] = $index } else { $empty.Add($index) }
    }
    return [pscustomobject]@{ ByName = $byName; Empty = $empty }
}

function Test-TextureSet([byte[]]$Data, [object[]]$Wanted) {
    $index = Get-TextureIndex $Data
    foreach ($texture in $Wanted) {
        $key = ([string]$texture.name).ToLowerInvariant()
        if (-not $index.ByName.ContainsKey($key)) { return $false }
        $row = [int]$index.ByName[$key]
        if ($Data[$row * $recordSize + 255] -ne [byte]$texture.alpha) { return $false }
    }
    return $true
}

function Test-TextureNamesAbsent([byte[]]$Data, [object[]]$Textures) {
    $index = Get-TextureIndex $Data
    foreach ($texture in $Textures) {
        if ($index.ByName.ContainsKey(([string]$texture.name).ToLowerInvariant())) { return $false }
    }
    return $true
}

if (-not (Test-Path -LiteralPath $MeshTextureList -PathType Leaf)) { throw "MeshTextureList.bin ausente: $MeshTextureList" }
if (-not (Test-Path -LiteralPath $Manifest -PathType Leaf)) { throw "manifesto de trajes ausente: $Manifest" }
if (-not (Test-Path -LiteralPath $MountManifest -PathType Leaf)) { throw "manifesto de montarias ausente: $MountManifest" }
if (-not (Test-Path -LiteralPath $MobManifest -PathType Leaf)) { throw "manifesto de mobs ausente: $MobManifest" }

$definition = Get-Content -LiteralPath $Manifest -Raw | ConvertFrom-Json
$mountDefinition = Get-Content -LiteralPath $MountManifest -Raw | ConvertFrom-Json
$mobDefinition = Get-Content -LiteralPath $MobManifest -Raw | ConvertFrom-Json
$mountCatalogTextures = @($mountDefinition.textures)
$mountTextures = @($mountCatalogTextures | Where-Object { $_.available -ne $false })
$unavailableMountTextures = @($mountCatalogTextures | Where-Object { $_.available -eq $false })
if (@($definition.textures).Count -ne 176) { throw "manifesto de trajes possui $(@($definition.textures).Count) texturas; esperado 176" }
if ($mountCatalogTextures.Count -ne 61 -or $mountTextures.Count -ne 58 -or $unavailableMountTextures.Count -ne 3) {
    throw "manifesto de montarias possui catalogo=$($mountCatalogTextures.Count), disponiveis=$($mountTextures.Count), indisponiveis=$($unavailableMountTextures.Count); esperado 61/58/3"
}
if (@($mobDefinition.items).Count -ne 19 -or @($mobDefinition.textures).Count -ne 30) {
    throw "manifesto de mobs possui $(@($mobDefinition.items).Count) faces e $(@($mobDefinition.textures).Count) texturas; esperado 19/30"
}

$wantedByName = @{}
foreach ($texture in @($definition.textures) + $mountTextures + @($mobDefinition.textures)) {
    $key = ([string]$texture.name).ToLowerInvariant()
    $sourceAlpha = [int]$texture.alpha
    if ($sourceAlpha -notin @([byte][char]'A', [byte][char]'C', [byte][char]'N', [byte][char]'a')) {
        throw "modo alpha invalido para $($texture.name): $sourceAlpha"
    }
    $legacyAlpha = [byte]$sourceAlpha
    $legacyTexture = [pscustomobject]@{ name = [string]$texture.name; alpha = $legacyAlpha }
    if ($wantedByName.ContainsKey($key) -and [int]$wantedByName[$key].alpha -ne [int]$legacyTexture.alpha) {
        throw "modo alpha conflitante para $($texture.name)"
    }
    $wantedByName[$key] = $legacyTexture
}
$wanted = @($wantedByName.Values | Sort-Object name)
if ($wanted.Count -ne 264) { throw "colecao visual possui $($wanted.Count) texturas unicas; esperado 264" }

$current = [IO.File]::ReadAllBytes($MeshTextureList)
if ($current.Length -ne $recordCount * $recordSize) { throw "MeshTextureList.bin com tamanho inesperado: $($current.Length)" }
$hasWanted = Test-TextureSet $current $wanted
$unavailableAbsent = Test-TextureNamesAbsent $current $unavailableMountTextures
if ($hasWanted -and $unavailableAbsent) {
    Write-Host "Todas as $($wanted.Count) texturas KR habilitadas estao registradas e os $($unavailableMountTextures.Count) pathnames indisponiveis estao ausentes."
    Write-Host "SHA-256: $(Get-Sha $MeshTextureList)"
    return
}
if ($VerifyOnly) { throw 'MeshTextureList.bin ainda nao possui a colecao visual KR estrita.' }
if ($hasWanted -and -not $unavailableAbsent) {
    $legacyIndex = Get-TextureIndex $current
    foreach ($texture in $unavailableMountTextures) {
        $key = ([string]$texture.name).ToLowerInvariant()
        if ($legacyIndex.ByName.ContainsKey($key)) { Set-EmptyRecord $current ([int]$legacyIndex.ByName[$key]) }
    }
    [IO.File]::WriteAllBytes($MeshTextureList, $current)
    $cleaned = [IO.File]::ReadAllBytes($MeshTextureList)
    if (-not (Test-TextureSet $cleaned $wanted) -or -not (Test-TextureNamesAbsent $cleaned $unavailableMountTextures)) {
        throw 'limpeza dos pathnames de montaria indisponiveis falhou'
    }
    Write-Host "MeshTextureList migrado: $($wanted.Count) texturas habilitadas; $($unavailableMountTextures.Count) pathnames sem asset autentico removidos."
    Write-Host "SHA-256: $(Get-Sha $MeshTextureList)"
    return
}

$backup = Join-Path (Split-Path -Parent $MeshTextureList) 'MeshTextureList.pre-costumes-kr.bin'
if (-not (Test-Path -LiteralPath $backup -PathType Leaf)) {
    $currentHash = Get-Sha $MeshTextureList
    if ($currentHash -notin @($originalHash, $pilotHash)) {
        throw "MeshTextureList.bin alterado sem backup integro (SHA-256: $currentHash)"
    }
    Copy-Item -LiteralPath $MeshTextureList -Destination $backup
}

# Always rebuild from the known pre-collection snapshot. This also repairs the
# former patch that copied KR byte 255 ('m', the second pathname) as alpha.
$data = [IO.File]::ReadAllBytes($backup)
if ($data.Length -ne $recordCount * $recordSize) { throw 'backup de MeshTextureList com tamanho inesperado' }
$backupHash = Get-Sha $backup
if ($backupHash -eq $pilotHash) {
    if ((Get-RecordName $data 1847) -ne 'mesh\WhitePolice.wys') { throw 'registro piloto 1847 divergente' }
    Set-EmptyRecord $data 1847
    $temporary = [IO.Path]::GetTempFileName()
    try {
        [IO.File]::WriteAllBytes($temporary, $data)
        if ((Get-Sha $temporary) -ne $originalHash) { throw 'restauracao do piloto nao recompos a tabela original' }
    } finally {
        Remove-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
    }
} elseif ($backupHash -ne $originalHash) {
    throw "backup de MeshTextureList fora da base suportada (SHA-256: $backupHash)"
}

$index = Get-TextureIndex $data
$missing = @($wanted | Where-Object { -not $index.ByName.ContainsKey($_.name.ToLowerInvariant()) })
foreach ($texture in $wanted) {
    $key = $texture.name.ToLowerInvariant()
    if ($index.ByName.ContainsKey($key)) {
        $row = [int]$index.ByName[$key]
        if ($data[$row * $recordSize + 255] -ne [byte]$texture.alpha) {
            throw "a textura existente $($texture.name) possui alpha diferente da fonte KR"
        }
    }
}
if ($index.Empty.Count -lt $missing.Count) { throw "a tabela possui $($index.Empty.Count) vagas para $($missing.Count) texturas" }

for ($i = 0; $i -lt $missing.Count; $i++) {
    Set-TextureRecord $data $index.Empty[$i] $missing[$i].name ([byte]$missing[$i].alpha)
}
[IO.File]::WriteAllBytes($MeshTextureList, $data)

$written = [IO.File]::ReadAllBytes($MeshTextureList)
if (-not (Test-TextureSet $written $wanted)) { throw 'verificacao final de MeshTextureList falhou' }
Write-Host "$($wanted.Count) texturas KR de trajes, montarias e monstros registradas no MeshTextureList 7.48."
Write-Host "SHA-256: $(Get-Sha $MeshTextureList)"
