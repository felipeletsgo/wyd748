#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// S->C affect snapshot consumed by the source client. The 7.48 handler reads sixteen eight-byte records.
// Level/Value use the coordinated STRUCT_AFFECT layout compiled here and the WYD-Go encoder.
constexpr auto MSG_UpdateAffect_Opcode = 0x3B9;
constexpr std::size_t kUpdateAffectPacketSize = 140;
constexpr std::size_t kUpdateAffectArrayOffset = 12;
constexpr std::size_t kUpdateAffectCount = 16;
constexpr std::size_t kUpdateAffectEntrySize = 8;
constexpr std::size_t kUpdateAffectTypeOffset = 0;
constexpr std::size_t kUpdateAffectLevelOffset = 1;
constexpr std::size_t kUpdateAffectValueOffset = 2;
constexpr std::size_t kUpdateAffectTimeOffset = 4;

// Optional 0x3B9 tail with owner-only seconds remaining, Asder then Sophia.
using GuardTimers = std::array<std::uint32_t, 2>;
constexpr std::size_t kGuardAffectPacketSize = kUpdateAffectPacketSize + 8;
constexpr std::size_t kOwnerAffectIconCount = kUpdateAffectCount + GuardTimers{}.size();

inline bool ReadGuardTimers(const void* bytes, std::size_t size, GuardTimers& out)
{
    out = {};
    if (!bytes || (size != kUpdateAffectPacketSize && size != kGuardAffectPacketSize))
        return false;
    if (size == kUpdateAffectPacketSize)
        return true;

    const auto* tail = static_cast<const unsigned char*>(bytes) + kUpdateAffectPacketSize;
    GuardTimers timers{};
    for (auto& seconds : timers)
    {
        seconds = std::uint32_t(tail[0]) | (std::uint32_t(tail[1]) << 8)
            | (std::uint32_t(tail[2]) << 16) | (std::uint32_t(tail[3]) << 24);
        if (seconds > 3600)
            return false;
        tail += 4;
    }
    out = timers;
    return true;
}

