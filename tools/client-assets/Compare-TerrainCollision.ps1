param(
    [string]$ClientRoot = (Join-Path $PSScriptRoot '../../tmproject/client748'),
    [string]$ServerRoot = (Join-Path $PSScriptRoot '../../wydgo748'),
    [ValidateRange(1, 4094)][int]$CenterX = 3647,
    [ValidateRange(1, 4094)][int]$CenterY = 3112,
    [ValidateRange(0, 40)][int]$Radius = 20,
    [switch]$RequireParity
)

$ErrorActionPreference = 'Stop'

function Read-SignedByte([byte]$Value) {
    if ($Value -gt 127) { return [int]$Value - 256 }
    return [int]$Value
}

function Get-TileHeight([byte[]]$Bytes, [int]$Offset, [int]$X, [int]$Y) {
    # LoadTileMap copies the second row/column into the first before sampling.
    if ($X -eq 0) { $X = 1 }
    if ($Y -eq 0) { $Y = 1 }
    return Read-SignedByte $Bytes[$Offset + (($Y * 64 + $X) * 12)]
}

function Get-ClientHeight([byte[]]$Bytes, [int]$Offset, [int]$LocalX, [int]$LocalY) {
    $tileX = [int][math]::Floor($LocalX / 2)
    $tileY = [int][math]::Floor($LocalY / 2)
    $nextX = if ($tileX -lt 62) { $tileX + 1 } else { $tileX }
    $nextY = if ($tileY -lt 62) { $tileY + 1 } else { $tileY }
    $f1 = Get-TileHeight $Bytes $Offset $tileX $tileY
    $f2 = Get-TileHeight $Bytes $Offset $nextX $tileY
    $f3 = Get-TileHeight $Bytes $Offset $tileX $nextY
    $f4 = Get-TileHeight $Bytes $Offset $nextX $nextY
    $center = ($f1 + $f2 + $f3 + $f4) / 4.0
    $corner = if (($LocalY % 2) -eq 0) {
        if (($LocalX % 2) -eq 0) { $f1 } else { $f2 }
    } else {
        if (($LocalX % 2) -eq 0) { $f3 } else { $f4 }
    }
    # C++ static_cast<char> truncates this interpolated float toward zero.
    return [int][math]::Truncate(($corner + $center) / 2.0)
}

