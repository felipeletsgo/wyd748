#pragma once

#include <cstddef>

// Lista S->C de loja de NPC: tipo, 27 itens de oito bytes e imposto final.
constexpr auto MSG_ShopList_Opcode = 0x17C;
constexpr std::size_t kShopListPacketSize = 236;
constexpr std::size_t kShopListTypeOffset = 12;
constexpr std::size_t kShopListItemsOffset = 16;
constexpr std::size_t kShopListItemCount = 27;
constexpr std::size_t kShopListItemSize = 8;
constexpr std::size_t kShopListTaxOffset = 232;

