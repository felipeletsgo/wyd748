#pragma once

#include "../../application/ports/ITransport.h"

// Adaptador local para CPSock::SendPacket, sem alterar o layout de CPSock.
// Backend permite testar o mesmo encaminhamento sem Win32/socket real.
// Deve oferecer SendPacket(view) com validacao e retorno zero em falha.
template <typename Backend>
class SocketTransport final : public ITransport
{
public:
    // Empresta backend nao nulo; seu dono deve mante-lo vivo ate o fim do uso.
    explicit SocketTransport(Backend& backend) : backend_(backend) {}

    // Encaminha exatamente uma vez, sem copia, retry ou alteracao de opcode.
    // A validacao permanece na fachada do backend, antes do envio legado.
    bool Send(const MutablePacketView& packet) override
    {
        return backend_.SendPacket(packet) != 0;
    }

private:
    Backend& backend_;
};
