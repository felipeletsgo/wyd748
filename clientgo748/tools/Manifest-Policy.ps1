$script:ManifestTextExtensions = [Collections.Generic.HashSet[string]]::new(
    [StringComparer]::OrdinalIgnoreCase
)
foreach ($extension in @(
    ".bak", ".c", ".cpp", ".csv", ".filters", ".go", ".h", ".hex",
    ".idl", ".inl", ".java", ".json", ".md", ".mod", ".odl", ".prp",
    ".ps1", ".py", ".sha256", ".sln", ".tsv", ".txt", ".vcxproj",
    ".yaml", ".yml"
)) {
    [void]$script:ManifestTextExtensions.Add($extension)
}

function Test-ManifestExcludedPath([string]$relative) {
    $normalized = $relative.Replace('\', '/')
    return $normalized -eq "MANIFEST.sha256" -or
        $normalized -match '^bin/' -or
        $normalized -eq 'CLIENT OFICIAL 7.48/wydclient.exe' -or
        $normalized -match '(^|/)__pycache__(/|$)' -or
        $normalized -match '\.py[cod]$' -or
        $normalized -match '\.log$' -or
        $normalized -match '(^|/)[^/]+\.tmp(?:-[^/]*)?$' -or
        $normalized -match '~$'
}

function Test-ManifestTextPath([string]$relative) {
    $normalized = $relative.Replace('\', '/')
    $extension = [IO.Path]::GetExtension($normalized)
    return $script:ManifestTextExtensions.Contains($extension) -or
        $normalized -match '(^|/)itemhelp\.dat$' -or
        $normalized -match '/[^/]+\.rep/(idata|user|versioned)/~index\.dat$'
}

function Get-ManifestDigest([string]$path, [string]$relative) {
    $bytes = [IO.File]::ReadAllBytes($path)
    if (Test-ManifestTextPath $relative) {
        # Git may materialize text as CRLF on Windows and LF on Linux. Hash the
        # canonical LF form while leaving every binary asset byte-exact.
        $byteEncoding = [Text.Encoding]::GetEncoding(28591)
        $text = $byteEncoding.GetString($bytes)
        if ($text.Contains("`r`n")) {
            $bytes = $byteEncoding.GetBytes($text.Replace("`r`n", "`n"))
        }
    }

    $digest = [Security.Cryptography.SHA256]::HashData($bytes)
    return [Convert]::ToHexString($digest).ToLowerInvariant()
}
