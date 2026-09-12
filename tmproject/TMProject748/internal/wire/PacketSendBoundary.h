#pragma once

#include "../application/ports/PacketView.h"
#include <climits>

// Adaptacao local entre tamanho portavel e emissor legado (char*, int).
// Nao conhece sockets, estado global, opcodes ou criptografia; permite testar
// rejeicao, emprestimo e propagacao de resultado com um emissor falso.
template <typename Sender>
bool SendValidatedPacket(const MutablePacketView& packet,
    std::size_t headerSize, Sender&& sender)
{
    if (!packet.HasSizeBetween(headerSize, INT_MAX))
        return false;

    // A chamada e unica e sincrona. O emissor nao pode reter data; alteracoes
    // realizadas por ele permanecem visiveis no buffer original, sem copia.
    // true conserva o significado dado pelo emissor, nao prova entrega remota.
    return sender(packet.data, static_cast<int>(packet.size));
}
