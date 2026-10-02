#include "pch.h"
#include "SControlContainer.h"
#include "SGrid.h"
#include "UIBinary.h"
#include "TMGround.h"
#include "TMSky.h"
#include "TMSun.h"
#include "TMLight.h"
#include "TMObject.h"
#include "TMCamera.h"
#include "TMObjectContainer.h"
#include "TMItem.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMScene.h"
#include "TMFieldScene.h"
#include "TMEffectFirework.h"
#include "TMSnow.h"
#include "TMRain.h"
#include "WYD748Assets.h"


namespace
{
	constexpr size_t kIndexedMessageCapacity = MAX_STRING_LENGTH;
	constexpr size_t kIndexedParameterCount = 6;
	constexpr size_t kIndexedParameterCapacity = sizeof(MSG_MessageChat::String) - 3;

	bool DecodeIndexedMessage(const MSG_STANDARD* pStd, const MSG_MessageChat*& pMessage,
		std::int16_t& relativeIndex, int& tableIndex)
	{
		if (!pStd || pStd->Size != sizeof(MSG_MessageChat))
			return false;

		pMessage = reinterpret_cast<const MSG_MessageChat*>(pStd);
		if (pMessage->String[0] != '\0')
			return false;

		// The extension keeps the legacy TMProject signed relative index, but
		// avoids the old unaligned short dereference on packet memory.
		std::memcpy(&relativeIndex, &pMessage->String[2], sizeof(relativeIndex));
		tableIndex = static_cast<int>(relativeIndex) + 1000;
		return true;
	}

	bool CopyIndexedTemplate(int tableIndex, char* output, size_t outputCapacity)
	{
		if (!output || outputCapacity == 0 || tableIndex < 0 || tableIndex >= MAX_STRING)
			return false;

		const char* source = g_pMessageStringTable[tableIndex];
		size_t length = 0;
		while (length + 1 < outputCapacity && length < MAX_STRING_LENGTH - 1 && source[length] != '\0')
			++length;

		if (length == 0)
			return false;

		std::memcpy(output, source, length);
		output[length] = '\0';
		return true;
	}

	void CopyRelativeIndexFallback(std::int16_t relativeIndex, char* output, size_t outputCapacity)
	{
		if (!output || outputCapacity == 0)
			return;

		sprintf_s(output, outputCapacity, "%d", static_cast<int>(relativeIndex));
	}

	size_t ParseIndexedParameters(const MSG_MessageChat* pMessage,
		char parameters[kIndexedParameterCount][kIndexedParameterCapacity])
	{
		std::memset(parameters, 0, kIndexedParameterCount * kIndexedParameterCapacity);
		size_t parameterIndex = 0;
		size_t parameterOffset = 0;
		size_t parameterCount = 1;

		for (size_t i = 4; i < sizeof(pMessage->String) && pMessage->String[i] != '\0'; ++i)
		{
			if (pMessage->String[i] == ',')
			{
				if (parameterIndex + 1 >= kIndexedParameterCount)
					break;

				++parameterIndex;
				parameterOffset = 0;
				parameterCount = parameterIndex + 1;
				continue;
			}

			if (parameterOffset + 1 < kIndexedParameterCapacity)
				parameters[parameterIndex][parameterOffset++] = pMessage->String[i];
		}

		return parameterCount;
	}

	void AppendIndexedText(char* output, size_t outputCapacity, size_t& outputOffset, const char* text)
	{
		if (!text)
			return;

		for (size_t i = 0; text[i] != '\0' && outputOffset + 1 < outputCapacity; ++i)
			output[outputOffset++] = text[i];
		output[outputOffset] = '\0';
	}

	void FormatIndexedTemplate(const char* messageTemplate,
		const char parameters[kIndexedParameterCount][kIndexedParameterCapacity],
		size_t parameterCount, char* output, size_t outputCapacity)
	{
		if (!output || outputCapacity == 0)
			return;

		output[0] = '\0';
		size_t outputOffset = 0;
		size_t parameterIndex = 0;
		for (size_t i = 0; messageTemplate && messageTemplate[i] != '\0' && outputOffset + 1 < outputCapacity; ++i)
		{
			if (messageTemplate[i] == '%' && messageTemplate[i + 1] == '%')
			{
				output[outputOffset++] = '%';
				output[outputOffset] = '\0';
				++i;
				continue;
			}

			if (messageTemplate[i] == '%' && messageTemplate[i + 1] == 's')
			{
				if (parameterIndex < parameterCount)
					AppendIndexedText(output, outputCapacity, outputOffset, parameters[parameterIndex]);
				++parameterIndex;
				++i;
				continue;
			}

			// Unsupported printf directives remain literal asset text. The packet
			// never becomes a format string, so malformed assets cannot consume
			// stack arguments or write outside the destination.
			output[outputOffset++] = messageTemplate[i];
			output[outputOffset] = '\0';
		}
	}

	DWORD IndexedMessageDuration(int tableIndex)
	{
		return tableIndex == 465 || tableIndex == 466 || tableIndex == 484 || tableIndex == 485
			? 600000
			: 4000;
	}
}

