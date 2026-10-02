#include "pch.h"
#include "TMFieldScene.h"
#include "ClientDiagnostics.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMItem.h"
#include "TMObjectContainer.h"
#include "TMUtil.h"
#include "ItemEffect.h"
#include "TMSkinMesh.h"
#include "WYD748Assets.h"
#include "../../application/FieldInteractionPolicy.h"
#include "../../game/entities/AppearanceRefinementRefresh.h"

void TMFieldScene::InitializeCompatInventory()
{
	// Ghidra FUN_00435b13 proves that FieldScene2.bin already owns the native
	// SGridControl objects.  Bind those objects directly: creating overlay grids
	// duplicates hit-test surfaces, hides 3D items and corrupts close/drag events.
	if (!m_pControlContainer)
		return;

	auto findGrid = [this](unsigned int controlID)
	{
		return static_cast<SGridControl*>(m_pControlContainer->FindControl(controlID));
	};
	m_pGridInv = findGrid(TMG_INV_GRID);
	m_pGridInvList[0] = m_pGridInv;
	m_pGridHelm = findGrid(TMG_HELMET_GRID);
	m_pGridRight = findGrid(TMG_RIGHT_GRID);
	m_pGridLeft = findGrid(TMG_LEFT_GRID);
	m_pGridCoat = findGrid(TMG_COAT_GRID);
	m_pGridPants = findGrid(TMG_PANTS_GRID);
	m_pGridGloves = findGrid(TMG_GLOVES_GRID);
	m_pGridBoots = findGrid(TMG_BOOTS_GRID);
	m_pGridGuild = findGrid(TMG_GUILD_GRID);
	m_pGridEvent = findGrid(TMG_EVENT_GRID);
	m_pGridRing = findGrid(TMG_RING_GRID);
	m_pGridOrb = findGrid(TMG_ORB_GRID);
	m_pGridCabuncle = findGrid(TMG_CABUNCLE_GRID);
	m_pGridDRing = findGrid(TMG_DRING_GRID);
	m_pGridMantua = findGrid(TMG_MANTUA_GRID);
	// The emulator's 7.48 contract has no Necklace equipment or the NewSlot
	// entries used by later versions. Clearing and hiding inherited controls
	// prevents a modern item from surviving in shared root 257.
	auto disableUnsupportedEquipGrid = [this](unsigned int controlID,
		SGridControl*& member)
	{
		auto existing = member;
		if (existing)
		{
			existing->Empty();
			existing->m_bSelectEnable = 0;
			existing->SetVisible(0);
		}
		auto grid = static_cast<SGridControl*>(m_pControlContainer->FindControl(controlID));
		if (grid && grid != existing)
		{
			grid->Empty();
			grid->m_bSelectEnable = 0;
			grid->SetVisible(0);
		}
		member = nullptr;
	};
	disableUnsupportedEquipGrid(TMG_NECKLACE_GRID, m_pGridNecklace);
	disableUnsupportedEquipGrid(1048976u, m_pGridNewSlot1);
	disableUnsupportedEquipGrid(1048977u, m_pGridNewSlot2);
	m_pGridShop = findGrid(TMG_SHOP_GRID);
	m_pCargoGrid = findGrid(TMG_CARGO_GRID);
	m_pCargoGridList[0] = m_pCargoGrid;
	m_pGridSkillMaster = findGrid(TMG_SKILLM_GRID);
	int autoTradeGridCount = 0;
	for (int slot = 0; slot < 12; ++slot)
	{
		// Ghidra FUN_00435b13 binds all native auto-trade controls 653..664;
		// keeping the twelve pointers contiguous is also the packet slot contract.
		m_pGridAutoTrade[slot] = findGrid(TMG_ATRADE_MY1 + slot);
		if (m_pGridAutoTrade[slot])
		{
			m_pGridAutoTrade[slot]->m_bSelectEnable = 1;
			m_pGridAutoTrade[slot]->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
			++autoTradeGridCount;
		}
		else
			WYD748_DiagnosticsLog("compat auto-trade grid missing id=%u\r\n", TMG_ATRADE_MY1 + slot);
	}
	WYD748_DiagnosticsLog("compat auto-trade grids bound=%d/12\r\n", autoTradeGridCount);

	if (!m_pGridInv || !m_pGridShop || !m_pCargoGrid || !m_pGridSkillMaster)
	{
		WYD748_DiagnosticsLog("compat native grid binding failed inv=%p shop=%p cargo=%p skill=%p\r\n",
			m_pGridInv, m_pGridShop, m_pCargoGrid, m_pGridSkillMaster);
		return;
	}

	// These flags reproduce native FUN_00435b13: the resource determines grid
	// geometry while code supplies interaction semantics for each gameplay area.
	m_pGridInv->m_bSelectEnable = 1;
	m_pGridShop->m_bSelectEnable = 1;
	m_pGridShop->m_eGridType = TMEGRIDTYPE::GRID_SHOP;
	m_pCargoGrid->m_bSelectEnable = 1;
	m_pCargoGrid->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
	m_pGridSkillMaster->m_bSelectEnable = 1;
	m_pGridSkillMaster->m_eGridType = TMEGRIDTYPE::GRID_SKILLM;
	if (m_pGridMantua)
		m_pGridMantua->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;

	// SGridControlItem owns its STRUCT_ITEM copy and derives its dimensions from
	// the normalized table.  This preserves the 3D renderer while enforcing the
	// source-owned 7.48 contract in which every inventory icon is 1x1.
	auto populateGridItem = [](SGridControl* grid, const STRUCT_ITEM& source, int x, int y)
	{
		if (!grid || source.sIndex <= 40)
			return;
		auto itemCopy = new STRUCT_ITEM;
		memcpy(itemCopy, &source, sizeof(STRUCT_ITEM));
		auto gridItem = new SGridControlItem(grid, itemCopy, 0.0f, 0.0f);
		if (!gridItem)
		{
			delete itemCopy;
			return;
		}
		// The grid assumes ownership only after insertion succeeds; the destructor
		// also releases the STRUCT_ITEM copy when the list rejects the visual.
		if (!grid->AddItem(gridItem, x, y))
		{
			delete gridItem;
		}
	};

	m_pGridInv->Empty();
	for (int slot = 0; slot < MAX_VISIBLE_CARRY; ++slot)
		populateGridItem(m_pGridInv, g_pObjectManager->m_stMobData.Carry[slot], slot % 9, slot / 9);

	struct EquipBinding
	{
		SGridControl* grid;
		int slot;
	};
	const EquipBinding equipment[] = {
		{ m_pGridHelm, 1 }, { m_pGridCoat, 2 }, { m_pGridPants, 3 },
		{ m_pGridGloves, 4 }, { m_pGridBoots, 5 }, { m_pGridLeft, 6 },
		{ m_pGridRight, 7 }, { m_pGridRing, 8 },
		{ m_pGridOrb, 10 }, { m_pGridCabuncle, 11 }, { m_pGridGuild, 12 },
		{ m_pGridEvent, 13 }, { m_pGridDRing, 14 }, { m_pGridMantua, 15 }
	};
	for (const auto& binding : equipment)
	{
		if (!binding.grid)
			continue;
		binding.grid->Empty();
		binding.grid->m_bSelectEnable = 1;
		populateGridItem(binding.grid, g_pObjectManager->m_stMobData.Equip[binding.slot], 0, 0);
	}

	m_pCargoGrid->Empty();
	for (int slot = 0; slot < 120; ++slot)
		populateGridItem(m_pCargoGrid, g_pObjectManager->m_stItemCargo[slot], slot % 9, slot / 9);

	// Merchant content is populated only after an NPC response; clearing the
	// already-serialized grid prevents stale items from a previous scene.
	m_pGridShop->Empty();
	WYD748_DiagnosticsLog("compat native grids bound invItems=%d cargoItems=%d\r\n",
		m_pGridInv->m_nNumItem, m_pCargoGrid->m_nNumItem);
}

