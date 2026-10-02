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

namespace
{
	void ClearDeletePassword(SEditableText* edit)
	{
		char empty[] = "";
		edit->SetText(empty);
	}
}

int TMSelectCharScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)
{
	STRUCT_SELCHAR* pSelChar = &g_pObjectManager->m_stSelCharData;
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (idwControlID == 5673)
	{
		if (m_bSelect)
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[11], 65796, 0);
			m_pMessageBox->SetVisible(1);
		}
		else
			VisibleSelectCreate(1);

		return 1;
	}
	if (idwControlID == 4617 && !idwEvent && m_pMessageBox->m_dwMessage == 65796)
		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
	if (idwControlID == 4628)
	{
		if (m_pControlContainer->m_pFocusControl && m_pControlContainer->m_pFocusControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT
			&& m_pControlContainer->m_pFocusControl->m_bVisible)
		{
			if (g_pEventTranslator->IsNative())
				g_pEventTranslator->SetIMEAlphaNumeric();
			else
				g_pEventTranslator->SetIMENative();
		}
		return 1;
	}
	if (idwControlID == 4612 && dwServerTime - m_dwLastClickLoginBtnTime > 2000)
	{
		int nSlot = g_pObjectManager->m_cCharacterSlot;
		if (nSlot < 0 || nSlot >= 4)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[14], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		else if (m_pHuman[nSlot] && pSelChar->MobName[nSlot][0])
		{
			// The adapter is borrowed only for the send; the scene keeps its controls,
			// timestamp and the legacy policy of advancing even if the send fails.
			SocketTransport<CPSock> socketTransport(*g_pSocketManager);
			CharacterLoginSender loginSender(socketTransport);
			RequestCharacterLogin(loginSender, nSlot);
			m_dwLastClickLoginBtnTime = dwServerTime;

			m_pBtnLogin->SetEnable(0);
			m_pBtnCancel->SetEnable(0);
			m_pBtnDelete->SetEnable(0);
		}
		return 1;
	}
	if (idwControlID == 1545)
	{
		SEditableText* pEditID = static_cast<SEditableText*>(m_pControlContainer->FindControl(4626));
		if (strlen(pEditID->GetText()) < 4)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[15], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		if (strlen(pEditID->GetText()) > 12)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[16], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		if (!BASE_CheckValidString(pEditID->GetText()))
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		char* szName = BASE_TransCurse(pEditID->GetText());
		for (size_t i = 0; i < strlen(szName) - 1; i++)
		{
			if (szName[i] == -95 && szName[i + 1] == -95)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
		}

		int slotId = 0;
		for (int i = 0; i < 4 && (!pSelChar || pSelChar->MobName[i][0]); i++)
			slotId++;

		int nClass = -1;
		for (int i = 0; i < 4; i++)
		{
			if (g_pObjectManager->m_pTargetObject == m_pSampleHuman[i])
				nClass = i;
		}

		if (!g_pObjectManager->m_pTargetObject || nClass == -1)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[14], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		if (slotId >= 0 && slotId < 4)
		{
			MSG_NewCharacter stNewCharacter{};
			stNewCharacter.Header.ID = 0;
			stNewCharacter.Header.Type = MSG_NewCharacter_Opcode;
			stNewCharacter.Class = nClass;
			stNewCharacter.Slot = slotId;

			sprintf(stNewCharacter.MobName, "%s", szName);

			g_pSocketManager->SendPacket({stNewCharacter.Header.Type,
				reinterpret_cast<char*>(&stNewCharacter), sizeof(stNewCharacter)});
			m_dwLastClickCreateBtnTime = dwServerTime;
			m_pBtnCreate->SetEnable(0);
		}
		return 1;
	}
	if (idwControlID == 4615)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[18], 4615, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == 4613)
	{
		VisibleSelectCreate(0);
		return 1;
	}
	if (idwControlID == 4616)
	{
		int nIndex = 0;
		for (nIndex = 0; nIndex < 4; nIndex++)
		{
			if (m_pHuman[nIndex] && m_pHuman[nIndex]->m_bSelected)
				break;
		}

		if (nIndex == 4)
		{
			VisibleSelectCreate(1);
		}
		else
		{
			LookSampleHuman(nIndex, 0, 1);
			m_pHuman[nIndex]->m_bSelected = 0;

			SPanel* pPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1282));
			pPanel->SetVisible(0);
		}
		if (m_pBtnDelete && m_pBtnDelete->m_bVisible == 1)
			m_pBtnDelete->SetVisible(0);

		return 1;
	}
	if (idwControlID == 1552)
	{
		int nIndex = 0;
		for (nIndex = 0; nIndex < 4; nIndex++)
		{
			if (m_pSampleHuman[nIndex] && m_pSampleHuman[nIndex]->m_bSelected)
				break;
		}
		if (nIndex == 4)
		{
			VisibleSelectCreate(1);
		}
		else
		{
			LookSampleHuman(nIndex, 0, 0);
			m_pSampleHuman[nIndex]->m_bSelected = 0;

			SPanel* pPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1542));
			pPanel->SetVisible(0);
		}
		return 1;
	}
	if (idwControlID == 628)
	{
		if (m_pRename)
			m_pRename->SetVisible(0);
		if (!m_pEditRename)
			return 1;
		// The selection may have been cleared since the rename prompt opened. Never
		// use the -1 sentinel (or an invalid received index) to read MobName.
		const int characterSlot = g_pObjectManager->m_cCharacterSlot;
		if (characterSlot < 0 || characterSlot >= 4)
			return 1;

		const size_t size = strnlen_s(m_pEditRename->GetText(), sizeof(m_pEditRename->m_strText));
		if (size < 4)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[15], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		if (size > 12)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[16], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		if (!BASE_CheckValidString(m_pEditRename->GetText()))
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		char* szName = BASE_TransCurse(m_pEditRename->GetText());
		if (strcmp(m_pEditRename->GetText(), szName))
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}

		const char* buf = m_pEditRename->GetText();
		for (size_t i = 0; i + 1 < size; i++)
		{
			if (buf[i] == -95 && buf[i + 1] == -95)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
		}

		MSG_ReqTransper stReqTransper{};
		stReqTransper.Header.ID = 0;
		stReqTransper.Header.Type = MSG_ReqTransper_Opcode;
		stReqTransper.Slot = characterSlot;
		stReqTransper.Result = 0;
		if (!character_transfer::CopyRequestNames(stReqTransper,
			pSelChar->MobName[characterSlot], buf, sizeof(m_pEditRename->m_strText)))
			return 1;

		g_pSocketManager->SendPacket({stReqTransper.Header.Type,
			reinterpret_cast<char*>(&stReqTransper), sizeof(stReqTransper)});
		m_bMovingNow = 1;
		m_dwLastMoveTime = dwServerTime;
		return 1;
	}
	if (idwControlID == 629)
	{
		if (m_pRename)
			m_pRename->SetVisible(0);
		m_bMovingNow = 0;
		return 1;
	}

	if (idwControlID == 1024)
	{
		if (!m_pPWEdit || !m_pInputPWPanel)
			return 1;
		const int characterSlot = g_pObjectManager->m_cCharacterSlot;
		if (characterSlot < 0 || characterSlot >= 4)
			return 1;
		const char* password = m_pPWEdit->GetText();
		if (strnlen_s(password, sizeof(m_pPWEdit->m_strText)) >= sizeof(MSG_DeleteCharacter::Password))
			return 1;

		MSG_DeleteCharacter stDelCharacter{};
		stDelCharacter.Header.ID = 0;
		stDelCharacter.Header.Type = MSG_DeleteCharacter_Opcode;
		stDelCharacter.Slot = characterSlot;
		sprintf_s(stDelCharacter.MobName, "%s", pSelChar->MobName[characterSlot]);
		sprintf_s(stDelCharacter.Password, "%s", password);

		g_pSocketManager->SendPacket({stDelCharacter.Header.Type,
			reinterpret_cast<char*>(&stDelCharacter), sizeof(stDelCharacter)});
		ClearDeletePassword(m_pPWEdit);
		m_pInputPWPanel->SetVisible(0);
		m_pControlContainer->SetFocusedControl(nullptr);
		return 1;
	}
	if (idwControlID == 921)
	{
		if (m_pPWEdit)
			ClearDeletePassword(m_pPWEdit);
		if (m_pInputPWPanel)
			m_pInputPWPanel->SetVisible(0);
		m_pControlContainer->SetFocusedControl(nullptr);
		return 1;
	}
	if (idwControlID == 4617)
	{
		if (idwEvent)
		{
			if (idwEvent == 1)
				m_pMessageBox->SetVisible(0);

			return 1;
		}

		int characterSlot = g_pObjectManager->m_cCharacterSlot;
		if (m_pMessageBox->m_dwMessage == 65796)
		{
			g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
			return 1;
		}
		if (m_pMessageBox->m_dwMessage == 4615 && characterSlot >= 0 && characterSlot < 4)
		{
			if (m_pInputPWPanel && m_pPWEdit)
			{
				ClearDeletePassword(m_pPWEdit);
				m_pInputPWPanel->SetVisible(1);
				m_pControlContainer->SetFocusedControl(m_pPWEdit);
			}
			return 1;
		}
		if (m_pMessageBox->m_dwMessage == 65875)
		{
			ShellExecute(0, 0, g_pMessageStringTable[263], 0, 0, 3);
			return 1;
		}
		if (m_pMessageBox->m_dwMessage == 1)
		{
			if (characterSlot < 0 || characterSlot >= 4)
				return 1;

			MSG_ReqTransper stReqTransper{};
			stReqTransper.Header.ID = 0;
			stReqTransper.Header.Type = MSG_ReqTransper_Opcode;
			stReqTransper.Slot = characterSlot;
			stReqTransper.Result = 0;
			if (!character_transfer::CopyRequestNames(stReqTransper,
				pSelChar->MobName[characterSlot], pSelChar->MobName[characterSlot],
				sizeof(pSelChar->MobName[characterSlot])))
				return 1;

			g_pSocketManager->SendPacket({stReqTransper.Header.Type,
				reinterpret_cast<char*>(&stReqTransper), sizeof(stReqTransper)});

			m_bMovingNow = 1;
			m_dwLastMoveTime = dwServerTime;
			m_pMessagePanel->SetMessage(g_pMessageStringTable[206], 0);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}
		if (m_pMessageBox->m_dwMessage == 0)
		{
			m_pMessageBox->SetVisible(0);
			return 1;
		}
		return 1;
	}

	return 0;
}

