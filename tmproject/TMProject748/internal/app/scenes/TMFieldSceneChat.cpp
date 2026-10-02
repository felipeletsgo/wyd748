#include "pch.h"
#include "TMFieldScene.h"
#include "TMGlobal.h"
#include "ObjectManager.h"
#include "SControl.h"
#include "SControlContainer.h"
#include "TMUtil.h"

void TMFieldScene::SetWhisper(char cOn)
{
	m_cWhisper = cOn;
	if (m_pChatWhisper)
	{
		m_pChatWhisper->m_bSelected = cOn != 0;
		m_pChatWhisper->Update();
	}
	if (m_pChatWhisper_C)
	{
		m_pChatWhisper_C->m_bSelected = m_bCompatFieldScene ? cOn != 0 : cOn == 0;
		m_pChatWhisper_C->Update();
	}
}

void TMFieldScene::SetPartyChat(char cOn)
{
	m_cPartyChat = cOn;
	if (m_pChatParty)
	{
		m_pChatParty->m_bSelected = cOn != 0;
		m_pChatParty->Update();
	}
	if (m_pChatParty_C)
	{
		m_pChatParty_C->m_bSelected = m_bCompatFieldScene ? cOn != 0 : cOn == 0;
		m_pChatParty_C->Update();
	}
}

void TMFieldScene::SetGuildChat(char cOn)
{
	m_cGuildChat = cOn;
	if (m_pChatGuild)
	{
		m_pChatGuild->m_bSelected = cOn != 0;
		m_pChatGuild->Update();
	}
	if (m_pChatGuild_C)
	{
		m_pChatGuild_C->m_bSelected = m_bCompatFieldScene ? cOn != 0 : cOn == 0;
		m_pChatGuild_C->Update();
	}
}

void TMFieldScene::SetKingDomChat(char cOn)
{
	if (m_pKingDomGuild)
	{
		m_pKingDomGuild->m_bSelected = cOn != 0;
		m_pKingDomGuild->Update();
	}
}

