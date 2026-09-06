#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intencao C->S para interagir com um NPC. TargetID identifica o NPC e
// ClickOk distingue o primeiro clique da confirmacao da caixa nativa.
constexpr auto MSG_UseNPC_Opcode = 0x28B;
struct MSG_UseNPC
{
    MSG_STANDARD Header;
    int TargetID;
    int ClickOk;
};

static_assert(sizeof(MSG_UseNPC) == 20, "WYD 7.48 NPC request must be 20 bytes");
static_assert(offsetof(MSG_UseNPC, TargetID) == 12, "WYD 7.48 NPC target offset changed");
static_assert(offsetof(MSG_UseNPC, ClickOk) == 16, "WYD 7.48 NPC confirmation offset changed");
