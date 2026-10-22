#pragma once
#include "MessageHeader.h"
#include <cstdint>
#include <cstring>

// EXTENSAO_COORDENADA v1: display only; answers remain authoritative positions.
namespace instance_question
{
    constexpr unsigned short Opcode = 0x7F12;
    struct Packet
    {
        MSG_STANDARD Header;
        std::uint16_t Version, Reserved;
        std::uint32_t DurationMs;
        char Question[96];
    };
    static_assert(sizeof(Packet) == 116 && offsetof(Packet, Question) == 20,
        "Instance question ABI");

    inline bool Parse(const void* bytes, std::size_t size, Packet& out)
    {
        if (!bytes || size != sizeof(Packet)) return false;
        Packet p{};
        std::memcpy(&p, bytes, sizeof(p));
        if (p.Header.Size != sizeof(p) || p.Header.Type != Opcode ||
            p.Version != 1 || p.Reserved != 0 || p.DurationMs > 10000 ||
            !std::memchr(p.Question, 0, sizeof(p.Question)) ||
            (p.DurationMs != 0 && !p.Question[0])) return false;
        out = p;
        return true;
    }

    struct State
    {
        std::uint32_t OpenedAt = 0, DurationMs = 0;
        void Apply(const Packet& p, std::uint32_t now)
        { OpenedAt = now; DurationMs = p.DurationMs; }
        void Clear() { DurationMs = 0; }
        bool Visible(std::uint32_t now) const
        { return DurationMs != 0 && std::uint32_t(now - OpenedAt) < DurationMs; }
    };
}
