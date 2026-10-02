#include "pch.h"
#include "TMFieldScene.h"
#include "DirShow.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "TMEffectBillBoard.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectMesh.h"
#include "TMEffectParticle.h"
#include "TMFont3.h"
#include "TMItem.h"
#include "WYD748Assets.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"
#include "TMSkinMesh.h"

namespace
{
	bool WYD748_ParseDecimal(const char* text, int maximum, int* value)
	{
		if (!text || !text[0] || !value || maximum < 0)
			return false;

		int parsed = 0;
		for (const unsigned char* cursor = reinterpret_cast<const unsigned char*>(text); *cursor; ++cursor)
		{
			if (*cursor < '0' || *cursor > '9')
				return false;

			const int digit = *cursor - '0';
			if (parsed > (maximum - digit) / 10)
				return false;
			parsed = parsed * 10 + digit;
		}

		*value = parsed;
		return true;
	}
}

void TMFieldScene::PGTVisible(unsigned int dwServerTime)
{
	if (!g_pTimerManager || !m_pMyHuman || !m_pPGTPanel || !m_pPGTText || !m_pBtnPGTParty || !m_pBtnPGTGuild ||
		!m_pBtnPGTTrade || !m_pBtnPGTChallenge || !m_pBtnPGT1_V_1 ||
		!m_pBtnPGT5_V_5 || !m_pBtnPGT10_V_10 || !m_pBtnPGTAll_V_All ||
		!m_pBtnPGTGuildDrop || !m_pBtnPGTGuildWar || !m_pBtnPGTGuildAlly ||
		!m_pBtnPGTGuildInvite || !m_pBtnPGTGICommon || !m_pBtnPGTGIChief1 ||
		!m_pBtnPGTGIChief2 || !m_pBtnPGTGIChief3)
		return;

	auto pOver = m_pMouseOverHuman;
	if (pOver && (pOver->m_dwID <= 0 || pOver->m_dwID > 1000))
		return;

	if (!pOver || pOver->m_bMouseOver != 1 || dwServerTime < m_dwPGTTime + 500)
		return;

	m_dwOpID = pOver->m_dwID;
	auto pPGTText = m_pPGTText;

	char szStr[128]{};
	sprintf(szStr, g_pMessageStringTable[60], pOver->m_szName);
	pPGTText->SetText((char*)"", 0);

	RECT rt;
	rt.left = 2564;
	rt.top = 1689;
	rt.right = 2579;
	rt.bottom = 1711;

	POINT pt;
	pt.x = (int)m_pMyHuman->m_vecPosition.x;
	pt.y = (int)m_pMyHuman->m_vecPosition.y;
	PtInRect(&rt, pt);

	m_pBtnPGTGuild->SetVisible(1);
	m_pBtnPGTParty->SetVisible(1);
	m_pBtnPGTTrade->SetVisible(1);
	m_pBtnPGTGuildDrop->SetVisible(0);
	m_pBtnPGTGuildWar->SetVisible(0);
	m_pBtnPGTGuildAlly->SetVisible(0);
	m_pBtnPGTGuildInvite->SetVisible(0);
	m_pBtnPGT1_V_1->SetVisible(0);
	m_pBtnPGT5_V_5->SetVisible(0);
	m_pBtnPGT10_V_10->SetVisible(0);
	m_pBtnPGTAll_V_All->SetVisible(0);
	m_pBtnPGTChallenge->SetVisible(1);
	m_pBtnPGTGICommon->SetVisible(0);
	m_pBtnPGTGIChief1->SetVisible(0);
	m_pBtnPGTGIChief2->SetVisible(0);
	m_pBtnPGTGIChief3->SetVisible(0);
	m_pPGTOver = pOver;
	m_pPGTPanel->SetVisible(1);
	m_dwPGTTime = g_pTimerManager->GetServerTime();
}

