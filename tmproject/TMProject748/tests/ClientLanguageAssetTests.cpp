#include "../internal/core/ClientLanguage.h"
#include "../internal/core/WYD748Assets.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

int BASE_GetSum(char* data, int size);

namespace {
bool Write(const std::filesystem::path& path, const char* bytes, std::size_t size)
{
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes, size);
    output.close();
    return output.good();
}

void Sign(std::vector<char>& bytes)
{
    const int checksum = BASE_GetSum(bytes.data(), static_cast<int>(bytes.size() - 4));
    std::memcpy(bytes.data() + bytes.size() - 4, &checksum, 4);
}
}

int RunClientLanguageAssetTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL client language: %s\n", name);
        }
    };
    using namespace client_language;
    check(Parse("PT\r\n") == Language::Portuguese && Parse(" \tPT ") == Language::Portuguese,
        "saved Portuguese preference accepts line endings");
    for (const auto value : {"", "pt", "EN", "ES", "PT EN", "PT/../EN"})
        check(Parse(value) == Language::English, "missing or unsupported preference defaults to English");
    check(IsTextResource(Normalize(".\\UI\\QuestContents2.txt")), "known text paths normalize");
    for (const auto path : {"SkillData.bin", "UI/selchar.bin", "UI/selchar.txt", "mesh/item.msh", "config.txt", "../Itemname.bin"})
        check(Resolve(path, Language::Portuguese) == path, "language cannot shadow shared gameplay or assets");

    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH) {
        check(false, "fixture executable path is available");
        return failures;
    }
    const auto root = std::filesystem::path(executable).parent_path() /
        ("language-test-" + std::to_string(GetCurrentProcessId()));
    const bool created = std::filesystem::create_directory(root);
    check(created, "isolated fixture directory is new");
    if (!created) return failures;
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const auto preference = root / "lang.txt";
    check(Read(preference.wstring()) == Language::English, "missing preference reads English");
    check(Save(preference.wstring(), Language::Portuguese) && Read(preference.wstring()) == Language::Portuguese,
        "Portuguese preference persists atomically");
    check(Save(preference.wstring(), Language::English) && Read(preference.wstring()) == Language::English,
        "existing preference can switch to English");
    check(Write(preference, "PT_______________________", 25) && Read(preference.wstring()) == Language::English,
        "oversized preferences cannot select a language");
    check(Save(preference.wstring(), Language::Portuguese), "restore valid preference");
    HANDLE locked = CreateFileW(preference.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    check(locked != INVALID_HANDLE_VALUE && !Save(preference.wstring(), Language::English),
        "failed atomic replacement is reported");
    if (locked != INVALID_HANDLE_VALUE) CloseHandle(locked);
    check(Read(preference.wstring()) == Language::Portuguese, "failed save preserves the previous preference");

    std::filesystem::create_directories(root / "EN/UI");
    std::filesystem::create_directories(root / "PT/UI");
    check(Resolve("UI/UIString.txt", Language::Portuguese) == "UI/UIString.txt", "missing packs use canonical text");
    Write(root / "EN/UI/UIString.txt", "EN", 2);
    check(Resolve("UI/UIString.txt", Language::Portuguese) == "EN/ui/uistring.txt", "missing Portuguese file uses English");
    Write(root / "PT/UI/UIString.txt", "PT", 2);
    check(Resolve("UI/UIString.txt", Language::Portuguese) == "PT/ui/uistring.txt", "Portuguese file wins when present");
    check(Resolve("UI/UIString.txt", Language::English) == "EN/ui/uistring.txt", "English ignores Portuguese resources");

    std::vector<char> canonical(2 * 140 + 4, '\x5A');
    auto localized = canonical;
    localized[0] = 'P' ^ 0x5A;
    localized[140] = 'T' ^ 0x5A;
    Sign(canonical); Sign(localized);
    const auto base = (root / "base.bin").string();
    const auto pack = (root / "names.bin").string();
    Write(base, canonical.data(), canonical.size());
    Write(pack, localized.data(), localized.size());
    std::array<char, 160> output{};
    output.fill('!');
    check(WYD748_LoadLocalizedItemListNames(pack.c_str(), base.c_str(), output.data(), 2, 80) &&
        output[0] == 'P' && output[80] == 'T' && output[64] == '!',
        "only localized name fields reach the destination with its actual stride");
    std::vector<char> nameTable(2 * 68, 0);
    for (int index = 0; index < 2; ++index) {
        std::memcpy(nameTable.data() + index * 68, &index, 4);
        for (int column = 0; column < 62; ++column)
            nameTable[index * 68 + 4 + column] = static_cast<char>(column + (column ? 0 : index ? 'T' : 'P'));
    }
    const auto companion = (root / "itemnames.bin").string();
    Write(companion, nameTable.data(), nameTable.size());
    check(WYD748_LoadLocalizedItemListNames(pack.c_str(), base.c_str(), output.data(), 2, 80, companion.c_str()),
        "matching Itemname companion is accepted");
    output.fill('!');
    nameTable[4] = 'X';
    Write(companion, nameTable.data(), nameTable.size());
    check(!WYD748_LoadLocalizedItemListNames(pack.c_str(), base.c_str(), output.data(), 2, 80, companion.c_str()) && output[0] == '!',
        "mismatched Itemname companion rejects the entire overlay");
    output.fill('!');
    localized[140 + 70] ^= 1; Sign(localized);
    Write(pack, localized.data(), localized.size());
    check(!WYD748_LoadLocalizedItemListNames(pack.c_str(), base.c_str(), output.data(), 2, 80) && output[0] == '!',
        "a changed requirement rejects the entire locale before writing any name");
    localized = canonical;
    std::fill(localized.begin() + 140, localized.begin() + 204, 'A'); Sign(localized);
    Write(pack, localized.data(), localized.size());
    check(!WYD748_LoadLocalizedItemListNames(pack.c_str(), base.c_str(), output.data(), 2, 80) && output[0] == '!',
        "unterminated names reject the entire locale");
    localized = canonical; localized.back() ^= 1;
    Write(pack, localized.data(), localized.size());
    check(!WYD748_LoadLocalizedItemListNames(pack.c_str(), base.c_str(), output.data(), 2, 80),
        "corrupt locale checksum is rejected");

    std::filesystem::current_path(previous);
    std::error_code error;
    std::filesystem::remove_all(root, error);
    check(!error, "isolated test files are removed");

    // Exercise the real shipped packs through the actual C++ decoders, not
    // only the generator's interpretation of their binary formats.
    auto current = std::filesystem::path(executable).parent_path();
    std::filesystem::path runtime;
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        if (std::filesystem::is_regular_file(current / "tmproject/client748/ItemList.bin")) {
            runtime = current / "tmproject/client748";
            break;
        }
        current = current.parent_path();
    }
    check(!runtime.empty(), "shared runtime is available for locale integration checks");
    if (!runtime.empty()) {
        const auto sharedCatalog = (runtime / "ItemList.bin").string();
        for (const auto language : {"EN", "PT"}) {
            const auto languageRoot = runtime / language;
            std::vector<char> items(6500 * 64);
            const auto localizedCatalog = (languageRoot / "ItemList.bin").string();
            const auto localizedNames = (languageRoot / "Itemname.bin").string();
            check(WYD748_LoadLocalizedItemListNames(localizedCatalog.c_str(), sharedCatalog.c_str(),
                items.data(), 6500, 64, localizedNames.c_str()), "shipped catalog and names pass the native-format adapter");
            std::vector<char> messages(2000 * 128);
            check(WYD748_LoadMessageStrings((languageRoot / "UI/strdef.bin").string().c_str(),
                messages.data(), 2000, 128), "shipped messages pass the checksum and table adapter");
            for (int index = 440; index <= 444; ++index)
                check(messages[index * 128] != '\0', "Mystic Land guidance is available in each locale");
            check(std::string(messages.data() + 441 * 128).find("Exploit_Leader") != std::string::npos,
                "Mystic Land guidance retains the reward NPC identity");
            check(std::string(messages.data() + 444 * 128).find(
                std::strcmp(language, "EN") == 0 ? "two random bonuses" : "dois adicionais") != std::string::npos,
                "Mystic Land reward guidance follows the selected language");
            std::vector<char> labels(500 * 64);
            check(WYD748_LoadUIStrings((languageRoot / "UI/UIString.txt").string().c_str(),
                labels.data(), 500, 64), "shipped indexed labels fit the actual UI buffers");
            std::vector<char> effects(50 * 24);
            check(WYD748_LoadEffectStrings((languageRoot / "UI/EffectString.txt").string().c_str(),
                effects.data(), 50, 24, 1), "shipped effect names fit the actual UI buffers");
            if (std::filesystem::is_regular_file(runtime / "UI/EffectSubString.txt"))
                check(WYD748_LoadEffectStrings((languageRoot / "UI/EffectSubString.txt").string().c_str(),
                    effects.data(), 10, 24, 0), "shipped effect suffixes fit the actual UI buffers");
            else
                check(!std::filesystem::exists(languageRoot / "UI/EffectSubString.txt"),
                    "an absent optional shared suffix table is not invented by a locale");
        }
    }
    return failures;
}
