[CmdletBinding()]
param(
    [string]$AssetRoot = (Join-Path (Split-Path $PSScriptRoot -Parent) '..\client748')
)

$ErrorActionPreference = 'Stop'
$AssetRoot = (Resolve-Path -LiteralPath $AssetRoot).Path

# These sizes are format contracts consumed by the source adapters, not merely
# presence checks. A changed size must be investigated before the client boots.
$requiredFiles = [ordered]@{
    'ItemList.bin'                    = 910004
    'SkillData.bin'                   = 9984
    'serverlist.bin'                  = 7040
    'object.bin'                      = 524292
    'Env\AttributeMap.dat'            = 1048580
    'UI\strdef.bin'                   = 56324
    'UI\RC.bin'                       = 106496
    # The live 7.48 selection screen uses compact RC records and supplements
    # them with the NewUI 391..396 panels constructed by the scene source.
    'UI\SelServerScene2.bin'          = 1352
    # Server group names are 11 x 9-byte names plus 11 little-endian integers.
    'sn.bin'                           = 143
    'UI\UITextureList.bin'            = 135168
    'Effect\EffectTextureList.bin'    = 135168
    'mesh\MeshTextureList.bin'        = 540672
    'Env\EnvTextureList3.bin'         = 135168
    'mesh\ValidIndex.bin'             = 74400
    'Shader\skinmesh1.bin'            = 468
    'Shader\skinmesh2.bin'            = 616
    'Shader\skinmesh3.bin'            = 700
    'Shader\skinmesh4.bin'            = 784
    'Shader\skinmesh5.bin'            = 464
    'Shader\skinmesh6.bin'            = 612
    'Shader\skinmesh7.bin'            = 696
    'Shader\skinmesh8.bin'            = 780
    'Shader\vseffect1.bin'            = 644
    'Shader\vseffect2.bin'            = 748
    'Shader\vseffect3.bin'            = 252
    'Shader\vseffect4.bin'            = 236
    'Shader\pseffect1.bin'            = 112
    'Shader\pseffect2.bin'            = 140
    'Shader\pseffect3.bin'            = 256
    'Shader\pseffect4.bin'            = 184
    'Shader\pseffect5.bin'            = 112
    'Shader\pseffect6.bin'            = 116
}

# The recompilable renderer consumes precompiled DirectX 9 bytecode. Checking
# its provenance hash prevents a same-sized shader from silently changing the
# vertex declarations expected by the legacy 7.48 meshes.
$requiredShaderHashes = [ordered]@{
    'Shader\skinmesh1.bin' = '337309191810159FB6CE02C92C9F5E33BC1AC80E86C755FAAB5E01A64B7994C7'
    'Shader\skinmesh2.bin' = '7481A7A80F07E5AF00A7AF215B431C18861C91E2FE683C3FAA69DB7BFAB300CC'
    'Shader\skinmesh3.bin' = 'DCB20F5EA3AA9808CF1C9005A259B2F5EA96D4379348BBCD1D791CBC70D24E81'
    'Shader\skinmesh4.bin' = 'E79473FD21B0D37F2720D7F18E9D3304B2E8CCE94CD0D0C52C08DEF8682CE877'
    'Shader\skinmesh5.bin' = '206FD0736B1636F87995D97FFDDEF26D3B6381C3DB6F44E709515DF2B79DECCD'
    'Shader\skinmesh6.bin' = 'BBA33540AAB7A66399ECBDDF90882B1358E338D875A004DD51E37D935999B421'
    'Shader\skinmesh7.bin' = '753C23A67CC72E794EFAF6DEFF1945FD64D640E2C6172ED23EAF3E7DD7029355'
    'Shader\skinmesh8.bin' = '2F73583FA7BC366B2558CE7AB8DCF833C57F679C1DA5F8F69DE073A5AD65DCBA'
    'Shader\vseffect1.bin' = '227C283D3ADBCBC46923EEBD93F6839B844DD42339E5ABA4EB6FC555F3974A65'
    'Shader\vseffect2.bin' = '99308C8D8B4F6A102ED04B9A07840188494195369318C8EFD7739DD33E0F3E2C'
    'Shader\vseffect3.bin' = '24F90E105C1B57D5E9A10DFBF21EE94261B532915BB3E67C889F1CB25960ACDE'
    'Shader\vseffect4.bin' = '9B01F77FFD77D48A8052B9CE154452E75B52D5360A898A9405087BEA4C74C164'
    'Shader\pseffect1.bin' = 'A5B062C3D7EFF45037F5AF012B6EA38FD0E260D0437141B519BA83731605EAC2'
    'Shader\pseffect2.bin' = 'E91BEEF87C1A0C408E445FFA09340E66372C22D22AFD7B5F30CD8BF016D3D079'
    'Shader\pseffect3.bin' = 'FD1FF36522EDC9F8D0022CEC046E7B819A941E4799AE4536159C6A14869AEDB2'
    'Shader\pseffect4.bin' = '976B1DA04A9D966AD2A5A8C9C3135A73A81B470F9C2C9904F70B10C2275B6730'
    'Shader\pseffect5.bin' = 'A4685AE670357125F0F73A08C9878F3E5FD07037B51CB10E13DD858991445D88'
    'Shader\pseffect6.bin' = '05CBB7880D0D321595B8C57110B4BEA830D9D67040C450CC258437E861C2552F'
}

$errors = [System.Collections.Generic.List[string]]::new()
foreach ($entry in $requiredFiles.GetEnumerator()) {
    $path = Join-Path $AssetRoot $entry.Key
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        $errors.Add("missing: $($entry.Key)")
        continue
    }

    $length = (Get-Item -LiteralPath $path).Length
    if ($length -ne $entry.Value) {
        $errors.Add("invalid size: $($entry.Key) = $length, expected $($entry.Value)")
    }
}

