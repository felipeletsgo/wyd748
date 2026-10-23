#include "../src/PatchCache.h"
#include "../src/PatchManifest.h"

#include <cstdio>
#include <string>

using namespace wydpatch;

namespace {

int g_checks = 0;
int g_failures = 0;

void Check(bool condition, const char* name)
{
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("FAIL launcher: %s\n", name);
    }
}

const std::string kHashA(64, 'a');
const std::string kHashB = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

std::string Valid()
{
    return "WYDPATCH 1\n"
           "release 20261011.1\n"
           "launcher " + kHashA + " 1024\n"
           "file " + kHashB + " 7040 replace serverlist.bin\n"
           "file " + kHashA + " 0 keep config.txt\n"
           "file " + kHashB + " 12 replace mesh/ch01.bon\n"
           "end 3\n";
}

bool Rejects(const std::string& text)
{
    Manifest manifest;
    std::string error;
    const bool parsed = ParseManifest(text, manifest, error);
    return !parsed && !error.empty() && manifest.files.empty() && manifest.release.empty();
}

std::string Replace(std::string text, const std::string& from, const std::string& to)
{
    const std::size_t at = text.find(from);
    if (at != std::string::npos)
        text.replace(at, from.size(), to);
    return text;
}

void ManifestTests()
{
    Manifest manifest;
    std::string error;
    Check(ParseManifest(Valid(), manifest, error), "a valid manifest parses");
    Check(manifest.release == "20261011.1" && manifest.launcherSha256 == kHashA && manifest.launcherSize == 1024,
        "release and launcher fields are read");
    Check(manifest.files.size() == 3 && manifest.files[0].path == "serverlist.bin" &&
            manifest.files[0].size == 7040 && manifest.files[0].install == Install::Replace &&
            manifest.files[1].install == Install::Keep && manifest.files[1].size == 0 &&
            manifest.files[2].path == "mesh/ch01.bon",
        "file entries keep order, sizes and install modes");

    std::string crlf;
    for (char c : Valid()) {
        if (c == '\n')
            crlf += '\r';
        crlf += c;
    }
    Check(ParseManifest(crlf, manifest, error) && manifest.files.size() == 3, "CRLF line endings are accepted");

    Check(Rejects(""), "an empty body is rejected");
    Check(Rejects(Replace(Valid(), "WYDPATCH 1", "WYDPATCH 2")), "an unknown format is rejected");
    Check(Rejects(Replace(Valid(), "end 3\n", "")), "a truncated manifest is rejected");
    Check(Rejects(Replace(Valid(), "end 3\n", "end 3")), "a missing final newline is rejected");
    Check(Rejects(Replace(Valid(), "end 3", "end 2")), "a count mismatch is rejected");
    Check(Rejects(Valid() + "file " + kHashB + " 1 replace extra.bin\n"), "content after end is rejected");
    Check(Rejects(Replace(Valid(), "release 20261011.1", "release a/b")), "an unsafe release ID is rejected");
    Check(Rejects(Replace(Valid(), "launcher " + kHashA + " 1024", "launcher " + kHashA + " 0")),
        "an empty launcher is rejected");
    Check(Rejects(Replace(Valid(), kHashB + " 7040", std::string(64, 'A') + " 7040")),
        "uppercase hashes are rejected");
    Check(Rejects(Replace(Valid(), kHashB + " 7040", kHashB.substr(1) + " 7040")), "short hashes are rejected");
    Check(Rejects(Replace(Valid(), " 7040 ", " 07040 ")), "leading zeros are rejected");
    Check(Rejects(Replace(Valid(), " 7040 ", " 99999999999999999999 ")), "oversized files are rejected");
    Check(Rejects(Replace(Valid(), " 7040 ", " -1 ")), "negative sizes are rejected");
    Check(Rejects(Replace(Valid(), "replace serverlist.bin", "overwrite serverlist.bin")),
        "unknown install modes are rejected");
    Check(Rejects(Replace(Valid(), "replace serverlist.bin", "replace  serverlist.bin")),
        "empty fields are rejected");
    Check(Rejects(Replace(Valid(), "mesh/ch01.bon", "MESH/CH01.BON\nfile " + kHashB + " 1 replace mesh/ch01.bon")),
        "case-insensitive duplicate paths are rejected");
    Check(Rejects(Replace(Valid(), "serverlist.bin", "WYDLauncher.exe")), "the launcher path is reserved");
    Check(Rejects(Replace(Valid(), "serverlist.bin", "patch.cache")), "the cache path is reserved");
    Check(Rejects(Replace(Valid(), "serverlist.bin", "LaNg.TxT")), "patches cannot replace the language preference");
    Check(ParseManifest(Replace(Valid(), "config.txt", "lang.txt"), manifest, error),
        "a language preference can be declared as keep");
    Check(Rejects(Replace(Valid(), "serverlist.bin", "data.wydnew")), "partial-download names are reserved");
    Check(Rejects(Replace(Valid(), "serverlist.bin", "../serverlist.bin")), "parent traversal is rejected");
}

