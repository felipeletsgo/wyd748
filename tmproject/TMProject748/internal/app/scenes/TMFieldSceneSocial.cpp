#include "pch.h"
#include "TMFieldScene.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "WYD748Assets.h"
#include <WinInet.h>
#include "TMHuman.h"
#include "TMObjectContainer.h"

DWORD WINAPI Guildmark_Download(void* pArg)
{
	// TODO: we have to find a better way to download the guildmark
	// currently we have a great treat of data race...
	auto pMark = (stGuildMarkInfo*)pArg;
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

	if (!g_pCurrentScene || !pMark || !pMark->strMarkFileName[0] || !pMark->pGuildMark)
	{
		g_pTextureManager->m_stGuildMark[pMark->nMarkIndex].nGuild = -1;
		return 0;
	}

	pFScene->m_dwLastGetGuildmarkTime = timeGetTime();

	char strMarkBuffer[632]{};
	char strURL[64]{};

	auto URLMark = "http://meusite.com/guilds/";
	strcpy(strURL, URLMark); // g_pMessageStringTable[377] stores the guild mark address
	strcat(strURL, pMark->strMarkFileName);

	if (!pFScene->m_hInternetSession)
	{
		pFScene->m_hInternetSession = InternetOpen("MS", 0, 0, 0, 0);
		if (!pFScene->m_hInternetSession)
			return 0;
	}

	auto m_hHttpFile = InternetOpenUrl(pFScene->m_hInternetSession, strURL, 0, 0, 0x4000000, 0);

	DWORD dwBytesRead = 0;
	if (m_hHttpFile)
	{
		char szData[1024]{};
		InternetReadFile(m_hHttpFile, szData, 632, &dwBytesRead);
		memcpy(strMarkBuffer, szData, dwBytesRead);
		InternetCloseHandle(m_hHttpFile);

		int bIsCorrectBMP = pFScene->Guildmark_IsCorrectBMP(strMarkBuffer);
		if (bIsCorrectBMP == 1 && g_pTextureManager->LoadGuildTexture(pMark->nMarkIndex, strMarkBuffer) == 1)
		{
			++pFScene->m_nGuildMarkCount;
			g_pTextureManager->m_stGuildMark[pMark->nMarkIndex].nGuild = pMark->nGuild + (pMark->nGuildChannel << 16);
			pMark->pGuildMark->m_GCPanel.nMarkIndex = pMark->nMarkIndex;
			if (pMark->sGuildIndex == 509)
				pMark->pGuildMark->m_GCPanel.nMarkLayout = 1;
			else if (pMark->sGuildIndex >= 526 && pMark->sGuildIndex <= 531)
				pMark->pGuildMark->m_GCPanel.nMarkLayout = 2;
			else
				pMark->pGuildMark->m_GCPanel.nMarkLayout = 3;
		}
	}

	if (m_hHttpFile != nullptr)
		InternetCloseHandle(m_hHttpFile);

	return 1;
}

void TMFieldScene::SetVisibleParty()
{
	if (!m_pPartyPanel)
		return;

	const int visible = m_pPartyPanel->m_bVisible == 0;
	if (visible)
		PositionCompatPartyPanel();
	m_pPartyPanel->SetVisible(visible);
	if (m_pPartyBtn)
		m_pPartyBtn->SetSelected(visible == 0);
}

void TMFieldScene::SetVisibleServerWar()
{
	if (!m_pControlContainer || !m_pInputGoldPanel)
		return;

	const unsigned int textControlID = m_bCompatFieldScene ? TMT_INPUT_GOLD : T_INPUT_GOLD;
	const unsigned int editControlID = m_bCompatFieldScene ? TME_INPUT_GOLD : E_INPUT_GOLD;
	auto pText = static_cast<SText*>(m_pControlContainer->FindControl(textControlID));
	auto pEdit = static_cast<SEditableText*>(m_pControlContainer->FindControl(editControlID));
	if (!pText || !pEdit)
		return;

	m_nCoinMsgType = kDeclareServerWarPromptMode;
	pText->SetText(g_pMessageStringTable[374], 0);
	pEdit->SetText((char*)"");
	m_pControlContainer->SetFocusedControl(pEdit);
	m_pInputGoldPanel->SetVisible(1);
}

