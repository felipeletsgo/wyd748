#include "pch.h"
// SGridControl split by responsibility; see SGrid.cpp for the grid model.
#include "SGrid.h"
#include "GridInsertion.h"
#include "../application/NativeVolatileRoutes.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMMesh.h"
#include "TMFieldScene.h"
#include "TMUtil.h"
#include "ItemEffect.h"
#include "ClientDiagnostics.h"
#include "SellConfirmationText.h"
#include "NativeSaleQuote.h"
#include "SGridSupport.h"


void SGridControl::BuyItem(int nCellX, int nCellY)
{
	auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
		{

			// WYD748: the shop packet addresses the native 27-slot, 9-column
			// shop directly; the 7.59 5-column/page remap selected wrong goods.
			int SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
			int nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
			if (nGridIndex > 7 || nGridIndex < 0)
				nGridIndex = 0;

			IVector2 vecGrid;
			auto pMyGrid = pScene->m_pGridInv;
			if (!pMyGrid)
				return;

			vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);

			MSG_Buy stBuy{};
			stBuy.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
			stBuy.Header.Type = MSG_Buy_Opcode;
			stBuy.TargetID = pScene->m_sShopTarget;

			stBuy.TargetCarryPos = SourPos;
			// A 7.48 purchase targets the first fitting cell in the sole 9-column
			// Carry control; there is no modern page number in MSG_Buy.
			stBuy.MyCarryPos = vecGrid.x + 9 * vecGrid.y;

			if (vecGrid.x > -1 && vecGrid.y > -1)
			{
				if (pItem->m_pItem->sIndex == 4147)
				{
					memset(&pScene->m_stToto, 0, sizeof(pScene->m_stToto));
					pScene->m_stToto.Header.Size = sizeof(pScene->m_stToto);
					pScene->m_stToto.Header.ID = stBuy.Header.ID;
					pScene->m_stToto.Header.Type = MSG_BuyToto_Opcode;
					pScene->m_stToto.TargetID = stBuy.TargetID;
					pScene->m_stToto.TargetCarryPos = static_cast<short>(stBuy.TargetCarryPos);
					pScene->m_stToto.MyCarryPos = static_cast<short>(stBuy.MyCarryPos);
					if (pScene->m_pTotoPanel)
					{
						if (pScene->m_pShopPanel && pScene->m_pShopPanel->IsVisible())
							pScene->SetVisibleShop(0);

						if (pScene->m_pDescPanel && pScene->m_pDescPanel->IsVisible())
							pScene->m_pDescPanel->SetVisible(0);

						pScene->m_pTotoPanel->SetVisible(1);
						if (pScene->m_pTotoNumber_Edit)
							pScene->m_pTotoNumber_Edit->SetText((char*)"");
						if (pScene->m_pTotoScoreA_Edit)
							pScene->m_pTotoScoreA_Edit->SetText((char*)"");
						if (pScene->m_pTotoScoreB_Edit)
							pScene->m_pTotoScoreB_Edit->SetText((char*)"");
						if (pScene->m_pControlContainer && pScene->m_pTotoNumber_Edit)
							pScene->m_pControlContainer->SetFocusedControl(pScene->m_pTotoNumber_Edit);
					}
					return;
				}

				SendOneMessage((char*)&stBuy, 24);
			}
			else
			{
				auto pListBox = pScene->m_pChatList;
				auto ipNewItem = new SListBoxItem(g_pMessageStringTable[1],
					0xFFFFAAAA,
					0.0f,
					0.0f,
					280.0f,
					16.0f,
					0,
					0x77777777,
					1u,
					0);

				if (ipNewItem)
					pListBox->AddItem(ipNewItem);

				auto pSoundManager = g_pSoundManager;
				if (pSoundManager && pSoundManager->GetSoundData(33))
				{
					pSoundManager->GetSoundData(33)->Play();
				}
			}
		}
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_SKILLM)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
		{
			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[47], g_pItemList[pItem->m_pItem->sIndex].Name);
			g_pCurrentScene->m_pMessageBox->SetMessage(szMsg, 4, 0);
			g_pCurrentScene->m_pMessageBox->SetVisible(1);
			g_pCurrentScene->m_pMessageBox->m_dwArg = pScene->m_sShopTarget | (pItem->m_pItem->sIndex << 16);
		}
	}
}

int SGridControl::TradeItem(int nCellX, int nCellY)
{
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (!pFScene || !pFScene->m_pControlContainer || !g_pObjectManager)
		return 0;
	// ItemMix2..6 use raw grid values that the imported source later renamed as
	// cargo/quickslot modes. Resolve the visible 7.48 panel before enum dispatch.
	auto nativeMixItem = GetItem(nCellX, nCellY);
	int nativeMixResult = pFScene->TryStageNativeMixItem(this, nativeMixItem, nullptr);
	if (nativeMixResult >= 0)
		return nativeMixResult;
	nativeMixResult = pFScene->TryRemoveNativeMixItem(this);
	if (nativeMixResult >= 0)
		return nativeMixResult;

	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV)
	{
		return TradeItemOnTradeInv(nCellX, nCellY, pFScene);
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV2)
	{
		return TradeItemOnTradeInv2(nCellX, nCellY, pFScene);
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV3)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
		{
			// Missions consume Carry page zero because the 7.48 client has no page
			// selector or secondary inventory controls.
			pFScene->m_MissionClass.ClickInvItem(pItem, pFScene->m_pGridInvList, 0);
		}
		return 2;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV6)
	{
		return TradeItemOnTradeInv6(nCellX, nCellY, pFScene);
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV8)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
		{
			// Item mix uses the sole native 7.48 Carry page; deriving a page from
			// 67072..67075 could select controls absent from FieldScene2.bin.
			pFScene->m_ItemMixClass.ClickInvItem(pItem, pFScene->m_pGridInvList, 0);
		}
		return 2;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEMY)
	{
		return TradeItemOnTradeMy(pFScene);
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_ITEMMIX)
	{
		return TradeItemOnItemMix(nCellX, nCellY, pFScene);
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_ITEMMIX4)
	{
		return TradeItemOnItemMix4(nCellX, nCellY, pFScene);
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEMY2)
	{
		return TradeItemOnTradeMy2(nCellX, nCellY, pFScene);
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP || m_eGridType == TMEGRIDTYPE::GRID_SKILLM ||
		m_eGridType == TMEGRIDTYPE::GRID_ITEMMIXRESULT || m_eGridType == TMEGRIDTYPE::GRID_ITEMMIXNEED ||
		m_eGridType == TMEGRIDTYPE::GRID_MISSION_RESULT || m_eGridType == TMEGRIDTYPE::GRID_MISSION_NEED ||
		m_eGridType == TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		return 2;
	}

	auto pItem = SelectItem(nCellX, nCellY);
	if (pItem && IsPassiveSkill(pItem->m_pItem->sIndex) == 1)
		return 1;

	if (pItem && pItem->m_GCObj.dwColor != 0xFFFF0000)
		g_pCursor->AttachItem(pItem);

	return 1;
}

