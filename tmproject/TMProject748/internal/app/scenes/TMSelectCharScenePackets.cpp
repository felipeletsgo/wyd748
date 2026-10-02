#include "pch.h"
// TMSelectCharScene split by responsibility; scene setup and character list stay in TMSelectCharScene.cpp.
#include "TMSelectCharScene.h"
#include "TMCamera.h"
#include "TMGlobal.h"
#include "TMHuman.h"
#include "TMGround.h"
#include "TMSky.h"
#include "DirShow.h"
#include "SControlContainer.h"
#include "TMObjectContainer.h"
#include "TMLog.h"
#include "TMSkillJudgement.h"
#include "TMEffectSkinMesh.h"
#include "ClientDiagnostics.h"
#include "WYD748Assets.h"
#include "../../platform/windows/SocketTransport.h"
#include "../../application/RequestCharacterLogin.h"
#include "../../wire/CharacterLoginSender.h"

bool TMSelectCharScene::HandleJudgementEffect(char* buf)
{
	if (buf == nullptr)
		return false;

	MSG_STANDARDPARM2* parameters = reinterpret_cast<MSG_STANDARDPARM2*>(buf);
	const int index = parameters->Parm1;
	if (index < 0 || index > 3)
		return true;

	TMVector3 position{};
	if (index < 2)
		position = TMVector3(m_vecSelPos.x, 0.0f,
			(static_cast<float>(m_vecSelPos.y) - 2.8f) + static_cast<float>(index) * 1.4f);
	else
		position = TMVector3(m_vecSelPos.x - static_cast<float>(index - 1) * 1.4f,
			0.0f, m_vecSelPos.y);

	if (m_pEffectContainer)
	{
		TMSkillJudgement* effect = new TMSkillJudgement(position, 1, 0.1f);
		m_pEffectContainer->AddChild(effect);
	}
	return true;
}

void TMSelectCharScene::HandleCharacterLogin(char* buf)
{
	MSG_CNFCharacterLogin* login = reinterpret_cast<MSG_CNFCharacterLogin*>(buf);
	g_pTimerManager->SetServerTime(login->Header.Tick);

	g_pObjectManager->m_dwCharID = login->ClientID;
	memcpy(&g_pObjectManager->m_stMobData, &login->MOB, sizeof(login->MOB));
	// The 7.48 packet already carries the canonical Score; later sidecars
	// would reinterpret bytes that belong to other fields.
	g_pObjectManager->m_nFakeExp = login->Ext1.Data[0];
	g_pObjectManager->m_stMobData.HomeTownX = login->PosX;
	g_pObjectManager->m_stMobData.HomeTownY = login->PosY;
	memcpy(g_pObjectManager->m_cShortSkill,
		g_pObjectManager->m_stMobData.ShortSkill, 4);
	memcpy(&g_pObjectManager->m_cShortSkill[4], login->ShortSkill,
		sizeof(login->ShortSkill));

	for (int skill = 0; skill < 20; ++skill)
	{
		if (static_cast<unsigned char>(g_pObjectManager->m_cShortSkill[skill]) < 24)
			g_pObjectManager->m_cShortSkill[skill] +=
				24 * g_pObjectManager->m_stMobData.Class;
	}

	g_nWeather = login->Weather;
	g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_FIELD_STATE);
}

void TMSelectCharScene::HandleCharacterCreated(char* buf)
{
	MSG_CNFNewCharacter* pNewCharacter = reinterpret_cast<MSG_CNFNewCharacter*>(buf);
	m_pMessagePanel->SetVisible(0, 1);
	memcpy(&g_pObjectManager->m_stSelCharData, &pNewCharacter->SelChar, sizeof STRUCT_SELCHAR);
	ReloadCharList(RELOAD_CHARLIST_TYPE::CREATE_CHARACTER);
}

void TMSelectCharScene::HandleCharacterDeleted(char* buf)
{
	MSG_CNFDeleteCharacter* pDeleteCharacter = reinterpret_cast<MSG_CNFDeleteCharacter*>(buf);
	m_pMessagePanel->SetVisible(0, 1);
	memcpy(&g_pObjectManager->m_stSelCharData, &pDeleteCharacter->SelChar, sizeof(pDeleteCharacter->SelChar));
	OnControlEvent(4616, 0);
	ReloadCharList(RELOAD_CHARLIST_TYPE::DELETE_CHARACTER);
}

void TMSelectCharScene::ShowCharacterOperationMessage(int messageIndex)
{
	m_pMessagePanel->SetMessage(g_pMessageStringTable[messageIndex], 2000);
	m_pMessagePanel->SetVisible(1, 1);
}

void TMSelectCharScene::HandleMoveServerNotification()
{
	g_bMoveServer = 1;
}

