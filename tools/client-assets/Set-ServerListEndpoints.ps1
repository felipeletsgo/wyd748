<#
.SYNOPSIS
Rewrites the game-channel endpoints of a WYD 7.48 server list.
.DESCRIPTION
Decodes and validates the 7.48 table, replaces every configured game channel
with one IPv4 literal (the client resolves endpoints with inet_addr), and can
replace the per-group population status URL. Group and slot layout, empty
records and the encoding are preserved. The output is written atomically.
.PARAMETER Address
IPv4 literal for every configured game channel.
.PARAMETER StatusUrl
Optional replacement for every configured group status URL (slot 0).
.PARAMETER KeepGroupCount
Optional. Clears every record of groups at or beyond this index, so only the
first groups named by sn.bin remain selectable.
.PARAMETER ChannelPorts
Optional channel-slot to TCP-port map. Existing explicit ports are preserved.
.PARAMETER EnsureSecondChannel
Adds channel 2 to the first group using the explicit ChannelPorts mapping.
.PARAMETER SourcePath
Input table. Defaults to tmproject/client748/serverlist.bin.
.PARAMETER DestinationPath
Output table. Defaults to the source path.
.PARAMETER Force
Replace an existing destination other than the source if its contents differ.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string]$Address,
    [string]$StatusUrl,
    [hashtable]$ChannelPorts = @{},
    [switch]$EnsureSecondChannel,
    [ValidateRange(1, 10)] [int]$KeepGroupCount = 10,
    [string]$SourcePath,
    [string]$DestinationPath,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$width = 64
$slotsPerGroup = 11
$entryCount = 110

$parsed = $null
if (-not [Net.IPAddress]::TryParse($Address, [ref]$parsed) -or
    $parsed.AddressFamily -ne [Net.Sockets.AddressFamily]::InterNetwork -or
    $Address -notmatch '^\d{1,3}(\.\d{1,3}){3}$') {
    throw 'Address must be a dotted IPv4 literal; the client does not resolve host names.'
}
if ($StatusUrl -and ($StatusUrl -notmatch '^https?://[\x21-\x7E]+$' -or $StatusUrl.Length -ge $width)) {
    throw "StatusUrl must be a printable ASCII http(s) URL shorter than $width bytes."
}
foreach ($slot in $ChannelPorts.Keys) {
    if ("$slot" -notmatch '^(10|[1-9])$' -or [int]$ChannelPorts[$slot] -lt 1 -or [int]$ChannelPorts[$slot] -gt 65535) {
        throw 'ChannelPorts must map channel slots 1..10 to TCP ports 1..65535.'
    }
}
function Game-Endpoint([int]$slot, [string]$current) {
    foreach ($configured in $ChannelPorts.Keys) {
        if ([int]$configured -eq $slot) {
            $port = [int]$ChannelPorts[$configured]
            if ($port -eq 8281) { return $Address }
            return "${Address}:$port"
        }
    }
    # Preserve a destination port when generating local overrides or changing IP.
    if ($current -match ':(\d+)$') { return "${Address}:$($Matches[1])" }
    return $Address
}

$assetRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../tmproject/client748'))
if (-not $SourcePath) { $SourcePath = Join-Path $assetRoot 'serverlist.bin' }
if (-not $DestinationPath) { $DestinationPath = $SourcePath }
$source = [IO.Path]::GetFullPath($SourcePath)
$destination = [IO.Path]::GetFullPath($DestinationPath)
$inPlace = [string]::Equals($source, $destination, [StringComparison]::OrdinalIgnoreCase)
if (-not [IO.File]::Exists($source)) { throw "Source table is missing: $source" }
if (-not [IO.Directory]::Exists([IO.Path]::GetDirectoryName($destination))) {
    throw 'The output directory does not exist.'
}

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

function Write-Record([byte[]]$table, [int]$entry, [string]$text) {
    $plain = [Text.Encoding]::ASCII.GetBytes($text)
    $offset = $entry * $width
    for ($position = 0; $position -lt $width; $position++) {
        $value = if ($position -lt $plain.Length) { [int]$plain[$position] } else { 0 }
        $table[$offset + $position] = [byte](($value + $key[$position]) % 256)
    }
}

$output = [byte[]]::new($sourceBytes.Length)
[Array]::Copy($sourceBytes, $output, $sourceBytes.Length)
$channels = 0
$statuses = 0
$cleared = 0
$firstChannel = $false
$secondChannel = $false
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
    if ($end -eq 0) { continue }
    if ([math]::Floor($entry / $slotsPerGroup) -ge $KeepGroupCount) {
        Write-Record $output $entry ''
        $cleared++
        continue
    }
    if (($entry % $slotsPerGroup) -eq 0) {
        if ($StatusUrl) { Write-Record $output $entry $StatusUrl; $statuses++ }
        continue
    }
    $current = [Text.Encoding]::ASCII.GetString($plain, 0, $end)
    if ($current -notmatch '^\d{1,3}(\.\d{1,3}){3}(:[1-9]\d{0,4})?$') {
        throw "Server-list entry $entry is not a game-channel address."
    }
    $hostPart, $portPart = $current.Split(':')
    $validIP = $null
    if (-not [Net.IPAddress]::TryParse($hostPart, [ref]$validIP) -or
        $validIP.AddressFamily -ne [Net.Sockets.AddressFamily]::InterNetwork -or
        ($portPart -and [int]$portPart -gt 65535)) { throw "Invalid endpoint at entry $entry." }
    if ($entry -eq 1) { $firstChannel = $true }
    if ($entry -eq 2) { $secondChannel = $true }
    Write-Record $output $entry (Game-Endpoint ($entry % $slotsPerGroup) $current)
    $channels++
}
if ($EnsureSecondChannel) {
    if (-not $firstChannel) { throw 'The first server group must have channel 1 before adding channel 2.' }
    if (-not $ChannelPorts.ContainsKey(2) -and -not $ChannelPorts.ContainsKey('2')) {
        throw 'EnsureSecondChannel requires an explicit port for channel 2.'
    }
    Write-Record $output 2 (Game-Endpoint 2 '')
    if (-not $secondChannel) { $channels++ }
}
if ($channels -eq 0) { throw 'The server list has no configured game channels.' }

$summary = "$channels game channels -> $Address" + $(if ($StatusUrl) { "; $statuses status URLs -> $StatusUrl" } else { '' }) +
    $(if ($cleared) { "; $cleared records cleared beyond group $($KeepGroupCount - 1)" } else { '' })
if ([IO.File]::Exists($destination)) {
    $existing = [IO.File]::ReadAllBytes($destination)
    if ([Convert]::ToBase64String($existing) -eq [Convert]::ToBase64String($output)) {
        Write-Output "Server list is already current ($summary): $destination"
        return
    }
    if (-not $inPlace -and -not $Force) { throw 'The destination differs; use -Force to replace it.' }
}
$temporary = "$destination.tmp-$PID"
[IO.File]::WriteAllBytes($temporary, $output)
Move-Item -LiteralPath $temporary -Destination $destination -Force
Write-Output "Wrote server list ($summary): $destination"
