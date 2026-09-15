param([string]$Root = (Resolve-Path "$PSScriptRoot/../../..").Path,
      [string]$OutputPath)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$runtime = Join-Path $Root 'tmproject/client748'
$human = Get-Content (Join-Path $Root 'tmproject/TMProject748/internal/game/entities/TMHuman.cpp') -Raw
$failures = [Collections.Generic.List[string]]::new()
function Check([bool]$condition, [string]$message) {
    if (!$condition) { $failures.Add($message) }
}

# Static regression checks on production code, not an in-game visual test.
Check (!$human.Contains('Act1[i + m_cMount ?') -and !$human.Contains('Act2[i + m_cMount ?')) 'Ground motion precedence regression'
foreach ($act in 'Act1','Act2') {
    Check ($human.Contains("$act[i + (m_cMount == 1 ? 3 : 0)]")) "Missing native ground $act indexing"
}
$eventStart = $human.IndexOf('if (m_stEffectEvent.dwTime &&')
$eventEnd = $human.IndexOf('memset(&m_stEffectEvent', $eventStart)
$event = $human.Substring($eventStart, $eventEnd - $eventStart)
Check (!$event.Contains('m_stEffectEvent.sEffectIndex = 0')) 'Ground cast ID is erased'
Check (!$event.Contains('sEffectIndex != 90')) 'Non-projectile skill 90 can repeat forever'

# Extract only the outer deferred dispatcher, not its nested level selections.
$branches = @{}
foreach ($match in [regex]::Matches($event, '(?m)^            (?:else )?if \(m_stEffectEvent\.sEffectIndex')) {
    $open = $event.IndexOf('{', $match.Index)
    $depth = 1; $end = $open + 1
    while ($depth -gt 0 -and $end -lt $event.Length) {
        if ($event[$end] -eq '{') { $depth++ }
        if ($event[$end] -eq '}') { $depth-- }
        $end++
    }
    $condition = $event.Substring($match.Index, $open - $match.Index)
    $body = $event.Substring($open, $end - $open)
    foreach ($idMatch in [regex]::Matches($condition, 'sEffectIndex == (\d+)')) {
        $id = [int]$idMatch.Groups[1].Value
        if (!$branches.ContainsKey($id)) { $branches[$id] = $body }
    }
}
# FUN_00506f9d, native 7.48: all explicit deferred branches in 0..95.
# IDs without a branch may use packet-time, persistent-affect or attack trails.
$nativeDeferred = @(2,3,8,10,11,9,15,16,12,28,17,18,19,20,22,23,24,25,27,29,31,32,34,37,38,40,21,41,43,90,54,42,44,46,48,49,50,51,53,72,80,74,76,77,79,86,87,88,89,91,95)
foreach ($id in $nativeDeferred) { Check ($branches.ContainsKey($id)) "Native deferred ID $id missing" }
$expected = @{2='new TMSkillSlowSlash';16='new TMSkillDoubleSwing';12='new TMSkillDoubleSwing';28='new TMSkillDoubleSwing';74='0xFF55AAFF';79='0xFFFFFFFF';86='new TMEffectLevelUp';90='nType = 2';91='GetSoundAndPlay(169';95='new TMArrow'}
foreach ($id in $expected.Keys) {
    Check ($branches.ContainsKey($id) -and $branches[$id].Contains($expected[$id])) "Wrong effect for native ID $id"
}
Check (!$branches[2].Contains('pTarget->')) 'Resistance cast requires overlapping target or dereferences null'
Check ($branches[3].Contains('pTarget && m_stEffectEvent.pTarget->m_pRescue') -and $branches[3].Contains('m_pRescue->StartVisible(dwServerTime)') -and $branches[3].Contains('GetSoundAndPlay(158') -and !$branches[3].Contains('new TMSkillHaste')) 'Rescue must activate the target shield with sound 158'
Check ($branches[22].Contains('vecDest.x, vecDest.y + 1.0f, vecDest.z')) 'Bash destination swaps height and map Z'
Check ($branches[42].Contains('vecStart.x, vecStart.y - 1.0f, vecStart.z')) 'Portal destination swaps height and map Z'
$bash = Get-Content (Join-Path $Root 'tmproject/TMProject748/internal/render/effects/skills/TMSkillBash.cpp') -Raw
Check ($bash -match 'm_vecPosition.x,\s+m_vecPosition.y,\s+[^\r\n]+m_vecPosition.z') 'Bash particles swap height and map Z'
$shield = Get-Content (Join-Path $Root 'tmproject/TMProject748/internal/render/effects/skills/TMSkillMagicShield.cpp') -Raw
$shieldConstructor = $shield.Substring(0, $shield.IndexOf('TMSkillMagicShield::~TMSkillMagicShield'))
Check ($shieldConstructor.Contains('m_pOwner->m_vecPosition.x, m_pOwner->m_fHeight, m_pOwner->m_vecPosition.y')) 'Shield origin does not follow its owner'

