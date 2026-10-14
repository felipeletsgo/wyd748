#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Native FUN_0049889A reads a single text body at +12 and terminates +106/+107.
constexpr auto MSG_MessageShout_Opcode = 0xD1D;
struct MSG_MessageShout
{
    MSG_STANDARD Header;
    char String[96];
};
constexpr unsigned int kNativeShoutARGB = 0xFF00CD00;
static_assert(sizeof(MSG_MessageShout) == 108, "WYD 7.48 shout must be 108 bytes");
static_assert(offsetof(MSG_MessageShout, String) == 12, "WYD 7.48 shout text offset changed");
