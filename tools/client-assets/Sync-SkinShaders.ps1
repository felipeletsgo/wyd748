<#
.SYNOPSIS
Adapts the preserved 7.48 vertex programs to the active D3D9 declaration contract.
.DESCRIPTION
Only input declarations and D3D9 destination masks change.
Material, fog, inverse-view lighting and animated dye constants remain native.
Use -Check to verify runtime assets without writing them. No SDK is required.
#>
[CmdletBinding()]
param([switch]$Check)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$runtime = Join-Path $PSScriptRoot '../../tmproject/client748'
# Operand counts for the instructions used by this fixed vs_1_1 corpus.
$operands = @{ 1=2; 2=3; 3=3; 4=4; 5=3; 7=2; 8=3; 10=3; 11=3; 16=2; 20=3; 21=3; 23=3 }
$generated = @()
for ($variant = 1; $variant -le 4; ++$variant) {
    $inputPath = Join-Path $runtime "shader$variant.bin"
    $bytes = [IO.File]::ReadAllBytes($inputPath)
    if ($bytes.Length % 4) { throw "Unaligned shader: $inputPath" }
    $words = [uint32[]]::new($bytes.Length / 4)
    [Buffer]::BlockCopy($bytes, 0, $words, 0, $bytes.Length)
    if ($words[0] -ne 0xFFFE0101u) { throw "Expected vs_1_1: $inputPath" }
    $output = [Collections.Generic.List[uint32]]::new()
    $output.Add($words[0])
    # POSITION0, BLENDWEIGHT0, BLENDINDICES0, NORMAL0, TEXCOORD0.
    foreach ($declaration in @(@(0,0), @(1,1), @(2,2), @(3,3), @(5,4))) {
        # VertexDecl1 has no stored blend weight: its only bone has weight 1.
        # Declaring a missing input can crash D3D9 when the mesh is drawn.
        if ($variant -eq 1 -and $declaration[0] -eq 1) { continue }
        $output.Add(31)
        $output.Add(0x80000000u -bor [uint32]$declaration[0])
        $output.Add(0x900F0000u -bor [uint32]$declaration[1])
    }
    $ended = $false
    $blending = $true
    for ($i = 1; $i -lt $words.Length;) {
        $opcode = [int]($words[$i] -band 0xFFFF)
        if ($opcode -eq 0xFFFE) {
            $i += 1 + (($words[$i] -shr 16) -band 0x7FFF)
            if ($i -gt $words.Length) { throw "Truncated shader comment: $inputPath" }
            continue
        }
        if ($opcode -eq 0xFFFF) {
            $output.Add($words[$i]); ++$i; $ended = $true
            if ($i -ne $words.Length) { throw "Trailing shader instructions: $inputPath" }
            break
        }
        if (-not $operands.ContainsKey($opcode)) { throw "Unsupported opcode $opcode in $inputPath" }
        $count = $operands[$opcode]
        if ($i + $count -ge $words.Length) { throw "Truncated shader instruction: $inputPath" }
        $output.Add($words[$i])
        for ($operand = 1; $operand -le $count; ++$operand) {
            $word = $words[$i + $operand]
            if ($operand -eq 1 -and $opcode -in @(21,23)) {
                # D3D9 requires xyz for m4x3/m3x3; D3D8 accepted the implicit mask.
                $word = ($word -band 0xFFF0FFFFu) -bor 0x00070000u
            }
            if ($operand -eq 1 -and $blending -and $opcode -in @(4,5) -and
                $word -in @(0x800F0004u,0x800F0005u)) {
                # The matrix instructions initialize xyz only. Blend those same
                # components; w is initialized explicitly after bone blending.
                $word = ($word -band 0xFFF0FFFFu) -bor 0x00070000u
            }
            if ($operand -eq 1 -and $word -eq 0x80080004u) { $blending = $false }
            if ($operand -eq 1 -and $word -eq 0xC0010001u) {
                # oFog is scalar; retain the existing D3D9 asset's full write mask.
                $word = 0xC00F0001u
            }
            $output.Add($word)
        }
        $i += 1 + $count
    }
    if (-not $ended) { throw "Missing shader END: $inputPath" }
    $result = [byte[]]::new($output.Count * 4)
    [Buffer]::BlockCopy($output.ToArray(), 0, $result, 0, $result.Length)
    $generated += [pscustomobject]@{ Path = Join-Path $runtime "Shader/skinmesh$variant.bin"; Bytes = $result }
}
# Validate the whole input corpus before changing any runtime asset.
foreach ($shader in $generated) {
    if ($Check) {
        $actual = [IO.File]::ReadAllBytes($shader.Path)
        if ([Convert]::ToBase64String($actual) -cne [Convert]::ToBase64String($shader.Bytes)) {
            throw "Skin shader differs from the native constant contract: $($shader.Path)"
        }
    } else {
        [IO.File]::WriteAllBytes($shader.Path, $shader.Bytes)
    }
}
Write-Output "Skin shader contract verified: 4 weighted-lighting variants; unlit shaders unchanged."
