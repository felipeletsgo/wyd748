#pragma once

#include "PacketView.h"

namespace packet_dispatch
{
    // Dispatch only frames with complete storage and envelopes.
    inline bool CanDispatch(const PacketView& packet, std::size_t minimumSize)
    {
        return packet.HasAtLeast(minimumSize);
    }

    // Deliver once, synchronously, without copying, retention or retry. Pass the original size.
    // True means delivery to the handler, not semantic acceptance. The caller keeps storage alive throughout the callback.
    template <typename Receiver>
    bool Dispatch(const PacketView& packet, std::size_t minimumSize, Receiver&& receive)
    {
        if (!CanDispatch(packet, minimumSize))
            return false;
        receive(packet);
        return true;
    }
}
