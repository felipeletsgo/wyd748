#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: WorldState. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchWorldStateTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    for (unsigned int opcode : {MSG_InstanceTime_Opcode, MSG_InstanceMobs_Opcode})
    {
        std::array<char, kInstanceCounterPacketSize + 1> counter{};
        counter[0] = static_cast<char>(kInstanceCounterPacketSize);
        counter[4] = static_cast<char>(opcode & 0xFF);
        counter[5] = static_cast<char>((opcode >> 8) & 0xFF);
        counter[kInstanceCounterValueOffset] = static_cast<char>(0x78);
        counter[kInstanceCounterValueOffset + 1] = static_cast<char>(0x56);
        counter[kInstanceCounterValueOffset + 2] = static_cast<char>(0x34);
        counter[kInstanceCounterValueOffset + 3] = static_cast<char>(0x12);
        const auto counterBefore = counter;
        int counterCalls = 0;
        const auto receiveCounter = [&](const PacketView& view) {
            ++counterCalls;
            check(view.data == counter.data() && view.size == kInstanceCounterPacketSize &&
                view.opcode == opcode,
                "instance counter preserves frame and opcode");
        };
        for (std::size_t n = 0; n < kInstanceCounterPacketSize; ++n)
            check(!received_packet::Dispatch({opcode, counter.data(), n}, receiveCounter),
                "instance counter rejects every truncated prefix");
        check(!received_packet::Dispatch({opcode, nullptr, kInstanceCounterPacketSize},
            receiveCounter), "null instance counter rejected");
        check(!received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize + 1}, receiveCounter),
            "oversized instance counter rejected");
        check(!received_packet::Dispatch({0x119, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter),
            "an outer opcode cannot hide the instance counter");
        counter[4] = static_cast<char>((opcode + 1) & 0xFF);
        counter[5] = static_cast<char>(((opcode + 1) >> 8) & 0xFF);
        check(!received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter),
            "mismatched Header.Type rejected for the instance counter");
        counter[4] = static_cast<char>(opcode & 0xFF);
        counter[5] = static_cast<char>((opcode >> 8) & 0xFF);
        counter[0] = static_cast<char>(kInstanceCounterPacketSize - 1);
        check(!received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter),
            "mismatched Header.Size rejected for the instance counter");
        counter[0] = static_cast<char>(kInstanceCounterPacketSize);
        check(counterCalls == 0, "invalid counter does not reach the consumer");
        check(received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter) && counterCalls == 1,
            "valid counter delivered once");
        check(static_cast<unsigned char>(counter[kInstanceCounterValueOffset]) == 0x78 &&
            static_cast<unsigned char>(counter[kInstanceCounterValueOffset + 1]) == 0x56 &&
            static_cast<unsigned char>(counter[kInstanceCounterValueOffset + 2]) == 0x34 &&
            static_cast<unsigned char>(counter[kInstanceCounterValueOffset + 3]) == 0x12,
            "counter preserves the value at offset 12");
        check(counter == counterBefore, "gate preserves the instance counter bytes");
    }

    // RemoveMob and UpdateCargoGold share the one-DWORD envelope, but
    // Header.ID still selects different receivers. Validating both prevents
    // a short frame from reaching either the entity tree or the Field.
    for (unsigned int opcode : {MSG_RemoveMob_Opcode, MSG_UpdateCargoGold_Opcode})
    {
        std::array<char, kWorldStateParameterPacketSize + 1> worldState{};
        worldState[0] = static_cast<char>(kWorldStateParameterPacketSize);
        worldState[4] = static_cast<char>(opcode & 0xFF);
        worldState[5] = static_cast<char>((opcode >> 8) & 0xFF);
        worldState[6] = static_cast<char>(0x34);
        worldState[7] = static_cast<char>(0x12);
        worldState[kWorldStateParameterValueOffset] = static_cast<char>(0xEF);
        worldState[kWorldStateParameterValueOffset + 1] = static_cast<char>(0xCD);
        worldState[kWorldStateParameterValueOffset + 2] = static_cast<char>(0xAB);
        worldState[kWorldStateParameterValueOffset + 3] = static_cast<char>(0x89);
        const auto worldStateBefore = worldState;
        int worldStateCalls = 0;
        const auto receiveWorldState = [&](const PacketView& view) {
            ++worldStateCalls;
            check(view.data == worldState.data() &&
                view.size == kWorldStateParameterPacketSize && view.opcode == opcode,
                "world state preserves frame and opcode");
        };
        for (std::size_t n = 0; n < kWorldStateParameterPacketSize; ++n)
            check(!received_packet::Dispatch({opcode, worldState.data(), n},
                receiveWorldState), "world state rejects every truncated prefix");
        check(!received_packet::Dispatch({opcode, nullptr,
            kWorldStateParameterPacketSize}, receiveWorldState),
            "null world state rejected");
        check(!received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize + 1}, receiveWorldState),
            "oversized world state rejected");
        check(!received_packet::Dispatch({0x119, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState),
            "an outer opcode cannot hide the world state");
        worldState[4] = static_cast<char>((opcode + 1) & 0xFF);
        worldState[5] = static_cast<char>(((opcode + 1) >> 8) & 0xFF);
        check(!received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState),
            "mismatched Header.Type rejected for world state");
        worldState[4] = static_cast<char>(opcode & 0xFF);
        worldState[5] = static_cast<char>((opcode >> 8) & 0xFF);
        worldState[0] = static_cast<char>(kWorldStateParameterPacketSize - 1);
        check(!received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState),
            "mismatched Header.Size rejected for world state");
        worldState[0] = static_cast<char>(kWorldStateParameterPacketSize);
        check(worldStateCalls == 0, "invalid state does not reach the consumer");
        check(received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState) && worldStateCalls == 1,
            "valid world state delivered once");
        check(static_cast<unsigned char>(worldState[6]) == 0x34 &&
            static_cast<unsigned char>(worldState[7]) == 0x12,
            "world state preserves Header.ID");
        check(static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset]) == 0xEF &&
            static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset + 1]) == 0xCD &&
            static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset + 2]) == 0xAB &&
            static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset + 3]) == 0x89,
            "world state preserves the DWORD at offset 12");
        check(worldState == worldStateBefore,
            "gate preserves every world-state byte");
    }

    // Deposit and Withdraw return the same envelope the client sent, before
    // the authoritative gold snapshots. Both must reach the legacy
    // MSG_STANDARDPARM cast in TMFieldScene complete.
    for (unsigned int opcode : {MSG_Withdraw_Opcode, MSG_Deposit_Opcode})
    {
        std::array<char, kCargoGoldTransferPacketSize + 1> transfer{};
        transfer[0] = static_cast<char>(kCargoGoldTransferPacketSize);
        transfer[4] = static_cast<char>(opcode & 0xFF);
        transfer[5] = static_cast<char>((opcode >> 8) & 0xFF);
        transfer[6] = static_cast<char>(0x34);
        transfer[7] = static_cast<char>(0x12);
        transfer[kCargoGoldTransferAmountOffset] = static_cast<char>(0x78);
        transfer[kCargoGoldTransferAmountOffset + 1] = static_cast<char>(0x56);
        transfer[kCargoGoldTransferAmountOffset + 2] = static_cast<char>(0x34);
        transfer[kCargoGoldTransferAmountOffset + 3] = static_cast<char>(0x12);
        const auto transferBefore = transfer;
        int transferCalls = 0;
        const auto receiveTransfer = [&](const PacketView& view) {
            ++transferCalls;
            check(view.data == transfer.data() &&
                view.size == kCargoGoldTransferPacketSize && view.opcode == opcode,
                "gold transfer preserves frame and opcode");
        };
        for (std::size_t n = 0; n < kCargoGoldTransferPacketSize; ++n)
            check(!received_packet::Dispatch({opcode, transfer.data(), n},
                receiveTransfer), "gold transfer rejects every truncated prefix");
        check(!received_packet::Dispatch({opcode, nullptr,
            kCargoGoldTransferPacketSize}, receiveTransfer),
            "null gold transfer rejected");
        check(!received_packet::Dispatch({opcode, transfer.data(),
            kCargoGoldTransferPacketSize + 1}, receiveTransfer),
            "oversized gold transfer rejected");
        check(!received_packet::Dispatch({0x119, transfer.data(),
            kCargoGoldTransferPacketSize}, receiveTransfer),
            "an outer opcode cannot hide the gold transfer");
        transfer[4] = static_cast<char>((opcode + 1) & 0xFF);
        transfer[5] = static_cast<char>(((opcode + 1) >> 8) & 0xFF);
        check(!received_packet::Dispatch({opcode, transfer.data(),
            kCargoGoldTransferPacketSize}, receiveTransfer),
            "mismatched Header.Type rejected for the gold transfer");
        transfer[4] = static_cast<char>(opcode & 0xFF);
        transfer[5] = static_cast<char>((opcode >> 8) & 0xFF);
        transfer[0] = static_cast<char>(kCargoGoldTransferPacketSize - 1);
        check(!received_packet::Dispatch({opcode, transfer.data(),
            kCargoGoldTransferPacketSize}, receiveTransfer),
            "mismatched Header.Size rejected for the gold transfer");
        transfer[0] = static_cast<char>(kCargoGoldTransferPacketSize);
        check(transferCalls == 0, "invalid transfer does not reach the consumer");
        check(received_packet::Dispatch({opcode, transfer.data(),
            kCargoGoldTransferPacketSize}, receiveTransfer) && transferCalls == 1,
            "valid gold transfer delivered once");
        check(static_cast<unsigned char>(transfer[6]) == 0x34 &&
            static_cast<unsigned char>(transfer[7]) == 0x12,
            "gold transfer preserves Header.ID");
        check(static_cast<unsigned char>(transfer[kCargoGoldTransferAmountOffset]) == 0x78 &&
            static_cast<unsigned char>(transfer[kCargoGoldTransferAmountOffset + 1]) == 0x56 &&
            static_cast<unsigned char>(transfer[kCargoGoldTransferAmountOffset + 2]) == 0x34 &&
            static_cast<unsigned char>(transfer[kCargoGoldTransferAmountOffset + 3]) == 0x12,
            "gold transfer preserves the value at offset 12");
        check(transfer == transferBefore,
            "gate preserves every gold-transfer byte");
    }

    // The source client and WYD-Go use a single 28-byte 0x181. This fixture
    // pins the coordinated extension and prevents silently accepting the
    // historical 20/36-byte layouts under the same opcode.
    std::array<char, kHpMpPacketSize + 1> hpMp{};
    hpMp[0] = static_cast<char>(kHpMpPacketSize);
    hpMp[4] = static_cast<char>(MSG_SetHpMp_Opcode & 0xFF);
    hpMp[5] = static_cast<char>((MSG_SetHpMp_Opcode >> 8) & 0xFF);
    hpMp[6] = 0x34;
    hpMp[7] = 0x12;
    const std::array<std::size_t, 4> hpMpOffsets{
        kHpMpCurrentHpOffset, kHpMpCurrentMpOffset,
        kHpMpMaximumHpOffset, kHpMpMaximumMpOffset
    };
    for (std::size_t field = 0; field < hpMpOffsets.size(); ++field)
    {
        const auto offset = hpMpOffsets[field];
        hpMp[offset] = static_cast<char>(0x10 + field);
        hpMp[offset + 1] = static_cast<char>(0x20 + field);
        hpMp[offset + 2] = static_cast<char>(0x30 + field);
        hpMp[offset + 3] = static_cast<char>(0x40 + field);
    }
    const auto hpMpBefore = hpMp;
    int hpMpCalls = 0;
    const auto receiveHpMp = [&](const PacketView& view) {
        ++hpMpCalls;
        check(view.data == hpMp.data() && view.size == kHpMpPacketSize &&
            view.opcode == MSG_SetHpMp_Opcode,
            "coordinated HP/MP preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kHpMpPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), n},
            receiveHpMp), "coordinated HP/MP rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, nullptr, kHpMpPacketSize},
        receiveHpMp), "coordinated HP/MP rejects a null buffer");
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(),
        kHpMpPacketSize + 1}, receiveHpMp), "coordinated HP/MP rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, hpMp.data(), kHpMpPacketSize},
        receiveHpMp), "an outer opcode cannot hide coordinated HP/MP");
    hpMp[4] = static_cast<char>((MSG_SetHpMp_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), kHpMpPacketSize},
        receiveHpMp), "coordinated HP/MP rejects a mismatched Header.Type");
    hpMp[4] = static_cast<char>(MSG_SetHpMp_Opcode & 0xFF);
    hpMp[0] = static_cast<char>(kHpMpPacketSize - 1);
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), kHpMpPacketSize},
        receiveHpMp), "coordinated HP/MP rejects a mismatched Header.Size");
    hpMp[0] = static_cast<char>(kHpMpPacketSize);
    check(hpMpCalls == 0, "invalid HP/MP does not reach the consumer");
    check(received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), kHpMpPacketSize},
        receiveHpMp) && hpMpCalls == 1, "coordinated HP/MP delivered once");
    check(static_cast<unsigned char>(hpMp[6]) == 0x34 &&
        static_cast<unsigned char>(hpMp[7]) == 0x12,
        "coordinated HP/MP preserves the receiver in Header.ID");
    for (std::size_t field = 0; field < hpMpOffsets.size(); ++field)
    {
        const auto offset = hpMpOffsets[field];
        check(static_cast<unsigned char>(hpMp[offset]) == 0x10 + field &&
            static_cast<unsigned char>(hpMp[offset + 1]) == 0x20 + field &&
            static_cast<unsigned char>(hpMp[offset + 2]) == 0x30 + field &&
            static_cast<unsigned char>(hpMp[offset + 3]) == 0x40 + field,
            "coordinated HP/MP preserves the uint32 resource at the contracted offset");
    }
    check(hpMp == hpMpBefore, "gate preserves every coordinated HP/MP byte");

    // The active 0x336 snapshot is 232 bytes. The markers cover each structural
    // block without depending on the C++ constructor and tell the current frame
    // apart from the 92-byte base and the historical 236-byte XSC2.
    std::array<char, kUpdateScorePacketSize + 5> updateScore{};
    updateScore[0] = static_cast<char>(kUpdateScorePacketSize);
    updateScore[4] = static_cast<char>(MSG_UpdateScore_Opcode & 0xFF);
    updateScore[5] = static_cast<char>((MSG_UpdateScore_Opcode >> 8) & 0xFF);
    updateScore[6] = 0x34;
    updateScore[7] = 0x12;
    updateScore[kUpdateScoreScoreOffset] = 0x11;
    updateScore[kUpdateScoreAffectOffset] = 0x22;
    updateScore[kUpdateScoreAffectOffset + (kUpdateScoreAffectCount - 1) * 2] = 0x23;
    updateScore[kUpdateScoreGuildOffset] = 0x31;
    updateScore[kUpdateScoreGuildLevelOffset] = 0x32;
    updateScore[kUpdateScoreReqHpOffset] = 0x41;
    updateScore[kUpdateScoreReqMpOffset] = 0x42;
    updateScore[kUpdateScoreLearnedSkillOffset] = 0x51;
    const auto updateScoreBefore = updateScore;
    int updateScoreCalls = 0;
    const auto receiveUpdateScore = [&](const PacketView& view) {
        ++updateScoreCalls;
        check(view.data == updateScore.data() && view.size == kUpdateScorePacketSize &&
            view.opcode == MSG_UpdateScore_Opcode,
            "coordinated UpdateScore preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kUpdateScorePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_UpdateScore_Opcode, updateScore.data(), n},
            receiveUpdateScore), "UpdateScore rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_UpdateScore_Opcode, nullptr,
        kUpdateScorePacketSize}, receiveUpdateScore), "UpdateScore rejects a null buffer");
    check(!received_packet::Dispatch({MSG_UpdateScore_Opcode, updateScore.data(),
        kUpdateScorePacketSize + 1}, receiveUpdateScore), "UpdateScore rejects an oversized frame");
    check(!received_packet::Dispatch({MSG_UpdateScore_Opcode, updateScore.data(), 236},
        receiveUpdateScore), "UpdateScore rejects the historical XSC2 extension");
    check(!received_packet::Dispatch({0x119, updateScore.data(), kUpdateScorePacketSize},
        receiveUpdateScore), "an outer opcode cannot hide UpdateScore");
    updateScore[4] = static_cast<char>((MSG_UpdateScore_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_UpdateScore_Opcode, updateScore.data(),
        kUpdateScorePacketSize}, receiveUpdateScore),
        "UpdateScore rejects a mismatched Header.Type");
    updateScore[4] = static_cast<char>(MSG_UpdateScore_Opcode & 0xFF);
    updateScore[0] = static_cast<char>(kUpdateScorePacketSize - 1);
    check(!received_packet::Dispatch({MSG_UpdateScore_Opcode, updateScore.data(),
        kUpdateScorePacketSize}, receiveUpdateScore),
        "UpdateScore rejects a mismatched Header.Size");
    updateScore[0] = static_cast<char>(kUpdateScorePacketSize);
    check(updateScoreCalls == 0, "invalid UpdateScore does not reach the consumer");
    check(received_packet::Dispatch({MSG_UpdateScore_Opcode, updateScore.data(),
        kUpdateScorePacketSize}, receiveUpdateScore) && updateScoreCalls == 1,
        "coordinated UpdateScore delivered once");
    check(static_cast<unsigned char>(updateScore[6]) == 0x34 &&
        static_cast<unsigned char>(updateScore[7]) == 0x12,
        "UpdateScore preserves the receiver in Header.ID");
    check(updateScore[kUpdateScoreScoreOffset] == 0x11 &&
        updateScore[kUpdateScoreAffectOffset] == 0x22 &&
        updateScore[kUpdateScoreAffectOffset + (kUpdateScoreAffectCount - 1) * 2] == 0x23 &&
        updateScore[kUpdateScoreGuildOffset] == 0x31 &&
        updateScore[kUpdateScoreGuildLevelOffset] == 0x32 &&
        updateScore[kUpdateScoreReqHpOffset] == 0x41 &&
        updateScore[kUpdateScoreReqMpOffset] == 0x42 &&
        updateScore[kUpdateScoreLearnedSkillOffset] == 0x51,
        "UpdateScore preserves the bounds and fields of the coordinated payload");
    check(updateScore == updateScoreBefore,
        "gate preserves every coordinated UpdateScore byte");

    // Native snapshot: 64 structural items and a final Coin. Slot 63 remains
    // on the wire, although the character grid only draws 0..62.
    std::array<char, kCarrySnapshotPacketSize + 1> carry{};
    carry[0] = static_cast<char>(kCarrySnapshotPacketSize & 0xFF);
    carry[1] = static_cast<char>((kCarrySnapshotPacketSize >> 8) & 0xFF);
    carry[4] = static_cast<char>(MSG_UpdateCarry_Opcode & 0xFF);
    carry[5] = static_cast<char>((MSG_UpdateCarry_Opcode >> 8) & 0xFF);
    carry[6] = 0x34;
    carry[7] = 0x12;
    carry[kCarrySnapshotItemsOffset] = 0x11;
    carry[kCarrySnapshotItemsOffset + 62 * kCarrySnapshotItemSize] = 0x22;
    carry[kCarrySnapshotItemsOffset + 63 * kCarrySnapshotItemSize] = 0x23;
    carry[kCarrySnapshotCoinOffset] = 0x31;
    carry[kCarrySnapshotCoinOffset + 1] = 0x32;
    carry[kCarrySnapshotCoinOffset + 2] = 0x33;
    carry[kCarrySnapshotCoinOffset + 3] = 0x34;
    const auto carryBefore = carry;
    int carryCalls = 0;
    const auto receiveCarry = [&](const PacketView& view) {
        ++carryCalls;
        check(view.data == carry.data() && view.size == kCarrySnapshotPacketSize &&
            view.opcode == MSG_UpdateCarry_Opcode,
            "UpdateCarry preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kCarrySnapshotPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_UpdateCarry_Opcode, carry.data(), n},
            receiveCarry), "UpdateCarry rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_UpdateCarry_Opcode, nullptr,
        kCarrySnapshotPacketSize}, receiveCarry), "UpdateCarry rejects a null buffer");
    check(!received_packet::Dispatch({MSG_UpdateCarry_Opcode, carry.data(),
        kCarrySnapshotPacketSize + 1}, receiveCarry), "UpdateCarry rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, carry.data(), kCarrySnapshotPacketSize},
        receiveCarry), "an outer opcode cannot hide UpdateCarry");
    carry[4] = static_cast<char>((MSG_UpdateCarry_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_UpdateCarry_Opcode, carry.data(),
        kCarrySnapshotPacketSize}, receiveCarry), "UpdateCarry rejects a mismatched Header.Type");
    carry[4] = static_cast<char>(MSG_UpdateCarry_Opcode & 0xFF);
    carry[0] = static_cast<char>((kCarrySnapshotPacketSize - 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_UpdateCarry_Opcode, carry.data(),
        kCarrySnapshotPacketSize}, receiveCarry), "UpdateCarry rejects a mismatched Header.Size");
    carry[0] = static_cast<char>(kCarrySnapshotPacketSize & 0xFF);
    check(carryCalls == 0, "invalid UpdateCarry does not reach the consumer");
    check(received_packet::Dispatch({MSG_UpdateCarry_Opcode, carry.data(),
        kCarrySnapshotPacketSize}, receiveCarry) && carryCalls == 1,
        "valid UpdateCarry delivered once");
    check(static_cast<unsigned char>(carry[6]) == 0x34 &&
        static_cast<unsigned char>(carry[7]) == 0x12,
        "UpdateCarry preserves the receiver in Header.ID");
    check(carry[kCarrySnapshotItemsOffset] == 0x11 &&
        carry[kCarrySnapshotItemsOffset + 62 * kCarrySnapshotItemSize] == 0x22 &&
        carry[kCarrySnapshotItemsOffset + 63 * kCarrySnapshotItemSize] == 0x23,
        "UpdateCarry preserves the first slot, the last visible slot and the structural slot");
    check(carry[kCarrySnapshotCoinOffset] == 0x31 &&
        carry[kCarrySnapshotCoinOffset + 1] == 0x32 &&
        carry[kCarrySnapshotCoinOffset + 2] == 0x33 &&
        carry[kCarrySnapshotCoinOffset + 3] == 0x34,
        "UpdateCarry preserves Coin at offset 524");
    check(carry == carryBefore, "gate preserves every UpdateCarry byte");

    // The equipment visual carries two parallel 16-entry arrays.
    // The gate also keeps the structural positions that the server projects
    // as empty when the 7.48 UI has no matching slot.
    std::array<char, kUpdateEquipPacketSize + 1> updateEquip{};
    updateEquip[0] = static_cast<char>(kUpdateEquipPacketSize);
    updateEquip[4] = static_cast<char>(MSG_UpdateEquip_Opcode & 0xFF);
    updateEquip[5] = static_cast<char>((MSG_UpdateEquip_Opcode >> 8) & 0xFF);
    updateEquip[6] = 0x34;
    updateEquip[7] = 0x12;
    updateEquip[kUpdateEquipVisualOffset] = 0x11;
    updateEquip[kUpdateEquipVisualOffset +
        (kUpdateEquipSlotCount - 1) * kUpdateEquipVisualSize] = 0x12;
    updateEquip[kUpdateEquipAncientOffset] = 0x21;
    updateEquip[kUpdateEquipAncientOffset +
        (kUpdateEquipSlotCount - 1) * kUpdateEquipAncientSize] = 0x22;
    const auto updateEquipBefore = updateEquip;
    int updateEquipCalls = 0;
    const auto receiveUpdateEquip = [&](const PacketView& view) {
        ++updateEquipCalls;
        check(view.data == updateEquip.data() && view.size == kUpdateEquipPacketSize &&
            view.opcode == MSG_UpdateEquip_Opcode,
            "UpdateEquip preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kUpdateEquipPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_UpdateEquip_Opcode, updateEquip.data(), n},
            receiveUpdateEquip), "UpdateEquip rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_UpdateEquip_Opcode, nullptr,
        kUpdateEquipPacketSize}, receiveUpdateEquip), "UpdateEquip rejects a null buffer");
    check(!received_packet::Dispatch({MSG_UpdateEquip_Opcode, updateEquip.data(),
        kUpdateEquipPacketSize + 1}, receiveUpdateEquip), "UpdateEquip rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, updateEquip.data(), kUpdateEquipPacketSize},
        receiveUpdateEquip), "an outer opcode cannot hide UpdateEquip");
    updateEquip[4] = static_cast<char>((MSG_UpdateEquip_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_UpdateEquip_Opcode, updateEquip.data(),
        kUpdateEquipPacketSize}, receiveUpdateEquip),
        "UpdateEquip rejects a mismatched Header.Type");
    updateEquip[4] = static_cast<char>(MSG_UpdateEquip_Opcode & 0xFF);
    updateEquip[0] = static_cast<char>(kUpdateEquipPacketSize - 1);
    check(!received_packet::Dispatch({MSG_UpdateEquip_Opcode, updateEquip.data(),
        kUpdateEquipPacketSize}, receiveUpdateEquip),
        "UpdateEquip rejects a mismatched Header.Size");
    updateEquip[0] = static_cast<char>(kUpdateEquipPacketSize);
    check(updateEquipCalls == 0, "invalid UpdateEquip does not reach the consumer");
    check(received_packet::Dispatch({MSG_UpdateEquip_Opcode, updateEquip.data(),
        kUpdateEquipPacketSize}, receiveUpdateEquip) && updateEquipCalls == 1,
        "valid UpdateEquip delivered once");
    check(static_cast<unsigned char>(updateEquip[6]) == 0x34 &&
        static_cast<unsigned char>(updateEquip[7]) == 0x12,
        "UpdateEquip preserves the receiver in Header.ID");
    check(updateEquip[kUpdateEquipVisualOffset] == 0x11 &&
        updateEquip[kUpdateEquipVisualOffset +
            (kUpdateEquipSlotCount - 1) * kUpdateEquipVisualSize] == 0x12 &&
        updateEquip[kUpdateEquipAncientOffset] == 0x21 &&
        updateEquip[kUpdateEquipAncientOffset +
            (kUpdateEquipSlotCount - 1) * kUpdateEquipAncientSize] == 0x22,
        "UpdateEquip preserves the bounds of the visual and AnctCode arrays");
    check(updateEquip == updateEquipBefore,
        "gate preserves every UpdateEquip byte");

    // The 7.48 consumer walks exactly 16 eight-byte affects. The markers
    // of the first and last record pin every offset used by the coordinated
    // STRUCT_AFFECT layout.
    std::array<char, kUpdateAffectPacketSize + 1> updateAffect{};
    updateAffect[0] = static_cast<char>(kUpdateAffectPacketSize);
    updateAffect[4] = static_cast<char>(MSG_UpdateAffect_Opcode & 0xFF);
    updateAffect[5] = static_cast<char>((MSG_UpdateAffect_Opcode >> 8) & 0xFF);
    updateAffect[6] = 0x34;
    updateAffect[7] = 0x12;
    const auto lastAffect = kUpdateAffectArrayOffset +
        (kUpdateAffectCount - 1) * kUpdateAffectEntrySize;
    updateAffect[kUpdateAffectArrayOffset + kUpdateAffectTypeOffset] = 0x11;
    updateAffect[kUpdateAffectArrayOffset + kUpdateAffectLevelOffset] = 0x12;
    updateAffect[kUpdateAffectArrayOffset + kUpdateAffectValueOffset] = 0x13;
    updateAffect[kUpdateAffectArrayOffset + kUpdateAffectTimeOffset] = 0x14;
    updateAffect[lastAffect + kUpdateAffectTypeOffset] = 0x21;
    updateAffect[lastAffect + kUpdateAffectLevelOffset] = 0x22;
    updateAffect[lastAffect + kUpdateAffectValueOffset] = 0x23;
    updateAffect[lastAffect + kUpdateAffectTimeOffset] = 0x24;
    const auto updateAffectBefore = updateAffect;
    int updateAffectCalls = 0;
    const auto receiveUpdateAffect = [&](const PacketView& view) {
        ++updateAffectCalls;
        check(view.data == updateAffect.data() && view.size == kUpdateAffectPacketSize &&
            view.opcode == MSG_UpdateAffect_Opcode,
            "UpdateAffect preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kUpdateAffectPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_UpdateAffect_Opcode, updateAffect.data(), n},
            receiveUpdateAffect), "UpdateAffect rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_UpdateAffect_Opcode, nullptr,
        kUpdateAffectPacketSize}, receiveUpdateAffect), "UpdateAffect rejects a null buffer");
    check(!received_packet::Dispatch({MSG_UpdateAffect_Opcode, updateAffect.data(),
        kUpdateAffectPacketSize + 1}, receiveUpdateAffect),
        "UpdateAffect rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, updateAffect.data(), kUpdateAffectPacketSize},
        receiveUpdateAffect), "an outer opcode cannot hide UpdateAffect");
    updateAffect[4] = static_cast<char>((MSG_UpdateAffect_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_UpdateAffect_Opcode, updateAffect.data(),
        kUpdateAffectPacketSize}, receiveUpdateAffect),
        "UpdateAffect rejects a mismatched Header.Type");
    updateAffect[4] = static_cast<char>(MSG_UpdateAffect_Opcode & 0xFF);
    updateAffect[0] = static_cast<char>(kUpdateAffectPacketSize - 1);
    check(!received_packet::Dispatch({MSG_UpdateAffect_Opcode, updateAffect.data(),
        kUpdateAffectPacketSize}, receiveUpdateAffect),
        "UpdateAffect rejects a mismatched Header.Size");
    updateAffect[0] = static_cast<char>(kUpdateAffectPacketSize);
    check(updateAffectCalls == 0, "invalid UpdateAffect does not reach the consumer");
    check(received_packet::Dispatch({MSG_UpdateAffect_Opcode, updateAffect.data(),
        kUpdateAffectPacketSize}, receiveUpdateAffect) && updateAffectCalls == 1,
        "valid UpdateAffect delivered once");
    check(static_cast<unsigned char>(updateAffect[6]) == 0x34 &&
        static_cast<unsigned char>(updateAffect[7]) == 0x12,
        "UpdateAffect preserves the receiver in Header.ID");
    check(updateAffect[kUpdateAffectArrayOffset + kUpdateAffectTypeOffset] == 0x11 &&
        updateAffect[kUpdateAffectArrayOffset + kUpdateAffectLevelOffset] == 0x12 &&
        updateAffect[kUpdateAffectArrayOffset + kUpdateAffectValueOffset] == 0x13 &&
        updateAffect[kUpdateAffectArrayOffset + kUpdateAffectTimeOffset] == 0x14 &&
        updateAffect[lastAffect + kUpdateAffectTypeOffset] == 0x21 &&
        updateAffect[lastAffect + kUpdateAffectLevelOffset] == 0x22 &&
        updateAffect[lastAffect + kUpdateAffectValueOffset] == 0x23 &&
        updateAffect[lastAffect + kUpdateAffectTimeOffset] == 0x24,
        "UpdateAffect preserves the fields of the first and last record");
    check(updateAffect == updateAffectBefore,
        "gate preserves every UpdateAffect byte");

    // CreateMob and CreateMobTrade share the coordinated entity prefix;
    // the second adds a 24-byte title. Each format has a unique size
    // so the handler cannot read missing or mixed blocks.
    return failures;
}
