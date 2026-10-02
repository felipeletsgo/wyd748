#include "pch.h"
#include "TMFieldScene.h"
#include "ServerEndpoint.h"
#include "../../application/FieldInteractionPolicy.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "AdapterIdentity.h"
#include "ClientDiagnostics.h"

int TMFieldScene::OnPacketCNFCharacterLogout(MSG_STANDARD* pStd)
{
	if (pStd->ID == g_pObjectManager->m_dwCharID)
	{
		g_pDevice->m_nWidthShift = 0;

		int nSlot = g_pObjectManager->m_cCharacterSlot;
		auto pSelChar = &g_pObjectManager->m_stSelCharData;

		memcpy(&g_pObjectManager->m_stSelCharData.Score[nSlot], &m_pMyHuman->m_stScore, sizeof(m_pMyHuman->m_stScore));
		memcpy(pSelChar->Equip[nSlot], g_pObjectManager->m_stMobData.Equip, sizeof(g_pObjectManager->m_stMobData.Equip));

		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTCHAR_STATE);
	}

	return 1;
}

int TMFieldScene::OnPacketCNFRemoveServer(MSG_CNFRemoveServer* pStd)
{
	// A migration response belongs only to the active character; accepting a
	// stale response can reconnect this scene with another session's ticket.
	if (pStd->Header.ID != g_pObjectManager->m_dwCharID)
		return 1;

	if (!g_pSocketManager->Sock)
	{
		// Validate before publishing the destination or indexing the local table.
		int nServer = 0;
		const int group = g_pObjectManager->m_nServerGroupIndex;
		if (group < 0 || group >= MAX_SERVERGROUP ||
			!ParseMigrationServer(pStd->TID, MAX_SERVERNUMBER, nServer))
			return 1;
		const auto& address = g_pServerList[group][nServer];
		// The loader decodes the entire entry and does not guarantee NUL.
		// Do not read the next entry or connect with a truncated address.
		if (!CopyServerEndpoint(g_pApp->m_szServerIP, address))
			return 1;
		m_pMessagePanel->SetMessage(g_pMessageStringTable[7], 0);
		m_pMessagePanel->SetVisible(1, 0);

		g_bMoveServer = 0;
		g_pObjectManager->m_nServerIndex = nServer;
		CheckPKNonePK(g_pObjectManager->m_nServerIndex);

		if (g_pSocketManager->ConnectServer(g_pApp->m_szServerIP, TM_CONNECTION_PORT, 0, 1124))
		{

			MSG_AccountLogin stAccountLogin{};
			stAccountLogin.Header.ID = 0;
			stAccountLogin.Header.Type = MSG_AccountLogin_Opcode;
			// Channel migration reuses the account-login ABI, so it must retain the
			// same 7.48 version marker as the initial login instead of reverting to
			// the newer TMProject protocol during reconnect.
			stAccountLogin.ClientVersion = 748;
			// WYD-Go now exposes one canonical 7.48 protocol, so reconnect must keep
			// the legacy DBNeedSave field zero instead of selecting a second ABI.
			stAccountLogin.DBNeedSave = 0;
			stAccountLogin.Header.Size = sizeof(MSG_AccountLogin);

			ReadFirstAdapterIdentity(stAccountLogin.AdapterName);

			strncpy(stAccountLogin.AccountName, pStd->AccountName, sizeof(pStd->AccountName));
			strncpy(stAccountLogin.Zero, pStd->TID, sizeof(pStd->TID));
			sprintf(stAccountLogin.AccountPassword, "");
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAccountLogin)->Type, reinterpret_cast<char*>(&stAccountLogin), sizeof(stAccountLogin)});

			return 1;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[8], 4000);
		m_pMessagePanel->SetVisible(1, 1);

		if (m_eSceneType != ESCENE_TYPE::ESCENE_LOGIN)
		{
			g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
		}

		return 1;
	}

	// Native 7.48 defers an already-connected migration packet until the
	// disconnect callback, preserving the complete 0x50-byte wire image.
	memmove(&m_stRemoveServer, pStd, sizeof(m_stRemoveServer));
	m_bMsgRemoveServer = 1;

	return 1;
}

