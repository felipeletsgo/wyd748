[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'Get-VpsRuntimeInventory.ps1')
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$fixture = Join-Path $repo ('build/tests/vps-runtime-inventory-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $fixture -Force | Out-Null
function Write-Fixture([string]$Path) {
    $target = Join-Path $fixture $Path
    New-Item -ItemType Directory -Path (Split-Path $target) -Force | Out-Null
    [IO.File]::WriteAllText($target, 'fixture')
}
function Expect-Rejection([scriptblock]$Action, [string]$Message) {
    $rejected = $false
    try { & $Action | Out-Null } catch { $rejected = $_.Exception.Message -match $Message }
    if (!$rejected) { throw "Expected rejection: $Message" }
}
& git init --quiet $fixture
if ($LASTEXITCODE -ne 0) { throw 'Fixture Git initialization failed' }
$required = @(
    'spawns/NPCGener.txt', 'world/teleports.ini', 'world/init_items.csv',
    'spawns/instances/RuneTrack0.txt', 'spawns/instances/RuneTrack1.txt',
    'spawns/instances/RuneTrack2.txt', 'spawns/instances/RuneTrack3.txt',
    'spawns/instances/RuneTrack4.txt', 'spawns/instances/RuneTrack5.txt', 'spawns/instances/RuneTrack6.txt',
    'catalog/itemlist.csv', 'catalog/Itemname.csv', 'catalog/ItemEffect.h', 'catalog/SkillData.csv',
    'content/quest_zones.json', 'content/quests.json', 'content/droprate.json', 'content/volatiles.json',
    'content/instances.json', 'content/repliction.json', 'content/mounts.json', 'content/character_templates.json',
    'config/network_admission.json', 'config/client_integrity.json', 'world/maps/HeightMap.dat', 'world/maps/AttributeMap.dat'
)
foreach ($file in $required) { Write-Fixture "wydgo748/data/$file" }
Write-Fixture 'wydgo748/data/npcs/Existing.json'
Write-Fixture 'wydgo748/data/npcs/Removed.json'
& git -C $fixture add -- wydgo748/data
if ($LASTEXITCODE -ne 0) { throw 'Fixture index setup failed' }
Write-Fixture 'wydgo748/data/npcs/New.json'
Write-Fixture 'wydgo748/data/npcs/New.json.bak'
Write-Fixture 'wydgo748/data/npcs/error.log'
Write-Fixture 'wydgo748/data/config/server.txt'
Write-Fixture 'wydgo748/data/config/staff.json'
Write-Fixture 'wydgo748/data/content/boss/New.lua'
Write-Fixture 'wydgo748/data/npcs/Ignored.json'
[IO.File]::WriteAllText((Join-Path $fixture '.gitignore'), "wydgo748/data/npcs/Ignored.json`n")
$tracked = Get-VpsRuntimeInventory -Repository $fixture
if ($tracked.Files -contains 'wydgo748/data/npcs/New.json' -or $tracked.Files.Count -ne ($required.Count + 2)) { throw 'Tracked mode included untracked input' }
$explicit = Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @('wydgo748/data/npcs/New.json')
if ($explicit.Files -notcontains 'wydgo748/data/npcs/New.json') { throw 'Explicit runtime input was omitted' }
Remove-Item -LiteralPath (Join-Path $fixture 'wydgo748/data/npcs/Removed.json')
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture } 'Tracked runtime input is missing'
$current = Get-VpsRuntimeInventory -Repository $fixture -WorkingTree
foreach ($path in @('wydgo748/data/npcs/Existing.json', 'wydgo748/data/npcs/New.json', 'wydgo748/data/content/boss/New.lua')) {
    if ($current.Files -notcontains $path) { throw "Working tree omitted $path" }
}
foreach ($path in @('wydgo748/data/npcs/Removed.json', 'wydgo748/data/npcs/Ignored.json', 'wydgo748/data/npcs/New.json.bak', 'wydgo748/data/npcs/error.log', 'wydgo748/data/config/server.txt', 'wydgo748/data/config/staff.json')) {
    if ($current.Files -contains $path) { throw "Working tree included forbidden input $path" }
}
if ($current.Removed.Count -ne 1 -or $current.Removed[0] -ne 'wydgo748/data/npcs/Removed.json') { throw 'Removed input provenance was lost' }
foreach ($path in @('wydgo748/data/npcs/../config/server.txt', 'wydgo748/data/config/server.txt', 'wydgo748/data/npcs/error.log', 'wydgo748\data\npcs\New.json')) {
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree -AdditionalRuntimeData @($path) } 'Invalid additional runtime input'
}
Remove-Item -LiteralPath (Join-Path $fixture 'wydgo748/data/catalog/itemlist.csv')
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree } 'Missing runtime data: catalog/itemlist.csv'
Write-Output 'VPS runtime inventory checks passed: tracked, explicit, current tree, removed files, private/ignored outputs, unsafe paths, and missing required inputs.'
