[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

$manifestPath = Join-Path $PSScriptRoot 'MobFaces-KR.json'
if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) { throw 'MobFaces-KR.json ausente' }
$definition = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
$items = @($definition.items)
$textures = @($definition.textures)
if ($items.Count -ne 19 -or $textures.Count -ne 30) { throw 'manifesto KR de mobs nao possui 19 faces/30 texturas' }

$targetIDs = @{}
foreach ($item in $items) {
    if ([int]$item.targetItem -le 0 -or [int]$item.targetItem -gt 4095) { throw "face compacta invalida: $($item.targetItem)" }
    if ($targetIDs.ContainsKey([int]$item.targetItem)) { throw "face duplicada: $($item.targetItem)" }
    $targetIDs[[int]$item.targetItem] = $true
    if ([Text.Encoding]::ASCII.GetByteCount([string]$item.name) -gt 12) { throw "nome excede CreateMob: $($item.name)" }
}

& (Join-Path $PSScriptRoot 'Patch-WYD748-CostumeTextures.ps1') -VerifyOnly
& (Join-Path $PSScriptRoot 'Patch-WYD748-KRMobItems.ps1') -VerifyOnly
& (Join-Path $PSScriptRoot 'Patch-WYD748-KRMobAssets.ps1') -VerifyOnly
& (Join-Path $PSScriptRoot 'Patch-WYD748-KRMobs.ps1') -VerifyOnly
& (Join-Path $PSScriptRoot 'Patch-WYD748-KRMobPoses.ps1') -VerifyOnly

Write-Host 'OK: 19 faces KR, IDs compactos, 30 texturas, 8 skeleton families e orientacao moderna validados.'
Write-Host 'STATICALLY VERIFIED; a aparencia final ainda requer validacao in-game no client 7.48.'