// Extracted from SGridControl::TradeItem; behavior is unchanged.
int SGridControl::TradeItemOnTradeInv(int& nCellX, int& nCellY, TMFieldScene*& pFScene)
{
	if (pFScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD ||
		!pFScene->m_pMyHuman || !pFScene->m_pTradePanel ||
		!pFScene->m_pTradePanel->IsVisible() ||
		!g_pObjectManager->m_stTrade.OpponentID ||
		!g_pApp || !g_pApp->m_pTimerManager)
		return 0;

	auto pItem = GetItem(nCellX, nCellY);
	if (pItem && pItem->m_pItem && pItem->m_GCObj.dwColor == 0xFFFFFFFF)
	{
		int SourPos = pFScene->GetCarrySlotForCell(this,
			pItem->m_nCellIndexX, pItem->m_nCellIndexY);
		if (SourPos < 0 || SourPos >= MAX_VISIBLE_CARRY)
			return 2;

		// The grid item is only a presentation copy.  First prove that the cell
		// still resolves to the same authoritative Carry entry.  A rebuilt/stale
		// inventory surface may retain the visual item while its cell metadata no
		// longer points at the slot that contains it.
		int authoritativeSlot = SourPos;
		if (memcmp(pItem->m_pItem,
			&g_pObjectManager->m_stMobData.Carry[authoritativeSlot],
			sizeof(STRUCT_ITEM)) != 0)
		{
			authoritativeSlot = -1;
			int matchingSlots = 0;
			for (int slot = 0; slot < MAX_VISIBLE_CARRY; ++slot)
			{
				if (memcmp(pItem->m_pItem, &g_pObjectManager->m_stMobData.Carry[slot],
					sizeof(STRUCT_ITEM)) == 0)
				{
					++matchingSlots;
					authoritativeSlot = slot;
				}
			}
			if (matchingSlots != 1)
			{
				WYD748_DiagnosticsLog("TRADE_CLICK %s item=%d cell=%d,%d expected=%d matches=%d\r\n",
					matchingSlots == 0 ? "stale" : "ambiguous",
					pItem->m_pItem->sIndex, pItem->m_nCellIndexX, pItem->m_nCellIndexY, SourPos, matchingSlots);
				return 2;
			}
		}

		const STRUCT_ITEM& sourceItem = g_pObjectManager->m_stMobData.Carry[authoritativeSlot];
		if (sourceItem.sIndex <= 0)
			return 2;

		SGridControl* pGridMyItem[15];
		for (size_t i = 0; i < 15; ++i)
			pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(i + TMG_TRADE_MY1);

		bool bEmptyFind = false;
		for (size_t i = 0; i < 15; ++i)
		{
			if (pGridMyItem[i] && !pGridMyItem[i]->GetItem(0, 0))
			{
				auto pstItem = new STRUCT_ITEM;
				if (!pstItem)
					return 0;
				memcpy(pstItem, &sourceItem, sizeof(STRUCT_ITEM));

				auto newItem = new SGridControlItem(nullptr, pstItem, 0.0f, 0.0f);
				if (!newItem)
				{
					delete pstItem;
					return 0;
				}
				// Without an accepted visual, do not publish an offer or mark Carry used.
				if (!pGridMyItem[i]->AddItem(newItem, 0, 0))
				{
					delete newItem;
					return 0;
				}

				memcpy(&g_pObjectManager->m_stTrade.Item[i], &sourceItem, sizeof(STRUCT_ITEM));

				g_pObjectManager->m_stTrade.CarryPos[i] = authoritativeSlot;
				pItem->m_GCObj.dwColor = 0xFFFF0000;
				WYD748_DiagnosticsLog("TRADE_CLICK cell=%d,%d visual=%d carry=%d trade=%d\r\n",
					nCellX, nCellY, pItem->m_pItem->sIndex, authoritativeSlot, i);
				bEmptyFind = true;
				break;
			}
		}
		if (!bEmptyFind)
			return 0;

		auto pMyCheck = (SButton*)pFScene->m_pControlContainer->FindControl(617);
		auto pOPCheck = (SButton*)pFScene->m_pControlContainer->FindControl(601);
		if (pMyCheck)
			pMyCheck->m_bSelected = 0;
		if (pOPCheck)
			pOPCheck->m_bSelected = 0;

		pFScene->m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
		g_pObjectManager->m_stTrade.MyCheck = pMyCheck ? pMyCheck->m_bSelected : 0;
		g_pObjectManager->m_stTrade.Header.ID = pFScene->m_pMyHuman->m_dwID;
		g_pObjectManager->m_stTrade.Header.Type = MSG_Trade_Opcode;
		WYD748_DiagnosticsLog(
			"TRADE_SEND origin=item opponent=%u carry0=%d item0=%d size=%u\r\n",
			g_pObjectManager->m_stTrade.OpponentID,
			static_cast<int>(g_pObjectManager->m_stTrade.CarryPos[0]),
			g_pObjectManager->m_stTrade.Item[0].sIndex,
			static_cast<unsigned int>(sizeof(g_pObjectManager->m_stTrade)));
		SendOneMessage((char*)&g_pObjectManager->m_stTrade, sizeof(g_pObjectManager->m_stTrade));
		return 1;
	}
	return 2;

}

// Extracted from SGridControl::TradeItem; behavior is unchanged.
int SGridControl::TradeItemOnTradeInv2(int& nCellX, int& nCellY, TMFieldScene*& pFScene)
{
	if (pFScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && pFScene->m_pCargoGrid &&
		pFScene->m_pControlContainer)
	{
		auto pInputGold = (SPanel*)pFScene->m_pControlContainer->FindControl(65885);
		// Select from the grid that actually received the click.  The old code
		// always queried m_pCargoGrid, so an overlapping/translated native
		// surface could hit visually yet resolve no item for AutoTrade.
		auto pItem = GetItem(nCellX, nCellY);
		auto pRunAutoTrade = (SButton*)pFScene->m_pControlContainer->FindControl(667);

		if (!pInputGold || !pRunAutoTrade || !pRunAutoTrade->IsVisible())
			return 1;

		if (!pItem || !pItem->m_pItem)
			return 1;

		if (BASE_GetItemAbility(pItem->m_pItem, 111))
		{
			pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[309], 2000);
			pFScene->m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		// Native 7.48 FUN_004110f5 rejects 508, 509, 522, 526..537, 747
		// and 3200..3299. Item 4905 belonged to the imported newer client rule.
		if (pItem->m_pItem->sIndex == 508 || pItem->m_pItem->sIndex == 509 || pItem->m_pItem->sIndex == 522 ||
			pItem->m_pItem->sIndex >= 526 && pItem->m_pItem->sIndex <= 537 || pItem->m_pItem->sIndex == 747 ||
			pItem->m_pItem->sIndex >= 3200 && pItem->m_pItem->sIndex <= 3299)
		{
			pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[309], 2000);
			pFScene->m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}

		if (!pInputGold->IsVisible() && pItem->m_GCObj.dwColor == -1)
		{
			auto pText = (SText*)pFScene->m_pControlContainer->FindControl(65888);
			auto pEdit = (SEditableText*)pFScene->m_pControlContainer->FindControl(65889);
			if (!pText || !pEdit)
				return 1;
			// Resolve the packet slot from the clicked native grid so the visual
			// cell and the Cargo wire position cannot diverge.
			const int cargoSlot = pFScene->GetCargoSlotForCell(
				this, pItem->m_nCellIndexX, pItem->m_nCellIndexY);
			if (cargoSlot < 0)
				return 1;

			// Commit selection only after all prompt dependencies are valid.
			pItem->m_GCObj.dwColor = 0xFFFF00FF;
			pFScene->m_nCoinMsgType = 4;
			pFScene->m_nLastAutoTradePos = cargoSlot;

			pText->SetText(g_pMessageStringTable[142], 0);

			pFScene->m_pControlContainer->SetFocusedControl(pEdit);

			memset(pEdit->m_strComposeText, 0, sizeof(pEdit->m_strComposeText));
			pEdit->SetText((char*)"");
			pInputGold->SetVisible(1);
			// FieldScene2.bin has no 7.59 chat selector. The native 7.48 path
			// opens only input panel 626, edit 627 and caption 630 here.
			if (pFScene->m_pChatSelectPanel)
				pFScene->m_pChatSelectPanel->SetVisible(0);
		}
	}
	return 1;

}

