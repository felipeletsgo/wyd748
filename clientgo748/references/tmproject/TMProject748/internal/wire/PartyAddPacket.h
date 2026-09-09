#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Snapshot de um membro Party usado pelo client 7.48 nos frames 0x37D e
// 0x37F. O WORD final mantém o alinhamento e os dois bytes reservados nativos.
struct PARTY
{
	char Class;
	char PartyIndex;
	short Level;
	short MaxHp;
	short Hp;
	unsigned short ID;
	char Name[16];
	short Reserved;
};

constexpr auto MSG_AddParty_Opcode = 0x37D;
struct MSG_AddParty
{
	MSG_STANDARD Header;
	PARTY Party;
};

static_assert(sizeof(PARTY) == 28, "WYD 7.48 party member must be 28 bytes");
static_assert(offsetof(PARTY, ID) == 8, "WYD 7.48 party member ID offset changed");
static_assert(offsetof(PARTY, Name) == 10, "WYD 7.48 party member name offset changed");
static_assert(offsetof(PARTY, Reserved) == 26, "WYD 7.48 party reserved offset changed");
static_assert(sizeof(MSG_AddParty) == 40, "WYD 7.48 party-member packet must be 40 bytes");
static_assert(offsetof(MSG_AddParty, Party) == 12, "WYD 7.48 party payload offset changed");