TMScene::TMScene() : TreeNode(0)
{
	m_eSceneType = ESCENE_TYPE::ESCENE_NONE;
	m_pControlContainer = nullptr;
	m_pExtraContainer = nullptr;
	m_pGround = nullptr;
	m_pMouseOverHuman = nullptr;
	m_pMouseOverItem = nullptr;
	m_sPlayDemo = -1;
	m_nCurrentGroundIndex = 0;
	m_dwStartCamTime = 0;
	g_ClipFar = 70.0f;
	SControl::m_dwStaticID = 0;
	m_nAdjustTime = 0;
	m_dwInitTime = 0;
	n_bPrtScreen = 0;
	m_pMyHuman = nullptr;
	m_pSky = nullptr;
	m_pSun = nullptr;
	m_pHumanContainer = nullptr;
	m_pItemContainer = nullptr;
	m_pAlphaNative = nullptr;
	m_pDescPanel = nullptr;
	m_pTextBillMsg = nullptr;
	m_bShowNameLabel = 0;
	m_bCriticalError = 0;
	m_bAutoRun = 0;
	m_bReverse = 0;
	TMLight::m_dwBaseLightIndex = 2;
	m_pTextIMEDesc = 0;

	for (int nImeCount = 0; nImeCount < 10; ++nImeCount)
		m_pTextCandidate[nImeCount] = 0;

	m_pTextCompose = nullptr;
	m_pTextComposeB = nullptr;
	m_pTextReadCompose = nullptr;
	m_pTextReadComposeB = nullptr;
	m_dwDelayDisconnectTime = 0;
	m_bMsgRemoveServer = 0;

	for (int i = 0; i < 2; ++i)
	{
		m_pGroundList[i] = 0;
		m_pObjectContainerList[i] = 0;
	}

	m_pControlContainer = new SControlContainer(this);

	m_pExtraContainer = new TreeNode(0);

	AddChild(m_pExtraContainer);

	m_pEffectContainer = new TreeNode(0);

	AddChild(m_pEffectContainer);

	m_pShadeContainer = new TreeNode(0);

	AddChild(m_pShadeContainer);

	m_pGroundObjectContainer = new TreeNode(0);

	AddChild(m_pGroundObjectContainer);

	m_pHumanContainer = new TreeNode(0);

	AddChild(m_pHumanContainer);

	m_pTextBillMsg = new SText(-2, "ºô", 0xFFFFFFFF, 120.0f, 70.0f, 540.0f, 20.0f, 1, 0xAAFF0000, 1, 1);
	m_pTextBillMsg->m_bSelectEnable = 0;
	m_pTextBillMsg->SetVisible(0);

	// FUN_00493e70 constructs the 7.48 MessagePanel2 at 480x28.  Keep that exact
	// geometry for the shared login/notice/exit notification while the rest of the
	// HUD remains classic; the 40px branch belongs to the stock opaque composition.
	m_pMessagePanel = new SMessagePanel("Message Panel", 150.0f, 80.0f, 480.0f,
		28.0f, 2000);

	m_pMessageBox = new SMessageBox(
		"Message Box",
		0,
		(((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) - 256.0f) / 2.0f,
		(((float)g_pDevice->m_dwScreenHeight / RenderDevice::m_fHeightRatio) - 152.0f) / 2.0f);

	m_pMessageBox2 = new SMessageBox(
		"Message Box",
		4,
		(((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) - 256.0f) / 2.0f,
		(((float)g_pDevice->m_dwScreenHeight / RenderDevice::m_fHeightRatio) - 152.0f) / 2.0f);

	for (int nImeCounta = 0; nImeCounta < 10; ++nImeCounta)
	{
		m_pTextCandidate[nImeCounta] = new SText(
			-2,
			"",
			0xFFFFFFFF,
			((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) - 180.0f,
			(((float)g_pDevice->m_dwScreenHeight / RenderDevice::m_fHeightRatio) - 161.0f) + (float)(16 * nImeCounta),
			179.0f,
			16.0f,
			1,
			0x9955AA55,
			1,
			0);
	}

	m_pTextCompose = new SText(
		-2,
		"",
		0xFFFFFFFF,
		0.0f,
		((float)g_pDevice->m_dwScreenHeight / RenderDevice::m_fHeightRatio) - 36.0f,
		200.0f,
		16.0f,
		1,
		0xAA55AA55,
		1,
		0);

	m_pTextReadCompose = new SText(
		-2,
		"",
		0xFFFFFFFF,
		0.0f,
		((float)g_pDevice->m_dwScreenHeight / RenderDevice::m_fHeightRatio) - 36.0f,
		200.0f,
		16.0f,
		1,
		0xAA55AA55,
		1,
		0);

	m_pTextComposeB = new SText(
		-2,
		"",
		0xFFFFFF00,
		0.0f,
		((float)g_pDevice->m_dwScreenHeight / RenderDevice::m_fHeightRatio) - 36.0f,
		200.0f,
		16.0f,
		0,
		0xAA55AA55,
		1,
		0);

	m_pTextReadComposeB = new SText(
		-2,
		"",
		0xFFFFFF00,
		0.0f,
		((float)g_pDevice->m_dwScreenHeight / RenderDevice::m_fHeightRatio) - 36.0f,
		200.0f,
		16.0f,
		0,
		0xAA55AA55,
		1,
		0);

	m_pTextCompose->SetVisible(0);
	m_pTextReadCompose->SetVisible(0);
	m_pTextComposeB->SetVisible(0);
	m_pTextReadComposeB->SetVisible(0);
	m_pMessagePanel->SetVisible(0, 1);
	m_pMessageBox->SetVisible(0);
	m_pMessageBox2->SetVisible(0);

	if (m_pControlContainer)
		m_pMessageBox->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		m_pMessageBox->SetEventListener(nullptr);

	if (m_pControlContainer)
		m_pMessageBox2->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		m_pMessageBox2->SetEventListener(nullptr);

	m_pMessageBox->m_bModal = 1;
	m_pMessageBox2->m_bModal = 1;
	m_pMessagePanel->m_bSelectEnable = 0;

	m_pControlContainer->m_pModalControl[0] = dynamic_cast<SControl*>(m_pMessageBox);
	m_pControlContainer->m_pModalControl[4] = dynamic_cast<SControl*>(m_pMessageBox2);

	m_pControlContainer->AddItem(static_cast<SControl*>(m_pMessagePanel));
	m_pControlContainer->AddItem(static_cast<SControl*>(m_pMessageBox));
	m_pControlContainer->AddItem(static_cast<SControl*>(m_pMessageBox2));
	m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextBillMsg));

	for (int nImeCountb = 0; nImeCountb < 10; ++nImeCountb)
	{
		m_pTextCandidate[nImeCountb]->SetVisible(0);

		m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextCandidate[nImeCountb]));
	}

	m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextComposeB));
	m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextReadComposeB));
	m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextCompose));
	m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextReadCompose));

	float fHeight = (float)g_pDevice->m_dwScreenHeight - 20.0f;

	m_pAlphaNative = new SText(-2, "Ch", 0xFFFFFFDD, 0.0f, fHeight, 28.0f, 16.0f, 1, 0xAA000077, 1, 0);
	m_pAlphaNative->SetVisible(0);

	m_pControlContainer->AddItem(static_cast<SControl*>(m_pAlphaNative));

	m_pTextIMEDesc = new SText(-2, "IME Desc", 0xFFFFFFDD, 30.0f, fHeight, 180.0f, 16.0f, 1, 0xAA000077, 1, 0);
	m_pTextIMEDesc->SetVisible(0);

	m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextIMEDesc));

	if (g_pEventTranslator)
	{
		if (g_pEventTranslator->IsNative())
		{
			char tmp[] = "Ch";
			m_pAlphaNative->SetText(tmp, 0);
			
			char szDesc[256]{};

			ImmGetDescriptionA(GetKeyboardLayout(0), szDesc, sizeof(szDesc));

			m_pTextIMEDesc->SetText(szDesc, 0);
			m_pTextIMEDesc->SetSize((float)(8 * strlen(szDesc)) + 16.0f, 16.0f);
		}
		else
		{
			char tmp[] = "En";
			m_pAlphaNative->SetText(tmp, 0);
		}
	}

	memset(m_HeightMapData, 0, sizeof(m_HeightMapData));
	memset(m_GateMapData, 0, sizeof(m_GateMapData));

	BASE_ApplyAttribute((char*)m_HeightMapData, 256);

	for (int i = 0; i < 32; ++i)
		m_TargetAffect[i] = 0;
}