// Extracted from SGridControl::TradeItem; behavior is unchanged.
int SGridControl::TradeItemOnTradeInv6(int& nCellX, int& nCellY, TMFieldScene*& pFScene)
{
	auto pItem = GetItem(nCellX, nCellY);

	if (!pItem || pItem->m_GCObj.dwColor != 0xFFFFFFFF)
		return 2;

	int SourPos = pFScene->GetCarrySlotForCell(this,
		pItem->m_nCellIndexX, pItem->m_nCellIndexY);

	SGridControl* pGridMyItem[3];
	for (size_t i = 0; i < 3; i++)
		pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(i + 6436);

	float nY = 0.0f;
	float nX = 0.0f;
	bool bEmptyFind = false;
	for (int i = 0; i < 3; i++)
	{
		if (!pGridMyItem[i] || pGridMyItem[i]->GetItem(0, 0))
			continue;

		if (i == 0)
		{
			int target1trans = BASE_GetItemAbility(pItem->m_pItem, 112);
			int target1pos = g_pItemList[pItem->m_pItem->sIndex].nPos;
			int target1look = BASE_GetItemAbility(pItem->m_pItem, 18);
			int target1Level = BASE_GetItemAbility(pItem->m_pItem, 87);
			int target1Unique = g_pItemList[pItem->m_pItem->sIndex].nUnique;

			bool bOK = true;

			if (target1trans != 1)
				bOK = false;
			if (!bOK)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[273], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			if (target1Level != 6)
				bOK = false;
			if (!bOK)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[302], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			if (target1pos & 0x3F)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[276], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
		}
		if (i == 1)
		{
			int target2trans = BASE_GetItemAbility(pItem->m_pItem, 112);
			int target2grade = g_pItemList[pItem->m_pItem->sIndex].nUnique / 10;
			int target2pos = g_pItemList[pItem->m_pItem->sIndex].nPos;
			int target2look = BASE_GetItemAbility(pItem->m_pItem, 18);
			int target2Unique = g_pItemList[pItem->m_pItem->sIndex].nUnique;
			int target2Level = BASE_GetItemAbility(pItem->m_pItem, 87);

			bool bOK = true;

			if (target2grade < 4)
				bOK = false;
			if (!bOK)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[276], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			if (target2Level != 6)
				bOK = false;
			if (!bOK)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[302], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			auto pItem1 = pGridMyItem[0]->GetItem(0, 0);
			int target1grade = g_pItemList[pItem1->m_pItem->sIndex].nUnique % 10;
			int nPos = g_pItemList[pItem1->m_pItem->sIndex].nPos;
			int nClass = BASE_GetItemAbility(pItem1->m_pItem, 18);
			int nUnique = g_pItemList[pItem1->m_pItem->sIndex].nUnique;

			if (nUnique != target2Unique)
				bOK = nUnique == 45 && target2Unique == 48 && nPos == target2pos;
			if (nPos != target2pos)
				bOK = false;

			if (!bOK)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[276], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
		}
		if (i == 2)
		{
			nX = 13.0f;
			nY = 28.0f;
		}

		if (i == 0 || i == 1 || i == 2)
		{
			int nItemSanc = BASE_GetItemSanc(pItem->m_pItem);
			if (nItemSanc < 9 || !g_pItemList[pItem->m_pItem->sIndex].nPos)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[192], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
		}

		auto dst = new STRUCT_ITEM;
		if (!dst)
			return 0;
		memcpy(dst, pItem->m_pItem, sizeof(STRUCT_ITEM));

		auto newItem = new SGridControlItem(nullptr, dst, nX, nY);
		if (!newItem)
		{
			delete dst;
			return 0;
		}
		// Composition claims the slot only after transferring the visual.
		if (!pGridMyItem[i]->AddItem(newItem, 0, 0))
		{
			delete newItem;
			return 0;
		}

		memcpy(&g_pObjectManager->m_stCombineItem4.Item[i], pItem->m_pItem, sizeof(STRUCT_ITEM));
		g_pObjectManager->m_stCombineItem4.CarryPos[i] = SourPos;
		pItem->m_GCObj.dwColor = 0xFFFF0000;
		bEmptyFind = true;
		break;
	}

	return bEmptyFind != 0;

}

// Extracted from SGridControl::TradeItem; behavior is unchanged.
int SGridControl::TradeItemOnTradeMy(TMFieldScene*& pFScene)
{
	if (pFScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD ||
		!pFScene->m_pMyHuman || !pFScene->m_pTradePanel ||
		!pFScene->m_pTradePanel->IsVisible() ||
		!g_pObjectManager->m_stTrade.OpponentID ||
		!g_pApp || !g_pApp->m_pTimerManager)
		return 0;

	SGridControl* pGridMyItem[15]{};
	for (int i = 0; i < 15; ++i)
		pGridMyItem[i] = static_cast<SGridControl*>(
			pFScene->m_pControlContainer->FindControl(i + TMG_TRADE_MY1));

	for (int i = 0; i < 15; ++i)
	{
		if (pGridMyItem[i] != this)
			continue;

		const int carrySlot = g_pObjectManager->m_stTrade.CarryPos[i];
		if (carrySlot >= 0 && carrySlot < MAX_VISIBLE_CARRY)
		{
			int cellX = 0;
			int cellY = 0;
			pFScene->GetCarryCellForSlot(carrySlot, cellX, cellY);
			auto pCarryGrid = pFScene->GetCarryGridForSlot(carrySlot);
			auto pCarryItem = pCarryGrid ? pCarryGrid->GetItem(cellX, cellY) : nullptr;
			if (pCarryItem)
				pCarryItem->m_GCObj.dwColor = 0xFFFFFFFF;
		}

		auto pTradeItem = pGridMyItem[i]->PickupItem(0, 0);
		if (g_pCursor && g_pCursor->m_pAttachedItem == pTradeItem)
			g_pCursor->m_pAttachedItem = nullptr;
		SAFE_DELETE(pTradeItem);

		memset(&g_pObjectManager->m_stTrade.Item[i], 0,
			sizeof(g_pObjectManager->m_stTrade.Item[i]));
		g_pObjectManager->m_stTrade.CarryPos[i] = -1;

		auto pMyCheck = static_cast<SButton*>(
			pFScene->m_pControlContainer->FindControl(TMB_TRADE_MYCHECK));
		auto pOPCheck = static_cast<SButton*>(
			pFScene->m_pControlContainer->FindControl(TMB_TRADE_OPCHECK));
		if (pMyCheck)
			pMyCheck->m_bSelected = 0;
		if (pOPCheck)
			pOPCheck->m_bSelected = 0;

		pFScene->m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
		g_pObjectManager->m_stTrade.MyCheck = 0;
		g_pObjectManager->m_stTrade.Header.ID = pFScene->m_pMyHuman->m_dwID;
		g_pObjectManager->m_stTrade.Header.Type = MSG_Trade_Opcode;
		WYD748_DiagnosticsLog(
			"TRADE_SEND origin=remove opponent=%u slot=%d carry=%d size=%u\r\n",
			g_pObjectManager->m_stTrade.OpponentID, i, carrySlot,
			static_cast<unsigned int>(sizeof(g_pObjectManager->m_stTrade)));
		SendOneMessage(reinterpret_cast<char*>(&g_pObjectManager->m_stTrade),
			sizeof(g_pObjectManager->m_stTrade));
		return 1;
	}
	return 0;

}

