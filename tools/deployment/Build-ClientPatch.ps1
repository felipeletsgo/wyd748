<#
.SYNOPSIS
Builds an incremental client patch ZIP for WYDLauncher.exe.
.DESCRIPTION
Collects every file under tmproject/client748 that was added or changed
between two Git revisions (never uncommitted working-tree edits) and writes
build/patch/patch-<Version>.zip with paths relative to the client folder.
WYDLauncher.exe extracts the ZIP over the client and records <Version> in
config.txt. Publish it with Publish-ClientPatch.ps1.

The launcher cannot delete files, cannot replace itself while running, and
must not overwrite player settings, so deletions are reported and
WYDLauncher.exe, config.txt and ClientSettings.txt are refused or skipped.
.PARAMETER From
Revision that players already have.
.PARAMETER To
Revision to publish. Defaults to HEAD.
.PARAMETER Version
New client version (10001..19999), greater than every published version.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string]$From,
    [string]$To = 'HEAD',
    [Parameter(Mandatory)] [ValidateRange(10001, 19999)] [int]$Version
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem

$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$prefix = 'tmproject/client748/'
$skipped = @('config.txt', 'ClientSettings.txt')
$refused = @('WYDLauncher.exe', 'update.dat')

$changed = @(git -C $repo -c core.quotepath=off diff --name-only --diff-filter=ACMRT $From $To -- $prefix)
if ($LASTEXITCODE -ne 0) { throw "git diff $From..$To failed." }
$deleted = @(git -C $repo -c core.quotepath=off diff --name-only --diff-filter=D $From $To -- $prefix)
foreach ($path in $deleted) { Write-Warning "Deleted in $To but left on players' disks: $path" }

$entries = [Collections.Generic.List[string]]::new()
foreach ($path in $changed) {
    $relative = $path.Substring($prefix.Length)
    if ($refused -contains $relative) { throw "$relative cannot be patched by the running launcher; ship a new full client." }
    if ($skipped -contains $relative) { Write-Warning "Skipping player settings file: $relative"; continue }
    if ($relative -notmatch '^[A-Za-z0-9._ -]+(/[A-Za-z0-9._ -]+)*$') { throw "Unsupported path for the 7.4x unzipper: $relative" }
    $entries.Add($relative)
}
if ($entries.Count -eq 0) { throw "No client files changed between $From and $To." }

$outDir = Join-Path $repo 'build/patch'
New-Item -ItemType Directory -Path $outDir -Force | Out-Null
$zipPath = Join-Path $outDir "patch-$Version.zip"
if (Test-Path -LiteralPath $zipPath) { throw "Refusing to overwrite $zipPath." }

$zip = [IO.Compression.ZipFile]::Open($zipPath, [IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($relative in $entries) {
        # Bytes come from Git with the Windows checkout conversion pinned, so
        # text assets match a fresh Windows clone (CRLF) on every host.
        $temp = [IO.Path]::GetTempFileName()
        try {
            $process = Start-Process -FilePath git -NoNewWindow -Wait -PassThru -RedirectStandardOutput $temp `
                -ArgumentList @('-C', $repo, '-c', 'core.autocrlf=true', 'cat-file', '--filters', "${To}:$prefix$relative")
            if ($process.ExitCode -ne 0) { throw "git cat-file $To`:$prefix$relative failed." }
            $entry = $zip.CreateEntry($relative, [IO.Compression.CompressionLevel]::Optimal)
            $stream = $entry.Open()
            try { $bytes = [IO.File]::ReadAllBytes($temp); $stream.Write($bytes, 0, $bytes.Length) } finally { $stream.Dispose() }
        } finally { Remove-Item -LiteralPath $temp -Force -ErrorAction SilentlyContinue }
    }
} finally { $zip.Dispose() }

[pscustomobject]@{
    Version = $Version
    From = (git -C $repo rev-parse --short $From)
    To = (git -C $repo rev-parse --short $To)
    Files = $entries.Count
    Zip = $zipPath
    KiB = [math]::Round((Get-Item -LiteralPath $zipPath).Length / 1KB, 1)
}
