#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Confirmacao C->S da disputa de zona. Os dois DWORDs sao mantidos com os
// nomes wire nativos porque o significado completo da disputa pertence ao
// TMSrv; a source 7.48 usa Parm1 para o alvo e Parm2=0 neste caminho.
constexpr auto MSG_ChallengeConfirm_Opcode = 0x28F;
struct MSG_ChallengeConfirm
{
    MSG_STANDARD Header;
    int Parm1;
    int Parm2;
};

static_assert(sizeof(MSG_ChallengeConfirm) == 20, "WYD 7.48 challenge confirmation must be 20 bytes");
static_assert(offsetof(MSG_ChallengeConfirm, Parm1) == 12, "WYD 7.48 challenge Parm1 offset changed");
static_assert(offsetof(MSG_ChallengeConfirm, Parm2) == 16, "WYD 7.48 challenge Parm2 offset changed");
