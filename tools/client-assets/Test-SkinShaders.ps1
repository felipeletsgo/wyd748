<#
.SYNOPSIS
Validates D3D9 skin shader declarations and instruction dataflow before deployment.
.DESCRIPTION
Disassembly alone accepts bytecode with uninitialized reads. Reassembly with
validation enabled rejects those programs without creating a graphics device.
Actual client rendering remains a separate required gate.
#>
[CmdletBinding()]
param([string]$ShaderDirectory = (Join-Path $PSScriptRoot '../../tmproject/client748/Shader'))
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if (-not ('SkinShaderValidation' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class SkinShaderValidation {
    [DllImport("d3dcompiler_47.dll", CharSet=CharSet.Ansi)]
    static extern int D3DDisassemble(byte[] data, UIntPtr size, uint flags,
        string comments, out IntPtr disassembly);
    [DllImport("d3dcompiler_47.dll", CharSet=CharSet.Ansi)]
    static extern int D3DAssemble(byte[] data, UIntPtr size, string name,
        IntPtr defines, IntPtr include, uint flags, out IntPtr code, out IntPtr errors);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    delegate IntPtr GetBufferPointer(IntPtr self);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    delegate uint Release(IntPtr self);
    static IntPtr Method(IntPtr blob, int slot) {
        return Marshal.ReadIntPtr(Marshal.ReadIntPtr(blob), slot * IntPtr.Size);
    }
    static string Text(IntPtr blob) {
        if (blob == IntPtr.Zero) return "";
        var get = Marshal.GetDelegateForFunctionPointer<GetBufferPointer>(Method(blob, 3));
        return Marshal.PtrToStringAnsi(get(blob));
    }
    static void Free(IntPtr blob) {
        if (blob != IntPtr.Zero)
            Marshal.GetDelegateForFunctionPointer<Release>(Method(blob, 2))(blob);
    }
    public static string Disassemble(byte[] data) {
        IntPtr blob;
        int result = D3DDisassemble(data, (UIntPtr)data.Length, 0, null, out blob);
        try {
            if (result < 0) throw new InvalidOperationException(
                "Shader disassembly failed: 0x" + result.ToString("X8"));
            return Text(blob);
        } finally { Free(blob); }
    }
    public static string Errors(string source) {
        byte[] data = Encoding.ASCII.GetBytes(source);
        IntPtr code, errors;
        int result = D3DAssemble(data, (UIntPtr)data.Length, "skinshader",
            IntPtr.Zero, IntPtr.Zero, 0, out code, out errors);
        try {
            return result < 0 ? "0x" + result.ToString("X8") + " " + Text(errors) : "";
        } finally { Free(code); Free(errors); }
    }
}
'@
}

function Assert-Inputs([string]$Source, [int]$Variant) {
    # RenderDevice::InitVertexShader / VertexDecl1..4. A one-bone vertex
    # contains indices but no weight field; its weight is implicitly one.
    $expected = @('dcl_position v0')
    if ($Variant -ne 1) { $expected += 'dcl_blendweight v1' }
    $expected += @('dcl_blendindices v2', 'dcl_normal v3', 'dcl_texcoord v4')
    $actual = @([regex]::Matches($Source, '(?m)^\s*(dcl_\w+\s+v\d+)\s*$') |
        ForEach-Object { $_.Groups[1].Value -replace '\s+', ' ' })
    if (($actual -join ';') -cne ($expected -join ';')) {
        throw "Shader $Variant input declarations do not match the vertex layout."
    }
}

$sources = @{}
foreach ($variant in 1..4) {
    $path = Join-Path $ShaderDirectory "skinmesh$variant.bin"
    $source = [SkinShaderValidation]::Disassemble([IO.File]::ReadAllBytes($path))
    Assert-Inputs $source $variant
    $errors = [SkinShaderValidation]::Errors($source)
    if ($errors) { throw "Invalid skin shader ${variant}: $errors" }
    $sources[$variant] = $source
}

# Regression fixtures are in memory; never overwrite the installed shaders.
$invalidWeights = $sources[1] -replace 'dcl_position v0', "dcl_position v0`ndcl_blendweight v1"
$rejected = $false
try { Assert-Inputs $invalidWeights 1 } catch { $rejected = $true }
if (-not $rejected) { throw 'Missing-weight declaration regression was not detected.' }
$invalidBlend = $sources[2] -replace 'mul r4\.xyz,', 'mul r4,'
$errors = [SkinShaderValidation]::Errors($invalidBlend)
if ($errors -notmatch 'X5326') { throw 'Uninitialized blend-component regression was not detected.' }
Write-Output 'Skin shaders validated: 4 programs; missing input and uninitialized blend regressions rejected.'
