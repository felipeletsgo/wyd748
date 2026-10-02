#include "../internal/application/ports/PacketView.h"
#include "../internal/application/FieldInteractionPolicy.h"
#include "../internal/application/NativeVolatileRoutes.h"
#include "../internal/application/ports/PacketDispatch.h"
#include "../internal/wire/PacketSendBoundary.h"
#include "../internal/platform/windows/SocketTransport.h"
#include "../internal/application/RequestCharacterLogin.h"
#include "../internal/wire/CharacterLoginSender.h"
#include "../internal/wire/PartyAcceptPacket.h"
#include "../internal/wire/MissingEntityRequestPacket.h"
#include "../internal/wire/RestartRecallPacket.h"
#include "../internal/wire/KeepalivePingPacket.h"
#include "../internal/wire/ChangeCityPacket.h"
#include "../internal/wire/ReqTeleportPacket.h"
#include "../internal/wire/UseNPCPacket.h"
#include "../internal/wire/GuildDeprivatePacket.h"
#include "../internal/wire/ChallengeConfirmPacket.h"
#include "../internal/wire/GuildRelationPacket.h"
#include "../internal/wire/AttackFrameContract.h"
#include "../internal/wire/AirMoveContract.h"
#include "../internal/game/entities/AirMoveMotion.h"
#include "../internal/wire/ServerWarLetterContract.h"
#include <array>
#include <cstring>
#include <type_traits>
#include <climits>
#include <cstdio>
#include <limits>
#include "../internal/platform/network/SendBuffer.h"
#include "../internal/platform/network/ReceiveBuffer.h"

// Isolated application suite, compiled without this runner's wire includes.
int RunCharacterLoginUseCaseTests(int& checks);
int RunReceivedPacketDispatchTests(int& checks);
int RunCargoSlotTests(int& checks);
int RunGridInsertionTests(int& checks);
int RunCostumeSelectionTests(int& checks);
int RunHumanCostumeRefinementTests(int& checks);
int RunFieldSceneChatContractTests(int& checks);
int RunFieldChatControlTests(int& checks);
int RunFieldChatSubmitPolicyTests(int& checks);
int RunFieldCoinInputPolicyTests(int& checks);
int RunSkillAttackRequestTests(int& checks);
int RunAttackVisualDamageTests(int& checks);
int RunAttackAttackerStateTests(int& checks);
int RunAttackTargetStateTests(int& checks);
int RunGroundAttachTableTests(int& checks);
int RunFallbackCostumeTableTests(int& checks);
int RunResourceBarProjectionTests(int& checks);
int RunObservedAffectProjectionTests(int& checks);
int RunCCModePolicyTests(int& checks);
int RunEffectVertexColorTests(int& checks);
int RunSceneDisconnectContractTests(int& checks);
int RunLoginCredentialContractTests(int& checks);
int RunServerNameAssetTests(int& checks);
int RunTerrainTileMapReaderTests(int& checks);
int RunCharacterTransferResponseTests(int& checks);

// Socket-free backend: records metadata and uses the production guard.
// Does not retain the buffer; mutation simulates synchronous header filling.
struct FakeSocket
{
    int calls = 0;
    int accepted = 0;
    bool result = true;
    unsigned int opcode = 0;
    std::size_t size = 0;
    int SendPacket(const MutablePacketView& packet)
    {
        ++calls;
        opcode = packet.opcode;
        size = packet.size;
        return SendValidatedPacket(packet, 12, [&](char* data, int) {
            ++accepted;
            data[1] = 23;
            return result;
        });
    }
};

// Port spy: copies only for test inspection, never retains the temporary
// use-case pointer. No socket is needed to check the wire representation.
struct RecordingTransport final : ITransport
{
    int calls = 0;
    bool result = true;
    unsigned int opcode = 0;
    std::size_t size = 0;
    std::array<char, 36> bytes{};
    bool Send(const MutablePacketView& packet) override
    {
        ++calls;
        opcode = packet.opcode;
        size = packet.size;
        if (!packet.data || packet.size != bytes.size()) return false;
        std::memcpy(bytes.data(), packet.data, bytes.size());
        return result;
    }
};

