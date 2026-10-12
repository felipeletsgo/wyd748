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
if ([string]::Equals([IO.Path]::GetFullPath($SourcePath), [IO.Path]::GetFullPath($DestinationPath),
        [StringComparison]::OrdinalIgnoreCase)) {
    throw 'The output must differ from the source table.'
}
& (Join-Path $PSScriptRoot 'Set-ServerListEndpoints.ps1') -Address '127.0.0.1' `
    -SourcePath $SourcePath -DestinationPath $DestinationPath -Force:$Force