SGridControl* TMFieldScene::GetCarryGridForSlot(int slot) const
{
	// WYD 7.48 owns one 9x7 Carry control. Returning a page selected by a newer
	// resource would address a different UI ABI and corrupt drag targets.
	// Slot 63 exists in the wire array, but has no cell in the 9x7 control.
	if (slot < 0 || slot >= MAX_VISIBLE_CARRY)
		return nullptr;
	return m_pGridInv;
}

SGridControl* TMFieldScene::GetCargoGridForSlot(int slot) const
{
	// WYD 7.48 owns one nine-column Cargo control; no page selector participates
	// in packet slot translation for this executable.
	if (slot < 0 || slot >= 120)
		return nullptr;
	return m_pCargoGrid;
}

void TMFieldScene::GetCarryCellForSlot(int slot, int& cellX, int& cellY) const
{
	// Ghidra FUN_0052a737 proves the only supported Carry transform is row-major
	// x=slot%9, y=slot/9 for the 7.48 FieldScene2 resource.
	cellX = slot % 9;
	cellY = slot / 9;
}

void TMFieldScene::GetCargoCellForSlot(int slot, int& cellX, int& cellY) const
{
	// Ghidra FUN_0052a737 uses one nine-column cargo surface in 7.48.
	cellX = slot % 9;
	cellY = slot / 9;
}

int TMFieldScene::GetCarrySlotForCell(const SGridControl* grid, int cellX, int cellY) const
{
	// Ghidra FUN_0052a737 addresses native Carry as x + 9*y; the control pointer
	// is intentionally ignored because 7.48 has no inventory pages.
	(void)grid;
	return cellX + 9 * cellY;
}

int TMFieldScene::GetCargoSlotForCell(const SGridControl* grid, int cellX, int cellY) const
{
	// The 7.48 Cargo surface uses the same nine-column linearization. Cargo page
	// IDs from later clients must never alter the packet slot sent to this server.
	(void)grid;
	return cellX + 9 * cellY;
}

void TMFieldScene::DropItem(unsigned int dwServerTime)
{
	auto pAttachItem = g_pCursor->m_pAttachedItem;
	if (!pAttachItem)
		return;

	static int dwLastDropTime = 0;
	if (dwServerTime - dwLastDropTime <= 1000)
		return;

	int nAX = pAttachItem->m_nCellIndexX;
	int nAY = pAttachItem->m_nCellIndexY;

	MSG_DropItem stDrop{};
	stDrop.Header.ID = m_pMyHuman->m_dwID;
	stDrop.Header.Type = MSG_DropItem_Opcode;
	stDrop.Rotate = 0;
	stDrop.SourType = pAttachItem->m_pGridControl->CheckType(pAttachItem->m_pGridControl->m_eItemType,
		pAttachItem->m_pGridControl->m_eGridType);

	if (stDrop.SourType)
	{
		if (stDrop.SourType == 1)
		{
			// Drop from native Carry uses its single 9-column row-major slot.
			stDrop.SourPos = nAX + 9 * nAY;
		}
		else if (stDrop.SourType == 2)
		{
			// Native Cargo also uses nine columns (Ghidra FUN_0052a737).
			stDrop.SourPos = nAX + 9 * nAY;
		}
		else
			stDrop.SourPos = pAttachItem->m_pGridControl->CheckPos(pAttachItem->m_pGridControl->m_eItemType);

		stDrop.GridX = (int)m_pMyHuman->m_vecPosition.x;
		stDrop.GridY = (int)m_pMyHuman->m_vecPosition.y;
		SendOneMessage((char*)&stDrop, sizeof(stDrop));
		dwLastDropTime = dwServerTime;
	}
}

int TMFieldScene::GetItem(TMItem* pItem)
{
	IVector2 vecGrid{};
	// Native 7.48 exposes one 9x7 Carry grid. This source no longer queries the
	// four page controls imported from TMProject 7.59.
	const int carryPages = 1;
	for (int i = 0; i < carryPages; ++i)
	{
		auto pGrid = m_pGridInv;
		int nGridIndex = BASE_GetItemAbility(&pItem->m_stItem, 33);
		vecGrid = pGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
		if (vecGrid.x > -1 && vecGrid.y > -1 || BASE_GetItemAbility(&pItem->m_stItem, 38) == 2)
		{
			m_pMyHuman->MoveGet(pItem);
			return 1;
		}
	}

	auto pListBox = m_pChatList;
	auto pBoxItem = new SListBoxItem(g_pMessageStringTable[1], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);

	pListBox->AddItem(pBoxItem);

	GetSoundAndPlay(33, 0, 0);
	m_dwGetItemTime = g_pTimerManager->GetServerTime();
	return 0;
}

