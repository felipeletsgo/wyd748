#include "pch.h"
#include "TMFieldScene.h"
#include "ItemEffect.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMCamera.h"
#include "TMUtil.h"
#include "TMItem.h"
#include "ClientDiagnostics.h"
#include "WYD748Assets.h"
#include "../../application/FieldInteractionPolicy.h"
#include "TMObjectContainer.h"
#include "TMSkinMesh.h"

int TMFieldScene::OnMouseEventCompat(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	// Preserve the complete 7.48 input order: UI, hover, skill/NPC/attack, then
	// movement.  The compact bootstrap only replaces unsafe HUD dereferences;
	// it must not reduce world input to a movement-only shortcut.
	// The PGT request is the one exception to the UI-first order.  In the 7.48
	// resource the hidden PGT root is registered as modal; forwarding its
	// opening click to SControlContainer first makes the container consume the
	// WM_RBUTTONUP before this scene can inspect MK_CONTROL.
	const bool bOpenPGT = dwFlags == WM_RBUTTONUP && (wParam & MK_CONTROL);
	const bool bPGTAlreadyVisible = m_pPGTPanel && m_pPGTPanel->IsVisible();
	if (!bOpenPGT || bPGTAlreadyVisible)
	{
		if (TMScene::OnMouseEvent(dwFlags, wParam, nX, nY) == 1)
			return 1;
	}
	if (!m_pMyHuman || !m_pGround)
		return 0;
	if ((dwFlags == WM_LBUTTONDOWN || dwFlags == WM_RBUTTONDOWN) && OfferRespawnPrompt(true))
		return 1;
	if (dwFlags == 512)
	{
		MouseMove(nX, nY);
		return 0;
	}
	if (g_pCursor && g_pCursor->m_pAttachedItem)
	{
		// Native 7.48 processes a left click on the field as DropItem while the
		// inventory is visible.  Returning here unconditionally leaves the item
		// permanently attached to the cursor and makes ground drops impossible.
		if (dwFlags == WM_LBUTTONDOWN && m_pInvenPanel && m_pInvenPanel->IsVisible())
		{
			const SGridControlItem* pAttachedItem = g_pCursor->m_pAttachedItem;
			const short itemIndex = pAttachedItem->m_pItem ? pAttachedItem->m_pItem->sIndex : 0;
			// WYD.exe 7.48 protects the quest/event range 5000..5095 from field
			// drops; every other attached inventory item follows the packet path.
			if (itemIndex < 5000 || itemIndex >= 5096)
				DropItem(g_pTimerManager->GetServerTime());
		}
		return 1;
	}
	if (g_pObjectManager->m_stMobData.CurrentScore.CurHP <= 0 || m_pMyHuman->m_cCantMove)
		return 1;

	const int screenWidth = static_cast<int>(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift);
	const int screenHeight = static_cast<int>(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift);
	if (nX <= 0 || nY <= 0 || nX >= screenWidth || nY >= screenHeight)
		return 1;

	const D3DXVECTOR3 pick = GroundGetPickPos();
	const unsigned int serverTime = g_pTimerManager->GetServerTime();
	// 7.48 opens the player interaction menu on WM_RBUTTONUP while Ctrl is
	// held.  The full initializer handles this below its larger input state
	// machine, but the compat path returns through this compact adapter first.
	if (dwFlags == 517 && (wParam & 8))
	{
		if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
			return 1;

		if (m_pMouseOverHuman &&
			m_pMouseOverHuman->m_dwEdgeColor != 0x8800FF00)
			PGTVisible(serverTime);

		return 1;
	}
	if (dwFlags == 516)
		return SkillUse(nX, nY, pick, serverTime, (wParam & 4) ? 0 : 1, 0);
	if (dwFlags == 514)
	{
		m_bMoveing = 0;
		return MouseClick_NPC(nX, nY, pick, serverTime);
	}
	if (dwFlags != 513 || (wParam & 4))
		return 0;

	// Native clicks first resolve a combat or merchant target.  Only an empty
	// field click becomes a route, keeping NPC shops and attacks reachable.
	int handled = MobAttack(wParam, pick, serverTime);
	if (!handled)
		handled = CheckMerchant(m_pMouseOverHuman);
	if (handled)
		return 1;

	if (pick.y < -5000.0f)
		return 1;
	m_pMyHuman->SetSpeed(m_bMountDead);
	m_vecMyNext.x = static_cast<int>(pick.x);
	m_vecMyNext.y = static_cast<int>(pick.z);
	if (m_pMyHuman->m_fProgressRate >= 0.89999998f || m_pMyHuman->m_fProgressRate == 0.0f)
		m_pMyHuman->GetRoute(m_vecMyNext, 0, 0);

	m_pTargetHuman = nullptr;
	m_pMyHuman->m_pMoveTargetHuman = nullptr;
	m_pMyHuman->m_pMoveSkillTargetHuman = nullptr;
	m_pMouseOverHuman = nullptr;
	m_pTargetItem = nullptr;
	m_bMoveing = 1;
	return 1;
}

