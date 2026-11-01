function Get-VpsRuntimeInventory {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)] [string]$Repository,
        [switch]$WorkingTree,
        [string[]]$AdditionalRuntimeData = @()
    )
    $ErrorActionPreference = 'Stop'
    $required = @(
        'spawns/NPCGener.txt',
        'spawns/instances/RuneTrack0.txt', 'spawns/instances/RuneTrack1.txt',
        'spawns/instances/RuneTrack2.txt', 'spawns/instances/RuneTrack3.txt',
        'spawns/instances/RuneTrack4.txt', 'spawns/instances/RuneTrack5.txt', 'spawns/instances/RuneTrack6.txt',
        'world/teleports.ini', 'world/init_items.csv',
        'catalog/itemlist.csv', 'catalog/Itemname.csv', 'catalog/ItemEffect.h', 'catalog/SkillData.csv',
        'content/quest_zones.json', 'content/quests.json', 'content/droprate.json',
        'content/volatiles.json', 'content/instances.json', 'content/repliction.json',
        'content/mounts.json', 'content/character_templates.json',
        'content/language/en.txt', 'content/language/pt.txt',
        'config/network_admission.json', 'config/client_integrity.json',
        'world/maps/HeightMap.dat', 'world/maps/AttributeMap.dat'
    )
    $requiredPattern = ($required | ForEach-Object { [regex]::Escape($_) }) -join '|'
    $allowed = "^wydgo748/data/(?:npcs/[^/]+\.json|content/boss/[^/]+\.lua|spawns/instances/[^/]+\.txt|(?:$requiredPattern))$"
    $options = @('ls-files', '--cached')
    if ($WorkingTree) { $options += @('--others', '--exclude-standard') }
    $files = @(& git -C $Repository -c core.quotepath=false @options -- wydgo748/data)
    if ($LASTEXITCODE -ne 0) { throw 'Data inventory failed' }
    foreach ($additional in $AdditionalRuntimeData) {
        if ($additional -cnotmatch $allowed -or $additional -match '(^|/)\.\.(/|$)' -or
            $additional.Contains('\') -or !(Test-Path -LiteralPath (Join-Path $Repository $additional) -PathType Leaf)) {
            throw "Invalid additional runtime input: $additional"
        }
    }
    $included = [Collections.Generic.List[string]]::new()
    $removed = [Collections.Generic.List[string]]::new()
    $repositoryRoot = (Get-Item -LiteralPath $Repository).FullName
    $pathComparer = if ($IsWindows) { [StringComparer]::OrdinalIgnoreCase } else { [StringComparer]::Ordinal }
    $checkedComponents = [Collections.Generic.HashSet[string]]::new($pathComparer)
    foreach ($file in @($files + $AdditionalRuntimeData | Sort-Object -Unique)) {
        if ($file -cnotmatch $allowed) { continue }
        $path = Join-Path $Repository $file
        if (!(Test-Path -LiteralPath $path -PathType Leaf)) {
            if (!$WorkingTree) { throw "Tracked runtime input is missing: $file. Commit the reorganization or use -WorkingTree." }
            $removed.Add($file)
            continue
        }
        $entry = Get-Item -LiteralPath $path
        # MODERNIZACAO_COMPATIVEL: reject links in every runtime path component.
        while ($null -ne $entry) {
            if (!$checkedComponents.Add($entry.FullName)) { break }
            if ($entry.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Linked runtime input is not allowed: $file" }
            if ($pathComparer.Equals($entry.FullName, $repositoryRoot)) { break }
            $entry = if ($entry -is [IO.FileInfo]) { $entry.Directory } else { $entry.Parent }
        }
        $included.Add($file)
    }
    foreach ($file in $required) {
        if ($included -cnotcontains "wydgo748/data/$file") { throw "Missing runtime data: $file" }
    }
    [pscustomobject]@{
        Files = $included.ToArray()
        Removed = $removed.ToArray()
    }
}