void TMFieldScene::SetVisibleInventory()
{
	SGridControl::m_sLastMouseOverIndex = -1;

	// Native FUN_00447691 has a shared UI2 closing cascade. Keep its order and
	// six artisan roots without importing the later multi-page/mix topology.
	if (m_bCompatFieldScene)
	{
		if (!m_pInvenPanel && m_pControlContainer)
			m_pInvenPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_INV_PANEL));
		if (!m_pInvenPanel)
		{
			WYD748_DiagnosticsLog("compat inventory missing panel id=%u\r\n", TMP_INV_PANEL);
			return;
		}

		if (m_pGambleStore && m_pGambleStore->IsVisible() == 1)
		{
			m_pInvenPanel->m_bVisible = 0;
			if (m_pCPanel)
				m_pCPanel->m_bVisible = 0;
		}

		// Capture the target before closing peers: AutoTrade itself hides Carry.
		const int visible = m_pInvenPanel->IsVisible() == 0;
		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);
		if (m_pGambleStore && m_pGambleStore->IsVisible() == 1)
			SetVisibleGamble(0, 0);

		m_pInvenPanel->SetVisible(!visible);
		if (!visible)
		{
			for (int mixIndex = 1; mixIndex <= 6; ++mixIndex)
			{
				ClearNativeMix(mixIndex);
				if (auto panel = GetNativeMixPanel(mixIndex))
					panel->SetVisible(0);
			}
			SPanel* peers[] = {
				m_pCargoPanel, m_pShopPanel, m_pHellgateStore, m_pInputGoldPanel
			};
			for (auto panel : peers)
			{
				if (panel)
					panel->SetVisible(0);
			}
			SetGridState();
			if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
				SetVisibleTrade(0);
			if (g_pCursor)
				g_pCursor->DetachItem();
		}
		if (m_pControlContainer)
		{
			if (auto skillButton = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SKILL)))
				skillButton->SetSelected(m_pSkillPanel && m_pSkillPanel->IsVisible());
		}
		m_pInvenPanel->SetVisible(visible);
		WYD748_DiagnosticsLog("compat inventory visible=%d\r\n", visible);
		if (g_pSoundManager)
		{
			if (auto pSoundData = g_pSoundManager->GetSoundData(51))
				pSoundData->Play(0, 0);
		}
		return;
	}

	auto pCargoPanel = m_pCargoPanel;
	auto pPanel = m_pInvenPanel;
	auto pCPanel = m_pCPanel;
	auto pSkillPanel = m_pSkillPanel;
	auto pSkillMPanel = m_pSkillMPanel;
	auto pTradePanel = m_pTradePanel;
	auto pATradePanel = m_pAutoTrade;
	auto pItemMixPanel = m_pItemMixPanel;
	auto pItemMixPanel4 = m_pItemMixPanel4;
	auto pHellgateStore = m_pHellgateStore;
	auto pGambleStore = m_pGambleStore;

	if (m_pGambleStore && m_pGambleStore->IsVisible() == 1)
		pPanel->m_bVisible = 0;

	if (pGambleStore && pGambleStore->IsVisible() == 1)
		pCPanel->m_bVisible = 0;

	int bInv = pPanel->m_bVisible == 0;

	if (pATradePanel && pATradePanel->IsVisible() == 1)
		SetVisibleAutoTrade(0, 0);

	if (pGambleStore && pGambleStore->IsVisible() == 1)
		SetVisibleGamble(0, 0);

	pPanel->SetVisible(bInv == 0);

	if (!bInv)
	{
		ClearCombine();
		ClearCombine4();
		ClearMixPannel();

		m_ItemMixClass.m_pMixPanel->SetVisible(0);

		if (m_ItemMixClass.m_pMixPanel)
			m_ItemMixClass.m_pMixPanel->SetVisible(0);

		ClearMissionPannel();

		m_MissionClass.m_pMissionPanel->SetVisible(0);

		if (m_MissionClass.m_pMissionPanel)
			m_MissionClass.m_pMissionPanel->SetVisible(0);

		SetVisibleShop(0);

		pCargoPanel->SetVisible(0);
		pItemMixPanel->SetVisible(0);
		pItemMixPanel4->SetVisible(0);
		pHellgateStore->SetVisible(0);

		SetGridState();

		m_pInputGoldPanel->SetVisible(0);

		if (pTradePanel && pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);

		if (g_pCursor)
			g_pCursor->DetachItem();
	}

	pPanel->SetVisible(bInv);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleCargo(int bShow)
{
	// Cargo in FieldScene2.bin is a single 7.48 page.  Do not touch the second
	// page or gamble controls that exist only in the imported 7.59 resource.
	if (m_bCompatFieldScene)
	{
		if (bShow && m_pCargoPanel && m_pInvenPanel && g_pDevice)
		{
			// Reapply the native Cargo pair geometry on every open so AutoTrade,
			// shops, or other panels cannot leave either window at a stale position.
			const float cargoX = ((float)g_pDevice->m_dwScreenWidth - m_pCargoPanel->m_nWidth) * 0.5f;
			const float sharedY = RenderDevice::m_fHeightRatio * 35.0f;
			const float panelGap = RenderDevice::m_fWidthRatio * 24.4f;
			m_pCargoPanel->SetPos(cargoX, sharedY);
			m_pInvenPanel->SetPos(cargoX + m_pCargoPanel->m_nWidth + panelGap, sharedY);
		}
		if (m_pCargoPanel)
			m_pCargoPanel->SetVisible(bShow);
		if (m_pInvenPanel)
			m_pInvenPanel->SetVisible(bShow);
		GetSoundAndPlay(51, 0, 0);
		return;
	}
	if (m_pGambleStore && m_pGambleStore->IsVisible() == 1)
		bShow = 0;

	SGridControl::m_sLastMouseOverIndex = -1;

	m_pCargoPanel->SetVisible(bShow);
	m_pCargoPanel1->SetVisible(bShow);

	if (bShow)
	{
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 650.0f, RenderDevice::m_fHeightRatio * 35.0f);
		m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 423.0f, RenderDevice::m_fHeightRatio * 35.0f);
		m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 423.0f, RenderDevice::m_fHeightRatio * 35.0f);
		if (g_pApp->m_dwScreenWidth == 1024)
		{//new
			m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 650.0f, RenderDevice::m_fHeightRatio * 35.0f);
			m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 372.0f, RenderDevice::m_fHeightRatio * 35.0f);
			m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 423.0f, RenderDevice::m_fHeightRatio * 35.0f);
		}
	}

	m_pInvenPanel->SetVisible(bShow);
	m_pSkillPanel->SetVisible(bShow == 0);
	m_pShopPanel->SetVisible(bShow == 0);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleCargo1(int bShow)
{
	if (m_pGambleStore1->IsVisible() == 1)
		bShow = 0;

	SGridControl::m_sLastMouseOverIndex = -1;
	auto pTradePanel = m_pTradePanel;
	auto pATradePanel = m_pAutoTrade;
	auto pShopPanel = m_pShopPanel;
	auto pPanel = m_pInvenPanel;
	auto pCPanel = m_pCPanel;
	auto pHellgateStore = m_pHellgateStore;
	m_pCargoPanel1->SetVisible(bShow);

	if (bShow)
	{
		pPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f,
			RenderDevice::m_fHeightRatio * 35.0f);
		m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
			RenderDevice::m_fHeightRatio * 35.0f);
	}

	pPanel->SetVisible(bShow);
	m_pSkillPanel->SetVisible(bShow == 0);
	pShopPanel->SetVisible(bShow == 0);

	GetSoundAndPlay(51, 0, 0);
}

void TMFieldScene::SetInVisibleInputCoin()
{
	if (m_nCoinMsgType == 12)
	{
		auto pSplitItem = m_pGridInv ? field_interaction::FindOwnedItem(
			m_pGridInv->m_pItemList, m_pGridInv->m_nNumItem, SGridControl::m_pSellItem) : nullptr;
		SGridControl::m_pSellItem = nullptr;
		m_nCoinMsgType = -1;
		if (pSplitItem)
			pSplitItem->m_GCObj.dwColor = 0xFFFFFFFF;
	}
	// Native FUN_00447594 closes edit 627; the imported scene uses 65889.
	// Resolve through the active resource ABI and tolerate an absent optional UI.
	const unsigned int editControlID = m_bCompatFieldScene ? TME_INPUT_GOLD : E_INPUT_GOLD;
	auto pEdit = m_pControlContainer
		? static_cast<SEditableText*>(m_pControlContainer->FindControl(editControlID))
		: nullptr;
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;

	if (m_pControlContainer)
		m_pControlContainer->SetFocusedControl(nullptr);

	if (pInputGoldPanel)
		pInputGoldPanel->SetVisible(0);
	if (pEdit)
		pEdit->SetText((char*)"");

	if (m_nLastAutoTradePos >= 0)
	{
		// Restore the selected cargo icon through the same version-aware mapping
		// used when it was inserted into the auto-trade window.
		int cargoX = 0;
		int cargoY = 0;
		GetCargoCellForSlot(m_nLastAutoTradePos, cargoX, cargoY);
		auto pCargoGrid = GetCargoGridForSlot(m_nLastAutoTradePos);
		auto pCargoItem = pCargoGrid ? pCargoGrid->GetAtItem(cargoX, cargoY) : nullptr;
		if (pCargoItem)
			pCargoItem->m_GCObj.dwColor = 0xFFFFFFFF;

		m_nLastAutoTradePos = -1;
	}
	if (g_nKeyType == 1 && m_pControlContainer && m_pEditChat)
		m_pControlContainer->SetFocusedControl(m_pEditChat);
}

void TMFieldScene::SetInventoryGridType(TMEGRIDTYPE gridType)
{
	// The 7.48 resource materializes only list[0]. Null-aware iteration keeps
	// newer source-only pages from becoming a dereference or a second slot ABI.
	for (auto* pGridInv : m_pGridInvList)
	{
		if (pGridInv)
			pGridInv->m_eGridType = gridType;
	}
}

void TMFieldScene::SetGridState()
{
	SetInventoryGridType(TMEGRIDTYPE::GRID_DEFAULT);

	SetEquipGridState(1);
}

