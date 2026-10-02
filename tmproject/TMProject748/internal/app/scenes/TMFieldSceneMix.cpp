#include "pch.h"
#include "TMFieldScene.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "TMItem.h"
#include "WYD748Assets.h"
#include "Mission.h"
#include "TMGround.h"
#include "TMHuman.h"

SPanel* TMFieldScene::GetNativeMixPanel(int mixIndex) const
{
	switch (mixIndex)
	{
	case 1: return m_pItemMixPanel;
	case 2: return m_pItemMixPanel2;
	case 3: return m_pItemMixPanel3;
	case 4: return m_pItemMixPanel4;
	case 5: return m_pItemMixPanel5;
	case 6: return m_pItemMixPanel6;
	default: return nullptr;
	}
}

SGridControl* TMFieldScene::GetNativeMixGrid(int mixIndex, int slot) const
{
	if (slot < 0 || slot >= GetNativeMixSlotCount(mixIndex))
		return nullptr;

	switch (mixIndex)
	{
	case 1: return m_pGridItemMix[slot];
	case 2: return m_pGridItemMix2[slot];
	case 3: return m_pGridItemMix3[slot];
	case 4: return m_pGridItemMix4[slot];
	case 5: return m_pGridItemMix5[slot];
	case 6: return m_pGridItemMix6[slot];
	default: return nullptr;
	}
}

MSG_CombineItem* TMFieldScene::GetNativeMixPacket(int mixIndex)
{
	if (!g_pObjectManager)
		return nullptr;

	switch (mixIndex)
	{
	case 1: return &g_pObjectManager->m_stCombineItem;
	case 2: return &g_pObjectManager->m_stCombineItem2;
	case 3: return &g_pObjectManager->m_stCombineItem3;
	case 4: return &g_pObjectManager->m_stCombineItem4;
	case 5: return &g_pObjectManager->m_stCombineItem5;
	case 6: return &g_pObjectManager->m_stCombineItem6;
	default: return nullptr;
	}
}

int TMFieldScene::GetNativeMixSlotCount(int mixIndex) const
{
	switch (mixIndex)
	{
	case 1:
	case 2:
		return 8;
	case 3:
		return 6;
	case 4:
	case 6:
		return 3;
	case 5:
		return 7;
	default:
		return 0;
	}
}

void TMFieldScene::ResetNativeMixPacket(int mixIndex)
{
	auto packet = GetNativeMixPacket(mixIndex);
	if (!packet)
		return;

	// Every 7.48 artisan packet starts with no staged inventory position. Keep
	// this reset side-effect free so it is safe before any mix UI is opened.
	memset(packet, 0, sizeof(*packet));
	packet->Header.ID = m_pMyHuman ? m_pMyHuman->m_dwID : g_pObjectManager->m_dwCharID;
	switch (mixIndex)
	{
	case 1: packet->Header.Type = MSG_CombineItem_Opcode; break;
	case 2: packet->Header.Type = MSG_CombineItemAylin_Opcode; break;
	case 3: packet->Header.Type = MSG_CombineItemAgatha_Opcode; break;
	case 4: packet->Header.Type = MSG_CombineItemTiny_Opcode; break;
	case 5: packet->Header.Type = MSG_CombineItemLindy_Opcode; break;
	case 6: packet->Header.Type = MSG_CombineItemEhre_Opcode; break;
	}
	for (int slot = 0; slot < 8; ++slot)
		packet->CarryPos[slot] = -1;
}

