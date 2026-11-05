#pragma once

#include "../application/ports/PacketView.h"
#include <climits>

// Local adapter between portable size and the legacy sender (char*, int).
// No socket, global state, opcode or cipher dependency. A fake sender can test rejection, borrowing and result propagation.
template <typename Sender>
bool SendValidatedPacket(const MutablePacketView& packet,
    std::size_t headerSize, Sender&& sender)
{
    if (!packet.HasSizeBetween(headerSize, INT_MAX))
        return false;

    // Call once, synchronously. The sender cannot retain data. Changes remain in the original buffer without copying.
    // True preserves the sender result; it does not prove remote delivery.
    return sender(packet.data, static_cast<int>(packet.size));
}
