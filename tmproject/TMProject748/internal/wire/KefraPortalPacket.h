#pragma once

#include "MessageHeader.h"
#include <cstdint>
#include <cstring>

// EXTENSAO_COORDENADA v1; DOCS/protocol/kefra-portal.md.
namespace kefra_portal
{
    constexpr unsigned short StateOpcode = 0x7F30;
    constexpr unsigned short FieldSceneID = 0x7530;
    constexpr std::uint16_t Version = 1;

    struct State
    {
        MSG_STANDARD Header;
        std::uint16_t ContractVersion;
        std::uint16_t Defeated;
    };
    static_assert(sizeof(State) == 16 && offsetof(State, ContractVersion) == 12 &&
        offsetof(State, Defeated) == 14, "Kefra portal state v1 ABI");

    inline bool Parse(const void* bytes, std::size_t size, State& out)
    {
        if (!bytes || size != sizeof(State))
            return false;
        State packet{};
        std::memcpy(&packet, bytes, sizeof(packet));
        if (packet.Header.Size != sizeof(State) || packet.Header.Type != StateOpcode ||
            packet.Header.ID != FieldSceneID || packet.ContractVersion != Version || packet.Defeated > 1)
            return false;
        out = packet;
        return true;
    }

    constexpr bool IsHallPortal(unsigned int objectType, float x, float y)
    {
        return objectType == 2035 && x == 2366.5f && y == 3926.5f;
    }

    class ClientState
    {
    public:
        void Reset() { defeated_ = false; }
        bool IsOpen() const { return defeated_; }
        bool Apply(const void* bytes, std::size_t size)
        {
            State packet{};
            if (!Parse(bytes, size, packet))
                return false;
            defeated_ = packet.Defeated == 1;
            return true;
        }
    private:
        bool defeated_ = false;
    };
}