void TMFieldScene::SetEquipGridState(int bDefault)
{
	const auto gridType = bDefault == 1 ? TMEGRIDTYPE::GRID_DEFAULT : TMEGRIDTYPE::GRID_TRADENONE;
	SGridControl* equipGrids[] = {
		m_pGridHelm, m_pGridCoat, m_pGridPants, m_pGridGloves, m_pGridBoots,
		m_pGridRight, m_pGridLeft, m_pGridGuild, m_pGridEvent, m_pGridRing,
		m_pGridNecklace, m_pGridOrb, m_pGridCabuncle, m_pGridDRing,
		m_pGridNewSlot1, m_pGridNewSlot2
	};
	// FieldScene2.bin 7.48 does not materialize the newer NewSlot controls.
	// Mutating only bound grids matches the native optional-control lifecycle.
	for (auto* grid : equipGrids)
	{
		if (grid)
			grid->m_eGridType = gridType;
	}

	// Mantua is never accepted as a trade source in the original behavior.
	if (m_pGridMantua)
		m_pGridMantua->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
}

void TMFieldScene::UpdateMyHuman()
{
	STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;

	appearance_refinement::Rebuild(*m_pMyHuman, *pMobData);

	float fCon = (float)m_pMyHuman->m_stScore.Con;
	m_pMyHuman->SetCharHeight(fCon);
	m_pMyHuman->SetRace(pMobData->Equip[0].sIndex);

	int nWeaponTypeL = BASE_GetItemAbility(&pMobData->Equip[6], 21);
	if (nWeaponTypeL == 41)
	{
		m_pMyHuman->m_stLookInfo.RightMesh = m_pMyHuman->m_stLookInfo.LeftMesh;
		m_pMyHuman->m_stLookInfo.RightSkin = m_pMyHuman->m_stLookInfo.LeftSkin;
		m_pMyHuman->m_stSancInfo.Sanc6 = m_pMyHuman->m_stSancInfo.Sanc7;
		m_pMyHuman->m_stSancInfo.Legend6 = m_pMyHuman->m_stSancInfo.Legend7;
	}

	m_pMyHuman->InitObject();
	m_pMyHuman->CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
	m_pMyHuman->InitAngle(0.0f, m_pMyHuman->m_fAngle, 0.0f);

	m_pMyHuman->CheckAffect();
	SetSanc();
}

void TMFieldScene::SetSanc()
{
	m_nMySanc = BASE_GetItemSanc(&g_pObjectManager->m_stMobData.Equip[4]);
}

int TMFieldScene::GetItemFromGround(unsigned int dwServerTime)
{
	if (g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill] == 31)
		return 0;

	auto pOverItem = m_pMouseOverItem;
	if (!pOverItem)
		return 0;
	if (dwServerTime <= m_dwGetItemTime + 1000)
		return 0;
	if (!pOverItem->m_bMouseOver)
		return 0;
	if (pOverItem->m_stItem.sIndex >= 1733 && pOverItem->m_stItem.sIndex <= 1736)
		return 1;
	if (pOverItem->m_stItem.sIndex >= 3145 && pOverItem->m_stItem.sIndex <= 3149)
		return 1;

	if (BASE_GetItemAbility(&pOverItem->m_stItem, 34) <= 0)
	{
		GetItem(pOverItem);
		return 1;
	}

	if (BASE_GetItemAbility(&pOverItem->m_stItem, 34) == 10 && pOverItem->m_stItem.sIndex >= 4100 && pOverItem->m_stItem.sIndex < 4200)
	{
		if (m_pGambleStore && m_pGambleStore->m_bVisible == 1)
			return 1;

		if (pOverItem->m_stItem.sIndex == 4102)
		{
			SetVisibleGamble(1, 2);
			return 1;
		}
		if (pOverItem->m_stItem.sIndex == 4103)
		{
			SetVisibleGamble(1, 1);
			return 1;
		}
	}

	MSG_UpdateItem stUpdateItem{};
	stUpdateItem.Header.ID = m_pMyHuman->m_dwID;
	stUpdateItem.ItemID = pOverItem->m_dwID;
	stUpdateItem.Header.Type = MSG_UpdateItem_Opcode;
	stUpdateItem.State = 1;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUpdateItem)->Type, reinterpret_cast<char*>(&stUpdateItem), sizeof(stUpdateItem)});

	m_dwGetItemTime = g_pTimerManager->GetServerTime();

	return 1;
}

char TMFieldScene::UseHPotion()
{
	SGridControl* pGridInv = m_pGridInv;
	SGridControlItem* pItem = nullptr;
	if (!pGridInv)
		return 0;

	bool bFind = false;
	// FUN_0044effc searches the sole 9x7 Carry from the last row/column toward
	// zero. There is no 7.59 page selector in the client 7.48 executable.
	for (int j = 6; j >= 0; --j)
	{
		for (int k = 8; k >= 0; --k)
		{
			pItem = pGridInv->GetItem(k, j);
			if (pItem &&
				(pItem->m_pItem->sIndex == 4097
					|| pItem->m_pItem->sIndex == 3431
					|| pItem->m_pItem->sIndex == 3322
					|| pItem->m_pItem->sIndex == 3477
					|| pItem->m_pItem->sIndex >= 400 && pItem->m_pItem->sIndex <= 404
					|| pItem->m_pItem->sIndex >= 428 && pItem->m_pItem->sIndex <= 431
					|| pItem->m_pItem->sIndex >= 680 && pItem->m_pItem->sIndex <= 685))
			{
				bFind = true;
				break;
			}
		}
		if (bFind)
			break;
	}

	if (bFind != 1 || !pItem)
		return 0;

	if (BASE_GetItemAbility(pItem->m_pItem, 38) == 1)
	{
		if (m_pMyHuman->m_cCancel == 1)
			return 0;

		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
			return 0;

		pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
			pItem->m_pGridControl->m_eGridType);

		int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
		// Slot 63 exists only as structure padding; the 7.48 Carry exposes 0..62.
		if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
			return 0;

		auto vec = m_pMyHuman->m_vecPosition;

		MSG_UseItem stUseItem{};
		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPos;
		stUseItem.ItemID = 0;
		stUseItem.GridX = (int)vec.x;
		stUseItem.GridY = (int)vec.y;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});
		m_dwUseItemTime = dwServerTime;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);

		if (nAmount <= 1)
		{
			// Remove the consumed potion from the exact native slot sent on the wire.
			int carryX = 0;
			int carryY = 0;
			GetCarryCellForSlot(SourPos, carryX, carryY);
			auto pCarryGrid = GetCarryGridForSlot(SourPos);
			auto pPickedItem = pCarryGrid ? pCarryGrid->PickupItem(carryX, carryY) : nullptr;
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = 0;

			SAFE_DELETE(pPickedItem);
			g_pCursor->DetachItem();
		}
		else
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
			sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		if (nAmount <= 1)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));

		GetSoundAndPlay(41, 0, 0);
	}

	UpdateScoreUI(16);
	return 1;
}