void TMFieldScene::SetVisibleRefuseServerWar()
{
	if (!m_pControlContainer || !m_pInputGoldPanel)
		return;

	const unsigned int textControlID = m_bCompatFieldScene ? TMT_INPUT_GOLD : T_INPUT_GOLD;
	const unsigned int editControlID = m_bCompatFieldScene ? TME_INPUT_GOLD : E_INPUT_GOLD;
	auto pText = static_cast<SText*>(m_pControlContainer->FindControl(textControlID));
	auto pEdit = static_cast<SEditableText*>(m_pControlContainer->FindControl(editControlID));
	if (!pText || !pEdit)
		return;

	m_nCoinMsgType = kRefuseServerWarPromptMode;
	pText->SetText(g_pMessageStringTable[375], 0);
	pEdit->SetText((char*)"");
	m_pControlContainer->SetFocusedControl(pEdit);
	m_pInputGoldPanel->SetVisible(1);
}

void TMFieldScene::VisibleInputGuildName()
{
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pText = (SText*)m_pControlContainer->FindControl(65888);
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889);
	if (pText && pEdit)
	{
		m_nCoinMsgType = 8;
		pText->SetText(g_pMessageStringTable[363], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		pInputGoldPanel->SetVisible(1);

		auto pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);
		if (pInputBG2)
			pInputBG2->SetVisible(1);

		pEdit->m_nMaxStringLen = 20;
	}
}

int TMFieldScene::OnPacketREQParty(MSG_REQParty* pStd)
{
	auto pPartyList = m_pPartyList;
	if (!pStd || !pPartyList)
		return 0;

	pStd->Leader.Name[15] = 0;

	auto pPartyItem = new SListBoxPartyItem(pStd->Leader.Name,
		0xFFFFFFFF,
		0.0f,
		0.0f,
		104.0f,
		20.0f,
		pStd->Leader.ID,
		pStd->Leader.Class,
		pStd->Leader.Level,
		pStd->Leader.Hp,
		// PARTY retains the native member name MaxHp.
		pStd->Leader.MaxHp);

	if (!pStd->Leader.PartyIndex)
		pPartyItem->m_nState = 1;
	if (pPartyList->m_nNumItem > 0)
		pPartyList->Empty();

	pPartyList->AddItem(pPartyItem);

	if (m_pPartyPanel && !m_pPartyPanel->IsVisible())
		SetVisibleParty();

	auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->Leader.ID);
	if (pNode)
		pNode->SetInMiniMap(0xAAFFFF00);
	auto pChatList = m_pChatList;

	char szMsg[128]{};
	sprintf(szMsg, g_pMessageStringTable[62], pStd->Leader.Name);

	if (pChatList)
		pChatList->AddItem(new SListBoxItem(szMsg, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

	if (!m_bAutoParty)
	{
		sprintf(szMsg, g_pMessageStringTable[63], pStd->Leader.Name);

		if (pChatList)
			pChatList->AddItem(new SListBoxItem(szMsg, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

		if (!m_bCompatFieldScene)
		{
			if (m_pPartyAutoButton)
				m_pPartyAutoButton->SetVisible(0);
			if (m_pPartyAutoText)
				m_pPartyAutoText->SetVisible(0);

			auto partyback = m_pControlContainer
				? static_cast<SPanel*>(m_pControlContainer->FindControl(7602196))
				: nullptr;
			if (partyback)
				partyback->SetVisible(0);
		}
	}
	else
	{
		MSG_CNFParty2 stCnfParty{};

		stCnfParty.Header.ID = m_pMyHuman->m_dwID;
		stCnfParty.Header.Type = MSG_CNFParty2_Opcode;
		stCnfParty.LeaderID = pStd->Leader.ID;
		sprintf(stCnfParty.LeaderName, pStd->Leader.Name);

		if (pNode)
			pNode->m_bParty = 1;

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stCnfParty)->Type, reinterpret_cast<char*>(&stCnfParty), sizeof(stCnfParty)});
	}

	m_dwChatTime = g_pTimerManager->GetServerTime();

	GetSoundAndPlay(33, 0, 0);

	return 1;
}

int TMFieldScene::OnPacketAddParty(MSG_AddParty* pStd)
{
	auto pPartyList = m_pPartyList;
	if (!pStd || !pPartyList)
		return 0;

	pStd->Party.Name[15] = 0;
	for (int i = 0; i < pPartyList->m_nNumItem; ++i)
	{
		auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
		if (pPartyItem->m_dwCharID == pStd->Party.ID)
		{
			m_pPartyList->DeleteItem(pPartyItem);
			break;
		}
	}

	unsigned int dwColor = 0xFFFFFFFF;
	if (!pStd->Party.PartyIndex)
		dwColor = 0xFFAAAAFF;

	auto pPartyItem = new SListBoxPartyItem(pStd->Party.Name,
		dwColor,
		0.0f,
		0.0f,
		114.0f,
		20.0f,
		pStd->Party.ID,
		pStd->Party.Class,
		pStd->Party.Level,
		pStd->Party.Hp,
		// PARTY retains the native member name MaxHp.
		pStd->Party.MaxHp);

	if (!pStd->Party.PartyIndex)
		pPartyItem->m_nState = 2;

	pPartyList->AddItem(pPartyItem);

	auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->Party.ID);
	if (pNode)
	{
		pNode->m_bParty = 1;
		pNode->SetInMiniMap(0xAAFFFF00);
	}
	// Roster refreshes can arrive repeatedly while the party remains active.
	// Preserve the visibility chosen by the player instead of reopening the
	// panel for every MSG_AddParty update.

	return 1;
}