void TMFieldScene::SetVisibleGamble(int bShow, char cType)
{
	SGridControl::m_sLastMouseOverIndex = -1;
	if (!m_pGambleStore)
		return;

	if (bShow)
	{
		if (!m_pReelPanel || !m_pReelPanel2)
			return;
		if (cType != 1 && cType != 2)
			return;
		if (m_pInputGoldPanel && m_pInputGoldPanel->IsVisible() == 1)
			SetInVisibleInputCoin();
		if (m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
			SetVisibleSkill();
		if (m_pCPanel && m_pCPanel->m_bVisible == 1)
			SetVisibleCharInfo();
		if (m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
			SetVisibleCargo(0);
		if (m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
			SetVisibleCargo(0);
		if (m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
			SetVisibleAutoTrade(0, 0);
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();
		if (m_pShopPanel && m_pShopPanel->m_bVisible == 1)
			SetVisibleShop(0);
		m_cGambleType = static_cast<unsigned char>(cType);
		m_cPendingGambleType = 0;
		m_dwGambleRequestTime = 0;
		PositionCompatGamblePanel();
		m_pGambleStore->SetVisible(1);
		m_pReelPanel->SetVisible(cType == 1);
		m_pReelPanel2->SetVisible(cType == 2);
		if (m_pGridHelm)
			m_pGridHelm->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		SetEquipGridState(0);
	}
	else
	{
		m_pGambleStore->SetVisible(0);
		if (m_pReelPanel)
		{
			m_pReelPanel->SetVisible(0);
			m_pReelPanel->m_bRoling = false;
			m_pReelPanel->m_dwStopTime = 0;
		}
		if (m_pReelPanel2)
		{
			m_pReelPanel2->SetVisible(0);
			m_pReelPanel2->m_bRoling = false;
			m_pReelPanel2->m_dwStopTime = 0;
		}
		m_cGambleType = 0;
		m_cPendingGambleType = 0;
		m_dwGambleRequestTime = 0;
		if (g_pDevice)
			g_pDevice->m_nWidthShift = 0;
		SetEquipGridState(1);
	}
}

void TMFieldScene::UpdateGambleRequestTimeout()
{
	if (m_dwGambleRequestTime == 0 ||
		(m_cPendingGambleType != 1 && m_cPendingGambleType != 2))
		return;

	const unsigned int now = g_pTimerManager->GetServerTime();
	if (now - m_dwGambleRequestTime < 10000)
		return;

	// Rejections are reported through MessagePanel without a synthetic 0x1BF.
	// Release only the requested reel so the UI can retry without inventing a
	// result or changing the native Gamble wire contract.
	SReelPanel* reel = m_cPendingGambleType == 1 ? m_pReelPanel : m_pReelPanel2;
	if (reel)
	{
		reel->m_bRoling = false;
		reel->m_dwStopTime = 0;
	}
	m_cPendingGambleType = 0;
	m_dwGambleRequestTime = 0;
}

void TMFieldScene::InitializeFireWorkControls()
{
	if (!m_pControlContainer || m_pFireWorkPanel)
		return;
	m_pFireWorkPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(8705));
	if (!m_pFireWorkPanel)
		return;

	// FUN_00435b13 creates these 100 children; they are not stored in the RC.
	for (int i = 0; i < 100; ++i)
	{
		m_pFireWorkButton[i] = new SButton(375, 25.0f * (i % 10) + 3.0f,
			25.0f * (i / 10) + 3.0f, 24.0f, 24.0f, 0x77777777u, 1, (char*)"");
		m_pFireWorkButton[i]->SetControlID(8706 + i);
		m_pFireWorkButton[i]->SetEventListener(m_pControlContainer);
		m_pFireWorkPanel->AddChild(m_pFireWorkButton[i]);
	}
	m_pFireWorkPanel->m_bModal = 1;
	m_pControlContainer->m_pModalControl[5] = m_pFireWorkPanel;
	m_pFireWorkPanel->SetVisible(0);
	m_pFireWorkPanel->SetPos((g_pDevice->m_dwScreenWidth - m_pFireWorkPanel->m_nWidth) / 2.0f,
		(g_pDevice->m_dwScreenHeight - m_pFireWorkPanel->m_nHeight) / 2.0f);
}

void TMFieldScene::UpdateFireWorkButton(int nIndex)
{
	if (nIndex >= 0 && nIndex <= 99 && m_pFireWorkPanel)
	{
		if (m_pFireWorkButton[nIndex])
		{
			m_pFireWorkButton[nIndex]->m_bSelected = m_pFireWorkButton[nIndex]->m_bSelected == 0;
			m_pFireWorkButton[nIndex]->Update();
		}
	}
}

void TMFieldScene::ClearFireWork()
{
	if (m_pFireWorkPanel)
	{
		for (int i = 0; i < 100; ++i)
		{
			m_pFireWorkButton[i]->m_bSelected = 0;
			m_pFireWorkButton[i]->Update();
		}
	}
}

void TMFieldScene::UseFireWork()
{
	if (!m_pFireWorkPanel || !m_pMyHuman)
		return;

	if (m_nFireWorkCellX < 0 || m_nFireWorkCellY < 0)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return;

	if (!m_pGridInv)
		return;

	auto pItem = m_pGridInv->GetItem(m_nFireWorkCellX, m_nFireWorkCellY);
	if (!pItem || !pItem->m_pItem || !pItem->m_pGridControl || pItem->m_pItem->sIndex != 3442)
		return;

	int nItemSIndex = pItem->m_pItem->sIndex;
	auto pMyHuman = m_pMyHuman;

	int SourPos = m_pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
		SourPos = pItem->m_nCellIndexX + 9 * pItem->m_nCellIndexY;
	// MSG_UseItem2 shares the native Carry address space with MSG_UseItem.
	if (SourPos < 0 || SourPos >= MAX_CARRY - 1)
		return;

	int nType = BASE_GetItemAbility(pItem->m_pItem, 38);

	MSG_UseItem2 stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem2_Opcode;
	stUseItem.SourType = 1;
	stUseItem.SourPos = SourPos;

	if (nType == 15)
	{
		stUseItem.DestType = 0;
		stUseItem.DestPos = 14;
	}

	stUseItem.ItemID = 0;
	stUseItem.GridX = (int)pMyHuman->m_vecPosition.x;
	stUseItem.GridY = (int)pMyHuman->m_vecPosition.y;
	for (int i = 0; i < 100; ++i)
	{
		if (m_pFireWorkButton[i]->m_bSelected)
			BASE_SetBit(stUseItem.Parm, i);
	}

	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stUseItem)->Type, reinterpret_cast<char*>(&stUseItem), sizeof(stUseItem)});
	m_nFireWorkCellX = -1;
	m_nFireWorkCellY = -1;
	m_dwUseItemTime = dwServerTime;

	int nAmount = BASE_GetItemAmount(pItem->m_pItem);
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
		auto pPickedItem = m_pGridInv->PickupItem(carryX, carryY);
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;

		SAFE_DELETE(pPickedItem);
	}

	int nSoundIndex = 41;
	if (nType >= 11 && nType <= 13)
		nSoundIndex = 54;

	if (nType != 19)
		GetSoundAndPlay(nSoundIndex, 0, 0);

	UpdateScoreUI(0);

	if (nAmount <= 1)
		memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
}