int TMFieldScene::OnKeyDownEvent(unsigned int iKeyCode)
{
	DWORD dwServerTime = g_pTimerManager->GetServerTime();

	if (dwServerTime < g_dwStartQuitGameTime + 6000)
		return 1;

	if (dwServerTime < m_dwLastLogout + 6000)
		return 1;

	if (dwServerTime < m_dwLastSelServer + 6000)
		return 1;

	if (dwServerTime < m_dwLastTown + 6000)
		return 1;

	if (dwServerTime < m_dwLastResurrect + 6000)
		return 1;

	if (dwServerTime < m_dwLastTeleport + 6000)
		return 1;

	if (dwServerTime < m_dwLastRelo + 6000)
		return 1;

	if (dwServerTime < m_dwLastWhisper + 6000)
		return 1;

	if (TMScene::OnKeyDownEvent(iKeyCode) == 1)
		return 1;

	if (iKeyCode == VK_INSERT)
	{
		MSG_MessageWhisper stWhisper{};

		stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf_s(stWhisper.MobName, "time");

		SendOneMessage((char*)&stWhisper, sizeof(stWhisper));
	}

	// F1-F4 are not inventory-page shortcuts in the 7.48 executable.  Leaving
	// them out of this dispatcher preserves the native shortcut ownership.

	if (iKeyCode == VK_F11)
	{
		if (dwServerTime < m_dwKeyTime + 500 || m_bAirMove == 1)
			return 1;

		if (m_pMyHuman && m_pMyHuman->m_fProgressRate > 0.0f && m_pMyHuman->m_fProgressRate < 0.89999998f)
			return 1;

		int page{};

		SGridControl* pGridInv{};
		SGridControlItem* pItem{};
		int nX{};
		int nY{};
		int bFind{};

		// Ghidra proves the 7.48 inventory scan is exactly one 9x7 Carry grid.
		const int carryPages = 1;
		const int carryRows = 7;
		const int carryColumns = 9;
		for (int i = 0; i < carryPages; ++i)
		{
			pGridInv = m_pGridInv;

			for (nY = 0; nY < carryRows; ++nY)
			{
				for (nX = 0; nX < carryColumns; ++nX)
				{
					pItem = pGridInv->GetItem(nX, nY);

					if (pItem && BASE_GetItemAbility(pItem->m_pItem, EF_VOLATILE) == 11)
					{
						bFind = 1;
						// There is no page contribution in the native 7.48 Carry address.
						page = 0;
						break;
					}
				}
				if (bFind == 1)
					break;
			}
			if (bFind == 1)
				break;
		}

		if (bFind == 1 && pItem)
		{
			if (BASE_GetItemAbility(pItem->m_pItem, 38) == 11)
			{
				// F11 teleport consumables use the same 9-column Carry address as all
				// other native 7.48 item actions.
				short SourPos = nX + 9 * nY;
				m_dwGetItemTime = g_pTimerManager->GetServerTime();
				m_dwLastTeleport = m_dwGetItemTime;
				m_cLastTeleport = 1;

				MSG_DelayStart stDelayStart{};

				stDelayStart.Header.ID = m_pMyHuman->m_dwID;
				stDelayStart.Header.Type = MSG_DelayStart_Opcode;
				stDelayStart.Parm = 1;
				SendOneMessage((char*)&stDelayStart, sizeof(stDelayStart));

				m_stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
				m_stUseItem.Header.Type = MSG_UseItem_Opcode;
				m_stUseItem.SourType = 1;
				m_stUseItem.SourPos = page + SourPos;
				m_stUseItem.ItemID = 0;
				m_stUseItem.GridX = static_cast<unsigned short>(m_pMyHuman->m_vecPosition.x);
				m_stUseItem.GridY = static_cast<unsigned short>(m_pMyHuman->m_vecPosition.y);

				int nAmount = BASE_GetItemAmount(pItem->m_pItem);

				if (nAmount > 1)
				{
					BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);

					sprintf_s(pItem->m_GCText.strString, "%2d", nAmount - 1);

					pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
				}
				else
				{
					auto pPickedItem = pGridInv->PickupItem(nX, nY);

					if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
						g_pCursor->m_pAttachedItem = nullptr;

					if (pPickedItem)
						delete pPickedItem;
				}

				if (nAmount <= 1)
					memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));

				if (g_pSoundManager)
				{
					auto pSoundData = g_pSoundManager->GetSoundData(54);

					if (pSoundData)
						pSoundData->Play(0, 0);
				}
			}

			UpdateScoreUI(0);

			m_dwKeyTime = dwServerTime;
		}

		return 1;
	}

	if (iKeyCode >= VK_NUMPAD0 && iKeyCode <= VK_NUMPAD9)
	{
		return OnKeyNumPad(iKeyCode);
	}

	if (iKeyCode == VK_PRIOR)
	{
		SListBox* pChatList{};

		if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 0)
			pChatList = m_pMsgList;
		else
			pChatList = m_pChatList;

		if (pChatList && pChatList->IsVisible() == 0 && pChatList->m_pScrollBar)
			pChatList->m_pScrollBar->Up();

		return 1;
	}

	if (iKeyCode == VK_NEXT)
	{
		SListBox* pChatList{};

		if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 0)
			pChatList = m_pMsgList;
		else
			pChatList = m_pChatList;

		if (pChatList && pChatList->IsVisible() == 0 && pChatList->m_pScrollBar)
			pChatList->m_pScrollBar->Down();

		return 1;
	}

	return 0;
}

int TMFieldScene::OnAccel(int nMsg)
{
	if (g_nKeyType != 1)
		return 1;

	switch (nMsg)
	{
	case 40075:
		return OnKeySkill(33, 0);
	case 40076:
		return OnKeySkill(64, 0);
	case 40077:
		return OnKeySkill(35, 0);
	case 40078:
		return OnKeySkill(36, 0);
	case 40079:
		return OnKeySkill(37, 0);
	case 40080:
		return OnKeySkill(94, 0);
	case 40081:
		return OnKeySkill(38, 0);
	case 40082:
		return OnKeySkill(42, 0);
	case 40083:
		return OnKeySkill(40, 0);
	case 40074:
		return OnKeySkill(41, 0);
	case 40048:
		return OnKeyDash(45, 0);
	case 40047:
		return OnKeyPlus(43, 0);
	case 40092:
		SetPK();
		return 1;
	case 40094:
		return OnKeyName(110, 0);
	case 40099:
		return OnKeyAutoTarget(116, 0);
	case 40090:
		return OnKeyHelp(104, 0);
	case 40097:
		return OnKeyRun(114, 0);
	case 40100:
		return OnKeyFeedMount(118, 0);
	case 40098:
		return OnKeyVisibleSkill(115, 0);
	case 40091:
		return OnKeyVisibleInven(105, 0);
	case 40085:
		return OnKeyVisibleInven(105, 0);
	case 40086:
		return OnKeyVisibleCharInfo(99, 0);
	case 40093:
		return OnKeyVisibleMinimap(109, 0);
	case 40095:
		return OnKeyVisibleParty(112, 0);
	}
	if (g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0)
	{
		switch (nMsg)
		{
		case 40103:
			return OnKeySkillPage(122, 0);
		case 40102:
			return OnKeyQuestLog(120, 0);
		case 40027:
			return OnKeyReverse(91, 0);
		case 40028:
			return OnKeyAutoRun(93, 0);
		case 40029:
			return OnKeyGuildOnOff(39, 0);
		case 40065:
			return OnKeyShortSkill(49, 0);
		case 40066:
			return OnKeyShortSkill(50, 0);
		case 40067:
			return OnKeyShortSkill(51, 0);
		case 40068:
			return OnKeyShortSkill(52, 0);
		case 40069:
			return OnKeyShortSkill(53, 0);
		case 40070:
			return OnKeyShortSkill(54, 0);
		case 40071:
			return OnKeyShortSkill(55, 0);
		case 40072:
			return OnKeyShortSkill(56, 0);
		case 40073:
			return OnKeyShortSkill(57, 0);
		case 40064:
			return OnKeyShortSkill(48, 0);
		case 40104:
			return OnKeyNumPad(96);
		case 40105:
			return OnKeyNumPad(97);
		case 40106:
			return OnKeyNumPad(98);
		case 40107:
			return OnKeyNumPad(99);
		case 40108:
			return OnKeyNumPad(100);
		case 40109:
			return OnKeyNumPad(101);
		case 40110:
			return OnKeyNumPad(102);
		case 40111:
			return OnKeyNumPad(103);
		case 40112:
			return OnKeyNumPad(104);
		case 40113:
			return OnKeyNumPad(105);
		}
	}

	return 0;
}

