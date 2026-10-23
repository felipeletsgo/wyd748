#pragma once

#include <windows.h>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

// Shared by the launcher and game. This preference never changes gameplay state.
namespace client_language {
enum class Language { English, Portuguese };

inline Language Parse(std::string_view text)
{
    while (!text.empty() && (text.front() == ' ' || text.front() == '\r' || text.front() == '\n' || text.front() == '\t'))
        text.remove_prefix(1);
    while (!text.empty() && (text.back() == ' ' || text.back() == '\r' || text.back() == '\n' || text.back() == '\t'))
        text.remove_suffix(1);
    return text == "PT" ? Language::Portuguese : Language::English;
}

inline Language Read(const std::wstring& path)
{
    // Bound reads even when a user accidentally selects a large file.
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return Language::English;
    char bytes[17]{};
    DWORD count = 0;
    const bool valid = ReadFile(file, bytes, sizeof(bytes), &count, nullptr) && count < sizeof(bytes);
    CloseHandle(file);
    return valid ? Parse(std::string_view(bytes, count)) : Language::English;
}

inline bool Save(const std::wstring& path, Language language)
{
    const std::wstring temporary = path + L"." + std::to_wstring(GetCurrentProcessId()) + L".tmp";
    HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    const char* value = language == Language::Portuguese ? "PT\r\n" : "EN\r\n";
    DWORD count = 0;
    const bool written = WriteFile(file, value, 4, &count, nullptr) && count == 4 && FlushFileBuffers(file);
    CloseHandle(file);
    if (written && MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        return true;
    DeleteFileW(temporary.c_str());
    return false;
}

inline bool IsTextResource(std::string_view path)
{
    // An explicit allowlist prevents language folders from shadowing gameplay
    // tables, scene layouts, geometry, textures, connections, or executables.
    constexpr std::string_view files[] = {
        "itemlist.bin", "itemname.bin", "itemhelp.dat", "skillhelp.dat", "mixhelp.dat",
        "ui/uistring.txt", "ui/strdef.bin", "ui/effectstring.txt", "ui/effectsubstring.txt",
        "ui/chardesctrans.txt", "ui/chardescfoema.txt", "ui/chardescbeast.txt", "ui/chardeschunter.txt",
        "ui/interface.txt", "ui/interface1.txt", "ui/interface2.txt", "ui/interface3.txt",
        "ui/command.txt", "ui/etc.txt", "ui/mix3desc.txt", "ui/mix4desc.txt", "ui/mix6desc.txt", "ui/captions.txt", "ui/hellstoredesc.txt", "ui/descskillm.txt",
        "ui/questsubjects.txt", "ui/questsubjects1.txt", "ui/questsubjects2.txt", "ui/questsubjects3.txt", "ui/questsubjects4.txt",
        "ui/questcontents.txt", "ui/questcontents1.txt", "ui/questcontents2.txt", "ui/questcontents3.txt", "ui/questcontents4.txt",
        "ui/questmessage.txt", "ui/potalpos.txt", "notice.txt"
    };
    return std::find(std::begin(files), std::end(files), path) != std::end(files);
}

inline std::string Normalize(std::string path)
{
    std::replace(path.begin(), path.end(), '\\', '/');
    if (path.compare(0, 2, "./") == 0)
        path.erase(0, 2);
    for (char& value : path)
        if (value >= 'A' && value <= 'Z') value = static_cast<char>(value + ('a' - 'A'));
    return path;
}

inline std::string Resolve(const char* path, Language language)
{
    if (!path || !IsTextResource(Normalize(path)))
        return path ? path : "";
    const std::string relative = Normalize(path);
    if (language == Language::Portuguese) {
        const std::string localized = "PT/" + relative;
        const DWORD attributes = GetFileAttributesA(localized.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY))
            return localized;
    }
    const std::string english = "EN/" + relative;
    const DWORD attributes = GetFileAttributesA(english.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY) ? english : path;
}
} // namespace client_language