function Get-ObjectOverlay([string]$Root, [int]$TileX, [int]$TileY) {
    $fieldPath = Join-Path $Root ('Env/Field{0:D2}{1:D2}.dat' -f $TileX, $TileY)
    $objectPath = Join-Path $Root 'object.bin'
    $keySource = Get-Content (Join-Path $PSScriptRoot '../../tmproject/TMProject748/internal/render/mesh/MeshManager.h') -Raw
    $keyMatch = [regex]::Match($keySource, 'ObjectMaskEncKeys\[172\]\s*=\s*\{(?<bytes>.*?)\}', 'Singleline')
    if (!$keyMatch.Success) { throw 'The object mask decryption key could not be found.' }
    $keys = @([regex]::Matches($keyMatch.Groups['bytes'].Value, '0x[0-9A-Fa-f]{2}') | ForEach-Object { [Convert]::ToInt32($_.Value, 16) })
    if ($keys.Count -ne 172 -or $keys[170] -ne 0 -or $keys[171] -ne 0) {
        throw 'The object mask decryption key has an unexpected layout.'
    }
    $records = [IO.File]::ReadAllBytes($fieldPath)
    $masks = [IO.File]::ReadAllBytes($objectPath)
    if ($masks.Length -ne 2048 * 256 + 4) { throw 'The object mask file has an invalid length.' }
    $overlay = New-Object 'int[]' (128 * 128)
    for ($i = 0; $i -lt $overlay.Length; ++$i) { $overlay[$i] = [int]::MinValue }
    $recordOffset = 0
    $registered = 0
    while ($recordOffset -lt $records.Length) {
        if ($records.Length - $recordOffset -lt 28) { throw "Truncated object record at $recordOffset" }
        $type = [BitConverter]::ToUInt32($records, $recordOffset)
        $size = if (($type -ge 501 -and $type -le 506) -or
            ($type -ge 511 -and $type -le 518) -or
            ($type -ge 520 -and $type -lt 600)) { 36 } else { 28 }
        if ($records.Length - $recordOffset -lt $size) { throw "Truncated object record at $recordOffset" }
        $x = [BitConverter]::ToSingle($records, $recordOffset + 4)
        $y = [BitConverter]::ToSingle($records, $recordOffset + 8)
        $height = [BitConverter]::ToSingle($records, $recordOffset + 12)
        $angle = [BitConverter]::ToSingle($records, $recordOffset + 16)
        $maskIndex = [BitConverter]::ToInt32($records, $recordOffset + 24)
        $recordOffset += $size
        # These loader branches return before TMObject::RegisterMask.
        if ($type -eq 2 -or $type -eq 4 -or $type -eq 6 -or $type -eq 7 -or
            $type -eq 12 -or ($type -ge 311 -and $type -le 322) -or
            $type -eq 343 -or $type -eq 344 -or
            ($type -ge 501 -and $type -le 506) -or
            ($type -ge 511 -and $type -le 530) -or $type -eq 532 -or
            $type -eq 121 -or $type -eq 10 -or $type -eq 1980 -or
            $type -eq 1846 -or $maskIndex -lt 0 -or $maskIndex -ge 2048) { continue }
        $baseX = [int][math]::Truncate($x) - 7
        $baseY = [int][math]::Truncate($y) - 7
        if ($baseX -gt 127 -or $baseY -gt 127 -or $baseX + 15 -lt 0 -or $baseY + 15 -lt 0) { continue }
        ++$registered
        $rotation = [math]::PI + $angle
        $cos = [math]::Cos($rotation)
        $sin = [math]::Sin($rotation)
        for ($maskY = 0; $maskY -lt 16; ++$maskY) {
            for ($maskX = 0; $maskX -lt 16; ++$maskX) {
                $targetX = $baseX + $maskX
                $targetY = $baseY + $maskY
                if ($targetX -lt 0 -or $targetX -ge 128 -or $targetY -lt 0 -or $targetY -ge 128) { continue }
                $fx = $maskX - 7.0
                $fy = $maskY - 7.0
                $sourceX = [int][math]::Truncate(($cos * $fx - $sin * $fy) + 7.5)
                $sourceY = [int][math]::Truncate(($sin * $fx + $cos * $fy) + 7.5)
                if ($sourceX -lt 0 -or $sourceX -ge 16 -or $sourceY -lt 0 -or $sourceY -ge 16) { continue }
                $maskOffset = $maskIndex * 256 + $sourceY * 16 + $sourceX
                $maskValue = Read-SignedByte ([byte](($masks[$maskOffset] - $keys[$maskOffset % 170] - $maskOffset) -band 255))
                if ($maskValue -eq 0) { continue }
                $overlayValue = [int][math]::Truncate($height / 0.1 + 3 * $maskValue)
                if ($height -gt 0) { ++$overlayValue }
                $overlayValue = [math]::Min($overlayValue, 127)
                $index = $targetY * 128 + $targetX
                if ($overlayValue -gt $overlay[$index]) { $overlay[$index] = $overlayValue }
            }
        }
    }
    return [pscustomobject]@{ Height = $overlay; RegisteredObjects = $registered }
}

$zoneX = [int][math]::Floor($CenterX / 128)
$zoneY = [int][math]::Floor($CenterY / 128)
$originX = $zoneX * 128
$originY = $zoneY * 128
if ($CenterX - $Radius -lt $originX + 16 -or
    $CenterX + $Radius -gt $originX + 112 -or
    $CenterY - $Radius -lt $originY + 16 -or
    $CenterY + $Radius -gt $originY + 112) {
    throw 'The requested area must remain within the interior of a single terrain tile; edge and adjacent-tile masks are not modeled.'
}