# Reproduce the human skeleton loader, including its weapon fallbacks.
$valid = [IO.File]::ReadAllBytes((Join-Path $runtime 'mesh/ValidIndex.bin'))
$boneLines = Get-Content (Join-Path $runtime 'mesh/BoneAni4.txt')
$animations = @{}; $loadedFiles = @{}; $missingAnimations = [Collections.Generic.List[string]]::new()
for ($skin = 0; $skin -lt 2; $skin++) {
    $fields = $boneLines[$skin].Trim() -split '\s+'
    $count = [int]$fields[1]; $prefix = $fields[3]
    $files = [Collections.Generic.List[object]]::new()
    $slots = New-Object 'int[,]' 60,56
    for ($i = 0; $i -lt $count; $i++) {
        $number = [BitConverter]::ToInt32($valid, ($skin * 186 + $i) * 4) + 1
        $relative = '{0}{1:D4}.ani' -f $prefix,$number
        $path = Join-Path $runtime $relative
        if (!(Test-Path -LiteralPath $path)) { $missingAnimations.Add($relative); continue }
        $bytes = [IO.File]::ReadAllBytes($path)
        $ticks = [BitConverter]::ToInt32($bytes,0); $frames = [BitConverter]::ToInt32($bytes,4)
        Check ($ticks -gt 0 -and $frames -gt 0 -and $bytes.Length -ge (8L + 64L*$ticks*$frames)) "Truncated animation $relative"
        $fileIndex = $files.Count
        $files.Add([pscustomobject]@{File=$relative;Ticks=$ticks;Frames=$frames})
        $weapon = [int][Math]::Floor($number / 100) - 1; $motion = $number % 100 - 1
        $slots[$weapon,$motion] = $fileIndex
        if ($motion -ge 4 -and $motion -lt 9) { for ($m=$motion+1;$m -lt 10;$m++) {$slots[$weapon,$m]=$fileIndex} }
        if ($motion -ge 25 -and $motion -lt 29) { for ($m=$motion+1;$m -lt 30;$m++) {$slots[$weapon,$m]=$fileIndex} }
        if ($skin -eq 0 -and $weapon -ge 12) { foreach ($m in 11,12,14) {$slots[$weapon,$m]=$slots[11,$m]} }
        if ($skin -eq 1 -and $weapon -eq 2 -and $motion -eq 4) { for ($m=0;$m -lt 4;$m++) {$slots[2,$m]=$slots[1,$m]} }
        if (($skin -eq 0 -and $number -eq 138) -or ($skin -eq 1 -and $number -eq 137)) {
            for ($w=1;$w -lt 60;$w++) { for ($m=0;$m -lt 56;$m++) {$slots[$w,$m]=$slots[0,$m]} }
        }
    }
    $animations[$skin]=$slots; $loadedFiles[$skin]=$files
}
Check ($missingAnimations.Count -eq 0) "Missing listed animations: $($missingAnimations -join ', ')"

