<#
.SYNOPSIS
Checks the single rules file and its Claude Code import, Claude Code skill
pointers, documentation placement, and reproducible inventory.
#>
[CmdletBinding()]
param([switch]$UpdateMap)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$mapPath = Join-Path $repoRoot 'DOCS/documentation-map.md'
$issues = [Collections.Generic.List[string]]::new()
$documents = @(Get-ChildItem -LiteralPath $repoRoot -File -Filter '*.md')
foreach ($directory in @('DOCS', '.agents', '.claude')) {
    $documents += @(Get-ChildItem -LiteralPath (Join-Path $repoRoot $directory) -File -Recurse -Filter '*.md')
}
$documents = @($documents | Sort-Object FullName -Culture en-US)
$obsoletePath = '(?i)(?:clientgo748|(?:client-source|source-client|source-server)[/\\]|CLIENTS[/\\]WYD|(?:wyd-go-feature|wyd-dev-knowledge)[/\\])'
foreach ($directory in @('clientgo748', 'client-source', 'source-client', 'source-server', 'CLIENTS')) {
    if (Test-Path -LiteralPath (Join-Path $repoRoot $directory)) {
        $issues.Add("Obsolete root present: $directory")
    }
}
foreach ($directory in @('tmproject', 'wydgo748', 'tools', '.github')) {
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repoRoot $directory) -Recurse -File -Filter '*.md') {
        # Installed dependencies and build outputs are generated, not authored source.
        $relativeSource = [IO.Path]::GetRelativePath($repoRoot, $file.FullName).Replace('\', '/')
        if ($relativeSource -match '^(?:tmproject/(?:build|bin|obj)/|wydgo748/web/portal/(?:node_modules|dist|\.astro)/)') { continue }
        # Site news posts are product content rendered by the portal, not documentation.
        if ($relativeSource -match '^wydgo748/web/portal/src/content/news/[a-z0-9-]+\.md$') { continue }
        $issues.Add("Documentation outside its canonical location: $($file.FullName)")
    }
}
# The root CLAUDE.md is a Claude Code import of AGENTS.md; it may hold nothing else.
$claudeShimPath = Join-Path $repoRoot 'CLAUDE.md'
$claudeShim = "<!-- Claude Code entry point: imports AGENTS.md, the single rules file. Add no rules here. -->`n@AGENTS.md"
foreach ($file in $documents) {
    if ($file.Name -notmatch '^(AGENTS|CLAUDE)(.*)\.md$' -or $file.FullName -eq (Join-Path $repoRoot 'AGENTS.md')) { continue }
    if ($file.FullName -eq $claudeShimPath) {
        if ([IO.File]::ReadAllText($file.FullName).Replace("`r`n", "`n").Trim() -cne $claudeShim) {
            $issues.Add("CLAUDE.md must contain only the AGENTS.md import shim: $($file.FullName)")
        }
        continue
    }
    $issues.Add("Duplicate rules file: $($file.FullName)")
}