void TMFieldScene::DrawCustomFireWork(int nIndex)
{
	// This func is funny haha xD
}

void TMFieldScene::TotoSelect()
{
	if (!m_pTotoPanel || !m_pTotoNumber_Edit || !m_pTotoScoreA_Edit ||
		!m_pTotoScoreB_Edit || !m_pTotoTime_Txt || !m_pTotoTeamA_Txt ||
		!m_pTotoTeamB_Txt)
	{
		m_nTotoNum = 0;
		return;
	}

	int totoNumber = 0;
	const bool validNumber = WYD748_ParseDecimal(m_pTotoNumber_Edit->GetText(), 80, &totoNumber);
	m_pTotoNumber_Edit->SetText((char*)"");
	m_pTotoScoreA_Edit->SetText((char*)"");
	m_pTotoScoreB_Edit->SetText((char*)"");

	if (!validNumber || totoNumber < 1 || totoNumber > 80)
	{
		m_nTotoNum = 0;
		return;
	}

	STRUCT_TOTOLIST& toto = g_pTOTOList[totoNumber - 1];
	char matchFallback[32]{};
	sprintf_s(matchFallback, "Match %d", totoNumber);
	m_pTotoTime_Txt->SetText(toto.szTime[0] ? toto.szTime : matchFallback, 0);
	m_pTotoTeamA_Txt->SetText(toto.szTeamA[0] ? toto.szTeamA : (char*)"Team A", 0);
	m_pTotoTeamB_Txt->SetText(toto.szTeamB[0] ? toto.szTeamB : (char*)"Team B", 0);
	m_nTotoNum = totoNumber;
}