// Extracted from SGridControl::TradeItem; behavior is unchanged.
int SGridControl::TradeItemOnItemMix(int& nCellX, int& nCellY, TMFieldScene*& pFScene)
{
	auto pItem = GetItem(nCellX, nCellY);
	if (!pItem)
		return 1;

	// MSG_CombineItem and the panel initialization both define eight recipe
	// slots; scanning thirteen controls indexed beyond both packet arrays.
	SGridControl* pGridMyItem[8];
	for (size_t i = 0; i < 8; ++i)
		pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(i + 65861);

	for (size_t i = 0; i < 8; ++i)
	{
		if (pGridMyItem[i] != this)
			continue;

		// WYD748 compatibility: resolve recipe slots through the native 9-column Carry topology before restoring their color.
		int CellIndexX = 0;
		int CellIndexY = 0;
		pFScene->GetCarryCellForSlot(g_pObjectManager->m_stCombineItem.CarryPos[i], CellIndexX, CellIndexY);

		auto pPickupItem = pGridMyItem[i]->PickupItem(0, 0);
		auto pControlInv = pFScene->GetCarryGridForSlot(g_pObjectManager->m_stCombineItem.CarryPos[i]);

		auto pItemInv = pControlInv != nullptr ? pControlInv->GetItem(CellIndexX, CellIndexY) : nullptr;
		if (pItemInv != nullptr)
			pItemInv->m_GCObj.dwColor = 0xFFFFFFFF;

		g_pObjectManager->m_stCombineItem.CarryPos[i] = -1;

		memset(&g_pObjectManager->m_stCombineItem.Item[i], 0, sizeof(g_pObjectManager->m_stCombineItem.Item[i]));
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickupItem)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pPickupItem);
		break;
	}
	return 1;

}

// Extracted from SGridControl::TradeItem; behavior is unchanged.
int SGridControl::TradeItemOnItemMix4(int& nCellX, int& nCellY, TMFieldScene*& pFScene)
{
	auto pItem = GetItem(nCellX, nCellY);
	if (!pItem)
		return 1;

	SGridControl* pGridMyItem[3];
	for (size_t i = 0; i < 3; ++i)
		pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(i + 6436);

	for (size_t i = 0; i < 3; ++i)
	{
		if (pGridMyItem[i] != this)
			continue;

		// WYD748 compatibility: the three-slot mix panel shares the same native Carry slot resolver.
		int CellIndexX = 0;
		int CellIndexY = 0;
		pFScene->GetCarryCellForSlot(g_pObjectManager->m_stCombineItem4.CarryPos[i], CellIndexX, CellIndexY);

		auto pPickupItem = pGridMyItem[i]->PickupItem(0, 0);
		auto pControlInv = pFScene->GetCarryGridForSlot(g_pObjectManager->m_stCombineItem4.CarryPos[i]);

		auto pItemInv = pControlInv != nullptr ? pControlInv->GetItem(CellIndexX, CellIndexY) : nullptr;
		if (pItemInv != nullptr)
			pItemInv->m_GCObj.dwColor = 0xFFFFFFFF;

		g_pObjectManager->m_stCombineItem4.CarryPos[i] = -1;

		memset(&g_pObjectManager->m_stCombineItem4.Item[i], 0, sizeof(g_pObjectManager->m_stCombineItem4.Item[i]));
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickupItem)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pPickupItem);
	}

	return 1;

}

// Extracted from SGridControl::TradeItem; behavior is unchanged.
int SGridControl::TradeItemOnTradeMy2(int& nCellX, int& nCellY, TMFieldScene*& pFScene)
{
	auto pItem = GetItem(nCellX, nCellY);
	if (!pItem)
		return 1;

	auto pGrid = pFScene->m_pGridInv;
	if (!pGrid)
		return 1;

	int nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
	if (nGridIndex < 0 || nGridIndex > 7)
		nGridIndex = 0;

	IVector2 vecGrid = pGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);

	// Trade return validation must inspect the one native Carry grid. Iterating
	// four 7.59 pages could approve a return into storage that 7.48 cannot show.
	if ((vecGrid.x > -1 && vecGrid.y > -1) || BASE_GetItemAbility(pItem->m_pItem, 38) == 2)
	{
		if (pFScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
		{
			// The native 7.48 flow confirms control 646 before sending MSG_ReqBuy.
			// The server remains authoritative and revalidates every purchase field.
			pFScene->m_pMessageBox->SetMessage(g_pMessageStringTable[144], 646, 0);
			pFScene->m_pMessageBox->SetVisible(1);
			pFScene->m_pMessageBox->m_dwArg = m_dwControlID;
		}
		return 1;
	}

	auto pListBox = pFScene->m_pChatList;

	auto pItem2 = new SListBoxItem(g_pMessageStringTable[1], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0,
		0x77777777, 1, 0);
	if (pItem2)
		pListBox->AddItem(pItem2);

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(33);
		if (pSoundData)
			pSoundData->Play(0, 0);
	}
	return 1;

}


