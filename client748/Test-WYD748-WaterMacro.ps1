param(
    [string]$SourceExecutable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Static/reproducible check for the fifth link. It applies Lindy and Water to a
# disposable copy only; the tracked client executable is never modified.
$work = Join-Path ([IO.Path]::GetTempPath()) ('wyd-water-static-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $work | Out-Null
$exe = Join-Path $work 'WYD.exe'
try {
    Copy-Item -LiteralPath $SourceExecutable -Destination $exe
    & (Join-Path $PSScriptRoot 'Patch-WYD748-Lindy.ps1') -Executable $exe
    & (Join-Path $PSScriptRoot 'Patch-WYD748-WaterMacro.ps1') -Executable $exe

    $bytes = [IO.File]::ReadAllBytes($exe)
    $caveOffset = 0x1D3243
    $caveVA = [uint32]0x013C0243
    $caveLength = 0x5BD
    foreach ($pair in @(@{Offset=0x678E4; Name='chat'},
                        @{Offset=0x779A7; Name='frame'})) {
        if ($bytes[$pair.Offset] -ne 0xE8) { throw "hook $($pair.Name) nao e CALL." }
        $rel = @($bytes[($pair.Offset + 1)..($pair.Offset + 4)])
        if (@($rel | Where-Object { $_ -ne 0 }).Count -eq 0) {
            throw "hook $($pair.Name) nao aponta para a cave."
        }
        $hookVA = if ($pair.Name -eq 'chat') { [uint32]0x004678E4 } else { [uint32]0x004779A7 }
        $hookTarget = [int64]$hookVA + 5 + [BitConverter]::ToInt32($bytes, $pair.Offset + 1)
        if ($hookTarget -lt $caveVA -or $hookTarget -ge ($caveVA + $caveLength)) {
            throw "hook $($pair.Name) aponta fora da cave Water: 0x$('{0:X8}' -f $hookTarget)"
        }
    }
    $nonZero = 0
    for ($i = $caveOffset; $i -lt $caveOffset + $caveLength; $i++) {
        if ($bytes[$i] -ne 0) { $nonZero++ }
    }
    if ($nonZero -eq 0) { throw 'a cave Water permaneceu vazia.' }

    # Validate the unsigned rectangle semantics, not only the presence of a
    # hook.  The emitted sequence is cmp min,current / JB and
    # cmp max,current / JA for both axes.
    function Find-AreaBranch([byte[]]$prefix, [byte]$condition) {
        $conditionOffset = $prefix.Count + 4
        for ($at = $caveOffset; $at -le $caveOffset + $caveLength - $conditionOffset - 2; $at++) {
            $ok = $true
            for ($j = 0; $j -lt $prefix.Count; $j++) {
                if ($bytes[$at + $j] -ne $prefix[$j]) { $ok = $false; break }
            }
            if ($ok -and $bytes[$at + $conditionOffset] -eq 0x0F -and
                $bytes[$at + $conditionOffset + 1] -eq (0x80 + $condition)) { return $true }
        }
        return $false
    }
    if (-not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x1A,0x3B,0x1D)) 2) -or
        -not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x5A,0x04,0x3B,0x1D)) 7) -or
        -not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x5A,0x02,0x3B,0x1D)) 2) -or
        -not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x5A,0x06,0x3B,0x1D)) 7)) {
        throw 'as comparacoes de area nao implementam min/current=JB e max/current=JA.'
    }

    # The local chat branches must land on the parser's handled-return path,
    # never return to 0x4678E9 (which would transmit the command as chat).
    $handledReturnJumps = 0
    $nativeUseCalls = 0
    for ($at = $caveOffset; $at -lt $caveOffset + $caveLength - 5; $at++) {
        if ($bytes[$at] -eq 0xE9) {
            $rel = [BitConverter]::ToInt32($bytes, $at + 1)
            $target = [int64]$caveVA + ($at - $caveOffset) + 5 + $rel
            if ($target -eq 0x00470D31) { $handledReturnJumps++ }
        } elseif ($bytes[$at] -eq 0xE8) {
            $rel = [BitConverter]::ToInt32($bytes, $at + 1)
            $target = [int64]$caveVA + ($at - $caveOffset) + 5 + $rel
            if ($target -eq 0x00465F85) { $nativeUseCalls++ }
        }
    }
    if ($handledReturnJumps -lt 2) {
        throw "os dois comandos locais nao apontam para o retorno tratado (encontrados=$handledReturnJumps)."
    }
    if ($nativeUseCalls -lt 1) { throw 'a cave nao chama a rotina nativa de UseItem.' }
    $ascii = [Text.Encoding]::ASCII.GetString($bytes, $caveOffset, $caveLength)
    if ($ascii.IndexOf('/macropergaon' + [char]0, [StringComparison]::Ordinal) -lt 0 -or
        $ascii.IndexOf('/macropergaoff' + [char]0, [StringComparison]::Ordinal) -lt 0) {
        throw 'comandos locais nao foram materializados na cave.'
    }
    $sha = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToUpperInvariant()
    Write-Host "Water macro static check OK; hooks=cave; handled-jumps=$handledReturnJumps; native-use=$nativeUseCalls; SHA-256=$sha"
} finally {
    [IO.Directory]::Delete($work, $true)
}