void TMFieldScene::ClearNativeMix(int mixIndex)
{
	auto packet = GetNativeMixPacket(mixIndex);
	if (!packet)
		return;

	const int slotCount = GetNativeMixSlotCount(mixIndex);
	for (int slot = 0; slot < slotCount; ++slot)
	{
		// A red Carry item is only restored from the packet that owns its clone;
		// this prevents one mix panel from unlocking an item staged elsewhere.
		const int carrySlot = packet->CarryPos[slot];
		if (carrySlot >= 0)
		{
			int carryX = 0;
			int carryY = 0;
			GetCarryCellForSlot(carrySlot, carryX, carryY);
			auto carryGrid = GetCarryGridForSlot(carrySlot);
			auto carryItem = carryGrid ? carryGrid->GetAtItem(carryX, carryY) : nullptr;
			if (carryItem)
				carryItem->m_GCObj.dwColor = 0xFFFFFFFF;
		}

		auto mixGrid = GetNativeMixGrid(mixIndex, slot);
		auto stagedItem = mixGrid ? mixGrid->PickupItem(0, 0) : nullptr;
		if (g_pCursor && g_pCursor->m_pAttachedItem == stagedItem)
			g_pCursor->m_pAttachedItem = nullptr;
		SAFE_DELETE(stagedItem);
	}

	ResetNativeMixPacket(mixIndex);

	SetInventoryGridType(TMEGRIDTYPE::GRID_DEFAULT);
	SetEquipGridState(1);
}

void TMFieldScene::DoNativeMix(int mixIndex)
{
	auto packet = GetNativeMixPacket(mixIndex);
	if (!packet)
		return;

	auto rejectRecipe = [this]()
	{
		if (m_pMessagePanel)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[274], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
	};
	auto hasItem = [packet](int slot)
	{
		return packet->Item[slot].sIndex > 0;
	};

	bool valid = true;
	switch (mixIndex)
	{
	case 1:
		if (g_nCombineMode == 1)
		{
			for (int slot = 0; slot < 6; ++slot)
				valid = valid && hasItem(slot);
		}
		packet->Header.Type = MSG_CombineItem_Opcode;
		break;
	case 2:
		packet->Header.Type = MSG_CombineItemAylin_Opcode;
		break;
	case 3:
		valid = hasItem(0) && hasItem(1);
		for (int slot = 2; slot < 6; ++slot)
			valid = valid && packet->Item[slot].sIndex == 3140;
		packet->Header.Type = MSG_CombineItemAgatha_Opcode;
		break;
	case 4:
		valid = hasItem(0) && hasItem(1);
		packet->Header.Type = MSG_CombineItemTiny_Opcode;
		break;
	case 5:
		if (g_nCombineMode == 0 || g_nCombineMode == 1)
		{
			valid = hasItem(0) && hasItem(1);
			packet->Header.Type = g_nCombineMode == 0
				? MSG_CombineItemLindy_Opcode : MSG_CombineItemLindyAlt_Opcode;
		}
		else if (g_nCombineMode == 2)
		{
			const int first = packet->Item[0].sIndex;
			if (first == 413)
			{
				for (int slot = 0; slot < 7; ++slot)
					valid = valid && packet->Item[slot].sIndex == 413;
			}
			else if (first == 4127)
			{
				valid = packet->Item[1].sIndex == 4127 && packet->Item[2].sIndex == 5135;
				for (int slot = 3; slot < 7; ++slot)
					valid = valid && packet->Item[slot].sIndex == 413;
			}
			else if (first >= 5110 && first <= 5133)
			{
				for (int slot = 0; slot < 7; ++slot)
					valid = valid && packet->Item[slot].sIndex >= 5110 && packet->Item[slot].sIndex <= 5133;
			}
			else if (first == 421)
			{
				for (int slot = 0; slot < 7; ++slot)
					valid = valid && packet->Item[slot].sIndex == 421 + slot;
			}
			else if (first == 4146)
			{
				valid = packet->Item[1].sIndex == 4146 && packet->Item[2].sIndex == 5135;
				for (int slot = 3; slot < 7; ++slot)
					valid = valid && packet->Item[slot].sIndex >= 5110 && packet->Item[slot].sIndex <= 5133;
			}
			else
				valid = false;
			packet->Header.Type = MSG_CombineItemOdin_Opcode;
		}
		else
			valid = false;
		break;
	case 6:
		valid = hasItem(0) && hasItem(1) && hasItem(2);
		packet->Header.Type = MSG_CombineItemEhre_Opcode;
		break;
	default:
		valid = false;
		break;
	}

	if (!valid)
	{
		rejectRecipe();
		return;
	}

	bool hasCarryItem = false;
	for (int slot = 0; slot < GetNativeMixSlotCount(mixIndex); ++slot)
		hasCarryItem = hasCarryItem || packet->CarryPos[slot] >= 0;
	if (hasCarryItem)
		SendOneMessage(reinterpret_cast<char*>(packet), sizeof(*packet));
}

