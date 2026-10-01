#include "../internal/ui/UIBinary.h"
#include "../internal/core/ServerListAsset.h"
#include "../internal/core/ServerStatus.h"
#include "../internal/core/WYD748Assets.h"
#include "../internal/render/world/objects/ObjectFileRecordLayout.h"
#include "../internal/ui/SellConfirmationText.h"
#include "../internal/wire/AttackFrameContract.h"
#include "../internal/wire/ReceivedPacketDispatch.h"
#include "../internal/wire/LegacySalePacket.h"
#include "../internal/wire/SendItemContract.h"
#include "../internal/game/entities/AirMoveMotion.h"
#include "../internal/application/FieldInteractionPolicy.h"

#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>
#include <windows.h>

namespace {
std::filesystem::path FindSource(const char* relativePath)
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};

    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / relativePath;
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
        current = current.parent_path();
    }
    return {};
}

std::string NormalizeLineEndings(std::string source)
{
    source.erase(std::remove(source.begin(), source.end(), '\r'), source.end());
    return source;
}

std::string LoadSource(const char* relativePath)
{
    const auto path = FindSource(relativePath);
    if (path.empty())
        return {};

    std::ifstream input(path, std::ios::binary);
    // Git checkouts on Windows may use CRLF even when the committed source is
    // LF. Keep these source-contract checks independent of checkout policy.
    return NormalizeLineEndings({std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()});
}
}

