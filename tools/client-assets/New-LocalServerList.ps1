<#
.SYNOPSIS
Creates a local-only WYD 7.48 server list for same-machine development.
.DESCRIPTION
Decodes and validates the versioned table, changes configured game channels
to loopback, and writes an ignored override. Status URLs remain unchanged.
The versioned serverlist.bin is never modified.
.PARAMETER SourcePath
Optional input table. Defaults to tmproject/client748/serverlist.bin.
.PARAMETER DestinationPath
Optional output table. Defaults to tmproject/client748/serverlist.local.bin.
.PARAMETER Force
Replace an existing local override if its contents differ.
#>
[CmdletBinding()]
param(
    [string]$SourcePath,
    [string]$DestinationPath,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$assetRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../tmproject/client748'))
if (-not $SourcePath) { $SourcePath = Join-Path $assetRoot 'serverlist.bin' }
if (-not $DestinationPath) { $DestinationPath = Join-Path $assetRoot 'serverlist.local.bin' }
$source = [IO.Path]::GetFullPath($SourcePath)
$destination = [IO.Path]::GetFullPath($DestinationPath)
if ([string]::Equals($source, $destination, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'The output must differ from the source table.'
}
if (-not [IO.File]::Exists($source)) { throw "Source table is missing: $source" }
if (-not [IO.Directory]::Exists([IO.Path]::GetDirectoryName($destination))) {
    throw 'The output directory does not exist.'
}

$width = 64
$slotsPerGroup = 11
$entryCount = 110
$sourceBytes = [IO.File]::ReadAllBytes($source)
if ($sourceBytes.Length -ne ($entryCount * $width)) {
    throw "Expected a $($entryCount * $width)-byte 7.48 table."
}

# The most common encoded record is the all-zero plaintext record. Require it
# to repeat so a populated endpoint cannot accidentally become the key.
$counts = @{}
for ($entry = 0; $entry -lt $entryCount; $entry++) {
    $record = [Convert]::ToBase64String($sourceBytes, $entry * $width, $width)
    if (-not $counts.ContainsKey($record)) { $counts[$record] = 0 }
    $counts[$record]++
}
$emptyRecord = $counts.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 1
if ($emptyRecord.Value -lt 2) { throw 'No repeated empty server-list record was found.' }
$key = [Convert]::FromBase64String($emptyRecord.Key)
$output = [byte[]]::new($sourceBytes.Length)
[Array]::Copy($sourceBytes, $output, $sourceBytes.Length)
$changed = 0
for ($entry = 0; $entry -lt $entryCount; $entry++) {
    $offset = $entry * $width
    $plain = [byte[]]::new($width)
    for ($position = 0; $position -lt $width; $position++) {
        $plain[$position] = [byte](($sourceBytes[$offset + $position] - $key[$position] + 256) % 256)
    }
    $end = [Array]::IndexOf($plain, [byte]0)
    if ($end -lt 0) { throw "Server-list entry $entry is not null-terminated." }
    for ($position = 0; $position -lt $width; $position++) {
        if ($position -lt $end) {
            if ($plain[$position] -lt 32 -or $plain[$position] -gt 126) {
                throw "Server-list entry $entry contains a non-ASCII endpoint."
            }
        } elseif ($plain[$position] -ne 0) {
            throw "Server-list entry $entry has nonzero padding."
        }
    }
    if ($end -eq 0 -or ($entry % $slotsPerGroup) -eq 0) { continue }
    $address = [Text.Encoding]::ASCII.GetString($plain, 0, $end)
    if ($address -notmatch '^[A-Za-z0-9][A-Za-z0-9.-]*$') {
        throw "Server-list entry $entry is not a game-channel address."
    }
    $local = [Text.Encoding]::ASCII.GetBytes('127.0.0.1')
    for ($position = 0; $position -lt $width; $position++) {
        $value = if ($position -lt $local.Length) { [int]$local[$position] } else { 0 }
        $output[$offset + $position] = [byte](($value + $key[$position]) % 256)
    }
    $changed++
}
if ($changed -eq 0) { throw 'The server list has no configured game channels.' }

if ([IO.File]::Exists($destination)) {
    $existing = [IO.File]::ReadAllBytes($destination)
    if ([Convert]::ToBase64String($existing) -eq [Convert]::ToBase64String($output)) {
        Write-Output "Local server list is already current ($changed game channels): $destination"
        return
    }
    if (-not $Force) { throw 'The local override differs; use -Force to replace it.' }
}
[IO.File]::WriteAllBytes($destination, $output)
Write-Output "Created local server list ($changed game channels): $destination"