void TMFieldScene::SetVisibleNativeMix(int mixIndex, int bShow)
{
	auto panel = GetNativeMixPanel(mixIndex);
	if (!panel)
		return;

	SGridControl::m_sLastMouseOverIndex = -1;
	if (m_pInputGoldPanel && m_pInputGoldPanel->IsVisible())
		SetInVisibleInputCoin();

	if (bShow)
	{
		// The stock client exposes one artisan window at a time. Close and clear
		// every other native panel before assigning Carry to this panel's raw type.
		for (int other = 1; other <= 6; ++other)
		{
			if (other == mixIndex)
				continue;
			auto otherPanel = GetNativeMixPanel(other);
			if (otherPanel && otherPanel->IsVisible())
			{
				otherPanel->SetVisible(0);
				ClearNativeMix(other);
			}
		}

		if (m_pSkillPanel) m_pSkillPanel->SetVisible(0);
		if (m_pSkillMPanel) m_pSkillMPanel->SetVisible(0);
		if (m_pCPanel) m_pCPanel->SetVisible(0);
		if (m_pCargoPanel) m_pCargoPanel->SetVisible(0);
		if (m_pAutoTrade && m_pAutoTrade->IsVisible())
			SetVisibleAutoTrade(0, 0);
		if (m_pInvenPanel) m_pInvenPanel->SetVisible(1);
		PositionCompatNativeMixPanels();

		ClearNativeMix(mixIndex);
		panel->SetVisible(1);
		if (g_pDevice) g_pDevice->m_nWidthShift = 0;
		if (g_pCursor) g_pCursor->DetachItem();
		// Native 7.48 uses odd Carry types 13..23 for ItemMix1..6. The modern
		// enum names at these values are unrelated and must not drive dispatch.
		SetInventoryGridType(static_cast<TMEGRIDTYPE>(mixIndex * 2 + 11));
		SetEquipGridState(0);
	}
	else
	{
		panel->SetVisible(0);
		ClearNativeMix(mixIndex);
		if (m_pInvenPanel && m_pInvenPanel->IsVisible())
			m_pInvenPanel->SetVisible(0);
		if (g_pDevice) g_pDevice->m_nWidthShift = 0;
	}

	GetSoundAndPlay(51, 0, 0);
}

int TMFieldScene::TryStageNativeMixItem(SGridControl* sourceGrid,
	SGridControlItem* sourceItem, SGridControl* preferredTarget)
{
	if (!m_bCompatFieldScene || !sourceGrid || !sourceItem || !sourceItem->m_pItem)
		return -1;

	int activeMix = 0;
	for (int mixIndex = 1; mixIndex <= 6; ++mixIndex)
	{
		auto panel = GetNativeMixPanel(mixIndex);
		if (panel && panel->IsVisible())
		{
			activeMix = mixIndex;
			break;
		}
	}
	if (!activeMix)
		return -1;

	const int carrySlot = GetCarrySlotForCell(sourceGrid,
		sourceItem->m_nCellIndexX, sourceItem->m_nCellIndexY);
	if (carrySlot < 0)
		return -1;
	if (sourceItem->m_GCObj.dwColor != 0xFFFFFFFF)
		return 1;

	int targetSlot = -1;
	for (int slot = 0; slot < GetNativeMixSlotCount(activeMix); ++slot)
	{
		auto targetGrid = GetNativeMixGrid(activeMix, slot);
		if (preferredTarget && targetGrid == preferredTarget)
		{
			targetSlot = slot;
			break;
		}
		if (!preferredTarget && targetSlot < 0 && targetGrid && !targetGrid->GetItem(0, 0))
			targetSlot = slot;
	}
	if (targetSlot < 0)
		return preferredTarget ? -1 : 1;

	auto targetGrid = GetNativeMixGrid(activeMix, targetSlot);
	if (!targetGrid || targetGrid->GetItem(0, 0))
		return 1;
	auto packet = GetNativeMixPacket(activeMix);
	if (!packet)
		return 1;

	auto itemCopy = new STRUCT_ITEM;
	if (!itemCopy)
		return 1;
	memcpy(itemCopy, sourceItem->m_pItem, sizeof(*itemCopy));
	auto stagedItem = new SGridControlItem(nullptr, itemCopy, 0.0f, 0.0f);
	if (!stagedItem)
	{
		delete itemCopy;
		return 1;
	}
	// Publish the staged item only after the grid takes ownership; on failure,
	// the original item remains on the cursor without a partial local packet mutation.
	if (targetGrid->AddItem(stagedItem, 0, 0) != 1)
	{
		SAFE_DELETE(stagedItem);
		return 1;
	}
	memcpy(&packet->Item[targetSlot], sourceItem->m_pItem, sizeof(packet->Item[targetSlot]));
	packet->CarryPos[targetSlot] = static_cast<char>(carrySlot);
	sourceItem->m_GCObj.dwColor = 0xFFFF0000;
	if (g_pCursor) g_pCursor->DetachItem();
	return 1;
}