TMScene::~TMScene()
{
	g_pTimerManager->m_dwDelayTime = 20000;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	g_pObjectManager->EffectFrameMove(m_pEffectContainer, dwServerTime);

	SAFE_DELETE(m_pHumanContainer);

	SAFE_DELETE(m_pItemContainer);

	SAFE_DELETE(m_pControlContainer);

	g_pObjectManager->m_pPreviousScene = nullptr;
	g_pObjectManager->CleanUp();

	m_pGround = nullptr;

	g_nUnDelMobCount = 0;

	g_pTimerManager->m_dwDelayTime = 0;
}

SControlContainer* TMScene::GetCtrlContainer()
{
	return m_pControlContainer;
}

int TMScene::InitializeScene()
{
	SetFocus(g_pApp->m_hWnd);
	return 1;
}

int TMScene::OnPacketEvent(unsigned int dwCode, char* pSBuffer)
{
	if (g_pCurrentScene != this)
		return 0;

	auto pStd = reinterpret_cast<MSG_STANDARD*>(pSBuffer);
	auto dwServerTime = g_pTimerManager->GetServerTime();

	//if (!pSBuffer && (!m_dwDelayDisconnectTime || m_dwDelayDisconnectTime + 15000000000000000 < dwServerTime))
	//{
	//	if (!m_pMessagePanel->IsVisible())
	//	{
	//		m_pMessagePanel->SetMessage(g_pMessageStringTable[13], 4000u);
	//		m_pMessagePanel->SetVisible(1, 1);
	//	}

	//	if (m_eSceneType != ESCENE_TYPE::ESCENE_LOGIN)
	//		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);

	//	m_dwDelayDisconnectTime = 0;
	//	return 1;
	//}

	if (!pStd)
	{
		// During a channel migration the native client defers packet 0x52A until
		// the socket-close callback, but only inside its 15-second retry window.
		if (m_dwDelayDisconnectTime != 0
			&& m_dwDelayDisconnectTime + 15000U >= dwServerTime
			&& m_bMsgRemoveServer == 1)
		{
			if (m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
			{
				auto pFieldScene = static_cast<TMFieldScene*>(this);
				pFieldScene->OnPacketEvent(MSG_CNFRemoveServer_Opcode, reinterpret_cast<char*>(&pFieldScene->m_stRemoveServer));
			}

			// Native 7.48 consumes the pending migration in every scene, preventing
			// an obsolete ticket from being replayed after a later return to Field.
			m_dwDelayDisconnectTime = 0;
			m_bMsgRemoveServer = 0;
			return 1;
		}

		if (m_pMessagePanel && !m_pMessagePanel->IsVisible())
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[13], 4000u);
			m_pMessagePanel->SetVisible(1, 1);
		}

		if (m_eSceneType != ESCENE_TYPE::ESCENE_LOGIN)
			g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);

		m_dwDelayDisconnectTime = 0;
		return 1;
	}

	if (pStd->Size < sizeof(MSG_STANDARD))
		return 1;

	if (pStd->Type == MSG_BillingNotice_Opcode)
	{
		if (pStd->Size != sizeof(MSG_BillingNotice))
			return 1;

		g_pObjectManager->m_bBilling = 1;
		if (m_pMessageBox)
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[132], B_CREATE_ID, nullptr);
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}

	if (pStd->Type == MSG_Encode_Opcode)
	{
		if (pStd->Size != sizeof(MSG_Encode))
			return 1;

		const MSG_Encode* pEncode = reinterpret_cast<const MSG_Encode*>(pStd);

		const int EncodeByte1 = pEncode->Parm[40];
		const int EncodeByte2 = pEncode->Parm[41];

		int type = (3 * (EncodeByte1 / 7) + 7 * (EncodeByte2 / 3) + 220) % 3;
		if (type)
		{
			if (type == 1)
				pStd->Type = 0x13BD;
			else if (type == 2)
				pStd->Type = 0x7BE;
		}
		else
			pStd->Type = 0xFBC;
		return 1;
	}

	if (pStd->Type == 0xFBC)
	{
		// need to decompile here... and other encode packets
		return 1;
	}

	if (pStd->Type == 0x13BD)
	{
		// need to decompile here... and other encode packets
		return 1;
	}

	if (pStd->Type == 0x7BE)
	{
		// need to decompile here... and other encode packets
		return 1;
	}
	if (!pStd->ID && (pStd->Type == MSG_MessagePanel_Opcode ||
		pStd->Type == MSG_LegacySceneMessage102_Opcode || pStd->Type == MSG_LegacySceneMessage104_Opcode ||
		pStd->Type == MSG_MessageIndexed_Opcode || pStd->Type == MSG_MessageParameterized_Opcode))
	{
		return OnMessagePanelPacket(pStd);
	}
	if (pStd->Type == MSG_MessageShout_Opcode)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnShoutMessage(pStd, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (pStd->Type == MSG_ChinaPlaytime_Opcode)
	{
		if (pStd->Size != sizeof(MSG_STANDARDPARM))
			return 1;

		MSG_STANDARDPARM* m = reinterpret_cast<MSG_STANDARDPARM*>(pStd);

		g_pApp->china_bWrite = 1;
		g_pApp->china_Playtime = m->Parm / 60;
		LOG_WRITELOG("get start Time : %d \r\n", g_pApp->china_Playtime);
		return 1;
	}

	if (m_pControlContainer && m_pControlContainer->OnPacketEvent(dwCode, pSBuffer) == 1)
		return 1;

	TreeNode::OnPacketEvent(dwCode, pSBuffer);
	return 0;
}

