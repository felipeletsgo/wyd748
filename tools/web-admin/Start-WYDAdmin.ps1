param(
    [switch]$SkipBuild,
    [switch]$NoBrowser
)

# Prepara/abre o painel integrado. Nao inicia, reinicia ou encerra o servidor.
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
            if ($LASTEXITCODE -ne 0) { throw "Falha ao instalar dependencias do painel." }
        }
        & $npm.Source run build
        if ($LASTEXITCODE -ne 0) { throw "Falha ao compilar o painel." }
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
    throw "Habilite web_admin_enabled=true em $configFile e abra o tm.exe atualizado."
}
$address = $settings["web_admin_address"]
if ([string]::IsNullOrWhiteSpace($address)) { $address = "127.0.0.1:8082" }
try {
    $uri = [System.Uri]("http://$address")
    $ip = [System.Net.IPAddress]::Parse($uri.DnsSafeHost)
    if (-not [System.Net.IPAddress]::IsLoopback($ip) -or $uri.Port -lt 1 -or
        $uri.Port -gt 65535 -or $uri.AbsolutePath -ne "/" -or $uri.UserInfo -ne "" -or
        $uri.Query -ne "" -or $uri.Fragment -ne "") { throw "endereco nao local" }
}
catch { throw "web_admin_address invalido: use IP literal de loopback e porta, como 127.0.0.1:8082." }

$panelUrl = "http://$address/admin/"
try {
    $response = Invoke-WebRequest -Uri "http://$address/healthz" -UseBasicParsing -TimeoutSec 3
    if ($response.StatusCode -ne 200) { throw "healthcheck falhou" }
}
catch {
    Write-Host "Abra o tm.exe atualizado na pasta $serverRoot."
    Write-Host "O painel sera iniciado automaticamente em $panelUrl"
    Write-Host "Se o servidor ja estiver aberto, consulte seu console; um executavel antigo precisa ser atualizado e reiniciado por voce."
    return
}
Write-Host "Painel disponivel: $panelUrl"
Write-Host "Use a conta felipetr, a senha normal da conta e o PIN de data/server.txt."
Write-Host "Servidor e logs permanecem na janela do tm.exe."
if (-not $NoBrowser) { Start-Process $panelUrl }
