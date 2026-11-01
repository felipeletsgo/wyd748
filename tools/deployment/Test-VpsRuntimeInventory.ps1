[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'Get-VpsRuntimeInventory.ps1')
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$fixture = Join-Path $repo ('build/tests/vps-runtime-inventory-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $fixture -Force | Out-Null
function Get-FixturePath([string]$Path) {
    $target = [IO.Path]::GetFullPath((Join-Path $fixture $Path))
    $prefix = [IO.Path]::GetFullPath($fixture) + [IO.Path]::DirectorySeparatorChar
    if (!$target.StartsWith($prefix, [StringComparison]::Ordinal)) { throw "Fixture path escaped its root: $Path" }
    $target
}
function Write-Fixture([string]$Path) {
    $target = Get-FixturePath $Path
    New-Item -ItemType Directory -Path (Split-Path $target) -Force | Out-Null
    [IO.File]::WriteAllText($target, 'fixture')
}
function Expect-Rejection([scriptblock]$Action, [string]$Message) {
    $actual = $null
    try { & $Action | Out-Null } catch { $actual = $_.Exception.Message }
    if ($null -eq $actual -or $actual -notmatch $Message) { throw "Expected rejection: $Message; received: $actual" }
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
    'content/language/en.txt', 'content/language/pt.txt',
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
Write-Fixture 'wydgo748/data/content/language/en.txt.bak'
Write-Fixture 'wydgo748/data/content/language/fr.txt'
Write-Fixture 'wydgo748/data/config/game.env'
Write-Fixture 'wydgo748/data/config/server.txt'
Write-Fixture 'wydgo748/data/config/staff.json'
Write-Fixture 'wydgo748/data/content/boss/New.lua'
Write-Fixture 'wydgo748/data/npcs/Ignored.json'
[IO.File]::WriteAllText((Join-Path $fixture '.gitignore'), "wydgo748/data/npcs/Ignored.json`n")
$tracked = Get-VpsRuntimeInventory -Repository $fixture
if ($tracked.Files -contains 'wydgo748/data/npcs/New.json' -or $tracked.Files.Count -ne ($required.Count + 2)) { throw 'Tracked mode included untracked input' }
$explicit = Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @('wydgo748/data/npcs/New.json')
if ($explicit.Files -notcontains 'wydgo748/data/npcs/New.json') { throw 'Explicit runtime input was omitted' }
$duplicates = Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @(
    'wydgo748/data/npcs/New.json', 'wydgo748/data/npcs/Existing.json', 'wydgo748/data/npcs/New.json')
if ($duplicates.Files.Count -ne $explicit.Files.Count -or
    ($duplicates.Files -join "`n") -cne ($explicit.Files -join "`n")) { throw 'Explicit duplicates changed the inventory or its order' }
$ignored = Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @('wydgo748/data/npcs/Ignored.json')
if ($ignored.Files -notcontains 'wydgo748/data/npcs/Ignored.json') { throw 'Explicit ignored input was omitted' }
$forbidden = @('wydgo748/data/config/server.txt', 'wydgo748/data/config/staff.json', 'wydgo748/data/config/game.env',
    'wydgo748/data/npcs/New.json.bak', 'wydgo748/data/npcs/error.log',
    'wydgo748/data/content/language/en.txt.bak', 'wydgo748/data/content/language/fr.txt')
& git -C $fixture add -- @forbidden
if ($LASTEXITCODE -ne 0) { throw 'Forbidden fixture tracking failed' }
$filtered = Get-VpsRuntimeInventory -Repository $fixture
foreach ($path in $forbidden) {
    if ($filtered.Files -contains $path) { throw "Tracked forbidden input was included: $path" }
}
Remove-Item -LiteralPath (Get-FixturePath 'wydgo748/data/npcs/Removed.json')
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture } 'Tracked runtime input is missing'
$current = Get-VpsRuntimeInventory -Repository $fixture -WorkingTree
foreach ($path in @('wydgo748/data/npcs/Existing.json', 'wydgo748/data/npcs/New.json', 'wydgo748/data/content/boss/New.lua',
    'wydgo748/data/content/language/en.txt', 'wydgo748/data/content/language/pt.txt')) {
    if ($current.Files -notcontains $path) { throw "Working tree omitted $path" }
}
foreach ($path in @('wydgo748/data/npcs/Removed.json', 'wydgo748/data/npcs/Ignored.json') + $forbidden) {
    if ($current.Files -contains $path) { throw "Working tree included forbidden input $path" }
}
if ($current.Removed.Count -ne 1 -or $current.Removed[0] -ne 'wydgo748/data/npcs/Removed.json') { throw 'Removed input provenance was lost' }
foreach ($path in @('', 'wydgo748/data/npcs/../config/server.txt', 'wydgo748/data/npcs/..\config\staff.json',
    'wydgo748/data/config/server.txt', 'wydgo748/data/npcs/error.log', 'wydgo748\data\npcs\New.json',
    'wydgo748/data/content/language/fr.txt', 'wydgo748/data/content/language/en.txt.bak',
    'wydgo748/data/content/language/EN.txt', 'wydgo748/data/npcs/Missing.json',
    (Get-FixturePath 'wydgo748/data/npcs/Existing.json'))) {
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree -AdditionalRuntimeData @($path) } 'Invalid additional runtime input'
}
New-Item -ItemType Directory -Path (Get-FixturePath 'wydgo748/data/npcs/Directory.json') | Out-Null
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @('wydgo748/data/npcs/Directory.json') } 'Invalid additional runtime input'
Write-Fixture 'wydgo748/data/npcs/Removed.json'
foreach ($name in @('en.txt', 'pt.txt')) {
    $relative = "wydgo748/data/content/language/$name"
    $missing = [regex]::Escape("Missing runtime data: content/language/$name")
    Remove-Item -LiteralPath (Get-FixturePath $relative)
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture } 'Tracked runtime input is missing'
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree } $missing
    Write-Fixture $relative
    & git -C $fixture update-index --force-remove -- $relative
    if ($LASTEXITCODE -ne 0) { throw 'Language fixture index removal failed' }
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture } $missing
    $untracked = Get-VpsRuntimeInventory -Repository $fixture -WorkingTree
    if ($untracked.Files -cnotcontains $relative) { throw "Working tree omitted untracked $name" }
    $selected = Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @($relative, $relative)
    if (@($selected.Files | Where-Object { $_ -ceq $relative }).Count -ne 1) { throw "Explicit $name was missing or duplicated" }
    [IO.File]::WriteAllText((Join-Path $fixture '.gitignore'), "wydgo748/data/npcs/Ignored.json`n$relative`n")
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree } $missing
    $selected = Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @($relative)
    if ($selected.Files -cnotcontains $relative) { throw "Explicit ignored $name was omitted" }
    [IO.File]::WriteAllText((Join-Path $fixture '.gitignore'), "wydgo748/data/npcs/Ignored.json`n")
    & git -C $fixture add -- $relative
    if ($LASTEXITCODE -ne 0) { throw 'Language fixture index restore failed' }
}
Remove-Item -LiteralPath (Get-FixturePath 'wydgo748/data/catalog/itemlist.csv')
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree } 'Missing runtime data: catalog/itemlist.csv'
Write-Fixture 'wydgo748/data/catalog/itemlist.csv'
# Check file links before the final parent-directory link case.
$linkedFile = Get-FixturePath 'wydgo748/data/npcs/Linked.json'
$ignoredLink = Get-FixturePath 'wydgo748/data/npcs/Linked.tmp'
$fileLinkCreated = $false
try {
    New-Item -ItemType SymbolicLink -Path $linkedFile -Target (Get-FixturePath 'wydgo748/data/npcs/Existing.json') | Out-Null
    $fileLinkCreated = $true
} catch {
    if (!$IsWindows -or $_.Exception.Message -notmatch 'Administrator privilege required') { throw }
    Write-Warning 'Skipped file-symbolic-link checks: Windows requires administrator privilege or Developer Mode.'
}
if ($fileLinkCreated) {
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree } 'Linked runtime input is not allowed'
    Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @('wydgo748/data/npcs/Linked.json') } 'Linked runtime input is not allowed'
    Rename-Item -LiteralPath $linkedFile -NewName (Split-Path $ignoredLink -Leaf)
}
# All rename and link targets stay inside this disposable fixture.
$languageDirectory = Get-FixturePath 'wydgo748/data/content/language'
$languageSource = Get-FixturePath 'wydgo748/data/content/language-source'
Rename-Item -LiteralPath $languageDirectory -NewName (Split-Path $languageSource -Leaf)
$linkType = if ($IsWindows) { 'Junction' } else { 'SymbolicLink' }
New-Item -ItemType $linkType -Path $languageDirectory -Target $languageSource | Out-Null
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture } 'Linked runtime input is not allowed'
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree } 'Linked runtime input is not allowed'
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @('wydgo748/data/content/language/en.txt') } 'Linked runtime input is not allowed'
# A linked data root must fail even when each file's immediate directory is regular.
$dataDirectory = Get-FixturePath 'wydgo748/data'
$dataSource = Get-FixturePath 'wydgo748/data-source'
Rename-Item -LiteralPath $dataDirectory -NewName (Split-Path $dataSource -Leaf)
New-Item -ItemType $linkType -Path $dataDirectory -Target $dataSource | Out-Null
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture } 'Linked runtime input is not allowed'
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -WorkingTree } 'Linked runtime input is not allowed'
Expect-Rejection { Get-VpsRuntimeInventory -Repository $fixture -AdditionalRuntimeData @('wydgo748/data/content/language/pt.txt') } 'Linked runtime input is not allowed'
Write-Output "VPS runtime inventory checks passed: language loss, tracking, ignores, explicit recovery, deduplication, forbidden files, unsafe paths, directories and linked ancestors. File-symbolic-link checks executed: $fileLinkCreated."
