#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S zone-challenge confirmation. Keep the native DWORD names; TMSrv owns the full challenge meaning.
// This 7.48 path sends the target in Parm1 and zero in Parm2.
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