int TMFieldScene::MouseClick_NPC(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime)
{
	auto pOver = m_pMouseOverHuman;

	if (m_pAutoTrade->m_bVisible)
		return 1;

	m_bTeleportMsg = 0;

	if (!pOver || pOver->m_bMouseOver != 1)
	{
		if (dwServerTime - m_dwLastMouseDownTime > 1000 &&
			m_dwLastMouseDownTime && !m_pMyHuman->m_cLastMoveStop && m_pMyHuman->m_stScore.CurHP > 0)
		{
			MobStop(vec);
		}
		return 1;
	}

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 1 &&
		(int)m_pMyHuman->m_vecPosition.x >> 7 < 11 &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 < 5)
		return 1;

	if (dwServerTime - m_dwNPCClickTime < 1000)
		return 1;

	if (pOver->m_TradeDesc[0])
	{
		MSG_STANDARDPARM stQuest{};

		stQuest.Header.Type = 0x39A;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.Parm = pOver->m_dwID;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stQuest)->Type, reinterpret_cast<char*>(&stQuest), sizeof(stQuest)});
		m_dwNPCClickTime = dwServerTime;
		return 1;
	}

	if (pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 51 &&
		((int)m_pMyHuman->m_vecPosition.x >> 7 == 13 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 14) &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 == 28)
		return 1;

	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 1)
	{
		if (!m_pShopPanel->IsVisible())
		{
			if (pOver->m_sHeadIndex == 58 && pOver->m_sHelmIndex == 1110)
				m_nIsMP = 1;
			else if (pOver->m_sHeadIndex == 59 && pOver->m_sHelmIndex == 1257)
				m_nIsMP = 2;

			if (pOver->m_sHeadIndex != 59 || pOver->m_sHelmIndex != 1260)
				m_nIsMP = 0;
			else
				m_bEventCouponClick = 1;

			MSG_REQShopList stReqShopList{};

			stReqShopList.Header.Type = MSG_REQShopList_Opcode;
			stReqShopList.Header.ID = m_pMyHuman->m_dwID;
			stReqShopList.TargetID = pOver->m_dwID;

			m_pGridShop->m_dwMerchantID = pOver->m_dwID;

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stReqShopList)->Type, reinterpret_cast<char*>(&stReqShopList), sizeof(stReqShopList)});

			m_dwNPCClickTime = dwServerTime;
			m_sShopTarget = pOver->m_dwID;
		}
		return 1;
	}
	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 3)
	{
		MouseClick_SkillMasterNPC(dwServerTime, pOver);
		return 1;
	}
	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 3)
	{
		if (!m_pShopPanel->IsVisible())
		{
			if (pOver->m_sHeadIndex == 58 && pOver->m_sHelmIndex == 1110)
				m_nIsMP = 1;
			else if (pOver->m_sHeadIndex == 59 && pOver->m_sHelmIndex == 1257)
				m_nIsMP = 2;

			if (pOver->m_sHeadIndex != 59 || pOver->m_sHelmIndex != 1260)
				m_nIsMP = 0;
			else
				m_bEventCouponClick = 1;

			MSG_REQShopList stReqShopList{};

			stReqShopList.Header.Type = MSG_REQShopList_Opcode;
			stReqShopList.Header.ID = m_pMyHuman->m_dwID;
			stReqShopList.TargetID = pOver->m_dwID;

			m_pGridShop->m_dwMerchantID = pOver->m_dwID;

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stReqShopList)->Type, reinterpret_cast<char*>(&stReqShopList), sizeof(stReqShopList)});

			m_dwNPCClickTime = dwServerTime;
			m_sShopTarget = pOver->m_dwID;
		}
		return 1;
	}

	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 2 && pOver->m_sHeadIndex != 51)
	{
		if (!m_pCargoPanel->IsVisible())
		{
			SetVisibleCargo(1);
			TMFieldScene::m_dwCargoID = pOver->m_dwID;
			m_dwNPCClickTime = dwServerTime;
		}
		return 1;
	}

	if (pOver->m_dwID < 1000 ||
		pOver->m_vecTargetPos.x < 2148 ||
		pOver->m_vecTargetPos.x > 2156 ||
		pOver->m_vecTargetPos.y < 2067 ||
		pOver->m_vecTargetPos.y > 2076 ||
		pOver->m_sHeadIndex != 51 ||
		m_pGround->m_vecOffsetIndex.x != 16 ||
		m_pGround->m_vecOffsetIndex.y != 16)
	{

		if (pOver->m_dwID >= 1000 &&
			pOver->m_sHeadIndex == 67 &&
			m_pGround->m_vecOffsetIndex.x == 16 &&
			m_pGround->m_vecOffsetIndex.y == 16)
		{
			m_MissionClass.ResultItemListSet();
			SetVisibleMissionPanel(m_MissionClass.m_pMissionPanel->m_bVisible == 0);
			return 1;
		}

		if (MouseClick_MixNPC(pOver))
			return 1;

		if (pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 57)
		{
			MouseClick_PremiumNPC(pOver);
			return 1;
		}

		if (pOver->m_dwID >= 1000 &&
			pOver->m_sHeadIndex == 63 &&
			(pOver->m_stScore.Merchant & 0xF) == 7 &&
			m_pGround->m_vecOffsetIndex.x == 16 &&
			m_pGround->m_vecOffsetIndex.y == 16)
		{
			AirMove_ShowUI(1);
			return 1;
		}

		if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 4 || (pOver->m_stScore.Merchant & 0xF) >= 8 && (pOver->m_stScore.Merchant & 0xF) <= 15)
		{
			MouseClick_QuestNPC(dwServerTime, pOver);
			return 1;
		}

		if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) >= 6 && (pOver->m_stScore.Merchant & 0xF) <= 8)
		{
			MSG_STANDARDPARM stPacket{};

			stPacket.Header.Type = 0x28E;
			stPacket.Header.ID = m_pMyHuman->m_dwID;
			m_dwTID = pOver->m_dwID;
			stPacket.Parm = m_dwTID;

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stPacket)->Type, reinterpret_cast<char*>(&stPacket), sizeof(stPacket)});

			m_dwNPCClickTime = dwServerTime;
			return 1;
		}
		return 1;
	}

	if (!m_pMessageBox->IsVisible())
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[437], pOver->m_sHeadIndex, 0);
		m_pMessageBox->m_dwArg = pOver->m_dwID;
		m_pMessageBox->SetVisible(1);
	}

	return 1;
}

