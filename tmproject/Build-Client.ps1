<#
.SYNOPSIS
Builds the x86 client and deploys the candidate after verifying its SHA-256 identity.
.DESCRIPTION
Local build infrastructure. Does not start or stop processes.
A build or copy failure prevents the candidate from being reported as validated.
.PARAMETER Configuration
Selects Debug or Release; both use the solution's x86 configuration.
.PARAMETER Rebuild
Recompiles all objects instead of using an incremental build.
.PARAMETER MSBuildPath
Optional path to MSBuild.exe; defaults to the Visual Studio installation.
.PARAMETER NoDeploy
Builds and validates without copying the client to client748/WYD.exe. Used by CI.
#>
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [switch]$Rebuild,
    [switch]$NoDeploy,
    [string]$MSBuildPath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# Resolve all paths from this script, regardless of the shell's working directory.
$solution = Join-Path $PSScriptRoot 'WYDESTINY.sln'
$candidate = Join-Path $PSScriptRoot 'client748\WYD.exe'
$wideItemCatalog = Join-Path $PSScriptRoot 'client748\ItemListWide.csv'
$targetName = if ($Configuration -eq 'Release') { 'WYD.exe' } else { 'WYDestiny.exe' }
$artifact = Join-Path $PSScriptRoot "build\TMProject748\$Configuration\$targetName"
$reporter = Join-Path (Split-Path $artifact -Parent) 'WydCrashReporter.dll'

if (-not (Test-Path -LiteralPath $wideItemCatalog -PathType Leaf)) {
    throw "Required client contract asset is missing: $wideItemCatalog"
}

if (-not $MSBuildPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $matches = @(& $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe')
        if ($LASTEXITCODE -ne 0) { throw 'MSBuild discovery failed.' }
        if ($matches.Count -gt 0) { $MSBuildPath = $matches[0] }
    }
    if (-not $MSBuildPath) {
        $command = Get-Command MSBuild.exe -ErrorAction SilentlyContinue
        if ($command) { $MSBuildPath = $command.Source }
    }
}
if (-not $MSBuildPath -or -not (Test-Path -LiteralPath $MSBuildPath -PathType Leaf)) {
    throw 'MSBuild not found. Supply -MSBuildPath for an existing installation.'
}
$buildTarget = if ($Rebuild) { 'Rebuild' } else { 'Build' }
# The compiled table must match the manifest and cataloged assets.
& (Join-Path $PSScriptRoot '..\tools\client-assets\Export-CostumeTable.ps1') -Check
& (Join-Path $PSScriptRoot '..\tools\client-assets\Sync-SkinShaders.ps1') -Check
& (Join-Path $PSScriptRoot '..\tools\client-assets\Test-SkinShaders.ps1')
# Run the pure test gate before building or copying the client; a failure preserves the candidate.
$testProject = Join-Path $PSScriptRoot 'TMProject748\tests\ArchitectureTests.vcxproj'
& $MSBuildPath $testProject "/t:$buildTarget" "/p:Configuration=$Configuration" '/p:Platform=Win32' /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw 'Architecture tests failed to build; candidate not updated.' }
$testExecutable = Join-Path $PSScriptRoot "build\tests\$Configuration\ArchitectureTests.exe"
if (-not (Test-Path -LiteralPath $testExecutable -PathType Leaf)) { throw 'Architecture test executable is missing.' }
& $testExecutable
if ($LASTEXITCODE -ne 0) { throw 'Architecture tests failed; candidate not updated.' }

# Exercise production socket framing against fragmented buffers and loopback TCP.
$socketTestProject = Join-Path $PSScriptRoot 'TMProject748\tests\SocketReceiveTests.vcxproj'
& $MSBuildPath $socketTestProject "/t:$buildTarget" "/p:Configuration=$Configuration" '/p:Platform=Win32' /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw 'Socket receive tests failed to build; candidate not updated.' }
$socketTestExecutable = Join-Path $PSScriptRoot "build\tests\$Configuration\SocketReceiveTests.exe"
if (-not (Test-Path -LiteralPath $socketTestExecutable -PathType Leaf)) { throw 'Socket receive test executable is missing.' }
& $socketTestExecutable
if ($LASTEXITCODE -ne 0) { throw 'Socket receive tests failed; candidate not updated.' }

