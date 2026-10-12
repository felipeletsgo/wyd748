// WYD launcher: keeps the client directory identical to the published
// release, then starts the game. It must live in the client directory.

#include "Updater.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#include <atomic>
#include <iterator>
#include <string>
#include <thread>

#include "resource.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

constexpr wchar_t kWindowClass[] = L"WYDLauncherWindow";
constexpr wchar_t kTitle[] = L"WYD Launcher";
constexpr wchar_t kGameExecutable[] = L"WYD.exe";
constexpr wchar_t kWebsite[] = L"https://wyd.forjago.com/";
constexpr UINT kDoneMessage = WM_APP + 1;
constexpr UINT_PTR kRefreshTimer = 1;

enum ControlId { kStatus = 100, kProgress, kDetail, kPlay, kRepair, kSite };

struct LauncherState {
    std::wstring directory;
    std::wstring launcherPath;
    bool allowSelfUpdate = true;
    wydpatch::UpdateProgress progress;
    std::atomic<bool> cancel{false};
    std::thread worker;
    std::wstring error;
    bool busy = false;
    HWND window = nullptr;
    HWND status = nullptr;
    HWND bar = nullptr;
    HWND detail = nullptr;
    HWND play = nullptr;
    HWND repair = nullptr;
    HWND site = nullptr;
    HFONT font = nullptr;
};

LauncherState g_state;

std::wstring FormatMiB(std::uint64_t bytes)
{
    wchar_t text[32];
    swprintf_s(text, L"%.1f", static_cast<double>(bytes) / (1024.0 * 1024.0));
    return text;
}

void StartUpdate(bool repair)
{
    if (g_state.worker.joinable())
        g_state.worker.join();
    g_state.busy = true;
    g_state.cancel = false;
    g_state.error.clear();
    EnableWindow(g_state.play, FALSE);
    EnableWindow(g_state.repair, FALSE);
    SendMessageW(g_state.bar, PBM_SETPOS, 0, 0);
    wydpatch::UpdateOptions options;
    options.clientDirectory = g_state.directory;
    options.launcherPath = g_state.launcherPath;
    options.repair = repair;
    options.allowSelfUpdate = g_state.allowSelfUpdate;
    g_state.worker = std::thread([options] {
        const wydpatch::UpdateResult result =
            wydpatch::RunUpdate(options, g_state.progress, g_state.cancel, g_state.error);
        PostMessageW(g_state.window, kDoneMessage, static_cast<WPARAM>(result), 0);
    });
}

void Refresh()
{
    SetWindowTextW(g_state.status, g_state.progress.Status().c_str());
    const std::uint64_t total = g_state.progress.total;
    const std::uint64_t done = g_state.progress.done;
    if (total > 0) {
        SendMessageW(g_state.bar, PBM_SETPOS, static_cast<WPARAM>(done * 1000 / total), 0);
        SetWindowTextW(g_state.detail, (FormatMiB(done) + L" / " + FormatMiB(total) + L" MiB").c_str());
    } else {
        SetWindowTextW(g_state.detail, L"");
    }
}

bool StartProcess(const std::wstring& executable, const std::wstring& arguments)
{
    std::wstring commandLine = L"\"" + executable + L"\"" + (arguments.empty() ? L"" : L" " + arguments);
    STARTUPINFOW startup{};
    startup.cb = sizeof startup;
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(executable.c_str(), commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr,
            g_state.directory.c_str(), &startup, &process))
        return false;
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}

void OnUpdateFinished(wydpatch::UpdateResult result)
{
    if (g_state.worker.joinable())
        g_state.worker.join();
    g_state.busy = false;
    Refresh();
    switch (result) {
    case wydpatch::UpdateResult::UpToDate:
        SendMessageW(g_state.bar, PBM_SETPOS, 1000, 0);
        EnableWindow(g_state.play, TRUE);
        EnableWindow(g_state.repair, TRUE);
        SetFocus(g_state.play);
        break;
    case wydpatch::UpdateResult::SelfUpdated:
        // The verified build now holds the launcher's name; hand over to it.
        if (StartProcess(g_state.launcherPath, L"--updated")) {
            DestroyWindow(g_state.window);
            return;
        }
        SetWindowTextW(g_state.status, L"The launcher was updated. Start it again.");
        break;
    case wydpatch::UpdateResult::Cancelled:
        break;
    case wydpatch::UpdateResult::Failed:
        SetWindowTextW(g_state.status, L"The update did not finish.");
        EnableWindow(g_state.repair, TRUE);
        MessageBoxW(g_state.window, g_state.error.c_str(), kTitle, MB_OK | MB_ICONWARNING);
        break;
    }
}

int Scale(int value, int dpi) { return MulDiv(value, dpi, 96); }

