[CmdletBinding()]
param(
    [string]$ReleaseId = ('wyd-deploy-' + (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssZ')),
    [string[]]$AdditionalRuntimeData = @(),
    [switch]$WorkingTree,
    [switch]$GameOnly
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if ($ReleaseId -notmatch '^wyd-deploy-[A-Za-z0-9-]+$') { throw 'Invalid release ID' }
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
. (Join-Path $PSScriptRoot 'Get-VpsRuntimeInventory.ps1')
$inventory = Get-VpsRuntimeInventory -Repository $repo -WorkingTree:$WorkingTree -AdditionalRuntimeData $AdditionalRuntimeData
function Get-InputRecords([string[]]$Paths, [string]$Root) {
    @($Paths | Sort-Object -Unique | ForEach-Object {
        $path = Join-Path $Root $_
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            [ordered]@{ path = $_; size = (Get-Item -LiteralPath $path).Length; sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() }
        }
    })
}
$sourcePaths = @(& git -C $repo -c core.quotepath=false ls-files --cached --others --exclude-standard -- wydgo748 tools/deployment)
if ($LASTEXITCODE -ne 0) { throw 'Source inventory failed' }
$sourcePaths = @($sourcePaths | Where-Object {
    $_ -match '^wydgo748/(?:.*\.go|go\.(?:mod|sum)|api/.*|web/portal/(?:package(?:-lock)?\.json|astro\.config\.mjs|tsconfig\.json|src/.*|public/.*))$|^tools/deployment/' -and
    $_ -notmatch '/(?:node_modules|dist|__pycache__)/'
}) + @($inventory.Files)
$sourceRecords = @(Get-InputRecords $sourcePaths $repo)
$head = & git -C $repo rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw 'HEAD lookup failed' }
$dirty = @(& git -C $repo -c core.quotepath=false status --porcelain=v1 --untracked-files=all)
if ($LASTEXITCODE -ne 0) { throw 'Working tree status failed' }
$goVersion = & go version
if ($LASTEXITCODE -ne 0) { throw 'Go version lookup failed' }
$portalBuild = $null
if (!$GameOnly) {
    $nodeVersion = & node --version
    if ($LASTEXITCODE -ne 0) { throw 'Node version lookup failed' }
    $npmVersion = & npm --version
    if ($LASTEXITCODE -ne 0) { throw 'npm version lookup failed' }
    $portalBuild = [ordered]@{ node = $nodeVersion; npm = $npmVersion; command = 'npm run build'; lockfile = 'wydgo748/web/portal/package-lock.json' }
}
$stage = Join-Path $repo "build/deploy/$ReleaseId"
if (Test-Path -LiteralPath $stage) { throw 'Release staging already exists; refusing overwrite' }
$release = Join-Path $stage 'release'
New-Item -ItemType Directory -Path "$release/game/data", "$release/web", "$stage/config" -Force | Out-Null
Copy-Item -LiteralPath "$PSScriptRoot/vps/data-layout.version" -Destination "$release/game/data-layout.version"
$priorGoOS, $priorGoArch, $priorCGO = $env:GOOS, $env:GOARCH, $env:CGO_ENABLED
try {
    $env:GOOS='linux'; $env:GOARCH='amd64'; $env:CGO_ENABLED='0'
    Push-Location (Join-Path $repo 'wydgo748')
    try {
        go build -trimpath -o "$release/game/wyd-server" ./cmd/server
        if ($LASTEXITCODE -ne 0) { throw 'Game cross-build failed' }
        if (!$GameOnly) {
        go build -trimpath -o "$release/game/wyd-player-api" ./cmd/player-api
        if ($LASTEXITCODE -ne 0) { throw 'Player API cross-build failed' }
        go build -trimpath -o "$release/game/wyd-log-reader" ./cmd/log-reader
        if ($LASTEXITCODE -ne 0) { throw 'Log reader cross-build failed' }
        go test -c -o "$stage/playerportal.test" ./internal/playerportal
        if ($LASTEXITCODE -ne 0) { throw 'Database test cross-build failed' }
        }
    } finally { Pop-Location }
} finally {
    $env:GOOS=$priorGoOS; $env:GOARCH=$priorGoArch; $env:CGO_ENABLED=$priorCGO
}
foreach ($file in $inventory.Files) {
    $relative = $file.Substring('wydgo748/data/'.Length)
    $target = Join-Path "$release/game/data" $relative
    New-Item -ItemType Directory -Path (Split-Path $target) -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $repo $file) -Destination $target
}
if (!$GameOnly) {
$dist = Join-Path $repo 'wydgo748/web/portal/dist'
if (!(Test-Path -LiteralPath "$dist/register/index.html")) { throw 'Build the public portal before packaging' }
foreach ($entry in Get-ChildItem -LiteralPath $dist) {
    if ($entry.Name -eq 'admin') { continue }
    Copy-Item -LiteralPath $entry.FullName -Destination "$release/web" -Recurse
}
# The staff panel is served only by the game's loopback listener (behind the
# authenticated /admin proxy), never from the public static root.
if (!(Test-Path -LiteralPath "$dist/admin/index.html")) { throw 'Build the admin panel before packaging' }
Copy-Item -LiteralPath $dist -Destination "$release/game/web" -Recurse
}
# Files only: local caches such as __pycache__ must never reach the VPS.
Get-ChildItem -LiteralPath "$PSScriptRoot/vps" -File | Copy-Item -Destination "$stage/config"
$currentInventory = Get-VpsRuntimeInventory -Repository $repo -WorkingTree:$WorkingTree -AdditionalRuntimeData $AdditionalRuntimeData
$currentPaths = @(& git -C $repo -c core.quotepath=false ls-files --cached --others --exclude-standard -- wydgo748 tools/deployment)
if ($LASTEXITCODE -ne 0) { throw 'Source inventory recheck failed' }
$currentPaths = @($currentPaths | Where-Object {
    $_ -match '^wydgo748/(?:.*\.go|go\.(?:mod|sum)|api/.*|web/portal/(?:package(?:-lock)?\.json|astro\.config\.mjs|tsconfig\.json|src/.*|public/.*))$|^tools/deployment/' -and
    $_ -notmatch '/(?:node_modules|dist|__pycache__)/'
}) + @($currentInventory.Files)
if ((ConvertTo-Json -InputObject $sourceRecords -Depth 4 -Compress) -cne
    (ConvertTo-Json -InputObject @(Get-InputRecords $currentPaths $repo) -Depth 4 -Compress)) {
    throw 'Release inputs changed during packaging; rebuild from a stable snapshot'
}
if ($GameOnly) {
    tar -czf "$stage/release.tar.gz" -C $release game
} else {
    tar -czf "$stage/release.tar.gz" -C $release game web
}
if ($LASTEXITCODE -ne 0) { throw 'Release archive failed' }
$hash = (Get-FileHash -LiteralPath "$stage/release.tar.gz" -Algorithm SHA256).Hash.ToLowerInvariant()
# Generated checksum metadata is a mechanical artifact, not authored source.
[IO.File]::WriteAllText("$stage/release.sha256", "$hash  release.tar.gz`n", [Text.UTF8Encoding]::new($false))
$payloadPaths = @(Get-ChildItem -LiteralPath $release -File -Recurse | ForEach-Object { [IO.Path]::GetRelativePath($release, $_.FullName).Replace('\', '/') })
$configPaths = @(Get-ChildItem -LiteralPath "$stage/config" -File | ForEach-Object { "config/$($_.Name)" })
$manifest = [ordered]@{
    version = 1; releaseId = $ReleaseId
    mode = $(if ($WorkingTree) { 'working-tree' } else { 'tracked-working-tree' })
    head = $head; dirty = $dirty; gameOnly = [bool]$GameOnly
    build = [ordered]@{ go = $goVersion; os = 'linux'; arch = 'amd64'; cgo = '0'; trimpath = $true; portal = $portalBuild }
    sources = $sourceRecords; removedRuntimeInputs = @($inventory.Removed)
    payload = @(Get-InputRecords $payloadPaths $release)
    configuration = @(Get-InputRecords $configPaths $stage)
    archive = [ordered]@{ path = 'release.tar.gz'; size = (Get-Item -LiteralPath "$stage/release.tar.gz").Length; sha256 = $hash }
}
[IO.File]::WriteAllText("$stage/release-manifest.json", (ConvertTo-Json -InputObject $manifest -Depth 8), [Text.UTF8Encoding]::new($false))
Write-Output "Release staging: $stage"
Write-Output "Release SHA256: $hash"
