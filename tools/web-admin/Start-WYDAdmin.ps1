param(
    [switch]$SkipBuild,
    [switch]$NoBrowser
)

# Prepare/open the integrated panel. Do not start, restart, or stop the server.
$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$serverRoot = Join-Path $repoRoot "wydgo748"
$portalRoot = Join-Path $serverRoot "web\portal"
$configFile = Join-Path $serverRoot "data\server.txt"

if (-not $SkipBuild) {
    $npm = Get-Command npm.cmd -ErrorAction Stop
    Push-Location $portalRoot
    try {
        if (-not (Test-Path -LiteralPath "node_modules" -PathType Container)) {
            & $npm.Source ci
            if ($LASTEXITCODE -ne 0) { throw "Failed to install panel dependencies." }
        }
        & $npm.Source run build
        if ($LASTEXITCODE -ne 0) { throw "Failed to build the panel." }
    }
    finally { Pop-Location }
}

$settings = @{}
foreach ($line in Get-Content -LiteralPath $configFile) {
    $value = $line.Trim()
    if ($value.Length -eq 0 -or $value.StartsWith("#") -or $value.StartsWith(";")) { continue }
    $parts = $value -split "=", 2
    if ($parts.Count -eq 2) { $settings[$parts[0].Trim()] = $parts[1].Trim().Trim('"') }
}
if ($settings["web_admin_enabled"] -notin @("true", "1")) {
    throw "Enable web_admin_enabled=true in $configFile and start the updated tm.exe."
}
$address = $settings["web_admin_address"]
if ([string]::IsNullOrWhiteSpace($address)) { $address = "127.0.0.1:8082" }
try {
    $uri = [System.Uri]("http://$address")
    $ip = [System.Net.IPAddress]::Parse($uri.DnsSafeHost)
    if (-not [System.Net.IPAddress]::IsLoopback($ip) -or $uri.Port -lt 1 -or
        $uri.Port -gt 65535 -or $uri.AbsolutePath -ne "/" -or $uri.UserInfo -ne "" -or
        $uri.Query -ne "" -or $uri.Fragment -ne "") { throw "non-local address" }
}
catch { throw "Invalid web_admin_address: use a loopback IP address and port, such as 127.0.0.1:8082." }

$panelUrl = "http://$address/admin/"
try {
    $response = Invoke-WebRequest -Uri "http://$address/healthz" -UseBasicParsing -TimeoutSec 3
    if ($response.StatusCode -ne 200) { throw "Health check failed" }
}
catch {
    Write-Host "Start the updated tm.exe in $serverRoot."
    Write-Host "The panel will be available automatically at $panelUrl"
    Write-Host "If the server is already running, check its console; update and restart an older executable yourself."
    return
}
Write-Host "Panel available at: $panelUrl"
Write-Host "Use the felipetr account, its usual password, and the PIN from data/server.txt."
Write-Host "Server output and logs remain in the tm.exe window."
if (-not $NoBrowser) { Start-Process $panelUrl }
