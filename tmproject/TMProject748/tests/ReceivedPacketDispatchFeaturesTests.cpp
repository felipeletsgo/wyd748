#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: Features. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchFeaturesTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    std::array<char, kAutoTradePacketSize + 1> autoTrade{};
    autoTrade[0] = static_cast<char>(kAutoTradePacketSize);
    autoTrade[4] = static_cast<char>(MSG_AutoTrade_Opcode & 0xFF);
    autoTrade[5] = static_cast<char>((MSG_AutoTrade_Opcode >> 8) & 0xFF);
    autoTrade[6] = 0x34;
    autoTrade[7] = 0x12;
    autoTrade[kAutoTradeDescriptionOffset] = 'L';
    autoTrade[kAutoTradeDescriptionOffset + kAutoTradeDescriptionSize - 1] = 0;
    autoTrade[kAutoTradeItemsOffset] = 0x21;
    autoTrade[kAutoTradeItemsOffset +
        (kAutoTradeItemCount - 1) * kAutoTradeItemSize] = 0x32;
    autoTrade[kAutoTradeCarryPositionsOffset] = 0x43;
    autoTrade[kAutoTradePricesOffset] = 0x54;
    autoTrade[kAutoTradePricesOffset +
        (kAutoTradeItemCount - 1) * kAutoTradePriceSize] = 0x65;
    autoTrade[kAutoTradeTaxOffset] = 0x76;
    autoTrade[kAutoTradeTargetIdOffset] = static_cast<char>(0x87);
    const auto autoTradeBefore = autoTrade;
    int autoTradeCalls = 0;
    const auto receiveAutoTrade = [&](const PacketView& view) {
        ++autoTradeCalls;
        check(view.data == autoTrade.data() && view.size == kAutoTradePacketSize &&
            view.opcode == MSG_AutoTrade_Opcode,
            "AutoTrade preserves frame and opcode");
    };
    check(received_packet::ExpectedSize(MSG_AutoTrade_Opcode) ==
        kAutoTradePacketSize,
        "AutoTrade publica tamanho esperado no gate");
    for (std::size_t n = 0; n < kAutoTradePacketSize; ++n)
        check(!received_packet::Dispatch(
            {MSG_AutoTrade_Opcode, autoTrade.data(), n}, receiveAutoTrade),
            "AutoTrade rejects every truncated prefix");
    check(!received_packet::Dispatch(
        {MSG_AutoTrade_Opcode, nullptr, kAutoTradePacketSize}, receiveAutoTrade),
        "AutoTrade rejects a null buffer");
    check(!received_packet::Dispatch(
        {MSG_AutoTrade_Opcode, autoTrade.data(), kAutoTradePacketSize + 1},
        receiveAutoTrade), "AutoTrade rejects an oversized frame");
    check(!received_packet::Dispatch(
        {0x119, autoTrade.data(), kAutoTradePacketSize}, receiveAutoTrade),
        "an outer opcode cannot hide AutoTrade");
    autoTrade[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
    autoTrade[5] = static_cast<char>((MSG_SetShortSkill_Opcode >> 8) & 0xFF);
    check(!received_packet::Dispatch(
        {MSG_AutoTrade_Opcode, autoTrade.data(), kAutoTradePacketSize},
        receiveAutoTrade), "AutoTrade rejects a mismatched Header.Type");
    autoTrade[4] = static_cast<char>(MSG_AutoTrade_Opcode & 0xFF);
    autoTrade[5] = static_cast<char>((MSG_AutoTrade_Opcode >> 8) & 0xFF);
    autoTrade[0] = static_cast<char>(kAutoTradePacketSize - 1);
    check(!received_packet::Dispatch(
        {MSG_AutoTrade_Opcode, autoTrade.data(), kAutoTradePacketSize},
        receiveAutoTrade), "AutoTrade rejects a mismatched Header.Size");
    autoTrade[0] = static_cast<char>(kAutoTradePacketSize);
    check(autoTradeCalls == 0, "invalid AutoTrade does not reach the consumer");
    check(received_packet::Dispatch(
        {MSG_AutoTrade_Opcode, autoTrade.data(), kAutoTradePacketSize},
        receiveAutoTrade) && autoTradeCalls == 1,
        "valid AutoTrade delivered once");
    check(autoTrade[kAutoTradeDescriptionOffset] == 'L' &&
        static_cast<unsigned char>(autoTrade[kAutoTradeItemsOffset]) == 0x21 &&
        static_cast<unsigned char>(autoTrade[kAutoTradeItemsOffset +
            (kAutoTradeItemCount - 1) * kAutoTradeItemSize]) == 0x32 &&
        static_cast<unsigned char>(autoTrade[kAutoTradeCarryPositionsOffset]) == 0x43 &&
        static_cast<unsigned char>(autoTrade[kAutoTradePricesOffset]) == 0x54 &&
        static_cast<unsigned char>(autoTrade[kAutoTradePricesOffset +
            (kAutoTradeItemCount - 1) * kAutoTradePriceSize]) == 0x65 &&
        static_cast<unsigned char>(autoTrade[kAutoTradeTaxOffset]) == 0x76 &&
        static_cast<unsigned char>(autoTrade[kAutoTradeTargetIdOffset]) == 0x87,
        "AutoTrade preserves description, offers, prices, tax and target");
    check(autoTrade == autoTradeBefore,
        "gate preserves every AutoTrade byte");

    // The capsule cache and tooltip consume the compact 52-byte 7.48
    // snapshot. The four masteries inherited from 7.69 shifted skill/Quest.
    std::array<char, kCapsuleInfoPacketSize + 1> capsuleInfo{};
    capsuleInfo[0] = static_cast<char>(kCapsuleInfoPacketSize);
    capsuleInfo[4] = static_cast<char>(MSG_CapsuleInfo_Opcode & 0xFF);
    capsuleInfo[5] = static_cast<char>((MSG_CapsuleInfo_Opcode >> 8) & 0xFF);
    capsuleInfo[6] = 0x34;
    capsuleInfo[7] = 0x12;
    capsuleInfo[kCapsuleInfoIndexOffset] = 0x11;
    capsuleInfo[kCapsuleInfoClassOffset] = 0x22;
    capsuleInfo[kCapsuleInfoLevelOffset] = 0x33;
    capsuleInfo[kCapsuleInfoStrengthOffset] = 0x44;
    capsuleInfo[kCapsuleInfoIntelligenceOffset] = 0x55;
    capsuleInfo[kCapsuleInfoDexterityOffset] = 0x66;
    capsuleInfo[kCapsuleInfoConstitutionOffset] = 0x77;
    capsuleInfo[kCapsuleInfoMasteryOffset] = static_cast<char>(0x88);
    capsuleInfo[kCapsuleInfoMasteryOffset +
        (kCapsuleInfoMasteryCount - 1) * sizeof(short)] = static_cast<char>(0x99);
    capsuleInfo[kCapsuleInfoSkillOffset] = static_cast<char>(0xAA);
    capsuleInfo[kCapsuleInfoSkillOffset +
        (kCapsuleInfoSkillCount - 1) * sizeof(short)] = static_cast<char>(0xBB);
    capsuleInfo[kCapsuleInfoQuestOffset] = static_cast<char>(0xCC);
    const auto capsuleInfoBefore = capsuleInfo;
    int capsuleInfoCalls = 0;
    const auto receiveCapsuleInfo = [&](const PacketView& view) {
        ++capsuleInfoCalls;
        check(view.data == capsuleInfo.data() &&
            view.size == kCapsuleInfoPacketSize &&
            view.opcode == MSG_CapsuleInfo_Opcode,
            "CapsuleInfo preserves frame and opcode");
    };
    check(received_packet::ExpectedSize(MSG_CapsuleInfo_Opcode) ==
        kCapsuleInfoPacketSize,
        "CapsuleInfo publica tamanho esperado no gate");
    for (std::size_t n = 0; n < kCapsuleInfoPacketSize; ++n)
        check(!received_packet::Dispatch(
            {MSG_CapsuleInfo_Opcode, capsuleInfo.data(), n}, receiveCapsuleInfo),
            "CapsuleInfo rejects every truncated prefix");
    check(!received_packet::Dispatch(
        {MSG_CapsuleInfo_Opcode, nullptr, kCapsuleInfoPacketSize},
        receiveCapsuleInfo), "CapsuleInfo rejects a null buffer");
    check(!received_packet::Dispatch(
        {MSG_CapsuleInfo_Opcode, capsuleInfo.data(), kCapsuleInfoPacketSize + 1},
        receiveCapsuleInfo), "CapsuleInfo rejects an oversized frame");
    check(!received_packet::Dispatch(
        {0x119, capsuleInfo.data(), kCapsuleInfoPacketSize}, receiveCapsuleInfo),
        "an outer opcode cannot hide CapsuleInfo");
    capsuleInfo[4] = static_cast<char>(MSG_SetShortSkill_Opcode & 0xFF);
    capsuleInfo[5] = static_cast<char>((MSG_SetShortSkill_Opcode >> 8) & 0xFF);
    check(!received_packet::Dispatch(
        {MSG_CapsuleInfo_Opcode, capsuleInfo.data(), kCapsuleInfoPacketSize},
        receiveCapsuleInfo), "CapsuleInfo rejects a mismatched Header.Type");
    capsuleInfo[4] = static_cast<char>(MSG_CapsuleInfo_Opcode & 0xFF);
    capsuleInfo[5] = static_cast<char>((MSG_CapsuleInfo_Opcode >> 8) & 0xFF);
    capsuleInfo[0] = static_cast<char>(kCapsuleInfoPacketSize - 1);
    check(!received_packet::Dispatch(
        {MSG_CapsuleInfo_Opcode, capsuleInfo.data(), kCapsuleInfoPacketSize},
        receiveCapsuleInfo), "CapsuleInfo rejects a mismatched Header.Size");
    capsuleInfo[0] = static_cast<char>(kCapsuleInfoPacketSize);
    check(capsuleInfoCalls == 0,
        "invalid CapsuleInfo does not reach the consumer");
    check(received_packet::Dispatch(
        {MSG_CapsuleInfo_Opcode, capsuleInfo.data(), kCapsuleInfoPacketSize},
        receiveCapsuleInfo) && capsuleInfoCalls == 1,
        "valid CapsuleInfo delivered once");
    check(static_cast<unsigned char>(capsuleInfo[kCapsuleInfoIndexOffset]) == 0x11 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoClassOffset]) == 0x22 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoLevelOffset]) == 0x33 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoStrengthOffset]) == 0x44 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoIntelligenceOffset]) == 0x55 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoDexterityOffset]) == 0x66 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoConstitutionOffset]) == 0x77 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoMasteryOffset]) == 0x88 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoMasteryOffset +
            (kCapsuleInfoMasteryCount - 1) * sizeof(short)]) == 0x99 &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoSkillOffset]) == 0xAA &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoSkillOffset +
            (kCapsuleInfoSkillCount - 1) * sizeof(short)]) == 0xBB &&
        static_cast<unsigned char>(capsuleInfo[kCapsuleInfoQuestOffset]) == 0xCC,
        "CapsuleInfo preserves index, score, mastery, skills and quest");
    check(capsuleInfo == capsuleInfoBefore,
        "gate preserves every CapsuleInfo byte");

    // PremiumFirework carries eight reserved bytes and the 16-byte bitmap at
    // +20; the visual effect receives exactly that slice without copying the frame.
    std::array<char, sizeof(MSG_PremiumFirework) + 1> premiumFirework{};
    premiumFirework[0] = static_cast<char>(sizeof(MSG_PremiumFirework));
    premiumFirework[4] = static_cast<char>(MSG_PremiumFirework_Opcode & 0xFF);
    premiumFirework[5] = static_cast<char>((MSG_PremiumFirework_Opcode >> 8) & 0xFF);
    premiumFirework[6] = 0x34;
    premiumFirework[7] = 0x12;
    premiumFirework[20] = 0x11;
    premiumFirework[35] = 0x22;
    const auto premiumBefore = premiumFirework;
    int premiumCalls = 0;
    const auto receivePremium = [&](const PacketView& view) {
        ++premiumCalls;
        check(view.data == premiumFirework.data() && view.size == sizeof(MSG_PremiumFirework) &&
            view.opcode == MSG_PremiumFirework_Opcode,
            "PremiumFirework preserves frame and opcode");
    };
    for (std::size_t n = 0; n < sizeof(MSG_PremiumFirework); ++n)
        check(!received_packet::Dispatch({MSG_PremiumFirework_Opcode,
            premiumFirework.data(), n}, receivePremium),
            "PremiumFirework rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_PremiumFirework_Opcode, nullptr,
        sizeof(MSG_PremiumFirework)}, receivePremium),
        "PremiumFirework rejects a null buffer");
    check(!received_packet::Dispatch({MSG_PremiumFirework_Opcode,
        premiumFirework.data(), sizeof(MSG_PremiumFirework) + 1}, receivePremium),
        "PremiumFirework rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, premiumFirework.data(),
        sizeof(MSG_PremiumFirework)}, receivePremium),
        "an outer opcode cannot hide PremiumFirework");
    premiumFirework[4] = static_cast<char>((MSG_PremiumFirework_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_PremiumFirework_Opcode,
        premiumFirework.data(), sizeof(MSG_PremiumFirework)}, receivePremium),
        "PremiumFirework rejects a mismatched Header.Type");
    premiumFirework[4] = static_cast<char>(MSG_PremiumFirework_Opcode & 0xFF);
    premiumFirework[0] = static_cast<char>(sizeof(MSG_PremiumFirework) - 1);
    check(!received_packet::Dispatch({MSG_PremiumFirework_Opcode,
        premiumFirework.data(), sizeof(MSG_PremiumFirework)}, receivePremium),
        "PremiumFirework rejects a mismatched Header.Size");
    premiumFirework[0] = static_cast<char>(sizeof(MSG_PremiumFirework));
    check(premiumCalls == 0, "invalid PremiumFirework does not reach the consumer");
    check(received_packet::Dispatch({MSG_PremiumFirework_Opcode,
        premiumFirework.data(), sizeof(MSG_PremiumFirework)}, receivePremium) && premiumCalls == 1,
        "valid PremiumFirework delivered once");
    check(premiumFirework[20] == 0x11 && premiumFirework[35] == 0x22,
        "PremiumFirework preserves the bitmap bounds at +20");
    check(premiumFirework == premiumBefore,
        "gate preserves every PremiumFirework byte");

    // Gamble result: five symbols, three stops, a signed prize and the
    // jackpot. The eight reserved bytes between the stops and the prize remain part
    // of the 36-byte ABI and cannot be removed.
    std::array<char, sizeof(MSG_ResultGamble) + 1> resultGamble{};
    resultGamble[0] = static_cast<char>(sizeof(MSG_ResultGamble));
    resultGamble[4] = static_cast<char>(MSG_ResultGamble_Opcode & 0xFF);
    resultGamble[5] = static_cast<char>((MSG_ResultGamble_Opcode >> 8) & 0xFF);
    resultGamble[6] = 0x34;
    resultGamble[7] = 0x12;
    resultGamble[12] = 1;
    resultGamble[16] = 2;
    resultGamble[17] = 3;
    resultGamble[19] = 4;
    resultGamble[28] = static_cast<char>(0xEF);
    resultGamble[29] = static_cast<char>(0xCD);
    resultGamble[30] = static_cast<char>(0xAB);
    resultGamble[31] = static_cast<char>(0x89);
    resultGamble[32] = 0x11;
    resultGamble[33] = 0x22;
    resultGamble[34] = 0x33;
    resultGamble[35] = 0x44;
    const auto resultGambleBefore = resultGamble;
    int resultGambleCalls = 0;
    const auto receiveResultGamble = [&](const PacketView& view) {
        ++resultGambleCalls;
        check(view.data == resultGamble.data() && view.size == sizeof(MSG_ResultGamble) &&
            view.opcode == MSG_ResultGamble_Opcode,
            "ResultGamble preserves frame and opcode");
    };
    for (std::size_t n = 0; n < sizeof(MSG_ResultGamble); ++n)
        check(!received_packet::Dispatch({MSG_ResultGamble_Opcode,
            resultGamble.data(), n}, receiveResultGamble),
            "ResultGamble rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_ResultGamble_Opcode, nullptr,
        sizeof(MSG_ResultGamble)}, receiveResultGamble),
        "ResultGamble rejects a null buffer");
    check(!received_packet::Dispatch({MSG_ResultGamble_Opcode,
        resultGamble.data(), sizeof(MSG_ResultGamble) + 1}, receiveResultGamble),
        "ResultGamble rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, resultGamble.data(),
        sizeof(MSG_ResultGamble)}, receiveResultGamble),
        "an outer opcode cannot hide ResultGamble");
    resultGamble[4] = static_cast<char>((MSG_ResultGamble_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_ResultGamble_Opcode,
        resultGamble.data(), sizeof(MSG_ResultGamble)}, receiveResultGamble),
        "ResultGamble rejects a mismatched Header.Type");
    resultGamble[4] = static_cast<char>(MSG_ResultGamble_Opcode & 0xFF);
    resultGamble[0] = static_cast<char>(sizeof(MSG_ResultGamble) - 1);
    check(!received_packet::Dispatch({MSG_ResultGamble_Opcode,
        resultGamble.data(), sizeof(MSG_ResultGamble)}, receiveResultGamble),
        "ResultGamble rejects a mismatched Header.Size");
    resultGamble[0] = static_cast<char>(sizeof(MSG_ResultGamble));
    check(resultGambleCalls == 0, "invalid ResultGamble does not reach the consumer");
    check(received_packet::Dispatch({MSG_ResultGamble_Opcode,
        resultGamble.data(), sizeof(MSG_ResultGamble)}, receiveResultGamble) &&
        resultGambleCalls == 1, "valid ResultGamble delivered once");
    check(resultGamble[12] == 1 && resultGamble[16] == 2 &&
        resultGamble[17] == 3 && resultGamble[19] == 4 &&
        static_cast<unsigned char>(resultGamble[28]) == 0xEF &&
        static_cast<unsigned char>(resultGamble[35]) == 0x44,
        "ResultGamble preserves symbols, stops, prize and jackpot");
    check(resultGamble == resultGambleBefore,
        "gate preserves every ResultGamble byte");

    // Go MessagePanel frame: zero ID, text at +12 and the final NUL at +107.
    return failures;
}
