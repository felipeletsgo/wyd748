#pragma once

#include <cstddef>

// Notificacao S->C de venda em loja fantasma. O ID do clone fica em Parm1 e a
// posicao do anuncio em Parm2; ambos permanecem DWORD no frame final.
constexpr auto MSG_ItemSold_Opcode = 0x39B;
constexpr std::size_t kItemSoldPacketSize = 20;
constexpr std::size_t kItemSoldEntityOffset = 12;
constexpr std::size_t kItemSoldPositionOffset = 16;

