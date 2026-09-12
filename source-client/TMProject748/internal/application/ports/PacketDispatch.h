#pragma once

#include "PacketView.h"

namespace packet_dispatch
{
    // O dispatcher so entrega frames com armazenamento e envelope completos.
    inline bool CanDispatch(const PacketView& packet, std::size_t minimumSize)
    {
        return packet.HasAtLeast(minimumSize);
    }

    // Entrega sincrona e unica, sem copia, retencao ou retry. O receptor recebe
    // o tamanho original; true indica entrega, nao aceite semantico do handler.
    // O chamador garante armazenamento vivo durante todo o callback.
    template <typename Receiver>
    bool Dispatch(const PacketView& packet, std::size_t minimumSize, Receiver&& receive)
    {
        if (!CanDispatch(packet, minimumSize))
            return false;
        receive(packet);
        return true;
    }
}