// Local size-boundary tests without sockets, Win32 or DirectX.
// Do not use assert: checks must remain active in Release.
int main()
{
    char storage[16] = {};
    int failures = 0;
    int checks = 0;
    const auto check = [&failures, &checks](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            std::fprintf(stderr, "FAIL: %s\n", name);
            ++failures;
        }
    };

    check(!PacketView{}.HasAtLeast(0), "empty view has no buffer");
    check(!PacketView{0, nullptr, 16}.HasSizeBetween(12, INT_MAX), "null buffer with size");
    check(!PacketView{0, storage, 0}.HasSizeBetween(12, INT_MAX), "zero size");
    check(!PacketView{0, storage, 11}.HasSizeBetween(12, INT_MAX), "below minimum");
    check(PacketView{0, storage, 12}.HasSizeBetween(12, INT_MAX), "inclusive minimum");
    check(PacketView{0, storage, 16}.HasSizeBetween(12, 16), "inclusive maximum");
    check(!PacketView{0, storage, 16}.HasSizeBetween(12, 15), "above maximum");
    check(!PacketView{0, storage, 16}.HasSizeBetween(17, 16), "reversed interval");

    // Synthetic lengths: the predicate must never dereference data.
    const auto intOverflow = static_cast<std::size_t>(INT_MAX) + 1;
    check(!PacketView{0, storage, intOverflow}.HasSizeBetween(12, INT_MAX), "int overflow");
    check(PacketView{0, storage, INT_MAX}.HasSizeBetween(12, INT_MAX), "int limit");
    const auto sizeMaximum = (std::numeric_limits<std::size_t>::max)();
    check(!PacketView{0, storage, sizeMaximum}.HasSizeBetween(12, INT_MAX), "size_t limit");

    for (std::size_t size = 0; size <= 160; ++size)
    {
        check(IsClientToServerAttackPacketSize(MSG_Attack_One_Opcode, size) ==
            (size == 48 || size == 96), "AttackOne C->S preserves the closed set");
        check(IsClientToServerAttackPacketSize(MSG_Attack_Two_Opcode, size) ==
            (size == 52), "AttackTwo C->S preserves the closed set");
        check(IsClientToServerAttackPacketSize(MSG_Attack_Multi_Opcode, size) ==
            (size == 96), "AttackMulti C->S preserves the closed set");
    }
    check(!IsClientToServerAttackPacketSize(0x123, 48),
        "non-attack opcode does not reuse the C->S contract");
    check(!IsClientToServerAttackPacketSize(MSG_Attack_One_Opcode, 72),
        "AttackOne C->S rejects the incorrect legacy 72-byte envelope");

    check(MSG_AirMove_Start_Opcode == 0xAD9 && kAirMovePacketSize == 20 &&
        kAirMoveRouteOffset == 12 && kAirMoveModeOffset == 16,
        "AirMove C->S preserves the 7.48 opcode and ABI");
    check(kAirMoveStartMode == 1 && kAirMoveEndMode == 2,
        "AirMove C->S preserves both native modes");
    for (int route = -1; route <= kAirMoveRouteCount; ++route)
        check(IsValidAirMoveRouteIndex(route) == (route >= 0 && route < 5),
            "AirMove accepts only five native routes");
    struct AirMovePoint { float x; float y; };
    AirMovePoint airPosition{100.0f, 200.0f};
    AirMovePoint airDelta{0.25f, -0.5f};
    ConsumeAirMoveDelta(airPosition, airDelta);
    check(airPosition.x == 100.25f && airPosition.y == 199.5f &&
        airDelta.x == 0.0f && airDelta.y == 0.0f,
        "AirMove applies the frame delta exactly once");
    ConsumeAirMoveDelta(airPosition, airDelta);
    check(airPosition.x == 100.25f && airPosition.y == 199.5f,
        "AirMove does not reapply the consumed delta in the next frame");
    struct AirMoveLook { short mesh; short skin; };
    int bodySkin = 12;
    int mountSkin = 40;
    AirMoveLook mountLook{};
    const AirMoveLook savedMountLook{7, 3};
    RestoreAirMoveMountVisual(mountSkin, mountLook, 20, savedMountLook);
    check(bodySkin == 12 && mountSkin == 20 && mountLook.mesh == 7 &&
        mountLook.skin == 3,
        "AirMove restores mount appearance without changing the body mesh");
    struct AirMoveWaypoint { int nX; int nY; };
    AirMoveWaypoint airRoute[10]{{100, 200}, {300, 400}, {0, 0}};
    check(HasNextAirMoveWaypoint(airRoute, 0) &&
        !HasNextAirMoveWaypoint(airRoute, 1) &&
        !HasNextAirMoveWaypoint(airRoute, -1),
        "AirMove advances only to an existing waypoint");
    for (auto& point : airRoute)
        point = {500, 600};
    check(HasNextAirMoveWaypoint(airRoute, 8) &&
        !HasNextAirMoveWaypoint(airRoute, 9) &&
        !HasNextAirMoveWaypoint(airRoute, 10) &&
        !HasNextAirMoveWaypoint(airRoute, INT_MAX),
        "AirMove does not read beyond the last waypoint");

    check(MSG_UseDeclarationOfWar_Opcode == 0xED7,
        "declaration letter preserves the native opcode");
    check(MSG_UseRefuseServerWar_Opcode == 0xED8,
        "refusal letter preserves the native opcode");
    check(kServerWarLetterPacketSize == 16 && kServerWarTargetChannelOffset == 12,
        "war letters preserve the MSG_STANDARDPARM ABI");
    check(ServerWarPromptModeForItem(kDeclarationOfWarLetterItemIndex) ==
        kDeclareServerWarPromptMode, "item 4030 opens the declaration prompt");
    check(ServerWarPromptModeForItem(kWarRejectionLetterItemIndex) ==
        kRefuseServerWarPromptMode, "item 4031 opens the refusal prompt");
    check(ServerWarPromptModeForItem(4029) == 0 &&
        ServerWarPromptModeForItem(4032) == 0,
        "other items do not reuse the inter-channel war prompt");
    check(!IsEncodableServerWarTargetChannel(0) &&
        !IsEncodableServerWarTargetChannel(-1) &&
        IsEncodableServerWarTargetChannel(1) &&
        IsEncodableServerWarTargetChannel(2147483647LL) &&
        !IsEncodableServerWarTargetChannel(2147483648LL) &&
        !IsEncodableServerWarTargetChannel(4294967297LL),
        "war channel does not truncate a value outside the int32 wire field");

    const PacketView valid{77, storage, sizeof(storage)};
    check(valid.HasSizeBetween(12, INT_MAX) && valid.opcode == 77 &&
        valid.data == storage && storage[0] == 0, "validation without mutation");
    // The compiler prevents converting a read-only receive view into a send view.
    static_assert(!std::is_convertible<PacketView, MutablePacketView>::value,
        "Receive cannot remove const implicitly");
    static_assert(std::is_same<decltype(MutablePacketView::data), char*>::value,
        "Send requires writable storage");
    MutablePacketView outgoing{77, storage, sizeof(storage)};
    const auto incoming = outgoing.AsReadOnly();
    check(incoming.data == storage && incoming.size == sizeof(storage) &&
        incoming.opcode == 77, "read view borrows the same buffer");

    int sends = 0;
    const auto fakeSender = [&](char* data, int size) {
        ++sends;
        check(data == storage && size == sizeof(storage), "sender receives the original buffer and size");
        data[0] = 42;
        return true;
    };
    check(SendValidatedPacket(outgoing, 12, fakeSender), "success result preserved");
    check(sends == 1 && storage[0] == 42, "single send and visible mutation");
    check(!SendValidatedPacket({77, nullptr, 16}, 12, fakeSender), "null send rejected");
    check(!SendValidatedPacket({77, storage, 11}, 12, fakeSender), "short send rejected");
    check(!SendValidatedPacket({77, storage, intOverflow}, 12, fakeSender), "overflow send rejected");
    check(sends == 1, "rejections do not invoke sender");
    check(!SendValidatedPacket(outgoing, 12, [&](char*, int) {
        ++sends;
        return false;
    }), "sender failure propagated");
    check(sends == 2, "failure does not cause an implicit retry");
    // The virtual call exercises the real port without including Basedef or Win32.
    FakeSocket socket;
    {
        SocketTransport<FakeSocket> adapter(socket);
        ITransport& transport = adapter;
        check(transport.Send(outgoing), "port propagates success");
        check(socket.calls == 1 && socket.accepted == 1, "adapter sends once");
        check(socket.opcode == 77 && socket.size == sizeof(storage), "adapter preserves metadata");
        check(storage[1] == 23, "adapter preserves mutation in the original buffer");
        socket.result = false;
        check(!transport.Send(outgoing), "port propagates failure");
        check(socket.calls == 2, "adapter does not retry failure");
        check(!transport.Send({77, nullptr, 16}), "port rejects null in backend");
        check(!transport.Send({77, storage, 11}), "port rejects short size in backend");
        check(!transport.Send({77, storage, intOverflow}), "port rejects overflow in backend");
        check(socket.accepted == 2 && socket.calls == 5, "rejections do not reach sender");
    }
    check(socket.calls == 5, "destroying adapter neither closes nor sends through backend");
    RecordingTransport login;
    CharacterLoginSender loginSender(login);
    for (int slot = 0; slot < 4; ++slot)
    {
        check(RequestCharacterLogin(loginSender, slot), "login accepts valid slot");
        check(login.calls == slot + 1, "login requests a single send");
        check(login.opcode == 0x213 && login.size == 36, "login preserves opcode and size");
        // Expectation independent of the struct: all bytes zero except
        // little-endian opcode at +4 and slot at +12, before framing.
        std::array<char, 36> expected{};
        expected[4] = 0x13;
        expected[5] = 0x02;
        expected[12] = static_cast<char>(slot);
        check(login.bytes == expected, "login preserves all 36 bytes before sending");
    }
    check(!RequestCharacterLogin(loginSender, -1), "login rejects negative slot");
    check(!RequestCharacterLogin(loginSender, 4), "login rejects out-of-range slot");
    check(!RequestCharacterLogin(loginSender, INT_MAX), "login rejects extreme index");
    check(login.calls == 4, "invalid login does not send");
    login.result = false;
    check(!RequestCharacterLogin(loginSender, 0), "login propagates transport failure");
    check(login.calls == 5, "login does not retry sending after failure");

    // Dispatcher-independent fixture: 0x3AB exists only in the C->S direction.
    MSG_CNFParty2 partyAccept{};
    partyAccept.Header.Type = MSG_CNFParty2_Opcode;
    partyAccept.Header.ID = 0x1234;
    partyAccept.LeaderID = 0x0234;
    std::memcpy(partyAccept.LeaderName, "PartyLeader", 11);
    const auto* partyAcceptBytes = reinterpret_cast<const unsigned char*>(&partyAccept);
    check(sizeof(partyAccept) == 32 && partyAcceptBytes[4] == 0xAB &&
        partyAcceptBytes[5] == 0x03, "PartyAccept preserves opcode and 32-byte frame");
    check(partyAcceptBytes[12] == 0x34 && partyAcceptBytes[13] == 0x02 &&
        std::memcmp(partyAcceptBytes + 14, "PartyLeader", 11) == 0,
        "PartyAccept preserves leader and name at native offsets");
    check(partyAcceptBytes[25] == 0 && partyAcceptBytes[30] == 0 &&
        partyAcceptBytes[31] == 0, "PartyAccept zeroes terminator and reserved WORD");

    MSG_REQMobByID missingEntity{};
    missingEntity.Header.Type = MSG_REQMobByID_Opcode;
    missingEntity.Header.ID = 0x1234;
    missingEntity.MobID = 0x0234;
    const auto* missingEntityBytes = reinterpret_cast<const unsigned char*>(&missingEntity);
    check(sizeof(missingEntity) == 16 && missingEntityBytes[4] == 0x69 &&
        missingEntityBytes[5] == 0x03 && missingEntityBytes[12] == 0x34 &&
        missingEntityBytes[13] == 0x02,
        "MissingEntity preserves opcode, size and MobID");
    check(missingEntityBytes[14] == 0 && missingEntityBytes[15] == 0,
        "MissingEntity zeroes reserved WORD");

    MSG_STANDARD restartRecall{};
    restartRecall.Type = MSG_Recall_Opcode;
    restartRecall.ID = 0x1234;
    const auto* restartRecallBytes = reinterpret_cast<const unsigned char*>(&restartRecall);
    check(sizeof(restartRecall) == 12 && restartRecallBytes[4] == 0x89 &&
        restartRecallBytes[5] == 0x02 && restartRecallBytes[6] == 0x34 &&
        restartRecallBytes[7] == 0x12,
        "RestartRecall preserves opcode, size and ID");

    MSG_STANDARD fieldPing{};
    fieldPing.Type = MSG_Ping_Opcode;
    fieldPing.ID = 0x1234;
    const auto* fieldPingBytes = reinterpret_cast<const unsigned char*>(&fieldPing);
    check(sizeof(fieldPing) == 12 && fieldPingBytes[4] == 0xA0 &&
        fieldPingBytes[5] == 0x03 && fieldPingBytes[6] == 0x34 &&
        fieldPingBytes[7] == 0x12,
        "Keepalive Field preserves opcode, size and local ID");
    MSG_STANDARD selectCharPing{};
    selectCharPing.Type = MSG_Ping_Opcode;
    check(selectCharPing.ID == 0 && sizeof(selectCharPing) == 12,
        "Keepalive SelectChar preserves zero ID and header-only frame");

    MSG_ChangeCity changeCity{};
    changeCity.Header.Type = MSG_ChangeCity_Opcode;
    changeCity.Header.ID = 0x1234;
    changeCity.Village = 3;
    const auto* changeCityBytes = reinterpret_cast<const unsigned char*>(&changeCity);
    check(sizeof(changeCity) == 16 && changeCityBytes[4] == 0x91 &&
        changeCityBytes[5] == 0x02 && changeCityBytes[6] == 0x34 &&
        changeCityBytes[7] == 0x12 && changeCityBytes[12] == 3,
        "ChangeCity preserves opcode, size, ID and village");
    check(offsetof(MSG_ChangeCity, Village) == 12 && changeCityBytes[13] == 0 &&
        changeCityBytes[14] == 0 && changeCityBytes[15] == 0,
        "ChangeCity keeps Village DWORD at the native offset");

    MSG_ReqTeleport teleport{};
    teleport.Header.Type = MSG_ReqTeleport_Opcode;
    teleport.Header.ID = 0x1234;
    const auto* teleportBytes = reinterpret_cast<const unsigned char*>(&teleport);
    check(sizeof(teleport) == 16 && teleportBytes[4] == 0x90 &&
        teleportBytes[5] == 0x02 && teleportBytes[6] == 0x34 &&
        teleportBytes[7] == 0x12 && teleport.Reserved == 0,
        "ReqTeleport preserves opcode, size, ID and zero reserved field");
    check(offsetof(MSG_ReqTeleport, Reserved) == 12 && teleportBytes[13] == 0 &&
        teleportBytes[14] == 0 && teleportBytes[15] == 0,
        "ReqTeleport keeps reserved payload at the native offset");

    MSG_UseNPC useNPC{};
    useNPC.Header.Type = MSG_UseNPC_Opcode;
    useNPC.Header.ID = 0x1234;
    useNPC.TargetID = 0x5678;
    useNPC.ClickOk = 1;
    const auto* useNPCBytes = reinterpret_cast<const unsigned char*>(&useNPC);
    check(sizeof(useNPC) == 20 && useNPCBytes[4] == 0x8B &&
        useNPCBytes[5] == 0x02 && useNPCBytes[12] == 0x78 &&
        useNPCBytes[13] == 0x56 && useNPCBytes[16] == 1,
        "UseNPC preserves opcode, size, target and confirmation");
    check(offsetof(MSG_UseNPC, TargetID) == 12 &&
        offsetof(MSG_UseNPC, ClickOk) == 16 && useNPCBytes[17] == 0 &&
        useNPCBytes[18] == 0 && useNPCBytes[19] == 0,
        "UseNPC keeps native offsets");

    MSG_GuildDeprivate guildDeprivate{};
    guildDeprivate.Header.Type = MSG_GuildDeprivate_Opcode;
    guildDeprivate.Header.ID = 0x1234;
    guildDeprivate.TargetID = 0x5678;
    const auto* guildDeprivateBytes = reinterpret_cast<const unsigned char*>(&guildDeprivate);
    check(sizeof(guildDeprivate) == 16 && guildDeprivateBytes[4] == 0x8C &&
        guildDeprivateBytes[5] == 0x02 && guildDeprivateBytes[12] == 0x78 &&
        guildDeprivateBytes[13] == 0x56,
        "GuildDeprivate preserves opcode, size and target");
    check(offsetof(MSG_GuildDeprivate, TargetID) == 12 && guildDeprivateBytes[14] == 0 &&
        guildDeprivateBytes[15] == 0,
        "GuildDeprivate keeps target at the native offset");

    MSG_ChallengeConfirm challengeConfirm{};
    challengeConfirm.Header.Type = MSG_ChallengeConfirm_Opcode;
    challengeConfirm.Header.ID = 0x1234;
    challengeConfirm.Parm1 = 0x5678;
    challengeConfirm.Parm2 = 0;
    const auto* challengeConfirmBytes = reinterpret_cast<const unsigned char*>(&challengeConfirm);
    check(sizeof(challengeConfirm) == 20 && challengeConfirmBytes[4] == 0x8F &&
        challengeConfirmBytes[5] == 0x02 && challengeConfirmBytes[12] == 0x78 &&
        challengeConfirmBytes[13] == 0x56 && challengeConfirm.Parm2 == 0,
        "ChallengeConfirm preserves opcode, size and native Parm1 and Parm2");
    check(offsetof(MSG_ChallengeConfirm, Parm1) == 12 &&
        offsetof(MSG_ChallengeConfirm, Parm2) == 16 && challengeConfirmBytes[17] == 0 &&
        challengeConfirmBytes[18] == 0 && challengeConfirmBytes[19] == 0,
        "ChallengeConfirm keeps native offsets");

    MSG_GuildRelation guildRelation{};
    guildRelation.Header.Type = MSG_GuildWar_Opcode;
    guildRelation.Header.ID = 0x1234;
    guildRelation.GuildID = 0x5678;
    guildRelation.TargetGuildID = 0x9ABC;
    const auto* guildRelationBytes = reinterpret_cast<const unsigned char*>(&guildRelation);
    check(sizeof(guildRelation) == 20 && guildRelationBytes[4] == 0x0E &&
        guildRelationBytes[5] == 0x0E && guildRelationBytes[12] == 0x78 &&
        guildRelationBytes[13] == 0x56 && guildRelationBytes[16] == 0xBC &&
        guildRelationBytes[17] == 0x9A,
        "GuildRelation preserves war opcode, size and guilds");
    guildRelation.Header.Type = MSG_GuildAlly_Opcode;
    check(guildRelationBytes[4] == 0x12 && guildRelationBytes[5] == 0x0E &&
        offsetof(MSG_GuildRelation, GuildID) == 12 &&
        offsetof(MSG_GuildRelation, TargetGuildID) == 16,
        "GuildRelation preserves alliance opcode and native offsets");

    for (char key : {'Q', 'q', 'W', 'w', 'E', 'e', 'R', 'r', 'T', 't'})
        check(field_interaction::QuickSlotIndex(true, key) == -1,
            "native HUD never dereferences modern quick slots");
    check(field_interaction::QuickSlotIndex(false, 'q') == 0 &&
        field_interaction::QuickSlotIndex(false, 'T') == 4 &&
        field_interaction::QuickSlotIndex(false, '-') == -1,
        "modern quick slots preserve key mapping");
    check(!field_interaction::ShouldCloseOnDelayAck(0),
        "recall/logout/server-select/unsolicited ack does not close client");
    check(field_interaction::ShouldCloseOnDelayAck(1) &&
        field_interaction::ShouldCloseOnDelayAck(UINT_MAX),
        "pending local quit accepts delay ack");
    for (const auto& entry : native_volatile::CodeRoutes)
        for (int code = entry.first; code <= entry.last; ++code)
            check(native_volatile::Resolve(code, -1) == entry.route, "volatile code route");
    for (const auto& entry : native_volatile::ItemRoutes)
        for (int item = entry.first; item <= entry.last; ++item)
            check(native_volatile::Resolve(-1, item) == entry.route, "volatile item override");
    check(native_volatile::Resolve(19, 3442) == native_volatile::Route::CustomFirework,
        "custom firework must not send ordinary UseItem");
    check(native_volatile::Resolve(0, 4003) == native_volatile::Route::Direct,
        "catalogued box overrides non-direct volatile zero");
    check(native_volatile::Resolve(206, 3455) == native_volatile::Route::Interaction,
        "sealed item is not an extraction capsule");
    check(native_volatile::Resolve(-1, -1) == native_volatile::Route::Unknown &&
        native_volatile::Resolve(999, 999) == native_volatile::Route::Unknown,
        "unknown items preserve the legacy fallback");
    for (auto route : {native_volatile::Route::Direct, native_volatile::Route::Confirm,
        native_volatile::Route::Capsule})
        check(native_volatile::AllowsRightClick(route), "direct/modal right click route");
    for (auto route : {native_volatile::Route::Unknown, native_volatile::Route::Target,
        native_volatile::Route::Interaction, native_volatile::Route::Recall,
        native_volatile::Route::Portal, native_volatile::Route::CustomFirework})
        check(!native_volatile::AllowsRightClick(route), "specialized route is not direct consumption");

    failures += RunCharacterLoginUseCaseTests(checks);
    // Queue limits without signed addition or dependence on a real socket.
    check(send_buffer::CanAppendPacket(12, 0, 131072, 12), "queue accepts header");
    check(send_buffer::CanAppendPacket(65535, 0, 131072, 12), "queue accepts WORD limit");
    check(!send_buffer::CanAppendPacket(65536, 0, 131072, 12), "queue rejects WORD truncation");
    check(!send_buffer::CanAppendPacket(INT_MAX, INT_MAX, 131072, 12), "queue rejects overflow");
    check(!send_buffer::CanAppendPacket(-1, 0, 131072, 12), "queue rejects negative size");
    check(!send_buffer::CanAppendPacket(11, 0, 131072, 12), "queue rejects short header");
    check(!send_buffer::CanAppendPacket(12, -1, 131072, 12), "queue rejects negative index");
    check(!send_buffer::CanAppendPacket(12, 131060, 131072, 12), "queue preserves strict limit");
    check(send_buffer::CanAppendPacket(12, 131059, 131072, 12), "queue accepts last valid interval");
    check(send_buffer::CanAppendRaw(0, 131072, 131072), "empty raw append accepts full queue");
    check(receive_buffer::CanReadFrame(12, 12, 12), "receive accepts minimum frame");
    PacketView dispatchable{1, "1234", 4};
    check(packet_dispatch::CanDispatch(dispatchable, 4), "dispatch accepts complete view");
    check(!packet_dispatch::CanDispatch(dispatchable, 5), "dispatch rejects short view");
    // The policy delivers the same borrowed view once; rejected frames
    // do not reach the receiver. The callback can observe all metadata.
    int dispatchCalls = 0;
    auto receiver = [&](const PacketView& frame) {
        ++dispatchCalls;
        check(frame.data == dispatchable.data && frame.size == dispatchable.size &&
            frame.opcode == dispatchable.opcode, "dispatch preserves borrowed view and metadata");
    };
    check(packet_dispatch::Dispatch(dispatchable, 4, receiver), "dispatch delivers complete frame");
    check(dispatchCalls == 1, "dispatch does not repeat callback");
    check(!packet_dispatch::Dispatch(dispatchable, 5, receiver), "dispatch does not deliver short frame");
    check(!packet_dispatch::Dispatch({1, nullptr, 4}, 4, receiver), "dispatch does not deliver null");
    check(!packet_dispatch::Dispatch({}, 4, receiver), "dispatch does not confuse local event with frame");
    check(dispatchCalls == 1, "rejections do not call receiver");
    check(receive_buffer::HasValidWindow(0, 0, 131072), "receive accepts empty window");
    check(!receive_buffer::HasValidWindow(4, 3, 131072), "receive rejects reversed cursor");
    check(!receive_buffer::HasValidWindow(0, 0, 0), "receive rejects zero capacity");
    check(!receive_buffer::HasValidWindow(-1, 0, 131072), "receive rejects negative cursor");
    check(!receive_buffer::CanReadFrame(11, 12, 12), "receive rejects short frame");
    check(!receive_buffer::CanReadFrame(13, 12, 12), "receive rejects incomplete frame");
    check(send_buffer::CanAppendRaw(4, 131068, 131072), "raw append accepts exact limit");
    check(!send_buffer::CanAppendRaw(5, 131068, 131072), "raw append rejects overflow");
    check(!send_buffer::CanAppendRaw(INT_MAX, INT_MAX, 131072), "raw append rejects signed overflow");
    char queuedBytes[] = "abcdefgh";
    int queued = 8;
    int sent = 2;
    check(send_buffer::Compact(queuedBytes, 8, queued, sent), "compaction accepts partial send");
    check(queued == 6 && sent == 0 && std::memcmp(queuedBytes, "cdefgh", 6) == 0,
        "compaction preserves overlapping output suffix");
    check(send_buffer::Compact(queuedBytes, 8, queued, sent) && queued == 6,
        "repeated compaction without prefix is idempotent");
    sent = queued;
    check(send_buffer::Compact(queuedBytes, 8, queued, sent) && queued == 0 && sent == 0,
        "compaction clears complete send");
    queued = 9;
    sent = 1;
    check(!send_buffer::Compact(queuedBytes, 8, queued, sent) && queued == 9 && sent == 1,
        "invalid compaction does not change indices");
    check(!send_buffer::Compact(nullptr, 8, queued, sent), "compaction rejects null");
    queued = 3;
    sent = 4;
    check(!send_buffer::Compact(queuedBytes, 8, queued, sent), "compaction rejects sent greater than queued");
    for (int accepted = 0; accepted < 2; ++accepted) {
        for (int flushed = 0; flushed < 2; ++flushed) {
            int stage = 0;
            const bool result = send_buffer::EnqueueAndFlush(
                [&] { check(stage++ == 0, "enqueue precedes flush"); return accepted; },
                [&] { check(stage++ == 1, "flush occurs even on rejection"); return flushed; });
            check(result == (accepted != 0 && flushed != 0) && stage == 2,
                "result neither masks failure nor repeats calls");
        }
    }
    failures += RunReceivedPacketDispatchTests(checks);
    failures += RunCargoSlotTests(checks);
    failures += RunGridInsertionTests(checks);
    failures += RunCostumeSelectionTests(checks);
    failures += RunHumanCostumeRefinementTests(checks);
    failures += RunFieldSceneChatContractTests(checks);
    failures += RunFieldChatControlTests(checks);
    failures += RunFieldChatSubmitPolicyTests(checks);
    failures += RunFieldCoinInputPolicyTests(checks);
    failures += RunSkillAttackRequestTests(checks);
    failures += RunAttackVisualDamageTests(checks);
    failures += RunAttackAttackerStateTests(checks);
    failures += RunAttackTargetStateTests(checks);
    failures += RunGroundAttachTableTests(checks);
    failures += RunFallbackCostumeTableTests(checks);
    failures += RunResourceBarProjectionTests(checks);
    failures += RunObservedAffectProjectionTests(checks);
    failures += RunCCModePolicyTests(checks);
    failures += RunEffectVertexColorTests(checks);
    failures += RunSceneDisconnectContractTests(checks);
    failures += RunLoginCredentialContractTests(checks);
    failures += RunServerNameAssetTests(checks);
    failures += RunTerrainTileMapReaderTests(checks);
    failures += RunCharacterTransferResponseTests(checks);
    if (failures == 0) std::printf("ArchitectureTests: %d checks PASS; static assertions PASS\n", checks);
    return failures == 0 ? 0 : 1;
}