void TMFieldScene::TotoBuy()
{
	if (!m_pTotoPanel || !m_pTotoScoreA_Edit || !m_pTotoScoreB_Edit ||
		m_nTotoNum < 1 || m_nTotoNum > 80 ||
		m_stToto.Header.Size != sizeof(m_stToto) ||
		m_stToto.Header.Type != MSG_BuyToto_Opcode ||
		m_stToto.TargetID == 0 || m_stToto.TargetCarryPos < 0 ||
		m_stToto.MyCarryPos < 0 || m_stToto.MyCarryPos >= 63 ||
		!g_pSocketManager || !g_pSocketManager->Sock)
	{
		return;
	}

	int scoreA = 0;
	int scoreB = 0;
	if (!WYD748_ParseDecimal(m_pTotoScoreA_Edit->GetText(), 127, &scoreA) ||
		!WYD748_ParseDecimal(m_pTotoScoreB_Edit->GetText(), 127, &scoreB))
	{
		return;
	}

	m_stToto.Gindex = m_nTotoNum;
	m_stToto.A_Score = scoreA;
	m_stToto.B_Score = scoreB;
	SendOneMessage((char*)&m_stToto, sizeof(m_stToto));
	TotoClose();
}

void TMFieldScene::TotoClose()
{
	if (!m_pTotoPanel)
		return;

	if (m_pTotoPanel->IsVisible())
	{
		if (m_pControlContainer)
			m_pControlContainer->SetFocusedControl(nullptr);

		m_pTotoPanel->SetVisible(0);
		m_nTotoNum = 0;
	}
}

void TMFieldScene::SetQuestStatus(bool bStart)
{
	if (bStart == 1)
		m_dwQuestStartTime = timeGetTime();
	else
		m_dwQuestStartTime = 0;

	if (m_pQuestRemainTime)
		m_pQuestRemainTime->SetVisible(bStart);
}

void TMFieldScene::UpdateQuestTime()
{
	if (m_dwQuestStartTime && m_pQuestRemainTime)
	{
		int nLeftSecond = 900 - (timeGetTime() - m_dwQuestStartTime) / 1000;

		char strText[128]{};
		sprintf(strText, "[%02d:%02d]", nLeftSecond / 60, nLeftSecond % 60);

		m_pQuestRemainTime->SetText(strText, 0);
		if (!m_pQuestRemainTime->IsVisible())
			m_pQuestRemainTime->SetVisible(1);
	}
}