// Extracted from TMScene::OnPacketEvent; behavior is unchanged.
int TMScene::OnMessagePanelPacket(MSG_STANDARD*& pStd)
{
	char szStr[128] = { 0 };
	if (pStd->Type != MSG_MessagePanel_Opcode)
	{
		return OnIndexedSceneMessage(pStd);
	}

	if (pStd->Size != sizeof(MSG_MessagePanel))
		return 1;

	auto pMsgPanel = reinterpret_cast<MSG_MessagePanel*>(pStd);

	// The 7.48 message-panel payload is 96 bytes (108 including Header).
	// The imported newer client used a 128-byte body and wrote past the
	// canonical packet, corrupting the next receive-buffer frame.
	pMsgPanel->String[sizeof(pMsgPanel->String) - 1] = 0;

	if (m_eSceneType == ESCENE_TYPE::ESCENE_SELCHAR && pMsgPanel->String[0] == '^')
	{
		char szMsg[128]{ 0 };
		sprintf_s(szMsg, "%s", &pMsgPanel->String[1]);

		m_pMessageBox2->SetMessage(szMsg, 0, nullptr);
		m_pMessageBox2->SetVisible(1);
	}
	else if (pMsgPanel->String[0] == '^')
	{
		char szMsg[128]{ 0 };
		sprintf_s(szMsg, "%s", &pMsgPanel->String[1]);

		m_pMessagePanel->SetMessage(szMsg, 7000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	else if (m_eSceneType == ESCENE_TYPE::ESCENE_FIELD &&
		pMsgPanel->String[0] == '!' && pMsgPanel->String[1] == '#')
	{
		for (int i = 2; i <= 6; ++i)
		{
			if (pMsgPanel->String[i] < '0' || pMsgPanel->String[i] > '9')
				pMsgPanel->String[i] = '0';
		}

		TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
		pFScene->m_nYear = static_cast<unsigned short>(pMsgPanel->String[3] - 48)
			+ 10 * static_cast<unsigned short>(pMsgPanel->String[2] - 48);
		pFScene->m_nDays = static_cast<unsigned short>(pMsgPanel->String[6] - 48)
			+ 10 * static_cast<unsigned short>(pMsgPanel->String[5] - 48)
			+ 100 * static_cast<unsigned short>(pMsgPanel->String[4] - 48);

		return 1;
	}
	else if (pMsgPanel->String[1] == '!' && pMsgPanel->String[2] == '!' && pMsgPanel->String[3] == '!')
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = ShowFireworkMessage(pMsgPanel, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	else if (pMsgPanel->String[1] == '!' && pMsgPanel->String[2] == '!' && pMsgPanel->String[3] == '#')
	{
		if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && pMsgPanel->String[4] == 'E')
		{
			auto pKilled = static_cast<TMHuman*>(g_pObjectManager->GetHumanByID(static_cast<TMFieldScene*>(g_pCurrentScene)->m_dwKhepraID));

			if (pKilled)
			{
				pKilled->m_stScore.CurHP = 0;
				pKilled->Die();
			}

			static_cast<TMFieldScene*>(g_pCurrentScene)->m_dwKhepraID = 0;

			for (int iSt = 0; iSt < 5; ++iSt)
				pMsgPanel->String[iSt] = '_';
		}
	}
	else if (pMsgPanel->String[0] == '!' || pMsgPanel->String[1] == '!')
	{
		if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
		{
			for (int n = 2; n < 8; ++n)
			{
				if (pMsgPanel->String[n] < '0' || pMsgPanel->String[n] > '9')
					pMsgPanel->String[n] = '0';
			}

			TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
			pFScene->m_NightmareTime.wHour = pMsgPanel->String[3] - 48 + 10 * (pMsgPanel->String[2] - 48);
			pFScene->m_NightmareTime.wMonth = pMsgPanel->String[5] - 48 + 10 * (pMsgPanel->String[4] - 48);
			pFScene->m_NightmareTime.wSecond = pMsgPanel->String[7] - 48 + 10 * (pMsgPanel->String[6] - 48);
			pFScene->m_dwLastNightmareTime = g_pTimerManager->GetServerTime();
			return 1;
		}
	}
	else if (pMsgPanel->String[0] == '!')
	{
		ShowSmsMessage(pMsgPanel);
	}
	else if (pMsgPanel->String[0] == '2' && pMsgPanel->String[1] == '0' && pMsgPanel->String[2] == '0')
	{
		char Msg[128]{ 0 };
		if (m_pMyHuman)
			sprintf_s(Msg, "%s %d %d %d", pMsgPanel->String, g_pObjectManager->m_nServerIndex, static_cast<int>(m_pMyHuman->m_vecPosition.x),
				static_cast<int>(m_pMyHuman->m_vecPosition.y));
		else
			sprintf_s(Msg, "%s %d", pMsgPanel->String, g_pObjectManager->m_nServerIndex);

		m_pMessagePanel->SetMessage(Msg, 4000u);
		m_pMessagePanel->SetVisible(1, 1);
	}
	else
	{
		m_pMessagePanel->SetMessage(pMsgPanel->String, 4000u);
		m_pMessagePanel->SetVisible(1, 1);
	}

	AppendMessagePanelToChat(pMsgPanel);
	SControl* pLoginOK = m_pControlContainer
		? m_pControlContainer->FindControl(65873)
		: nullptr;
	if (pLoginOK)
		pLoginOK->SetEnable(1);
	return 1;

}

// Extracted from TMScene::OnMessagePanelPacket; behavior is unchanged.
int TMScene::OnIndexedSceneMessage(MSG_STANDARD*& pStd)
{
	// Native FUN_0055890A admits only these exact frame sizes, while
	// FUN_0049889A consumes both opcodes without reading their payload.
	// Silently consume malformed variants as well so they cannot fall
	// through to unrelated controls in this rebuilt dispatcher.
	if ((pStd->Type == MSG_LegacySceneMessage102_Opcode &&
		pStd->Size != sizeof(MSG_LegacySceneMessage102)) ||
		(pStd->Type == MSG_LegacySceneMessage104_Opcode &&
			pStd->Size != sizeof(MSG_LegacySceneMessage104)))
	{
		return 1;
	}

	if (pStd->Type == MSG_MessageIndexed_Opcode || pStd->Type == MSG_MessageParameterized_Opcode)
	{
		const MSG_MessageChat* pMessageChat = nullptr;
		std::int16_t relativeIndex = 0;
		int tableIndex = 0;
		if (DecodeIndexedMessage(pStd, pMessageChat, relativeIndex, tableIndex) && m_pMessagePanel)
		{
			char messageTemplate[kIndexedMessageCapacity]{};
			char messageText[kIndexedMessageCapacity]{};
			if (!CopyIndexedTemplate(tableIndex, messageTemplate, sizeof(messageTemplate)))
				CopyRelativeIndexFallback(relativeIndex, messageText, sizeof(messageText));
			else if (pStd->Type == MSG_MessageParameterized_Opcode)
			{
				char parameters[kIndexedParameterCount][kIndexedParameterCapacity]{};
				const size_t parameterCount = ParseIndexedParameters(pMessageChat, parameters);
				FormatIndexedTemplate(messageTemplate, parameters, parameterCount, messageText, sizeof(messageText));
			}
			else
			{
				std::memcpy(messageText, messageTemplate, sizeof(messageText));
				messageText[sizeof(messageText) - 1] = '\0';
			}

			m_pMessagePanel->SetMessage(messageText, IndexedMessageDuration(tableIndex));
			m_pMessagePanel->SetVisible(1, 1);
		}
	}

	return 1;

}

// Extracted from TMScene::OnMessagePanelPacket; behavior is unchanged.
ExtractedFlow TMScene::ShowFireworkMessage(MSG_MessagePanel*& pMsgPanel, int& extractedResult)
{
	if (m_eSceneType != ESCENE_TYPE::ESCENE_FIELD || !m_pMyHuman)
		{ extractedResult = 1; return ExtractedFlow::Return; }

	auto pFocusedObject = static_cast<TMObject*>(m_pMyHuman);

	if ((int)pFocusedObject->m_vecPosition.x >> 7 == 31 && (int)pFocusedObject->m_vecPosition.y >> 7 == 31)
	{
		for (int nType = 0; nType < 5; ++nType)
		{
			for (int j = 0; j < 5; ++j)
			{
				auto pFireWork = new TMEffectFireWork({ (pFocusedObject->m_vecPosition.x) - 10.0f + (5.0f * nType), 7.0f, ((pFocusedObject->m_vecPosition.y - 10.0f) + 5.0f * j) + 8.0f }, nType);

				g_pCurrentScene->AddChild(pFireWork);
			}
		}
	}

	if (pFocusedObject->IsInTown() == 1 || (int)pFocusedObject->m_vecPosition.x >> 7 != 31 || (int)pFocusedObject->m_vecPosition.y >> 7 != 31)
	{
		m_pMessagePanel->SetMessage(pMsgPanel->String, 4000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	return ExtractedFlow::Next;
}

// Extracted from TMScene::OnMessagePanelPacket; behavior is unchanged.
void TMScene::ShowSmsMessage(MSG_MessagePanel*& pMsgPanel)
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
	{
		auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

		auto pItem3 = new SListBoxItem(" ", 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);
		pFScene->m_pHelpList[3]->AddItem(pItem3);

		char szTime[128]{ 0 };
		char _Buffer[128]{ 0 };

		sprintf_s(szTime, "SMS [%02d:%02d:%02d]", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		auto pItem = new SListBoxItem(szTime, 0xFFBBFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);
		pFScene->m_pHelpList[3]->AddItem(pItem);

		sprintf_s(_Buffer, "%s", &pMsgPanel->String[1]);

		auto pItem2 = new SListBoxItem(_Buffer, 0xFFBBFFCC, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);
		pFScene->m_pHelpList[3]->AddItem(pItem2);

		if (pFScene->m_pHelpMemo)
			pFScene->m_pHelpMemo->SetVisible(1);
	}
	else
	{
		for (int l = 0; l < 98; ++l)//
		{
			if (!g_pObjectManager->m_stMemo[l].szString[0] && !g_pObjectManager->m_stMemo[l + 1].szString[0])
			{
				g_pObjectManager->m_stMemo[l].dwColor = -1;
				sprintf_s(g_pObjectManager->m_stMemo[l].szString, "");
				g_pObjectManager->m_stMemo[l + 1].dwColor = 0xFFBBFFFF;
				sprintf_s(g_pObjectManager->m_stMemo[l + 1].szString, "SMS [%02d:%02d:%02d]", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

				g_pObjectManager->m_stMemo[l + 2].dwColor = 0xFFBBFFCC;
				sprintf_s(g_pObjectManager->m_stMemo[l + 2].szString, "%s", &pMsgPanel->String[1]);
			}
		}
	}

	m_pMessagePanel->SetMessage(&pMsgPanel->String[1], 4000u);
	m_pMessagePanel->SetVisible(1, 1);

}

// Extracted from TMScene::OnMessagePanelPacket; behavior is unchanged.
void TMScene::AppendMessagePanelToChat(MSG_MessagePanel*& pMsgPanel)
{
	SListBox* pChatList = m_pControlContainer
		? static_cast<SListBox*>(m_pControlContainer->FindControl(65667))
		: nullptr;

	int len = strlen(pMsgPanel->String);
	int size = 50;
	if (len > size)
	{
		char szMsg2[128]{};
		char szMsg3[128]{};

		if (IsClearString(pMsgPanel->String, size - 1))
		{
			strncpy(szMsg3, pMsgPanel->String, size);
			sprintf_s(szMsg2, "%s", &pMsgPanel->String[size]);
		}
		else
		{
			strncpy(szMsg3, pMsgPanel->String, size - 1);
			sprintf_s(szMsg2, "%s", &pMsgPanel->String[size - 1]);
		}

		SListBoxItem* ipNewItem = new SListBoxItem(szMsg3, 0xFFCCAAFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);

		SListBoxItem* ipNewItem2 = new SListBoxItem(szMsg2, 0xFFCCAAFF, 0.0f, 0.0f, 280.0f, 16.0, 0, 0x77777777, 1u, 0);
		if (ipNewItem2 && pChatList)
			pChatList->AddItem(ipNewItem2);
	}
	else
	{
		if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
		{
			TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
			if (!strcmp(pMsgPanel->String, "Whisper : Off"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[448], g_pMessageStringTable[447]);
				pFScene->SetWhisper(0);
			}
			if (!strcmp(pMsgPanel->String, "Whisper : On"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[448], g_pMessageStringTable[446]);
				pFScene->SetWhisper(1);
			}
			if (!strcmp(pMsgPanel->String, "Citizen Chatting : Off"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[449], g_pMessageStringTable[447]);
				pFScene->SetPartyChat(0);
			}
			if (!strcmp(pMsgPanel->String, "Citizen Chatting : On"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[449], g_pMessageStringTable[446]);
				pFScene->SetPartyChat(1);
			}
			if (!strcmp(pMsgPanel->String, "Guild Chatting : Off"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[450], g_pMessageStringTable[447]);
				pFScene->SetGuildChat(0);
			}
			if (!strcmp(pMsgPanel->String, "Guild Chatting : On"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[450], g_pMessageStringTable[446]);
				pFScene->SetGuildChat(1);
			}
			if (!strcmp(pMsgPanel->String, "Kingdom Chatting : On"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[451], g_pMessageStringTable[446]);
				pFScene->SetKingDomChat(1);
			}
			if (!strcmp(pMsgPanel->String, "Kingdom Chatting : Off"))
			{
				sprintf_s(pMsgPanel->String, "%s%s", g_pMessageStringTable[451], g_pMessageStringTable[447]);
				pFScene->SetKingDomChat(0);
			}
		}

		SListBoxItem* ipNewItem = new SListBoxItem(pMsgPanel->String, 0xFFCCAAFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);
	}
}



// Extracted from TMScene::OnPacketEvent; behavior is unchanged.
ExtractedFlow TMScene::OnShoutMessage(MSG_STANDARD*& pStd, int& extractedResult)
{
	if (pStd->Size != sizeof(MSG_MessageWhisper))
		{ extractedResult = 1; return ExtractedFlow::Return; }

	MSG_MessageWhisper* pShoutMessage = reinterpret_cast<MSG_MessageWhisper*>(pStd);
	pShoutMessage->MobName[sizeof(pShoutMessage->MobName) - 1] = 0;
	pShoutMessage->String[sizeof(pShoutMessage->String) - 1] = 0;

	if (GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
		{ extractedResult = 1; return ExtractedFlow::Return; }

	TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (!pFScene->m_pChatGeneral || !pFScene->m_pCHP || pFScene->m_pCHP->m_bSelectEnable)
	{
		int nIndex = 0;
		unsigned int dwColor = 0xFF00CD00;

		SListBox* pChatList = pFScene->m_pChatList;

		int nLen = strlen(pShoutMessage->MobName);

		SListBoxItem* ipNewItem = new SListBoxItem(pShoutMessage->MobName,
			dwColor,
			0.0f,
			0.0f,
			(float)nLen * 6.6999998f,
			16.0f,
			0,
			0xFFFFFF00,
			1u,
			0);

		ipNewItem->m_bBGColor = 0;
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);

		pFScene->m_dwChatTime = g_pTimerManager->GetServerTime();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	else
		{ extractedResult = 1; return ExtractedFlow::Return; }
	return ExtractedFlow::Next;
}


int TMScene::OnKeyDownEvent(unsigned int iKeyCode)
{
	if (g_pCurrentScene != this)
		return 0;

	if (m_pControlContainer && m_pControlContainer->OnKeyDownEvent(iKeyCode) == 1)
		return 1;

	TreeNode::OnKeyDownEvent(iKeyCode);
	return 0;
}

int TMScene::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (g_pCurrentScene != this)
		return 0;

	if (m_pControlContainer && m_pControlContainer->OnMouseEvent(dwFlags, wParam, nX, nY) == 1)
		return 1;

	TreeNode::OnMouseEvent(dwFlags, wParam, nX, nY);
	return 0;
}

int TMScene::OnKeyUpEvent(unsigned int iKeyCode)
{
	if (g_pCurrentScene != this)
		return 0;

	if (m_pControlContainer && m_pControlContainer->OnKeyUpEvent(iKeyCode) == 1)
		return 1;

	TreeNode::OnKeyUpEvent(iKeyCode);
	return 0;
}

int TMScene::OnCharEvent(char iCharCode, int lParam)
{
	if (g_pCurrentScene != this)
		return 0;

	if (m_pControlContainer && m_pControlContainer->OnCharEvent(iCharCode, lParam) == 1)
		return 1;
	
	return TreeNode::OnCharEvent(iCharCode, lParam);
}

int TMScene::OnIMEEvent(char* ipComposeString)
{
	if (g_pCurrentScene != this)
		return 0;

	if (m_pControlContainer && m_pControlContainer->OnIMEEvent(ipComposeString) == 1)
		return 1;
	
	return 0;
}

int TMScene::OnChangeIME()
{
	return m_pControlContainer && m_pControlContainer->OnChangeIME() == 1;
}

int TMScene::OnAccel(int nMsg)
{
	return 1;
}

int TMScene::FrameMove(unsigned int dwServerTime)
{
	if (g_pCurrentScene != this)
		return 1;

	if (m_pControlContainer)
		m_pControlContainer->FrameMove(dwServerTime);

	if (m_sPlayDemo >= 0)
		CameraAction();

	if (m_bCriticalError == 1)
		return 1;

	m_pMouseOverHuman = nullptr;
	m_pMouseOverItem = nullptr;

	auto pFocusedObject = static_cast<TMObject*>(m_pMyHuman);

	if (pFocusedObject && m_pGround)
	{
		float dX = pFocusedObject->m_vecPosition.x - m_pGround->m_vecOffset.x;
		float dY = pFocusedObject->m_vecPosition.y - m_pGround->m_vecOffset.y;

		int nShold = 14;

		if (((int)pFocusedObject->m_vecPosition.x >> 7) > 26 &&
			((int)pFocusedObject->m_vecPosition.x >> 7) < 31 &&
			((int)pFocusedObject->m_vecPosition.y >> 7) > 20 &&
			((int)pFocusedObject->m_vecPosition.y >> 7) < 25)
		{
			nShold = 20;
		}

		if (m_pGround->m_vecOffsetIndex.x == 17 && m_pGround->m_vecOffsetIndex.y == 10)
			nShold = 20;

		if (dX >= 0.0f && (float)nShold > dX)
		{
			if (!m_pGround->m_pLeftGround)
				GroundNewAttach(EDirection::EDIR_LEFT);
		}
		else if (dX < 0.0f && dX > (float)-nShold)
		{
			if (m_pGround->m_pLeftGround)
			{
				m_pGround = m_pGround->m_pLeftGround;

				m_nCurrentGroundIndex = (m_nCurrentGroundIndex + 1) % 2;
				
				m_pGround->SetMiniMapData();
			}
		}
		else if (dX > (float)(128 - nShold) && dX < 128.0f)
		{
			if (!m_pGround->m_pRightGround)
				GroundNewAttach(EDirection::EDIR_RIGHT);
		}
		else if (dX >= 128.0f && (float)(nShold + 128) > dX && m_pGround->m_pRightGround)
		{
			m_pGround = m_pGround->m_pRightGround;

			m_nCurrentGroundIndex = (m_nCurrentGroundIndex + 1) % 2;

			m_pGround->SetMiniMapData();
		}

		if (dY >= 0.0f && (float)nShold > dY)
		{
			if (!m_pGround->m_pUpGround)
				GroundNewAttach(EDirection::EDIR_UP);
		}
		else if (dY < 0.0f && dY > (float)-nShold)
		{
			if (m_pGround->m_pUpGround)
			{
				m_pGround = m_pGround->m_pUpGround;

				m_nCurrentGroundIndex = (m_nCurrentGroundIndex + 1) % 2;

				m_pGround->SetMiniMapData();
			}
		}
		else if (dY > (float)(128 - nShold) && (float)128 > dY)
		{
			if (!m_pGround->m_pDownGround)
				GroundNewAttach(EDirection::EDIR_DOWN);
		}
		else if (dY >= (float)128 && (float)(nShold + 128) > dY && m_pGround->m_pDownGround)
		{
			m_pGround = m_pGround->m_pDownGround;

			m_nCurrentGroundIndex = (m_nCurrentGroundIndex + 1) % 2;

			m_pGround->SetMiniMapData();
		}
	}

	return 1;
}

int TMScene::ReloadScene()
{
	if (m_pControlContainer != nullptr)
	{
		delete m_pControlContainer;

		m_pControlContainer = nullptr;
	}

	return InitializeScene();
}

ESCENE_TYPE TMScene::GetSceneType()
{
	return m_eSceneType;
}

void TMScene::Cleanup()
{
	;
}

void TMScene::CameraAction()
{
	if (m_dwStartCamTime == 0)
		return;

	DWORD dwTick = (g_pTimerManager->GetServerTime() - m_dwStartCamTime) + 100;

	TMCamera* pCamera = g_pObjectManager->m_pCamera;

	pCamera->m_fSightLength = 11.0f;

	for (int i = 0; i < (m_nCameraLoop - 1); ++i)
	{
		if (m_stCameraTick[i].dwTick >= dwTick || m_stCameraTick[i + 1].dwTick <= dwTick)
			continue;

		float fRatio =
			(float)(dwTick - m_stCameraTick[i].dwTick) /
			(float)(m_stCameraTick[i + 1].dwTick - m_stCameraTick[i].dwTick);

		auto fHorizonAngle = ((1.0f - fRatio) * m_stCameraTick[i].fHorizonAngle) + (m_stCameraTick[i + 1].fHorizonAngle * fRatio);

		pCamera->m_fHorizonAngle = fHorizonAngle;
		pCamera->m_fBackHorizonAngle = fHorizonAngle;

		auto fVerticalAngle = ((1.0f - fRatio) * m_stCameraTick[i].fVerticalAngle) + (m_stCameraTick[i + 1].fVerticalAngle * fRatio);

		pCamera->m_fVerticalAngle = fVerticalAngle;
		pCamera->m_fBackVerticalAngle = fVerticalAngle;

		TMVector3 vecLoc1{ 0.0f, 0.0f, 0.0f };
		TMVector3 vecLoc2{ 0.0f, 0.0f, 0.0f };

		if (m_stCameraTick[i].sLocal == 1 && m_pMyHuman != nullptr)
		{
			vecLoc1.x = m_pMyHuman->m_vecPosition.x;
			vecLoc1.y = m_pMyHuman->m_fHeight;
			vecLoc1.z = m_pMyHuman->m_vecPosition.y;
		}

		if (m_stCameraTick[i + 1].sLocal == 1 && m_pMyHuman != nullptr)
		{
			vecLoc2.x = m_pMyHuman->m_vecPosition.x;
			vecLoc2.y = m_pMyHuman->m_fHeight;
			vecLoc2.z = m_pMyHuman->m_vecPosition.y;
		}

		pCamera->m_cameraPos.x = ((vecLoc1.x + m_stCameraTick[i].fX) * (1.0f - fRatio))
			+ ((vecLoc2.x + m_stCameraTick[i + 1].fX) * fRatio);

		pCamera->m_cameraPos.y = ((vecLoc1.y + m_stCameraTick[i].fY) * (1.0f - fRatio))
			+ ((vecLoc2.y + m_stCameraTick[i + 1].fY) * fRatio);

		pCamera->m_cameraPos.z = ((vecLoc1.z + m_stCameraTick[i].fZ) * (1.0f - fRatio))
			+ ((vecLoc2.z + m_stCameraTick[i + 1].fZ) * fRatio);

		return;
	}

	if (m_stCameraTick[m_nCameraLoop - 1].dwTick < dwTick)
	{
		pCamera->m_fHorizonAngle = m_stCameraTick[m_nCameraLoop - 1].fHorizonAngle;
		pCamera->m_fVerticalAngle = m_stCameraTick[m_nCameraLoop - 1].fVerticalAngle;

		TMVector3 vecLoc3{ 0.0f, 0.0f, 0.0f };

		if (m_pMyHuman && m_stCameraTick[m_nCameraLoop - 1].sLocal == 1)
		{
			vecLoc3.x = m_pMyHuman->m_vecPosition.x;
			vecLoc3.y = m_pMyHuman->m_fHeight;
			vecLoc3.z = m_pMyHuman->m_vecPosition.y;
		}

		pCamera->m_cameraPos.x = m_stCameraTick[m_nCameraLoop - 1].fX + vecLoc3.x;
		pCamera->m_cameraPos.y = m_stCameraTick[m_nCameraLoop - 1].fY + vecLoc3.y;
		pCamera->m_cameraPos.z = m_stCameraTick[m_nCameraLoop - 1].fZ + vecLoc3.z;
	}
}

void TMScene::ReadCameraPos(const char* szFileName)
{
	m_nCameraLoop = 0;

	memset(m_stCameraTick, 0, sizeof(m_stCameraTick));

	char szBinFileName[128]{};

	sprintf_s(szBinFileName, "%s.bin", szFileName);

	FILE* fpBin = nullptr;

	fopen_s(&fpBin, szBinFileName, "rb");

	fread(&m_nCameraLoop, 1, 4, fpBin);

	if (m_nCameraLoop > 1000)
		m_nCameraLoop = 1000;

	else if (m_nCameraLoop < 0)
		m_nCameraLoop = 0;

	fread(m_stCameraTick, 1, 28 * m_nCameraLoop, fpBin);
	fclose(fpBin);
}

void TMScene::CheckPKNonePK(int nServerIndex)
{
	g_NonePKServer = 1;
	for (int i = 0; i < 2; ++i)
	{
		if (nServerIndex == g_pPKServerNum[i])
			g_NonePKServer = 0;
	}
	g_NonePKServer = 0;
}

void TMScene::LogMsgCriticalError(int Type, int ID, int nMesh, int X, int Y)
{
	if (!m_pMyHuman || !g_pSocketManager)
		return;

	MSG_MessageLog stLog{};
	stLog.Header.ID = m_pMyHuman->m_dwID;
	stLog.Header.Type = MSG_MessageLog_Opcode;

	if(Type == 10)
		sprintf_s(stLog.String, "00000000 , Load Tile Map Fail");
	else
		sprintf_s(stLog.String, "%08d , Critical Data Err Cl,%d,%d,%d,%d,%d, %d", ID,	nMesh, (int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y,
			X,
			Y,
			Type);

	g_pSocketManager->SendPacket({
		stLog.Header.Type,
		reinterpret_cast<char*>(&stLog),
		sizeof(stLog)
	});
}

void TMScene::DeleteOwnerAllContainer()
{
	m_pEffectContainer->DeleteOwner(nullptr);
	m_pShadeContainer->DeleteOwner(nullptr);
	m_pHumanContainer->DeleteOwner(nullptr);
	m_pItemContainer->DeleteOwner(nullptr);
	m_pExtraContainer->DeleteOwner(nullptr);
}
