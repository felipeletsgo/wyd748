#include "pch.h"
#include "TMFieldScene.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "TMItem.h"
#include "ClientDiagnostics.h"
#include "WYD748Assets.h"
#include "../../application/FieldInteractionPolicy.h"
#include "FieldSceneTradeSupport.h"

	void WYD748_ResetTradeOffer(MSG_Trade& trade, unsigned short opponentID)
	{
		memset(&trade, 0, sizeof(trade));
		for (int i = 0; i < 15; ++i)
			trade.CarryPos[i] = -1;
		trade.OpponentID = opponentID;
	}

	void WYD748_LogTradeSend(const char* origin, const MSG_Trade& trade)
	{
		WYD748_DiagnosticsLog(
			"TRADE_SEND origin=%s opponent=%u carry0=%d item0=%d size=%u\r\n",
			origin, trade.OpponentID, static_cast<int>(trade.CarryPos[0]),
			trade.Item[0].sIndex, static_cast<unsigned int>(sizeof(trade)));
	}

	void WYD748_CancelAutoTradePurchase(SMessageBox* dialog, int invalidatedSlot)
	{
		if (!dialog || !field_interaction::ShouldCancelAutoTradePurchase(
			dialog->m_dwMessage, dialog->m_dwArg, invalidatedSlot))
			return;

		// Hiding alone retains the callback discriminator and argument. Invalidate
		// them too so a queued confirmation cannot purchase a replacement offer.
		dialog->m_dwMessage = static_cast<unsigned int>(-1);
		dialog->m_dwArg = 0;
		if (dialog->IsVisible())
			dialog->SetVisible(0);
	}

	void WYD748_ReleaseAutoTradeItem(SGridControlItem*& pItem)
	{
		if (!pItem)
			return;

		// Pickup transfers ownership without clearing interaction aliases. A
		// sale, snapshot replacement, or panel close can invalidate selections.
		if (SGridControl::m_pLastMouseOverItem == pItem)
		{
			SGridControl::m_pLastMouseOverItem = nullptr;
			SGridControl::m_sLastMouseOverIndex = -1;
		}
		if (SGridControl::m_pLastAttachedItem == pItem)
			SGridControl::m_pLastAttachedItem = nullptr;
		if (SGridControl::m_pSellItem == pItem)
			SGridControl::m_pSellItem = nullptr;
		if (g_pCursor && g_pCursor->m_pAttachedItem == pItem)
			g_pCursor->DetachItem();
		SAFE_DELETE(pItem);
	}