char TMFieldScene::UseMPotion()
{
	SGridControl* pGridInv = m_pGridInv;
	SGridControlItem* pItem = nullptr;
	if (!pGridInv)
		return 0;

	bool bFind = false;
	// FUN_0044f46b uses the same reverse 9x7 scan as the native HP shortcut.
	for (int j = 6; j >= 0; --j)
	{
		for (int k = 8; k >= 0; --k)
		{
			pItem = pGridInv->GetItem(k, j);
			if (pItem && (pItem->m_pItem->sIndex == 3323
				|| pItem->m_pItem->sIndex == 3472
				|| pItem->m_pItem->sIndex >= 405 && pItem->m_pItem->sIndex <= 409
				|| pItem->m_pItem->sIndex >= 432 && pItem->m_pItem->sIndex <= 435
				|| pItem->m_pItem->sIndex >= 686 && pItem->m_pItem->sIndex <= 691))
			{
				bFind = true;
				break;
			}
		}
		if (bFind)
			break;
	}

	if (bFind != 1 || !pItem)
		return 0;

	if (BASE_GetItemAbility(pItem->m_pItem, 38) == 1)
	{
		if (m_pMyHuman->m_cCancel == 1)
			return 0;

		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
			return 0;

		pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
			pItem->m_pGridControl->m_eGridType);

		int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
		if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
			return 0;

		auto vec = m_pMyHuman->m_vecPosition;

		MSG_UseItem stUseItem{};
		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPos;
		stUseItem.ItemID = 0;
		stUseItem.GridX = (int)vec.x;
		stUseItem.GridY = (int)vec.y;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});
		m_dwUseItemTime = dwServerTime;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);

		if (nAmount <= 1)
		{
			// Keep the visual removal and server Carry index on one ABI mapping.
			int carryX = 0;
			int carryY = 0;
			GetCarryCellForSlot(SourPos, carryX, carryY);
			auto pCarryGrid = GetCarryGridForSlot(SourPos);
			auto pPickedItem = pCarryGrid ? pCarryGrid->PickupItem(carryX, carryY) : nullptr;
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = 0;

			SAFE_DELETE(pPickedItem);
			g_pCursor->DetachItem();
		}
		else
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
			sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		if (nAmount <= 1)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));

		GetSoundAndPlay(41, 0, 0);
	}

	UpdateScoreUI(16);
	return 1;
}

void TMFieldScene::UsePPotion()
{
	if (!m_pGridInv || !m_pMyHuman || !g_pObjectManager || !g_pTimerManager)
		return;

	SGridControlItem* pItem = nullptr;
	// FUN_0044F88F scans X first, then Y, and deliberately skips item 3378.
	for (int nX = 8; nX >= 0 && !pItem; --nX)
	{
		for (int nY = 6; nY >= 0; --nY)
		{
			auto* pCandidate = m_pGridInv->GetItem(nX, nY);
			if (pCandidate && pCandidate->m_pItem
				&& BASE_GetItemAbility(pCandidate->m_pItem, 38) == 230
				&& pCandidate->m_pItem->sIndex != 3378)
			{
				pItem = pCandidate;
				break;
			}
		}
	}

	if (!pItem || !pItem->m_pItem || !pItem->m_pGridControl)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return;

	m_pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
		pItem->m_pGridControl->m_eGridType);

	int SourPos = m_pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
		SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
	if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
		return;

	auto vec = m_pMyHuman->m_vecPosition;

	MSG_UseItem stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem_Opcode;
	stUseItem.SourType = 1;
	stUseItem.SourPos = SourPos;
	stUseItem.ItemID = 0;
	stUseItem.GridX = (int)vec.x;
	stUseItem.GridY = (int)vec.y;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});
	m_dwUseItemTime = dwServerTime;

	// This shortcut is one of the native optimistic-consumption paths. Once the
	// server resolves a valid occupied slot, rejected uses republish that slot.
	int nAmount = BASE_GetItemAmount(pItem->m_pItem);
	if (nAmount <= 1)
	{
		int carryX = 0;
		int carryY = 0;
		GetCarryCellForSlot(SourPos, carryX, carryY);
		auto* pCarryGrid = GetCarryGridForSlot(SourPos);
		auto* pPickedItem = pCarryGrid ? pCarryGrid->PickupItem(carryX, carryY) : nullptr;
		if (g_pCursor && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pPickedItem);
		if (g_pCursor)
			g_pCursor->DetachItem();
		memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
	}
	else
	{
		BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
		sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
		if (pItem->m_GCText.pFont)
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
	}

	GetSoundAndPlay(41, 0, 0);
	UpdateScoreUI(16);
}

int TMFieldScene::IsFeedPotion(short sMountIndex, short sItemIndex)
{
	if ((sMountIndex == 2330 || sMountIndex == 2360) && sItemIndex == 2420)
		return 1;
	if ((sMountIndex == 2331 || sMountIndex == 2361) && (sItemIndex == 2421 || sItemIndex == 3368))
		return 1;
	if ((sMountIndex == 2332 || sMountIndex == 2362) && (sItemIndex == 2422 || sItemIndex == 3369))
		return 1;
	if ((sMountIndex == 2333 || sMountIndex == 2363) && (sItemIndex == 2423 || sItemIndex == 3370))
		return 1;
	if ((sMountIndex == 2334 || sMountIndex == 2364) && (sItemIndex == 2424 || sItemIndex == 3371))
		return 1;
	if ((sMountIndex == 2335 || sMountIndex == 2365) && (sItemIndex == 2425 || sItemIndex == 3372))
		return 1;
	if ((sMountIndex >= 2336 && sMountIndex <= 2345 || sMountIndex >= 2366 && sMountIndex <= 2375 || sMountIndex >= 2960 && sMountIndex < 3000) &&
		(sItemIndex == 2426 || sItemIndex == 3373))
	{
		return 1;
	}
	if ((sMountIndex == 2346 || sMountIndex == 2376) && (sItemIndex == 2436 || sItemIndex == 3383))
		return 1;
	if ((sMountIndex == 2347 || sMountIndex == 2377) && (sItemIndex == 2437 || sItemIndex == 3384))
		return 1;
	if ((sMountIndex == 2349 || sMountIndex == 2379) && (sItemIndex == 2427 || sItemIndex == 3374))
		return 1;
	if ((sMountIndex == 2350 || sMountIndex == 2380) && (sItemIndex == 2428 || sItemIndex == 3375))
		return 1;
	if ((sMountIndex >= 2351 && sMountIndex <= 2353 || sMountIndex >= 2381 && sMountIndex <= 2383) && (sItemIndex == 2429 || sItemIndex == 3376))
		return 1;
	if ((sMountIndex >= 2354 && sMountIndex <= 2356 || sMountIndex >= 2384 && sMountIndex <= 2386) && (sItemIndex == 2430 || sItemIndex == 3377))
		return 1;
	if ((sMountIndex == 2357 || sMountIndex == 2387) && (sItemIndex == 2426 || sItemIndex == 3373))
		return 1;
	if ((sMountIndex == 2358 || sMountIndex == 2388) && (sItemIndex == 2429 || sItemIndex == 3376))
		return 1;
	if ((sMountIndex == 2378 || sMountIndex == 2348) && (sItemIndex == 3465 || sItemIndex == 2438))
		return 1;
	if ((sMountIndex == 2389 || sMountIndex == 2359) && (sItemIndex == 3466 || sItemIndex == 2439))
		return 1;
	return 0;
}

char TMFieldScene::FeedMount()
{
	SGridControl* pGridInv = m_pGridInv;
	SGridControlItem* pItem = nullptr;
	if (!pGridInv)
		return 0;

	bool bFind = false;
	// FUN_004502a7 searches mount food in reverse over the sole 9x7 Carry.
	for (int j = 6; j >= 0; --j)
	{
		for (int k = 8; k >= 0; --k)
		{
			pItem = pGridInv->GetItem(k, j);
			if (pItem && IsFeedPotion(g_pObjectManager->m_stMobData.Equip[14].sIndex,
				pItem->m_pItem->sIndex))
			{
				bFind = true;
				break;
			}
		}
		if (bFind)
			break;
	}

	if (bFind != 1 || !pItem)
		return 0;

	if (BASE_GetItemAbility(pItem->m_pItem, EF_VOLATILE) == 15)
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
			return 0;

		pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
			pItem->m_pGridControl->m_eGridType);

		int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
		if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
			return 0;

		auto vec = m_pMyHuman->m_vecPosition;

		MSG_UseItem stUseItem{};
		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.DestType = 0;
		stUseItem.DestPos = 14;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPos;
		stUseItem.ItemID = 0;
		stUseItem.GridX = (int)vec.x;
		stUseItem.GridY = (int)vec.y;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});
		m_dwUseItemTime = dwServerTime;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);
		if (nAmount <= 1)
		{
			// Remove food by its real 7.48 Carry coordinate.
			int carryX = 0;
			int carryY = 0;
			GetCarryCellForSlot(SourPos, carryX, carryY);
			auto pCarryGrid = GetCarryGridForSlot(SourPos);
			auto pPickedItem = pCarryGrid ? pCarryGrid->PickupItem(carryX, carryY) : nullptr;
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = 0;

			SAFE_DELETE(pPickedItem);
			g_pCursor->DetachItem();
		}
		else
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
			sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		if (nAmount <= 1)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
	}

	UpdateScoreUI(0);
	return 1;
}

