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
}