void TMFieldScene::MouseMove(int nX, int nY)
{
	if (m_pMouseOverHuman &&
		m_pMouseOverHuman->m_pSkinMesh &&
		nX > 0 &&
		nY > 0 &&
		nX < static_cast<int>(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) &&
		nY < static_cast<int>(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
	{
		if (m_pMouseOverHuman->m_pSkinMesh->m_materials.Emissive.r >= 0.99900001f &&
			m_pMouseOverHuman->m_pSkinMesh->m_materials.Emissive.g <= 0.001f &&
			m_pMouseOverHuman->m_pSkinMesh->m_materials.Emissive.b <= 0.001f)
		{
			g_pCursor->m_GCPanel.nTextureIndex = 1;
		}
		else if (g_pCursor->m_GCPanel.nTextureIndex == 1)
		{
			g_pCursor->m_GCPanel.nTextureIndex = 0;
		}
	}
	else if (g_pCursor->m_GCPanel.nTextureIndex == 1)
	{
		g_pCursor->m_GCPanel.nTextureIndex = 0;
	}
}

int TMFieldScene::MouseLButtonDown(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime)
{
	// This function is just that
	return 1;
}

void TMFieldScene::OnESC()
{
	if (m_bCompatFieldScene && m_pccmode && m_pccmode->IsVisible())
	{
		m_pccmode->SetVisible(0);
		if (m_pCC_Btn)
			m_pCC_Btn->SetSelected(0);
		return;
	}
	if (m_bCompatFieldScene)
	{
		if (m_pControlContainer && m_pControlContainer->m_pFocusControl)
		{
			// The 7.48 resource has no 7.59 IME controls, so release focus locally
			// before applying FUN_0044df53's one-visible-window ESC cascade.
			m_pControlContainer->m_pFocusControl->SetFocused(0);
			m_pControlContainer->m_pFocusControl = nullptr;
		}

		if (m_pAutoTrade && m_pAutoTrade->IsVisible())
			SetVisibleAutoTrade(0, 0);
		else if (m_pQuestPanel && m_pQuestPanel->IsVisible())
		{
			// FUN_0044df53 closes the native quest panel before the inventory and
			// releases button 315, preserving the one-window-per-ESC cascade.
			SetQuestPanelVisible(false);
		}
		else if (m_pItemMixPanel && m_pItemMixPanel->IsVisible())
			SetVisibleMixItem(0);
		else if (m_pItemMixPanel2 && m_pItemMixPanel2->IsVisible())
			SetVisibleMixItem2(0);
		else if (m_pItemMixPanel3 && m_pItemMixPanel3->IsVisible())
			SetVisibleMixItem3(0);
		else if (m_pItemMixPanel4 && m_pItemMixPanel4->IsVisible())
			SetVisibleMixItemTiini(0);
		else if (m_pItemMixPanel5 && m_pItemMixPanel5->IsVisible())
			SetVisibleMixItem5(0);
		else if (m_pItemMixPanel6 && m_pItemMixPanel6->IsVisible())
			SetVisibleMixItem6(0);
		else if (m_pInvenPanel && m_pInvenPanel->IsVisible())
			OnControlEvent(368, 0);
		else if (m_pCPanel && m_pCPanel->IsVisible())
			OnControlEvent(TMB_CHAR_CLOSE, 0);
		else if ((m_pSkillPanel && m_pSkillPanel->IsVisible()) ||
			(m_pSkillMPanel && m_pSkillMPanel->IsVisible()))
			OnControlEvent(TMB_SKILL_CLOSE, 0);
		else if (m_pTradePanel && m_pTradePanel->IsVisible())
			SetVisibleTrade(0);
		else if (m_pPartyPanel && m_pPartyPanel->IsVisible())
		{
			// Party panel 1857 is the native 7.48 window. Use the same transition
			// as click/keyboard so the inverse selected state of button 5742 cannot
			// become stale, without touching the absent 7.59 Party list.
			SetVisibleParty();
		}
		else if (m_pShopPanel && m_pShopPanel->IsVisible())
			SetVisibleShop(0);
		else if (m_pCargoPanel && m_pCargoPanel->IsVisible())
			SetVisibleCargo(0);
		else if (m_pGambleStore && m_pGambleStore->IsVisible())
			SetVisibleGamble(0, 0);
		else if (m_pInputGoldPanel && m_pInputGoldPanel->IsVisible())
		{
			m_pInputGoldPanel->SetVisible(0);
			if (m_pInputBG2)
				m_pInputBG2->SetVisible(0);
		}
		else if (m_pMsgPanel && m_pMsgPanel->IsVisible())
			m_pMsgPanel->SetVisible(0);
		else if (m_pHelpPanel && m_pHelpPanel->IsVisible())
		{
			m_pHelpPanel->SetVisible(0);
			if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_HELP)))
				button->SetSelected(0);
		}
		else if (m_pServerPanel && m_pServerPanel->IsVisible())
			m_pServerPanel->SetVisible(0);
		else if (m_pPotalPanel && m_pPotalPanel->IsVisible())
			m_pPotalPanel->SetVisible(0);
		else if (m_pMessageBox && m_pMessageBox->IsVisible())
			m_pMessageBox->SetVisible(0);
		else if (m_pSystemPanel)
		{
			// With no modal window open, native ESC toggles the system panel and
			// mirrors that state on the bottom-bar system button.
			const int visible = m_pSystemPanel->IsVisible() == 0;
			m_pSystemPanel->SetVisible(visible);
			if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SYSTEM)))
				button->SetSelected(visible);
		}
		return;
	}

	if (g_bActiveWB == 1)
	{
		g_pApp->SwitchWebBrowserState(0);
	}
	else if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
	{
		SetVisibleAutoTrade(0, 0);
	}
	else if (m_pPGTPanel && m_pPGTPanel->IsVisible() == 1)
	{
		m_pPGTPanel->SetVisible(0);
	}
	else if (m_pQuestPanel && m_pQuestPanel->IsVisible() == 1)
	{
		SetQuestPanelVisible(false);
	}
	else if (m_pFireWorkPanel && m_pFireWorkPanel->IsVisible() == 1)
	{
		m_pFireWorkPanel->SetVisible(0);
	}
	else if (m_pTotoPanel && m_pTotoPanel->IsVisible() == 1)
	{
		TotoClose();
	}
	else if (m_pInvenPanel && m_pInvenPanel->IsVisible() == 1)
	{
		OnControlEvent(65562u, 0);
	}
	else if (m_pSkillPanel && m_pSkillPanel->IsVisible() == 1)
	{
		OnControlEvent(65568u, 0);
	}
	else if (m_pDonateStore && m_pDonateStore->IsVisible() == 1)// Esc closes the donation store
	{

		m_pDonateStore->SetVisible(0);
	}
	else if (m_pDropListPanel && m_pDropListPanel->IsVisible() == 1)// Esc handles the drop list
	{
		m_pDropPanel[49]->SetVisible(0);//
	}
	else if (m_pCPanel && m_pCPanel->IsVisible() == 1)
	{
		OnControlEvent(65696u, 0);
		//OnControlEvent(65769u, 0);
	}
	else if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
	{
		SetVisibleTrade(0);
	}
	else if (m_pPartyPanel && m_pPartyPanel->IsVisible() == 1)
	{
		SetVisibleParty();
	}
	else if (m_pShopPanel && m_pShopPanel->IsVisible() == 1)
	{
		SetVisibleShop(0);
	}
	else if (m_pCargoPanel && m_pCargoPanel->IsVisible() == 1)
	{
		SetVisibleCargo(0);
	}
	else if (m_pGambleStore && m_pGambleStore->IsVisible() == 1)
	{
		SetVisibleGamble(0, 0);
	}
	else if (m_pInputGoldPanel && m_pInputGoldPanel->IsVisible() == 1)
	{
		m_pInputGoldPanel->SetVisible(0);
	}
	else if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 1)//esc
	{
		m_pMsgPanel->SetVisible(0);
		m_pControlContainer->SetFocusedControl(0);
	}
	else if (m_pHelpPanel && m_pHelpPanel->IsVisible() == 1)
	{
		m_pHelpPanel->SetVisible(0);
		m_pHelpBtn->SetSelected(0);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(51);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}
	}
	else if (m_pServerPanel && m_pServerPanel->IsVisible() == 1)
	{
		m_pServerPanel->SetVisible(0);
	}
	else if (m_pPotalPanel && m_pPotalPanel->IsVisible() == 1)
	{
		m_pPotalPanel->SetVisible(0);
	}
	else if (m_pMessageBox && m_pMessageBox->IsVisible() == 1)
	{
		m_pMessageBox->SetVisible(0);
	}
	else if (!m_pSystemPanel->IsVisible())
	{
		m_pSystemPanel->SetVisible(1);
	}
	else if (m_pSystemPanel->IsVisible() == 1)
	{
		m_pSystemPanel->SetVisible(0);
	}
}

