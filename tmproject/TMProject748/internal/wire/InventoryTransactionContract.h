#pragma once

#include <cstddef>

// Confirmacoes autoritativas que concluem a movimentacao de item e a compra
// na UI 7.48. Os dominios abaixo espelham exatamente os slots que o WYD-Go
// valida antes de publicar a resposta.
constexpr auto MSG_SwapItem_Opcode = 0x376;
constexpr auto MSG_Buy_Opcode = 0x379;

constexpr std::size_t kSwapItemPacketSize = 20;
constexpr std::size_t kSwapItemSourceTypeOffset = 12;
constexpr std::size_t kSwapItemSourcePositionOffset = 13;
constexpr std::size_t kSwapItemDestinationTypeOffset = 14;
constexpr std::size_t kSwapItemDestinationPositionOffset = 15;
constexpr std::size_t kSwapItemTargetIdOffset = 16;
constexpr std::size_t kSwapItemReservedOffset = 18;

constexpr unsigned char kSwapPlaceEquip = 0;
constexpr unsigned char kSwapPlaceCarry = 1;
constexpr unsigned char kSwapPlaceCargo = 2;
constexpr int kSwapEquipSlotCount = 16;
constexpr int kSwapUnsupportedNecklaceSlot = 9;
constexpr int kSwapVisibleCarrySlotCount = 63;
constexpr int kSwapUsableCargoSlotCount = 120;

constexpr bool IsSwapPlacePosition(unsigned char type, unsigned char position)
{
    switch (type)
    {
    case kSwapPlaceEquip:
        return position < kSwapEquipSlotCount &&
            position != kSwapUnsupportedNecklaceSlot;
    case kSwapPlaceCarry:
        return position < kSwapVisibleCarrySlotCount;
    case kSwapPlaceCargo:
        return position < kSwapUsableCargoSlotCount;
    default:
        return false;
    }
}

// A 0x376 confirmation carries positions only. Commit the cached items even
// when one of the corresponding UI grids is absent or rejects its visual.
template <typename Item>
void ApplyConfirmedItemSwap(Item& source, Item& destination)
{
    const Item previous = source;
    source = destination;
    destination = previous;
}

constexpr std::size_t kBuyPacketSize = 24;
constexpr std::size_t kBuyTargetIdOffset = 12;
constexpr std::size_t kBuyShopPositionOffset = 14;
constexpr std::size_t kBuyCarryPositionOffset = 16;
constexpr std::size_t kBuyCoinOffset = 20;
constexpr int kBuyShopBlockStride = 27;
constexpr int kBuyShopBlockWidth = 9;
constexpr int kBuyShopBlockCount = 3;
constexpr int kBuyShopPositionLimit =
    kBuyShopBlockStride * kBuyShopBlockCount;
constexpr int kBuyVisibleCarrySlotCount = 63;

constexpr bool IsBuyShopPosition(int position)
{
    return position >= 0 && position < kBuyShopPositionLimit &&
        position % kBuyShopBlockStride < kBuyShopBlockWidth;
}

constexpr bool IsBuyCarryPosition(int position)
{
    return position >= 0 && position < kBuyVisibleCarrySlotCount;
}
