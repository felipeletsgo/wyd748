#pragma once
namespace unsupported_item_features {
// These later-client features have no supported 7.48 transaction contract.
constexpr bool Reject(int itemIndex, int volatileType)
{
    return (itemIndex >= 3467 && itemIndex <= 3471) || itemIndex == 4906 || itemIndex == 4907 ||
        (volatileType >= 235 && volatileType <= 241);
}
}
