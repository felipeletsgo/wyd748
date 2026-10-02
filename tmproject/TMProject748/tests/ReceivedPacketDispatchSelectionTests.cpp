#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: Selection. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchSelectionTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    check(IsRouteCorrectionAction(MSG_Action_Opcode, kActionRouteCorrectionEffect),
          "paired route correction is recognized on the action opcode");
    check(!IsRouteCorrectionAction(MSG_Action_Stop_Opcode, kActionRouteCorrectionEffect) &&
          !IsRouteCorrectionAction(MSG_Action_Opcode, 1),
          "route correction cannot alias native stop or teleport effects");

    // Every envelope emitted during login/selection passes through the same
    // global boundary before the scenes' large casts.
    struct SelectionEnvelope { unsigned int opcode; std::size_t size; };
    const std::array<SelectionEnvelope, 6> selectionEnvelopes{{
        {MSG_CNFAccountLogin_Opcode, kAccountLoginConfirmPacketSize},
        {MSG_CNFNewCharacter_Opcode, kCharacterSelectionUpdatePacketSize},
        {MSG_CNFDeleteCharacter_Opcode, kCharacterSelectionUpdatePacketSize},
        {MSG_CNFCharacterLogin_Opcode, kCharacterLoginConfirmPacketSize},
        {MSG_CNFNewCharacterFail_Opcode, kSelectionFailurePacketSize},
        {MSG_AlreadyPlaying_Opcode, kSelectionFailurePacketSize},
    }};
    for (const auto& contract : selectionEnvelopes)
    {
        std::vector<char> packet(contract.size + 1, 0);
        packet[0] = static_cast<char>(contract.size & 0xFF);
        packet[1] = static_cast<char>((contract.size >> 8) & 0xFF);
        packet[4] = static_cast<char>(contract.opcode & 0xFF);
        packet[5] = static_cast<char>((contract.opcode >> 8) & 0xFF);
        const auto originalPacket = packet;
        int selectionCalls = 0;
        const auto receiveSelection = [&](const PacketView& frame) {
            ++selectionCalls;
            check(frame.data == packet.data() && frame.size == contract.size &&
                frame.opcode == contract.opcode,
                "selection dispatch preserves the transport view");
        };

        for (std::size_t size = 0; size < contract.size; ++size)
            check(!received_packet::Dispatch({contract.opcode, packet.data(), size},
                receiveSelection), "every truncated selection prefix is rejected");
        check(!received_packet::Dispatch({contract.opcode, nullptr, contract.size},
            receiveSelection), "null selection frame rejected");
        check(!received_packet::Dispatch({contract.opcode, packet.data(), contract.size + 1},
            receiveSelection), "oversized selection frame rejected");
        check(!received_packet::Dispatch({0x119, packet.data(), contract.size},
            receiveSelection), "metadata cannot hide the selection Type");

        packet[0] = static_cast<char>((contract.size - 1) & 0xFF);
        packet[1] = static_cast<char>(((contract.size - 1) >> 8) & 0xFF);
        check(!received_packet::Dispatch({contract.opcode, packet.data(), contract.size},
            receiveSelection), "selection rejects a mismatched declared Size");
        packet[0] = static_cast<char>(contract.size & 0xFF);
        packet[1] = static_cast<char>((contract.size >> 8) & 0xFF);

        packet[4] = 0x19;
        packet[5] = 0x01;
        check(!received_packet::Dispatch({contract.opcode, packet.data(), contract.size},
            receiveSelection), "selection rejects a mismatched inner Type");
        packet[4] = static_cast<char>(contract.opcode & 0xFF);
        packet[5] = static_cast<char>((contract.opcode >> 8) & 0xFF);

        check(selectionCalls == 0, "invalid selection frame does not call the consumer");
        check(received_packet::Dispatch({contract.opcode, packet.data(), contract.size},
            receiveSelection) && selectionCalls == 1,
            "exact selection frame delivered once");
        check(packet == originalPacket, "selection gate does not modify the buffer");
    }

    // Inventory of the fixed envelopes actually produced by WYD-Go. The
    // numeric sizes deliberately mirror the Go builders: using sizeof of the
    // C++ structs here would hide a mismatch between the two peers.
    struct ActiveServerEnvelope { unsigned int opcode; std::size_t size; };
    const ActiveServerEnvelope activeServerEnvelopes[] = {
        {MSG_MessagePanel_Opcode, 108},
        {MSG_MessageIndexed_Opcode, 108},
        {MSG_MessageParameterized_Opcode, 108},
        {MSG_REQArray_Opcode, 24},
        {MSG_CNFAccountLogin_Opcode, 2360},
        {MSG_CNFNewCharacter_Opcode, 1288},
        {MSG_CNFDeleteCharacter_Opcode, 1288},
        {MSG_CNFCharacterLogin_Opcode, 2104},
        {MSG_CNFNewCharacterFail_Opcode, 12},
        {MSG_AlreadyPlaying_Opcode, 12},
        {MSG_CNFCharacterLogout_Opcode, 12},
        {MSG_CreateMobTrade_Opcode, 352},
        {MSG_CreateMob_Opcode, 328},
        {MSG_SetHpMp_Opcode, 28},
        {MSG_UpdateScore_Opcode, 232},
        {MSG_UpdateAffect_Opcode, 140},
        {MSG_UpdateEtc_Opcode, 36},
        {MSG_CNFMobKill_Opcode, 24},
        {MSG_UpdateCarry_Opcode, 528},
        {MSG_UpdateCargoGold_Opcode, 16},
        {MSG_UpdateEquip_Opcode, 60},
        {MSG_Action_Opcode, 52},
        {MSG_Action2_Opcode, 52},
        {MSG_Motion_Opcode, 20},
        {MSG_InstanceTime_Opcode, 16},
        {MSG_InstanceMobs_Opcode, 16},
        {MSG_SwapItem_Opcode, 20},
        {MSG_Withdraw_Opcode, 16},
        {MSG_Deposit_Opcode, 16},
        {MSG_PremiumFirework_Opcode, 36},
        {MSG_ResultGamble_Opcode, 36},
        {MSG_CapsuleInfo_Opcode, 52},
        {MSG_SendItem_Opcode, 24},
        {MSG_WarInfo_Opcode, 24},
        {MSG_CNFDropItem_Opcode, 28},
        {MSG_CreateItem_Opcode, 32},
        {MSG_CNFGetItem_Opcode, 28},
        {MSG_RemoveItem_Opcode, 16},
        {MSG_UpdateItem_Opcode, 20},
        {MSG_MessageChat_Opcode, 108},
        {MSG_MessageWhisper_Opcode, 128},
        {MSG_SetShortSkill_Opcode, 32},
        {MSG_DelayStart_Opcode, 16},
        {MSG_ReqChallenge_Opcode, 12},
        {MSG_ShopList_Opcode, 236},
        {MSG_Buy_Opcode, 24},
        {MSG_AddParty_Opcode, 40},
        {MSG_RemoveParty_Opcode, 16},
        {MSG_REQParty_Opcode, 44},
        {MSG_Trade_Opcode, 156},
        {MSG_CloseTrade_Opcode, 12},
        {MSG_CNFTradeCheck_Opcode, 12},
        {MSG_AutoTrade_Opcode, 196},
        {MSG_ItemSold_Opcode, 20},
        {MSG_PlayerChallenge_Opcode, 20},
        {MSG_CombineComplete_Opcode, 16},
        {MSG_RemoveMob_Opcode, 16},
        {quiz_event::ChallengeOpcode, 148},
    };
    for (const auto& contract : activeServerEnvelopes)
    {
        char description[96]{};
        std::snprintf(description, sizeof(description),
            "active emitter 0x%X has an S->C gate of %zu bytes",
            contract.opcode, contract.size);
        check(received_packet::ExpectedSize(contract.opcode) == contract.size,
            description);
    }

    // The server sends the challenge invitation as MSG_STANDARDPARM2. The handler
    // reads both DWORDs, so no prefix shorter than 20 bytes is safe.
    std::array<char, kPlayerChallengePacketSize + 1> playerChallenge{};
    playerChallenge[0] = static_cast<char>(kPlayerChallengePacketSize);
    playerChallenge[4] = static_cast<char>(MSG_PlayerChallenge_Opcode & 0xFF);
    playerChallenge[5] = static_cast<char>((MSG_PlayerChallenge_Opcode >> 8) & 0xFF);
    playerChallenge[kPlayerChallengePlayerIdOffset] = 0x34;
    playerChallenge[kPlayerChallengePlayerIdOffset + 1] = 0x12;
    playerChallenge[kPlayerChallengeModeOffset] = 3;
    const auto originalPlayerChallenge = playerChallenge;
    int playerChallengeCalls = 0;
    const auto receivePlayerChallenge = [&](const PacketView& frame) {
        ++playerChallengeCalls;
        check(frame.data == playerChallenge.data() &&
            frame.size == kPlayerChallengePacketSize &&
            frame.opcode == MSG_PlayerChallenge_Opcode,
            "challenge preserves the 20-byte transport view");
        check(static_cast<unsigned char>(frame.data[kPlayerChallengePlayerIdOffset]) == 0x34 &&
            static_cast<unsigned char>(frame.data[kPlayerChallengeModeOffset]) == 3,
            "challenge preserves player and mode at the contract offsets");
    };
    check(received_packet::ExpectedSize(MSG_PlayerChallenge_Opcode) ==
        kPlayerChallengePacketSize, "desafio possui tamanho exato no dispatcher");
    for (std::size_t size = 0; size < kPlayerChallengePacketSize; ++size)
        check(!received_packet::Dispatch({MSG_PlayerChallenge_Opcode,
            playerChallenge.data(), size}, receivePlayerChallenge),
            "every truncated challenge prefix is rejected");
    check(!received_packet::Dispatch({MSG_PlayerChallenge_Opcode, nullptr,
        kPlayerChallengePacketSize}, receivePlayerChallenge),
        "challenge with a null frame rejected");
    check(!received_packet::Dispatch({MSG_PlayerChallenge_Opcode,
        playerChallenge.data(), kPlayerChallengePacketSize + 1}, receivePlayerChallenge),
        "challenge with an oversized frame rejected");
    check(!received_packet::Dispatch({0x119, playerChallenge.data(),
        kPlayerChallengePacketSize}, receivePlayerChallenge),
        "unknown metadata does not hide the challenge Type");

    playerChallenge[0] = static_cast<char>(kPlayerChallengePacketSize - 1);
    check(!received_packet::Dispatch({MSG_PlayerChallenge_Opcode,
        playerChallenge.data(), kPlayerChallengePacketSize}, receivePlayerChallenge),
        "challenge rejects a mismatched declared Size");
    playerChallenge[0] = static_cast<char>(kPlayerChallengePacketSize);
    playerChallenge[4] = 0x19;
    playerChallenge[5] = 0x01;
    check(!received_packet::Dispatch({MSG_PlayerChallenge_Opcode,
        playerChallenge.data(), kPlayerChallengePacketSize}, receivePlayerChallenge),
        "challenge rejects a mismatched inner Type");
    playerChallenge[4] = static_cast<char>(MSG_PlayerChallenge_Opcode & 0xFF);
    playerChallenge[5] = static_cast<char>((MSG_PlayerChallenge_Opcode >> 8) & 0xFF);

    check(playerChallengeCalls == 0, "invalid challenge does not call the consumer");
    check(received_packet::Dispatch({MSG_PlayerChallenge_Opcode,
        playerChallenge.data(), kPlayerChallengePacketSize}, receivePlayerChallenge) &&
        playerChallengeCalls == 1, "exact challenge delivered once");
    check(playerChallenge == originalPlayerChallenge,
        "challenge gate does not modify the buffer");

    // Both selection emitters use this copy before sending 0xFAA.
    char oldTransferName[16] = "OldCharacter";
    char newTransferName[16] = "NewCharacter";
    MSG_ReqTransper outgoing{};
    check(character_transfer::CopyRequestNames(outgoing, oldTransferName,
        newTransferName, sizeof(newTransferName)),
        "valid transfer names are accepted");
    check(std::memcmp(outgoing.OldName, oldTransferName, sizeof(oldTransferName)) == 0 &&
        std::memcmp(outgoing.NewName, newTransferName, sizeof(newTransferName)) == 0,
        "transfer names preserve wire bytes and padding");
    const MSG_ReqTransper validOutgoing = outgoing;
    char unterminatedTransferName[16];
    std::memset(unterminatedTransferName, 'X', sizeof(unterminatedTransferName));
    check(!character_transfer::CopyRequestNames(outgoing, unterminatedTransferName,
        newTransferName, sizeof(newTransferName)) &&
        std::memcmp(&outgoing, &validOutgoing, sizeof(outgoing)) == 0,
        "old name without NUL is not read past its cell and does not change the packet");
    check(!character_transfer::CopyRequestNames(outgoing, oldTransferName,
        unterminatedTransferName, sizeof(unterminatedTransferName)) &&
        std::memcmp(&outgoing, &validOutgoing, sizeof(outgoing)) == 0,
        "new name without NUL does not change the packet");
    check(!character_transfer::CopyRequestNames(outgoing, oldTransferName,
        newTransferName, 4) &&
        std::memcmp(&outgoing, &validOutgoing, sizeof(outgoing)) == 0,
        "truncated declared capacity rejected before the copy");

    // The leading byte deliberately shifts the frame to an unaligned address.
    alignas(MSG_STANDARD) std::array<char, 54> storage{};
    char* bytes = storage.data() + 1;
    bytes[0] = 52;
    bytes[4] = static_cast<char>(0xAA);
    bytes[5] = 0x0F;
    for (int i = 12; i < 16; ++i) bytes[i] = static_cast<char>(0xFF);
    bytes[16] = 3;
    std::memcpy(bytes + 20, "OldCharacter", 12);
    std::memcpy(bytes + 36, "NewCharacter", 12);
    const auto original = storage;

    MSG_ReqTransper decoded{};
    std::memcpy(&decoded, bytes, 52);
    check(decoded.Header.Type == 0xFAA && decoded.Header.Size == 52,
        "known bytes preserve the transfer envelope");
    check(decoded.Result == -1 && decoded.Slot == 3,
        "known bytes preserve offsets and signedness");
    check(std::memcmp(decoded.OldName, "OldCharacter", 12) == 0 &&
        std::memcmp(decoded.NewName, "NewCharacter", 12) == 0,
        "known bytes preserve both names");

    int delivered = 0;
    const auto receive = [&](const PacketView& frame) {
        ++delivered;
        check(frame.data == bytes && frame.size == 52 && frame.opcode == 0xFAA,
            "dispatch preserves address, size and opcode without copying");
    };
    for (std::size_t size = 0; size < 52; ++size)
        check(!received_packet::Dispatch({0xFAA, bytes, size}, receive),
            "every truncated prefix is rejected before the consumer");
    check(!received_packet::Dispatch({0xFAA, nullptr, 52}, receive),
        "null frame rejected");
    check(!received_packet::Dispatch({0xFAA, bytes, 53}, receive),
        "oversized transfer frame rejected");
    check(delivered == 0, "rejections do not change scene state through the callback");
    check(received_packet::Dispatch({0xFAA, bytes, 52}, receive) && delivered == 1,
        "exact frame delivered once");
    check(storage == original, "validation does not modify transport bytes");

    bytes[0] = 51;
    check(!received_packet::Dispatch({0xFAA, bytes, 52}, receive),
        "mismatched declared size rejected");
    bytes[0] = 52;
    check(!received_packet::Dispatch({0x119, bytes, 52}, receive),
        "metadata cannot hide the transfer Type");
    bytes[4] = 0x19;
    bytes[5] = 0x01;
    check(!received_packet::Dispatch({0xFAA, bytes, 52}, receive),
        "metadado transferencia exige Type correspondente");
    check(delivered == 1, "mismatches run neither the callback nor a retry");

    int otherDelivered = 0;
    check(received_packet::Dispatch({0x119, bytes, 12}, [&](const PacketView& frame) {
        ++otherDelivered;
        check(frame.data == bytes && frame.size == 12 && frame.opcode == 0x119,
            "another opcode preserves the legacy path view");
    }) && otherDelivered == 1, "an opcode outside this batch keeps the fallback");

    // The native city-war prompt is only the MSG_STANDARD (0x18D).
    std::array<char, sizeof(MSG_STANDARD) + 1> challengePrompt{};
    challengePrompt[0] = static_cast<char>(sizeof(MSG_STANDARD));
    challengePrompt[4] = static_cast<char>(MSG_ReqChallenge_Opcode & 0xFF);
    challengePrompt[5] = static_cast<char>((MSG_ReqChallenge_Opcode >> 8) & 0xFF);
    int challengePromptCalls = 0;
    const auto receiveChallengePrompt = [&](const PacketView& frame) {
        ++challengePromptCalls;
        check(frame.data == challengePrompt.data() && frame.size == sizeof(MSG_STANDARD) &&
            frame.opcode == MSG_ReqChallenge_Opcode,
            "contest prompt preserves the native 12-byte frame");
    };
    for (std::size_t n = 0; n < sizeof(MSG_STANDARD); ++n)
        check(!received_packet::Dispatch({MSG_ReqChallenge_Opcode, challengePrompt.data(), n},
            receiveChallengePrompt), "truncated contest prompt rejected");
    check(!received_packet::Dispatch({MSG_ReqChallenge_Opcode, challengePrompt.data(),
        sizeof(MSG_STANDARD) + 1}, receiveChallengePrompt),
        "oversized contest prompt rejected");
    check(!received_packet::Dispatch({0x119, challengePrompt.data(), sizeof(MSG_STANDARD)},
        receiveChallengePrompt), "contest prompt does not accept a mismatched opcode");
    challengePrompt[0] = static_cast<char>(sizeof(MSG_STANDARD) - 1);
    check(!received_packet::Dispatch({MSG_ReqChallenge_Opcode, challengePrompt.data(),
        sizeof(MSG_STANDARD)}, receiveChallengePrompt),
        "contest prompt rejects a mismatched Size");
    challengePrompt[0] = static_cast<char>(sizeof(MSG_STANDARD));
    check(challengePromptCalls == 0, "invalid contest prompt does not call the consumer");
    check(received_packet::Dispatch({MSG_ReqChallenge_Opcode, challengePrompt.data(),
        sizeof(MSG_STANDARD)}, receiveChallengePrompt) && challengePromptCalls == 1,
        "exact contest prompt delivered once");

    // Same canonical fixture consumed by the Go test. It does not depend on the Basedef struct.
    return failures;
}
