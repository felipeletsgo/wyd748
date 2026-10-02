#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: RequestsAndEvents. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchRequestsAndEventsTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    MSG_BuyToto toto{};
    toto.Header.Type = MSG_BuyToto_Opcode;
    toto.Header.Size = sizeof(toto);
    toto.TargetID = 7;
    toto.TargetCarryPos = 14;
    toto.MyCarryPos = 3;
    toto.Coin = 0x7FFFFFFF;
    toto.Gindex = 80;
    toto.A_Score = 127;
    toto.B_Score = 0;
    check(sizeof(toto) == 36 && toto.TargetID == 7 && toto.MyCarryPos == 3 &&
        toto.Header.Type == 0x3CE && toto.Header.Size == 36,
        "TOTO preserves the C-S intention layout and fields");
    MSG_ApplyBonus applyBonus{};
    applyBonus.Header.Type = MSG_ApplyBonus_Opcode;
    applyBonus.Header.Size = sizeof(applyBonus);
    applyBonus.BonusType = 2;
    applyBonus.Detail = 5000;
    applyBonus.TargetID = 42;
    check(sizeof(applyBonus) == 20 && applyBonus.BonusType == 2 &&
        applyBonus.Detail == 5000 && applyBonus.TargetID == 42 &&
        applyBonus.Header.Type == 0x277 && applyBonus.Header.Size == 20,
        "ApplyBonus preserves type, detail, master and size");
    MSG_UseItem useItem{};
    useItem.Header.Type = MSG_UseItem_Opcode;
    useItem.Header.Size = sizeof(useItem);
    useItem.SourType = 1;
    useItem.SourPos = 17;
    useItem.DestType = 0;
    useItem.DestPos = 0;
    useItem.GridX = 8;
    useItem.GridY = 6;
    useItem.ItemID = 3377;
    check(sizeof(useItem) == 36 && useItem.SourType == 1 && useItem.SourPos == 17 &&
        useItem.GridX == 8 && useItem.GridY == 6 && useItem.ItemID == 3377 &&
        useItem.Header.Type == 0x373 && useItem.Header.Size == 36,
        "UseItem preserves source, cell, item and size");
    MSG_SetPKMode pk{};
    pk.Header.Type = MSG_SetPKMode_Opcode;
    pk.Header.Size = sizeof(pk);
    pk.Header.ID = 0x1234;
    pk.Parm = 1;
    check(sizeof(pk) == 16 && pk.Header.Type == 0x399 && pk.Header.Size == 16 &&
        pk.Header.ID == 0x1234 && (pk.Parm == 0 || pk.Parm == 1),
        "PK mode preserves identity, domain and size");
    MSG_PremiumFirework firework{};
    firework.Header.Type = MSG_PremiumFirework_Opcode;
    firework.Header.Size = sizeof(firework);
    firework.Header.ID = 77;
    firework.Bitmap[0] = 1;
    firework.Bitmap[12] = static_cast<char>(0x80);
    check(sizeof(firework) == 36 && firework.Header.Type == 0x3CA &&
        firework.Header.Size == 36 && firework.Header.ID == 77 &&
        firework.Bitmap[0] == 1 && static_cast<unsigned char>(firework.Bitmap[12]) == 0x80,
        "premium firework preserves ID, reserve, bitmap and size");
    MSG_UseItem2 fireworkUse{};
    fireworkUse.Header.Type = MSG_UseItem2_Opcode;
    fireworkUse.Header.Size = sizeof(fireworkUse);
    fireworkUse.SourType = 1;
    fireworkUse.SourPos = 62;
    fireworkUse.GridX = 2100;
    fireworkUse.GridY = 2101;
    fireworkUse.Parm[0] = 1;
    fireworkUse.Parm[12] = 0x0F;
    check(sizeof(fireworkUse) == 52 && fireworkUse.Header.Type == 0x3C9 &&
        fireworkUse.Header.Size == 52 && fireworkUse.SourType == 1 &&
        fireworkUse.SourPos == 62 && fireworkUse.GridX == 2100 &&
        fireworkUse.GridY == 2101 && fireworkUse.Parm[0] == 1 &&
        fireworkUse.Parm[12] == 0x0F,
        "premium firework request preserves slot, position, bitmap and size");
    MSG_DoJackpotBet gamble{};
    gamble.Header.Type = MSG_DoJackpotBet_Opcode;
    gamble.Header.Size = sizeof(gamble);
    gamble.GambleType = 2;
    gamble.Bet = 100000;
    MSG_ResultGamble result{};
    result.Header.Type = MSG_ResultGamble_Opcode;
    result.Header.Size = sizeof(result);
    result.Result[0] = 14;
    result.StopPosition[2] = 9;
    result.Prize = -7;
    result.Jackpot = 0x89ABCDEFu;
    check(sizeof(gamble) == 20 && gamble.Header.Type == 0x2BE && gamble.Bet == 100000 &&
        sizeof(result) == 36 && result.Header.Type == 0x1BF && result.Prize == -7 &&
        result.Jackpot == 0x89ABCDEFu,
        "Gamble preserves bet, result, prize, jackpot and sizes");
    std::array<char, 25> mobKill{};
    mobKill[0] = 24;
    mobKill[4] = 0x38;
    mobKill[5] = 3;
    mobKill[12] = static_cast<char>(0xD2);
    mobKill[13] = 0x04;
    mobKill[16] = 0x34;
    mobKill[17] = 0x12;
    mobKill[18] = 0x78;
    mobKill[19] = 0x56;
    mobKill[20] = 0x44;
    mobKill[21] = 0x33;
    mobKill[22] = 0x22;
    mobKill[23] = 0x11;
    int mobKillCalls = 0;
    const auto receiveMobKill = [&](const PacketView& view) {
        ++mobKillCalls;
        check(view.data == mobKill.data() && view.size == 24,
            "mob-kill preserves the 24-byte frame");
    };
    for (std::size_t n = 0; n < 24; n += 3)
        check(!received_packet::Dispatch({0x338, mobKill.data(), n}, receiveMobKill),
            "truncated mob-kill rejected");
    check(!received_packet::Dispatch({0x338, mobKill.data(), 25}, receiveMobKill),
        "oversized mob-kill rejected");
    check(!received_packet::Dispatch({0x338, nullptr, 24}, receiveMobKill),
        "null mob-kill rejected");
    mobKill[0] = 23;
    check(!received_packet::Dispatch({0x338, mobKill.data(), 24}, receiveMobKill),
        "mismatched mob-kill Size rejected");
    mobKill[0] = 24;
    check(received_packet::Dispatch({0x338, mobKill.data(), 24}, receiveMobKill) && mobKillCalls == 1,
        "valid mob-kill delivered once");
    MSG_CNFMobKill decodedKill{};
    std::memcpy(&decodedKill, mobKill.data(), sizeof(decodedKill));
    check(decodedKill.FakeExp == 1234 && decodedKill.KilledMob == 0x1234 &&
        decodedKill.Killer == 0x5678 && decodedKill.Exp == 0x11223344u,
        "mob-kill preserves Hold, IDs and uint32 EXP");
    std::array<char, 37> updateEtc{};
    updateEtc[0] = 36;
    updateEtc[4] = 0x37;
    updateEtc[5] = 3;
    updateEtc[12] = static_cast<char>(0xD2);
    updateEtc[13] = 0x04;
    updateEtc[16] = 0x44;
    updateEtc[17] = 0x33;
    updateEtc[18] = 0x22;
    updateEtc[19] = 0x11;
    updateEtc[20] = 0x08;
    updateEtc[24] = 1;
    updateEtc[26] = 2;
    updateEtc[28] = 3;
    updateEtc[30] = 4;
    updateEtc[32] = 5;
    int updateEtcCalls = 0;
    const auto receiveUpdateEtc = [&](const PacketView& view) {
        ++updateEtcCalls;
        check(view.data == updateEtc.data() && view.size == 36,
            "UpdateEtc preserves the compact frame");
    };
    for (std::size_t n = 0; n < 36; n += 4)
        check(!received_packet::Dispatch({0x337, updateEtc.data(), n}, receiveUpdateEtc),
            "truncated UpdateEtc rejected");
    check(!received_packet::Dispatch({0x337, updateEtc.data(), 37}, receiveUpdateEtc),
        "oversized UpdateEtc rejected");
    check(!received_packet::Dispatch({0x337, nullptr, 36}, receiveUpdateEtc),
        "null UpdateEtc rejected");
    updateEtc[0] = 35;
    check(!received_packet::Dispatch({0x337, updateEtc.data(), 36}, receiveUpdateEtc),
        "mismatched UpdateEtc Size rejected");
    updateEtc[0] = 36;
    check(received_packet::Dispatch({0x337, updateEtc.data(), 36}, receiveUpdateEtc) && updateEtcCalls == 1,
        "valid UpdateEtc delivered once");
    MSG_UpdateEtc decodedEtc{};
    std::memcpy(&decodedEtc, updateEtc.data(), sizeof(decodedEtc));
    check(decodedEtc.Hold == 1234 && decodedEtc.Exp == 0x11223344u &&
        decodedEtc.LearnedSkill == 8 && decodedEtc.StatusPoint == 1 &&
        decodedEtc.MasterPoint == 2 && decodedEtc.SkillPoint == 3 &&
        decodedEtc.Magic == 4 && decodedEtc.Coin == 5,
        "UpdateEtc preserves Hold, EXP, skill, points and gold");
    quiz_event::Challenge quiz{};
    quiz.Header.Size = sizeof(quiz); quiz.Header.Type = quiz_event::ChallengeOpcode; quiz.Header.ID = 7;
    quiz.Version = 1; quiz.Kind = 1; quiz.Token[0] = 0xA5; quiz.TimeoutMs = 10000;
    std::memcpy(quiz.Question, "88 x 4 = ?", 11);
    quiz.Answers[0] = 351; quiz.Answers[1] = 352; quiz.Answers[2] = 353; quiz.Answers[3] = 359;
    quiz_event::Challenge parsedQuiz{};
    check(quiz_event::Parse(&quiz, sizeof(quiz), parsedQuiz) && parsedQuiz.Answers[1] == 352,
        "Quiz v1 parses Go-compatible layout");
    for (std::size_t n = 0; n < sizeof(quiz); ++n)
        check(!quiz_event::Parse(&quiz, n, parsedQuiz), "Quiz rejects truncation");
    check(!quiz_event::Parse(&quiz, sizeof(quiz) + 1, parsedQuiz), "Quiz rejects tail");
    quiz.Version = 2;
    check(!quiz_event::Parse(&quiz, sizeof(quiz), parsedQuiz), "Quiz rejects version");
    quiz.Version = 1; quiz.Answers[2] = 352;
    check(!quiz_event::Parse(&quiz, sizeof(quiz), parsedQuiz), "Quiz rejects duplicate choices");
    quiz.Answers[2] = 353;
    auto invalidQuiz = quiz; std::memset(invalidQuiz.Question, 'x', sizeof(invalidQuiz.Question));
    check(!quiz_event::Parse(&invalidQuiz, sizeof(invalidQuiz), parsedQuiz), "Quiz requires terminated text");
    int quizDispatches = 0;
    auto receiveQuiz = [&](const PacketView&) { ++quizDispatches; };
    check(received_packet::Dispatch({quiz_event::ChallengeOpcode, reinterpret_cast<char*>(&quiz), sizeof(quiz)}, receiveQuiz),
        "Quiz dispatch accepts exact frame");
    check(!received_packet::Dispatch({quiz_event::ChallengeOpcode, reinterpret_cast<char*>(&quiz), sizeof(quiz)-1}, receiveQuiz)
        && quizDispatches == 1, "Quiz dispatch rejects short frame before scene");
    quiz_event::State quizState{};
    quiz_event::Answer quizAnswer{};
    check(!quizState.Respond(7, 1, 0, quizAnswer), "Quiz cannot answer without show");
    check(quizState.Apply(quiz, 100) && !quizState.Apply(quiz, 9000), "Quiz show replay cannot extend timer");
    check(!quizState.Expired(10099) && quizState.Expired(10100), "Quiz exact ten-second deadline");
    check(quizState.Respond(7, 1, 10099, quizAnswer) && quizAnswer.Header.Size == 36 &&
        quizAnswer.Header.Type == 0x7F11 && quizAnswer.Header.ID == 7 && quizAnswer.Version == 1 &&
        quizAnswer.Choice == 1 && quizAnswer.Token[0] == 0xA5 && quizAnswer.Reserved == 0,
        "Quiz answer contains only chosen index and token");
    check(!quizState.Respond(7, 1, 10099, quizAnswer) && !quizState.Apply(quiz, 10100), "Quiz response is single-use");
    quiz.Token[1] = 1;
    check(quizState.Apply(quiz, 0xFFFFFF00u) && !quizState.Expired(0x00000020u), "Quiz timer survives DWORD wrap");
    check(!quizState.Respond(7, 1, 0xFFFFFF00u + 10000u, quizAnswer), "Quiz rejects late answer");
    quiz.Token[1] = 2; quizState.Apply(quiz, 0);
    auto closeQuiz = quiz; closeQuiz.Kind = 0; closeQuiz.Token[1] = 1;
    check(!quizState.Apply(closeQuiz, 1) && quizState.Active, "Old close cannot close new round");
    closeQuiz.Token = quiz.Token;
    check(quizState.Apply(closeQuiz, 1) && !quizState.Active, "Matching close dismisses quiz");
    return failures;
}
