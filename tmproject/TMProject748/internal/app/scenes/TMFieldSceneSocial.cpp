#include "pch.h"
#include "TMFieldScene.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "WYD748Assets.h"
#include <WinInet.h>
#include <mutex>
#include <vector>
#include "GuildMarkDownload.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"

namespace
{
// A download only fetches bytes on its own thread. The texture is created by
// PumpGuildMarkDownloads on the render thread, and a request never points at a
// TMHuman, which may be destroyed while the download is in flight.
struct GuildMarkRequest
{
	int markIndex;
	int guildKey;
	char url[192];
	char data[640];
	DWORD size;
};

std::mutex g_guildMarkMutex;
std::vector<GuildMarkRequest*> g_guildMarkDone;
// Key of the request in flight per cache slot (0 = none) and the last failure,
// so a missing mark is retried after guild_mark_download::RetryMs.
int g_guildMarkPendingKey[64]{};
unsigned int g_guildMarkFailedAt[64]{};

DWORD WINAPI DownloadGuildMark(void* pArg)
{
	auto request = static_cast<GuildMarkRequest*>(pArg);
	HINTERNET session = InternetOpenA("WYD", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (session)
	{
		HINTERNET file = InternetOpenUrlA(session, request->url, nullptr, 0,
			INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
		if (file)
		{
			DWORD read = 0;
			while (request->size < sizeof(request->data) &&
				InternetReadFile(file, request->data + request->size,
					static_cast<DWORD>(sizeof(request->data)) - request->size, &read) && read)
			{
				request->size += read;
			}
			InternetCloseHandle(file);
		}
		InternetCloseHandle(session);
	}
	std::lock_guard<std::mutex> lock(g_guildMarkMutex);
	g_guildMarkDone.push_back(request);
	return 0;
}

void RequestGuildMark(int markIndex, int guildKey, const char* fileName)
{
	auto request = new GuildMarkRequest{};
	request->markIndex = markIndex;
	request->guildKey = guildKey;
	sprintf_s(request->url, "%s%s", guild_mark_download::Address, fileName);
	g_guildMarkPendingKey[markIndex] = guildKey;
	HANDLE thread = CreateThread(nullptr, 0, DownloadGuildMark, request, 0, nullptr);
	if (!thread)
	{
		g_guildMarkPendingKey[markIndex] = 0;
		g_guildMarkFailedAt[markIndex] = timeGetTime();
		delete request;
		return;
	}
	CloseHandle(thread);
}
}

void TMFieldScene::PumpGuildMarkDownloads()
{
	std::vector<GuildMarkRequest*> done;
	{
		std::lock_guard<std::mutex> lock(g_guildMarkMutex);
		done.swap(g_guildMarkDone);
	}
	for (auto request : done)
	{
		const int index = request->markIndex;
		const bool current = g_guildMarkPendingKey[index] == request->guildKey;
		if (current)
			g_guildMarkPendingKey[index] = 0;
		auto& slot = g_pTextureManager->m_stGuildMark[index];
		if (current && slot.nGuild == request->guildKey && !slot.pTexture)
		{
			if (request->size >= 630 && Guildmark_IsCorrectBMP(request->data) &&
				g_pTextureManager->LoadGuildTexture(index, request->data) == 1)
				++m_nGuildMarkCount;
			else
				g_guildMarkFailedAt[index] = timeGetTime();
		}
		delete request;
	}
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

	const unsigned int textControlID = TMT_INPUT_GOLD;
	const unsigned int editControlID = TME_INPUT_GOLD;
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

	const unsigned int textControlID = TMT_INPUT_GOLD;
	const unsigned int editControlID = TME_INPUT_GOLD;
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

	const int guildKey = pMark->nGuild + (pMark->nGuildChannel << 16);
	char strFileName[64]{};
	Guildmark_MakeFileName(strFileName, pMark->nGuild, 0, pMark->nGuildChannel);
	strcpy_s(pMark->strMarkFileName, strFileName);
	pMark->bLoadedGuildmark = 1;

	int nMarkIndex = Guildmark_Find_ArrayIndex(guildKey);
	if (nMarkIndex != -1)
	{
		Guildmark_Link(pMark->pGuildMark, nMarkIndex, pMark->sGuildIndex);
		// A guild without a mark (or a failed download) is retried later, so a
		// mark the leader uploads afterwards appears without restarting.
		if (!g_pTextureManager->m_stGuildMark[nMarkIndex].pTexture && !g_guildMarkPendingKey[nMarkIndex] &&
			timeGetTime() - g_guildMarkFailedAt[nMarkIndex] >= guild_mark_download::RetryMs)
			RequestGuildMark(nMarkIndex, guildKey, strFileName);
		return 1;
	}

	nMarkIndex = Guildmark_Find_EmptyArrayIndex();
	if (nMarkIndex == -1)
		nMarkIndex = Guildmark_DeleteIdleGuildmark();
	if (nMarkIndex < 0 || nMarkIndex >= 64)
		return 1;

	pMark->nMarkIndex = nMarkIndex;
	g_pTextureManager->m_stGuildMark[nMarkIndex].nGuild = guildKey;
	Guildmark_Link(pMark->pGuildMark, nMarkIndex, pMark->sGuildIndex);
	RequestGuildMark(nMarkIndex, guildKey, strFileName);
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
	// The native mark has a 54-byte header; a larger offset would copy past
	// the info header from a downloaded (untrusted) buffer.
	if (header.bfOffBits != sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
		return 0;

	BITMAPINFOHEADER info;
	memcpy(&info, &szMarkBuffer[sizeof(BITMAPFILEHEADER)], sizeof(info));
	if (info.biWidth == 16 && info.biHeight == 12)
		return info.biBitCount == 24 && info.biCompression == BI_RGB;

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
