#pragma once

// Client patch manifest, format 1. Published by
// tools/deployment/Build-ClientPatch.ps1 at /patch/manifest.txt:
//
//   WYDPATCH 1
//   release <id>
//   launcher <sha256> <size>
//   file <sha256> <size> <replace|keep> <relative/path>
//   ...
//   end <file count>
//
// Blobs are content-addressed at /patch/files/<sha256>. The parser is strict:
// any unknown directive, malformed field, unsafe path, duplicate path or count
// mismatch rejects the whole manifest, so a truncated or tampered download
// never produces a partial update plan.

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace wydpatch {

constexpr int kManifestFormat = 1;
constexpr std::size_t kMaxManifestBytes = 8u << 20;
constexpr std::size_t kMaxFiles = 65536;
constexpr std::uint64_t kMaxFileSize = 1ull << 30;
constexpr std::size_t kMaxPathLength = 200;

// Names owned by the launcher itself; the manifest may never target them.
constexpr const char* kLauncherName = "WYDLauncher.exe";
constexpr const char* kPendingLauncherName = "TMPWYDLauncher.exe";
constexpr const char* kRetiredLauncherName = "WYDLauncher.old";
constexpr const char* kCacheName = "patch.cache";
constexpr const char* kPartialSuffix = ".wydnew";

enum class Install { Replace, Keep };

struct FileEntry {
    std::string sha256;
    std::uint64_t size = 0;
    Install install = Install::Replace;
    std::string path;
};

struct Manifest {
    std::string release;
    std::string launcherSha256;
    std::uint64_t launcherSize = 0;
    std::vector<FileEntry> files;
};

inline char FoldAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// Windows paths are case-insensitive; compare manifest paths the same way.
inline std::string FoldPath(std::string_view path)
{
    std::string folded(path);
    for (char& c : folded)
        c = FoldAscii(c);
    return folded;
}

inline bool EqualsFolded(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (FoldAscii(a[i]) != FoldAscii(b[i]))
            return false;
    return true;
}

inline bool IsSha256Hex(std::string_view text)
{
    if (text.size() != 64)
        return false;
    for (char c : text)
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
            return false;
    return true;
}

inline bool ParseSize(std::string_view text, std::uint64_t limit, std::uint64_t& value)
{
    if (text.empty() || text.size() > 20 || (text.size() > 1 && text[0] == '0'))
        return false;
    std::uint64_t result = 0;
    for (char c : text) {
        if (c < '0' || c > '9')
            return false;
        const std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
        if (result > (limit - digit) / 10)
            return false;
        result = result * 10 + digit;
    }
    value = result;
    return true;
}

inline bool IsReservedDeviceName(std::string_view segment)
{
    // CON.txt is as reserved as CON on Windows; only the stem matters.
    const std::size_t dot = segment.find('.');
    const std::string stem = FoldPath(segment.substr(0, dot));
    if (stem == "con" || stem == "prn" || stem == "aux" || stem == "nul")
        return true;
    return stem.size() == 4 && (stem.compare(0, 3, "com") == 0 || stem.compare(0, 3, "lpt") == 0) &&
        stem[3] >= '1' && stem[3] <= '9';
}

// A manifest path is relative to the client directory, '/'-separated, and
// restricted to characters that are identical on every Windows code page.
inline bool IsSafeRelativePath(std::string_view path)
{
    if (path.empty() || path.size() > kMaxPathLength)
        return false;
    std::size_t start = 0;
    while (start <= path.size()) {
        const std::size_t end = path.find('/', start);
        const std::string_view segment =
            path.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
        if (segment.empty() || segment == "." || segment == ".." ||
            segment.back() == '.' || IsReservedDeviceName(segment))
            return false;
        for (char c : segment) {
            const bool allowed = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
            if (!allowed)
                return false;
        }
        if (end == std::string_view::npos)
            break;
        start = end + 1;
    }
    return true;
}

inline bool IsLauncherOwnedPath(std::string_view path)
{
    if (path.size() >= std::char_traits<char>::length(kPartialSuffix) &&
        EqualsFolded(path.substr(path.size() - std::char_traits<char>::length(kPartialSuffix)), kPartialSuffix))
        return true;
    return EqualsFolded(path, kLauncherName) || EqualsFolded(path, kPendingLauncherName) ||
        EqualsFolded(path, kRetiredLauncherName) || EqualsFolded(path, kCacheName);
}

inline bool IsReleaseId(std::string_view text)
{
    if (text.empty() || text.size() > 64)
        return false;
    for (char c : text)
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                c == '.' || c == '_' || c == '-'))
            return false;
    return true;
}