int TMFieldScene::TryRemoveNativeMixItem(SGridControl* mixGrid)
{
	if (!m_bCompatFieldScene || !mixGrid)
		return -1;

	for (int mixIndex = 1; mixIndex <= 6; ++mixIndex)
	{
		auto panel = GetNativeMixPanel(mixIndex);
		if (!panel || !panel->IsVisible())
			continue;

		for (int slot = 0; slot < GetNativeMixSlotCount(mixIndex); ++slot)
		{
			if (GetNativeMixGrid(mixIndex, slot) != mixGrid)
				continue;

			auto packet = GetNativeMixPacket(mixIndex);
			auto stagedItem = mixGrid->PickupItem(0, 0);
			if (!stagedItem || !packet)
				return 1;

			const int carrySlot = packet->CarryPos[slot];
			if (carrySlot >= 0)
			{
				int carryX = 0;
				int carryY = 0;
				GetCarryCellForSlot(carrySlot, carryX, carryY);
				auto carry = GetCarryGridForSlot(carrySlot);
				auto sourceItem = carry ? carry->GetAtItem(carryX, carryY) : nullptr;
				if (sourceItem)
					sourceItem->m_GCObj.dwColor = 0xFFFFFFFF;
			}
			memset(&packet->Item[slot], 0, sizeof(packet->Item[slot]));
			packet->CarryPos[slot] = -1;
			if (g_pCursor && g_pCursor->m_pAttachedItem == stagedItem)
				g_pCursor->m_pAttachedItem = nullptr;
			SAFE_DELETE(stagedItem);
			return 1;
		}
	}

	return -1;
}

void TMFieldScene::ClearCombine()
{
	ClearNativeMix(1);
}

void TMFieldScene::ClearCombine2()
{
	ClearNativeMix(2);
}

void TMFieldScene::ClearCombine3()
{
	ClearNativeMix(3);
}

void TMFieldScene::ClearCombine4()
{
	ClearNativeMix(4);
}

void TMFieldScene::ClearCombine5()
{
	ClearNativeMix(5);
}

void TMFieldScene::ClearCombine6()
{
	ClearNativeMix(6);
}

void TMFieldScene::DoCombine()
{
	DoNativeMix(1);
}

void TMFieldScene::DoCombine2()
{
	DoNativeMix(2);
}

void TMFieldScene::DoCombine3()
{
	DoNativeMix(3);
}

void TMFieldScene::DoCombine4()
{
	DoNativeMix(4);
}

void TMFieldScene::DoCombine5()
{
	DoNativeMix(5);
}

void TMFieldScene::DoCombine6()
{
	DoNativeMix(6);
}

void TMFieldScene::SetVisibleMixItem(int bShow)
{
	SetVisibleNativeMix(1, bShow);
}

