#include "LanguageExtension.cpp"
#include <cstdio>

namespace {
int failures = 0;
int starts = 0;
void Check(bool condition, const char* message)
{
    if (!condition) {
        std::printf("FAIL: %s\n", message);
        ++failures;
    }
}
LRESULT CALLBACK OriginalProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_COMMAND && LOWORD(wParam) == native_launcher_language::kStartControl)
        ++starts;
    return DefWindowProcA(window, message, wParam, lParam);
}
HWND Fixture()
{
    HWND window = CreateWindowExA(0, "WYD.Language.Test", "", WS_OVERLAPPED, 0, 0, 640, 480,
        nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | CBS_DROPDOWNLIST, 0, 0, 160, 100,
        window, reinterpret_cast<HMENU>(native_launcher_language::kLanguageControl), GetModuleHandleW(nullptr), nullptr);
    return window;
}
}

int wmain(int argc, wchar_t** argv)
{
    using namespace native_launcher_language;
    WNDCLASSA klass{};
    klass.lpfnWndProc = OriginalProc;
    klass.hInstance = GetModuleHandleW(nullptr);
    klass.lpszClassName = "WYD.Language.Test";
    Check(RegisterClassA(&klass) != 0, "register hidden fixture class");
    wchar_t temporary[MAX_PATH]{};
    Check(GetTempPathW(MAX_PATH, temporary) != 0, "resolve temporary preference directory");
    const std::wstring preference = std::wstring(temporary) + L"wyd-native-language-" +
        std::to_wstring(GetCurrentProcessId()) + L".txt";
    DeleteFileW(preference.c_str());
    HWND window = Fixture();
    Check(window != nullptr, "create hidden native controls");
    Check(Attach(window, preference), "attach production subclass");
    Check(!Attach(window, preference), "reject duplicate attachment");
    Check(SendDlgItemMessageW(window, kLanguageControl, CB_GETCOUNT, 0, 0) == 2, "exactly two language options");
    Check(SendDlgItemMessageW(window, kLanguageControl, CB_GETCURSEL, 0, 0) == 0, "absent preference defaults to English");
    SendDlgItemMessageW(window, kLanguageControl, CB_SETCURSEL, 1, 0);
    SendMessageW(window, WM_COMMAND, MAKEWPARAM(kLanguageControl, CBN_SELCHANGE), 0);
    Check(client_language::Read(preference) == client_language::Language::Portuguese, "selection saves Portuguese");
    Check(GetFileAttributesW(preference.c_str()) != INVALID_FILE_ATTRIBUTES, "preference exists after selection");
    SendMessageW(window, WM_COMMAND, MAKEWPARAM(kStartControl, BN_CLICKED), 0);
    Check(starts == 1, "Start still reaches original native handler");
    SendMessageW(window, WM_COMMAND, MAKEWPARAM(1004, BN_CLICKED), 0);
    Check(starts == 1, "Settings is not treated as Start");
    DestroyWindow(window);
    window = Fixture();
    Check(Attach(window, preference), "attach after launcher recreation");
    Check(SendDlgItemMessageW(window, kLanguageControl, CB_GETCURSEL, 0, 0) == 1, "Portuguese persists after recreation");
    SendDlgItemMessageW(window, kLanguageControl, CB_SETCURSEL, 0, 0);
    SendMessageW(window, WM_COMMAND, MAKEWPARAM(kLanguageControl, CBN_SELCHANGE), 0);
    Check(client_language::Read(preference) == client_language::Language::English, "English selection is persisted");
    DestroyWindow(window);
    Check(DeleteFileW(preference.c_str()) != FALSE, "remove temporary preference");
    if (argc == 2) {
        HMODULE library = LoadLibraryW(argv[1]);
        Check(library != nullptr, "load built x86 extension DLL");
        if (library) {
            auto initialize = reinterpret_cast<BOOL (__stdcall*)()>(GetProcAddress(library, "InitializeClientLanguage"));
            Check(initialize != nullptr, "undecorated bootstrap export is present");
            // Keep the DLL loaded while its current-thread hook is installed.
            if (initialize) {
                Check(initialize() != FALSE, "DLL installs activation hook");
                HWND hooked = Fixture();
                SetActiveWindow(hooked);
                Check(SendDlgItemMessageW(hooked, kLanguageControl, CB_GETCOUNT, 0, 0) == 2,
                    "activation hook binds the marked native dialog");
                DestroyWindow(hooked);
            }
        }
    } else {
        Check(false, "built DLL path is required");
    }
    std::printf("Native launcher language tests: %d failures\n", failures);
    return failures ? 1 : 0;
}
