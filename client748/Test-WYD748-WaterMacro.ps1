param(
    [string]$SourceExecutable = (Join-Path $PSScriptRoot 'WYD.exe')
)

$ErrorActionPreference = 'Stop'

# Static/reproducible check for the sixth Water link. It applies only the links that
# are still missing from the supplied source copy; the tracked executable is
# never modified. This accepts the pre-Lindy, pre-Water, or final hashes so
# running the test against the default WYD.exe is safe and idempotent.
$preLindyHash = '4E916C1FD94D60D5EF7F8914B621BAB3787E7BF5460FB251C59F71BCC4D9BA2F'
$preWaterHash = '9762B1AC6EFB4AB3C800877DE1DA048DD43EA407FCEEA945C755DF6986607F18'
$postWaterHash = 'CC57B04F8A07480E4989B33FFDC5E65BFD74789A7FA9104394D7F13A5D9F2D44'
$work = Join-Path ([IO.Path]::GetTempPath()) ('wyd-water-static-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $work | Out-Null
$exe = Join-Path $work 'WYD.exe'
try {
    Copy-Item -LiteralPath $SourceExecutable -Destination $exe
    $sourceHash = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($sourceHash -eq $preLindyHash) {
        & (Join-Path $PSScriptRoot 'Patch-WYD748-Lindy.ps1') -Executable $exe
        & (Join-Path $PSScriptRoot 'Patch-WYD748-WaterMacro.ps1') -Executable $exe
    } elseif ($sourceHash -eq $preWaterHash) {
        & (Join-Path $PSScriptRoot 'Patch-WYD748-WaterMacro.ps1') -Executable $exe
    } elseif ($sourceHash -ne $postWaterHash) {
        throw "Fonte fora da cadeia Water: SHA-256 $sourceHash"
    }

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
    # hook.  The emitted sequence is cmp min,current / JA and
    # cmp max,current / JB for both axes.
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
    if (-not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x1A,0x3B,0x1D)) 7) -or
        -not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x5A,0x04,0x3B,0x1D)) 2) -or
        -not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x5A,0x02,0x3B,0x1D)) 7) -or
        -not (Find-AreaBranch ([byte[]](0x0F,0xB7,0x5A,0x06,0x3B,0x1D)) 2)) {
        throw 'as comparacoes de area nao implementam min/current=JA e max/current=JB.'
    }

    # CItemGrid::GetItem and the native UseItem routine both receive (x,y),
    # with y pushed first by the x86 calling convention.  The generated cave
    # stores row before cell, so the first absolute push must point to the row
    # variable and the second to the cell variable.  A reversed pair indexes
    # outside the single 9x7 carry grid and was the crash observed when the
    # macro was enabled.
    function Read-U32([byte[]]$Buffer, [int]$At) {
        return [BitConverter]::ToUInt32($Buffer, $At)
    }
    $gridOrderFound = $false
    $nativeOrderFound = $false
    for ($at = $caveOffset; $at -lt $caveOffset + $caveLength - 30; $at++) {
        if ($bytes[$at] -eq 0xFF -and $bytes[$at + 1] -eq 0x35 -and
            $bytes[$at + 6] -eq 0xFF -and $bytes[$at + 7] -eq 0x35 -and
            $bytes[$at + 12] -eq 0x8B -and $bytes[$at + 13] -eq 0x0D -and
            $bytes[$at + 18] -eq 0x8B -and $bytes[$at + 19] -eq 0x01 -and
            $bytes[$at + 20] -eq 0xFF -and $bytes[$at + 21] -eq 0x90 -and
            $bytes[$at + 22] -eq 0xB4 -and $bytes[$at + 23] -eq 0x00 -and
            $bytes[$at + 24] -eq 0x00 -and $bytes[$at + 25] -eq 0x00) {
            $first = Read-U32 $bytes ($at + 2)
            $second = Read-U32 $bytes ($at + 8)
            if ($first -ge $second) { throw 'GetItem empilha cell antes de row.' }
            $gridOrderFound = $true
        }
        if ($bytes[$at] -eq 0xFF -and $bytes[$at + 1] -eq 0x35 -and
            $bytes[$at + 6] -eq 0xFF -and $bytes[$at + 7] -eq 0x35 -and
            $bytes[$at + 12] -eq 0xFF -and $bytes[$at + 13] -eq 0x35 -and
            $bytes[$at + 18] -eq 0x50 -and
            $bytes[$at + 19] -eq 0xFF -and $bytes[$at + 20] -eq 0x35 -and
            $bytes[$at + 25] -eq 0x8B -and $bytes[$at + 26] -eq 0x0D) {
            $first = Read-U32 $bytes ($at + 2)
            $second = Read-U32 $bytes ($at + 8)
            if ($first -ge $second) { throw 'UseItem empilha cell antes de row.' }
            $nativeOrderFound = $true
        }
    }
    if (-not $gridOrderFound -or -not $nativeOrderFound) {
        throw 'ordem row/cell nao encontrada nos caminhos nativos do macro.'
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
    # The parser strips the leading slash before the formatter hook, so the
    # cave stores the command names without `/`; the player still types `/...`.
    if ($ascii.IndexOf('macropergaon' + [char]0, [StringComparison]::Ordinal) -lt 0 -or
        $ascii.IndexOf('macropergaoff' + [char]0, [StringComparison]::Ordinal) -lt 0) {
        throw 'comandos locais (sem a barra, como o parser nativo entrega) nao foram materializados na cave.'
    }
    $sha = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToUpperInvariant()
    Write-Host "Water macro static check OK; hooks=cave; handled-jumps=$handledReturnJumps; native-use=$nativeUseCalls; SHA-256=$sha"
} finally {
    [IO.Directory]::Delete($work, $true)
}
