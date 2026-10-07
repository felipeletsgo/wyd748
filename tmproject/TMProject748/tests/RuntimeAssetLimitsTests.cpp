#include "../internal/render/resources/RuntimeAssetLimits.h"
#include "../internal/application/AssetLoadValidation.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <windows.h>

namespace {
std::filesystem::path FindRuntimeFile(const char* relativePath)
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / "tmproject/client748" / relativePath;
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
        current = current.parent_path();
    }
    return {};
}

std::uintmax_t FileSize(const std::filesystem::path& path)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    return error ? 0 : size;
}
}

// Every shipped runtime table must fit the capacity its loader enforces; a
// table that does not fit makes the loader reject the whole file at startup.
int RunRuntimeAssetLimitsTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL runtime asset limits: %s\n", name); }
    };

    const auto meshList = FindRuntimeFile("Mesh/MeshList.txt");
    check(!meshList.empty(), "Mesh/MeshList.txt is available");
    if (!meshList.empty()) {
        std::ifstream input(meshList);
        std::string line;
        int entries = 0;
        bool allValid = true;
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (line.find_first_not_of(" \t") == std::string::npos)
                continue;
            int index = 0, count = 0, parts = 0;
            std::string path;
            if (!asset_load::ParseList(line.c_str(), index, path, count, parts, false, false,
                    MAX_COMMON_MESH, 64)) {
                std::printf("rejected MeshList line: %s\n", line.c_str());
                allValid = false;
            }
            ++entries;
        }
        check(allValid && entries > 0, "every MeshList entry fits MAX_COMMON_MESH");
    }

    const auto objectMask = FindRuntimeFile("object.bin");
    check(FileSize(objectMask) ==
            static_cast<std::uintmax_t>(MAX_OBJECT_MASK) * MAX_MASK_X * MAX_MASK_Y + 4,
        "object.bin holds MAX_OBJECT_MASK masks and a checksum");

    const auto modelTextures = FindRuntimeFile("Mesh/MeshTextureList.bin");
    const auto modelSize = FileSize(modelTextures);
    check(modelSize > 0 && modelSize % 528 == 0 && modelSize / 528 <= MAX_MODEL_TEXTURE,
        "MeshTextureList.bin fits MAX_MODEL_TEXTURE");

    const auto envTextures = FindRuntimeFile("Env/EnvTextureList3.bin");
    const auto envSize = FileSize(envTextures);
    check(envSize > 0 && envSize % 528 == 0 && envSize / 528 <= MAX_ENV_TEXTURE,
        "EnvTextureList3.bin fits MAX_ENV_TEXTURE");
    return failures;
}
