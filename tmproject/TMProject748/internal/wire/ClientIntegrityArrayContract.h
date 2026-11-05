#pragma once

#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>

// The S->C probe and C->S response share a 24-byte envelope.
// Value is int32: the handler reads a signed char and sign-extends it.
constexpr auto MSG_REQArray_Opcode = 0x1C1;
constexpr auto MSG_CNFArray_Opcode = 0x2C2;

struct MSG_REQArray
{
	MSG_STANDARD Header;
	std::int32_t Category;
	std::int32_t ByteOffset;
	std::int32_t Value;
};

static_assert(sizeof(MSG_REQArray) == 24, "WYD 7.48 array probe must be 24 bytes");
static_assert(offsetof(MSG_REQArray, Category) == 12, "array category offset changed");
static_assert(offsetof(MSG_REQArray, ByteOffset) == 16, "array byte offset changed");
static_assert(offsetof(MSG_REQArray, Value) == 20, "array value offset changed");