int TMFieldScene::OnPacketLongMessagePanel(MSG_LongMessagePanel* pMsg)
{
	pMsg->Line[0][127] = 0;
	pMsg->Line[1][127] = 0;
	pMsg->Line[2][127] = 0;
	pMsg->Line[3][127] = 0;

	if (pMsg->Parm1 == 10)
	{
		for (int i = 0; i < 4; ++i)
			strcpy(m_szEventTextTemp[i], pMsg->Line[i]);
		m_dwEventStartTime = 0;

		return 1;
	}
	if (m_pQuizPanel)
	{
		if (!pMsg->Parm1 && m_pQuizCaption)
			m_pQuizCaption->SetText(g_pMessageStringTable[260], 0);
		else if (pMsg->Parm1 == 1 && m_pQuizCaption)
			m_pQuizCaption->SetText(g_pMessageStringTable[259], 0);

		for (int j = 0; j < 4; ++j)
		{
			m_pChatListnotice->AddItem(new SListBoxItem(pMsg->Line[j], 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
			m_pQuizText[j]->SetText(pMsg->Line[j], 0);
		}

		m_dwQuizStart = g_pTimerManager->GetServerTime();
		m_pQuizPanel->SetVisible(1);

		GetSoundAndPlay(33, 0, 0);
	}
	return 1;
}

int TMFieldScene::OnPacketClearMenu(MSG_STANDARD* pStd)
{
	SetVisibleCargo(0);
	return 1;
}

int TMFieldScene::OnPacketCastleState(MSG_STANDARDPARM* pStd)
{
	g_bCastleWar2 = pStd->Parm;

	DS_SOUND_MANAGER::m_nCastleIndex = -1;
	if (!g_bCastleWar2)
	{
		DS_SOUND_MANAGER::m_nMusicIndex = -1;
		DS_SOUND_MANAGER::m_nCastleIndex = -1;
	}

	return 1;
}

int TMFieldScene::OnPacketStartTime(MSG_STANDARDPARM* pStd)
{
	if (!pStd || !g_pTimerManager)
		return 0;
	m_nLastTime = pStd->Parm;
	if (m_nLastTime <= 0)
	{
		m_nLastTime = 0;
		m_bRankTimeOn = 0;
		if (m_pRankTimeText)
			m_pRankTimeText->SetVisible(0);
		return 1;
	}

	m_dwStartRankTime = g_pTimerManager->GetServerTime();
	m_bRankTimeOn = 1;
	return 1;
}

int TMFieldScene::OnPacketRemainCount(MSG_STANDARDPARM* pStd)
{
	if (!pStd || !g_pTimerManager)
		return 0;
	if (pStd->Parm == 0)
	{
		m_bInstanceRemainOn = 0;
		if (m_pRemainText)
			m_pRemainText->SetVisible(0);
		return 1;
	}
	char szText[128]{};
	// Do not use strdef index 230 here: the English asset maps that slot to
	// the placeholder sequence "1 2 3 4 5".  The server packet and value are
	// unchanged; only the client-visible label is corrected.
	sprintf(szText, "Monsters %d", pStd->Parm);
	m_dwRemainTime = g_pTimerManager->GetServerTime();
	m_bInstanceRemainOn = 1;
	if (m_pRemainText)
	{
		if (g_pDevice)
			m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 200.0f,
				30.0f * RenderDevice::m_fHeightRatio);
		m_pRemainText->SetText(szText, 0);
		m_pRemainText->SetVisible(1);
	}
	return 1;
}

int TMFieldScene::OnPacketWarInfo(MSG_STANDARDPARM3* pStd)
{
	switch (pStd->Header.Size)
	{
	case sizeof(MSG_STANDARDPARM):
		g_pObjectManager->m_usWarGuild = pStd->Parm1;
		if (!pStd->Parm1)
			g_pObjectManager->m_usWarGuild = -1;
		break;
	case sizeof(MSG_STANDARDPARM2):
		g_pObjectManager->m_usWarGuild = pStd->Parm1;
		if (!pStd->Parm1)
			g_pObjectManager->m_usWarGuild = -1;
		m_cWarClan = pStd->Parm2;
		break;
	case sizeof(MSG_STANDARDPARM3):
		g_pObjectManager->m_usWarGuild = pStd->Parm1;
		if (!pStd->Parm1)
			g_pObjectManager->m_usWarGuild = -1;
		m_cWarClan = pStd->Parm2;
		g_pObjectManager->m_usAllyGuild = pStd->Parm3;
		break;
	}

	return 1;
}

int TMFieldScene::OnPacketRemainNPCCount(MSG_STANDARDPARM* pStd)
{
	if (!pStd || !g_pTimerManager)
		return 0;
	if (pStd->Parm == 0)
	{
		m_bInstanceRemainOn = 0;
		if (m_pRemainText)
			m_pRemainText->SetVisible(0);
		return 1;
	}
	char szText[128]{};
	sprintf(szText, "%d / %d", pStd->Parm & 0xFF, pStd->Parm >> 16);

	m_dwRemainTime = g_pTimerManager->GetServerTime();
	m_bInstanceRemainOn = 1;
	if (m_pRemainText)
	{
		if (g_pDevice)
			m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 200.0f,
				30.0f * RenderDevice::m_fHeightRatio);
		m_pRemainText->SetText(szText, 0);
		m_pRemainText->SetVisible(1);
	}
	return 1;
}

