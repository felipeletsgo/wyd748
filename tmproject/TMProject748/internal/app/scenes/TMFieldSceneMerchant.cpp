#include "pch.h"
#include "TMFieldScene.h"
#include "TMGround.h"
#include "TMSkinMesh.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "ItemEffect.h"
#include "TMItem.h"
#include "ClientDiagnostics.h"
#include "../../core/NativeSalePrice.h"
#include "FieldSceneInventorySupport.h"

int TMFieldScene::CheckMerchant(TMHuman* pOver)
{
	// The 7.48 compact bootstrap can run with optional commerce controls absent;
	// merchant hit-testing must remain valid without dereferencing such a panel.
	if (m_pAutoTrade && m_pAutoTrade->m_bVisible)
		return 1;

	if (!pOver || pOver->m_bMouseOver != 1)
		return 0;

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 1 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 11 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 5)
		return 1;

	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) &&
		pOver->m_sHeadIndex == 51 &&
		((int)m_pMyHuman->m_vecPosition.x >> 7 == 13 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 14) && (int)m_pMyHuman->m_vecPosition.y >> 7 == 28)
	{
		return 1;
	}

	if (pOver->m_TradeDesc[0])
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 1)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 2)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 3)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 67 && m_pGround->m_vecOffsetIndex.x == 13 && m_pGround->m_vecOffsetIndex.y == 13)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 67 && m_pGround->m_vecOffsetIndex.x == 28 && m_pGround->m_vecOffsetIndex.y == 24)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 54 && m_pGround->m_vecOffsetIndex.x == 19 && m_pGround->m_vecOffsetIndex.y == 13)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 55)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 56)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 68)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 57)
		return 1;

	if (((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 4) ||
		(pOver->m_stScore.Merchant & 0xF) >= 8 && (pOver->m_stScore.Merchant & 0xF) <= 15)
	{
		if ((pOver->m_stScore.Merchant & 0xF) == 15)
		{
			if (!m_pMyHuman->IsInTown())
			{
				if (pOver->m_cMantua > 0 && m_pMyHuman->m_cMantua > 0 && pOver->m_cMantua != m_pMyHuman->m_cMantua &&
					m_pMyHuman->m_cMantua != 4 && m_pMyHuman->m_cMantua != 3)
				{
					return 1;
				}

				if (m_pMyHuman->m_pMantua &&
					((int)m_pMyHuman->m_pMantua->m_Look.Skin0 < 2 ||
					((int)m_pMyHuman->m_pMantua->m_Look.Skin0 >= 8 && (int)m_pMyHuman->m_pMantua->m_Look.Skin0 <= 14)))
				{
					if (g_pObjectManager->m_stMobData.Equip[10].sIndex == 1742
						&& (g_pObjectManager->m_stMobData.Equip[11].sIndex < 1760 || g_pObjectManager->m_stMobData.Equip[11].sIndex > 1763))
					{
						return 1;
					}
				}
			}
		}

		if ((pOver->m_stScore.Merchant & 0xF) == 13)
			return 1;
		if ((pOver->m_stScore.Merchant & 0xF) == 14)
			return 1;

		return 1;
	}

	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) >= 6 && (pOver->m_stScore.Merchant & 0xF) <= 8)
		return 1;

	return 0;
}

