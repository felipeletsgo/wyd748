#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: Trade. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchTradeTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    std::array<char, kTradePacketSize + 1> trade{};
    trade[0] = static_cast<char>(kTradePacketSize & 0xFF);
    trade[1] = static_cast<char>((kTradePacketSize >> 8) & 0xFF);
    trade[4] = static_cast<char>(MSG_Trade_Opcode & 0xFF);
    trade[5] = static_cast<char>((MSG_Trade_Opcode >> 8) & 0xFF);
    trade[6] = 0x78;
    trade[7] = 0x56;
    trade[kTradeItemsOffset] = 0x34;
    trade[kTradeItemsOffset + 1] = 0x12;
    trade[kTradeCarryPositionsOffset] = 7;
    trade[kTradeMoneyOffset] = 0x40;
    trade[kTradeMoneyOffset + 1] = static_cast<char>(0xE2);
    trade[kTradeMoneyOffset + 2] = 0x01;
    trade[kTradeCheckOffset] = 1;
    trade[kTradeOpponentIdOffset] = static_cast<char>(0xCD);
    trade[kTradeOpponentIdOffset + 1] = static_cast<char>(0xAB);
    const auto tradeBefore = trade;
    int tradeCalls = 0;
    const auto receiveTrade = [&](const PacketView& view) {
        ++tradeCalls;
        check(view.data == trade.data() && view.size == kTradePacketSize &&
            view.opcode == MSG_Trade_Opcode,
            "Trade preserves the 156-byte frame and opcode");
    };
    for (std::size_t n = 0; n < kTradePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_Trade_Opcode, trade.data(), n}, receiveTrade),
            "Trade rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_Trade_Opcode, nullptr, kTradePacketSize}, receiveTrade),
        "Trade rejects a null buffer");
    check(!received_packet::Dispatch({MSG_Trade_Opcode, trade.data(), kTradePacketSize + 1},
        receiveTrade), "Trade rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, trade.data(), kTradePacketSize}, receiveTrade),
        "an outer opcode cannot hide Trade");
    trade[4] = static_cast<char>(MSG_CloseTrade_Opcode & 0xFF);
    check(!received_packet::Dispatch({MSG_Trade_Opcode, trade.data(), kTradePacketSize},
        receiveTrade), "Trade rejects a mismatched Header.Type");
    trade[4] = static_cast<char>(MSG_Trade_Opcode & 0xFF);
    trade[0] = static_cast<char>((kTradePacketSize - 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_Trade_Opcode, trade.data(), kTradePacketSize},
        receiveTrade), "Trade rejects a mismatched Header.Size");
    trade[0] = static_cast<char>(kTradePacketSize & 0xFF);
    check(tradeCalls == 0, "invalid Trade does not reach the consumer");
    check(received_packet::Dispatch({MSG_Trade_Opcode, trade.data(), kTradePacketSize},
        receiveTrade) && tradeCalls == 1, "valid Trade delivered once");
    check(static_cast<unsigned char>(trade[kTradeItemsOffset]) == 0x34 &&
        static_cast<unsigned char>(trade[kTradeCarryPositionsOffset]) == 7 &&
        static_cast<unsigned char>(trade[kTradeCheckOffset]) == 1 &&
        static_cast<unsigned char>(trade[kTradeOpponentIdOffset]) == 0xCD,
        "Trade preserves the fields consumed by the handler");
    check(trade == tradeBefore, "gate preserves every Trade byte");

    // CloseTrade is header-only. With no payload, the exact size is an
    // integral part of the contract, not just a minimum for the cast.
    std::array<char, kCloseTradePacketSize + 1> closeTrade{};
    closeTrade[0] = static_cast<char>(kCloseTradePacketSize);
    closeTrade[4] = static_cast<char>(MSG_CloseTrade_Opcode & 0xFF);
    closeTrade[5] = static_cast<char>((MSG_CloseTrade_Opcode >> 8) & 0xFF);
    closeTrade[6] = 0x78;
    closeTrade[7] = 0x56;
    const auto closeTradeBefore = closeTrade;
    int closeTradeCalls = 0;
    const auto receiveCloseTrade = [&](const PacketView& view) {
        ++closeTradeCalls;
        check(view.data == closeTrade.data() && view.size == kCloseTradePacketSize &&
            view.opcode == MSG_CloseTrade_Opcode,
            "CloseTrade preserves the header-only frame and opcode");
    };
    for (std::size_t n = 0; n < kCloseTradePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CloseTrade_Opcode, closeTrade.data(), n},
            receiveCloseTrade), "CloseTrade rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_CloseTrade_Opcode, nullptr,
        kCloseTradePacketSize}, receiveCloseTrade), "CloseTrade rejects a null buffer");
    check(!received_packet::Dispatch({MSG_CloseTrade_Opcode, closeTrade.data(),
        kCloseTradePacketSize + 1}, receiveCloseTrade), "CloseTrade rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, closeTrade.data(), kCloseTradePacketSize},
        receiveCloseTrade), "an outer opcode cannot hide CloseTrade");
    closeTrade[4] = static_cast<char>(MSG_Trade_Opcode & 0xFF);
    check(!received_packet::Dispatch({MSG_CloseTrade_Opcode, closeTrade.data(),
        kCloseTradePacketSize}, receiveCloseTrade), "CloseTrade rejects a mismatched Header.Type");
    closeTrade[4] = static_cast<char>(MSG_CloseTrade_Opcode & 0xFF);
    closeTrade[0] = static_cast<char>(kCloseTradePacketSize - 1);
    check(!received_packet::Dispatch({MSG_CloseTrade_Opcode, closeTrade.data(),
        kCloseTradePacketSize}, receiveCloseTrade), "CloseTrade rejects a mismatched Header.Size");
    closeTrade[0] = static_cast<char>(kCloseTradePacketSize);
    check(closeTradeCalls == 0, "invalid CloseTrade does not reach the consumer");
    check(received_packet::Dispatch({MSG_CloseTrade_Opcode, closeTrade.data(),
        kCloseTradePacketSize}, receiveCloseTrade) && closeTradeCalls == 1,
        "valid CloseTrade delivered once");
    check(static_cast<unsigned char>(closeTrade[6]) == 0x78 &&
        static_cast<unsigned char>(closeTrade[7]) == 0x56,
        "CloseTrade preserves Header.ID");
    check(closeTrade == closeTradeBefore,
        "gate preserves every CloseTrade byte");

    // CNFTradeCheck carries no payload: the arrival of the header confirms the
    // first check, and the ID still identifies the receiving character.
    std::array<char, kTradeCheckConfirmationPacketSize + 1> tradeCheck{};
    tradeCheck[0] = static_cast<char>(kTradeCheckConfirmationPacketSize);
    tradeCheck[4] = static_cast<char>(MSG_CNFTradeCheck_Opcode & 0xFF);
    tradeCheck[5] = static_cast<char>((MSG_CNFTradeCheck_Opcode >> 8) & 0xFF);
    tradeCheck[6] = 0x78;
    tradeCheck[7] = 0x56;
    const auto tradeCheckBefore = tradeCheck;
    int tradeCheckCalls = 0;
    const auto receiveTradeCheck = [&](const PacketView& view) {
        ++tradeCheckCalls;
        check(view.data == tradeCheck.data() &&
            view.size == kTradeCheckConfirmationPacketSize &&
            view.opcode == MSG_CNFTradeCheck_Opcode,
            "CNFTradeCheck preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kTradeCheckConfirmationPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CNFTradeCheck_Opcode,
            tradeCheck.data(), n}, receiveTradeCheck),
            "CNFTradeCheck rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_CNFTradeCheck_Opcode, nullptr,
        kTradeCheckConfirmationPacketSize}, receiveTradeCheck),
        "CNFTradeCheck rejects a null buffer");
    check(!received_packet::Dispatch({MSG_CNFTradeCheck_Opcode,
        tradeCheck.data(), kTradeCheckConfirmationPacketSize + 1}, receiveTradeCheck),
        "CNFTradeCheck rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, tradeCheck.data(),
        kTradeCheckConfirmationPacketSize}, receiveTradeCheck),
        "an outer opcode cannot hide CNFTradeCheck");
    tradeCheck[4] = static_cast<char>((MSG_CNFTradeCheck_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_CNFTradeCheck_Opcode,
        tradeCheck.data(), kTradeCheckConfirmationPacketSize}, receiveTradeCheck),
        "CNFTradeCheck rejects a mismatched Header.Type");
    tradeCheck[4] = static_cast<char>(MSG_CNFTradeCheck_Opcode & 0xFF);
    tradeCheck[0] = static_cast<char>(kTradeCheckConfirmationPacketSize - 1);
    check(!received_packet::Dispatch({MSG_CNFTradeCheck_Opcode,
        tradeCheck.data(), kTradeCheckConfirmationPacketSize}, receiveTradeCheck),
        "CNFTradeCheck rejects a mismatched Header.Size");
    tradeCheck[0] = static_cast<char>(kTradeCheckConfirmationPacketSize);
    check(tradeCheckCalls == 0, "invalid CNFTradeCheck does not reach the consumer");
    check(received_packet::Dispatch({MSG_CNFTradeCheck_Opcode,
        tradeCheck.data(), kTradeCheckConfirmationPacketSize}, receiveTradeCheck) &&
        tradeCheckCalls == 1, "valid CNFTradeCheck delivered once");
    check(static_cast<unsigned char>(tradeCheck[6]) == 0x78 &&
        static_cast<unsigned char>(tradeCheck[7]) == 0x56,
        "CNFTradeCheck preserves Header.ID");
    check(tradeCheck == tradeCheckBefore,
        "gate preserves every CNFTradeCheck byte");

    // The native client recognizes partial 16/20-byte forms, but the only active
    // peer publishes the three fields together. The gate avoids mixing stale
    // clan/alliance state with a newly received war guild.
    std::array<char, kWarInfoPacketSize + 1> warInfo{};
    warInfo[0] = static_cast<char>(kWarInfoPacketSize);
    warInfo[4] = static_cast<char>(MSG_WarInfo_Opcode & 0xFF);
    warInfo[5] = static_cast<char>((MSG_WarInfo_Opcode >> 8) & 0xFF);
    warInfo[6] = 0x30;
    warInfo[7] = 0x75;
    warInfo[kWarInfoGuildOffset] = 0x34;
    warInfo[kWarInfoGuildOffset + 1] = 0x12;
    warInfo[kWarInfoClanOffset] = 7;
    warInfo[kWarInfoAllyOffset] = 0x78;
    warInfo[kWarInfoAllyOffset + 1] = 0x56;
    const auto warInfoBefore = warInfo;
    int warInfoCalls = 0;
    const auto receiveWarInfo = [&](const PacketView& view) {
        ++warInfoCalls;
        check(view.data == warInfo.data() && view.size == kWarInfoPacketSize &&
            view.opcode == MSG_WarInfo_Opcode,
            "WarInfo preserves frame and opcode");
    };
    for (std::size_t n = 0; n < kWarInfoPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_WarInfo_Opcode, warInfo.data(), n},
            receiveWarInfo), "WarInfo rejects every truncated prefix");
    check(!received_packet::Dispatch({MSG_WarInfo_Opcode, nullptr, kWarInfoPacketSize},
        receiveWarInfo), "WarInfo rejects a null buffer");
    check(!received_packet::Dispatch({MSG_WarInfo_Opcode, warInfo.data(),
        kWarInfoPacketSize + 1}, receiveWarInfo), "WarInfo rejects an oversized frame");
    check(!received_packet::Dispatch({0x119, warInfo.data(), kWarInfoPacketSize},
        receiveWarInfo), "an outer opcode cannot hide WarInfo");
    warInfo[4] = static_cast<char>((MSG_WarInfo_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_WarInfo_Opcode, warInfo.data(),
        kWarInfoPacketSize}, receiveWarInfo), "WarInfo rejects a mismatched Header.Type");
    warInfo[4] = static_cast<char>(MSG_WarInfo_Opcode & 0xFF);
    warInfo[0] = 16;
    check(!received_packet::Dispatch({MSG_WarInfo_Opcode, warInfo.data(), 16}, receiveWarInfo),
        "WarInfo rejects a partial native one-parameter snapshot");
    warInfo[0] = 20;
    check(!received_packet::Dispatch({MSG_WarInfo_Opcode, warInfo.data(), 20}, receiveWarInfo),
        "WarInfo rejects a partial native two-parameter snapshot");
    warInfo[0] = static_cast<char>(kWarInfoPacketSize);
    check(warInfoCalls == 0, "invalid WarInfo does not reach the consumer");
    check(received_packet::Dispatch({MSG_WarInfo_Opcode, warInfo.data(),
        kWarInfoPacketSize}, receiveWarInfo) && warInfoCalls == 1,
        "complete WarInfo delivered once");
    check(static_cast<unsigned char>(warInfo[6]) == 0x30 &&
        static_cast<unsigned char>(warInfo[7]) == 0x75 &&
        static_cast<unsigned char>(warInfo[kWarInfoGuildOffset]) == 0x34 &&
        warInfo[kWarInfoClanOffset] == 7 &&
        static_cast<unsigned char>(warInfo[kWarInfoAllyOffset + 1]) == 0x56,
        "WarInfo preserves receiver, guild, clan and ally");
    check(warInfo == warInfoBefore, "gate preserves every WarInfo byte");

    // SetShortSkill is an authoritative echo: the legacy handler copies twenty bytes
    // immediately, so no prefix can reach the consumer.
    return failures;
}
