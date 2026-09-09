#pragma once

#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>

// Probe S->C e resposta C->S usam o mesmo envelope de 24 bytes. O valor e
// int32: o handler le o byte como signed char e o promove com sinal.
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