void TMFieldScene::SetVisibleShop(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;
	// The 7.48 shop and inventory are one-page legacy panels.  Keep the modern
	// grid state but avoid four-page inventory and optional store dereferences.
	if (m_bCompatFieldScene)
	{
		if (bShow)
		{
			// Native FUN_004481C5 closes AutoTrade before showing the paired Shop
			// and Inventory roots. AutoTrade is allowed to keep its own 280/530 layout.
			if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
				SetVisibleAutoTrade(0, 0);
			PositionCompatShopPanels();
		}
		if (m_pGridInv)
			m_pGridInv->m_eGridType = bShow ? TMEGRIDTYPE::GRID_SELL : TMEGRIDTYPE::GRID_DEFAULT;
		if (m_pShopPanel)
			m_pShopPanel->SetVisible(bShow);
		if (m_pInvenPanel)
			m_pInvenPanel->SetVisible(bShow);
		if (bShow && m_pCPanel)
			m_pCPanel->SetVisible(0);
		if (!bShow)
		{
			g_pObjectManager->m_RMBShopOpen = 0;
			m_bEventCouponOpen = 0;
		}
		GetSoundAndPlay(51, 0, 0);
		return;
	}

	if (bShow)
	{
		g_pCursor->DetachItem();

		m_pCPanel->SetVisible(0);
		m_pCargoPanel->SetVisible(0);

		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);

		if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);

		SetInventoryGridType(TMEGRIDTYPE::GRID_SELL);

		SetEquipGridState(0);

		m_pSkillPanel->SetVisible(0);
	}
	else
	{
		SetInventoryGridType(TMEGRIDTYPE::GRID_DEFAULT);

		SetEquipGridState(1);

		g_pObjectManager->m_RMBShopOpen = 0;
		m_bEventCouponOpen = 0;
	}

	m_pShopPanel->SetVisible(bShow);
	m_pShopPanel->SetPos(RenderDevice::m_fWidthRatio * 425.0f, RenderDevice::m_fHeightRatio * 35.0f);//alterado

	if (g_pApp->m_dwScreenWidth <= 1024)
	{
		m_pShopPanel->SetVisible(bShow);
		m_pShopPanel->SetPos(RenderDevice::m_fWidthRatio * 372.0f, RenderDevice::m_fHeightRatio * 35.0f);//alterado
	}

	if (bShow)
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 650.0f, RenderDevice::m_fHeightRatio * 35.0f);

	m_pInvenPanel->SetVisible(bShow);

	UpdateScoreUI(0);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleHellGateStore(int bShow)
{
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
		if (m_pInvenPanel && !m_pInvenPanel->m_bVisible)
			SetVisibleInventory();
		if (m_pHellStoreDesc)
			LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 0, 20);

		m_pHellgateStore->SetVisible(1);
		g_pDevice->m_nWidthShift = 0;
		SetInventoryGridType(TMEGRIDTYPE::GRID_SELL);
		SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_pHellgateStore->SetVisible(0);
		m_dwHellStoreID = 0;
		m_nHellStoreValue = 0;
		SetInventoryGridType(TMEGRIDTYPE::GRID_DEFAULT);
		SetEquipGridState(1);
	}
	if (!bShow)
	{
		for (int i = 0; i < 4; ++i)
		{
			auto pBtnHellStore = (SButton*)m_pControlContainer->FindControl(i + 6193);
			if (pBtnHellStore)
				pBtnHellStore->SetSelected(0);
		}
	}
}

