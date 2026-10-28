[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$version = '1.0.3856.49'
$expected = 'BC0F76EB911B569838DC4AA8F8D325269B966BEDB592863D26211AEF3A099F1A'
$repository = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$destination = Join-Path $repository "tmproject\build\tools\webview2\$version"
$header = Join-Path $destination 'build\native\include\WebView2.h'
$library = Join-Path $destination 'build\native\x86\WebView2LoaderStatic.lib'
if ((Test-Path -LiteralPath $header) -and (Test-Path -LiteralPath $library)) { return }
New-Item -ItemType Directory -Force -Path $destination | Out-Null
$archive = Join-Path $destination 'sdk.zip'
try {
    Invoke-WebRequest "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/$version/microsoft.web.webview2.$version.nupkg" -OutFile $archive
    if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $expected) { throw 'WebView2 SDK checksum mismatch.' }
    Expand-Archive -LiteralPath $archive -DestinationPath $destination -Force
    if (-not (Test-Path -LiteralPath $header) -or -not (Test-Path -LiteralPath $library)) { throw 'WebView2 SDK extraction is incomplete.' }
} finally {
    if (Test-Path -LiteralPath $archive) { Remove-Item -LiteralPath $archive }
}
