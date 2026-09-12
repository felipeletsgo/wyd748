#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intencao C->S para acionar o portal sob o personagem. O frame nao carrega
// destino nem preco: o servidor resolve a rota e cobra de forma autoritativa.
constexpr auto MSG_ReqTeleport_Opcode = 0x290;
struct MSG_ReqTeleport
{
    MSG_STANDARD Header;
    int Reserved;
};

static_assert(sizeof(MSG_ReqTeleport) == 16, "WYD 7.48 teleport request must be 16 bytes");
static_assert(offsetof(MSG_ReqTeleport, Reserved) == 12, "WYD 7.48 teleport reserved offset changed");