int TMFieldScene::OnPacketShopList(MSG_STANDARD* pStd)
{
	auto pShopList = reinterpret_cast<MSG_ShopList*>(pStd);

	if (pShopList->ShopType == 1)
	{
		// A resource failure can leave the merchant grid unbound when the
		// packet arrives. Do not change coupon or shop state in that case.
		if (!m_pGridShop)
		{
			WYD748_DiagnosticsLog("merchant grid missing for ShopList\r\n");
			return 0;
		}

		if (m_bEventCouponClick == 1)
		{
			m_bEventCouponClick = 0;
			m_bEventCouponOpen = 1;
		}

		m_pGridShop->Empty();

		for (int i = 0; i < 27; ++i)
		{
			auto pItemList = new STRUCT_ITEM;
			memcpy(pItemList, &pShopList->List[i], sizeof(STRUCT_ITEM));

			if (pShopList->List[i].sIndex <= 0)
			{
				delete pItemList;
				continue;
			}

			auto pItem = new SGridControlItem(0, pItemList, 0.0f, 0.0f);
			if (!pItem)
			{
				delete pItemList;
				continue;
			}
			// Native WYD 7.48 FUN_004875c0 lays merchant entries out in three
			// bands (rows 0, 3 and 6) across nine columns.  Keep TMProject's
			// newer 5x8 layout only for its own UI resource.
			const bool added = m_bCompatFieldScene
				? m_pGridShop->AddItem(pItem, i % 9, (i / 9) * 3) == 1
				: m_pGridShop->AddItem(pItem, i % 5, i / 5) == 1;
			if (!added)
			{
				SAFE_DELETE(pItem);
				continue;
			}

			int nAmount = BASE_GetItemAmount(pItemList);
			if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
				nAmount = 0;
			if (nAmount > 0)
			{
				sprintf_s(pItem->m_GCText.strString, "%2d", nAmount);

				pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
			}
		}

		g_pObjectManager->m_nTax = pShopList->Tax;
		SetVisibleShop(1);
	}
	else if (pShopList->ShopType == 3)
	{
		// The native 7.48 window is composed of independently bound controls.
		// Reject a damaged/incomplete resource tree instead of dereferencing the
		// first missing child and terminating the process inside packet dispatch.
		if (!m_pGridSkillMaster || !m_pSkillMPanel ||
			!m_pSkillMSec1 || !m_pSkillMSec2 || !m_pSkillMSec3)
		{
			WYD748_DiagnosticsLog(
				"skill-master controls missing grid=%p panel=%p sections=%p/%p/%p\r\n",
				m_pGridSkillMaster, m_pSkillMPanel,
				m_pSkillMSec1, m_pSkillMSec2, m_pSkillMSec3);
			return 0;
		}
		if (pShopList->List[0].sIndex < 5000)
		{
			WYD748_DiagnosticsLog(
				"skill-master malformed first item=%d\r\n",
				pShopList->List[0].sIndex);
			return 0;
		}

		m_pGridSkillMaster->Empty();

		switch ((pShopList->List[0].sIndex - 5000) / 24)
		{
		case 0:
			m_pSkillMSec1->SetText(g_pMessageStringTable[107], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[108], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[109], 0);
			break;
		case 1:
			m_pSkillMSec1->SetText(g_pMessageStringTable[110], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[111], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[112], 0);
			break;
		case 2:
			m_pSkillMSec1->SetText(g_pMessageStringTable[113], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[114], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[115], 0);
			break;
		case 3:
			m_pSkillMSec1->SetText(g_pMessageStringTable[133], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[134], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[135], 0);
			break;
		}

		for (int j = 0; j + 2 < 27; ++j)
		{
			if (pShopList->List[j].sIndex == 5027)
			{
				// The stock three-way reorder reads j+1 and j+2.  Bounding the scan
				// preserves it while preventing a malformed list tail from escaping
				// the fixed 27-entry 7.48 packet.
				std::swap(pShopList->List[j], pShopList->List[j + 1]);
				std::swap(pShopList->List[j + 1], pShopList->List[j + 2]);
				break;
			}
		}

		for (int k = 0; k < 27; ++k)
		{
			auto dst = new STRUCT_ITEM;
			memcpy(dst, &pShopList->List[k], sizeof(STRUCT_ITEM));

			if (pShopList->List[k].sIndex <= 0)
			{
				delete dst;
				continue;
			}

			auto pItem = new SGridControlItem(0, dst, 0.0f, 0.0f);
			if (!pItem)
			{
				delete dst;
				continue;
			}
			if (!m_pGridSkillMaster->AddItem(pItem, k % 9 % 4, k / 9 + (k - k / 9) / 4))
				SAFE_DELETE(pItem);
		}

		if (!m_pSkillMPanel->IsVisible())
			SetVisibleSkillMaster();
	}

	return 1;
}