void TMFieldScene::SetVisibleTrade(int bShow)
{
	if (!m_pControlContainer || !m_pMyHuman || !g_pObjectManager ||
		!g_pApp || !g_pApp->m_pTimerManager)
		return;

	SGridControl::m_sLastMouseOverIndex = -1;

	auto pBtnChar = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));
	auto pBtnInv = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));
	auto pMyGold = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_MYGOLD));
	auto pOPGold = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_OPGOLD));

	if (pOPGold)
		pOPGold->m_cComma = 1;
	if (pMyGold)
		pMyGold->m_cComma = 1;

	bool bSendQuit = false;

	if (!m_pTradePanel)
		return;

	if (m_pTradePanel->IsVisible() == 1 && !bShow)
		bSendQuit = true;

	if (bShow == 1)
	{
		if (g_pCursor)
			g_pCursor->DetachItem();
		if (m_pGambleStore)
			SetVisibleGamble(0, 0);

		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);
		PositionCompatTradePanels();

		if (m_pSystemPanel)
			m_pSystemPanel->SetVisible(0);
		if (m_pCPanel)
			m_pCPanel->SetVisible(0);
		if (m_pCargoPanel)
			m_pCargoPanel->SetVisible(0);
		m_pTradePanel->SetVisible(bShow);
		if (m_pInvenPanel)
			m_pInvenPanel->SetVisible(bShow);
		if (m_pSkillPanel)
			m_pSkillPanel->SetVisible(0);
		if (m_pSkillMPanel)
			m_pSkillMPanel->SetVisible(0);
		if (m_pShopPanel)
			m_pShopPanel->SetVisible(0);
		if (m_pHellgateStore)
			m_pHellgateStore->SetVisible(0);
		if (pBtnInv)
			pBtnInv->SetSelected(0);
		if (pBtnChar)
			pBtnChar->SetSelected(0);
		if (m_pSkillPanel && m_pSkillPanel->IsVisible() == 1)
			m_pSkillPanel->SetVisible(0);
	}
	else
	{
		// Restore trade highlighting across the native 9x7 Carry grid instead of
		// walking the four page controls that do not exist in FieldScene2.
		const int carryPages = m_bCompatFieldScene ? 1 : 4;
		const int carryRows = m_bCompatFieldScene ? 7 : 3;
		const int carryColumns = m_bCompatFieldScene ? 9 : 5;
		for (int i = 0; i < carryPages; ++i)
		{
			SGridControl* pGridInv = m_bCompatFieldScene ? m_pGridInv : m_pGridInvList[i];
			if (!pGridInv)
				continue;

			for (int nY = 0; nY < carryRows; ++nY)
			{
				for (int nX = 0; nX < carryColumns; ++nX)
				{
					SGridControlItem* pItem = pGridInv->GetItem(nX, nY);

					if (pItem && pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
		if (pMyGold)
			pMyGold->SetText((char*)"         0", 0);
		if (pOPGold)
			pOPGold->SetText((char*)"         0", 0);
		m_pTradePanel->SetVisible(bShow);
		if (m_pInvenPanel)
			m_pInvenPanel->SetVisible(0);
		if (m_pInputGoldPanel)
			m_pInputGoldPanel->SetVisible(0);
	}

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}

	if (!bShow && g_pObjectManager->m_stTrade.OpponentID > 0)
	{
		g_pObjectManager->m_stTrade.OpponentID = 0;
		g_pObjectManager->m_stTrade.MyCheck = 0;
		m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();

		if (bSendQuit)
		{
			MSG_STANDARD stStandard{};

			stStandard.Type = MSG_CloseTrade_Opcode;
			stStandard.ID = m_pMyHuman->m_dwID;
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stStandard)->Type, reinterpret_cast<char*>(&stStandard), sizeof(stStandard)});
		}
	}

	if (bShow == 1)
	{
		SetInventoryGridType(TMEGRIDTYPE::GRID_TRADEINV);
		SetEquipGridState(0);
		// Start every trade window with an empty local offer.  Keep the opponent
		// identity supplied by the invite, but never reuse item bytes or positions
		// from a previous session.
		WYD748_ResetTradeOffer(g_pObjectManager->m_stTrade,
			g_pObjectManager->m_stTrade.OpponentID);
		g_pObjectManager->m_stTrade.Header.Type = MSG_Trade_Opcode;
		g_pObjectManager->m_stTrade.Header.ID = m_pMyHuman->m_dwID;
	}
	else
	{
		SetInventoryGridType(TMEGRIDTYPE::GRID_DEFAULT);
		SetEquipGridState(1);
		WYD748_ResetTradeOffer(g_pObjectManager->m_stTrade, 0);

		SGridControl* pGridOp[15]{};
		SGridControl* pGridMy[15]{};

		for (int l = 0; l < 15; ++l)
		{
			SGridControlItem* pPickedItem{};

			pGridOp[l] = static_cast<SGridControl*>(m_pControlContainer->FindControl(l + TMG_TRADE_OP1));

			if (pGridOp[l])
				pPickedItem = pGridOp[l]->PickupItem(0, 0);

			if (g_pCursor && g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = nullptr;

			if (pPickedItem)
			{
				delete pPickedItem;

				pPickedItem = nullptr;
			}

			pGridMy[l] = static_cast<SGridControl*>(m_pControlContainer->FindControl(l + TMG_TRADE_MY1));

			if (pGridMy[l])
				pPickedItem = pGridMy[l]->PickupItem(0, 0);

			if (g_pCursor && g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = nullptr;

			if (pPickedItem)
				delete pPickedItem;
		}

		auto pOpCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_TRADE_OPCHECK));
		auto pMyCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_TRADE_MYCHECK));

		if (pOpCheckButton)
			pOpCheckButton->m_bSelected = 0;
		if (pMyCheckButton)
			pMyCheckButton->m_bSelected = 0;

		m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();

		if (m_pInvenPanel && m_pInvenPanel->IsVisible() == 1)
			SetVisibleInventory();

	}
}