void PathTests()
{
    Check(IsSafeRelativePath("WYD.exe") && IsSafeRelativePath("UI/ServerList2.wyt") &&
            IsSafeRelativePath("mesh/a_b-c.1.msh"),
        "ordinary client paths are accepted");
    for (const char* unsafe : {"", "/abs.bin", "a//b", "a/", "./a", "a/./b", "a/../b", "..", "C:/x", "C:x",
             "a\\b", "a b", "a.", "dir./x", "CON", "con.txt", "mesh/Nul.msh", "COM1", "lpt9.dat", "a%20b",
             "\xC3\xA9.bin"})
        Check(!IsSafeRelativePath(unsafe), unsafe[0] ? unsafe : "empty path");
    Check(IsSafeRelativePath("COM10") && IsSafeRelativePath("console.bin"),
        "names that only resemble devices are accepted");
    Check(!IsSafeRelativePath(std::string(kMaxPathLength + 1, 'a')), "overlong paths are rejected");
    Check(IsLauncherOwnedPath("wydlauncher.EXE") && IsLauncherOwnedPath("TMPWYDLauncher.exe") &&
            IsLauncherOwnedPath("WYDLauncher.old") && IsLauncherOwnedPath("mesh/x.WYDNEW") &&
            !IsLauncherOwnedPath("WYD.exe"),
        "launcher-owned names are recognized case-insensitively");
}

void CacheTests()
{
    const std::string text = SerializeCacheLine("mesh/ch01.bon", CacheEntry{kHashB, 12, 133000000000000000ull}) +
        "garbage line\n" + kHashA + " 5 6 ../escape.bin\n" + kHashA + " 1 2 config.txt";
    const PatchCache cache = ParseCache(text);
    Check(cache.size() == 2, "malformed and unsafe cache lines are skipped");
    Check(CacheMatches(cache, "MESH/ch01.bon", 12, 133000000000000000ull, kHashB),
        "cache lookups fold case and round-trip large timestamps");
    Check(!CacheMatches(cache, "mesh/ch01.bon", 12, 133000000000000001ull, kHashB),
        "a changed timestamp forces rehashing");
    Check(!CacheMatches(cache, "mesh/ch01.bon", 13, 133000000000000000ull, kHashB), "a changed size forces rehashing");
    Check(!CacheMatches(cache, "mesh/ch01.bon", 12, 133000000000000000ull, kHashA),
        "a new published hash forces rehashing");
    Check(CacheMatches(cache, "config.txt", 1, 2, kHashA), "the last line needs no newline");
    Check(ParseCache("").empty(), "an empty cache is valid");
}

} // namespace

int main()
{
    ManifestTests();
    PathTests();
    CacheTests();
    std::printf("Launcher tests: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