int TMFieldScene::OnPacketRMBShopList(MSG_RMBShopList* pMsg)
{
	if (pMsg->ShopType == 1)
	{
		if (!m_pGridShop)
		{
			WYD748_DiagnosticsLog("rmb merchant grid missing for ShopList\r\n");
			return 0;
		}

		if (m_bEventCouponClick == 1)
		{
			m_bEventCouponClick = 0;
			m_bEventCouponOpen = 1;
		}

		auto pGrid = m_pGridShop;
		pGrid->Empty();

		// A native 7.48 merchant packet owns 27 entries.  The 39-entry RMB
		// extension is valid only with TMProject's newer UI and would address
		// rows that do not exist in FieldScene2.bin.
		const int shopEntryCount = m_bCompatFieldScene ? 27 : 39;
		for (int i = 0; i < shopEntryCount; ++i)
		{
			auto pItemList = new STRUCT_ITEM;
			memcpy(pItemList, &pMsg->List[i], sizeof(STRUCT_ITEM));

			if (pMsg->List[i].sIndex <= 0)
			{
				delete pItemList;
				continue;
			}

			auto pItem = new SGridControlItem(0, pItemList, 0.0f, 0.0f);
			if (!pItem)
			{
				delete pItemList;
				continue;
			}

			// The compatibility UI keeps the native 7.48 nine-column merchant
			// mapping even if a newer server sends the RMB-shop packet family.
			const bool added = m_bCompatFieldScene
				? pGrid->AddItem(pItem, i % 9, (i / 9) * 3) == 1
				: pGrid->AddItem(pItem, i % 5, i / 5) == 1;
			if (!added)
			{
				SAFE_DELETE(pItem);
				continue;
			}
			int nAmount = BASE_GetItemAmount(pItemList);
			if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
				nAmount = 0;
			if (nAmount > 0)
			{
				sprintf(pItem->m_GCText.strString, "%2d", nAmount);
				pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
			}
		}

		g_pObjectManager->m_nTax = pMsg->Tax;
		SetVisibleShop(1);
	}
	else if (pMsg->ShopType == 3)
	{
		// RMB and ordinary shop-list packets converge on the same stock 7.48
		// Skill Apprentice controls.  Validate the complete native control set in
		// both handlers so an alternate packet family cannot reintroduce the null
		// dereference fixed in OnPacketShopList.
		if (!m_pGridSkillMaster || !m_pSkillMPanel ||
			!m_pSkillMSec1 || !m_pSkillMSec2 || !m_pSkillMSec3)
		{
			WYD748_DiagnosticsLog(
				"rmb skill-master controls missing grid=%p panel=%p sections=%p/%p/%p\r\n",
				m_pGridSkillMaster, m_pSkillMPanel,
				m_pSkillMSec1, m_pSkillMSec2, m_pSkillMSec3);
			return 0;
		}

		if (pMsg->List[0].sIndex < 5000)
		{
			// The first skill determines the three section captions in the native
			// packet.  A missing/out-of-range row is malformed and must not index the
			// message table or construct catalog-backed grid objects.
			WYD748_DiagnosticsLog(
				"rmb skill-master malformed first item=%d\r\n",
				pMsg->List[0].sIndex);
			return 0;
		}

		switch ((pMsg->List[0].sIndex - 5000) / 24)
		{
		case 0:
			m_pSkillMSec1->SetText(g_pMessageStringTable[107], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[108], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[109], 0);
			break;
		case 1:
			m_pSkillMSec1->SetText(g_pMessageStringTable[110], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[111], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[112], 0);
			break;
		case 2:
			m_pSkillMSec1->SetText(g_pMessageStringTable[113], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[114], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[115], 0);
			break;
		case 3:
			m_pSkillMSec1->SetText(g_pMessageStringTable[133], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[134], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[135], 0);
			break;
		}

		for (int j = 0; j + 2 < 27; ++j)
		{
			if (pMsg->List[j].sIndex == 5027)
			{
				// The stock three-way reorder consumes j+1 and j+2.  Keep its
				// fixed 27-entry packet boundary explicit for malformed list tails.
				std::swap(pMsg->List[j], pMsg->List[j + 1]);
				std::swap(pMsg->List[j + 1], pMsg->List[j + 2]);
				break;
			}
		}

		auto pGridSkillMaster = m_pGridSkillMaster;
		pGridSkillMaster->Empty();
		for (int k = 0; k < 27; ++k)
		{
			auto pNewItem = new STRUCT_ITEM;
			memcpy(pNewItem, &pMsg->List[k], sizeof(STRUCT_ITEM));

			if (pMsg->List[k].sIndex <= 0)
			{
				delete pNewItem;
				continue;
			}

			auto pItem = new SGridControlItem(0, pNewItem, 0.0f, 0.0f);
			if (!pItem)
			{
				delete pNewItem;
				continue;
			}
			if (!pGridSkillMaster->AddItem(pItem, k % 9 % 4, k / 9 + (k - k / 9) / 4))
				SAFE_DELETE(pItem);
		}

		if (!m_pSkillMPanel->IsVisible())
			SetVisibleSkillMaster();
	}

	return 1;
}

