#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "../../TMProject748/internal/core/ClientLanguage.h"

// Local UI extension for the production OnGame launcher. The patcher, browser,
// settings and Start handler remain owned by the original executable.
namespace native_launcher_language {
constexpr int kLanguageControl = 47100;
constexpr int kStartControl = 1003;
constexpr wchar_t kStateProperty[] = L"WYD.NativeLanguage.State";
struct WindowState {
    WNDPROC previous;
    std::wstring preference;
};
HHOOK activationHook = nullptr;

void RestoreSelection(HWND window, const std::wstring& preference)
{
    SendDlgItemMessageW(window, kLanguageControl, CB_SETCURSEL,
        client_language::Read(preference) == client_language::Language::Portuguese ? 1 : 0, 0);
}

bool SaveSelection(HWND window, WindowState& state)
{
    const auto language = SendDlgItemMessageW(window, kLanguageControl, CB_GETCURSEL, 0, 0) == 1
        ? client_language::Language::Portuguese : client_language::Language::English;
    if (client_language::Save(state.preference, language))
        return true;
    RestoreSelection(window, state.preference);
    MessageBoxW(window, L"Cannot save lang.txt. Check write access to the client folder.",
        L"WYD 7.48", MB_OK | MB_ICONWARNING);
    return false;
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<WindowState*>(GetPropW(window, kStateProperty));
    if (!state)
        return DefWindowProcA(window, message, wParam, lParam);
    const auto previous = state->previous;
    if (message == WM_COMMAND) {
        if (LOWORD(wParam) == kLanguageControl && HIWORD(wParam) == CBN_SELCHANGE) {
            SaveSelection(window, *state);
            return 0;
        }
        if (LOWORD(wParam) == kStartControl && HIWORD(wParam) == BN_CLICKED && !SaveSelection(window, *state))
            return 0;
    }
    if (message == WM_NCDESTROY) {
        RemovePropW(window, kStateProperty);
        SetWindowLongPtrA(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(previous));
        delete state;
    }
    return CallWindowProcA(previous, window, message, wParam, lParam);
}

bool Attach(HWND window, const std::wstring& preference)
{
    HWND selector = GetDlgItem(window, kLanguageControl);
    if (!selector || GetPropW(window, kStateProperty))
        return false;
    auto* state = new WindowState{nullptr, preference};
    if (!SetPropW(window, kStateProperty, state)) {
        delete state;
        return false;
    }
    SetLastError(ERROR_SUCCESS);
    const auto previous = SetWindowLongPtrA(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));
    if (!previous) {
        RemovePropW(window, kStateProperty);
        delete state;
        return false;
    }
    state->previous = reinterpret_cast<WNDPROC>(previous);
    SendMessageW(selector, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"English"));
    SendMessageW(selector, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Portuguese (Brazil)"));
    RestoreSelection(window, preference);
    return true;
}

LRESULT CALLBACK ActivationHook(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HCBT_ACTIVATE) {
        HWND window = reinterpret_cast<HWND>(wParam);
        if (GetDlgItem(window, kLanguageControl)) {
            // Unhook before any failure dialog can activate recursively.
            UnhookWindowsHookEx(activationHook);
            activationHook = nullptr;
            wchar_t path[32768]{};
            const DWORD count = GetModuleFileNameW(nullptr, path, static_cast<DWORD>(std::size(path)));
            std::wstring preference(path, count);
            const auto slash = preference.find_last_of(L"\\/");
            if (!count || count >= std::size(path) || slash == std::wstring::npos ||
                !Attach(window, preference.substr(0, slash + 1) + L"lang.txt")) {
                MessageBoxW(window, L"Cannot initialize the client language selector. Reinstall the launcher.",
                    L"WYD 7.48", MB_OK | MB_ICONERROR);
                ExitProcess(ERROR_INVALID_DATA);
            }
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}
} // namespace native_launcher_language

extern "C" __declspec(dllexport) BOOL __stdcall InitializeClientLanguage()
{
    using namespace native_launcher_language;
    if (activationHook)
        return TRUE;
    activationHook = SetWindowsHookExW(WH_CBT, ActivationHook, nullptr, GetCurrentThreadId());
    return activationHook != nullptr;
}
