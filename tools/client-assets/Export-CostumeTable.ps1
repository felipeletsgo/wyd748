[CmdletBinding()]
param([switch]$Check)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$manifestPath = Join-Path $repo 'tools/client-assets/inputs/Costumes-KR.json'
$headerPath = Join-Path $repo 'tmproject/TMProject748/internal/render/mesh/CostumeTable.generated.h'
$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
$items = @($manifest.items | Where-Object available | Sort-Object item)
$renderers = @($manifest.renderers | Sort-Object type)
if ($items.Count -ne 143 -or $renderers.Count -ne 135) { throw 'Unexpected validated costume collection.' }
$textures = @($manifest.textures | Where-Object { $_.PSObject.Properties['available'] -eq $null -or $_.available } | Sort-Object name)
if (@($items | ForEach-Object { $_.item } | Sort-Object -Unique).Count -ne $items.Count -or
    @($renderers.type | Sort-Object -Unique).Count -ne $renderers.Count) { throw 'Duplicate costume ID/type.' }

function ConvertTo-CppString([string]$Value, [int]$Capacity) {
    if ($Value.Length -ge $Capacity -or $Value -match '[^a-zA-Z0-9_./\\-]') { throw "Invalid asset path: $Value" }
    if ($Value -and -not (Test-Path -LiteralPath (Join-Path $repo "tmproject/client748/$Value") -PathType Leaf)) {
        throw "Missing costume asset: $Value"
    }
    return '"' + $Value.Replace('\', '\\') + '"'
}

$lines = [Collections.Generic.List[string]]::new()
$lines.Add('#pragma once')
$lines.Add('// Generated from tools/client-assets/inputs/Costumes-KR.json by tools/client-assets/Export-CostumeTable.ps1.')
$lines.Add('// Validated 7.48 data; renderer IDs are not the upstream TMProject switch IDs.')
$lines.Add('namespace costume748 {')
$lines.Add('struct Item { int index; int maleType; int femaleType; int skin; };')
$lines.Add('struct Part { const char* texture; const char* mesh; };')
$lines.Add('struct Renderer { int type; Part parts[6]; };')
$lines.Add('struct Texture { const char* name; char alpha; };')
$lines.Add('inline constexpr Item Items[] = {')
foreach ($item in $items) {
    # Types 1..5 retain the native SetHumanCostume/SetCostume path.
    $supportedTypes = @($renderers.type) + @(1..5)
    if ($item.maleType -notin $supportedTypes -or $item.femaleType -notin $supportedTypes -or
        $item.skin -notin @(-1, 0, 1)) { throw "Invalid selector: $($item.item)" }
    $lines.Add(('    {{ {0}, {1}, {2}, {3} }},' -f $item.item, $item.maleType, $item.femaleType, $item.skin))
}
$lines.Add('};')
$lines.Add('inline constexpr Renderer Renderers[] = {')
foreach ($renderer in $renderers) {
    if ($renderer.parts.Count -ne 6 -or $renderer.type -le 0 -or $renderer.type -ge 0x4000) {
        throw "Invalid renderer: $($renderer.type)"
    }
    $parts = foreach ($part in $renderer.parts) {
        if ($part.mesh -and -not $part.texture) { throw 'Mesh without texture.' }
        '{ ' + (ConvertTo-CppString $part.texture 64) + ', ' + (ConvertTo-CppString $part.mesh 32) + ' }'
    }
    $lines.Add(('    {{ {0}, {{ {1} }} }},' -f $renderer.type, ($parts -join ', ')))
}
$lines.Add('};')
# Renderer textures must be registered in the model texture table; the
# runtime appends any that the shipped MeshTextureList.bin omits.
$lines.Add('inline constexpr Texture Textures[] = {')
foreach ($texture in $textures) {
    if ($texture.alpha -notin @(65, 67, 78)) { throw "Invalid texture alpha: $($texture.name)" }
    $lines.Add(('    {{ {0}, {1} }},' -f (ConvertTo-CppString $texture.name 64), $texture.alpha))
}
$lines.Add('};')
$lines.Add('} // namespace costume748')
$result = ($lines -join "`n") + "`n"
if ($Check) {
    $actual = [IO.File]::ReadAllText($headerPath).Replace("`r`n", "`n")
    if ($actual -cne $result) { throw 'CostumeTable.generated.h is stale; regenerate through the reviewed source edit.' }
    Write-Output "Costume table: $($items.Count) items, $($renderers.Count) renderers, $($textures.Count) textures and asset dependencies PASS."
} else {
    # Output only: the caller reviews/applies the generated source; no runtime writes.
    [Console]::Out.Write($result)
}
