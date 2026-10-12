<#
.SYNOPSIS
Publishes a patch ZIP from Build-ClientPatch.ps1 to the VPS launcher feed.
.DESCRIPTION
Reads the live /srv/wyd-go/patch/update.htm, appends "<Version> patch-<Version>.zip",
uploads both files and runs vps/publish-patch.sh with sudo. The ZIP is
installed before update.htm is replaced, so a launcher never sees a listed
patch that is missing.
.PARAMETER Version
Version passed to Build-ClientPatch.ps1.
.PARAMETER SshTarget
user@host with passwordless sudo.
.PARAMETER IdentityFile
Optional SSH private key.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)] [ValidateRange(10001, 19999)] [int]$Version,
    [Parameter(Mandatory)] [string]$SshTarget,
    [string]$IdentityFile
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$zipName = "patch-$Version.zip"
$zipPath = Join-Path $repo "build/patch/$zipName"
if (-not (Test-Path -LiteralPath $zipPath -PathType Leaf)) { throw "Build $zipName first." }

$openSsh = Join-Path $env:SystemRoot 'System32\OpenSSH'
$ssh = if (Test-Path -LiteralPath "$openSsh\ssh.exe") { "$openSsh\ssh.exe" } else { 'ssh' }
$scp = if (Test-Path -LiteralPath "$openSsh\scp.exe") { "$openSsh\scp.exe" } else { 'scp' }
$options = @('-o', 'BatchMode=yes', '-o', 'ConnectTimeout=15', '-o', 'StrictHostKeyChecking=yes')
if ($IdentityFile) { $options += @('-o', 'IdentitiesOnly=yes', '-i', $IdentityFile) }

$current = @(& $ssh @options $SshTarget 'cat /srv/wyd-go/patch/update.htm')
if ($LASTEXITCODE -ne 0 -or $current.Count -eq 0 -or $current[0] -ne '[WYD]') { throw 'Cannot read the live update.htm.' }
$versions = @($current | Select-Object -Skip 1 | ForEach-Object { [int](($_ -split ' ')[0]) })
if ($versions.Count -gt 0 -and $Version -le ($versions | Measure-Object -Maximum).Maximum) {
    throw "Version $Version is not newer than the published list."
}
$list = (@($current) + "$Version $zipName") -join "`n"
if ($list.Length -ge 8000) { throw 'update.htm would exceed the launcher''s single 8 KiB read.' }

$staging = Join-Path $repo "build/patch/publish-$Version"
New-Item -ItemType Directory -Path $staging | Out-Null
try {
    [IO.File]::WriteAllText((Join-Path $staging 'update.htm'), $list + "`n", [Text.Encoding]::ASCII)
    $remote = "/tmp/wyd-patch-$Version"
    & $ssh @options $SshTarget "mkdir -m 700 '$remote'"
    if ($LASTEXITCODE -ne 0) { throw 'Cannot create the remote staging directory.' }
    & $scp @options $zipPath (Join-Path $staging 'update.htm') (Join-Path $PSScriptRoot 'vps/publish-patch.sh') "${SshTarget}:$remote/"
    if ($LASTEXITCODE -ne 0) { throw 'Upload failed.' }
    & $ssh @options $SshTarget "sudo bash '$remote/publish-patch.sh' '$remote' '$zipName'"
    if ($LASTEXITCODE -ne 0) { throw 'Remote publication failed; the previous update.htm remains live.' }
} finally {
    $resolvedStaging = [IO.Path]::GetFullPath($staging)
    $expectedStaging = [IO.Path]::GetFullPath((Join-Path $repo "build/patch/publish-$Version"))
    if ($resolvedStaging -ne $expectedStaging -or -not $resolvedStaging.StartsWith($repo + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Refusing cleanup outside the patch publication directory.'
    }
    Remove-Item -LiteralPath $staging -Recurse -Force -ErrorAction SilentlyContinue
}