int SGridControl::SellItem(int nCellX, int nCellY, unsigned int dwFlags, unsigned int wParam)
{
	auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	// A drop onto a native mix slot must win over the modern quickslot branches
	// sharing raw values 18..22, otherwise recipes silently become shortcuts.
	if (g_pCursor && g_pCursor->m_pAttachedItem)
	{
		auto sourceItem = g_pCursor->m_pAttachedItem;
		int nativeMixResult = pScene->TryStageNativeMixItem(
			sourceItem->m_pGridControl, sourceItem, this);
		if (nativeMixResult >= 0)
			return nativeMixResult;
	}

	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = SellItemOnShop(pScene, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_DELETE)
	{
		SGridControl::m_pSellItem = g_pCursor->m_pAttachedItem;
		if (!SGridControl::m_pSellItem || !SGridControl::m_pSellItem->m_pItem ||
			!WYD748_IsValidItemIndex(SGridControl::m_pSellItem->m_pItem->sIndex))
		{
			SGridControl::m_pSellItem = nullptr;
			return 1;
		}

		char szMessage[128]{};
		// The catalog name is data, not a printf format; cap even a malformed
		// non-terminated ItemList record to its 7.48 fixed-width field.
		sprintf_s(szMessage, sizeof(szMessage), "%.*s",
			static_cast<int>(sizeof(g_pItemList[0].Name)),
			g_pItemList[SGridControl::m_pSellItem->m_pItem->sIndex].Name);
		pScene->m_pMessageBox->SetMessage(szMessage, 740, g_pMessageStringTable[18]);
		pScene->m_pMessageBox->SetVisible(1);
		g_pCursor->m_pAttachedItem = 0;
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT1 ||
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT2 ||
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT3 ||
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT4 ||
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT5)
	{
		if (!g_pCursor || !g_pCursor->m_pAttachedItem ||
			!g_pCursor->m_pAttachedItem->m_pItem)
			return 1;
		auto pReturnItem = g_pCursor->m_pAttachedItem;
		int itemcheck = 0;
		int itemidx = g_pCursor->m_pAttachedItem->m_pItem->sIndex;

		if (itemidx >= 400 && itemidx <= 409)
			itemcheck = 1;
		else if (itemidx >= 415 && itemidx <= 416)
			itemcheck = 1;
		else if (itemidx >= 428 && itemidx <= 435)
			itemcheck = 1;
		else if (itemidx >= 646 && itemidx <= 647)
			itemcheck = 1;
		else if (itemidx >= 680 && itemidx <= 691)
			itemcheck = 1;
		else if (itemidx >= 3310 && itemidx <= 3312)
			itemcheck = 1;
		else if (itemidx >= 3319 && itemidx <= 3323)
			itemcheck = 1;
		else if (itemidx >= 3368 && itemidx <= 3377)
			itemcheck = 1;
		else if (itemidx >= 3383 && itemidx <= 3384)
			itemcheck = 1;
		else if (itemidx == 3431 || itemidx == 4145 || itemidx == 1739 || itemidx == 3477)
			itemcheck = 1;
		else if (itemidx == 3472 || itemidx == 4097)
			itemcheck = 1;
		else if (itemidx >= 3200 && itemidx <= 3210)
			itemcheck = 1;
		if (itemidx == 3207)
			itemcheck = 0;

		if (!itemcheck)
		{
			g_pCursor->DetachItem();
			return 1;
		}

		if (BASE_GetItemAbility(pReturnItem->m_pItem, 33) >= 1)
			return 1;

		auto quickGrid = pScene->m_pQuick_Sloat[
			(int)m_eGridType - (int)TMEGRIDTYPE::GRID_QUICKSLOAT1];
		if (!quickGrid)
			return 1;

		auto pNewItem = new STRUCT_ITEM;
		memcpy(pNewItem, pReturnItem->m_pItem, sizeof(STRUCT_ITEM));

		auto pNewControlItem = new SGridControlItem(0, pNewItem, 0.0f, 0.0f);
		pNewControlItem->m_nWidth = pNewControlItem->m_nWidth * 0.9f;
		pNewControlItem->m_nHeight = pNewControlItem->m_nHeight * 0.9f;
		pNewControlItem->m_GCObj.m_fWidth = pNewControlItem->m_GCObj.m_fWidth * 0.9f;
		pNewControlItem->m_GCObj.m_fHeight = pNewControlItem->m_GCObj.m_fHeight * 0.9f;

		SGridControlItem* pOldItem = nullptr;
		if (!WYD748_ReplaceGridVisual(quickGrid, pNewControlItem,
			nCellX, nCellY, pOldItem))
		{
			SAFE_DELETE(pNewControlItem);
			return 1;
		}
		g_pCursor->DetachItem();
		SAFE_DELETE(pOldItem);
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = SellItemOnSkillBelt(nCellX, nCellY, pScene, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_CUBEBOX)
	{
		if (!g_pCursor || !g_pCursor->m_pAttachedItem ||
			!g_pCursor->m_pAttachedItem->m_pItem)
			return 0;

		// Pickup transfers ownership out of the grid. No later branch consumes this
		// visual, so clear interaction aliases before releasing it.
		auto pItem = PickupItem(nCellX, nCellY);
		if (pItem)
		{
			if (m_pLastMouseOverItem == pItem)
				m_pLastMouseOverItem = nullptr;
			if (m_pLastAttachedItem == pItem)
				m_pLastAttachedItem = nullptr;
			if (m_pSellItem == pItem)
				m_pSellItem = nullptr;
			if (g_pCursor->m_pAttachedItem == pItem)
				g_pCursor->m_pAttachedItem = nullptr;
			SAFE_DELETE(pItem);
		}

		pScene->UpdateScoreUI(0);
		pScene->UpdateSkillBelt();
	}
	else
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = SellItemOnOtherGrid(nCellX, nCellY, dwFlags, wParam, pScene, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	return 2;
}

// Extracted from SGridControl::SellItem; behavior is unchanged.
ExtractedFlow SGridControl::SellItemOnShop(TMFieldScene*& pScene, int& extractedResult)
{
	SGridControl::m_pSellItem = g_pCursor ? g_pCursor->m_pAttachedItem : nullptr;
	if (!SGridControl::m_pSellItem || !SGridControl::m_pSellItem->m_pItem ||
		!WYD748_IsValidItemIndex(SGridControl::m_pSellItem->m_pItem->sIndex))
	{
		SGridControl::m_pSellItem = nullptr;
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	if (g_pEventTranslator->m_bCtrl)
	{
		short sDestType = CheckType(SGridControl::m_pSellItem->m_pGridControl->m_eItemType,
			SGridControl::m_pSellItem->m_pGridControl->m_eGridType);

		short sDestPos = CheckPos(SGridControl::m_pSellItem->m_pGridControl->m_eItemType);
		if (sDestPos == -1)
			sDestPos = WYD748_ResolveWireSlot(pScene,
				SGridControl::m_pSellItem->m_pGridControl, sDestType,
				SGridControl::m_pSellItem->m_nCellIndexX,
				SGridControl::m_pSellItem->m_nCellIndexY);

		MSG_Sell stSell{};
		stSell.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
		stSell.Header.Type = MSG_Sell_Opcode;
		stSell.TargetID = m_dwMerchantID;
		stSell.MyType = sDestType;
		stSell.MyPos = sDestPos;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSell)->Type, reinterpret_cast<char*>(&stSell), sizeof(stSell)});
		SGridControl::m_pSellItem = 0;
	}
	else
	{
		char szMessage[128]{};
		if (!WYD748_FormatSellConfirmation(szMessage, SGridControl::m_pSellItem))
		{
			SGridControl::m_pSellItem = nullptr;
			{ extractedResult = 1; return ExtractedFlow::Return; }
		}
		pScene->m_pMessageBox->SetMessage(szMessage, 890, g_pMessageStringTable[343]);
		pScene->m_pMessageBox->SetVisible(1);
		g_pCursor->m_pAttachedItem = 0;
	}

	m_dwEnableColor = 0;
	return ExtractedFlow::Next;
}

// Extracted from SGridControl::SellItem; behavior is unchanged.
ExtractedFlow SGridControl::SellItemOnSkillBelt(int& nCellX, int& nCellY, TMFieldScene*& pScene, int& extractedResult)
{
	if (!g_pCursor || !g_pCursor->m_pAttachedItem ||
		!g_pCursor->m_pAttachedItem->m_pItem || nCellX < 0 || nCellX >= 10 || nCellY != 0)
		{ extractedResult = 1; return ExtractedFlow::Return; }
	if (!IsSkill(g_pCursor->m_pAttachedItem->m_pItem->sIndex))
		{ extractedResult = 1; return ExtractedFlow::Return; }

	// The scene binds page two to native control 586. The later ID 65645
	// incorrectly stored its shortcuts in slots 0..9.
	const int nSeg = this == pScene->m_pGridSkillBelt3 ? 10 : 0;

	SGridControlItem* pItem = nullptr;
	auto pNewItem = new STRUCT_ITEM;

	memcpy(pNewItem, g_pCursor->m_pAttachedItem->m_pItem, sizeof(STRUCT_ITEM));

	auto pNewControlItem = new SGridControlItem(0, pNewItem, 0.0f, 0.0f);
	if (!WYD748_ReplaceGridVisual(this, pNewControlItem, nCellX, nCellY, pItem))
	{
		SAFE_DELETE(pNewControlItem);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	if (g_pObjectManager->m_cSelectShortSkill - nSeg == nCellX)
		pNewControlItem->m_GCObj.nTextureSetIndex = 200;

	g_pCursor->DetachItem();

	SAFE_DELETE(pItem);

	g_pObjectManager->m_cShortSkill[nSeg + nCellX] = static_cast<char>(g_pItemList[pNewItem->sIndex].nIndexTexture);

	MSG_SetShortSkill stSetShortSkill{};
	stSetShortSkill.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
	stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;

	memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));

	for (int j = 0; j < 20; ++j)
	{
		if (stSetShortSkill.Skill[j] >= 0 && stSetShortSkill.Skill[j] < 96)
		{
			stSetShortSkill.Skill[j] -= 24 * g_pObjectManager->m_stMobData.Class;
		}
		else if (stSetShortSkill.Skill[j] >= 105 && stSetShortSkill.Skill[j] < 153)
		{
			stSetShortSkill.Skill[j] -= 12 * g_pObjectManager->m_stMobData.Class;
		}
	}

	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSetShortSkill)->Type, reinterpret_cast<char*>(&stSetShortSkill), sizeof(stSetShortSkill)});

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(31);
		if (pSoundData)
			pSoundData->Play();
	}

	pScene->UpdateScoreUI(0);
	pScene->UpdateSkillBelt();
	return ExtractedFlow::Next;
}

