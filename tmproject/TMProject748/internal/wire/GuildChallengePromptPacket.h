#pragma once

#include "MessageHeader.h"

// S->C notice opens the native city-challenge confirmation. The 7.48 client receives only MSG_STANDARD.
// Target, city and fee remain in server context and the subsequent C->S confirmation.
constexpr auto MSG_ReqChallenge_Opcode = 0x18D;

static_assert(sizeof(MSG_STANDARD) == 12, "Guild challenge prompt must be header-only");
