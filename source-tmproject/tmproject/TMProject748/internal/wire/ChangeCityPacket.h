#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intencao C->S enviada quando o personagem entra ou troca de cidade. O
// servidor continua decidindo o estado persistido; Village apenas identifica
// a regiao calculada pelo client para manter a sessao informada.
constexpr auto MSG_ChangeCity_Opcode = 0x291;
struct MSG_ChangeCity
{
    MSG_STANDARD Header;
    int Village;
};

static_assert(sizeof(MSG_ChangeCity) == 16, "WYD 7.48 change-city packet must be 16 bytes");
static_assert(offsetof(MSG_ChangeCity, Village) == 12, "WYD 7.48 change-city village offset changed");
