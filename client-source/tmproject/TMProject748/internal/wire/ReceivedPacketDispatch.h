#pragma once

#include "CharacterTransferPacket.h"
#include "SendItemContract.h"
#include "../application/ports/PacketDispatch.h"
#include <cstring>

// Fronteira incremental entre frame de transporte e callbacks legados.
// Somente 0xFAA e 0x182 possuem validacao especifica aqui; outros opcodes continuam
// sujeitos aos seus consumidores. Eventos locais nao passam por esta entrada.
namespace received_packet
{
    // Zero identifica contrato ainda nao migrado; nunca significa frame vazio.
    inline std::size_t ExpectedSize(unsigned int opcode)
    {
        switch (opcode)
        {
        case MSG_ReqTransper_Opcode: return sizeof(MSG_ReqTransper);
        case MSG_SendItem_Opcode: return kSendItemPacketSize;
        default: return 0;
        }
    }

    inline bool CanDispatch(const PacketView& packet)
    {
        if (!packet_dispatch::CanDispatch(packet, sizeof(MSG_STANDARD)))
            return false;

        // A view pode comecar em endereco nao alinhado. Copiar apenas o header
        // para inspecao; nunca escrever nem copiar o payload dos handlers.
        MSG_STANDARD header{};
        std::memcpy(&header, packet.data, sizeof(header));
        const auto expected = ExpectedSize(header.Type);
        if (expected != 0 || ExpectedSize(packet.opcode) != 0)
        {
            // Usar ambos os discriminantes impede que metadados divergentes
            // contornem o guard: a cena antiga decide pelo Type do buffer.
            return packet.opcode == header.Type &&
                header.Size == expected && packet.size == expected;
        }
        return true;
    }

    // true significa entrega unica, nao sucesso da operacao de personagem.
    // O receptor empresta o mesmo buffer e comprimento apenas durante a chamada.
    template <typename Receiver>
    bool Dispatch(const PacketView& packet, Receiver&& receive)
    {
        if (!CanDispatch(packet))
            return false;
        receive(packet);
        return true;
    }
}
