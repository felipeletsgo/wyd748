<#
.SYNOPSIS
Confere regras únicas, documentação fora das sources e inventário reproduzível.
#>
[CmdletBinding()]
param([switch]$UpdateMap)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$mapPath = Join-Path $repoRoot 'DOCS/documentation-map.md'
$issues = [Collections.Generic.List[string]]::new()
$documents = @(Get-ChildItem -LiteralPath $repoRoot -File -Filter '*.md')
foreach ($directory in @('DOCS', '.agents')) {
    $documents += @(Get-ChildItem -LiteralPath (Join-Path $repoRoot $directory) -File -Recurse -Filter '*.md')
}
$documents = @($documents | Sort-Object FullName -Culture en-US)
$obsoletePath = '(?i)(?:clientgo748|(?:client-source|source-client|source-server)[/\\]|CLIENTS[/\\]WYD|(?:wyd-go-feature|wyd-dev-knowledge)[/\\])'
foreach ($directory in @('clientgo748', 'client-source', 'source-client', 'source-server', 'CLIENTS')) {
    if (Test-Path -LiteralPath (Join-Path $repoRoot $directory)) {
        $issues.Add("Raiz obsoleta presente: $directory")
    }
}
foreach ($directory in @('tmproject', 'wydgo748', 'references', 'tools', '.github')) {
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repoRoot $directory) -Recurse -File -Filter '*.md') {
        $issues.Add("Documentacao fora do local canonico: $($file.FullName)")
    }
}
foreach ($file in $documents) {
    if ($file.Name -match '^(AGENTS|CLAUDE)(.*)\.md$' -and $file.FullName -ne (Join-Path $repoRoot 'AGENTS.md')) {
        $issues.Add("Regras duplicadas: $($file.FullName)")
    }
}

$lines = [Collections.Generic.List[string]]::new()
$lines.Add('# Inventário da documentação')
$lines.Add('')
$lines.Add('Gerado por `tools/repository/Test-RepositoryLayout.ps1 -UpdateMap`.')
$lines.Add('Caminhos foram normalizados para a arquitetura atual. Fichas e handoffs')
$lines.Add('preservam o estado histórico da validação; não são prova de conclusão atual.')
$lines.Add('Assets e dumps binários não são documentos. O acervo nativo está em `references/client748/`.')
$lines.Add('')
$lines.Add("Documentos Markdown: $($documents.Count).")
$lines.Add('')
$lines.Add('| Documento | Tipo |')
$lines.Add('| --- | --- |')
foreach ($file in $documents) {
    $relative = [IO.Path]::GetRelativePath($repoRoot, $file.FullName).Replace('\', '/')
    $target = [IO.Path]::GetRelativePath((Split-Path $mapPath), $file.FullName).Replace('\', '/')
    $kind = if ($relative -like '.agents/skills/*/SKILL.md') { 'Skill ativa' }
        elseif ($relative -like '.agents/skills/*') { 'Referencia operacional' }
        elseif ($relative -like '.agents/research/*') { 'Evidencia de pesquisa' }
        elseif ($relative -like '.agents/handoffs/*') { 'Continuidade historica' }
        elseif ($relative -eq 'AGENTS.md') { 'Regras unicas' }
        else { 'Documentacao do produto' }
    $lines.Add("| [$relative](<$target>) | $kind |")
}
$lines.Add('')
$lines.Add('## Catálogos estruturados de pesquisa')
$lines.Add('')
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repoRoot '.agents/research') -Recurse -File -Filter '*.tsv' | Sort-Object FullName) {
    $relative = [IO.Path]::GetRelativePath($repoRoot, $file.FullName).Replace('\', '/')
    $target = [IO.Path]::GetRelativePath((Split-Path $mapPath), $file.FullName).Replace('\', '/')
    $lines.Add("- [$relative](<$target>)")
}
$expected = ($lines -join "`n") + "`n"
if ($UpdateMap) {
    [IO.File]::WriteAllText($mapPath, $expected, [Text.UTF8Encoding]::new($false))
} elseif (-not (Test-Path -LiteralPath $mapPath) -or [IO.File]::ReadAllText($mapPath).Replace("`r`n", "`n") -cne $expected) {
    $issues.Add('Inventario desatualizado; execute com -UpdateMap.')
}

# Links inline locais: URLs, anchors e exemplos dentro de fences não são paths.
foreach ($file in $documents) {
    $body = [IO.File]::ReadAllText($file.FullName)
    if ($body -match $obsoletePath) {
        $issues.Add("Referencia a projeto/caminho obsoleto: $($file.FullName)")
    }
    $body = [regex]::Replace($body, '(?ms)^```.*?^```[^\r\n]*', '')
    foreach ($match in [regex]::Matches($body, '\[[^\]\r\n]+\]\((?<target><[^>]+>|[^\s)]+)\)')) {
        $target = $match.Groups['target'].Value.Trim('<', '>')
        if ($target -match '^(?:[a-zA-Z][a-zA-Z0-9+.-]*:|#|/)') { continue }
        $target = [Uri]::UnescapeDataString(($target -split '#', 2)[0])
        if (-not $target) { continue }
        $resolved = [IO.Path]::GetFullPath((Join-Path $file.DirectoryName $target))
        if (-not (Test-Path -LiteralPath $resolved)) {
            $issues.Add("Link ausente em $([IO.Path]::GetRelativePath($repoRoot, $file.FullName)): $target")
        }
    }
}
if ($issues.Count) { throw ($issues -join "`n") }
Write-Output "Layout e links locais OK; $($documents.Count) documentos indexados."