void TMFieldScene::SetVisibleMixItem2(int bShow)
{
	SetVisibleNativeMix(2, bShow);
}

void TMFieldScene::SetVisibleMixItem3(int bShow)
{
	SetVisibleNativeMix(3, bShow);
}

void TMFieldScene::SetVisibleMixItemTiini(int bShow)
{
	SetVisibleNativeMix(4, bShow);
}

void TMFieldScene::SetVisibleMixItem5(int bShow)
{
	SetVisibleNativeMix(5, bShow);
}

void TMFieldScene::SetVisibleMixItem6(int bShow)
{
	SetVisibleNativeMix(6, bShow);
}

int TMFieldScene::OnPacketCombineComplete(MSG_STANDARD* pStd)
{
	if (m_bCompatFieldScene)
	{
		// The acknowledgement belongs to whichever one of the six stock artisan
		// windows is active; never close the unrelated generic 7.59 mix panel.
		for (int mixIndex = 1; mixIndex <= 6; ++mixIndex)
		{
			auto panel = GetNativeMixPanel(mixIndex);
			if (panel && panel->IsVisible())
			{
				SetVisibleNativeMix(mixIndex, 0);
				break;
			}
		}
	}
	else
	{
		SetVisibleMixItem(0);
		SetVisibleMixItemTiini(0);
		SetVisibleMixPanel(0);
	}
	return 1;
}

void TMFieldScene::SetVisibleMixPanel(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;
	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);

	if (bShow == 1)
	{
		if (m_pInvenPanel)
		{
			if (!m_pInvenPanel->m_bVisible)
				SetVisibleInventory();
		}

		m_ItemMixClass.m_pMixPanel->SetVisible(1);
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 650.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

		g_pDevice->m_nWidthShift = 0;
		g_pCursor->DetachItem();
		SetInventoryGridType(TMEGRIDTYPE::GRID_TRADEINV8);
		SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_ItemMixClass.m_pMixPanel->SetVisible(0);
		ClearMixPannel();
	}
}

