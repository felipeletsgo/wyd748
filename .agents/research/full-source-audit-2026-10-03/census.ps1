param([Parameter(Mandatory)][ValidateSet('tmproject', 'wydgo748')][string]$Root)
$ErrorActionPreference = 'Stop'
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../..'))
$extensions = if ($Root -eq 'tmproject') { @('.cpp', '.h', '.hpp', '.c') } else { @('.go') }
$patterns = [ordered]@{
    raw_memory = '\b(memcpy|memmove|malloc|realloc|free)\s*\('
    text_input = '\b(fscanf|sscanf|sprintf|strcpy|strcat|_itoa)\s*\('
    raw_file = '\b(_open|_read|_filelength|fread|fwrite)\s*\('
    persistence = '\b(Save\w*|Load\w*|BeginTx|Commit|Rollback|ExecContext|QueryContext)\s*\('
    concurrency = '\b(go\s+func|go\s+\w+|Lock\s*\(|Unlock\s*\(|make\s*\(\s*chan)'
    hard_exit = '\b(panic|Fatalf|Fatal|os\.Exit)\s*\('
    pending = '\b(TODO|FIXME|HACK)\b'
}
$files = @(& rg --files (Join-Path $repo $Root))
if ($LASTEXITCODE -ne 0) { throw 'Source enumeration failed.' }
$entries = foreach ($file in ($files | Sort-Object)) {
    if ([IO.Path]::GetExtension($file) -notin $extensions) { continue }
    $relative = [IO.Path]::GetRelativePath($repo, $file).Replace('\', '/')
    $bytes = [IO.File]::ReadAllBytes($file)
    $body = [Text.Encoding]::UTF8.GetString($bytes)
    $group = if ($relative -match '/Dependencies/') { 'third-party Dependencies' }
        elseif ($relative -match '/internal/([^/]+)/') { "internal/$($Matches[1])" }
        elseif ($relative -match '/cmd/([^/]+)/') { "cmd/$($Matches[1])" }
        elseif ($relative -match '/tests/') { 'tests' }
        elseif ($relative -match '/features/([^/]+)/') { "features/$($Matches[1])" }
        elseif ($relative -match '/scenes/([^/]+)/') { "scenes/$($Matches[1])" }
        elseif ($relative -match '/CommonFiles/') { 'CommonFiles' }
        else { 'other' }
    $markers = [ordered]@{}
    foreach ($pattern in $patterns.GetEnumerator()) {
        $count = [regex]::Matches($body, $pattern.Value).Count
        if ($count) { $markers[$pattern.Key] = $count }
    }
    [ordered]@{
        path = $relative
        group = $group
        bytes = $bytes.Length
        lines = [regex]::Matches($body, '\n').Count + [int]($body.Length -gt 0 -and -not $body.EndsWith("`n"))
        test = $relative.EndsWith('_test.go') -or $group -eq 'tests'
        coverage = 'read and pattern-screened; not a per-line manual approval'
        markers = $markers
    }
}
[ordered]@{
    root = $Root
    enumeration = 'rg --files, active non-ignored C/C++ source or Go files; generated build outputs excluded'
    decoding = 'UTF-8 for lexical markers only; no legacy-encoding language-completeness claim'
    marker_policy = 'Signals for investigation, not findings and not AST or behavior equivalence.'
    files = @($entries)
} | ConvertTo-Json -Depth 8 -Compress