// The full scene depends on DirectX and Win32 UI state. This source contract
// guards the ordering required by the native 7.48 FD_CLOSE lifecycle: the base
// scene must receive the null payload before the derived scene returns.
int RunSceneDisconnectContractTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL scene disconnect: %s\n", name);
        }
    };
    check(NormalizeLineEndings("case 12:\r\n\t\t\t{\r\n") == "case 12:\n\t\t\t{\n",
        "source contract normalizes CRLF checkout line endings");
    for (int total : {-1, 0, 1, 2, 10, 255, 256, INT_MAX}) {
        for (long long amount : {(std::numeric_limits<long long>::min)(), -1LL, 0LL,
            1LL, 2LL, 9LL, 10LL, 254LL, 255LL, 256LL,
            (std::numeric_limits<long long>::max)()}) {
            check(field_interaction::IsValidStackSplitQuantity(amount, total) ==
                (total >= 2 && total <= 255 && amount >= 1 && amount <= total - 1LL),
                "split quantity leaves two positive byte-sized stacks");
        }
    }
    int first = 1, last = 2, foreign = 3;
    int* ownedItems[] = {&first, nullptr, &last};
    for (int count : {-1, 0, 1, 2, 3, 4, INT_MAX}) {
        for (int* selected : {static_cast<int*>(nullptr), &first, &last, &foreign}) {
            int* expected = count >= 1 && count <= 3 && selected == &first ? &first :
                count == 3 && selected == &last ? &last : nullptr;
            check(field_interaction::FindOwnedItem(ownedItems, count, selected) == expected,
                "split selection requires membership in the active grid prefix");
        }
    }
    for (long long price : {(std::numeric_limits<long long>::min)(), -1LL, 0LL,
        1LL, 1999999999LL, 2000000000LL, 2147483647LL, 4294967295LL,
        (std::numeric_limits<long long>::max)()}) {
        check(field_interaction::IsValidAutoTradePrice(price) ==
            (price >= 1 && price <= 1999999999LL),
            "auto-trade price requires positive gold and preserves the existing prompt ceiling");
    }
    struct ListingItem { short sIndex; };
    for (unsigned int occupied = 0; occupied < (1u << 12); ++occupied) {
        ListingItem items[12]{};
        for (unsigned int slot = 0; slot < 12; ++slot)
            items[slot].sIndex = (occupied & (1u << slot)) ? 4011 : 0;
        check(field_interaction::HasAutoTradeOffers(items) == (occupied != 0),
            "shop publication accepts every nonempty twelve-slot occupancy pattern");
    }
    ListingItem invalidOffers[12]{};
    for (auto& item : invalidOffers) item.sIndex = -1;
    check(!field_interaction::HasAutoTradeOffers(invalidOffers),
        "negative item sentinels do not make a shop publishable");
    struct SoldItem { short sIndex; unsigned char effects[6]; };
    for (int sold : {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, INT_MAX}) {
        SoldItem items[12]{};
        unsigned char positions[12]{};
        int prices[12]{};
        for (int slot = 0; slot < 12; ++slot) {
            items[slot] = {static_cast<short>(4000 + slot), {1, 2, 3, 4, 5, 6}};
            positions[slot] = static_cast<unsigned char>(20 + slot);
            prices[slot] = 1000 + slot;
        }
        const bool valid = sold >= 0 && sold < 12;
        for (int repeat = 0; repeat < 2; ++repeat) {
            check(field_interaction::ClearAutoTradeOffer(items, positions, prices, sold) == valid,
                "sold-slot delta rejects invalid slots and accepts repeated valid notifications");
            for (int slot = 0; slot < 12; ++slot) {
                const SoldItem expected = slot == sold ? SoldItem{} :
                    SoldItem{static_cast<short>(4000 + slot), {1, 2, 3, 4, 5, 6}};
                check(memcmp(&items[slot], &expected, sizeof(expected)) == 0 &&
                    positions[slot] == (slot == sold ? 255 : 20 + slot) &&
                    prices[slot] == (slot == sold ? 0 : 1000 + slot),
                    "sold-slot delta clears item effects, carry mapping and price without changing other offers");
            }
        }
    }
    for (bool nativeHUD : {false, true}) {
        for (unsigned int control = 0; control <= 665; ++control) {
            const int expected = control >= 653 && control < (nativeHUD ? 665u : 663u)
                ? static_cast<int>(control - 653) : -1;
            check(field_interaction::AutoTradeSlotIndex(nativeHUD, control) == expected,
                "auto-trade control maps only to an available listing slot");
        }
        check(field_interaction::AutoTradeSlotIndex(nativeHUD, UINT_MAX) == -1,
            "auto-trade control rejects unsigned overflow boundary");
    }
    for (int sold = 0; sold < 12; ++sold) {
        for (int selected = 0; selected < 12; ++selected) {
            check(field_interaction::ShouldCancelAutoTradePurchase(646, 653u + selected, sold) == (sold == selected),
                "sold listing invalidates only its own purchase confirmation");
        }
    }
    check(field_interaction::ShouldCancelAutoTradePurchase(646, 0, -1) &&
        field_interaction::ShouldCancelAutoTradePurchase(646, UINT_MAX, -1),
        "snapshot replacement and close invalidate even malformed purchase selections");
    check(!field_interaction::ShouldCancelAutoTradePurchase(601, 653, -1) &&
        !field_interaction::ShouldCancelAutoTradePurchase(646, 652, 0) &&
        !field_interaction::ShouldCancelAutoTradePurchase(646, UINT_MAX, 0) &&
        !field_interaction::ShouldCancelAutoTradePurchase(646, 664, 12) &&
        !field_interaction::ShouldCancelAutoTradePurchase(646, 653, -2),
        "purchase invalidation preserves other dialogs and rejects invalid sold slots");
    // Exercise the actual receive gate, not a source-text proxy. This protects
    // the native 0x37A/20 envelope before the inherited lengthless callback.
    MSG_Sell sale{};
    sale.Header.Type = MSG_Sell_Opcode;
    sale.Header.Size = sizeof(sale);
    sale.TargetID = 17;
    sale.MyType = 1;
    sale.MyPos = 4;
    const unsigned char nativeSaleFrame[20] = {
        0x14, 0x00, 0x00, 0x00, 0x7A, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x01, 0x00,
        0x04, 0x00, 0x00, 0x00
    };
    check(std::memcmp(&sale, nativeSaleFrame, sizeof(nativeSaleFrame)) == 0,
        "legacy sale representation matches the fixed little-endian 20-byte fixture");
    char saleBytes[sizeof(sale) + 4]{};
    std::memcpy(saleBytes + 1, nativeSaleFrame, sizeof(nativeSaleFrame));
    const auto saleBefore = std::string(saleBytes, sizeof(saleBytes));
    int saleDeliveries = 0;
    const auto receiveSale = [&](const PacketView& frame) {
        ++saleDeliveries;
        check(frame.data == saleBytes + 1 && frame.size == sizeof(sale),
            "legacy sale borrows the original unaligned frame exactly once");
    };
    for (std::size_t size = 0; size < sizeof(sale); ++size) {
        check(!received_packet::Dispatch({MSG_Sell_Opcode, saleBytes + 1, size}, receiveSale),
            "legacy sale rejects every truncated representation before callback");
    }
    check(saleDeliveries == 0, "truncated legacy sale never reaches its handler");
    check(!received_packet::Dispatch({MSG_Sell_Opcode, nullptr, sizeof(sale)}, receiveSale),
        "legacy sale rejects null storage");
    check(received_packet::Dispatch({MSG_Sell_Opcode, saleBytes + 1, sizeof(sale)}, receiveSale) &&
        saleDeliveries == 1, "complete legacy sale is delivered once");
    check(std::string(saleBytes, sizeof(saleBytes)) == saleBefore,
        "legacy sale gate never modifies borrowed bytes");
    check(!received_packet::CanDispatch({0xFFFF, saleBytes + 1, sizeof(sale)}),
        "legacy sale embedded opcode cannot bypass the guard through metadata");
    sale.Header.Type = 0xFFFF;
    std::memcpy(saleBytes + 1, &sale, sizeof(sale));
    check(!received_packet::CanDispatch({MSG_Sell_Opcode, saleBytes + 1, sizeof(sale)}),
        "legacy sale metadata cannot disguise a different embedded opcode");
    sale.Header.Type = MSG_Sell_Opcode;
    for (unsigned short declared = 0; declared <= sizeof(sale) + 1; ++declared) {
        sale.Header.Size = declared;
        std::memcpy(saleBytes + 1, &sale, sizeof(sale));
        check(received_packet::CanDispatch({MSG_Sell_Opcode, saleBytes + 1, sizeof(sale)}) ==
            (declared == sizeof(sale)), "legacy sale requires consistent declared and actual lengths");
    }
    std::vector<char> oversizedSale(65536, 0);
    for (unsigned int declared : {21u, 24u, 65535u}) {
        sale.Header.Size = static_cast<unsigned short>(declared);
        std::memcpy(oversizedSale.data() + 1, &sale, sizeof(sale));
        const auto oversizedBefore = oversizedSale;
        check(!received_packet::Dispatch(
            {MSG_Sell_Opcode, oversizedSale.data() + 1, declared},
            [&](const PacketView&) { ++saleDeliveries; }),
            "legacy sale rejects consistent oversized frames before callback");
        check(oversizedSale == oversizedBefore,
            "rejected oversized sale preserves the original borrowed storage");
    }
    check(saleDeliveries == 1,
        "oversized sale frames never reach the inherited receiver");
    check(received_packet::ExpectedSize(MSG_Sell_Opcode) == 20,
        "legacy sale receive policy uses the native exact 20-byte envelope");
    struct AirMovePosition { float x; float y; };
    AirMovePosition flightPosition{ 2200.5f, 2100.5f };
    AirMovePosition flightDelta{ 3.0f, -2.0f };
    const AirMovePosition flightOrigin{ 2100.5f, 2100.5f };
    CancelAirMoveAtOrigin(flightPosition, flightDelta, flightOrigin);
    check(flightPosition.x == flightOrigin.x && flightPosition.y == flightOrigin.y &&
        flightDelta.x == 0.0f && flightDelta.y == 0.0f,
        "death discards flight displacement and restores the captured origin");
    flightPosition = { 2400.5f, 2300.5f };
    flightDelta = { 4.0f, 5.0f };
    DiscardAirMoveDelta(flightDelta);
    check(flightPosition.x == 2400.5f && flightPosition.y == 2300.5f &&
        flightDelta.x == 0.0f && flightDelta.y == 0.0f,
        "authoritative teleport keeps its position and discards flight displacement");

    const std::string source = LoadSource(
        "TMProject748/internal/app/scenes/TMSelectServerScene.cpp");
    check(!source.empty(), "select-server source is available");

    const std::string header = LoadSource(
        "TMProject748/internal/app/scenes/TMSelectServerScene.h");
    check(header.find("TMHuman* m_pCheckHumanList[50]{};") != std::string::npos,
        "demo humans are null before partial select-server initialization can fail");
    const auto loadRc = source.find("if (!LoadRC(\"UI\\\\SelServerScene2.txt\"))");
    const auto loadNames = source.find("if (!WYD748_LoadServerNameList(");
    const auto initializeHumans = source.find("memset(m_pCheckHumanList, 0, sizeof m_pCheckHumanList);");
    check(loadRc != std::string::npos && loadNames != std::string::npos &&
        initializeHumans != std::string::npos && loadRc < loadNames &&
        loadNames < initializeHumans &&
        source.find("if (m_pCheckHumanList[nPerson] != nullptr)") != std::string::npos,
        "resource failures can reach the destructor before in-scene initialization");
    const auto requiredControls = source.find("const int requiredControls[] = {");
    const auto initializeUI = source.find("InitializeUI();", requiredControls);
    const auto requireControl = source.find("if (!m_pControlContainer->FindControl(controlID))", requiredControls);
    check(requiredControls != std::string::npos && requireControl != std::string::npos &&
        initializeUI != std::string::npos && loadNames < requiredControls &&
        requiredControls < requireControl && requireControl < initializeUI &&
        source.find("P_SERVER_SEL, L_SELECT_SERVERG, L_SELECT_SERVER, P_LOGIN_BOX", requiredControls) != std::string::npos &&
        source.find("B_LOGIN_OK, B_QUIT, B_CREATE_ID", requiredControls) != std::string::npos &&
        source.find("E_LOGIN_ID, E_LOGIN_PASSWORD", requiredControls) != std::string::npos,
        "incomplete Scene2 controls are rejected before UI construction and login");
    const auto loadTerrain = source.find("if (!m_pGroundList[0]->LoadTileMap(szMapPath))");
    const auto copyTerrainMask = source.find("memcpy(m_HeightMapData[nY], m_pGround->m_pMaskData[nY], 128);", loadTerrain);
    check(loadTerrain != std::string::npos && copyTerrainMask != std::string::npos &&
        source.substr(loadTerrain, copyTerrainMask - loadTerrain).find("m_bCriticalError = 1;") != std::string::npos &&
        source.substr(loadTerrain, copyTerrainMask - loadTerrain).find("return 0;") != std::string::npos,
        "missing 7.48 terrain stops scene initialization before the mask is used");

    const auto ground = LoadSource("TMProject748/internal/render/world/terrain/TMGround.cpp");
    const auto record = ground.find("ReadTerrainTileMapRecord(fp, m_MapName,");
    const auto setPosition = ground.find("SetPos(bPosX, bPosY);", record);
    check(record != std::string::npos && setPosition != std::string::npos &&
        ground.substr(record, setPosition - record).find("if (!validRecord)") != std::string::npos &&
        ground.substr(record, setPosition - record).find("return 0;") != std::string::npos,
        "invalid terrain is rejected before ground position and masks are derived");
    const auto groundAttach = ground.find("int TMGround::Attach(TMGround* pGround)");
    const auto groundLoad = ground.find("int TMGround::LoadTileMap", groundAttach);
    const auto attachBody = groundAttach != std::string::npos && groundLoad != std::string::npos
        ? ground.substr(groundAttach, groundLoad - groundAttach) : std::string{};
    const auto rejectNonNeighbor = attachBody.find("if (!((deltaX == 1 || deltaX == -1) && deltaY == 0) &&");
    const auto clearLinks = attachBody.find("m_pLeftGround = 0;");
    check(rejectNonNeighbor != std::string::npos && clearLinks != std::string::npos &&
        rejectNonNeighbor < clearLinks &&
        attachBody.substr(rejectNonNeighbor, clearLinks - rejectNonNeighbor).find("return 0;") != std::string::npos &&
        attachBody.find("!((deltaY == 1 || deltaY == -1) && deltaX == 0)") != std::string::npos,
        "terrain attach rejects diagonal and distant cells without dropping existing links");
    const auto object = LoadSource("TMProject748/internal/game/entities/TMObject.cpp");
    const auto registerMask = object.find("int TMObject::RegisterMask(TMGround* pGround, float fX, float fY)");
    const auto objectSave = object.find("void TMObject::Save(FILE* fp)", registerMask);
    const auto maskBody = registerMask != std::string::npos && objectSave != std::string::npos
        ? object.substr(registerMask, objectSave - registerMask) : std::string{};
    check(maskBody.find("m_nMaskIndex < 0 || m_nMaskIndex >= MAX_OBJECT_MASK") != std::string::npos &&
        maskBody.find("maskX >= 0 && maskX < 128 && maskY >= 0 && maskY < 128") != std::string::npos &&
        maskBody.find("m_pMaskData[maskY][maskX]") != std::string::npos &&
        maskBody.find("m_pMaskData[y][128 * nBaseY") == std::string::npos,
        "object masks reject invalid indices and stay within the 128-by-128 terrain");
    const auto selectCharacter = LoadSource("TMProject748/internal/app/scenes/TMSelectCharScene.cpp");
    const auto reloadCharacters = selectCharacter.find("void TMSelectCharScene::ReloadCharList(");
    const auto previewFaceOverride = selectCharacter.find(
        "pSelChar->Equip[i][0].sIndex = g_nBattleMaster;", reloadCharacters);
    const auto previewIndexGuard = selectCharacter.find(
        "itemListIndices[slot] = itemId >= 0 ? itemId % MAX_ITEMLIST : 0;", reloadCharacters);
    const auto previewFaceLookup = selectCharacter.find(
        "g_pItemList[itemListIndices[0]]", reloadCharacters);
    const auto previewCapeLookup = selectCharacter.find(
        "m_wMantuaSkin = g_pItemList[itemListIndices[15]]", reloadCharacters);
    check(reloadCharacters != std::string::npos && previewFaceOverride != std::string::npos &&
        previewIndexGuard != std::string::npos &&
        previewFaceLookup != std::string::npos && previewCapeLookup != std::string::npos &&
        previewFaceOverride < previewIndexGuard && previewIndexGuard < previewFaceLookup &&
        previewFaceLookup < previewCapeLookup &&
        selectCharacter.find("g_pItemList[pSelChar->Equip[i]", reloadCharacters) == std::string::npos,
        "character preview bounds face, equipment, and cape ItemList lookups");
    const auto sampleLoad = selectCharacter.find(
        "if (!WYD748_LoadCharacterSamples(\"UI\\\\selchar.txt\", samples, 4))");
    const auto sampleItemLookup = selectCharacter.find("g_pItemList[nFace].nIndexMesh");
    check(sampleLoad != std::string::npos && sampleItemLookup != std::string::npos &&
        sampleLoad < sampleItemLookup &&
        selectCharacter.substr(sampleLoad, sampleItemLookup - sampleLoad).find("return 0;") != std::string::npos,
        "character samples are validated before the first ItemList lookup");
    const auto characterTerrain = selectCharacter.find("if (!m_pGroundList[0]->LoadTileMap(szMapPath))");
    const auto characterMiniMap = selectCharacter.find("m_pGround->SetMiniMapData();", characterTerrain);
    check(characterTerrain != std::string::npos && characterMiniMap != std::string::npos &&
        selectCharacter.substr(characterTerrain, characterMiniMap - characterTerrain).find("return 0;") != std::string::npos,
        "character selection stops before using an invalid terrain");
    const auto fieldSource = LoadSource("TMProject748/internal/app/scenes/TMFieldScene.cpp");
    const auto fieldTickStart = fieldSource.find("int TMFieldScene::FrameMove(unsigned int dwServerTime)");
    const auto compactTickStart = fieldSource.find("\tif (m_bCompatFieldScene)\n", fieldTickStart);
    const auto regularTickStart = fieldSource.find("\tif (g_bEffectFirst == 1)", compactTickStart);
    const auto compactRecall = fieldSource.find("recallDeadPlayer(dwServerTime);", compactTickStart);
    const auto regularRecall = fieldSource.find("recallDeadPlayer(dwServerTime);", regularTickStart);
    check(fieldTickStart != std::string::npos && compactTickStart != std::string::npos &&
        regularTickStart != std::string::npos && compactRecall != std::string::npos &&
        regularRecall != std::string::npos && compactRecall < regularTickStart &&
        regularRecall > regularTickStart &&
        fieldSource.find("MSG_STANDARD request{};", fieldTickStart) < compactTickStart &&
        fieldSource.find("m_dwLastDeadTime = 0;", fieldTickStart) < compactTickStart,
        "both field lifecycles send one initialized recall request after prolonged death");
    const auto restoredHpGuard = fieldSource.find(
        "m_pMessageBox && g_pObjectManager &&", fieldTickStart);
    const auto restoredHpCheck = fieldSource.find(
        "CurrentScore.CurHP > 0", restoredHpGuard);
    const auto closeRespawnPrompt = fieldSource.find(
        "m_pMessageBox->SetVisible(0);", restoredHpCheck);
    check(restoredHpGuard != std::string::npos && restoredHpCheck != std::string::npos &&
        closeRespawnPrompt != std::string::npos &&
        restoredHpGuard < restoredHpCheck && restoredHpCheck < closeRespawnPrompt &&
        closeRespawnPrompt < compactTickStart &&
        fieldSource.substr(restoredHpGuard, closeRespawnPrompt - restoredHpGuard).find(
            "m_pMessageBox->m_dwMessage == 11") != std::string::npos,
        "both field lifecycles close only the respawn prompt after authoritative HP recovery");
    const auto timeDelayStart = fieldSource.find("int TMFieldScene::TimeDelay(unsigned int dwServerTime)");
    const auto recallCountdownStart = fieldSource.find("if (m_dwLastTown)", timeDelayStart);
    const auto recallCountdownEnd = fieldSource.find("if (m_dwLastResurrect)", recallCountdownStart);
    const auto recallCountdown = recallCountdownStart != std::string::npos &&
        recallCountdownEnd != std::string::npos
        ? fieldSource.substr(recallCountdownStart, recallCountdownEnd - recallCountdownStart)
        : std::string{};
    check(!recallCountdown.empty() &&
        recallCountdown.find("ShouldAdvanceRespawnRecallCountdown(") != std::string::npos &&
        recallCountdown.find("RespawnRecallSecondsRemaining(") != std::string::npos &&
        recallCountdown.find("m_dwLastSelServer") == std::string::npos,
        "respawn recall countdown uses its own timer and stops after sending the request");
    check(!recallCountdown.empty() &&
        recallCountdown.find("m_pMyHuman->m_cDie = 0;") == std::string::npos &&
        recallCountdown.find("m_pMyHuman->SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP") == std::string::npos &&
        recallCountdown.find("SendOneMessage((char*)&stRecall, sizeof(stRecall));") != std::string::npos,
        "recall request retains death until the server confirms revival");
    const auto recallRequest = recallCountdown.find("SendOneMessage((char*)&stRecall, sizeof(stRecall));");
    const auto recallEffectGuard = recallCountdown.find(
        "if (!m_pMyHuman->m_cHide && m_pEffectContainer)", recallRequest);
    const auto recallEffect = recallCountdown.find("new TMEffectLevelUp(", recallEffectGuard);
    const auto portalEffectGuard = recallCountdown.find("if (m_pEffectContainer)", recallEffect);
    const auto portalEffect = recallCountdown.find("new TMSkillTownPortal(", portalEffectGuard);
    check(recallRequest != std::string::npos && recallEffectGuard != std::string::npos &&
        recallEffect != std::string::npos && portalEffectGuard != std::string::npos &&
        portalEffect != std::string::npos && recallRequest < recallEffectGuard &&
        recallEffectGuard < recallEffect && recallEffect < portalEffectGuard &&
        portalEffectGuard < portalEffect,
        "recall sends its request while optional effects require a scene owner");
    const auto teleportCountdownStart = fieldSource.find("if (m_dwLastTeleport)", recallCountdownEnd);
    const auto relocationCountdownStart = fieldSource.find("if (m_dwLastRelo)", teleportCountdownStart);
    const auto relocationCountdownEnd = fieldSource.find("if (m_dwLastWhisper)", relocationCountdownStart);
    const auto hasGuardedPortal = [&](std::size_t start, std::size_t end) {
        if (start == std::string::npos || end == std::string::npos || start >= end)
            return false;
        const auto body = fieldSource.substr(start, end - start);
        const auto ownerGuard = body.find("if (m_pEffectContainer)");
        const auto portalEffect = body.find("new TMSkillTownPortal(");
        return ownerGuard != std::string::npos && portalEffect != std::string::npos &&
            ownerGuard < portalEffect;
    };
    check(hasGuardedPortal(teleportCountdownStart, relocationCountdownStart) &&
        hasGuardedPortal(relocationCountdownStart, relocationCountdownEnd),
        "teleport and relocation countdowns skip portal effects without a scene owner");
    const auto deathStart = fieldSource.find("int TMFieldScene::OnPacketCNFMobKill(MSG_CNFMobKill* pStd)");
    const auto deathEnd = fieldSource.find("int TMFieldScene::OnPacketREQParty(", deathStart);
    const auto deathBody = deathStart != std::string::npos && deathEnd != std::string::npos
        ? fieldSource.substr(deathStart, deathEnd - deathStart) : std::string{};
    check(deathBody.find("pAttacker ? pAttacker->m_szName : \"Unknown\"") != std::string::npos &&
        deathBody.find("sysTime.wSecond, killerName") != std::string::npos,
        "death notification tolerates a killer absent from the local scene");
    check(deathBody.find("if (!pGridInv)\n\t\t\t\tcontinue;") != std::string::npos &&
        deathBody.find("pItem && pItem->m_pItem && pItem->m_pItem->sIndex == 3463") != std::string::npos &&
        deathBody.find("if (bFind && m_pHelpList[3])") != std::string::npos,
        "death notification tolerates missing inventory and help controls");
    const auto deathHumanSource = LoadSource("TMProject748/internal/game/entities/TMHuman.cpp");
    const auto deathClipEffectStart = deathHumanSource.find("if (m_nClass == 64 && m_sHeadIndex == 397)");
    const auto corpseTransition = deathHumanSource.find(
        "SetAnimation(ECHAR_MOTION::ECMOTION_DEAD, 1);", deathHumanSource.find("int TMHuman::FrameMove("));
    check(corpseTransition != std::string::npos &&
        deathHumanSource.find("m_eMotion = ECHAR_MOTION::ECMOTION_DEAD;", corpseTransition) < deathClipEffectStart &&
        deathHumanSource.find("m_nLoop = 1;", corpseTransition) < deathClipEffectStart,
        "death completion commits the corpse state even when its mesh clip is unavailable");
    const auto deathClipEffectEnd = deathHumanSource.find(
        "if (g_pCurrentScene->m_pMyHuman == this)", deathClipEffectStart);
    const auto deathClipEffectBody = deathClipEffectStart != std::string::npos &&
        deathClipEffectEnd != std::string::npos
        ? deathHumanSource.substr(deathClipEffectStart,
            deathClipEffectEnd - deathClipEffectStart) : std::string{};
    const auto deathClipHide = deathClipEffectBody.find("m_cHide = 1;");
    const auto deathClipEffectGuard = deathClipEffectBody.find(
        "if (g_pCurrentScene->m_pEffectContainer)");
    const auto deathClipEffect = deathClipEffectBody.find("new TMEffectParticle(");
    check(!deathClipEffectBody.empty() && deathClipHide != std::string::npos &&
        deathClipEffectGuard != std::string::npos && deathClipEffect != std::string::npos &&
        deathClipHide < deathClipEffectGuard && deathClipEffectGuard < deathClipEffect,
        "death animation completion never allocates an effect without its owner");
    const auto vitalsStart = deathHumanSource.find("int TMHuman::OnPacketSetHpMp(MSG_SetHpMp* pStd)");
    const auto vitalsEnd = deathHumanSource.find("int TMHuman::OnPacketSetHpDam(", vitalsStart);
    const auto vitalsBody = vitalsStart != std::string::npos && vitalsEnd != std::string::npos
        ? deathHumanSource.substr(vitalsStart, vitalsEnd - vitalsStart) : std::string{};
    const auto hpClamp = vitalsBody.find("m_stScore.CurHP = m_stScore.MaxHP;");
    const auto localScoreCopy = vitalsBody.find("if (isLocalHuman)\n    {\n        auto& localScore =");
    const auto localHpCopy = vitalsBody.find("localScore.CurHP = m_stScore.CurHP;", localScoreCopy);
    const auto localMpCopy = vitalsBody.find("localScore.CurMP = m_stScore.CurMP;", localHpCopy);
    const auto localMaxHpCopy = vitalsBody.find("localScore.MaxHP = m_stScore.MaxHP;", localMpCopy);
    const auto localMaxMpCopy = vitalsBody.find("localScore.MaxMP = m_stScore.MaxMP;", localMaxHpCopy);
    const auto lethalTransition = vitalsBody.find(
        "if (death_motion::ShouldEnterDeath(m_stScore.CurHP, m_cDie == 1))\n        Die();");
    const auto airMoveVisualGuard = vitalsBody.find("if (isLocalHuman && !pFScene->m_bAirMove)");
    const auto localHpProjection = vitalsBody.find(
        "resource_ui::ProjectNativeHpVisual(pFScene->m_pHPBar", airMoveVisualGuard);
    check(!vitalsBody.empty() && hpClamp != std::string::npos &&
        localScoreCopy != std::string::npos && localHpCopy != std::string::npos &&
        localMpCopy != std::string::npos && localMaxHpCopy != std::string::npos &&
        localMaxMpCopy != std::string::npos && airMoveVisualGuard != std::string::npos &&
        hpClamp < localScoreCopy && localScoreCopy < localHpCopy &&
        localHpCopy < localMpCopy && localMpCopy < localMaxHpCopy &&
        localMaxHpCopy < localMaxMpCopy && localMaxMpCopy < airMoveVisualGuard &&
        vitalsBody.find("memcpy(&g_pObjectManager->m_stMobData.CurrentScore") == std::string::npos,
        "air travel keeps all local vitals current without replacing unrelated score fields");
    check(lethalTransition != std::string::npos && localHpProjection != std::string::npos &&
        localMaxMpCopy < lethalTransition && lethalTransition < airMoveVisualGuard &&
        airMoveVisualGuard < localHpProjection,
        "lethal vitals cancel flight before the local HP visual gate and redraw");
    const auto airMoveStart = fieldSource.find("void TMFieldScene::AirMove_Start(int nIndex)");
    const auto airMoveEndStart = fieldSource.find("void TMFieldScene::AirMove_End(AirMoveEndReason reason)");
    const auto airMoveStartBody = airMoveStart != std::string::npos && airMoveEndStart != std::string::npos
        ? fieldSource.substr(airMoveStart, airMoveEndStart - airMoveStart) : std::string{};
    const auto startOwnerGuard = airMoveStartBody.find("!m_pEffectContainer");
    const auto startEffect = airMoveStartBody.find("new TMEffectParticle", startOwnerGuard);
    const auto startPacket = airMoveStartBody.find("SendPacket(", startEffect);
    check(!airMoveStartBody.empty() && startOwnerGuard != std::string::npos &&
        startEffect != std::string::npos && startPacket != std::string::npos &&
        startOwnerGuard < startEffect && startEffect < startPacket,
        "air travel cannot start without the scene-owned effect container");
    const auto airMoveEndEnd = fieldSource.find("int TMFieldScene::AirMove_ShowUI(", airMoveEndStart);
    const auto airMoveEndBody = airMoveEndStart != std::string::npos && airMoveEndEnd != std::string::npos
        ? fieldSource.substr(airMoveEndStart, airMoveEndEnd - airMoveEndStart) : std::string{};
    check(!airMoveEndBody.empty() &&
        airMoveEndBody.find("if (m_pMyHuman->m_cDie != 1 && m_pMyHuman->m_stScore.CurHP > 0)\n"
            "\t\t\tm_pMyHuman->SetAnimation(m_eOldMotion, 1);") != std::string::npos,
        "air travel completion cannot restore the pre-death motion");
    const auto deadFlightReturn = airMoveEndBody.find("if (interruptedByDeath)",
        airMoveEndBody.find("m_dwAirMove_TickTime = 0;"));
    const auto teleportReturn = airMoveEndBody.find("if (externalTeleport)\n\t\t\treturn;",
        deadFlightReturn);
    const auto flightEndPacket = airMoveEndBody.find("SendPacket({reinterpret_cast<MSG_STANDARD*>",
        teleportReturn);
    const auto endOwnerGuard = airMoveEndBody.find("if (m_pEffectContainer)", teleportReturn);
    const auto endEffect = airMoveEndBody.find("new TMEffectParticle", endOwnerGuard);
    check(!airMoveEndBody.empty() && endOwnerGuard != std::string::npos &&
        endEffect != std::string::npos && flightEndPacket != std::string::npos &&
        endOwnerGuard < endEffect && endEffect < flightEndPacket,
        "flight completion skips an unavailable effect owner but still sends its packet");
    check(!airMoveEndBody.empty() &&
        airMoveEndBody.find("reason == AirMoveEndReason::Death") != std::string::npos &&
        airMoveEndBody.find("CancelAirMoveAtOrigin(") != std::string::npos &&
        airMoveEndBody.find("DiscardAirMoveDelta(") != std::string::npos &&
        airMoveEndBody.find("m_nAirMove_State = interruptedByDeath || externalTeleport ? 0 : -1;") != std::string::npos &&
        deadFlightReturn != std::string::npos && teleportReturn != std::string::npos &&
        flightEndPacket != std::string::npos && deadFlightReturn < teleportReturn &&
        teleportReturn < flightEndPacket,
        "death and authoritative teleport cancel flight before any completion packet");
    const auto dieStart = deathHumanSource.find("void TMHuman::Die()");
    const auto dieEnd = deathHumanSource.find("void TMHuman::Stand()", dieStart);
    const auto dieBody = dieStart != std::string::npos && dieEnd != std::string::npos
        ? deathHumanSource.substr(dieStart, dieEnd - dieStart) : std::string{};
    check(!dieBody.empty() &&
        dieBody.find("routePoint = m_vecPosition;") != std::string::npos &&
        dieBody.find("m_nMaxRouteIndex = 0;") != std::string::npos &&
        dieBody.find("m_bMoveing = 0;") != std::string::npos,
        "death freezes unfinished movement at the current position");
    check(!dieBody.empty() &&
        dieBody.find("AirMove_End(TMFieldScene::AirMoveEndReason::Death);") != std::string::npos &&
        dieBody.find("AirMove_End(TMFieldScene::AirMoveEndReason::Death);") <
            dieBody.find("routePoint = m_vecPosition;"),
        "death cancels visual flight before freezing the route even with positive HP");
    check(deathHumanSource.find("AirMove_End(TMFieldScene::AirMoveEndReason::ExternalTeleport);") !=
        std::string::npos,
        "authoritative action cancels visual flight without sending a second destination");
    check(!dieBody.empty() &&
        dieBody.find("m_SendeMotion = ECHAR_MOTION::ECMOTION_NONE;") != std::string::npos,
        "death clears pending emotes before ignoring late responses");
    const auto deathEffectGuard = dieBody.find(
        "if (auto* effectContainer = g_pCurrentScene->m_pEffectContainer)");
    const auto deathEffect = dieBody.find("new TMEffectParticle(", deathEffectGuard);
    const auto deathEffectEnd = dieBody.find("effectContainer->AddChild(pBill);\n        }", deathEffect);
    const auto deathSound = dieBody.find("GetSoundAndPlay(309, 0, 0);", deathEffectEnd);
    check(deathEffectGuard != std::string::npos && deathEffect != std::string::npos &&
        deathEffectEnd != std::string::npos && deathSound != std::string::npos &&
        deathEffectGuard < deathEffect && deathEffect < deathEffectEnd &&
        deathEffectEnd < deathSound,
        "death skips orphaned cosmetic effects without suppressing sound or state");
    check(!dieBody.empty() &&
        dieBody.find("SetAnimation(ECHAR_MOTION::ECMOTION_DIE, 0);") != std::string::npos &&
        dieBody.find("m_eMotion = ECHAR_MOTION::ECMOTION_DIE;") >
            dieBody.find("SetAnimation(ECHAR_MOTION::ECMOTION_DIE, 0);") &&
        dieBody.find("m_eMotion = ECHAR_MOTION::ECMOTION_DIE;") <
            dieBody.find("m_nLoop = 0;", dieBody.find("SetAnimation(ECHAR_MOTION::ECMOTION_DIE, 0);")) &&
        dieBody.find("m_nLoop = 0;") != std::string::npos &&
        dieBody.find("m_dwStartAnimationTime = g_pTimerManager->GetServerTime();") != std::string::npos,
        "death commits its logical state and one-shot completion when the mesh rejects its animation");
    const auto motionStart = deathHumanSource.find("int TMHuman::OnPacketFireWork(MSG_Motion* pStd)");
    const auto motionEnd = deathHumanSource.find("int TMHuman::OnPacketPremiumFireWork(", motionStart);
    const auto motionBody = motionStart != std::string::npos && motionEnd != std::string::npos
        ? deathHumanSource.substr(motionStart, motionEnd - motionStart) : std::string{};
    check(!motionBody.empty() &&
        motionBody.find("(m_cDie == 1 || m_stScore.CurHP <= 0) && pStd->Parm != 2") != std::string::npos,
        "late motion packets cannot replace death before an explicit revival");
    check(!motionBody.empty() &&
        motionBody.find("if (pStd->Parm == 3 && g_pCurrentScene->m_pEffectContainer)") != std::string::npos &&
        motionBody.find("m_pEffectContainer->AddChild(new TMEffectFireWork(") != std::string::npos &&
        motionBody.find("m_pEffectContainer->AddChild(pFireWork)") == std::string::npos,
        "motion effects are allocated only when their scene owner exists");
    const auto shopListStart = fieldSource.find("int TMFieldScene::OnPacketShopList(MSG_STANDARD* pStd)");
    const auto shopListEnd = fieldSource.find("int TMFieldScene::OnPacket", shopListStart + 1);
    const auto shopListHandler = shopListStart != std::string::npos &&
        shopListEnd != std::string::npos
        ? fieldSource.substr(shopListStart, shopListEnd - shopListStart) : std::string{};
    const auto merchantGridGuard = shopListHandler.find("if (!m_pGridShop)");
    const auto couponStateChange = shopListHandler.find("m_bEventCouponClick = 0;");
    const auto merchantGridUse = shopListHandler.find("m_pGridShop->Empty();");
    check(!shopListHandler.empty() && merchantGridGuard != std::string::npos &&
        couponStateChange != std::string::npos && merchantGridUse != std::string::npos &&
        merchantGridGuard < couponStateChange && couponStateChange < merchantGridUse &&
        shopListHandler.substr(merchantGridGuard, couponStateChange - merchantGridGuard)
            .find("return 0;") != std::string::npos,
        "merchant ShopList rejects an unbound grid before changing coupon or shop state");
    const auto skillBranch = shopListHandler.find("else if (pShopList->ShopType == 3)");
    const auto skillFirstItemGuard = shopListHandler.find(
        "if (pShopList->List[0].sIndex < 5000)", skillBranch);
    const auto skillGridUse = shopListHandler.find("m_pGridSkillMaster->Empty();", skillBranch);
    check(skillBranch != std::string::npos && skillFirstItemGuard != std::string::npos &&
        skillGridUse != std::string::npos && skillBranch < skillFirstItemGuard &&
        skillFirstItemGuard < skillGridUse &&
        shopListHandler.substr(skillFirstItemGuard, skillGridUse - skillFirstItemGuard)
            .find("return 0;") != std::string::npos,
        "skill ShopList rejects a malformed first item before replacing the visible grid");
    const auto rmbShopStart = fieldSource.find("int TMFieldScene::OnPacketRMBShopList(");
    const auto rmbShopEnd = fieldSource.find("int TMFieldScene::OnPacketBuy(", rmbShopStart);
    const auto rmbShopHandler = rmbShopStart != std::string::npos &&
        rmbShopEnd != std::string::npos
        ? fieldSource.substr(rmbShopStart, rmbShopEnd - rmbShopStart) : std::string{};
    const auto rmbMerchantGuard = rmbShopHandler.find("if (!m_pGridShop)");
    const auto rmbCouponStateChange = rmbShopHandler.find("m_bEventCouponClick = 0;");
    const auto rmbMerchantGridUse = rmbShopHandler.find("pGrid->Empty();");
    check(!rmbShopHandler.empty() && rmbMerchantGuard != std::string::npos &&
        rmbCouponStateChange != std::string::npos && rmbMerchantGridUse != std::string::npos &&
        rmbMerchantGuard < rmbCouponStateChange &&
        rmbCouponStateChange < rmbMerchantGridUse &&
        rmbShopHandler.substr(rmbMerchantGuard, rmbCouponStateChange - rmbMerchantGuard)
            .find("return 0;") != std::string::npos,
        "RMB merchant ShopList rejects an unbound grid before changing coupon or shop state");
    const auto carryGrid = fieldSource.find("SGridControl* TMFieldScene::GetCarryGridForSlot(int slot) const");
    const auto cargoGrid = fieldSource.find("SGridControl* TMFieldScene::GetCargoGridForSlot(int slot) const", carryGrid);
    const auto carryGridBody = carryGrid != std::string::npos && cargoGrid != std::string::npos
        ? fieldSource.substr(carryGrid, cargoGrid - carryGrid) : std::string{};
    check(carryGridBody.find("slot < 0 || slot >= MAX_VISIBLE_CARRY") != std::string::npos,
        "reserved Carry slot remains in the wire array but cannot address a nonexistent grid row");
    const auto humanSource = LoadSource("TMProject748/internal/game/entities/TMHuman.cpp");
    const auto sendItemStart = humanSource.find("int TMHuman::OnPacketSendItem(MSG_STANDARD* pStd)");
    const auto sendItemEnd = humanSource.find("int TMHuman::OnPacketUpdateEquip", sendItemStart);
    const auto sendItemHandler = sendItemStart != std::string::npos && sendItemEnd != std::string::npos
        ? humanSource.substr(sendItemStart, sendItemEnd - sendItemStart) : std::string{};
    const auto localOnly = sendItemHandler.find("if (g_pCurrentScene->m_pMyHuman != this)");
    const auto itemIndexGuard = sendItemHandler.find(
        "if (pSendItem->Item.sIndex < 0 || pSendItem->Item.sIndex >= MAX_ITEMLIST)");
    const auto bagView = sendItemHandler.find("pFScene->Bag_View();");
    const auto firstModelWrite = sendItemHandler.find("memcpy(&pMobData->Equip[");
    const auto applyAppearance = sendItemHandler.find("SetPacketMOBItem(pMobData);");
    for (const int type : {-32768, -1, 0, 1, 2, 3, 32767})
        for (const int position : {-32768, -1, 0, 17, 18, 63, 64, 127, 128, 32767})
        {
            const bool expected = position >= 0 &&
                ((type == 0 && position < 18) || (type == 1 && position < 64) ||
                 (type == 2 && position < 128));
            check(IsSendItemDestination(type, position, 18, 64, 128) == expected,
                "SendItem accepts only real storage types and preserves their complete array capacities");
        }
    for (const int type : {0, 1, 2})
        check(!IsSendItemDestination(type, 0, 0, 0, 0),
            "SendItem rejects positions when the destination has no storage");
    const auto destinationGuard = sendItemHandler.find(
        "if (!IsSendItemDestination(pSendItem->DestType, pSendItem->DestPos,");
    check(destinationGuard != std::string::npos && bagView != std::string::npos &&
        firstModelWrite != std::string::npos && applyAppearance != std::string::npos &&
        destinationGuard < bagView && destinationGuard < firstModelWrite &&
        destinationGuard < applyAppearance &&
        sendItemHandler.substr(destinationGuard, bagView - destinationGuard).find("return 1;") != std::string::npos,
        "SendItem rejects an unsupported destination before UI, model, or mesh updates");
    check(!sendItemHandler.empty() && localOnly != std::string::npos &&
        bagView != std::string::npos && firstModelWrite != std::string::npos &&
        applyAppearance != std::string::npos && localOnly < bagView &&
        localOnly < firstModelWrite && localOnly < applyAppearance &&
        sendItemHandler.substr(localOnly, bagView - localOnly).find("return 1;") != std::string::npos,
        "SendItem for another human cannot apply the local inventory or appearance");
    check(itemIndexGuard != std::string::npos && bagView != std::string::npos &&
        firstModelWrite != std::string::npos && itemIndexGuard < bagView &&
        itemIndexGuard < firstModelWrite &&
        sendItemHandler.substr(itemIndexGuard, bagView - itemIndexGuard).find("return 1;") != std::string::npos,
        "SendItem rejects an item outside the 7.48 catalog before changing local state");
    const auto cargoUpdateStart = sendItemHandler.find("else if (pSendItem->DestType == 2)");
    const auto cargoUpdate = cargoUpdateStart != std::string::npos &&
        applyAppearance != std::string::npos && cargoUpdateStart < applyAppearance
        ? sendItemHandler.substr(cargoUpdateStart, applyAppearance - cargoUpdateStart) : std::string{};
    const auto cargoModelWrite = cargoUpdate.find("memcpy(&g_pObjectManager->m_stItemCargo[");
    const auto cargoGridLookup = cargoUpdate.find("pFScene->GetCargoGridForSlot(");
    const auto cargoOptionalGrid = cargoUpdate.find("if (pGrid)");
    const auto cargoPickup = cargoUpdate.find("pGrid->PickupAtItem(");
    const auto cargoAllocation = cargoUpdate.find("new STRUCT_ITEM()");
    const auto cargoRelease = cargoUpdate.find("releaseReplacedItem(pOldGridItem);");
    check(!cargoUpdate.empty() && cargoModelWrite != std::string::npos &&
        cargoGridLookup != std::string::npos && cargoModelWrite < cargoGridLookup,
        "SendItem commits Cargo state before consulting the optional visual grid");
    check(cargoOptionalGrid != std::string::npos && cargoPickup != std::string::npos &&
        cargoAllocation != std::string::npos && cargoGridLookup < cargoOptionalGrid &&
        cargoOptionalGrid < cargoPickup && cargoPickup < cargoAllocation &&
        cargoUpdate.find("if (!pGrid)") == std::string::npos &&
        cargoUpdate.find("return ") == std::string::npos &&
        cargoRelease != std::string::npos && cargoPickup < cargoRelease &&
        cargoRelease < cargoAllocation,
        "missing Cargo grid skips only visual replacement and still reaches common SendItem finalization");
    check(applyAppearance != std::string::npos &&
        sendItemHandler.find("InitObject();", applyAppearance) != std::string::npos &&
        sendItemHandler.find("pFScene->UpdateScoreUI(0);", applyAppearance) != std::string::npos &&
        sendItemHandler.find("SGridControl::m_sLastMouseOverIndex = -1;", applyAppearance) != std::string::npos,
        "SendItem keeps appearance, score UI, and hover invalidation after all destination branches");
    const auto abilitySource = LoadSource("TMProject748/internal/core/Basedef.cpp");
    const auto itemAbilityStart = abilitySource.find("int BASE_GetItemAbility(STRUCT_ITEM* item, char Type)");
    const auto staticAbilityStart = abilitySource.find("int BASE_GetStaticItemAbility(STRUCT_ITEM* item, char Type)");
    const auto validCatalogIndex = [](const std::string& source, std::size_t start) {
        if (start == std::string::npos)
            return false;
        const auto body = source.substr(start, 512);
        const auto guard = body.find("if (idx <= 0 || idx >= MAX_ITEMLIST)");
        const auto lookup = body.find("g_pItemList[idx]");
        return guard != std::string::npos && lookup != std::string::npos && guard < lookup;
    };
    check(validCatalogIndex(abilitySource, itemAbilityStart) &&
        validCatalogIndex(abilitySource, staticAbilityStart),
        "item ability readers reject the first index beyond the 7.48 ItemList before lookup");
    const auto itemAbilityBody = itemAbilityStart != std::string::npos
        ? abilitySource.substr(itemAbilityStart, 1024) : std::string{};
    const auto volatileGuard = itemAbilityBody.find("if (Type == EF_VOLATILE)");
    const auto volatileReturn = itemAbilityBody.find(
        "return native_item_volatile::GetAbility(idx, g_pItemList[idx].stEffect, item->stEffect);");
    const auto legacyAbilityStart = itemAbilityBody.find("int nUnique = g_pItemList[idx].nUnique;");
    check(volatileGuard != std::string::npos && volatileReturn != std::string::npos &&
        legacyAbilityStart != std::string::npos && volatileGuard < volatileReturn &&
        volatileReturn < legacyAbilityStart &&
        itemAbilityBody.substr(volatileGuard, volatileReturn - volatileGuard) ==
            "if (Type == EF_VOLATILE)\n        ",
        "only type 38 uses the shared native lookup before legacy ability branches");
    const auto bonusNoSancStart = abilitySource.find("int BASE_GetBonusItemAbilityNosanc(");
    const auto bonusStart = abilitySource.find("int BASE_GetBonusItemAbility(", bonusNoSancStart);
    const auto bonusEnd = abilitySource.find("int BASE_GetItemAbilityNosanc(", bonusStart);
    const auto bonusGuard = "if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)";
    check(bonusNoSancStart != std::string::npos && bonusStart != std::string::npos &&
        bonusEnd != std::string::npos &&
        abilitySource.substr(bonusNoSancStart, bonusStart - bonusNoSancStart).find(bonusGuard) != std::string::npos &&
        abilitySource.substr(bonusStart, bonusEnd - bonusStart).find(bonusGuard) != std::string::npos &&
        abilitySource.substr(bonusStart, bonusEnd - bonusStart).find("g_pItemList[item->sIndex]") != std::string::npos,
        "bonus ability readers reject index 6500 before the catalog lookup");
    const auto passiveStart = abilitySource.find("int IsPassiveSkill(int nSkillIndex)");
    const auto passiveBody = passiveStart != std::string::npos
        ? abilitySource.substr(passiveStart, 400) : std::string{};
    const auto passiveGuard = passiveBody.find("nSkillIndex < 0 || nSkillIndex >= MAX_SPELL_LIST");
    const auto passiveLookup = passiveBody.find("g_pSpell[nSkillIndex]");
    check(passiveGuard != std::string::npos && passiveLookup != std::string::npos &&
        passiveGuard < passiveLookup &&
        passiveBody.substr(passiveGuard, passiveLookup - passiveGuard).find("return 0;") != std::string::npos,
        "passive-skill lookup rejects normalized indexes outside the 7.48 spell table");
    const auto shortcutStart = fieldSource.find("void TMFieldScene::SetShortSkill(");
    const auto shortcutGuard = fieldSource.find("pGridItem->m_pItem->sIndex >= MAX_ITEMLIST", shortcutStart);
    const auto shortcutPassive = fieldSource.find("IsPassiveSkill(pGridItem->m_pItem->sIndex)", shortcutStart);
    const auto shortcutCatalog = fieldSource.find("g_pItemList[pGridItem->m_pItem->sIndex]", shortcutStart);
    check(shortcutStart != std::string::npos && shortcutGuard != std::string::npos &&
        shortcutPassive != std::string::npos && shortcutCatalog != std::string::npos &&
        shortcutGuard < shortcutPassive && shortcutPassive < shortcutCatalog,
        "shortcuts reject invalid 7.48 item IDs before passive-skill and catalog lookups");
    check(fieldSource.find("g_pObjectManager->m_stMobData.Equip[4].sIndex < MAX_ITEMLIST") != std::string::npos &&
        fieldSource.find("capeIndex > 0 && capeIndex < MAX_ITEMLIST") != std::string::npos,
        "equipment-derived skill delay and cape text bound ItemList indexes");
    const auto storeStart = fieldSource.find("void TMFieldScene::UpdateNewStore(int idwControlID)");
    const auto storeEnd = fieldSource.find("int TMFieldScene::OnPacketNewCashRev(", storeStart);
    const auto storeBody = storeStart != std::string::npos && storeEnd != std::string::npos
        ? fieldSource.substr(storeStart, storeEnd - storeStart) : std::string{};
    const auto purchaseLoop = storeBody.find("for (const int buttonControlId : Buttons)");
    const auto purchaseGridGuard = storeBody.find("if (!GridSlot)", purchaseLoop);
    const auto purchaseItemGuard = storeBody.find("Item->m_pItem->sIndex < MAX_ITEMLIST", purchaseLoop);
    const auto purchaseName = storeBody.find("g_pItemList[Item->m_pItem->sIndex].Name", purchaseLoop);
    check(purchaseLoop != std::string::npos && purchaseGridGuard != std::string::npos &&
        purchaseItemGuard != std::string::npos && purchaseName != std::string::npos &&
        purchaseGridGuard < purchaseItemGuard && purchaseItemGuard < purchaseName &&
        storeBody.find("sizeof(Buttons)") == std::string::npos,
        "donation-store purchase walks only its buttons and checks item bounds before naming it");
    const auto weaponDamageStart = fieldSource.find("int TMFieldScene::GetWeaponDamage()");
    const auto weaponDamageEnd = fieldSource.find("void TMFieldScene::SetMyHumanMagic()", weaponDamageStart);
    const auto weaponDamageBody = weaponDamageStart != std::string::npos &&
        weaponDamageEnd != std::string::npos
        ? fieldSource.substr(weaponDamageStart, weaponDamageEnd - weaponDamageStart) : std::string{};
    check(!weaponDamageBody.empty() &&
        weaponDamageBody.find("idx1 >= 0 && idx1 < MAX_ITEMLIST ? g_pItemList[idx1].nUnique : 0") != std::string::npos &&
        weaponDamageBody.find("idx2 >= 0 && idx2 < MAX_ITEMLIST ? g_pItemList[idx2].nUnique : 0") != std::string::npos &&
        weaponDamageBody.find("idx1 >= 0 || idx1 < MAX_ITEMLIST") == std::string::npos &&
        weaponDamageBody.find("idx2 >= 0 || idx2 < MAX_ITEMLIST") == std::string::npos,
        "weapon damage bounds both 7.48 ItemList uniqueness lookups");
    const auto updateEquipEnd = humanSource.find("int TMHuman::OnPacketUpdateAffect", sendItemEnd);
    const auto updateEquipHandler = sendItemEnd != std::string::npos &&
        updateEquipEnd != std::string::npos
        ? humanSource.substr(sendItemEnd, updateEquipEnd - sendItemEnd) : std::string{};
    const auto mountHudGuard = updateEquipHandler.find(
        "if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)");
    const auto mountHudCast = updateEquipHandler.find(
        "auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);");
    const auto mountHudWrite = updateEquipHandler.find("pFScene->m_pMHPBar->SetCurrentProgress(nMountHP);");
    check(!updateEquipHandler.empty() && mountHudGuard != std::string::npos &&
        mountHudCast != std::string::npos && mountHudWrite != std::string::npos &&
        mountHudGuard < mountHudCast && mountHudCast < mountHudWrite,
        "UpdateEquip accesses mount HUD only after verifying the field scene type");
    const auto quitTradeStart = humanSource.find("int TMHuman::OnPacketQuitTrade(");
    const auto quitTradeEnd = humanSource.find("int TMHuman::OnPacketCarry(", quitTradeStart);
    const auto quitTradeBody = quitTradeStart != std::string::npos && quitTradeEnd != std::string::npos
        ? humanSource.substr(quitTradeStart, quitTradeEnd - quitTradeStart) : std::string{};
    const auto quitTradeLocalGuard = quitTradeBody.find("if (g_pCurrentScene->m_pMyHuman == this)");
    const auto quitTradeOpponentClear = quitTradeBody.find("m_stTrade.OpponentID = 0;");
    const auto quitTradeCheckClear = quitTradeBody.find("m_stTrade.MyCheck = 0;");
    const auto quitTradeHoverClear = quitTradeBody.find("SGridControl::m_sLastMouseOverIndex = -1;");
    const auto quitTradeContainer = quitTradeBody.find("m_pControlContainer");
    const auto quitTradeFieldGuard = quitTradeBody.find("GetSceneType() == ESCENE_TYPE::ESCENE_FIELD");
    const auto quitTradeLookup = quitTradeBody.find("->FindControl(576)");
    check(quitTradeLocalGuard != std::string::npos &&
        quitTradeOpponentClear != std::string::npos && quitTradeCheckClear != std::string::npos &&
        quitTradeHoverClear != std::string::npos &&
        quitTradeLocalGuard < quitTradeOpponentClear && quitTradeLocalGuard < quitTradeCheckClear &&
        quitTradeLocalGuard < quitTradeHoverClear,
        "trade closure clears state only for the local human");
    check(quitTradeContainer != std::string::npos &&
        quitTradeOpponentClear < quitTradeContainer && quitTradeCheckClear < quitTradeContainer &&
        quitTradeHoverClear < quitTradeContainer,
        "trade closure clears opponent, check, and hover state even without a UI container");
    check(quitTradeFieldGuard != std::string::npos && quitTradeLookup != std::string::npos &&
        quitTradeHoverClear < quitTradeFieldGuard && quitTradeFieldGuard < quitTradeLookup &&
        quitTradeContainer < quitTradeLookup,
        "trade closure checks field-scene UI availability only after model cleanup");
    check(quitTradeBody.find("if (!g_pCurrentScene || !g_pObjectManager)") != std::string::npos &&
        quitTradeBody.find("if (pTradePanel && pTradePanel->IsVisible() == 1)") != std::string::npos &&
        quitTradeBody.find("if (pATradePanel && pATradePanel->IsVisible() == 1)") != std::string::npos,
        "trade closure preserves null scene, model, and optional-panel guards");
    const auto carryEnd = humanSource.find("int TMHuman::OnPacketCNFCheck(", quitTradeEnd);
    const auto carryBody = quitTradeEnd != std::string::npos && carryEnd != std::string::npos
        ? humanSource.substr(quitTradeEnd, carryEnd - quitTradeEnd) : std::string{};
    const auto carryCopy = carryBody.find(
        "memcpy(g_pObjectManager->m_stMobData.Carry, pStd->Carry, sizeof(pStd->Carry));");
    const auto carryCoin = carryBody.find("m_stMobData.Coin = pStd->Coin;");
    const auto carryOpponentClear = carryBody.find("m_stTrade.OpponentID = 0;");
    const auto carryCheckClear = carryBody.find("m_stTrade.MyCheck = 0;");
    const auto carryGridGuard = carryBody.find("if (!pScene->m_pGridInv)");
    const auto carryClose = carryBody.find("pScene->SetVisibleTrade(0);");
    const auto carryToggle = carryBody.find("pScene->SetVisibleInventory();");
    check(carryBody.find("if (!pStd || !g_pObjectManager || pScene->m_pMyHuman != this)") < carryCopy &&
        carryBody.find("GetSceneType() != ESCENE_TYPE::ESCENE_FIELD") < carryCopy &&
        carryCopy != std::string::npos,
        "Carry snapshot validates scene, payload, model, and local receiver before copying state");
    check(carryCopy != std::string::npos && carryCoin != std::string::npos &&
        carryGridGuard != std::string::npos && carryCopy < carryGridGuard && carryCoin < carryGridGuard,
        "Carry snapshot preserves all 64 items and Coin without an inventory grid");
    check(carryOpponentClear != std::string::npos && carryCheckClear != std::string::npos &&
        carryClose != std::string::npos && carryOpponentClear < carryClose && carryCheckClear < carryClose &&
        carryClose < carryGridGuard,
        "Carry snapshot clears native trade flags before optional UI closure and the missing-grid return");
    check(carryBody.find("if (pScene->m_pControlContainer)") < carryClose &&
        carryBody.find("if (pScene->m_pTradePanel && pScene->m_pTradePanel->IsVisible() == 1)") == std::string::npos &&
        carryClose != std::string::npos && carryToggle != std::string::npos &&
        carryClose < carryToggle && carryToggle < carryBody.find("pScene->UpdateScoreUI(0);") &&
        carryToggle < carryGridGuard &&
        carryBody.find("SendPacket") == std::string::npos,
        "Carry snapshot closes even hidden trade state before the native inventory toggle and score projection");
    check(carryGridGuard != std::string::npos &&
        carryGridGuard < carryBody.find("pScene->m_pGridInv->Empty();") &&
        carryBody.find("pScene->m_pGridInv->Empty();") < carryBody.find("new STRUCT_ITEM") &&
        carryBody.find("nCarryIndex < 63") != std::string::npos &&
        carryBody.find("nCarryIndex % 9, nCarryIndex / 9") != std::string::npos &&
        carryBody.find("SAFE_DELETE(pGridItem);") != std::string::npos,
        "Carry snapshot retains alias cleanup, native 63-cell projection, and rejected-visual ownership");
    const auto inventoryStart = fieldSource.find("void TMFieldScene::SetVisibleInventory()");
    const auto inventoryEnd = fieldSource.find("auto pCargoPanel = m_pCargoPanel;", inventoryStart);
    const auto inventoryCompat = inventoryStart != std::string::npos && inventoryEnd != std::string::npos
        ? fieldSource.substr(inventoryStart, inventoryEnd - inventoryStart) : std::string{};
    const auto inventoryTarget = inventoryCompat.find("const int visible = m_pInvenPanel->IsVisible() == 0;");
    const auto inventoryAutoTrade = inventoryCompat.find("SetVisibleAutoTrade(0, 0);");
    const auto inventoryGamble = inventoryCompat.find("SetVisibleGamble(0, 0);");
    const auto inventoryClosing = inventoryCompat.find("if (!visible)");
    check(inventoryTarget != std::string::npos && inventoryAutoTrade != std::string::npos &&
        inventoryGamble != std::string::npos && inventoryTarget < inventoryAutoTrade &&
        inventoryAutoTrade < inventoryGamble && inventoryGamble < inventoryClosing &&
        inventoryCompat.find("m_pInvenPanel->SetVisible(!visible);") < inventoryClosing,
        "native UI2 inventory captures the toggle target before closing AutoTrade and Gamble");
    check(inventoryCompat.find("if (m_pGambleStore && m_pGambleStore->IsVisible() == 1)") < inventoryTarget &&
        inventoryCompat.find("m_pInvenPanel->m_bVisible = 0;") < inventoryTarget &&
        inventoryCompat.find("if (m_pCPanel)") < inventoryTarget &&
        inventoryCompat.find("m_pCPanel->m_bVisible = 0;") < inventoryTarget,
        "native Gamble visibility forces Inventory and Character hidden before deciding the inventory target");
    check(inventoryClosing != std::string::npos &&
        inventoryCompat.find("mixIndex <= 6") != std::string::npos &&
        inventoryClosing < inventoryCompat.find("ClearNativeMix(mixIndex);") &&
        inventoryCompat.find("if (auto panel = GetNativeMixPanel(mixIndex))") != std::string::npos &&
        inventoryCompat.find("SetVisibleNativeMix(") == std::string::npos,
        "closing native inventory clears all six artisan packets and hides bound roots without recursive toggles");
    check(inventoryCompat.find("m_pCargoPanel, m_pShopPanel, m_pHellgateStore, m_pInputGoldPanel") != std::string::npos &&
        inventoryCompat.find("if (panel)") != std::string::npos &&
        inventoryCompat.find("SetGridState();") < inventoryCompat.find("SetVisibleTrade(0);") &&
        inventoryCompat.find("if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)") != std::string::npos &&
        inventoryCompat.find("if (g_pCursor)") < inventoryCompat.find("g_pCursor->DetachItem();"),
        "inventory closure hides only bound native peers and restores grids, trade, and cursor state");
    check(inventoryCompat.find("FindControl(TMB_SKILL)") != std::string::npos &&
        inventoryCompat.find("skillButton->SetSelected(m_pSkillPanel && m_pSkillPanel->IsVisible());") != std::string::npos &&
        inventoryCompat.find("m_pInvenPanel->SetVisible(visible);") > inventoryClosing &&
        inventoryCompat.find("m_ItemMixClass") == std::string::npos &&
        inventoryCompat.find("m_MissionClass") == std::string::npos,
        "native inventory finalizes its target and skill button without imported mix or mission topology");
    const auto listingSoldStart = fieldSource.find("int TMFieldScene::OnPacketItemSold(MSG_STANDARDPARM2* pStd)");
    const auto listingSoldEnd = fieldSource.find("int TMFieldScene::OnPacketUpdateCargoCoin", listingSoldStart);
    const auto listingSoldHandler = listingSoldStart != std::string::npos && listingSoldEnd != std::string::npos
        ? fieldSource.substr(listingSoldStart, listingSoldEnd - listingSoldStart) : std::string{};
    const auto listingSoldPickup = listingSoldHandler.find("->PickupAtItem(0, 0);");
    const auto listingSoldModelClear = listingSoldHandler.find("field_interaction::ClearAutoTradeOffer(");
    check(listingSoldModelClear != std::string::npos &&
        listingSoldHandler.find("pStd->Parm1 == m_stAutoTrade.TargetID") < listingSoldModelClear &&
        listingSoldHandler.find("pStd->Parm2 < autoTradeSlotCount") < listingSoldModelClear &&
        listingSoldModelClear < listingSoldHandler.find("if (!pGrid)") &&
        listingSoldHandler.find("pPrice->SetText(emptyPrice, 0);") < listingSoldPickup &&
        listingSoldHandler.find("pPrice->SetVisible(0);") < listingSoldPickup &&
        listingSoldHandler.find("pGrid->m_nTradeMoney = 0;") < listingSoldPickup,
        "sold listing clears model independently of optional grids and hides its stale price");
    check(listingSoldHandler.find("WYD748_CancelAutoTradePurchase(m_pMessageBox, pStd->Parm2);") < listingSoldPickup,
        "sold listing cancels its confirmation even when its visual is already absent");
    const auto listingSoldRelease = listingSoldHandler.find("WYD748_ReleaseAutoTradeItem(pItem);");
    const auto listingCleanupStart = fieldSource.find("void WYD748_ReleaseAutoTradeItem(SGridControlItem*& pItem)");
    const auto listingCleanupEnd = fieldSource.find("SAFE_DELETE(pItem);", listingCleanupStart);
    const auto listingSoldCleanup = listingCleanupStart != std::string::npos && listingCleanupEnd != std::string::npos
        ? fieldSource.substr(listingCleanupStart, listingCleanupEnd - listingCleanupStart) : std::string{};
    check(listingSoldPickup != std::string::npos && listingSoldRelease != std::string::npos &&
        listingSoldHandler.find("pPanel->IsVisible() == 1") < listingSoldPickup &&
        listingSoldHandler.find("pStd->Parm1 == m_stAutoTrade.TargetID") < listingSoldPickup &&
        listingSoldHandler.find("pStd->Parm2 < autoTradeSlotCount") < listingSoldPickup &&
        listingSoldHandler.find("if (!pItem)\n\t\t\treturn 1;", listingSoldPickup) < listingSoldRelease,
        "sold listing verifies the active shop and treats an already absent visual as a no-op");
    check(listingSoldCleanup.find("SGridControl::m_pLastMouseOverItem == pItem") != std::string::npos &&
        listingSoldCleanup.find("SGridControl::m_pLastMouseOverItem = nullptr;") != std::string::npos &&
        listingSoldCleanup.find("SGridControl::m_sLastMouseOverIndex = -1;") != std::string::npos &&
        listingSoldCleanup.find("SGridControl::m_pLastAttachedItem == pItem") != std::string::npos &&
        listingSoldCleanup.find("SGridControl::m_pLastAttachedItem = nullptr;") != std::string::npos &&
        listingSoldCleanup.find("SGridControl::m_pSellItem == pItem") != std::string::npos &&
        listingSoldCleanup.find("SGridControl::m_pSellItem = nullptr;") != std::string::npos,
        "sold listing clears matching hover, drag and sale-dialog aliases before destroying the visual");
    check(listingSoldCleanup.find("g_pCursor && g_pCursor->m_pAttachedItem == pItem") != std::string::npos &&
        listingSoldCleanup.find("g_pCursor->DetachItem();") != std::string::npos &&
        listingSoldHandler.find("g_pCursor->m_pAttachedItem = nullptr;") == std::string::npos,
        "sold listing resets the matching cursor through DetachItem without disrupting unrelated items");
    check(listingSoldCleanup.find("if (!pItem)\n\t\t\treturn;") <
        listingSoldCleanup.find("SGridControl::m_pLastMouseOverItem == pItem"),
        "empty auto-trade cleanup leaves unrelated interaction state unchanged");
    const auto listingSnapshotStart = fieldSource.find("int TMFieldScene::OnPacketAutoTrade(MSG_STANDARD* pStd)");
    const auto listingSnapshotEnd = fieldSource.find("int TMFieldScene::OnPacketSwapItem", listingSnapshotStart);
    const auto listingSnapshot = listingSnapshotStart != std::string::npos && listingSnapshotEnd != std::string::npos
        ? fieldSource.substr(listingSnapshotStart, listingSnapshotEnd - listingSnapshotStart) : std::string{};
    const auto listingSnapshotRelease = listingSnapshot.find("WYD748_ReleaseAutoTradeItem(pItem);");
    check(listingSnapshot.find("WYD748_CancelAutoTradePurchase(m_pMessageBox);") <
        listingSnapshot.find("memcpy(&m_stAutoTrade, pAutoTrade, sizeof(m_stAutoTrade));"),
        "new shop snapshot cancels confirmation before replacing selected offer data");
    check(listingSnapshotRelease != std::string::npos &&
        listingSnapshot.find("pGrid->PickupAtItem(0, 0);") < listingSnapshotRelease &&
        listingSnapshotRelease < listingSnapshot.find("auto pstItem = new STRUCT_ITEM();") &&
        listingSnapshot.find("delete pItem;") == std::string::npos &&
        listingSnapshot.find("g_pCursor->m_pAttachedItem = nullptr;") == std::string::npos,
        "auto-trade snapshot releases previous interaction aliases before allocating replacement visuals");
    const auto listingCloseStart = fieldSource.find("void TMFieldScene::SetVisibleAutoTrade(");
    const auto listingCloseEnd = fieldSource.find("void TMFieldScene::SetWhisper(", listingCloseStart);
    const auto listingClose = listingCloseStart != std::string::npos && listingCloseEnd != std::string::npos
        ? fieldSource.substr(listingCloseStart, listingCloseEnd - listingCloseStart) : std::string{};
    const auto nativeListingCloseRelease = listingClose.find("WYD748_ReleaseAutoTradeItem(pItem);");
    check(listingClose.find("if (!bShow)\n\t\tWYD748_CancelAutoTradePurchase(m_pMessageBox);") <
        listingClose.find("if (m_bCompatFieldScene)"),
        "closing either shop layout cancels purchase before optional-control early returns");
    const auto importedListingCloseRelease = nativeListingCloseRelease != std::string::npos
        ? listingClose.find("WYD748_ReleaseAutoTradeItem(pItem);", nativeListingCloseRelease + 1) : std::string::npos;
    check(nativeListingCloseRelease != std::string::npos && importedListingCloseRelease != std::string::npos &&
        listingClose.find("for (int slot = 0; slot < 12; ++slot)") < nativeListingCloseRelease &&
        listingClose.find("SAFE_DELETE(pItem);") == std::string::npos &&
        listingClose.find("g_pCursor->m_pAttachedItem = nullptr;") == std::string::npos,
        "both auto-trade close paths release matching interaction aliases through the shared cleanup");
    const auto buyStart = fieldSource.find("void TMFieldScene::SendReqBuy(");
    const auto prepareStart = fieldSource.find("auto pATradeTitle = (SText*)m_pControlContainer->FindControl(TMT_ATRADE_TITLE);");
    const auto prepareEnd = fieldSource.find("SetVisibleAutoTrade(1, 1);", prepareStart);
    const auto prepareBody = prepareStart != std::string::npos && prepareEnd != std::string::npos
        ? fieldSource.substr(prepareStart, prepareEnd - prepareStart) : std::string{};
    const auto prepareRelease = prepareBody.find("WYD748_ReleaseAutoTradeItem(pAutoTradeItem);");
    check(prepareRelease != std::string::npos &&
        prepareBody.find("m_bCompatFieldScene ? 12 : 10") < prepareRelease &&
        prepareBody.find("pAutoTradeGrid->PickupAtItem(0, 0);") < prepareRelease &&
        prepareBody.find("pCargoItem->m_GCObj.dwColor = -1;") < prepareRelease &&
        prepareBody.find("delete pAutoTradeItem;") == std::string::npos &&
        prepareBody.find("g_pCursor->m_pAttachedItem = nullptr;") == std::string::npos,
        "seller preparation releases all listing aliases after restoring cargo highlighting");
    const auto priceStart = fieldSource.find("constexpr int kNativeAutoTradeSlots = 12;");
    const auto priceGuard = fieldSource.find(
        "if (m_nCoinMsgType == 4 && !field_interaction::IsValidAutoTradePrice(nInputValue))");
    const auto priceGuardEnd = fieldSource.find("switch (m_nCoinMsgType)", priceGuard);
    const auto priceGuardBody = priceGuard != std::string::npos && priceGuardEnd != std::string::npos
        ? fieldSource.substr(priceGuard, priceGuardEnd - priceGuard) : std::string{};
    check(!priceGuardBody.empty() && priceGuardEnd < priceStart &&
        priceGuardBody.find("if (nInputValue <= 0)") != std::string::npos &&
        priceGuardBody.find("g_pMessageStringTable[34]") != std::string::npos &&
        priceGuardBody.find("g_pMessageStringTable[143]") != std::string::npos &&
        priceGuardBody.find("SetFocusedControl(pInputText);") != std::string::npos &&
        priceGuardBody.find("return 1;") != std::string::npos,
        "invalid shop price keeps the prompt focused and returns before reserving cargo or an offer slot");
    const auto priceEnd = fieldSource.find("m_nLastAutoTradePos = -1;", priceStart);
    const auto priceBody = priceStart != std::string::npos && priceEnd != std::string::npos
        ? fieldSource.substr(priceStart, priceEnd - priceStart) : std::string{};
    const auto cargoPayloadGuard = priceBody.find("if (!pCargoItem || !pCargoItem->m_pItem)");
    check(cargoPayloadGuard != std::string::npos &&
        cargoPayloadGuard < priceBody.find("memcpy(&selectedItem, pCargoItem->m_pItem, sizeof(STRUCT_ITEM));"),
        "seller offer preparation rejects a missing cargo payload before copying it");
    const auto publishStart = fieldSource.find("if (idwControlID == 667)");
    const auto publishSend = fieldSource.find("SendOneMessage((char*)&m_stAutoTrade, sizeof(m_stAutoTrade));", publishStart);
    const auto publishBody = publishStart != std::string::npos && publishSend != std::string::npos
        ? fieldSource.substr(publishStart, publishSend - publishStart) : std::string{};
    check(publishBody.find("if (!field_interaction::HasAutoTradeOffers(m_stAutoTrade.Item))") != std::string::npos &&
        publishBody.find("i < 10") == std::string::npos,
        "shop publication checks the complete wire item array before sending");
    const auto buyEnd = fieldSource.find("void TMFieldScene::SetSanc()", buyStart);
    const auto buyHandler = buyStart != std::string::npos && buyEnd != std::string::npos
        ? fieldSource.substr(buyStart, buyEnd - buyStart) : std::string{};
    check(buyHandler.find("field_interaction::AutoTradeSlotIndex(m_bCompatFieldScene != 0, dwControlID)") != std::string::npos &&
        buyHandler.find("if (slot < 0") < buyHandler.find("m_pGridAutoTrade[slot]") &&
        buyHandler.find("!m_pMyHuman || !m_pAutoTrade || !m_pAutoTrade->IsVisible()") != std::string::npos &&
        buyHandler.find("grid->GetAtItem(0, 0)") < buyHandler.find("MSG_ReqBuy stReqBuy{};") &&
        buyHandler.find("stReqBuy.Price = m_stAutoTrade.TradeMoney[slot];") != std::string::npos &&
        buyHandler.find("dwControlID - 653") == std::string::npos,
        "purchase sender rejects invalid control, absent scene and missing listing before reading arrays");
    const auto cancelStart = fieldSource.find("void WYD748_CancelAutoTradePurchase(");
    const auto cancelEnd = fieldSource.find("void WYD748_ReleaseAutoTradeItem(", cancelStart);
    const auto cancelBody = cancelStart != std::string::npos && cancelEnd != std::string::npos
        ? fieldSource.substr(cancelStart, cancelEnd - cancelStart) : std::string{};
    check(cancelBody.find("if (!dialog || !field_interaction::ShouldCancelAutoTradePurchase(") != std::string::npos &&
        cancelBody.find("dialog->m_dwMessage = static_cast<unsigned int>(-1);") != std::string::npos &&
        cancelBody.find("dialog->m_dwArg = 0;") != std::string::npos &&
        cancelBody.find("if (dialog->IsVisible())\n\t\t\tdialog->SetVisible(0);") != std::string::npos,
        "purchase invalidation clears stale callback data without stealing focus from unrelated hidden dialogs");
    const auto dropStart = fieldSource.find("int TMFieldScene::OnPacketCNFDropItem(MSG_CNFDropItem* pMsg)");
    const auto dropEnd = fieldSource.find("int TMFieldScene::OnPacketCNFGetItem", dropStart);
    const auto dropHandler = dropStart != std::string::npos && dropEnd != std::string::npos
        ? fieldSource.substr(dropStart, dropEnd - dropStart) : std::string{};
    const auto dropRelease = dropHandler.find("SAFE_DELETE(pGridItem);");
    const auto dropHumanGuard = dropHandler.find("if (!m_pMyHuman)\n\t\treturn 1;");
    const auto dropFamiliar = dropHandler.find("m_pMyHuman->m_sFamiliar =");
    check(!dropHandler.empty() && dropRelease != std::string::npos &&
        dropHumanGuard != std::string::npos && dropFamiliar != std::string::npos &&
        dropHandler.find("memset(&g_pObjectManager->m_stMobData.Equip[pMsg->SourPos]") < dropRelease &&
        dropHandler.find("memset(&g_pObjectManager->m_stMobData.Carry[pMsg->SourPos]") < dropRelease &&
        dropHandler.find("memset(&g_pObjectManager->m_stItemCargo[pMsg->SourPos]") < dropRelease &&
        dropRelease < dropHumanGuard && dropHumanGuard < dropFamiliar &&
        dropHandler.find("if (g_pCursor)\n\t\tg_pCursor->DetachItem();") < dropRelease,
        "confirmed drop commits model and releases visual before requiring a local human");
    const auto dropCleanup = dropRelease != std::string::npos
        ? dropHandler.substr(0, dropRelease) : std::string{};
    check(dropCleanup.find("SGridControl::m_pLastMouseOverItem == pGridItem") != std::string::npos &&
        dropCleanup.find("SGridControl::m_pLastMouseOverItem = nullptr;") != std::string::npos &&
        dropCleanup.find("SGridControl::m_sLastMouseOverIndex = -1;") != std::string::npos &&
        dropCleanup.find("SGridControl::m_pLastAttachedItem == pGridItem") != std::string::npos &&
        dropCleanup.find("SGridControl::m_pLastAttachedItem = nullptr;") != std::string::npos &&
        dropCleanup.find("SGridControl::m_pSellItem == pGridItem") != std::string::npos &&
        dropCleanup.find("SGridControl::m_pSellItem = nullptr;") != std::string::npos,
        "confirmed drop clears hover, drag and sell aliases before destroying the grid item");
    const auto saleStart = fieldSource.find("int TMFieldScene::OnPacketSell(MSG_STANDARD* pStd)");
    const auto saleEnd = fieldSource.find("int TMFieldScene::OnPacketCNFMobKill", saleStart);
    const auto saleHandler = saleStart != std::string::npos && saleEnd != std::string::npos
        ? fieldSource.substr(saleStart, saleEnd - saleStart) : std::string{};
    const auto salePrice = saleHandler.find("native_sale_price::Calculate(catalogPrice)");
    const auto saleCredit = saleHandler.find("m_stMobData.Coin += nPrice;");
    check(salePrice != std::string::npos && saleCredit != std::string::npos &&
        saleHandler.find("itemIndex > 0 && itemIndex < MAX_ITEMLIST") < salePrice &&
        salePrice < saleCredit && saleHandler.find("(float)") == std::string::npos &&
        saleHandler.find("native_sale_quote::Calculate") == std::string::npos,
        "legacy sale uses bounded native integer arithmetic without quote-only overrides or float32 rounding");
    const auto salePickup = saleHandler.find("pGridDest[pSell->MyPos]->PickupItem(0, 0)");
    check(!saleHandler.empty() && salePickup != std::string::npos &&
        saleHandler.find("!g_pObjectManager") < salePickup &&
        saleHandler.find("pSell->MyType < 0 || pSell->MyType > 1") < salePickup &&
        saleHandler.find("pSell->MyPos < 0") < salePickup &&
        saleHandler.find("pSell->MyPos >= MAX_EQUIPITEM") < salePickup &&
        saleHandler.find("pSell->MyPos >= MAX_CARRY") < salePickup &&
        saleHandler.find("m_pGridHellStore &&") < salePickup &&
        saleHandler.find("m_pGridShop &&") < salePickup &&
        saleHandler.find("if (pGridDest[pSell->MyPos])") < salePickup,
        "legacy sale bounds slots and tolerates absent merchant and equipment grids");
    const auto saleRelease = saleHandler.find("SAFE_DELETE(pDestItem);");
    const auto saleCleanup = saleRelease != std::string::npos
        ? saleHandler.substr(0, saleRelease) : std::string{};
    check(!saleCleanup.empty() &&
        saleCleanup.find("SGridControl::m_pLastMouseOverItem == pDestItem") != std::string::npos &&
        saleCleanup.find("SGridControl::m_pLastMouseOverItem = nullptr;") != std::string::npos &&
        saleCleanup.find("SGridControl::m_sLastMouseOverIndex = -1;") != std::string::npos &&
        saleCleanup.find("SGridControl::m_pLastAttachedItem == pDestItem") != std::string::npos &&
        saleCleanup.find("SGridControl::m_pLastAttachedItem = nullptr;") != std::string::npos &&
        saleCleanup.find("SGridControl::m_pSellItem == pDestItem") != std::string::npos &&
        saleCleanup.find("SGridControl::m_pSellItem = nullptr;") != std::string::npos &&
        saleCleanup.find("g_pCursor && g_pCursor->m_pAttachedItem == pDestItem") != std::string::npos &&
        saleCleanup.find("g_pCursor->DetachItem();") != std::string::npos,
        "legacy sale clears matching interaction aliases before deleting the detached visual");
    check(!saleHandler.empty() && saleRelease != std::string::npos &&
        saleHandler.find("const bool hasItem = pDestItem->m_pItem != nullptr;") < saleRelease &&
        saleHandler.find("if (!hasItem)", saleRelease) != std::string::npos &&
        saleHandler.find("if (m_pMyHuman)\n\t\tUpdateMyHuman();") != std::string::npos,
        "legacy sale releases an incomplete visual without crediting gold or requiring a renderer");
    const auto swapStart = fieldSource.find("int TMFieldScene::OnPacketSwapItem(MSG_STANDARD* pStd)");
    const auto swapEnd = fieldSource.find("int TMFieldScene::OnPacketShopList", swapStart);
    const auto swapHandler = swapStart != std::string::npos && swapEnd != std::string::npos
        ? fieldSource.substr(swapStart, swapEnd - swapStart) : std::string{};
    const auto resolveModel = swapHandler.find("STRUCT_ITEM* sourceModel = modelItem(");
    const auto firstPickup = swapHandler.find("pSrcGrid->PickupItem(0, 0);");
    const auto modelCommit = swapHandler.find("ApplyConfirmedItemSwap(*sourceModel, *destinationModel);");
    const auto restoreVisual = swapHandler.find("restoreMissingVisual(pSrcGrid", modelCommit);
    const auto restoreDestination = swapHandler.find("restoreMissingVisual(pDestGrid", restoreVisual);
    const auto updateHuman = swapHandler.find("UpdateMyHuman();");
    check(!swapHandler.empty() && resolveModel != std::string::npos &&
        firstPickup != std::string::npos && modelCommit != std::string::npos &&
        restoreVisual != std::string::npos && restoreDestination != std::string::npos &&
        updateHuman != std::string::npos && resolveModel < firstPickup &&
        firstPickup < modelCommit && modelCommit < restoreVisual &&
        restoreVisual < restoreDestination && restoreDestination < updateHuman &&
        swapHandler.find("if (!grid->GetAtItem(cellX, cellY))") != std::string::npos &&
        swapHandler.find("grid->SetItemOnGrid(model, cellX, cellY);") != std::string::npos &&
        swapHandler.find("memset(&g_pObjectManager->m_stMobData.Equip[pSwapItem->") == std::string::npos &&
        swapHandler.find("memcpy(&g_pObjectManager->m_stMobData.Carry[pSwapItem->") == std::string::npos &&
        swapHandler.find("if (pSrcGrid)\n\t\t\tpSrcItem = pSrcGrid->PickupItem(0, 0);") != std::string::npos &&
        swapHandler.find("if (pDestGrid)\n\t\t\tpDestItem = pDestGrid->PickupItem(0, 0);") != std::string::npos,
        "confirmed swap commits model items independently of optional equipment grids");
    check(swapHandler.find("pSwapItem->SourType == kSwapPlaceEquip && pSwapItem->SourPos == 14") != std::string::npos,
        "Carry/Cargo slot 14 cannot be mistaken for the equipment mount slot");
    const auto releaseStart = swapHandler.find("auto releaseRejectedVisual =");
    const auto releaseEnd = swapHandler.find("if (!pSwapItem->SourType)", releaseStart);
    const auto releaseBody = releaseStart != std::string::npos && releaseEnd != std::string::npos
        ? swapHandler.substr(releaseStart, releaseEnd - releaseStart) : std::string{};
    check(!releaseBody.empty() &&
        releaseBody.find("m_pLastMouseOverItem == item") != std::string::npos &&
        releaseBody.find("m_pLastAttachedItem == item") != std::string::npos &&
        releaseBody.find("m_pSellItem == item") != std::string::npos &&
        releaseBody.find("m_pAttachedItem == item") != std::string::npos &&
        releaseBody.find("SAFE_DELETE(item);") != std::string::npos &&
        swapHandler.find("SAFE_DELETE(pSrcItem)") == std::string::npos &&
        swapHandler.find("SAFE_DELETE(pDestItem)") == std::string::npos,
        "rejected swap visuals clear hover, sell and cursor aliases before deletion");
    const auto splitStart = fieldSource.find("case 12:\n\t\t\t{");
    const auto splitEnd = fieldSource.find("m_pControlContainer->SetFocusedControl(0);", splitStart);
    const auto splitHandler = splitStart != std::string::npos && splitEnd != std::string::npos
        ? fieldSource.substr(splitStart, splitEnd - splitStart) : std::string{};
    const auto splitOwned = splitHandler.find("field_interaction::FindOwnedItem(");
    const auto splitAmount = splitHandler.find("BASE_GetItemAmount(pSplitItem->m_pItem)");
    const auto splitGuard = splitHandler.find("if (!pSplitItem || !pSplitItem->m_pItem ||");
    check(!splitHandler.empty() && splitOwned != std::string::npos && splitGuard != std::string::npos &&
        splitAmount != std::string::npos && splitOwned < splitGuard && splitGuard < splitAmount &&
        splitHandler.find("pSplitItem->m_pGridControl != m_pGridInv") < splitAmount &&
        splitHandler.find("SGridControl::m_pSellItem->") == std::string::npos,
        "split dialog does not dereference an item invalidated by swap cleanup");
    const auto splitQuantityGuard = splitHandler.find("!field_interaction::IsValidStackSplitQuantity(");
    const auto splitSend = splitHandler.find("SendPacket(");
    check(splitQuantityGuard != std::string::npos && splitSend != std::string::npos &&
        splitQuantityGuard < splitSend &&
        splitHandler.find("SetInVisibleInputCoin();", splitSend) != std::string::npos &&
        splitHandler.find("PickupItem(") == std::string::npos,
        "split validates quantity before send and closes selection without changing inventory");
    const auto closeSplitStart = fieldSource.find("void TMFieldScene::SetInVisibleInputCoin()");
    const auto closeSplitEnd = fieldSource.find("void TMFieldScene::SetInventoryGridType", closeSplitStart);
    const auto closeSplit = closeSplitStart != std::string::npos && closeSplitEnd != std::string::npos
        ? fieldSource.substr(closeSplitStart, closeSplitEnd - closeSplitStart) : std::string{};
    check(!closeSplit.empty() && closeSplit.find("if (m_nCoinMsgType == 12)") != std::string::npos &&
        closeSplit.find("field_interaction::FindOwnedItem(") < closeSplit.find("pSplitItem->m_GCObj.dwColor") &&
        closeSplit.find("SGridControl::m_pSellItem = nullptr;") != std::string::npos &&
        closeSplit.find("m_nCoinMsgType = -1;") != std::string::npos,
        "closing a split clears its mode and alias and restores only a live item highlight");
    const auto fieldTerrain = fieldSource.find("if (!m_pGroundList[0]->LoadTileMap(szMapPath))");
    const auto fieldMiniMap = fieldSource.find("m_pGround->SetMiniMapData();", fieldTerrain);
    check(fieldTerrain != std::string::npos && fieldMiniMap != std::string::npos &&
        fieldSource.substr(fieldTerrain, fieldMiniMap - fieldTerrain).find("return 0;") != std::string::npos,
        "field initialization stops before using an invalid terrain");
    const auto deleteConfirm = fieldSource.find("case 740:");
    const auto sellConfirm = fieldSource.find("case 890:", deleteConfirm);
    const auto deleteHandler = deleteConfirm != std::string::npos && sellConfirm != std::string::npos
        ? fieldSource.substr(deleteConfirm, sellConfirm - deleteConfirm) : std::string{};
    check(!deleteHandler.empty() &&
        deleteHandler.find("!pSellItem ||") != std::string::npos &&
        deleteHandler.find("MSG_DeleteItem_Opcode") != std::string::npos &&
        deleteHandler.find("PickupAtItem(") == std::string::npos &&
        deleteHandler.find("SendPacket(") < deleteHandler.find("m_pSellItem = nullptr;"),
        "delete confirmation retains the grid-owned item until the server sends its authoritative slot");
	const auto sellEnd = fieldSource.find("case 271:", sellConfirm);
	const auto sellHandler = sellConfirm != std::string::npos && sellEnd != std::string::npos
		? fieldSource.substr(sellConfirm, sellEnd - sellConfirm) : std::string{};
	check(!sellHandler.empty() &&
		sellHandler.find("GetCarrySlotForCell(pSellItem->m_pGridControl") != std::string::npos &&
		sellHandler.find("sDestType == 1") != std::string::npos &&
		sellHandler.find("m_pGridShop->m_dwMerchantID") != std::string::npos,
		"shop sell confirmation sends the native Carry slot and visible merchant");

    const auto gridSource = LoadSource("TMProject748/internal/ui/SGrid.cpp");
    const auto splitPromptStart = gridSource.find("else if (dwFlags == 513 && g_pEventTranslator->m_bShift)");
    const auto splitPromptEnd = gridSource.find("else if (!bClick && dwFlags == 517", splitPromptStart);
    const auto splitPrompt = splitPromptStart != std::string::npos && splitPromptEnd != std::string::npos
        ? gridSource.substr(splitPromptStart, splitPromptEnd - splitPromptStart) : std::string{};
    const auto splitControlsGuard = splitPrompt.find("if (!pText || !pEdit || !pInputGold)\n\t\t\treturn 0;");
    check(!splitPrompt.empty() && splitControlsGuard != std::string::npos &&
        splitPrompt.find("if (!pFScene || !pFScene->m_pControlContainer)") < splitPrompt.find("SelectItem(") &&
        splitPrompt.find("if (!pItem || !pItem->m_pItem)") < splitPrompt.find("BASE_GetItemAmount(") &&
        splitControlsGuard < splitPrompt.find("pItem->m_GCObj.dwColor =") &&
        splitControlsGuard < splitPrompt.find("m_nCoinMsgType = 12;") &&
        splitControlsGuard < splitPrompt.find("SGridControl::m_pSellItem = pItem;") &&
        splitControlsGuard < splitPrompt.find("pText->SetText("),
        "split prompt rejects missing controls before reserving or dereferencing dialog state");
    const auto splitSelection = splitPrompt.find("SelectItem(");
    const auto splitHitGuard = splitPrompt.find("if (!bPtInRect || this != pFScene->m_pGridInv)\n\t\t\treturn 0;");
    const auto splitBusyGuard = splitPrompt.find("if (pInputGold->IsVisible())\n\t\t\treturn 0;");
    check(splitSelection != std::string::npos && splitHitGuard != std::string::npos &&
        splitHitGuard < splitSelection,
        "broadcast shift-click cannot select a split source outside the Carry grid");
    check(splitSelection != std::string::npos && splitBusyGuard != std::string::npos &&
        splitControlsGuard < splitBusyGuard && splitBusyGuard < splitSelection,
        "split entry preserves selection and pending intent while the shared prompt is open");
    const auto pricePromptStart = gridSource.find("if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV2)");
    const auto pricePromptEnd = gridSource.find("if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV3)", pricePromptStart);
    const auto pricePrompt = pricePromptStart != std::string::npos && pricePromptEnd != std::string::npos
        ? gridSource.substr(pricePromptStart, pricePromptEnd - pricePromptStart) : std::string{};
    const auto priceControlsGuard = pricePrompt.find("if (!pText || !pEdit)\n\t\t\t\t\treturn 1;");
    const auto priceSlotGuard = pricePrompt.find("if (cargoSlot < 0)\n\t\t\t\t\treturn 1;");
    check(!pricePrompt.empty() && priceControlsGuard != std::string::npos && priceSlotGuard != std::string::npos &&
        priceControlsGuard < priceSlotGuard &&
        priceSlotGuard < pricePrompt.find("pItem->m_GCObj.dwColor = 0xFFFF00FF;") &&
        priceSlotGuard < pricePrompt.find("m_nCoinMsgType = 4;") &&
        priceSlotGuard < pricePrompt.find("m_nLastAutoTradePos = cargoSlot;") &&
        priceSlotGuard < pricePrompt.find("pText->SetText("),
        "auto-trade price prompt validates controls and Cargo slot before committing selection");
    const auto cubeStart = gridSource.find("else if (m_eGridType == TMEGRIDTYPE::GRID_CUBEBOX)");
    const auto cubeEnd = gridSource.find("\n\telse\n\t{", cubeStart);
    const auto cubeHandler = cubeStart != std::string::npos && cubeEnd != std::string::npos
        ? gridSource.substr(cubeStart, cubeEnd - cubeStart) : std::string{};
    const auto cubePickup = cubeHandler.find("auto pItem = PickupItem(nCellX, nCellY);");
    const auto cubeRelease = cubeHandler.find("SAFE_DELETE(pItem);");
    check(!cubeHandler.empty() && cubePickup != std::string::npos &&
        cubeRelease != std::string::npos && cubePickup < cubeRelease &&
        cubeHandler.find("m_pLastMouseOverItem = nullptr;", cubePickup) < cubeRelease &&
        cubeHandler.find("m_pLastAttachedItem = nullptr;", cubePickup) < cubeRelease &&
        cubeHandler.find("m_pSellItem = nullptr;", cubePickup) < cubeRelease &&
        cubeHandler.find("g_pCursor->m_pAttachedItem = nullptr;", cubePickup) < cubeRelease,
        "cube-box removal clears all interaction aliases before releasing its visual");
    const auto deleteDrop = gridSource.find("else if (m_eGridType == TMEGRIDTYPE::GRID_DELETE)");
    const auto nextDrop = gridSource.find("else if (m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT1", deleteDrop);
    const auto deleteDropHandler = deleteDrop != std::string::npos && nextDrop != std::string::npos
        ? gridSource.substr(deleteDrop, nextDrop - deleteDrop) : std::string{};
    const auto validItem = deleteDropHandler.find("WYD748_IsValidItemIndex");
    const auto catalogName = deleteDropHandler.find("g_pItemList[SGridControl::m_pSellItem->m_pItem->sIndex].Name");
    check(!deleteDropHandler.empty() && validItem != std::string::npos &&
        catalogName != std::string::npos && validItem < catalogName &&
        deleteDropHandler.find("!SGridControl::m_pSellItem ||") != std::string::npos &&
        deleteDropHandler.find("sprintf_s(szMessage, sizeof(szMessage), \"%.*s\"") != std::string::npos &&
        deleteDropHandler.find("sizeof(g_pItemList[0].Name)") != std::string::npos,
        "delete prompt validates the item and treats its fixed-width catalog name as data");
    char malformedName[64];
    std::fill_n(malformedName, sizeof(malformedName), 'X');
    malformedName[0] = '%';
    malformedName[1] = 's';
    char boundedMessage[128]{};
    const int copied = sprintf_s(boundedMessage, sizeof(boundedMessage), "%.*s",
        static_cast<int>(sizeof(malformedName)), malformedName);
    check(copied == sizeof(malformedName) && boundedMessage[0] == '%' &&
        boundedMessage[1] == 's' && boundedMessage[sizeof(malformedName)] == '\0',
        "delete prompt bounds unterminated names and preserves percent signs literally");

    char sellMessage[128]{};
    check(wyd748::ui::FormatSellConfirmation(sellMessage, sizeof(sellMessage),
        "'%s'", sizeof("'%s'"), malformedName, sizeof(malformedName)) &&
        sellMessage[0] == '\'' && sellMessage[1] == '%' && sellMessage[2] == 's' &&
        sellMessage[65] == '\'' && sellMessage[66] == '\0',
        "sell prompt preserves the 7.48 quotes and bounds a percent-bearing item name");
    check(!wyd748::ui::FormatSellConfirmation(sellMessage, 66,
        "'%s'", sizeof("'%s'"), malformedName, sizeof(malformedName)) &&
        !wyd748::ui::FormatSellConfirmation(sellMessage, sizeof(sellMessage),
            "%s%s", sizeof("%s%s"), malformedName, sizeof(malformedName)),
        "sell prompt rejects output overflow and ambiguous templates");
    check(gridSource.find("sprintf(szMessage, g_pMessageStringTable[342]") == std::string::npos &&
        gridSource.find("WYD748_FormatSellConfirmation(szMessage, pItem)") != std::string::npos &&
        gridSource.find("WYD748_FormatSellConfirmation(szMessage, SGridControl::m_pSellItem)") != std::string::npos,
        "both sell interactions and the drag path use the bounded formatter");

    // Exercise the actual record-boundary reader without constructing DirectX UI.
    const auto sceneLoader = LoadSource("TMProject748/internal/app/scenes/TMScene.cpp");
    const auto warp = sceneLoader.find("void TMScene::Warp2(int nZoneX, int nZoneY)");
    const auto warpTerrain = sceneLoader.find("if (!pGround->LoadTileMap(szMapPath))", warp);
    const auto warpCommit = sceneLoader.find("m_pGround = pGround;", warpTerrain);
    check(warp != std::string::npos && warpTerrain != std::string::npos &&
        warpCommit != std::string::npos &&
        sceneLoader.substr(warpTerrain, warpCommit - warpTerrain).find("delete pGround;") != std::string::npos,
        "failed warp terrain load releases the uncommitted ground");
    const auto warpCandidate = warpTerrain != std::string::npos && warpCommit != std::string::npos
        ? sceneLoader.substr(warpTerrain, warpCommit - warpTerrain) : std::string{};
    check(warpCandidate.find("pGround->m_vecOffsetIndex.x != nZoneX || pGround->m_vecOffsetIndex.y != nZoneY") != std::string::npos &&
        warpCandidate.find("TerrainFile Position Mismatch") != std::string::npos &&
        warpCandidate.find("delete pGround;") != std::string::npos &&
        warpCandidate.find("m_bCriticalError = 1;") != std::string::npos,
        "warp rejects a terrain with a different embedded position before replacing the active ground");
    const auto attachStart = sceneLoader.find("int TMScene::GroundNewAttach(EDirection eDir)");
    const auto attachEnd = sceneLoader.find("D3DXVECTOR3 TMScene::GroundGetPickPos()", attachStart);
    check(attachStart != std::string::npos && attachEnd != std::string::npos,
        "neighbor terrain loader is delimited");
    if (attachStart != std::string::npos && attachEnd != std::string::npos) {
        const auto attach = sceneLoader.substr(attachStart, attachEnd - attachStart);
        const auto guard = attach.find("if (m_bCriticalError == 1 || !m_pGround)");
        const auto neighborLoad = attach.find("if (!pGround->LoadTileMap(fileNameTrn))");
        const auto commitTerrain = attach.find("delete m_pObjectContainerList[gId];", neighborLoad);
        const auto rejectedTerrain = neighborLoad != std::string::npos && commitTerrain != std::string::npos
            ? attach.substr(neighborLoad, commitTerrain - neighborLoad) : std::string{};
        check(guard != std::string::npos && neighborLoad != std::string::npos && guard < neighborLoad &&
            rejectedTerrain.find("LogMsgCriticalError(10, 0, 0, 0, 0);") != std::string::npos &&
            rejectedTerrain.find("m_bCriticalError = 1;") != std::string::npos &&
            rejectedTerrain.find("delete pGround;") != std::string::npos &&
            rejectedTerrain.find("return 0;") != std::string::npos,
            "failed neighbor terrain load is reported once before replacing the active ground");
        check(rejectedTerrain.find("pGround->m_vecOffsetIndex.x != x || pGround->m_vecOffsetIndex.y != y") != std::string::npos &&
            rejectedTerrain.find("TerrainFile Position Mismatch") != std::string::npos &&
            rejectedTerrain.find("m_bCriticalError = 1;") != std::string::npos,
            "neighbor load rejects a terrain with a different embedded position before attaching it");
        const auto releaseObjects = attach.find("delete m_pObjectContainerList[gId];");
        const auto releaseGround = attach.find("delete m_pGroundList[gId];");
        check(releaseObjects != std::string::npos && releaseGround != std::string::npos &&
            releaseObjects < releaseGround,
            "previous neighbor objects are released before their ground");
        const auto detachOld = attach.find("if (m_pGround->m_pLeftGround == m_pGroundList[gId])", releaseObjects);
        check(detachOld != std::string::npos && detachOld < releaseGround &&
            attach.find("m_pGround->m_pRightGround = nullptr;", detachOld) < releaseGround &&
            attach.find("m_pGround->m_pUpGround = nullptr;", detachOld) < releaseGround &&
            attach.find("m_pGround->m_pDownGround = nullptr;", detachOld) < releaseGround,
            "old neighbor links are cleared before its ground is released");
        const auto failedLoad = attach.find("if (!m_pObjectContainerList[gId]->Load(fileNameDat))");
        const auto reportFailure = attach.find("LOG_WRITELOG(\"DataFile Not Found", failedLoad);
        const auto rollback = failedLoad != std::string::npos && reportFailure != std::string::npos
            ? attach.substr(failedLoad, reportFailure - failedLoad) : std::string{};
        check(rollback.find("SAFE_DELETE(m_pObjectContainerList[gId]);") != std::string::npos &&
            rollback.find("m_pGround->m_pLeftGround = nullptr;") != std::string::npos &&
            rollback.find("m_pGround->m_pRightGround = nullptr;") != std::string::npos &&
            rollback.find("m_pGround->m_pUpGround = nullptr;") != std::string::npos &&
            rollback.find("m_pGround->m_pDownGround = nullptr;") != std::string::npos &&
            rollback.find("m_pGround->m_TileMapData[index] = previousBorder[i];") != std::string::npos &&
            rollback.find("m_pGround->m_TileNormalVector[index] = previousNormals[i];") != std::string::npos &&
            rollback.find("m_pGround->m_nMiniMapPos = previousMiniMapPos;") != std::string::npos &&
            rollback.find("SAFE_DELETE(m_pGroundList[gId]);") != std::string::npos,
            "failed neighbor object load restores the active border and removes the candidate");
        const auto attachGround = attach.find("->Attach(m_pGroundList[gId])", releaseGround);
        const auto reportAttachFailure = attach.find("TerrainFile Attach Failed", attachGround);
        check(attachGround != std::string::npos && failedLoad != std::string::npos &&
            attachGround < failedLoad && reportAttachFailure != std::string::npos &&
            attach.find("SAFE_DELETE(m_pGroundList[gId]);", attachGround) < reportAttachFailure,
            "neighbor terrain is attached before light objects use scene terrain color");
        const auto clearMask = attach.find("memset(m_HeightMapData, 0, sizeof(m_HeightMapData));");
        const auto applyAttributes = attach.find("BASE_ApplyAttribute((char*)m_HeightMapData, 256);");
        check(clearMask != std::string::npos && applyAttributes != std::string::npos &&
            clearMask < applyAttributes,
            "neighbor transition clears the complete mask before applying attributes");
    }
    const auto maskStart = sceneLoader.find("int TMScene::GroundGetMask(TMVector2 vecPosition)");
    const auto maskEnd = sceneLoader.find("float TMScene::GroundGetHeight(TMVector2 vecPosition)", maskStart);
    check(maskStart != std::string::npos && maskEnd != std::string::npos,
        "terrain mask accessors are delimited");
    if (maskStart != std::string::npos && maskEnd != std::string::npos) {
        const auto maskAccessors = sceneLoader.substr(maskStart, maskEnd - maskStart);
        const auto firstX = maskAccessors.find("if (nXIndex >= 256)");
        const auto firstY = maskAccessors.find("if (nYIndex >= 256)");
        check(firstX != std::string::npos && firstY != std::string::npos &&
            maskAccessors.find("if (nXIndex >= 256)", firstX + 1) != std::string::npos &&
            maskAccessors.find("if (nYIndex >= 256)", firstY + 1) != std::string::npos,
            "both terrain mask accessors clamp the first out-of-range cell");
    }
    const auto warpMask = sceneLoader.find("memset(m_HeightMapData, 0, sizeof(m_HeightMapData));", warp);
    const auto warpMaskCopy = sceneLoader.find("memcpy(m_HeightMapData[nY], m_pGround->m_pMaskData[nY], 128);", warp);
    check(warpMask != std::string::npos && warpMaskCopy != std::string::npos &&
        warpMask < warpMaskCopy,
        "warp clears the complete mask before copying the destination terrain");
    check(sceneLoader.find("ReadRCControlType(fpBinary, rawControlType)") != std::string::npos &&
        sceneLoader.find("readResult != RCControlTypeReadResult::Record") != std::string::npos,
        "scene loader rejects partial control-type records");
    auto testTypeRead = [&](const void* bytes, std::size_t size,
        RCControlTypeReadResult expected, const char* name) {
        std::FILE* stream = nullptr;
        if (tmpfile_s(&stream) != 0 || !stream) {
            check(false, name);
            return;
        }
        const bool written = size == 0 || std::fwrite(bytes, 1, size, stream) == size;
        std::rewind(stream);
        int type = 0;
        check(written && ReadRCControlType(stream, type) == expected, name);
        std::fclose(stream);
    };
    const int panelType = 1;
    testTypeRead(nullptr, 0, RCControlTypeReadResult::End,
        "empty RC stream ends cleanly");
    testTypeRead(&panelType, sizeof(panelType), RCControlTypeReadResult::Record,
        "complete RC control type is accepted");
    testTypeRead(&panelType, 2, RCControlTypeReadResult::Invalid,
        "truncated RC control type is rejected");
    std::FILE* trailing = nullptr;
    if (tmpfile_s(&trailing) == 0 && trailing) {
        const bool written = std::fwrite(&panelType, 1, sizeof(panelType), trailing) == sizeof(panelType) &&
            std::fwrite(&panelType, 1, 2, trailing) == 2;
        std::rewind(trailing);
        int type = 0;
        check(written && ReadRCControlType(trailing, type) == RCControlTypeReadResult::Record &&
            ReadRCControlType(trailing, type) == RCControlTypeReadResult::Invalid,
            "trailing partial RC tag is not mistaken for clean EOF");
        std::fclose(trailing);
    } else {
        check(false, "trailing partial RC tag is not mistaken for clean EOF");
    }

    const auto functionStart = source.find(
        "int TMSelectServerScene::OnPacketEvent(unsigned int dwCode, char* buf)");
    const auto functionEnd = source.find(
        "int TMSelectServerScene::FrameMove", functionStart);
    check(functionStart != std::string::npos && functionEnd != std::string::npos,
        "select-server packet handler is delimited");

    if (functionStart != std::string::npos && functionEnd != std::string::npos) {
        const std::string handler = source.substr(functionStart, functionEnd - functionStart);
        const auto baseDispatch = handler.find(
            "const int sceneResult = TMScene::OnPacketEvent(dwCode, buf);");
        const auto nullGuard = handler.find("if (!buf)");
        const auto nullReturn = handler.find("return sceneResult;", nullGuard);
        const auto packetCast = handler.find(
            "reinterpret_cast<MSG_STANDARD*>(buf)");

        check(baseDispatch != std::string::npos,
            "base scene receives every packet event");
        check(nullGuard != std::string::npos && nullReturn != std::string::npos,
            "null payload returns the base-scene result");
        check(packetCast != std::string::npos,
            "payload cast remains present for non-null packets");
        check(baseDispatch < nullGuard && nullGuard < nullReturn && nullReturn < packetCast,
            "disconnect is dispatched before any payload access");
    }

    // Rebuilding the channel list must invalidate the previous group's row;
    // otherwise Connect can reuse the same index for a different endpoint.
    const auto groupEvent = source.find("case L_SELECT_SERVERG:");
    const auto connectEvent = source.find("case B_SERVER_SEL_OK:", groupEvent);
    check(groupEvent != std::string::npos && connectEvent != std::string::npos,
        "group selection precedes connect handler");
    if (groupEvent != std::string::npos && connectEvent != std::string::npos) {
        const std::string handler = source.substr(groupEvent, connectEvent - groupEvent);
        const auto clearRows = handler.find("pServerList->Empty();");
        const auto clearSlots = handler.find("m_nVisibleChannelCount = 0;", clearRows);
        const auto clearSelection = handler.find("pServerList->SetSelectedIndex(-1);");
        const auto addRows = handler.find("for (int num = 1;; ++num)");
        check(clearRows != std::string::npos && clearSelection != std::string::npos &&
            addRows != std::string::npos && clearRows < clearSelection &&
            clearSelection < addRows,
            "group switch clears stale channel selection before adding rows");
        check(clearSlots != std::string::npos && clearSlots < addRows &&
            handler.find("m_nVisibleChannelSlots[m_nVisibleChannelCount++] = num;") !=
                std::string::npos,
            "channel rows retain their configured endpoint slots");
    }

    const auto eventStart = source.find("int TMSelectServerScene::OnControlEvent(");
    const auto uiStart = source.find("void TMSelectServerScene::InitializeUI()");
    if (eventStart != std::string::npos && uiStart != std::string::npos) {
        const std::string eventHandler = source.substr(eventStart, uiStart - eventStart);
        const std::string uiBuilder = source.substr(uiStart);
        check(eventHandler.find("VisibleServerGroupSlots(") == std::string::npos &&
            eventHandler.find("m_nVisibleGroupSlots[idwEvent]") != std::string::npos &&
            eventHandler.find("m_nVisibleGroupSlots[selectedGroup]") != std::string::npos &&
            uiBuilder.find("m_nVisibleGroupSlots[row] = i;") != std::string::npos,
            "group events use the row snapshot built before aggregate endpoint mutation");
        check(uiBuilder.find("52.0f + static_cast<float>(row * 26)") != std::string::npos &&
            uiBuilder.find("52.0f + static_cast<float>(i * 26)") == std::string::npos,
            "group row skins follow compacted list rows, not sparse asset slots");
        const auto fadeStart = source.find("void TMSelectServerScene::SetAlphaServer(");
        const auto fadeEnd = source.find("void TMSelectServerScene::SetAlphaLogin(", fadeStart);
        check(fadeStart != std::string::npos && fadeEnd != std::string::npos,
            "server fade is bounded by the following login fade");
        if (fadeStart != std::string::npos && fadeEnd != std::string::npos) {
            const std::string fade = source.substr(fadeStart, fadeEnd - fadeStart);
            const auto complete = fade.find("else if (dwServerTime - dwStartTime >= dwTerm)");
            const auto partial = fade.find("float fAlpha =", complete);
            check(complete != std::string::npos && partial != std::string::npos &&
                fade.substr(complete, partial - complete).find("for (SPanel* panel : m_pGroupPanel)") != std::string::npos,
                "completed server fade updates every configured group skin");
        }
        check(eventHandler.find("ChannelForVisibleRow(") != std::string::npos &&
            eventHandler.find("int nServerIndex = displayedChannel;") != std::string::npos,
            "channel confirmation resolves the endpoint from its displayed row");
    }

    const std::string field = LoadSource("TMProject748/internal/app/scenes/TMFieldScene.cpp");
    check(AttackTargetCapacity(MSG_Attack_One_Opcode) == 1 &&
        AttackTargetCapacity(MSG_Attack_Two_Opcode) == 2 &&
        AttackTargetCapacity(MSG_Attack_Multi_Opcode) == 13 &&
        AttackTargetCapacity(0) == 0,
        "attack target capacity follows the native opcode prefixes");
    const auto attackStart = field.find("int TMFieldScene::OnPacketAttack(MSG_STANDARD* pStd)");
    const auto attackEnd = field.find("int TMFieldScene::OnPacketNuke(", attackStart);
    check(attackStart != std::string::npos && attackEnd != std::string::npos,
        "attack handler is available for missing-attacker checks");
    if (attackStart != std::string::npos && attackEnd != std::string::npos) {
        const std::string attack = field.substr(attackStart, attackEnd - attackStart);
        check(attack.find("const int targetCount = static_cast<int>(AttackTargetCapacity(pAttack->Header.Type));") != std::string::npos &&
            attack.find("for (int i = 0; i < 13;") == std::string::npos &&
            attack.find("for (int i = 0; i < targetCount;") != std::string::npos,
            "attack damage loops cannot exceed the target entries in a short frame");
        const auto missingAttacker = attack.find("vecAttackerPos = TMVector2((float)pAttack->PosX + 0.5f, (float)pAttack->PosY + 0.5f);");
        const auto targetFilter = attack.find("pAttack->Dam[i].TargetID == m_pMyHuman->m_dwID", missingAttacker);
        const auto request = attack.find("MSG_REQMobByID stReqMobById{};", targetFilter);
        const auto fallbackStart = attack.find("vecStart = TMVector3(vecAttackerPos.x, fY + 1.0f, vecAttackerPos.y);", request);
        check(missingAttacker != std::string::npos && targetFilter != std::string::npos &&
            request != std::string::npos && fallbackStart != std::string::npos &&
            attack.find("pAttack->Dam[i].TargetID = m_pMyHuman->m_dwID") == std::string::npos,
            "missing-attacker lookup reads target IDs without mutating the attack frame");
        check(missingAttacker != std::string::npos && fallbackStart != std::string::npos &&
            missingAttacker < fallbackStart &&
            attack.find("TMVector2 vecAttackerPos{", missingAttacker) == std::string::npos,
            "missing-attacker fallback uses packet position without shadowing it");
    }
    const std::string selectChar = LoadSource("TMProject748/internal/app/scenes/TMSelectCharScene.cpp");
    const auto charLoad = selectChar.find("if (!LoadRC(\"UI\\\\SelCharScene2.txt\"))");
    const auto charSceneSetup = selectChar.find("g_pDevice->m_dwClearColor", charLoad);
    check(charLoad != std::string::npos && charSceneSetup != std::string::npos &&
        selectChar.substr(charLoad, charSceneSetup - charLoad).find("return 0;") != std::string::npos,
        "character selection stops before scene setup when its RC fails");
    const auto fieldLoad = field.find("if (!LoadRC(\"UI\\\\FieldScene2.txt\"))");
    const auto fieldFallback = field.find("InitializeCompatFieldScene();", fieldLoad);
    check(fieldLoad != std::string::npos && fieldFallback != std::string::npos &&
        field.substr(fieldLoad, fieldFallback - fieldLoad).find("return 0;") != std::string::npos,
        "field RC failure cannot enter the valid 7.48 compatibility fallback");
    const auto rebuild = field.find("SListBox* pServerList = m_pServerList;");
    const auto moveEvent = field.find("if (idwControlID == 12289)", rebuild);
    check(rebuild != std::string::npos && moveEvent != std::string::npos,
        "field server switch and row event are available");
    if (rebuild != std::string::npos && moveEvent != std::string::npos) {
        const std::string listBuilder = field.substr(rebuild, moveEvent - rebuild);
        const auto clearRows = listBuilder.find("pServerList->Empty();");
        const auto clearSelection = listBuilder.find("pServerList->SetSelectedIndex(-1);");
        const auto addRows = listBuilder.find("for (int num = 1;; ++num)");
        check(clearRows != std::string::npos && clearSelection != std::string::npos &&
            addRows != std::string::npos && clearRows < clearSelection &&
            clearSelection < addRows,
            "field server switch clears stale selection before adding rows");
        const auto nullGuard = field.find("if (!pItem)", moveEvent);
        const auto itemRead = field.find("if (pItem->m_nCurrent < 500)", moveEvent);
        check(nullGuard != std::string::npos && itemRead != std::string::npos &&
            nullGuard < itemRead,
            "field server switch rejects a missing row before reading it");
    }

    // Channel population is advisory. A failed or full-buffer HTTP read must
    // not leak stale/unterminated text into the selection parser.
    const std::string basedef = LoadSource("TMProject748/internal/core/Basedef.cpp");
    const auto httpStart = basedef.find("int BASE_GetHttpRequest(");
    const auto httpEnd = basedef.find("int BASE_GetWeekNumber()", httpStart);
    check(httpStart != std::string::npos && httpEnd != std::string::npos,
        "channel status HTTP reader is available");
    if (httpStart != std::string::npos && httpEnd != std::string::npos) {
        const std::string reader = basedef.substr(httpStart, httpEnd - httpStart);
        check(reader.find("MaxBuffer <= 0") != std::string::npos &&
            reader.find("Request[0] = '\\0';") != std::string::npos &&
            reader.find("MaxBuffer - 1") != std::string::npos &&
            reader.find("Request[dwBytesRead] = 0;") != std::string::npos &&
            reader.find("if (!readSucceeded)") != std::string::npos,
            "channel status read leaves room for NUL and clears failed results");
        const auto openUrl = reader.find("InternetOpenUrl(");
        const auto connectTimeout = reader.find("INTERNET_OPTION_CONNECT_TIMEOUT");
        const auto receiveTimeout = reader.find("INTERNET_OPTION_RECEIVE_TIMEOUT");
        check(connectTimeout != std::string::npos &&
            receiveTimeout != std::string::npos && openUrl != std::string::npos &&
            connectTimeout < openUrl && receiveTimeout < openUrl,
            "channel status configures connect and receive timeouts before HTTP");
    }

    check(source.find("ParseServerStatus(szUserCount, nUserCount, 11);") != std::string::npos &&
        source.find("ParseServerStatus(szUserCount, nUserCount2, 11);") != std::string::npos &&
        source.find("ParseServerStatus(szUserCount, statusValues, 12);") != std::string::npos &&
        field.find("ParseServerStatus(szUserCount, nUserCount2, 11);") != std::string::npos &&
        field.find("ParseServerStatus(szUserCount, nUserCount, 10);") != std::string::npos,
        "selection and field scenes use the same status parser");

    int counts[4]{-1, -1, -1, -1};
    check(ParseServerStatus("12\n34\r\n56\n78\n", counts, 4) == 4 &&
        counts[0] == 12 && counts[1] == 34 && counts[2] == 56 && counts[3] == 78,
        "status parser accepts multiline and CRLF feeds");
    std::fill(std::begin(counts), std::end(counts), -1);
    check(ParseServerStatus("12\\n34\\n56\\n78\\n", counts, 4) == 4 &&
        counts[0] == 12 && counts[1] == 34 && counts[2] == 56 && counts[3] == 78,
        "status parser preserves legacy literal separators");
    std::fill(std::begin(counts), std::end(counts), -1);
    check(ParseServerStatus("12\n34x56\n", counts, 4) == 2 &&
        counts[0] == 12 && counts[1] == 34 && counts[2] == -1,
        "malformed status cannot overwrite unread channels");
    std::fill(std::begin(counts), std::end(counts), -1);
    check(ParseServerStatus("12\n999999999999999999999\n", counts, 4) == 1 &&
        counts[0] == 12 && counts[1] == -1,
        "overflowing population is rejected");

    const auto listStart = basedef.find("int BASE_InitializeServerList()");
    const auto listEnd = basedef.find("int BASE_GetLanguage()", listStart);
    check(listStart != std::string::npos && listEnd != std::string::npos,
        "serverlist loader is available");
    if (listStart != std::string::npos && listEnd != std::string::npos) {
        const std::string loader = basedef.substr(listStart, listEnd - listStart);
        check(loader.find("WYD748_OpenServerListAsset(\"./serverlist.local.bin\", \"./serverlist.bin\")") != std::string::npos,
            "local serverlist takes precedence over the versioned table");
        const auto read = loader.find("WYD748_ReadServerList(fpBin, g_pServerList)");
        const auto reject = loader.find("if (!loaded)", read);
        const auto decode = loader.find("g_pServerList[k][j][i] -=", reject);
        check(read != std::string::npos && reject != std::string::npos &&
            decode != std::string::npos && read < reject && reject < decode,
            "serverlist must be complete before endpoint decoding");
    }

    char servers[2][2][64];
    std::memset(servers, 0x5a, sizeof servers);
    check(!WYD748_ReadServerList(nullptr, servers),
        "missing serverlist is rejected");
    char zeros[sizeof servers]{};
    check(std::memcmp(servers, zeros, sizeof servers) == 0,
        "missing serverlist clears stale endpoints");

    const auto fixtureDirectory = std::filesystem::temp_directory_path() /
        ("wyd-serverlist-" + std::to_string(GetCurrentProcessId()) + "-" +
            std::to_string(GetTickCount64()));
    const bool createdFixtureDirectory = std::filesystem::create_directory(fixtureDirectory);
    check(createdFixtureDirectory, "local serverlist fixture directory is created");
    if (createdFixtureDirectory) {
        const auto localPath = fixtureDirectory / "serverlist.local.bin";
        const auto standardPath = fixtureDirectory / "serverlist.bin";
        {
            std::ofstream standard(standardPath, std::ios::binary);
            standard << std::string(sizeof servers, 'S');
        }
        {
            std::ofstream local(localPath, std::ios::binary);
            local << 'L';
        }
        std::FILE* selected = WYD748_OpenServerListAsset(localPath.string().c_str(),
            standardPath.string().c_str());
        check(selected && !WYD748_ReadServerList(selected, servers),
            "truncated local table fails closed instead of using the standard table");
        if (selected) std::fclose(selected);
        {
            std::ofstream local(localPath, std::ios::binary | std::ios::trunc);
            local << std::string(sizeof servers, 'L');
        }
        selected = WYD748_OpenServerListAsset(localPath.string().c_str(),
            standardPath.string().c_str());
        check(selected && WYD748_ReadServerList(selected, servers) && servers[0][0][0] == 'L',
            "complete local table overrides the standard table");
        if (selected) std::fclose(selected);
        HANDLE lockedLocal = CreateFileW(localPath.c_str(), GENERIC_READ, 0, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        check(lockedLocal != INVALID_HANDLE_VALUE, "local serverlist fixture can be locked");
        if (lockedLocal != INVALID_HANDLE_VALUE) {
            selected = WYD748_OpenServerListAsset(localPath.string().c_str(),
                standardPath.string().c_str());
            check(selected == nullptr,
                "unreadable local table does not fall back to the standard endpoint");
            if (selected) std::fclose(selected);
            CloseHandle(lockedLocal);
        }
        std::filesystem::remove(localPath);
        selected = WYD748_OpenServerListAsset(localPath.string().c_str(),
            standardPath.string().c_str());
        check(selected && WYD748_ReadServerList(selected, servers) && servers[0][0][0] == 'S',
            "standard table is used when no local override exists");
        if (selected) std::fclose(selected);
        std::filesystem::remove(standardPath);
        std::filesystem::remove(fixtureDirectory);
    }

    char payload[sizeof servers];
    std::memset(payload, 0x35, sizeof payload);
    std::FILE* complete = nullptr;
    tmpfile_s(&complete);
    check(complete != nullptr, "complete serverlist fixture opens");
    if (complete) {
        const bool written = std::fwrite(payload, 1, sizeof payload, complete) == sizeof payload;
        std::rewind(complete);
        check(written && WYD748_ReadServerList(complete, servers) &&
            std::memcmp(servers, payload, sizeof servers) == 0,
            "complete serverlist remains unchanged before decoding");
        std::fclose(complete);
    }

    std::FILE* truncated = nullptr;
    tmpfile_s(&truncated);
    check(truncated != nullptr, "truncated serverlist fixture opens");
    if (truncated) {
        const bool written = std::fwrite(payload, 1, sizeof payload - 1, truncated) == sizeof payload - 1;
        std::rewind(truncated);
        check(written && !WYD748_ReadServerList(truncated, servers),
            "truncated serverlist is rejected");
        check(std::memcmp(servers, zeros, sizeof servers) == 0,
            "truncated serverlist clears partial endpoints");
        std::fclose(truncated);
    }

    std::FILE* oversized = nullptr;
    tmpfile_s(&oversized);
    check(oversized != nullptr, "oversized serverlist fixture opens");
    if (oversized) {
        const bool written = std::fwrite(payload, 1, sizeof payload, oversized) == sizeof payload &&
            std::fputc('X', oversized) != EOF;
        std::rewind(oversized);
        check(written && !WYD748_ReadServerList(oversized, servers),
            "oversized serverlist is rejected");
        check(std::memcmp(servers, zeros, sizeof servers) == 0,
            "oversized serverlist clears partial endpoints");
        std::fclose(oversized);
    }

    unsigned char shortRecord[28]{};
    unsigned char scaledRecord[36]{};
    const std::uint32_t scaledType = 520;
    std::memcpy(scaledRecord, &scaledType, sizeof scaledType);
    check(ValidateObjectFileRecords(shortRecord, sizeof shortRecord, 1, 10, 6) &&
        !ValidateObjectFileRecords(shortRecord, sizeof shortRecord - 1, 1, 10, 6) &&
        !ValidateObjectFileRecords(shortRecord, sizeof shortRecord, 0, 10, 6),
        "base object record requires all 28 bytes and an available slot");
    check(ValidateObjectFileRecords(scaledRecord, sizeof scaledRecord, 1, 10, 6) &&
        !ValidateObjectFileRecords(scaledRecord, sizeof scaledRecord - 1, 1, 10, 6),
        "scaled object record requires all 36 bytes");
    check(ObjectFileRecordSize(506) == 36 && ObjectFileRecordSize(507) == 28 &&
        ObjectFileRecordSize(518) == 36 && ObjectFileRecordSize(519) == 28 &&
        ObjectFileRecordSize(599) == 36 && ObjectFileRecordSize(600) == 28,
        "object record boundaries match the scene branches");
    check(ObjectFileSpatialKey(64, 96) == 0x00030002u &&
        ObjectFileSpatialKey(31, 31) == 0u &&
        ObjectFileSpatialKey(32, 32) == 0x00010001u,
        "field object spatial keys retain their legacy grid lanes");

    const auto ordinaryPositionValid = [&](float x, float y, float height,
        float offsetX = 0.0f, float offsetY = 0.0f) {
        std::memcpy(shortRecord + 4, &x, sizeof x);
        std::memcpy(shortRecord + 8, &y, sizeof y);
        std::memcpy(shortRecord + 12, &height, sizeof height);
        return ValidateObjectFileRecords(shortRecord, sizeof shortRecord, 1, 10, 6,
            offsetX, offsetY);
    };
    check(ordinaryPositionValid(-0.6f, 127.3f, -12.5f, 128.0f, 256.0f) &&
        ordinaryPositionValid(-2147483648.0f, 0.0f, 0.0f),
        "ordinary field records retain defined fractional and integer coordinates");
    check(!ordinaryPositionValid(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f) &&
        !ordinaryPositionValid(0.0f, std::numeric_limits<float>::infinity(), 0.0f) &&
        !ordinaryPositionValid(0.0f, 0.0f, std::numeric_limits<float>::quiet_NaN()) &&
        !ordinaryPositionValid(2147483648.0f, 0.0f, 0.0f) &&
        !ordinaryPositionValid(2147483520.0f, 0.0f, 0.0f, 128.0f),
        "malformed field records cannot reach undefined integer conversions");

    std::vector<unsigned char> seaRecords(11 * 28);
    const std::uint32_t seaType = 2;
    for (std::size_t i = 0; i < 11; ++i)
        std::memcpy(seaRecords.data() + i * 28, &seaType, sizeof seaType);
    check(ValidateObjectFileRecords(seaRecords.data(), 10 * 28, 20, 10, 6) &&
        !ValidateObjectFileRecords(seaRecords.data(), seaRecords.size(), 20, 10, 6),
        "a field may fill but not overflow its ten sea slots");

    std::vector<unsigned char> lightRecords(7 * 36);
    const std::uint32_t lightType = 511;
    for (std::size_t i = 0; i < 7; ++i)
        std::memcpy(lightRecords.data() + i * 36, &lightType, sizeof lightType);
    check(ValidateObjectFileRecords(lightRecords.data(), 6 * 36, 20, 10, 6) &&
        !ValidateObjectFileRecords(lightRecords.data(), lightRecords.size(), 20, 10, 6),
        "a field may fill but not overflow its six light slots");

    unsigned char leafRecord[28]{};
    const std::uint32_t leafType = 312;
    std::memcpy(leafRecord, &leafType, sizeof leafType);
    const auto leafPositionValid = [&](float x, float y) {
        std::memcpy(leafRecord + 4, &x, sizeof x);
        std::memcpy(leafRecord + 8, &y, sizeof y);
        return ValidateObjectFileRecords(leafRecord, sizeof leafRecord, 1, 10, 6);
    };
    check(leafPositionValid(126.5f, -0.6f) && leafPositionValid(10.0f, 127.3f),
        "native fractional leaf coordinates retain their truncated mask indices");
    check(!leafPositionValid(-1.0f, 0.0f) && !leafPositionValid(0.0f, -1.0f) &&
        !leafPositionValid(std::numeric_limits<float>::quiet_NaN(), 0.0f) &&
        !leafPositionValid(0.0f, std::numeric_limits<float>::infinity()) &&
        !leafPositionValid(2147483648.0f, 0.0f),
        "leaf records cannot produce negative or undefined mask indices");

    const auto objectLoader = LoadSource(
        "TMProject748/internal/render/world/objects/TMObjectContainer.cpp");
    check(objectLoader.find("ValidateObjectFileRecords(buff.get(), sz, MAX_OBJECT_LIST,") != std::string::npos &&
        objectLoader.find("std::extent_v<decltype(TMGround::m_pSeaList)>, MAX_LIGHT_CONTAINER") != std::string::npos,
        "field loader checks the real sea and light array capacities before constructing objects");

    const auto sample = FindSource("client748/Env/Field1616.dat");
    check(!sample.empty(), "7.48 field object assets are available");
    if (!sample.empty()) {
        std::size_t fieldCount = 0;
        bool allValid = true;
        for (const auto& entry : std::filesystem::directory_iterator(sample.parent_path())) {
            const auto name = entry.path().filename().string();
            if (!entry.is_regular_file() || entry.path().extension() != ".dat" ||
                name.rfind("Field", 0) != 0)
                continue;
            std::ifstream input(entry.path(), std::ios::binary);
            const std::vector<unsigned char> data{
                std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
            const bool valid = !input.bad() &&
                ValidateObjectFileRecords(data.data(), data.size(), 4096, 10, 6);
            if (!valid)
                std::fprintf(stderr, "invalid field object asset: %s (%zu bytes)\n",
                    name.c_str(), data.size());
            allValid = allValid && valid;
            ++fieldCount;
        }
        check(fieldCount == 96 && allValid,
            "all 96 native 7.48 Field object files retain their record layout");
    }

    const auto sampleAsset = FindSource("client748/UI/selchar.txt");
    check(!sampleAsset.empty(), "7.48 character preview samples are available");
    if (!sampleAsset.empty()) {
        WYD748CharacterSample samples[4]{};
        const auto samplePath = sampleAsset.string();
        check(WYD748_LoadCharacterSamples(samplePath.c_str(), samples, 4),
            "all four native character previews load");
        check(samples[0].face == 1 && samples[0].helm == 1211 &&
            samples[0].left == 2891 && samples[3].face == 31 &&
            samples[3].body == 1646 && samples[3].refinement == 9,
            "native preview item indexes and refinement are preserved");
        check(!WYD748_LoadCharacterSamples(samplePath.c_str(), samples, 3) &&
            !WYD748_LoadCharacterSamples(nullptr, samples, 4),
            "missing path and undersized preview array are rejected");

        wchar_t executable[MAX_PATH]{};
        const DWORD executableLength = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        check(executableLength > 0 && executableLength < MAX_PATH,
            "test executable path is available for preview fixtures");
        const auto fixturePath = std::filesystem::path(executable).parent_path() /
            "selchar-invalid-test.txt";
        const auto writeFixture = [&](const char* content) {
            std::ofstream output(fixturePath, std::ios::trunc);
            output << content;
            output.close();
            return output.good();
        };
        const auto invalidPath = fixturePath.string();
        const auto checkRejected = [&](const char* content, const char* name) {
            check(writeFixture(content), "invalid preview fixture can be written");
            samples[0].face = 777;
            check(!WYD748_LoadCharacterSamples(invalidPath.c_str(), samples, 4) &&
                samples[0].face == 777, name);
        };
        checkRejected("1,1211,1211,0,0,2891,9\n",
            "truncated preview table leaves the destination untouched");
        checkRejected("1,1211,1211,0,0,2891,9\n11,1346,no-item,0,0,2848,9\n21,1496,1496,0,0,2669,9\n31,1643,1646,0,0,2550,9\n",
            "malformed preview item is rejected before lookup");
        checkRejected("1,6500,1211,0,0,2891,9\n11,1346,1346,0,0,2848,9\n21,1496,1496,0,0,2669,9\n31,1643,1646,0,0,2550,9\n",
            "out-of-range preview item is rejected before lookup");
        checkRejected("1,1211,1211,0,0,2891,9\n11,1346,1346,0,0,2848,9\n21,1496,1496,0,0,2669,9\n31,1643,1646,0,0,2550,9 trailing\n",
            "trailing preview fields are rejected");
        std::error_code error;
        std::filesystem::remove(fixturePath, error);
        check(!error, "temporary preview fixture is removed");
    }

    const auto effectAsset = FindSource("client748/UI/EffectString.txt");
    check(!effectAsset.empty(), "7.48 effect name asset is available");
    if (!effectAsset.empty()) {
        char names[50][24]{};
        const auto effectPath = effectAsset.string();
        check(WYD748_LoadEffectStrings(effectPath.c_str(), &names[0][0], 50, 24, 1),
            "native effect names load into the indexed fixed-width table");
        check(std::strcmp(names[0], "") == 0 &&
            std::strcmp(names[1], "Slow") == 0 &&
            std::strcmp(names[39], "Exp") == 0 &&
            std::strcmp(names[40], "") == 0,
            "native effect indexes and absent later rows remain unchanged");

        wchar_t executable[MAX_PATH]{};
        const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        check(length > 0 && length < MAX_PATH,
            "test executable path is available for effect fixtures");
        if (length > 0 && length < MAX_PATH) {
            const auto fixturePath = std::filesystem::path(executable).parent_path() /
                "effect-string-invalid-test.txt";
            const auto fixtureName = fixturePath.string();
            const auto rejected = [&](const char* content, const char* label) {
                std::ofstream output(fixturePath, std::ios::trunc);
                output << content;
                output.close();
                names[1][0] = 'X';
                check(output.good() &&
                    !WYD748_LoadEffectStrings(fixtureName.c_str(), &names[0][0], 50, 24, 1) &&
                    names[1][0] == 'X', label);
            };
            rejected("123456789012345678901234\n",
                "overlong effect name is rejected without publishing partial data");
            std::string tooMany;
            for (int index = 0; index < 50; ++index)
                tooMany += "Name\n";
            rejected(tooMany.c_str(),
                "effect table overflow is rejected without publishing partial data");
            std::error_code error;
            std::filesystem::remove(fixturePath, error);
            check(!error, "temporary effect fixture is removed");
        }
        check(!WYD748_LoadEffectStrings(nullptr, &names[0][0], 50, 24, 1) &&
            !WYD748_LoadEffectStrings(effectPath.c_str(), &names[0][0], 1, 24, 1),
            "missing path and out-of-range effect start index are rejected");
    }

    const auto uiStringAsset = FindSource("client748/UI/UIString.txt");
    check(!uiStringAsset.empty(), "7.48 English UI string asset is available");
    if (!uiStringAsset.empty()) {
        char labels[500][64]{};
        const auto assetPath = uiStringAsset.string();
        check(WYD748_LoadUIStrings(assetPath.c_str(), &labels[0][0], 500, 64),
            "native indexed UI strings load into the bounded table");
        check(std::strcmp(labels[1], "Connect") == 0 &&
            std::strcmp(labels[226], "C.C Mode(MC)") == 0 &&
            std::strcmp(labels[227], "C.C Mode(SC)") == 0 &&
            std::strcmp(labels[228], "Lv") == 0 && labels[229][0] == '\0',
            "UI string indexes and embedded spaces survive parsing");

        wchar_t executable[MAX_PATH]{};
        const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        check(length > 0 && length < MAX_PATH,
            "UI string fixture directory is available");
        if (length > 0 && length < MAX_PATH) {
            const auto fixturePath = std::filesystem::path(executable).parent_path() /
                "ui-string-invalid-test.txt";
            const auto fixtureName = fixturePath.string();
            const auto rejected = [&](const std::string& content, const char* label) {
                std::ofstream output(fixturePath, std::ios::trunc);
                output << content;
                output.close();
                labels[1][0] = 'X';
                check(output.good() &&
                    !WYD748_LoadUIStrings(fixtureName.c_str(), &labels[0][0], 500, 64) &&
                    labels[1][0] == 'X', label);
            };
            rejected("1\tConnect\n500\tOutOfRange\n",
                "out-of-range UI index is rejected without partial publication");
            rejected("1\tConnect\n1\tDuplicate\n",
                "duplicate UI index is rejected without partial publication");
            rejected("1\tConnect\n2\t" + std::string(64, 'A') + "\n",
                "overlong UI label is rejected without partial publication");
            rejected("1\tConnect\n2\t\n",
                "missing UI label is rejected without partial publication");
            std::error_code error;
            std::filesystem::remove(fixturePath, error);
            check(!error, "temporary UI string fixture is removed");
        }
        check(!WYD748_LoadUIStrings(nullptr, &labels[0][0], 500, 64) &&
            !WYD748_LoadUIStrings(assetPath.c_str(), nullptr, 500, 64) &&
            !WYD748_LoadUIStrings(assetPath.c_str(), &labels[0][0], 1, 64) &&
            !WYD748_LoadUIStrings(assetPath.c_str(), &labels[0][0], 500, 1),
            "invalid UI string destinations and dimensions are rejected");
    }

    const auto itemNameAsset = FindSource("client748/Itemname.bin");
    check(!itemNameAsset.empty(), "7.48 item-name asset is available");
    if (!itemNameAsset.empty()) {
        constexpr std::size_t itemCount = 6500;
        constexpr std::size_t itemStride = 156;
        std::vector<char> items(itemCount * itemStride, 0);
        items[itemStride + 64] = 'K';
        const auto assetPath = itemNameAsset.string();
        check(WYD748_LoadItemNames(assetPath.c_str(), items.data(),
            itemCount, itemStride, 64),
            "native item-name records load into the 7.48 item table");
        check(std::strcmp(items.data() + itemStride, "Transknight") == 0 &&
            std::strcmp(items.data() + 6 * itemStride, "TransKnight") == 0 &&
            std::strcmp(items.data() + 5338 * itemStride, "Ideal_Stone") == 0 &&
            items[itemStride + 64] == 'K' && items[5339 * itemStride] == '\0',
            "item-name indexes, decoding, and non-name fields are preserved");

        wchar_t executable[MAX_PATH]{};
        const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        check(length > 0 && length < MAX_PATH,
            "item-name fixture directory is available");
        if (length > 0 && length < MAX_PATH) {
            const auto fixturePath = std::filesystem::path(executable).parent_path() /
                "item-name-invalid-test.bin";
            const auto fixtureName = fixturePath.string();
            const auto encodeNameRecord = [](std::int32_t index, const char* name) {
                std::string bytes(68, '\0');
                std::memcpy(bytes.data(), &index, sizeof(index));
                for (std::size_t position = 0; position < 62 && name[position] != '\0'; ++position)
                    bytes[sizeof(index) + position] = static_cast<char>(
                        static_cast<unsigned char>(name[position] + position));
                return bytes;
            };
            const auto rejected = [&](const std::string& content, const char* label) {
                std::ofstream output(fixturePath, std::ios::binary | std::ios::trunc);
                output.write(content.data(), content.size());
                output.close();
                items[itemStride] = 'X';
                check(output.good() && !WYD748_LoadItemNames(fixtureName.c_str(),
                    items.data(), itemCount, itemStride, 64) &&
                    items[itemStride] == 'X' && items[itemStride + 64] == 'K', label);
            };
            rejected(encodeNameRecord(1, "Valid") + encodeNameRecord(-2, "Invalid"),
                "negative item index is rejected without partial publication");
            rejected(encodeNameRecord(1, "Valid") + encodeNameRecord(6500, "Invalid"),
                "out-of-range item index is rejected without partial publication");
            rejected(encodeNameRecord(1, "Valid") + encodeNameRecord(1, "Duplicate"),
                "duplicate item index is rejected without partial publication");
            rejected(encodeNameRecord(1, "Valid") + "tail",
                "truncated item-name record is rejected without partial publication");
            std::error_code error;
            std::filesystem::remove(fixturePath, error);
            check(!error, "temporary item-name fixture is removed");
        }
        check(!WYD748_LoadItemNames(nullptr, items.data(), itemCount, itemStride, 64) &&
            !WYD748_LoadItemNames(assetPath.c_str(), nullptr, itemCount, itemStride, 64) &&
            !WYD748_LoadItemNames(assetPath.c_str(), items.data(), itemCount, 63, 64) &&
            !WYD748_LoadItemNames(assetPath.c_str(), items.data(), itemCount, itemStride, 63) &&
            !WYD748_LoadItemNames(assetPath.c_str(), items.data(), itemCount, itemStride, 65),
            "invalid item-name destinations and dimensions are rejected");
    }

    const auto startupSource = LoadSource("TMProject748/internal/app/scenes/NewApp.cpp");
    const auto basedefSource = LoadSource("TMProject748/internal/core/Basedef.cpp");
    const auto receiveLoop = startupSource.find("ReadPacketView(&ErrorCode, &ErrorType)");
    const auto readError = startupSource.find("if (ErrorCode != 0)", receiveLoop);
    const auto closeOnError = startupSource.find("m_pSocketManager->CloseSocket();", readError);
    const auto notifyOnError = startupSource.find("m_pObjectManager->OnPacketEvent(0, nullptr);", closeOnError);
    const auto dispatchCheck = startupSource.find("if (!packet_dispatch::CanDispatch(packet", readError);
    check(receiveLoop != std::string::npos && readError != std::string::npos &&
        closeOnError != std::string::npos && notifyOnError != std::string::npos &&
        dispatchCheck != std::string::npos &&
        receiveLoop < readError && readError < closeOnError &&
        closeOnError < notifyOnError && notifyOnError < dispatchCheck,
        "protocol errors close the socket and notify the scene before dispatch");
    check(startupSource.find("ReadItemName();") != std::string::npos &&
        startupSource.find("ReadUIString();") != std::string::npos &&
        startupSource.find("ReadItemicon(") == std::string::npos &&
        basedefSource.find("g_itemicon") == std::string::npos,
        "7.48 startup keeps required item labels without the unused item-icon table");

    const auto readAsset = [](const char* path) {
        const auto found = FindSource(path);
        if (found.empty())
            return std::vector<unsigned char>{};
        std::ifstream input(found, std::ios::binary);
        return std::vector<unsigned char>(std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>());
    };
    const auto clientAttribute = readAsset("client748/Env/AttributeMap.dat");
    const auto serverAttribute = readAsset("wydgo748/data/maps/AttributeMap.dat");
    check(clientAttribute.size() == 1048580 && serverAttribute.size() == 1048576 &&
        std::equal(serverAttribute.begin(), serverAttribute.end(), clientAttribute.begin()),
        "the server consumes the complete 7.48 client attribute payload");
    const auto clientHeight = readAsset("client748/Env/HeightMap.dat");
    const auto serverHeight = readAsset("wydgo748/data/maps/HeightMap.dat");
    check(clientHeight.size() == 16777216 && clientHeight == serverHeight,
        "the client and server load an identical world height map");
    check(startupSource.find("!BASE_InitializeHeightMap()") != std::string::npos &&
        basedefSource.find("sharedHeightMap[worldY * SharedHeightMapWidth + worldX]") != std::string::npos &&
        basedefSource.find("BASE_ApplyAttribute(char* pHeight, int size)") != std::string::npos,
        "client startup requires the shared height map before route masks are projected");

    const auto ccModeScene = LoadSource("TMProject748/internal/app/scenes/TMFieldScene.cpp");
    check(ccModeScene.find("{\"Off\", \"Physical\", \"Magic\", \"Support\"}") != std::string::npos &&
        ccModeScene.find("{\"Free\", \"Cycle\", \"Fixed\"}") != std::string::npos,
        "7.48 compact combat controls use English labels");

    return failures;
}