# Crash only isolated console subprocesses; never launch the game for this gate.
$crashTestProject = Join-Path $PSScriptRoot 'TMProject748\tests\CrashCaptureTests.vcxproj'
& $MSBuildPath $crashTestProject "/t:$buildTarget" "/p:Configuration=$Configuration" '/p:Platform=Win32' /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw 'Crash capture tests failed to build; candidate not updated.' }
$crashTestExecutable = Join-Path $PSScriptRoot "build\tests\$Configuration\CrashCaptureTests.exe"
if (-not (Test-Path -LiteralPath $crashTestExecutable -PathType Leaf)) { throw 'Crash capture test executable is missing.' }
& $crashTestExecutable
if ($LASTEXITCODE -ne 0) { throw 'Crash capture tests failed; candidate not updated.' }

# Validate the source launcher's manifest, path and cache rules without replacing
# the separately distributed launcher, which uses a different production feed.
$launcherTestProject = Join-Path $PSScriptRoot 'WYDLauncher\tests\LauncherTests.vcxproj'
& $MSBuildPath $launcherTestProject "/t:$buildTarget" "/p:Configuration=$Configuration" '/p:Platform=Win32' /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw 'Launcher tests failed to build; candidate not updated.' }
$launcherTestExecutable = Join-Path $PSScriptRoot "build\tests\$Configuration\LauncherTests.exe"
if (-not (Test-Path -LiteralPath $launcherTestExecutable -PathType Leaf)) { throw 'Launcher test executable is missing.' }
& $launcherTestExecutable
if ($LASTEXITCODE -ne 0) { throw 'Launcher tests failed; candidate not updated.' }

& $MSBuildPath $solution "/t:$buildTarget" "/p:Configuration=$Configuration" '/p:Platform=x86' /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "MSBuild failed with exit code $LASTEXITCODE; candidate not updated." }
if (-not (Test-Path -LiteralPath $artifact -PathType Leaf)) { throw "Build artifact is missing: $artifact" }
if (-not (Test-Path -LiteralPath $reporter -PathType Leaf)) { throw 'Crash reporter artifact is missing.' }

if ($NoDeploy) {
    [pscustomobject]@{
        Configuration = $Configuration
        Artifact = $artifact
        SHA256 = (Get-FileHash -LiteralPath $artifact -Algorithm SHA256).Hash
        Reporter = $reporter
        ReporterSHA256 = (Get-FileHash -LiteralPath $reporter -Algorithm SHA256).Hash
        Validation = 'BUILD_VERIFIED; deployment and in-game testing pending'
    }
    return
}

if (-not (Test-Path -LiteralPath (Split-Path $candidate -Parent) -PathType Container)) {
    throw 'client748 directory is missing; candidate deployment canceled.'
}

# Synchronous copy: a locked file causes failure; never terminate the game process.
$sourceHash = (Get-FileHash -LiteralPath $artifact -Algorithm SHA256).Hash
$reporterHash = (Get-FileHash -LiteralPath $reporter -Algorithm SHA256).Hash
Copy-Item -LiteralPath $reporter -Destination (Split-Path $candidate -Parent) -Force
if ((Get-FileHash -LiteralPath (Join-Path (Split-Path $candidate -Parent) 'WydCrashReporter.dll') -Algorithm SHA256).Hash -ne $reporterHash) {
    throw 'Installed crash reporter differs from the build artifact.'
}
Copy-Item -LiteralPath $artifact -Destination $candidate -Force
$candidateHash = (Get-FileHash -LiteralPath $candidate -Algorithm SHA256).Hash
if ($sourceHash -ne $candidateHash) { throw 'Candidate hash differs from the build artifact.' }
[pscustomobject]@{
    Configuration = $Configuration
    Artifact = $artifact
    Candidate = $candidate
    SHA256 = $candidateHash
    Validation = 'BUILD_AND_DEPLOY_VERIFIED; in-game testing pending'
}
