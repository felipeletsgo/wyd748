#include "../internal/platform/windows/FontTextureUpload.h"
#include <array>
#include <climits>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {
struct TextureFixture {
    std::array<unsigned char, 656> bytes{};
    D3DSURFACE_DESC description{};
    D3DLOCKED_RECT locked{};
    HRESULT describeResult = S_OK;
    HRESULT lockResult = S_OK;
    HRESULT unlockResult = S_OK;
    int descriptions = 0;
    int locks = 0;
    int unlocks = 0;
    TextureFixture() {
        bytes.fill(0xA5);
        description.Format = D3DFMT_A4R4G4B4;
        description.Width = 16;
        description.Height = 16;
        locked.Pitch = 40;
        locked.pBits = bytes.data() + 8;
    }
    HRESULT GetLevelDesc(UINT, D3DSURFACE_DESC* output) {
        ++descriptions;
        *output = description;
        return describeResult;
    }
    HRESULT LockRect(UINT, D3DLOCKED_RECT* output, const RECT*, DWORD) {
        ++locks;
        *output = locked;
        return lockResult;
    }
    HRESULT UnlockRect(UINT) { ++unlocks; return unlockResult; }
};

// The file is created in the system temporary directory and owned by this test.
struct SeedFixture {
    char path[MAX_PATH]{};
    SeedFixture() {
        char directory[MAX_PATH]{};
        const DWORD length = GetTempPathA(MAX_PATH, directory);
        if (!length || length >= MAX_PATH || !GetTempFileNameA(directory, "wft", 0, path))
            path[0] = '\0';
    }
    ~SeedFixture() { if (path[0]) std::remove(path); }
    bool Write(const std::vector<char>& bytes) const {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        file.close();
        return !file.fail();
    }
};

std::filesystem::path FindSeed() {
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH) return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto path = current / "client748" / "UI" / "minimap.wyt";
        if (std::filesystem::is_regular_file(path)) return path;
        current = current.parent_path();
    }
    return {};
}
}