int TMFieldScene::OnPacketBuy(MSG_STANDARD* pStd)
{
	// Native 7.48 FUN_00487b92 confirms a purchase with the same 0x379 packet:
	// copy the advertised shop item into the server-selected Carry cell, then
	// adopt the authoritative Coin value.  SendItem/UpdateEtc cannot replace
	// this lifecycle because only 0x379 materializes the bought grid control.
	if (!pStd || !m_pGridShop || !m_pGridInv || !g_pObjectManager)
		return 1;

	auto pBuy = reinterpret_cast<MSG_Buy*>(pStd);
	if (!IsBuyShopPosition(pBuy->TargetCarryPos) ||
		!IsBuyCarryPosition(pBuy->MyCarryPos))
		return 1;

	// The merchant identity binds the confirmation to the list currently shown;
	// accepting a stale packet would copy an item from an unrelated shop panel.
	if (m_pGridShop->m_dwMerchantID != pBuy->TargetID)
		return 1;

	auto pShopItem = m_pGridShop->GetItem(pBuy->TargetCarryPos % 9,
		pBuy->TargetCarryPos / 9);
	if (!pShopItem || !pShopItem->m_pItem || pShopItem->m_pItem->sIndex <= 0)
		return 1;

	STRUCT_ITEM boughtItem{};
	memcpy(&boughtItem, pShopItem->m_pItem, sizeof(STRUCT_ITEM));

	// The allocated payload belongs to the visual control. The local copy
	// remains valid if the grid rejects the control and its destructor frees it.
	auto pStructItem = new STRUCT_ITEM;
	if (pStructItem)
	{
		memcpy(pStructItem, &boughtItem, sizeof(STRUCT_ITEM));
		auto pControlItem = new SGridControlItem(m_pGridInv, pStructItem, 0.0f, 0.0f);
		if (!pControlItem)
			delete pStructItem;
		else if (!m_pGridInv->AddItem(pControlItem, pBuy->MyCarryPos % 9,
			pBuy->MyCarryPos / 9))
			SAFE_DELETE(pControlItem);
	}

	// The server chooses MyCarryPos after validating the 9x7 Carry. The logical
	// cache must accept this confirmation even if the visual representation fails.
	memcpy(&g_pObjectManager->m_stMobData.Carry[pBuy->MyCarryPos],
		&boughtItem, sizeof(STRUCT_ITEM));
	g_pObjectManager->m_stMobData.Coin = pBuy->Coin;

	GetSoundAndPlay(31, 0, 0);
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketSell(MSG_STANDARD* pStd)
{
	auto pSell = reinterpret_cast<MSG_Sell*>(pStd);
	if (!pSell || !g_pObjectManager ||
		pSell->MyType < 0 || pSell->MyType > 1 || pSell->MyPos < 0 ||
		(pSell->MyType == 0 && pSell->MyPos >= MAX_EQUIPITEM) ||
		(pSell->MyType == 1 && pSell->MyPos >= MAX_CARRY) ||
		WYD748_IsUnsupportedCompatEquipSlot(m_bCompatFieldScene,
		pSell->MyType == 0 ? pSell->MyPos : -1))
		return 1;

	const bool knownMerchant =
		(m_pGridHellStore && m_pGridHellStore->m_dwMerchantID == pSell->TargetID) ||
		(m_pGridShop && m_pGridShop->m_dwMerchantID == pSell->TargetID) ||
		(!pSell->TargetID &&
		g_pObjectManager->m_stMobData.Class == 3 &&
		(g_pObjectManager->m_stMobData.LearnedSkill[0] & 0x1000));
	if (!knownMerchant)
		return 1;

	SGridControlItem* pDestItem{};
	if (pSell->MyType == 0)
	{
		SGridControl* pGridDest[MAX_EQUIPITEM]{};

		pGridDest[0] = m_pGridInv;
		pGridDest[1] = m_pGridHelm;
		pGridDest[2] = m_pGridCoat;
		pGridDest[3] = m_pGridPants;
		pGridDest[4] = m_pGridGloves;
		pGridDest[5] = m_pGridBoots;
		pGridDest[6] = m_pGridLeft;
		pGridDest[7] = m_pGridRight;
		pGridDest[8] = m_pGridRing;
		pGridDest[9] = m_pGridNecklace;
		pGridDest[10] = m_pGridOrb;
		pGridDest[11] = m_pGridCabuncle;
		pGridDest[12] = m_pGridGuild;
		pGridDest[13] = m_pGridEvent;
		pGridDest[14] = m_pGridDRing;
		pGridDest[15] = m_pGridMantua;
		pGridDest[16] = m_pGridNewSlot1;
		pGridDest[17] = m_pGridNewSlot2;
		if (pGridDest[pSell->MyPos])
			pDestItem = pGridDest[pSell->MyPos]->PickupItem(0, 0);
	}
	else
	{
		// Resolve Carry through the active topology, not newer 15-slot pages.
		int cellX = 0;
		int cellY = 0;
		GetCarryCellForSlot(pSell->MyPos, cellX, cellY);
		SGridControl* carryGrid = GetCarryGridForSlot(pSell->MyPos);
		pDestItem = carryGrid ? carryGrid->PickupAtItem(cellX, cellY) : nullptr;
	}
	if (!pDestItem)
		return 1;

	const bool hasItem = pDestItem->m_pItem != nullptr;
	const int itemIndex = hasItem ? pDestItem->m_pItem->sIndex : 0;
	// Pickup transferred ownership out of the grid. Clear only aliases to
	// this visual, including when a partially constructed item has no payload.
	if (SGridControl::m_pLastMouseOverItem == pDestItem)
	{
		SGridControl::m_pLastMouseOverItem = nullptr;
		SGridControl::m_sLastMouseOverIndex = -1;
	}
	if (SGridControl::m_pLastAttachedItem == pDestItem)
		SGridControl::m_pLastAttachedItem = nullptr;
	if (SGridControl::m_pSellItem == pDestItem)
		SGridControl::m_pSellItem = nullptr;
	if (g_pCursor && g_pCursor->m_pAttachedItem == pDestItem)
		g_pCursor->DetachItem();
	SAFE_DELETE(pDestItem);
	if (!hasItem)
		return 1;

	// Use the native response bands without intermediate float32 rounding.
	// WYD-Go does not use this reply:
	// its sale path sends authoritative SendItem and UpdateEtc snapshots.
	const int catalogPrice = itemIndex > 0 && itemIndex < MAX_ITEMLIST
		? g_pItemList[itemIndex].nPrice : 0;
	const int nPrice = native_sale_price::Calculate(catalogPrice);

	STRUCT_ITEM* soldSlot = pSell->MyType == 0
		? &g_pObjectManager->m_stMobData.Equip[pSell->MyPos]
		: &g_pObjectManager->m_stMobData.Carry[pSell->MyPos];
	memset(soldSlot, 0, sizeof(STRUCT_ITEM));
	g_pObjectManager->m_stMobData.Coin += nPrice;
	UpdateScoreUI(0);
	GetSoundAndPlay(31, 0, 0);
	if (m_pMyHuman)
		UpdateMyHuman();
	return 1;
}

int TMFieldScene::OnPacketCloseShop(MSG_STANDARD* pStd)
{
	SetVisibleShop(0);
	return 1;
}

int TMFieldScene::OnPacketItemPrice(MSG_STANDARDPARM2* pStd)
{
	g_pItemList[412].nPrice = pStd->Parm1;
	g_pItemList[413].nPrice = pStd->Parm2;
	return 1;
}

void TMFieldScene::BuyItemNewStore(int idwControlID)
{
	struct
	{
		MSG_STANDARD	Header;
		char			Cmd[16];
		char			Msg[100];
	}Packet;
	memset(&Packet, 0x0, sizeof(Packet));

	Packet.Header.Size = sizeof(Packet);
	Packet.Header.Type = 0x334;
	Packet.Header.ID = g_pObjectManager->m_dwCharID;

	strcpy(Packet.Cmd, "xLojaDonateX");
	strcpy(Packet.Msg, strfmt("%d %d %d",
		ControlLojaDonateInfor.Type,
		ControlLojaDonateInfor.Page,
		ControlLojaDonateInfor.Slot
	));

	SendOneMessage((char*)&Packet, sizeof(Packet));
}

void TMFieldScene::UpdateNewStore(int idwControlID)
{

	auto GuiAlvo = (SPanel*)m_pControlContainer->FindControl(3000011); // donation store entry point

	if (GuiAlvo == NULL || !GuiAlvo->m_bVisible)
		return;

	if (idwControlID == 3000060)
		GuiAlvo->m_bVisible = false;

#pragma region Type_Produt_Change
	if (idwControlID >= 3000064 && idwControlID <= 3000067)
	{
		ControlLojaDonateInfor.Type = idwControlID - 3000064;
		ControlLojaDonateInfor.Page = 0;

		for (int i = 0; i < 12; i++)
		{
			auto GuildSlotEmpty = (SGridControl*)m_pControlContainer->FindControl(3000012 + (i * 4));

			GuildSlotEmpty->Empty();

			auto LabelPrice = (SText*)m_pControlContainer->FindControl(3000013 + (i * 4));
			auto LabelEstoque = (SText*)m_pControlContainer->FindControl(3000014 + (i * 4));
			char str[80];

			sprintf(str, "");
			LabelPrice->SetText(str, 0);
			LabelEstoque->SetText(str, 0);
		}

		for (auto& i : ControlLojaDonate)
		{
			try
			{
				if (i.type != ControlLojaDonateInfor.Type || i.page != ControlLojaDonateInfor.Page)
					continue;

				if (i.item.sIndex == NULL)
					continue;

					int SlotGui = i.slot -1;

					auto GuiItem = (SGridControl*)m_pControlContainer->FindControl(3000012 + (SlotGui * 4));
					auto LabelPrice = (SText*)m_pControlContainer->FindControl(3000013 + (SlotGui * 4));
					auto LabelEstoque = (SText*)m_pControlContainer->FindControl(3000014 + (SlotGui * 4));

					auto pREQItem = new STRUCT_ITEM;
					memset(pREQItem, 0, sizeof(STRUCT_ITEM));
					pREQItem->sIndex = i.item.sIndex;

					auto pItem = new SGridControlItem(0, pREQItem, 0.0f, 0.0f);
					GuiItem->AddItem(pItem, 0, 0);

					if (g_pApp->m_dwScreenWidth == 1280)
					{pItem->m_nHeight = 35;pItem->m_nWidth = 35;}
					if (g_pApp->m_dwScreenWidth == 1024)
					{pItem->m_nHeight = 38; pItem->m_nWidth = 28;}
					if (g_pApp->m_dwScreenWidth == 800)
					{pItem->m_nHeight = 30; pItem->m_nWidth = 22;}
					if (g_pApp->m_dwScreenWidth == 640)
					{pItem->m_nHeight = 23; pItem->m_nWidth = 16;}
					if (g_pApp->m_dwScreenHeight == 800)
					{pItem->m_nHeight = 41; pItem->m_nWidth = 35;}

					char str[80];
					char str1[80]{};

					sprintf(str, "$: %d", i.price);
					LabelPrice->SetText(str, 0);

					sprintf(str1, "Quantity: %d", i.stuck);
					LabelEstoque->SetText(str1, 0);
			}
			catch (...)
			{

			}
		}

	}
#pragma endregion
#pragma region Page_Produt_Change
	if (idwControlID >= 3000061 && idwControlID <= 3000063)
	{
		ControlLojaDonateInfor.Page = idwControlID - 3000061;

		for (int i = 0; i < 12; i++)
		{
			auto GuildSlotEmpty = (SGridControl*)m_pControlContainer->FindControl(3000012 + (i * 4));

			GuildSlotEmpty->Empty();

			auto LabelPrice = (SText*)m_pControlContainer->FindControl(3000013 + (i * 4));
			auto LabelEstoque = (SText*)m_pControlContainer->FindControl(3000014 + (i * 4));
			char str[80];

			sprintf(str, "");
			LabelPrice->SetText(str, 0);
			LabelEstoque->SetText(str, 0);
		}

		for (auto& i : ControlLojaDonate)
		{
			try
			{
				if (i.type != ControlLojaDonateInfor.Type || i.page != ControlLojaDonateInfor.Page)
					continue;

				if (i.item.sIndex == NULL)
					continue;

				int SlotGui = i.slot -1;

				auto GuiItem = (SGridControl*)m_pControlContainer->FindControl(3000012 + (SlotGui * 4));
				auto LabelPrice = (SText*)m_pControlContainer->FindControl(3000013 + (SlotGui * 4));
				auto LabelEstoque = (SText*)m_pControlContainer->FindControl(3000014 + (SlotGui * 4));

				auto pREQItem = new STRUCT_ITEM;
				memset(pREQItem, 0, sizeof(STRUCT_ITEM));
				pREQItem->sIndex = i.item.sIndex;

				auto pItem = new SGridControlItem(0, pREQItem, 0.0f, 0.0f);
				GuiItem->AddItem(pItem, 0, 0);

				if (g_pApp->m_dwScreenWidth == 1280)
				{
					pItem->m_nHeight = 35; pItem->m_nWidth = 35;
				}
				if (g_pApp->m_dwScreenWidth == 1024)
				{
					pItem->m_nHeight = 38; pItem->m_nWidth = 28;
				}
				if (g_pApp->m_dwScreenWidth == 800)
				{
					pItem->m_nHeight = 30; pItem->m_nWidth = 22;
				}
				if (g_pApp->m_dwScreenWidth == 640)
				{
					pItem->m_nHeight = 23; pItem->m_nWidth = 16;
				}
				if (g_pApp->m_dwScreenHeight == 800)
				{
					pItem->m_nHeight = 41; pItem->m_nWidth = 35;
				}

				char str[80];
				char str1[80]{};

				sprintf(str, "$: %d", i.price);
				LabelPrice->SetText(str, 0);

				sprintf(str, "Quantity: %d", i.stuck);
				LabelEstoque->SetText(str, 0);

			}
			catch (...)
			{

			}
		}
	}
#pragma endregion

#pragma region Button_Buy
	int Buttons[] =
	{
		3000015,
		3000019,
		3000023,
		3000027,
		3000031,
		3000035,
		3000039,
		3000043,
		3000047,
		3000051,
		3000055,
		3000059
	};

	for (const int buttonControlId : Buttons)
	{
		/* Handle */
		if (buttonControlId == idwControlID)
		{
			int ButtonIndex = ((idwControlID - 3000015) / 4) + 1;
			int GridIndex = idwControlID - 3;
			auto GridSlot = (SGridControl*)m_pControlContainer->FindControl(GridIndex);
			if (!GridSlot)
				break;

			auto Item = GridSlot->GetItem(0, 0);
			if (Item && Item->m_pItem && Item->m_pItem->sIndex > 0 &&
				Item->m_pItem->sIndex < MAX_ITEMLIST)
			{
				auto panelbuy = (SGridControl*)m_pControlContainer->FindControl(3000080);
				auto labelbuyitem = (SText*)m_pControlContainer->FindControl(3000083);
				if (!panelbuy || !labelbuyitem)
					break;

				ControlLojaDonateInfor.Slot = ButtonIndex;
				char msg[102] = { 0, };
				panelbuy->m_bVisible = true;

				sprintf_s(msg, 102, "Buy item %s", g_pItemList[Item->m_pItem->sIndex].Name);
				labelbuyitem->SetText(msg, 0);

			}
			break;
		}
	}

	return;
}

int TMFieldScene::OnPacketNewCashRev(PacketRevDonate* P)
{
	for (auto& i : ControlLojaDonate)
	{
		if (i.type != P->type)
			continue;

		if (i.page != P->page)
			continue;

		if (i.slot != P->slot)
			continue;

		i.stuck = P->stuck;
		i.price = P->price;
		memcpy_s(&i.item, sizeof(STRUCT_ITEM), &P->item, sizeof(STRUCT_ITEM));
		break;
	}

	if (ControlLojaDonateInfor.Type == P->type)
		if (ControlLojaDonateInfor.Page == P->page)

		{
			int SlotGui = P->slot -1;
			auto GuiItem = (SGridControl*)m_pControlContainer->FindControl(3000012 + (SlotGui * 4));
			auto LabelPrice = (SText*)m_pControlContainer->FindControl(3000013 + (SlotGui * 4));
			auto LabelEstoque = (SText*)m_pControlContainer->FindControl(3000014 + (SlotGui * 4));
			auto pREQItem = new STRUCT_ITEM;
			memset(pREQItem, 0, sizeof(STRUCT_ITEM));
			pREQItem->sIndex = P->item.sIndex;

			auto pItem = new SGridControlItem(0, pREQItem, 0.0f, 0.0f);
			GuiItem->AddItem(pItem, 0, 0);
			if (g_pApp->m_dwScreenWidth == 1280)
			{
				pItem->m_nHeight = 35; pItem->m_nWidth = 35;
			}
			if (g_pApp->m_dwScreenWidth == 1024)
			{
				pItem->m_nHeight = 38; pItem->m_nWidth = 28;
			}
			if (g_pApp->m_dwScreenWidth == 800)
			{
				pItem->m_nHeight = 30; pItem->m_nWidth = 22;
			}
			if (g_pApp->m_dwScreenWidth == 640)
			{
				pItem->m_nHeight = 23; pItem->m_nWidth = 16;
			}
			if (g_pApp->m_dwScreenHeight == 800)
			{
				pItem->m_nHeight = 41; pItem->m_nWidth = 35;
			}
			char str[80];
			char str1[80]{};

			sprintf(str, "$: %d", P->price);
			LabelPrice->SetText(str, 0);

			sprintf(str, "Quantity: %d", P->stuck);
			LabelEstoque->SetText(str, 0);

		}

	return 1;
}

int TMFieldScene::OnPacketNewBuyCash(MSG_STANDARD* pStd)
{
	struct
	{
		MSG_STANDARD	Header;
		char			Cmd[16];
		char			Msg[100];
	}Packet;
	memset(&Packet, 0x0, sizeof(Packet));

	Packet.Header.Size = sizeof(Packet);
	Packet.Header.Type = 0x334;
	Packet.Header.ID = g_pObjectManager->m_dwCharID;

	strcpy(Packet.Cmd, "xLojaDonateX");
	strcpy(Packet.Msg, strfmt("%d %d %d",
		ControlLojaDonateInfor.Type,
		ControlLojaDonateInfor.Page,
		ControlLojaDonateInfor.Slot
	));

	SendOneMessage((char*)&Packet, sizeof(Packet));
	return 1;
}

int TMFieldScene::OnPacketNewCashRev2(PacketRevDonate2* pStd)
{
	ControlLojaDonate.clear();

	for (int i = 0; i < pStd->quantidade; i++)
	{
		auto Temp = LojaDonate(
			pStd->Produts[i].type,
			pStd->Produts[i].page,
			pStd->Produts[i].item,
			pStd->Produts[i].price,
			pStd->Produts[i].stuck,
			pStd->Produts[i].slot
		);

		ControlLojaDonate.push_back(Temp);
	}

	return 1;
}

void TMFieldScene::MouseClick_PremiumNPC(TMHuman* pOver)
{
	// Preserved later extension point. Native 7.48 only classifies head 57 as a
	// merchant; no Premium request contract has been recovered or implemented.
	(void)pOver;
}