int TMSelectCharScene::OnCharEvent(char iCharCode, int lParam)
{
	auto pEditChar = m_pControlContainer->FindControl(4626u);

	if (iCharCode == VK_RETURN)
	{
		if (pEditChar->IsFocused() == 1)
			OnControlEvent(1545u, 0);

		return TMScene::OnCharEvent(iCharCode, lParam);
	}

	if (iCharCode != VK_ESCAPE)
		return TMScene::OnCharEvent(iCharCode, lParam);

	auto pSelChar = m_pControlContainer->FindControl(1282u);
	if (pSelChar->IsVisible() != 1)
	{
		if (m_pCreateWin->IsVisible() == 1)
			OnControlEvent(1552u, 0);
		else
			OnControlEvent(5673u, 0);
	}
	else
		OnControlEvent(4616u, 0);

	return 1;
}

int TMSelectCharScene::OnKeyDownEvent(unsigned int iKeyCode)
{
	if (TMScene::OnKeyDownEvent(iKeyCode) == 1)
		return 1;

	if (iKeyCode == VK_INSERT)
	{
		MSG_MessageWhisper stWhisper{};
		stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf_s(stWhisper.MobName, "time");

		g_pSocketManager->SendPacket({stWhisper.Header.Type,
			reinterpret_cast<char*>(&stWhisper), sizeof stWhisper});
	}

	return 1;
}

