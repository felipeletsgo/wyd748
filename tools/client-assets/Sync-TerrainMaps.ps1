<#
.SYNOPSIS
Keeps the client and server collision maps byte-identical where consumed.
.DESCRIPTION
The 7.48 client AttributeMap has a four-byte trailer that its loader ignores.
The server consumes its first 1,048,576 bytes. The server HeightMap is a
16,777,216-byte world map; the client loads a copy for route collision.
#>
param(
    [string]$ClientRoot = (Join-Path $PSScriptRoot '../../tmproject/client748'),
    [string]$ServerRoot = (Join-Path $PSScriptRoot '../../wydgo748'),
    [switch]$Apply
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$clientAttributePath = Join-Path $ClientRoot 'Env/AttributeMap.dat'
$clientHeightPath = Join-Path $ClientRoot 'Env/HeightMap.dat'
$serverAttributePath = Join-Path $ServerRoot 'data/maps/AttributeMap.dat'
$serverHeightPath = Join-Path $ServerRoot 'data/maps/HeightMap.dat'

$clientAttribute = [IO.File]::ReadAllBytes($clientAttributePath)
$serverHeight = [IO.File]::ReadAllBytes($serverHeightPath)
if ($clientAttribute.Length -ne 1048580 -or $serverHeight.Length -ne 16777216) {
    throw 'The canonical client attribute or server height map has an unexpected size.'
}

$attributePayload = New-Object 'byte[]' 1048576
[Array]::Copy($clientAttribute, $attributePayload, $attributePayload.Length)

if ($Apply) {
    if ((Test-Path -LiteralPath $serverAttributePath) -and
        ([IO.FileInfo]$serverAttributePath).Length -ne $attributePayload.Length) {
        throw 'The destination server attribute map has an unexpected size.'
    }
    if ((Test-Path -LiteralPath $clientHeightPath) -and
        ([IO.FileInfo]$clientHeightPath).Length -ne $serverHeight.Length) {
        throw 'The destination client height map has an unexpected size.'
    }
    [IO.File]::WriteAllBytes($serverAttributePath, $attributePayload)
    [IO.File]::WriteAllBytes($clientHeightPath, $serverHeight)
}

if (!(Test-Path -LiteralPath $serverAttributePath) -or
    !(Test-Path -LiteralPath $clientHeightPath)) {
    throw 'A synchronized terrain asset is missing. Run with -Apply.'
}
$serverAttribute = [IO.File]::ReadAllBytes($serverAttributePath)
$clientHeight = [IO.File]::ReadAllBytes($clientHeightPath)
if ($serverAttribute.Length -ne $attributePayload.Length -or
    $clientHeight.Length -ne $serverHeight.Length -or
    ![System.Linq.Enumerable]::SequenceEqual[byte]($attributePayload, $serverAttribute) -or
    ![System.Linq.Enumerable]::SequenceEqual[byte]($serverHeight, $clientHeight)) {
    throw 'Client and server terrain assets are not synchronized.'
}

[pscustomobject]@{
    AttributePayloadBytes = $attributePayload.Length
    AttributeSHA256 = (Get-FileHash -LiteralPath $serverAttributePath -Algorithm SHA256).Hash
    HeightPayloadBytes = $serverHeight.Length
    HeightSHA256 = (Get-FileHash -LiteralPath $serverHeightPath -Algorithm SHA256).Hash
    Status = 'Client and server terrain assets match.'
}
