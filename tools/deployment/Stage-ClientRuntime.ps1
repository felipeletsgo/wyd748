<#
.SYNOPSIS
Stage the signed x86 VC++ installer without installing or publishing it.
#>
[CmdletBinding()]
param(
    [string]$OutputDirectory,
    [string]$VisualStudioPath
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $repo 'build/client-runtime' }
$output = [IO.Path]::GetFullPath($OutputDirectory)
$buildRoot = [IO.Path]::GetFullPath((Join-Path $repo 'build')) + [IO.Path]::DirectorySeparatorChar
if (-not $output.StartsWith($buildRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Stage runtime prerequisites only under the root build directory.'
}
if (-not $VisualStudioPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    $VisualStudioPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -ne 0 -or -not $VisualStudioPath) { throw 'Visual Studio C++ tools are required.' }
}
$vc = Join-Path $VisualStudioPath 'VC'
$toolset = ([IO.File]::ReadAllText((Join-Path $vc 'Auxiliary/Build/Microsoft.VCToolsVersion.default.txt'))).Trim()
$headerPath = Join-Path $vc "Tools/MSVC/$toolset/include/crtversion.h"
$header = [IO.File]::ReadAllText($headerPath)
$parts = foreach ($name in @('MAJOR', 'MINOR', 'BUILD', 'RBUILD')) {
    $match = [regex]::Match($header, "(?m)^#define _VC_CRT_$($name)_VERSION\s+(\d+)")
    if (-not $match.Success) { throw "Cannot read runtime $name version." }
    $match.Groups[1].Value
}
$required = [version]($parts -join '.')
$installer = Get-ChildItem -LiteralPath (Join-Path $vc 'Redist/MSVC') -Filter 'vc_redist.x86.exe' -Recurse -File |
    Where-Object { [version]$_.VersionInfo.FileVersion -ge $required } |
    Sort-Object { [version]$_.VersionInfo.FileVersion } -Descending | Select-Object -First 1
if (-not $installer) { throw "No x86 redistributable meets the toolset requirement $required." }
$signature = Get-AuthenticodeSignature -LiteralPath $installer.FullName
if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'O=Microsoft Corporation(?:,|$)') {
    throw "Runtime signature rejected: $($signature.Status)."
}
$destination = Join-Path $output 'prerequisites/vc_redist.x86.exe'
$null = New-Item -ItemType Directory -Path (Split-Path $destination) -Force
Copy-Item -LiteralPath $installer.FullName -Destination $destination
$hash = (Get-FileHash -LiteralPath $installer.FullName -Algorithm SHA256).Hash
if ((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash -cne $hash) { throw 'Staged runtime hash mismatch.' }
$receipt = [ordered]@{
    schemaVersion=1; source=$installer.FullName; destination=$destination
    toolset=$toolset; requiredVersion=$required.ToString(); installerVersion=$installer.VersionInfo.FileVersion
    size=(Get-Item -LiteralPath $destination).Length; sha256=$hash
    signer=$signature.SignerCertificate.Subject; signerThumbprint=$signature.SignerCertificate.Thumbprint
    signatureStatus=$signature.Status.ToString(); architecture='x86'
    installed=$false; published=$false
}
$receipt | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $output 'runtime-receipt.json') -Encoding utf8
[pscustomobject]$receipt