$fieldName = 'Field{0:D2}{1:D2}.trn' -f $zoneX, $zoneY
$terrainPath = Join-Path $ClientRoot "Env/$fieldName"
$clientAttributePath = Join-Path $ClientRoot 'Env/AttributeMap.dat'
$clientHeightPath = Join-Path $ClientRoot 'Env/HeightMap.dat'
$serverHeightPath = Join-Path $ServerRoot 'data/maps/HeightMap.dat'
$serverAttributePath = Join-Path $ServerRoot 'data/maps/AttributeMap.dat'
$terrain = [System.IO.File]::ReadAllBytes($terrainPath)
$objects = Get-ObjectOverlay $ClientRoot $zoneX $zoneY
$clientAttribute = [System.IO.File]::ReadAllBytes($clientAttributePath)
$clientSharedHeight = [System.IO.File]::ReadAllBytes($clientHeightPath)
$serverHeight = [System.IO.File]::ReadAllBytes($serverHeightPath)
$serverAttribute = [System.IO.File]::ReadAllBytes($serverAttributePath)
if ($terrain.Length -lt 4 -or $clientAttribute.Length -lt 1048576 -or
    $clientSharedHeight.Length -ne 16777216 -or
    $serverHeight.Length -ne 16777216 -or $serverAttribute.Length -ne 1048576) {
    throw 'One or more terrain or attribute maps have an invalid length.'
}
$nameLength = [int]$terrain[0]
$tileOffset = $nameLength + 3
if ($nameLength -ge 128 -or $terrain.Length -lt $tileOffset + 4096 * 12 -or
    [System.Text.Encoding]::ASCII.GetString($terrain, 1, $nameLength) -ne 'Field' -or
    $terrain[$nameLength + 1] -ne $zoneX -or $terrain[$nameLength + 2] -ne $zoneY) {
    throw "Invalid 7.48 terrain tile record: $terrainPath"
}

$clientOnlyBlocked = 0
$serverOnlyBlocked = 0
$bothBlocked = 0
$neitherBlocked = 0
$clientOnlyWithServerAttribute = 0
$serverOnlyWithServerAttribute = 0
$attributeDisagreements = 0
$differentWalkableHeights = 0
$maxWalkableHeightDifference = 0
$legacyTerrainHeightDisagreements = 0
$serverOnlyCoordinates = [System.Collections.Generic.List[string]]::new()
$clientOnlyCoordinates = [System.Collections.Generic.List[string]]::new()
$centerResult = $null
$side = $Radius * 2 + 1
$clientHeights = New-Object 'int[]' ($side * $side)
$serverHeights = New-Object 'int[]' ($side * $side)
$jointlyWalkable = New-Object 'bool[]' ($side * $side)
for ($y = $CenterY - $Radius; $y -le $CenterY + $Radius; ++$y) {
    for ($x = $CenterX - $Radius; $x -le $CenterX + $Radius; ++$x) {
        $legacyTerrainHeight = Get-ClientHeight $terrain $tileOffset ($x - $originX) ($y - $originY)
        $legacyTerrainHeight = [math]::Max($legacyTerrainHeight, $objects.Height[(($y - $originY) * 128) + ($x - $originX)])
        $clientHeight = Read-SignedByte ([byte]$clientSharedHeight[$y * 4096 + $x])
        $serverHeightByte = [int]$serverHeight[$y * 4096 + $x]
        $serverSignedHeight = Read-SignedByte ([byte]$serverHeightByte)
        if ($legacyTerrainHeight -ne $serverSignedHeight) { ++$legacyTerrainHeightDisagreements }
        $attributeIndex = ([int][math]::Floor($y / 4) * 1024) + [int][math]::Floor($x / 4)
        $clientAttributeByte = [int]$clientAttribute[$attributeIndex]
        $serverAttributeByte = [int]$serverAttribute[$attributeIndex]
        $clientBlocked = $clientHeight -eq 127 -or ($clientAttributeByte -band 2) -ne 0
        $serverBlocked = $serverHeightByte -eq 127 -or ($serverAttributeByte -band 2) -ne 0
        $clientBlockedWithServerAttribute = $clientHeight -eq 127 -or ($serverAttributeByte -band 2) -ne 0
        $regionIndex = ($y - ($CenterY - $Radius)) * $side + ($x - ($CenterX - $Radius))
        $clientHeights[$regionIndex] = $clientHeight
        $serverHeights[$regionIndex] = $serverSignedHeight
        $jointlyWalkable[$regionIndex] = !$clientBlocked -and !$serverBlocked
        if ($clientBlocked -and $serverBlocked) { ++$bothBlocked }
        elseif ($clientBlocked) { ++$clientOnlyBlocked; $clientOnlyCoordinates.Add("$x,$y") }
        elseif ($serverBlocked) { ++$serverOnlyBlocked; $serverOnlyCoordinates.Add("$x,$y") }
        else { ++$neitherBlocked }
        if ($clientBlockedWithServerAttribute -and !$serverBlocked) { ++$clientOnlyWithServerAttribute }
        if (!$clientBlockedWithServerAttribute -and $serverBlocked) { ++$serverOnlyWithServerAttribute }
        if (($clientAttributeByte -band 2) -ne ($serverAttributeByte -band 2)) { ++$attributeDisagreements }
        if (!$clientBlocked -and !$serverBlocked) {
            $heightDifference = [math]::Abs($clientHeight - $serverSignedHeight)
            if ($heightDifference -gt 0) { ++$differentWalkableHeights }
            $maxWalkableHeightDifference = [math]::Max($maxWalkableHeightDifference, $heightDifference)
        }
        if ($x -eq $CenterX -and $y -eq $CenterY) {
            $centerResult = [pscustomobject]@{
                X = $x; Y = $y; ClientHeight = $clientHeight
                LegacyTerrainHeight = $legacyTerrainHeight
                ClientAttribute = $clientAttributeByte; ClientBlocked = $clientBlocked
                ServerHeight = $serverSignedHeight
                ServerAttribute = $serverAttributeByte; ServerBlocked = $serverBlocked
            }
        }
    }
}
$routeHeightEdgeDisagreements = 0
for ($y = 0; $y -lt $side; ++$y) {
    for ($x = 0; $x -lt $side; ++$x) {
        $from = $y * $side + $x
        if (!$jointlyWalkable[$from]) { continue }
        foreach ($step in @(@(1, 0), @(0, 1), @(1, 1), @(-1, 1))) {
            $nextX = $x + $step[0]
            $nextY = $y + $step[1]
            if ($nextX -lt 0 -or $nextX -ge $side -or $nextY -ge $side) { continue }
            $to = $nextY * $side + $nextX
            if (!$jointlyWalkable[$to]) { continue }
            $clientPass = [math]::Abs($clientHeights[$from] - $clientHeights[$to]) -lt 8
            $serverPass = [math]::Abs($serverHeights[$from] - $serverHeights[$to]) -lt 8
            if ($clientPass -ne $serverPass) { ++$routeHeightEdgeDisagreements }
        }
    }
}

