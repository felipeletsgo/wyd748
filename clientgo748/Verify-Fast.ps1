[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$clientRoot = $PSScriptRoot

function Invoke-Checked {
    param(
        [string]$Program,
        [string[]]$Arguments
    )

    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $Program $($Arguments -join ' ')"
    }
}

$buildOutput = Join-Path ([IO.Path]::GetTempPath()) (
    "wydclient-fast-{0}{1}" -f [Guid]::NewGuid().ToString("N"),
    $(if ($IsWindows) { ".exe" } else { "" })
)

Push-Location $clientRoot
try {
    Invoke-Checked "go" @("test", "-count=1", "./...")
    Invoke-Checked "go" @("vet", "./...")
    Invoke-Checked "go" @("build", "-trimpath", "-o", $buildOutput, "./cmd/wydclient")

    $git = Get-Command git -ErrorAction SilentlyContinue
    if ($null -ne $git) {
        & $git.Source -C $clientRoot rev-parse --is-inside-work-tree *> $null
        if ($LASTEXITCODE -eq 0) {
            Invoke-Checked $git.Source @("-C", $clientRoot, "diff", "--check", "--", ".")
        }
        else {
            Write-Host "Git worktree not found; skipping whitespace check."
        }
    }
    else {
        Write-Host "Git not found; skipping whitespace check."
    }
}
finally {
    Pop-Location
    if (Test-Path -LiteralPath $buildOutput) {
        Remove-Item -LiteralPath $buildOutput -Force
    }
}

Write-Host "Fast verification passed: Go tests, vet, build, and available worktree checks."