void CreateControls(HWND window)
{
    HDC screen = GetDC(window);
    const int dpi = GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(window, screen);

    NONCLIENTMETRICSW metrics{};
    metrics.cbSize = sizeof metrics;
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof metrics, &metrics, 0);
    g_state.font = CreateFontIndirectW(&metrics.lfMessageFont);

    const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(window, GWLP_HINSTANCE));
    auto make = [&](const wchar_t* type, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id) {
        HWND control = CreateWindowExW(0, type, text, WS_CHILD | WS_VISIBLE | style, Scale(x, dpi), Scale(y, dpi),
            Scale(w, dpi), Scale(h, dpi), window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance, nullptr);
        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(g_state.font), TRUE);
        return control;
    };
    g_state.status = make(L"STATIC", L"", SS_LEFT | SS_ENDELLIPSIS, 16, 16, 448, 20, kStatus);
    g_state.bar = make(PROGRESS_CLASSW, L"", PBS_SMOOTH, 16, 44, 448, 20, kProgress);
    SendMessageW(g_state.bar, PBM_SETRANGE32, 0, 1000);
    g_state.detail = make(L"STATIC", L"", SS_RIGHT, 16, 70, 448, 20, kDetail);
    g_state.site = make(L"BUTTON", L"Website", WS_TABSTOP | BS_PUSHBUTTON, 16, 104, 100, 30, kSite);
    g_state.repair = make(L"BUTTON", L"Repair", WS_TABSTOP | BS_PUSHBUTTON | WS_DISABLED, 248, 104, 100, 30, kRepair);
    g_state.play = make(L"BUTTON", L"Play", WS_TABSTOP | BS_DEFPUSHBUTTON | WS_DISABLED, 364, 104, 100, 30, kPlay);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        g_state.window = window;
        CreateControls(window);
        SetTimer(window, kRefreshTimer, 100, nullptr);
        StartUpdate(false);
        return 0;
    case WM_TIMER:
        if (wParam == kRefreshTimer && g_state.busy)
            Refresh();
        return 0;
    case kDoneMessage:
        OnUpdateFinished(static_cast<wydpatch::UpdateResult>(wParam));
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kPlay:
            if (!g_state.busy) {
                if (StartProcess(g_state.directory + L"\\" + kGameExecutable, L""))
                    DestroyWindow(window);
                else
                    MessageBoxW(window, L"Cannot start the game. Use Repair to restore the client files.", kTitle,
                        MB_OK | MB_ICONWARNING);
            }
            return 0;
        case kRepair:
            if (!g_state.busy)
                StartUpdate(true);
            return 0;
        case kSite:
            ShellExecuteW(window, L"open", kWebsite, nullptr, nullptr, SW_SHOWNORMAL);
            return 0;
        }
        break;
    case WM_CTLCOLORSTATIC:
        SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
        return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        KillTimer(window, kRefreshTimer);
        g_state.cancel = true;
        if (g_state.worker.joinable())
            g_state.worker.join();
        if (g_state.font)
            DeleteObject(g_state.font);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int show)
{
    // Two launchers patching the same directory would race on every file.
    HANDLE singleInstance = CreateMutexW(nullptr, TRUE, L"Local\\WYDLauncher");
    if (!singleInstance || GetLastError() == ERROR_ALREADY_EXISTS) {
        // A self-update restart may briefly overlap the exiting image.
        if (!singleInstance || WaitForSingleObject(singleInstance, 5000) == WAIT_TIMEOUT) {
            MessageBoxW(nullptr, L"The launcher is already running.", kTitle, MB_OK | MB_ICONINFORMATION);
            return 0;
        }
    }

    wchar_t path[MAX_PATH * 4];
    const DWORD length = GetModuleFileNameW(nullptr, path, static_cast<DWORD>(std::size(path)));
    if (length == 0 || length >= std::size(path))
        return 1;
    g_state.launcherPath.assign(path, length);
    g_state.directory = g_state.launcherPath.substr(0, g_state.launcherPath.find_last_of(L'\\'));
    // After a self-update, never chain into another one, even if the
    // published launcher hash and the installed image disagree.
    const std::wstring arguments = commandLine ? commandLine : L"";
    g_state.allowSelfUpdate = arguments.find(L"--updated") == std::wstring::npos &&
        arguments.find(L"--no-self-update") == std::wstring::npos;

    INITCOMMONCONTROLSEX controls{sizeof controls, ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof windowClass;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_LAUNCHER));
    windowClass.hIconSm = windowClass.hIcon;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    windowClass.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&windowClass))
        return 1;

    HDC screen = GetDC(nullptr);
    const int dpi = GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(nullptr, screen);
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT bounds{0, 0, Scale(480, dpi), Scale(150, dpi)};
    AdjustWindowRect(&bounds, style, FALSE);
    HWND window = CreateWindowExW(0, kWindowClass, kTitle, style, CW_USEDEFAULT, CW_USEDEFAULT,
        bounds.right - bounds.left, bounds.bottom - bounds.top, nullptr, nullptr, instance, nullptr);
    if (!window)
        return 1;
    ShowWindow(window, show);
    UpdateWindow(window);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    ReleaseMutex(singleInstance);
    CloseHandle(singleInstance);
    return 0;
}
