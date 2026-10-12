#include "pch.h"
#include "WeatherPacket.h"
#include "TMFieldScene.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "ItemEffect.h"
#include "WYD748Assets.h"
#include "DirShow.h"
#include "FieldSceneInventorySupport.h"
#include "FieldSceneTradeSupport.h"
#include "FieldSceneWorldSupport.h"
#include "../../application/FieldInteractionPolicy.h"
#include "../../application/CCModePolicy.h"
#include "../../application/FieldChatSubmitPolicy.h"
#include "../../application/FieldCoinInputPolicy.h"
#include "../../ui/FieldChatControl.h"
#include "../../core/NativeSalePrice.h"
#include "TMLog.h"
#include "dsutil.h"
#include "features/macro/MacroMsg.h"
#include "ClientDiagnostics.h"
#include "TMGround.h"
#include "TMHuman.h"
#include "../../game/entities/DeathMotionPolicy.h"
#include "TMObjectContainer.h"
#include "TMCamera.h"
#include "ServerEndpoint.h"
#include "ServerStatus.h"
#include "ServerChannelLabel.h"
#include "TMSun.h"
#include "TMSky.h"
#include "TMSnow.h"
#include "TMRain.h"
#include "TMSkinMesh.h"
#include "TMEffectSWSwing.h"
#include "TMEffectMesh.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectBillBoard.h"
#include "TMHouse.h"
#include "TMSkillMagicArrow.h"
#include "TMSkillTownPortal.h"
#include "TMEffectLevelUp.h"

// TMFieldScene remains the orchestration boundary for the live field scene.
// Domain members can use private state from independent translation units
// without changing the class layout. Dispatch and lifecycle ordering remain
// here; complete domain definitions move without changing their behavior.
namespace
{
	// Compile-time guards preserve native AutoTrade slot mapping.

	static_assert(field_interaction::AutoTradeFirstListingControl == TMG_ATRADE_MY1);
	static_assert(field_interaction::AutoTradeSlotIndex(TMG_ATRADE_MY12) == 11);

	void WYD748_AddOwnedGridItem(SGridControl* grid, STRUCT_ITEM* item)
	{
		if (!grid)
		{
			SAFE_DELETE(item);
			return;
		}
		auto controlItem = new SGridControlItem(nullptr, item, 0.0f, 0.0f);
		if (controlItem)
		{
			if (!grid->AddItem(controlItem, 0, 0))
				SAFE_DELETE(controlItem);
		}
		else
			SAFE_DELETE(item);
	}

}

RECT TMFieldScene::m_rectWarning[7] =
{
  { 2255, 1535, 2263, 1538 },
  { 150, 3788, 156, 3793 },
  { 150, 3711, 156, 3772 },
  { 154, 3773, 157, 3780 },
  { 1075, 1708, 1078, 1713 },
  { 1171, 4076, 1177, 4079 },
  { 2362, 4041, 2369, 4044 }
};

int TMFieldScene::m_bPK = 0;
unsigned short TMFieldScene::m_usProgress = 0;
unsigned int TMFieldScene::m_dwCargoID = 0;

TMFieldScene::TMFieldScene()
	: TMScene()
{
	g_nTempArray[1] = (int)g_pObjectManager->m_stMobData.MobName;
	g_nTempArray2[1] = (int)g_pObjectManager->m_stMobData.MobName;

	LOG_WRITELOG(">> New Field Scene\r\n");
	m_eSceneType = ESCENE_TYPE::ESCENE_FIELD;
	m_dwID = static_cast<unsigned int>(m_eSceneType);
	// Start in the full source path; InitializeScene switches this flag only
	// when the deployed 7.48 resource lacks the newer HUD controls.
	// FieldScene2.bin exposes only the native 7.48 cargo page; keep the
	// imported second-page pointer deterministic when that 7.59 control is absent.
	m_pCargoPanel1 = nullptr;

	g_bLastStop = 0;
	g_bCastleWar = 0;
	g_bCastleWar2 = 0;

	m_dwLastGuildNameCheckTime = 0;
	m_dwLastWolfSound = 0;
	m_nMySanc = 0;
	m_cResurrect = 0;
	m_dwDustTerm = 2000;
	m_cLastFlagLButtonUp = 1;
	m_dwInTownTime = 0;
	m_dwFieldTime = 0;
	m_dwLastMouseDownTime = 0;
	m_dwLastPappusTime = 0;
	m_dwLastDustTime = 0;
	m_dwLastRemain = 0;
	m_dwLastLogout = 0;
	m_dwLastSelServer = 0;
	m_dwLastTeleport = 0;
	m_dwLastTown = 0;
	m_dwLastResurrect = 0;
	m_dwLastRelo = 0;
	m_dwStartFlashTime = 0;
	m_dwOldAttackTime = 0;
	m_dwWeatherTime = 0;
	m_dwLastDeadTime = 0;
	m_nWTime = 0;
	m_dwKeyTime = 0;
	m_dwPGTTime = 0;
	m_dwGetItemTime = 0;
	m_dwUseItemTime = 0;
	m_dwNPCClickTime = 0;
	m_dwLastCheckTime = 0;
	m_dwLastSetTargetHuman = 0;
	m_dwRemainTime = 0;
	m_dwQuizStart = 0;
	m_nChatListSize = 0;
	m_nReqHP = 0;
	m_nReqMP = 0;
	m_nAutoSkillNum = 4;
	m_nLastAutoTradePos = -1;
	m_sDestType = -1;
	m_sDestPos = -1;
	m_dwLastMousePosTime = 0;
	m_nLastMousePosX = 0;
	m_nLastMousePosY = 0;
	m_bSandWind = 1;
	m_cLastTown = 0;
	m_cLastTeleport = 0;
	m_cLastRelo = 0;
	m_cLastWhisper = 0;
	m_cGuildOnOff = 1;
	m_bMoveing = 0;
	m_nDustCount = 0;
	m_pSysMsgList = 0;
	m_dwLastNightmareTime = 0;
	m_dwNightmareTime = 0;

	g_GameAuto = 0;

	m_dwAttackDelay = 0;
	m_dwLastWhisper = 0;

	memset(&m_stLastWhisper, 0, sizeof(m_stLastWhisper));
	memset(m_dwStartAffectTime, 0, sizeof(m_dwStartAffectTime));
	memset(&m_stAutoTrade, 0, sizeof(m_stAutoTrade));
	memset(&m_stUseItem, 0, sizeof(m_stUseItem));
	memset(&m_stCapsuleItem, 0, sizeof(m_stCapsuleItem));
	memset(&m_stPotalItem, 0, sizeof(m_stPotalItem));
	memset(m_szSummoner, 0, sizeof(m_szSummoner));
	memset(m_szSummoner2, 0, sizeof(m_szSummoner2));
	memset(m_pLottoNumber, 0, sizeof(m_pLottoNumber));

	m_bNumPad = 0;
	m_bMountDead = 0;
	m_pTarget1 = nullptr;
	m_pTarget2 = nullptr;
	m_pTargetBill = nullptr;
	m_pHelpMemo = nullptr;
	m_pHelpSummon = nullptr;
	m_pMyHuman = nullptr;
	m_pTargetItem = nullptr;
	m_pTargetHuman = nullptr;
	m_pAutoSkillPanel = nullptr;
	// The 7.48 FieldScene2 resource does not expose the newer PK button.
	// Keep this optional binding deterministic; SetPK still sends the native
	// mode packet when no visual button is materialized.
	m_PkButton = nullptr;
	m_pPGTOver = nullptr;
	m_pTradePanel = nullptr;
	m_pLottoPanel = nullptr;
	m_pLottoCost = nullptr;
	m_pHellgateStore = nullptr;
	m_pHellStoreDesc = nullptr;
	m_pGridHellStore = nullptr;
	m_pGambleStore = nullptr;
	m_pReelPanel = nullptr;
	m_pReelPanel2 = nullptr;
	m_cGambleType = 0;
	m_cPendingGambleType = 0;
	m_dwGambleRequestTime = 0;
	m_pQuestPanel = nullptr;
	m_pQuestQuitBtn = nullptr;
	m_pQuestMemo = nullptr;
	m_pCargoGrid = nullptr;
	// Compatibility mode creates the legacy inventory grid at runtime.  Keep
	// every grid pointer deterministic because the 7.59 initializer that would
	// normally assign them is intentionally skipped for FieldScene2.bin 7.48.
	m_pGridInv = nullptr;
	memset(m_pGridInvList, 0, sizeof(m_pGridInvList));
	m_pGridHelm = nullptr;
	m_pGridCoat = nullptr;
	m_pGridPants = nullptr;
	m_pGridGloves = nullptr;
	m_pGridBoots = nullptr;
	m_pGridRight = nullptr;
	m_pGridLeft = nullptr;
	m_pGridGuild = nullptr;
	m_pGridEvent = nullptr;
	m_pGridRing = nullptr;
	m_pGridNecklace = nullptr;
	m_pGridOrb = nullptr;
	m_pGridCabuncle = nullptr;
	m_pGridDRing = nullptr;
	m_pGridMantua = nullptr;
	m_pEventPanel = nullptr;
	m_pGridNewSlot1 = nullptr;
	m_pGridNewSlot2 = nullptr;
	m_dwLastAutoAttackTime = 0;
	m_nCoinMsgType = 0;
	m_dwOpID = 0;
	m_dwChatTime = 0;
	m_fFlashTerm = 2000.0f;
	m_dwLastCheckAutoMouse = 0;
	m_bLastMyAttr = 0;
	m_nLastPotal = -1;
	m_dwTID = 0;
	m_bWarning = 0;
	m_bQuater = 1;
	m_cWarClan = -1;
	m_cAutoAttack = 0;
	g_pDevice->m_bFog = 1;

	TMFieldScene::m_bPK = 0;

	m_bShowNameLabel = 1;
	m_cWhisper = 0;
	m_cPartyChat = 0;
	m_cGuildChat = 0;
	m_bTab = 0;

	TMFieldScene::m_dwCargoID = 0;

	m_dwHellStoreID = 0;
	m_nHellStoreValue = 0;
	m_nFireWorkCellX = -1;
	m_nFireWorkCellY = -1;
	m_nTotoNum = 0;

	memset(&m_stToto, 0, sizeof(m_stToto));

	m_sDay = 0;

	memset(m_dwLastChatTime, 0, sizeof(m_dwLastChatTime));
	memset(m_pGridAutoTrade, 0, sizeof(m_pGridAutoTrade));
	memset(m_pGridItemMix, 0, sizeof(m_pGridItemMix));
	// Mix2 and Mix3 are stock FieldScene2 panels, not optional modern systems.
	// Their pointers stay deterministic until the 7.48 control tree is loaded.
	memset(m_pGridItemMix2, 0, sizeof(m_pGridItemMix2));
	memset(m_pGridItemMix3, 0, sizeof(m_pGridItemMix3));
	// The 7.48 compatibility initializer skips the later tiny-mix bindings, so
	// these optional pointers must remain deterministic until a real control is found.
	memset(m_pGridItemMix4, 0, sizeof(m_pGridItemMix4));
	// ItemMix5 is present in the stock 7.48 resource even though the imported
	// source had lost its bindings. Keep the seven native receptacles deterministic.
	memset(m_pGridItemMix5, 0, sizeof(m_pGridItemMix5));
	memset(m_pGridItemMix6, 0, sizeof(m_pGridItemMix6));
	memset(m_pGridMixResult, 0, sizeof(m_pGridMixResult));
	memset(m_pSlotTrade, 0, sizeof(m_pSlotTrade));

	m_pChatList = nullptr;
	m_pChatPanel = nullptr;
	m_pChatBack = nullptr;
	m_pChatSelectPanel = nullptr;
	m_pChatListPanel = nullptr;
	m_pChatType = nullptr;
	m_pPositionText = nullptr;
	// WYD-Go 7.48 compatibility: the compact field UI does not instantiate the
	// newer minimap controls, so keep every optional minimap pointer deterministic.
	m_pMiniMapDir = nullptr;
	m_pMiniMapPanel = nullptr;
	m_pMiniMapZoomIn = nullptr;
	m_pMiniMapZoomOut = nullptr;
	m_pMiniMapServerPanel = nullptr;
	m_pMiniMapServerText = nullptr;
	// Both initializers create the 256 legacy marker pairs only after the map
	// panel has been resolved. Deterministic nulls keep partial initialization
	// and allocation failure from turning the first M shortcut into stale access.
	memset(m_pInMiniMapPosPanel, 0, sizeof(m_pInMiniMapPosPanel));
	memset(m_pInMiniMapPosText, 0, sizeof(m_pInMiniMapPosText));
	m_pFadePanel = nullptr;
	m_pEditChat = nullptr;
	m_pInputBG2 = nullptr;
	m_pInvenPanel = nullptr;
	m_pAutoTrade = nullptr;
	m_pItemMixPanel = nullptr;
	m_pItemMixPanel2 = nullptr;
	m_pItemMixPanel3 = nullptr;
	m_pItemMixPanel4 = nullptr;
	m_pItemMixPanel5 = nullptr;
	m_pItemMixPanel6 = nullptr;
	m_pSystemPanel = nullptr;
	m_pPGTPanel = nullptr;
	m_pInputGoldPanel = nullptr;
	m_pCPanel = nullptr;
	m_pSkillPanel = nullptr;
	m_pKingDomFlag = nullptr;
	m_pFlagDesc = nullptr;
	m_pFlagDescText[0] = nullptr;
	m_pFlagDescText[1] = nullptr;
	m_pFlagDescText[2] = nullptr;
	m_pRankTimeText = nullptr;
	m_pRemainText = nullptr;
	m_nLastTime = 0;
	m_dwStartRankTime = 0;
	m_bInstanceRemainOn = 0;
	m_pMsgPanel = nullptr;
	m_pMsgList = nullptr;
	m_pMsgText = nullptr;
	m_nVillage = -1;
	m_pQuizPanel = nullptr;
	m_pQuizCaption = nullptr;

	memset(m_pQuizText, 0, sizeof(m_pQuizText));
	m_pQuizBG = 0;
	m_pQuizQuestion = 0;
	memset(m_pQuizButton, 0, sizeof(m_pQuizButton));

	m_pQuestBtn = 0;
	m_pAutoRunBtn = 0;
	m_pHelpBtn = 0;
	m_pMGameAutoBtn = 0;
	m_pSGameAutoBtn = 0;
	m_pNativeCCPhysicalBtn = nullptr;
	m_pNativeCCMagicBtn = nullptr;
	m_pAutoTarget = 0;
	m_pCCPotionBtn = 0;
	m_pCCFeedBtn = 0;
	m_pccmode = 0;
	m_pCCModeHpSte = 0;
	m_pCCModeMountSte = 0;
	m_AutoStartPointX = 0;
	m_AutoStartPointY = 0;
	m_AutoPostionUse = 2;
	m_AutoHpMp = 1;
	m_pSetType = 0;
	m_pHelpPanel = 0;
	m_pHelpText = 0;

	memset(m_pHelpButton, 0, sizeof(m_pHelpButton));
	memset(m_pHelpList, 0, sizeof(m_pHelpList));

	m_pFireWorkPanel = 0;
	memset(m_pFireWorkButton, 0, sizeof(m_pFireWorkButton));
	m_pFireWorkOKButton = 0;
	m_pFireWorkQuitButton = 0;

	m_pTotoPanel = 0;
	m_pTotoSelect_Btn = 0;
	m_pTotoBuy_Btn = 0;
	m_pTotoQuit_Btn = 0;
	m_pTotoNumber_Edit = 0;
	m_pTotoTime_Txt = 0;
	m_pTotoTeamA_Txt = 0;
	m_pTotoTeamB_Txt = 0;
	m_pTotoScoreA_Edit = 0;
	m_pTotoScoreB_Edit = 0;

	memset(m_szAutoCaption, 0, sizeof(m_szAutoCaption));
	memset(m_szAutoClass, 0, sizeof(m_szAutoClass));
	memset(m_szAutoFolder, 0, sizeof(m_szAutoFolder));
	memset(m_szAutoFolder2, 0, sizeof(m_szAutoFolder2));
	memset(m_szAutoProcess, 0, sizeof(m_szAutoProcess));

	m_szAutoFolder[0] = 107;
	m_szAutoFolder[1] = 48;
	m_szAutoFolder[2] = 101;
	m_szAutoFolder[3] = 48;
	m_szAutoFolder[4] = 98;
	m_szAutoFolder[5] = 48;
	m_szAutoFolder[6] = 105;
	m_szAutoFolder[7] = 48;
	m_szAutoFolder[8] = 97;
	m_szAutoFolder[9] = 48;
	m_szAutoFolder[10] = 117;
	m_szAutoFolder[11] = 116;
	m_szAutoFolder[12] = 111;
	m_szAutoFolder[13] = 92;
	m_szAutoFolder[14] = 107;
	m_szAutoFolder[15] = 101;
	m_szAutoFolder[16] = 98;
	m_szAutoFolder[17] = 105;
	m_szAutoFolder[18] = 97;
	m_szAutoFolder[19] = 117;
	m_szAutoFolder[20] = 116;
	m_szAutoFolder[21] = 111;
	m_szAutoFolder[22] = 46;
	m_szAutoFolder[23] = 101;
	m_szAutoFolder[24] = 120;
	m_szAutoFolder[25] = 101;
	m_szAutoFolder2[0] = 65;
	m_szAutoFolder2[1] = 99;
	m_szAutoFolder2[2] = 116;
	m_szAutoFolder2[3] = 105;
	m_szAutoFolder2[4] = 118;
	m_szAutoFolder2[5] = 101;
	m_szAutoFolder2[6] = 74;
	m_szAutoFolder2[7] = 111;
	m_szAutoFolder2[8] = 121;
	m_szAutoFolder2[9] = 46;
	m_szAutoFolder2[10] = 111;
	m_szAutoFolder2[11] = 99;
	m_szAutoFolder2[12] = 120;

	memset(m_pQuestButton, 0, sizeof(m_pQuestButton));
	memset(m_pQuestList, 0, sizeof(m_pQuestList));
	memset(m_pQuestContentList, 0, sizeof(m_pQuestContentList));

	m_pPartyPanel = 0;
	m_pBtnPGTParty = 0;
	m_pBtnPGTGuild = 0;
	m_pBtnPGTTrade = 0;
	m_pBtnPGTChallenge = 0;
	m_pBtnPGT1_V_1 = 0;
	m_pBtnPGT5_V_5 = 0;
	m_pBtnPGT10_V_10 = 0;
	m_pBtnPGTAll_V_All = 0;
	m_pBtnPGTGuildDrop = 0;
	m_pBtnPGTGuildWar = 0;
	m_pBtnPGTGuildAlly = 0;
	m_pBtnPGTGuildInvite = 0;
	m_pBtnPGTGICommon = 0;
	m_pBtnPGTGIChief1 = 0;
	m_pBtnPGTGIChief2 = 0;
	m_pBtnPGTGIChief3 = 0;
	m_pHPBar = 0;
	m_pMPBar = 0;
	m_pMHPBar = 0;
	m_pMHPBarT = 0;
	m_pCurrentHPText = 0;
	m_pMaxHPText = 0;
	m_pCurrentMPText = 0;
	m_pMaxMPText = 0;
	m_pCurrentMHPText = 0;
	m_pMaxMHPText = 0;
	m_pInfoText = 0;

	memset(m_szLastAttackerName, 0, sizeof(m_szLastAttackerName));

	for (int i = 0; i < 12; ++i)
	{
		float fAngle = (((float)i * 3.1415927f) * 2.0f) / 12.0f;
		m_vecKnifePos[i] = TMVector2(cosf(fAngle) * 10.0f, sinf(fAngle) * 10.0f);
	}
	for (int i = 0; i < 24; ++i)
		m_pSkillSecGrid[i] = 0;
	for (int i = 0; i < 12; ++i)
		m_pSkillSecGrid2[i] = 0;

	m_pGridSkillBelt = 0;
	m_pGridSkillBelt2 = 0;
	m_pGridSkillBelt3 = 0;
	m_pShortSkillTglBtn1 = 0;
	m_pShortSkillTglBtn2 = 0;
	g_pGBPanel = 0;
	m_nCameraLoop = 0;

	TMFieldScene::m_usProgress = 0;

	m_nYear = 0;
	m_nDays = 0;
	m_nBet = 1000;
	m_dwEventTime = 0;
	m_dwLastClbuttonTime = 0;
	m_bClbutton = 0;
	m_dwKhepraID = 0;

	memset(&m_stRemoveServer, 0, sizeof(m_stRemoveServer));

	m_nServerMove = 0;
	m_pPotalPanel = 0;
	m_pPotalList = 0;
	m_pPotalText = 0;
	g_nTempArray[2] = (int)m_bShowNameLabel + 28;
	g_nTempArray2[2] = (int)m_bShowNameLabel + 28;
	g_pObjectManager->m_cSelectShortSkill = 0;

	m_pKhepraPortal = 0;
	m_pKhepraPortalEff1 = 0;
	m_pKhepraPortalEff2 = 0;
	m_dwKhepraDieTime = 0;
	m_nKhepraDieFlag = 0;
	m_nQuest12MaxMobs = 0;
	m_pQuestRemainTime = 0;
	m_nCameraMode = 0;
	m_bCantAttk = 0;
	m_dwCantMoveTime = 0;
	m_bTeleportMsg = 0;
	m_dwLastSecProcessTime = 0;
	m_nGuildMarkCount = 0;
	m_hInternetSession = 0;
	m_dwLastGetGuildmarkTime = 0;
	m_bIsDungeon = 0;
	m_nMouseRDownX = 0;
	m_nMouseRDownY = 0;
	m_dwDeleteURLTime = 0;
	m_bAirMove = 0;
	m_nOldMountSkinMeshType = -1;
	m_nAirMove_State = 0;
	m_dwAirMove_TickTime = 0;
	m_bAirMove_Wing = 0;
	m_vecAirMove_Origin.x = 0.0f;
	m_vecAirMove_Origin.y = 0.0f;
	m_vecAirMove_Dest.x = 0.0;
	m_vecAirMove_Dest.y = 0.0;
	m_fAirMove_Speed = 0.6f;
	m_bAirmove_ShowUI = 0;
	m_nAirMove_Index = 0;
	m_nAirMove_RouteIndex = 0;
	m_eOldMotion = ECHAR_MOTION::ECMOTION_NONE;
	m_pPotalText1 = 0;
	m_pPotalText2 = 0;
	m_pPotalText3 = 0;

	for (int i = 0; i < 32; ++i)
	{
		m_pAffectIcon[i] = 0;
		m_pTargetAffectIcon[i] = 0;
		m_dwAffectBlinkTime[i] = 0;
	}

	for (int i = 0; i < 13; ++i)
	{
		for (int j = 0; j < 32; ++j)
			m_pPartyAffectIcon[i][j] = 0;
	}

	m_pPartyAffectText = 0;
	m_pPartyAutoButton = 0;
	m_pPartyAutoText = 0;
	m_bAutoParty = 0;
	m_pDailyQuestButton = 0;
	m_pAffectDesc = 0;
	m_pChatGeneral_C = 0;
	m_pChatParty_C = 0;
	m_pChatWhisper_C = 0;
	m_pChatGuild_C = 0;
	m_pMiniBtn = 0;
	m_pMiniPanel = 0;
	m_pMainInfo2 = 0;
	m_pShortSkillPanel = 0;
	m_pMainInfo1 = 0;
	m_pMainInfo1_BG = 0;
	m_nIsMP = 0;
	m_bShowExp = 0;
	m_bTempCastlewar = 0;

	for (int i = 0; i < 4; ++i)
		m_bJPNBag[i] = 0;

	strcpy(m_cChatType, "");
	strcpy(m_cChatSelect, "");
	strcpy(m_cWhisperName, "");

	m_bShowBoss = 0;
	m_berielAura = {};
	m_berielAura.SetServer(g_nWeather);
	m_Coin = 0;
	m_sShopTarget = 0;
	m_bEventCouponClick = 0;
	m_bEventCouponOpen = 0;
}