int TMFieldScene::OnPacketMessageChat(MSG_MessageChat* pStd)
{
	// The 7.48 native handler (FUN_00481dd6) verifies both optional UI controls
	// before consuming party chat; FieldScene2 can legitimately leave them unbound.
	if (!pStd || !m_pChatList || !m_pPartyList || m_pPartyList->m_nNumItem <= 0)
		return 0;

	if (!g_pObjectManager->GetHumanByID(pStd->Header.ID))
	{
		auto pPartyList = m_pPartyList;
		for (int i = 0; i < pPartyList->m_nNumItem; ++i)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
			if (pPartyItem->m_dwCharID == pStd->Header.ID)
			{
				pStd->String[sizeof(pStd->String) - 2] = 0;
				pStd->String[sizeof(pStd->String) - 1] = 0;

				auto pChatList = m_pChatList;

				char szMsg[128]{};
				sprintf(szMsg, "[%s]> %s", pPartyItem->GetText(), pStd->String);

				pChatList->AddItem(new SListBoxItem(szMsg, 0xFFAAFFAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

				m_dwChatTime = g_pTimerManager->GetServerTime();
				return 0;
			}
		}
	}

	return 0;
}

int TMFieldScene::OnPacketMessageChat_Index(MSG_MessageChat* pStd)
{
	// Keep the indexed form on the same native 7.48 lifecycle contract: packets
	// may arrive while the chat/party controls are absent during scene transitions.
	if (!pStd || !m_pChatList || !m_pPartyList)
		return 0;

	if (pStd->String[0] != 1)
		return 0;

	char str[128]{};
	char num[5]{};

	g_pMessageStringTable[*(short*)&pStd->String[2] + 1000][127] = 0;
	g_pMessageStringTable[*(short*)&pStd->String[2] + 1000][126] = 0;

	strcpy(str, g_pMessageStringTable[*(short*)&pStd->String[2] + 1000]);

	if (strlen(str) < 1)
	{
		_itoa(*(short*)&pStd->String[2], num, 10);
		strcpy(str, num);
	}

	if (m_pPartyList->m_nNumItem <= 0)
		return 0;

	if (!g_pObjectManager->GetHumanByID(pStd->Header.ID))
	{
		auto pPartyList = m_pPartyList;
		for (int i = 0; i < pPartyList->m_nNumItem; ++i)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
			if (pPartyItem->m_dwCharID == pStd->Header.ID)
			{
				pStd->String[sizeof(pStd->String) - 2] = 0;
				pStd->String[sizeof(pStd->String) - 1] = 0;

				auto pChatList = m_pChatList;

				char szMsg[128]{};
				sprintf(szMsg, "[%s]> %s", pPartyItem->GetText(), pStd->String);

				pChatList->AddItem(new SListBoxItem(szMsg, 0xFFAAFFAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

				m_dwChatTime = g_pTimerManager->GetServerTime();
				return 0;
			}
		}
	}

	return 0;
}

int TMFieldScene::OnPacketMessageChat_Param(MSG_STANDARD* pStd)
{
	return 0;
}

int TMFieldScene::OnPacketMessageWhisper(MSG_MessageWhisper* pMsg)
{
	if (g_pObjectManager->GetHumanByID(pMsg->Header.ID))
		return 0;

	auto pChatList = m_pChatList;
	pMsg->MobName[15] = 0;
	pMsg->String[sizeof(pMsg->String) - 2] = 0;
	pMsg->String[sizeof(pMsg->String) - 1] = 0;

	int nIndex = 0;
	unsigned int dwColor = 0xFFFFFF00;
	bool bDrawText = true;

	char szMsg[128]{};
	if (pMsg->String[0] == '-' && pMsg->Color == 3)
	{
		if (m_pChatGuild && !m_pChatGuild->m_bSelected)
			bDrawText = false;

		dwColor = 0xFFAAFFFF;
		nIndex = 1;
		if (pMsg->String[1] == '-')
		{
			dwColor = 0xFF00FFFF;
			nIndex = 2;
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
	}
	else if (pMsg->Color == 7)
	{
		dwColor = 0xFFBBBBBB;
		nIndex = 0;
		sprintf(szMsg, "[%s]> %s", pMsg->MobName, pMsg->String);
	}
	else if (pMsg->String[0] == '=')
	{
		if (m_pChatParty && !m_pChatParty->m_bSelected)
			bDrawText = false;

		// A party list can still be unbound while the Field resource is opening.
		// The server may already deliver 0x334; do not dereference it or show
		// the raw '=' prefix as ordinary chat in that partial state.
		if (!m_pPartyList)
			return 1;

		if (m_pPartyList->m_nNumItem > 1)
		{
			dwColor = 0xFFFF99FF;
			nIndex = 1;
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
	}
	else if (pMsg->String[0] == '@')
	{
		if (pMsg->String[1] == '@')
		{
			dwColor = 0xF0F60AFF;
			nIndex = 2;
		}
		else
		{
			dwColor = 0xFF00AAFF;
			nIndex = 1;
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
	}
	else if (pMsg->String[0] == '!')
	{
		SYSTEMTIME sysTime{};
		GetLocalTime(&sysTime);

		if (m_pHelpList[3])
		{
			m_pHelpList[3]->AddItem(new SListBoxItem((char*)" ", 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

			sprintf(szMsg, g_pMessageStringTable[226], pMsg->MobName);
			char szTime[128]{};
			sprintf(szTime, "%s [%02d:%02d:%02d]", szMsg, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
			m_pHelpList[3]->AddItem(new SListBoxItem(szTime, 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

			sprintf(szMsg, "%s", &pMsg->String[1]);
			m_pHelpList[3]->AddItem(new SListBoxItem(szMsg, 0xFFFFFFCC, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		}

		if (m_pHelpMemo)
			m_pHelpMemo->SetVisible(1);

		return 1;
	}
	else
	{
		if (m_pChatWhisper && !m_pChatWhisper->m_bSelected)
			bDrawText = false;

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, pMsg->String);
	}

	if (!bDrawText)
		return 1;

	if (strlen(pMsg->MobName) + strlen(pMsg->String) <= 43)
	{
		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);

		if(pChatList)
			pChatList->AddItem(new SListBoxItem(szMsg, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0));
	}
	else
	{
		char szMsg2[128]{};
		char szMsg3[128]{};

		if (IsClearString(pMsg->String, 42))
		{
			strncpy(szMsg3, pMsg->String, 43);
			sprintf(szMsg2, "%s", &pMsg->String[43]);
		}
		else
		{
			strncpy(szMsg3, pMsg->String, 42);
			sprintf(szMsg2, "%s", &pMsg->String[42]);
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &szMsg3[nIndex]);

		if(pChatList)
			pChatList->AddItem(new SListBoxItem(szMsg, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1, 0));

		if (strlen(pMsg->String) > 43 && pChatList)
			pChatList->AddItem(new SListBoxItem(szMsg2, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1, 0));
	}

	m_dwChatTime = g_pTimerManager->GetServerTime();
	return 1;
}

void TMFieldScene::SysMsgChat(char* str)
{
	auto pEdit = m_pEditChat;
	auto pChatList = m_pChatListnotice;

	pChatList->AddItem(new SListBoxItem(str, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
	pEdit->SetText((char*)"");

	m_pControlContainer->SetFocusedControl(nullptr);
	if (g_pObjectManager->m_bTvControl != 1)
		pEdit->SetText((char*)"");
}

void TMFieldScene::InsertInChatList(SListBox* pChatList, STRUCT_MOB *pMobData, SEditableText* pEditChat, unsigned int dwColor, int colorId, unsigned int startId)
{
	MSG_MessageWhisper stMsgWhisper{};
	stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
	stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;
	stMsgWhisper.Color = colorId;

	sprintf(stMsgWhisper.MobName, "");
	sprintf(stMsgWhisper.String, "%s", pEditChat->GetText());
	BASE_TransCurse(stMsgWhisper.String);

	pEditChat->SetText((char*)"");

	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});

	int len = strlen(stMsgWhisper.String) + strlen(pMobData->MobName);
	const size_t maxLen = 55;
	if (len <= maxLen)
	{
		char istrText[128]{};
		sprintf(istrText, "[%s]> %s", pMobData->MobName, &stMsgWhisper.String[startId]);

		auto ipNewItem = new SListBoxItem(istrText, dwColor, 0.0, 0.0, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);
	}
	else
	{
		char dest[128]{};
		char dest2[128]{};
		if (IsClearString(stMsgWhisper.String, maxLen - 1))
		{
			strncpy(dest, stMsgWhisper.String, maxLen);
			sprintf(dest2, "%s", &stMsgWhisper.String[maxLen]);
		}
		else
		{
			strncpy(dest, stMsgWhisper.String, maxLen - 1);
			sprintf(dest2, "%s", &stMsgWhisper.String[maxLen - 1]);
		}

		char istrText[128]{};
		sprintf(istrText, "[%s]> %s", g_pObjectManager->m_stMobData.MobName, &dest[startId]);

		auto ipNewItem = new SListBoxItem(istrText, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);

		auto ipNewItem2 = new SListBoxItem(dest2, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (dest2[0] && ipNewItem2 && pChatList)
			pChatList->AddItem(ipNewItem2);
	}
}