void TMFieldScene::UseTicket(int nCellX, int nCellY)
{
	// FUN_0045061f resolves EF_VOLATILE 14 directly against FieldScene2 Carry;
	// control 65554 and page arithmetic belong to a later client resource.
	auto pGrid = m_pGridInv;
	if (!pGrid)
		return;
	auto pItem = pGrid->GetItem(nCellX, nCellY);
	if (!pItem)
		return;

	if (BASE_GetItemAbility(pItem->m_pItem, 38) != 14)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return;

	int SourPos = pGrid->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
		SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
	if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
		return;

	MSG_UseItem stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem_Opcode;
	stUseItem.SourType = 1;
	stUseItem.SourPos = SourPos;
	stUseItem.ItemID = 0;
	stUseItem.GridX = (int)m_pMyHuman->m_vecPosition.x;
	stUseItem.GridY = (int)m_pMyHuman->m_vecPosition.y;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});

	m_dwUseItemTime = dwServerTime;
	g_pEventTranslator->m_bRBtn = 1;

	int nAmount = BASE_GetItemAmount(pItem->m_pItem);
	if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
		nAmount = 0;

	if (nAmount > 1)
	{
		BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
		sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
		pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
	}
	else
	{
		int carryX = 0;
		int carryY = 0;
		GetCarryCellForSlot(SourPos, carryX, carryY);
		auto pPickedItem = pGrid->PickupItem(carryX, carryY);
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;

		SAFE_DELETE(pPickedItem);
	}

	GetSoundAndPlay(47, 0, 0);

	UpdateScoreUI(0);
	if (nAmount <= 1)
		memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
}

char TMFieldScene::UseQuickSloat(char key)
{
	const int slot = field_interaction::QuickSlotIndex(m_bCompatFieldScene != 0, key);
	if (slot < 0)
		return 0;
	SGridControl* pGridSloat = m_pQuick_Sloat[slot];
	if (!pGridSloat)
		return 0;
	SGridControlItem* pItemFind = pGridSloat->GetAtItem(0, 0);
	if (!pItemFind || !pItemFind->m_pItem)
		return 0;

	SGridControlItem* pItem = nullptr;
	SGridControl* pGridInv = m_pGridInv;
	if (!pGridInv)
		return 0;
	bool bFind = false;
	// FUN_0044effc..FUN_0044fc4b resolve shortcut items by scanning the single
	// native Carry from (8,6) to (0,0); no page contributes to SourPos.
	for (int j = 6; j >= 0; --j)
	{
		for (int k = 8; k >= 0; --k)
		{
			pItem = pGridInv->GetItem(k, j);
			if (pItem && pItem->m_pItem->sIndex == pItemFind->m_pItem->sIndex)
			{
				bFind = true;
				break;
			}
		}
		if (bFind)
			break;
	}
	if (bFind != true || !pItem)
		return 0;

	int nType = BASE_GetItemAbility(pItem->m_pItem, 38);
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return 0;

	int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
		SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
	if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
		return 0;

	MSG_UseItem stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem_Opcode;
	stUseItem.SourType = 1;
	stUseItem.SourPos = SourPos;
	stUseItem.ItemID = 0;
	stUseItem.GridX = (int)m_pMyHuman->m_vecPosition.x;
	stUseItem.GridY = (int)m_pMyHuman->m_vecPosition.y;

	if (nType == 15)
	{
		stUseItem.DestType = 0;
		stUseItem.DestPos = 14;
	}

	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});

	m_dwUseItemTime = dwServerTime;
	int nAmount = BASE_GetItemAmount(pItem->m_pItem);

	if (nAmount > 1)
	{
		BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
		sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
		pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		sprintf(pItemFind->m_GCText.strString, "%2d", nAmount - 1);
		pItemFind->m_GCText.pFont->SetText(pItemFind->m_GCText.strString, pItemFind->m_GCText.dwColor, 0);
	}
	else
	{
		// Remove the exhausted item from the active Carry ABI surface.
		int carryX = 0;
		int carryY = 0;
		GetCarryCellForSlot(stUseItem.SourPos, carryX, carryY);
		auto pCarryGrid = GetCarryGridForSlot(stUseItem.SourPos);
		auto pPickedItem = pCarryGrid ? pCarryGrid->PickupItem(carryX, carryY) : nullptr;
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;

		SAFE_DELETE(pPickedItem);

		g_pCursor->DetachItem();
		bool bFind = false;
		for (int l = 6; l >= 0; --l)
		{
			for (int m = 8; m >= 0; --m)
			{
				auto pCandidate = pGridInv->GetItem(m, l);
				if (pCandidate && pCandidate->m_pItem->sIndex == pItemFind->m_pItem->sIndex)
				{
					bFind = true;
					break;
				}
			}
			if (bFind)
				break;
		}
		if (!bFind)
		{
			auto pReturnItem = pGridSloat->PickupItem(0, 0);
			SAFE_DELETE(pReturnItem);
		}
	}
	if (nAmount <= 1)
		memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));

	GetSoundAndPlay(41, 0, 0);
	UpdateScoreUI(16);

	return 1;
}

void TMFieldScene::VisibleInputCharName(SGridControlItem* pItem, int nCellX, int nCellY)
{
	if (!pItem || !pItem->m_pItem || !pItem->m_pGridControl || !m_pGridInv || !m_pInputGoldPanel)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (!m_dwUseItemTime || (dwServerTime - m_dwUseItemTime) >= 200)
	{
		short SourPos = m_pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
		// Capsule naming stores the same 0..62 Carry index for the later send.
		if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
			return;

		memset(&m_stCapsuleItem, 0, sizeof(m_stCapsuleItem));
		m_stCapsuleItem.Header.ID = g_pObjectManager->m_dwCharID;
		m_stCapsuleItem.Header.Type = 972;
		m_stCapsuleItem.SourType = 1;
		m_stCapsuleItem.SourPos = SourPos;
		m_stCapsuleItem.ItemID = 0;
		m_stCapsuleItem.GridX = nCellX;
		m_stCapsuleItem.GridY = nCellY;

		auto pText = static_cast<SText*>(m_pControlContainer->FindControl(m_bCompatFieldScene ? 630 : T_INPUT_GOLD));
		auto pEdit = static_cast<SEditableText*>(m_pControlContainer->FindControl(m_bCompatFieldScene ? 627 : E_INPUT_GOLD));

		if (pText && pEdit)
		{
			m_nCoinMsgType = 6;
			pText->SetText(g_pMessageStringTable[349], 0);
			m_pControlContainer->SetFocusedControl(pEdit);
			m_pInputGoldPanel->SetVisible(1);

			auto pInputBG2 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_INPUT_BG2));

			if (pInputBG2)
				pInputBG2->SetVisible(1);

			pEdit->m_nMaxStringLen = 16;
		}
	}
}