[pscustomobject]@{
    Tile = $fieldName
    Center = $centerResult
    CellsCompared = ($Radius * 2 + 1) * ($Radius * 2 + 1)
    ClientOnlyBlocked = $clientOnlyBlocked
    ServerOnlyBlocked = $serverOnlyBlocked
    BothBlocked = $bothBlocked
    NeitherBlocked = $neitherBlocked
    ClientOnlyWithServerAttribute = $clientOnlyWithServerAttribute
    ServerOnlyWithServerAttribute = $serverOnlyWithServerAttribute
    AttributeBitDisagreements = $attributeDisagreements
    DifferentWalkableHeights = $differentWalkableHeights
    MaxWalkableHeightDifference = $maxWalkableHeightDifference
    LegacyTerrainHeightDisagreements = $legacyTerrainHeightDisagreements
    RouteHeightEdgeDisagreements = $routeHeightEdgeDisagreements
    ServerOnlyCoordinates = @($serverOnlyCoordinates)
    ClientOnlyCoordinates = @($clientOnlyCoordinates)
    RegisteredObjects = $objects.RegisteredObjects
    Scope = 'Shared static route maps; legacy terrain and object masks are compared separately. Dynamic collision is excluded.'
}
if ($RequireParity -and ($clientOnlyBlocked -ne 0 -or $serverOnlyBlocked -ne 0 -or $routeHeightEdgeDisagreements -ne 0)) {
    throw "Client/server static route collision differs in the inspected area: client-only=$clientOnlyBlocked server-only=$serverOnlyBlocked height-edges=$routeHeightEdgeDisagreements"
}