# Each canonical skill needs Codex metadata and a Claude Code pointer with
# identical name and description.
function Get-SkillFrontmatter([string]$path) {
    $fields = @{}
    $text = [IO.File]::ReadAllText($path).Replace("`r`n", "`n")
    $match = [regex]::Match($text, '\A---\n(?<body>.*?)\n---\n', 'Singleline')
    if (-not $match.Success) { return $fields }
    foreach ($line in $match.Groups['body'].Value -split "`n") {
        if ($line -match '^(?<key>[a-z-]+):\s*(?<value>.*)$') { $fields[$Matches['key']] = $Matches['value'].Trim() }
    }
    return $fields
}
$canonicalSkills = @(Get-ChildItem -LiteralPath (Join-Path $repoRoot '.agents/skills') -Directory | Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'SKILL.md') })
$pointerRoot = Join-Path $repoRoot '.claude/skills'
foreach ($skill in $canonicalSkills) {
    $canonical = Get-SkillFrontmatter (Join-Path $skill.FullName 'SKILL.md')
    $metadataPath = Join-Path $skill.FullName 'agents/openai.yaml'
    if (-not (Test-Path -LiteralPath $metadataPath)) {
        $issues.Add("Missing Codex skill metadata: .agents/skills/$($skill.Name)/agents/openai.yaml")
    } else {
        $metadata = [IO.File]::ReadAllText($metadataPath)
        $metadataFields = @{
            interface = '(?m)^\s*interface\s*:\s*$'
            display_name = '(?m)^\s+display_name\s*:\s*.+$'
            short_description = '(?m)^\s+short_description\s*:\s*.+$'
            default_prompt = '(?m)^\s+default_prompt\s*:\s*.+$'
        }
        foreach ($field in $metadataFields.Keys) {
            if ($metadata -notmatch $metadataFields[$field]) {
                $issues.Add("Codex skill metadata lacks ${field}: .agents/skills/$($skill.Name)/agents/openai.yaml")
            }
        }
    }
    $pointerPath = Join-Path $pointerRoot "$($skill.Name)/SKILL.md"
    if (-not $canonical['name'] -or -not $canonical['description']) {
        $issues.Add("Skill without name/description frontmatter: .agents/skills/$($skill.Name)/SKILL.md")
    }
    if (-not (Test-Path -LiteralPath $pointerPath)) {
        $issues.Add("Missing Claude Code skill pointer: .claude/skills/$($skill.Name)/SKILL.md")
        continue
    }
    $pointer = Get-SkillFrontmatter $pointerPath
    foreach ($key in @('name', 'description')) {
        if ($pointer[$key] -cne $canonical[$key]) {
            $issues.Add("Claude Code skill pointer $key differs from canonical: .claude/skills/$($skill.Name)/SKILL.md")
        }
    }
    if (-not [IO.File]::ReadAllText($pointerPath).Contains("(../../../.agents/skills/$($skill.Name)/SKILL.md)")) {
        $issues.Add("Claude Code skill pointer does not link its canonical skill: .claude/skills/$($skill.Name)/SKILL.md")
    }
}
if (Test-Path -LiteralPath $pointerRoot) {
    foreach ($pointer in Get-ChildItem -LiteralPath $pointerRoot -Directory) {
        if (-not (Test-Path -LiteralPath (Join-Path $repoRoot ".agents/skills/$($pointer.Name)/SKILL.md"))) {
            $issues.Add("Claude Code skill pointer without a canonical skill: .claude/skills/$($pointer.Name)")
        }
    }
}

