#include "../internal/render/world/terrain/TerrainTileMapReader.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>
#include <windows.h>

namespace {
std::filesystem::path FindNativeTerrain()
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};

    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / "tmproject/client748/Env/Field1616.trn";
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
        current = current.parent_path();
    }
    return {};
}

std::vector<unsigned char> MakeRecord(unsigned char nameLength)
{
    std::vector<unsigned char> bytes{ nameLength };
    bytes.insert(bytes.end(), nameLength, 'A');
    bytes.push_back(16);
    bytes.push_back(16);
    bytes.insert(bytes.end(), 4096 * 12, 0);
    return bytes;
}
}

int RunTerrainTileMapReaderTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL terrain record: %s\n", name);
        }
    };

    const auto testBytes = [&](const std::vector<unsigned char>& bytes,
        bool expected, const char* name) {
        std::FILE* file = nullptr;
        if (tmpfile_s(&file) != 0 || !file) {
            check(false, name);
            return;
        }
        const bool written = bytes.empty() ||
            std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size();
        std::rewind(file);
        char mapName[128]{};
        unsigned char tiles[4096][12]{};
        int posX = -1;
        int posY = -1;
        const bool loaded = ReadTerrainTileMapRecord(file, mapName, posX, posY, tiles);
        check(written && loaded == expected &&
            (!expected || (posX == 16 && posY == 16 &&
                std::strlen(mapName) == bytes[0])), name);
        std::fclose(file);
    };

    testBytes(MakeRecord(127), true, "127-byte map name fits with terminator");
    testBytes(MakeRecord(128), false, "128-byte map name is rejected before overflow");
    testBytes({}, false, "missing name length is rejected");
    testBytes({5, 'A', 'B', 'C'}, false, "truncated map name is rejected");
    testBytes({1, 'A', 16}, false, "missing Y coordinate is rejected");
    auto shortTiles = MakeRecord(5);
    shortTiles.pop_back();
    testBytes(shortTiles, false, "truncated tile array is rejected");

    const auto path = FindNativeTerrain();
    std::FILE* file = nullptr;
    const bool opened = !path.empty() &&
        fopen_s(&file, path.string().c_str(), "rb") == 0 && file;
    if (opened) {
        char mapName[128]{};
        unsigned char tiles[4096][12]{};
        int posX = -1;
        int posY = -1;
        check(ReadTerrainTileMapRecord(file, mapName, posX, posY, tiles) &&
            std::strcmp(mapName, "Field") == 0 && posX == 16 && posY == 16,
            "native Field1616.trn loads without changing its 7.48 layout");
        std::fclose(file);
    } else {
        check(false, "native Field1616.trn asset is available");
    }
    return failures;
}
