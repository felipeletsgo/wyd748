#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: ActionsAndInventory. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchActionsAndInventoryTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    std::array<char, kShortSkillSnapshotPacketSize + 1> shortSkills{};
    shortSkills[0] = static_cast<char>(kShortSkillSnapshotPacketSize);
    shortSkills[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
    shortSkills[5] = static_cast<char>((MSG_SetShortSkill_Opcode >> 8) & 0xFF);
    shortSkills[6] = 0x30;
    shortSkills[7] = 0x75;
    for (std::size_t i = 0; i < kShortSkillSnapshotSkillCount; ++i)
        shortSkills[kShortSkillSnapshotSkillsOffset + i] = static_cast<char>(i + 1);
    const auto shortSkillsBefore = shortSkills;
    int shortSkillCalls = 0;
    const auto receiveShortSkills = [&](const PacketView& view) {
        ++shortSkillCalls;
        check(view.data == shortSkills.data() &&
            view.size == kShortSkillSnapshotPacketSize &&
            view.opcode == MSG_SetShortSkill_Opcode,
            "SetShortSkill preserves frame and opcode");
    };
    check(received_packet::ExpectedSize(MSG_SetShortSkill_Opcode) ==
        kShortSkillSnapshotPacketSize,
        "SetShortSkill publica tamanho esperado no gate");
    for (std::size_t n = 0; n < kShortSkillSnapshotPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_SetShortSkill_Opcode,
            shortSkills.data(), n}, receiveShortSkills),
            "SetShortSkill rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_SetShortSkill_Opcode, nullptr,
        kShortSkillSnapshotPacketSize}, receiveShortSkills),
        "SetShortSkill rejects a null buffer");
    check(!received_packet::Dispatch({MSG_SetShortSkill_Opcode,
        shortSkills.data(), kShortSkillSnapshotPacketSize + 1}, receiveShortSkills),
        "SetShortSkill rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, shortSkills.data(),
        kShortSkillSnapshotPacketSize}, receiveShortSkills),
        "an outer opcode cannot hide SetShortSkill");
    shortSkills[4] = static_cast<char>((MSG_SetShortSkill_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_SetShortSkill_Opcode,
        shortSkills.data(), kShortSkillSnapshotPacketSize}, receiveShortSkills),
        "SetShortSkill rejects a mismatched Header.Type");
    shortSkills[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
    shortSkills[0] = static_cast<char>(kShortSkillSnapshotPacketSize - 1);
    check(!received_packet::Dispatch({MSG_SetShortSkill_Opcode,
        shortSkills.data(), kShortSkillSnapshotPacketSize}, receiveShortSkills),
        "SetShortSkill rejects a mismatched Header.Size");
    shortSkills[0] = static_cast<char>(kShortSkillSnapshotPacketSize);
    check(shortSkillCalls == 0, "invalid SetShortSkill does not reach the consumer");
    check(received_packet::Dispatch({MSG_SetShortSkill_Opcode,
        shortSkills.data(), kShortSkillSnapshotPacketSize}, receiveShortSkills) &&
        shortSkillCalls == 1,
        "complete SetShortSkill delivered once");
    for (std::size_t i = 0; i < kShortSkillSnapshotSkillCount; ++i)
        check(static_cast<unsigned char>(shortSkills[
            kShortSkillSnapshotSkillsOffset + i]) == i + 1,
            "SetShortSkill preserves the twenty shortcuts");
    check(shortSkills == shortSkillsBefore,
        "gate preserves every SetShortSkill byte");

    // Action, ActionStop and Illusion share the 52-byte frame. The
    // consumer reads the destination and Route without a second size check.
    std::array<char, kActionPacketSize + 1> action{};
    action[0] = static_cast<char>(kActionPacketSize);
    action[6] = 0x34;
    action[7] = 0x12;
    action[kActionPositionOffset] = 0x45;
    action[kActionPositionOffset + 1] = 0x03;
    action[kActionPositionOffset + 2] = 0x56;
    action[kActionPositionOffset + 3] = 0x04;
    action[kActionSpeedOffset] = 0x78;
    action[kActionEffectOffset] = 0x21;
    action[kActionTargetOffset] = 0x67;
    action[kActionTargetOffset + 1] = 0x05;
    action[kActionTargetOffset + 2] = 0x78;
    action[kActionTargetOffset + 3] = 0x06;
    for (std::size_t i = 0; i < kActionRouteSize; ++i)
        action[kActionRouteOffset + i] = static_cast<char>('0' + (i % 8));

    const std::array<unsigned int, 3> actionOpcodes{
        MSG_Action_Opcode, MSG_Action_Stop_Opcode, MSG_Action2_Opcode
    };
    for (const auto opcode : actionOpcodes)
    {
        action[4] = static_cast<char>(opcode & 0xFF);
        action[5] = static_cast<char>((opcode >> 8) & 0xFF);
        const auto actionBefore = action;
        int actionCalls = 0;
        const auto receiveAction = [&](const PacketView& view) {
            ++actionCalls;
            check(view.data == action.data() && view.size == kActionPacketSize &&
                view.opcode == opcode,
                "Action family preserves frame and opcode");
        };
        check(received_packet::ExpectedSize(opcode) == kActionPacketSize,
            "Action family publica tamanho esperado no gate");
        for (std::size_t n = 0; n < kActionPacketSize; ++n)
            check(!received_packet::Dispatch({opcode, action.data(), n}, receiveAction),
                "Action family rejects every truncated prefix");
        check(!received_packet::Dispatch({opcode, nullptr, kActionPacketSize},
            receiveAction), "Action family rejects a null buffer");
        check(!received_packet::Dispatch({opcode, action.data(), kActionPacketSize + 1},
            receiveAction), "Action family rejects an oversized frame");
        check(!received_packet::Dispatch({0x119, action.data(), kActionPacketSize},
            receiveAction), "an outer opcode cannot hide the Action family");
        action[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
        action[5] = static_cast<char>((MSG_SetShortSkill_Opcode >> 8) & 0xFF);
        check(!received_packet::Dispatch({opcode, action.data(), kActionPacketSize},
            receiveAction), "Action family rejects a mismatched Header.Type");
        action[4] = static_cast<char>(opcode & 0xFF);
        action[5] = static_cast<char>((opcode >> 8) & 0xFF);
        action[0] = static_cast<char>(kActionPacketSize - 1);
        check(!received_packet::Dispatch({opcode, action.data(), kActionPacketSize},
            receiveAction), "Action family rejects a mismatched Header.Size");
        action[0] = static_cast<char>(kActionPacketSize);
        check(actionCalls == 0, "invalid Action family frame does not reach the consumer");
        check(received_packet::Dispatch({opcode, action.data(), kActionPacketSize},
            receiveAction) && actionCalls == 1,
            "valid Action family frame delivered once");
        check(static_cast<unsigned char>(action[kActionPositionOffset]) == 0x45 &&
            static_cast<unsigned char>(action[kActionSpeedOffset]) == 0x78 &&
            static_cast<unsigned char>(action[kActionEffectOffset]) == 0x21 &&
            static_cast<unsigned char>(action[kActionTargetOffset]) == 0x67 &&
            action[kActionRouteOffset] == '0' &&
            action[kActionRouteOffset + kActionRouteSize - 1] == '7',
            "Action family preserves position, speed, effect, destination and route");
        check(action == actionBefore,
            "gate preserves every Action family byte");
    }

    // SwapItem completes the drag and indexes Equip/Carry/Cargo directly. Both the
    // envelope and the domains published by the server must both match.
    std::array<char, kSwapItemPacketSize + 1> swapItem{};
    swapItem[0] = static_cast<char>(kSwapItemPacketSize);
    swapItem[4] = static_cast<char>(MSG_SwapItem_Opcode & 0xFF);
    swapItem[5] = static_cast<char>((MSG_SwapItem_Opcode >> 8) & 0xFF);
    swapItem[kSwapItemSourceTypeOffset] = kSwapPlaceCarry;
    swapItem[kSwapItemSourcePositionOffset] = 3;
    swapItem[kSwapItemDestinationTypeOffset] = kSwapPlaceCargo;
    swapItem[kSwapItemDestinationPositionOffset] = 14;
    swapItem[kSwapItemTargetIdOffset] = 0x34;
    swapItem[kSwapItemTargetIdOffset + 1] = 0x12;
    swapItem[kSwapItemReservedOffset] = 0x56;
    const auto swapItemBefore = swapItem;
    int swapItemCalls = 0;
    const auto receiveSwapItem = [&](const PacketView& view) {
        ++swapItemCalls;
        check(view.data == swapItem.data() && view.size == kSwapItemPacketSize &&
            view.opcode == MSG_SwapItem_Opcode,
            "SwapItem preserves frame and opcode");
    };
    check(received_packet::ExpectedSize(MSG_SwapItem_Opcode) ==
        kSwapItemPacketSize, "SwapItem publica tamanho esperado no gate");
    for (std::size_t n = 0; n < kSwapItemPacketSize; ++n)
        check(!received_packet::Dispatch(
            {MSG_SwapItem_Opcode, swapItem.data(), n}, receiveSwapItem),
            "SwapItem rejects every truncated prefix");
    check(!received_packet::Dispatch(
        {MSG_SwapItem_Opcode, nullptr, kSwapItemPacketSize}, receiveSwapItem),
        "SwapItem rejects a null buffer");
    check(!received_packet::Dispatch(
        {MSG_SwapItem_Opcode, swapItem.data(), kSwapItemPacketSize + 1},
        receiveSwapItem), "SwapItem rejects an oversized frame");
    check(!received_packet::Dispatch(
        {0x119, swapItem.data(), kSwapItemPacketSize}, receiveSwapItem),
        "an outer opcode cannot hide SwapItem");
    swapItem[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
    swapItem[5] = static_cast<char>((MSG_SetShortSkill_Opcode >> 8) & 0xFF);
    check(!received_packet::Dispatch(
        {MSG_SwapItem_Opcode, swapItem.data(), kSwapItemPacketSize},
        receiveSwapItem), "SwapItem rejects a mismatched Header.Type");
    swapItem[4] = static_cast<char>(MSG_SwapItem_Opcode & 0xFF);
    swapItem[5] = static_cast<char>((MSG_SwapItem_Opcode >> 8) & 0xFF);
    swapItem[0] = static_cast<char>(kSwapItemPacketSize - 1);
    check(!received_packet::Dispatch(
        {MSG_SwapItem_Opcode, swapItem.data(), kSwapItemPacketSize},
        receiveSwapItem), "SwapItem rejects a mismatched Header.Size");
    swapItem[0] = static_cast<char>(kSwapItemPacketSize);
    check(swapItemCalls == 0, "invalid SwapItem does not reach the consumer");
    check(received_packet::Dispatch(
        {MSG_SwapItem_Opcode, swapItem.data(), kSwapItemPacketSize},
        receiveSwapItem) && swapItemCalls == 1,
        "valid SwapItem delivered once");
    check(swapItem == swapItemBefore,
        "gate preserves every SwapItem byte");

    for (int position = 0; position <= 255; ++position)
    {
        check(IsSwapPlacePosition(kSwapPlaceEquip,
            static_cast<unsigned char>(position)) ==
            (position < kSwapEquipSlotCount &&
                position != kSwapUnsupportedNecklaceSlot),
            "SwapItem validates the active peer's Equip domain");
        check(IsSwapPlacePosition(kSwapPlaceCarry,
            static_cast<unsigned char>(position)) ==
            (position < kSwapVisibleCarrySlotCount),
            "SwapItem validates the visible Carry domain");
        check(IsSwapPlacePosition(kSwapPlaceCargo,
            static_cast<unsigned char>(position)) ==
            (position < kSwapUsableCargoSlotCount),
            "SwapItem validates the usable Cargo domain");
    }
    check(!IsSwapPlacePosition(3, 0),
        "SwapItem rejects an unknown place type");

    struct CachedItem { short index; unsigned short effect[3]; };
    CachedItem sourceItem{123, {4, 5, 6}};
    CachedItem destinationItem{456, {7, 8, 9}};
    ApplyConfirmedItemSwap(sourceItem, destinationItem);
    check(sourceItem.index == 456 && sourceItem.effect[0] == 7 &&
        sourceItem.effect[2] == 9 && destinationItem.index == 123 &&
        destinationItem.effect[0] == 4 && destinationItem.effect[2] == 6,
        "SwapItem swaps the eight logical bytes without depending on visual controls");
    ApplyConfirmedItemSwap(sourceItem, sourceItem);
    check(sourceItem.index == 456 && sourceItem.effect[1] == 8,
        "SwapItem in the same position preserves the logical item");
    CachedItem emptyItem{};
    ApplyConfirmedItemSwap(sourceItem, emptyItem);
    check(sourceItem.index == 0 && emptyItem.index == 456 && emptyItem.effect[2] == 9,
        "SwapItem moves an item to an empty slot even without a visual at the source");

    // Buy uses the sparse shop-grid position and an authoritative Carry slot.
    std::array<char, kBuyPacketSize + 1> buy{};
    buy[0] = static_cast<char>(kBuyPacketSize);
    buy[4] = static_cast<char>(MSG_Buy_Opcode & 0xFF);
    buy[5] = static_cast<char>((MSG_Buy_Opcode >> 8) & 0xFF);
    buy[kBuyTargetIdOffset] = 0x34;
    buy[kBuyTargetIdOffset + 1] = 0x12;
    buy[kBuyShopPositionOffset] = 27;
    buy[kBuyCarryPositionOffset] = 14;
    buy[kBuyCoinOffset] = 0x21;
    buy[kBuyCoinOffset + 1] = 0x43;
    buy[kBuyCoinOffset + 2] = 0x65;
    buy[kBuyCoinOffset + 3] = static_cast<char>(0x87);
    const auto buyBefore = buy;
    int buyCalls = 0;
    const auto receiveBuy = [&](const PacketView& view) {
        ++buyCalls;
        check(view.data == buy.data() && view.size == kBuyPacketSize &&
            view.opcode == MSG_Buy_Opcode,
            "Buy preserves frame and opcode");
    };
    check(received_packet::ExpectedSize(MSG_Buy_Opcode) == kBuyPacketSize,
        "Buy publica tamanho esperado no gate");
    for (std::size_t n = 0; n < kBuyPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_Buy_Opcode, buy.data(), n}, receiveBuy),
            "Buy rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_Buy_Opcode, nullptr, kBuyPacketSize},
        receiveBuy), "Buy rejects a null buffer");
    check(!received_packet::Dispatch(
        {MSG_Buy_Opcode, buy.data(), kBuyPacketSize + 1}, receiveBuy),
        "Buy rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, buy.data(), kBuyPacketSize}, receiveBuy),
        "an outer opcode cannot hide Buy");
    buy[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
    buy[5] = static_cast<char>((MSG_SetShortSkill_Opcode >> 8) & 0xFF);
    check(!received_packet::Dispatch({MSG_Buy_Opcode, buy.data(), kBuyPacketSize},
        receiveBuy), "Buy rejects a mismatched Header.Type");
    buy[4] = static_cast<char>(MSG_Buy_Opcode & 0xFF);
    buy[5] = static_cast<char>((MSG_Buy_Opcode >> 8) & 0xFF);
    buy[0] = static_cast<char>(kBuyPacketSize - 1);
    check(!received_packet::Dispatch({MSG_Buy_Opcode, buy.data(), kBuyPacketSize},
        receiveBuy), "Buy rejects a mismatched Header.Size");
    buy[0] = static_cast<char>(kBuyPacketSize);
    check(buyCalls == 0, "invalid Buy does not reach the consumer");
    check(received_packet::Dispatch({MSG_Buy_Opcode, buy.data(), kBuyPacketSize},
        receiveBuy) && buyCalls == 1, "valid Buy delivered once");
    check(buy == buyBefore, "gate preserves every Buy byte");

    for (int position = -1; position <= kBuyShopPositionLimit; ++position)
    {
        const bool expected = position >= 0 &&
            position < kBuyShopPositionLimit &&
            position % kBuyShopBlockStride < kBuyShopBlockWidth;
        check(IsBuyShopPosition(position) == expected,
            "Buy accepts only published shop cells");
    }
    for (int position = -1; position <= kBuyVisibleCarrySlotCount; ++position)
        check(IsBuyCarryPosition(position) ==
            (position >= 0 && position < kBuyVisibleCarrySlotCount),
            "Buy accepts only the visible 9x7 Carry");

    // Attacks use three native prefixes and coordinated tails of variable
    // size. Header.Size can never authorize a read past the view.
    std::array<char, kAttackMultiWideMaxPacketSize + 1> attack{};
    attack[kAttackAttackerIdOffset] = 0x34;
    attack[kAttackAttackerIdOffset + 1] = 0x12;
    attack[kAttackProgressOffset] = 0x56;
    attack[kAttackPositionOffset] = 0x21;
    attack[kAttackTargetPositionOffset] = 0x43;
    attack[kAttackSkillIndexOffset] = 0x65;
    attack[kAttackCurrentMpOffset] = 0x76;
    attack[kAttackMotionOffset] = 0x12;
    attack[kAttackSkillParameterOffset] = 0x23;
    attack[kAttackLocalFlagOffset] = 0x34;
    attack[kAttackDoubleCriticalOffset] = 0x45;
    attack[kAttackCurrentExpOffset] = 0x56;
    attack[kAttackRequiredMpOffset] = 0x67;
    attack[kAttackReservedOffset] = 0x78;
    attack[kAttackFakeExpOffset] = static_cast<char>(0x89);
    attack[kAttackDamagesOffset] = static_cast<char>(0x9A);

    const std::array<unsigned int, 3> attackOpcodes{
        MSG_Attack_One_Opcode, MSG_Attack_Two_Opcode,
        MSG_Attack_Multi_Opcode
    };
    for (const auto opcode : attackOpcodes)
    {
        attack[4] = static_cast<char>(opcode & 0xFF);
        attack[5] = static_cast<char>((opcode >> 8) & 0xFF);
        int attackCalls = 0;
        const auto receiveAttack = [&](const PacketView& view) {
            ++attackCalls;
            check(view.data == attack.data() && view.opcode == opcode &&
                IsAttackPacketSize(view.opcode, view.size),
                "Attack family preserves view, opcode and valid size");
        };

        check(received_packet::ExpectedSize(opcode) == 0,
            "Attack family uses a variable contract outside ExpectedSize");
        int expectedCalls = 0;
        for (std::size_t n = 0; n <= kAttackMultiWideMaxPacketSize + 1; ++n)
        {
            attack[0] = static_cast<char>(n);
            const auto before = attack;
            const bool expected = IsAttackPacketSize(opcode, n);
            check(received_packet::Dispatch({opcode, attack.data(), n}, receiveAttack) ==
                expected, "Attack family accepts only published envelopes");
            check(attack == before,
                "gate preserves every Attack family byte");
            if (expected)
                ++expectedCalls;
        }
        check(attackCalls == expectedCalls,
            "Attack family delivers each valid size once");

        const std::size_t nativeSize = opcode == MSG_Attack_One_Opcode ?
            kAttackOneBasePacketSize : opcode == MSG_Attack_Two_Opcode ?
            kAttackTwoBasePacketSize : kAttackMultiBasePacketSize;
        attack[0] = static_cast<char>(nativeSize);
        check(!received_packet::Dispatch({opcode, nullptr, nativeSize}, receiveAttack),
            "Attack family rejects a null buffer");
        check(!received_packet::Dispatch({0x119, attack.data(), nativeSize}, receiveAttack),
            "an outer opcode cannot hide the Attack family");

        attack[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
        attack[5] = static_cast<char>((MSG_SetShortSkill_Opcode >> 8) & 0xFF);
        check(!received_packet::Dispatch({opcode, attack.data(), nativeSize}, receiveAttack),
            "Attack family rejects a mismatched Header.Type");
        attack[4] = static_cast<char>(opcode & 0xFF);
        attack[5] = static_cast<char>((opcode >> 8) & 0xFF);

        const std::size_t otherValidSize = opcode == MSG_Attack_One_Opcode ?
            kAttackPhysicalWidePacketSize : opcode == MSG_Attack_Two_Opcode ?
            kAttackTwoWideMinPacketSize : kAttackMultiWideMinPacketSize;
        attack[0] = static_cast<char>(nativeSize);
        check(!received_packet::Dispatch(
            {opcode, attack.data(), otherValidSize}, receiveAttack),
            "Attack family rejects an actual size that differs from Header.Size");
        check(static_cast<unsigned char>(attack[kAttackAttackerIdOffset]) == 0x34 &&
            static_cast<unsigned char>(attack[kAttackProgressOffset]) == 0x56 &&
            static_cast<unsigned char>(attack[kAttackPositionOffset]) == 0x21 &&
            static_cast<unsigned char>(attack[kAttackTargetPositionOffset]) == 0x43 &&
            static_cast<unsigned char>(attack[kAttackSkillIndexOffset]) == 0x65 &&
            static_cast<unsigned char>(attack[kAttackDamagesOffset]) == 0x9A,
            "Attack family preserves the prefix and the first damage entry");
    }

    // AutoTrade writes terminators into the description, copies the whole frame and
    // walks the twelve items/prices; none of these accesses accepts truncation.
    return failures;
}
