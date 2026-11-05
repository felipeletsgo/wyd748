#pragma once

#include "MessageHeader.h"
#include <cstddef>

// S->C billing notice. The native handler marks state and displays a message.
// The four bytes after the header remain opaque.
constexpr auto MSG_BillingNotice_Opcode = 0x194;
struct MSG_BillingNotice
{
	MSG_STANDARD Header;
	unsigned char OpaquePayload[4];
};

static_assert(sizeof(MSG_BillingNotice) == 16, "WYD 7.48 billing notice must be 16 bytes");
static_assert(offsetof(MSG_BillingNotice, OpaquePayload) == 12, "Billing payload offset changed");
