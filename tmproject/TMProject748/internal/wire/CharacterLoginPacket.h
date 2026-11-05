#pragma once

#include "MessageHeader.h"

// Existing C->S request. Only the server decides admission for the slot.
// Force and SecretCode remain present and zero in the current scene flow.
constexpr auto MSG_CharacterLogin_Opcode = 0x213;
struct MSG_CharacterLogin
{
    MSG_STANDARD Header;
    int Slot;
    // Force ocupa os quatro bytes entre Slot e a prova de sessao; omiti-lo
    // reduziria o packet a 32 bytes e deslocaria SecretCode.
    int Force;
    char SecretCode[16];
};

static_assert(sizeof(MSG_CharacterLogin) == 36, "Character login size changed");
static_assert(offsetof(MSG_CharacterLogin, Header) == 0, "Header offset changed");
static_assert(offsetof(MSG_CharacterLogin, Slot) == 12, "Slot offset changed");
static_assert(offsetof(MSG_CharacterLogin, Force) == 16, "Force offset changed");
static_assert(offsetof(MSG_CharacterLogin, SecretCode) == 20, "SecretCode offset changed");
