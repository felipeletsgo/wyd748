#pragma once
#include "MessageHeader.h"
#include <cstddef>
#include <cstdlib>
#include <cerrno>
#include <cstring>

// Migracao S->C: imagem completa retida pela cena ate a desconexao.
// Campos inline nao possuem ponteiros; TID preserva os 52 bytes do servidor.
constexpr auto MSG_CNFRemoveServer_Opcode = 0x52A;

// Le o prefixo *%d legado sem ultrapassar o campo recebido. Sufixos do
// ticket permanecem opacos; falha preserva o indice de saida do chamador.
inline bool ParseMigrationServer(const char (&ticket)[52], int capacity, int& result)
{
    if (capacity <= 0 || ticket[0] != '*') return false;
    char bounded[53]{};
    std::memcpy(bounded, ticket, sizeof(ticket));
    char* end = nullptr;
    errno = 0;
    const long value = std::strtol(bounded + 1, &end, 10);
    if (end == bounded + 1 || errno == ERANGE || value < 0 || value >= capacity)
        return false;
    result = static_cast<int>(value);
    return true;
}
struct MSG_CNFRemoveServer
{
	MSG_STANDARD Header;
	char AccountName[16];
	char TID[52];
};
static_assert(sizeof(MSG_CNFRemoveServer) == 0x50, "WYD 7.48 remove-server packet size mismatch");
static_assert(offsetof(MSG_CNFRemoveServer, AccountName) == 0x0C, "WYD 7.48 remove-server AccountName offset mismatch");
static_assert(offsetof(MSG_CNFRemoveServer, TID) == 0x1C, "WYD 7.48 remove-server TID offset mismatch");