int TMFieldScene::OnKeyDebug(char iCharCode, int lParam)
{
	// Just that
	return 0;
}

int TMFieldScene::OnKeySkill(char iCharCode, int lParam)
{
	// Compatibility scenes bind the legacy skill panel, but retain this guard
	// because keyboard messages may arrive while a scene is still being built or
	// torn down; a shortcut probe must never make another UI key crash the client.
	if (!m_pSkillPanel || !g_pCursor)
		return 0;
	if (!m_pSkillPanel->IsVisible())
		return 0;

	int nBase = 0;
	float fWRatio = RenderDevice::m_fWidthRatio;
	float fHRatio = RenderDevice::m_fHeightRatio;
	if (m_pGridSkillBelt3 && m_pGridSkillBelt3->IsVisible() == 1)
		nBase = 10;

	int CodeIndex = -1;
	switch (iCharCode)
	{
	case '!':
		CodeIndex = 0;
		break;
	case '@':
		CodeIndex = 1;
		break;
	case '#':
		CodeIndex = 2;
		break;
	case '$':
		CodeIndex = 3;
		break;
	case '%':
		CodeIndex = 4;
		break;
	case '^':
		CodeIndex = 5;
		break;
	case '&':
		CodeIndex = 6;
		break;
	case '*':
		CodeIndex = 7;
		break;
	case '(':
		CodeIndex = 8;
		break;
	case ')':
		CodeIndex = 9;
		break;
	}
	if (CodeIndex == -1)
		return 0;

	for (int i = 0; i < 24; ++i)
	{
		auto pGrid = m_pSkillSecGrid[i];
		if (!pGrid)
			continue;
		if (PointInRect((int)g_pCursor->m_nPosX, (int)g_pCursor->m_nPosY, pGrid->m_GCPanel.nPosX, pGrid->m_GCPanel.nPosY, pGrid->m_GCPanel.nWidth * fWRatio, pGrid->m_GCPanel.nHeight * fHRatio) == 1)
		{
			auto pGridItem = pGrid->GetItem(0, 0);
			SetShortSkill(CodeIndex + nBase, pGridItem);
			break;
		}
	}
	for (int i = 0; i < 12; ++i)
	{
		auto pGrid = m_pSkillSecGrid2[i];
		if (!pGrid)
			continue;
		if (PointInRect((int)g_pCursor->m_nPosX, (int)g_pCursor->m_nPosY, pGrid->m_GCPanel.nPosX, pGrid->m_GCPanel.nPosY, pGrid->m_GCPanel.nWidth * fWRatio, pGrid->m_GCPanel.nHeight * fHRatio) == 1)
		{
			auto pGridItem = pGrid->GetItem(0, 0);
			SetShortSkill(CodeIndex + nBase, pGridItem);
			break;
		}
	}
	if (m_pGridSkillBelt && PointInRect((int)g_pCursor->m_nPosX, (int)g_pCursor->m_nPosY, m_pGridSkillBelt->m_GCPanel.nPosX, m_pGridSkillBelt->m_GCPanel.nPosY,
		m_pGridSkillBelt->m_GCPanel.nWidth * fWRatio, m_pGridSkillBelt->m_GCPanel.nHeight * fHRatio) == 1)
	{
		auto pItem = m_pGridSkillBelt->GetAtItem((int)((8 * (int)(g_pCursor->m_nPosX - m_pGridSkillBelt->m_GCPanel.nPosX)) / (m_pGridSkillBelt->m_GCPanel.nWidth * fWRatio)),
			(int)((int)(g_pCursor->m_nPosY - m_pGridSkillBelt->m_GCPanel.nPosY)	/ (m_pGridSkillBelt->m_GCPanel.nHeight * fHRatio)));

		if (pItem)
			SetShortSkill(nBase, pItem);
	}

	return 1;
}

int TMFieldScene::OnKeyDash(char iCharCode, int lParam)
{
	if (iCharCode != '-' && iCharCode != '_')
		return 0;

	if (m_cAutoAttack == 1)
	{
		int nCount = m_nAutoSkillNum + 1;

		if (nCount > 10)
			nCount = 1;

		SetAutoSkillNum(nCount);
	}
	return 1;
}

int TMFieldScene::OnKeyPlus(char iCharCode, int lParam)
{
	if (iCharCode != '=' && iCharCode != '+')
		return 0;

	++m_nChatListSize;
	m_nChatListSize %= 4;
	if (!m_pChatList)
		return 0;

	auto pChatList = m_pChatList;
	if (m_bCompatFieldScene)
	{
		// WYD.exe 7.48 FUN_00452271 resizes only legacy list 5377. Its
		// scrollbar/backdrop are not the 656xx controls imported from 7.59,
		// so touching those newer pointers makes the native '+' shortcut unsafe.
		pChatList->SetSize(300.0f, static_cast<float>(140 * m_nChatListSize + 112));
		pChatList->m_nVisibleCount = 10 * m_nChatListSize + 8;
		if (auto pChatPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_CHAT_PANEL)))
			pChatList->SetPos(pChatList->m_nPosX, pChatPanel->m_nPosY - pChatList->m_nHeight - 4.0f);
		if (m_nChatListSize == 3)
			pChatList->SetVisible(0);
		else if (!pChatList->m_bVisible)
			pChatList->SetVisible(1);

		GetSoundAndPlay(51, 0, 0);
		return 1;
	}

	pChatList->SetSize(pChatList->m_nWidth, (float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	pChatList->m_pScrollBar->SetSize(pChatList->m_pScrollBar->m_nWidth, (float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	pChatList->m_pScrollBar->m_pBackground1->SetSize(pChatList->m_pScrollBar->m_nWidth = 14.0f, (float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);

	m_pChatBack->SetSize(BASE_ScreenResize(10.0) + pChatList->m_nWidth, pChatList->m_nHeight);

	pChatList->m_pScrollBar->SetMaxValue(1000);
	auto pChatPanel = (SPanel*)m_pControlContainer->FindControl(65672u);

	if (pChatPanel)
	{
		m_pChatList->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - pChatList->m_nHeight) - 4.0f);
		m_pChatBack->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - pChatList->m_nHeight) - 4.0f);
	}

	if (m_nChatListSize == 3)
		m_pChatBack->SetVisible(0);
	else
		m_pChatBack->SetVisible(1);

	pChatList->m_nVisibleCount = 10 * m_nChatListSize + 5;
	pChatList->m_pScrollBar->Down();

	if (m_nChatListSize == 3)
		pChatList->SetVisible(0);
	else if (!pChatList->m_bVisible)
		pChatList->SetVisible(1);

	GetSoundAndPlay(51, 0, 0);

	return 1;
}

int TMFieldScene::OnKeyPK(char iCharCode, int lParam)
{
	if (iCharCode != 'k' && iCharCode != 'K')
		return 0;

	SetPK();
	return 1;
}

int TMFieldScene::OnKeyName(char iCharCode, int lParam)
{
	if (g_bEvent == 1)
		return 0;

	if (iCharCode != 'n' && iCharCode != 'N')
		return 0;

	SetVisibleNameLabel();
	return 1;
}

