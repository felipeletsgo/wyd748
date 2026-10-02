#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: SessionAndParty. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchSessionAndPartyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    std::array<char, 129> whisper{};
    whisper[0] = static_cast<char>(128);
    whisper[4] = 0x34; whisper[5] = 3;
    std::memcpy(whisper.data() + 12, "Remetente", 10);
    std::memcpy(whisper.data() + 28, "--Canal", 8);
    whisper[124] = 3;
    int whisperCalls = 0;
    const auto receiveWhisper = [&](const PacketView& view) {
        ++whisperCalls;
        check(view.data == whisper.data() && view.size == 128, "whisper preserves the view");
    };
    for (std::size_t n = 0; n < 128; ++n)
        check(!received_packet::Dispatch({0x334, whisper.data(), n}, receiveWhisper), "truncated whisper rejected");
    check(!received_packet::Dispatch({0x334, whisper.data(), 129}, receiveWhisper), "oversized whisper");
    check(!received_packet::Dispatch({0x334, nullptr, 128}, receiveWhisper), "null whisper");
    check(!received_packet::Dispatch({0x119, whisper.data(), 128}, receiveWhisper), "whisper Type hidden");
    whisper[4] = 0x19;
    check(!received_packet::Dispatch({0x334, whisper.data(), 128}, receiveWhisper), "mismatched whisper Type");
    whisper[4] = 0x34; whisper[0] = 127;
    check(!received_packet::Dispatch({0x334, whisper.data(), 128}, receiveWhisper), "mismatched whisper Size");
    whisper[0] = static_cast<char>(128);
    check(whisperCalls == 0, "whisper rejected without callback");
    const auto whisperBefore = whisper;
    check(received_packet::Dispatch({0x334, whisper.data(), 128}, receiveWhisper) && whisperCalls == 1,
        "valid whisper delivered once");
    MSG_MessageWhisper decodedWhisper{};
    std::memcpy(&decodedWhisper, whisper.data(), 128);
    check(std::strcmp(decodedWhisper.MobName, "Remetente") == 0 &&
        std::strcmp(decodedWhisper.String, "--Canal") == 0 && decodedWhisper.Color == 3,
        "fixture whisper confirma offsets nome texto e cor");
    check(whisper == whisperBefore, "whisper preserves bytes before the handler");
    std::array<char, 13> logout{};
    logout[0] = 12;
    logout[4] = 0x16;
    logout[5] = 1;
    logout[6] = 0x34;
    logout[7] = 0x12;
    int logoutCalls = 0;
    const auto receiveLogout = [&](const PacketView& view) {
        ++logoutCalls;
        check(view.data == logout.data() && view.size == 12,
            "logout confirms the 12-byte view");
    };
    for (std::size_t n = 0; n < 12; ++n)
        check(!received_packet::Dispatch({0x116, logout.data(), n}, receiveLogout),
            "truncated logout rejected");
    check(!received_packet::Dispatch({0x116, logout.data(), 13}, receiveLogout),
        "oversized logout rejected");
    check(!received_packet::Dispatch({0x116, nullptr, 12}, receiveLogout),
        "null logout rejected");
    check(!received_packet::Dispatch({0x119, logout.data(), 12}, receiveLogout),
        "logout Type cannot be hidden");
    logout[4] = 0x19;
    check(!received_packet::Dispatch({0x116, logout.data(), 12}, receiveLogout),
        "mismatched logout Type rejected");
    logout[4] = 0x16;
    logout[0] = 11;
    check(!received_packet::Dispatch({0x116, logout.data(), 12}, receiveLogout),
        "mismatched logout Size rejected");
    logout[0] = 12;
    check(logoutCalls == 0, "invalid logout does not reach the callback");
    const auto logoutBefore = logout;
    check(received_packet::Dispatch({0x116, logout.data(), 12}, receiveLogout) && logoutCalls == 1,
        "valid logout delivered once");
    check(logout == logoutBefore, "logout gate preserves header and ID");
    MSG_CharacterLogout request{};
    request.ID = 0x1234;
    request.Type = MSG_CharacterLogout_Opcode;
    request.Size = sizeof(request);
    check(sizeof(request) == 12 && request.ID == 0x1234 &&
        request.Type == 0x215 && request.Size == 12,
        "C-S logout request preserves ID, opcode and size");
    std::array<char, 2105> loginConfirm{};
    loginConfirm[0] = static_cast<char>(2104 & 0xFF);
    loginConfirm[1] = static_cast<char>((2104 >> 8) & 0xFF);
    loginConfirm[4] = 0x14;
    loginConfirm[5] = 1;
    loginConfirm[6] = 0x34;
    loginConfirm[7] = 0x12;
    int loginCalls = 0;
    const auto receiveLogin = [&](const PacketView& view) {
        ++loginCalls;
        check(view.data == loginConfirm.data() && view.size == 2104,
            "login confirm preserves the complete frame");
    };
    for (std::size_t n = 0; n < 2104; n += 17)
        check(!received_packet::Dispatch({0x114, loginConfirm.data(), n}, receiveLogin),
            "login confirm rejects truncated prefixes");
    check(!received_packet::Dispatch({0x114, loginConfirm.data(), 2105}, receiveLogin),
        "oversized login confirm rejected");
    check(!received_packet::Dispatch({0x114, nullptr, 2104}, receiveLogin),
        "null login confirm rejected");
    check(!received_packet::Dispatch({0x119, loginConfirm.data(), 2104}, receiveLogin),
        "login confirm Type cannot be hidden");
    loginConfirm[4] = 0x19;
    check(!received_packet::Dispatch({0x114, loginConfirm.data(), 2104}, receiveLogin),
        "mismatched login confirm Type rejected");
    loginConfirm[4] = 0x14;
    loginConfirm[0] = static_cast<char>(2103 & 0xFF);
    loginConfirm[1] = static_cast<char>((2103 >> 8) & 0xFF);
    check(!received_packet::Dispatch({0x114, loginConfirm.data(), 2104}, receiveLogin),
        "mismatched login confirm Size rejected");
    loginConfirm[0] = static_cast<char>(2104 & 0xFF);
    loginConfirm[1] = static_cast<char>((2104 >> 8) & 0xFF);
    check(loginCalls == 0, "invalid login confirm does not reach the callback");
    const auto loginBefore = loginConfirm;
    check(received_packet::Dispatch({0x114, loginConfirm.data(), 2104}, receiveLogin) && loginCalls == 1,
        "valid login confirm delivered once");
    check(loginConfirm == loginBefore, "login confirm preserves the relogin bytes");
    std::array<char, 25> arrayProbe{};
    arrayProbe[0] = 24;
    arrayProbe[4] = static_cast<char>(0xC1);
    arrayProbe[5] = 1;
    arrayProbe[12] = 99;
    arrayProbe[16] = static_cast<char>(0xFC);
    arrayProbe[17] = static_cast<char>(0xFF);
    arrayProbe[18] = static_cast<char>(0xFF);
    arrayProbe[19] = static_cast<char>(0xFF);
    int arrayCalls = 0;
    const auto receiveArray = [&](const PacketView& view) {
        ++arrayCalls;
        check(view.data == arrayProbe.data() && view.size == 24,
            "array probe preserves the 24-byte frame");
    };
    for (std::size_t n = 0; n < 24; n += 3)
        check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), n}, receiveArray),
            "truncated array probe rejected");
    check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), 25}, receiveArray),
        "oversized array probe rejected");
    check(!received_packet::Dispatch({0x1C1, nullptr, 24}, receiveArray),
        "null array probe rejected");
    check(!received_packet::Dispatch({0x119, arrayProbe.data(), 24}, receiveArray),
        "array probe Type cannot be hidden");
    arrayProbe[4] = 0x19;
    check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), 24}, receiveArray),
        "mismatched array probe Type rejected");
    arrayProbe[4] = static_cast<char>(0xC1);
    arrayProbe[0] = 23;
    check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), 24}, receiveArray),
        "mismatched array probe Size rejected");
    arrayProbe[0] = 24;
    check(arrayCalls == 0, "invalid array probe does not reach the callback");
    const auto arrayBefore = arrayProbe;
    check(received_packet::Dispatch({0x1C1, arrayProbe.data(), 24}, receiveArray) && arrayCalls == 1,
        "valid array probe delivered once");
    MSG_REQArray decodedArray{};
    std::memcpy(&decodedArray, arrayProbe.data(), sizeof(decodedArray));
    check(decodedArray.Category == 99 && decodedArray.ByteOffset == -4,
        "array probe preserves the category and the signed offset");
    check(arrayProbe == arrayBefore, "array probe preserves bytes before the handler");
    MSG_REQArray response = decodedArray;
    response.Header.Type = MSG_CNFArray_Opcode;
    response.Value = -128;
    check(response.Header.Type == 0x2C2 && response.Value == -128 && sizeof(response) == 24,
        "array response preserves opcode, signedness and size");
    std::array<char, 17> delayStart{};
    delayStart[0] = 16;
    delayStart[4] = static_cast<char>(0xAE);
    delayStart[5] = 0x03;
    delayStart[6] = 0x34;
    delayStart[7] = 0x12;
    delayStart[12] = 1;
    int delayCalls = 0;
    const auto receiveDelay = [&](const PacketView& view) {
        ++delayCalls;
        check(view.data == delayStart.data() && view.size == 16 && view.opcode == 0x3AE,
            "DelayStart preserves the 16-byte frame");
    };
    for (std::size_t n = 0; n < 16; ++n)
        check(!received_packet::Dispatch({0x3AE, delayStart.data(), n}, receiveDelay),
            "truncated DelayStart rejected");
    check(!received_packet::Dispatch({0x3AE, delayStart.data(), 17}, receiveDelay),
        "oversized DelayStart rejected");
    check(!received_packet::Dispatch({0x3AE, nullptr, 16}, receiveDelay),
        "null DelayStart rejected");
    check(!received_packet::Dispatch({0x119, delayStart.data(), 16}, receiveDelay),
        "DelayStart Type cannot be hidden");
    delayStart[4] = 0x19;
    check(!received_packet::Dispatch({0x3AE, delayStart.data(), 16}, receiveDelay),
        "mismatched DelayStart Type rejected");
    delayStart[4] = static_cast<char>(0xAE);
    delayStart[0] = 15;
    check(!received_packet::Dispatch({0x3AE, delayStart.data(), 16}, receiveDelay),
        "mismatched DelayStart Size rejected");
    delayStart[0] = 16;
    check(delayCalls == 0, "invalid DelayStart does not reach the consumer");
    const auto delayBefore = delayStart;
    check(received_packet::Dispatch({0x3AE, delayStart.data(), 16}, receiveDelay) && delayCalls == 1,
        "valid DelayStart delivered once");
    MSG_DelayStart decodedDelay{};
    std::memcpy(&decodedDelay, delayStart.data(), sizeof(decodedDelay));
    check(decodedDelay.Header.ID == 0x1234 && decodedDelay.Parm == 1,
        "DelayStart preserves the ID and the transition parameter");
    MSG_SysQuit sysQuit{};
    sysQuit.Header.Type = MSG_SysQuit_Opcode;
    sysQuit.Header.Size = sizeof(sysQuit);
    sysQuit.Header.ID = 0x1234;
    check(sizeof(sysQuit) == 16 && sysQuit.Header.Type == 0x3AE && sysQuit.Parm == 0,
        "SysQuit shares the ABI and preserves a zero Parm");
    check(delayStart == delayBefore, "gate preserves every DelayStart byte");
    std::array<char, 17> billing{};
    billing[0] = 16;
    billing[4] = static_cast<char>(0x94);
    billing[5] = 1;
    billing[12] = static_cast<char>(0xA5);
    billing[13] = static_cast<char>(0x5A);
    const auto billingBefore = billing;
    int billingCalls = 0;
    const auto receiveBilling = [&](const PacketView& view) {
        ++billingCalls;
        check(view.data == billing.data() && view.size == 16 && view.opcode == 0x194,
            "BillingNotice preserves the opaque 16-byte frame");
    };
    for (std::size_t n = 0; n < 16; ++n)
        check(!received_packet::Dispatch({0x194, billing.data(), n}, receiveBilling),
            "truncated BillingNotice rejected");
    check(!received_packet::Dispatch({0x194, billing.data(), 17}, receiveBilling),
        "oversized BillingNotice rejected");
    check(!received_packet::Dispatch({0x194, nullptr, 16}, receiveBilling),
        "null BillingNotice rejected");
    check(!received_packet::Dispatch({0x119, billing.data(), 16}, receiveBilling),
        "BillingNotice Type cannot be hidden");
    billing[4] = 0x19;
    check(!received_packet::Dispatch({0x194, billing.data(), 16}, receiveBilling),
        "mismatched BillingNotice Type rejected");
    billing[4] = static_cast<char>(0x94);
    billing[0] = 15;
    check(!received_packet::Dispatch({0x194, billing.data(), 16}, receiveBilling),
        "mismatched BillingNotice Size rejected");
    billing[0] = 16;
    check(billingCalls == 0, "invalid BillingNotice does not reach the consumer");
    check(received_packet::Dispatch({0x194, billing.data(), 16}, receiveBilling) && billingCalls == 1,
        "valid BillingNotice delivered once");
    MSG_BillingNotice decodedBilling{};
    std::memcpy(&decodedBilling, billing.data(), sizeof(decodedBilling));
    check(decodedBilling.Header.Type == 0x194 && decodedBilling.OpaquePayload[0] == 0xA5 &&
        decodedBilling.OpaquePayload[1] == 0x5A,
        "BillingNotice preserves opcode and opaque payload");
    check(billing == billingBefore, "gate preserves every BillingNotice byte");
    std::array<char, 21> motion{};
    motion[0] = 20;
    motion[4] = 0x6A;
    motion[5] = 0x03;
    motion[6] = 0x34;
    motion[7] = 0x12;
    motion[12] = 100;
    motion[14] = 5;
    motion[18] = static_cast<char>(0x80);
    motion[19] = 0x3F;
    int motionCalls = 0;
    const auto receiveMotion = [&](const PacketView& view) {
        ++motionCalls;
        check(view.data == motion.data() && view.size == 20 && view.opcode == 0x36A,
            "Motion preserves the 20-byte frame");
    };
    for (std::size_t n = 0; n < 20; ++n)
        check(!received_packet::Dispatch({0x36A, motion.data(), n}, receiveMotion),
            "truncated Motion rejected");
    check(!received_packet::Dispatch({0x36A, motion.data(), 21}, receiveMotion),
        "oversized Motion rejected");
    check(!received_packet::Dispatch({0x36A, nullptr, 20}, receiveMotion),
        "null Motion rejected");
    check(!received_packet::Dispatch({0x119, motion.data(), 20}, receiveMotion),
        "Motion Type cannot be hidden");
    motion[4] = 0x19;
    check(!received_packet::Dispatch({0x36A, motion.data(), 20}, receiveMotion),
        "mismatched Motion Type rejected");
    motion[4] = 0x6A;
    motion[0] = 19;
    check(!received_packet::Dispatch({0x36A, motion.data(), 20}, receiveMotion),
        "mismatched Motion Size rejected");
    motion[0] = 20;
    check(motionCalls == 0, "invalid Motion does not reach the consumer");
    const auto motionBefore = motion;
    check(received_packet::Dispatch({0x36A, motion.data(), 20}, receiveMotion) && motionCalls == 1,
        "valid Motion delivered once");
    MSG_Motion decodedMotion{};
    std::memcpy(&decodedMotion, motion.data(), sizeof(decodedMotion));
    check(decodedMotion.Header.ID == 0x1234 && decodedMotion.Motion == 100 &&
        decodedMotion.Parm == 5 && decodedMotion.Direction == 1.0f,
        "Motion preserves ID, motion, parameter and direction");
    check(motion == motionBefore, "gate preserves every Motion byte");

    std::array<char, 41> partyAdd{};
    partyAdd[0] = 40;
    partyAdd[4] = static_cast<char>(0x7D);
    partyAdd[5] = 0x03;
    partyAdd[6] = 0x34;
    partyAdd[7] = 0x12;
    partyAdd[12] = 2;
    partyAdd[13] = 1;
    partyAdd[14] = 55;
    partyAdd[16] = 0x20;
    partyAdd[18] = 0x10;
    partyAdd[20] = 0x34;
    partyAdd[21] = 0x12;
    std::memcpy(partyAdd.data() + 22, "Membro", 7);
    int partyCalls = 0;
    const auto receiveParty = [&](const PacketView& view) {
        ++partyCalls;
        check(view.data == partyAdd.data() && view.size == 40 && view.opcode == 0x37D,
            "PartyAdd preserves the 40-byte frame");
    };
    for (std::size_t n = 0; n < 40; ++n)
        check(!received_packet::Dispatch({0x37D, partyAdd.data(), n}, receiveParty),
            "truncated PartyAdd rejected");
    check(!received_packet::Dispatch({0x37D, partyAdd.data(), 41}, receiveParty),
        "oversized PartyAdd rejected");
    check(!received_packet::Dispatch({0x37D, nullptr, 40}, receiveParty),
        "null PartyAdd rejected");
    check(!received_packet::Dispatch({0x119, partyAdd.data(), 40}, receiveParty),
        "PartyAdd Type cannot be hidden");
    partyAdd[4] = 0x19;
    check(!received_packet::Dispatch({0x37D, partyAdd.data(), 40}, receiveParty),
        "mismatched PartyAdd Type rejected");
    partyAdd[4] = static_cast<char>(0x7D);
    partyAdd[0] = 39;
    check(!received_packet::Dispatch({0x37D, partyAdd.data(), 40}, receiveParty),
        "mismatched PartyAdd Size rejected");
    partyAdd[0] = 40;
    check(partyCalls == 0, "invalid PartyAdd does not reach the consumer");
    const auto partyBefore = partyAdd;
    check(received_packet::Dispatch({0x37D, partyAdd.data(), 40}, receiveParty) && partyCalls == 1,
        "valid PartyAdd delivered once");
    MSG_AddParty decodedParty{};
    std::memcpy(&decodedParty, partyAdd.data(), sizeof(decodedParty));
    check(decodedParty.Header.ID == 0x1234 && decodedParty.Party.ID == 0x1234 &&
        decodedParty.Party.Level == 55 && std::strcmp(decodedParty.Party.Name, "Membro") == 0,
        "PartyAdd preserves the member's ID, level and name");
    check(partyAdd == partyBefore, "gate preserves every PartyAdd byte");
    std::array<char, 17> partyRemove{};
    partyRemove[0] = 16;
    partyRemove[4] = static_cast<char>(0x7E);
    partyRemove[5] = 0x03;
    partyRemove[6] = 0x34;
    partyRemove[7] = 0x12;
    partyRemove[12] = 0x78;
    partyRemove[13] = 0x56;
    int removeCalls = 0;
    const auto receivePartyRemove = [&](const PacketView& view) {
        ++removeCalls;
        check(view.data == partyRemove.data() && view.size == 16 && view.opcode == 0x37E,
            "PartyRemove preserves the 16-byte frame");
    };
    for (std::size_t n = 0; n < 16; ++n)
        check(!received_packet::Dispatch({0x37E, partyRemove.data(), n}, receivePartyRemove),
            "truncated PartyRemove rejected");
    check(!received_packet::Dispatch({0x37E, partyRemove.data(), 17}, receivePartyRemove),
        "oversized PartyRemove rejected");
    check(!received_packet::Dispatch({0x37E, nullptr, 16}, receivePartyRemove),
        "null PartyRemove rejected");
    check(!received_packet::Dispatch({0x119, partyRemove.data(), 16}, receivePartyRemove),
        "PartyRemove Type cannot be hidden");
    partyRemove[4] = 0x19;
    check(!received_packet::Dispatch({0x37E, partyRemove.data(), 16}, receivePartyRemove),
        "mismatched PartyRemove Type rejected");
    partyRemove[4] = static_cast<char>(0x7E);
    partyRemove[0] = 15;
    check(!received_packet::Dispatch({0x37E, partyRemove.data(), 16}, receivePartyRemove),
        "mismatched PartyRemove Size rejected");
    partyRemove[0] = 16;
    check(removeCalls == 0, "invalid PartyRemove does not reach the consumer");
    const auto removeBefore = partyRemove;
    check(received_packet::Dispatch({0x37E, partyRemove.data(), 16}, receivePartyRemove) && removeCalls == 1,
        "valid PartyRemove delivered once");
    MSG_RemoveParty decodedRemove{};
    std::memcpy(&decodedRemove, partyRemove.data(), sizeof(decodedRemove));
    check(decodedRemove.Header.ID == 0x1234 && decodedRemove.Parm == 0x5678,
        "PartyRemove preserves the sender and the removed member");
    MSG_RemoveParty clearParty{};
    clearParty.Header.Type = MSG_RemoveParty_Opcode;
    clearParty.Header.Size = sizeof(clearParty);
    check(clearParty.Parm == 0, "PartyRemove preserves zero as clearing the party");
    check(partyRemove == removeBefore, "gate preserves every PartyRemove byte");
    std::array<char, 45> partyRequest{};
    partyRequest[0] = 44;
    partyRequest[4] = static_cast<char>(0x7F);
    partyRequest[5] = 0x03;
    partyRequest[6] = 0x34;
    partyRequest[7] = 0x12;
    partyRequest[12] = 3;
    partyRequest[14] = 80;
    partyRequest[16] = static_cast<char>(0xE8);
    partyRequest[17] = 0x03;
    partyRequest[18] = static_cast<char>(0xBC);
    partyRequest[19] = 0x02;
    partyRequest[20] = 0x34;
    partyRequest[21] = 0x12;
    std::memcpy(partyRequest.data() + 22, "Lider", 6);
    partyRequest[40] = 0x78;
    partyRequest[41] = 0x56;
    int requestCalls = 0;
    const auto receivePartyRequest = [&](const PacketView& view) {
        ++requestCalls;
        check(view.data == partyRequest.data() && view.size == 44 && view.opcode == 0x37F,
            "PartyRequest preserves the 44-byte frame");
    };
    for (std::size_t n = 0; n < 44; ++n)
        check(!received_packet::Dispatch({0x37F, partyRequest.data(), n}, receivePartyRequest),
            "truncated PartyRequest rejected");
    check(!received_packet::Dispatch({0x37F, partyRequest.data(), 45}, receivePartyRequest),
        "oversized PartyRequest rejected");
    check(!received_packet::Dispatch({0x37F, nullptr, 44}, receivePartyRequest),
        "null PartyRequest rejected");
    check(!received_packet::Dispatch({0x119, partyRequest.data(), 44}, receivePartyRequest),
        "PartyRequest Type cannot be hidden");
    partyRequest[4] = 0x19;
    check(!received_packet::Dispatch({0x37F, partyRequest.data(), 44}, receivePartyRequest),
        "mismatched PartyRequest Type rejected");
    partyRequest[4] = static_cast<char>(0x7F);
    partyRequest[0] = 43;
    check(!received_packet::Dispatch({0x37F, partyRequest.data(), 44}, receivePartyRequest),
        "mismatched PartyRequest Size rejected");
    partyRequest[0] = 44;
    check(requestCalls == 0, "invalid PartyRequest does not reach the consumer");
    const auto requestBefore = partyRequest;
    check(received_packet::Dispatch({0x37F, partyRequest.data(), 44}, receivePartyRequest) && requestCalls == 1,
        "valid PartyRequest delivered once");
    MSG_REQParty decodedRequest{};
    std::memcpy(&decodedRequest, partyRequest.data(), sizeof(decodedRequest));
    check(decodedRequest.Header.ID == 0x1234 && decodedRequest.Leader.ID == 0x1234 &&
        decodedRequest.Leader.Level == 80 && decodedRequest.Leader.MaxHp == 1000 &&
        decodedRequest.Leader.Hp == 700 && std::strcmp(decodedRequest.Leader.Name, "Lider") == 0 &&
        decodedRequest.TargetID == 0x5678,
        "PartyRequest preserves leader, HP and destination");
    check(partyRequest == requestBefore, "gate preserves every PartyRequest byte");
    return failures;
}