void TMFieldScene::SetVisibleAutoTrade(int bShow, int bCargo)
{
	if (!bShow)
		WYD748_CancelAutoTradePurchase(m_pMessageBox);

	SGridControl::m_sLastMouseOverIndex = -1;

	if (m_pInputGoldPanel && m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();

	if (m_bCompatFieldScene)
	{
		// Native WYD 7.48 FUN_0044ae38 owns panel 646, twelve grids 653..664,
		// price labels 800..811 and a single inventory/cargo surface.  The imported
		// 7.59 implementation below assumes different panels and page arrays, which
		// is why clicking control 313 previously dereferenced a null panel at +0x28.
		if (!m_pControlContainer || !m_pAutoTrade)
			return;

		auto pBtnCloseAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_CLOSE));
		// FieldScene2.bin owns the native bottom-bar IDs; using the 7.59 aliases
		// left their selected state detached from the visible 7.48 buttons.
		auto pBtnAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_AUTOTRADEBTN));
		auto pBtnChar = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_CHAR));
		auto pBtnInv = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_EQUIP));
		auto pBtnSkill = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SKILL));
		auto pRunAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_RUN));
		auto pMyCargoCoin = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATRADE_COIN));
		auto pMyCargoCoinB = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_COIN));

		if (pBtnCloseAutoTrade)
			pBtnCloseAutoTrade->SetVisible(0);

		const bool bSendQuit = m_pAutoTrade->IsVisible() == 1 && !bShow
			&& (!m_pInvenPanel || !m_pInvenPanel->IsVisible());
		if (pBtnAutoTrade)
			pBtnAutoTrade->SetSelected(bShow);
		m_pAutoTrade->SetVisible(bShow);

		auto setPanelVisible = [](SPanel* panel, int visible)
		{
			if (panel)
				panel->SetVisible(visible);
		};
		auto setEquipGridState748 = [this](bool enabled)
		{
			// FUN_00447f6f exposes fourteen normal equipment cells and keeps Mantua
			// non-tradeable in both states; the two 7.59 extension slots do not exist.
			SGridControl* equipment[] = {
				m_pGridHelm, m_pGridCoat, m_pGridPants, m_pGridGloves,
				m_pGridBoots, m_pGridRight, m_pGridLeft, m_pGridGuild,
				m_pGridEvent, m_pGridRing, m_pGridNecklace, m_pGridOrb,
				m_pGridCabuncle, m_pGridDRing
			};
			for (auto grid : equipment)
			{
				if (grid)
					grid->m_eGridType = enabled
						? TMEGRIDTYPE::GRID_DEFAULT
						: TMEGRIDTYPE::GRID_TRADENONE;
			}
			if (m_pGridMantua)
				m_pGridMantua->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		};

		if (bShow == 1)
		{
			if (g_pCursor)
				g_pCursor->DetachItem();
			setPanelVisible(m_pSystemPanel, 0);
			setPanelVisible(m_pCPanel, 0);
			setPanelVisible(m_pSkillPanel, 0);
			setPanelVisible(m_pSkillMPanel, 0);
			setPanelVisible(m_pShopPanel, 0);

			// FUN_0044ae38 closes normal trade before exposing auto-trade. Keep that
			// transition on the same cleanup path used by every other trade close so
			// grids, equipment, offer items, buttons and gold are reset together.
			if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
				SetVisibleTrade(0);

			if (pBtnInv)
				pBtnInv->SetSelected(0);
			if (pBtnChar)
				pBtnChar->SetSelected(0);
			if (pBtnSkill)
				pBtnSkill->SetSelected(0);

			if (bCargo == 1)
			{
				// Native FUN_0044ae38 places the sale chooser at x=530 while
				// AutoTrade remains at its x=280 anchor.  Keeping both positions
				// explicit prevents the topmost AutoTrade panel from swallowing
				// Cargo clicks and makes the two 7.48 windows truly side by side.
				m_pAutoTrade->SetPos(RenderDevice::m_fWidthRatio * 280.0f,
					RenderDevice::m_fHeightRatio * 35.0f);
				if (m_pCargoPanel)
					m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 530.0f,
						RenderDevice::m_fHeightRatio * 35.0f);
				setPanelVisible(m_pCargoPanel, 1);
				setPanelVisible(m_pInvenPanel, 0);
				if (pRunAutoTrade)
					pRunAutoTrade->SetVisible(1);
				if (pMyCargoCoin)
					pMyCargoCoin->SetVisible(1);
				if (pMyCargoCoinB)
					pMyCargoCoinB->SetVisible(1);
				for (int slot = 0; slot < 12; ++slot)
				{
					if (m_pGridAutoTrade[slot])
						m_pGridAutoTrade[slot]->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
				}
			}
			else
			{
				// The classic branch of native FUN_0044ae38 only exposes Carry here;
				// it does not rewrite root 257. Preserve the responsive position built
				// by FUN_00435b13 so later Inventory toggles cannot inherit an
				// AutoTrade-only coordinate.
				m_pAutoTrade->SetPos(RenderDevice::m_fWidthRatio * 280.0f,
					RenderDevice::m_fHeightRatio * 35.0f);
				setPanelVisible(m_pCargoPanel, 0);
				setPanelVisible(m_pInvenPanel, 1);
				if (pRunAutoTrade)
					pRunAutoTrade->SetVisible(0);
				if (pMyCargoCoin)
					pMyCargoCoin->SetVisible(0);
				if (pMyCargoCoinB)
					pMyCargoCoinB->SetVisible(0);
				for (int slot = 0; slot < 12; ++slot)
				{
					if (m_pGridAutoTrade[slot])
						m_pGridAutoTrade[slot]->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY2;
				}
				if (m_pGridInv)
					m_pGridInv->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
				setEquipGridState748(false);
			}
			if (m_pCargoGrid)
				m_pCargoGrid->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
		}
		else
		{
			if (m_pMyHuman)
			{
				m_pMyHuman->m_TradeDesc[0] = 0;
				if (m_pMyHuman->m_pAutoTradeDesc)
					m_pMyHuman->m_pAutoTradeDesc->SetText(m_pMyHuman->m_TradeDesc, 0);
				m_stAutoTrade.Header.ID = m_pMyHuman->m_dwID;
			}
			m_stAutoTrade.TargetID = 0;

			if (m_pInvenPanel && m_pInvenPanel->IsVisible() == 1)
			{
				if (m_pGridInv)
					m_pGridInv->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
				setEquipGridState748(true);
			}

			for (int slot = 0; slot < 12; ++slot)
			{
				// Native labels 800..811 are price state, not decoration; clearing them
				// with their comma mode prevents values leaking into the next trade.
				auto pPrice = static_cast<SText*>(m_pControlContainer->FindControl(800 + slot));
				if (pPrice)
				{
					pPrice->m_cComma = 1;
					pPrice->SetText((char*)"", 0);
					pPrice->SetVisible(0);
				}

				auto pGrid = m_pGridAutoTrade[slot];
				if (!pGrid)
					continue;
				auto pItem = pGrid->PickupAtItem(0, 0);
				int cargoX = 0;
				int cargoY = 0;
				GetCargoCellForSlot(m_stAutoTrade.CarryPos[slot], cargoX, cargoY);
				auto pCargoGrid = GetCargoGridForSlot(m_stAutoTrade.CarryPos[slot]);
				auto pSrcItem = pCargoGrid ? pCargoGrid->GetAtItem(cargoX, cargoY) : nullptr;
				if (pSrcItem)
					pSrcItem->m_GCObj.dwColor = 0xFFFFFFFF;
				WYD748_ReleaseAutoTradeItem(pItem);
			}

			setPanelVisible(m_pInvenPanel, 0);
			setPanelVisible(m_pCargoPanel, 0);
			if (g_pDevice)
				g_pDevice->m_nWidthShift = 0;
			if (m_pCargoGrid)
				m_pCargoGrid->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
			memset(&m_stAutoTrade, 0, sizeof(m_stAutoTrade));

			if (bSendQuit && m_pMyHuman)
			{
				MSG_STANDARD quitAutoTrade{};
				quitAutoTrade.Type = 0x384;
				quitAutoTrade.ID = m_pMyHuman->m_dwID;
				SendOneMessage(reinterpret_cast<char*>(&quitAutoTrade), sizeof(quitAutoTrade));
			}
		}
		return;
	}

	if (m_pAutoTrade)
	{
		auto pBtnCloseAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_CLOSE));
		auto pBtnAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(B_AUTOTRADEBTN));
		auto pBtnChar = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));
		auto pBtnInv = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));
		auto pMyCargoCoin = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATRADE_COIN));
		auto pMyCargoCoinB = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_COIN));

		if (pBtnCloseAutoTrade)
			pBtnCloseAutoTrade->SetVisible(0);

		bool bSendQuit = false;

		if (pBtnAutoTrade)
			pBtnAutoTrade->SetSelected(bShow);

		if (m_pAutoTrade)
		{
			if (m_pAutoTrade->IsVisible() == 1 && !bShow && !m_pInvenPanel->IsVisible())
				bSendQuit = true;

			m_pAutoTrade->SetVisible(bShow);
			m_pAutoTrade->SetPos(RenderDevice::m_fWidthRatio * 254.0f, RenderDevice::m_fHeightRatio * 35.0f);
		}
		if (bShow == 1)
		{
			g_pCursor->DetachItem();
			m_pSystemPanel->SetVisible(0);
			m_pCPanel->SetVisible(0);
			m_pSkillPanel->SetVisible(0);
			m_pSkillMPanel->SetVisible(0);
			pBtnInv->SetSelected(0);
			pBtnChar->SetSelected(0);
			SetVisibleTrade(0);

			auto pRunAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_RUN));

			if (bCargo == 1)
			{
				m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 650.0f, RenderDevice::m_fHeightRatio * 35.0f);
				m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 650.0f, RenderDevice::m_fHeightRatio * 35.0f);
				m_pCargoPanel->SetVisible(1);
				m_pCargoPanel1->SetVisible(1);
				m_pInvenPanel->SetVisible(0);
				m_pShopPanel->SetVisible(0);

				if (pRunAutoTrade)
					pRunAutoTrade->SetVisible(1);

				if (pMyCargoCoin)
					pMyCargoCoin->SetVisible(1);

				if (pMyCargoCoinB)
					pMyCargoCoinB->SetVisible(1);

				for (int i = 0; i < 10; ++i)
				{
					if (m_pGridAutoTrade[i])
						m_pGridAutoTrade[i]->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
				}
			}
			else
			{
				m_pCargoPanel->SetVisible(0);
				m_pCargoPanel1->SetVisible(0);

				if (pMyCargoCoin)
					pMyCargoCoin->SetVisible(0);

				if (pMyCargoCoinB)
					pMyCargoCoinB->SetVisible(0);

				m_pInvenPanel->SetVisible(1);

				if (pRunAutoTrade)
					pRunAutoTrade->SetVisible(0);

				for (int j = 0; j < 11; ++j)
				{
					if (m_pGridAutoTrade[j])
						m_pGridAutoTrade[j]->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY2;
				}
				SetInventoryGridType(TMEGRIDTYPE::GRID_TRADEINV2);
				SetEquipGridState(0);
			}
			m_pCargoGridList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
			m_pCargoGridList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
			m_pCargoGridList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
		}
		else
		{
			sprintf(m_pMyHuman->m_TradeDesc, "");
			m_pMyHuman->m_pAutoTradeDesc->SetText(m_pMyHuman->m_TradeDesc, 0);

			if (m_pInvenPanel->IsVisible() == 1)
			{
				SetInventoryGridType(TMEGRIDTYPE::GRID_DEFAULT);
				SetEquipGridState(1);
			}

			for (int k = 0; k < 10; ++k)
			{
				m_stAutoTrade.Header.ID = m_pMyHuman->m_dwID;
				m_stAutoTrade.TargetID = 0;

				SGridControlItem* pItem = m_pGridAutoTrade[k]->PickupAtItem(0, 0);

				// Auto-trade cleanup must address the native 7.48 cargo cell, not
				// the 7.59 40-slot page that happened to occupy the same pointer.
				int cargoX = 0;
				int cargoY = 0;
				GetCargoCellForSlot(m_stAutoTrade.CarryPos[k], cargoX, cargoY);
				auto pCargoGrid = GetCargoGridForSlot(m_stAutoTrade.CarryPos[k]);
				SGridControlItem* pSrcItem = pCargoGrid ? pCargoGrid->GetAtItem(cargoX, cargoY) : nullptr;

				if (pSrcItem)
					pSrcItem->m_GCObj.dwColor = 0xFFFFFFFF;

				WYD748_ReleaseAutoTradeItem(pItem);
			}
			m_pInvenPanel->SetVisible(0);
			m_pCargoPanel->SetVisible(0);
			m_pCargoPanel1->SetVisible(0);
			g_pDevice->m_nWidthShift = 0;
			m_pCargoGridList[0]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
			m_pCargoGridList[1]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
			m_pCargoGridList[2]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;

			memset(&m_stAutoTrade, 0, sizeof(m_stAutoTrade));

			if (bSendQuit)
			{
				MSG_STANDARD stStandard{};

				stStandard.Type = 0x384;
				stStandard.ID = m_pMyHuman->m_dwID;
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stStandard)->Type, reinterpret_cast<char*>(&stStandard), sizeof(stStandard)});
			}
		}
	}
}

