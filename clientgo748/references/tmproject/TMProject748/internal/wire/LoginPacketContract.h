#pragma once

#include <cstddef>
#include "Basedef.h"

// Identifies only the login packets shared by the selection scenes. Keeping
// this classification independent of scene state makes it safe to reuse in a
// future size-aware transport boundary without changing packet layouts.
enum class LoginPacketKind { Unknown, AccountLogin, CharacterLogin };

// Maps the legacy wire opcode to a semantic packet kind; unknown opcodes are
// intentionally preserved for the existing generic dispatch path.
inline LoginPacketKind ClassifyLoginPacket(unsigned int opcode)
{
	if (opcode == MSG_CNFAccountLogin_Opcode) return LoginPacketKind::AccountLogin;
	if (opcode == MSG_CNFCharacterLogin_Opcode) return LoginPacketKind::CharacterLogin;
	return LoginPacketKind::Unknown;
}

// Returns the complete struct size required before a typed packet cast. The
// current event API does not carry a buffer length, so callers must opt into
// this check when that metadata is available.
inline std::size_t LoginPacketMinimumSize(LoginPacketKind kind)
{
	switch (kind)
	{
	case LoginPacketKind::AccountLogin: return sizeof(MSG_CNFAccountLogin);
	case LoginPacketKind::CharacterLogin: return sizeof(MSG_CNFCharacterLogin);
	default: return 0;
	}
}
