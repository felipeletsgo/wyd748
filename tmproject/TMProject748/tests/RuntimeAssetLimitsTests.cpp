#include "../internal/render/resources/RuntimeAssetLimits.h"
#include "../internal/application/AssetLoadValidation.h"
#include "../internal/application/WrappedTexture.h"
#include "../internal/render/resources/ModelTextureDirectory.h"
#include "../internal/render/mesh/CostumeTable.generated.h"
#include "../internal/core/WYD748Assets.h"
#include <d3d9.h>
#include "../internal/render/resources/TextureManager.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>
#include <vector>
#include <memory>
#include <fcntl.h>
#include <share.h>

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

    std::vector<stTextureListInfo> textures(MAX_MODEL_TEXTURE);
    const bool loadedTextures = WYD748_LoadTextureList(modelTextures.string().c_str(), nullptr,
        textures.data(), textures.size());
    check(loadedTextures, "real model texture table loads through the production decoder");
    if (loadedTextures) {
        const auto before = textures;
        const auto runtime = modelTextures.parent_path().parent_path();
        auto assetPath = [&](const char* name) { return runtime / name; };
        const bool registered = model_texture_748::RegisterCitizenMantles(textures.data(), textures.size(),
            [&](const char* name) { return std::filesystem::is_regular_file(assetPath(name)); });
        check(registered, "all citizen textures register without changing existing IDs");
        for (std::size_t index = 0; index < before.size(); ++index)
            if (before[index].szFileName[0])
                check(!std::memcmp(&before[index], &textures[index], sizeof(stTextureListInfo)),
                    "every original model texture row is preserved byte for byte");
        const auto once = textures;
        check(model_texture_748::RegisterCitizenMantles(textures.data(), textures.size(),
                [&](const char* name) { return std::filesystem::is_regular_file(assetPath(name)); })
            && !std::memcmp(once.data(), textures.data(), once.size() * sizeof(stTextureListInfo)),
            "registration is idempotent");
        // Eleven imported costume renderers (e.g. ch0101282) were never added
        // to the shipped table and drew untextured until registered at load.
        check(model_texture_748::Find("mesh\\ch0101282.wys", textures.data(), textures.size()) < 0,
            "the shipped table still omits the late costume textures");
        const auto beforeCostumes = textures;
        check(model_texture_748::RegisterTextures(textures.data(), textures.size(),
                costume748::Textures, std::size(costume748::Textures),
                [&](const char* name) { return std::filesystem::is_regular_file(assetPath(name)); }),
            "every costume renderer texture registers");
        bool costumesResolve = true;
        for (const auto& texture : costume748::Textures)
            if (model_texture_748::Find(texture.name, textures.data(), textures.size()) < 0)
                costumesResolve = false;
        check(costumesResolve, "every costume renderer texture resolves after registration");
        for (std::size_t index = 0; index < beforeCostumes.size(); ++index)
            if (beforeCostumes[index].szFileName[0])
                check(!std::memcmp(&beforeCostumes[index], &textures[index], sizeof(stTextureListInfo)),
                    "costume registration preserves existing texture IDs");
        const auto costumesOnce = textures;
        check(model_texture_748::RegisterTextures(textures.data(), textures.size(),
                costume748::Textures, std::size(costume748::Textures), [](const char*) { return false; })
            && !std::memcmp(costumesOnce.data(), textures.data(), costumesOnce.size() * sizeof(stTextureListInfo)),
            "costume registration is idempotent");
        for (int file = 41; file <= 200; ++file) {
            char logical[32]{};
            std::snprintf(logical, sizeof(logical), "mesh\\mt0101%d.wyt", file);
            const std::string original = logical;
            const int index = model_texture_748::Find(logical, textures.data(), textures.size());
            check(index >= 0 && original == logical, "every native citizen skin resolves without mutating its name");
            if (index < 0) continue;
            const auto path = assetPath(textures[index].szFileName).string();
            int descriptor = -1;
            const auto openError = _sopen_s(&descriptor, path.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYNO, 0);
            check(openError == 0 && descriptor >= 0, "registered cape texture file opens");
            if (descriptor < 0) continue;
            int length = 0;
            std::unique_ptr<char[]> payload(wrapped_texture::Read(descriptor, true, nullptr, length));
            check(payload && length >= 128, "production wrapper decoder loads each cape texture");
            if (!payload || length < 128) continue;
            unsigned int headerSize = 0, width = 0, height = 0;
            std::memcpy(&headerSize, payload.get() + 4, 4);
            std::memcpy(&height, payload.get() + 12, 4);
            std::memcpy(&width, payload.get() + 16, 4);
            const bool dxt1 = !std::memcmp(payload.get() + 84, "DXT1", 4);
            const bool dxt3 = !std::memcmp(payload.get() + 84, "DXT3", 4);
            const auto required = 128ull + ((width + 3ull) / 4) * ((height + 3ull) / 4) * (dxt1 ? 8ull : 16ull);
            check(!std::memcmp(payload.get(), "DDS ", 4) && headerSize == 124 && width > 0 && height > 0
                && width <= 4096 && height <= 4096 && (dxt1 || dxt3)
                && required <= static_cast<unsigned long long>(length),
                "decoded cape has a bounded DDS header and complete base-level compressed pixels");
        }
    }
    std::vector<stTextureListInfo> rejected(200);
    strcpy_s(rejected[5].szFileName, "mesh\\preserved.wys");
    const auto unchanged = rejected;
    check(!model_texture_748::RegisterCitizenMantles(rejected.data(), rejected.size(),
            [](const char* name) { return std::strcmp(name, "mesh\\mt0101194.wys") != 0; })
        && !std::memcmp(unchanged.data(), rejected.data(), unchanged.size() * sizeof(stTextureListInfo)),
        "missing cape asset rejects registration before any mutation");
    check(!model_texture_748::RegisterCitizenMantles(rejected.data(), 165, [](const char*) { return true; })
        && !std::memcmp(unchanged.data(), rejected.data(), unchanged.size() * sizeof(stTextureListInfo)),
        "insufficient append capacity rejects registration without reusing reserved holes");
    check(model_texture_748::Find(nullptr, rejected.data(), rejected.size()) == -1
        && model_texture_748::Find("", rejected.data(), rejected.size()) == -1
        && model_texture_748::Find("x", rejected.data(), rejected.size()) == -1
        && model_texture_748::Find("mesh\\missing.wyt", rejected.data(), rejected.size()) == -1,
        "null, short and missing names safely skip empty rows");
    std::memset(rejected[0].szFileName, 'x', sizeof(rejected[0].szFileName));
    check(model_texture_748::Find("mesh\\preserved.wyt", rejected.data(), rejected.size()) == 5,
        "unterminated rows are skipped without an out-of-bounds string read");

    const auto envTextures = FindRuntimeFile("Env/EnvTextureList3.bin");
    const auto envSize = FileSize(envTextures);
    check(envSize > 0 && envSize % 528 == 0 && envSize / 528 <= MAX_ENV_TEXTURE,
        "EnvTextureList3.bin fits MAX_ENV_TEXTURE");

    const auto soundList = FindRuntimeFile("sound/soundlist.txt");
    check(!soundList.empty(), "sound/soundlist.txt is available");
    if (!soundList.empty()) {
        const auto runtime = soundList.parent_path().parent_path();
        std::ifstream input(soundList);
        std::string line;
        int entries = 0;
        while (std::getline(input, line)) {
            if (line.find_first_not_of(" \t\r") == std::string::npos)
                continue;
            std::istringstream fields(line);
            int identifier = -1, channel = -1;
            std::string relativePath, extra;
            const bool parsed = static_cast<bool>(fields >> identifier >> relativePath >> channel);
            check(parsed && !(fields >> extra), "sound entry has exactly three fields");
            if (!parsed)
                continue;
            // Preserve the shipped table's repeated ID 220; this gate checks
            // dependencies, not a reinterpretation of legacy loader ordering.
            check(identifier >= 0, "sound identifiers are nonnegative");
            for (char& character : relativePath)
                if (character == '\\')
                    character = '/';
            const auto relative = std::filesystem::path(relativePath).lexically_normal();
            const bool insideSound = !relative.is_absolute() &&
                relative.begin() != relative.end() && *relative.begin() == "sound";
            check(insideSound, "sound dependencies stay inside the runtime sound directory");
            check(insideSound && std::filesystem::is_regular_file(runtime / relative),
                relativePath.c_str());
            ++entries;
        }
        check(entries > 0 && input.eof(), "the complete shipped sound table was checked");
    }
    return failures;
}
