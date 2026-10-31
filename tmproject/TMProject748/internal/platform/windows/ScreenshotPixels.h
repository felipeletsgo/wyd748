#pragma once
#include <d3d9.h>
#include <cstdint>

// Back-buffer rows to 24-bit BGR rows for the screenshot JPEG encoder.
// The static D3DX9 JPEG writer swaps width and height, so the client encodes
// the pixels itself through WIC.
namespace screenshot_pixels {
enum class Layout { Unsupported, Xrgb8888, Rgb565, Xrgb1555 };

constexpr Layout FromFormat(D3DFORMAT format)
{
    switch (format) {
    case D3DFMT_X8R8G8B8:
    case D3DFMT_A8R8G8B8: return Layout::Xrgb8888;
    case D3DFMT_R5G6B5: return Layout::Rgb565;
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5: return Layout::Xrgb1555;
    default: return Layout::Unsupported;
    }
}

constexpr std::uint8_t Expand5(unsigned value) { return static_cast<std::uint8_t>((value << 3) | (value >> 2)); }
constexpr std::uint8_t Expand6(unsigned value) { return static_cast<std::uint8_t>((value << 2) | (value >> 4)); }

// Writes width BGR triplets. Returns false for an unsupported layout.
inline bool RowToBgr24(Layout layout, const std::uint8_t* source, std::uint32_t width, std::uint8_t* destination)
{
    if (!source || !destination) return false;
    for (std::uint32_t x = 0; x < width; ++x, destination += 3) {
        switch (layout) {
        case Layout::Xrgb8888:
            destination[0] = source[x * 4];
            destination[1] = source[x * 4 + 1];
            destination[2] = source[x * 4 + 2];
            break;
        case Layout::Rgb565: {
            const unsigned value = source[x * 2] | (source[x * 2 + 1] << 8);
            destination[0] = Expand5(value & 0x1F);
            destination[1] = Expand6((value >> 5) & 0x3F);
            destination[2] = Expand5((value >> 11) & 0x1F);
            break;
        }
        case Layout::Xrgb1555: {
            const unsigned value = source[x * 2] | (source[x * 2 + 1] << 8);
            destination[0] = Expand5(value & 0x1F);
            destination[1] = Expand5((value >> 5) & 0x1F);
            destination[2] = Expand5((value >> 10) & 0x1F);
            break;
        }
        default:
            return false;
        }
    }
    return true;
}
}
