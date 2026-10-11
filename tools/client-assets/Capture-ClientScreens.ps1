<#
.SYNOPSIS
Starts the built client at one or more resolutions and saves a screenshot of
its window, optionally after simulated clicks or keys.

.DESCRIPTION
UI layout check for tmproject/client748. For each [RES] index the script
writes it into config.txt, starts project.exe, waits, sends the requested
input, captures the client area with PrintWindow and closes the client. The
original config.txt is always restored. Clicks are given in 800x600 layout
coordinates and are mapped through the centre of the window with the same
uniform scale the client uses (ClientSettings UIScale=Auto).

.EXAMPLE
pwsh -File tools/client-assets/Capture-ClientScreens.ps1 -Res 2,3 -Tag login
pwsh -File tools/client-assets/Capture-ClientScreens.ps1 -Res 3 -Tag connect -Clicks 343:299,310:472
#>
param(
    # Comma-separated config.txt [RES] indexes, e.g. 2,3 (2 = 800x600, 3 = 1024x768).
    [string]$Res = '2,3',
    [int]$WaitSeconds = 12,
    [string]$Tag = 'shot',
    [string[]]$Clicks = @(),
    # Comma-separated virtual-key codes, e.g. 13 for Enter.
    [string]$Keys = '',
    [string]$OutDir = (Join-Path ([System.IO.Path]::GetTempPath()) 'wyd-client-shots')
)

$ErrorActionPreference = 'Stop'
$client = Join-Path $PSScriptRoot '..\..\tmproject\client748' | Resolve-Path
New-Item -ItemType Directory -Force $OutDir | Out-Null

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class WydCapture {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  [DllImport("user32.dll", CharSet=CharSet.Ansi)] public static extern IntPtr FindWindow(string c, string t);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
}
'@

$configPath = Join-Path $client 'config.txt'
$original = [System.IO.File]::ReadAllBytes($configPath)
try {
    $resolutions = ($Res -split '[,; ]+' | Where-Object { $_ }) | ForEach-Object { [int]$_ }
    $keyCodes = @($Keys -split '[,; ]+' | Where-Object { $_ } | ForEach-Object { [int]$_ })
    foreach ($r in $resolutions) {
        $text = [System.Text.Encoding]::ASCII.GetString($original) -replace '\[RES\] \d+', "[RES] $r"
        [System.IO.File]::WriteAllText($configPath, $text, [System.Text.Encoding]::ASCII)
        $process = Start-Process -FilePath (Join-Path $client 'project.exe') -WorkingDirectory $client -PassThru
        $hwnd = [IntPtr]::Zero
        for ($t = 0; $t -lt 900 -and $hwnd -eq [IntPtr]::Zero; $t++) {
            Start-Sleep -Milliseconds 100
            $hwnd = [WydCapture]::FindWindow([NullString]::Value, 'WYDESTINY MMORPG')
            if ($process.HasExited) { throw "client exited (code $($process.ExitCode))" }
        }
        if ($hwnd -eq [IntPtr]::Zero) { throw 'client window not found' }
        Start-Sleep -Seconds $WaitSeconds
        [void][WydCapture]::SetForegroundWindow($hwnd)

        $rect = New-Object WydCapture+RECT
        [void][WydCapture]::GetClientRect($hwnd, [ref]$rect)
        $width = $rect.R - $rect.L; $height = $rect.B - $rect.T
        $scale = [Math]::Max(1.0, [Math]::Min($width / 800.0, $height / 600.0))
        # "x:y" pairs separated by commas or spaces (pwsh -File passes one string).
        $pairs = (($Clicks -join ',') -replace '[''"]', '') -split '[,; ]+' | Where-Object { $_ }
        foreach ($click in $pairs) {
            $xy = $click.Split(':')
            $x = [int]($width / 2 + ([double]$xy[0] - 400) * $scale)
            $y = [int]($height / 2 + ([double]$xy[1] - 300) * $scale)
            $lParam = [IntPtr](($y -shl 16) -bor ($x -band 0xFFFF))
            [void][WydCapture]::PostMessage($hwnd, 0x0200, [IntPtr]0, $lParam)   # WM_MOUSEMOVE
            Start-Sleep -Milliseconds 150
            [void][WydCapture]::PostMessage($hwnd, 0x0201, [IntPtr]1, $lParam)   # WM_LBUTTONDOWN
            Start-Sleep -Milliseconds 120
            [void][WydCapture]::PostMessage($hwnd, 0x0202, [IntPtr]0, $lParam)   # WM_LBUTTONUP
            Start-Sleep -Milliseconds 2500
        }
        foreach ($key in $keyCodes) {
            [void][WydCapture]::PostMessage($hwnd, 0x0100, [IntPtr]$key, [IntPtr]0)   # WM_KEYDOWN
            [void][WydCapture]::PostMessage($hwnd, 0x0102, [IntPtr]$key, [IntPtr]0)   # WM_CHAR
            [void][WydCapture]::PostMessage($hwnd, 0x0101, [IntPtr]$key, [IntPtr]0)   # WM_KEYUP
            Start-Sleep -Milliseconds 800
        }
        Start-Sleep -Seconds 1
        if ($process.HasExited) { "client exited before the capture (code $($process.ExitCode))"; continue }

        $bitmap = New-Object System.Drawing.Bitmap $width, $height
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        $dc = $graphics.GetHdc()
        [void][WydCapture]::PrintWindow($hwnd, $dc, 3)   # PW_CLIENTONLY | PW_RENDERFULLCONTENT
        $graphics.ReleaseHdc($dc); $graphics.Dispose()
        $file = Join-Path $OutDir "$Tag-res$r-${width}x$height.png"
        $bitmap.Save($file, [System.Drawing.Imaging.ImageFormat]::Png); $bitmap.Dispose()
        "saved $file"

        if (-not $process.HasExited) { $process.CloseMainWindow() | Out-Null; Start-Sleep -Seconds 2 }
        if (-not $process.HasExited) { Stop-Process -Id $process.Id -Force }
        Start-Sleep -Seconds 1
    }
}
finally {
    [System.IO.File]::WriteAllBytes($configPath, $original)
}
