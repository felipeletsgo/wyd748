#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intent C->S de uso de item. Posicoes e ItemID sao revalidados no servidor;
// o client pode atualizar a UI otimisticamente e receber SendItem depois.
constexpr auto MSG_UseItem_Opcode = 0x373;
struct MSG_UseItem
{
	MSG_STANDARD Header;
	int SourType;
	int SourPos;
	int DestType;
	int DestPos;
	unsigned short GridX;
	unsigned short GridY;
	unsigned short ItemID;
};

static_assert(sizeof(MSG_UseItem) == 36, "WYD 7.48 UseItem must be 36 bytes");
static_assert(offsetof(MSG_UseItem, SourType) == 12, "UseItem source type offset changed");
static_assert(offsetof(MSG_UseItem, SourPos) == 16, "UseItem source position offset changed");
static_assert(offsetof(MSG_UseItem, DestType) == 20, "UseItem destination type offset changed");
static_assert(offsetof(MSG_UseItem, DestPos) == 24, "UseItem destination position offset changed");
static_assert(offsetof(MSG_UseItem, GridX) == 28, "UseItem grid X offset changed");
static_assert(offsetof(MSG_UseItem, GridY) == 30, "UseItem grid Y offset changed");
static_assert(offsetof(MSG_UseItem, ItemID) == 32, "UseItem item ID offset changed");