TMFieldScene::~TMFieldScene()
{
	g_nTempArray[1] = 0;
	g_nTempArray[2] = 0;

	if (m_pHelpList[3] && g_pObjectManager)
	{
		memset(g_pObjectManager->m_stMemo, 0, sizeof(g_pObjectManager->m_stMemo));
		for (int i = 0; i < m_pHelpList[3]->m_nNumItem; ++i)
		{
			g_pObjectManager->m_stMemo[i].dwColor = m_pHelpList[3]->m_pItemList[i]->m_GCText.dwColor;
			sprintf(g_pObjectManager->m_stMemo[i].szString, "%s", m_pHelpList[3]->m_pItemList[i]->m_GCText.strString);
		}
	}

	if (g_pDevice)
		g_pDevice->m_nWidthShift = 0;

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(107);
		if (pSoundData && pSoundData->IsSoundPlaying())
			pSoundData->Stop();

		pSoundData = pSoundManager->GetSoundData(6);
		if (pSoundData && pSoundData->IsSoundPlaying())
			pSoundData->Stop();
	}

	if (g_pCurrentScene && g_pApp
		&& g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
		&& g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD
		&& g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO
		&& g_pApp->m_pBGMManager)
	{
		g_pApp->m_pBGMManager->StopBGM();
	}

	if (m_pMyHuman && g_pObjectManager)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (!strcmp(m_pMyHuman->m_szName, g_pObjectManager->m_stSelCharData.MobName[j]))
			{
				g_pObjectManager->m_stSelCharData.Exp[j] = g_pObjectManager->m_stMobData.Exp;
				break;
			}
		}
	}
}

int TMFieldScene::InitializeScene()
{
	LOG_WRITELOG(">> Init Field Scene::Start\r\n");
	if (!LoadRC("UI\\FieldScene2.txt"))
	{
		LOG_WRITELOG("Can't load FieldScene2 resource\r\n");
		return 0;
	}
	// FieldScene2 is the native 7.48 resource. The later client's HUD path,
	// which needed controls such as 66817, was removed with those controls:
	// the world and the character are initialized by the 7.48 path only.
	const int result = InitializeCompatFieldScene();
	LOG_WRITELOG(">> Init Field Scene: result=%d\r\n", result);
	return result;
}

int TMFieldScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)
{
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
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
	const bool isChatEdit = idwControlID == TME_CHAT;

	int extractedResult{};
	const ExtractedFlow extractedFlow = OnCompatFieldControl(dwServerTime, idwControlID, extractedResult);
	if (extractedFlow == ExtractedFlow::Return)
		return extractedResult;

	auto pMobData = &g_pObjectManager->m_stMobData;
	int cktrans = 0;
	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		cktrans = 1;

	if (idwControlID == TMB_IME_BUTTON)
	{
		if (m_pControlContainer->m_pFocusControl
			&& m_pControlContainer->m_pFocusControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT && m_pControlContainer->m_pFocusControl->m_bVisible)
		{
			if (g_pEventTranslator->IsNative())
				g_pEventTranslator->SetIMEAlphaNumeric();
			else
				g_pEventTranslator->SetIMENative();
		}
	}
	else if (isChatEdit && idwEvent == 8)
		return 1;

	if (idwControlID == TMB_HELLSTORE_OK)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[144], m_nHellStoreValue, 0);
			m_pMessageBox->m_dwArg = m_dwHellStoreID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}
	if (idwControlID >= TMB_HELLSTORE_SELECT_1 && idwControlID <= TMB_HELLSTORE_SELECT_4)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnHellStoreSelect(idwControlID, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID >= TMB_GAMBLE_LEFTX2 && idwControlID <= TMB_GAMBLE_RIGHT)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnGambleButton(idwControlID, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == TMB_GAMBLE_START)
	{
		if ((m_cGambleType != 1 && m_cGambleType != 2) ||
			!m_pGambleStore || !m_pGambleStore->IsVisible() ||
			m_cPendingGambleType != 0 || m_nBet < 1000 || m_nBet > 100000 ||
			pMobData->Coin < m_nBet)
			return 1;

		SReelPanel* reel = m_cGambleType == 1 ? m_pReelPanel : m_pReelPanel2;
		if (!reel || !reel->IsVisible() || reel->m_dwStopTime || reel->m_bRoling)
			return 1;

		MSG_STANDARDPARM2 stParm2{};
		stParm2.Header.ID = g_pObjectManager->m_dwCharID;
		stParm2.Header.Type = MSG_DoJackpotBet_Opcode;
		stParm2.Parm1 = m_cGambleType;
		stParm2.Parm2 = m_nBet;
		m_cPendingGambleType = m_cGambleType;
		m_dwGambleRequestTime = g_pTimerManager->GetServerTime();
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm2)->Type, reinterpret_cast<char*>(&stParm2), sizeof(stParm2)});

		reel->m_dwBatCoin = m_nBet;
		reel->SetRoll(true, 0, 0, 0, 3000);
		GetSoundAndPlay(338, 0, 0);
		return 1;
	}
	if (idwControlID == TMB_GAMBLE_QUIT)
	{
		SetVisibleGamble(0, 0);
		return 0;
	}
	if (isChatEdit && !idwEvent)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnChatEditSubmit(dwServerTime, pMobData, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (isChatEdit && (idwEvent == 2 || idwEvent == 3))
	{
		char szText[128]{};
		sprintf_s(szText, m_szLastChatList[m_sChatIndex]);

		m_pEditChat->SetText(szText);
		++m_sChatIndex;
		m_sChatIndex %= 5;
		return 0;
	}
	if (isChatEdit && (idwEvent == 4 || idwEvent == 5))
	{
		char szWhisperList[19]{};
		sprintf(szWhisperList, "/%s ", m_szWhisperList[m_sWhisperIndex]);

		m_pEditChat->SetText(szWhisperList);
		++m_sWhisperIndex;
		m_sWhisperIndex %= 5;
		return 0;
	}
	if (isChatEdit && idwEvent == 6)
	{
		m_pEditChat->SetText((char*)"");
		return 0;
	}
	// FieldScene2.bin (7.48) uses the original resource IDs. Route the whole
	// input modal, including keyboard confirmation and cancel, through the
	// shared handlers instead of translating only its OK button.
	if (idwControlID == TMB_MONEY)
		idwControlID = B_MONEY;
	else if (idwControlID == TMB_CARGO_MONEY)
		idwControlID = B_CARGO_MONEY;
	else if (idwControlID == TMB_IG_OK)
		idwControlID = B_IG_OK;
	else if (idwControlID == TMB_IG_CANCEL)
		idwControlID = B_IG_CANCEL;
	else if (idwControlID == TME_INPUT_GOLD)
		idwControlID = E_INPUT_GOLD;
	if (idwControlID == B_MONEY)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnMoneyButton(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == B_CARGO_MONEY)
	{
		if (!m_pMyHuman || !m_pControlContainer || !m_pInputGoldPanel ||
			m_pMyHuman->m_cDie == 1)
			return 1;

		if (!m_pAutoTrade || m_pAutoTrade->IsVisible() != 1)
		{
			auto pText = static_cast<SText*>(m_pControlContainer->FindControl(
				TMT_INPUT_GOLD));
			auto pEdit = m_pControlContainer->FindControl(
				TME_INPUT_GOLD);

			if (pText && pEdit && m_pCargoPanel && m_pCargoPanel->IsVisible() == 1)
			{
				m_nCoinMsgType = 2;
				pText->SetText(g_pMessageStringTable[138], 0);
				m_pControlContainer->SetFocusedControl(pEdit);
				m_pInputGoldPanel->SetVisible(1);
				if (m_pChatSelectPanel)
					m_pChatSelectPanel->SetVisible(0);
			}
		}
		return 1;
	}
	if (idwControlID == B_IG_OK)
	{
		auto pInputText = static_cast<SEditableText*>(m_pControlContainer->FindControl(
			TME_INPUT_GOLD));
		// FieldScene2.bin dispatches the native edit through the compatibility ID
		// translator.  Treat a missing control as an incomplete resource instead of
		// dereferencing it while processing the modal confirmation.
		if (!pInputText)
			return 1;

		// A capsule name is not currency (including the valid name "all...").
		// Keep the prompt and its text on rejection instead of consuming the modal.
		if (m_nCoinMsgType == 6)
		{
			if (SendCapsuleItem())
			{
				m_pInputGoldPanel->SetVisible(0);
				pInputText->SetText((char*)"");
				m_pControlContainer->SetFocusedControl(g_nKeyType == 1 ? m_pEditChat : nullptr);
			}
			return 1;
		}

		char* inputText = pInputText->GetText();

		int inputTextLen = strlen(inputText);

		if (strcmp(inputText, "all"))
		{
			const bool bFind = coin_input::ScanAndSanitize(inputText, inputTextLen);

			long long nInputValue = _atoi64(pInputText->GetText());

			switch (coin_input::Validate(m_nCoinMsgType, strlen(pInputText->GetText()), bFind,
				nInputValue, [] { return g_pObjectManager->m_stMobData.Coin; }))
			{
			case coin_input::Rejection::Empty:
			{
				m_pControlContainer->SetFocusedControl(pInputText);
				return 1;
			}
			case coin_input::Rejection::InvalidAmount:
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[34], 1000);
				m_pMessagePanel->SetVisible(1, 1);
				m_pControlContainer->SetFocusedControl(pInputText);
				// The stock 7.48 price prompt has no newer chat selector; keep
				// invalid-price feedback usable when that optional control is absent.
				if (m_pChatSelectPanel)
					m_pChatSelectPanel->SetVisible(0);
				return 1;
			}
			case coin_input::Rejection::InvalidName:
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[409], 1000);
				m_pMessagePanel->SetVisible(1, 1);
				m_pControlContainer->SetFocusedControl(pInputText);
				return 1;
			}
			case coin_input::Rejection::AutoTradeNonPositive:
			case coin_input::Rejection::AutoTradeAboveLimit:
			{
				char istrMessage[128]{};

				if (nInputValue <= 0)
					sprintf_s(istrMessage, "%s", g_pMessageStringTable[34]);
				else
					sprintf_s(istrMessage, g_pMessageStringTable[143], 2000000000);

				m_pMessagePanel->SetMessage(istrMessage, 1000);
				m_pMessagePanel->SetVisible(1, 1);

				m_pControlContainer->SetFocusedControl(pInputText);
				return 1;
			}
			case coin_input::Rejection::ServerWarChannel:
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[34], 1000);
				m_pMessagePanel->SetVisible(1, 1);
				m_pControlContainer->SetFocusedControl(pInputText);
				return 1;
			}
			case coin_input::Rejection::None:
				break;
			}

			switch (m_nCoinMsgType)
			{
			case 2:
			{
				MSG_STANDARDPARM stWithdraw{};

				stWithdraw.Header.Type = MSG_Withdraw_Opcode;
				stWithdraw.Header.ID = m_pMyHuman->m_dwID;
				stWithdraw.Parm = static_cast<int>(nInputValue);
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stWithdraw)->Type, reinterpret_cast<char*>(&stWithdraw), sizeof(stWithdraw)});
			}
			break;
			case 0:
			{
				MSG_STANDARDPARM stDeposit{};

				stDeposit.Header.Type = MSG_Deposit_Opcode;
				stDeposit.Header.ID = m_pMyHuman->m_dwID;
				stDeposit.Parm = static_cast<int>(nInputValue);
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stDeposit)->Type, reinterpret_cast<char*>(&stDeposit), sizeof(stDeposit)});
			}
			break;
			case 1:
			{
				auto pMyCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(617u));
				auto pOpCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(601u));

				if (!pMyCheckButton || !pOpCheckButton || !m_pMyHuman ||
					!m_pTradePanel || !m_pTradePanel->IsVisible() ||
					!g_pApp || !g_pApp->m_pTimerManager ||
					!g_pObjectManager || !g_pObjectManager->m_stTrade.OpponentID)
					break;

				pMyCheckButton->m_bSelected = 0;
				pOpCheckButton->m_bSelected = 0;

				m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();

				g_pObjectManager->m_stTrade.MyCheck = pMyCheckButton->m_bSelected;
				g_pObjectManager->m_stTrade.TradeMoney = static_cast<int>(nInputValue);
				g_pObjectManager->m_stTrade.Header.ID = m_pMyHuman->m_dwID;
				g_pObjectManager->m_stTrade.Header.Type = MSG_Trade_Opcode;

				WYD748_LogTradeSend("gold", g_pObjectManager->m_stTrade);
				SendOneMessage((char*)&g_pObjectManager->m_stTrade, sizeof(g_pObjectManager->m_stTrade));

				auto pMyGold = static_cast<SText*>(m_pControlContainer->FindControl(619u));

				char szText[11]{};

				sprintf_s(szText, "%10lld", nInputValue);

				if (pMyGold)
					pMyGold->SetText(szText, 0);
			}
			break;
			case 3:
			{
				auto pATradeTitle = (SText*)m_pControlContainer->FindControl(TMT_ATRADE_TITLE);
				auto pATradeName = (SText*)m_pControlContainer->FindControl(TMT_ATRADE_ID);

				if (pATradeTitle)
				{
					pATradeTitle->SetText(pInputText->GetText(), 0);
				}

				if (!m_pMyHuman)
					return 1;
				if (pATradeName)
					pATradeName->SetText(m_pMyHuman->m_szName, 1);

				m_stAutoTrade.TargetID = m_pMyHuman->m_dwID;

				sprintf_s(m_stAutoTrade.Desc, "%s", pInputText->GetText());

				if (m_pInputBG2)
					m_pInputBG2->SetVisible(0);

				pInputText->m_nMaxStringLen = 10;

				// MSG_AutoTrade and native FUN_004656af both carry twelve 7.48
				// slots. Newer resources expose ten, so only compatibility mode
				// widens the cleanup loop to the packet's native boundary.
				const int autoTradeSlotCount = 12;
				for (int i = 0; i < autoTradeSlotCount; ++i)
				{
					auto pAutoTradeGrid = m_pGridAutoTrade[i];
					if (!pAutoTradeGrid)
						continue;
					SGridControlItem* pAutoTradeItem = pAutoTradeGrid->PickupAtItem(0, 0);

					// The 7.48 cargo is one 9-column surface (Ghidra FUN_0052a737),
					// while the imported source assumed a five-column cargo page.
					int cargoX = 0;
					int cargoY = 0;
					GetCargoCellForSlot(m_stAutoTrade.CarryPos[i], cargoX, cargoY);
					auto pCargoGrid = GetCargoGridForSlot(m_stAutoTrade.CarryPos[i]);
					auto pCargoItem = pCargoGrid ? pCargoGrid->GetAtItem(cargoX, cargoY) : nullptr;

					if (pCargoItem)
						pCargoItem->m_GCObj.dwColor = -1;

					WYD748_ReleaseAutoTradeItem(pAutoTradeItem);
				}
				SetVisibleAutoTrade(1, 1);
			}
			break;
			case 11:
			{
				sprintf_s(m_stPass, "%s", pInputText->GetText());

				m_pitemPassGrid->SellItem2();

				memset(m_stPass, 0, sizeof(m_stPass));
			}
			break;
			case 4:
			{
				if (m_nLastAutoTradePos >= 0)
				{
					// MSG_AutoTrade and FUN_004662c5 both enumerate twelve native
					// sale cells.  Stopping at ten made the full-store branch
					// unreachable and silently discarded clicks once those cells filled.
					constexpr int kNativeAutoTradeSlots = 12;
					for (int j = 0; j < kNativeAutoTradeSlots; ++j)
					{
						SGridControl* pParent = m_pGridAutoTrade[j];
						if (!pParent)
							continue;
						SGridControlItem* pAutoTradeItem = pParent->GetAtItem(0, 0);

						// Resolve the selected cargo slot through the active ABI instead of
						// hard-coding the newer 40-slot page transform.
						int cargoX = 0;
						int cargoY = 0;
						GetCargoCellForSlot(m_nLastAutoTradePos, cargoX, cargoY);
						SGridControl* pCargoGrid = GetCargoGridForSlot(m_nLastAutoTradePos);
						SGridControlItem* pCargoItem = pCargoGrid ? pCargoGrid->GetAtItem(cargoX, cargoY) : nullptr;

						if (!pAutoTradeItem)
						{
							if (!pCargoItem || !pCargoItem->m_pItem)
								return 1;

							STRUCT_ITEM selectedItem{};
							memcpy(&selectedItem, pCargoItem->m_pItem, sizeof(STRUCT_ITEM));
							auto pItem = new STRUCT_ITEM;
							if (!pItem)
								continue;
							memcpy(pItem, &selectedItem, sizeof(STRUCT_ITEM));

							// Cargo reservation and announcement happen only after the grid
							// assumes ownership of the visual. Rejection leaves state unchanged.
							auto pTradeItem = new SGridControlItem(pParent, pItem, 0.0f, 0.0f);
							if (!pTradeItem)
							{
								delete pItem;
								continue;
							}
							if (!pParent->AddItem(pTradeItem, 0, 0))
							{
								SAFE_DELETE(pTradeItem);
								continue;
							}

							pCargoItem->m_GCObj.dwColor = 0xFFFF0000;

							m_stAutoTrade.CarryPos[j] = m_nLastAutoTradePos;
							m_stAutoTrade.TradeMoney[j] = static_cast<int>(nInputValue);

							memcpy(&m_stAutoTrade.Item[j], &selectedItem, sizeof(STRUCT_ITEM));

							pParent->m_nTradeMoney = static_cast<int>(nInputValue);
							break;
						}

						if (pAutoTradeItem && j == kNativeAutoTradeSlots - 1)
						{
							m_pMessagePanel->SetMessage(g_pMessageStringTable[1], 2000);
							m_pMessagePanel->SetVisible(1, 1);

							if (pCargoItem)
								pCargoItem->m_GCObj.dwColor = 0xFFFFFFFF;
						}
					}
					m_nLastAutoTradePos = -1;
				}
			}
			break;
			case 7:
			{
				MSG_STANDARDPARM stPacket{};

				stPacket.Header.Type = 977;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm = static_cast<int>(nInputValue);
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stPacket)->Type, reinterpret_cast<char*>(&stPacket), sizeof(stPacket)});
			}
			break;
			case 8:
			{
				char str2[128]{};

				sprintf_s(str2, "%s %s", m_pPGTOver->m_szName, inputText);

				MSG_MessageWhisper stMessageWhisper{};

				stMessageWhisper.Header.ID = g_pObjectManager->m_dwCharID;
				stMessageWhisper.Header.Type = MSG_MessageWhisper_Opcode;
				sprintf_s(stMessageWhisper.MobName, "subcreate");
				sprintf_s(stMessageWhisper.String, "%s", str2);

				if (strlen(str2) >= 16)
				{
					str2[15] = 0;
					str2[14] = 0;
				}

				if (!CheckGuildName(stMessageWhisper.String, 1))
				{
					m_pMessagePanel->SetMessage(g_pMessageStringTable[370], 2000);
					m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
				if (m_szWhisperList[0][0])
				{
					if (strcmp((char*)m_szWhisperList, str2) != 0)
					{
						for (int i = 4; i > 0; --i)
							memcpy(m_szWhisperList[i], m_szWhisperList[i - 1], 16);

						sprintf_s(m_szWhisperList[0], "%s", str2);
					}
				}
				else
				{
					for (int i = 0; i < 5; ++i)
						sprintf_s(m_szWhisperList[i], "%s", str2);
				}
				m_sWhisperIndex = 0;

				char* text = pInputText->GetText();

				if (text[strlen(str2) + 1])
				{
					sprintf(stMessageWhisper.String, "%s", &text[strlen(str2) + 2]);
				}

				BASE_TransCurse(stMessageWhisper.String);
				pInputText->SetText((char*)"");
				SendOneMessage((char*)&stMessageWhisper, sizeof(stMessageWhisper));
			}
			break;
			case 9:
			{
				MSG_STANDARDPARM stPacket{};

				stPacket.Header.Type = MSG_UseDeclarationOfWar_Opcode;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm = static_cast<int>(nInputValue);
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stPacket)->Type, reinterpret_cast<char*>(&stPacket), sizeof(stPacket)});
			}
			break;
			case 10:
			{
				MSG_STANDARDPARM stPacket{};

				stPacket.Header.Type = MSG_UseRefuseServerWar_Opcode;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm = static_cast<int>(nInputValue);
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stPacket)->Type, reinterpret_cast<char*>(&stPacket), sizeof(stPacket)});
			}
			break;
			case 12:
			{
				// Establish live grid ownership before dereferencing a selection
				// that may have outlived an authoritative inventory update.
				auto pSplitItem = m_pGridInv ? field_interaction::FindOwnedItem(
					m_pGridInv->m_pItemList, m_pGridInv->m_nNumItem, SGridControl::m_pSellItem) : nullptr;
				if (!pSplitItem || !pSplitItem->m_pItem ||
					pSplitItem->m_pGridControl != m_pGridInv || !m_pMyHuman)
				{
					SetInVisibleInputCoin();
					return 0;
				}
				int nItemAmount = BASE_GetItemAmount(pSplitItem->m_pItem);

				if (!field_interaction::IsValidStackSplitQuantity(nInputValue, nItemAmount))
				{
					if (m_pMessagePanel)
					{
						m_pMessagePanel->SetMessage(g_pMessageStringTable[409], 1000);
						m_pMessagePanel->SetVisible(1, 1);
					}
					m_pControlContainer->SetFocusedControl(pInputText);
					return 1;
				}

				if (m_pGridInv->CheckType(
					pSplitItem->m_pGridControl->m_eItemType,
					pSplitItem->m_pGridControl->m_eGridType) != 1)
				{
					SetInVisibleInputCoin();
					return 0;
				}

				// WYD 7.48 keeps all 63 Carry slots in one 9x7 grid. Splitting an item
				// therefore sends the native row-major slot with no synthetic page.
				int pos = pSplitItem->m_nCellIndexX + 9 * pSplitItem->m_nCellIndexY;

				MSG_STANDARDPARM3 stPacket{};

				stPacket.Header.Type = MSG_SplitItem_Opcode;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm1 = pos;
				stPacket.Parm2 = pSplitItem->m_pItem->sIndex;
				stPacket.Parm3 = static_cast<int>(nInputValue);
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stPacket)->Type, reinterpret_cast<char*>(&stPacket), sizeof(stPacket)});
				SetInVisibleInputCoin();
				return 1;
			}
			break;
			}

			m_pControlContainer->SetFocusedControl(0);
			m_pInputGoldPanel->SetVisible(0);

			pInputText->SetText((char*)"");

			if (g_nKeyType == 1)
				m_pControlContainer->SetFocusedControl(m_pEditChat);
		}
		else
		{
			char szText[11]{};
		 //   if (pItem->m_pItem->sIndex == 5652)// daily reward
			//{

			//}
			const auto allSource = coin_input::AllAmountSource(m_nCoinMsgType);
			if (allSource == coin_input::AllSource::Carried)
			{
				sprintf_s(szText, "%d", g_pObjectManager->m_stMobData.Coin);

				pInputText->SetText(szText);
			}
			else if (allSource == coin_input::AllSource::Cargo)
			{
				sprintf_s(szText, "%d", g_pObjectManager->m_nCargoCoin);

				pInputText->SetText(szText);
			}
		}

		return 1;
	}
	if (idwControlID == E_INPUT_GOLD)
	{
		if (m_pMyHuman->m_cDie == 1 || m_nCoinMsgType == 5)
			return 1;

		OnControlEvent(B_IG_OK, 0);
		return 1;
	}
	if (idwControlID == B_IG_CANCEL)
	{
		SetInVisibleInputCoin();
		return 0;
	}
	if (idwControlID == B_INV_CLOSE)
	{
		SetVisibleInventory();

		auto pEquipBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));

		pEquipBtn->SetSelected(m_pInvenPanel->m_bVisible);

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}
	if (idwControlID == B_CHAR_CLOSE)
	{
		SetVisibleCharInfo();

		auto pCharBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));

		pCharBtn->SetSelected(m_pCPanel->m_bVisible);

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}

	if (idwControlID == B_SKILL_CLOSE)
	{
		SetVisibleSkill();

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}
	if (idwControlID == B_CHAR)
	{
		auto pATradePanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ATRADE_PANEL));

		if (pATradePanel && pATradePanel->IsVisible())
			return 1;

		if (m_pShopPanel && m_pShopPanel->IsVisible() == 1)
			return 1;

		if (!m_pTradePanel || m_pTradePanel->IsVisible() != 1)
		{
			auto pCharBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));

			if (pCharBtn)
				pCharBtn->SetSelected(pCharBtn->m_bSelected == 0);

			SetVisibleCharInfo();

			return 0;
		}

		return 1;
	}
	if (idwControlID == B_EQUIP)
	{
		auto pEquipBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));

		pEquipBtn->SetSelected(pEquipBtn->m_bSelected == 0);

		SetVisibleInventory();
		return 0;
	}
	if (idwControlID == B_CCMODE_SYSTEM)
	{
		if (!m_pccmode)
			return 0;

		if (m_pccmode->IsVisible())
		{
			m_pccmode->SetVisible(0);
		}
		else
		{
			m_pccmode->SetVisible(1);

			char szText[128]{};

			sprintf_s(szText, g_pMessageStringTable[476], g_GameAuto_hpValue, g_GameAuto_mountValue);

			auto pChatItem = new SListBoxItem(szText, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

			if (pChatItem)
				m_pChatList->AddItem(pChatItem);
		}

		return 0;
	}
	if (idwControlID == B_CCMODE_DLG_MOUNT)
	{
		g_GameAuto_mountValue += 10;

		if (g_GameAuto_mountValue > 90)
			g_GameAuto_mountValue = 30;

		char szText[128]{};

		if (g_GameAuto_mountValue)
			sprintf_s(szText, g_pMessageStringTable[478], g_GameAuto_mountValue);
		else
			sprintf_s(szText, g_pMessageStringTable[477]);

		auto pChatItem = new SListBoxItem(szText, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pChatItem)
			m_pChatList->AddItem(pChatItem);

		NewCCMode();

		return 0;
	}
	if (idwControlID == B_CCMODE_DLG_HP)
	{
		g_GameAuto_hpValue += 10;

		if (g_GameAuto_hpValue > 90)
			g_GameAuto_hpValue = 0;

		char szText[128]{};

		if (g_GameAuto_hpValue)
			sprintf_s(szText, g_pMessageStringTable[480], g_GameAuto_hpValue);
		else
			sprintf_s(szText, g_pMessageStringTable[479]);

		auto pChatItem = new SListBoxItem(szText, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pChatItem)
			m_pChatList->AddItem(pChatItem);

		NewCCMode();
		return 0;
	}
	if (idwControlID == B_CCMODE_DLG_MODE)
	{
		if (++g_GameAuto > 3)
			g_GameAuto = 0;

		NewCCMode(true);

		return 0;
	}
	const field_chat::Buttons<SButton> chatButtons{
		{m_pChatGeneral, m_pChatGeneral_C}, {m_pChatParty, m_pChatParty_C},
		{m_pChatWhisper, m_pChatWhisper_C}, {m_pChatGuild, m_pChatGuild_C}};
	const auto chatOutcome = field_chat::Handle(idwControlID,
		chatButtons, [this](field_chat::Channel channel, bool enabled)
		{
			switch (channel)
			{
			case field_chat::Channel::Whisper: SetWhisper(enabled); break;
			case field_chat::Channel::Party: SetPartyChat(enabled); break;
			case field_chat::Channel::Guild: SetGuildChat(enabled); break;
			default: break;
			}
		});
	// Missing native controls still consume this event; never fall through.
	if (chatOutcome != field_chat::Outcome::Unhandled)
		return 0;
	// FieldScene2 7.48 has one Carry and one Cargo grid.  It intentionally has
	// no handlers for the Japanese bag/page controls introduced after 7.48.
	if (idwControlID == P_CCMODE_DLG_PONT)
	{
		if (++m_AutoPostionUse >= 3)
			m_AutoPostionUse = 0;

		NewCCMode(false, true);

		return 0;
	}
	if (idwControlID == TMB_MINIMAP)
	{
		SetVisibleMiniMap();
		return 0;
	}
	if (idwControlID == B_SYSTEM)
	{
		m_pSystemPanel->SetVisible(1);
		g_pCursor->DetachItem();

		return 1;
	}
	if (idwControlID == TMB_CAMERABTN)
	{
		SetCameraView();
		return 1;
	}
	if (idwControlID == TMB_PKBTN)
	{
		SetPK();
		return 1;
	}
	if (idwControlID == TMB_NAMEBTN)
	{
		SetVisibleNameLabel();
		return 1;
	}
	if (idwControlID == TMB_AUTOTARGETBTN)
	{
		SetAutoTarget();
		return 1;
	}
	if (idwControlID == TMB_HPOTION)
	{
		UseHPotion();
		return 1;
	}
	if (idwControlID == TMB_MPOTION)
	{
		UseMPotion();
		return 1;
	}
	if (idwControlID == TMB_PPOTION)
	{
		UsePPotion();
		return 1;
	}
	if (idwControlID == TMB_MSG_OK)
	{
		m_pMsgPanel->SetVisible(0);
		m_pControlContainer->SetFocusedControl(0);
		return 1;
	}
	if (idwControlID == B_AUTOTRADEBTN)
	{
		// The bottom-bar event can be delivered during scene bootstrap; town state
		// is meaningful only after the local human has been attached to the scene.
		if (!m_pMyHuman || !m_pMyHuman->IsInTown())
			return 1;

		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);
		else
			VisibleInputTradeName();
		return 1;
	}
	if (idwControlID == TMB_GUILDONOFF)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;

		if (m_pBtnGuildOnOff)
		{
			if(m_pBtnGuildOnOff->m_bSelected == 1)
				strcpy_s(stMsgChat.String, "guildon");
			else
				strcpy_s(stMsgChat.String, "guildoff");
		}
		else
		{
			m_cGuildOnOff = m_cGuildOnOff == 0;
			if (m_cGuildOnOff == 1)
				strcpy_s(stMsgChat.String, "guildon");
			else
				strcpy_s(stMsgChat.String, "guildoff");
		}
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgChat)->Type, reinterpret_cast<char*>(&stMsgChat), sizeof(stMsgChat)});
		return 1;
	}
	if (idwControlID == TMB_RUNMODE)
	{
		SetRunMode();
		return 0;
	}
	if (idwControlID == TMB_QUEST_BUTTON)
	{
		SelectQuestTab(0);
		return 1;
	}
	if (idwControlID == TMB_QUEST_BUTTON2)
	{
		SelectQuestTab(1);
		return 1;
	}
	if (idwControlID == TMB_QUEST_BUTTON3)
	{
		SelectQuestTab(2);
		return 1;
	}
	if (idwControlID == TMB_QUEST_BUTTON4)
	{
		SelectQuestTab(3);
		return 1;
	}
	if (idwControlID == TMB_QUEST_MEMO)
	{
		if (m_pMsgPanel && m_pMsgList && g_pDevice)
		{
			if (!LoadMsgText(m_pMsgList, (char*)"notice.txt"))
				m_pMsgList->SetVisible(1);
			m_pMsgPanel->SetVisible(1);
			m_pMsgPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pMsgPanel->m_nWidth * 0.5f),
				((float)g_pDevice->m_dwScreenHeight * 0.3f) - (m_pMsgPanel->m_nHeight * 0.3f));

			if (m_pQuestMemo)
				m_pQuestMemo->SetVisible(0);
		}
		return 1;
	}

	if (idwControlID == TMB_HELP_BUTTON1)
	{
		SelectHelpTab(0);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON2)
	{
		SelectHelpTab(1);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON3)
	{
		SelectHelpTab(2);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON4)
	{
		SelectHelpTab(3);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON5)
	{
		SelectHelpTab(4);
		return 1;
	}
	if (idwControlID == TMB_HELP_MEMO)
	{
		if (m_pHelpPanel)
		{
			m_pHelpPanel->SetVisible(1);
			m_pHelpBtn->SetSelected(1);
			OnControlEvent(873, 0);
		}
		if (m_pHelpMemo)
		m_pHelpMemo->SetVisible(0);
		return 1;
	}
	if (idwControlID == TMB_HELP_SUMMON)
	{
		char szStr[128]{};
		sprintf(szStr, g_pMessageStringTable[228], m_szSummoner);

		m_pMessageBox->SetMessage(szStr, 228u, g_pMessageStringTable[229]);
		m_pMessageBox->SetVisible(1);
		m_pHelpSummon->SetVisible(0);

		return 0;
	}
	if (idwControlID == TMB_HELP_OK)
	{
		if (m_pHelpPanel)
		{
			m_pHelpPanel->SetVisible(0);
			m_pHelpBtn->SetSelected(0);
		}

		return 0;
	}
	if (idwControlID == TMB_QUEST_QUIT)
	{
		SetQuestPanelVisible(false);
		return 0;
	}
	if (idwControlID == B_ITEMMIX_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], B_ITEMMIX_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == TMB_ITEMMIX4_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], TMB_ITEMMIX4_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == B_HELP)
	{
		m_pControlContainer->SetFocusedControl(0);
		OnKeyHelp(104, 0);

		return 0;
	}

	if (idwControlID == B_QUESTLOG || idwControlID == TMB_QUESTLOG)
	{
		if (m_pQuestPanel)
			SetQuestPanelVisible(m_pQuestPanel->IsVisible() == 0);
		return 0;
	}
	if (idwControlID == TML_QUEST_LIST)
	{
		if (m_pQuestContentList[0])
		{
			TMScene::LoadMsgText2(
				m_pQuestContentList[0],
				(char*)"UI\\QuestContents.txt",
				20 * idwEvent,
				20 * (idwEvent + 1) - 1);
		}

		return 0;
	}
	if (idwControlID == TML_QUEST_LIST2)
	{
		if (m_pQuestContentList[1])
		{
			TMScene::LoadMsgText2(
				m_pQuestContentList[1],
				(char*)"UI\\QuestContents2.txt",
				20 * idwEvent,
				20 * (idwEvent + 1) - 1);
		}

		return 0;
	}
	if (idwControlID == TML_QUEST_LIST3)
	{
		if (m_pQuestContentList[2])
		{
			TMScene::LoadMsgText2(
				m_pQuestContentList[2],
				(char*)"UI\\QuestContents3.txt",
				20 * idwEvent,
				20 * (idwEvent + 1) - 1);
		}
		return 0;
	}
	if (idwControlID == TML_QUEST_LIST4)
	{
		if (m_pQuestContentList[3])
		{
			TMScene::LoadMsgText2(
				m_pQuestContentList[3],
				(char*)"UI\\QuestContents4.txt",
				20 * idwEvent,
				20 * (idwEvent + 1) - 1);
		}
		return 0;
	}
	if (idwControlID == 5696)
	{
		if (m_nChatListSize == 3)
			m_nChatListSize = 3;
		else
			m_nChatListSize = 2;

		OnKeyPlus(43, 0);
		return 0;
	}
	if (idwControlID == 5742)
	{
		SetVisibleParty();
		return 0;
	}
	if (idwControlID == 6068)
	{
		if (m_nCurrInterfacePanelIndex > 0)
			--m_nCurrInterfacePanelIndex;
		SelectHelpTab(0);
		return 0;
	}
	if (idwControlID == 6069)
	{
		if (m_nCurrInterfacePanelIndex < 2)
			++m_nCurrInterfacePanelIndex;
		SelectHelpTab(0);
		return 0;
	}
	if (idwControlID >= 897 && idwControlID <= 900)
	{
		if (m_pQuizBG) m_pQuizBG->SetVisible(0);
		quiz_event::Answer answer{};
		if (g_pObjectManager && m_quizEvent.Respond(static_cast<unsigned short>(g_pObjectManager->m_dwCharID),
			idwControlID - 897, GetTickCount(), answer))
			SendPacket({answer.Header.Type, reinterpret_cast<char*>(&answer), sizeof(answer)});
		return 1;
	}
	if (idwControlID >= 8706 && idwControlID <= 8806)
	{
		UpdateFireWorkButton(idwControlID - 8706);
		return 1;
	}
	if (idwControlID == 8810)
	{
		ClearFireWork();
		return 1;
	}
	if (idwControlID == 8807)
	{
		if (m_pFireWorkPanel)
		{
			m_pFireWorkPanel->SetVisible(0);
			m_nFireWorkCellX = -1;
			m_nFireWorkCellY = -1;
		}
		return 1;
	}
	if (idwControlID == 8809)
	{
		if (m_pFireWorkPanel)
		{
			m_pFireWorkPanel->SetVisible(0);
			UseFireWork();
		}
		return 1;
	}
	if (idwControlID == 8964)
	{
		TotoSelect();
		return 1;
	}
	if (idwControlID == 8966)
	{
		TotoClose();
		return 1;
	}
	if (idwControlID == 8978)
	{
		TotoBuy();
		return 1;
	}
	if (idwControlID == 12291)
	{
		m_dwLastSelServer = g_pTimerManager->GetServerTime();
		MSG_SysQuit stParm{};
		stParm.Header.ID = m_pMyHuman->m_dwID;
		stParm.Header.Type = MSG_SysQuit_Opcode;
		stParm.Parm = 0;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm)->Type, reinterpret_cast<char*>(&stParm), sizeof(stParm)});
		return 1;
	}
	if (idwControlID == 12289)
	{
		SListBoxServerItem* pItem = m_pServerList ?
			static_cast<SListBoxServerItem*>(m_pServerList->GetItem(idwEvent)) : nullptr;
		if (!pItem)
			return 1;
		// FUN_004662c5 0x3001 refuses a channel above 699 users.
		if (pItem->m_nCurrent < 700)
		{
			m_nServerMove = idwEvent + 1;
			m_dwLastTeleport = dwServerTime;
			m_cLastTeleport = 1;
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[25], 4000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		return 1;
	}
	if (idwControlID == 12549)
	{
			SetVisiblePotal(0, 0);
		return 1;
	}
	if (idwControlID == 12545)
	{
		if (m_bAirmove_ShowUI == 1)
			return 1;
		if (m_stPotalItem.Header.ID)
			m_stPotalItem.ItemID = idwEvent + 1;
		else
			SetVisiblePotal(0, 0);
		return 1;
	}
	if (idwControlID == 641)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPgtPartyButton(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 643)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPgtTradeButton(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 642)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPgtGuildButton(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 863)
	{
		if (!m_pMyHuman || !m_pPGTOver || !m_pPGTPanel ||
			!m_pBtnPGTGuildDrop || !m_pBtnPGTGuildWar || !m_pBtnPGTGuildAlly || !m_pBtnPGTGuildInvite ||
			!m_pBtnPGTGICommon || !m_pBtnPGTGIChief1 || !m_pBtnPGTGIChief2 || !m_pBtnPGTGIChief3)
			return 0;
		m_pBtnPGTGuildDrop->SetVisible(0);
		m_pBtnPGTGuildWar->SetVisible(0);
		m_pBtnPGTGuildAlly->SetVisible(0);
		m_pBtnPGTGuildInvite->SetVisible(0);
		if (!m_pMyHuman)
			return 0;
		if (!m_pPGTOver)
			return 0;
		if (!m_pPGTOver->m_usGuild)
			m_pBtnPGTGICommon->SetVisible(0);

		m_pBtnPGTGICommon->SetVisible(1);
		if (m_pMyHuman->m_sGuildLevel == 9 && (m_pPGTOver->m_sGuildLevel < 3 || m_pPGTOver->m_sGuildLevel > 8) &&
			m_pMyHuman->m_usGuild == m_pPGTOver->m_usGuild)
		{
			m_pBtnPGTGIChief1->SetVisible(1);
			m_pBtnPGTGIChief2->SetVisible(1);
			m_pBtnPGTGIChief3->SetVisible(1);
		}
		return 0;
	}
	if (idwControlID == 912)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPgtGuildCommonButton(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID >= 913 && idwControlID <= 915)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPgtGuildChiefButton(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 816)
	{
		if (!m_pMessageBox || !m_pPGTPanel)
			return 0;
		m_pPGTOver = 0;
		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}
		m_pMessageBox->SetMessage(g_pMessageStringTable[36], 816u, 0);
		m_pMessageBox->m_dwArg = m_dwOpID;
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == 817)
	{
		if (!m_pMessageBox || !m_pPGTPanel)
			return 0;
		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}
		if (m_pPGTOver)
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[158], 817u, 0);
			m_pMessageBox->m_dwArg = m_pPGTOver->m_usGuild;
			m_pMessageBox->SetVisible(1);
			m_pPGTOver = 0;
		}
		return 1;
	}
	if (idwControlID == 818)
	{
		if (!m_pMessageBox || !m_pPGTPanel)
			return 0;
		m_pPGTOver = 0;

		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}

		m_pMessageBox->SetMessage(g_pMessageStringTable[159], 818u, 0);
		m_pMessageBox->m_dwArg = m_dwOpID;
		m_pMessageBox->SetVisible(1);

		return 1;
	}
	if (idwControlID == 862)
	{
		if (!m_pMessageBox || !m_pPGTPanel)
			return 0;
		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}
		if (m_pPGTOver)
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[221], 862, 0);
			m_pMessageBox->m_dwArg = m_pPGTOver->m_usGuild;
			m_pMessageBox->SetVisible(1);
			m_pPGTOver = 0;
		}
		return 1;
	}
	if (idwControlID == 644)
	{
		if (!m_pPGTPanel)
			return 0;
		m_pPGTOver = 0;
		m_pPGTPanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 668)
	{
		SetVisibleAutoTrade(0, 0);
		return 1;
	}
	if (idwControlID == 620)
	{
		if (m_pPGTPanel && m_pBtnPGTParty && m_pBtnPGTGuild && m_pBtnPGTTrade && m_pBtnPGTChallenge &&
			m_pBtnPGT1_V_1 && m_pBtnPGT5_V_5 && m_pBtnPGT10_V_10 && m_pBtnPGTAll_V_All &&
			m_pBtnPGTGICommon && m_pBtnPGTGIChief1 && m_pBtnPGTGIChief2 && m_pBtnPGTGIChief3)
		{
			m_pBtnPGTParty->SetVisible(0);
			m_pBtnPGTGuild->SetVisible(0);
			m_pBtnPGTTrade->SetVisible(0);
			m_pBtnPGTChallenge->SetVisible(0);
			m_pBtnPGT1_V_1->SetVisible(1);
			m_pBtnPGT5_V_5->SetVisible(1);
			m_pBtnPGT10_V_10->SetVisible(1);
			m_pBtnPGTAll_V_All->SetVisible(1);
			m_pPGTPanel->SetVisible(1);
			m_pBtnPGTGICommon->SetVisible(0);
			m_pBtnPGTGIChief1->SetVisible(0);
			m_pBtnPGTGIChief2->SetVisible(0);
			m_pBtnPGTGIChief3->SetVisible(0);
		}
		return 0;
	}
	if (idwControlID == 639 || idwControlID == 621 || idwControlID == 622 || idwControlID == 623)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPgtChallengeButton(idwControlID, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 1617)
	{
		MSG_MessageWhisper stMsgWhisper{};
		stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf(stMsgWhisper.MobName, "_RPS_");
		sprintf(stMsgWhisper.String, "rock");
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		m_pRPSGamePanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 1618)
	{
		MSG_MessageWhisper stMsgWhisper{};
		stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf(stMsgWhisper.MobName, "_RPS_");
		sprintf(stMsgWhisper.String, "paper");
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		m_pRPSGamePanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 1619)
	{
		MSG_MessageWhisper stMsgWhisper{};
		stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf(stMsgWhisper.MobName, "_RPS_");
		sprintf(stMsgWhisper.String, "scissor");
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		m_pRPSGamePanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 4617)
	{
		if (!idwEvent)
			return TMFieldScene::OnMsgBoxEvent(4617, 0, dwServerTime);

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);
		return 0;
	}
	if (idwControlID == 65716)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		// Point-button gates use the uint32 sidecars populated by 0x337; relying
		// on STRUCT_MOB shorts would disable allocation after 32767 points.
		if (g_pObjectManager->m_stMobData.CurrentScore.StatusPts == 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 0;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
		return 1;
	}
	if (idwControlID == 65719)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (g_pObjectManager->m_stMobData.CurrentScore.StatusPts == 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 1;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
		return 1;
	}
	if (idwControlID == 65722)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (g_pObjectManager->m_stMobData.CurrentScore.StatusPts == 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 2;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
		return 1;
	}
	if (idwControlID == 65725)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (g_pObjectManager->m_stMobData.CurrentScore.StatusPts == 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 3;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
		return 1;
	}
	if (idwControlID == 65754)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnSpecialMastery1Button(pMobData, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 65757)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnSpecialMastery2Button(pMobData, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 65760)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnSpecialMastery3Button(pMobData, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 65763)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnSpecialMastery4Button(pMobData, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 1863)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPartyListSelect(cktrans, dwServerTime, idwEvent, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 617)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnTradeMyCheck(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	if (idwControlID == 667)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnAutoTradeRun(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}

	// The newer source assumes InitBoard always created the teleport list.  The
	// 7.48 resource may omit it, so unrelated buttons must never enter portal
	// bookkeeping through a null list pointer.
	if (idwControlID == 12546 && m_pPotalList)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = OnPortalButton(extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	else if (idwControlID == 12547)
	{
		if (m_bAirmove_ShowUI == 1)
			AirMove_ShowUI(0);

		memset(&m_stPotalItem, 0, sizeof(m_stPotalItem));
		SetVisiblePotal(0, 0);
		return 1;
	}

	return 0;
}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnCompatFieldControl(unsigned int& dwServerTime, unsigned int& idwControlID, int& extractedResult)
{
	// Ghidra FUN_004662c5 dispatches these native FieldScene2 IDs before any
	// modern aliasing. Keeping their raw identity prevents close/menu clicks
	// from entering unrelated 7.59 handlers and dereferencing absent windows.
	switch (idwControlID)
	{
	case B_CCMODE_SYSTEM:
	case B_CCMODE_COMPAT_CLOSE:
		if (m_pccmode)
		{
			if (!m_pccmode->IsVisible() && m_pCC_Btn)
			{
				// Both controls share root coordinates, already scaled by SControl.
				// Re-anchor on every opening, with no gap above the launcher.
				m_pccmode->SetPos(
					m_pCC_Btn->m_nPosX + (m_pCC_Btn->m_nWidth - m_pccmode->m_nWidth) * 0.5f,
					m_pCC_Btn->m_nPosY - m_pccmode->m_nHeight);
			}
			m_pccmode->SetVisible(!m_pccmode->IsVisible());
			if (m_pCC_Btn)
				m_pCC_Btn->SetSelected(m_pccmode->IsVisible());
		}
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case B_CCMODE_DLG_HP:
		g_GameAuto_hpValue = cc_mode::NextThreshold(g_GameAuto_hpValue);
		NewCCMode();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case B_CCMODE_DLG_MOUNT:
		g_GameAuto_mountValue = cc_mode::NextThreshold(g_GameAuto_mountValue);
		NewCCMode();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case B_CCMODE_DLG_MODE:
		g_GameAuto = (g_GameAuto + 1) % 4;
		NewCCMode(true, true);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case P_CCMODE_DLG_PONT:
		m_AutoPostionUse = (m_AutoPostionUse + 1) % 3;
		NewCCMode(true, true);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 318: // native physical C.C selector
		{ extractedResult = ToggleNativeCCMode(1); return ExtractedFlow::Return; }
	case 319: // native magic C.C selector
		{ extractedResult = ToggleNativeCCMode(2); return ExtractedFlow::Return; }
	case 1375: // ItemMix1 run (Compositor)
		DoCombine();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 1376: // ItemMix1 close
		SetVisibleMixItem(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 6111: // ItemMix2 run (Aylin)
		DoCombine2();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 6146: // ItemMix3 run (Agatha)
		DoCombine3();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 6434: // ItemMix4 run (Tiny)
		DoCombine4();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 6482: // ItemMix5 run (Lindy/Odin)
		DoCombine5();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 6514: // ItemMix6 run (Ehre)
		DoCombine6();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 5744: // native main-menu button toggles flyout panel 292
		if (auto panel = m_pControlContainer->FindControl(292))
		{
			const int visible = panel->IsVisible() == 0;
			panel->SetVisible(visible);
			if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(5744)))
				button->SetSelected(visible);
		}
		{ extractedResult = 0; return ExtractedFlow::Return; }
	case 368: // native inventory/shop close button
		// FUN_004662c5 sends 0x170 to FUN_00447691, the shared closing cascade:
		// closing Inventory also closes Cargo, Shop and the other peers. Hiding
		// only the Inventory panel left Cargo open after its X.
		if (m_pInvenPanel && m_pInvenPanel->IsVisible())
			SetVisibleInventory();
		else if (m_pShopPanel && m_pShopPanel->IsVisible())
			SetVisibleShop(0);
		if (m_pDescPanel)
			m_pDescPanel->SetVisible(0);
		// Button 294 is the native 7.48 equipment toggle.  Synchronizing it
		// here prevents the menu from remaining visually pressed after X/ESC.
		if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_EQUIP)))
			button->SetSelected(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 533: // native character close button
		if (m_pCPanel)
			m_pCPanel->SetVisible(0);
		// The 7.48 Character X releases the same button selected by shortcut C.
		if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_CHAR)))
			button->SetSelected(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 633: // native System: select server
		{ extractedResult = OpenNativeServerPanel(); return ExtractedFlow::Return; }
	case 634: // native System: return to character selection
	case 635: // native System: quit game
	{
		// FieldScene2 binds 634-635 directly to the native countdowns. Do not
		// translate them to the newer System controls: that path owns 7.59
		// grids which are deliberately absent from the 7.48 layout.
		if (idwControlID == 634)
			m_dwLastLogout = dwServerTime;
		else
			g_dwStartQuitGameTime = dwServerTime;

		MSG_SysQuit stParm{};
		stParm.Header.ID = m_pMyHuman->m_dwID;
		stParm.Header.Type = MSG_SysQuit_Opcode;
		stParm.Parm = 0;
		g_pSocketManager->SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm)->Type, reinterpret_cast<char*>(&stParm), sizeof(stParm)});

		if (m_pSystemPanel)
			m_pSystemPanel->SetVisible(0);
		if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SYSTEM)))
			button->SetSelected(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case 636: // native System cancel button
		if (m_pSystemPanel)
			m_pSystemPanel->SetVisible(0);
		// Native cancel and ESC both release the bottom-bar System toggle.
		if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SYSTEM)))
			button->SetSelected(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case TMB_ATRADE_CLOSE:
		// Ghidra FUN_004662c5 routes native close 668 through the same
		// state-restoring routine as main-menu auto-trade button 313.
		SetVisibleAutoTrade(0, 0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 1913: // native skill close button
		if (m_pSkillPanel)
			m_pSkillPanel->SetVisible(0);
		if (m_pSkillMPanel)
			m_pSkillMPanel->SetVisible(0);
		if (m_pDescPanel)
			m_pDescPanel->SetVisible(0);
		// Skill and mastery are one native 7.48 window and share button 295.
		if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SKILL)))
			button->SetSelected(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case TMB_SHORTSKILL_TGL1:
	case TMB_SHORTSKILL_TGL2:
	{
		// FUN_004662c5 handles 587/588 entirely through the two native belt
		// grids and selectors. The 7.59 page text does not exist in FieldScene2.
		const bool secondPage = idwControlID == TMB_SHORTSKILL_TGL2;
		if (m_pGridSkillBelt2)
			m_pGridSkillBelt2->SetVisible(!secondPage);
		if (m_pGridSkillBelt3)
			m_pGridSkillBelt3->SetVisible(secondPage);
		if (m_pShortSkillTglBtn1)
			m_pShortSkillTglBtn1->SetSelected(!secondPage);
		if (m_pShortSkillTglBtn2)
			m_pShortSkillTglBtn2->SetSelected(secondPage);
		GetSoundAndPlay(53, 0, 0);
		m_pControlContainer->SetFocusedControl(0);
		m_bSkillBeltSwitch = secondPage ? 1 : 0;
		OnKeyShortSkill('1', 0);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}
	case TMB_AUTORUN:
		// Native button 316 and the ']' shortcut share FUN_00452733; routing
		// both here prevents a visible button that cannot change autorun state.
		{ extractedResult = OnKeyAutoRun(']', 0) ? 0 : 1; return ExtractedFlow::Return; }
	default:
		break;
	}

	// Only controls whose 7.48-to-semantic mapping is explicitly documented
	// are translated after the raw native-only cases above have been consumed.
	idwControlID = WYD748_TranslateControlID(idwControlID);
	switch (idwControlID)
	{
	case 65791: // inventory
		SetVisibleInventory();
		{ extractedResult = 0; return ExtractedFlow::Return; }
	case 65790: // character
		SetVisibleCharInfo();
		{ extractedResult = 0; return ExtractedFlow::Return; }
	case 65792: // skill
		SetVisibleSkill();
		{ extractedResult = 0; return ExtractedFlow::Return; }
	case B_AUTOTRADEBTN:
		// FUN_004662c5 routes native button 313 directly to the title prompt
		// while AutoTrade is closed and to the native close path while visible.
		if (!m_pMyHuman || !m_pMyHuman->IsInTown())
			{ extractedResult = 1; return ExtractedFlow::Return; }
		if (m_pAutoTrade && m_pAutoTrade->IsVisible())
			SetVisibleAutoTrade(0, 0);
		else
			VisibleInputTradeName();
		{ extractedResult = 1; return ExtractedFlow::Return; }
	case 65799: // party
		SetVisibleParty();
		{ extractedResult = 0; return ExtractedFlow::Return; }
	case 65793: // quest
		if (m_pQuestPanel)
			SetQuestPanelVisible(m_pQuestPanel->IsVisible() == 0);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	case 65795: // helper
		if (auto panel = m_pControlContainer->FindControl(864))
			panel->SetVisible(panel->IsVisible() == 0);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	case 65796: // system
		if (m_pSystemPanel)
		{
			// Keep visual button state coupled to panel visibility; the imported
			// 7.59 handler toggled only the panel and left a stuck pressed button.
			const int visible = m_pSystemPanel->IsVisible() == 0;
			m_pSystemPanel->SetVisible(visible);
			if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SYSTEM)))
				button->SetSelected(visible);
		}
		{ extractedResult = 0; return ExtractedFlow::Return; }
	default:
		break;
	}
	return ExtractedFlow::Next;
}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnHellStoreSelect(unsigned int& idwControlID, int& extractedResult)
{
	switch (idwControlID)
	{
	case TMB_HELLSTORE_SELECT_1:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 21, 40);
		break;
	case TMB_HELLSTORE_SELECT_2:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 41, 60);
		break;
	case TMB_HELLSTORE_SELECT_3:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 61, 80);
		break;
	case TMB_HELLSTORE_SELECT_4:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 81, 100);
		break;
	}

	m_nHellStoreValue = idwControlID;
	auto pHellgateStore = m_pHellgateStore;
	SButton* pHellSelect = nullptr;
	for (int i = 0; i < 4; ++i)
	{
		pHellSelect = (SButton*)m_pControlContainer->FindControl(i + TMB_HELLSTORE_SELECT_1);
		if (pHellSelect)
		{
			if (i + 6193 == idwControlID)
				pHellSelect->SetSelected(pHellgateStore->m_bVisible);
			else
				pHellSelect->SetSelected(0);
		}
	}
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnGambleButton(unsigned int& idwControlID, int& extractedResult)
{
	switch (idwControlID)
	{
	case TMB_GAMBLE_LEFTX2:
		m_nBet -= 10000;
		if (m_nBet < 1000)
			m_nBet = 1000;
		break;
	case TMB_GAMBLE_LEFT:
		m_nBet -= 1000;
		if (m_nBet < 1000)
			m_nBet = 1000;
		break;
	case TMB_GAMBLE_RIGHTX2:
		if (g_pObjectManager->m_stMobData.Coin - m_nBet < 10000)
			{ extractedResult = 1; return ExtractedFlow::Return; }
		m_nBet += 10000;
		if (m_nBet > 100000)
			m_nBet = 100000;
		break;
	case TMB_GAMBLE_RIGHT:
		if (g_pObjectManager->m_stMobData.Coin - m_nBet < 1000)
			{ extractedResult = 1; return ExtractedFlow::Return; }
		m_nBet += 1000;
		if (m_nBet > 100000)
			m_nBet = 100000;
		break;
	}

	auto pText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_GAMBLE_BETCOUNT));

	char szText[128]{};
	sprintf(szText, "%6d", m_nBet);
	if (pText)
		pText->SetText(szText, 0);

	sprintf(szText, "%10d", g_pObjectManager->m_stMobData.Coin);

	if (m_pMoney3)
	{
		m_pMoney3->m_cComma = 1;
		m_pMoney3->SetText(szText, 0);
	}
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnChatEditSubmit(unsigned int& dwServerTime, STRUCT_MOB*& pMobData, int& extractedResult)
{
	auto pEditChat = m_pEditChat;
	if (!strlen(pEditChat->GetText()))
	{
		if (!g_nKeyType)
		{
			m_pEditChatPanel->SetVisible(0);
			m_pChatPanel->SetVisible(1);
			m_pControlContainer->SetFocusedControl(0);
		}
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	const bool chatFlood = chat_submit::RecordSubmission(m_dwLastChatTime, dwServerTime);

	char istrText[128]{};

	if (chatFlood)
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[33], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		pEditChat->SetText((char*)"");
		m_pControlContainer->SetFocusedControl(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	chat_submit::Remember(m_szLastChatList, pEditChat->GetText());

	m_sChatIndex = 0;

	switch (chat_submit::ClassifyLocal(pEditChat->GetText(), g_pMessageStringTable[191]))
	{
	case chat_submit::LocalCommand::Clear:
	{
		pEditChat->SetText((char*)"");
		m_pControlContainer->SetFocusedControl(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case chat_submit::LocalCommand::Help:
	{
		OnKeyHelp(104, 0);
		pEditChat->SetText((char*)"");
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case chat_submit::LocalCommand::Effects:
	{
		if (g_bHideEffect)
		{
			g_bHideEffect = 0;
			g_bHideSkillBuffEffect = 0;
			g_bHideSkillBuffEffect2 = 0;
		}
		else
		{
			g_bHideEffect = 1;
			g_bHideSkillBuffEffect = 1;
			g_bHideSkillBuffEffect2 = 1;
		}

		pEditChat->SetText((char*)"");
		m_pControlContainer->SetFocusedControl(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case chat_submit::LocalCommand::Fps:
	{
		g_pDevice->m_bDrawFPS = g_pDevice->m_bDrawFPS == 0;
		pEditChat->SetText((char*)"");
		m_pControlContainer->SetFocusedControl(0);
		UpdateScoreUI(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case chat_submit::LocalCommand::Effects2:
	{
		g_pDevice->m_bShowEffects = !g_pDevice->m_bShowEffects;
		pEditChat->SetText((char*)"");
		m_pControlContainer->SetFocusedControl(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case chat_submit::LocalCommand::EffectOld:
	{
		g_bHideSkillBuffEffect = !g_bHideSkillBuffEffect;
		pEditChat->SetText((char*)"");
		m_pControlContainer->SetFocusedControl(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case chat_submit::LocalCommand::Exp:
	{
		m_bShowExp = m_bShowExp == 0;
		char str[128]{};
		if (m_bShowExp)
			sprintf(str, "Show Exp : OFF");
		else
			sprintf(str, "Show Exp : ON");

		SysMsgChat(str);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	case chat_submit::LocalCommand::None:
		break;
	}

	unsigned int idwFontColor = chat_submit::kPlainColor;

	char Chat[128]{};
	sprintf(Chat, "%s", pEditChat->GetText());
	if (!BASE_CheckChatValid(Chat))
		pEditChat->SetText(g_pMessageStringTable[1521]);

	auto pPartyList = m_pPartyList;
	auto pChatList = m_pChatList;
	const auto route = chat_submit::ClassifyPrefix(Chat);
	int idx = 0;
	switch (route.prefix)
	{
	case chat_submit::Prefix::Party:
		if (pPartyList->m_nNumItem <= 0)
		{
			pEditChat->SetText((char*)"");
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		[[fallthrough]];
	case chat_submit::Prefix::Dash:
	case chat_submit::Prefix::DoubleDash:
	case chat_submit::Prefix::At:
	case chat_submit::Prefix::DoubleAt:
		idwFontColor = route.color;
		idx = route.idx;
		InsertInChatList(pChatList, pMobData, pEditChat, idwFontColor, route.colorId, route.startIndex);
		break;
	case chat_submit::Prefix::Slash:
	{
		idx = route.idx;
		char str1[128]{};
		sscanf(Chat, "/%127s", str1);

		if (chat_submit::IsRelocate(str1, g_pMessageStringTable[234]))
		{
			if (!m_pAutoTrade->IsVisible())
			{
				sscanf(Chat, "/%127s %255s", str1, m_szSummoner2);

				m_cLastRelo = 2;
				m_dwLastRelo = g_pTimerManager->GetServerTime();

				pEditChat->SetText((char*)"");

				m_pControlContainer->SetFocusedControl(0);
				m_pEditChatPanel->SetVisible(0);
				m_pChatPanel->SetVisible(0);
			}
			{ extractedResult = 1; return ExtractedFlow::Return; }
		}

		idwFontColor = chat_submit::kCommandColor;
		// MobName has 16 bytes including NUL. Reject malformed names before
		// any member/packet copy or history mutation; truncating first also
		// changes the offset of the message body in the original input.
		if (!chat_submit::FitsWhisperName(str1))
			{ extractedResult = 1; return ExtractedFlow::Return; }

		MSG_MessageWhisper stMsgWhisper{};
		stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf(stMsgWhisper.MobName, "%s", str1);
		strcpy(m_cWhisperName, str1);

		chat_submit::TruncateCommandName(str1);
		chat_submit::Remember(m_szWhisperList, str1);

		m_sWhisperIndex = 0;
		auto pChatText = pEditChat->GetText();
		char* str = &pChatText[strlen(str1) + 1];
		if (str[0])
		{
			snprintf(stMsgWhisper.String, sizeof(stMsgWhisper.String), "%s", &pChatText[strlen(str1) + 2]);
		}

		BASE_TransCurse(stMsgWhisper.String);

		const chat_submit::CommandNames commandNames{
			g_pMessageStringTable[389], g_pMessageStringTable[386], g_pMessageStringTable[387],
			g_pMessageStringTable[391], g_pMessageStringTable[390], g_pMessageStringTable[496],
			g_pMessageStringTable[497]};
		const auto command = chat_submit::ClassifyCommand(str1, commandNames);
		switch (command)
		{
		case chat_submit::Command::SummonGuild:
		{
			if (chat_submit::IsBlockedByProgress(m_pMyHuman->m_fProgressRate))
				{ extractedResult = 1; return ExtractedFlow::Return; }

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		}
		break;
		case chat_submit::Command::Kingdom:
		{
			if (chat_submit::IsBlockedByProgress(m_pMyHuman->m_fProgressRate))
				{ extractedResult = 1; return ExtractedFlow::Return; }

			m_stLastWhisper = stMsgWhisper;
			m_cLastWhisper = 1;
			m_dwLastWhisper = g_pTimerManager->GetServerTime();
		}
		break;
		case chat_submit::Command::Speaker:
		{
			sprintf(stMsgWhisper.MobName, "%s", chat_submit::ServerKeyword(command));
			strcpy(m_cWhisperName, g_pMessageStringTable[389]);
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		}
		break;
		case chat_submit::Command::GuildCreate:
		{
			if (m_pMyHuman->m_sGuildLevel)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[370], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				{ extractedResult = 1; return ExtractedFlow::Return; }
			}
			if (!CheckGuildName(stMsgWhisper.String, 0))
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[370], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				{ extractedResult = 1; return ExtractedFlow::Return; }
			}
			sprintf(stMsgWhisper.MobName, "%s", chat_submit::ServerKeyword(command));
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		}
		break;
		case chat_submit::Command::GuildHandover:
		{
			if (!chat_submit::CanHandOverGuild(m_pMyHuman->m_sGuildLevel))
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[373], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				{ extractedResult = 1; return ExtractedFlow::Return; }
			}
			sprintf(stMsgWhisper.MobName, "%s", chat_submit::ServerKeyword(command));
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		}
		break;
		case chat_submit::Command::GuildExpel:
		case chat_submit::Command::GuildWar:
		case chat_submit::Command::ItemLock:
		case chat_submit::Command::ItemUnlock:
		{
			sprintf(stMsgWhisper.MobName, "%s", chat_submit::ServerKeyword(command));
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		}
		break;
		case chat_submit::Command::ServerQuery:
			{ extractedResult = 1; return ExtractedFlow::Return; }
		case chat_submit::Command::Nickname:
		{
			if (chat_submit::IsBlockedByProgress(m_pMyHuman->m_fProgressRate))
				{ extractedResult = 1; return ExtractedFlow::Return; }

			if (!strcmp(m_pMyHuman->m_szNickName, stMsgWhisper.String))
				{ extractedResult = 1; return ExtractedFlow::Return; }

			sprintf(g_TempNick, "%s", stMsgWhisper.String);

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
		}
		break;
		case chat_submit::Command::Whisper:
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
			break;
		}

		if (chat_submit::IsReplyAlias(str1))
		{
			sprintf(str1, g_pMessageStringTable[61]);
			sprintf(istrText, "[%s] [%s]> %s", g_pObjectManager->m_stMobData.MobName, str1, stMsgWhisper.String);
		}
		else
		{
			sprintf(istrText, "[%s] : %s> %s", g_pObjectManager->m_stMobData.MobName, str1, stMsgWhisper.String);
		}

		int len = strlen(istrText) + strlen(pMobData->MobName);
		const size_t maxLen = 55;
		if (len <= maxLen)
		{
			auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
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

			auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
			if (ipNewItem && pChatList)
				pChatList->AddItem(ipNewItem);

			auto ipNewItem2 = new SListBoxItem(dest2, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
			if (dest2[0] && ipNewItem2 && pChatList)
				pChatList->AddItem(ipNewItem2);
		}
	}
	break;
	default:
	{
		idx = 0;
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;

		strncpy_s(stMsgChat.String, pEditChat->GetText(), _TRUNCATE);

		BASE_TransCurse(stMsgChat.String);

		pEditChat->SetText((char*)"");

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgChat)->Type, reinterpret_cast<char*>(&stMsgChat), sizeof(stMsgChat)});

		size_t len = strlen(stMsgChat.String) + strlen(pMobData->MobName);
		const size_t maxLen = 40;
		if (len <= maxLen)
		{
			sprintf(istrText, "[%s]> %s", pMobData->MobName, stMsgChat.String);

			auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
			if (ipNewItem && pChatList)
				pChatList->AddItem(ipNewItem);
		}
		else
		{
			char dest[128]{};
			char dest2[128]{};
			if (IsClearString(stMsgChat.String, maxLen - 1))
			{
				strncpy(dest, stMsgChat.String, maxLen);
				dest[maxLen] = 0;
				dest[maxLen + 1] = 0;
				sprintf(dest2, "%s", &stMsgChat.String[maxLen]);
			}
			else
			{
				strncpy(dest, stMsgChat.String, maxLen - 1);
				dest2[maxLen - 1] = 0;
				sprintf(dest2, "%s", &stMsgChat.String[maxLen - 1]);
			}

			sprintf(istrText, "[%s]> %s", g_pObjectManager->m_stMobData.MobName, dest);

			auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
			if (ipNewItem && pChatList)
				pChatList->AddItem(ipNewItem);

			auto ipNewItem2 = new SListBoxItem(dest2, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
			if (strlen(stMsgChat.String) > maxLen && ipNewItem && pChatList)
				pChatList->AddItem(ipNewItem);
		}

		sprintf(istrText, "%s", stMsgChat.String);
		if (stMsgChat.String[0] == '*')
		{
			m_pMyHuman->m_dwChatDelayTime = 10000;
			sprintf(istrText, "%s", &stMsgChat.String[1]);
		}
		else
		{
			m_pMyHuman->m_dwChatDelayTime = 3000;
		}
	}
	break;
	}

	m_dwChatTime = g_pTimerManager->GetServerTime();
	if (strncmp(Chat, "+set ", 4) && !idx)
		m_pMyHuman->SetChatMessage(istrText);
	{ extractedResult = 0; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnMoneyButton(int& extractedResult)
{
	if (m_pMyHuman->m_cDie == 1)
		{ extractedResult = 1; return ExtractedFlow::Return; }

	auto pText = static_cast<SText*>(m_pControlContainer->FindControl(
		TMT_INPUT_GOLD));
	auto pEdit = m_pControlContainer->FindControl(
		TME_INPUT_GOLD);
	if (!pText || !pEdit || !m_pInputGoldPanel)
		{ extractedResult = 1; return ExtractedFlow::Return; }

	// The native 7.48 layout has no imported second Cargo page, so the shared
	// money button may inspect it only when a newer resource actually bound it.
	if (m_pCargoPanel1 && m_pCargoPanel1->IsVisible() == 1)
	{
		m_nCoinMsgType = 0;
		pText->SetText(g_pMessageStringTable[136], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		m_pInputGoldPanel->SetVisible(1);
	}
	if (m_pCargoPanel && m_pCargoPanel->IsVisible() == 1)
	{
		m_nCoinMsgType = 0;
		pText->SetText(g_pMessageStringTable[136], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		m_pInputGoldPanel->SetVisible(1);
	}
	else if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
	{
		if (g_pObjectManager->m_stTrade.TradeMoney > 0)
			{ extractedResult = 1; return ExtractedFlow::Return; }

		m_nCoinMsgType = 1;
		pText->SetText(g_pMessageStringTable[137], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		m_pInputGoldPanel->SetVisible(1);
	}
	else
	{
		m_pInputGoldPanel->SetVisible(0);
	}

	if (m_pChatSelectPanel)
		m_pChatSelectPanel->SetVisible(0);
	{ extractedResult = 0; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPgtPartyButton(int& extractedResult)
{
	if (!g_pObjectManager || !m_pMyHuman || !m_pPartyList || !m_pPGTPanel || !m_pMessagePanel)
	{
		m_pPGTOver = 0;
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(m_dwOpID);
	if (!pHuman)
	{
		m_pPGTOver = 0;
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	else if (pHuman->m_bParty)
	{
		m_pPGTOver = 0;
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	else if (m_pMyHuman->m_cDie == 1)
	{
		m_pPGTOver = 0;
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	if (!m_pPartyList->m_pItemList[0] || static_cast<SListBoxPartyItem*>(m_pPartyList->m_pItemList[0])->m_nState != 1)
	{
		MSG_REQParty stReqParty{};
		stReqParty.Header.Type = MSG_REQParty_Opcode;
		stReqParty.Header.ID = m_pMyHuman->m_dwID;
		stReqParty.Leader.Class = m_pMyHuman->m_nSkinMeshType - 1;
		stReqParty.Leader.PartyIndex = 0;
		stReqParty.Leader.Level = m_pMyHuman->m_stScore.Level;
		stReqParty.Leader.Hp = m_pMyHuman->m_stScore.CurHP;
		// PARTY keeps the native 7.48 MaxHp spelling while the canonical Score
		// supplies its uint32 value.
		stReqParty.Leader.MaxHp = m_pMyHuman->m_stScore.MaxHP;
		stReqParty.Leader.ID = m_pMyHuman->m_dwID;
		sprintf(stReqParty.Leader.Name, "%s", m_pMyHuman->m_szName);
		stReqParty.TargetID = m_dwOpID;
		SendOneMessage((char*)&stReqParty, sizeof(stReqParty));
		m_dwOpID = 0;
		m_pPGTPanel->SetVisible(0);
		m_pPGTOver = 0;
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	m_pMessagePanel->SetMessage(g_pMessageStringTable[156], 2000);
	m_pPGTOver = 0;
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPgtTradeButton(int& extractedResult)
{
	const unsigned int tradeTargetID = m_pPGTOver ? m_pPGTOver->m_dwID : m_dwOpID;
	TMHuman* pTradeTarget = m_pPGTOver;
	if (!pTradeTarget && tradeTargetID && g_pObjectManager)
		pTradeTarget = (TMHuman*)g_pObjectManager->GetHumanByID(tradeTargetID);
	int tradeDistance = -1;
	if (m_pMyHuman && pTradeTarget)
	{
		tradeDistance = BASE_GetDistance(
			(int)m_pMyHuman->m_vecPosition.x,
			(int)m_pMyHuman->m_vecPosition.y,
			(int)pTradeTarget->m_vecPosition.x,
			(int)pTradeTarget->m_vecPosition.y);
	}
	WYD748_DiagnosticsLog(
		"TRADE_CLICK target=%u opid=%u over=%p target_ptr=%p human=%p pgt=%p msg=%p obj=%p camera=%p panel=%p visible=%d distance=%d die=%d opponent=%u\r\n",
		tradeTargetID, m_dwOpID, m_pPGTOver, pTradeTarget, m_pMyHuman, m_pPGTPanel,
		m_pMessagePanel, g_pObjectManager, g_pObjectManager ? g_pObjectManager->m_pCamera : nullptr,
		m_pTradePanel, m_pTradePanel ? m_pTradePanel->IsVisible() : -1,
		tradeDistance, m_pMyHuman ? m_pMyHuman->m_cDie : -1,
		g_pObjectManager ? g_pObjectManager->m_stTrade.OpponentID : 0);
	if (!m_pTradePanel || !m_pMyHuman || !m_pPGTPanel || !m_pMessagePanel ||
		!g_pObjectManager || !g_pObjectManager->m_pCamera)
	{
		WYD748_DiagnosticsLog("TRADE_CLICK blocked=missing-control\r\n");
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	if (!pTradeTarget || tradeDistance > (int)(((float)(g_pObjectManager->m_pCamera->m_fMaxCamLen - 11.0) + 6.0)))
	{
		WYD748_DiagnosticsLog("TRADE_CLICK blocked=target-or-distance\r\n");
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	m_pPGTOver = pTradeTarget;
	if (m_pMyHuman->m_cDie == 1)
	{
		WYD748_DiagnosticsLog("TRADE_CLICK blocked=dead\r\n");
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	RECT rc;

	rc.left = 2601;
	rc.top = 1702;
	rc.right = 2652;
	rc.bottom = 1750;

	POINT pt;
	pt.x = (int)m_pMyHuman->m_vecPosition.x;
	pt.y = (int)m_pMyHuman->m_vecPosition.y;
	if (PtInRect(&rc, pt) == 1)
	{
		WYD748_DiagnosticsLog("TRADE_CLICK blocked=restricted-position\r\n");
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	auto pTradePanel = m_pTradePanel;
	if (!g_pObjectManager->m_stTrade.OpponentID || pTradePanel->IsVisible() != 1)
	{
		// Invitation packets carry no offer. Clear the reusable buffer before the
		// server parses CarryPos/Item, otherwise stale trade bytes abort the invite.
		// The interaction menu stores its target in m_pPGTOver.  m_dwOpID is
		// only the legacy message-box target and may be zero on this path.
		WYD748_ResetTradeOffer(
			g_pObjectManager->m_stTrade,
			static_cast<unsigned short>(tradeTargetID));
		g_pObjectManager->m_stTrade.Header.Type = MSG_Trade_Opcode;
		g_pObjectManager->m_stTrade.Header.ID = m_pMyHuman->m_dwID;
		WYD748_LogTradeSend("invite", g_pObjectManager->m_stTrade);
		SendOneMessage((char*)&g_pObjectManager->m_stTrade, sizeof(MSG_Trade));

		m_pPGTOver = nullptr;
		m_dwOpID = 0;
		m_pPGTPanel->SetVisible(0);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	m_pMessagePanel->SetMessage(g_pMessageStringTable[35], 2000);
	m_pMessagePanel->SetVisible(1, 1);
	WYD748_DiagnosticsLog("TRADE_CLICK blocked=trade-busy opponent=%u visible=%d\r\n",
		g_pObjectManager->m_stTrade.OpponentID, pTradePanel->IsVisible());
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPgtGuildButton(int& extractedResult)
{
	if (!m_pMyHuman || !m_pPGTOver || !m_pPGTPanel ||
		!m_pBtnPGTGuild || !m_pBtnPGTParty || !m_pBtnPGTTrade || !m_pBtnPGTChallenge ||
		!m_pBtnPGT1_V_1 || !m_pBtnPGT5_V_5 || !m_pBtnPGT10_V_10 || !m_pBtnPGTAll_V_All ||
		!m_pBtnPGTGuildDrop || !m_pBtnPGTGuildWar || !m_pBtnPGTGuildAlly || !m_pBtnPGTGuildInvite ||
		!m_pBtnPGTGICommon || !m_pBtnPGTGIChief1 || !m_pBtnPGTGIChief2 || !m_pBtnPGTGIChief3)
		{ extractedResult = 0; return ExtractedFlow::Return; }
	m_pBtnPGTGuild->SetVisible(0);
	m_pBtnPGTParty->SetVisible(0);
	m_pBtnPGTTrade->SetVisible(0);
	m_pBtnPGTChallenge->SetVisible(0);
	m_pBtnPGT1_V_1->SetVisible(0);
	m_pBtnPGT5_V_5->SetVisible(0);
	m_pBtnPGT10_V_10->SetVisible(0);
	m_pBtnPGTAll_V_All->SetVisible(0);
	m_pBtnPGTGuildDrop->SetVisible(1);
	m_pBtnPGTGuildWar->SetVisible(1);
	m_pBtnPGTGuildWar->SetVisible(0);
	m_pBtnPGTGuildAlly->SetVisible(1);

	if (m_pMyHuman && (m_pMyHuman->m_sGuildLevel >= 3 && m_pMyHuman->m_sGuildLevel <= 8 || m_pMyHuman->m_sGuildLevel == 9) &&
		(!m_pPGTOver->m_usGuild	|| m_pPGTOver->m_usGuild == m_pMyHuman->m_usGuild && (!m_pPGTOver->m_sGuildLevel || m_pPGTOver->m_sGuildLevel == 1)) &&
		m_pPGTOver->m_sGuildLevel != 9)
	{
		m_pBtnPGTGuildInvite->SetVisible(1);
	}
	m_pBtnPGTGICommon->SetVisible(0);
	m_pBtnPGTGIChief1->SetVisible(0);
	m_pBtnPGTGIChief2->SetVisible(0);
	m_pBtnPGTGIChief3->SetVisible(0);
	{ extractedResult = 0; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPgtGuildCommonButton(int& extractedResult)
{
	if (!m_pPGTOver || !m_pMessagePanel || !g_pObjectManager || !m_pMyHuman || !m_pPGTPanel)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	if (!g_pCurrentScene)
		{ extractedResult = 0; return ExtractedFlow::Return; }
	if (m_pPGTOver->m_usGuild)
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[364], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	if (g_pObjectManager->m_stMobData.Coin >= 1000000)
	{
		MSG_STANDARDPARM2 stParm2{};
		stParm2.Header.Type = MSG_InviteGuild_Opcode;
		stParm2.Header.ID = m_pMyHuman->m_dwID;
		stParm2.Parm1 = m_pPGTOver->m_dwID;

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm2)->Type, reinterpret_cast<char*>(&stParm2), sizeof(stParm2)});
		m_pPGTPanel->SetVisible(0);
		m_pPGTOver = 0;
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	m_pMessagePanel->SetMessage(g_pMessageStringTable[155], 2000);
	m_pMessagePanel->SetVisible(1, 1);
	{ extractedResult = 0; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPgtGuildChiefButton(int& extractedResult)
{
	if (!g_pCurrentScene || !m_pMessagePanel || !m_pMyHuman || !g_pObjectManager || !m_pPGTPanel)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	if (m_pPGTOver != nullptr)
	{
		if (!m_pPGTOver->m_usGuild)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[367], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		if (m_pPGTOver->m_usGuild != m_pMyHuman->m_usGuild)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[365], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		if (m_pPGTOver->m_sGuildLevel >= 3 && m_pPGTOver->m_sGuildLevel <= 8)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[366], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		if (g_pObjectManager->m_stMobData.Coin < 50000000)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[155], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
	}

	VisibleInputGuildName();
	m_pPGTPanel->SetVisible(0);
	{ extractedResult = 0; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPgtChallengeButton(unsigned int& idwControlID, int& extractedResult)
{
	if (!m_pMyHuman || !m_pPGTPanel)
		{ extractedResult = 0; return ExtractedFlow::Return; }
	if (m_pPGTOver)
	{
		MSG_STANDARDPARM2 stParm2{};
		stParm2.Header.Type = MSG_PlayerChallenge_Opcode;
		stParm2.Header.ID = m_pMyHuman->m_dwID;;
		stParm2.Parm1 = m_pPGTOver->m_dwID;

		switch (idwControlID)
		{
		case 0x27Fu:
			stParm2.Parm2 = 0;
			break;
		case 0x26Du:
			stParm2.Parm2 = 1;
			break;
		case 0x26Eu:
			stParm2.Parm2 = 2;
			break;
		case 0x26Fu:
			stParm2.Parm2 = 3;
			break;
		}

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm2)->Type, reinterpret_cast<char*>(&stParm2), sizeof(stParm2)});

		m_pPGTOver = 0;
		if (m_pPGTPanel)
			m_pPGTPanel->SetVisible(0);
	}
	{ extractedResult = 0; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnSpecialMastery1Button(STRUCT_MOB*& pMobData, int& extractedResult)
{
	if (m_pMyHuman->m_cDie == 1)
		{ extractedResult = 1; return ExtractedFlow::Return; }
	// Mastery allocation follows the same wide-counter contract as status.
	if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	int totalSpecial = 0;
	if (m_pMyHuman->Is2stClass() != 2)
	{
		totalSpecial = 3 * (pMobData->CurrentScore.Level + 1) / 2;
	}
	else if (!pMobData->Class && IsValidSkill(205) == 1)
	{
		totalSpecial = 280;
	}
	else if (pMobData->Class != 2 || IsValidSkill(233) != 1)
	{
		totalSpecial = 200;
	}
	else
	{
		totalSpecial = 230;
	}

	if (static_cast<long long>(pMobData->CurrentScore.Mastery[0]) < totalSpecial)
	{
		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 1;
		stApplyBonus.Detail = 0;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
	}
	else
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnSpecialMastery2Button(STRUCT_MOB*& pMobData, int& extractedResult)
{
	if (m_pMyHuman->m_cDie == 1)
		{ extractedResult = 1; return ExtractedFlow::Return; }
	if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	int totalSpecial = 0;
	if (m_pMyHuman->Is2stClass() == 2)
		totalSpecial = 200;
	else
		totalSpecial = 3 * (pMobData->CurrentScore.Level + 1) / 2;
	if (pMobData->Class == 3 && IsValidSkill(238) == 1)
	{
		totalSpecial = 400;
	}
	else if (IsValidSkill(200) == 1)
	{
		totalSpecial = 320;
	}
	else if (IsValidSkill(31) == 1)
	{
		totalSpecial = 255;
	}
	else if (totalSpecial > 200)
	{
		totalSpecial = 200;
	}

	if (static_cast<long long>(pMobData->CurrentScore.Mastery[1]) < totalSpecial)
	{
		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 1;
		stApplyBonus.Detail = 1;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
	}
	else
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnSpecialMastery3Button(STRUCT_MOB*& pMobData, int& extractedResult)
{
	if (m_pMyHuman->m_cDie == 1)
		{ extractedResult = 1; return ExtractedFlow::Return; }
	if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	int totalSpecial = 0;
	if (m_pMyHuman->Is2stClass() == 2)
		totalSpecial = 200;
	else
		totalSpecial = 3 * (pMobData->CurrentScore.Level + 1) / 2;
	if (IsValidSkill(204) == 1)
	{
		totalSpecial = 320;
	}
	else if (IsValidSkill(39) == 1)
	{
		totalSpecial = 255;
	}
	else if (totalSpecial > 200)
	{
		totalSpecial = 200;
	}

	if (static_cast<long long>(pMobData->CurrentScore.Mastery[2]) < totalSpecial)
	{
		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 1;
		stApplyBonus.Detail = 2;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
	}
	else
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnSpecialMastery4Button(STRUCT_MOB*& pMobData, int& extractedResult)
{
	if (m_pMyHuman->m_cDie == 1)
		{ extractedResult = 1; return ExtractedFlow::Return; }
	if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	int totalSpecial = 3 * (pMobData->CurrentScore.Level + 1) / 2;
	if (m_pMyHuman->Is2stClass() == 2)
		totalSpecial = 200;
	if (IsValidSkill(208) == 1)
	{
		totalSpecial = 320;
	}
	else if (IsValidSkill(47) == 1)
	{
		totalSpecial = 255;
	}
	else if (totalSpecial > 200)
	{
		totalSpecial = 200;
	}

	if (static_cast<long long>(pMobData->CurrentScore.Mastery[3]) < totalSpecial)
	{
		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 1;
		stApplyBonus.Detail = 3;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
	}
	else
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPartyListSelect(int& cktrans, unsigned int& dwServerTime, unsigned int& idwEvent, int& extractedResult)
{
	auto pPartyList = m_pPartyList;
	if (!pPartyList)
		{ extractedResult = 0; return ExtractedFlow::Return; }

	auto pPartyItem = (SListBoxPartyItem*)pPartyList->GetItem(idwEvent);

	if (pPartyItem && pPartyItem->m_nState == 1 && !pPartyList->m_bRButton)
	{
		MSG_CNFParty2 stCNFParty2{};
		stCNFParty2.Header.ID = m_pMyHuman->m_dwID;
		stCNFParty2.Header.Type = MSG_CNFParty2_Opcode;
		stCNFParty2.LeaderID = pPartyItem->m_dwCharID;

		sprintf(stCNFParty2.LeaderName, pPartyItem->GetText());

		auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
		if (pHuman)
			pHuman->m_bParty = 1;

		SendOneMessage((char*)&stCNFParty2, sizeof(stCNFParty2));
	}
	else if (pPartyItem	&& g_pEventTranslator->m_bCtrl == 1	&& pPartyItem->m_bSelectEnable == 1	&& !pPartyList->m_bRButton)
	{
		int charId = pPartyItem->m_dwCharID;

		if (charId >= 0 && charId < 1000)
		{
			char szText[128]{};
			sprintf(szText, pPartyItem->GetText());
			m_pMessageBox->SetMessage(szText, 50001u, 0);
			m_pMessageBox->m_dwArg = pPartyItem->m_dwCharID;
			m_pMessageBox->SetVisible(1);
		}
	}
	else if (pPartyItem && pPartyList->m_bRButton == 1)
	{
		int skillId = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
		if (IsSkillCoolingDown(skillId, dwServerTime))
			{ extractedResult = 1; return ExtractedFlow::Return; }

		int Special = m_pMyHuman->m_stScore.Level;
		int classId = skillId - 24 * g_pObjectManager->m_stMobData.Class;
		if (skillId < 96)
			Special = g_pObjectManager->m_stMobData.CurrentScore.Mastery[classId / 8 + 1];

		if (BASE_GetManaSpent(skillId, g_pObjectManager->m_stMobData.CurrentScore.SaveMana, Special) > static_cast<long long>(g_pObjectManager->m_stMobData.CurrentScore.CurMP))
		{
			auto ipNewItem = new SListBoxItem(g_pMessageStringTable[30],
				0xFFFFAAAA,
				0.0,
				0.0,
				300.0f,
				16.0f,
				0,
				0x77777777,
				1,
				0);

			auto pChatList = m_pChatList;

			if (pChatList && ipNewItem)
				pChatList->AddItem(ipNewItem);

			GetSoundAndPlay(33, 0, 0);

			{ extractedResult = 1; return ExtractedFlow::Return; }
		}

		if (dwServerTime > m_dwOldAttackTime + 1000	&& skillId == 42 && pPartyItem->m_dwCharID == m_pMyHuman->m_dwID)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[40], 1000);
			m_pMessagePanel->SetVisible(1, 1);
			{ extractedResult = 1; return ExtractedFlow::Return; }
		}

		if (dwServerTime > m_dwOldAttackTime + 1000
			&& (skillId == 25
				|| skillId == 27
				|| skillId == 42
				|| skillId == 43
				|| skillId == 44
				|| skillId == 45
				|| skillId == 13))
		{
			if (skillId != 42)
			{
				auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);

				if (!pHuman)
					{ extractedResult = 1; return ExtractedFlow::Return; }
				if (!pHuman->m_bParty)
					{ extractedResult = 1; return ExtractedFlow::Return; }

				int x1 = (int)m_pMyHuman->m_vecPosition.x;
				int y1 = (int)m_pMyHuman->m_vecPosition.y;
				if (m_stMoveStop.NextX)
				{
					x1 = m_stMoveStop.NextX;
					y1 = m_stMoveStop.NextY;
				}

				int x2 = (int)pHuman->m_vecPosition.x;
				int y2 = (int)pHuman->m_vecPosition.y;

				int distance = BASE_GetDistance(x1, y1, x2, y2);
				int range = cktrans + g_pSpell[skillId].Range;

				int ty = y2;
				int tx = x2;

				BASE_GetHitPosition(x1, y1, &tx, &ty, (char*)m_HeightMapData, 8);
				if (distance > range || tx != x2 || ty != y2)
					{ extractedResult = 1; return ExtractedFlow::Return; }

				int my_att = g_pAttribute[y1 / 4][x1 / 4];
				int other_att = g_pAttribute[y2 / 4][x2 / 4];
				if (!(my_att & 0x40))
				{
					if (other_att & 0x40)
						{ extractedResult = 1; return ExtractedFlow::Return; }
				}
			}

			MSG_Attack stAttack{};
			stAttack.Header.Type = MSG_Attack_Multi_Opcode;
			stAttack.Header.ID = m_pMyHuman->m_dwID;
			stAttack.AttackerID = m_pMyHuman->m_dwID;
			stAttack.PosX = (int)m_pMyHuman->m_vecPosition.x;
			stAttack.PosY = (int)m_pMyHuman->m_vecPosition.y;
			stAttack.CurrentMp = -1;
			stAttack.SkillIndex = skillId;
			stAttack.SkillParm = 0;
			stAttack.Motion = -1;
			stAttack.Dam[0].TargetID = pPartyItem->m_dwCharID;
			stAttack.Dam[0].Damage = -1;
			stAttack.TargetX = (int)m_pMyHuman->m_vecPosition.x;
			stAttack.TargetY = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				stAttack.PosX = m_stMoveStop.NextX;
				stAttack.TargetX = stAttack.PosX;
				stAttack.PosY = m_stMoveStop.NextY;
				stAttack.TargetY = stAttack.PosY;
			}
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAttack)->Type, reinterpret_cast<char*>(&stAttack), sizeof(stAttack)});

			MSG_Attack localAttack{};
			memcpy(&localAttack, (char*)&stAttack, sizeof(localAttack));

			localAttack.Header.ID = m_dwID;
			localAttack.FlagLocal = 1;
			if (cktrans)
				localAttack.DoubleCritical |= 8u;

			OnPacketEvent(MSG_Attack_Multi_Opcode, (char*)&localAttack);
			m_dwOldAttackTime = dwServerTime;
			m_dwSkillLastTime[skillId] = dwServerTime;
		}
	}
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnTradeMyCheck(int& extractedResult)
{
	if (!m_pTradePanel || !m_pTradePanel->IsVisible() || !m_pMyHuman ||
		!g_pObjectManager || !g_pObjectManager->m_stTrade.OpponentID ||
		!g_pApp || !g_pApp->m_pTimerManager)
		{ extractedResult = 1; return ExtractedFlow::Return; }

	if (m_dwLastCheckTime + 2000 <= g_pApp->m_pTimerManager->GetServerTime())
	{
		auto pButton = (SButton*)m_pControlContainer->FindControl(617u);
		if (pButton)
		{
			pButton->m_bSelected = pButton->m_bSelected == 0;
			g_pObjectManager->m_stTrade.MyCheck = pButton->m_bSelected;
		}
		else
		{
			g_pObjectManager->m_stTrade.MyCheck = !g_pObjectManager->m_stTrade.MyCheck;
		}

		MSG_Trade stTrade{};

		memcpy(&stTrade, &g_pObjectManager->m_stTrade, sizeof(stTrade));
		stTrade.Header.ID = m_pMyHuman->m_dwID;
		stTrade.Header.Type = MSG_Trade_Opcode;
		WYD748_LogTradeSend("confirm", stTrade);
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stTrade)->Type, reinterpret_cast<char*>(&stTrade), sizeof(stTrade)});

		m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	if (m_pMessagePanel)
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[41], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnAutoTradeRun(int& extractedResult)
{
	if (!field_interaction::HasAutoTradeOffers(m_stAutoTrade.Item))
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[145], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	auto pButtonRun = (SButton*)m_pControlContainer->FindControl(667u);
	m_stAutoTrade.Header.Type = 919;
	m_stAutoTrade.TargetID = m_pMyHuman->m_dwID;
	SendOneMessage((char*)&m_stAutoTrade, sizeof(m_stAutoTrade));

	auto pButtonCancel = (SButton*)m_pControlContainer->FindControl(668u);
	if (pButtonRun)
		pButtonRun->SetVisible(0);
	if (pButtonCancel)
		pButtonCancel->SetVisible(1);

	auto pAutoTrade =  m_pAutoTrade;
	if (pAutoTrade && pAutoTrade->IsVisible() == 1)
	{
		// FUN_004662c5 hides each available 7.48 Cargo surface independently;
		// publishing AutoTrade cannot depend on the absent 7.59 second page.
		if (m_pCargoPanel)
			m_pCargoPanel->SetVisible(0);
		if (m_pCargoPanel1)
			m_pCargoPanel1->SetVisible(0);

		pAutoTrade->SetRealPos((float)g_pDevice->m_dwScreenWidth - pAutoTrade->m_nWidth, 0.0f);
	}

	{ extractedResult = 1; return ExtractedFlow::Return; }

}

// Extracted from TMFieldScene::OnControlEvent; behavior is unchanged.
ExtractedFlow TMFieldScene::OnPortalButton(int& extractedResult)
{
	if (m_bAirmove_ShowUI == 1)
	{
		auto pItem = m_pPotalList->GetSelectedIndex();

		if (pItem + 1 != 0 && !m_bAirMove)
			AirMove_Start(pItem);

		AirMove_ShowUI(0);
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}

	if (!m_stPotalItem.ItemID)
	{
		m_stPotalItem.ItemID = m_pPotalList->GetSelectedIndex() + 1;
	}
	if (m_stPotalItem.Header.ID)
	{
		// Portal consumables are Carry slots.  Use the native 9x7 transform
		// in compatibility mode so consuming a ticket removes the same item.
		int nSourRes = 0;
		int nSourDiv = 0;
		GetCarryCellForSlot(m_stPotalItem.SourPos, nSourRes, nSourDiv);
		auto pGridInvList = GetCarryGridForSlot(m_stPotalItem.SourPos);
		if (!pGridInvList)
			{ extractedResult = 1; return ExtractedFlow::Return; }
		auto pItem = pGridInvList->GetItem(nSourRes, nSourDiv);

		if (pItem)
		{
			int amount = BASE_GetItemAmount(pItem->m_pItem);
			if (amount > 1)
			{
				BASE_SetItemAmount(pItem->m_pItem, amount - 1);
				sprintf(pItem->m_GCText.strString, "%2d", amount - 1);
				pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
			}
			else
			{
				auto pPickItem = pGridInvList->PickupItem(nSourRes, nSourDiv);

				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pPickItem);
			}

			if (amount <= 1)
			{
				if (!m_stPotalItem.SourType)
				{
					memset(&g_pObjectManager->m_stMobData.Equip[m_stPotalItem.SourPos], 0, sizeof(STRUCT_ITEM));
				}
				else if (m_stPotalItem.SourType == 1)
				{
					memset(&g_pObjectManager->m_stMobData.Carry[m_stPotalItem.SourPos], 0, sizeof(STRUCT_ITEM));
				}
				else if (m_stPotalItem.SourType == 2)
				{
					memset(&g_pObjectManager->m_stItemCargo[m_stPotalItem.SourPos], 0, sizeof(STRUCT_ITEM));
				}
			}

			m_dwGetItemTime = g_pTimerManager->GetServerTime();
			m_dwLastTeleport = m_dwGetItemTime;
			m_cLastTeleport = 1;

			auto vec = m_pMyHuman->m_vecPosition;

			memset(&m_stUseItem, 0, sizeof(m_stUseItem));
			m_stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
			m_stUseItem.Header.Type = m_stPotalItem.Header.Type;
			m_stUseItem.SourType = m_stPotalItem.SourType;
			m_stUseItem.SourPos = m_stPotalItem.SourPos;
			m_stUseItem.ItemID = m_stPotalItem.ItemID;
			m_stUseItem.GridX = (int)vec.x;
			m_stUseItem.GridY = (int)vec.y;

			memset((char*)&m_stPotalItem, 0, 0x24u);

			MSG_DelayStart stParm{};
			stParm.Header.ID = m_pMyHuman->m_dwID;
			stParm.Header.Type = MSG_DelayStart_Opcode;
			stParm.Parm = 1;
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm)->Type, reinterpret_cast<char*>(&stParm), sizeof(stParm)});
		}
	}

	SetVisiblePotal(0, 0);
	{ extractedResult = 1; return ExtractedFlow::Return; }

}


int TMFieldScene::OnCharEvent(char iCharCode, int lParam)
{
	if (m_bAirMove == 1)
		return 0;

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

	// FieldScene2 does not create the full message-box graph used by the
	// newer source tree.  Keyboard input must remain safe while compatibility
	// mode is active instead of dereferencing an absent optional control.
	if (m_pMessageBox && m_pMessageBox->IsVisible() == 1
		&& (m_pMessageBox->m_dwMessage == 601 || m_pMessageBox->m_dwMessage == MSG_PlayerChallenge_Opcode)
		&& iCharCode == 13)
	{
		return 1;
	}

	if (g_nKeyType == 1)
	{
		if (iCharCode == 27)
		{
			OnESC();
			return 1;
		}

		if (TMScene::OnCharEvent(iCharCode, lParam) == 1)
			return 1;
	}
	else
	{
		if (TMScene::OnCharEvent(iCharCode, lParam) == 1)
			return 1;

		if (iCharCode == 27)
		{
			OnESC();
			return 1;
		}
	}

	if (m_bCriticalError == 1)
		return 1;

	if (OnKeyTotoTab(iCharCode, lParam))
		return 1;

	if (OnKeyCamView(iCharCode, lParam))
		return 1;

	if (g_nKeyType)
		return 0;

	if (iCharCode == 'a' || iCharCode == 'A')
		return ToggleNativeCCMode(1);
	if (iCharCode == 'd' || iCharCode == 'D')
		return ToggleNativeCCMode(2);

	if (OnKeyTotoEnter(iCharCode, lParam))
		return 1;

	if (OnKeySkill(iCharCode, lParam))
		return 1;

	if (OnKeyDash(iCharCode, lParam))
		return 1;

	if (OnKeyPlus(iCharCode, lParam))
		return 1;

	if (OnKeyPK(iCharCode, lParam))
		return 1;

	if (OnKeyName(iCharCode, lParam))
		return 1;

	if (OnKeyAutoTarget(iCharCode, lParam))
		return 1;

	if (OnKeyHelp(iCharCode, lParam))
		return 1;

	if (OnKeyRun(iCharCode, lParam))
		return 1;

	if (OnKeyFeedMount(iCharCode, lParam))
		return 1;

	if (g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0)
	{
		if (OnKeyAuto(iCharCode, lParam))
			return 1;

		if (OnKeyHPotion(iCharCode, lParam))
			return 1;

		if (OnKeyMPotion(iCharCode, lParam))
			return 1;

		if (OnKeyPPotion(iCharCode, lParam))
			return 1;

		if (OnKeySkillPage(iCharCode, lParam))
			return 1;

		if (OnKeyQuestLog(iCharCode, lParam))
			return 1;

		if (OnKeyAutoRun(iCharCode, lParam))
			return 1;

		if (OnKeyGuildOnOff(iCharCode, lParam))
			return 1;

		if (OnKeyShortSkill(iCharCode, lParam))
			return 1;
	}

	if (OnKeyVisibleSkill(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleInven(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleCharInfo(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleMinimap(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleParty(iCharCode, lParam))
		return 1;

	if (OnKeyReturn(iCharCode, lParam))
		return 1;

	return 0;
}

int TMFieldScene::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	// FieldScene2 (7.48) input goes through the byte-compatible field adapter.
	return OnMouseEventCompat(dwFlags, wParam, nX, nY);
}

int TMFieldScene::OnPacketEvent(unsigned int dwCode, char* buf)
{
	// The 7.48 compatibility layer changes resource bindings, not gameplay
	// protocol.  Dispatch every proven opcode through the normal handler table;
	// dropping the default cases here disabled inventory, stats, shops and NPCs.
	if (TMScene::OnPacketEvent(dwCode, buf) == 1)
		return 1;
	if (buf == nullptr)
		return 1;

	auto pStd = (MSG_STANDARD*)buf;

	switch (pStd->Type)
	{
	case MSG_CreateMob_Opcode:
	case MSG_CreateMobTrade_Opcode:
		// The 7.48 STRUCT_MOB materializer is byte-compatible and intentionally
		// avoids the optional 7.59 HUD controls during entity creation.
		return OnPacketCreateMobCompat(pStd);
	case MSG_Action_Opcode:
	case MSG_Action_Stop_Opcode:
		return OnPacketAction(pStd);
	case MSG_REQArray_Opcode:
		return OnPacketREQArray(pStd);
	case 0x333:
		return OnPacketMessageChat(reinterpret_cast<MSG_MessageChat*>(pStd));
	case 0x105:
		return OnPacketMessageChat_Index(reinterpret_cast<MSG_MessageChat*>(pStd));
	case 0x334:
		return OnPacketMessageWhisper(reinterpret_cast<MSG_MessageWhisper*>(pStd));
	case 0x301:
		return OnPacketNewBuyCash(pStd);
	case 0x7B1:
		return OnPacketLongMessagePanel(reinterpret_cast<MSG_LongMessagePanel*>(pStd));
	case 0x3B2:
		return OnPacketReqSummon(reinterpret_cast<MSG_ReqSummon*>(pStd));
	case 0x3B3:
		return OnPacketCancelSummon(pStd);
	case 0x3A3:
		return OnPacketSoundEffect(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case MSG_CNFCharacterLogout_Opcode:
		return OnPacketCNFCharacterLogout(pStd);
	case MSG_CNFRemoveServer_Opcode:
		return OnPacketCNFRemoveServer(reinterpret_cast<MSG_CNFRemoveServer*>(pStd));
	case 0x10A:
		// Opcode 0x10A is the normal account-login confirmation here. Casting it
		// as the channel-migration aggregate shifts SelChar by sixteen bytes.
		return OnPacketCNFAccountLogin(reinterpret_cast<MSG_CNFAccountLogin*>(pStd));
	case 0x114:
		return OnPacketCNFCharacterLogin(reinterpret_cast<MSG_CNFCharacterLogin*>(pStd));
	case MSG_ItemSold_Opcode:
		return OnPacketItemSold(reinterpret_cast<MSG_STANDARDPARM2*>(pStd));
	case MSG_UpdateCargoGold_Opcode:
		return OnPacketUpdateCargoCoin(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case MSG_Weather_Opcode:
		return OnPacketWeather(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case MSG_CreateItem_Opcode:
		return OnPacketCreateItem(reinterpret_cast<MSG_CreateItem*>(pStd));
	case MSG_CNFDropItem_Opcode:
		return OnPacketCNFDropItem(reinterpret_cast<MSG_CNFDropItem*>(pStd));
	case MSG_CNFGetItem_Opcode:
		return OnPacketCNFGetItem(reinterpret_cast<MSG_CNFGetItem*>(pStd));
	case MSG_UpdateItem_Opcode:
		return OnPacketUpdateItem(reinterpret_cast<MSG_UpdateItem*>(pStd));
	case MSG_RemoveItem_Opcode:
		return OnPacketRemoveItem(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case 0x1D0:
		g_pObjectManager->m_RMBShopOpen = 1;
		return OnPacketShopList(pStd);
		break;
	case 0x379:
		return OnPacketBuy(pStd);
		break;
	case MSG_CNFMobKill_Opcode:
		return OnPacketCNFMobKill(reinterpret_cast<MSG_CNFMobKill*>(pStd));
		break;
	case MSG_REQParty_Opcode:
		return OnPacketREQParty(reinterpret_cast<MSG_REQParty*>(pStd));
		break;
	case 0x37D:
		return OnPacketAddParty(reinterpret_cast<MSG_AddParty*>(pStd));
		break;
	case MSG_RemoveParty_Opcode:
		return OnPacketRemoveParty(reinterpret_cast<MSG_RemoveParty*>(pStd));
		break;
	case 0x292:
		return OnPacketSetHpMode(reinterpret_cast<MSG_SetHpMode*>(pStd));
		break;
	case 0x18D:
		return OnPacketReqChallange(pStd);
		break;
	case 0x378:
		return OnPacketSetShortSkill(reinterpret_cast<MSG_SetShortSkill*>(pStd));
		break;
	case 0x19C:
		return OnPacketClearMenu(pStd);
		break;
	case MSG_CombineComplete_Opcode:
		return OnPacketCombineComplete(pStd);
		break;
	case 0x3AC:
		return OnPacketCastleState(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case MSG_InstanceTime_Opcode:
		return OnPacketStartTime(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case MSG_InstanceMobs_Opcode:
		return OnPacketRemainCount(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3A8:
		return OnPacketWarInfo(reinterpret_cast<MSG_STANDARDPARM3*>(pStd));
		break;
	case 0x3A4:
		return OnPacketGuildDisable(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3A2:
		return OnPacketEnvEffect(pStd);
		break;
	case 0x3BB:
		return OnPacketRemainNPCCount(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case MSG_ResultGamble_Opcode:
		return OnPacketRESULTGAMBLE(reinterpret_cast<MSG_ResultGamble*>(pStd));
		break;
	case MSG_AutoTrade_Opcode:
		return OnPacketAutoTrade(pStd);
	case MSG_SwapItem_Opcode:
		return OnPacketSwapItem(pStd);
	case MSG_ShopList_Opcode:
		return OnPacketShopList(pStd);
	case MSG_Sell_Opcode:
		return OnPacketSell(pStd);
	case MSG_Deposit_Opcode:
		return OnPacketDeposit(pStd);
	case MSG_Withdraw_Opcode:
		return OnPacketWithdraw(pStd);
	case MSG_CloseShop_Opcode:
		return OnPacketCloseShop(pStd);
	case MSG_Attack_Multi_Opcode:
	case MSG_Attack_One_Opcode:
	case MSG_Attack_Two_Opcode:
		return OnPacketAttack(pStd);
	case quiz_event::ChallengeOpcode:
		return OnPacketQuizEvent(pStd);
		break;
	case premium_shop::StateOpcode:
		return OnPacketPremiumShopState(pStd);
	case 0x1C2:
		return OnPacketItemPrice(reinterpret_cast<MSG_STANDARDPARM2*>(pStd));
		break;
	case MSG_CapsuleInfo_Opcode:
		return OnPacketCapsuleInfo(reinterpret_cast<MSG_CAPSULEINFO*>(pStd));
		break;
	case 0x3CF:
		return OnPacketRunQuest12Start(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3D0:
		return OnPacketRunQuest12Count(reinterpret_cast<MSG_STANDARDPARM2*>(pStd));
		break;
	case MSG_SysQuit_Opcode:
		return OnPacketDelayQuit(reinterpret_cast<MSG_SysQuit*>(pStd));
		break;

	case 0x2132:
		return OnPacketInforPlay(reinterpret_cast<MSG_SendInfoPlay*>(pStd));
		break;

	case 0x457:
		return OnPacketBattle((MSG_TowerWar*)pStd);
		break;

	case 0x755:
		return OnPacketMacroWater(reinterpret_cast<stWaterScrollMacro*>(pStd));
		break;

	case 0x5000:
		return OnPacketSendExpMsg(reinterpret_cast<MSG_Exp_MsgPanel*>(pStd));
		break;

	//case 0x671:
	//	return DailyRewardInfoPacket((MSG_DAILYREWARDINFO*)pStd);
	//	break;

	}

	return 0;
}

int TMFieldScene::FrameMove(unsigned int dwServerTime)
{
	if (g_pObjectManager && g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0)
		m_bRespawnPromptOffered = false;
	auto recallDeadPlayer = [&](unsigned int now)
	{
		if (!m_pMyHuman || !g_pObjectManager ||
			!death_motion::ShouldAutoRecallDeadPlayer(m_dwLastDeadTime, now,
				m_pMyHuman->m_cDie == 1,
				static_cast<int>(m_pMyHuman->m_vecPosition.x),
				static_cast<int>(m_pMyHuman->m_vecPosition.y)))
			return;

		MSG_STANDARD request{};
		request.ID = g_pObjectManager->m_dwCharID;
		request.Type = MSG_Recall_Opcode;
		SendOneMessage(reinterpret_cast<char*>(&request), sizeof(request));
		m_dwLastDeadTime = 0;
	};
	// The compact 7.48 path returns before the regular HUD tick. Close a stale
	// respawn prompt for both paths as soon as authoritative HP is restored.
	if (m_pMessageBox && g_pObjectManager &&
		m_pMessageBox->m_dwMessage == 11 && m_pMessageBox->IsVisible() == 1 &&
		g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0)
	{
		m_pMessageBox->SetVisible(0);
	}

	// The 7.48 FieldScene2 HUD tick. The base scene advances terrain
	// streaming, camera and child objects.
	if (!g_pTimerManager)
		return 0;
	dwServerTime = g_pTimerManager->GetServerTime();
	// A nearby BerialGuard refreshes its aura every rendered frame. Once none
	// does, return to the ground's own light state and reapply the weather the
	// server sent last (also while the aura was active).
	if (m_berielAura.Release(dwServerTime))
	{
		if (m_pGround)
			RenderDevice::m_bDungeon = m_pGround->m_bDungeon;
		if (m_pMyHuman)
			SetWeather(m_berielAura.server);
		else
			g_nWeather = m_berielAura.Effective();
	}
	// Consume the last completed entity hover pass before the base scene
	// clears m_pMouseOverHuman and submits controls for this frame.
	UpdateCompatObservedAffects();
	if (m_pMyHuman && m_pMiniMapPanel && m_pMiniMapPanel->IsVisible())
	{
		if (m_pPositionText)
		{
			char position[64]{};
			sprintf_s(position, "X: %d Y: %d", static_cast<int>(m_pMyHuman->m_vecPosition.x),
				static_cast<int>(m_pMyHuman->m_vecPosition.y));
			m_pPositionText->SetText(position, 0);
		}
		if (m_pMiniMapDir)
			m_pMiniMapDir->GetGeomControl()->fAngle = m_pMyHuman->m_fAngle - 2.3561945f;
	}
	TMScene::FrameMove(dwServerTime);
	UpdateGambleRequestTimeout();
	UpdateInstanceTimerHud(dwServerTime);
	// Native field lifecycle advances the five-second quit/logout/server-change
	// countdown immediately after the base scene.  Keep that proven slice even
	// while unsafe 7.59-only HUD ticks remain excluded from the compact path.
	if (TimeDelay(dwServerTime) == 1)
		return 1;
	// FUN_004776c3 invokes FUN_0047ef1d immediately after TimeDelay.  The
	// source implementation is the same guarded 7.48 air-move state machine;
	// omitting it leaves a completed teleport transition permanently pending.
	AirMove_Main(dwServerTime);
	// The mesh may reject the death clip, so its completion callback cannot be
	// the only way to open the native respawn prompt in the compact scene.
	if (m_pMyHuman && m_pMessageBox && g_pObjectManager)
	{
		POINT position{};
		position.x = static_cast<int>(m_pMyHuman->m_vecPosition.x);
		position.y = static_cast<int>(m_pMyHuman->m_vecPosition.y);
		if (death_motion::ShouldOfferTimedRespawnPrompt(m_dwLastDeadTime,
			dwServerTime, g_pObjectManager->m_stMobData.CurrentScore.CurHP,
			m_pMyHuman->m_cDie == 1, m_pMyHuman->m_sFamCount != 0,
			m_pMyHuman->IsInTown() != 0,
			PtInRect(&rectTownInCastle, position) == 1,
			m_pMessageBox->IsVisible() != 0))
		{
			OfferRespawnPrompt(false);
		}
	}
	recallDeadPlayer(dwServerTime);
	// The field lifecycle must also advance the stock affect row.  Returning
	// before this call left every 0x3B9 duration entry permanently invisible,
	// even though InitializeCompatFieldScene created the native 7.48 icons.
	Affect_Main(dwServerTime);
	UpdateScoreButtonRepeat(dwServerTime);
	UpdateCompatStatusTexts();
	if (UpdateTeleportPrompt() == 1)
		return 1;
	// The 7.48 HUD has its own lifecycle, but CC still needs the existing
	// item/skill/attack scheduler after transition gates, once per frame.
	GameAuto();
	UpdateSkillCooldownUI(dwServerTime);
	return 1;
}

int TMFieldScene::TimeDelay(unsigned int dwServerTime)
{
	if (m_quizEvent.Expired(GetTickCount()))
	{
		m_quizEvent.Active = false;
		if (m_pQuizBG) m_pQuizBG->SetVisible(0);
	}
	if (g_dwStartQuitGameTime)
	{
		if (dwServerTime > g_dwStartQuitGameTime + 5000)
		{
			PostMessage(g_pApp->m_hWnd, 16, 0, 0);
			return 1;
		}

		unsigned int dwRemain = (g_dwStartQuitGameTime + 5000 - dwServerTime) / 1000;
		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[224], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
	}
	if (m_dwLastLogout)
	{
		if (dwServerTime > m_dwLastLogout + 5000)
		{
			MSG_CharacterLogout stStandard{};
			stStandard.ID = g_pObjectManager->m_dwCharID;
			stStandard.Type = MSG_CharacterLogout_Opcode;
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stStandard)->Type, reinterpret_cast<char*>(&stStandard), sizeof(stStandard)});
			m_dwLastLogout = 0;
			return 1;
		}

		unsigned int dwRemain = (m_dwLastLogout + 5000 - dwServerTime) / 1000;
		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[225], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
	}
	if (m_dwLastSelServer)
	{
		if (dwServerTime > m_dwLastSelServer + 5000)
		{
			m_dwLastSelServer = 0;
			g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
			return 1;
		}

		unsigned int dwRemain = (m_dwLastSelServer + 5000 - dwServerTime) / 1000;
		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[224], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
	}
	if (m_dwLastTown)
	{
		if (dwServerTime > m_dwLastTown + 6000 && !m_cLastTown)
			m_dwLastTown = 0;
		else if (dwServerTime > m_dwLastTown + 5000 && m_cLastTown == 1)
		{
			MSG_STANDARD stRecall{};
			stRecall.ID = m_pMyHuman->m_dwID;
			stRecall.Type = MSG_Recall_Opcode;
			SendOneMessage((char*)&stRecall, sizeof(stRecall));

			m_cResurrect = 0;
			// The request does not revive the player. Keep the death state until
			// the server confirms positive HP through SetHpMp; otherwise a lost or
			// rejected recall suppresses the dead-player fallback.

			if (!m_pMyHuman->m_cHide && m_pEffectContainer)
			{
				m_pEffectContainer->AddChild(new TMEffectLevelUp(
					TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight,
						m_pMyHuman->m_vecPosition.y), 0));
			}

			m_cLastTown = 0;
		}
		else if (death_motion::ShouldAdvanceRespawnRecallCountdown(
			m_dwLastTown, dwServerTime, m_cLastTown == 1))
		{
			unsigned int dwRemain = death_motion::RespawnRecallSecondsRemaining(
				m_dwLastTown, dwServerTime);
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;

				if (m_pEffectContainer)
					m_pEffectContainer->AddChild(new TMSkillTownPortal(
						TMVector3(m_pMyHuman->m_vecPosition.x,
							m_pMyHuman->m_fHeight + 0.05f,
							m_pMyHuman->m_vecPosition.y), 1));
			}
		}
	}
	if (m_dwLastResurrect)
	{
		MSG_AttackOne stAttack{};
		stAttack.Header.Type = MSG_Attack_One_Opcode;
		stAttack.Header.ID = m_pMyHuman->m_dwID;
		stAttack.AttackerID = m_pMyHuman->m_dwID;
		stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
		stAttack.Dam[0].Damage = -1;
		stAttack.PosX = (int)m_pMyHuman->m_vecPosition.x;
		stAttack.PosY = (int)m_pMyHuman->m_vecPosition.y;
		stAttack.TargetX = (int)m_pMyHuman->m_vecPosition.x;
		stAttack.TargetY = (int)m_pMyHuman->m_vecPosition.y;
		if (m_stMoveStop.NextX)
		{
			stAttack.PosX = m_stMoveStop.NextX;
			stAttack.TargetX = stAttack.PosX;
			stAttack.PosY = m_stMoveStop.NextY;
			stAttack.TargetY = stAttack.PosY;
		}
		stAttack.CurrentMp = -1;
		stAttack.SkillIndex = 99;
		stAttack.SkillParm = 0;
		stAttack.Motion = -1;

		if (dwServerTime > m_dwLastResurrect + 6000 && !m_cResurrect)
			m_dwLastResurrect = 0;
		else if (dwServerTime > m_dwLastResurrect + 5000 && m_cResurrect == 1)
		{
			m_cResurrect = 0;
			stAttack.FlagLocal = 0;
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAttack)->Type, reinterpret_cast<char*>(&stAttack), sizeof(stAttack)});
		}
		else
		{
			unsigned int dwRemain = (m_dwLastResurrect + 5000 - dwServerTime) / 2000;
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;
				stAttack.FlagLocal = 1;

				OnPacketEvent(stAttack.Header.Type, (char*)&stAttack);
			}
		}
	}
	if (m_dwLastTeleport)
	{
		if (dwServerTime > m_dwLastTeleport + 6000 && !m_cLastTeleport)
			m_dwLastTeleport = 0;
		else if (dwServerTime > m_dwLastTeleport + 5000 && m_cLastTeleport == 1)
		{
			if (m_nServerMove)
			{
				m_dwDelayDisconnectTime = dwServerTime;

				MSG_MessageWhisper stWhisper{};
				stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
				stWhisper.Header.Type = MSG_MessageWhisper_Opcode;
				sprintf(stWhisper.MobName, "srv");
				sprintf(stWhisper.String, "%d", m_nServerMove);
				SendOneMessage((char*)&stWhisper, sizeof(stWhisper));
				m_nServerMove = 0;
			}
			else
			{
				SendOneMessage((char*)&m_stUseItem, sizeof(m_stUseItem));
				memset(&m_stUseItem, 0, sizeof(m_stUseItem));
				m_dwUseItemTime = dwServerTime;
			}
			m_cLastTeleport = 0;
		}
		else
		{
			unsigned int dwRemain = (m_dwLastTeleport + 5000 - dwServerTime) / 1000;
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;

				if (m_pEffectContainer)
					m_pEffectContainer->AddChild(new TMSkillTownPortal(
						TMVector3(m_pMyHuman->m_vecPosition.x,
							m_pMyHuman->m_fHeight + 0.05f,
							m_pMyHuman->m_vecPosition.y), 1));
			}
		}
	}
	if (m_dwLastRelo)
	{
		if (dwServerTime > m_dwLastRelo + 6000 && !m_cLastRelo)
			m_dwLastRelo = 0;
		else if (dwServerTime > m_dwLastRelo + 5000 && m_cLastRelo == 1)
		{
			MSG_MessageWhisper stMsgImp{};
			stMsgImp.Header.ID = g_pObjectManager->m_dwCharID;
			stMsgImp.Header.Type = MSG_MessageWhisper_Opcode;
			sprintf(stMsgImp.MobName, "relo");
			sprintf(stMsgImp.String, "%s", m_szSummoner);
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgImp)->Type, reinterpret_cast<char*>(&stMsgImp), sizeof(stMsgImp)});
			m_cLastRelo = 0;
		}
		else if (dwServerTime > m_dwLastRelo + 5000 && m_cLastRelo == 2)
		{
			MSG_MessageWhisper stMsgImp{};
			stMsgImp.Header.ID = g_pObjectManager->m_dwCharID;
			stMsgImp.Header.Type = MSG_MessageWhisper_Opcode;
			sprintf(stMsgImp.MobName, "relo");
			sprintf(stMsgImp.String, "%s", m_szSummoner2);
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgImp)->Type, reinterpret_cast<char*>(&stMsgImp), sizeof(stMsgImp)});
			m_cLastRelo = 0;
		}
		else
		{
			unsigned int dwRemain = (m_dwLastRelo + 5000 - dwServerTime) / 1000;
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;

				if (m_pEffectContainer)
					m_pEffectContainer->AddChild(new TMSkillTownPortal(
						TMVector3(m_pMyHuman->m_vecPosition.x,
							m_pMyHuman->m_fHeight + 0.05f,
							m_pMyHuman->m_vecPosition.y), 1));
			}
		}
	}

	if (m_dwLastWhisper)
	{
		if (dwServerTime > m_dwLastWhisper + 6000 && !m_cLastWhisper)
		{
			m_dwLastWhisper = 0;
			return 0;
		}
		if (dwServerTime > m_dwLastWhisper + 5000 && m_cLastWhisper == 1)
		{
			SendOneMessage((char*)&m_stLastWhisper, sizeof(m_stLastWhisper));
			memset((char*)&m_stLastWhisper, 0, sizeof(m_stLastWhisper));
			m_dwLastWhisper = 0;
			return 1;
		}
		// Continue to the button handling below.

		unsigned int dwRemain = (m_dwLastWhisper + 5000 - dwServerTime) / 1000;

		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[223], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
	}

	return 0;
}

int TMFieldScene::OnMsgBoxEvent(unsigned int idwControlID, unsigned int idwEvent, unsigned int dwServerTime)
{
	if (!m_pMessageBox || !m_pMyHuman || !m_pControlContainer || !g_pObjectManager)
		return 1;

	switch (m_pMessageBox->m_dwMessage)
	{
	case 601:
	{
		if (!m_pTradePanel)
			return 1;

		auto pNode = g_pObjectManager->GetHumanByID(m_pMessageBox->m_dwArg);
		if (!pNode)
		{
			if (m_pMessagePanel)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[37], 2000);
				m_pMessagePanel->SetVisible(1, 1);
			}
			return 1;
		}

		// The received invitation is opponent state, not our local offer. Build an
		// empty acceptance before sending so stale Item/CarryPos bytes cannot be
		// interpreted as an offer and prevent the trade window from opening.
		WYD748_ResetTradeOffer(g_pObjectManager->m_stTrade,
			static_cast<unsigned short>(m_pMessageBox->m_dwArg));

		MSG_Trade stTrade{};
		memcpy(&stTrade, &g_pObjectManager->m_stTrade, sizeof(stTrade));
		stTrade.Header.ID = m_pMyHuman->m_dwID;
		stTrade.Header.Type = MSG_Trade_Opcode;
		WYD748_LogTradeSend("accept", stTrade);
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stTrade)->Type, reinterpret_cast<char*>(&stTrade), sizeof(stTrade)});
		if (!m_pTradePanel->IsVisible())
		{
			auto pTextMyName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_MYNAME));
			auto pTextOPName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_OPNAME));

			char szMyName[128]{};
			char szOPName[128]{};
			sprintf_s(szMyName, "[%s]:%d", m_pMyHuman->m_szName, strlen(m_pMyHuman->m_szName));
			sprintf_s(szOPName, "[%s]:%d", pNode->m_szName, strlen(pNode->m_szName));
			if (pTextMyName)
				pTextMyName->SetText(szMyName, 1);
			if (pTextOPName)
				pTextOPName->SetText(szOPName, 1);
			SetVisibleTrade(1);
		}
	}
	break;
	case 11:
	{
		m_pMessageBox->SetVisible(0);
		m_bRespawnPromptOffered = true;
		m_dwLastTown = g_pTimerManager->GetServerTime();
		m_dwLastRemain = ~0u;
		m_cLastTown = 1;
		m_pMyHuman->m_bCNFMobKill = 0;

		MSG_DelayStart stDelayStart{};
		stDelayStart.Header.ID = m_pMyHuman->m_dwID;
		stDelayStart.Header.Type = MSG_DelayStart_Opcode;
		stDelayStart.Parm = 2;
		SendOneMessage((char*)&stDelayStart, sizeof(stDelayStart));
	}
	break;
	case 4:
	{
		MSG_ApplyBonus stApplyBonus{};

		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 2;
		stApplyBonus.Detail = m_pMessageBox->m_dwArg >> 16;
		stApplyBonus.TargetID = m_pMessageBox->m_dwArg;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stApplyBonus)->Type, reinterpret_cast<char*>(&stApplyBonus), sizeof(stApplyBonus)});
	}
	break;
	case 6193:
	case 6194:
	case 6195:
	case 6196:
	{
		char Value = 0;
		int GridX = 0;
		int GridY = 0;
		if (m_pMessageBox->m_dwMessage == 6193 || m_pMessageBox->m_dwMessage == 6196)
		{
			GridX = 2;
			GridY = 4;
			if (m_pMessageBox->m_dwMessage == 6193)
				Value = 1;
			else
				Value = 4;
		}
		else if (m_pMessageBox->m_dwMessage == 6194)
		{
			GridX = 2;
			GridY = 3;
			Value = 2;
		}
		if (m_pMessageBox->m_dwMessage == 6195)
		{
			GridX = 1;
			GridY = 1;
			Value = 3;
		}

		// The 7.48 recipient slot comes from its single 9x7 Carry grid.  Keep the
		// paged search only for TMProject's newer UI resources.
		const int carryPages = 1;
		for (int i = 0; i < carryPages; ++i)
		{
			SGridControl* pMyGrid = m_pGridInv;

			IVector2 vecGrid = pMyGrid->CanAddItemInEmpty(GridX, GridY);

			if (vecGrid.x > -1 && vecGrid.y > -1)
			{
				MSG_HellBuy stHellBuy{};

				stHellBuy.Header.ID = m_pMyHuman->m_dwID;
				stHellBuy.Header.Type = 701;
				stHellBuy.TargetID = m_pMessageBox->m_dwArg;
				stHellBuy.TargetCarryPos = static_cast<unsigned char>(Value);
				stHellBuy.MyCarryPos = vecGrid.x + 9 * vecGrid.y;

				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stHellBuy)->Type, reinterpret_cast<char*>(&stHellBuy), sizeof(stHellBuy)});

				if (g_pSoundManager)
				{
					auto pSoundData = g_pSoundManager->GetSoundData(336);

					if (pSoundData)
						pSoundData->Play(0, 0);
				}
				return 1;
			}
		}

		auto pItem = new SListBoxItem(g_pMessageStringTable[1], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pItem)
			m_pChatList->AddItem(pItem);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(33);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}
	}
	break;
	case 816:
	{
		MSG_GuildDeprivate stGuildDep{};
		stGuildDep.Header.ID = m_pMyHuman->m_dwID;
		stGuildDep.Header.Type = MSG_GuildDeprivate_Opcode;
		stGuildDep.TargetID = m_pMessageBox->m_dwArg;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stGuildDep)->Type, reinterpret_cast<char*>(&stGuildDep), sizeof(stGuildDep)});
		m_pPGTPanel->SetVisible(0);
	}
	break;
	case 817:
	{
		MSG_GuildRelation stParam{};

		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.Header.Type = MSG_GuildWar_Opcode;
		stParam.GuildID = m_pMyHuman->m_usGuild;
		stParam.TargetGuildID = m_pMessageBox->m_dwArg;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParam)->Type, reinterpret_cast<char*>(&stParam), sizeof(stParam)});
		m_pPGTPanel->SetVisible(0);
	}
	break;
	case 862:
	{
		MSG_GuildRelation stParam{};

		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.Header.Type = MSG_GuildAlly_Opcode;
		stParam.GuildID = m_pMyHuman->m_usGuild;
		stParam.TargetGuildID = m_pMessageBox->m_dwArg;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParam)->Type, reinterpret_cast<char*>(&stParam), sizeof(stParam)});
		m_pPGTPanel->SetVisible(0);
	}
	break;
	case 818:
		m_pPGTPanel->SetVisible(0);
		break;
	case 16:
	{
		m_dwGetItemTime = g_pTimerManager->GetServerTime();
		int nX = (int)m_pMyHuman->m_vecPosition.x;
		int nY = (int)m_pMyHuman->m_vecPosition.y;
		char bAttr = BASE_GetAttr(nX, nY);
		if (bAttr & 0x10)
		{
			MSG_ReqTeleport stParam{};
			stParam.Header.ID = g_pObjectManager->m_dwCharID;
			stParam.Header.Type = MSG_ReqTeleport_Opcode;
			stParam.Reserved = 0;
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParam)->Type, reinterpret_cast<char*>(&stParam), sizeof(stParam)});
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[38], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		m_pMessageBox->SetVisible(0);
		return 1;
	}
	break;
	case MSG_PlayerChallenge_Opcode:
	{
		MSG_STANDARDPARM2 stQuest{};

		stQuest.Header.Type = MSG_PlayerChallenge_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.Parm1 = m_pMessageBox->m_dwArg;
		stQuest.Parm2 = 4;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stQuest)->Type, reinterpret_cast<char*>(&stQuest), sizeof(stQuest)});
		m_pPGTOver = nullptr;
	}
	break;
	case 883:
	{
		int nCellX = m_pMessageBox->m_dwArg >> 16;
		int nCellY = m_pMessageBox->m_dwArg & 0xFFFF;

		SGridControlItem* pItem = m_pGridInv->GetItem(nCellX, nCellY);

		if (pItem)
		{
			int nType = BASE_GetItemAbility(pItem->m_pItem, EF_VOLATILE);

			UseItem(pItem, nType, pItem->m_pItem->sIndex, nCellX, nCellY);
		}
	}
	break;
	case 13:
	{
		MSG_UseNPC stParam{};

		stParam.Header.Type = MSG_UseNPC_Opcode;
		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.TargetID = m_pMessageBox->m_dwArg;
		stParam.ClickOk = 0;

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParam)->Type, reinterpret_cast<char*>(&stParam), sizeof(stParam)});
	}
	break;
	case 15:
	case 10:
	case 1742:
	case 233:
	case 58:
	{
		MSG_UseNPC stParam{};

		stParam.Header.Type = MSG_UseNPC_Opcode;
		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.TargetID = m_pMessageBox->m_dwArg;
		stParam.ClickOk = 1;

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParam)->Type, reinterpret_cast<char*>(&stParam), sizeof(stParam)});
	}
	break;
	case 646:
		SendReqBuy(m_pMessageBox->m_dwArg);
		break;
	case 38:
	{
		int nCellX = (m_pMessageBox->m_dwArg & 0xFFFF0000) >> 16;
		int nCellY = m_pMessageBox->m_dwArg & 0xFFFF;
		UseTicket(nCellX, nCellY);
	}
	break;
	case 99:
		m_cResurrect = 1;
		m_dwLastResurrect = g_pTimerManager->GetServerTime();
		return 1;
	case 228:
		m_cLastRelo = 1;
		m_dwLastRelo = g_pTimerManager->GetServerTime();
		break;
	case 50001:
	{
		MSG_STANDARDPARM stRemoveParty{};

		stRemoveParty.Header.Type = 894;
		stRemoveParty.Header.ID = m_pMyHuman->m_dwID;
		stRemoveParty.Parm = m_pMessageBox->m_dwArg;
		SendOneMessage((char*)&stRemoveParty, sizeof(stRemoveParty));
		break;
	}
	case 60:
	{
		MSG_ChallengeConfirm stParam{};

		stParam.Header.ID = g_pObjectManager->m_dwCharID;
		stParam.Header.Type = MSG_ChallengeConfirm_Opcode;
		stParam.Parm1 = m_dwTID;
		stParam.Parm2 = 0;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParam)->Type, reinterpret_cast<char*>(&stParam), sizeof(stParam)});
	}
	break;
	case 65859:
		DoCombine();
		break;
	case 6434:
		DoCombine4();
		break;
	case 51:
	{
		MSG_UseNPC stQuest{};

		stQuest.Header.Type = MSG_UseNPC_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.TargetID = m_pMessageBox->m_dwArg;
		stQuest.ClickOk = 0;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stQuest)->Type, reinterpret_cast<char*>(&stQuest), sizeof(stQuest)});
		m_dwNPCClickTime = dwServerTime;
	}
	break;
	case 740:
	{
		SGridControlItem* pSellItem = SGridControl::m_pSellItem;
		if (!m_pGridInv || !m_pMyHuman || !pSellItem ||
			!pSellItem->m_pGridControl || !pSellItem->m_pItem)
			return 1;

		short sDestType = m_pGridInv->CheckType(
			pSellItem->m_pGridControl->m_eItemType,
			pSellItem->m_pGridControl->m_eGridType);

		if (sDestType != 1)
			return 1;

		// Delete requests use the canonical 7.48 row-major Carry slot.
		short sDestPos = pSellItem->m_nCellIndexX
			+ 9 * pSellItem->m_nCellIndexY;

		int DestPage = 0;
		if (DestPage < 0 || DestPage > 45)
			DestPage = 0;

		MSG_STANDARDPARM2 stDeleteItem{};

		stDeleteItem.Header.ID = m_pMyHuman->m_dwID;
		stDeleteItem.Header.Type = MSG_DeleteItem_Opcode;
		stDeleteItem.Parm1 = DestPage + sDestPos;
		stDeleteItem.Parm2 = pSellItem->m_pItem->sIndex;
		// Keep the grid item owned by its grid until the authoritative SendItem
		// replaces the slot. Picking it up here leaked the detached visual and
		// made a rejected request disappear locally before the server replied.
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stDeleteItem)->Type, reinterpret_cast<char*>(&stDeleteItem), sizeof(stDeleteItem)});
		SGridControl::m_pSellItem = nullptr;
	}
	break;
	case 890:
	{
		if (m_pGridInv == nullptr || m_pGridShop == nullptr || m_pMyHuman == nullptr)
		{
			SGridControl::m_pSellItem = nullptr;
			return 1;
		}

		SGridControlItem* pSellItem = SGridControl::m_pSellItem;

		if (pSellItem && pSellItem->m_pGridControl && pSellItem->m_pItem)
		{
			short sDestType = m_pGridInv->CheckType(
				pSellItem->m_pGridControl->m_eItemType,
				pSellItem->m_pGridControl->m_eGridType);

			short sDestPos = m_pGridInv->CheckPos(pSellItem->m_pGridControl->m_eItemType);
			if (sDestPos == -1)
			{
				// Use the same native Carry projection as the Ctrl-sell path.
				sDestPos = sDestType == 1
					? GetCarrySlotForCell(pSellItem->m_pGridControl,
						pSellItem->m_nCellIndexX, pSellItem->m_nCellIndexY)
					: pSellItem->m_nCellIndexX + 5 * pSellItem->m_nCellIndexY;
			}

			int DestPage = 0;
			if (DestPage < 0 || DestPage > 45)
				DestPage = 0;

			MSG_Sell stSell{};

			stSell.Header.ID = m_pMyHuman->m_dwID;
			stSell.Header.Type = MSG_Sell_Opcode;
			stSell.TargetID = m_pGridShop->m_dwMerchantID;
			stSell.MyType = sDestType;
			stSell.MyPos = DestPage + sDestPos;

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSell)->Type, reinterpret_cast<char*>(&stSell), sizeof(stSell)});
		}
		SGridControl::m_pSellItem = nullptr;
	}
	break;
	case 271:
	case 88:
	{
		MSG_UseNPC stQuest{};

		stQuest.Header.Type = MSG_UseNPC_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.TargetID = m_pMessageBox->m_dwArg;
		stQuest.ClickOk = 0;

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stQuest)->Type, reinterpret_cast<char*>(&stQuest), sizeof(stQuest)});
	}
	break;
	case 84:
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if ((dwServerTime - m_dwOldAttackTime) < 1000)
			return 1;

		int nSkill = 83 - 24 * g_pObjectManager->m_stMobData.Class;

		if (BASE_GetManaSpent(
			83,
			(unsigned char)g_pObjectManager->m_stMobData.CurrentScore.SaveMana,
			g_pObjectManager->m_stMobData.CurrentScore.Mastery[nSkill / 8 + 1]) <= static_cast<long long>(g_pObjectManager->m_stMobData.CurrentScore.CurMP))
		{
			MSG_AttackOne stAttack{};

			stAttack.Header.Type = MSG_Attack_One_Opcode;
			stAttack.Header.ID = m_pMyHuman->m_dwID;
			stAttack.AttackerID = m_pMyHuman->m_dwID;
			stAttack.PosX = m_stMoveStop.NextX;
			stAttack.TargetX = stAttack.PosX;
			stAttack.PosY = m_stMoveStop.NextY;
			stAttack.TargetY = stAttack.PosY;
			stAttack.CurrentMp = -1;
			stAttack.SkillIndex = 83;
			stAttack.SkillParm = 0;
			stAttack.Motion = -1;
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAttack)->Type, reinterpret_cast<char*>(&stAttack), sizeof(stAttack)});
			m_dwOldAttackTime = dwServerTime;
			m_dwSkillLastTime[83] = dwServerTime;

			MSG_Sell stSell{};

			stSell.Header.ID = m_pMyHuman->m_dwID;
			stSell.Header.Type = 724;
			stSell.MyType = m_sDestType;
			stSell.MyPos = m_sDestPos;

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSell)->Type, reinterpret_cast<char*>(&stSell), sizeof(stSell)});

			m_sDestType = -1;
			m_sDestPos = -1;

			if (m_pMyHuman->m_pSkinMesh->m_pSwingEffect[0])
				m_pMyHuman->m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 1;
			if (m_pMyHuman->m_pSkinMesh->m_pSwingEffect[1])
				m_pMyHuman->m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 1;
			break;
		}

		auto pItem = new SListBoxItem(g_pMessageStringTable[30], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pItem)
			m_pChatList->AddItem(pItem);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(33);
			if (pSoundData)
				pSoundData->Play(0, 0);
		}
	}
	break;
	default:
		break;
	}

	return 1;
}