int TMFieldScene::OnKeyAutoTarget(char iCharCode, int lParam)
{
	if (iCharCode != 'T' && iCharCode != 't')
		return 0;

	SetAutoTarget();
	return 1;
}

int TMFieldScene::OnKeyAuto(char iCharCode, int lParam)
{
	if (iCharCode != 'f' && iCharCode != 'F')
		return 0;

	if (!m_pGridInv || !m_pMyHuman || !g_pObjectManager || !g_pTimerManager || !g_pEventTranslator)
		return 1;

	auto* pEquippedItem = &g_pObjectManager->m_stMobData.Equip[12];
	if (!pEquippedItem->sIndex)
		return 1;

	SGridControlItem* pItem = nullptr;
	for (int nX = 8; nX >= 0 && !pItem; --nX)
	{
		for (int nY = 6; nY >= 0; --nY)
		{
			auto* pCandidate = m_pGridInv->GetItem(nX, nY);
			if (pCandidate && pCandidate->m_pItem && BASE_GetItemAbility(pCandidate->m_pItem, 38) == 17)
			{
				pItem = pCandidate;
				break;
			}
		}
	}

	if (!pItem || !pItem->m_pItem || !pItem->m_pGridControl)
		return 1;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return 1;

	const unsigned short equippedKey = static_cast<unsigned short>(
		((BASE_GetItemAbility(pEquippedItem, 56) & 0xFF) << 8)
		| (BASE_GetItemAbility(pEquippedItem, 57) & 0xFF));
	const unsigned short consumableKey = static_cast<unsigned short>(
		((BASE_GetItemAbility(pItem->m_pItem, 56) & 0xFF) << 8)
		| (BASE_GetItemAbility(pItem->m_pItem, 57) & 0xFF));
	if (equippedKey != consumableKey)
		return 1;

	m_pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
		pItem->m_pGridControl->m_eGridType);

	int SourPos = m_pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
		SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
	if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
		return 1;

	auto vec = m_pMyHuman->m_vecPosition;

	// FUN_0044FC4B sends only the server-authoritative intent; it never consumes
	// or removes this matched item optimistically in the client.
	MSG_UseItem stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem_Opcode;
	stUseItem.SourType = 1;
	stUseItem.SourPos = SourPos;
	stUseItem.GridX = (int)vec.x;
	stUseItem.GridY = (int)vec.y;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});

	m_dwUseItemTime = dwServerTime;
	g_pEventTranslator->m_bRBtn = 1;
	GetSoundAndPlay(54, 0, 0);
	return 1;
}

int TMFieldScene::OnKeyHelp(char iCharCode, int lParam)
{
	if (iCharCode != 'h' && iCharCode != 'H')
		return 0;

	if (m_pHelpPanel)
	{
		int bVisible = m_pHelpPanel->IsVisible();
		if (bVisible == 0)
			SelectHelpTab(0);
		m_pHelpPanel->SetVisible(bVisible == 0);
		// Native resources pair panel 864 with button 314, but keep the guard so
		// a malformed RC cannot turn an optional Help window into an input crash.
		if (m_pHelpBtn)
			m_pHelpBtn->SetSelected(bVisible == 0);
		GetSoundAndPlay(51, 0, 0);
	}

	return 1;
}

int TMFieldScene::OnKeyRun(char iCharCode, int lParam)
{
	if (iCharCode != 'r' && iCharCode != 'R')
		return 0;

	SetRunMode();
	return 1;
}

int TMFieldScene::OnKeyFeedMount(char iCharCode, int lParam)
{
	if (iCharCode != 'v' && iCharCode != 'V')
		return 0;

	return FeedMount();
}

int TMFieldScene::OnKeyHPotion(char iCharCode, int lParam)
{
	if (iCharCode != 'q' && iCharCode != 'Q')
		return 0;

	return UseHPotion();
}

int TMFieldScene::OnKeyMPotion(char iCharCode, int lParam)
{
	if (iCharCode != 'w' && iCharCode != 'W')
		return 0;

	return UseMPotion();
}

int TMFieldScene::OnKeyPPotion(char iCharCode, int lParam)
{
	if (iCharCode != 'e' && iCharCode != 'E')
		return 0;

	TMFieldScene::UsePPotion();
	return 1;
}

int TMFieldScene::OnKeySkillPage(char iCharCode, int lParam)
{
	if (iCharCode != 'z' && iCharCode != 'Z')
		return 0;

	if (m_bCompatFieldScene)
	{
		// WYD.exe 7.48 FUN_00452691 dispatches the resource-owned buttons
		// 587/588. The 6564x controls belong to the imported 7.59 layout.
		if (m_pGridSkillBelt2)
			OnControlEvent(m_pGridSkillBelt2->m_bVisible ? TMB_SHORTSKILL_TGL2 : TMB_SHORTSKILL_TGL1, 0);
		return 1;
	}

	if (m_pGridSkillBelt2->m_bVisible)
		OnControlEvent(65647, 0);
	else
		OnControlEvent(65646u, 0);

	return 1;
}

int TMFieldScene::OnKeyQuestLog(char iCharCode, int lParam)
{
	if (iCharCode != 'x' && iCharCode != 'X')
		return 0;

	if (m_bCompatFieldScene)
	{
		// FUN_004526ee routes X through classic quest button 315; 65793 is a
		// newer resource ID and can neither open nor close the 7.48 window.
		OnControlEvent(TMB_QUESTLOG, 0);
		return 1;
	}

	OnControlEvent(65793, 0);
	return 1;
}

int TMFieldScene::OnKeyReverse(char iCharCode, int lParam)
{
	// Just that
	return 0;
}

int TMFieldScene::OnKeyAutoRun(char iCharCode, int lParam)
{
	if (m_pMyHuman->m_cCantMove)
		return 0;

	if (iCharCode != ']' && iCharCode != '}')
		return 0;

	if (m_pGround->m_vecOffsetIndex.x == 13 && m_pGround->m_vecOffsetIndex.y == 31
		|| m_pGround->m_vecOffsetIndex.x == 14 && m_pGround->m_vecOffsetIndex.y == 30
		|| m_pGround->m_vecOffsetIndex.x == 15 && m_pGround->m_vecOffsetIndex.y == 31
		|| m_pGround->m_vecOffsetIndex.x == 9 && m_pGround->m_vecOffsetIndex.y == 28
		|| m_pGround->m_vecOffsetIndex.x == 8 && m_pGround->m_vecOffsetIndex.y == 27
		|| m_pGround->m_vecOffsetIndex.x == 10 && m_pGround->m_vecOffsetIndex.y == 27)
	{
		return 1;
	}

	m_bAutoRun = m_bAutoRun == 0;
	if (m_pAutoRunBtn)
		m_pAutoRunBtn->SetSelected(m_bAutoRun);

	return 1;
}

int TMFieldScene::OnKeyGuildOnOff(char iCharCode, int lParam)
{
	if (iCharCode != '\'' && iCharCode != '"')
		return 0;

	OnControlEvent(299, 0);
	return 1;
}