void TMFieldScene::ClearMixPannel()
{
	// Clear mix highlighting across the native 9x7 Carry grid or the newer
	// paged topology, whichever the loaded UI resource actually supplies.
	const int carryPages = m_bCompatFieldScene ? 1 : 4;
	const int carryRows = m_bCompatFieldScene ? 7 : 3;
	const int carryColumns = m_bCompatFieldScene ? 9 : 5;
	for (int i = 0; i < carryPages; ++i)
	{
		auto pGridInv = m_bCompatFieldScene ? m_pGridInv : m_pGridInvList[i];
		if (!pGridInv)
			continue;
		for (int nY = 0; nY < carryRows; ++nY)
		{
			for (int nX = 0; nX < carryColumns; ++nX)
			{
				auto pItem = pGridInv->GetItem(nX, nY);
				if (pItem)
				{
					if (pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
	}

	m_ItemMixClass.ClearGridList();
	m_ItemMixClass.m_stCombineItem.Header.ID = m_pMyHuman->m_dwID;
	m_ItemMixClass.m_stCombineItem.Header.Type = MSG_CombineItemTiny_Opcode;

	// FieldScene2.bin binds only the native 7.48 Carry grid; newer resources
	// can bind four pages, so reset every control that actually exists.
	for (auto pGridInv : m_pGridInvList)
	{
		if (pGridInv)
			pGridInv->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	}
	SetEquipGridState(1);
}

void TMFieldScene::SetVisibleMissionPanel(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;
	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);

	if (bShow == 1)
	{
		if (m_pInvenPanel)
		{
			if (!m_pInvenPanel->m_bVisible)
				SetVisibleInventory();
		}

		m_MissionClass.m_pMissionPanel->SetVisible(1);
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 650.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

		g_pDevice->m_nWidthShift = 0;
		g_pCursor->DetachItem();
		SetInventoryGridType(TMEGRIDTYPE::GRID_TRADEINV3);
		SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_MissionClass.m_pMissionPanel->SetVisible(0);
		ClearMissionPannel();
	}
}

void TMFieldScene::ClearMissionPannel()
{
	// Mission cleanup follows the same native single-grid Carry contract.
	const int carryPages = m_bCompatFieldScene ? 1 : 4;
	const int carryRows = m_bCompatFieldScene ? 7 : 3;
	const int carryColumns = m_bCompatFieldScene ? 9 : 5;
	for (int i = 0; i < carryPages; ++i)
	{
		auto pGridInv = m_bCompatFieldScene ? m_pGridInv : m_pGridInvList[i];
		if (!pGridInv)
			continue;
		for (int nY = 0; nY < carryRows; ++nY)
		{
			for (int nX = 0; nX < carryColumns; ++nX)
			{
				auto pItem = pGridInv->GetItem(nX, nY);
				if (pItem)
				{
					if (pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
	}

	m_MissionClass.ClearGridList();
	m_MissionClass.m_stCombineItem.Header.ID = m_pMyHuman->m_dwID;
	// Mission is not part of the native 7.48 combine ABI. Keep it unavailable
	// until a distinct, coordinated client/server contract is implemented.
	m_MissionClass.m_stCombineItem.Header.Type = 0;
	// Mission cleanup must not manufacture the three 7.59 Carry pages that
	// do not exist in the stock 7.48 control tree.
	for (auto pGridInv : m_pGridInvList)
	{
		if (pGridInv)
			pGridInv->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	}
	SetEquipGridState(1);
}

int TMFieldScene::MouseClick_MixNPC(TMHuman* pOver)
{
	if (!pOver || !m_pGround || pOver->m_dwID < 1000)
		return 0;

	const int chunkX = static_cast<int>(m_pGround->m_vecOffsetIndex.x);
	const int chunkY = static_cast<int>(m_pGround->m_vecOffsetIndex.y);
	auto toggleMix = [this](int mixIndex)
	{
		auto panel = GetNativeMixPanel(mixIndex);
		if (!panel)
			return false;
		SetVisibleNativeMix(mixIndex, panel->IsVisible() == 0);
		return true;
	};

	// FUN_0047e4d6 checks these routes in this exact order. Head alone is not
	// sufficient because 7.48 deliberately reuses Lindy/Odin and Tiny/Ehre skins.
	if (pOver->m_sHeadIndex == 67 && chunkX == 13 && chunkY == 13)
	{
		g_nCombineMode = 0;
		return toggleMix(5) ? 1 : 0; // Lindy
	}
	if (pOver->m_sHeadIndex == 67 && chunkX == 28 && chunkY == 24 && pOver->m_dwID != 1033)
	{
		g_nCombineMode = 1;
		return toggleMix(5) ? 1 : 0; // dormant stock 0x2C4 mode
	}
	if (pOver->m_sHeadIndex == 54 && chunkX == 19 && chunkY == 13)
	{
		g_nCombineMode = 0;
		return toggleMix(1) ? 1 : 0; // Compositor, normal
	}
	if (pOver->m_sHeadIndex == 54 && chunkX == 25 && chunkY == 13)
	{
		g_nCombineMode = 1;
		return toggleMix(1) ? 1 : 0; // Compositor, six-slot mode
	}
	if (pOver->m_sHeadIndex == 55)
		return toggleMix(2) ? 1 : 0; // Aylin
	if (pOver->m_sHeadIndex == 56)
		return toggleMix(3) ? 1 : 0; // Agatha
	if (pOver->m_sHeadIndex == 68 && chunkX == 19 && chunkY == 15)
		return toggleMix(6) ? 1 : 0; // Ehre
	if (pOver->m_sHeadIndex == 68)
		return toggleMix(4) ? 1 : 0; // Tiny
	if (pOver->m_sHeadIndex == 67 && (pOver->m_stScore.Merchant & 0xF) == 8 &&
		chunkX == 25 && chunkY == 13)
	{
		g_nCombineMode = 2;
		return toggleMix(5) ? 1 : 0; // Odin
	}

	return 0;
}