void TMFieldScene::SendReqBuy(unsigned int dwControlID)
{
	const int slot = field_interaction::AutoTradeSlotIndex(m_bCompatFieldScene != 0, dwControlID);
	if (slot < 0 || !m_pMyHuman || !m_pAutoTrade || !m_pAutoTrade->IsVisible() ||
		!m_stAutoTrade.TargetID)
		return;

	auto grid = m_pGridAutoTrade[slot];
	if (!grid || !grid->GetAtItem(0, 0) || m_stAutoTrade.Item[slot].sIndex <= 0 ||
		m_stAutoTrade.TradeMoney[slot] <= 0)
		return;

	MSG_ReqBuy stReqBuy{};
	stReqBuy.Header.ID = m_pMyHuman->m_dwID;
	stReqBuy.Header.Type = MSG_ReqBuy_Opcode;
	stReqBuy.TargetID = m_stAutoTrade.TargetID;
	stReqBuy.Pos = slot;
	stReqBuy.Price = m_stAutoTrade.TradeMoney[slot];
	stReqBuy.Tax = m_stAutoTrade.Tax;
	stReqBuy.item = m_stAutoTrade.Item[slot];

	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stReqBuy)->Type, reinterpret_cast<char*>(&stReqBuy), sizeof(stReqBuy)});
}