int RunFontTextureUploadTests(int& checks) {
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL font texture: %s\n", name);
        }
    };
    std::array<std::uint32_t, 256> bitmap{};
    for (std::size_t i = 0; i < bitmap.size(); ++i)
        bitmap[i] = 0xABCD0000u | static_cast<std::uint32_t>(i);
    TextureFixture texture;
    check(font_texture::UploadMask(&texture, bitmap.data(), 16, 16, 16),
        "valid pitched texture upload succeeds");
    check(texture.descriptions == 1 && texture.locks == 1 && texture.unlocks == 1,
        "successful upload describes, locks and unlocks exactly once");
    for (std::size_t i = 0; i < bitmap.size(); ++i) {
        std::uint16_t pixel = 0;
        std::memcpy(&pixel, texture.bytes.data() + 8 + (i / 16) * 40 + (i % 16) * 2, 2);
        const auto alpha = i >> 4;
        check(pixel == (alpha ? (alpha << 12) | 0xFFFu : 0),
            "all 256 blue intensities preserve the legacy alpha conversion");
    }
    for (std::size_t i = 0; i < texture.bytes.size(); ++i) {
        const bool written = i >= 8 && i < 648 && (i - 8) % 40 < 32;
        if (!written) check(texture.bytes[i] == 0xA5, "upload preserves row padding and guards");
    }
    check(!font_texture::UploadMask<TextureFixture>(nullptr, bitmap.data(), 16, 16, 16),
        "null texture is rejected");
    TextureFixture untouched;
    check(!font_texture::UploadMask(&untouched, nullptr, 16, 16, 16), "null bitmap is rejected");
    for (const auto dimensions : {std::array<int, 3>{0, 16, 1}, {-1, 16, 1},
        {16, 0, 1}, {16, -1, 1}, {16, 16, 0}, {16, 16, -1}, {16, 16, 17},
        {INT_MAX, INT_MAX, INT_MAX}}) {
        check(!font_texture::UploadMask(&untouched, bitmap.data(), dimensions[0], dimensions[1], dimensions[2]),
            "invalid or overflowing mask extent is rejected before texture access");
    }
    check(untouched.descriptions == 0 && untouched.locks == 0 && untouched.unlocks == 0,
        "invalid inputs make no texture calls");
    for (int failure = 0; failure < 4; ++failure) {
        TextureFixture rejected;
        if (failure == 0) rejected.describeResult = E_FAIL;
        if (failure == 1) rejected.description.Format = D3DFMT_A8R8G8B8;
        if (failure == 2) rejected.description.Width = 15;
        if (failure == 3) rejected.description.Height = 15;
        check(!font_texture::UploadMask(&rejected, bitmap.data(), 16, 16, 16) &&
            rejected.locks == 0 && rejected.unlocks == 0,
            "failed or incompatible texture description is rejected before locking");
    }
    TextureFixture lockFailure;
    lockFailure.lockResult = E_FAIL;
    const auto pristine = lockFailure.bytes;
    check(!font_texture::UploadMask(&lockFailure, bitmap.data(), 16, 16, 16) &&
        lockFailure.unlocks == 0 && lockFailure.bytes == pristine,
        "failed lock is neither written nor unlocked even when it supplies bits");
    for (int failure = 0; failure < 4; ++failure) {
        TextureFixture malformed;
        if (failure == 0) malformed.locked.pBits = nullptr;
        if (failure == 1) malformed.locked.Pitch = 0;
        if (failure == 2) malformed.locked.Pitch = -40;
        if (failure == 3) malformed.locked.Pitch = 31;
        check(!font_texture::UploadMask(&malformed, bitmap.data(), 16, 16, 16) &&
            malformed.unlocks == 1 && malformed.bytes == pristine,
            "malformed successful lock is unwound once without writing");
    }
    if (sizeof(std::ptrdiff_t) == 4) {
        TextureFixture overflow;
        overflow.locked.Pitch = INT_MAX;
        check(!font_texture::UploadMask(&overflow, bitmap.data(), 16, 16, 3) &&
            overflow.unlocks == 1 && overflow.bytes == pristine,
            "32-bit pitched extent overflow is rejected without writing");
    }
    TextureFixture unlockFailure;
    unlockFailure.unlockResult = E_FAIL;
    check(!font_texture::UploadMask(&unlockFailure, bitmap.data(), 16, 16, 16) &&
        unlockFailure.unlocks == 1, "unlock failure propagates");
    unlockFailure.unlockResult = S_OK;
    check(font_texture::UploadMask(&unlockFailure, bitmap.data(), 16, 16, 16) &&
        unlockFailure.locks == 2 && unlockFailure.unlocks == 2, "a later upload can succeed after failure");
    TextureFixture oneRow;
    check(font_texture::UploadMask(&oneRow, bitmap.data(), 16, 16, 1) &&
        oneRow.bytes[48] == 0xA5, "upload writes only the requested mask rows");

    SeedFixture fixture;
    check(fixture.path[0] != '\0', "seed fixture is available");
    if (fixture.path[0]) {
        std::unique_ptr<char[]> seed(new char[1]{'K'});
        char* old = seed.get();
        unsigned int length = 123;
        check(!font_texture::LoadSeed(nullptr, seed, length) && seed.get() == old && length == 123,
            "null seed path retains prior state");
        const auto missing = std::string(fixture.path) + ".missing";
        check(!font_texture::LoadSeed(missing.c_str(), seed, length) && seed.get() == old && length == 123,
            "missing seed retains prior state");
        for (std::size_t size = 0; size <= 4; ++size) {
            check(fixture.Write(std::vector<char>(size, 'P')), "short seed fixture writes successfully");
            check(!font_texture::LoadSeed(fixture.path, seed, length) && seed.get() == old &&
                length == 123 && seed[0] == 'K', "prefix-only or truncated seed retains prior state");
        }
        const std::vector<char> input{'W', 'Y', 'T', '0', 'A', '\0', 'B', static_cast<char>(0xFF)};
        check(fixture.Write(input) && font_texture::LoadSeed(fixture.path, seed, length),
            "complete seed payload is loaded");
        check(length == 4 && std::memcmp(seed.get(), input.data() + 4, 4) == 0,
            "seed skips exactly four prefix bytes and preserves binary payload");
        check(length == 4 && std::memcmp(seed.get() + length, "TRUEVISION-XFILE", 16) == 0 &&
            seed[length + 16] == 0 && seed[length + 17] == 0,
            "existing trailer area is terminated and fully initialized");
    }
    const auto runtime = FindSeed();
    check(!runtime.empty(), "real runtime seed is available; missing fixtures do not pass");
    if (!runtime.empty()) {
        std::ifstream file(runtime, std::ios::binary);
        const std::vector<char> raw((std::istreambuf_iterator<char>(file)), {});
        std::unique_ptr<char[]> seed;
        unsigned int length = 0;
        const bool loaded = font_texture::LoadSeed(runtime.string().c_str(), seed, length);
        check(loaded && raw.size() > 4 && length == raw.size() - 4,
            "real runtime seed length matches its complete payload");
        check(loaded && raw.size() > 4 && length == raw.size() - 4 &&
            std::memcmp(seed.get(), raw.data() + 4, length) == 0,
            "real runtime seed payload remains byte-identical");
    }
    return failures;
}