void TMFieldScene::UseItem(SGridControlItem* pItem, int nType, int nItemSIndex, int nCellX, int nCellY)
{
	if (!pItem || !pItem->m_pItem || !pItem->m_pGridControl || !m_pGridInv || !m_pMyHuman)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (!m_dwUseItemTime || (dwServerTime - m_dwUseItemTime) >= 200)
	{
		short SourPos = m_pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);

		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
		// FUN_00465f85 sends one row-major Carry index; slot 63 is not exposed.
		if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
			return;

		MSG_UseItem stUseItem{};

		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPos;

		if (nType == 15)
		{
			stUseItem.DestType = 0;
			stUseItem.DestPos = 14;
		}

		stUseItem.ItemID = 0;
		stUseItem.GridX = static_cast<int>(m_pMyHuman->m_vecPosition.x);
		stUseItem.GridY = static_cast<int>(m_pMyHuman->m_vecPosition.y);

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});

		m_dwUseItemTime = dwServerTime;
		g_pEventTranslator->m_bRBtn = 1;

		int delAmountCnt = 1;

		if (pItem->m_pItem->sIndex == 4049)
			delAmountCnt = 10;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);

		if (nItemSIndex >= 2330 && nItemSIndex < 2390)
			nAmount = 0;

		if (nAmount > delAmountCnt)
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - delAmountCnt);
			sprintf_s(pItem->m_GCText.strString, "%2d", nAmount - delAmountCnt);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		else
		{
			// Optimistic removal must target the exact slot encoded in the packet.
			int carryX = 0;
			int carryY = 0;
			GetCarryCellForSlot(SourPos, carryX, carryY);
			SGridControlItem* pPickedItem = m_pGridInv->PickupItem(carryX, carryY);

			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = nullptr;

			if (pPickedItem)
				delete pPickedItem;
		}

		int nSoundIndex = 41;

		if (nType >= 11 && nType <= 13)
			nSoundIndex = 54;

		if (nType != 19 && g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}

		UpdateScoreUI(0);

		if (nAmount <= delAmountCnt)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
	}
}

bool TMFieldScene::SendCapsuleItem()
{
	if (!m_pControlContainer || !m_pGridInv || !m_pMyHuman || !m_pInputGoldPanel)
		return false;
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return false;
	auto pEditID = static_cast<SEditableText*>(m_pControlContainer->FindControl(m_bCompatFieldScene ? 627 : E_INPUT_GOLD));
	if (!pEditID || m_stCapsuleItem.SourPos < 0 || m_stCapsuleItem.SourPos >= MAX_CARRY - 1)
		return false;
	auto pItem = m_pGridInv->GetItem(m_stCapsuleItem.GridX, m_stCapsuleItem.GridY);
	if (!pItem || !pItem->m_pItem || pItem->m_pItem->sIndex != 3443 ||
		pItem->m_pItem->stEffect[0].cEffect != 59 ||
		pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY != m_stCapsuleItem.SourPos)
		return false;

	int len = strlen(pEditID->GetText());
	if (len < 4)
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[15], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return false;
	}
	if (len > 12)
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[16], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return false;
	}
	if (!BASE_CheckValidString(pEditID->GetText()))
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return false;
	}

	char* szName = BASE_TransCurse(pEditID->GetText());

	for (int i = 0; i < len - 1; ++i)
	{
		if (szName[i] == -95 && szName[i + 1] == -95)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return false;
		}
	}

	int nCellX = m_stCapsuleItem.GridX;
	int nCellY = m_stCapsuleItem.GridY;
	auto vec = m_pMyHuman->m_vecPosition;

	m_stCapsuleItem.GridX = (int)vec.x;
	m_stCapsuleItem.GridY = (int)vec.y;

	sprintf_s(m_stCapsuleItem.NewMobname, "%s", pEditID->GetText());
	SendOneMessage((char*)&m_stCapsuleItem, sizeof(m_stCapsuleItem));

	m_dwUseItemTime = dwServerTime;
	g_pEventTranslator->m_bRBtn = 1;

	auto pPickedItem = m_pGridInv->PickupItem(nCellX, nCellY);
	if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
		g_pCursor->m_pAttachedItem = nullptr;

	SAFE_DELETE(pPickedItem);

	UpdateScoreUI(0);
	memset(&g_pObjectManager->m_stMobData.Carry[m_stCapsuleItem.SourPos], 0, sizeof(STRUCT_ITEM));
	m_stCapsuleItem.SourPos = -1;
	return true;
}

void TMFieldScene::DropListUpdate()
{
	auto ListBox = (SListBox*)m_pControlContainer->FindControl(478472);

	if (ListBox)
	{
		ListBox->Empty();
		ListBox->m_bSelectEnable = 1;
		ListBox->m_bSelectEnable = 1;

		//if (ListBox->m_pScrollBar)
		//	ListBox->m_pScrollBar->SetCurrentPos(0);

		for (auto& i : _HudControl.DropListEvent.Drop)
		{
			//auto labelname = new SText(-1, i.name, 0xFFFFFFAA, 0.0f, 0.0f, 0.0f, 0.0f, 0, -1, 1u, 1);
			auto labelname = (new SListBoxItem(i.name, -210/*ok*/, -9.0f/*ok*/, 15.0f/*ok*/, 90.0f/*ok*/, -13.0f/*ok*/, 0/*ok*/, 0/*ok*/, 0/*ok*/, 0/*ok*/));

			labelname->m_dwAlignType = 1;
			ListBox->AddItem(labelname);
		}

		if (ListBox->m_nNumItem > 0)
		{
			ListBox->m_nSelectedItem = 0;
			UpdateGridDropList(0);

		}
	}
}

void TMFieldScene::ClearInventorySelectedItem()
{
	// FieldScene2 owns one inventory grid.  Restrict the cleanup loop to the
	// controls that were actually constructed for the active client version.
	const int carryPages = m_bCompatFieldScene ? 1 : 4;
	for (int i = 0; i < carryPages; i++)
	{
		SGridControl* carryGrid = m_bCompatFieldScene ? m_pGridInv : m_pGridInvList[i];
		if (!carryGrid)
			continue;
		for (int slots = 0; slots < carryGrid->m_nNumItem; slots++)
			carryGrid->m_pItemList[slots]->m_GCObj.dwColor = 0xFFFFFFFF;
	}
}

