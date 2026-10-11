#pragma once

#include <d3d9.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>

namespace font_texture {

// Preserve the existing WYT seed contract: skip its four-byte prefix, retain
// the payload unchanged, then append the existing 18-byte TGA trailer area.
// Publish neither the bytes nor their length after a missing or short read.
inline bool LoadSeed(const char* path, std::unique_ptr<char[]>& destination,
    unsigned int& payloadLength)
{
    if (!path)
        return false;
    std::FILE* file = nullptr;
    if (fopen_s(&file, path, "rb") != 0 || !file)
        return false;
    const std::unique_ptr<std::FILE, decltype(&std::fclose)> owner(file, &std::fclose);
    if (std::fseek(file, 0, SEEK_END) != 0)
        return false;
    const long fileLength = std::ftell(file);
    if (fileLength <= 4 || static_cast<unsigned long>(fileLength - 4) >
        (std::numeric_limits<unsigned int>::max)() - 18u ||
        std::fseek(file, 4, SEEK_SET) != 0)
        return false;
    const auto length = static_cast<unsigned int>(fileLength - 4);
    std::unique_ptr<char[]> bytes(new char[static_cast<std::size_t>(length) + 18]{});
    if (std::fread(bytes.get(), 1, length, file) != length || std::ferror(file))
        return false;
    std::memcpy(bytes.get() + length, "TRUEVISION-XFILE", sizeof("TRUEVISION-XFILE"));
    destination = std::move(bytes);
    payloadLength = length;
    return true;
}

inline bool ValidMaskDimensions(int width, int bitmapHeight, int rows)
{
    return width > 0 && bitmapHeight > 0 && rows > 0 && rows <= bitmapHeight &&
        static_cast<std::size_t>(width) <=
            (std::numeric_limits<std::size_t>::max)() / sizeof(std::uint32_t) /
            static_cast<std::size_t>(rows);
}

// The real texture and the failure-injection test texture use the same API.
// Only successful locks are unlocked, including malformed successful results.
template<typename Texture>
bool UploadMask(Texture* texture, const std::uint32_t* bitmap,
    int width, int bitmapHeight, int rows)
{
    if (!texture || !bitmap || !ValidMaskDimensions(width, bitmapHeight, rows))
        return false;
    D3DSURFACE_DESC description{};
    if (FAILED(texture->GetLevelDesc(0, &description)) ||
        description.Format != D3DFMT_A4R4G4B4 ||
        description.Width < static_cast<unsigned int>(width) ||
        description.Height < static_cast<unsigned int>(rows))
        return false;
    D3DLOCKED_RECT locked{};
    if (FAILED(texture->LockRect(0, &locked, nullptr, 0)))
        return false;
    const auto rowBytes = static_cast<std::size_t>(width) * sizeof(std::uint16_t);
    const auto maxOffset = static_cast<std::size_t>((std::numeric_limits<std::ptrdiff_t>::max)());
    const bool valid = locked.pBits && locked.Pitch > 0 &&
        static_cast<std::size_t>(locked.Pitch) >= rowBytes && rowBytes <= maxOffset &&
        static_cast<std::size_t>(rows - 1) <=
            (maxOffset - rowBytes) / static_cast<std::size_t>(locked.Pitch);
    if (valid) {
        auto* output = static_cast<unsigned char*>(locked.pBits);
        for (int y = 0; y < rows; ++y) {
            const auto sourceRow = static_cast<std::size_t>(y) * width;
            for (int x = 0; x < width; ++x) {
                const auto alpha = (bitmap[sourceRow + x] & 0xFFu) >> 4;
                const auto pixel = static_cast<std::uint16_t>(alpha ? (alpha << 12) | 0xFFFu : 0);
                std::memcpy(output + static_cast<std::size_t>(x) * sizeof(pixel), &pixel, sizeof(pixel));
            }
            if (y + 1 < rows)
                output += locked.Pitch;
        }
    }
    const HRESULT unlocked = texture->UnlockRect(0);
    return valid && SUCCEEDED(unlocked);
}

} // namespace font_texture
