#include "../internal/core/WYD748Assets.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <windows.h>

namespace {
std::filesystem::path ExecutablePath()
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};
    return executable;
}

std::filesystem::path FindFile(const char* relativePath)
{
    const auto executable = ExecutablePath();
    if (executable.empty())
        return {};

    auto current = executable.parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / relativePath;
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
        current = current.parent_path();
    }
    return {};
}
}

int RunCharacterTransferResponseTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL character transfer response: %s\n", name);
        }
    };

    const auto asset = FindFile("client748/UI/strdef.bin");
    check(!asset.empty(), "7.48 message table is available");
    if (!asset.empty()) {
        std::ifstream input(asset, std::ios::binary);
        std::vector<unsigned char> bytes{
            std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        constexpr std::size_t rowWidth = 128;
        check(bytes.size() == 440 * rowWidth + 4, "7.48 table has 440 rows");
        if (bytes.size() == 440 * rowWidth + 4) {
            check(1131 >= (bytes.size() - 4) / rowWidth,
                "newer-client message row 1131 is absent");

            std::vector<char> loaded(2000 * rowWidth, '\x7f');
            const auto assetPath = asset.string();
            const bool loadedOk = WYD748_LoadMessageStrings(
                assetPath.c_str(), loaded.data(), 2000, rowWidth);
            check(loadedOk,
                "real 7.48 message table passes checksum and loads");
            check(loadedOk && std::strcmp(loaded.data() + 204 * rowWidth, "Unknown error.") == 0,
                "runtime loader exposes the generic error at row 204");
            check(loadedOk && std::string(loaded.data() + 201 * rowWidth)
                .find("Integrated server") != std::string::npos,
                "runtime loader exposes the integrated-server success text");
            check(loadedOk && loaded[1131 * rowWidth] == '\0',
                "runtime loader leaves row 1131 empty");

            const auto fixture = ExecutablePath().parent_path() / "strdef-bad-checksum-test.bin";
            bytes[204 * rowWidth] ^= 1;
            std::ofstream output(fixture, std::ios::binary | std::ios::trunc);
            output.write(reinterpret_cast<const char*>(bytes.data()),
                static_cast<std::streamsize>(bytes.size()));
            output.close();
            check(output.good(), "corrupt message-table fixture can be written");
            if (output.good()) {
                const auto fixturePath = fixture.string();
                const bool rejected = !WYD748_LoadMessageStrings(
                    fixturePath.c_str(), loaded.data(), 2000, rowWidth);
                check(rejected,
                    "corrupt checksum is rejected");
                check(rejected && loadedOk &&
                    std::strcmp(loaded.data() + 204 * rowWidth, "Unknown error.") == 0,
                    "rejected asset preserves loaded messages");
            }
            std::error_code error;
            std::filesystem::remove(fixture, error);
            check(!error, "temporary fixture is removed");
        }
    }

    const auto source = FindFile("TMProject748/internal/app/scenes/TMSelectCharScene.cpp");
    check(!source.empty(), "selection scene source is available");
    if (!source.empty()) {
        std::ifstream input(source, std::ios::binary);
        const std::string text{std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
        const auto start = text.find("case MSG_ReqTransper_Opcode:");
        const auto end = text.find("case MSG_InitGuldName_Opcode:", start);
        check(start != std::string::npos && end != std::string::npos,
            "transfer response handler is delimited");
        if (start != std::string::npos && end != std::string::npos) {
            const auto handler = text.substr(start, end - start);
            check(handler.find("g_pMessageStringTable[204], 3500") != std::string::npos,
                "fallback displays native generic error for 3500 ms");
            check(handler.find("g_pMessageStringTable[1131]") == std::string::npos,
                "fallback never displays an absent message row");
        }
    }
    return failures;
}