void TMFieldScene::UpdateGridDropList(int page)
{
	auto ListBox = (SListBox*)m_pControlContainer->FindControl(478472);
	auto GridDrop = (SGridControl*)m_pControlContainer->FindControl(478473);

	int selected = ListBox->m_nSelectedItem;
	int total = ListBox->m_nNumItem;

	if (selected < 0 || selected > total || !total) return;

	auto item = ListBox->m_pItemList[selected];

	if (!item) return;

	auto lstName = item->GetText();

	auto it = std::find_if(_HudControl.DropListEvent.Drop.begin(), _HudControl.DropListEvent.Drop.end(), [&lstName](stDropList const& c)
		{
			if (!strcmp(c.name, lstName))
				return true;

			return false;
		}
	);

	if (it == _HudControl.DropListEvent.Drop.end())
		return;

	if (page == 0)
	{
		int lb[] = { 478487, 478477, 478489, 478490 };
		char str[80];

		for (int i = 0; i < 4; i++)
		{
			memset(str, 0, sizeof(str));

			if (lb[i] == 478487)
				sprintf(str, "%s", lstName);
			else if (lb[i] == 478477)
				sprintf(str, "Position: %d %d", it->X, it->Y);
			else if (lb[i] == 478489)
				sprintf(str, " %d", it->gold);
			else if (lb[i] == 478490)
				sprintf(str, " %d", it->exp);

			auto _lb = (SText*)m_pControlContainer->FindControl(lb[i]);
			if (_lb)
				_lb->SetText(str, 0);
		}
	}

	// Where is it located?

	if (strcmp(_HudControl.DropListEvent.DropSelected.Name, lstName))
	{
		strcpy(_HudControl.DropListEvent.DropSelected.Name, lstName);
		_HudControl.DropListEvent.DropSelected.LastSlot = 0;
		memset(&_HudControl.DropListEvent.DropSelected.Slot, 0, sizeof(_HudControl.DropListEvent.DropSelected.Slot));
	}

	GridDrop->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;

	//memset(GridDrop->m_pbFilled, 0, GridDrop->m_nColumnGridCount * sizeof(int) * GridDrop->m_nRowGridCount);
	//memset(GridDrop->m_pItemList, 0, sizeof(GridDrop->m_pItemList));
	//GridDrop->m_nNumItem = 0;
	//g_pCursor->m_pAttachedItem = 0;

	memset(GridDrop->m_pbFilled, 0, GridDrop->m_nColumnGridCount * sizeof(int) * GridDrop->m_nRowGridCount);
	for (int i = 0; i < GridDrop->m_nNumItem; ++i)
	{
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == GridDrop->m_pItemList[i])
			g_pCursor->m_pAttachedItem = 0;

		memset(GridDrop->m_pItemList, 0, sizeof(GridDrop->m_pItemList));
	}

	GridDrop->m_nNumItem = 0;

	int start = page == 0 ? 0 : _HudControl.DropListEvent.DropSelected.LastSlot + 1;
	_HudControl.DropListEvent.DropPage = page;

	int column = 0;
	int row = 0;
	int slot = 0;

	GridDrop->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;

	int g_pDropRate[64] =
	{
		900, 900, 900, 900, 900, 900, 900, 900, 4, 4, 4,  4, 900, 900, 900, 900, 20000, 20000, 20000, 20000,
		20000, 20000, 20000, 20000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000,
		2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 3000, 3000, 3000, 3000, 3000,
		3000, 3000, 3000, 1, 35, 500, 2500, 5000, 5000, 10000, 20000
	};

	for (int i = start; i < 64; i++)
	{
		if (slot == 35)
			return;

		if (it->carry[i].sIndex == 0)
			continue;

		auto ItemDrop = new SGridControlItem(0, &it->carry[i], 0.0f, 0.0f);

		GridDrop->AddItem(ItemDrop, column, row);

		if (g_pApp->m_dwScreenWidth == 640)
		{//
			ItemDrop->m_nHeight = 27;
			ItemDrop->m_nWidth = 29;
		}
		if (g_pApp->m_dwScreenWidth == 800)
		{//
			ItemDrop->m_nHeight = 34;
			ItemDrop->m_nWidth = 37;
		}
		if (g_pApp->m_dwScreenWidth == 1024)
		{//
			ItemDrop->m_nHeight = 43;
			ItemDrop->m_nWidth = 48;
		}
		if (g_pApp->m_dwScreenWidth == 1280)
		{//
		  ItemDrop->m_nHeight = 40 ;
		  ItemDrop->m_nWidth = 45 ;
		}

		_HudControl.DropListEvent.DropSelected.LastSlot = i;
		_HudControl.DropListEvent.DropSelected.Slot[slot] = i;

		slot++;
		column++;

		if (column == 5)
		{
			column = 0;
			row++;
		}
	}

	return;

	int end = 0;

	for (int i = start; i < end; i++)
	{
		if (it->carry[i].sIndex == 0)
		{
			slot++;
			continue;
		}

		auto ItemDrop = new SGridControlItem(0, &it->carry[i], 0.0f, 0.0f);

		GridDrop->AddItem(ItemDrop, column, row);

		ItemDrop->m_nHeight = 35;
		ItemDrop->m_nWidth = 35 ;

		slot++;
		column++;

		if (column == 5)
		{
			column = 0;
			row++;
		}
	}
}

void TMFieldScene::Bag_View()
{
	// WYD 7.48 has one 63-slot inventory and does not define the four Japanese
	// bag-page controls added by the 7.59 UI.  CreateMob still calls Bag_View for
	// the local player, so skip only that unavailable extension in compatibility
	// mode instead of dereferencing its null buttons during world entry.
	if (m_bCompatFieldScene && (!m_pInvPageBtn3 || !m_pInvPageBtn4
		|| !m_pJPNBag_Day1 || !m_pJPNBag_Day2))
		return;

	char str[128]{};
	if (g_pObjectManager->m_stMobData.Carry[60].sIndex == 3467)
	{
		if (m_bJPNBag[2])
			m_pInvPageBtn3->SetTextureSetIndex(527);
		else
			m_pInvPageBtn3->SetTextureSetIndex(528);

		auto pEffect = g_pObjectManager->m_stMobData.Carry[60].stEffect;

		char szStrDay[128]{};
		char szStrMonth[128]{};

		int nMonth = 0;
		int nDate = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (pEffect[i].cEffect == EF_DATE)
			{
				nDate = (unsigned char)pEffect[i].cValue;
			}
			else if (pEffect[i].cEffect == 110)
			{
				nMonth = (unsigned char)pEffect[i].cValue;
			}
		}

		if (nDate)
			sprintf(szStrDay, g_pMessageStringTable[291], nDate);
		else
			sprintf(szStrDay, (char*)"");
		if (nMonth)
			sprintf(szStrMonth, g_pMessageStringTable[296], nMonth);
		else
			sprintf(szStrMonth, (char*)"");
		if (nDate)
			sprintf(str, "%s%s", szStrMonth, szStrDay);
		else
			sprintf(str, (char*)"");

		m_pJPNBag_Day1->SetText(str, 0);
		m_pJPNBag_Day1->SetTextColor(0xFFFFFF00);
	}
	else if (m_bJPNBag[2])
	{
		m_pInvPageBtn3->SetTextureSetIndex(548);
	}
	else
	{
		m_pInvPageBtn3->SetTextureSetIndex(549);
	}

	if (g_pObjectManager->m_stMobData.Carry[61].sIndex == 3467)
	{
		if (m_bJPNBag[3])
			m_pInvPageBtn3->SetTextureSetIndex(527);
		else
			m_pInvPageBtn3->SetTextureSetIndex(528);

		auto pEffect = g_pObjectManager->m_stMobData.Carry[61].stEffect;

		char szStrDay[128]{};
		char szStrMonth[128]{};

		int nMonth = 0;
		int nDate = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (pEffect[i].cEffect == EF_DATE)
			{
				nDate = (unsigned char)pEffect[i].cValue;
			}
			else if (pEffect[i].cEffect == 110)
			{
				nMonth = (unsigned char)pEffect[i].cValue;
			}
		}

		if (nDate)
			sprintf(szStrDay, g_pMessageStringTable[291], nDate);
		else
			sprintf(szStrDay, (char*)"");
		if (nMonth)
			sprintf(szStrMonth, g_pMessageStringTable[296], nMonth);
		else
			sprintf(szStrMonth, (char*)"");
		if (nDate)
			sprintf(str, "%s%s", szStrMonth, szStrDay);
		else
			sprintf(str, (char*)"");

		m_pJPNBag_Day2->SetText(str, 0);
		m_pJPNBag_Day2->SetTextColor(0xFFFFFF00);
	}
	else if (m_bJPNBag[3])
	{
		m_pInvPageBtn4->SetTextureSetIndex(548);
	}
	else
	{
		m_pInvPageBtn4->SetTextureSetIndex(549);
	}
}
