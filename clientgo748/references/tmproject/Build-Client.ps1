<#
.SYNOPSIS
Compila o client x86 e instala o candidato com identidade SHA-256 conferida.
.DESCRIPTION
Implementacao local de infraestrutura. Nao inicia nem encerra processos.
Falha de compilacao ou copia impede que o candidato seja declarado validado.
.PARAMETER Configuration
Seleciona Debug ou Release; ambos usam o mapeamento x86 da solucao.
.PARAMETER Rebuild
Recompila todos os objetos em vez de usar o build incremental.
.PARAMETER MSBuildPath
Caminho opcional para MSBuild.exe; por padrao usa a instalacao do Visual Studio.
#>
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [switch]$Rebuild,
    [string]$MSBuildPath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# Todos os caminhos partem do script, independentemente do diretorio do shell.
$repositoryRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$solution = Join-Path $PSScriptRoot 'WYDESTINY.sln'
$candidate = Join-Path $repositoryRoot 'client748\project.exe'
$targetName = if ($Configuration -eq 'Release') { 'WYD.exe' } else { 'WYDestiny.exe' }
$artifact = Join-Path $PSScriptRoot "build\TMProject748\$Configuration\$targetName"

if (-not $MSBuildPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $matches = @(& $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe')
        if ($LASTEXITCODE -ne 0) { throw 'Falha na descoberta do MSBuild.' }
        if ($matches.Count -gt 0) { $MSBuildPath = $matches[0] }
    }
    if (-not $MSBuildPath) {
        $command = Get-Command MSBuild.exe -ErrorAction SilentlyContinue
        if ($command) { $MSBuildPath = $command.Source }
    }
}
if (-not $MSBuildPath -or -not (Test-Path -LiteralPath $MSBuildPath -PathType Leaf)) {
    throw 'MSBuild nao encontrado. Informe -MSBuildPath com a instalacao existente.'
}
if (-not (Test-Path -LiteralPath (Split-Path $candidate -Parent) -PathType Container)) {
    throw 'Diretorio client748 ausente; instalacao do candidato cancelada.'
}

$buildTarget = if ($Rebuild) { 'Rebuild' } else { 'Build' }
# O gate puro roda antes do client e da copia; falha de teste preserva o candidato.
$testProject = Join-Path $PSScriptRoot 'TMProject748\tests\ArchitectureTests.vcxproj'
& $MSBuildPath $testProject "/t:$buildTarget" "/p:Configuration=$Configuration" '/p:Platform=Win32' /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar os testes de arquitetura; candidato nao atualizado.' }
$testExecutable = Join-Path $PSScriptRoot "build\tests\$Configuration\ArchitectureTests.exe"
if (-not (Test-Path -LiteralPath $testExecutable -PathType Leaf)) { throw 'Executavel de testes ausente.' }
& $testExecutable
if ($LASTEXITCODE -ne 0) { throw 'Testes de arquitetura falharam; candidato nao atualizado.' }

& $MSBuildPath $solution "/t:$buildTarget" "/p:Configuration=$Configuration" '/p:Platform=x86' /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "MSBuild falhou com codigo $LASTEXITCODE; candidato nao atualizado." }
if (-not (Test-Path -LiteralPath $artifact -PathType Leaf)) { throw "Artefato ausente: $artifact" }

# Copia sincrona: arquivo em uso causa falha, nunca encerramento forcado do jogo.
$sourceHash = (Get-FileHash -LiteralPath $artifact -Algorithm SHA256).Hash
Copy-Item -LiteralPath $artifact -Destination $candidate -Force
$candidateHash = (Get-FileHash -LiteralPath $candidate -Algorithm SHA256).Hash
if ($sourceHash -ne $candidateHash) { throw 'Hash do candidato diverge do artefato compilado.' }
[pscustomobject]@{
    Configuration = $Configuration
    Artifact = $artifact
    Candidate = $candidate
    SHA256 = $candidateHash
    Validation = 'BUILD_AND_DEPLOY_VERIFIED; teste in-game pendente'
}
