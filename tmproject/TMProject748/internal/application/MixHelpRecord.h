#pragma once

#include <cstddef>
#include <cstdio>
#include <cstring>

// Mixhelp.dat holds records of ten lines: "Name ItemIndex Icon", then nine
// rows "RRGGBB" with the help text from column 9 when column 8 is a tab or a
// space. The file is data: an index outside the table skips its record, and
// names and rows are truncated to the record fields.
namespace mix_help
{
    constexpr int kRecordLines = 10;
    constexpr int kHelpRows = kRecordLines - 1;
    constexpr int kMaxRecords = 1500;
    constexpr std::size_t kLineCapacity = 256;
    constexpr std::size_t kTextColumn = 9;

    // Copies at most capacity - 1 bytes and always terminates out.
    inline void CopyBounded(char* out, std::size_t capacity, const char* text)
    {
        if (!out || capacity == 0)
            return;
        const std::size_t length = text ? strnlen(text, capacity - 1) : 0;
        if (length)
            std::memcpy(out, text, length);
        out[length] = '\0';
    }

    // Parses a record header. False when a field is missing or the index is
    // outside [0, capacity); name receives the first token, bounded by nameCapacity.
    inline bool ParseHeader(const char* line, int capacity, char* name,
        std::size_t nameCapacity, int& index, int& icon)
    {
        if (!line || !name || nameCapacity == 0 || capacity <= 0)
            return false;
        char token[kLineCapacity]{};
        int parsedIndex = 0, parsedIcon = 0;
        if (sscanf_s(line, "%255s %d %d", token, static_cast<unsigned int>(sizeof(token)),
            &parsedIndex, &parsedIcon) != 3 || parsedIndex < 0 || parsedIndex >= capacity)
            return false;
        CopyBounded(name, nameCapacity, token);
        index = parsedIndex;
        icon = parsedIcon;
        return true;
    }

    // Reads up to kMaxRecords records with readLine(buffer, capacity), which
    // returns false at the end of the input. Returns the records applied.
    template <typename Record, typename ReadLine>
    int Load(ReadLine&& readLine, Record* records, int capacity)
    {
        if (!records || capacity <= 0)
            return 0;
        int applied = 0;
        // As in the legacy parser, a row without a color keeps the last color.
        unsigned int color = 0;
        char line[kLineCapacity];
        for (int record = 0; record < kMaxRecords; ++record)
        {
            bool valid = false;
            int index = 0, icon = 0;
            char name[sizeof(records[0].Name)]{};
            int row = 0;
            for (; row < kRecordLines && readLine(line, static_cast<int>(sizeof(line))); ++row)
            {
                line[sizeof(line) - 1] = '\0';
                if (row == 0)
                {
                    valid = ParseHeader(line, capacity, name, sizeof(name), index, icon);
                    applied += valid ? 1 : 0;
                    continue;
                }
                if (!valid)
                    continue;

                char hex[7]{};
                std::memcpy(hex, line, strnlen(line, 6));
                sscanf_s(hex, "%x", &color);
                if (char* end = std::strchr(line, '\n'))
                    *end = '\0';

                Record& target = records[index];
                if (std::strlen(line) > kTextColumn - 1 &&
                    (line[kTextColumn - 1] == '\t' || line[kTextColumn - 1] == ' '))
                    CopyBounded(target.Help[row - 1], sizeof(target.Help[row - 1]), line + kTextColumn);
                target.Color[row - 1] = static_cast<short>(color);
                target.Icon = static_cast<short>(icon);
                CopyBounded(target.Name, sizeof(target.Name), name);
            }
            if (row == 0)
                break;
        }
        return applied;
    }
}
