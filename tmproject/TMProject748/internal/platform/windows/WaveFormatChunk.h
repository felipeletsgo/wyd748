#pragma once
#include <cstddef>
#include <cstdint>

namespace wave_format {
constexpr std::uint16_t PcmTag = 1;
constexpr std::uint32_t BaseBytes = 16;   // PCMWAVEFORMAT
constexpr std::uint32_t HeaderBytes = 18; // WAVEFORMATEX including cbSize

// Non-PCM formats store cbSize after the 16-byte base only when the chunk has room.
inline bool HasExtraSize(std::uint16_t tag, std::uint32_t chunkBytes)
{
    return tag != PcmTag && chunkBytes >= HeaderBytes;
}

// Bytes to allocate for the format structure and its extra bytes, or zero when
// the chunk cannot hold them. PCM ignores cbSize.
inline std::size_t AllocationBytes(std::uint16_t tag, std::uint32_t chunkBytes, std::uint16_t extraBytes)
{
    if (chunkBytes < BaseBytes) return 0;
    if (!HasExtraSize(tag, chunkBytes)) return extraBytes == 0 ? HeaderBytes : 0;
    if (extraBytes > chunkBytes - HeaderBytes) return 0;
    return static_cast<std::size_t>(HeaderBytes) + extraBytes;
}
}
