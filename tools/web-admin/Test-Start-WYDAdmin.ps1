# Exercise the real shortcut with local HTTP mocked; never start a browser or server.
$ErrorActionPreference = 'Stop'
function Invoke-WebRequest { return @{ StatusCode = 200 } }
function Start-Process { throw 'The shortcut must not open an application during this test.' }
$output = & "$PSScriptRoot/Start-WYDAdmin.ps1" -SkipBuild -NoBrowser 6>&1 | Out-String
if ($output -notmatch 'Panel available at: http://127\.0\.0\.1:8082/admin/') {
    throw "The shortcut did not load the canonical local profile: $output"
}
Write-Output 'Panel shortcut loaded the canonical profile; HTTP was mocked.'
