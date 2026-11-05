#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S intent on city entry or change. The server owns persisted state.
// Village identifies the client-computed region for session notification.
constexpr auto MSG_ChangeCity_Opcode = 0x291;
struct MSG_ChangeCity
{
    MSG_STANDARD Header;
    int Village;
};

static_assert(sizeof(MSG_ChangeCity) == 16, "WYD 7.48 change-city packet must be 16 bytes");
static_assert(offsetof(MSG_ChangeCity, Village) == 12, "WYD 7.48 change-city village offset changed");
