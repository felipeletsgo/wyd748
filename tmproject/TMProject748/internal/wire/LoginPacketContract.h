#pragma once

#include <cstddef>
#include "CharacterLoginConfirmContract.h"

// Contratos S->C consumidos pelas telas de login e selecao. Estes valores sao
// independentes das structs legadas para que a fronteira de transporte possa
// rejeitar frames incompletos antes de qualquer cast da cena.
constexpr auto MSG_CNFAccountLogin_Opcode = 0x10A;
constexpr auto MSG_CNFNewCharacter_Opcode = 0x110;
constexpr auto MSG_CNFDeleteCharacter_Opcode = 0x112;
constexpr auto MSG_CNFNewCharacterFail_Opcode = 0x11A;
constexpr auto MSG_AlreadyPlaying_Opcode = 0x11C;

constexpr std::size_t kAccountLoginConfirmPacketSize = 2360;
constexpr std::size_t kCharacterSelectionUpdatePacketSize = 1288;
constexpr std::size_t kSelectionFailurePacketSize = 12;

enum class LoginPacketKind
{
	Unknown,
	AccountLogin,
	CharacterCreated,
	CharacterDeleted,
	CharacterLogin,
	CharacterCreateRejected,
	AlreadyPlaying
};

// Maps the legacy wire opcode to a semantic packet kind; unknown opcodes are
// intentionally preserved for the existing generic dispatch path.
inline LoginPacketKind ClassifyLoginPacket(unsigned int opcode)
{
	if (opcode == MSG_CNFAccountLogin_Opcode) return LoginPacketKind::AccountLogin;
	if (opcode == MSG_CNFNewCharacter_Opcode) return LoginPacketKind::CharacterCreated;
	if (opcode == MSG_CNFDeleteCharacter_Opcode) return LoginPacketKind::CharacterDeleted;
	if (opcode == MSG_CNFCharacterLogin_Opcode) return LoginPacketKind::CharacterLogin;
	if (opcode == MSG_CNFNewCharacterFail_Opcode) return LoginPacketKind::CharacterCreateRejected;
	if (opcode == MSG_AlreadyPlaying_Opcode) return LoginPacketKind::AlreadyPlaying;
	return LoginPacketKind::Unknown;
}

inline std::size_t LoginPacketExpectedSize(LoginPacketKind kind)
{
	switch (kind)
	{
	case LoginPacketKind::AccountLogin: return kAccountLoginConfirmPacketSize;
	case LoginPacketKind::CharacterCreated:
	case LoginPacketKind::CharacterDeleted: return kCharacterSelectionUpdatePacketSize;
	case LoginPacketKind::CharacterLogin: return kCharacterLoginConfirmPacketSize;
	case LoginPacketKind::CharacterCreateRejected:
	case LoginPacketKind::AlreadyPlaying: return kSelectionFailurePacketSize;
	default: return 0;
	}
}