int TMFieldScene::OnPacketRemoveParty(MSG_RemoveParty* pStd)
{
	if (!pStd || !m_pPartyList)
		return 0;

	auto pPartyList = m_pPartyList;

	if (!pStd->Parm)
	{
		m_pMyHuman->m_bParty = 0;

		for (int i = 0; i < pPartyList->m_nNumItem; ++i)
		{
			auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(static_cast<SListBoxPartyItem*>(pPartyList->m_pItemList[i])->m_dwCharID);
			if (pNode)
			{
				pNode->m_bParty = 0;
				if (pNode)
				{
					if (_locationCheck(pNode->m_vecFromPos, 8, 15) || _locationCheck(pNode->m_vecFromPos, 8, 16) ||
						_locationCheck(pNode->m_vecFromPos, 9, 15) || _locationCheck(pNode->m_vecFromPos, 9, 16))
					{
						if (pNode->m_cMantua == 1)
							pNode->SetInMiniMap(0xAA0000FF);
						if (pNode->m_cMantua == 2)
							pNode->SetInMiniMap(0xAAFF0000);
					}
					else if (pNode->m_pInMiniMap)
						SAFE_DELETE(pNode->m_pInMiniMap);
				}
			}
		}

		pPartyList->Empty();
	}
	else
	{
		for (int inItemIndex = 0; inItemIndex < pPartyList->m_nNumItem; ++inItemIndex)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[inItemIndex];
			if (pPartyItem->m_dwCharID == pStd->Parm)
			{
				auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
				if (pNode)
				{
					pNode->m_bParty = 0;
					if (_locationCheck(pNode->m_vecFromPos, 8, 15) || _locationCheck(pNode->m_vecFromPos, 8, 16) ||
						_locationCheck(pNode->m_vecFromPos, 9, 15) || _locationCheck(pNode->m_vecFromPos, 9, 16))
					{
						if (pNode->m_cMantua == 1)
							pNode->SetInMiniMap(0xAA0000FF);
						if (pNode->m_cMantua == 2)
							pNode->SetInMiniMap(0xAAFF0000);
					}
					else if (pNode->m_pInMiniMap)
						SAFE_DELETE(pNode->m_pInMiniMap);
				}

				pPartyList->DeleteItem(inItemIndex);
				break;
			}
		}
	}

	if (pPartyList->m_nNumItem == 1)
	{
		pPartyList->Empty();
		m_pMyHuman->m_bParty = 0;
	}

	return 1;
}

int TMFieldScene::OnPacketReqChallange(MSG_STANDARD* pStd)
{
	if (pStd == nullptr || m_pMessageBox == nullptr)
		return 0;

	if (!m_pMessageBox->IsVisible())
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[407], 60, 0);
		m_pMessageBox->SetVisible(1);
	}

	return 1;
}

int TMFieldScene::OnPacketGuildDisable(MSG_STANDARDPARM* pStd)
{
	if (m_pBtnGuildOnOff)
		m_pBtnGuildOnOff->SetSelected(pStd->Parm == 1 ? 1 : 0);

	return 1;
}

