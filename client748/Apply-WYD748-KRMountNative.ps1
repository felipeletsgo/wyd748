[CmdletBinding()]
param(
    [string]$Executable = (Join-Path $PSScriptRoot 'WYD.exe'),
    [switch]$VerifyOnly,
    [switch]$ResetFromOriginal
)

$ErrorActionPreference='Stop'
$nativeHash='91EAB0CBDAC8E8957A2138B0D6060587D5F701F1178EF5CE40105A028E849209'
function Get-Sha([string]$Path){return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()}

$apply=Join-Path $PSScriptRoot 'Apply-WYD748.ps1'
$nativePatch=Join-Path $PSScriptRoot 'Patch-WYD748-KRMountNative.ps1'
$assets=Join-Path $PSScriptRoot 'Patch-WYD748-KRMountAssets.ps1'
$test=Join-Path $PSScriptRoot 'Test-WYD748-KRMountNative.ps1'
foreach($path in @($apply,$nativePatch,$assets,$test)){if(-not(Test-Path -LiteralPath $path -PathType Leaf)){throw "elo ausente: $path"}}

$current=if(Test-Path -LiteralPath $Executable -PathType Leaf){Get-Sha $Executable}else{''}
if($VerifyOnly){
    if($current-ne$nativeHash){throw "WYD.exe ainda nao esta no estado standalone ($current)"}
    & $nativePatch -Executable $Executable -VerifyOnly
    & $assets -ClientRoot $PSScriptRoot -VerifyOnly
    & $test -Executable $Executable
    return
}

if($current-ne$nativeHash){
    if($ResetFromOriginal){& $apply -Executable $Executable -ResetFromOriginal}
    else{& $apply -Executable $Executable}
    & $nativePatch -Executable $Executable
}
& $assets -ClientRoot $PSScriptRoot
& $test -Executable $Executable
Write-Host "Client 7.48 com montarias KR standalone concluido: $(Get-Sha $Executable)"
