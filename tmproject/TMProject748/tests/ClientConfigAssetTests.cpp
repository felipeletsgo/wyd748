#include "../internal/core/WYD748Assets.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <windows.h>

namespace {
std::filesystem::path FixturePath()
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};
    return std::filesystem::path(executable).parent_path() / "config-test.txt";
}

bool WriteFixture(const std::filesystem::path& path, const std::string& text)
{
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
    output.close();
    return output.good();
}

// Exactly what WYDLauncher.exe writes: every key except CAMERAVIEW, CRLF.
const char* const kLauncherConfig =
    "[VERSION] 10000\r\n[RES] 2\r\n[ANIMATION] 2\r\n[SOUND] 0\r\n[MUSIC] 0\r\n"
    "[SERVER] 0\r\n[BRIGHT] 0\r\n[CURSOR] 1\r\n[DEMO] 1\r\n[WINDOW] 1\r\n"
    "[CLASSIC] 0\r\n[CAMERAROTATE] 1\r\n[DXT] 0\r\n[KEYTYPE] 0\r\n";
}

int RunClientConfigAssetTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL client config: %s\n", name);
        }
    };

    const auto path = FixturePath();
    check(!path.empty(), "fixture path is available");
    if (path.empty())
        return failures;
    const auto file = path.string();

    int version = 0;
    short values[14]{};
    values[13] = 1; // caller default for CAMERAVIEW
    check(WriteFixture(path, kLauncherConfig) &&
        WYD748_LoadClientConfig(file.c_str(), &version, values, 14),
        "launcher-written config without CAMERAVIEW loads");
    check(version == 10000 && values[0] == 2 && values[8] == 1 && values[12] == 0,
        "launcher values reach RES, WINDOW and KEYTYPE");
    check(values[13] == 1, "missing CAMERAVIEW keeps the caller default");

    values[13] = 1;
    check(WriteFixture(path, std::string(kLauncherConfig) + "[CAMERAVIEW] 0\r\n") &&
        WYD748_LoadClientConfig(file.c_str(), &version, values, 14) && values[13] == 0,
        "an explicit CAMERAVIEW is honored");

    std::string missingWindow = kLauncherConfig;
    missingWindow.erase(missingWindow.find("[WINDOW] 1\r\n"), 12);
    check(WriteFixture(path, missingWindow) &&
        !WYD748_LoadClientConfig(file.c_str(), &version, values, 14),
        "any other missing key still rejects the file");
    check(WriteFixture(path, std::string(kLauncherConfig) + "[WINDOW] 0\r\n") &&
        !WYD748_LoadClientConfig(file.c_str(), &version, values, 14),
        "duplicate keys still reject the file");

    // Release line 7.48 writes [VERSION] 48001, above the short range. It must
    // not reject the file and silently fall back to full-screen defaults.
    std::string releaseLine = kLauncherConfig;
    releaseLine.replace(releaseLine.find("10000"), 5, "48001");
    check(WriteFixture(path, releaseLine) &&
        WYD748_LoadClientConfig(file.c_str(), &version, values, 14) &&
        version == 48001 && values[0] == 2 && values[8] == 1,
        "7.48.001 version keeps the window and resolution settings");
    releaseLine.replace(releaseLine.find("48001"), 5, "0");
    check(WriteFixture(path, releaseLine) &&
        !WYD748_LoadClientConfig(file.c_str(), &version, values, 14),
        "a zero VERSION is rejected like the native launcher");
    std::string wideValue = kLauncherConfig;
    wideValue.replace(wideValue.find("[RES] 2"), 7, "[RES] 40000");
    check(WriteFixture(path, wideValue) &&
        !WYD748_LoadClientConfig(file.c_str(), &version, values, 14),
        "non-version keys keep the short range");

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    return failures;
}