// Extracted from SGridControl::SellItem; behavior is unchanged.
ExtractedFlow SGridControl::SellItemOnOtherGrid(int& nCellX, int& nCellY, unsigned int& dwFlags, unsigned int& wParam, TMFieldScene*& pScene, int& extractedResult)
{
	auto pItem = GetItem(nCellX, nCellY);
	int nVolatile = BASE_GetItemAbility(g_pCursor->m_pAttachedItem->m_pItem, 38);
	int nDestVolatile = -1;
	int itemidx = -1;
	short sDestType = CheckType(m_eItemType, m_eGridType);
	short sDestPos = CheckPos(m_eItemType);
	if (pItem)
	{
	if (!pScene->m_bCompatFieldScene && nVolatile == 5 && sDestType == 1)
	{
		nVolatile = 190;
	}
	nDestVolatile = BASE_GetItemAbility(pItem->m_pItem, 38);
	itemidx = pItem->m_pItem->sIndex;
	}

	if ((((nVolatile >= 4 && nVolatile <= 6 || nVolatile == 9 || nVolatile == 15 || nVolatile == 16 || nVolatile >= 180 && nVolatile <= 183 ||
	nVolatile >= 235 && nVolatile <= 238 || nVolatile >= 239 && nVolatile <= 240 || nVolatile >= 90 && nVolatile < 95 || nVolatile == 179 ||
	nVolatile == 186 || nVolatile == 196) &&
	!sDestType || nVolatile == 190 && !sDestType && m_dwEnableColor == 0x330000FF) &&
	!nDestVolatile || nVolatile == 241 && sDestType == 1 && nDestVolatile == 16 ||
	(nVolatile == 4 || nVolatile == 5) && sDestType == 1 && m_dwEnableColor == 0x3300FF00 ||
	g_pCursor->m_pAttachedItem->m_pItem->sIndex == 3465 || (nVolatile == 4 || nVolatile == 5) &&
	(sDestType >= 1901 && sDestType <= 1910 || sDestType >= 1234 && sDestType <= 1237 || sDestType >= 1369 && sDestType <= 1372 ||
		sDestType >= 1519 && sDestType <= 1522 || sDestType >= 1669 && sDestType <= 1672 || sDestType == 1714)) && pItem)
	{
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (pScene->m_dwUseItemTime && dwServerTime - pScene->m_dwUseItemTime < 200)
		{ extractedResult = 1; return ExtractedFlow::Return; }

	short sSrcType = CheckType(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType,
		g_pCursor->m_pAttachedItem->m_pGridControl->m_eGridType);
	short sSrcPos = CheckPos(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType);

	if (sSrcPos == -1)
		sSrcPos = WYD748_ResolveWireSlot(pScene,
			g_pCursor->m_pAttachedItem->m_pGridControl, sSrcType,
			g_pCursor->m_pAttachedItem->m_nCellIndexX,
			g_pCursor->m_pAttachedItem->m_nCellIndexY);

	if (sDestPos == -1)
		sDestPos = WYD748_ResolveWireSlot(pScene, pItem->m_pGridControl,
			sDestType, pItem->m_nCellIndexX, pItem->m_nCellIndexY);


	if (sSrcType == 2)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	if (nVolatile >= 239 && nVolatile <= 240)
	{
		_nCellX = nCellX;
		_nCellY = nCellY;
		_dwFlags = dwFlags;
		_wParam = wParam;
		pScene->m_pitemPassGrid = this;
		pScene->VisibleInputPass();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	MSG_UseItem stUseItem{};
	// Native 7.48 validates the resolved Carry slot directly; modern bag-key
	// gates would reject valid rows 4-7 in its contiguous 9x7 inventory.

	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = 883;
	stUseItem.SourType = sSrcType;
	stUseItem.SourPos = sSrcPos;
	stUseItem.DestType = sDestType;
	stUseItem.DestPos = sDestPos;
	stUseItem.ItemID = 0;
	stUseItem.GridX = 0;
	stUseItem.GridY = 0;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});

	pScene->m_dwUseItemTime = dwServerTime;
	int nCellTempX = g_pCursor->m_pAttachedItem->m_nCellIndexX;
	int nCellTempY = g_pCursor->m_pAttachedItem->m_nCellIndexY;
	auto pGrid = g_pCursor->m_pAttachedItem->m_pGridControl;
	auto pPickedItem = pGrid->GetItem(nCellTempX, nCellTempY);

	int nAmount = 0;

	if (pPickedItem)
	{
		nAmount = BASE_GetItemAmount(pPickedItem->m_pItem);

		if (pPickedItem->m_pItem->sIndex >= 2330 && pPickedItem->m_pItem->sIndex < 2390)
			nAmount = 0;
		if (nAmount <= 1)
		{
			pGrid->PickupItem(nCellTempX, nCellTempY);
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = nullptr;

			SAFE_DELETE(pPickedItem);
		}
		else
		{
			BASE_SetItemAmount(pPickedItem->m_pItem, nAmount - 1);
			auto pGItem = pPickedItem;
			sprintf(pPickedItem->m_GCText.strString, "%2d", nAmount - 1);
			pGItem->m_GCText.pFont->SetText(pGItem->m_GCText.strString, pGItem->m_GCText.dwColor, 0);
		}
	}

	g_pCursor->DetachItem();
	if (nAmount <= 1)
	{
		if (!sSrcType)
			memset(&g_pObjectManager->m_stMobData.Equip[sSrcPos], 0, sizeof(STRUCT_ITEM));
		else if (sSrcType == 1)
			memset(&g_pObjectManager->m_stMobData.Carry[sSrcPos], 0, sizeof(STRUCT_ITEM));
		else if (sSrcType == 2)
			memset(&g_pObjectManager->m_stItemCargo[sSrcPos], 0, sizeof(STRUCT_ITEM));
	}
	}
	else if (nVolatile == 190 && !sDestType && m_dwEnableColor != 0x330000FF && pItem)
	{
	g_pCurrentScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[303], 3000);
	g_pCurrentScene->m_pMessagePanel->SetVisible(1, 1);
	}
	else if (CanChangeItem(g_pCursor->m_pAttachedItem, nCellX, nCellY, 0))
	{
	// The 7.48 grid has no locked pages; every valid CanChangeItem target must
	// reach the scene swap handler so the blue preview and mouse-up agree.
	SGridControl::m_pLastAttachedItem = g_pCursor->m_pAttachedItem;

	auto pos = g_pCursor->GetPos();
	g_pCurrentScene->OnMouseEvent(dwFlags, wParam, (int)pos.x, (int)pos.y);
	}
	return ExtractedFlow::Next;
}