# Base billboard textures for the corrected SlowSlash/Haste/DoubleSwing paths.
# This checks resource identity/existence, not the content of every particle frame.
$textureBytes = [IO.File]::ReadAllBytes((Join-Path $runtime 'Effect/EffectTextureList.bin'))
Check ($textureBytes.Length % 264 -eq 0 -and $textureBytes[255] -in @(78,65,97,67) -and $textureBytes[510] -notin @(78,65,97,67)) 'Unexpected legacy effect texture table format'
$textures = @{2='Effect\bright.wys';51='Effect\blight.wys';52='Effect\redlight.wys';56='Effect\flare.wys';60='Effect\line.wys';122='Effect\glevelup.wys'}
foreach ($index in $textures.Keys) {
    $relative = [Text.Encoding]::ASCII.GetString($textureBytes,$index*264,255).Split([char]0)[0]
    Check ($relative -eq $textures[$index]) "Changed base texture identity at $index"
    Check (Test-Path -LiteralPath (Join-Path $runtime $relative)) "Missing base effect texture $relative"
}
$aniTables=@{}; $tableSkin=-1; $row=0
foreach ($line in Get-Content (Join-Path $runtime 'AniSound4.txt')) {
    if ($line -match '^\[.*\]\s+(\d+)') { $tableSkin=[int]$Matches[1];$row=0;continue }
    if ($tableSkin -lt 0 -or $tableSkin -gt 1 -or [string]::IsNullOrWhiteSpace($line)) { continue }
    $fields=$line.Trim() -split '\s+'
    Check ($fields.Count -eq 10) "Unexpected AniSound human row: $line"
    for ($class=0;$class -lt 4;$class++) { $aniTables["$tableSkin/$class/$row"]=[int]$fields[1+2*$class] }
    $row++
}
$binary = [IO.File]::ReadAllBytes((Join-Path $runtime 'SkillData.bin'))
Check ($binary.Length -eq 104*96) 'Unexpected SkillData.bin record count/size'
for ($i=0;$i -lt $binary.Length;$i++) {$binary[$i]=$binary[$i] -bxor 0x5a}
$csv = @(Get-Content (Join-Path $runtime 'SkillData.csv'))
Check ($csv.Count -eq 104) 'Unexpected SkillData.csv row count'
$report = [Collections.Generic.List[object]]::new(); $projections=0
for ($id=0;$id -lt 104;$id++) {
    $fields=$csv[$id].Split(','); $offset=$id*96
    Check ([int]$fields[0] -eq $id) "CSV ID mismatch $id"
    for ($f=0;$f -lt 12;$f++) { Check ([BitConverter]::ToInt32($binary,$offset+4*$f) -eq [int]$fields[1+$f]) "CSV scalar mismatch ID $id field $f" }
    for ($f=0;$f -lt 7;$f++) { Check ([BitConverter]::ToInt32($binary,$offset+64+4*$f) -eq [int]$fields[15+$f]) "CSV tail mismatch ID $id field $f" }
    $passive=[BitConverter]::ToInt32($binary,$offset+88) -ne 0
    $resolved=[Collections.Generic.HashSet[string]]::new(); $idle=[Collections.Generic.HashSet[string]]::new()
    for ($skin=0;$skin -lt 2;$skin++) {
        $acts=@($binary[($offset+48+$skin*8)..($offset+55+$skin*8)])
        Check (($acts -join '.') -eq $fields[13+$skin]) "CSV Acts mismatch ID $id skin $skin"
        foreach ($value in $acts) { Check ($value -le 24) "Invalid motion $value ID $id" }
        if ($passive -or $id -eq 103) {continue}
        for ($mount=0;$mount -lt 2;$mount++) {
            Check ($acts[3*$mount] -gt 0) "Empty animation ID $id skin $skin mount $mount"
            for ($step=0;$step -lt 3;$step++) {
                $motion=[int]$acts[$step+3*$mount]-1
                if ($motion -lt 0) {break}
                for ($class=0;$class -lt 4;$class++) {
                    $mapped=$aniTables["$skin/$class/$($motion+28*$mount)"]
                    Check ($null -ne $mapped -and $mapped -ge 0 -and $mapped -lt 56) "Invalid animation mapping ID $id"
                    for ($w=0;$w -lt 60;$w++) {
                        $file=$loadedFiles[$skin][$animations[$skin][$w,$mapped]]
                        [void]$resolved.Add($file.File)
                        if ($mapped -eq 0 -or $mapped -eq 1 -or $mapped -eq 24 -or $mapped -eq 30) { [void]$idle.Add("skin=$skin,mount=$mount,motion=$mapped") }
                        $projections++
                    }
                }
            }
        }
    }
    $effect = if ($branches.ContainsKey($id)) { @([regex]::Matches($branches[$id], 'new (TM\w+)') | ForEach-Object {$_.Groups[1].Value} | Sort-Object -Unique) -join '+' } else { 'packet/affect/trail or no dedicated deferred effect; visual proof pending' }
    $report.Add([pscustomobject]@{ID=$id;Label=$fields[-1];Passive=$passive;Act1=$fields[13];Act2=$fields[14];Deferred=$effect;NativeDeferred=$nativeDeferred.Contains($id);AnimationFiles=@($resolved | Sort-Object);IdleMappings=@($idle | Sort-Object)})
}
$result=[pscustomobject]@{Records=$report.Count;Active=@($report | Where-Object {!$_.Passive -and $_.ID -ne 103}).Count;Projections=$projections;MissingListedAnimations=@($missingAnimations);Failures=@($failures);Coverage='Static data, human skeletons 0/1 and deferred dispatch; not full particle parity or CLIENT_TESTED';Skills=@($report)}
if ($OutputPath) { $result | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $OutputPath -Encoding utf8 }
$result | Select-Object Records,Active,Projections,MissingListedAnimations,Failures,Coverage | Format-List
if ($failures.Count) {throw "$($failures.Count) audit failures: $($failures -join '; ')"}
