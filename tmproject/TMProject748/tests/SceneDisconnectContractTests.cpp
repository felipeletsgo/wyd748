#include "../internal/ui/UIBinary.h"
#include "../internal/core/ServerListAsset.h"
#include "../internal/core/ServerStatus.h"
#include "../internal/core/WYD748Assets.h"
#include "../internal/render/world/objects/ObjectFileRecordLayout.h"
#include "../internal/ui/SellConfirmationText.h"

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
    const auto splitAmount = fieldSource.find("BASE_GetItemAmount(SGridControl::m_pSellItem->m_pItem)", splitStart);
    const auto splitGuard = fieldSource.find("if (!SGridControl::m_pSellItem ||", splitStart);
    check(splitStart != std::string::npos && splitGuard != std::string::npos &&
        splitAmount != std::string::npos && splitStart < splitGuard && splitGuard < splitAmount &&
        fieldSource.substr(splitGuard, splitAmount - splitGuard).find("!SGridControl::m_pSellItem->m_pGridControl") != std::string::npos,
        "split dialog does not dereference an item invalidated by swap cleanup");
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
    const auto attackStart = field.find("int TMFieldScene::OnPacketAttack(MSG_STANDARD* pStd)");
    const auto attackEnd = field.find("int TMFieldScene::OnPacketNuke(", attackStart);
    check(attackStart != std::string::npos && attackEnd != std::string::npos,
        "attack handler is available for missing-attacker checks");
    if (attackStart != std::string::npos && attackEnd != std::string::npos) {
        const std::string attack = field.substr(attackStart, attackEnd - attackStart);
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
    check(startupSource.find("ReadItemName();") != std::string::npos &&
        startupSource.find("ReadUIString();") != std::string::npos &&
        startupSource.find("ReadItemicon(") == std::string::npos &&
        basedefSource.find("g_itemicon") == std::string::npos,
        "7.48 startup keeps required item labels without the unused item-icon table");

    const auto ccModeScene = LoadSource("TMProject748/internal/app/scenes/TMFieldScene.cpp");
    check(ccModeScene.find("{\"Off\", \"Physical\", \"Magic\", \"Support\"}") != std::string::npos &&
        ccModeScene.find("{\"Free\", \"Cycle\", \"Fixed\"}") != std::string::npos,
        "7.48 compact combat controls use English labels");

    return failures;
}