int TMFieldScene::OnPacketCNFAccountLogin(MSG_CNFAccountLogin* pStd)
{
	// Read every selection field through MSG_CNFAccountLogin so the SecretCode
	// prefix and compiler alignment remain identical to WYD-Go's 1992-byte ABI.
	memcpy(&g_pObjectManager->m_stSelCharData, &pStd->SelChar, sizeof(pStd->SelChar));
	memcpy(g_pObjectManager->m_stItemCargo, pStd->Cargo, sizeof(pStd->Cargo));
	g_pObjectManager->m_nCargoCoin = pStd->Coin;
	memset(g_pObjectManager->m_stMemo, 0, sizeof(g_pObjectManager->m_stMemo));

	/*for (int i = 0; i < 16; ++i)
		g_pSocketManager->SendQueue[i] = *((unsigned char*)&pStd->Tick + i + 4);*/

	g_pSocketManager->SendCount = 0;
	g_pSocketManager->RecvCount = 0;
	return 1;
}

int TMFieldScene::OnPacketCNFCharacterLogin(MSG_CNFCharacterLogin* pStd)
{
	// The compact 7.48 field resource may not provide the inherited message
	// panel.  Character materialization must not depend on that optional HUD
	// element, otherwise the valid 0x114 confirmation can crash before the
	// world scene creates the local human.
	if (m_pMessagePanel)
		m_pMessagePanel->SetVisible(0, 1);
	g_pTimerManager->SetServerTime(pStd->Header.Tick);
	g_pObjectManager->m_dwCharID = pStd->ClientID;
	memcpy(&g_pObjectManager->m_stMobData, &pStd->MOB, sizeof(pStd->MOB));
	// The complete canonical Score is embedded in STRUCT_MOB. No legacy point
	// sidecars are copied here; doing so used stale offsets and zeroed live data.
	g_pObjectManager->m_nFakeExp = pStd->Ext1.Data[0];
	g_pObjectManager->m_stMobData.HomeTownX = pStd->PosX;
	g_pObjectManager->m_stMobData.HomeTownY = pStd->PosY;
	memcpy(g_pObjectManager->m_cShortSkill, g_pObjectManager->m_stMobData.ShortSkill, sizeof(g_pObjectManager->m_stMobData.ShortSkill));

	memcpy(&g_pObjectManager->m_cShortSkill[4], pStd->ShortSkill, sizeof(pStd->ShortSkill));
	for (int i = 0; i < 20; ++i)
	{
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 24)
			g_pObjectManager->m_cShortSkill[i] += 24 * g_pObjectManager->m_stMobData.Class;
	}

	g_nWeather = pStd->Weather;

	// The original 7.48 FieldScene2 resource has no modern HP/MP controls.
	// The compatibility scene materializes the player without those optional
	// widgets, so the login confirmation must not dereference absent bars.
	if (m_pHPBar)
		m_pHPBar->ResetBar();
	if (m_pMPBar)
		m_pMPBar->ResetBar();
	if (m_pMHPBar)
		m_pMHPBar->ResetBar();
	if (m_pMHPBarT)
		m_pMHPBarT->ResetBar();

	g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_FIELD2_STATE);
	return 1;
}

int TMFieldScene::OnPacketAutoKick(MSG_STANDARD* pStd)
{
	return 0;
}

int TMFieldScene::OnPacketDelayQuit(MSG_SysQuit* pStd)
{
	if (pStd && field_interaction::ShouldCloseOnDelayAck(g_dwStartQuitGameTime))
		PostMessage(g_pApp->m_hWnd, WM_CLOSE, 0, 0);
	return 0;
}