//Reviewed

// GetWYD748AttackVisualDamage reads only the optional server-authoritative
// uint32 damage tail. The signed WORD in STRUCT_DAM remains responsible for
// the legacy HP-bar projection, so extending the floating text cannot make the
// old 7.48 score overflow or subtract the same hit twice.

void TMFieldScene::GetTimeString(char* szVal, int sTime, int nTime, int i)
{
	if (nTime >= 1000000)
	{
		int add = m_pMyHuman->m_stAffect[i].Time / 1000000;
		int Year = m_pMyHuman->m_stAffect[i].Time % 1000000 / 10000;
		int nYearDay = 365;
		if (!(Year % 4))
			nYearDay = 366;

		int nDafaultDay = 0;
		switch (add)
		{
		case 4:
			nDafaultDay = 7;
			break;
		case 5:
			nDafaultDay = 15;
			break;
		case 6:
			nDafaultDay = 30;
			break;
		}

		if (add)
			sprintf(szVal, g_pMessageStringTable[291], nDafaultDay - nYearDay * (m_nYear - Year) - (m_nDays - m_pMyHuman->m_stAffect[i].Time % 10000));
	}
	else if (sTime > 86400)
		sprintf(szVal, g_pMessageStringTable[291], sTime / 86400);
	else if (sTime > 3600)
		sprintf(szVal, g_pMessageStringTable[292], sTime / 3600);
	else if (sTime <= 600)
		sprintf(szVal, "%5d", sTime);
	else
		sprintf(szVal, g_pMessageStringTable[293], sTime / 60);
}

/*
DWORD WINAPI Guildmark_Download(void* pArg)// Legacy behavior was reported as unreliable; not yet verified for 7.48.
{
	// TODO: we have to find a better way to download the guildmark
	// currently we have a great treat of data race...
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	auto pMark = static_cast<stGuildMarkInfo*>(pArg);

	if (!g_pCurrentScene || !pMark->strMarkFileName[0] || !pMark->pGuildMark)
	{
		g_pTextureManager->m_stGuildMark[pMark->nMarkIndex].nGuild = -1;
		return 0;
	}

	pFScene->m_dwLastGetGuildmarkTime = timeGetTime();

	char strMarkBuffer[632]{};
	char strURL[65]{};

	strcpy(strURL, g_pMessageStringTable[377]);
	strcat(strURL, pMark->strMarkFileName);

	////Check if is valid string for guildmark download
	//std::string pattern = "https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,4}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)";

	//// Construct regex object
	//std::regex url_regex(pattern);

	//// Check for match
	//if (!std::regex_match(strURL, url_regex) == true) {
	//	return FALSE;
	//}

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
*/
