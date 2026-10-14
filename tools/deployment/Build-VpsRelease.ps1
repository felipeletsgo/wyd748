[CmdletBinding()]
param([string]$ReleaseId = ('wyd-deploy-' + (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssZ')))
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
if ($ReleaseId -notmatch '^wyd-deploy-[A-Za-z0-9-]+$') { throw 'Invalid release ID' }
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$stage = Join-Path $repo "build/deploy/$ReleaseId"
if (Test-Path -LiteralPath $stage) { throw 'Release staging already exists; refusing overwrite' }
$release = Join-Path $stage 'release'
New-Item -ItemType Directory -Path "$release/game/data", "$release/web", "$stage/config" -Force | Out-Null
$priorGoOS, $priorGoArch, $priorCGO = $env:GOOS, $env:GOARCH, $env:CGO_ENABLED
try {
    $env:GOOS='linux'; $env:GOARCH='amd64'; $env:CGO_ENABLED='0'
    Push-Location (Join-Path $repo 'wydgo748')
    try {
        go build -trimpath -o "$release/game/wyd-server" ./cmd/server
        if ($LASTEXITCODE -ne 0) { throw 'Game cross-build failed' }
        go build -trimpath -o "$release/game/wyd-player-api" ./cmd/player-api
        if ($LASTEXITCODE -ne 0) { throw 'Player API cross-build failed' }
        go test -c -o "$stage/playerportal.test" ./internal/playerportal
        if ($LASTEXITCODE -ne 0) { throw 'Database test cross-build failed' }
    } finally { Pop-Location }
} finally {
    $env:GOOS=$priorGoOS; $env:GOARCH=$priorGoArch; $env:CGO_ENABLED=$priorCGO
}
$required = 'NPCGener.txt|teleports.ini|quest_zones.json|quests.json|init_items.csv|itemlist.csv|Itemname.csv|ItemEffect.h|SkillData.csv|droprate.json|volatiles.json|instances.json|repliction.json|mounts.json|character_templates.json|network_admission.json|client_integrity.json'
$files = @(git -C $repo ls-files wydgo748/data)
if ($LASTEXITCODE -ne 0) { throw 'Data inventory failed' }
foreach ($file in $files) {
    if ($file -notmatch "^wydgo748/data/(?:npcs/|maps/|boss/|NPCGener/|(?:$required)$)") { continue }
    $relative = $file.Substring('wydgo748/data/'.Length)
    $target = Join-Path "$release/game/data" $relative
    New-Item -ItemType Directory -Path (Split-Path $target) -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $repo $file) -Destination $target
}
foreach ($requiredFile in $required.Split('|')) {
    if (!(Test-Path -LiteralPath "$release/game/data/$requiredFile")) { throw "Missing runtime data: $requiredFile" }
}
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
# Files only: local caches such as __pycache__ must never reach the VPS.
Get-ChildItem -LiteralPath "$PSScriptRoot/vps" -File | Copy-Item -Destination "$stage/config"
tar -czf "$stage/release.tar.gz" -C $release game web
if ($LASTEXITCODE -ne 0) { throw 'Release archive failed' }
$hash = (Get-FileHash -LiteralPath "$stage/release.tar.gz" -Algorithm SHA256).Hash.ToLowerInvariant()
# Generated checksum metadata is a mechanical artifact, not authored source.
[IO.File]::WriteAllText("$stage/release.sha256", "$hash  release.tar.gz`n", [Text.UTF8Encoding]::new($false))
Write-Output "Release staging: $stage"
Write-Output "Release SHA256: $hash"