void TMFieldScene::VisibleInputTradeName()
{
	if (!m_pControlContainer)
		return;

	// Ghidra FUN_004656af uses caption/edit 630/627 and background 574 in
	// FieldScene2.bin. Keep newer IDs only for the non-7.48 resource path.
	const unsigned int textControlID = m_bCompatFieldScene ? TMT_INPUT_GOLD : T_INPUT_GOLD;
	const unsigned int editControlID = m_bCompatFieldScene ? TME_INPUT_GOLD : E_INPUT_GOLD;
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pText = (SText*)m_pControlContainer->FindControl(textControlID);
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(editControlID);
	if (pInputGoldPanel && pText && pEdit)
	{
		m_nCoinMsgType = 3;
		pText->SetText(g_pMessageStringTable[141], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		pInputGoldPanel->SetVisible(1);
		auto pInputBG2 = m_pInputBG2
			? m_pInputBG2
			: static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_INPUT_BG2));
		if (pInputBG2)
			pInputBG2->SetVisible(1);
		pEdit->m_nMaxStringLen = 20;
		if (m_pChatSelectPanel)
			m_pChatSelectPanel->SetVisible(0);
	}
}

int TMFieldScene::OnPacketItemSold(MSG_STANDARDPARM2* pStd)
{
	if (!pStd)
		return 1;

	auto pPanel = this->m_pAutoTrade;
	const int autoTradeSlotCount = m_bCompatFieldScene ? 12 : 10;
	// Parm2 originates at the server boundary.  Validate it against the active
	// packet/UI ABI before indexing the grid array so a stale or malformed sale
	// acknowledgement cannot turn into an out-of-bounds client write.
	if (pPanel && pPanel->IsVisible() == 1
		&& pStd->Parm1 == m_stAutoTrade.TargetID
		&& pStd->Parm2 >= 0 && pStd->Parm2 < autoTradeSlotCount)
	{
		WYD748_CancelAutoTradePurchase(m_pMessageBox, pStd->Parm2);
		field_interaction::ClearAutoTradeOffer(m_stAutoTrade.Item,
			m_stAutoTrade.CarryPos, m_stAutoTrade.TradeMoney, pStd->Parm2);
		auto pPrice = m_pControlContainer
			? static_cast<SText*>(m_pControlContainer->FindControl(800 + pStd->Parm2)) : nullptr;
		if (pPrice)
		{
			pPrice->m_cComma = 1;
			char emptyPrice[] = "";
			pPrice->SetText(emptyPrice, 0);
			pPrice->SetVisible(0);
		}
		auto pGrid = m_pGridAutoTrade[pStd->Parm2];
		if (!pGrid)
			return 1;
		pGrid->m_nTradeMoney = 0;
		auto pItem = pGrid->PickupAtItem(0, 0);
		if (!pItem)
			return 1;

		WYD748_ReleaseAutoTradeItem(pItem);
	}

	return 1;
}