int TMFieldScene::OnKeyShortSkill(char iCharCode, int lParam)
{
	if ((iCharCode < '0' || iCharCode > '9') && iCharCode != '!' && iCharCode != '@' &&
		iCharCode != '#' && iCharCode != '$' && iCharCode != '%' && iCharCode != '^' &&
		iCharCode != '&' && iCharCode != '*' && iCharCode != '(' && iCharCode != ')')
	{
		return 0;
	}

	if (m_bNumPad == 1)
	{
		m_bNumPad = 0;
		return 1;
	}

	if (iCharCode >= '0' && iCharCode <= '9')
	{
		g_pObjectManager->m_cSelectShortSkill = iCharCode - '1';
		if (iCharCode == 48)
			g_pObjectManager->m_cSelectShortSkill = 9;
	}
	else
	{
		switch (iCharCode)
		{
		case '!':
			g_pObjectManager->m_cSelectShortSkill = 0;
			break;
		case '@':
			g_pObjectManager->m_cSelectShortSkill = 1;
			break;
		case '#':
			g_pObjectManager->m_cSelectShortSkill = 2;
			break;
		case '$':
			g_pObjectManager->m_cSelectShortSkill = 3;
			break;
		case '%':
			g_pObjectManager->m_cSelectShortSkill = 4;
			break;
		case '^':
			g_pObjectManager->m_cSelectShortSkill = 5;
			break;
		case '&':
			g_pObjectManager->m_cSelectShortSkill = 6;
			break;
		case '*':
			g_pObjectManager->m_cSelectShortSkill = 7;
			break;
		case '(':
			g_pObjectManager->m_cSelectShortSkill = 8;
			break;
		case ')':
			g_pObjectManager->m_cSelectShortSkill = 9;
			break;
		}
	}
	// FUN_004528c5 always receives both native belt grids from FUN_00435b13.
	// A malformed resource must consume the shortcut safely instead of crashing.
	if (!m_pGridSkillBelt2 || !m_pGridSkillBelt3)
		return 1;

	if (m_pGridSkillBelt3->IsVisible())
		g_pObjectManager->m_cSelectShortSkill += 10;

	if (g_pObjectManager->m_cSelectShortSkill < 10)
	{
		auto pBeltGrid2 = m_pGridSkillBelt2;
		for (int i = 0; i < 10; ++i)
		{
			auto ipCtrlItem2 = pBeltGrid2->GetItem(i, 0);
			if (ipCtrlItem2)
			{
				if (i == g_pObjectManager->m_cSelectShortSkill)
					ipCtrlItem2->m_GCObj.nTextureSetIndex = 200;
				else
					ipCtrlItem2->m_GCObj.nTextureSetIndex = 199;
			}
		}
	}
	else
	{
		auto pBeltGrid3 = m_pGridSkillBelt3;
		for (int j = 0; j < 10; ++j)
		{
			auto ipCtrlItem3 = pBeltGrid3->GetItem(j, 0);
			if (j + 10 == g_pObjectManager->m_cSelectShortSkill && ipCtrlItem3)
				ipCtrlItem3->m_GCObj.nTextureSetIndex = 200;
			else if (ipCtrlItem3)
				ipCtrlItem3->m_GCObj.nTextureSetIndex = 199;
		}
	}

	int cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
	if (cSkillIndex >= 105)
		cSkillIndex += 95;
	if (cSkillIndex >= 0 && cSkillIndex < 248)
	{
		SetMyHumanMagic();

		int nIdx = g_pObjectManager->m_stMobData.Equip[6].sIndex;
		auto pMobData = &g_pObjectManager->m_stMobData;
		int nWeather = g_nWeather;
		if (g_nWeather == 3)
			nWeather = 2;

		if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
			(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
		{
			nWeather = 2;
		}

		int nSkillDam = BASE_GetSkillDamage(cSkillIndex, pMobData, nWeather, GetWeaponDamage(),
			g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex);
		char szStr[128]{};
		if (nSkillDam < 0)
		{
			sprintf(szStr, "%d", -nSkillDam);
			// Native FUN_004528c5 writes the compact HUD damage member; 65735 is
			// a 7.59 Character control and is absent from FieldScene2.bin 7.48.
			if (m_pSkillDam)
			{
				m_pSkillDam->SetText(szStr, 0);
				m_pSkillDam->SetTextColor(0xFFAAFFAA);
			}
		}
		else
		{
			sprintf(szStr, "%d", nSkillDam);
			if (m_pSkillDam)
			{
				m_pSkillDam->SetText(szStr, 0);
				m_pSkillDam->SetTextColor(0xFFBBBBFF);
			}
		}
	}
	else
	{
		char szStr[128]{};
		sprintf(szStr, "%d", 0);
		if (m_pSkillDam)
			m_pSkillDam->SetText(szStr, 0);
	}

	return 1;
}

int TMFieldScene::OnKeyVisibleSkill(char iCharCode, int lParam)
{
	if (iCharCode != 's' && iCharCode != 'S')
		return 0;

	if (m_pAutoTrade && m_pAutoTrade->IsVisible())
		return 1;

	if (!m_pShopPanel || m_pShopPanel->IsVisible() != 1)
	{
		SetVisibleSkill();
		// Native 7.48 shortcut S drives bottom button 295.  The imported alias
		// 65792 belongs to the newer resource and does not exist in FieldScene2.
		const unsigned int buttonID = m_bCompatFieldScene ? TMB_SKILL : 65792u;
		if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(buttonID)))
			button->SetSelected(m_pSkillPanel && m_pSkillPanel->IsVisible());
	}

	return 1;
}

int TMFieldScene::OnKeyCamView(char iCharCode, int lParam)
{
	if (iCharCode != 9)
		return 0;

	SetCameraView();
	return 1;
}

int TMFieldScene::OnKeyVisibleInven(char iCharCode, int lParam)
{
	if (iCharCode != 'i' && iCharCode != 'I' && iCharCode != 'g' && iCharCode != 'G')
		return 0;

	if (m_pAutoTrade && m_pAutoTrade->IsVisible())
		return 1;

	SetVisibleInventory();

	auto pPanel = m_pInvenPanel;
	// Native 7.48 inventory shortcut I/G selects button 294, not the 7.59
	// compatibility alias 65791.
	const unsigned int buttonID = m_bCompatFieldScene ? TMB_EQUIP : 65791u;
	auto pBtnEquip = static_cast<SButton*>(m_pControlContainer->FindControl(buttonID));

	if (pBtnEquip && pPanel)
		pBtnEquip->SetSelected(pPanel->m_bVisible);

	return 1;
}

int TMFieldScene::OnKeyVisibleCharInfo(char iCharCode, int lParam)
{
	if (iCharCode != 'c' && iCharCode != 'C')
		return 0;
	if (m_pAutoTrade && m_pAutoTrade->IsVisible())
		return 1;
	if (m_pShopPanel && m_pShopPanel->IsVisible())
		return 1;
	if (m_pTradePanel && m_pTradePanel->IsVisible())
		return 1;

	SetVisibleCharInfo();

	auto pPanel = m_pCPanel;
	// Native 7.48 shortcut C selects button 293; 65790 is a newer resource ID.
	const unsigned int buttonID = m_bCompatFieldScene ? TMB_CHAR : 65790u;
	auto pBtnChar = static_cast<SButton*>(m_pControlContainer->FindControl(buttonID));
	if (pBtnChar && pPanel)
		pBtnChar->SetSelected(pPanel->m_bVisible);

	return 1;
}

