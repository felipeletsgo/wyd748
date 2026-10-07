#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

// Open-field lighting loaded from Env/Lighting.txt so it can be tuned without
// a rebuild. Defaults are the values the client used before the file existed,
// so a missing file or key keeps the previous look. Sky states follow TMSky:
// 0 day, 1 rain, 2 sunset, 3 night.
struct LightingColor
{
    float r;
    float g;
    float b;
};

struct LightingProfile
{
    // Directional sun light per sky state (0..1); the alpha stays in TMSky.
    LightingColor sun[4] = {
        { 0.7f, 0.7f, 0.7f }, { 0.3f, 0.3f, 0.3f },
        { 0.5f, 0.4f, 0.2f }, { 0.26f, 0.34f, 0.34f } };
    // Fill light from the opposite direction in open fields (0..1).
    LightingColor fill = { 0.4f, 0.4f, 0.4f };
    // World ambient light (0..255 per channel).
    unsigned int ambient[3] = { 255, 255, 255 };
    // Sky and fog color per sky state (0..255 per channel).
    unsigned int sky[4][3] = { { 28, 120, 189 }, { 51, 54, 42 }, { 98, 47, 4 }, { 19, 46, 51 } };

    unsigned int AmbientColor() const
    {
        return 0x33000000u | (ambient[0] << 16) | (ambient[1] << 8) | ambient[2];
    }
};

constexpr const char* LightingProfile_Path = "Env\\Lighting.txt";

inline bool ParseLightingFloats(const char* text, LightingColor& color)
{
    float values[3]{};
    char* end = nullptr;
    for (float& value : values)
    {
        value = std::strtof(text, &end);
        if (end == text || value < 0.0f || value > 2.0f)
            return false;
        text = end;
    }
    color = { values[0], values[1], values[2] };
    return true;
}

inline bool ParseLightingBytes(const char* text, unsigned int (&color)[3])
{
    unsigned int values[3]{};
    char* end = nullptr;
    for (unsigned int& value : values)
    {
        const long parsed = std::strtol(text, &end, 10);
        if (end == text || parsed < 0 || parsed > 255)
            return false;
        value = static_cast<unsigned int>(parsed);
        text = end;
    }
    for (int i = 0; i < 3; ++i)
        color[i] = values[i];
    return true;
}

// Parses one "Key=r g b" line; invalid values leave the profile unchanged.
inline bool ParseLightingLine(const char* line, LightingProfile& profile)
{
    if (!line)
        return false;
    while (*line == ' ' || *line == '\t')
        ++line;
    const char* equals = std::strchr(line, '=');
    if (*line == '#' || !equals)
        return false;
    std::size_t length = static_cast<std::size_t>(equals - line);
    while (length > 0 && (line[length - 1] == ' ' || line[length - 1] == '\t'))
        --length;
    char key[32]{};
    if (length == 0 || length >= sizeof(key))
        return false;
    std::memcpy(key, line, length);
    const char* value = equals + 1;

    static constexpr const char* states[4] = { "Day", "Rain", "Sunset", "Night" };
    for (int state = 0; state < 4; ++state)
    {
        char sunKey[16]{};
        char skyKey[16]{};
        std::snprintf(sunKey, sizeof(sunKey), "Sun%s", states[state]);
        std::snprintf(skyKey, sizeof(skyKey), "Sky%s", states[state]);
        if (std::strcmp(key, sunKey) == 0)
            return ParseLightingFloats(value, profile.sun[state]);
        if (std::strcmp(key, skyKey) == 0)
            return ParseLightingBytes(value, profile.sky[state]);
    }
    if (std::strcmp(key, "FillLight") == 0)
        return ParseLightingFloats(value, profile.fill);
    if (std::strcmp(key, "Ambient") == 0)
        return ParseLightingBytes(value, profile.ambient);
    return false;
}

inline LightingProfile LoadLightingProfile(const char* path)
{
    LightingProfile profile;
    std::FILE* file = nullptr;
    if (!path || fopen_s(&file, path, "rt") != 0 || !file)
        return profile;
    char line[160]{};
    while (std::fgets(line, static_cast<int>(sizeof(line)), file))
        ParseLightingLine(line, profile);
    std::fclose(file);
    return profile;
}

// Loaded once at startup (wWinMain).
inline LightingProfile g_LightingProfile;