// Splits on single spaces; empty fields (double spaces) are malformed.
inline std::vector<std::string_view> SplitFields(std::string_view line)
{
    std::vector<std::string_view> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = line.find(' ', start);
        fields.push_back(line.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start));
        if (end == std::string_view::npos)
            break;
        start = end + 1;
    }
    return fields;
}

inline bool ParseManifest(std::string_view text, Manifest& out, std::string& error)
{
    out = Manifest{};
    auto fail = [&](std::size_t lineNumber, const char* reason) {
        error = "manifest line " + std::to_string(lineNumber) + ": " + reason;
        out = Manifest{};
        return false;
    };
    if (text.size() > kMaxManifestBytes)
        return fail(0, "manifest is too large");

    enum class Stage { Header, Release, Launcher, Files, Done } stage = Stage::Header;
    std::unordered_set<std::string> seen;
    std::size_t lineNumber = 0;
    std::size_t position = 0;
    while (position < text.size()) {
        std::size_t end = text.find('\n', position);
        if (end == std::string_view::npos)
            return fail(lineNumber + 1, "missing final newline");
        std::string_view line = text.substr(position, end - position);
        position = end + 1;
        ++lineNumber;
        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);
        if (stage == Stage::Done)
            return fail(lineNumber, "content after end");

        const std::vector<std::string_view> fields = SplitFields(line);
        for (std::string_view field : fields)
            if (field.empty())
                return fail(lineNumber, "empty field");

        switch (stage) {
        case Stage::Header:
            if (fields.size() != 2 || fields[0] != "WYDPATCH" || fields[1] != "1")
                return fail(lineNumber, "unsupported manifest format");
            stage = Stage::Release;
            break;
        case Stage::Release:
            if (fields.size() != 2 || fields[0] != "release" || !IsReleaseId(fields[1]))
                return fail(lineNumber, "invalid release");
            out.release = std::string(fields[1]);
            stage = Stage::Launcher;
            break;
        case Stage::Launcher:
            if (fields.size() != 3 || fields[0] != "launcher" || !IsSha256Hex(fields[1]) ||
                !ParseSize(fields[2], kMaxFileSize, out.launcherSize) || out.launcherSize == 0)
                return fail(lineNumber, "invalid launcher entry");
            out.launcherSha256 = std::string(fields[1]);
            stage = Stage::Files;
            break;
        case Stage::Files:
            if (fields.size() == 2 && fields[0] == "end") {
                std::uint64_t count = 0;
                if (!ParseSize(fields[1], kMaxFiles, count) || count != out.files.size())
                    return fail(lineNumber, "file count mismatch");
                stage = Stage::Done;
                break;
            }
            if (fields.size() != 5 || fields[0] != "file")
                return fail(lineNumber, "unknown directive");
            if (out.files.size() >= kMaxFiles)
                return fail(lineNumber, "too many files");
            {
                FileEntry entry;
                if (!IsSha256Hex(fields[1]) || !ParseSize(fields[2], kMaxFileSize, entry.size))
                    return fail(lineNumber, "invalid file hash or size");
                if (fields[3] == "replace")
                    entry.install = Install::Replace;
                else if (fields[3] == "keep")
                    entry.install = Install::Keep;
                else
                    return fail(lineNumber, "invalid install mode");
                if (!IsSafeRelativePath(fields[4]) || IsLauncherOwnedPath(fields[4]))
                    return fail(lineNumber, "unsafe path");
                if (FoldPath(fields[4]) == "lang.txt" && entry.install != Install::Keep)
                    return fail(lineNumber, "language preference must be kept");
                if (!seen.insert(FoldPath(fields[4])).second)
                    return fail(lineNumber, "duplicate path");
                entry.sha256 = std::string(fields[1]);
                entry.path = std::string(fields[4]);
                out.files.push_back(std::move(entry));
            }
            break;
        case Stage::Done:
            break;
        }
    }
    if (stage != Stage::Done)
        return fail(lineNumber, "manifest is truncated");
    return true;
}

} // namespace wydpatch
