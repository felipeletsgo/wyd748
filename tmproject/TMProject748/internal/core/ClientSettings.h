#pragma once

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Modern client options kept outside the native 7.48 config.txt, whose strict
// schema must stay byte-compatible with the original launcher. The file holds
// "Key=Value" lines; '#' starts a comment, unknown keys are ignored and
// values are clamped, so a missing or partial file keeps the defaults.
struct ClientSettings
{
    int focusedFps = 0;      // frame cap with focus; 0 = monitor refresh (VSync)
    int idleFps = 15;        // light profile cap: unfocused or minimized window
    int idleHideAmbient = 1; // light profile pauses ambient creatures and weather particles
    int anisotropy = 4;      // anisotropic filtering level: 0 (off), 2, 4, 8 or 16
    int fieldCache = 4;      // visited fields kept in memory; 0 disables the cache
};

constexpr const char* ClientSettings_Path = "ClientSettings.txt";

inline int ClampSetting(int value, int low, int high)
{
    return value < low ? low : (value > high ? high : value);
}

inline bool ParseClientSettingsLine(const char* line, ClientSettings& settings)
{
    if (!line)
        return false;
    while (*line == ' ' || *line == '\t')
        ++line;
    if (*line == '#' || *line == '\0' || *line == '\r' || *line == '\n')
        return false;

    char key[32]{};
    std::size_t length = 0;
    while (line[length] && line[length] != '=' && length + 1 < sizeof(key))
    {
        key[length] = line[length];
        ++length;
    }
    if (line[length] != '=')
        return false;
    const char* text = line + length + 1;
    while (length > 0 && (key[length - 1] == ' ' || key[length - 1] == '\t'))
        key[--length] = '\0';

    while (*text == ' ' || *text == '\t')
        ++text;
    char* end = nullptr;
    const long parsed = std::strtol(text, &end, 10);
    if (end == text)
        return false;
    const int value = static_cast<int>(parsed);

    if (std::strcmp(key, "FocusedFps") == 0)
        settings.focusedFps = value <= 0 ? 0 : ClampSetting(value, 10, 500);
    else if (std::strcmp(key, "IdleFps") == 0)
        settings.idleFps = value <= 0 ? 0 : ClampSetting(value, 5, 500);
    else if (std::strcmp(key, "IdleHideAmbient") == 0)
        settings.idleHideAmbient = value ? 1 : 0;
    else if (std::strcmp(key, "Anisotropy") == 0)
        settings.anisotropy = value >= 16 ? 16 : value >= 8 ? 8 : value >= 4 ? 4 : value >= 2 ? 2 : 0;
    else if (std::strcmp(key, "FieldCache") == 0)
        settings.fieldCache = ClampSetting(value, 0, 16);
    else
        return false;
    return true;
}

inline ClientSettings LoadClientSettings(const char* path)
{
    ClientSettings settings;
    std::FILE* file = nullptr;
    if (!path || fopen_s(&file, path, "rt") != 0 || !file)
        return settings;
    char line[160]{};
    while (std::fgets(line, static_cast<int>(sizeof(line)), file))
        ParseClientSettingsLine(line, settings);
    std::fclose(file);
    return settings;
}

// Loaded once at startup (wWinMain) before the device is created.
inline ClientSettings g_ClientSettings;

// True while the current frame uses the light profile (window unfocused or
// minimized). Set by NewApp::Run before each frame.
inline bool g_bIdleRenderProfile = false;

inline bool IdleHidesAmbient()
{
    return g_bIdleRenderProfile && g_ClientSettings.idleHideAmbient != 0;
}

// Anisotropy level actually used: the setting clamped to the device limit,
// or 0 (plain linear filtering) when disabled or unsupported.
inline int EffectiveAnisotropy(int setting, unsigned int deviceMax, bool deviceSupports)
{
    if (setting < 2 || !deviceSupports || deviceMax < 2)
        return 0;
    return setting > static_cast<int>(deviceMax) ? static_cast<int>(deviceMax) : setting;
}

// Milliseconds to wait before the next frame may start under an FPS cap.
// cap <= 0 or no previous frame means "run now".
inline double FrameWaitMs(double nowMs, double lastFrameStartMs, int cap)
{
    if (cap <= 0 || lastFrameStartMs < 0.0)
        return 0.0;
    const double due = lastFrameStartMs + 1000.0 / cap;
    return due > nowMs ? due - nowMs : 0.0;
}