int TMSelectCharScene::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (TMScene::OnMouseEvent(dwFlags, wParam, nX, nY) == 1)
		return 1;

	if (m_pMessageBox2->IsVisible() == 1)
		return 1;

	if (m_pMessageBox->IsVisible() == 1)
		return 1;

	// Keep clicks in the native delete-password modal off the character models.
	if (m_pInputPWPanel && m_pInputPWPanel->IsVisible() == 1)
		return 1;

	if (m_bCriticalError == 1)
		return 1;

	if (m_pRename && m_pRename->m_bVisible == 1)
		return 1;

	if (dwFlags != 514 ||
		nX <= 0 ||
		nY <= 0 ||
		nX >= static_cast<int>(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) ||
		nY >= static_cast<int>(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
	{
		return 0;
	}

	auto pOver = m_pMouseOverHuman;

	if (m_bSelect == 1)
	{
		return OnMouseEventWhileSelecting(pOver);
	}
	if (m_pCreateWin->IsVisible() == 1)
	{
		return 1;
	}

	for (int nIndex = 0; nIndex < 4; ++nIndex)
	{
		if (m_pSampleHuman[nIndex])
			m_pSampleHuman[nIndex]->m_bSelected = 0;
	}

	char* szStr[4]{};
	char* szInt[4]{};
	char* szDex[4]{};
	char* szCon[4]{};

	for (int nIndex = 0; nIndex < 4; ++nIndex)
	{
		if (pOver && m_pSampleHuman[nIndex] && m_pSampleHuman[nIndex] == pOver && pOver->m_bMouseOver == 1)
		{
			m_pSampleHuman[nIndex]->m_bSelected = 1;

			g_pObjectManager->m_pTargetObject = static_cast<TMObject*>(m_pSampleHuman[nIndex]);

			// Oh my god, this must be like this temporarily
			szStr[0] = (char*)"8";
			szStr[1] = (char*)"5";
			szStr[2] = (char*)"6";
			szStr[3] = (char*)"8";
			szInt[0] = (char*)"4";
			szInt[1] = (char*)"8";
			szInt[2] = (char*)"6";
			szInt[3] = (char*)"9";
			szDex[0] = (char*)"7";
			szDex[1] = (char*)"5";
			szDex[2] = (char*)"9";
			szDex[3] = (char*)"13";
			szCon[0] = (char*)"6";
			szCon[1] = (char*)"5";
			szCon[2] = (char*)"5";
			szCon[3] = (char*)"6";

			float fHeight[4]{ 1.8f, 1.6f, 0.80000001f, 0.60000002f };

			auto pStrText = static_cast<SText*>(m_pControlContainer->FindControl(1562u));
			auto pIntText = static_cast<SText*>(m_pControlContainer->FindControl(1563u));
			auto pDexText = static_cast<SText*>(m_pControlContainer->FindControl(1564u));
			auto pConText = static_cast<SText*>(m_pControlContainer->FindControl(1565u));

			pStrText->SetText(szStr[nIndex], 0);
			pIntText->SetText(szInt[nIndex], 0);
			pDexText->SetText(szDex[nIndex], 0);
			pConText->SetText(szCon[nIndex], 0);

			m_fFocusHeight = fHeight[nIndex];

			LookSampleHuman(nIndex, 1, 0);

			float fStrWidth[4]{ 80.0f, 50.0f, 60.0f, 80.0f };
			float fIntWidth[4]{ 40.0f, 80.0f, 60.0f, 90.0f };
			float fDexWidth[4]{ 70.0f, 50.0f, 90.0f, 130.0f };
			float fConWidth[4]{ 60.0f, 50.0f, 50.0f, 60.0f };

			SPanel* pStat[4]{};

			pStat[0] = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_CREATE_CHAR_STAT_BAR1));
			pStat[1] = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_CREATE_CHAR_STAT_BAR2));
			pStat[2] = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_CREATE_CHAR_STAT_BAR3));
			pStat[3] = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_CREATE_CHAR_STAT_BAR4));

			auto pListBox = static_cast<SListBox*>(m_pControlContainer->FindControl(TMP_CREATE_CHAR_DESC));

			if (nIndex >= 0 && nIndex < 4)
			{
				pStat[0]->m_nWidth = fStrWidth[nIndex];
				pStat[1]->m_nWidth = fIntWidth[nIndex];
				pStat[2]->m_nWidth = fDexWidth[nIndex];
				pStat[3]->m_nWidth = fConWidth[nIndex];

				static_cast<SPanel*>(m_pControlContainer->FindControl(1542u))->SetVisible(1);
			}

			switch (nIndex)
			{
			case 0:
				static_cast<SText*>(m_pControlContainer->FindControl(TMT_CREATE_CLASS))->SetText(g_pMessageStringTable[121], 0);
				pListBox->Empty();
				TMScene::LoadMsgText(pListBox, (char*)"UI\\chardesctrans.txt");
				break;
			case 1:
				static_cast<SText*>(m_pControlContainer->FindControl(TMT_CREATE_CLASS))->SetText(g_pMessageStringTable[122], 0);
				pListBox->Empty();
				TMScene::LoadMsgText(pListBox, (char*)"UI\\chardescfoema.txt");
				break;
			case 2:
				static_cast<SText*>(m_pControlContainer->FindControl(TMT_CREATE_CLASS))->SetText(g_pMessageStringTable[123], 0);
				pListBox->Empty();
				TMScene::LoadMsgText(pListBox, (char*)"UI\\chardescbeast.txt");
				break;
			case 3:
				static_cast<SText*>(m_pControlContainer->FindControl(TMT_CREATE_CLASS))->SetText(g_pMessageStringTable[124], 0);
				pListBox->Empty();
				TMScene::LoadMsgText(pListBox, (char*)"UI\\chardeschunter.txt");
				break;
			}

			if (g_pSoundManager)
			{
				auto pSoundData = g_pSoundManager->GetSoundData(57);

				if (pSoundData)
					pSoundData->Play(0, 0);
			}

			switch (nIndex)
			{
			case 0:
				m_pSampleHuman[nIndex]->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK02, 0);
				break;
			case 1:
				m_pSampleHuman[nIndex]->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK06, 0);
				break;
			case 2:
				m_pSampleHuman[nIndex]->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK05, 0);
				break;
			case 3:
				m_pSampleHuman[nIndex]->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK05, 0);
				break;
			}

			if (nIndex == 3 && g_pSoundManager)
			{
				auto pSoundData = g_pSoundManager->GetSoundData(134);

				if (pSoundData)
					pSoundData->Play(0, 0);
			}

			return 1;
		}
	}

	m_pMouseOverHuman = nullptr;

	return 1;
}