int TMFieldScene::OnKeyVisibleMinimap(char iCharCode, int lParam)
{
	if (iCharCode != 'm' && iCharCode != 'M')
		return 0;

	SetVisibleMiniMap();
	return 1;
}

int TMFieldScene::OnKeyVisibleParty(char iCharCode, int lParam)
{
	if (iCharCode != 'p' && iCharCode != 'P')
		return 0;

	SetVisibleParty();
	return 1;
}

int TMFieldScene::OnKeyReturn(char iCharCode, int lParam)
{
	if (m_bCompatFieldScene)
	{
		// Native FUN_00453b65 only toggles edit 5123 (and panel 5739 when
		// g_UIVer == 2).  The 901xx chat selector is a later-client control set
		// and must never be reached by the 7.48 resource path.
		if (iCharCode != 13 || !m_pEditChat)
			return 0;

		if (m_pEditChat->IsFocused())
		{
			m_pEditChat->SetVisible(0);
			if (g_UIVer == 2 && m_pEditChatPanel)
				m_pEditChatPanel->SetVisible(0);
			m_pControlContainer->SetFocusedControl(0);
		}
		else
		{
			m_pEditChat->SetVisible(1);
			if (g_UIVer == 2 && m_pEditChatPanel)
				m_pEditChatPanel->SetVisible(1);
			m_pControlContainer->SetFocusedControl(m_pEditChat);
		}
		return 1;
	}

	if (!m_pEditChatPanel)
		return 0;
	if (!m_pEditChat)
		return 0;
	if (iCharCode != 13)
		return 0;

	auto pEdit = m_pEditChat;
	if (pEdit->IsFocused())
	{
		m_pEditChatPanel->SetVisible(0);
		m_pChatPanel->SetVisible(1);
		m_pControlContainer->SetFocusedControl(0);
		return 1;
	}

	m_pEditChatPanel->SetVisible(1);
	m_pChatPanel->SetVisible(0);
	m_pControlContainer->SetFocusedControl(pEdit);
	auto Button = (SButton*)m_pControlContainer->FindControl(90130u);
	auto Button1 = (SButton*)m_pControlContainer->FindControl(90114u);
	auto Button2 = (SButton*)m_pControlContainer->FindControl(90129u);
	auto Button3 = (SButton*)m_pControlContainer->FindControl(90131u);
	auto Button4 = (SButton*)m_pControlContainer->FindControl(90132u);
	auto Button5 = (SButton*)m_pControlContainer->FindControl(90133u);
	auto Button6 = (SButton*)m_pControlContainer->FindControl(90134u);
	auto Button7 = (SButton*)m_pControlContainer->FindControl(90135u);
	auto Button8 = (SButton*)m_pControlContainer->FindControl(90136u);

	if (!strcmp(Button1->m_GCPanel.strString, Button2->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button3->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"=");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button4->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"-");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button5->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"--");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button6->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"@@");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button7->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"@");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button8->m_GCPanel.strString))
	{
		char temp[32]{};
		sprintf(temp, "/%s ", g_pMessageStringTable[389]);

		pEdit->SetText(temp);
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button->m_GCPanel.strString))
	{
		char temp[32]{};
		sprintf(temp, "/%s ", m_cWhisperName);
		pEdit->SetText(temp);
	}

	return 1;
}

int TMFieldScene::OnKeyNumPad(unsigned int iKeyCode)
{
	if (iKeyCode < VK_NUMPAD0 || iKeyCode > VK_NUMPAD9)
		return 0;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (m_pControlContainer->m_pFocusControl &&
		m_pControlContainer->m_pFocusControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT && !g_nKeyType)
	{
		return 1;
	}

	if (m_pMyHuman->m_cMount)
		return 1;

	if (m_pMyHuman->m_cDie == 1)
		return 1;

	if (dwServerTime < m_dwKeyTime + 500)
		return 1;

	if (m_pMyHuman->m_SendeMotion != ECHAR_MOTION::ECMOTION_NONE)
		return 1;

	if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEAT ||
		m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNISH ||
		m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND03 ||
		m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNEND)
	{
		return 1;
	}

	MSG_Motion stMotion{};

	stMotion.Header.ID = g_pObjectManager->m_dwCharID;
	stMotion.Header.Type = MSG_Motion_Opcode;
	stMotion.Motion = iKeyCode - 81;

	if (iKeyCode == VK_NUMPAD9)
	{
		if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEATING)
		{
			stMotion.Motion = 25;
		}
		else
		{
			if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNISHING)
				return 1;

			stMotion.Motion = 13;
		}
	}
	else if (iKeyCode == VK_NUMPAD0)
	{
		if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNISHING)
		{
			stMotion.Motion = 27;
		}
		else
		{
			if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEATING)
				return 1;

			stMotion.Motion = 15;
		}
	}

	m_pMyHuman->m_SendeMotion = static_cast<ECHAR_MOTION>(stMotion.Motion);

	stMotion.Direction = 0.0f;

	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMotion)->Type, reinterpret_cast<char*>(&stMotion), sizeof(stMotion)});

	m_dwKeyTime = dwServerTime;
	m_bNumPad = 1;
	return 1;
}

int TMFieldScene::OnKeyTotoTab(char iCharCode, int lParam)
{
	if (iCharCode != '\t' || !m_pTotoPanel || !m_pTotoPanel->IsVisible() ||
		!m_pControlContainer || !m_pTotoNumber_Edit || !m_pTotoScoreA_Edit ||
		!m_pTotoScoreB_Edit)
	{
		return 0;
	}

	if (m_pControlContainer->m_pFocusControl == m_pTotoNumber_Edit)
		m_pControlContainer->SetFocusedControl(m_pTotoScoreA_Edit);
	else if (m_pControlContainer->m_pFocusControl == m_pTotoScoreA_Edit)
		m_pControlContainer->SetFocusedControl(m_pTotoScoreB_Edit);
	else if (m_pControlContainer->m_pFocusControl == m_pTotoScoreB_Edit)
		m_pControlContainer->SetFocusedControl(m_pTotoNumber_Edit);
	else
		return 0;

	return 1;
}

int TMFieldScene::OnKeyTotoEnter(char iCharCode, int lParam)
{
	if (iCharCode != '\r' || !m_pTotoPanel || !m_pTotoPanel->IsVisible() ||
		!m_pControlContainer || !m_pTotoNumber_Edit || !m_pTotoScoreA_Edit ||
		!m_pTotoScoreB_Edit)
	{
		return 0;
	}

	if (m_pControlContainer->m_pFocusControl == m_pTotoNumber_Edit)
		TotoSelect();
	else if (m_pControlContainer->m_pFocusControl == m_pTotoScoreA_Edit)
		m_pControlContainer->SetFocusedControl(m_pTotoScoreB_Edit);
	else if (m_pControlContainer->m_pFocusControl == m_pTotoScoreB_Edit)
		m_pControlContainer->SetFocusedControl(m_pTotoNumber_Edit);
	else
		return 0;

	return 1;
}