int TMFieldScene::OnPacketAutoTrade(MSG_STANDARD* pStd)
{
	// The response materializes controls and twelve packet slots.  Reject a
	// scene-bootstrap delivery before touching either resource-owned pointer.
	if (!pStd || !m_pControlContainer)
		return 1;

	auto pAutoTrade = reinterpret_cast<MSG_AutoTrade*>(pStd);
	WYD748_CancelAutoTradePurchase(m_pMessageBox);

	auto pTitle = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATRADE_TITLE));
	auto pName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATRADE_ID));

	pAutoTrade->Desc[23] = 0;
	pAutoTrade->Desc[22] = 0;

	if (pTitle)
		pTitle->SetText(pAutoTrade->Desc, 0);

	auto pHuman = static_cast<TMHuman*>(g_pObjectManager->GetHumanByID(pAutoTrade->TargetID));

	memcpy(&m_stAutoTrade, pAutoTrade, sizeof(m_stAutoTrade));

	if (pHuman && pName)
		pName->SetText(pHuman->m_szName, 1);

	// MSG_AutoTrade in the 7.48 wire contract has twelve slots; retain ten only
	// for the imported 7.59 resource whose UI genuinely exposes that boundary.
	const int autoTradeSlotCount = m_bCompatFieldScene ? 12 : 10;
	for (int i = 0; i < autoTradeSlotCount; ++i)
	{
		// FieldScene2.bin reserves controls 800..811 for the native price row
		// beneath each item.  Empty offers are hidden to prevent stale values.
		auto pPrice = static_cast<SText*>(m_pControlContainer->FindControl(800 + i));
		const bool hasOffer = pAutoTrade->Item[i].sIndex > 0 && pAutoTrade->TradeMoney[i] > 0;
		if (pPrice)
		{
			pPrice->m_cComma = 1;

			char price[32]{};
			if (hasOffer)
				sprintf_s(price, "%d", pAutoTrade->TradeMoney[i]);

			pPrice->SetText(price, 0);
			pPrice->SetVisible(hasOffer ? 1 : 0);
		}

		SGridControl* pGrid = m_pGridAutoTrade[i];
		if (!pGrid)
			continue;

		pGrid->m_nTradeMoney = pAutoTrade->TradeMoney[i];

		SGridControlItem* pItem = pGrid->PickupAtItem(0, 0);
		WYD748_ReleaseAutoTradeItem(pItem);

		if (pAutoTrade->Item[i].sIndex > 0)
		{
			auto pstItem = new STRUCT_ITEM();

			if (pstItem)
			{
				memcpy(pstItem, &pAutoTrade->Item[i], sizeof(STRUCT_ITEM));

				auto ipNewItem = new SGridControlItem(pGrid, pstItem, 0.0f, 0.0f);

				if (ipNewItem)
				{
					// Rejection does not transfer ownership: the packet remains
					// authoritative, but the temporary visual must be released.
					if (!pGrid->AddItem(ipNewItem, 0, 0))
						SAFE_DELETE(ipNewItem);
				}
				else
					delete pstItem;
			}
		}
	}

	auto pButton = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_RUN));

	if (pButton)
		pButton->SetVisible(0);

	if (!m_pMyHuman)
		return 1;

	if (pAutoTrade->TargetID == m_pMyHuman->m_dwID)
	{
		// Desc is network data, never a printf format string.  An auto-trade title
		// containing '%' must be copied literally and remain bounded to 24 bytes.
		sprintf_s(m_pMyHuman->m_TradeDesc, sizeof(m_pMyHuman->m_TradeDesc), "%s", pAutoTrade->Desc);

		if (m_pMyHuman->m_pAutoTradeDesc)
			m_pMyHuman->m_pAutoTradeDesc->SetText(m_pMyHuman->m_TradeDesc, 0);
	}
	else
	{
		SetVisibleAutoTrade(1, 0);
	}

	return 1;
}