$lines = [Collections.Generic.List[string]]::new()
$lines.Add('# Documentation inventory')
$lines.Add('')
$lines.Add('Generated by `tools/repository/Test-RepositoryLayout.ps1 -UpdateMap`.')
$lines.Add('Paths are normalized for the current architecture. Evidence records and handoffs')
$lines.Add('preserve historical validation state; they do not prove current completion.')
$lines.Add('Assets and binary dumps are not documents. Optional native evidence in `references/` is local-only and ignored by Git.')
$lines.Add('')
$lines.Add("Markdown documents: $($documents.Count).")
$lines.Add('')
$lines.Add('| Document | Type |')
$lines.Add('| --- | --- |')
foreach ($file in $documents) {
    $relative = [IO.Path]::GetRelativePath($repoRoot, $file.FullName).Replace('\', '/')
    $target = [IO.Path]::GetRelativePath((Split-Path $mapPath), $file.FullName).Replace('\', '/')
    $kind = if ($relative -like '.agents/skills/*/SKILL.md') { 'Active skill' }
        elseif ($relative -like '.agents/skills/*') { 'Operational reference' }
        elseif ($relative -like '.agents/research/*') { 'Research evidence' }
        elseif ($relative -like '.agents/handoffs/*') { 'Historical continuity' }
        elseif ($relative -like '.claude/skills/*/SKILL.md') { 'Claude Code skill pointer' }
        elseif ($relative -eq 'AGENTS.md') { 'Single rules file' }
        elseif ($relative -eq 'CLAUDE.md') { 'Claude Code import of AGENTS.md' }
        else { 'Product documentation' }
    $lines.Add("| [$relative](<$target>) | $kind |")
}
$lines.Add('')
$lines.Add('## Structured research catalogs')
$lines.Add('')
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repoRoot '.agents/research') -Recurse -File -Filter '*.tsv' | Sort-Object FullName) {
    $relative = [IO.Path]::GetRelativePath($repoRoot, $file.FullName).Replace('\', '/')
    $target = [IO.Path]::GetRelativePath((Split-Path $mapPath), $file.FullName).Replace('\', '/')
    $lines.Add("- [$relative](<$target>)")
}
$lines.Add('')
$lines.Add('## Local native server research')
$lines.Add('')
$lines.Add('The REA archive is local-only and ignored by Git. Read its index first for native server functions or logic.')
$lines.Add('')
$lines.Add('- [REA server research index](<../references/decompilacao server/dlls/rea-analysis/README.md>)')
$expected = ($lines -join "`n") + "`n"
if ($UpdateMap) {
    [IO.File]::WriteAllText($mapPath, $expected, [Text.UTF8Encoding]::new($false))
} elseif (-not (Test-Path -LiteralPath $mapPath) -or [IO.File]::ReadAllText($mapPath).Replace("`r`n", "`n") -cne $expected) {
    $issues.Add('Inventory is outdated; run with -UpdateMap.')
}

# Local inline links: URLs, anchors, and fenced examples are not paths.
foreach ($file in $documents) {
    $body = [IO.File]::ReadAllText($file.FullName)
    if ($body -match $obsoletePath) {
        $issues.Add("Reference to an obsolete project/path: $($file.FullName)")
    }
    $body = [regex]::Replace($body, '(?ms)^```.*?^```[^\r\n]*', '')
    foreach ($match in [regex]::Matches($body, '\[[^\]\r\n]+\]\((?<target><[^>]+>|[^\s)]+)\)')) {
        $target = $match.Groups['target'].Value.Trim('<', '>')
        if ($target -match '^(?:[a-zA-Z][a-zA-Z0-9+.-]*:|#|/)') { continue }
        $target = [Uri]::UnescapeDataString(($target -split '#', 2)[0])
        if (-not $target) { continue }
        $resolved = [IO.Path]::GetFullPath((Join-Path $file.DirectoryName $target))
        if (-not (Test-Path -LiteralPath $resolved)) {
            # Native evidence is optional in checkouts because references/ is local-only.
            $referencePrefix = (Join-Path $repoRoot 'references') + [IO.Path]::DirectorySeparatorChar
            if ($resolved.StartsWith($referencePrefix, [StringComparison]::OrdinalIgnoreCase)) { continue }
            $issues.Add("Missing link in $([IO.Path]::GetRelativePath($repoRoot, $file.FullName)): $target")
        }
    }
}

# The local Codex limit must cover the complete root rules document.
$projectDocLimit = 12288
$codexConfigPath = Join-Path $repoRoot '.codex/config.toml'
if (Test-Path -LiteralPath $codexConfigPath) {
    $codexConfig = [IO.File]::ReadAllText($codexConfigPath)
    $limitMatch = [regex]::Match($codexConfig, '(?m)^\s*project_doc_max_bytes\s*=\s*(\d+)\s*$')
    if ($limitMatch.Success) {
        $projectDocLimit = [int]::Parse($limitMatch.Groups[1].Value)
    } else {
        $issues.Add('Codex config does not define project_doc_max_bytes.')
    }
}
$agentsPath = Join-Path $repoRoot 'AGENTS.md'
if ((Get-Item -LiteralPath $agentsPath).Length -gt $projectDocLimit) {
    $issues.Add("AGENTS.md exceeds project_doc_max_bytes ($projectDocLimit bytes).")
}

# Current operational examples must follow the active server output and patch range.
foreach ($relativeBuildDocument in @('DOCS/build-and-integration.md', 'DOCS/server/operations.md')) {
    $buildDocumentPath = Join-Path $repoRoot $relativeBuildDocument
    $buildDocument = [IO.File]::ReadAllText($buildDocumentPath)
    if ($buildDocument -match '(?im)(?:wydgo748[/\\]bin[/\\]|go\s+build\s+-o\s+bin[/\\])') {
        $issues.Add("Build documentation contains a forbidden bin output path: $relativeBuildDocument")
    }
    if ($buildDocument -notmatch '(?im)build[/\\]server[/\\]tm\.exe') {
        $issues.Add("Build documentation does not define the root build/server runtime path: $relativeBuildDocument")
    }
}

foreach ($relativeCurrentDocument in @('DOCS/WYD-WEB-PLATAFORM.md')) {
    $currentDocumentPath = Join-Path $repoRoot $relativeCurrentDocument
    $currentDocument = [IO.File]::ReadAllText($currentDocumentPath)
    if ($currentDocument -match '(?im)(?:wydgo748[/\\]bin[/\\]|go\s+build\s+-o\s+bin[/\\])') {
        $issues.Add("Current documentation contains a forbidden bin output path: $relativeCurrentDocument")
    }
}

$launcherDocumentPath = Join-Path $repoRoot 'DOCS/client/launcher.md'
$launcherDocument = [IO.File]::ReadAllText($launcherDocumentPath)
$publishingSection = [regex]::Match($launcherDocument, '(?ms)^## Publishing a patch\r?\n(?<body>.*?)(?=^## |\z)')
if (-not $publishingSection.Success) {
    $issues.Add('Launcher documentation is missing the current Publishing a patch section.')
} else {
    $publishingBody = $publishingSection.Groups['body'].Value
    if ($publishingBody -match '(?im)-Version\s+100\d\d') {
        $issues.Add('Current launcher examples still use a retired 100xx patch version.')
    }
    if ($publishingBody -notmatch '(?im)48002\s*(?:through|\.\.)\s*49999') {
        $issues.Add('Current launcher documentation does not state the 48002..49999 patch range.')
    }
}

# Server data follows DOCS/server/data-layout.md: no loose files at the root.
$dataRoot = Join-Path $repoRoot 'wydgo748/data'
$dataAreas = @('catalog', 'config', 'content', 'npcs', 'spawns', 'world')
foreach ($entry in @(Get-ChildItem -LiteralPath $dataRoot -Force)) {
    if (!$entry.PSIsContainer -or $entry.Name -notin $dataAreas) {
        $issues.Add("Unexpected entry in wydgo748/data (see DOCS/server/data-layout.md): $($entry.Name)")
    }
}

# Generated output may live only in the sanctioned ignored locations below
# (AGENTS.md "Cleanup"). Anything else ignored by Git is leftover build,
# test or agent output and must be removed or moved under build/.
# Client crash evidence, the local client ZIP, and the legacy runtime log have
# explicit names and retention rules in DOCS/agent-environments.md.
$allowedIgnored = '^(?:references/|build/|tmproject/build/|\.codex/|\.venv/|\.claude/settings\.local\.json$' +
    '|tmproject/client748/(?:WYD\.exe|WYDLauncher\.exe|WYDLauncherLanguage\.dll$|update\.dat|guilds\.txt|lang\.txt$|SkillData\.csv|screenshot/|dumps/(?:crash-monitor-p\d+\.log|client-crash-[^/]+\.dmp)$|[^/]+\.log$|client-crash-[^/]+\.dmp$)' +
    '|tmproject/client748/dumps/$|tmproject/client748\.zip$|tmproject/WYD 7\.48 PORTUGUES/WYD\.log$' +
    '|wydgo748/data/catalog/SkillData\.bin$|wydgo748/runtime/' +
    '|wydgo748/web/portal/(?:node_modules/|dist/|\.astro/|src/data/catalog\.generated\.json$|\.env))'
$ignored = @(& git -C $repoRoot ls-files --others --ignored --exclude-standard --directory)
if ($LASTEXITCODE -ne 0) { $issues.Add('Cannot list ignored files with git.') }
foreach ($path in $ignored) {
    if ($path -notmatch $allowedIgnored) { $issues.Add("Generated artifact outside build/: $path") }
}

if ($issues.Count) { throw ($issues -join "`n") }
Write-Output "Layout and local links OK; $($documents.Count) documents indexed."