int SGridControl::SellItem2()
{
	int nCellX = _nCellX;
	int nCellY = _nCellY;
	unsigned int dwFlags = _dwFlags;
	int wParam = _wParam;

	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP)
	{
		auto pScene = g_pCurrentScene;
		SGridControl::m_pSellItem = g_pCursor ? g_pCursor->m_pAttachedItem : nullptr;
		if (!SGridControl::m_pSellItem || !SGridControl::m_pSellItem->m_pItem ||
			!WYD748_IsValidItemIndex(SGridControl::m_pSellItem->m_pItem->sIndex))
		{
			SGridControl::m_pSellItem = nullptr;
			return 1;
		}

		if (!g_pEventTranslator->m_bCtrl)
		{
			char szMessage[128]{};
			if (!WYD748_FormatSellConfirmation(szMessage, SGridControl::m_pSellItem))
			{
				SGridControl::m_pSellItem = nullptr;
				return 1;
			}
			pScene->m_pMessageBox->SetMessage(szMessage, 890, g_pMessageStringTable[343]);
			pScene->m_pMessageBox->SetVisible(1);
			g_pCursor->m_pAttachedItem = 0;
		}
		else
		{
			short sDestType = CheckType(SGridControl::m_pSellItem->m_pGridControl->m_eItemType,
				SGridControl::m_pSellItem->m_pGridControl->m_eGridType);
			short sDestPos = CheckPos(SGridControl::m_pSellItem->m_pGridControl->m_eItemType);
			if (sDestPos == -1)
				sDestPos = WYD748_ResolveWireSlot(pFScene,
					SGridControl::m_pSellItem->m_pGridControl, sDestType,
					SGridControl::m_pSellItem->m_nCellIndexX,
					SGridControl::m_pSellItem->m_nCellIndexY);

			MSG_Sell stSell{};
			stSell.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
			stSell.Header.Type = MSG_Sell_Opcode;
			stSell.TargetID = m_dwMerchantID;
			stSell.MyType = sDestType;
			stSell.MyPos = sDestPos;
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSell)->Type, reinterpret_cast<char*>(&stSell), sizeof(stSell)});
			SGridControl::m_pSellItem = 0;
		}
		m_dwEnableColor = 0;
		return 2;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
	{
		if (!g_pCursor || !g_pCursor->m_pAttachedItem ||
			!g_pCursor->m_pAttachedItem->m_pItem || nCellX < 0 || nCellX >= 10 || nCellY != 0)
			return 1;
		if (!IsSkill(g_pCursor->m_pAttachedItem->m_pItem->sIndex))
			return 1;

		// Deferred confirmation uses the same page as the direct drag.
		const int nSeg = this == pFScene->m_pGridSkillBelt3 ? 10 : 0;

		SGridControlItem* pReturnItem = nullptr;

		auto pNewItem = new STRUCT_ITEM;
		memcpy(pNewItem, g_pCursor->m_pAttachedItem->m_pItem, sizeof(STRUCT_ITEM));

		auto pNewControlItem = new SGridControlItem(0, pNewItem, 0.0, 0.0);
		if (!WYD748_ReplaceGridVisual(this, pNewControlItem, nCellX, nCellY, pReturnItem))
		{
			SAFE_DELETE(pNewControlItem);
			return 1;
		}

		if (g_pObjectManager->m_cSelectShortSkill - nSeg == nCellX)
			pNewControlItem->m_GCObj.nTextureSetIndex = 200;

		g_pCursor->DetachItem();

		SAFE_DELETE(pReturnItem);

		g_pObjectManager->m_cShortSkill[nSeg + nCellX] = static_cast<char>(g_pItemList[pNewItem->sIndex].nIndexTexture);

		MSG_SetShortSkill stSetShortSkill{};
		stSetShortSkill.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
		stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;
		memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));

		for (int i = 0; i < 20; ++i)
		{
			if (stSetShortSkill.Skill[i] >= 0 && stSetShortSkill.Skill[i] < 96)
			{
				stSetShortSkill.Skill[i] -= 24 * g_pObjectManager->m_stMobData.Class;
			}
			else if (stSetShortSkill.Skill[i] >= 105 && stSetShortSkill.Skill[i] < 153)
			{
				stSetShortSkill.Skill[i] -= 12 * g_pObjectManager->m_stMobData.Class;
			}
		}

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSetShortSkill)->Type, reinterpret_cast<char*>(&stSetShortSkill), sizeof(stSetShortSkill)});

		auto pSoundManager = g_pSoundManager;
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(31);
			if(pSoundData)
				pSoundData->Play(0, 0);
		}

		pFScene->UpdateScoreUI(0);
		pFScene->UpdateSkillBelt();
		return 2;
	}

	auto pItem = GetItem(nCellX, nCellY);
	int nVolatile = BASE_GetItemAbility(g_pCursor->m_pAttachedItem->m_pItem, 38);
	int nDestVolatile = -1;
	if (pItem)
		nDestVolatile = BASE_GetItemAbility(pItem->m_pItem, 38);

	short sDestType = CheckType(m_eItemType, m_eGridType);
	short sDestPos = CheckPos(m_eItemType);
	if (((nVolatile >= 4 && nVolatile <= 6
		|| nVolatile == 9
		|| nVolatile == 15
		|| nVolatile == 16
		|| nVolatile >= 180 && nVolatile <= 183
		|| nVolatile >= 235 && nVolatile <= 238
		|| nVolatile >= 239 && nVolatile <= 240
		|| nVolatile >= 90 && nVolatile < 95
		|| nVolatile == 179
		|| nVolatile == 186
		|| nVolatile == 196)
		&& !sDestType
		|| nVolatile == 190 && !sDestType && m_dwEnableColor == 0x330000FF)
		&& !nDestVolatile
		&& pItem)
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();
		if (pFScene->m_dwUseItemTime && dwServerTime - pFScene->m_dwUseItemTime < 200)
			return 1;

		short sSrcType = CheckType(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType,
			g_pCursor->m_pAttachedItem->m_pGridControl->m_eGridType);

		short sSrcPos = CheckPos(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType);

		if (sSrcPos == -1)
			sSrcPos = WYD748_ResolveWireSlot(pFScene,
				g_pCursor->m_pAttachedItem->m_pGridControl, sSrcType,
				g_pCursor->m_pAttachedItem->m_nCellIndexX,
				g_pCursor->m_pAttachedItem->m_nCellIndexY);

		if (sDestPos == -1)
		{
			// WYD748: item-on-item actions use the same native slot resolver as
			// drag, right-click and swap packets so all controls agree on identity.
			sDestPos = WYD748_ResolveWireSlot(pFScene, pItem->m_pGridControl,
				sDestType, pItem->m_nCellIndexX, pItem->m_nCellIndexY);
		}

		pFScene->m_dwUseItemTime = dwServerTime;
		int nCellTempX = g_pCursor->m_pAttachedItem->m_nCellIndexX;
		int nCellTempY = g_pCursor->m_pAttachedItem->m_nCellIndexY;
		auto pGrid = g_pCursor->m_pAttachedItem->m_pGridControl;
		auto pPickedItem = pGrid->GetItem(nCellTempX, nCellTempY);
		int nAmount = 0;

		if (pPickedItem)
		{
			nAmount = BASE_GetItemAmount(pPickedItem->m_pItem);
			if (pPickedItem->m_pItem->sIndex >= 2330 && pPickedItem->m_pItem->sIndex < 2390)
				nAmount = 0;
			if (nAmount <= 1)
			{
				pGrid->PickupItem(nCellTempX, nCellTempY);
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
					g_pCursor->m_pAttachedItem = 0;

				SAFE_DELETE(pPickedItem);
			}
			else
			{
				BASE_SetItemAmount(pPickedItem->m_pItem, nAmount - 1);
				auto pGItem = pPickedItem;
				sprintf(pPickedItem->m_GCText.strString, "%2d", nAmount - 1);
				pGItem->m_GCText.pFont->SetText(pGItem->m_GCText.strString, pGItem->m_GCText.dwColor, 0);
			}
		}

		g_pCursor->DetachItem();
		if (nAmount <= 1)
		{
			if (!sSrcType)
				memset(&g_pObjectManager->m_stMobData.Equip[sSrcPos], 0, sizeof(STRUCT_ITEM));
			else if (sSrcType == 1)
				memset(&g_pObjectManager->m_stMobData.Carry[sSrcPos], 0, sizeof(STRUCT_ITEM));
			else if (sSrcType == 2)
				memset(&g_pObjectManager->m_stItemCargo[sSrcPos], 0, sizeof(STRUCT_ITEM));
		}
	}
	else if (nVolatile == 190 && !sDestType && m_dwEnableColor != 0x330000FF && pItem)
	{
		pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[303], 3000);
		pFScene->m_pMessagePanel->SetVisible(1, 1);
	}
	else if (CanChangeItem(g_pCursor->m_pAttachedItem, nCellX, nCellY, 0))
	{
		SGridControl::m_pLastAttachedItem = g_pCursor->m_pAttachedItem;

		auto vecPos = g_pCursor->GetPos();
		pFScene->OnMouseEvent(dwFlags, wParam, (int)vecPos.x, (int)vecPos.y);
	}

	return 2;
}