# Runtime loaders require these textual indexes and directories but their
# lengths legitimately grow as assets are imported.
$requiredPaths = @(
    'mesh',
    'Effect',
    'Env',
    'UI',
    'mesh\MeshList.txt',
    'mesh\BoneAni4.txt',
    'UI\UITextureSetList.txt',
    'UI\FieldScene.bin',
    'UI\UIString.txt',
    'Itemname.bin',
    'itemhelp.dat',
    'config.txt'
)
foreach ($relative in $requiredPaths) {
    if (-not (Test-Path -LiteralPath (Join-Path $AssetRoot $relative))) {
        $errors.Add("missing: $relative")
    }
}

# The source client consumes the named 7.48 schema directly. Validate a full,
# unique key set so runtime never falls back to unrelated TMProject defaults.
$configKeys = @(
    'VERSION', 'RES', 'ANIMATION', 'SOUND', 'MUSIC', 'SERVER', 'BRIGHT',
    'CURSOR', 'DEMO', 'WINDOW', 'CLASSIC', 'CAMERAROTATE', 'DXT', 'KEYTYPE',
    'CAMERAVIEW'
)
$configValues = @{}
foreach ($line in Get-Content -LiteralPath (Join-Path $AssetRoot 'config.txt')) {
    if ([string]::IsNullOrWhiteSpace($line)) {
        continue
    }
    if ($line -notmatch '^\[([^]]+)\]\s+(-?\d+)\s*$') {
        $errors.Add("invalid config.txt line: $line")
        continue
    }
    if ($configValues.ContainsKey($Matches[1])) {
        $errors.Add("duplicate config.txt key: $($Matches[1])")
        continue
    }
    $configValues[$Matches[1]] = [int]$Matches[2]
}
foreach ($key in $configKeys) {
    if (-not $configValues.ContainsKey($key)) {
        $errors.Add("missing config.txt key: $key")
    }
}
foreach ($key in $configValues.Keys) {
    if ($key -notin $configKeys) {
        $errors.Add("unknown config.txt key: $key")
    }
}
if ($configValues.ContainsKey('RES') -and ($configValues.RES -lt 1 -or $configValues.RES -gt 11)) {
    $errors.Add("config.txt RES must be between 1 and 11")
}

foreach ($entry in $requiredShaderHashes.GetEnumerator()) {
    $path = Join-Path $AssetRoot $entry.Key
    if (Test-Path -LiteralPath $path -PathType Leaf) {
        $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash
        if ($actualHash -ne $entry.Value) {
            $errors.Add("invalid shader hash: $($entry.Key)")
        }
    }
}

# Scene2 is incomplete without these six texture-set records. Validate their
# symbolic names so a reordered or truncated UI atlas fails before rendering.
$serverSelectionSets = @(
    'NewUI_ServerList_Top',
    'NewUI_ChannelList_TopLeft',
    'NewUI_ChannelList_TopBottom',
    'NewUI_ChannelList_TopRight',
    'NewUI_ServerList_Button',
    'NewUI_ChannelList_Blank',
    # The original 7.48 SMessageBox constructor uses this compact NewUI pair;
    # the older TMProject skin produces a transparent, displaced login modal.
    'MessageBox_BG2',
    'MessageBox_Btn2'
)
$textureSetText = Get-Content -Raw -LiteralPath (Join-Path $AssetRoot 'UI\UITextureSetList.txt')
foreach ($textureSet in $serverSelectionSets) {
    # Accept CRLF and LF because the shipped 7.48 index is a Windows text file
    # while repository tooling may normalize line endings on other hosts.
    if ($textureSetText -notmatch "(?m)^\[$([regex]::Escape($textureSet))\]\r?$") {
        $errors.Add("missing 7.48 server selection texture set: $textureSet")
    }
}

# All four 7.48 texture tables use 264-byte rows with alpha at byte 255.
foreach ($relative in @(
    'UI\UITextureList.bin',
    'Effect\EffectTextureList.bin',
    'mesh\MeshTextureList.bin',
    'Env\EnvTextureList3.bin'
)) {
    $bytes = [IO.File]::ReadAllBytes((Join-Path $AssetRoot $relative))
    if (($bytes.Length % 264) -ne 0 -or @(0x4E, 0x41, 0x61, 0x43) -notcontains $bytes[255]) {
        $errors.Add("not a valid 7.48 texture table: $relative")
    }
}

# Validate the compact binary server-name schema consumed by the source
# adapter, including the range of every group-order value.
$serverNameBytes = [IO.File]::ReadAllBytes((Join-Path $AssetRoot 'sn.bin'))
for ($index = 0; $index -lt 11; $index++) {
    $groupOrder = [BitConverter]::ToInt32($serverNameBytes, 99 + ($index * 4))
    if ($groupOrder -lt 0 -or $groupOrder -gt 10) {
        $errors.Add("invalid sn.bin group order at row ${index}: $groupOrder")
    }
}

if ($errors.Count -ne 0) {
    throw "Client 7.48 asset validation failed:`n - $($errors -join "`n - ")"
}

[pscustomobject]@{
    AssetRoot = $AssetRoot
    Profile = 'WYD 7.48'
    TextureRecordSize = 264
    TextureRows = 512 + 512 + 2048 + 512
    SkillRows = 104
    ShaderPrograms = $requiredShaderHashes.Count
    Status = 'STATICALLY VERIFIED'
}