// Extracted from TMSelectCharScene::OnMouseEvent; behavior is unchanged.
int TMSelectCharScene::OnMouseEventWhileSelecting(TMHuman*& pOver)
{
	if (pOver)
		m_pBtnDelete->SetVisible(1);

	auto pSelChar = m_pControlContainer->FindControl(1282u);

	if (pSelChar && pSelChar->IsVisible() == 1)
		return 1;

	int nSlot = g_pObjectManager->m_cCharacterSlot;

	for (int i = 0; i < 4; ++i)
	{
		if (m_pHuman[i])
			m_pHuman[i]->m_bSelected = 0;
	}

	int i;
	for (i = 0; i < 4; ++i)
	{
		if (pOver && m_pHuman[i] && m_pHuman[i] == pOver && pOver->m_bMouseOver == 1)
			break;
	}

	if (i >= 4)
	{
		m_pMouseOverHuman = nullptr;
		g_pObjectManager->m_cCharacterSlot = -1;

		auto pSelWin = m_pControlContainer->FindControl(1282u);

		if (pSelWin->m_nPosY == (307.0f * RenderDevice::m_fHeightRatio))
		{
			if (g_pSoundManager)
			{
				auto pSoundData = g_pSoundManager->GetSoundData(57);

				if (pSoundData)
					pSoundData->Play(0, 0);
			}

			m_dwStartSet = g_pTimerManager->GetServerTime();

			CamAction("UI\\SelCamAction3");
		}

		return 1;
	}

	g_pObjectManager->m_cCharacterSlot = i;

	m_pHuman[i]->m_bSelected = 1;

	if (pSelChar->m_nPosY == (int)(547.0f * RenderDevice::m_fHeightRatio))
	{
		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(57);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}

		m_dwStartRise = g_pTimerManager->GetServerTime();

		CamAction("UI\\SelCamAction2");
	}

	auto pName = static_cast<SText*>(m_pControlContainer->FindControl(1286u));
	auto pClass = static_cast<SText*>(m_pControlContainer->FindControl(1288u));
	auto pLevel = static_cast<SText*>(m_pControlContainer->FindControl(1289u));
	auto pCoin = static_cast<SText*>(m_pControlContainer->FindControl(1313u));
	auto pExp = static_cast<SText*>(m_pControlContainer->FindControl(1314u));
	auto pExpC = static_cast<SText*>(m_pControlContainer->FindControl(1315u));
	auto pStr = static_cast<SText*>(m_pControlContainer->FindControl(1316u));
	auto pInt = static_cast<SText*>(m_pControlContainer->FindControl(1317u));
	auto pDex = static_cast<SText*>(m_pControlContainer->FindControl(1318u));
	auto pCon = static_cast<SText*>(m_pControlContainer->FindControl(1319u));
	auto pSp1 = static_cast<SText*>(m_pControlContainer->FindControl(1321u));
	auto pSp2 = static_cast<SText*>(m_pControlContainer->FindControl(1322u));
	auto pSp3 = static_cast<SText*>(m_pControlContainer->FindControl(1323u));
	auto pSp4 = static_cast<SText*>(m_pControlContainer->FindControl(1324u));
	auto pGuild = static_cast<SText*>(m_pControlContainer->FindControl(1312u));

	char* szClass[4]
	{
		g_pMessageStringTable[121],
		g_pMessageStringTable[122],
		g_pMessageStringTable[123],
		g_pMessageStringTable[124]
	};

	int nClassa{};

	int nClass = g_pObjectManager->m_stSelCharData.Equip[i][0].sIndex % 10;

	if (nClass == 1)
	{
		nClassa = g_pObjectManager->m_stSelCharData.Equip[i][0].sIndex / 10;
		g_nBattleMaster = g_pObjectManager->m_stSelCharData.Equip[i][0].sIndex;
	}
	else if (nClass >= 6)
	{
		g_nBattleMaster = g_pObjectManager->m_stSelCharData.Equip[i][0].sIndex;
		nClassa = nClass - 6;
	}
	else
		nClassa = 2;

	if (nClassa > 3)
		nClassa = 0;

	pName->SetText(m_pHuman[i]->m_szName, 1);

	if (g_pObjectManager->m_stSelCharData.Equip[i][0].sIndex >= 40)
		pClass->SetText(g_pMessageStringTable[294], 0);
	else
		pClass->SetText(szClass[nClassa], 0);

	char szValue[128]{};

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Level + 1);

	pLevel->SetText(szValue, 0);

	sprintf_s(szValue, "%10d", g_pObjectManager->m_stSelCharData.Coin[i]);

	pCoin->m_cComma = 1;
	pCoin->SetText(szValue, 0);

	sprintf(szValue, "%I64d", g_pObjectManager->m_stSelCharData.Exp[i]);

	pExp->m_cComma = 1;
	pExp->SetText(szValue, 0);

	const int mantua = g_pObjectManager->m_stSelCharData.Equip[i][15].sIndex;
	// 3197..3199 select the native 7.48 G2 table. The additional capes are
	// compatible extensions; clamping keeps malformed/max levels in bounds.
	const bool secondClass =
		mantua == 3197 || mantua == 3198 || mantua == 3199 ||
		mantua == 573 || mantua == 1767 || mantua == 1770;
	const long long* levelTable = secondClass ? g_pNextLevel_G2 : g_pNextLevel;
	const int levelTableCount = secondClass ? _countof(g_pNextLevel_G2) : _countof(g_pNextLevel);
	const unsigned int level = g_pObjectManager->m_stSelCharData.Score[i].Level;
	const int nextLevelIndex = level >= static_cast<unsigned int>(levelTableCount - 1)
		? levelTableCount - 1
		: static_cast<int>(level + 1);

	sprintf_s(szValue, "%I64d", levelTable[nextLevelIndex]);

	pExpC->m_cComma = 1;
	pExpC->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Str);
	pStr->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Int);
	pInt->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Dex);
	pDex->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Con);
	pCon->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Mastery[0]);
	pSp1->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Mastery[1]);
	pSp2->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Mastery[2]);
	pSp3->SetText(szValue, 0);

	sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Score[i].Mastery[3]);
	pSp4->SetText(szValue, 0);

	if (g_pObjectManager->m_stSelCharData.Equip[i][0].stEffect[1].cEffect == 28)
	{
		if (g_pObjectManager->m_stSelCharData.Equip[i][0].stEffect[1].cValue)
		{
			sprintf_s(szValue, "%d", g_pObjectManager->m_stSelCharData.Equip[i][0].stEffect[1].cValue);
			pGuild->SetText(szValue, 0);
		}
		else
		{
			pGuild->SetText((char*)"", 0);
			pGuild->SetTextColor(0xFFFFFFFF);
		}
	}

	char* szTownName[5]
	{
		g_pMessageStringTable[125],
		g_pMessageStringTable[126],
		g_pMessageStringTable[173],
		g_pMessageStringTable[321],
		g_pMessageStringTable[127]
	};

	int nTownIndex = BASE_GetVillage(
		g_pObjectManager->m_stSelCharData.HomeTownX[i],
		g_pObjectManager->m_stSelCharData.HomeTownY[i]);

	auto pTextTownName = static_cast<SText*>(m_pControlContainer->FindControl(1296u));

	if (nTownIndex >= 0 && nTownIndex < 5)
	{
		pTextTownName->SetText(szTownName[nTownIndex], 0);
	}
	else
	{
		char szStrPos[128]{};

		sprintf_s(szStrPos, "[%d, %d]",
			g_pObjectManager->m_stSelCharData.HomeTownX[i],
			g_pObjectManager->m_stSelCharData.HomeTownY[i]);

		pTextTownName->SetText(szStrPos, 0);
	}

	switch (nClassa)
	{
	case 0:
		m_pSp1Caption->SetText(g_pMessageStringTable[242], 0);
		m_pSp2Caption->SetText(g_pMessageStringTable[243], 0);
		m_pSp3Caption->SetText(g_pMessageStringTable[244], 0);
		m_pSp4Caption->SetText(g_pMessageStringTable[245], 0);
		break;
	case 1:
		m_pSp1Caption->SetText(g_pMessageStringTable[246], 0);
		m_pSp2Caption->SetText(g_pMessageStringTable[247], 0);
		m_pSp3Caption->SetText(g_pMessageStringTable[248], 0);
		m_pSp4Caption->SetText(g_pMessageStringTable[249], 0);
		break;
	case 2:
		m_pSp1Caption->SetText(g_pMessageStringTable[250], 0);
		m_pSp2Caption->SetText(g_pMessageStringTable[251], 0);
		m_pSp3Caption->SetText(g_pMessageStringTable[252], 0);
		m_pSp4Caption->SetText(g_pMessageStringTable[253], 0);
		break;
	case 3:
		m_pSp1Caption->SetText(g_pMessageStringTable[254], 0);
		m_pSp2Caption->SetText(g_pMessageStringTable[255], 0);
		m_pSp3Caption->SetText(g_pMessageStringTable[256], 0);
		m_pSp4Caption->SetText(g_pMessageStringTable[257], 0);
		break;
	}

	LookSampleHuman(i, 1, 1);

	auto pPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1282u));
	pPanel->SetVisible(1);

	if (!m_pHuman[i]->m_pMantua)
	{
		m_pKingDomFlag->SetVisible(0);
	}
	else if (m_pHuman[i]->m_pMantua->m_Look.Skin0 == 19)
	{
		m_pKingDomFlag->SetVisible(0);
	}
	else
	{
		m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pHuman[i]->m_pMantua->m_Look.Skin0;

		if ((m_pHuman[i]->m_sHelmIndex == 3503 ||
			m_pHuman[i]->m_sHelmIndex == 3504 ||
			m_pHuman[i]->m_sHelmIndex == 3505 ||
			m_pHuman[i]->m_sHelmIndex == 3506) &&
			m_pHuman[i]->m_pMantua->m_Look.Skin0 == 2)
		{
			m_pKingDomFlag->m_GCPanel.nTextureIndex = 33;
		}

		m_pKingDomFlag->SetVisible(1);
	}

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(57);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}

	if (g_bMoveServer == 1)
	{
		// The confirmation reads the human's name before reaching the sender.
		// Also guard this entry when the slot has already been removed.
		const int characterSlot = g_pObjectManager->m_cCharacterSlot;
		if (characterSlot < 0 || characterSlot >= 4 || !m_pHuman[characterSlot])
			return 1;

		if (m_bMovingNow == 1)
			return 1;

		if ((g_pTimerManager->GetServerTime() - m_dwLastMoveTime) < 0x3E8)
			return 0;

		char szName[128]{};

		sprintf(szName, g_pMessageStringTable[149], m_pHuman[characterSlot]->m_szName);

		m_pMessageBox->SetMessage(szName, 1u, g_pMessageStringTable[200]);
		m_pMessageBox->SetVisible(1);
	}
	return 1;

}