int TMFieldScene::OnPacketRESULTGAMBLE(MSG_ResultGamble* pStd)
{
	if (!pStd || pStd->Header.Size != sizeof(MSG_ResultGamble))
		return 0;

	for (unsigned int result : pStd->Result)
		if (result >= 19)
			return 0;
	for (unsigned int stop : pStd->StopPosition)
		if (stop >= 22)
			return 0;

	if ((m_cPendingGambleType != 1 && m_cPendingGambleType != 2) ||
		!m_pGambleStore || !m_pGambleStore->IsVisible())
		return 0;

	SReelPanel* reel = m_cPendingGambleType == 1 ? m_pReelPanel : m_pReelPanel2;
	if (!reel || !reel->IsVisible())
		return 0;

	if (m_pReelPanel)
		m_pReelPanel->m_dwJackpot = pStd->Jackpot;
	if (m_pReelPanel2)
		m_pReelPanel2->m_dwJackpot = pStd->Jackpot;

	for (char line = 0; line < 5; ++line)
		if (pStd->Result[line] > 0)
			reel->SetResult(line);
	if (pStd->Prize > 0)
		reel->m_nPresent = pStd->Prize;

	reel->SetRoll(false,
		pStd->StopPosition[0],
		pStd->StopPosition[1],
		pStd->StopPosition[2],
		3000);
	m_cPendingGambleType = 0;
	m_dwGambleRequestTime = 0;
	return 1;
}

int TMFieldScene::OnPacketREQArray(MSG_STANDARD* pStd)
{
	if (!pStd || !m_pMyHuman)
		return 0;

	MSG_REQArray response = *reinterpret_cast<MSG_REQArray*>(pStd);

	int category = response.Category;
	if (category < 0 || category >= MAX_BONE_ANIMATION_LIST)
		category = 0;

	const stBoneAni& animation = MeshManager::m_BoneAnimationList[category];
	int byteOffset = response.ByteOffset;
	if (byteOffset < 0 || static_cast<unsigned int>(byteOffset) >= animation.numBoneBytes)
		byteOffset = 0;

	INT32 value = 0;
	if (animation.pBone && animation.numBoneBytes != 0)
	{
		const auto bytes = reinterpret_cast<const std::int8_t*>(animation.pBone);
		value = static_cast<INT32>(bytes[byteOffset]);
	}

	response.Header.Type = MSG_CNFArray_Opcode;
	response.Header.ID = static_cast<WORD>(m_pMyHuman->m_dwID);
	response.Value = value;
	SendOneMessage(reinterpret_cast<char*>(&response), sizeof(response));
	return 1;
}

