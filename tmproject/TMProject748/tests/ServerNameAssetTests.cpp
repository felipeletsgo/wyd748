#include "../internal/core/WYD748Assets.h"
#include "../internal/core/Basedef.h"
#include "../internal/core/ServerChannelLabel.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include <windows.h>

// Match the legacy checksum used by Basedef.cpp so asset tests can exercise
// the real message-table loader without linking the entire game runtime.
int BASE_GetSum(char* bytes, int size)
{
    int sum = 0;
    for (int index = 0; index < size; ++index) {
        const int mod = index % 7;
        if (mod == 0)
            sum += bytes[index] / 2;
        if (mod == 1)
            sum += bytes[index] ^ 0xFF;
        if (mod == 2)
            sum += 3 * bytes[index];
        if (mod == 3)
            sum += 2 * bytes[index];
        if (mod == 4)
            sum -= bytes[index] / 7;
        if (mod == 5)
            sum -= bytes[index];
        else
            sum += bytes[index] / 3;
    }
    return sum;
}

namespace {
std::filesystem::path ExecutablePath()
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};
    return executable;
}

std::filesystem::path FindAsset()
{
    const auto executable = ExecutablePath();
    if (executable.empty())
        return {};

    auto current = executable.parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / "client748" / "sn.bin";
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
        current = current.parent_path();
    }
    return {};
}
}

int RunServerNameAssetTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL server names: %s\n", name);
        }
    };

    char shortLabel[128] = "Channel-1";
    check(AppendFullChannelLabel(shortLabel, sizeof(shortLabel)) &&
        std::strcmp(shortLabel, "Channel-1     FULL") == 0,
        "full channel marker pads short names to column 14");
    char exactLabel[19] = "12345678901234";
    check(AppendFullChannelLabel(exactLabel, sizeof(exactLabel)) &&
        std::strcmp(exactLabel, "12345678901234FULL") == 0,
        "full channel marker fits the minimum capacity");
    char longLabel[128] = "12345678901234567890";
    check(AppendFullChannelLabel(longLabel, sizeof(longLabel)) &&
        std::strcmp(longLabel, "12345678901234FULL") == 0,
        "full channel marker truncates after column 14");
    char smallLabel[18] = "Channel-1";
    check(!AppendFullChannelLabel(smallLabel, sizeof(smallLabel)) &&
        std::strcmp(smallLabel, "Channel-1") == 0,
        "undersized label remains unchanged");
    char unterminatedLabel[19];
    std::memset(unterminatedLabel, 'x', sizeof(unterminatedLabel));
    check(!AppendFullChannelLabel(unterminatedLabel, sizeof(unterminatedLabel)) &&
        unterminatedLabel[14] == 'x',
        "unterminated label remains unchanged");

    const auto asset = FindAsset();
    check(!asset.empty(), "7.48 sn.bin is available");
    if (asset.empty())
        return failures;

    char names[11][16]{};
    int orders[11]{};
    const auto path = asset.string();
    check(WYD748_LoadServerNameList(path.c_str(), names, 11, orders, 11),
        "localized 7.48 sn.bin loads");
    check(std::strcmp(names[0], "__VPS") == 0 && orders[0] == 1,
        "first group keeps VPS name and order");
    check(std::strcmp(names[1], "Channel") == 0 && orders[1] == 2,
        "second group has the English Channel name and unchanged order");
    check(std::strcmp(names[2], "Featured") == 0 && orders[2] == 0,
        "third group has the English Featured name and unchanged order");
    check(orders[7] == 3 && names[7][0] == '\0',
        "unnamed ordered group is preserved");
    check(!WYD748_LoadServerNameList(nullptr, names, 11, orders, 11),
        "null path is rejected");
    check(!WYD748_LoadServerNameList(path.c_str(), names, 10, orders, 11),
        "undersized name table is rejected");
    check(!WYD748_LoadServerNameList(path.c_str(), names, 11, orders, 10),
        "undersized order table is rejected");

    std::ifstream input(asset, std::ios::binary);
    std::vector<char> bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    check(bytes.size() == 143, "localized 7.48 file keeps the fixed 143-byte layout");
    if (bytes.size() == 143) {
        const auto fixture = ExecutablePath().parent_path() / "sn-invalid-test.bin";
        const auto writeFixture = [&](const std::vector<char>& contents) {
            std::ofstream output(fixture, std::ios::binary | std::ios::trunc);
            output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
            output.close();
            return output.good();
        };
        const auto fixturePath = fixture.string();

        bytes.pop_back();
        check(writeFixture(bytes), "short fixture can be written");
        check(!WYD748_LoadServerNameList(fixturePath.c_str(), names, 11, orders, 11),
            "short file is rejected");
        bytes.push_back(0);
        bytes[99] = 11; // first little-endian group order starts after 11 x 9 names
        bytes[100] = bytes[101] = bytes[102] = 0;
        check(writeFixture(bytes), "invalid-order fixture can be written");
        check(!WYD748_LoadServerNameList(fixturePath.c_str(), names, 11, orders, 11),
            "out-of-range group order is rejected");
        check(std::strcmp(names[0], "__VPS") == 0 && orders[0] == 1,
            "rejected order does not alter previously loaded state");
        std::error_code error;
        std::filesystem::remove(fixture, error);
        check(!error, "temporary fixture is removed");
    }

    constexpr std::size_t mapSize = 1024 * 1024;
    std::vector<char> map(mapSize, '\x5A');
    std::vector<char> published(mapSize, '\x33');
    std::FILE* mapFile = nullptr;
    check(tmpfile_s(&mapFile) == 0 && mapFile != nullptr,
        "attribute-map test file opens");
    if (mapFile != nullptr) {
        check(std::fwrite(map.data(), 1, map.size(), mapFile) == map.size(),
            "complete attribute grid is written");
        const char trailer[4]{};
        check(std::fwrite(trailer, 1, sizeof(trailer), mapFile) == sizeof(trailer),
            "7.48 attribute-map trailer is written");
        std::rewind(mapFile);
        check(WYD748_ReadAttributeMap(mapFile, published.data(), published.size()) &&
            std::memcmp(map.data(), published.data(), mapSize) == 0,
            "complete attribute grid loads without publishing the trailer");
        std::fclose(mapFile);
    }

    mapFile = nullptr;
    check(tmpfile_s(&mapFile) == 0 && mapFile != nullptr,
        "truncated attribute-map test file opens");
    if (mapFile != nullptr) {
        std::fill(published.begin(), published.end(), '\x33');
        check(std::fwrite(map.data(), 1, map.size() - 1, mapFile) == map.size() - 1,
            "truncated attribute grid is written");
        std::rewind(mapFile);
        check(!WYD748_ReadAttributeMap(mapFile, published.data(), published.size() - 1) &&
            published.front() == '\x33' && published.back() == '\x33',
            "wrong destination size is rejected without changing the grid");
        check(!WYD748_ReadAttributeMap(mapFile, nullptr, published.size()),
            "null destination is rejected");
        check(!WYD748_ReadAttributeMap(nullptr, published.data(), published.size()),
            "null file is rejected");
        check(!WYD748_ReadAttributeMap(mapFile, published.data(), published.size()) &&
            published.front() == '\x33' && published.back() == '\x33',
            "truncated attribute grid leaves the previous grid unchanged");
        std::fclose(mapFile);
    }

    const auto nativeMap = asset.parent_path() / "Env" / "AttributeMap.dat";
    check(std::filesystem::is_regular_file(nativeMap) &&
        std::filesystem::file_size(nativeMap) == mapSize + 4,
        "shipped 7.48 attribute map has the grid and four-byte trailer");
    mapFile = nullptr;
    if (fopen_s(&mapFile, nativeMap.string().c_str(), "rb") == 0 && mapFile != nullptr) {
        check(WYD748_ReadAttributeMap(mapFile, published.data(), published.size()),
            "shipped 7.48 attribute map loads");
        std::fclose(mapFile);
    } else {
        check(false, "shipped 7.48 attribute map opens");
    }

    // The server CSV references these native object definitions. Exercise the
    // production loader rather than inferring its output from the file layout.
    const auto itemList = asset.parent_path() / "ItemList.bin";
    std::vector<STRUCT_ITEMLIST> items(6500);
    check(std::filesystem::is_regular_file(itemList),
        "shipped 7.48 item list is available");
    if (std::filesystem::is_regular_file(itemList)) {
        const bool itemListLoaded = WYD748_LoadItemList(itemList.string().c_str(), items.data(), items.size());
        check(itemListLoaded,
            "shipped 7.48 item list loads into runtime definitions");
        const auto effectValue = [](const STRUCT_ITEMLIST& item, short effect) {
            for (const auto& entry : item.stEffect)
                if (entry.sEffect == effect)
                    return entry.sValue;
            return short{0};
        };
        check(items[471].nIndexMesh == 148 &&
            effectValue(items[471], 34) == 5 && effectValue(items[471], 39) == 9,
            "guild gate matches the server object definition");
        check(items[472].nIndexMesh == 141 &&
            effectValue(items[472], 34) == 3 && effectValue(items[472], 39) == 9,
            "steel window matches the server object definition");
        check(items[4101].nIndexMesh == 1749 &&
            effectValue(items[4101], 34) == 9 && effectValue(items[4101], 39) == 19,
            "time gate matches the server object definition");
        check(items[746].nIndexMesh == 1607 && effectValue(items[746], 34) == 0,
            "cannon matches the server object definition");

        const auto initialObjects = asset.parent_path() / "InitItem.bin";
        check(std::filesystem::is_regular_file(initialObjects),
            "shipped 7.48 initial objects are available");
        if (itemListLoaded && std::filesystem::is_regular_file(initialObjects)) {
            std::ifstream stream(initialObjects, std::ios::binary);
            const std::vector<unsigned char> encoded(
                (std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            constexpr std::size_t recordSize = 8;
            constexpr std::size_t recordCount = 96;
            check(encoded.size() == recordSize * recordCount,
                "shipped 7.48 initial objects contain 96 complete records");
            if (encoded.size() == recordSize * recordCount) {
                const auto decodedWord = [&](std::size_t offset) {
                    return static_cast<unsigned int>(encoded[offset] ^ 0xff) |
                        (static_cast<unsigned int>(encoded[offset + 1] ^ 0xff) << 8);
                };
                int cannons = 0;
                int gates = 0;
                int machines = 0;
                bool allObjectsRenderable = true;
                for (std::size_t record = 0; record < recordCount; ++record) {
                    const std::size_t offset = record * recordSize;
                    const unsigned int index = decodedWord(offset + 4);
                    const unsigned int rotation = decodedWord(offset + 6);
                    if (index == 0 || index >= items.size() || rotation > 3) {
                        std::fprintf(stderr, "Invalid initial object %zu: index=%u rotation=%u\n",
                            record, index, rotation);
                        allObjectsRenderable = false;
                        break;
                    }
                    const auto& definition = items[index];
                    const short groundMask = effectValue(definition, 34);
                    const bool cannon = definition.nIndexMesh == 1607;
                    const bool gate = groundMask > 0;
                    // Both slot machines use EF_GROUND=10 and mesh 2784. The
                    // client renders them as gates, but has no collision mask 10.
                    const bool machine = (index == 4102 || index == 4103) &&
                        definition.nIndexMesh == 2784 && groundMask == 10;
                    if (definition.nIndexMesh <= 0 || groundMask < 0 ||
                        (groundMask > 9 && !machine) ||
                        (cannon && (index != 746 || gate))) {
                        std::fprintf(stderr,
                            "Invalid initial object %zu: index=%u mesh=%d ground=%d rotation=%u\n",
                            record, index, definition.nIndexMesh, groundMask, rotation);
                        allObjectsRenderable = false;
                        break;
                    }
                    cannons += cannon ? 1 : 0;
                    gates += gate ? 1 : 0;
                    machines += machine ? 1 : 0;
                }
                check(allObjectsRenderable,
                    "all 96 initial objects have supported client renderers and rotations");
                check(cannons == 25 && gates > 0 && machines == 2,
                    "initial objects include 25 cannons, gates, and two unmasked slot machines");
            }
        }
    }
    return failures;
}
