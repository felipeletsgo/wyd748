#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: EntitiesAndShop. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchEntitiesAndShopTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    for (unsigned int opcode : {MSG_CreateMob_Opcode, MSG_CreateMobTrade_Opcode})
    {
        const std::size_t length = opcode == MSG_CreateMob_Opcode
            ? kCreateMobPacketSize : kCreateMobTradePacketSize;
        std::array<char, kCreateMobTradePacketSize + 1> createMob{};
        createMob[0] = static_cast<char>(length & 0xFF);
        createMob[1] = static_cast<char>((length >> 8) & 0xFF);
        createMob[4] = static_cast<char>(opcode & 0xFF);
        createMob[5] = static_cast<char>((opcode >> 8) & 0xFF);
        createMob[6] = 0x30;
        createMob[7] = 0x75;
        createMob[kCreateMobPositionOffset] = 0x11;
        createMob[kCreateMobIdOffset] = 0x12;
        createMob[kCreateMobNameOffset] = 0x13;
        createMob[kCreateMobEquipOffset] = 0x14;
        createMob[kCreateMobAffectOffset] = 0x15;
        createMob[kCreateMobGuildOffset] = 0x16;
        createMob[kCreateMobGuildLevelOffset] = 0x17;
        createMob[kCreateMobScoreOffset] = 0x18;
        createMob[kCreateMobTypeOffset] = 0x19;
        createMob[kCreateMobAncientOffset] = 0x1A;
        createMob[kCreateMobNickOffset] = 0x1B;
        if (opcode == MSG_CreateMob_Opcode)
            createMob[kCreateMobServerOffset] = 0x1C;
        else
        {
            createMob[kCreateMobTradeDescriptionOffset] = 0x1D;
            createMob[kCreateMobTradeServerOffset] = 0x1E;
        }
        const auto createMobBefore = createMob;
        int createMobCalls = 0;
        const auto receiveCreateMob = [&](const PacketView& view) {
            ++createMobCalls;
            check(view.data == createMob.data() && view.size == length && view.opcode == opcode,
                "coordinated CreateMob preserves frame and opcode");
        };
        for (std::size_t n = 0; n < length; ++n)
            check(!received_packet::Dispatch({opcode, createMob.data(), n}, receiveCreateMob),
                "coordinated CreateMob rejects every truncated prefix");
        check(!received_packet::Dispatch({opcode, nullptr, length}, receiveCreateMob),
            "coordinated CreateMob rejects a null buffer");
        check(!received_packet::Dispatch({opcode, createMob.data(), length + 1}, receiveCreateMob),
            "coordinated CreateMob rejects an oversized frame");
        check(!received_packet::Dispatch({0x119, createMob.data(), length}, receiveCreateMob),
            "an outer opcode cannot hide coordinated CreateMob");
        createMob[4] = static_cast<char>((opcode + 1) & 0xFF);
        check(!received_packet::Dispatch({opcode, createMob.data(), length}, receiveCreateMob),
            "coordinated CreateMob rejects a mismatched Header.Type");
        createMob[4] = static_cast<char>(opcode & 0xFF);
        createMob[0] = static_cast<char>((length - 1) & 0xFF);
        check(!received_packet::Dispatch({opcode, createMob.data(), length}, receiveCreateMob),
            "coordinated CreateMob rejects a mismatched Header.Size");
        createMob[0] = static_cast<char>(length & 0xFF);
        check(createMobCalls == 0, "invalid CreateMob does not reach the consumer");
        check(received_packet::Dispatch({opcode, createMob.data(), length}, receiveCreateMob) &&
            createMobCalls == 1, "coordinated CreateMob delivered once");
        check(createMob[kCreateMobPositionOffset] == 0x11 &&
            createMob[kCreateMobIdOffset] == 0x12 &&
            createMob[kCreateMobNameOffset] == 0x13 &&
            createMob[kCreateMobEquipOffset] == 0x14 &&
            createMob[kCreateMobAffectOffset] == 0x15 &&
            createMob[kCreateMobGuildOffset] == 0x16 &&
            createMob[kCreateMobGuildLevelOffset] == 0x17 &&
            createMob[kCreateMobScoreOffset] == 0x18 &&
            createMob[kCreateMobTypeOffset] == 0x19 &&
            createMob[kCreateMobAncientOffset] == 0x1A &&
            createMob[kCreateMobNickOffset] == 0x1B,
            "CreateMob preserves every shared block");
        check(opcode == MSG_CreateMob_Opcode
                ? createMob[kCreateMobServerOffset] == 0x1C
                : createMob[kCreateMobTradeDescriptionOffset] == 0x1D &&
                    createMob[kCreateMobTradeServerOffset] == 0x1E,
            "CreateMob preserves the normal tail or the shop title");
    check(createMob == createMobBefore,
            "gate preserves every coordinated CreateMob byte");
    }

    // The native ShopList has 27 cells and the tax in the last DWORD. The bound
    // must be closed before OnPacketShopList sorts or materializes items.
    std::array<char, kShopListPacketSize + 1> shopList{};
    shopList[0] = static_cast<char>(kShopListPacketSize);
    shopList[4] = static_cast<char>(MSG_ShopList_Opcode & 0xFF);
    shopList[5] = static_cast<char>((MSG_ShopList_Opcode >> 8) & 0xFF);
    shopList[6] = 0x34;
    shopList[7] = 0x12;
    shopList[kShopListTypeOffset] = 0x11;
    shopList[kShopListItemsOffset] = 0x12;
    shopList[kShopListItemsOffset + (kShopListItemCount - 1) * kShopListItemSize] = 0x13;
    shopList[kShopListTaxOffset] = 0x21;
    shopList[kShopListTaxOffset + 1] = 0x22;
    shopList[kShopListTaxOffset + 2] = 0x23;
    shopList[kShopListTaxOffset + 3] = 0x24;
    const auto shopListBefore = shopList;
    int shopListCalls = 0;
    const auto receiveShopList = [&](const PacketView& view) {
        ++shopListCalls;
        check(view.data == shopList.data() && view.size == kShopListPacketSize &&
            view.opcode == MSG_ShopList_Opcode,
            "ShopList preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kShopListPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_ShopList_Opcode, shopList.data(), n},
            receiveShopList), "ShopList rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_ShopList_Opcode, nullptr, kShopListPacketSize},
        receiveShopList), "ShopList rejects a null buffer");
    check(!received_packet::Dispatch({MSG_ShopList_Opcode, shopList.data(),
        kShopListPacketSize + 1}, receiveShopList), "ShopList rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, shopList.data(), kShopListPacketSize},
        receiveShopList), "an outer opcode cannot hide ShopList");
    shopList[4] = static_cast<char>((MSG_ShopList_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_ShopList_Opcode, shopList.data(),
        kShopListPacketSize}, receiveShopList), "ShopList rejects a mismatched Header.Type");
    shopList[4] = static_cast<char>(MSG_ShopList_Opcode & 0xFF);
    shopList[0] = static_cast<char>(kShopListPacketSize - 1);
    check(!received_packet::Dispatch({MSG_ShopList_Opcode, shopList.data(),
        kShopListPacketSize}, receiveShopList), "ShopList rejects a mismatched Header.Size");
    shopList[0] = static_cast<char>(kShopListPacketSize);
    check(shopListCalls == 0, "invalid ShopList does not reach the consumer");
    check(received_packet::Dispatch({MSG_ShopList_Opcode, shopList.data(),
        kShopListPacketSize}, receiveShopList) && shopListCalls == 1,
        "valid ShopList delivered once");
    check(shopList[kShopListTypeOffset] == 0x11 &&
        shopList[kShopListItemsOffset] == 0x12 &&
        shopList[kShopListItemsOffset + (kShopListItemCount - 1) * kShopListItemSize] == 0x13,
        "ShopList preserves the type and the bounds of the 27 cells");
    check(shopList[kShopListTaxOffset] == 0x21 &&
        shopList[kShopListTaxOffset + 1] == 0x22 &&
        shopList[kShopListTaxOffset + 2] == 0x23 &&
        shopList[kShopListTaxOffset + 3] == 0x24,
        "ShopList preserves the tax at offset 232");
    check(shopList == shopListBefore, "gate preserves every ShopList byte");

    // A ghost-shop sale removes a single listing identified by two
    // DWORDs; the handler must not receive partial or shifted frames.
    std::array<char, kItemSoldPacketSize + 1> itemSold{};
    itemSold[0] = static_cast<char>(kItemSoldPacketSize);
    itemSold[4] = static_cast<char>(MSG_ItemSold_Opcode & 0xFF);
    itemSold[5] = static_cast<char>((MSG_ItemSold_Opcode >> 8) & 0xFF);
    itemSold[6] = 0x34;
    itemSold[7] = 0x12;
    itemSold[kItemSoldEntityOffset] = 0x11;
    itemSold[kItemSoldPositionOffset] = 0x22;
    itemSold[kItemSoldPositionOffset + 1] = 0x23;
    itemSold[kItemSoldPositionOffset + 2] = 0x24;
    itemSold[kItemSoldPositionOffset + 3] = 0x25;
    const auto itemSoldBefore = itemSold;
    int itemSoldCalls = 0;
    const auto receiveItemSold = [&](const PacketView& view) {
        ++itemSoldCalls;
        check(view.data == itemSold.data() && view.size == kItemSoldPacketSize &&
            view.opcode == MSG_ItemSold_Opcode,
            "ItemSold preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kItemSoldPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_ItemSold_Opcode, itemSold.data(), n},
            receiveItemSold), "ItemSold rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_ItemSold_Opcode, nullptr, kItemSoldPacketSize},
        receiveItemSold), "ItemSold rejects a null buffer");
    check(!received_packet::Dispatch({MSG_ItemSold_Opcode, itemSold.data(),
        kItemSoldPacketSize + 1}, receiveItemSold), "ItemSold rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, itemSold.data(), kItemSoldPacketSize},
        receiveItemSold), "an outer opcode cannot hide ItemSold");
    itemSold[4] = static_cast<char>((MSG_ItemSold_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_ItemSold_Opcode, itemSold.data(),
        kItemSoldPacketSize}, receiveItemSold), "ItemSold rejects a mismatched Header.Type");
    itemSold[4] = static_cast<char>(MSG_ItemSold_Opcode & 0xFF);
    itemSold[0] = static_cast<char>(kItemSoldPacketSize - 1);
    check(!received_packet::Dispatch({MSG_ItemSold_Opcode, itemSold.data(),
        kItemSoldPacketSize}, receiveItemSold), "ItemSold rejects a mismatched Header.Size");
    itemSold[0] = static_cast<char>(kItemSoldPacketSize);
    check(itemSoldCalls == 0, "invalid ItemSold does not reach the consumer");
    check(received_packet::Dispatch({MSG_ItemSold_Opcode, itemSold.data(),
        kItemSoldPacketSize}, receiveItemSold) && itemSoldCalls == 1,
        "valid ItemSold delivered once");
    check(itemSold[kItemSoldEntityOffset] == 0x11 &&
        itemSold[kItemSoldPositionOffset] == 0x22 &&
        itemSold[kItemSoldPositionOffset + 1] == 0x23 &&
        itemSold[kItemSoldPositionOffset + 2] == 0x24 &&
        itemSold[kItemSoldPositionOffset + 3] == 0x25,
        "ItemSold preserves the entity and position at the contracted offsets");
    check(itemSold == itemSoldBefore, "gate preserves every ItemSold byte");

    // CombineComplete closes only the active ItemMix panel; the result in
    // Parm remains available for the flow's messages and diagnostics.
    std::array<char, kCombineCompletePacketSize + 1> combineComplete{};
    combineComplete[0] = static_cast<char>(kCombineCompletePacketSize);
    combineComplete[4] = static_cast<char>(MSG_CombineComplete_Opcode & 0xFF);
    combineComplete[5] = static_cast<char>((MSG_CombineComplete_Opcode >> 8) & 0xFF);
    combineComplete[6] = 0x34;
    combineComplete[7] = 0x12;
    combineComplete[kCombineCompleteResultOffset] = 2;
    combineComplete[kCombineCompleteResultOffset + 1] = 0;
    combineComplete[kCombineCompleteResultOffset + 2] = 0;
    combineComplete[kCombineCompleteResultOffset + 3] = 0;
    const auto combineCompleteBefore = combineComplete;
    int combineCompleteCalls = 0;
    const auto receiveCombineComplete = [&](const PacketView& view) {
        ++combineCompleteCalls;
        check(view.data == combineComplete.data() &&
            view.size == kCombineCompletePacketSize &&
            view.opcode == MSG_CombineComplete_Opcode,
            "CombineComplete preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kCombineCompletePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CombineComplete_Opcode,
            combineComplete.data(), n}, receiveCombineComplete),
            "CombineComplete rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_CombineComplete_Opcode, nullptr,
        kCombineCompletePacketSize}, receiveCombineComplete),
        "CombineComplete rejects a null buffer");
    check(!received_packet::Dispatch({MSG_CombineComplete_Opcode,
        combineComplete.data(), kCombineCompletePacketSize + 1}, receiveCombineComplete),
        "CombineComplete rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, combineComplete.data(),
        kCombineCompletePacketSize}, receiveCombineComplete),
        "an outer opcode cannot hide CombineComplete");
    combineComplete[4] = static_cast<char>((MSG_CombineComplete_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_CombineComplete_Opcode,
        combineComplete.data(), kCombineCompletePacketSize}, receiveCombineComplete),
        "CombineComplete rejects a mismatched Header.Type");
    combineComplete[4] = static_cast<char>(MSG_CombineComplete_Opcode & 0xFF);
    combineComplete[0] = static_cast<char>(kCombineCompletePacketSize - 1);
    check(!received_packet::Dispatch({MSG_CombineComplete_Opcode,
        combineComplete.data(), kCombineCompletePacketSize}, receiveCombineComplete),
        "CombineComplete rejects a mismatched Header.Size");
    combineComplete[0] = static_cast<char>(kCombineCompletePacketSize);
    check(combineCompleteCalls == 0, "invalid CombineComplete does not reach the consumer");
    check(received_packet::Dispatch({MSG_CombineComplete_Opcode,
        combineComplete.data(), kCombineCompletePacketSize}, receiveCombineComplete) &&
        combineCompleteCalls == 1, "valid CombineComplete delivered once");
    check(static_cast<unsigned char>(combineComplete[6]) == 0x34 &&
        static_cast<unsigned char>(combineComplete[7]) == 0x12 &&
        static_cast<unsigned char>(combineComplete[kCombineCompleteResultOffset]) == 2,
        "CombineComplete preserves the receiver and the result");
    check(combineComplete == combineCompleteBefore,
        "gate preserves every CombineComplete byte");

    // MSG_Trade carries the complete snapshot of the remote offer. The handler reads the
    // fifteen items and positions, money, check and OpponentID, so no
    // partial prefix may reach the legacy cast.
    return failures;
}
