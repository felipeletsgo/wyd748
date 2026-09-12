[CmdletBinding()]
param(
    [string]$BinaryPath = (Join-Path (Split-Path -Parent $PSScriptRoot) "bin\Client Limpo\wydclient.exe"),
    [string]$EnvironmentFile,
    [ValidateRange(0, 30)]
    [int]$RenderSeconds = 0
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if (-not $IsWindows) {
    throw "The WYD client bootstrap smoke test requires Windows."
}
if (-not (Test-Path -LiteralPath $BinaryPath -PathType Leaf)) {
    throw "The WYD client executable was not found: $BinaryPath"
}

# Um arquivo opcional NAME=VALUE torna reproduzível o bootstrap com assets
# protegidos sem gravar chaves no script. Start-Process recebe somente este
# ambiente adicional e não altera permanentemente o ambiente do chamador.
$clientEnvironment = @{}
if ($EnvironmentFile) {
    if (-not (Test-Path -LiteralPath $EnvironmentFile -PathType Leaf)) {
        throw "The client environment file was not found: $EnvironmentFile"
    }
    foreach ($line in Get-Content -LiteralPath $EnvironmentFile) {
        $trimmed = $line.Trim()
        if (-not $trimmed -or $trimmed.StartsWith("#")) {
            continue
        }
        $parts = $trimmed -split "=", 2
        if ($parts.Count -ne 2 -or -not $parts[0]) {
            throw "Invalid NAME=VALUE entry in the client environment file."
        }
        $clientEnvironment[$parts[0]] = $parts[1]
    }
}

# O helper mantém todo o acesso Win32 do teste em um ponto pequeno. A busca
# combina PID e classe para nunca confundir a janela com outro processo.
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;

public static class ClientGo748SmokeNative
{
    public const uint WM_CLOSE = 0x0010;
    public const uint WM_SYSKEYDOWN = 0x0104;
    public const int VK_F4 = 0x73;
    private const int GWL_STYLE = -16;
    private const int GWL_EXSTYLE = -20;
    private const uint SWP_NOMOVE = 0x0002;
    private const uint SWP_NOZORDER = 0x0004;
    private const uint SWP_NOACTIVATE = 0x0010;

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    private delegate bool EnumWindowsProc(IntPtr hwnd, IntPtr lparam);

    [DllImport("user32.dll")]
    private static extern bool EnumWindows(EnumWindowsProc callback, IntPtr lparam);

    [DllImport("user32.dll")]
    private static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint processId);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern int GetClassNameW(IntPtr hwnd, StringBuilder className, int capacity);

    [DllImport("user32.dll")]
    public static extern bool GetClientRect(IntPtr hwnd, out RECT rect);

    [DllImport("user32.dll", EntryPoint = "GetWindowLongW")]
    private static extern int GetWindowLong(IntPtr hwnd, int index);

    [DllImport("user32.dll")]
    private static extern bool AdjustWindowRectEx(ref RECT rect, uint style, bool menu, uint exStyle);

    [DllImport("user32.dll")]
    private static extern bool SetWindowPos(
        IntPtr hwnd, IntPtr insertAfter, int x, int y, int width, int height, uint flags);

    [DllImport("user32.dll")]
    public static extern bool PostMessageW(IntPtr hwnd, uint message, IntPtr wparam, IntPtr lparam);

    public static IntPtr FindWindow(uint wantedProcessId, string wantedClass)
    {
        IntPtr result = IntPtr.Zero;
        EnumWindows((hwnd, _) =>
        {
            uint processId;
            GetWindowThreadProcessId(hwnd, out processId);
            if (processId != wantedProcessId)
                return true;

            var name = new StringBuilder(256);
            if (GetClassNameW(hwnd, name, name.Capacity) > 0 &&
                String.Equals(name.ToString(), wantedClass, StringComparison.Ordinal))
            {
                result = hwnd;
                return false;
            }
            return true;
        }, IntPtr.Zero);
        return result;
    }

    public static bool ResizeClient(IntPtr hwnd, int width, int height)
    {
        var bounds = new RECT { Right = width, Bottom = height };
        uint style = unchecked((uint)GetWindowLong(hwnd, GWL_STYLE));
        uint exStyle = unchecked((uint)GetWindowLong(hwnd, GWL_EXSTYLE));
        if (!AdjustWindowRectEx(ref bounds, style, false, exStyle))
            return false;
        return SetWindowPos(
            hwnd,
            IntPtr.Zero,
            0,
            0,
            bounds.Right - bounds.Left,
            bounds.Bottom - bounds.Top,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}
'@

function Start-SmokeClient {
    $startOptions = @{
        FilePath = $BinaryPath
        PassThru = $true
		WindowStyle = 'Hidden'
    }
    if ($clientEnvironment.Count -gt 0) {
        $startOptions.Environment = $clientEnvironment
    }
    $process = Start-Process @startOptions
    $deadline = [DateTime]::UtcNow.AddSeconds(10)
    do {
        if ($process.HasExited) {
            throw "The WYD client exited before creating its main window (exit code $($process.ExitCode))."
        }
        $window = [ClientGo748SmokeNative]::FindWindow(
            [uint32]$process.Id,
            "WYDClientGo748Window"
        )
        if ($window -ne [IntPtr]::Zero) {
            return [pscustomobject]@{ Process = $process; Window = $window }
        }
        Start-Sleep -Milliseconds 50
    } while ([DateTime]::UtcNow -lt $deadline)

    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    throw "The WYD client did not create WYDClientGo748Window within 10 seconds."
}

function Stop-SmokeClient([Diagnostics.Process]$Process) {
    if (-not $Process.HasExited) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
        $Process.WaitForExit(5000) | Out-Null
    }
    $Process.Dispose()
}

function Assert-ClientSize([IntPtr]$Window, [int]$Width, [int]$Height, [string]$Stage) {
    $bounds = New-Object ClientGo748SmokeNative+RECT
    if (-not [ClientGo748SmokeNative]::GetClientRect($Window, [ref]$bounds)) {
        throw "Could not read the client area during $Stage."
    }
    $actualWidth = $bounds.Right - $bounds.Left
    $actualHeight = $bounds.Bottom - $bounds.Top
    if ($actualWidth -ne $Width -or $actualHeight -ne $Height) {
        throw "Unexpected client area during ${Stage}: ${actualWidth}x${actualHeight}, expected ${Width}x${Height}."
    }
}

function Wait-ForExit([Diagnostics.Process]$Process, [string]$Stage) {
    if (-not $Process.WaitForExit(5000)) {
        throw "The WYD client did not exit after $Stage."
    }
    if ($Process.ExitCode -ne 0) {
        throw "The WYD client exited with code $($Process.ExitCode) after $Stage."
    }
}

$first = $null
try {
    $first = Start-SmokeClient
    Assert-ClientSize $first.Window 800 600 "startup"
    if ($RenderSeconds -gt 0) {
        Start-Sleep -Seconds $RenderSeconds
        if ($first.Process.HasExited) {
            throw "The WYD client exited during the render sample."
        }
    }
    if (-not [ClientGo748SmokeNative]::ResizeClient($first.Window, 960, 720)) {
        throw "Could not resize the WYD client window."
    }
    Start-Sleep -Milliseconds 200
    Assert-ClientSize $first.Window 960 720 "resize"
    if (-not [ClientGo748SmokeNative]::PostMessageW(
        $first.Window,
        [ClientGo748SmokeNative]::WM_CLOSE,
        [IntPtr]::Zero,
        [IntPtr]::Zero
    )) {
        throw "Could not post WM_CLOSE to the WYD client."
    }
    Wait-ForExit $first.Process "WM_CLOSE"
}
finally {
    if ($null -ne $first) {
        Stop-SmokeClient $first.Process
    }
}

$second = $null
try {
    $second = Start-SmokeClient
    # WM_SYSKEYDOWN com o bit de contexto ALT reproduz a entrada que o
    # DefWindowProc transforma no comando de fechamento de Alt+F4.
    $altF4LParam = [IntPtr]([int64]1 -bor ([int64]0x3E -shl 16) -bor ([int64]1 -shl 29))
    if (-not [ClientGo748SmokeNative]::PostMessageW(
        $second.Window,
        [ClientGo748SmokeNative]::WM_SYSKEYDOWN,
        [IntPtr][ClientGo748SmokeNative]::VK_F4,
        $altF4LParam
    )) {
        throw "Could not post Alt+F4 to the WYD client."
    }
    Wait-ForExit $second.Process "Alt+F4"
}
finally {
    if ($null -ne $second) {
        Stop-SmokeClient $second.Process
    }
}

$mode = if ($clientEnvironment.Count -gt 0) { "configured environment" } else { "default environment" }
Write-Host "Client bootstrap verified with ${mode}: class, 800x600 startup, resize, WM_CLOSE, and Alt+F4."