int TMFieldScene::Guildmark_Create(stGuildMarkInfo* pMark)
{
	if (!pMark || !pMark->pGuildMark)
		return 0;

	int nFindMarkIndex = Guildmark_Find_ArrayIndex(pMark->nGuild + (pMark->nGuildChannel << 16));

	pMark->bLoadedGuildmark = 1;
	if (nFindMarkIndex != -1)
	{
		Guildmark_Link(pMark->pGuildMark, nFindMarkIndex, pMark->sGuildIndex);
		return 1;
	}

	pMark->nMarkIndex = Guildmark_Find_EmptyArrayIndex();
	if (pMark->nMarkIndex == -1)
		pMark->nMarkIndex = Guildmark_DeleteIdleGuildmark();

	if (pMark->nMarkIndex != -1)
	{
		char strFileName[64]{};

		int nChief = 0;
		if (pMark->sGuildIndex == 526 || pMark->sGuildIndex == 529 || pMark->sGuildIndex == 532 || pMark->sGuildIndex == 535)
			nChief = 1;
		if (pMark->sGuildIndex == 527 || pMark->sGuildIndex == 530 || pMark->sGuildIndex == 533 || pMark->sGuildIndex == 536)
			nChief = 2;
		if (pMark->sGuildIndex == 528 || pMark->sGuildIndex == 531 || pMark->sGuildIndex == 534 || pMark->sGuildIndex == 537)
			nChief = 3;

		Guildmark_MakeFileName(strFileName, pMark->nGuild, nChief, pMark->nGuildChannel);
		strcpy(pMark->strMarkFileName, strFileName);
		g_pTextureManager->m_stGuildMark[pMark->nMarkIndex].nGuild = pMark->nGuild + (pMark->nGuildChannel << 16);

		CreateThread(NULL, 0, Guildmark_Download, pMark, 0, NULL);
		return 1;
	}

	return 1;
}

void TMFieldScene::Guildmark_MakeFileName(char* szStr, int nGuild, int nChief, int nChannel)
{
	if (szStr)
		sprintf(szStr, "%c%02d%02d%04d.bmp", 'b', g_pObjectManager->m_nServerGroupIndex, nChannel, nGuild);
}

int TMFieldScene::Guildmark_Find_ArrayIndex(int nGuild)
{
	for (int i = 0; i < 64; ++i)
	{
		if (g_pTextureManager->m_stGuildMark[i].nGuild == nGuild)
			return i;
	}

	return -1;
}

int TMFieldScene::Guildmark_Find_EmptyArrayIndex()
{
	for (int i = 0; i < 64; ++i)
	{
		if (g_pTextureManager->m_stGuildMark[i].nGuild == -1)
			return i;
	}

	return -1;
}

int TMFieldScene::Guildmark_DeleteIdleGuildmark()
{
	int nMostOld = 0;
	unsigned int dwMostOldTime = 0;
	for (int i = 0; i < 64; ++i)
	{
		if (!g_pTextureManager->m_stGuildMark[i].nGuild)
			continue;

		if (!g_pTextureManager->m_stGuildMark[i].dwLastRenderTime)
		{
			nMostOld = i;
			break;
		}
		if (!dwMostOldTime)
		{
			nMostOld = i;
			dwMostOldTime = g_pTextureManager->m_stGuildMark[i].dwLastRenderTime;
		}
		else if (g_pTextureManager->m_stGuildMark[i].dwLastRenderTime < dwMostOldTime)
		{
			nMostOld = i;
			dwMostOldTime = g_pTextureManager->m_stGuildMark[i].dwLastRenderTime;
		}
	}

	g_pTextureManager->m_stGuildMark[nMostOld].nGuild = -1;

	SAFE_RELEASE(g_pTextureManager->m_stGuildMark[nMostOld].pTexture);

	g_pTextureManager->m_stGuildMark[nMostOld].dwLastRenderTime = 0;
	--m_nGuildMarkCount;
	return nMostOld;
}

int TMFieldScene::Guildmark_IsCorrectBMP(char* szMarkBuffer)
{
	if (!szMarkBuffer)
		return 0;

	BITMAPFILEHEADER header;
	memcpy(&header, szMarkBuffer, sizeof(BITMAPFILEHEADER));

	if (header.bfType != 19778)
		return 0;
	if (header.bfSize != 630 && header.bfSize != 632)
		return 0;

	BITMAPINFO info;
	memcpy(&info, &szMarkBuffer[14], header.bfOffBits - 14);
	if (info.bmiHeader.biWidth == 16 && info.bmiHeader.biHeight == 12)
		return info.bmiHeader.biBitCount == 24;

	return 0;
}

void TMFieldScene::Guildmark_Link(SPanel* pPanel, int nMarkIndex, int nGuildIndex)
{
	if (pPanel)
	{
		pPanel->m_GCPanel.nMarkIndex = nMarkIndex;
		if (nGuildIndex == 509 || nGuildIndex == 9)
			pPanel->m_GCPanel.nMarkLayout = 1;
		else if ((nGuildIndex < 526 || nGuildIndex > 531) && (nGuildIndex < 3 || nGuildIndex > 8))
			pPanel->m_GCPanel.nMarkLayout = 3;
		else
			pPanel->m_GCPanel.nMarkLayout = 2;
	}
}
