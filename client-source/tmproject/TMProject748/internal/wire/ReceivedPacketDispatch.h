#pragma once

#include "CharacterTransferPacket.h"
#include "SendItemContract.h"
#include "MessagePanelPacket.h"
#include "ServerMigrationPacket.h"
#include "ChatMessagePacket.h"
#include "WhisperMessagePacket.h"
#include "CharacterLogoutConfirmPacket.h"
#include "CharacterLoginConfirmContract.h"
#include "ClientIntegrityArrayContract.h"
#include "MobKillConfirmPacket.h"
#include "UpdateEtcPacket.h"
#include "DelayStartPacket.h"
#include "BillingNoticePacket.h"
#include "PartyAddPacket.h"
#include "IndexedMessageContract.h"
#include "LegacySceneMessagePacket.h"
#include "../application/ports/PacketDispatch.h"
#include <cstring>

// Fronteira incremental entre frame de transporte e callbacks legados.
// ExpectedSize enumera os contratos migrados; outros opcodes continuam
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
        case MSG_MessagePanel_Opcode: return sizeof(MSG_MessagePanel);
        case MSG_LegacySceneMessage102_Opcode: return sizeof(MSG_LegacySceneMessage102);
        case MSG_LegacySceneMessage104_Opcode: return sizeof(MSG_LegacySceneMessage104);
        case MSG_MessageChat_Opcode: return sizeof(MSG_MessageChat);
        case MSG_MessageIndexed_Opcode:
        case MSG_MessageParameterized_Opcode: return kIndexedMessagePacketSize;
        case MSG_CNFRemoveServer_Opcode: return sizeof(MSG_CNFRemoveServer);
        case MSG_MessageWhisper_Opcode: return sizeof(MSG_MessageWhisper);
        case MSG_CNFCharacterLogout_Opcode: return sizeof(MSG_CNFCharacterLogout);
        case MSG_CNFCharacterLogin_Opcode: return kCharacterLoginConfirmPacketSize;
        case MSG_REQArray_Opcode: return sizeof(MSG_REQArray);
        case MSG_CNFMobKill_Opcode: return sizeof(MSG_CNFMobKill);
        case MSG_UpdateEtc_Opcode: return sizeof(MSG_UpdateEtc);
        case MSG_DelayStart_Opcode: return sizeof(MSG_DelayStart);
        case MSG_BillingNotice_Opcode: return sizeof(MSG_BillingNotice);
        case MSG_AddParty_Opcode: return sizeof(MSG_AddParty);
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