int TMSelectCharScene::OnPacketEvent(unsigned int dwCode, char* buf)
{
	if (TMScene::OnPacketEvent(dwCode, buf) == 1)
		return 1;

	if (buf == nullptr)
		return 0;

	MSG_STANDARD* pStd = reinterpret_cast<MSG_STANDARD*>(buf);

	switch (pStd->Type)
	{
	case 0x3B4:
		return HandleJudgementEffect(buf) ? 1 : 0;
	case MSG_CNFNewCharacter_Opcode:
		HandleCharacterCreated(buf);
	return 1;
	case MSG_CNFNewCharacterFail_Opcode:
		ShowCharacterOperationMessage(19);
	return 1;
	case MSG_CNFDeleteCharacter_Opcode:
		HandleCharacterDeleted(buf);
	return 1;
	case 0x11B:
		ShowCharacterOperationMessage(20);
	return 1;
	case MSG_CNFCharacterLogin_Opcode:
		HandleCharacterLogin(buf);
	return 1;
	case 0x119:
		ShowCharacterOperationMessage(21);
	return 1;
	case 0x7A9:
		HandleMoveServerNotification();
	return 1;
	case MSG_ReqTransper_Opcode:
	{
		MSG_ReqTransper* pReqTransper = (MSG_ReqTransper*)pStd;
		m_bMovingNow = 0;

		// The response is not authoritative for indexing past the four slots.
		// Consume the pending operation even on rejection, as in the negative case.
		if (pReqTransper->Slot < 0 || pReqTransper->Slot >= 4)
			return 1;

		if (!pReqTransper->Result)
		{
			char szCharName[128]{};
			// The control is borrowed from the resource; its absence does not prevent
			// applying the confirmation. An already removed human uses the slot name,
			// terminated locally, without recreating a visual or keeping stale data.
			const char* renamed = m_pEditRename ? m_pEditRename->GetText() : "";
			if (!strcmp(renamed, ""))
			{
				char slotName[17]{};
				memcpy(slotName, g_pObjectManager->m_stSelCharData.MobName[pReqTransper->Slot], 16);
				const char* previousName = m_pHuman[pReqTransper->Slot]
					? m_pHuman[pReqTransper->Slot]->m_szName : slotName;
				sprintf(szCharName, g_pMessageStringTable[201], previousName);
			}
			else
			{
				sprintf(szCharName, g_pMessageStringTable[201], renamed);
				m_pEditRename->SetText((char*)"");
			}

			m_pMessagePanel->SetMessage(szCharName, 3500);
			m_pMessagePanel->SetVisible(1, 1);

			int _idx = 0;
			for (_idx = 0; _idx < 4; _idx++)
			{
				if (m_pHuman[_idx] && m_pHuman[_idx]->m_bSelected)
					break;
			}

			if (_idx == 4)
			{
				VisibleSelectCreate(1);
			}
			else
			{
				LookSampleHuman(_idx, 0, 1);
				m_pHuman[_idx]->m_bSelected = 0;
			}

			m_pControlContainer->FindControl(1282)->SetVisible(0);

			m_pBtnDelete->SetVisible(0);
			m_pNewCharPanel->SetVisible(1);

			m_pControlContainer->FindControl(4613)->SetEnable(1);

			g_pObjectManager->m_stSelCharData.HomeTownX[pReqTransper->Slot] = 0;
			g_pObjectManager->m_stSelCharData.HomeTownY[pReqTransper->Slot] = 0;
			g_pObjectManager->m_stSelCharData.Guild[pReqTransper->Slot] = 0;
			g_pObjectManager->m_stSelCharData.Coin[pReqTransper->Slot] = 0;
			g_pObjectManager->m_stSelCharData.Exp[pReqTransper->Slot] = 0;

			memset(g_pObjectManager->m_stSelCharData.MobName[pReqTransper->Slot], 0, sizeof(g_pObjectManager->m_stSelCharData.MobName[pReqTransper->Slot]));
			memset(g_pObjectManager->m_stSelCharData.Equip[pReqTransper->Slot], 0, sizeof(g_pObjectManager->m_stSelCharData.Equip[pReqTransper->Slot]));
			memset(&g_pObjectManager->m_stSelCharData.Score[pReqTransper->Slot], 0, sizeof(g_pObjectManager->m_stSelCharData.Score[pReqTransper->Slot]));

			g_pObjectManager->DeleteObject(m_pHuman[pReqTransper->Slot]);
			m_pHuman[pReqTransper->Slot] = nullptr;
			g_pObjectManager->m_cCharacterSlot = -1;
		}
		else if (pReqTransper->Result == 1)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[202], 3500);
			m_pMessagePanel->SetVisible(1, 1);
			if (m_pRename)
				m_pRename->SetVisible(1);

			SEditableText* pEdit = (SEditableText*)m_pControlContainer->FindControl(1569);
			if (pEdit)
				m_pControlContainer->SetFocusedControl(pEdit);
		}
		else if (pReqTransper->Result == 2)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[203], 3500);
			m_pMessagePanel->SetVisible(1, 1);
		}
		else if (pReqTransper->Result == 3)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[205], 3500);
			m_pMessagePanel->SetVisible(1, 1);
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[204], 3500);
			m_pMessagePanel->SetVisible(1, 1);
		}
	}
	return 1;
	case MSG_InitGuldName_Opcode:
	{
		MSG_INITGULDNAME* pGuildName = (MSG_INITGULDNAME*)pStd;
		memcpy(g_pObjectManager->m_strGuildName[pGuildName->Parm], pGuildName->GuildName, sizeof(pGuildName->GuildName));
	}
	return 1;
	}

	return 0;
}
