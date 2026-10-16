#include "../internal/render/resources/UITextureSetList.h"
#include <cstdio>
#include <climits>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <windows.h>

namespace {
std::filesystem::path FindUIFolder()
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        if (std::filesystem::is_regular_file(current / "client748" / "UI" / "UITextureSetList.txt"))
            return current / "client748" / "UI";
        current = current.parent_path();
    }
    return {};
}

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), {}};
}

// RC record payload sizes by control type (UIBinary.h); the texture-set index
// sits at offset 8 for every type except the 3D object (15).
int RCPayloadSize(int type)
{
    switch (type) {
    case 1: case 2: case 16: return 40;
    case 3: return 32;
    case 6: case 12: return 52;
    case 10: return 48;
    case 13: return 184;
    case 15: return 28;
    default: return -1;
    }
}
}

// UI\UITextureSetList.txt feeds every UI control's art. The loader used to
// write past the table for an out-of-range index, desynchronize after a
// skipped block and leave missing rows uninitialized; renderers read one entry
// past a set when the state index equalled its count.
int RunUITextureSetListTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL ui texture set: %s\n", name); }
    };
    constexpr int capacity = 600; // MAX_UI_TEXTURE_SET_LIST
    ui_texture_set::Report report{};

    int parsed = 17;
    check(!ui_texture_set::ParseIntAfter("SetIndex: 999999999999999999999999", "SetIndex:", parsed) &&
        parsed == 17, "overflowing header integers are rejected without changing the output");
    check(ui_texture_set::ParseIntAfter("SetIndex: -2147483648", "SetIndex:", parsed) && parsed == INT_MIN,
        "the integer parser accepts the lower signed boundary before block validation");
    const std::string nulHeader = std::string("SetIndex: 3") + '\0' + "garbage";
    check(!ui_texture_set::ParseIntAfter(nulHeader, "SetIndex:", parsed),
        "a NUL byte cannot hide an invalid header suffix");
    ControlTextureCoord nulCoord{};
    const std::string nulRow = std::string("1,0,0,1,1,0,0") + '\0' + ",8";
    check(!ui_texture_set::ParseRow(nulRow, nulCoord),
        "a NUL byte cannot hide an eighth coordinate");

    const auto records = ui_texture_set::Parse(
        "[A]\r\nSetIndex: 3\r\nItemCount: 2\r\n1,2,3,4,5,6,7\r\n8,9,10,11,12,13,14\r\n\r\n"
        "[B]\nSetIndex: 4\nItemCount: 1\n 0, 0, 0, 32, 32, 0, 0 \n", capacity, report);
    check(records.size() == 2 && report.sets == 2 && !report.skipped && !report.truncated &&
        records[0].name == "A" && records[0].setIndex == 3 && records[0].coords.size() == 2 &&
        records[0].coords[1].nTextureIndex == 8 && records[0].coords[1].nDestY == 14 &&
        records[1].coords[0].nWidth == 32, "well-formed blocks parse with CRLF, LF and spaces");

    const auto skipped = ui_texture_set::Parse(
        "[Neg]\nSetIndex: -5\nItemCount: 2\n1,1,1,1,1,1,1\n2,2,2,2,2,2,2\n"
        "[Big]\nSetIndex: 600\nItemCount: 1\n1,1,1,1,1,1,1\n"
        "[NoCount]\nSetIndex: 7\n1,1,1,1,1,1,1\n"
        "[BadCount]\nSetIndex: 8\nItemCount: -1\n"
        "[Huge]\nSetIndex: 9\nItemCount: 999999\n"
        "[Next]\nSetIndex: 10\nItemCount: 1\n5,0,0,1,1,0,0\n", capacity, report);
    check(skipped.size() == 1 && skipped[0].setIndex == 10 && skipped[0].coords[0].nTextureIndex == 5 &&
        report.skipped == 5, "invalid blocks are skipped with their rows and the next block still parses");

    const auto shortBlock = ui_texture_set::Parse(
        "[Short]\nSetIndex: 198\nItemCount: 5\n1,0,0,1,1,0,0\n2,0,0,1,1,0,0\n"
        "[Long]\nSetIndex: 199\nItemCount: 1\n3,0,0,1,1,0,0\n4,0,0,1,1,0,0\n", capacity, report);
    check(shortBlock.size() == 2 && shortBlock[0].coords.size() == 2 && report.truncated == 1 &&
        shortBlock[1].coords.size() == 1, "short blocks keep their rows; extra rows are dropped");

    const auto corrupt = ui_texture_set::Parse(
        "[Bad]\nSetIndex: 1\nItemCount: 3\n1,0,0,1,1,0,0\n1,0,x,1,1,0,0\n"
        "[After]\nSetIndex: 2\nItemCount: 1\n1,0,0,1,1,0,0\n", capacity, report);
    check(corrupt.size() == 2 && corrupt[0].coords.size() == 1 && corrupt[1].setIndex == 2 &&
        report.truncated == 1, "a malformed row ends its block without losing the next");

    ui_texture_set::Parse("[One]\nSetIndex: 5\nItemCount: 0\n[Two]\nSetIndex: 5\nItemCount: 0\n",
        capacity, report);
    check(report.duplicates == 1 && report.sets == 2, "a repeated index is reported");

    ControlTextureCoord coords[2]{{1, 0, 0, 1, 1, 0, 0}, {2, 0, 0, 1, 1, 0, 0}};
    ControlTextureSet set{2, coords};
    ControlTextureSet empty{3, nullptr};
    check(UITextureCoordAt(&set, 0) == &coords[0] && UITextureCoordAt(&set, 1) == &coords[1],
        "valid states resolve");
    check(!UITextureCoordAt(&set, 2) && !UITextureCoordAt(&set, -1) && !UITextureCoordAt(&empty, 0) &&
        !UITextureCoordAt(static_cast<ControlTextureSet*>(nullptr), 0),
        "index == count, negative states and absent sets are rejected");

    coords[0].nWidth = 19;
    coords[1].nWidth = 31;
    for (const float scale : {0.6f, 1.0f, 1.25f, 1.8f})
    {
        for (const float glyphScale : {0.6f, 0.7f, 0.8f, 1.1f})
            check(UITextureGlyphAdvance(&set, 0, scale, glyphScale) ==
                static_cast<int>((19.0f * scale) * glyphScale),
                "glyph scaling preserves the existing multiplication and rounding order");
        check(UITextureGlyphAdvance(&set, 0, scale) == static_cast<int>(19.0f * scale) &&
            UITextureGlyphAdvance(&set, 1, scale) == static_cast<int>(31.0f * scale),
            "digit advance retains the shipped glyph width at each scale");
        const int gap = static_cast<int>(5.0f * scale);
        check(UITextureGlyphAdvance(&set, 2, scale) == gap &&
            UITextureGlyphAdvance(&set, -1, scale) == gap &&
            UITextureGlyphAdvance(&empty, 0, scale) == gap &&
            UITextureGlyphAdvance(nullptr, 11, scale) == gap,
            "missing, short and empty glyph sets use a bounded spacing fallback");
    }

    const auto ui = FindUIFolder();
    check(!ui.empty(), "client748 UI folder is available");
    if (ui.empty())
        return failures;
    const auto shipped = ui_texture_set::Parse(ReadFile(ui / "UITextureSetList.txt"), capacity, report);
    check(report.sets == 446 && !report.skipped && !report.duplicates && report.truncated == 1,
        "the shipped list has 446 sets and only set 198 is short");
    std::set<int> defined;
    bool texturesValid = true;
    for (const auto& record : shipped) {
        defined.insert(record.setIndex);
        if (record.setIndex >= 398 && record.setIndex <= 404) {
            // The flyout buttons use states 0..4 (idle, hover, pressed,
            // selected, disabled). A one-row set disappears on selection.
            bool complete = record.coords.size() == 5;
            for (const auto& coord : record.coords)
                complete = complete && coord.nTextureIndex == 134 && coord.nStartX == 479 &&
                    coord.nStartY == 4 + (record.setIndex - 398) * 30 &&
                    coord.nWidth == 28 && coord.nHeight == 28 &&
                    coord.nDestX == 0 && coord.nDestY == 0;
            check(complete, "every flyout button state retains its original 7.48 icon");
        }
        if (record.setIndex == 198 && record.coords.size() != 4)
            texturesValid = false;
        for (const auto& coord : record.coords)
            if (coord.nTextureIndex < 0 || coord.nTextureIndex >= 512) // MAX_UI_TEXTURE
                texturesValid = false;
    }
    check(texturesValid, "every shipped row names a UI texture slot and set 198 keeps 4 rows");

    for (const char* scene : {"FieldScene2.bin", "SelCharScene2.bin", "SelServerScene2.bin"}) {
        const std::string bytes = ReadFile(ui / scene);
        std::size_t offset = 0;
        bool walked = !bytes.empty(), resolved = true;
        while (walked && offset < bytes.size()) {
            int type = 0, textureSet = 0;
            if (offset + 4 > bytes.size()) { walked = false; break; }
            std::memcpy(&type, bytes.data() + offset, 4);
            offset += 4;
            const int size = RCPayloadSize(type);
            if (size < 0 || offset + size > bytes.size()) { walked = false; break; }
            std::memcpy(&textureSet, bytes.data() + offset + 8, 4);
            offset += size;
            // Positive sets index the table; -1/-2 are text-only buttons and
            // smaller values select set -N (RenderGeomRectImage).
            const int index = type == 15 ? -1 : textureSet >= 0 ? textureSet : textureSet < -2 ? -textureSet : -1;
            if (index >= 0 && !defined.count(index))
                resolved = false;
        }
        check(walked && resolved, "every RC control references a defined texture set");
    }
    return failures;
}
