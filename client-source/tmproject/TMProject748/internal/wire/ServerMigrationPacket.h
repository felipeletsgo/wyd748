#pragma once
#include "MessageHeader.h"
#include <cstddef>

// Migracao S->C: imagem completa retida pela cena ate a desconexao.
// Campos inline nao possuem ponteiros; TID preserva os 52 bytes do servidor.
constexpr auto MSG_CNFRemoveServer_Opcode = 0x52A;
struct MSG_CNFRemoveServer
{
	MSG_STANDARD Header;
	char AccountName[16];
	char TID[52];
};
static_assert(sizeof(MSG_CNFRemoveServer) == 0x50, "WYD 7.48 remove-server packet size mismatch");
static_assert(offsetof(MSG_CNFRemoveServer, AccountName) == 0x0C, "WYD 7.48 remove-server AccountName offset mismatch");
static_assert(offsetof(MSG_CNFRemoveServer, TID) == 0x1C, "WYD 7.48 remove-server TID offset mismatch");