void TMFieldScene::InitializeQuizEventControls()
{
	if (!m_pControlContainer || !g_pDevice || m_pQuizBG) return;
	// Keep the 7.48 runtime control IDs (896..900), but render the extension with
	// the native NewUI message-box atlas used by SMessageBox (sets 164/165).
	// Coordinates below are logical 800x600 values; SControl scales them once.
	m_pQuizBG = new SPanel(164, 0.0f, 0.0f, 360.0f, 82.0f,
		0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
	m_pQuizBG->SetControlID(896);
	m_pQuizBG->m_bSelectEnable = 0;
	m_pControlContainer->AddItem(m_pQuizBG);

	const float topMargin = 16.0f * RenderDevice::m_fHeightRatio;
	m_pQuizBG->SetPos(
		(static_cast<float>(g_pDevice->m_dwScreenWidth) - m_pQuizBG->m_nWidth) * 0.5f,
		topMargin);

	m_pQuizQuestion = new SText(-1, "", 0xFFFFFFFF, 10.0f, 12.0f, 340.0f, 18.0f,
		0, 0x77777777, SText::TEXT_TYPE_SHADOW, SText::TEXT_ALIGN_CENTER);
	m_pQuizBG->AddChild(m_pQuizQuestion);

	SPanel* buttonSkins[4]{};
	for (int i = 0; i < 4; ++i)
	{
		const float buttonX = 10.0f + 86.0f * i;
		buttonSkins[i] = new SPanel(165, buttonX, 48.0f, 78.0f, 23.0f,
			0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		buttonSkins[i]->m_bSelectEnable = 0;

		m_pQuizButton[i] = new SButton(-2, buttonX, 49.0f, 78.0f, 21.0f,
			0, 1, (char*)"");
		m_pQuizButton[i]->SetControlID(897 + i);
		m_pQuizButton[i]->SetEventListener(m_pControlContainer);
		m_pQuizBG->AddChild(m_pQuizButton[i]);
	}
	// Children render in reverse insertion order. Add the skins after the text
	// buttons so they are drawn underneath them, matching SMessageBox NewUI.
	for (int i = 0; i < 4; ++i)
		m_pQuizBG->AddChild(buttonSkins[i]);

	m_pQuizBG->SetVisible(0);
}

int TMFieldScene::OnPacketQuizEvent(MSG_STANDARD* packet)
{
	quiz_event::Challenge challenge{};
	if (!packet || !g_pObjectManager || !m_pQuizBG ||
		packet->ID != g_pObjectManager->m_dwCharID ||
		!quiz_event::Parse(packet, packet->Size, challenge) ||
		!m_quizEvent.Apply(challenge, GetTickCount())) return 0;
	if (m_quizEvent.Active)
	{
		char question[128]{};
		sprintf_s(question, "%s  (10 segundos)", challenge.Question);
		m_pQuizQuestion->SetText(question, 0);
		for (int i = 0; i < 4; ++i)
		{
			char value[16]{};
			sprintf_s(value, "%d", challenge.Answers[i]);
			m_pQuizButton[i]->SetText(value);
		}
	}
	m_pQuizBG->SetVisible(m_quizEvent.Active ? 1 : 0);
	return 1;
}

int TMFieldScene::OnPacketRandomQuiz(MSG_RandomQuiz* pStd)
{
	// Unversioned legacy challenges must not replace an active secure round.
	if (m_bCompatFieldScene || m_quizEvent.Active || !pStd || pStd->Header.Size != sizeof(MSG_RandomQuiz)) return 0;
	pStd->Question[127] = 0;
	pStd->Answer[0][31] = 0;
	pStd->Answer[1][31] = 0;
	pStd->Answer[2][31] = 0;
	pStd->Answer[3][31] = 0;

	if (m_pQuizQuestion)
		m_pQuizQuestion->SetText(pStd->Question, 0);
	if (m_pQuizButton[0])
		m_pQuizButton[0]->SetText(pStd->Answer[0]);
	if (m_pQuizButton[1])
		m_pQuizButton[1]->SetText(pStd->Answer[1]);
	if (m_pQuizButton[2])
		m_pQuizButton[2]->SetText(pStd->Answer[2]);
	if (m_pQuizButton[3])
		m_pQuizButton[3]->SetText(pStd->Answer[3]);
	if (m_pQuizBG)
		m_pQuizBG->SetVisible(1);

	return 1;
}

int TMFieldScene::OnPacketSendExpMsg(MSG_Exp_MsgPanel* pStd)
{
	auto pEdit = m_pEditChat;
	auto pChatList = m_pChatListnotice;
	pChatList->AddItem(new SListBoxItem(pStd->Msg, pStd->Color32, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
	pEdit->SetText((char*)"");
	m_pControlContainer->SetFocusedControl(nullptr);
	if (g_pObjectManager->m_bTvControl != 1)
		pEdit->SetText((char*)"");
	return 1;
}

int TMFieldScene::OnPacketBattle(MSG_TowerWar* pStd)
{
	memcpy(&_HudControl.GuerraTorres.Packet, pStd, sizeof(MSG_TowerWar));

	return 1;
}

int TMFieldScene::OnPacketInforPlay(MSG_SendInfoPlay* pStd)
{
	Exp = pStd->ExpBonus;
	Drop = pStd->DropBonus;

	ValueEffectSrver[1] = pStd->AbsDamage;

	ValueEffectSrver[0] = pStd->PerfuDamage;

	Cash = pStd->Cash;

	return 1;
}

int TMFieldScene::OnPacketRunQuest12Start(MSG_STANDARDPARM* pStd)
{
	if (!pStd || !g_pDevice)
		return 0;
	if (pStd->Parm)
	{
		m_nQuest12MaxMobs = pStd->Parm;

		char szText[128]{};
		sprintf(szText, "0 / %d", m_nQuest12MaxMobs);
		if (m_pRemainText)
		{
			m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 130.0f,
				30.0f * RenderDevice::m_fHeightRatio);
			m_pRemainText->SetText(szText, 0);
			m_pRemainText->SetVisible(1);
		}
	}
	else if (m_pRemainText)
		m_pRemainText->SetVisible(0);

	return 1;
}

int TMFieldScene::OnPacketRunQuest12Count(MSG_STANDARDPARM2* pStd)
{
	if (!pStd || !g_pDevice)
		return 0;
	char szText[128]{};
	sprintf(szText, "%d / %d", pStd->Parm1, pStd->Parm2);

	if (m_pRemainText)
	{
		m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 130.0f,
			30.0f * RenderDevice::m_fHeightRatio);
		m_pRemainText->SetText(szText, 0);
	}
	return 1;
}

int TMFieldScene::MouseClick_QuestNPC(unsigned int dwServerTime, TMHuman* pOver)
{
	if ((pOver->m_stScore.Merchant & 0xF) == 15 && !m_pMyHuman->IsInTown())
	{
		if (pOver->m_cMantua > 0 && m_pMyHuman->m_cMantua > 0 && pOver->m_cMantua != m_pMyHuman->m_cMantua && m_pMyHuman->m_cMantua != 3)
			return 1;

		if (m_pMyHuman->m_pMantua &&
			((int)m_pMyHuman->m_pMantua->m_Look.Skin0 < 2 || (int)m_pMyHuman->m_pMantua->m_Look.Skin0 >= 8	&& (int)m_pMyHuman->m_pMantua->m_Look.Skin0 <= 14))
		{
			if (g_pObjectManager->m_stMobData.Equip[10].sIndex == 1742 && (g_pObjectManager->m_stMobData.Equip[11].sIndex < 1760 ||
				g_pObjectManager->m_stMobData.Equip[11].sIndex > 1763))
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[241], 4000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
			if (g_pObjectManager->m_stMobData.Equip[10].sIndex != 1742 || g_pObjectManager->m_stMobData.Equip[11].sIndex < 1760	||
				g_pObjectManager->m_stMobData.Equip[11].sIndex > 1763)
			{
				return 1;
			}
		}
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 13)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[131], 13, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}

		return 1;
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 14)
	{
		char cLifeStone = 0;
		char cSapha = 0;
		for (int i = 0; i < 63; ++i)
		{
			if (g_pObjectManager->m_stMobData.Carry[i].sIndex == 1740 && g_pObjectManager->m_stMobData.Carry[i + 1].sIndex == 1741)
				cLifeStone = 1;
			if (g_pObjectManager->m_stMobData.Carry[i].sIndex == 697)
				++cSapha;
			if (cLifeStone == 1 && cSapha >= 20 && m_pMyHuman->m_stScore.Level >= 299)
			{
				if (!m_pMessageBox->IsVisible())
				{
					m_pMessageBox->SetMessage(g_pMessageStringTable[233], 233, 0);
					m_pMessageBox->m_dwArg = pOver->m_dwID;
					m_pMessageBox->SetVisible(1);
				}
				return 1;
			}
		}
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 15 || (pOver->m_stScore.Merchant & 0xF) == 10)
	{
		if (!m_pMessageBox->IsVisible())
		{
			if (pOver->m_sHeadIndex == 51 && _locationCheck(pOver->m_vecPosition, 16, 16))
				m_pMessageBox->SetMessage(g_pMessageStringTable[404], pOver->m_stScore.Merchant & 0xF, 0);
			else
				m_pMessageBox->SetMessage(g_pMessageStringTable[152], pOver->m_stScore.Merchant & 0xF, 0);

			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}

		return 1;
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 4 && pOver->m_sHeadIndex == 271)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[348], 271, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}

	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 9 && pOver->m_sHeadIndex == 51)
	{
		m_pInputGoldPanel->SetVisible(1);
		auto pText = (SText*)m_pControlContainer->FindControl(65888);
		if (pText)
		{
			m_nCoinMsgType = 7;
			pText->SetText(g_pMessageStringTable[136], 0);
			auto pEdit = (SEditableText*)g_pCurrentScene->m_pControlContainer->FindControl(65889);
			m_pControlContainer->SetFocusedControl((SControl*)pEdit);
		}
	}
	else if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 58 && (pOver->m_stScore.Merchant & 0xF) == 11)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[152], pOver->m_sHeadIndex, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}
	else if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 58 && pOver->m_stScore.Merchant == 76)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[152], pOver->m_sHeadIndex, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}

	MSG_UseNPC stQuest{};
	stQuest.Header.Type = MSG_UseNPC_Opcode;
	stQuest.Header.ID = m_pMyHuman->m_dwID;
	stQuest.TargetID = pOver->m_dwID;
	stQuest.ClickOk = 0;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stQuest)->Type, reinterpret_cast<char*>(&stQuest), sizeof(stQuest)});
	m_dwNPCClickTime = dwServerTime;

	return 1;
}
