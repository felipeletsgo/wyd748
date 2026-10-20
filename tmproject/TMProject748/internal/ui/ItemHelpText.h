#pragma once

#include <charconv>
#include <cstddef>
#include <cstring>
#include <istream>
#include <string>
#include <string_view>

namespace item_help_748 {
inline constexpr int FirstTooltipRow = 2;
inline constexpr int Rows = 9;

struct TooltipPosition { float x, y; };
inline TooltipPosition Position(float cursorX, float cursorY, float width, float height,
    float screenWidth, float screenHeight, float scaleY)
{
    float x = cursorX - width / 2.0f;
    float y = cursorY >= screenHeight / 2.0f - 30.0f
        ? cursorY - height - 10.0f * scaleY : cursorY + 30.0f * scaleY;
    const float maxX = screenWidth > width ? screenWidth - width : 0.0f;
    const float maxY = screenHeight > height ? screenHeight - height : 0.0f;
    if (x < 0) x = 0;
    if (x > maxX) x = maxX;
    if (y < 0) y = 0;
    if (y > maxY) y = maxY;
    return {x, y};
}

// Help rows are prefixed with RRGGBBAA ("CC9900FF" is opaque gold). SText
// takes ARGB, so move the alpha byte to the top; reading the prefix as ARGB
// drew the gold help text purple.
inline int LegacyColor(unsigned int rgba)
{
    return static_cast<int>((rgba << 24) | (rgba >> 8));
}

// Runtime records contain an item ID followed by nine colour-prefixed lines.
// Preserve blank rows: their positions leave space for calculated requirements.
template<class Help>
std::size_t Load(std::istream& input, Help* table, std::size_t capacity)
{
    std::size_t loaded = 0;
    std::string line;
    const auto readLine = [&]() {
        if (!std::getline(input, line)) return false;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        return true;
    };
    while (readLine()) {
        int index = -1;
        std::string_view key(line);
        const auto first = key.find_first_not_of(" \t");
        if (first == std::string_view::npos) continue;
        key = key.substr(first, key.find_last_not_of(" \t") - first + 1);
        const auto id = std::from_chars(key.data(), key.data() + key.size(), index);
        if (id.ec != std::errc{} || id.ptr != key.data() + key.size()) continue;
        bool valid = index >= 0 && static_cast<std::size_t>(index) < capacity;
        Help record{};
        for (int row = 0; row < Rows; ++row) {
            if (!readLine()) return loaded; // Never publish a partial record.
            if (line.size() < 8 || (line.size() > 8 && line[8] != ' ' && line[8] != '\t')) {
                valid = false;
                continue;
            }
            unsigned int color = 0;
            const auto parsed = std::from_chars(line.data(), line.data() + 8, color, 16);
            if (parsed.ec != std::errc{} || parsed.ptr != line.data() + 8) {
                valid = false;
                continue;
            }
            record.Color[row] = LegacyColor(color);
            if (line.size() > 8) {
                const auto length = (line.size() - 9 < sizeof(record.Help[row]) - 1)
                    ? line.size() - 9 : sizeof(record.Help[row]) - 1;
                std::memcpy(record.Help[row], line.data() + 9, length);
            }
        }
        if (valid) {
            table[index] = record;
            ++loaded;
        }
    }
    return loaded;
}

// The ten Repliction records in the distributed itemhelp.dat are GBK text.
// Replace only their prose with the rules enforced by the current server.
template<class Help>
void LocalizeRepliction(Help* table, std::size_t capacity)
{
    if (capacity <= 4025) return;
    for (int index = 4016; index <= 4025; ++index) {
        Help& record = table[index];
        record = {};
        const auto set = [&](int row, unsigned int color, std::string_view value) {
            record.Color[row] = LegacyColor(color);
            const auto length = value.size() < sizeof(record.Help[row])
                ? value.size() : sizeof(record.Help[row]) - 1;
            std::memcpy(record.Help[row], value.data(), length);
        };
        const char grade = static_cast<char>('A' + (index - 4016) % 5);
        const char title[] = {'R', 'e', 'p', 'l', 'i', 'c', 't', 'i', 'o', 'n', ' ', grade, '\0'};
        set(0, 0xFFFF00FFu, title);
        set(2, 0xFFFFFFFFu, "Rerolls both armor bonus options.");
        set(4, 0xFFFF0000u, "Requires matching equipment grade.");
        set(5, 0xFFFF0000u, index < 4021
            ? "Maximum refinement: +6."
            : "Maximum refinement: +9.");
        set(6, 0xFFFF0000u, "Existing bonuses are replaced.");
        set(7, 0xFFFFFFFFu, "Refinement is preserved.");
    }
}
}