void SGridControl::SwapItem(int nCellX, int nCellY, int nCellVWidth, int nCellVHeight, STRUCT_ITEM* pItem)
{
	auto pGeom = SGridControl::m_pLastAttachedItem->GetGeomControl();
	auto nWidth = (int)(pGeom->nWidth / (float)nCellVWidth);
	auto nHeight = (int)(pGeom->nHeight / (float)nCellVHeight);
	unsigned int nAtItemPos = 0;

	if (SGridControl::m_pLastAttachedItem)
	{
		memcpy(pItem, SGridControl::m_pLastAttachedItem->m_pItem, sizeof(STRUCT_ITEM));
		nAtItemPos = BASE_GetItemAbility(pItem, 17);
	}

	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

	// This unconditional scope mirrors the native 7.48 swap lifecycle. The old
	// condition represented 7.59 bag pages and could silently discard mouse-up.
	{
		if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE)
		{
			short sDestType = CheckType(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType,
				SGridControl::m_pLastAttachedItem->m_pGridControl->m_eGridType);

			short sDestPos = CheckPos(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType);
			// WYD748: derive both endpoints from the active grid topology.  This
			// replaces every 5x3 page calculation in this swap packet at once.
			const short sSourceType = CheckType(m_eItemType, m_eGridType);
			const short sSourcePos = WYD748_ResolveWireSlot(pFScene, this, sSourceType,
				nCellX, nCellY);
			if (sDestPos == -1)
				sDestPos = WYD748_ResolveWireSlot(pFScene,
					SGridControl::m_pLastAttachedItem->m_pGridControl, sDestType,
					SGridControl::m_pLastAttachedItem->m_nCellIndexX,
					SGridControl::m_pLastAttachedItem->m_nCellIndexY);

			MSG_SwapItem stSwapItem{};
			stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
			stSwapItem.Header.Type = MSG_SwapItem_Opcode;
			stSwapItem.SourType = static_cast<char>(sSourceType);
			stSwapItem.SourPos = static_cast<char>(sSourcePos);
			stSwapItem.DestType = static_cast<char>(sDestType);
			stSwapItem.DestPos = static_cast<char>(sDestPos);
			stSwapItem.TargetID = TMFieldScene::m_dwCargoID;

			if (stSwapItem.DestPos != stSwapItem.SourPos ||
				stSwapItem.SourType != stSwapItem.DestType)
			{
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSwapItem)->Type, reinterpret_cast<char*>(&stSwapItem), sizeof(stSwapItem)});
			}
		}
		else if ((nAtItemPos & (int)m_eItemType) == (int)m_eItemType)
		{
			short sSrcType = CheckType(m_eItemType, m_eGridType);
			short sSrcPos = CheckPos(m_eItemType);

			short sDestType = CheckType(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType,
				SGridControl::m_pLastAttachedItem->m_pGridControl->m_eGridType);
			short sDestPos = CheckPos(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType);

			int nAX = SGridControl::m_pLastAttachedItem->m_nCellIndexX;
			int nAY = SGridControl::m_pLastAttachedItem->m_nCellIndexY;

			if (sSrcType || sDestType)
			{
				// WYD748: equipment positions remain fixed, while inventory/cargo
				// positions are resolved by their native 9x7/9-column grids.
				if (sSrcType)
					sSrcPos = WYD748_ResolveWireSlot(pFScene, this, sSrcType, nCellX, nCellY);
				if (sDestType)
					sDestPos = WYD748_ResolveWireSlot(pFScene,
						SGridControl::m_pLastAttachedItem->m_pGridControl, sDestType, nAX, nAY);

				MSG_SwapItem stSwapItem{};
				stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
				stSwapItem.Header.Type = MSG_SwapItem_Opcode;
				stSwapItem.SourType = static_cast<char>(sSrcType);
				stSwapItem.SourPos = static_cast<char>(sSrcPos);
				stSwapItem.DestType = static_cast<char>(sDestType);
				stSwapItem.TargetID = TMFieldScene::m_dwCargoID;

				if (sDestType)
					stSwapItem.DestPos = static_cast<char>(sDestPos);
				else
					stSwapItem.DestPos = static_cast<char>(sDestPos);

				if (stSwapItem.DestPos != stSwapItem.SourPos ||
					stSwapItem.SourType != stSwapItem.DestType)
				{
					SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSwapItem)->Type, reinterpret_cast<char*>(&stSwapItem), sizeof(stSwapItem)});
				}
			}
		}

		SGridControl::m_pLastAttachedItem = 0;
		return;
	}
}

char SGridControl::AutoSellShowPrice(char* Price)
{
	if (m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2 && m_eGridType != TMEGRIDTYPE::GRID_TRADEOP)
		return 0;

	int nPrice = m_nTradeMoney;

	char szPrice[128];
	char szPrice2[128];
	char szPrice3[128];
	char szPrice4[128];

	sprintf(szPrice2, "%d", nPrice % 10000);
	sprintf(szPrice3, g_pMessageStringTable[283], nPrice / 10000 % 10000);
	sprintf(szPrice4, g_pMessageStringTable[282], nPrice / 100000000);
	if (nPrice % 10000 <= 0)
	{
		if (nPrice / 10000 % 10000 <= 0)
		{
			if (nPrice / 100000000 <= 0)
				sprintf(szPrice, "");
			else
				sprintf(szPrice, "%s %s", szPrice4, g_pMessageStringTable[284]);
		}
		else if (nPrice / 100000000 <= 0)
		{
			sprintf(szPrice, "%s %s", szPrice3, g_pMessageStringTable[284]);
		}
		else
		{
			sprintf(szPrice, "%s %s %s", szPrice4, szPrice3, g_pMessageStringTable[284]);
		}
	}
	else if (nPrice / 10000 % 10000 <= 0)
	{
		if (nPrice / 100000000 <= 0)
			sprintf(szPrice, "%s %s", szPrice2, g_pMessageStringTable[284]);
		else
			sprintf(szPrice, "%s %s %s ", szPrice4, szPrice2, g_pMessageStringTable[284]);
	}
	else if (nPrice / 100000000 <= 0)
	{
		sprintf(szPrice, "%s %s%s", szPrice3, szPrice2, g_pMessageStringTable[284]);
	}
	else
	{
		sprintf(szPrice, "%s %s %s %s", szPrice4, szPrice3, szPrice2, g_pMessageStringTable[284]);
	}

	sprintf(Price, "%s", szPrice);
	return 1;
}
