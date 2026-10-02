#include "pch.h"
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
#include "Mission.h"
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
#include "TMEffectBillBoard4.h"
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
	static_assert(field_interaction::AutoTradeSlotIndex(true, TMG_ATRADE_MY12) == 11);

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
	m_bCompatFieldScene = false;
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
	m_Coin = 0;
	m_sShopTarget = 0;
	m_bEventCouponClick = 0;
	m_bEventCouponOpen = 0;
}

TMFieldScene::~TMFieldScene()
{
	g_nTempArray[1] = 0;
	g_nTempArray[2] = 0;

	if (g_bActiveWB == 1 && g_pApp)
	{
		g_pApp->RenderScene();
		g_pApp->SwitchWebBrowserState(0);
	}

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
	// The original 7.48 FieldScene2 resource is valid but lacks the newer
	// source tree's HUD IDs.  Detect that ABI before any optional control is
	// dereferenced and initialize the world/character through the safe path.
	if (!m_pControlContainer || !m_pControlContainer->FindControl(66817))
	{
		const int result = InitializeCompatFieldScene();
		LOG_WRITELOG(">> Init Field Scene: compat result=%d\\r\\n", result);
		return result;
	}
	g_pDevice->m_nHeightShift = 0;

	InitializeQuizEventControls();

	m_pChatList = (SListBox*)m_pControlContainer->FindControl(65667);

	m_pChatListnotice = (SListBox*)m_pControlContainer->FindControl(65944);
	m_pChatBack = (SPanel*)m_pControlContainer->FindControl(65943);
	m_pChatGeneral = (SButton*)m_pControlContainer->FindControl(65677);
	m_pChatParty = (SButton*)m_pControlContainer->FindControl(65678);
	m_pChatWhisper = (SButton*)m_pControlContainer->FindControl(65679);
	m_pChatGuild = (SButton*)m_pControlContainer->FindControl(65680);
	m_pChatGeneral_C = (SButton*)m_pControlContainer->FindControl(65673);
	m_pChatParty_C = (SButton*)m_pControlContainer->FindControl(65674);
	m_pChatWhisper_C = (SButton*)m_pControlContainer->FindControl(65675);
	m_pChatGuild_C = (SButton*)m_pControlContainer->FindControl(65676);
	m_pKingdomText = (SText*)m_pControlContainer->FindControl(131072);
	m_pKingDomGuild = (SButton*)m_pControlContainer->FindControl(131074);
	m_pKingDomGuild_C = (SButton*)m_pControlContainer->FindControl(131073);
	m_pQuestBtn = (SButton*)m_pControlContainer->FindControl(65793);
	m_pHelpBtn = (SButton*)m_pControlContainer->FindControl(65795);
	m_pccmode = (SPanel*)m_pControlContainer->FindControl(66817);

	m_pccmode->SetVisible(0);
	m_pccmode->m_nPosX = BASE_ScreenResize(210.0f);//alterado

	m_pGridCharFace = (SPanel*)m_pControlContainer->FindControl(69636);
	m_pKingDomGuild_C->m_bSelected = 1;
	m_pKingDomGuild_C->SetVisible(0);
	m_pChatGeneral_C->m_bSelected = 0;
	m_pChatParty_C->m_bSelected = 0;
	m_pChatWhisper_C->m_bSelected = 0;
	m_pChatGuild_C->m_bSelected = 0;
	m_pChatGeneral->m_bSelected = 1;
	m_pChatParty->m_bSelected = 1;
	m_pChatWhisper->m_bSelected = 1;
	m_pChatGuild->m_bSelected = 1;
	m_pKingDomGuild->m_bSelected = 0;
	m_pChatList->m_pScrollBar->SetVisible(1);
	m_pChatListnotice->m_nPosX = 490.0f * RenderDevice::m_fWidthRatio;
	m_pChatListnotice->m_nPosX = 440.0f * RenderDevice::m_fHeightRatio;
	m_pChatListnotice->m_pScrollBar->SetVisible(0);
	m_pChatListnotice->m_nPosX = BASE_ScreenResize(490.0f);//alterado

	m_pAutoRunBtn = (SButton*)m_pControlContainer->FindControl(316);
	m_pSysMsgList = (SListBox*)m_pControlContainer->FindControl(5695);
	m_PkButton = (SButton*)m_pControlContainer->FindControl(65786);
	if (m_PkButton)
	{
		m_PkButton->m_nPosX = BASE_ScreenResize(215.0f);
		m_PkButton->SetSelected(TMFieldScene::m_bPK == 0);
	}
	m_pPGTText = (SText*)m_pControlContainer->FindControl(645);

	char szStr[128]{};
	sprintf(szStr, " %s", g_pMessageStringTable[157]);

	m_pAutoSkillPanel = (SPanel*)m_pControlContainer->FindControl(65648);
	m_pAutoSkillPanelChild[0] = (SPanel*)m_pControlContainer->FindControl(65775);
	m_pAutoSkillPanelChild[1] = (SPanel*)m_pControlContainer->FindControl(65776);
	m_pAutoSkillPanelChild[2] = (SPanel*)m_pControlContainer->FindControl(65777);
	m_pAutoSkillPanelChild[3] = (SPanel*)m_pControlContainer->FindControl(65778);
	m_pAutoSkillPanelChild[4] = (SPanel*)m_pControlContainer->FindControl(65779);
	m_pAutoSkillPanelChild[5] = (SPanel*)m_pControlContainer->FindControl(65780);
	m_pAutoSkillPanelChild[6] = (SPanel*)m_pControlContainer->FindControl(65781);
	m_pAutoSkillPanelChild[7] = (SPanel*)m_pControlContainer->FindControl(65782);
	m_pAutoSkillPanelChild[8] = (SPanel*)m_pControlContainer->FindControl(65783);
	m_pAutoSkillPanelChild[9] = (SPanel*)m_pControlContainer->FindControl(65784);

	if (m_pAutoSkillPanel)
		m_pAutoSkillPanel->SetVisible(0);

	SetAutoSkillNum(m_nAutoSkillNum);

	WYD748_ResetTradeOffer(g_pObjectManager->m_stTrade, 0);
	memset(&g_pObjectManager->m_stCombineItem, 0, sizeof(g_pObjectManager->m_stCombineItem));
	memset(&g_pObjectManager->m_stCombineItem4, 0, sizeof(g_pObjectManager->m_stCombineItem4));
	g_pObjectManager->m_stCombineItem.Header.ID = g_pObjectManager->m_dwCharID;
	g_pObjectManager->m_stCombineItem.Header.Type = 0x3A6;
	g_pObjectManager->m_stCombineItem4.Header.ID = g_pObjectManager->m_dwCharID;
	g_pObjectManager->m_stCombineItem4.Header.Type = 0x3C0;

	for (int i = 0; i < 8; ++i)
	{
		g_pObjectManager->m_stCombineItem.CarryPos[i] = -1;
		g_pObjectManager->m_stCombineItem4.CarryPos[i] = -1;
	}

	m_pPositionText = (SText*)m_pControlContainer->FindControl(771);
	if (m_pPositionText)
		m_pPositionText->SetVisible(0);

	m_pFadePanel = (SPanel*)m_pControlContainer->FindControl(65566);
	if (m_pFadePanel)
		m_pFadePanel->m_bSelectEnable = 0;

	SPanel* pUnderBGPanel = (SPanel*)m_pControlContainer->FindControl(625);
	if (pUnderBGPanel)
		pUnderBGPanel->m_bSelectEnable = 0;

	m_pEditChat = (SEditableText*)m_pControlContainer->FindControl(65671);
	m_pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);

	if (m_pInputBG2)
		m_pInputBG2->SetVisible(0);
	if (m_pEditChat)
		m_pEditChat->SetVisible(1);

	m_pEditChat = (SEditableText*)m_pControlContainer->FindControl(65671);
	m_pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);

	if (m_pInputBG2)
		m_pInputBG2->SetVisible(0);
	if (m_pEditChat)
		m_pEditChat->SetVisible(1);

	memset(m_dwSkillLastTime, 0, sizeof(m_dwSkillLastTime));

	m_sWhisperIndex = 0;
	m_sChatIndex = 0;

	memset(m_szLastChatList, 0, sizeof(m_szLastChatList));
	memset(m_szWhisperList, 0, sizeof(m_szWhisperList));

	m_pFadePanel->SetSize((float)g_pDevice->m_dwScreenWidth, (float)g_pDevice->m_dwScreenHeight);

	if (!m_bCompatFieldScene)
	{
		m_pKingDomFlag = (SPanel*)m_pControlContainer->FindControl(65768);
		m_pFlagDesc = (SPanel*)m_pControlContainer->FindControl(65771);
		m_pFlagDescText[0] = (SText*)m_pControlContainer->FindControl(65772);
		m_pFlagDescText[1] = (SText*)m_pControlContainer->FindControl(65773);
		m_pFlagDescText[2] = (SText*)m_pControlContainer->FindControl(65774);

		if (m_pFlagDesc)
			m_pFlagDesc->SetVisible(0);
		if (m_pKingDomFlag)
			m_pKingDomFlag->m_pDescPanel = m_pFlagDesc;
	}
	if (m_pKingDomFlag)
		m_pKingDomFlag->SetVisible(0);

	InitializeRuntimeCounterTexts();

	m_pMessagePanels = (SPanel*)m_pControlContainer->FindControl(TMM_MESSAGE_PANEL);
	m_pMoney1 = (SText*)m_pControlContainer->FindControl(65564u);
	m_pMoney4 = (SText*)m_pControlContainer->FindControl(94221u);
	m_pSkBonus = (SText*)m_pControlContainer->FindControl(65600u);
	m_pCHP = (SText*)m_pControlContainer->FindControl(65614u);
	m_pCMP = (SText*)m_pControlContainer->FindControl(65616u);
	m_pExpLamp[0] = (SPanel*)m_pControlContainer->FindControl(65637u);
	m_pExpLamp[1] = (SPanel*)m_pControlContainer->FindControl(65638u);
	m_pExpLamp[2] = (SPanel*)m_pControlContainer->FindControl(65639u);
	m_pExpProgress[0] = (SProgressBar*)m_pControlContainer->FindControl(65655u);
	m_pExpProgress[1] = (SProgressBar*)m_pControlContainer->FindControl(65656u);
	m_pExpProgress[2] = (SProgressBar*)m_pControlContainer->FindControl(65657u);
	m_pExpProgress[3] = (SProgressBar*)m_pControlContainer->FindControl(65658u);
	m_pExpProgress[4] = (SProgressBar*)m_pControlContainer->FindControl(65659u);
	m_pExpProgress[5] = (SProgressBar*)m_pControlContainer->FindControl(65660u);
	m_pExpProgress[6] = (SProgressBar*)m_pControlContainer->FindControl(65661u);
	m_pExpProgress[7] = (SProgressBar*)m_pControlContainer->FindControl(65662u);
	m_pExpProgress[8] = (SProgressBar*)m_pControlContainer->FindControl(65663u);
	m_pExpProgress[9] = (SProgressBar*)m_pControlContainer->FindControl(65664u);
	m_pCCModeHpSte = (SText*)m_pControlContainer->FindControl(66823u);
	m_pCCModeMountSte = (SText*)m_pControlContainer->FindControl(66824u);

	m_pDonateStore = (SPanel*)m_pControlContainer->FindControl(3000011);
	m_pDonateStore->SetVisible(0);

	m_pDailyRewardInfo = (SPanel*)m_pControlContainer->FindControl(15715);
	m_pDailyRewardInfo->SetVisible(0);// daily reward panel

	m_pGuildInfo = (SPanel*)m_pControlContainer->FindControl(48000);
	m_pGuildInfo->SetVisible(0);// guild panel

	m_pNewPopup = (SPanel*)m_pControlContainer->FindControl(3000080);
	m_pNewPopup->SetVisible(0);// popup panel

	char chtmp[128]{};// initialized locally
	if (m_pCCModeHpSte)
	{
		sprintf(chtmp, "%d", g_GameAuto_hpValue);
		m_pCCModeHpSte->SetText(chtmp, 0);
	}
	if (m_pCCModeMountSte)
	{
		sprintf(chtmp, "%d", g_GameAuto_mountValue);
		m_pCCModeMountSte->SetText(chtmp, 0);
	}

	m_pCIName = (SText*)m_pControlContainer->FindControl(65703u);
	m_pCIClass = (SText*)m_pControlContainer->FindControl(65707u);
	m_pCIClass2 = (SText*)m_pControlContainer->FindControl(65708u);
	m_pCIHP = (SText*)m_pControlContainer->FindControl(65729u);
	m_pCIMP = (SText*)m_pControlContainer->FindControl(65731u);
	m_pCIEXP = (SText*)m_pControlContainer->FindControl(65710u);
	m_pCIEXPE = (SText*)m_pControlContainer->FindControl(65711u);
	m_pCLevel = (SText*)m_pControlContainer->FindControl(65713u);
	m_pScBonus = (SText*)m_pControlContainer->FindControl(65727u);
	m_pCIStr = (SText*)m_pControlContainer->FindControl(65715u);
	m_pCIInt = (SText*)m_pControlContainer->FindControl(65718u);
	m_pCIDex = (SText*)m_pControlContainer->FindControl(65721u);
	m_pCICon = (SText*)m_pControlContainer->FindControl(65724u);
	m_pCIFakeExp = (SText*)m_pControlContainer->FindControl(65767u);
	m_pCISpecial1 = (SText*)m_pControlContainer->FindControl(65753u);
	m_pCISpecial2 = (SText*)m_pControlContainer->FindControl(65756u);
	m_pCISpecial3 = (SText*)m_pControlContainer->FindControl(65759u);
	m_pCISpecial4 = (SText*)m_pControlContainer->FindControl(65762u);
	m_pSpBonus = (SText*)m_pControlContainer->FindControl(65765u);
	m_pMoney2 = (SText*)m_pControlContainer->FindControl(1041u);
	m_pMoney3 = (SText*)m_pControlContainer->FindControl(6408u);

	for (int i = 0; i < 24; ++i)
		m_pSkillSecGrid[i] = (SGridControl*)m_pControlContainer->FindControl(i + 65574);

	for (int i = 0; i < 12; ++i)
		m_pSkillSecGrid2[i] = (SGridControl*)m_pControlContainer->FindControl(i + 94208);

	m_pDamage = (SText*)m_pControlContainer->FindControl(65733u);
	m_pSkillDam = (SText*)m_pControlContainer->FindControl(65735u);
	m_pSpeed = (SText*)m_pControlContainer->FindControl(65737u);
	m_pDefence = (SText*)m_pControlContainer->FindControl(65739u);
	m_pAttackSpeed = (SText*)m_pControlContainer->FindControl(65741u);
	m_pCritical = (SText*)m_pControlContainer->FindControl(65743u);
	m_pRegist1 = (SText*)m_pControlContainer->FindControl(65745u);
	m_pRegist2 = (SText*)m_pControlContainer->FindControl(65747u);
	m_pRegist3 = (SText*)m_pControlContainer->FindControl(65749u);
	m_pRegist4 = (SText*)m_pControlContainer->FindControl(65751u);
	m_pSLPanel1 = (SPanel*)m_pControlContainer->FindControl(1923u);
	m_pSLPanel2 = (SPanel*)m_pControlContainer->FindControl(1924u);
	m_pSLPanel3 = (SPanel*)m_pControlContainer->FindControl(1925u);
	m_pSkillCover = (SPanel*)m_pControlContainer->FindControl(1157u);
	m_pMainInfo1 = (SPanel*)m_pControlContainer->FindControl(65628u);
	m_pMainInfo1_BG = (SPanel*)m_pControlContainer->FindControl(65641u);
	m_pCC_Btn = (SButton*)m_pControlContainer->FindControl(66570u);

	m_pPvpDamage = (SText*)m_pControlContainer->FindControl(67768u);
	m_pPvpAc = (SText*)m_pControlContainer->FindControl(67769u);
	m_pPerfuDamage = (SText*)m_pControlContainer->FindControl(67770u);
	m_pAbsDamage = (SText*)m_pControlContainer->FindControl(67771u);
	m_pPrecision = (SText*)m_pControlContainer->FindControl(67772u);
	m_pParryRate = (SText*)m_pControlContainer->FindControl(67773u);
	m_pSaveMana = (SText*)m_pControlContainer->FindControl(67774u);
	m_pRegenHP = (SText*)m_pControlContainer->FindControl(67775u);
	m_pRegenMP = (SText*)m_pControlContainer->FindControl(67776u);
	m_pCriticalDamInc = (SText*)m_pControlContainer->FindControl(67777u);
	m_pAttackRange = (SText*)m_pControlContainer->FindControl(67778u);
	m_pSkillDelayDec = (SText*)m_pControlContainer->FindControl(67779u);
	m_pCriticalNew = (SText*)m_pControlContainer->FindControl(67780u);
	m_pAtkMagic = (SText*)m_pControlContainer->FindControl(67781u);
	m_pBonusEXP = (SText*)m_pControlContainer->FindControl(67782u);
	m_pBonusDROP = (SText*)m_pControlContainer->FindControl(67783u);

	m_pNickDROP[6] = (SText*)m_pControlContainer->FindControl(478487u);//drop
	m_pNickDROP[6]->SetTextColor(0xFFFF0000);
	m_pNickDROP[5] = (SText*)m_pControlContainer->FindControl(478489u);//drop
	m_pNickDROP[5]->SetTextColor(0xFFFFFF00);
	m_pNickDROP[4] = (SText*)m_pControlContainer->FindControl(478490u);//drop
	m_pNickDROP[4]->SetTextColor(0xFF00FF00);

	m_pMainInfo1_BG->SetPos(((float)g_pDevice->m_dwScreenWidth - m_pMainInfo1_BG->m_nWidth) / 2.0f,
		m_pMainInfo1_BG->m_nPosX);

	m_pMainInfo1->SetStickLeft();
	m_pMainInfo1->SetStickBottom();

	m_pMessagePanels->m_nPosX = BASE_ScreenResize(190.0f);//alterado//alterado 2.0

	m_pCC_Btn->m_nPosX = BASE_ScreenResize(170.0f);//alterado//alterado 2.0
	//m_pMainInfo1_BG->m_nWidth = BASE_ScreenResize(460.0f);//alterado//alterado 2.0
	m_pExpProgress[0]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[1]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[2]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[3]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[4]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[5]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[6]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[7]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[8]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[9]->m_nWidth = BASE_ScreenResize(35.0f);//alterado//alterado 2.0
	m_pExpProgress[0]->m_nPosX = BASE_ScreenResize(216.0f);//alterado//alterado 2.0
	m_pExpProgress[1]->m_nPosX = BASE_ScreenResize(254.0f);//alterado//alterado 2.0
	m_pExpProgress[2]->m_nPosX = BASE_ScreenResize(291.0f);//alterado//alterado 2.0
	m_pExpProgress[3]->m_nPosX = BASE_ScreenResize(328.0f);//alterado//alterado 2.0
	m_pExpProgress[4]->m_nPosX = BASE_ScreenResize(364.7f);//alterado//alterado 2.0
	m_pExpProgress[5]->m_nPosX = BASE_ScreenResize(401.6f);//alterado//alterado 2.0
	m_pExpProgress[6]->m_nPosX = BASE_ScreenResize(439.0f);//alterado//alterado 2. 0
	m_pExpProgress[7]->m_nPosX = BASE_ScreenResize(475.5f);//alterado//alterado 2.0
	m_pExpProgress[8]->m_nPosX = BASE_ScreenResize(513.0f);//alterado//alterado 2.0
	m_pExpProgress[9]->m_nPosX = BASE_ScreenResize(550.0f);//alterado//alterado 2.0
	m_pExpLamp[0]->m_nPosX = BASE_ScreenResize(575.0f);//alterado//alterado 2.0
	m_pExpLamp[1]->m_nPosX = BASE_ScreenResize(575.0f);//alterado//alterado 2.0
	m_pExpLamp[2]->m_nPosX = BASE_ScreenResize(575.0f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[9]->m_nPosX = BASE_ScreenResize(68.0f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[8]->m_nPosX = BASE_ScreenResize(88.0f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[7]->m_nPosX = BASE_ScreenResize(109.0f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[6]->m_nPosX = BASE_ScreenResize(128.5f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[5]->m_nPosX = BASE_ScreenResize(147.5f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[4]->m_nPosX = BASE_ScreenResize(167.5f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[3]->m_nPosX = BASE_ScreenResize(187.5f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[2]->m_nPosX = BASE_ScreenResize(207.0f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[1]->m_nPosX = BASE_ScreenResize(227.5f);//alterado//alterado 2.0
	m_pAutoSkillPanelChild[0]->m_nPosX = BASE_ScreenResize(247.5f);//alterado//alterado 2.0

	SPanel* pChatPanel = (SPanel*)m_pControlContainer->FindControl(65672);
	m_pChatPanel = (SPanel*)m_pControlContainer->FindControl(65672);
	m_pChatList->m_nPosX = BASE_ScreenResize(10.0f);//alterado
	m_pChatList->m_nPosY = (pChatPanel->m_nPosY - m_pChatList->m_nHeight) - BASE_ScreenResize(10.0f);

	m_pChatBack->m_nPosX = BASE_ScreenResize(10.0f);
	m_pChatBack->m_nPosY = m_pChatList->m_nPosY - BASE_ScreenResize(10.0f);

	m_pChatBack->SetSize(BASE_ScreenResize(10.0f) + m_pChatList->m_nWidth,
		BASE_ScreenResize(20.0f) + m_pChatList->m_nHeight);
	m_pChatList->SetSize(m_pChatList->m_nWidth,
		(float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	m_pChatBack->SetSize(BASE_ScreenResize(10.0f) + m_pChatList->m_nWidth,
		m_pChatList->m_nHeight);
	m_pChatList->m_pScrollBar->SetSize(m_pChatList->m_pScrollBar->m_nWidth,
		(float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	m_pChatList->m_pScrollBar->m_pBackground1->SetSize(m_pChatList->m_pScrollBar->m_nWidth = 1.0f,
		(float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);

	m_pChatList->m_pScrollBar->m_pBackground1->SetSize(BASE_ScreenResize(10.0f) + m_pChatList->m_pScrollBar->m_pBackground1->m_nWidth,
		m_pChatList->m_pScrollBar->m_pBackground1->m_nHeight);

	m_pChatList->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - m_pChatList->m_nHeight) - 4.0f);
	m_pChatBack->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - m_pChatList->m_nHeight) - 4.0f);

	m_pChatSelectPanel = (SPanel*)m_pControlContainer->FindControl(90113);
	m_pChatListPanel = (SPanel*)m_pControlContainer->FindControl(90128);
	m_pChatType = (SButton*)m_pControlContainer->FindControl(90114);

	SButton* Button = (SButton*)m_pControlContainer->FindControl(90129);
	m_pChatType->SetText(Button->m_GCPanel.strString);
	m_pChatListPanel->SetVisible(0);
	m_pChatSelectPanel->SetVisible(0);
	m_pChatList->m_pScrollBar->m_pBackground1->SetVisible(1);
	m_pChatListnotice->SetVisible(1);
	m_pMainInfo2 = (SPanel*)m_pControlContainer->FindControl(65610);

	if (m_pMainInfo2)
	{
		m_pMainInfo2->SetAutoSize();
		m_pMainInfo2->SetStickLeft();
		m_pMainInfo2->SetStickTop();
		m_pMainInfo2_Name = (SText*)m_pControlContainer->FindControl(65611);
		m_pMainInfo2->SetVisible(0);
	}

	m_pMainInfo2_Lv = (SText*)m_pControlContainer->FindControl(65613);
	m_pEditChatPanel = (SPanel*)m_pControlContainer->FindControl(65670);

	if (m_pEditChatPanel)
	{
		m_pEditChatPanel->m_nPosY = m_pControlContainer->FindControl(65672)->m_nPosY - (float)(2.0f * RenderDevice::m_fHeightRatio);
		m_pEditChatPanel->m_nPosX = m_pChatSelectPanel->m_nPosX + m_pChatSelectPanel->m_nWidth;

		if (g_nKeyType != 1)
			g_nKeyType = 0;
		if (!g_nKeyType)
			m_pEditChatPanel->SetVisible(0);

		m_pChatPanel->SetVisible(1);
	}

	m_pMiniBtn = (SButton*)m_pControlContainer->FindControl(65787);
	m_pMiniPanel = (SPanel*)m_pControlContainer->FindControl(65788);
	m_pMiniBtn->m_nPosX = BASE_ScreenResize(595.0f);// alterado //alterado 2.0
	m_pMiniPanel->m_nPosX = BASE_ScreenResize(410.0f);// alterado //alterado 2.0

	if (m_pMiniPanel)
		m_pMiniPanel->SetVisible(0);

	m_pShortSkillPanel = (SPanel*)m_pControlContainer->FindControl(65642);
	m_pShortSkill_Txt = (SText*)m_pControlContainer->FindControl(65643);
	m_pShortSkill_Txt->m_nPosX = BASE_ScreenResize(585.0f);// alterado
	m_pCISp1Caption = (SText*)m_pControlContainer->FindControl(65755);
	m_pCISp2Caption = (SText*)m_pControlContainer->FindControl(65758);
	m_pCISp3Caption = (SText*)m_pControlContainer->FindControl(65761);
	m_pCISp4Caption = (SText*)m_pControlContainer->FindControl(65764);
	m_pCIGuild = (SText*)m_pControlContainer->FindControl(65705);
	m_pSkillSec1 = (SText*)m_pControlContainer->FindControl(65570);
	m_pSkillSec2 = (SText*)m_pControlContainer->FindControl(65571);
	m_pSkillSec3 = (SText*)m_pControlContainer->FindControl(65572);
	m_pSkillMSec1 = (SText*)m_pControlContainer->FindControl(65604);
	m_pSkillMSec2 = (SText*)m_pControlContainer->FindControl(65605);
	m_pSkillMSec3 = (SText*)m_pControlContainer->FindControl(65606);

	for (int id = 0; id < 32; ++id)
	{
		m_pAffectL[id] = (SText*)m_pControlContainer->FindControl(id + 1328);
		m_pAffect[id] = (SText*)m_pControlContainer->FindControl(id + 90432);
	}

	m_pCargoCoin = (SText*)m_pControlContainer->FindControl(65689);
	m_pMyCargoCoin = (SText*)m_pControlContainer->FindControl(812);

	SText* pMyGold = (SText*)m_pControlContainer->FindControl(619);
	SText* pOPGold = (SText*)m_pControlContainer->FindControl(603);
	pMyGold->m_cComma = 2;
	pOPGold->m_cComma = 2;
	pMyGold->SetText((char*)"         0", 0);
	pOPGold->SetText((char*)"         0", 0);
	m_pInvenPanel = (SPanel*)m_pControlContainer->FindControl(589832);

	SPanel* pPanel1 = (SPanel*)m_pControlContainer->FindControl(65565);

	m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 650.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pInvenPanel)
	{
		m_pInvenPanel->SetVisible(0);
		pPanel1->m_bSelectEnable = 0;
	}

	m_pAutoTrade = (SPanel*)m_pControlContainer->FindControl(646);

	if (m_pAutoTrade)
	{
		m_pAutoTrade->SetVisible(0);
		for (int k = 0; k < 10; ++k)
		{
			m_pGridAutoTrade[k] = (SGridControl*)m_pControlContainer->FindControl(k + 653);
			if (m_pGridAutoTrade[k])
				m_pGridAutoTrade[k]->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
		}

		m_pAutoTrade->SetPos(RenderDevice::m_fWidthRatio * 254.0f,
			RenderDevice::m_fHeightRatio * 35.0f);
	}

	m_pItemMixPanel4 = (SPanel*)m_pControlContainer->FindControl(6432);
	m_pMix4Desc = (SListBox*)m_pControlContainer->FindControl(6439);
	m_pItemMixPanel = (SPanel*)m_pControlContainer->FindControl(65857);

	m_ItemMixClass.Read_MixListFile();
	m_ItemMixClass.TakeItResource(m_pControlContainer, g_pObjectManager->m_dwCharID);
	m_MissionClass.TakeItResource(m_pControlContainer, g_pObjectManager->m_dwCharID);

	if (m_pItemMixPanel4)
		m_pItemMixPanel4->SetPos(RenderDevice::m_fWidthRatio * 423.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pItemMixPanel)
		m_pItemMixPanel->SetPos(RenderDevice::m_fWidthRatio * 423.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pMix4Desc)
		LoadMsgText(m_pMix4Desc, (char*)"UI\\mix4desc.txt");

	if (m_pItemMixPanel)
	{
		m_pItemMixPanel->SetVisible(0);
		for (int l = 0; l < 8; ++l)
		{
			m_pGridItemMix[l] = (SGridControl*)m_pControlContainer->FindControl(l + 65861);
			if (m_pGridItemMix[l])
				m_pGridItemMix[l]->m_eGridType = TMEGRIDTYPE::GRID_ITEMMIX;
		}
	}

	if (m_pItemMixPanel4)
	{
		m_pItemMixPanel4->SetVisible(0);
		for (int m = 0; m < 3; ++m)
		{
			m_pGridItemMix4[m] = (SGridControl*)m_pControlContainer->FindControl(m + 6436);
			if (m_pGridItemMix4[m])
				m_pGridItemMix4[m]->m_eGridType = TMEGRIDTYPE::GRID_ITEMMIX4;
		}
	}

	m_pHellgateStore = (SPanel*)m_pControlContainer->FindControl(6185);
	m_pHellStoreDesc = (SListBox*)m_pControlContainer->FindControl(6201);
	m_pGridHellStore = (SGridControl*)m_pControlContainer->FindControl(6208);
	m_pGridHellStore->m_bDrawGrid = 0;
	m_pGridHellStore->m_eGridType = TMEGRIDTYPE::GRID_SHOP;

	if (m_pHellgateStore)
		m_pHellgateStore->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pHellgateStore->m_nWidth * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pHellgateStore->m_nHeight * 0.5f));

	if (m_pHellgateStore)
		m_pHellgateStore->SetVisible(0);

	m_pGambleStore = (SPanel*)m_pControlContainer->FindControl(6400);
	if (m_pGambleStore)
	{
		m_pReelPanel = new SReelPanel(330, 21.0f, 79.0f, 62.0f, 62.0f, 1.0f);
		m_pReelPanel2 = new SReelPanel(340, 21.0f, 79.0f, 62.0f, 62.0f, 1.0f);
		m_pGambleStore->AddChild(m_pReelPanel);
		m_pGambleStore->AddChild(m_pReelPanel2);
		if (m_pGambleStore)
			SetVisibleGamble(0, 0);
	}

	m_pDescPanel = (SPanel*)m_pControlContainer->FindControl(258);
	m_pDescPanel->m_bSelectEnable = 0;
	m_pDescPanel->SetVisible(0);
	m_pDescNameText = (SText*)m_pControlContainer->FindControl(772);
	m_pParamText[0] = (SText*)m_pControlContainer->FindControl(773);
	m_pParamText[1] = (SText*)m_pControlContainer->FindControl(774);
	m_pParamText[2] = (SText*)m_pControlContainer->FindControl(775);
	m_pParamText[3] = (SText*)m_pControlContainer->FindControl(776);
	m_pParamText[4] = (SText*)m_pControlContainer->FindControl(777);
	m_pParamText[5] = (SText*)m_pControlContainer->FindControl(784);
	m_pParamText[6] = (SText*)m_pControlContainer->FindControl(794);
	m_pParamText[7] = (SText*)m_pControlContainer->FindControl(795);
	m_pParamText[8] = (SText*)m_pControlContainer->FindControl(796);
	m_pParamText[9] = (SText*)m_pControlContainer->FindControl(797);
	m_pParamText[10] = (SText*)m_pControlContainer->FindControl(798);
	m_pParamText[11] = (SText*)m_pControlContainer->FindControl(799);
	m_pSystemPanel = (SPanel*)m_pControlContainer->FindControl(65879);

	if (m_pSystemPanel)
	{
		m_pSystemPanel->SetVisible(0);
		m_pSystemPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[1] = m_pSystemPanel;

	}
	m_pPGTPanel = (SPanel*)m_pControlContainer->FindControl(640);
	if (m_pPGTPanel)
	{
		m_pPGTPanel->SetVisible(0);
		m_pPGTPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[2] = m_pPGTPanel;
	}
	m_pInputGoldPanel = (SPanel*)m_pControlContainer->FindControl(65885);
	if (m_pInputGoldPanel)
	{
		m_pInputGoldPanel->SetVisible(0);
		m_pInputGoldPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[3] = m_pInputGoldPanel;
	}

	m_pBtnGuildOnOff = (SButton*)m_pControlContainer->FindControl(299);
	m_pBtnMountRun = (SButton*)m_pControlContainer->FindControl(298);
	m_pBtnPGTParty = (SButton*)m_pControlContainer->FindControl(641);
	m_pBtnPGTGuild = (SButton*)m_pControlContainer->FindControl(642);
	m_pBtnPGTTrade = (SButton*)m_pControlContainer->FindControl(643);
	m_pBtnPGTChallenge = (SButton*)m_pControlContainer->FindControl(620);
	m_pBtnPGT1_V_1 = (SButton*)m_pControlContainer->FindControl(639);
	m_pBtnPGT5_V_5 = (SButton*)m_pControlContainer->FindControl(621);
	m_pBtnPGT10_V_10 = (SButton*)m_pControlContainer->FindControl(622);
	m_pBtnPGTAll_V_All = (SButton*)m_pControlContainer->FindControl(623);
	m_pBtnPGTGuildDrop = (SButton*)m_pControlContainer->FindControl(816);
	m_pBtnPGTGuildWar = (SButton*)m_pControlContainer->FindControl(817);
	m_pBtnPGTGuildAlly = (SButton*)m_pControlContainer->FindControl(862);
	m_pBtnPGTGuildInvite = (SButton*)m_pControlContainer->FindControl(863);
	m_pBtnPGTGICommon = (SButton*)m_pControlContainer->FindControl(912);
	m_pBtnPGTGIChief1 = (SButton*)m_pControlContainer->FindControl(913);
	m_pBtnPGTGIChief2 = (SButton*)m_pControlContainer->FindControl(914);
	m_pBtnPGTGIChief3 = (SButton*)m_pControlContainer->FindControl(915);
	SControl* pServer = m_pControlContainer->FindControl(65880);
	SControl* pChar = m_pControlContainer->FindControl(65881);
	SControl* pQuit = m_pControlContainer->FindControl(65882);
	SControl* pCancel = m_pControlContainer->FindControl(5883);
	m_pCPanel = (SPanel*)m_pControlContainer->FindControl(65696);

	SPanel* pCPanel1 = (SPanel*)m_pControlContainer->FindControl(65770);
	m_pCPanel->SetPos(RenderDevice::m_fWidthRatio * 60.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	m_pSystemPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pSystemPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pSystemPanel->m_nHeight * 0.5f));

	m_pPGTPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pPGTPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pPGTPanel->m_nHeight * 0.5f));

	m_pInputGoldPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pInputGoldPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pInputGoldPanel->m_nHeight * 0.5f));

	if (m_pCPanel)
	{
		m_pCPanel->SetVisible(0);
		pCPanel1->m_bSelectEnable = 0;
	}

	m_pShopPanel = (SPanel*)m_pControlContainer->FindControl(65692);

	SPanel *pShopPanel1 = (SPanel*)m_pControlContainer->FindControl(65695);

	m_pShopPanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pShopPanel)
	{
		m_pShopPanel->SetVisible(0);
		pShopPanel1->m_bSelectEnable = 0;
	}
	m_pCargoPanel = (SPanel*)m_pControlContainer->FindControl(65686);
	m_pCargoPanel1 = (SPanel*)m_pControlContainer->FindControl(65692);

	SPanel* pCargoPanel1 = (SPanel*)m_pControlContainer->FindControl(65691);
	m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 423.0f,
		RenderDevice::m_fHeightRatio * 35.0f);
	m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 423.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pCargoPanel)
	{
		m_pCargoPanel->SetVisible(0);
		m_pCargoPanel1->SetVisible(0);
		pCargoPanel1->m_bSelectEnable = 0;
	}

	SPanel* pTradePanel = (SPanel*)m_pControlContainer->FindControl(576);
	SPanel* pTradePanel1 = (SPanel*)m_pControlContainer->FindControl(577);

	if (pTradePanel)
		pTradePanel->SetPos(RenderDevice::m_fWidthRatio * 423.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

	m_pTradePanel = pTradePanel;

	if (pTradePanel)
	{
		pTradePanel->SetVisible(0);
		if (pTradePanel1)
			pTradePanel1->m_bSelectEnable = 0;
	}

	m_pLottoPanel = (SPanel*)m_pControlContainer->FindControl(2048);

	if (m_pLottoPanel)
	{
		m_pLottoPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - ((m_pLottoPanel->m_nWidth + 40.0f) * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pLottoPanel->m_nHeight * 0.5f));

		m_pLottoCost = (SText*)m_pControlContainer->FindControl(2051);
		for (int nLotto = 0; nLotto < 6; ++nLotto)
			m_pLottoNumber[nLotto] = (SText*)m_pControlContainer->FindControl(nLotto + 2053);

		m_pLottoPanel->m_bPickable = 1;
		m_pLottoPanel->m_bSelectEnable = 1;
		m_pLottoPanel->SetVisible(0);
	}

	m_pSkillMPanel = (SPanel*)m_pControlContainer->FindControl(65602);
	SPanel* pSkillMPanel1 = (SPanel*)m_pControlContainer->FindControl(65609);
	m_pSkillMDesc = (SListBox*)m_pControlContainer->FindControl(65608);

	m_pSkillMPanel->SetPos(RenderDevice::m_fWidthRatio * 155.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pSkillMPanel)
	{
		m_pSkillMPanel->SetVisible(0);
		pSkillMPanel1->m_bSelectEnable = 0;
	}

	m_pSkillPanel = (SPanel*)m_pControlContainer->FindControl(65567);

	SPanel* pSkillPanel1 = (SPanel*)m_pControlContainer->FindControl(65601);
	SText* pSkillBonus = (SText*)m_pControlContainer->FindControl(65600);

	m_pSkillPanel->SetPos(RenderDevice::m_fWidthRatio * 610.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	SPanel* pSkillTitlePanel = (SPanel*)m_pControlContainer->FindControl(1912);
	SPanel* pSkillGridPanel = (SPanel*)m_pControlContainer->FindControl(1910);
	SPanel* pSkillGridPanel2 = (SPanel*)m_pControlContainer->FindControl(1911);
	if (m_pSkillPanel)
	{
		m_pSkillPanel->SetVisible(0);
		if (pSkillTitlePanel)
			pSkillTitlePanel->m_bSelectEnable = 0;
		if (pSkillGridPanel)
			pSkillGridPanel->m_bSelectEnable = 0;
		if (pSkillGridPanel2)
			pSkillGridPanel2->m_bSelectEnable = 0;
		pSkillBonus->m_bSelectEnable = 0;
		pSkillPanel1->m_bSelectEnable = 0;
	}

	m_pInvPageBtn1 = (SButton*)m_pControlContainer->FindControl(67076);
	m_pInvPageBtn2 = (SButton*)m_pControlContainer->FindControl(67077);
	m_pInvPageBtn3 = (SButton*)m_pControlContainer->FindControl(67078);
	m_pInvPageBtn4 = (SButton*)m_pControlContainer->FindControl(67079);
	m_pJPNBag_Day1 = (SText*)m_pControlContainer->FindControl(77834);
	m_pJPNBag_Day2 = (SText*)m_pControlContainer->FindControl(77835);
	m_pStorePageBtn1 = (SButton*)m_pControlContainer->FindControl(67332);
	m_pStorePageBtn2 = (SButton*)m_pControlContainer->FindControl(67333);
	m_pStorePageBtn3 = (SButton*)m_pControlContainer->FindControl(67334);
	m_pMGameAutoBtn = (SButton*)m_pControlContainer->FindControl(66818);
	m_pSGameAutoBtn = (SButton*)m_pControlContainer->FindControl(B_CCPOTION);
	m_pCCPotionBtn = (SButton*)m_pControlContainer->FindControl(B_CCMODE_DLG_HP);
	m_pCCFeedBtn = (SButton*)m_pControlContainer->FindControl(B_CCMODE_DLG_MOUNT);
	m_pSetType = (SButton*)m_pControlContainer->FindControl(66821);
	m_pHPBar = (SProgressBar*)m_pControlContainer->FindControl(65621);
	m_pMPBar = (SProgressBar*)m_pControlContainer->FindControl(65623);
	m_pMHPBar = (SProgressBar*)m_pControlContainer->FindControl(65625);
	m_pMHPBarT = (SProgressBar*)m_pControlContainer->FindControl(65626);
	m_pMainCharName = (SText*)m_pControlContainer->FindControl(69635);
	m_pCurrentHPText = (SText*)m_pControlContainer->FindControl(65614);
	m_pMaxHPText = (SText*)m_pControlContainer->FindControl(65615);
	m_pCurrentMPText = (SText*)m_pControlContainer->FindControl(65616);
	m_pMaxMPText = (SText*)m_pControlContainer->FindControl(65617);
	m_pCurrentMHPText = (SText*)m_pControlContainer->FindControl(65618);
	m_pMaxMHPText = (SText*)m_pControlContainer->FindControl(65619);
	m_pExpHold = (SPanel*)m_pControlContainer->FindControl(65636);
	m_pInfoText = (SText*)m_pControlContainer->FindControl(770);

	if (m_pExpHold)
		m_pExpHold->SetVisible(0);

	m_pQuizPanel = (SPanel*)m_pControlContainer->FindControl(880);
	m_pQuizText[0] = (SText*)m_pControlContainer->FindControl(881);
	m_pQuizText[1] = (SText*)m_pControlContainer->FindControl(882);
	m_pQuizText[2] = (SText*)m_pControlContainer->FindControl(883);
	m_pQuizText[3] = (SText*)m_pControlContainer->FindControl(884);

	if (m_pQuizPanel)
		m_pQuizPanel->m_bSelectEnable = 0;
	if (m_pQuizPanel)
		m_pQuizPanel->SetVisible(0);

	m_pQuizCaption = (SText*)m_pControlContainer->FindControl(6108);

	m_pQuizPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pQuizPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (200.0f * RenderDevice::m_fHeightRatio));

	m_pEventText = (SText*)m_pControlContainer->FindControl(65669);
	m_pEventPanel = (SPanel*)m_pControlContainer->FindControl(65668);

	m_pEventText->SetPos(300.0f, 300.0f);

	m_dwEventStartTime = g_pTimerManager->GetServerTime();
	m_nCurrEventTextIndex = 0;

	memset(m_szEventTextTemp, 0, sizeof(m_szEventTextTemp));

	STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
	int nTownX = (int)g_pObjectManager->m_stMobData.HomeTownX >> 7;
	int nTownY = (int)g_pObjectManager->m_stMobData.HomeTownY >> 7;
	char szMapPath[128]{};
	char szDataPath[128]{};
	sprintf(szMapPath, "env\\Field%02d%02d.trn", nTownX, nTownY);
	sprintf(szDataPath, "env\\Field%02d%02d.dat", nTownX, nTownY);

	m_pGroundList[0] = new TMGround();
	if (!m_pGroundList[0]->LoadTileMap(szMapPath))
	{
		if (!m_bCriticalError)
			LogMsgCriticalError(2, 0, 0, 0, 0);

		m_bCriticalError = 1;
		return 0;
	}

	m_pGround = m_pGroundList[0];
	m_pGround->SetMiniMapData();

	m_pObjectContainerList[0] = new TMObjectContainer(m_pGround);

	if (m_pObjectContainerList[0])
		m_pGroundObjectContainer->AddChild(m_pObjectContainerList[0]);

	SetMinimapPos();

	m_pMyHuman = new TMHuman(this);
	if (m_pMyHuman)
		m_pMyHuman->m_dwID = g_pObjectManager->m_dwCharID;

	m_pMyHuman->m_nTotalKill = ((unsigned char)pMobData->MobName[15] << 8) + (unsigned char)pMobData->MobName[14];
	m_pMyHuman->m_nCurrentKill = m_pMyHuman->m_nTotalKill;
	m_pMyHuman->m_ucChaosLevel = pMobData->MobName[12];
	pMobData->MobName[12] = 0;
	pMobData->MobName[15] = 0;

	sprintf(m_pMyHuman->m_szName, "%s", pMobData->MobName);

	m_pMyHuman->SetPacketMOBItem(pMobData);
	m_pMyHuman->SetSpeed(m_bMountDead);
	if (m_pMyHuman->m_cMount == 1 &&
		(m_pMyHuman->m_nMountSkinMeshType == 31 || m_pMyHuman->m_nMountSkinMeshType == 40 || m_pMyHuman->m_nMountSkinMeshType == 20 &&
		 m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
	{
		m_pMyHuman->m_fMaxSpeed = 3.0f;
	}

	unsigned short usGuild = g_pObjectManager->m_stSelCharData.Guild[g_pObjectManager->m_cCharacterSlot];
	m_pMyHuman->m_sGuildLevel = (unsigned char)pMobData->GuildLevel;
	m_pMyHuman->m_usGuild = usGuild;

	SGridControl* pGridTradeOp1 = (SGridControl*)m_pControlContainer->FindControl(8192);
	SGridControl* pGridTradeOp2 = (SGridControl*)m_pControlContainer->FindControl(8193);
	SGridControl* pGridTradeOp3 = (SGridControl*)m_pControlContainer->FindControl(8194);
	SGridControl* pGridTradeOp4 = (SGridControl*)m_pControlContainer->FindControl(8195);
	SGridControl* pGridTradeOp5 = (SGridControl*)m_pControlContainer->FindControl(8196);
	SGridControl* pGridTradeOp6 = (SGridControl*)m_pControlContainer->FindControl(8197);
	SGridControl* pGridTradeOp7 = (SGridControl*)m_pControlContainer->FindControl(8198);
	SGridControl* pGridTradeOp8 = (SGridControl*)m_pControlContainer->FindControl(8199);
	SGridControl* pGridTradeOp9 = (SGridControl*)m_pControlContainer->FindControl(8200);
	SGridControl* pGridTradeOp10 = (SGridControl*)m_pControlContainer->FindControl(8201);
	SGridControl* pGridTradeOp11 = (SGridControl*)m_pControlContainer->FindControl(8202);
	SGridControl* pGridTradeOp12 = (SGridControl*)m_pControlContainer->FindControl(8203);
	SGridControl* pGridTradeOp13 = (SGridControl*)m_pControlContainer->FindControl(8204);
	SGridControl* pGridTradeOp14 = (SGridControl*)m_pControlContainer->FindControl(8205);
	SGridControl* pGridTradeOp15 = (SGridControl*)m_pControlContainer->FindControl(8206);

	SGridControl* pTradeOpGrids[15] = {
		pGridTradeOp1, pGridTradeOp2, pGridTradeOp3, pGridTradeOp4, pGridTradeOp5,
		pGridTradeOp6, pGridTradeOp7, pGridTradeOp8, pGridTradeOp9, pGridTradeOp10,
		pGridTradeOp11, pGridTradeOp12, pGridTradeOp13, pGridTradeOp14, pGridTradeOp15 };
	for (auto pGrid : pTradeOpGrids)
	{
		if (pGrid)
			pGrid->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	}

	SGridControl* pGridTradeMy1 = (SGridControl*)m_pControlContainer->FindControl(8448);
	SGridControl* pGridTradeMy2 = (SGridControl*)m_pControlContainer->FindControl(8449);
	SGridControl* pGridTradeMy3 = (SGridControl*)m_pControlContainer->FindControl(8450);
	SGridControl* pGridTradeMy4 = (SGridControl*)m_pControlContainer->FindControl(8451);
	SGridControl* pGridTradeMy5 = (SGridControl*)m_pControlContainer->FindControl(8452);
	SGridControl* pGridTradeMy6 = (SGridControl*)m_pControlContainer->FindControl(8453);
	SGridControl* pGridTradeMy7 = (SGridControl*)m_pControlContainer->FindControl(8454);
	SGridControl* pGridTradeMy8 = (SGridControl*)m_pControlContainer->FindControl(8455);
	SGridControl* pGridTradeMy9 = (SGridControl*)m_pControlContainer->FindControl(8456);
	SGridControl* pGridTradeMy10 = (SGridControl*)m_pControlContainer->FindControl(8457);
	SGridControl* pGridTradeMy11 = (SGridControl*)m_pControlContainer->FindControl(8458);
	SGridControl* pGridTradeMy12 = (SGridControl*)m_pControlContainer->FindControl(8459);
	SGridControl* pGridTradeMy13 = (SGridControl*)m_pControlContainer->FindControl(8460);
	SGridControl* pGridTradeMy14 = (SGridControl*)m_pControlContainer->FindControl(8461);
	SGridControl* pGridTradeMy15 = (SGridControl*)m_pControlContainer->FindControl(8462);

	SGridControl* pTradeMyGrids[15] = {
		pGridTradeMy1, pGridTradeMy2, pGridTradeMy3, pGridTradeMy4, pGridTradeMy5,
		pGridTradeMy6, pGridTradeMy7, pGridTradeMy8, pGridTradeMy9, pGridTradeMy10,
		pGridTradeMy11, pGridTradeMy12, pGridTradeMy13, pGridTradeMy14, pGridTradeMy15 };
	for (auto pGrid : pTradeMyGrids)
	{
		if (pGrid)
			pGrid->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	}

	SControl* pOpCheckButton = m_pControlContainer->FindControl(601);
	SControl* pMyCheckButton = m_pControlContainer->FindControl(617);

	m_pCargoGridList[0] = (SGridControl*)m_pControlContainer->FindControl(67328);
	m_pCargoGridList[1] = (SGridControl*)m_pControlContainer->FindControl(67329);
	m_pCargoGridList[2] = (SGridControl*)m_pControlContainer->FindControl(67330);
	m_pCargoGrid = m_pCargoGridList[0];
	m_pCargoGridList[0]->SetVisible(0);
	m_pCargoGridList[1]->SetVisible(0);
	m_pCargoGridList[2]->SetVisible(0);
	m_pCargoGrid->SetVisible(1);
	m_pCargoGridList[0]->m_bDrawGrid = 1;
	m_pCargoGridList[0]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
	m_pCargoGridList[1]->m_bDrawGrid = 1;
	m_pCargoGridList[1]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
	m_pCargoGridList[2]->m_bDrawGrid = 1;
	m_pCargoGridList[2]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;

	m_pGridInvList[0] = (SGridControl*)m_pControlContainer->FindControl(67072);
	m_pGridInvList[1] = (SGridControl*)m_pControlContainer->FindControl(67073);
	m_pGridInvList[2] = (SGridControl*)m_pControlContainer->FindControl(67074);
	m_pGridInvList[3] = (SGridControl*)m_pControlContainer->FindControl(67075);
	m_pGridInvList[0]->SetVisible(0);
	m_pGridInvList[1]->SetVisible(0);
	m_pGridInvList[2]->SetVisible(0);
	m_pGridInvList[3]->SetVisible(0);
	m_pGridInv = m_pGridInvList[0];
	m_pGridInv->SetVisible(1);

	//Arena Real @SkyDrive 16/08/2022
	m_pArenaGamePanel = (SPanel*)m_pControlContainer->FindControl(90631u);
	if(m_pArenaGamePanel)
		m_pArenaGamePanel->SetVisible(0);

	m_pArenaGameScorePanel = (SPanel*)m_pControlContainer->FindControl(90624u);
	if(m_pArenaGameScorePanel)
		m_pArenaGameScorePanel->SetVisible(0);

	m_pQuicInvDelete = (SGridControl*)m_pControlContainer->FindControl(67080);
	m_pQuicInvDelete->m_eGridType = TMEGRIDTYPE::GRID_DELETE;
	m_pGridHelm = (SGridControl*)m_pControlContainer->FindControl(65555);
	m_pGridCoat = (SGridControl*)m_pControlContainer->FindControl(65558);
	m_pGridPants = (SGridControl*)m_pControlContainer->FindControl(65559);
	m_pGridGloves = (SGridControl*)m_pControlContainer->FindControl(65560);
	m_pGridBoots = (SGridControl*)m_pControlContainer->FindControl(65561);
	m_pGridRight = (SGridControl*)m_pControlContainer->FindControl(65556);
	m_pGridLeft = (SGridControl*)m_pControlContainer->FindControl(65557);
	m_pGridGuild = (SGridControl*)m_pControlContainer->FindControl(65548);
	m_pGridEvent = (SGridControl*)m_pControlContainer->FindControl(65549);
	m_pGridRing = (SGridControl*)m_pControlContainer->FindControl(65550);
	m_pGridNecklace = (SGridControl*)m_pControlContainer->FindControl(65551);
	m_pGridOrb = (SGridControl*)m_pControlContainer->FindControl(65552);
	m_pGridCabuncle = (SGridControl*)m_pControlContainer->FindControl(65553);
	m_pGridDRing = (SGridControl*)m_pControlContainer->FindControl(65546);
	m_pGridMantua = (SGridControl*)m_pControlContainer->FindControl(65547);
	m_pGridSkillMaster = (SGridControl*)m_pControlContainer->FindControl(65607);
	m_pGridShop = (SGridControl*)m_pControlContainer->FindControl(65694);
	m_pGridNewSlot1 = (SGridControl*)m_pControlContainer->FindControl(1048976);
	m_pGridNewSlot2 = (SGridControl*)m_pControlContainer->FindControl(1048977);
	m_pBTNCLOSEINVENT = (SButton*)m_pControlContainer->FindControl(65562);
	m_pBTNgoldINVENT = (SButton*)m_pControlContainer->FindControl(65563);
	m_pmsgskill = (SText*)m_pControlContainer->FindControl(65569);
	m_btncloseskill = (SButton*)m_pControlContainer->FindControl(65568);
	m_pGridSkillBelt = (SGridControl*)m_pControlContainer->FindControl(65598);
	m_pSlotGuardaCarga[0] = (SButton*)m_pControlContainer->FindControl(3000);
	m_pSlotGuardaCarga[1] = (SButton*)m_pControlContainer->FindControl(3001);
	m_pSlotGuardaCarga[2] = (SButton*)m_pControlContainer->FindControl(3002);
	m_pSlotGuardaCarga[3] = (SButton*)m_pControlContainer->FindControl(3003);
	m_pSlotGuardaCarga[4] = (SButton*)m_pControlContainer->FindControl(3004);
	m_pSlotGuardaCarga[5] = (SButton*)m_pControlContainer->FindControl(3005);
	m_pSlotGuardaCarga[6] = (SButton*)m_pControlContainer->FindControl(3006);
	m_pSlotGuardaCarga[7] = (SButton*)m_pControlContainer->FindControl(3007);
	m_pSlotGuardaCarga[8] = (SButton*)m_pControlContainer->FindControl(3008);
	m_pSlotGuardaCarga[9] = (SButton*)m_pControlContainer->FindControl(3009);
	m_pSlotGuardaCarga[10] = (SButton*)m_pControlContainer->FindControl(3010);
	m_pSlotGuardaCarga[11] = (SButton*)m_pControlContainer->FindControl(3011);
	m_pSlotGuardaCarga[12] = (SButton*)m_pControlContainer->FindControl(3012);
	m_pSlotGuardaCarga[13] = (SButton*)m_pControlContainer->FindControl(3013);
	m_pSlotGuardaCarga[14] = (SButton*)m_pControlContainer->FindControl(3014);
	m_pSlotGuardaCarga[15] = (SButton*)m_pControlContainer->FindControl(3015);
	m_pSlotGuardaCarga[16] = (SButton*)m_pControlContainer->FindControl(3016);
	m_pSlotGuardaCarga[17] = (SButton*)m_pControlContainer->FindControl(3017);
	m_pSlotGuardaCarga[18] = (SButton*)m_pControlContainer->FindControl(3018);
	m_pSlotGuardaCarga[19] = (SButton*)m_pControlContainer->FindControl(3019);
	m_pSlotGuardaCarga[20] = (SButton*)m_pControlContainer->FindControl(3020);
	m_pSlotGuardaCarga[21] = (SButton*)m_pControlContainer->FindControl(3021);
	m_pSlotGuardaCarga[22] = (SButton*)m_pControlContainer->FindControl(3022);
	m_pSlotGuardaCarga[23] = (SButton*)m_pControlContainer->FindControl(3023);
	m_pSlotGuardaCarga[24] = (SButton*)m_pControlContainer->FindControl(3024);
	m_pSlotGuardaCarga[25] = (SButton*)m_pControlContainer->FindControl(3025);
	m_pSlotGuardaCarga[26] = (SButton*)m_pControlContainer->FindControl(3026);
	m_pSlotGuardaCarga[27] = (SButton*)m_pControlContainer->FindControl(3027);
	m_pSlotGuardaCarga[28] = (SButton*)m_pControlContainer->FindControl(3028);
	m_pSlotGuardaCarga[29] = (SButton*)m_pControlContainer->FindControl(3029);
	m_pSlotGuardaCarga[30] = (SButton*)m_pControlContainer->FindControl(3030);
	m_pSlotGuardaCarga[31] = (SButton*)m_pControlContainer->FindControl(3031);
	m_pSlotGuardaCarga[32] = (SButton*)m_pControlContainer->FindControl(3032);
	m_pSlotGuardaCarga[33] = (SButton*)m_pControlContainer->FindControl(3033);
	m_pSlotGuardaCarga[34] = (SButton*)m_pControlContainer->FindControl(3034);
	m_pSlotGuardaCarga[35] = (SButton*)m_pControlContainer->FindControl(3035);
	m_pSlotGuardaCarga[36] = (SButton*)m_pControlContainer->FindControl(3036);
	m_pSlotGuardaCarga[37] = (SButton*)m_pControlContainer->FindControl(3037);
	m_pSlotGuardaCarga[38] = (SButton*)m_pControlContainer->FindControl(3038);
	m_pSlotGuardaCarga[39] = (SButton*)m_pControlContainer->FindControl(3039);
	m_pSlotSkill[0] = (SButton*)m_pControlContainer->FindControl(66544);
	m_pSlotSkill[1] = (SButton*)m_pControlContainer->FindControl(66543);
	m_pSlotSkill[2] = (SButton*)m_pControlContainer->FindControl(66542);
	m_pSlotSkill[3] = (SButton*)m_pControlContainer->FindControl(66541);
	m_pSlotSkill[4] = (SButton*)m_pControlContainer->FindControl(66540);
	m_pSlotSkill[5] = (SButton*)m_pControlContainer->FindControl(66539);
	m_pSlotSkill[6] = (SButton*)m_pControlContainer->FindControl(66538);
	m_pSlotSkill[7] = (SButton*)m_pControlContainer->FindControl(66537);
	m_pSlotSkill[8] = (SButton*)m_pControlContainer->FindControl(66536);
	m_pSlotSkill[9] = (SButton*)m_pControlContainer->FindControl(66535);
	m_pSlotSkill[10] = (SButton*)m_pControlContainer->FindControl(66534);
	m_pSlotSkill[11] = (SButton*)m_pControlContainer->FindControl(66533);
	m_pSlotSkill[12] = (SButton*)m_pControlContainer->FindControl(66532);
	m_pSlotSkill[13] = (SButton*)m_pControlContainer->FindControl(66531);
	m_pSlotSkill[14] = (SButton*)m_pControlContainer->FindControl(66530);
	m_pSlotSkill[15] = (SButton*)m_pControlContainer->FindControl(66529);
	m_pSlotSkill[16] = (SButton*)m_pControlContainer->FindControl(66528);
	m_pSlotSkill[17] = (SButton*)m_pControlContainer->FindControl(66526);
	m_pSlotSkill[18] = (SButton*)m_pControlContainer->FindControl(66525);
	m_pSlotSkill[19] = (SButton*)m_pControlContainer->FindControl(66524);
	m_pSlotSkill[20] = (SButton*)m_pControlContainer->FindControl(66523);
	m_pSlotSkill[21] = (SButton*)m_pControlContainer->FindControl(66522);
	m_pSlotSkill[22] = (SButton*)m_pControlContainer->FindControl(66521);
	m_pSlotSkill[23] = (SButton*)m_pControlContainer->FindControl(66520);
	m_pSlotSkill[24] = (SButton*)m_pControlContainer->FindControl(66519);
	m_pSlotSkill[25] = (SButton*)m_pControlContainer->FindControl(66518);
	m_pSlotSkill[26] = (SButton*)m_pControlContainer->FindControl(66517);
	m_pSlotSkill[27] = (SButton*)m_pControlContainer->FindControl(66516);
	m_pSlotSkill[28] = (SButton*)m_pControlContainer->FindControl(66515);
	m_pSlotSkill[29] = (SButton*)m_pControlContainer->FindControl(66514);
	m_pSlotSkill[30] = (SButton*)m_pControlContainer->FindControl(66513);
	m_pSlotSkill[31] = (SButton*)m_pControlContainer->FindControl(66512);
	m_pSlotSkill[32] = (SButton*)m_pControlContainer->FindControl(66511);
	m_pSlotSkill[33] = (SButton*)m_pControlContainer->FindControl(66510);
	m_pSlotSkill[34] = (SButton*)m_pControlContainer->FindControl(66509);
	m_pSlotSkill[35] = (SButton*)m_pControlContainer->FindControl(66508);
	m_pSlotSkill[36] = (SButton*)m_pControlContainer->FindControl(66507);
	m_pSlotSkill[37] = (SButton*)m_pControlContainer->FindControl(66506);
	m_pSlotSkill[38] = (SButton*)m_pControlContainer->FindControl(66505);
	m_pSlotSkill[39] = (SButton*)m_pControlContainer->FindControl(66504);
	m_pSlotSkill[40] = (SButton*)m_pControlContainer->FindControl(66503);
	m_pSlotSkill[41] = (SButton*)m_pControlContainer->FindControl(66502);
	m_pSlotSkill[42] = (SButton*)m_pControlContainer->FindControl(66501);
	m_pSlotSkill[43] = (SButton*)m_pControlContainer->FindControl(66500);
	m_pSlotSkill[44] = (SButton*)m_pControlContainer->FindControl(66499);
	m_pSlotSkill[45] = (SButton*)m_pControlContainer->FindControl(66498);
	m_pSlotSkill[46] = (SButton*)m_pControlContainer->FindControl(66497);
	m_pSlotSkill[47] = (SButton*)m_pControlContainer->FindControl(66496);
	for (int i = 0; i < 40; ++i)
	{
		m_pSlotShop[i] = (SButton*)m_pControlContainer->FindControl(i + 3040);
	}
	for (int i = 0; i < 32; ++i)
	{
		m_pSlotInventori[i] = (SButton*)m_pControlContainer->FindControl(i + 3080);
	}
	for (int i = 0; i < 24; ++i)
	{
		m_pSlotSkillShop[i] = (SButton*)m_pControlContainer->FindControl(i + 3120);
	}
	for (int i = 0; i < 32; ++i)
	{
		m_pSlotComp[i] = (SButton*)m_pControlContainer->FindControl(i + 3150);
	}
	for (int i = 0; i < 24; ++i)
	{
		m_pGridResultItem[i] = (SGridControl*)m_pControlContainer->FindControl(i + 81924);
	}
	for (int i = 0; i < 30; ++i)
	{
		m_pSlotTrade[i] = (SButton*)m_pControlContainer->FindControl(i + 3250);
	}
	m_pGridResultItem[25] = (SGridControl*)m_pControlContainer->FindControl(81981);
	m_pGridResultItem[26] = (SGridControl*)m_pControlContainer->FindControl(81982);
	m_pGridResultItem[27] = (SGridControl*)m_pControlContainer->FindControl(81983);
	m_pGridResultItem[28] = (SGridControl*)m_pControlContainer->FindControl(81984);
	m_pGridResultItem[29] = (SGridControl*)m_pControlContainer->FindControl(81985);
	m_pGridResultItem[30] = (SGridControl*)m_pControlContainer->FindControl(81986);
	m_pGridResultItem[31] = (SGridControl*)m_pControlContainer->FindControl(81987);
	m_pGridResultItem[32] = (SGridControl*)m_pControlContainer->FindControl(81988);
	m_pMixPanel2 = (SPanel*)m_pControlContainer->FindControl(81921);
	m_pMixPanel1 = (SPanel*)m_pControlContainer->FindControl(81989);
	m_pMixPanel[1] = (SPanel*)m_pControlContainer->FindControl(81959);
	m_pMixPanel[2] = (SPanel*)m_pControlContainer->FindControl(81951);
	m_pMixPanel[3] = (SPanel*)m_pControlContainer->FindControl(81960);
	m_pMixPanel[4] = (SPanel*)m_pControlContainer->FindControl(81952);
	m_pMixPanel[5] = (SPanel*)m_pControlContainer->FindControl(81961);
	m_pMixPanel[6] = (SPanel*)m_pControlContainer->FindControl(81953);
	m_pMixPanel[7] = (SPanel*)m_pControlContainer->FindControl(81962);
	m_pMixPanel[8] = (SPanel*)m_pControlContainer->FindControl(81954);
	m_pMixPanel[9] = (SPanel*)m_pControlContainer->FindControl(81963);
	m_pMixPanel[10] = (SPanel*)m_pControlContainer->FindControl(81955);
	m_pMixPanel[11] = (SPanel*)m_pControlContainer->FindControl(81964);
	m_pMixPanel[12] = (SPanel*)m_pControlContainer->FindControl(81956);
	m_pMixPanel[13] = (SPanel*)m_pControlContainer->FindControl(81965);
	m_pMixPanel[14] = (SPanel*)m_pControlContainer->FindControl(81957);
	m_pMixPanel[15] = (SPanel*)m_pControlContainer->FindControl(81966);
	m_pMixPanel[16] = (SPanel*)m_pControlContainer->FindControl(81958);
	m_pItemMixPanel45 = (SPanel*)m_pControlContainer->FindControl(6440);
	m_pCompPanel[0] = (SButton*)m_pControlContainer->FindControl(3200);
	m_pCompPanel[1] = (SButton*)m_pControlContainer->FindControl(3201);
	m_pCompPanel[2] = (SButton*)m_pControlContainer->FindControl(3202);
	m_pCompbuton = (SButton*)m_pControlContainer->FindControl(6435);
	m_pCompbuton1 = (SButton*)m_pControlContainer->FindControl(6434);
	m_pCompvalue = (SText*)m_pControlContainer->FindControl(6452);
	m_pComptext = (SText*)m_pControlContainer->FindControl(6433);
	m_pMixPanelText = (SText*)m_pControlContainer->FindControl(81922);
	m_pMixPanelTextbuton = (SText*)m_pControlContainer->FindControl(81923);
	m_pPanelname = (SText*)m_pControlContainer->FindControl(65545u);

	m_pSelectServerPanel = (SPanel*)m_pControlContainer->FindControl(65910);

	m_pDropListPanel = (SPanel*)m_pControlContainer->FindControl(4784782);
	m_ListBoxPanel = (SListBox*)m_pControlContainer->FindControl(478472);
	m_GridPanel = (SGridControl*)m_pControlContainer->FindControl(478473);
	m_GridPanel1 = (SPanel*)m_pControlContainer->FindControl(478492);
	m_pPanelSend = (SButton*)m_pControlContainer->FindControl(478480);
	m_pPanelDropClose = (SButton*)m_pControlContainer->FindControl(478486);

	m_pMixPanelTextDrop[0] = (SText*)m_pControlContainer->FindControl(478476);
	m_pMixPanelTextDrop[1] = (SText*)m_pControlContainer->FindControl(478477);
	m_pMixPanelTextDrop[2] = (SText*)m_pControlContainer->FindControl(478481);
	m_pMixPanelTextDrop[3] = (SText*)m_pControlContainer->FindControl(478482);
	m_pMixPanelTextDrop[4] = (SText*)m_pControlContainer->FindControl(478483);
	m_pMixPanelTextDrop[5] = (SText*)m_pControlContainer->FindControl(478487);
	m_pMixPanelTextDrop[6] = (SText*)m_pControlContainer->FindControl(478488);
	m_pMixPanelTextDrop[7] = (SText*)m_pControlContainer->FindControl(478489);
	m_pMixPanelTextDrop[8] = (SText*)m_pControlContainer->FindControl(478490);
	m_pMixPanelTextDrop[9] = (SText*)m_pControlContainer->FindControl(478491);

	m_pbutonShop = (SButton*)m_pControlContainer->FindControl(656433);
	m_pbutonShop->m_cAlwaysAlt = 1;// blinking button
	m_pbutonShop->m_cBlink = 1;

	m_pbutonDrop = (SButton*)m_pControlContainer->FindControl(656434);
	// The 7.69 drop-list request has no 7.48 server contract.
	// Do not expose a panel that cannot load authoritative contents.
	if (m_pbutonDrop)
		m_pbutonDrop->SetVisible(false);
	if (auto* dropPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(478471)))
		dropPanel->SetVisible(false);

	m_pbutonNewShop[0] = (SButton*)m_pControlContainer->FindControl(3000100);
	m_pbutonNewShop[0]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[0]->m_cBlink = 1;
	m_pbutonNewShop[1] = (SButton*)m_pControlContainer->FindControl(3000101);
	m_pbutonNewShop[1]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[1]->m_cBlink = 1;
	m_pbutonNewShop[2] = (SButton*)m_pControlContainer->FindControl(3000102);
	m_pbutonNewShop[2]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[2]->m_cBlink = 1;
	m_pbutonNewShop[3] = (SButton*)m_pControlContainer->FindControl(3000103);
	m_pbutonNewShop[3]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[3]->m_cBlink = 1;
	m_pbutonNewShop[4] = (SButton*)m_pControlContainer->FindControl(3000104);
	m_pbutonNewShop[4]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[4]->m_cBlink = 1;
	m_pbutonNewShop[5] = (SButton*)m_pControlContainer->FindControl(3000105);
	m_pbutonNewShop[5]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[5]->m_cBlink = 1;
	m_pbutonNewShop[6] = (SButton*)m_pControlContainer->FindControl(3000106);
	m_pbutonNewShop[6]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[6]->m_cBlink = 1;
	m_pbutonNewShop[7] = (SButton*)m_pControlContainer->FindControl(3000107);
	m_pbutonNewShop[7]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[7]->m_cBlink = 1;
	m_pbutonNewShop[8] = (SButton*)m_pControlContainer->FindControl(3000108);
	m_pbutonNewShop[8]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[8]->m_cBlink = 1;
	m_pbutonNewShop[9] = (SButton*)m_pControlContainer->FindControl(3000109);
	m_pbutonNewShop[9]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[9]->m_cBlink = 1;
	m_pbutonNewShop[10] = (SButton*)m_pControlContainer->FindControl(3000110);
	m_pbutonNewShop[10]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[10]->m_cBlink = 1;
	m_pbutonNewShop[11] = (SButton*)m_pControlContainer->FindControl(3000111);
	m_pbutonNewShop[11]->m_cAlwaysAlt = 1;// blinking button
	m_pbutonNewShop[11]->m_cBlink = 1;

	m_pDropListPanel->m_nWidth = BASE_ScreenResize(300.0f);
	m_GridPanel->m_nPosX = BASE_ScreenResize(122.0f);
	m_GridPanel1->m_nPosX = BASE_ScreenResize(120.0f);
	m_pPanelDropClose->m_nPosX = BASE_ScreenResize(280.0f);
	m_pMixPanelTextDrop[8]->m_nPosX = BASE_ScreenResize(240.0f);
	m_pPanelSend->m_nPosX = BASE_ScreenResize(115.0f);
	m_pMixPanelTextDrop[4]->m_nPosX = BASE_ScreenResize(125.0f);

	m_pPotalPanel11 = (SPanel*)m_pControlContainer->FindControl(12550);

	if (g_pApp->m_dwScreenWidth <= 1024)
	{
		///////////////////////hunt list///////////////////////////
		m_pPotalPanel11->m_nWidth = BASE_ScreenResize(330.0f);

		///////////////////////Drop List///////////////////////////
		m_pDropListPanel->m_nWidth = BASE_ScreenResize(380.0f);

		m_ListBoxPanel->m_nWidth = BASE_ScreenResize(100.0f);
		m_ListBoxPanel->m_nPosX = BASE_ScreenResize(30.0f);

		m_GridPanel->m_nWidth = BASE_ScreenResize(233.0f);
		m_GridPanel->m_nPosX = BASE_ScreenResize(143.0f);

		m_GridPanel1->m_nWidth = BASE_ScreenResize(230.0f);
		m_GridPanel1->m_nPosX = BASE_ScreenResize(140.0f);

		m_pPanelSend->m_nPosX = BASE_ScreenResize(160.0f);
		m_pPanelDropClose->m_nPosX = BASE_ScreenResize(350.0f);

		m_pMixPanelTextDrop[4]->m_nPosX = BASE_ScreenResize(160.0f);
		m_pMixPanelTextDrop[2]->m_nPosX = BASE_ScreenResize(135.0f);
		m_pMixPanelTextDrop[3]->m_nPosX = BASE_ScreenResize(230.0f);
		m_pMixPanelTextDrop[7]->m_nPosX = BASE_ScreenResize(170.0f);//
		m_pMixPanelTextDrop[8]->m_nPosX = BASE_ScreenResize(290.0f);

		///////////////////////trade///////////////////////////
		if (pOpCheckButton)
			pOpCheckButton->m_nPosX = BASE_ScreenResize(208.0f);
		if (pMyCheckButton)
			pMyCheckButton->m_nPosX = BASE_ScreenResize(208.0f);
		if (pTradePanel)
		{
			pTradePanel->m_nPosX = BASE_ScreenResize(310.0f);
			pTradePanel->m_nWidth = BASE_ScreenResize(230.0f);
		}
		if (pTradePanel1)
			pTradePanel1->m_nWidth = BASE_ScreenResize(230.0f);

		const float tradeGridX[5] = { 13.5f, 55.4f, 97.3f, 139.2f, 181.1f };
		for (int i = 0; i < 30; ++i)
		{
			if (m_pSlotTrade[i])
			{
				m_pSlotTrade[i]->m_nWidth = BASE_ScreenResize(35.0f);
				m_pSlotTrade[i]->m_nPosX = BASE_ScreenResize(tradeGridX[i % 5]);
			}
		}

		for (int i = 0; i < 15; ++i)
		{
			if (pTradeOpGrids[i])
				pTradeOpGrids[i]->m_nPosX = BASE_ScreenResize(tradeGridX[i % 5]);
			if (pTradeMyGrids[i])
				pTradeMyGrids[i]->m_nPosX = BASE_ScreenResize(tradeGridX[i % 5]);
		}

		///////////////////////composition///////////////////////////
		m_pCompbuton->m_nPosX = BASE_ScreenResize(256.0f);
		m_pCompbuton1->m_nPosX = BASE_ScreenResize(256.0f);
		m_pCompvalue->m_nPosX = BASE_ScreenResize(270.0f);
		m_pComptext->m_nPosX = BASE_ScreenResize(235.0f);
		m_pGridItemMix4[0]->m_nPosX = BASE_ScreenResize(246.0f);
		m_pGridItemMix4[1]->m_nPosX = BASE_ScreenResize(303.0f);
		m_pGridItemMix4[2]->m_nPosX = BASE_ScreenResize(264.0f);
		m_pItemMixPanel4->m_nPosY = BASE_ScreenResize(80.0f);
		m_pItemMixPanel4->m_nPosX = BASE_ScreenResize(140.0f);
		m_pItemMixPanel4->m_nWidth = BASE_ScreenResize(100.0f);
		m_pItemMixPanel45->m_nWidth = BASE_ScreenResize(220.0f);

		m_pItemMixPanel45->m_nPosX = BASE_ScreenResize(180.0f);

		m_pMix4Desc->m_nPosX = BASE_ScreenResize(200.0f);
		for (int i = 0; i < 3; ++i)
			m_pCompPanel[i]->m_nWidth = BASE_ScreenResize(35.0f);
		m_pCompPanel[0]->m_nPosX = BASE_ScreenResize(246.0f);
		m_pCompPanel[1]->m_nPosX = BASE_ScreenResize(303.0f);
		m_pCompPanel[2]->m_nPosX = BASE_ScreenResize(273.0f);
		///////////////////////composition///////////////////////////
		m_pMixPanel2->m_nWidth = BASE_ScreenResize(375.0f);
		m_pMixPanel1->m_nWidth = BASE_ScreenResize(375.0f);
		m_pMixPanel1->m_nPosX = BASE_ScreenResize(-80.0f);
		m_pMixPanelText->m_nPosX = BASE_ScreenResize(45.0f);
		m_pMixPanelTextbuton->m_nPosX = BASE_ScreenResize(70.0f);
		for (int i = 0; i < 32; ++i)
			m_pSlotComp[i]->m_nWidth = BASE_ScreenResize(35.0f);
		m_pSlotComp[0]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pSlotComp[1]->m_nPosX = BASE_ScreenResize(-15.0f);
		m_pSlotComp[2]->m_nPosX = BASE_ScreenResize(27.0f);
		m_pSlotComp[3]->m_nPosX = BASE_ScreenResize(69.5f);
		m_pSlotComp[4]->m_nPosX = BASE_ScreenResize(112.0f);
		m_pSlotComp[5]->m_nPosX = BASE_ScreenResize(155.0f);
		m_pSlotComp[6]->m_nPosX = BASE_ScreenResize(197.5f);
		m_pSlotComp[7]->m_nPosX = BASE_ScreenResize(240.0f);
		m_pSlotComp[8]->m_nPosX = BASE_ScreenResize(-58.0f);
		m_pSlotComp[9]->m_nPosX = BASE_ScreenResize(-15.0f);
		m_pSlotComp[10]->m_nPosX = BASE_ScreenResize(27.0f);
		m_pSlotComp[11]->m_nPosX = BASE_ScreenResize(69.5f);
		m_pSlotComp[12]->m_nPosX = BASE_ScreenResize(112.0f);
		m_pSlotComp[13]->m_nPosX = BASE_ScreenResize(155.0f);
		m_pSlotComp[14]->m_nPosX = BASE_ScreenResize(197.5f);
		m_pSlotComp[15]->m_nPosX = BASE_ScreenResize(240.0f);
		m_pSlotComp[16]->m_nPosX = BASE_ScreenResize(-58.0f);
		m_pSlotComp[17]->m_nPosX = BASE_ScreenResize(-15.0f);
		m_pSlotComp[18]->m_nPosX = BASE_ScreenResize(27.0f);
		m_pSlotComp[19]->m_nPosX = BASE_ScreenResize(69.5f);
		m_pSlotComp[20]->m_nPosX = BASE_ScreenResize(112.0f);
		m_pSlotComp[21]->m_nPosX = BASE_ScreenResize(155.0f);
		m_pSlotComp[22]->m_nPosX = BASE_ScreenResize(197.5f);
		m_pSlotComp[23]->m_nPosX = BASE_ScreenResize(240.0f);
		m_pSlotComp[24]->m_nPosX = BASE_ScreenResize(-58.0f);
		m_pSlotComp[25]->m_nPosX = BASE_ScreenResize(112.0f);
		m_pSlotComp[26]->m_nPosX = BASE_ScreenResize(-58.0f);
		m_pSlotComp[27]->m_nPosX = BASE_ScreenResize(112.0f);
		m_pSlotComp[28]->m_nPosX = BASE_ScreenResize(-58.0f);
		m_pSlotComp[29]->m_nPosX = BASE_ScreenResize(112.0f);
		m_pSlotComp[30]->m_nPosX = BASE_ScreenResize(-58.0f);
		m_pSlotComp[31]->m_nPosX = BASE_ScreenResize(112.0f);
		m_pGridResultItem[0]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pGridResultItem[1]->m_nPosX = BASE_ScreenResize(-13.0f);
		m_pGridResultItem[2]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pGridResultItem[3]->m_nPosX = BASE_ScreenResize(70.5f);
		m_pGridResultItem[4]->m_nPosX = BASE_ScreenResize(113.0f);
		m_pGridResultItem[5]->m_nPosX = BASE_ScreenResize(156.0f);
		m_pGridResultItem[6]->m_nPosX = BASE_ScreenResize(198.5f);
		m_pGridResultItem[7]->m_nPosX = BASE_ScreenResize(241.0f);
		m_pGridResultItem[8]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pGridResultItem[9]->m_nPosX = BASE_ScreenResize(-13.0f);
		m_pGridResultItem[10]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pGridResultItem[11]->m_nPosX = BASE_ScreenResize(70.5f);
		m_pGridResultItem[12]->m_nPosX = BASE_ScreenResize(113.0f);
		m_pGridResultItem[13]->m_nPosX = BASE_ScreenResize(156.0f);
		m_pGridResultItem[14]->m_nPosX = BASE_ScreenResize(199.5f);
		m_pGridResultItem[15]->m_nPosX = BASE_ScreenResize(241.0f);
		m_pGridResultItem[16]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pGridResultItem[17]->m_nPosX = BASE_ScreenResize(-13.0f);
		m_pGridResultItem[18]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pGridResultItem[19]->m_nPosX = BASE_ScreenResize(70.5f);
		m_pGridResultItem[20]->m_nPosX = BASE_ScreenResize(113.0f);
		m_pGridResultItem[21]->m_nPosX = BASE_ScreenResize(156.0f);
		m_pGridResultItem[22]->m_nPosX = BASE_ScreenResize(198.5f);
		m_pGridResultItem[23]->m_nPosX = BASE_ScreenResize(241.0f);
		m_pGridResultItem[25]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pGridResultItem[26]->m_nPosX = BASE_ScreenResize(113.0f);
		m_pGridResultItem[27]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pGridResultItem[28]->m_nPosX = BASE_ScreenResize(113.0f);
		m_pGridResultItem[29]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pGridResultItem[30]->m_nPosX = BASE_ScreenResize(113.0f);
		m_pGridResultItem[31]->m_nPosX = BASE_ScreenResize(-56.0f);
		m_pGridResultItem[32]->m_nPosX = BASE_ScreenResize(113.0f);
		m_pMixPanel[1]->m_nPosX = BASE_ScreenResize(-18.0f);
		m_pMixPanel[2]->m_nPosX = BASE_ScreenResize(15.0f);
		m_pMixPanel[3]->m_nPosX = BASE_ScreenResize(150.0f);
		m_pMixPanel[4]->m_nPosX = BASE_ScreenResize(180.0f);
		m_pMixPanel[5]->m_nPosX = BASE_ScreenResize(-18.0f);
		m_pMixPanel[6]->m_nPosX = BASE_ScreenResize(15.0f);
		m_pMixPanel[7]->m_nPosX = BASE_ScreenResize(150.0f);
		m_pMixPanel[8]->m_nPosX = BASE_ScreenResize(180.0f);
		m_pMixPanel[9]->m_nPosX = BASE_ScreenResize(-18.0f);
		m_pMixPanel[10]->m_nPosX = BASE_ScreenResize(15.0f);
		m_pMixPanel[11]->m_nPosX = BASE_ScreenResize(150.0f);
		m_pMixPanel[12]->m_nPosX = BASE_ScreenResize(180.0f);
		m_pMixPanel[13]->m_nPosX = BASE_ScreenResize(-18.0f);
		m_pMixPanel[14]->m_nPosX = BASE_ScreenResize(15.0f);
		m_pMixPanel[15]->m_nPosX = BASE_ScreenResize(150.0f);
		m_pMixPanel[16]->m_nPosX = BASE_ScreenResize(180.0f);
		////////////////////////////////skill Shop///////////////////////////
		m_pSkillMPanel->m_nWidth = BASE_ScreenResize(225.0f);
		pSkillMPanel1->m_nWidth = BASE_ScreenResize(225.0f);
		m_pSkillMPanel->m_nPosX = BASE_ScreenResize(92.0f);
		m_pGridSkillMaster->m_nPosX = BASE_ScreenResize(30.0f);
		m_pGridSkillMaster->m_nWidth = BASE_ScreenResize(192.0f);

		for (int i = 0; i < 24; ++i)
			m_pSlotSkillShop[i]->m_nWidth = BASE_ScreenResize(24.0f);
		for (int i = 0; i < 24; ++i)
			m_pSlotSkillShop[i]->m_nHeight = BASE_ScreenResize(24.0f);

		m_pSlotSkillShop[0]->m_nPosX = BASE_ScreenResize(30.0f);
		m_pSlotSkillShop[1]->m_nPosX = BASE_ScreenResize(76.0f);
		m_pSlotSkillShop[2]->m_nPosX = BASE_ScreenResize(125.0f);
		m_pSlotSkillShop[3]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkillShop[4]->m_nPosX = BASE_ScreenResize(30.0f);
		m_pSlotSkillShop[5]->m_nPosX = BASE_ScreenResize(76.0f);
		m_pSlotSkillShop[6]->m_nPosX = BASE_ScreenResize(125.0f);
		m_pSlotSkillShop[7]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkillShop[8]->m_nPosX = BASE_ScreenResize(30.0f);
		m_pSlotSkillShop[9]->m_nPosX = BASE_ScreenResize(76.0f);
		m_pSlotSkillShop[10]->m_nPosX = BASE_ScreenResize(125.0f);
		m_pSlotSkillShop[11]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkillShop[12]->m_nPosX = BASE_ScreenResize(30.0f);
		m_pSlotSkillShop[13]->m_nPosX = BASE_ScreenResize(76.0f);
		m_pSlotSkillShop[14]->m_nPosX = BASE_ScreenResize(125.0f);
		m_pSlotSkillShop[15]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkillShop[16]->m_nPosX = BASE_ScreenResize(30.0f);
		m_pSlotSkillShop[17]->m_nPosX = BASE_ScreenResize(76.0f);
		m_pSlotSkillShop[18]->m_nPosX = BASE_ScreenResize(125.0f);
		m_pSlotSkillShop[19]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkillShop[20]->m_nPosX = BASE_ScreenResize(30.0f);
		m_pSlotSkillShop[21]->m_nPosX = BASE_ScreenResize(76.0f);
		m_pSlotSkillShop[22]->m_nPosX = BASE_ScreenResize(125.0f);
		m_pSlotSkillShop[23]->m_nPosX = BASE_ScreenResize(172.0f);

		////////////////////////////////skill ///////////////////////////
		m_pGridSkillBelt->m_nPosX = BASE_ScreenResize(30.0f);
		m_pGridSkillBelt->m_nWidth = BASE_ScreenResize(175.0f);
		pSkillPanel1->m_nWidth = BASE_ScreenResize(225.0f);
		pSkillBonus->m_nPosX = BASE_ScreenResize(150.0f);
		m_btncloseskill->m_nPosX = BASE_ScreenResize(190.0f);
		m_pmsgskill->m_nPosX = BASE_ScreenResize(80.0f);
		m_pSkillPanel->m_nWidth = BASE_ScreenResize(225.0f);
		m_pSkillSecGrid2[0]->m_nPosX = BASE_ScreenResize(90.0f);
		m_pSkillSecGrid2[1]->m_nPosX = BASE_ScreenResize(118.0f);
		m_pSkillSecGrid2[2]->m_nPosX = BASE_ScreenResize(145.0f);
		m_pSkillSecGrid2[3]->m_nPosX = BASE_ScreenResize(174.0f);
		m_pSkillSecGrid2[4]->m_nPosX = BASE_ScreenResize(90.0f);
		m_pSkillSecGrid2[5]->m_nPosX = BASE_ScreenResize(118.0f);
		m_pSkillSecGrid2[6]->m_nPosX = BASE_ScreenResize(145.0f);
		m_pSkillSecGrid2[7]->m_nPosX = BASE_ScreenResize(174.0f);
		m_pSkillSecGrid2[8]->m_nPosX = BASE_ScreenResize(90.0f);
		m_pSkillSecGrid2[9]->m_nPosX = BASE_ScreenResize(118.0f);
		m_pSkillSecGrid2[10]->m_nPosX = BASE_ScreenResize(145.0f);
		m_pSkillSecGrid2[11]->m_nPosX = BASE_ScreenResize(174.0f);
		m_pSkillSecGrid[0]->m_nPosX = BASE_ScreenResize(32.0f);
		m_pSkillSecGrid[1]->m_nPosX = BASE_ScreenResize(60.0f);
		m_pSkillSecGrid[2]->m_nPosX = BASE_ScreenResize(90.0f);
		m_pSkillSecGrid[3]->m_nPosX = BASE_ScreenResize(118.0f);
		m_pSkillSecGrid[4]->m_nPosX = BASE_ScreenResize(145.0f);
		m_pSkillSecGrid[5]->m_nPosX = BASE_ScreenResize(174.0f);
		m_pSkillSecGrid[6]->m_nPosX = BASE_ScreenResize(32.0f);
		m_pSkillSecGrid[7]->m_nPosX = BASE_ScreenResize(60.0f);
		m_pSkillSecGrid[8]->m_nPosX = BASE_ScreenResize(32.0f);
		m_pSkillSecGrid[9]->m_nPosX = BASE_ScreenResize(60.0f);
		m_pSkillSecGrid[10]->m_nPosX = BASE_ScreenResize(90.0f);
		m_pSkillSecGrid[11]->m_nPosX = BASE_ScreenResize(118.0f);
		m_pSkillSecGrid[12]->m_nPosX = BASE_ScreenResize(145.0f);
		m_pSkillSecGrid[13]->m_nPosX = BASE_ScreenResize(174.0f);
		m_pSkillSecGrid[14]->m_nPosX = BASE_ScreenResize(32.0f);
		m_pSkillSecGrid[15]->m_nPosX = BASE_ScreenResize(60.0f);
		m_pSkillSecGrid[16]->m_nPosX = BASE_ScreenResize(32.0f);
		m_pSkillSecGrid[17]->m_nPosX = BASE_ScreenResize(60.0f);
		m_pSkillSecGrid[18]->m_nPosX = BASE_ScreenResize(90.0f);
		m_pSkillSecGrid[19]->m_nPosX = BASE_ScreenResize(118.0f);
		m_pSkillSecGrid[20]->m_nPosX = BASE_ScreenResize(145.0f);
		m_pSkillSecGrid[21]->m_nPosX = BASE_ScreenResize(174.0f);
		m_pSkillSecGrid[22]->m_nPosX = BASE_ScreenResize(32.0f);
		m_pSkillSecGrid[23]->m_nPosX = BASE_ScreenResize(60.0f);

		for (int i = 0; i < 48; ++i)
			m_pSlotSkill[i]->m_nWidth = BASE_ScreenResize(24.0f);
		for (int i = 0; i < 48; ++i)
			m_pSlotSkill[i]->m_nHeight = BASE_ScreenResize(24.0f);

		m_pSlotSkill[0]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[1]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[2]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[3]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[4]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[5]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkill[6]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[7]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[8]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[9]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[10]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[11]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkill[12]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[13]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[14]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[15]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[16]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[17]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkill[18]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[19]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[20]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[21]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[22]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[23]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkill[24]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[25]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[26]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[27]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[28]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[29]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkill[30]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[31]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[32]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[33]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[34]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[35]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkill[36]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[37]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[38]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[39]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[40]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[41]->m_nPosX = BASE_ScreenResize(172.0f);
		m_pSlotSkill[42]->m_nPosX = BASE_ScreenResize(28.0f);
		m_pSlotSkill[43]->m_nPosX = BASE_ScreenResize(58.0f);
		m_pSlotSkill[44]->m_nPosX = BASE_ScreenResize(87.0f);
		m_pSlotSkill[45]->m_nPosX = BASE_ScreenResize(114.0f);
		m_pSlotSkill[46]->m_nPosX = BASE_ScreenResize(144.0f);
		m_pSlotSkill[47]->m_nPosX = BASE_ScreenResize(172.0f);

		/////////////////////////////////cargo /////////////////////////////////
		pCargoPanel1->m_nWidth = BASE_ScreenResize(230.0f);
		m_pCargoGridList[0]->m_nPosX = BASE_ScreenResize(13.0f);
		m_pCargoGridList[1]->m_nPosX = BASE_ScreenResize(13.0f);
		m_pCargoGridList[2]->m_nPosX = BASE_ScreenResize(13.0f);
		m_pCargoGridList[0]->m_nWidth = BASE_ScreenResize(214.0f);
		m_pCargoGridList[1]->m_nWidth = BASE_ScreenResize(214.0f);
		m_pCargoGridList[2]->m_nWidth = BASE_ScreenResize(214.0f);
		m_pStorePageBtn1->m_nPosX = BASE_ScreenResize(150.0f);
		m_pStorePageBtn2->m_nPosX = BASE_ScreenResize(170.0f);
		m_pStorePageBtn3->m_nPosX = BASE_ScreenResize(190.0f);
		m_pCargoCoin->m_nPosX = BASE_ScreenResize(80.0f);
		for (int i = 0; i < 40; ++i)
			m_pSlotGuardaCarga[i]->m_nWidth = BASE_ScreenResize(35.0f);
		m_pSlotGuardaCarga[0]->m_nPosX = BASE_ScreenResize(12.5f);
		m_pSlotGuardaCarga[1]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[2]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[3]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[4]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotGuardaCarga[5]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotGuardaCarga[6]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[7]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[8]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[9]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotGuardaCarga[10]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotGuardaCarga[11]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[12]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[13]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[14]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotGuardaCarga[15]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotGuardaCarga[16]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[17]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[18]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[19]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotGuardaCarga[20]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotGuardaCarga[21]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[22]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[23]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[24]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotGuardaCarga[25]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotGuardaCarga[26]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[27]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[28]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[29]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotGuardaCarga[30]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotGuardaCarga[31]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[32]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[33]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[34]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotGuardaCarga[35]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotGuardaCarga[36]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotGuardaCarga[37]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotGuardaCarga[38]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotGuardaCarga[39]->m_nPosX = BASE_ScreenResize(184.0f);
		///////////////////////////////// shop /////////////////////////////////

		pShopPanel1->m_nWidth = BASE_ScreenResize(230.0f);
		m_pGridShop->m_nWidth = BASE_ScreenResize(214.0f);
		m_pGridShop->m_nPosX = BASE_ScreenResize(13.0f);

		for (int i = 0; i < 40; ++i)
			m_pSlotShop[i]->m_nWidth = BASE_ScreenResize(35.0f);
		for (int i = 0; i < 40; ++i)
			m_pSlotShop[i]->m_nHeight = BASE_ScreenResize(35.0f);

		m_pSlotShop[0]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[1]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[2]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[3]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[4]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotShop[5]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[6]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[7]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[8]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[9]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotShop[10]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[11]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[12]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[13]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[14]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotShop[15]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[16]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[17]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[18]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[19]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotShop[20]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[21]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[22]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[23]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[24]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotShop[25]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[26]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[27]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[28]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[29]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotShop[30]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[31]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[32]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[33]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[34]->m_nPosX = BASE_ScreenResize(184.0f);
		m_pSlotShop[35]->m_nPosX = BASE_ScreenResize(12.0f);
		m_pSlotShop[36]->m_nPosX = BASE_ScreenResize(55.0f);
		m_pSlotShop[37]->m_nPosX = BASE_ScreenResize(98.0f);
		m_pSlotShop[38]->m_nPosX = BASE_ScreenResize(140.0f);
		m_pSlotShop[39]->m_nPosX = BASE_ScreenResize(184.0f);

		/////////////////////////////////inventario ////////////////////
		m_pInvenPanel->m_nWidth = BASE_ScreenResize(230.0f);
		pPanel1->m_nWidth = BASE_ScreenResize(230.0f);
		m_pInvPageBtn1->m_nPosX = BASE_ScreenResize(18.5f);//alterado
		m_pInvPageBtn2->m_nPosX = BASE_ScreenResize(61.0f);//alterado
		m_pInvPageBtn3->m_nPosX = BASE_ScreenResize(102.0f);//alterado
		m_pInvPageBtn4->m_nPosX = BASE_ScreenResize(145.0f);//alterado
		m_pGridInvList[0]->m_nWidth = BASE_ScreenResize(209.5f);//alterado
		m_pGridInvList[1]->m_nWidth = BASE_ScreenResize(209.5f);//alterado
		m_pGridInvList[2]->m_nWidth = BASE_ScreenResize(209.5f);//alterado
		m_pGridInvList[3]->m_nWidth = BASE_ScreenResize(209.5f);//alterado
		m_pGridInvList[0]->m_nPosX = BASE_ScreenResize(15.0f);//alterado
		m_pGridInvList[1]->m_nPosX = BASE_ScreenResize(15.0f);//alterado
		m_pGridInvList[2]->m_nPosX = BASE_ScreenResize(15.0f);//alterado
		m_pGridInvList[3]->m_nPosX = BASE_ScreenResize(15.0f);//alterado
		m_pQuicInvDelete->m_nPosX = BASE_ScreenResize(185.0f);//alterado
		m_pGridHelm->m_nPosX = BASE_ScreenResize(100.0f);// alterado
		m_pGridCoat->m_nPosX = BASE_ScreenResize(100.0f);// alterado
		m_pGridPants->m_nPosX = BASE_ScreenResize(100.0f);// alterado
		m_pGridBoots->m_nPosX = BASE_ScreenResize(142.0f);// alterado
		m_pGridRight->m_nPosX = BASE_ScreenResize(142.0f);// alterado
		m_pGridNewSlot1->m_nPosX = BASE_ScreenResize(142.0f);// alterado
		m_pGridNewSlot2->m_nPosX = BASE_ScreenResize(57.0f);// alterado
		m_pGridLeft->m_nPosX = BASE_ScreenResize(57.0f);// alterado //
		m_pGridGloves->m_nPosX = BASE_ScreenResize(57.0f);// alterado
		m_pGridGuild->m_nPosX = BASE_ScreenResize(14.5f);// alterado
		m_pGridDRing->m_nPosX = BASE_ScreenResize(14.5f);// alterado
		m_pGridRing->m_nPosX = BASE_ScreenResize(14.5f);// alterado
		m_pGridOrb->m_nPosX = BASE_ScreenResize(14.5f);// alterado
		m_pGridEvent->m_nPosX = BASE_ScreenResize(185.0f);// alterado
		m_pGridNecklace->m_nPosX = BASE_ScreenResize(185.0f);// alterado
		m_pGridCabuncle->m_nPosX = BASE_ScreenResize(185.0f);// alterado
		m_pGridMantua->m_nPosX = BASE_ScreenResize(185.0f);// alterado
		m_pPanelname->m_nPosX = BASE_ScreenResize(80.0f);// alterado
		m_pMoney1->m_nPosX = BASE_ScreenResize(145.0f);// alterado
		m_pBTNCLOSEINVENT->m_nPosX = BASE_ScreenResize(200.0f);// alterado
		m_pBTNgoldINVENT->m_nPosX = BASE_ScreenResize(85.0f);// alterado

		for (int i = 0; i < 32; ++i)
			m_pSlotInventori[i]->m_nWidth = BASE_ScreenResize(35.0f);

		m_pSlotInventori[0]->m_nPosX = BASE_ScreenResize(13.0f);// alterado
		m_pSlotInventori[1]->m_nPosX = BASE_ScreenResize(184.0f);// alterado
		m_pSlotInventori[2]->m_nPosX = BASE_ScreenResize(13.0f);// alterado
		m_pSlotInventori[3]->m_nPosX = BASE_ScreenResize(184.0f);// alterado
		m_pSlotInventori[4]->m_nPosX = BASE_ScreenResize(13.0f);// alterado
		m_pSlotInventori[5]->m_nPosX = BASE_ScreenResize(184.0f);// alterado
		m_pSlotInventori[6]->m_nPosX = BASE_ScreenResize(13.0f);// alterado
		m_pSlotInventori[7]->m_nPosX = BASE_ScreenResize(184.0f);// alterado
		m_pSlotInventori[8]->m_nPosX = BASE_ScreenResize(99.0f);// alterado
		m_pSlotInventori[9]->m_nPosX = BASE_ScreenResize(141.0f);// alterado
		m_pSlotInventori[10]->m_nPosX = BASE_ScreenResize(56.0f);// alterado
		m_pSlotInventori[11]->m_nPosX = BASE_ScreenResize(99.0f);// alterado
		m_pSlotInventori[12]->m_nPosX = BASE_ScreenResize(99.0f);// alterado
		m_pSlotInventori[13]->m_nPosX = BASE_ScreenResize(56.0f);// alterado
		m_pSlotInventori[14]->m_nPosX = BASE_ScreenResize(141.0f);// alterado
		m_pSlotInventori[15]->m_nPosX = BASE_ScreenResize(13.0f);// alterado
		m_pSlotInventori[16]->m_nPosX = BASE_ScreenResize(56.0f);// alterado
		m_pSlotInventori[17]->m_nPosX = BASE_ScreenResize(99.0f);// alterado
		m_pSlotInventori[18]->m_nPosX = BASE_ScreenResize(141.0f);// alterado
		m_pSlotInventori[19]->m_nPosX = BASE_ScreenResize(184.0f);// alterado
		m_pSlotInventori[20]->m_nPosX = BASE_ScreenResize(13.0f);// alterado
		m_pSlotInventori[21]->m_nPosX = BASE_ScreenResize(56.0f);// alterado
		m_pSlotInventori[22]->m_nPosX = BASE_ScreenResize(99.0f);// alterado
		m_pSlotInventori[23]->m_nPosX = BASE_ScreenResize(141.0f);// alterado
		m_pSlotInventori[24]->m_nPosX = BASE_ScreenResize(184.0f);// alterado
		m_pSlotInventori[25]->m_nPosX = BASE_ScreenResize(13.0f);// alterado
		m_pSlotInventori[26]->m_nPosX = BASE_ScreenResize(56.0f);// alterado
		m_pSlotInventori[27]->m_nPosX = BASE_ScreenResize(99.0f);// alterado
		m_pSlotInventori[28]->m_nPosX = BASE_ScreenResize(141.0f);// alterado
		m_pSlotInventori[29]->m_nPosX = BASE_ScreenResize(184.0f);// alterado
		m_pSlotInventori[30]->m_nPosX = BASE_ScreenResize(141.0f);// alterado
		m_pSlotInventori[31]->m_nPosX = BASE_ScreenResize(56.0f);// alterado
	}

	m_pGridShop->m_bDrawGrid = 1;
	m_pGridShop->m_eGridType = TMEGRIDTYPE::GRID_SHOP;
	m_pGridMantua->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
	m_pGridInvList[0]->m_bDrawGrid = 1;
	m_pGridInvList[1]->m_bDrawGrid = 1;
	m_pGridInvList[2]->m_bDrawGrid = 1;
	m_pGridInvList[3]->m_bDrawGrid = 1;
	m_pGridSkillMaster->m_bDrawGrid = 0;
	m_pGridSkillMaster->m_eGridType = TMEGRIDTYPE::GRID_SKILLM;
	m_pCargoGrid->m_bDrawGrid = 1;
	m_pCargoGrid->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
	m_pGridSkillBelt = (SGridControl*)m_pControlContainer->FindControl(65598);
	m_pGridSkillBelt2 = (SGridControl*)m_pControlContainer->FindControl(65644);
	m_pGridSkillBelt2->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;
	m_pGridSkillBelt2->m_nPosX = BASE_ScreenResize(371.0f);//alterado esq//alterado 2.0
	m_pGridSkillBelt2->m_nWidth = BASE_ScreenResize(198.0f);//alterado
	m_pGridSkillBelt3 = (SGridControl*)m_pControlContainer->FindControl(65645);
	m_pGridSkillBelt3->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;
	m_pGridSkillBelt3->m_nPosX = BASE_ScreenResize(371.0f);//alterado esq//alterado 2.0
	m_pGridSkillBelt3->m_nWidth = BASE_ScreenResize(198.0f);//alterado
	m_pGridSkillBelt3->SetVisible(0);
	m_pShortSkillTglBtn1 = (SButton*)m_pControlContainer->FindControl(65646);
	m_pShortSkillTglBtn2 = (SButton*)m_pControlContainer->FindControl(65647);
	m_pQuick_Sloat[0] = (SGridControl*)m_pControlContainer->FindControl(66560);
	m_pQuick_Sloat[0]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT1;
	m_pQuick_Sloat[0]->m_nPosX = BASE_ScreenResize(231.0f);// alterado
	m_pQuick_Sloat[1] = (SGridControl*)m_pControlContainer->FindControl(66561);
	m_pQuick_Sloat[1]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT2;
	m_pQuick_Sloat[1]->m_nPosX = BASE_ScreenResize(257.5f);// alterado
	m_pQuick_Sloat[2] = (SGridControl*)m_pControlContainer->FindControl(66562);
	m_pQuick_Sloat[2]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT3;
	m_pQuick_Sloat[2]->m_nPosX = BASE_ScreenResize(283.5f);// alterado
	m_pQuick_Sloat[3] = (SGridControl*)m_pControlContainer->FindControl(66563);
	m_pQuick_Sloat[3]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT4;
	m_pQuick_Sloat[3]->m_nPosX = BASE_ScreenResize(308.0f);// alterado
	m_pQuick_Sloat[4] = (SGridControl*)m_pControlContainer->FindControl(66564);
	m_pQuick_Sloat[4]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT5;
	m_pQuick_Sloat[4]->m_nPosX = BASE_ScreenResize(334.5f);// alterado
	m_pShortSkillTglBtn1->SetSelected(1);
	OnControlEvent(65646, 0);
	m_bSkillBeltSwitch = 0;

	m_pQuick_SloatQ[1] = (SButton*)m_pControlContainer->FindControl(66559);//new
	m_pQuick_SloatQ[2] = (SButton*)m_pControlContainer->FindControl(66558);//new
	m_pQuick_SloatQ[3] = (SButton*)m_pControlContainer->FindControl(66527);//new
	m_pQuick_SloatQ[4] = (SButton*)m_pControlContainer->FindControl(66556);//new
	m_pQuick_SloatQ[5] = (SButton*)m_pControlContainer->FindControl(66555);//new
	m_pQuick_SloatQ[6] = (SButton*)m_pControlContainer->FindControl(66554);//new
	m_pQuick_SloatQ[7] = (SButton*)m_pControlContainer->FindControl(66553);//new
	m_pQuick_SloatQ[8] = (SButton*)m_pControlContainer->FindControl(66552);//new
	m_pQuick_SloatQ[9] = (SButton*)m_pControlContainer->FindControl(66551);//new
	m_pQuick_SloatQ[10] = (SButton*)m_pControlContainer->FindControl(66550);//new
	m_pQuick_SloatQ[11] = (SButton*)m_pControlContainer->FindControl(66549);//new
	m_pQuick_SloatQ[12] = (SButton*)m_pControlContainer->FindControl(66548);//new
	m_pQuick_SloatQ[13] = (SButton*)m_pControlContainer->FindControl(66547);//new
	m_pQuick_SloatQ[14] = (SButton*)m_pControlContainer->FindControl(66546);//new
	m_pQuick_SloatQ[15] = (SButton*)m_pControlContainer->FindControl(66545);//new

	m_pQuick_SloatQ[1]->m_nPosX = BASE_ScreenResize(231.0f);//new
	m_pQuick_SloatQ[2]->m_nPosX = BASE_ScreenResize(257.5f);//new
	m_pQuick_SloatQ[3]->m_nPosX = BASE_ScreenResize(283.5f);//new
	m_pQuick_SloatQ[4]->m_nPosX = BASE_ScreenResize(308.5f);//new
	m_pQuick_SloatQ[5]->m_nPosX = BASE_ScreenResize(334.5f);//new
	m_pQuick_SloatQ[6]->m_nPosX = BASE_ScreenResize(370.5f);//newSkill
	m_pQuick_SloatQ[7]->m_nPosX = BASE_ScreenResize(390.5f);//new
	m_pQuick_SloatQ[8]->m_nPosX = BASE_ScreenResize(410.5f);//new
	m_pQuick_SloatQ[9]->m_nPosX = BASE_ScreenResize(429.5f);//new
	m_pQuick_SloatQ[10]->m_nPosX = BASE_ScreenResize(449.5f);//new
	m_pQuick_SloatQ[11]->m_nPosX = BASE_ScreenResize(469.5f);//new
	m_pQuick_SloatQ[12]->m_nPosX = BASE_ScreenResize(489.5f);//new
	m_pQuick_SloatQ[13]->m_nPosX = BASE_ScreenResize(508.5f);//new
	m_pQuick_SloatQ[14]->m_nPosX = BASE_ScreenResize(529.5f);//new
	m_pQuick_SloatQ[15]->m_nPosX = BASE_ScreenResize(548.5f);//new

	int nClass = BASE_GetItemAbility(&pMobData->Equip[0], 18);

	if (pMobData->Equip[1].sIndex > 40 && nClass != 21)
	{
		STRUCT_ITEM* pItemHelm = new STRUCT_ITEM;
		memcpy(pItemHelm, &pMobData->Equip[1], 8);

	WYD748_AddOwnedGridItem(	m_pGridHelm, pItemHelm);
	}
	if (pMobData->Equip[2].sIndex > 40)
	{
		STRUCT_ITEM* pItemCoat = new STRUCT_ITEM;
		memcpy(pItemCoat, &pMobData->Equip[2], 8);

	WYD748_AddOwnedGridItem(	m_pGridCoat, pItemCoat);
	}
	if (pMobData->Equip[3].sIndex > 40)
	{
		STRUCT_ITEM* pItemPants = new STRUCT_ITEM;
		memcpy(pItemPants, &pMobData->Equip[3], 8);

	WYD748_AddOwnedGridItem(	m_pGridPants, pItemPants);
	}
	if (pMobData->Equip[4].sIndex > 40)
	{
		STRUCT_ITEM* pItemGloves = new STRUCT_ITEM;
		memcpy(pItemGloves, &pMobData->Equip[4], 8);

	WYD748_AddOwnedGridItem(	m_pGridGloves, pItemGloves);
	}
	if (pMobData->Equip[5].sIndex > 40)
	{
		STRUCT_ITEM* pItemBoots = new STRUCT_ITEM;
		memcpy(pItemBoots, &pMobData->Equip[5], 8);

	WYD748_AddOwnedGridItem(	m_pGridBoots, pItemBoots);
	}
	if (pMobData->Equip[7].sIndex > 40)
	{
		STRUCT_ITEM* pItemRight = new STRUCT_ITEM;
		memcpy(pItemRight, &pMobData->Equip[7], 8);

	WYD748_AddOwnedGridItem(	m_pGridRight, pItemRight);
	}
	if (pMobData->Equip[6].sIndex > 40)
	{
		STRUCT_ITEM* pItemLeft = new STRUCT_ITEM;
		memcpy(pItemLeft, &pMobData->Equip[6], 8);

	WYD748_AddOwnedGridItem(	m_pGridLeft, pItemLeft);
	}
	if (pMobData->Equip[12].sIndex > 40)
	{
		STRUCT_ITEM* pItemGuild = new STRUCT_ITEM;
		memcpy(pItemGuild, &pMobData->Equip[12], 8);

	WYD748_AddOwnedGridItem(	m_pGridGuild, pItemGuild);
	}
	if (pMobData->Equip[13].sIndex > 40)
	{
		STRUCT_ITEM* pItemEvent = new STRUCT_ITEM;
		memcpy(pItemEvent, &pMobData->Equip[13], 8);

	WYD748_AddOwnedGridItem(	m_pGridEvent, pItemEvent);
	}
	if (pMobData->Equip[14].sIndex > 40)
	{
		STRUCT_ITEM* pItemDRing = new STRUCT_ITEM;
		memcpy(pItemDRing, &pMobData->Equip[14], 8);

	WYD748_AddOwnedGridItem(	m_pGridDRing, pItemDRing);
	}
	if (pMobData->Equip[15].sIndex > 40)
	{
		STRUCT_ITEM* pItemMantua = new STRUCT_ITEM;
		memcpy(pItemMantua, &pMobData->Equip[15], 8);

	WYD748_AddOwnedGridItem(	m_pGridMantua, pItemMantua);
	}
	if (pMobData->Equip[16].sIndex > 40)
	{
		STRUCT_ITEM* pItemDRing = new STRUCT_ITEM;
		memcpy(pItemDRing, &pMobData->Equip[16], 8);

		m_pGridNewSlot1->AddItem(new SGridControlItem(nullptr, pItemDRing, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[17].sIndex > 40)
	{
		STRUCT_ITEM* pItemMantua = new STRUCT_ITEM;
		memcpy(pItemMantua, &pMobData->Equip[17], 8);
		m_pGridNewSlot2->AddItem(new SGridControlItem(nullptr, pItemMantua, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[8].sIndex > 40)
	{
		STRUCT_ITEM* pItemRing = new STRUCT_ITEM;
		memcpy(pItemRing, &pMobData->Equip[8], 8);

	WYD748_AddOwnedGridItem(	m_pGridRing, pItemRing);
	}
	if (pMobData->Equip[9].sIndex > 40)
	{
		STRUCT_ITEM* pItemNecklace = new STRUCT_ITEM;
		memcpy(pItemNecklace, &pMobData->Equip[9], 8);

	WYD748_AddOwnedGridItem(	m_pGridNecklace, pItemNecklace);
	}
	if (pMobData->Equip[10].sIndex > 40)
	{
		STRUCT_ITEM* pItemOrb = new STRUCT_ITEM;
		memcpy(pItemOrb, &pMobData->Equip[10], 8);

	WYD748_AddOwnedGridItem(	m_pGridOrb, pItemOrb);
	}
	if (pMobData->Equip[11].sIndex > 40)
	{
		STRUCT_ITEM* pItemCabuncle = new STRUCT_ITEM;
		memcpy(pItemCabuncle, &pMobData->Equip[11], 8);

	WYD748_AddOwnedGridItem(	m_pGridCabuncle, pItemCabuncle);
	}

	for (int nCarryIndex = 0; nCarryIndex < 64; ++nCarryIndex)
	{
		if (pMobData->Carry[nCarryIndex].sIndex > 40)
		{
			STRUCT_ITEM* pItemCarry = new STRUCT_ITEM;
			memcpy(pItemCarry, &pMobData->Carry[nCarryIndex], 8);

			int Page = nCarryIndex / 15;
			if (nCarryIndex / 15 > -1 && Page < 4)
			{
				auto pControlItem = new SGridControlItem(0, pItemCarry, 0.0f, 0.0f);
				if (pControlItem)
				{
					if (!m_pGridInvList[Page]->AddItem(pControlItem,
						nCarryIndex % 15 % 5, nCarryIndex % 15 / 5))
						SAFE_DELETE(pControlItem);
				}
				else
					SAFE_DELETE(pItemCarry);
			}
			else
				delete pItemCarry;
		}
	}

	STRUCT_ITEM* pCargo = g_pObjectManager->m_stItemCargo;
	for (int nCargoIndex = 0; nCargoIndex < 120; ++nCargoIndex)
	{
		if (g_pObjectManager->m_stItemCargo[nCargoIndex].sIndex)
		{
			STRUCT_ITEM* pItemCargo = new STRUCT_ITEM;
			memcpy(pItemCargo, &pCargo[nCargoIndex], 8);

			int Page = nCargoIndex / 40;
			if (nCargoIndex / 40 > -1 && Page < 3)
			{
				auto pControlItem = new SGridControlItem(0, pItemCargo, 0.0f, 0.0f);
				if (pControlItem)
				{
					if (!m_pCargoGridList[Page]->AddItem(pControlItem,
						nCargoIndex % 40 % 5, nCargoIndex % 40 / 5))
						SAFE_DELETE(pControlItem);
				}
				else
					SAFE_DELETE(pItemCargo);
			}
			else
				delete pItemCargo;
		}
	}

	m_pMyHuman->m_stScore = pMobData->CurrentScore;

	m_pMyHuman->SetCharHeight((float)m_pMyHuman->m_stScore.Con);
	m_pMyHuman->SetRace(pMobData->Equip[0].sIndex);

	if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 41)
	{
		m_pMyHuman->m_stLookInfo.RightMesh = m_pMyHuman->m_stLookInfo.LeftMesh;
		m_pMyHuman->m_stLookInfo.RightSkin = m_pMyHuman->m_stLookInfo.LeftSkin;
		m_pMyHuman->m_stSancInfo.Sanc6 = m_pMyHuman->m_stSancInfo.Sanc7;
		m_pMyHuman->m_stSancInfo.Legend6 = m_pMyHuman->m_stSancInfo.Legend7;
	}

	m_pMyHuman->InitObject();
	m_pMyHuman->CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
	m_pMyHuman->InitAngle(0.0f, 0.39269909f, 0.0f);
	m_pMyHuman->InitPosition((float)pMobData->HomeTownX + 0.5f, 0, (float)pMobData->HomeTownY + 0.5f);

	g_pObjectManager->m_pCamera->SetFocusedObject(m_pMyHuman);
	g_pObjectManager->m_pCamera->m_nQuaterView = 0;
	m_bLastMyAttr = BASE_GetAttr(pMobData->HomeTownX, pMobData->HomeTownY);
	UpdateScoreUI(0);
	UpdateSkillBelt();
	g_pObjectManager->m_cSelectShortSkill = 0;

	for (int ie = 0; ie < 10; ++ie)
	{
		if ((unsigned char)g_pObjectManager->m_cShortSkill[ie] < 248)
			OnKeyShortSkill(ie + 49, 0);
	}

	if (!m_pObjectContainerList[0]->Load(szDataPath))
	{
		LOG_WRITELOG("DataFile Not Found\r\n");
		if (!m_bCriticalError)
			LogMsgCriticalError(3, 0, 0, 0, 0);

		m_bCriticalError = 1;
		SAFE_DELETE(m_pObjectContainerList[0]);
		SAFE_DELETE(m_pGroundList[0]);
		m_pGround = nullptr;
		return 0;
	}

	for (int nY = 0; nY < 128; ++nY)
		memcpy(m_HeightMapData[nY], m_pGround->m_pMaskData[nY], 128);

	g_HeightPosX = (int)m_pGround->m_vecOffset.x;
	g_HeightPosY = (int)m_pGround->m_vecOffset.y;
	BASE_ApplyAttribute((char*)m_HeightMapData, 256);

	memcpy(m_GateMapData, m_HeightMapData, sizeof(m_HeightMapData));

	m_pItemContainer = new TreeNode(0);
	m_pGroundObjectContainer->AddChild(m_pGroundList[0]);
	m_pSun = new TMSun();

	if (m_pSun)
	{
		m_pSun->InitObject();
		m_pEffectContainer->AddChild(m_pSun);
	}

	m_pSky = new TMSky();
	m_pSky->m_bVisible = 0;

	AddChild(m_pSky);
	AddChild(m_pItemContainer);

	SetSanc();

	m_pMyHuman->m_cHide = (m_pMyHuman->m_dwID >= 0 && m_pMyHuman->m_dwID < 1000) == 1 && m_pMyHuman->m_stScore.Merchant & 1;
	m_pHumanContainer->AddChild(m_pMyHuman);

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(102);
		if (pSoundData && !pSoundData->IsSoundPlaying())
			pSoundData->Play();
	}

	m_pRain = new TMRain();
	m_pRain->m_bVisible = 0;
	m_pEffectContainer->AddChild(m_pRain);

	m_pSnow = new TMSnow(1.0f);
	m_pSnow->m_bVisible = 0;
	m_pEffectContainer->AddChild(m_pSnow);

	m_pSnow2 = new TMSnow(2.0f);
	m_pSnow2->m_bVisible = 0;
	m_pEffectContainer->AddChild(m_pSnow2);

	m_pTarget1 = new TMEffectMesh(316, 0xFF111188, 0.0f, 0);
	m_pTarget1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
	m_pEffectContainer->AddChild(m_pTarget1);

	m_pTarget2 = new TMEffectMesh(317, 0xFFFF0000, 0.0f, 0);
	m_pEffectContainer->AddChild(m_pTarget2);

	m_pTargetBill = new TMEffectBillBoard2(232, 0, 1.0f, 1.0f, 1.0f, 0.0005f, 0);
	m_pTargetBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
	m_pTargetBill->SetColor(0xFF9999AA);
	m_pEffectContainer->AddChild(m_pTargetBill);

	g_pDevice->m_colorLight.r = 1.0f;
	g_pDevice->m_colorLight.g = 1.0f;
	g_pDevice->m_colorLight.b = 1.0f;
	g_pDevice->m_colorBackLight.r = 0.0;
	g_pDevice->m_colorBackLight.g = 0.2f;
	g_pDevice->m_colorBackLight.b = 0.3f;

	m_pMiniMapPanel = (SPanel*)m_pControlContainer->FindControl(289);
	m_pMiniMapZoomIn = (SButton*)m_pControlContainer->FindControl(5714);
	m_pMiniMapZoomOut = (SButton*)m_pControlContainer->FindControl(5715);

	if (m_pMainInfo2_Name)
		m_pMainInfo2_Name->SetText(m_pMyHuman->m_szName, 1);

	m_pMiniMapServerPanel = (SPanel*)m_pControlContainer->FindControl(6136);
	m_pMiniMapServerText = (SText*)m_pControlContainer->FindControl(6137);
	m_pMiniMapDir = (SPanel*)m_pControlContainer->FindControl(291);

	SButton* pMiniMapBtn = (SButton*)m_pControlContainer->FindControl(296);
	if (m_pMiniMapPanel)
	{
		m_pMiniMapPanel->GetGeomControl()->fAngle = m_bCompatFieldScene ? 0.0f : -0.78539819f;
		m_pMiniMapPanel->m_bSelectEnable = 0;

		if (m_pMiniMapDir)
			m_pMiniMapDir->m_bSelectEnable = 0;

		m_pMiniMapPanel->SetVisible(0);

		if (pMiniMapBtn)
			pMiniMapBtn->SetSelected(0);
		m_pMiniMapPanel->m_GCPanel.dwColor = 0x80FFFFFF;
	}

	for (int ig = 0; ig < 256; ++ig)
	{
		m_pInMiniMapPosPanel[ig] = new SPanel(-2, 0.0f, 0.0f, 4.0f, 4.0f, g_MinimapPos[ig].dwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

		if (m_pInMiniMapPosPanel[ig])
		{
			m_pInMiniMapPosPanel[ig]->m_bSelectEnable = 0;
			m_pInMiniMapPosPanel[ig]->SetVisible(0);
			m_pMiniMapPanel->AddChild(m_pInMiniMapPosPanel[ig]);
		}

		m_pInMiniMapPosText[ig] = new SText(-2,
			g_MinimapPos[ig].szTarget,
			g_MinimapPos[ig].dwColor,
			0.0f,
			0.0f,
			8.0f,
			12.0f,
			0,
			0x77777777u,
			1u,
			0);

		if (m_pInMiniMapPosText[ig])
		{
			m_pInMiniMapPosText[ig]->m_bSelectEnable = 0;
			m_pInMiniMapPosText[ig]->SetVisible(0);
			m_pMiniMapPanel->AddChild(m_pInMiniMapPosText[ig]);
		}
	}

	SPanel* pPartyPanel = (SPanel*)m_pControlContainer->FindControl(475136);
	SText* pPartyTitle = (SText*)m_pControlContainer->FindControl(475137);
	m_pPartyList = (SListBox*)m_pControlContainer->FindControl(475138);
	m_pPartyPanel = (SPanel*)pPartyPanel;
	m_pPartyBtn = (SButton*)m_pControlContainer->FindControl(5742);

	if (pPartyPanel)
	{
		pPartyPanel->SetStickLeft();
		pPartyPanel->SetStickTop();
		pPartyPanel->m_nPosY = pPartyPanel->m_nPosY + 135.0f;
	}

	if (m_pPartyBtn && pPartyPanel)
		m_pPartyBtn->SetPos(0.0f, ((float)g_pDevice->m_dwScreenHeight - pPartyPanel->m_nHeight) - 165.0f);

	if (m_pPartyBtn && m_pPartyBtn->m_pAltText && pPartyPanel)
		m_pPartyBtn->m_pAltText->SetPos(m_pPartyBtn->m_pAltText->m_nPosX + 5.0f, (pPartyPanel->m_nHeight * 0.5f) - 13.0f);

	OnControlEvent(5742, 0);

	m_pMsgPanel = (SPanel*)m_pControlContainer->FindControl(669);
	m_pMsgList = (SListBox*)m_pControlContainer->FindControl(670);
	m_pMsgText = (SText*)m_pControlContainer->FindControl(638);
	m_pGMsgPanel = (SPanel*)m_pControlContainer->FindControl(819);
	m_pGMsgText = (SText*)m_pControlContainer->FindControl(820);
	m_pGMsgListPanel = (SPanel*)m_pControlContainer->FindControl(823);
	m_pGMsgViewPanel = (SPanel*)m_pControlContainer->FindControl(828);
	m_pGMsgWritePanel = (SPanel*)m_pControlContainer->FindControl(838);
	m_pGMsgList = (SListBox*)m_pControlContainer->FindControl(825);
	m_pGMsgRContext = (SListBox*)m_pControlContainer->FindControl(829);
	m_pGMsgReplyList = (SListBox*)m_pControlContainer->FindControl(830);
	m_pGMsgWContext = (SListBox*)m_pControlContainer->FindControl(840);
	m_pGMsgEditRelpy = (SEditableText*)m_pControlContainer->FindControl(837);
	m_pGMsgEditTitle = (SEditableText*)m_pControlContainer->FindControl(839);
	m_pGMsgTitile = (SText*)m_pControlContainer->FindControl(836);
	m_pBtnBoardModify = (SButton*)m_pControlContainer->FindControl(832);
	m_pBtnBoardDel = (SButton*)m_pControlContainer->FindControl(834);
	m_pBtnBoardSaveGM = (SButton*)m_pControlContainer->FindControl(842);
	m_pBtnSaveReply = (SButton*)m_pControlContainer->FindControl(831);
	m_pBtnWrite = (SButton*)m_pControlContainer->FindControl(827);

	m_pRPSGamePanel = (SPanel*)m_pControlContainer->FindControl(1616);
	m_pRPSGamePanel->SetVisible(0);
	m_pRPSGamePanel->m_bPickable = 0;
	m_pRPSGameRock = (SButton*)m_pControlContainer->FindControl(1617);
	m_pRPSGamePaper = (SButton*)m_pControlContainer->FindControl(1618);
	m_pRPSGameScissor = (SButton*)m_pControlContainer->FindControl(1619u);

	m_pDropPanel[49] = (SPanel*)m_pControlContainer->FindControl(478471);// drop panel
	m_pDropPanel[49]->SetVisible(0);

	auto PainelC = (SPanel*)m_pControlContainer->FindControl(67658);

	PainelC->SetVisible(0);

	for (int i = 0; i < 4; i++)
		_HudControl.Object_World.inventarioGrid[i] = (int*)m_pControlContainer->FindControl(67072 + i);

	auto panelWarTower = (SPanel*)m_pControlContainer->FindControl(90624);
	panelWarTower->SetVisible(0);

	InitBoard();

		m_pMsgPanel->SetVisible(0);

	SetWeather(g_nWeather);

	if (m_pHelpList[3])
	{
		int n;
		for (n = 0; n < 99 && (g_pObjectManager->m_stMemo[n].szString[0] || g_pObjectManager->m_stMemo[n + 1].szString[0]); ++n)
		{
			SListBoxItem* pItem = new SListBoxItem(g_pObjectManager->m_stMemo[n].szString,
				g_pObjectManager->m_stMemo[n].dwColor,
				0.0f,
				0.0f,
				300.0f,
				16.0f,
				0,
				0x77777777u,
				1u,
				0);

			m_pHelpList[3]->AddItem(pItem);
		}

		if (n > 0 && m_pHelpMemo)
			m_pHelpMemo->SetVisible(1);
	}

	if (!strcmp(g_TempName, g_pObjectManager->m_stMobData.MobName))
	{
		if (strcmp(g_TempNick, " "))
		{
			MSG_MessageWhisper stWhisper{};
			stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
			stWhisper.Header.Type = MSG_MessageWhisper_Opcode;

			sprintf(stWhisper.MobName, "tab");
			sprintf(stWhisper.String, g_TempNick);
			SendOneMessage((char*)&stWhisper, sizeof(stWhisper));
		}
	}

	m_pServerPanel = (SPanel*)m_pControlContainer->FindControl(12288);
	m_pServerList = (SListBox*)m_pControlContainer->FindControl(12289);

	m_pServerPanel->SetPos((float)(g_pDevice->m_dwScreenWidth >> 1) - (m_pServerPanel->m_nWidth / 2.0f),
		(float)(g_pDevice->m_dwScreenHeight >> 1) - (m_pServerPanel->m_nHeight / 2.0f));
	m_pServerPanel->SetVisible(0);

	if (g_nKeyType == 1)
		m_pControlContainer->SetFocusedControl(m_pEditChat);

	InitCameraView();
	SetCameraView();

	m_pAlphaNative->SetPos((float)(m_pEditChat->m_nWidth + m_pChatPanel->m_nWidth) - 40.0f,
		(float)(m_pChatPanel->m_nPosY + m_pEditChat->m_nPosY) - 2.0f);

	InitializeFireWorkControls();

	m_pTotoPanel = (SPanel*)m_pControlContainer->FindControl(8961);

	if (m_pTotoPanel)
	{
		m_pTotoSelect_Btn = (SButton*)m_pControlContainer->FindControl(8964);
		m_pTotoBuy_Btn = (SButton*)m_pControlContainer->FindControl(8978);
		m_pTotoQuit_Btn = (SButton*)m_pControlContainer->FindControl(8966);
		m_pTotoNumber_Edit = (SEditableText*)m_pControlContainer->FindControl(8963);
		m_pTotoNumber_Edit->m_nMaxStringLen = 3;
		m_pTotoTime_Txt = (SText*)m_pControlContainer->FindControl(8968);
		m_pTotoTeamA_Txt = (SText*)m_pControlContainer->FindControl(8971);
		m_pTotoTeamB_Txt = (SText*)m_pControlContainer->FindControl(8972);
		m_pTotoScoreA_Edit = (SEditableText*)m_pControlContainer->FindControl(8973);
		m_pTotoScoreA_Edit->m_nMaxStringLen = 3;
		m_pTotoScoreB_Edit = (SEditableText*)m_pControlContainer->FindControl(8974);
		m_pTotoScoreB_Edit->m_nMaxStringLen = 3;
		m_pTotoPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[6] = (SControl*)m_pTotoPanel;
		m_pTotoPanel->SetVisible(0);

		m_pTotoPanel->SetPos((float)(g_pDevice->m_dwScreenWidth) - (m_pTotoPanel->m_nWidth / 2.0f),
			(float)(g_pDevice->m_dwScreenHeight) - (m_pTotoPanel->m_nHeight / 2.0f));
	}

	BASE_ReadTOTOList((char*)"UI\\TOTOGame.csv");

	SetAutoTarget();

	memset(&m_stMoveStop, 0, sizeof(m_stMoveStop));

	m_dwLastCheckAutoMouse = g_pTimerManager->GetServerTime();
	m_dwLastCheckPlayTime = g_pTimerManager->GetServerTime();
	m_nPlayTime = 0;

	g_pTextureManager->Release_GuildMarkList();
	g_pObjectManager->m_pCamera->m_fMaxCamLen = 20.0f; //new camera
	if (m_pPartyPanel)
	{
		m_pPartyPanel->SetVisible(0);
		if (m_pPartyBtn)
			m_pPartyBtn->SetSelected(1);
	}

	if (!m_bCompatFieldScene)
	{
		for (int ih = 0; ih < 32; ++ih)
		{
			m_pAffectIcon[ih] = (SPanel*)m_pControlContainer->FindControl(ih + 90400);
			m_pTargetAffectIcon[ih] = (SPanel*)m_pControlContainer->FindControl(ih + 90369);
		}

		m_pAffectDesc = (SText*)m_pControlContainer->FindControl(65798);
	}
	else
	{
		// FUN_00435b13 creates the sixteen 7.48 affect panels dynamically. The
		// imported 7.59 lookup above targets absent 90400+/65798 controls and used
		// to overwrite every valid icon/description pointer with null immediately
		// after construction, leaving Affect_Main with nothing it could display.
		for (int ih = 16; ih < 32; ++ih)
			m_pAffectIcon[ih] = nullptr;
	}
	m_pAffectDescList[0] = (SText*)m_pControlContainer->FindControl(773);
	m_pAffectDescList[1] = (SText*)m_pControlContainer->FindControl(774);
	m_pAffectDescList[2] = (SText*)m_pControlContainer->FindControl(775);
	m_pAffectDescList[3] = (SText*)m_pControlContainer->FindControl(776);
	m_pAffectDescList[4] = (SText*)m_pControlContainer->FindControl(777);
	m_pAffectDescList[5] = (SText*)m_pControlContainer->FindControl(784);
	m_pAffectDescList[6] = (SText*)m_pControlContainer->FindControl(794);
	m_pAffectDescList[7] = (SText*)m_pControlContainer->FindControl(795);
	m_pAffectDescList[8] = (SText*)m_pControlContainer->FindControl(796);
	m_pAffectDescList[9] = (SText*)m_pControlContainer->FindControl(797);
	m_pAffectDescList[10] = (SText*)m_pControlContainer->FindControl(798);
	m_pAffectDescList[11] = (SText*)m_pControlContainer->FindControl(799);

	for (int j = 0; j < 13; ++j)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			if (!m_bCompatFieldScene)
				m_pPartyAffectIcon[j][ii] = (SPanel*)m_pControlContainer->FindControl(32 * j + ii + 475152);
		}
	}

	m_pPartyAffectText = (SText*)m_pControlContainer->FindControl(7602193);
	m_pPartyAutoButton = (SButton*)m_pControlContainer->FindControl(7602194);
	m_pPartyAutoText = (SText*)m_pControlContainer->FindControl(7602195);

	SListBox* temp1 = (SListBox*)m_pControlContainer->FindControl(1054276u);
	SListBox* temp2 = (SListBox*)m_pControlContainer->FindControl(1054277u);

	m_pDailyQuestButton = (SButton*)m_pControlContainer->FindControl(1054278);

	if (temp1)
		temp1->SetVisible(0);
	if (temp2)
		temp2->SetVisible(0);

	if (m_pDailyQuestButton)
		m_pDailyQuestButton->SetVisible(0);

	NewCCMode();

	LOG_WRITELOG(">> Init Field Scene::End\r\n");
	g_bEffectFirst = 1;
	return 1;
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
	const bool isChatEdit = idwControlID == E_CHAT ||
		(m_bCompatFieldScene && idwControlID == TME_CHAT);

	if (m_bCompatFieldScene)
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
			return 1;
		case B_CCMODE_DLG_HP:
			g_GameAuto_hpValue = cc_mode::NextThreshold(g_GameAuto_hpValue);
			NewCCMode();
			return 1;
		case B_CCMODE_DLG_MOUNT:
			g_GameAuto_mountValue = cc_mode::NextThreshold(g_GameAuto_mountValue);
			NewCCMode();
			return 1;
		case B_CCMODE_DLG_MODE:
			g_GameAuto = (g_GameAuto + 1) % 4;
			NewCCMode(true, true);
			return 1;
		case P_CCMODE_DLG_PONT:
			m_AutoPostionUse = (m_AutoPostionUse + 1) % 3;
			NewCCMode(true, true);
			return 1;
		case 318: // native physical C.C selector
			return ToggleNativeCCMode(1);
		case 319: // native magic C.C selector
			return ToggleNativeCCMode(2);
		case 1375: // ItemMix1 run (Compositor)
			DoCombine();
			return 1;
		case 1376: // ItemMix1 close
			SetVisibleMixItem(0);
			return 1;
		case 6111: // ItemMix2 run (Aylin)
			DoCombine2();
			return 1;
		case 6146: // ItemMix3 run (Agatha)
			DoCombine3();
			return 1;
		case 6434: // ItemMix4 run (Tiny)
			DoCombine4();
			return 1;
		case 6482: // ItemMix5 run (Lindy/Odin)
			DoCombine5();
			return 1;
		case 6514: // ItemMix6 run (Ehre)
			DoCombine6();
			return 1;
		case 5744: // native main-menu button toggles flyout panel 292
			if (auto panel = m_pControlContainer->FindControl(292))
			{
				const int visible = panel->IsVisible() == 0;
				panel->SetVisible(visible);
				if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(5744)))
					button->SetSelected(visible);
			}
			return 0;
		case 368: // native inventory/shop close button
			if (m_pShopPanel && m_pShopPanel->IsVisible())
				SetVisibleShop(0);
			else if (m_pInvenPanel)
				m_pInvenPanel->SetVisible(0);
			if (m_pDescPanel)
				m_pDescPanel->SetVisible(0);
			// Button 294 is the native 7.48 equipment toggle.  Synchronizing it
			// here prevents the menu from remaining visually pressed after X/ESC.
			if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_EQUIP)))
				button->SetSelected(0);
			return 1;
		case 533: // native character close button
			if (m_pCPanel)
				m_pCPanel->SetVisible(0);
			// The 7.48 Character X releases the same button selected by shortcut C.
			if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_CHAR)))
				button->SetSelected(0);
			return 1;
		case 633: // native System: select server
		case 634: // native System: return to character selection
		case 635: // native System: quit game
		{
			// FieldScene2 binds 633-635 directly to the three native countdowns.
			// Do not translate them to the newer System controls: that path owns
			// 7.59 grids which are deliberately absent from the 7.48 layout.
			if (idwControlID == 633)
				m_dwLastSelServer = dwServerTime;
			else if (idwControlID == 634)
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
			return 1;
		}
		case 636: // native System cancel button
			if (m_pSystemPanel)
				m_pSystemPanel->SetVisible(0);
			// Native cancel and ESC both release the bottom-bar System toggle.
			if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SYSTEM)))
				button->SetSelected(0);
			return 1;
		case TMB_ATRADE_CLOSE:
			// Ghidra FUN_004662c5 routes native close 668 through the same
			// state-restoring routine as main-menu auto-trade button 313.
			SetVisibleAutoTrade(0, 0);
			return 1;
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
			return 1;
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
			return 0;
		}
		case TMB_AUTORUN:
			// Native button 316 and the ']' shortcut share FUN_00452733; routing
			// both here prevents a visible button that cannot change autorun state.
			return OnKeyAutoRun(']', 0) ? 0 : 1;
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
			return 0;
		case 65790: // character
			SetVisibleCharInfo();
			return 0;
		case 65792: // skill
			SetVisibleSkill();
			return 0;
		case B_AUTOTRADEBTN:
			// FUN_004662c5 routes native button 313 directly to the title prompt
			// while AutoTrade is closed and to the native close path while visible.
			if (!m_pMyHuman || !m_pMyHuman->IsInTown())
				return 1;
			if (m_pAutoTrade && m_pAutoTrade->IsVisible())
				SetVisibleAutoTrade(0, 0);
			else
				VisibleInputTradeName();
			return 1;
		case 65799: // party
			SetVisibleParty();
			return 0;
		case 65793: // quest
			if (m_pQuestPanel)
				SetQuestPanelVisible(m_pQuestPanel->IsVisible() == 0);
			return 0;
		case 65795: // helper
			if (auto panel = m_pControlContainer->FindControl(864))
				panel->SetVisible(panel->IsVisible() == 0);
			return 0;
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
			return 0;
		default:
			break;
		}
	}

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
		return 1;
	}
	if (idwControlID >= TMB_GAMBLE_LEFTX2 && idwControlID <= TMB_GAMBLE_RIGHT)
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
				return 1;
			m_nBet += 10000;
			if (m_nBet > 100000)
				m_nBet = 100000;
			break;
		case TMB_GAMBLE_RIGHT:
			if (g_pObjectManager->m_stMobData.Coin - m_nBet < 1000)
				return 1;
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
		return 1;
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
		auto pEditChat = m_pEditChat;
		if (!strlen(pEditChat->GetText()))
		{
			if (!g_nKeyType)
			{
				m_pEditChatPanel->SetVisible(0);
				m_pChatPanel->SetVisible(1);
				m_pControlContainer->SetFocusedControl(0);
			}
			return 1;
		}

		const bool chatFlood = chat_submit::RecordSubmission(m_dwLastChatTime, dwServerTime);

		char istrText[128]{};

		if (chatFlood)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[33], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}

		chat_submit::Remember(m_szLastChatList, pEditChat->GetText());

		m_sChatIndex = 0;

		switch (chat_submit::ClassifyLocal(pEditChat->GetText(), g_pMessageStringTable[191]))
		{
		case chat_submit::LocalCommand::Clear:
		{
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}
		case chat_submit::LocalCommand::Help:
		{
			OnKeyHelp(104, 0);
			pEditChat->SetText((char*)"");
			return 1;
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
			return 1;
		}
		case chat_submit::LocalCommand::Fps:
		{
			g_pDevice->m_bDrawFPS = g_pDevice->m_bDrawFPS == 0;
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			UpdateScoreUI(0);
			return 1;
		}
		case chat_submit::LocalCommand::Effects2:
		{
			g_pDevice->m_bShowEffects = !g_pDevice->m_bShowEffects;
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}
		case chat_submit::LocalCommand::EffectOld:
		{
			g_bHideSkillBuffEffect = !g_bHideSkillBuffEffect;
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
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
			return 1;
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
				return 0;
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
			sscanf(Chat, "/%s", str1);

			if (chat_submit::IsRelocate(str1, g_pMessageStringTable[234]))
			{
				if (!m_pAutoTrade->IsVisible())
				{
					sscanf(Chat, "/%s %s", str1, m_szSummoner2);

					m_cLastRelo = 2;
					m_dwLastRelo = g_pTimerManager->GetServerTime();

					pEditChat->SetText((char*)"");

					m_pControlContainer->SetFocusedControl(0);
					m_pEditChatPanel->SetVisible(0);
					m_pChatPanel->SetVisible(0);
				}
				return 1;
			}

			idwFontColor = chat_submit::kCommandColor;

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
				sprintf(stMsgWhisper.String, "%s", &pChatText[strlen(str1) + 2]);
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
					return 1;

				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgWhisper)->Type, reinterpret_cast<char*>(&stMsgWhisper), sizeof(stMsgWhisper)});
			}
			break;
			case chat_submit::Command::Kingdom:
			{
				if (chat_submit::IsBlockedByProgress(m_pMyHuman->m_fProgressRate))
					return 1;

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
					return 1;
				}
				if (!CheckGuildName(stMsgWhisper.String, 0))
				{
					m_pMessagePanel->SetMessage(g_pMessageStringTable[370], 2000);
					m_pMessagePanel->SetVisible(1, 1);
					return 1;
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
					return 1;
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
				return 1;
			case chat_submit::Command::Nickname:
			{
				if (chat_submit::IsBlockedByProgress(m_pMyHuman->m_fProgressRate))
					return 1;

				if (!strcmp(m_pMyHuman->m_szNickName, stMsgWhisper.String))
					return 1;

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
		return 0;
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
	if (m_bCompatFieldScene)
	{
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
	}
	if (idwControlID == B_MONEY)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;

		auto pText = static_cast<SText*>(m_pControlContainer->FindControl(
			m_bCompatFieldScene ? TMT_INPUT_GOLD : T_INPUT_GOLD));
		auto pEdit = m_pControlContainer->FindControl(
			m_bCompatFieldScene ? TME_INPUT_GOLD : E_INPUT_GOLD);
		if (!pText || !pEdit || !m_pInputGoldPanel)
			return 1;

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
				return 1;

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
		return 0;
	}
	if (idwControlID == B_CARGO_MONEY)
	{
		if (!m_pMyHuman || !m_pControlContainer || !m_pInputGoldPanel ||
			m_pMyHuman->m_cDie == 1)
			return 1;

		if (!m_pAutoTrade || m_pAutoTrade->IsVisible() != 1)
		{
			auto pText = static_cast<SText*>(m_pControlContainer->FindControl(
				m_bCompatFieldScene ? TMT_INPUT_GOLD : T_INPUT_GOLD));
			auto pEdit = m_pControlContainer->FindControl(
				m_bCompatFieldScene ? TME_INPUT_GOLD : E_INPUT_GOLD);

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
			m_bCompatFieldScene ? TME_INPUT_GOLD : E_INPUT_GOLD));
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
				const int autoTradeSlotCount = m_bCompatFieldScene ? 12 : 10;
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

	if (idwControlID == 478484 || idwControlID == 478485)
	{
		auto button = (SButton*)m_pControlContainer->FindControl(idwControlID);
		auto button2 = (SButton*)m_pControlContainer->FindControl(idwControlID == 478484 ? 478485 : 478484);

		button->SetSelected(true);
		button2->SetSelected(false);

		UpdateGridDropList(idwControlID - 478484);

		return 1;
	}

	if (idwControlID == 3000060 || idwControlID == 3000061 || idwControlID == 3000062 || idwControlID == 3000063 || idwControlID == 3000064 ||
		idwControlID == 3000065 || idwControlID == 3000066 || idwControlID == 3000067 || idwControlID == 3000057 || idwControlID == 3000058 || idwControlID == 3000059 ||
		idwControlID == 3000056 || idwControlID == 3000015 || idwControlID == 3000019 || idwControlID == 3000023 || idwControlID == 3000027 ||
		idwControlID == 3000031 || idwControlID == 3000035 || idwControlID == 3000039 || idwControlID == 3000043 || idwControlID == 3000047 ||
		idwControlID == 3000051 || idwControlID == 3000055)
	{
		UpdateNewStore(idwControlID);
		return 1;
	}

	if (idwControlID == 3000081)
	{
		BuyItemNewStore(idwControlID);
		auto panelbuy = (SGridControl*)m_pControlContainer->FindControl(3000080);
		panelbuy->m_bVisible = false;
		return 1;
	}
	if (idwControlID == 3000082)
	{

		auto panelbuy = (SGridControl*)m_pControlContainer->FindControl(3000080);
		panelbuy->m_bVisible = false;
		return 1;
	}
	if (idwControlID == 67651)
	{
		auto panel = (SPanel*)m_pControlContainer->FindControl(67658);

		panel->m_bVisible = false;

		if (auto pnl = (SPanel*)m_pControlContainer->FindControl(65768))
			pnl->m_bVisible = true;

		if (auto pnl = (SPanel*)m_pControlContainer->FindControl(65771))
			pnl->m_bVisible = true;

		for (int i = 0; i < 66; i++)
		{
			if (auto pnl = (SPanel*)m_pControlContainer->FindControl(65701 + i))
				pnl->m_bVisible = true;
		}
	}

	if (idwControlID == 67654)
	{
		auto panel = (SPanel*)m_pControlContainer->FindControl(67658);

		if (panel)
			panel->m_bVisible = true;

		if (auto pnl = (SPanel*)m_pControlContainer->FindControl(65768))
			pnl->m_bVisible = false;

		if (auto pnl = (SPanel*)m_pControlContainer->FindControl(65771))
			pnl->m_bVisible = false;

		for (int i = 0; i < 66; i++)
		{
			if (auto pnl = (SPanel*)m_pControlContainer->FindControl(65701 + i))
				pnl->m_bVisible = false;
		}
	}
	if (idwControlID == 478472 )
	{

		auto button = (SButton*)m_pControlContainer->FindControl(478484);
		auto button2 = (SButton*)m_pControlContainer->FindControl(478485);

		button->SetSelected(true);
		button2->SetSelected(false);

		UpdateGridDropList(0);
		return 1;
	}

	if (idwControlID == 478486 )
	{
		auto pnl = (SPanel*)m_pControlContainer->FindControl(478471);
		if (pnl)
			pnl->SetVisible(false);

		return 1;
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
	if (idwControlID == B_CHATLIST_SIZEUP)
	{
		OnKeyPlus(43, 0);
		return 0;
	}

	if (idwControlID == B_CHATLIST_LIGHT)
	{
		if (!m_pChatBack)
			return 0;

		static short Color = 0;
		switch (Color)
		{
		case 0:
			m_pChatBack->m_GCPanel.dwColor = 0x66000000;
			Color = 1;
			break;
		case 1:
			m_pChatBack->m_GCPanel.dwColor = 0x88000000;
			Color = 2;
			break;
		case 2:
			m_pChatBack->m_GCPanel.dwColor = 0xAA000000;
			Color = 3;
			break;
		case 3:
			m_pChatBack->m_GCPanel.dwColor = 0xFF000000;
			Color = 4;
			break;
		case 4:
			m_pChatBack->m_GCPanel.dwColor = 0;
			Color = 0;
			break;
		}
		return 0;
	}
	if (idwControlID == B_CHAT_SELECT)
	{
		if (!m_pChatListPanel)
			return 0;

		if (m_pChatListPanel->IsVisible())
			m_pChatListPanel->SetVisible(0);
		else
			m_pChatListPanel->SetVisible(1);

		return 0;
	}
	const field_chat::Buttons<SButton> chatButtons{
		{m_pChatGeneral, m_pChatGeneral_C}, {m_pChatParty, m_pChatParty_C},
		{m_pChatWhisper, m_pChatWhisper_C}, {m_pChatGuild, m_pChatGuild_C}};
	const auto chatOutcome = field_chat::Handle(m_bCompatFieldScene, idwControlID,
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
	if (idwControlID >= B_CHAT_SELECT_NOMAL && idwControlID <= B_CHAT_SELECT_SHOUT)
	{
		auto pBtn = static_cast<SButton*>(m_pControlContainer->FindControl(idwControlID));

		switch (idwControlID)
		{
		case B_CHAT_SELECT_NOMAL:
			strcpy(m_cChatType, "");
			break;
		case B_CHAT_SELECT_WHISPER:
			sprintf(m_cChatType, "/%s ", m_cWhisperName);
			break;
		case B_CHAT_SELECT_PARTY:
			strcpy(m_cChatType, "=");
			break;
		case B_CHAT_SELECT_GUILD:
			strcpy(m_cChatType, "-");
			break;
		case B_CHAT_SELECT_GUILD2:
			strcpy(m_cChatType, "--");
			break;
		case B_CHAT_SELECT_CITY:
			strcpy(m_cChatType, "@@");
			break;
		case B_CHAT_SELECT_KINGDOM:
			strcpy(m_cChatType, "@");
			break;
		case B_CHAT_SELECT_SHOUT:
			char src[128]{};
			sprintf(src, "/%s ", g_pMessageStringTable[389]);
			strcpy(m_cChatType, src);
			break;
		}

		strcpy(m_cChatSelect, pBtn->m_GCPanel.strString);

		auto pChatSelectBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAT_SELECT));

		if (pChatSelectBtn)
			pChatSelectBtn->SetText(pBtn->m_GCPanel.strString);

		if (m_pChatListPanel)
			m_pChatListPanel->SetVisible(0);

		if (m_pEditChat)
			m_pEditChat->SetText(m_cChatType);

		return 0;
	}
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
	if (idwControlID == TMB_PARTY_AUTOOK)
	{
		m_bAutoParty = m_bAutoParty == 0;
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
	if (idwControlID == B_CHAT_WHISPER)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		strcpy_s(stMsgChat.String, "whisper");
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgChat)->Type, reinterpret_cast<char*>(&stMsgChat), sizeof(stMsgChat)});
		return 1;
	}
	if (idwControlID == B_CHAT_PARTY)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		strcpy_s(stMsgChat.String, "partychat");
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgChat)->Type, reinterpret_cast<char*>(&stMsgChat), sizeof(stMsgChat)});
		return 1;
	}
	if (idwControlID == B_CHAT_KINGDOM)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		strcpy_s(stMsgChat.String, "kingdomchat");
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgChat)->Type, reinterpret_cast<char*>(&stMsgChat), sizeof(stMsgChat)});
		return 1;
	}
	if (idwControlID == B_CHAT_GUILD)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		strcpy_s(stMsgChat.String, "guildchat");
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMsgChat)->Type, reinterpret_cast<char*>(&stMsgChat), sizeof(stMsgChat)});
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
	if (idwControlID == B_SYS_QUIT)
	{
		SetRunMode();
		if (g_dwStartQuitGameTime)
			return 1;

		g_dwStartQuitGameTime = g_pTimerManager->GetServerTime();

		MSG_SysQuit stParm{};
		stParm.Header.ID = m_pMyHuman->m_dwID;
		stParm.Header.Type = MSG_SysQuit_Opcode;
		g_pSocketManager->SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm)->Type, reinterpret_cast<char*>(&stParm), sizeof(stParm)});
		return 0;
	}
	if (idwControlID == P_MINIBTNPANEL_BTN)
	{
		if (m_pMiniPanel->m_bVisible)
			m_pMiniPanel->SetVisible(0);
		else
			m_pMiniPanel->SetVisible(1);
		if (m_pMiniBtn)
			m_pMiniBtn->m_bSelected = m_pMiniPanel->m_bVisible;
		return 0;
	}
	if (idwControlID == B_SYS_SERVER)
	{
		if (!m_pServerPanel)
		{
			m_dwLastSelServer = g_pTimerManager->GetServerTime();
			MSG_SysQuit stParm{};
			stParm.Header.ID = m_pMyHuman->m_dwID;
			stParm.Header.Type = MSG_SysQuit_Opcode;
			g_pSocketManager->SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm)->Type, reinterpret_cast<char*>(&stParm), sizeof(stParm)});
			return 1;
		}

		m_pSystemPanel->SetVisible(0);

		m_pMessagePanel->SetMessage(g_pMessageStringTable[23], 0);
		m_pMessagePanel->SetVisible(1, 0);

		const int serverGroup = LastConfiguredServerGroup(g_pServerList);
		if (serverGroup < 0)
			return 1;

		int nDay[10] = { 0 }; // v678;

		_SYSTEMTIME time{};
		GetLocalTime(&time);
		int wDay = time.wDay % 10;
		if (!wDay)
			wDay = 10;

		for (int i = 0; i < MAX_SERVERGROUP; ++i)
		{
			for (int k = 1; k < MAX_SERVERNUMBER; ++k)
				if (g_pServerList[i][k][0] != 0)
					++nDay[i];

			if (nDay[i])
				nDay[i] = !(time.wDay % nDay[i]) ? nDay[i] : time.wDay % nDay[i];
		}

		auto currentServerGroupIndex = g_pObjectManager->m_nServerGroupIndex; // v438
		if (currentServerGroupIndex < 0 || currentServerGroupIndex >= MAX_SERVERGROUP)
			return 1;

		char szUserCount[1024] = { 0 };
		int nUserCount[MAX_SERVERNUMBER] = { 0 };
		int nUserCount2[MAX_SERVERNUMBER] = { 0 };
		if (currentServerGroupIndex == serverGroup)
		{
			for (int i = serverGroup; i < MAX_SERVERGROUP; ++i)
				g_pServerList[i][0][0] = 0;

			for (int i = 0; i < serverGroup; ++i)
			{
				memset(nUserCount2, -1, sizeof nUserCount2);
				szUserCount[0] = 0;
				char szStatusEndpoint[64]{};
				if (CopyServerEndpoint(szStatusEndpoint, g_pServerList[i][0]))
					BASE_GetHttpRequest(szStatusEndpoint, szUserCount, sizeof szUserCount);

				ParseServerStatus(szUserCount, nUserCount2, 11);

				//
				nUserCount[nDay[serverGroup- i]] = nUserCount2[nDay[serverGroup - i]];
				auto& aggregateEndpoint = g_pServerList[currentServerGroupIndex][i + 1];
				CopyServerEndpointAt(aggregateEndpoint, g_pServerList,
					serverGroup - i - 1, nDay[serverGroup - i] + 1);
			}
		}
		else
		{
			szUserCount[0] = 0;
			char szStatusEndpoint[64]{};
			if (CopyServerEndpoint(szStatusEndpoint, g_pServerList[currentServerGroupIndex][0]))
				BASE_GetHttpRequest(szStatusEndpoint, szUserCount, sizeof szUserCount);
			ParseServerStatus(szUserCount, nUserCount, 10);
		}

		m_pMessagePanel->SetVisible(0, 1);

		SListBox* pServerList = m_pServerList;
		if (pServerList)
		{
			pServerList->Empty();
			pServerList->SetSelectedIndex(-1);
			for (int num = 1;; ++num)
			{
				if (num >= MAX_SERVERNUMBER)
				{
					pServerList->SetVisible(1);

					if (m_pServerPanel)
						m_pServerPanel->SetVisible(1);
					break;
				}

				if (g_pServerList[currentServerGroupIndex][num][0])
				{
					char iStrText[32] = { 0 }; // original = 14
					if (serverGroup == currentServerGroupIndex)
					{
						if (currentServerGroupIndex - num < 0)
							continue;

						const int group = currentServerGroupIndex - num;
						const char* selectedName = ServerChannelNameAt(g_szServerName, group, nDay[group]);
						const char* displayName = ServerChannelNameAt(g_szServerName, group, nDay[group] - 1);
						if (selectedName && displayName)
							sprintf_s(iStrText, "%s-%s", g_szServerNameList[group], displayName);
						else
						{
							sprintf_s(iStrText, "%s-%d", g_szServerNameList[currentServerGroupIndex - num], nDay[currentServerGroupIndex - num]);
							if (nUserCount[num] > 500)
								AppendFullChannelLabel(iStrText, sizeof(iStrText));
						}
					}
					else if (g_szServerNameList[currentServerGroupIndex][0])
					{
						if (const char* channelName = ServerChannelNameAt(g_szServerName, currentServerGroupIndex, num - 1))
							sprintf_s(iStrText, "%s-%s", g_szServerNameList[currentServerGroupIndex], channelName);
						else
						{
							sprintf_s(iStrText, "%s-%d", g_szServerNameList[currentServerGroupIndex], num);

							if (nUserCount[num] > 600)
								AppendFullChannelLabel(iStrText, sizeof(iStrText));
						}
					}
					else
						sprintf_s(iStrText, g_pMessageStringTable[68], num + 1, num);

					int nCount = nUserCount[num];
					if (nCount < 0)
						nCount = 0;

					int nTextureSet = -1;
					if (nDay[currentServerGroupIndex] == num)
						nTextureSet = -2;

					if (currentServerGroupIndex == serverGroup)
						nTextureSet = -2;

					// -1??
					auto server = new SListBoxServerItem(nTextureSet, iStrText, 0xFFFFFFFF, 0.0f, 0.0f, static_cast<float>(g_nChannelWidth), 16.0f, nCount, 0, 0, num);

					if (nUserCount[num] < 0)
						server->m_cConnected = 0;
					pServerList->AddItem(server);
				}
				else if (serverGroup == currentServerGroupIndex && num < serverGroup)
				{
					char iStrTexr[14] = { 0 };
					sprintf_s(iStrTexr, g_pMessageStringTable[70]);

					auto server = new SListBoxServerItem(6, iStrTexr, 0xFFFFFFFF, 0.0f, 0.0f, static_cast<float>(g_nChannelWidth), 16.0f, nUserCount2[num], 0, 0, 0);
					if (nUserCount[num] < 0)
						server->m_cConnected = 0;

					pServerList->AddItem(server);
				}
			}
		}
	}
	if (idwControlID == B_QUEST_BUTTON || idwControlID == TMB_QUEST_BUTTON)
	{
		SelectQuestTab(0);
		return 1;
	}
	if (idwControlID == B_QUEST_BUTTON2 || idwControlID == TMB_QUEST_BUTTON2)
	{
		SelectQuestTab(1);
		return 1;
	}
	if (idwControlID == B_QUEST_BUTTON3 || idwControlID == TMB_QUEST_BUTTON3)
	{
		SelectQuestTab(2);
		return 1;
	}
	if (idwControlID == B_QUEST_BUTTON4 || idwControlID == TMB_QUEST_BUTTON4)
	{
		SelectQuestTab(3);
		return 1;
	}
	if (idwControlID == B_QUEST_MEMO || idwControlID == TMB_QUEST_MEMO)
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
	if (idwControlID == B_QUEST_QUIT || idwControlID == TMB_QUEST_QUIT)
	{
		SetQuestPanelVisible(false);
		return 0;
	}
	if (idwControlID == B_CCATTACK)
	{
		if (++g_GameAuto >= 4)
			g_GameAuto = 0;

		NewCCMode(true);
		return 1;
	}
	if (idwControlID == B_CCPOTION)
	{
		if (++m_AutoHpMp >= 4)
			m_AutoHpMp = 0;

		NewCCMode();
		return 1;
	}
	if (idwControlID == B_CCMOVE)
	{
		if (++m_AutoPostionUse >= 3)
			m_AutoPostionUse = 0;

		NewCCMode(false, true);
		return 1;
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
	if (idwControlID == B_ITEM_MIX_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], B_ITEM_MIX_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == B_MISSION_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], B_MISSION_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == B_SHORTSKILL_TGL1)
	{
		m_pGridSkillBelt2->SetVisible(1);
		m_pGridSkillBelt3->SetVisible(0);

		m_pShortSkillTglBtn1->SetSelected(1);
		m_pShortSkillTglBtn2->SetSelected(0);

		GetSoundAndPlay(53, 0, 0);

		m_pControlContainer->SetFocusedControl(0);
		m_pShortSkill_Txt->SetText((char*)"1", 0);
		OnKeyShortSkill(49, 0);
		m_bSkillBeltSwitch = 0;

		return 0;
	}
	if (idwControlID == B_SHORTSKILL_TGL2)
	{
		m_pGridSkillBelt2->SetVisible(0);
		m_pGridSkillBelt3->SetVisible(1);

		m_pShortSkillTglBtn1->SetSelected(0);
		m_pShortSkillTglBtn2->SetSelected(1);

		GetSoundAndPlay(53, 0, 0);

		m_pControlContainer->SetFocusedControl(0);
		m_pShortSkill_Txt->SetText((char*)"2", 0);
		OnKeyShortSkill(49, 0);
		m_bSkillBeltSwitch = 1;

		return 0;
	}
	if (idwControlID == B_HELP)
	{
		m_pControlContainer->SetFocusedControl(0);
		OnKeyHelp(104, 0);

		return 0;
	}

	if (idwControlID == 656433) //Shop
	{
		auto pnl = (SPanel*)m_pControlContainer->FindControl(3000011);
		if (pnl)
			pnl->SetVisible(true);

		return 0;
	}
	if (idwControlID == 3000060) //close Shop
	{
		auto pnl = (SPanel*)m_pControlContainer->FindControl(3000011);
		if (pnl)
			pnl->SetVisible(false);

		return 0;
	}
	//if (idwControlID == B_COMMUNITY)
	//{

	//	// g_pApp->SwitchWebBoard();
	//	g_pApp->SwitchWebBrowserState(1);

	//	return 0;
	//}
	if (idwControlID == B_COMMUNITY) //Shop
	{
		m_pDailyRewardInfo->SetVisible(true);
		return 0;
	}
	if (idwControlID == 15733) //close Shop
	{
		m_pDailyRewardInfo->SetVisible(false);
		return 0;
	}

	if (idwControlID == B_QUESTLOG || idwControlID == TMB_QUESTLOG)
	{
		if (m_pQuestPanel)
			SetQuestPanelVisible(m_pQuestPanel->IsVisible() == 0);
		return 0;
	}
	if (idwControlID == L_QUEST_LIST || idwControlID == TML_QUEST_LIST)
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
	if (idwControlID == L_QUEST_LIST2 || idwControlID == TML_QUEST_LIST2)
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
	if (idwControlID == L_QUEST_LIST3 || idwControlID == TML_QUEST_LIST3)
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
	if (idwControlID == L_QUEST_LIST4 || idwControlID == TML_QUEST_LIST4)
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
	if (idwControlID == 65677)
	{
		m_pChatGeneral->m_bSelected = m_pChatGeneral->m_bSelected == 0;
		m_pChatGeneral_C->m_bSelected = m_pChatGeneral->m_bSelected == 0;
		m_pChatGeneral->Update();
		m_pChatGeneral_C->Update();

		char szText[128]{};
		if (m_pChatGeneral->m_bSelected)
			sprintf(szText, "%s%s", g_pMessageStringTable[452], g_pMessageStringTable[446]);
		else
			sprintf(szText, "%s%s", g_pMessageStringTable[452], g_pMessageStringTable[447]);

		auto ipNewItem = new SListBoxItem(szText, 0xFFCCAAFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		auto pChatList = (SListBox*)m_pControlContainer->FindControl(65667);;
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);

		return 0;
	}
	if (idwControlID == 65785)
	{
		SetPK();
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
	if (idwControlID >= 8811 && idwControlID <= 8815)
	{
		DrawCustomFireWork(idwControlID - 8811);
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
		if (pItem->m_nCurrent < 500)
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
	if (idwControlID == 65881)
	{
		auto GridDrop = (SGridControl*)m_pControlContainer->FindControl(478473);

		memset(GridDrop->m_pbFilled, 0, GridDrop->m_nColumnGridCount * sizeof(int)* GridDrop->m_nRowGridCount);
		for (int i = 0; i < GridDrop->m_nNumItem; ++i)
		{
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == GridDrop->m_pItemList[i])
				g_pCursor->m_pAttachedItem = 0;

			memset(GridDrop->m_pItemList, 0, sizeof(GridDrop->m_pItemList));
		}

		GridDrop->m_nNumItem = 0;

		for (int i = 0; i < 12; i++)
		{

			auto GridDrop1 = (SGridControl*)m_pControlContainer->FindControl(3000012 + (i * 4));

			memset(GridDrop1->m_pbFilled, 0, GridDrop1->m_nColumnGridCount * sizeof(int) * GridDrop1->m_nRowGridCount);
			for (int i = 0; i < GridDrop1->m_nNumItem; ++i)
			{
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == GridDrop1->m_pItemList[i])
					g_pCursor->m_pAttachedItem = 0;

				memset(GridDrop1->m_pItemList, 0, sizeof(GridDrop->m_pItemList));
			}

			GridDrop1->m_nNumItem = 0;

		}
		m_dwLastLogout = g_pTimerManager->GetServerTime();
		MSG_SysQuit stParm{};
		stParm.Header.ID = m_pMyHuman->m_dwID;
		stParm.Header.Type = MSG_SysQuit_Opcode;
		stParm.Parm = 0;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm)->Type, reinterpret_cast<char*>(&stParm), sizeof(stParm)});
		return 1;
	}
	if (idwControlID == 65883)
	{
		m_pSystemPanel->SetVisible(0);
		return 1;
	}
	if (idwControlID == 641)
	{
		if (!g_pObjectManager || !m_pMyHuman || !m_pPartyList || !m_pPGTPanel || !m_pMessagePanel)
		{
			m_pPGTOver = 0;
			return 1;
		}
		auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(m_dwOpID);
		if (!pHuman)
		{
			m_pPGTOver = 0;
			return 1;
		}
		else if (pHuman->m_bParty)
		{
			m_pPGTOver = 0;
			return 1;
		}
		else if (m_pMyHuman->m_cDie == 1)
		{
			m_pPGTOver = 0;
			return 1;
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
			return 0;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[156], 2000);
		m_pPGTOver = 0;
		return 1;
	}
	if (idwControlID == 643)
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
			return 1;
		}

		if (!pTradeTarget || tradeDistance > (int)(((float)(g_pObjectManager->m_pCamera->m_fMaxCamLen - 11.0) + 6.0)))
		{
			WYD748_DiagnosticsLog("TRADE_CLICK blocked=target-or-distance\r\n");
			return 1;
		}

		m_pPGTOver = pTradeTarget;
		if (m_pMyHuman->m_cDie == 1)
		{
			WYD748_DiagnosticsLog("TRADE_CLICK blocked=dead\r\n");
			return 1;
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
			return 1;
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
			return 0;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[35], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		WYD748_DiagnosticsLog("TRADE_CLICK blocked=trade-busy opponent=%u visible=%d\r\n",
			g_pObjectManager->m_stTrade.OpponentID, pTradePanel->IsVisible());
		return 1;
	}
	if (idwControlID == 642)
	{
		if (!m_pMyHuman || !m_pPGTOver || !m_pPGTPanel ||
			!m_pBtnPGTGuild || !m_pBtnPGTParty || !m_pBtnPGTTrade || !m_pBtnPGTChallenge ||
			!m_pBtnPGT1_V_1 || !m_pBtnPGT5_V_5 || !m_pBtnPGT10_V_10 || !m_pBtnPGTAll_V_All ||
			!m_pBtnPGTGuildDrop || !m_pBtnPGTGuildWar || !m_pBtnPGTGuildAlly || !m_pBtnPGTGuildInvite ||
			!m_pBtnPGTGICommon || !m_pBtnPGTGIChief1 || !m_pBtnPGTGIChief2 || !m_pBtnPGTGIChief3)
			return 0;
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
		return 0;
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
		if (!m_pPGTOver || !m_pMessagePanel || !g_pObjectManager || !m_pMyHuman || !m_pPGTPanel)
			return 0;

		if (!g_pCurrentScene)
			return 0;
		if (m_pPGTOver->m_usGuild)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[364], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 0;
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
			return 0;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[155], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return 0;
	}
	if (idwControlID >= 913 && idwControlID <= 915)
	{
		if (!g_pCurrentScene || !m_pMessagePanel || !m_pMyHuman || !g_pObjectManager || !m_pPGTPanel)
			return 0;

		if (m_pPGTOver != nullptr)
		{
			if (!m_pPGTOver->m_usGuild)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[367], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
			if (m_pPGTOver->m_usGuild != m_pMyHuman->m_usGuild)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[365], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
			if (m_pPGTOver->m_sGuildLevel >= 3 && m_pPGTOver->m_sGuildLevel <= 8)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[366], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
			if (g_pObjectManager->m_stMobData.Coin < 50000000)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[155], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
		}

		VisibleInputGuildName();
		m_pPGTPanel->SetVisible(0);
		return 0;
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
		if (!m_pMyHuman || !m_pPGTPanel)
			return 0;
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
		return 0;
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
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		// Mastery allocation follows the same wide-counter contract as status.
		if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
			return 0;

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

		if (pMobData->CurrentScore.Mastery[0] < totalSpecial)
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
		return 1;
	}
	if (idwControlID == 65757)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
			return 0;

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

		if (pMobData->CurrentScore.Mastery[1] < totalSpecial)
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
		return 1;
	}
	if (idwControlID == 65760)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
			return 0;

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

		if (pMobData->CurrentScore.Mastery[2] < totalSpecial)
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
		return 1;
	}
	if (idwControlID == 65763)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (g_pObjectManager->m_stMobData.CurrentScore.MasterPts == 0)
			return 0;

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

		if (pMobData->CurrentScore.Mastery[3] < totalSpecial)
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
		return 1;
	}
	if (idwControlID == 475138 || (m_bCompatFieldScene && idwControlID == 1863))
	{
		auto pPartyList = m_pPartyList;
		if (!pPartyList)
			return 0;

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
				return 1;

			int Special = m_pMyHuman->m_stScore.Level;
			int classId = skillId - 24 * g_pObjectManager->m_stMobData.Class;
			if (skillId < 96)
				Special = g_pObjectManager->m_stMobData.CurrentScore.Mastery[classId / 8 + 1];

			if (BASE_GetManaSpent(skillId, g_pObjectManager->m_stMobData.CurrentScore.SaveMana, Special) > g_pObjectManager->m_stMobData.CurrentScore.CurMP)
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

				return 1;
			}

			if (dwServerTime > m_dwOldAttackTime + 1000	&& skillId == 42 && pPartyItem->m_dwCharID == m_pMyHuman->m_dwID)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[40], 1000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
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
						return 1;
					if (!pHuman->m_bParty)
						return 1;

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
						return 1;

					int my_att = g_pAttribute[y1 / 4][x1 / 4];
					int other_att = g_pAttribute[y2 / 4][x2 / 4];
					if (!(my_att & 0x40))
					{
						if (other_att & 0x40)
							return 1;
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
		return 1;
	}
	if (idwControlID == 475139)
	{
		if (!m_pChatListPanel || !m_pChatListPanel->IsVisible())
		{
			if (!m_pMyHuman)
				return 1;

			MSG_RemoveParty stParm{};
			stParm.Header.Type = MSG_RemoveParty_Opcode;
			stParm.Header.ID = m_pMyHuman->m_dwID;
			stParm.Parm = 0;

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParm)->Type, reinterpret_cast<char*>(&stParm), sizeof(stParm)});

			if (m_pPartyPanel && m_pPartyPanel->IsVisible())
				SetVisibleParty();
		}
		return 1;
	}
	if (idwControlID == 617)
	{
		if (!m_pTradePanel || !m_pTradePanel->IsVisible() || !m_pMyHuman ||
			!g_pObjectManager || !g_pObjectManager->m_stTrade.OpponentID ||
			!g_pApp || !g_pApp->m_pTimerManager)
			return 1;

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
			return 0;
		}

		if (m_pMessagePanel)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[41], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
		return 1;
	}
	if (idwControlID == 667)
	{
		if (!field_interaction::HasAutoTradeOffers(m_stAutoTrade.Item))
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[145], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
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

		return 1;
	}

	// The newer source assumes InitBoard always created the teleport list.  The
	// 7.48 resource may omit it, so unrelated buttons must never enter portal
	// bookkeeping through a null list pointer.
	if (idwControlID == 12546 && m_pPotalList)
	{
		if (m_bAirmove_ShowUI == 1)
		{
			auto pItem = m_pPotalList->GetSelectedIndex();

			if (pItem + 1 != 0 && !m_bAirMove)
				AirMove_Start(pItem);

			AirMove_ShowUI(0);
			return 1;
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
				return 1;
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
		return 1;
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

	if (m_bCompatFieldScene)
	{
		if (iCharCode == 'a' || iCharCode == 'A')
			return ToggleNativeCCMode(1);
		if (iCharCode == 'd' || iCharCode == 'D')
			return ToggleNativeCCMode(2);
	}

	if (UseQuickSloat(iCharCode))
		return 1;

	if (OnKeyTotoEnter(iCharCode, lParam))
		return 1;

	if (OnKeyDebug(iCharCode, lParam))
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

		if (OnKeyReverse(iCharCode, lParam))
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
	DWORD dwServerTime = g_pTimerManager->GetServerTime();
	// The 7.48 resource does not instantiate the 7.59 chat/target/message
	// controls dereferenced below.  Route it through the byte-compatible field
	// input adapter instead of manufacturing unrelated UI objects.
	if (m_bCompatFieldScene)
		return OnMouseEventCompat(dwFlags, wParam, nX, nY);

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

	if (dwServerTime < m_dwNPCClickTime + 1000)
		return 1;

	if (m_bAirMove == 1)
		return 0;

	if (TMScene::OnMouseEvent(dwFlags, wParam, nX, nY) == 1)
		return 1;

	if (g_bActiveWB == 1)
		return 1;

	if (m_bCriticalError == 1)
		return 1;

	for (int nModIndex = 0; nModIndex < 8; ++nModIndex)
	{
		if (m_pControlContainer->m_pModalControl[nModIndex] &&
			m_pControlContainer->m_pModalControl[nModIndex]->IsVisible())
		{
			if (m_pControlContainer->m_pModalControl[nModIndex] != static_cast<SControl*>(m_pMessageBox))
				return 1;

			if (m_pMessageBox->m_dwMessage != 601 && m_pMessageBox->m_dwMessage != MSG_PlayerChallenge_Opcode)
				return 1;
		}
	}

	if (nX != m_nLastMousePosX || nY != m_nLastMousePosY || !g_pEventTranslator->button[0] && !g_pEventTranslator->button[1])
	{
		m_nLastMousePosX = nX;
		m_nLastMousePosY = nY;
		m_dwLastMousePosTime = dwServerTime;
	}

	if (m_pMessageBox->m_bVisible == 1 &&
		m_pMessageBox->m_dwMessage == 99 &&
		g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0 &&
		!m_pMyHuman->m_cDie)
	{
		m_pMessageBox->SetVisible(0);
	}

	auto pMobData = &g_pObjectManager->m_stMobData;

	if (m_cResurrect ||
		pMobData->Equip[13].sIndex == 769 ||
		g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0 && m_pMyHuman->m_cDie != 1 ||
		dwFlags != 516 ||
		wParam & 8 ||
		nX <= 0 ||
		nY <= 0 ||
		nX >= static_cast<int>(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) ||
		nY >= static_cast<int>(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
	{
		if ((dwFlags == WM_LBUTTONDOWN || dwFlags == WM_RBUTTONDOWN) && OfferRespawnPrompt(true))
			return 1;
		if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_DEAD)
			return 1;

		if (wParam & 4 && dwFlags == 516)
		{
			if (m_pMouseOverHuman)
			{
				if (m_pMouseOverHuman->m_dwID > 0 && m_pMouseOverHuman->m_dwID < 1000)
				{
					char szStrTemp[128]{};
					sprintf_s(szStrTemp, "/%s ", m_pMouseOverHuman->m_szName);

					m_pEditChat->SetText(szStrTemp);

					m_dwOldAttackTime = dwServerTime;
					return 1;
				}
			}
		}

		auto vec = GroundGetPickPos();

		auto pPanel = m_pInvenPanel;

		if (m_pMyHuman->m_bSliding == 1)
			return 1;

		if (m_pMyHuman->m_cOnlyMove == 1
			&& (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 ||
				m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND02 ||
				m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_LEVELUP))
		{
			m_pMyHuman->m_cOnlyMove = 0;
			m_pMyHuman->SetSpeed(m_bMountDead);
		}

		if (m_pMyHuman->m_cOnlyMove == 1)
			return 1;

		if (m_pMyHuman->m_cSameHeight == 1)
			return 1;

		if (dwFlags == 512)
			MouseMove(nX, nY);

		if (dwFlags == 513)
		{
			if (m_cLastFlagLButtonUp == 1)
				m_dwLastMouseDownTime = dwServerTime;

			m_cLastFlagLButtonUp = 0;
		}

		if (g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0)
		{
			int SWidth = g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift;
			int SHeight = g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift;

			if (nX > 0 && nY > 0 && nX < SWidth && nY < SHeight)
			{
				if (dwFlags == 517 && wParam & 8)
				{
					if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
						return 1;

					if (m_pMouseOverHuman && m_pMouseOverHuman->m_dwEdgeColor != 0x8800FF00)
						PGTVisible(dwServerTime);

					return 1;
				}

				if (dwFlags == 516 && !(wParam & 8))
				{
					if (wParam & 4)
						return SkillUse(nX, nY, vec, dwServerTime, 0, 0);
					else
						return SkillUse(nX, nY, vec, dwServerTime, 1, 0);
				}

				if (dwFlags == 514)
				{
					m_cLastFlagLButtonUp = 1;
					m_bMoveing = 0;

					if (m_pMyHuman->m_cHide == 1)
						return 1;

					if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
						return 1;

					return MouseClick_NPC(nX, nY, vec, dwServerTime);
				}

				if (dwFlags == 513 && !m_pMyHuman->m_cHide && (!m_bMoveing || wParam & 4))
				{
					int nRet = MobAttack(wParam, vec, dwServerTime);

					if (!nRet)
						nRet = CheckMerchant(m_pMouseOverHuman);

					float fHeight = static_cast<float>(GroundGetMask(TMVector2{ vec.x, vec.z })) * 0.1f;

					TMVector3 vecTar{ vec.x, fHeight + 0.30000001f, vec.z };

					m_pTarget1->m_vecPosition = vecTar;
					m_pTarget2->m_vecPosition = vecTar;
					m_pTargetBill->m_vecPosition = vecTar;

					m_pTargetBill->m_vecPosition.y -= 0.2f;

					if (nRet)
						return nRet;
				}

				if (dwFlags != 513 || wParam & 4 || g_pCursor->m_pAttachedItem || m_pMyHuman->m_cCantMove)
				{
					if (dwFlags == 513 && pPanel->IsVisible() == 1 && g_pCursor->m_pAttachedItem)
					{
						if (g_pCursor->m_pAttachedItem->m_pItem->sIndex >= 5000 && g_pCursor->m_pAttachedItem->m_pItem->sIndex < 5096)
							return 1;

						DropItem(dwServerTime);
						return 1;
					}
				}
				else if (m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_SEATING && m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_PUNISHING)
				{
					if ((!m_pChatBack || static_cast<float>(nX) >= (m_pChatBack->GetPos().x + 10.0f)) &&
						abs(static_cast<int>(vec.x - m_pMyHuman->m_vecPosition.x)) < 15 &&
						abs(static_cast<int>(vec.y - m_pMyHuman->m_vecPosition.y)) > 15)
					{
						m_bMoveing = 1;

						float fHeight = static_cast<float>(GroundGetMask(TMVector2{ vec.x, vec.z })) * 0.1f;

						TMVector3 vecTar{ vec.x, fHeight + 0.30000001f, vec.z };

						m_pTarget1->m_vecPosition = vecTar;
						m_pTarget2->m_vecPosition = vecTar;
						m_pTargetBill->m_vecPosition = vecTar;
						m_pTargetBill->m_vecPosition.y -= 0.2f;

						MobMove(vec, dwServerTime);
					}
				}
				else
				{
					if (m_pMyHuman->m_SendeMotion != ECHAR_MOTION::ECMOTION_NONE)
						return 1;

					MSG_Motion stMotion{};

					stMotion.Header.ID = g_pObjectManager->m_dwCharID;
					stMotion.Header.Type = MSG_Motion_Opcode;

					if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEATING)
						stMotion.Motion = 25;
					else
						stMotion.Motion = 27;

					stMotion.Direction = 0.0f;

					m_pMyHuman->m_SendeMotion = static_cast<ECHAR_MOTION>(stMotion.Motion);

					SendPacket({reinterpret_cast<MSG_STANDARD*>(&stMotion)->Type, reinterpret_cast<char*>(&stMotion), sizeof(stMotion)});

					m_dwKeyTime = dwServerTime;
				}
			}
		}
		return 0;
	}

	for (int i = 0; i < 20; ++i)
	{
		char cSkillIndex = g_pObjectManager->m_cShortSkill[i];

		if (cSkillIndex == 99)
		{
			g_pObjectManager->m_cSelectShortSkill = cSkillIndex;

			UpdateSkillBelt();

			m_pMessageBox->SetMessage(g_pMessageStringTable[227], 99u, 0);
			m_pMessageBox->SetVisible(1);
			return 1;
		}
	}

	return 1;
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
		return m_bCompatFieldScene ? OnPacketCreateMobCompat(pStd) : OnPacketCreateMob(pStd);
	case MSG_Action_Opcode:
	case MSG_Action_Stop_Opcode:
		return OnPacketAction(pStd);
	case MSG_REQArray_Opcode:
		return OnPacketREQArray(pStd);
	case 0x333:
		return OnPacketMessageChat(reinterpret_cast<MSG_MessageChat*>(pStd));
	case 0x105:
		return OnPacketMessageChat_Index(reinterpret_cast<MSG_MessageChat*>(pStd));
	case 0x106:
		return OnPacketMessageChat_Param(pStd);
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
	case 0x18B:
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
	case 0x1C5:
		return OnPacketNuke(pStd);
		break;
	case 0x1C6:
		return OnPacketRandomQuiz(reinterpret_cast<MSG_RandomQuiz*>(pStd));
	case quiz_event::ChallengeOpcode:
		return OnPacketQuizEvent(pStd);
		break;
	case 0x2C8:
		return OnPacketAutoKick(pStd);
		break;
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

	case 0xAA1:
		return OnPacketNewCashRev((PacketRevDonate*)pStd);
		break;

	case 0xAA2:
		return OnPacketNewCashRev2((PacketRevDonate2*)pStd);
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

	// The compatibility scene has no source-tree HUD graph.  The base scene
	// still advances terrain streaming, camera and child objects safely; the
	// full gameplay HUD tick is skipped because it assumes controls absent in
		// the 7.48 FieldScene2.bin and was the remaining post-login crash path.
	if (m_bCompatFieldScene)
	{
		if (!g_pTimerManager)
			return 0;
		dwServerTime = g_pTimerManager->GetServerTime();
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
		// The compact lifecycle must also advance the stock affect row.  Returning
		// before this call left every 0x3B9 duration entry permanently invisible,
		// even though InitializeCompatFieldScene created the native 7.48 icons.
		Affect_Main(dwServerTime);
		if (UpdateTeleportPrompt() == 1)
			return 1;
		// The 7.48 HUD has its own lifecycle, but CC still needs the existing
		// item/skill/attack scheduler after transition gates, once per frame.
		GameAuto();
		UpdateSkillCooldownUI(dwServerTime);
		return 1;
	}

	if (g_bEffectFirst == 1)
	{
		UpdateMyHuman();
		g_bEffectFirst = 0;
	}

	dwServerTime = g_pTimerManager->GetServerTime();

	TMScene::FrameMove(dwServerTime);
	UpdateGambleRequestTimeout();

	if (TimeDelay(dwServerTime) == 1)
		return 1;

	//if (m_dwDeleteURLTime && dwServerTime > m_dwDeleteURLTime + 1000)
	//{
	//	if (DelTempFiles(g_pMessageStringTable[269]) == 1)
	//		m_dwDeleteURLTime = 0;
	//	else
	//		m_dwDeleteURLTime = dwServerTime;
	//}

	AirMove_Main(dwServerTime);
	Affect_Main(dwServerTime);
	if (UpdateTeleportPrompt() == 1)
		return 1;

	if ((m_pGround->m_vecOffsetIndex.x != 8 || m_pGround->m_vecOffsetIndex.y != 15) &&
		(m_pGround->m_vecOffsetIndex.x != 8 || m_pGround->m_vecOffsetIndex.y != 16) &&
		(m_pGround->m_vecOffsetIndex.x != 9 || m_pGround->m_vecOffsetIndex.y != 15) &&
		(m_pGround->m_vecOffsetIndex.x != 9 || m_pGround->m_vecOffsetIndex.y != 16))
	{
		if (m_bTempCastlewar == 1)
		{
			g_bCastleWar = 0;
			g_bCastleWar2 = 0;
		}
	}
	else if (!g_bCastleWar)
	{
		g_bCastleWar = 1;
		g_bCastleWar2 = 1;
		m_bTempCastlewar = 1;
	}
	if (!m_nAdjustTime && dwServerTime > m_dwInitTime + 3000)
		m_nAdjustTime = 1;

	GameAuto();

	int azran = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
	if (azran != 1 ||
		m_pMyHuman->m_vecPosition.x < 2604.0f || m_pMyHuman->m_vecPosition.x > 2651.0f ||
		m_pMyHuman->m_vecPosition.y < 1708.0f || m_pMyHuman->m_vecPosition.y > 1744.0f)
	{
		if (g_bEvent == 1)
		{
			g_bEvent = 0;
			m_bShowNameLabel = 1;
		}
	}
	else if (!g_bEvent)
	{
		g_bEvent = 1;
		m_bShowNameLabel = 0;
	}
	recallDeadPlayer(dwServerTime);
	if (dwServerTime - m_dwQuizStart > 5000 && m_pQuizPanel && m_pQuizPanel->m_bVisible)
		m_pQuizPanel->SetVisible(0);

	bool bInTown = m_pMyHuman->IsInTown();
	unsigned int WaitTime = 180000;

	if (dwServerTime - m_dwEventStartTime > WaitTime)
	{
		++m_nCurrEventTextIndex;
		m_nCurrEventTextIndex %= 4;
		m_dwEventStartTime = dwServerTime;

		auto pChatList = m_pChatListnotice;
		for (int k = 3; k >= 0; --k)
		{
			if (pChatList == nullptr)
				break;

			if (strlen(m_szEventTextTemp[k]))
			{
				auto pChatItem = new SListBoxItem(m_szEventTextTemp[k],
					0xFFFFF6A6,
					0.0f,
					0.0f,
					300.0f,
					16.0f,
					0,
					0x77777777,
					1,
					0);

				if (pChatItem)
					pChatList->AddItem(pChatItem);
			}
		}
	}

	auto pCPanel = m_pCPanel;

	if (pCPanel && pCPanel->IsVisible() == 1)
	{
		int btPressType = 0;
		auto pOpSTRButton = (SButton*)m_pControlContainer->FindControl(65716);
		auto pOpINTButton = (SButton*)m_pControlContainer->FindControl(65719);
		auto pOpDEXButton = (SButton*)m_pControlContainer->FindControl(65722);
		auto pOpCONButton = (SButton*)m_pControlContainer->FindControl(65725);
		if (pOpSTRButton->IsOver() == 1)
			btPressType = 65716;
		if (pOpINTButton->IsOver() == 1)
			btPressType = 65719;
		if (pOpDEXButton->IsOver() == 1)
			btPressType = 65722;
		if (pOpCONButton->IsOver() == 1)
			btPressType = 65725;

		auto pOpSP1Button = (SButton*)m_pControlContainer->FindControl(65754);
		auto pOpSP2Button = (SButton*)m_pControlContainer->FindControl(65757);
		auto pOpSP3Button = (SButton*)m_pControlContainer->FindControl(65760);
		auto pOpSP4Button = (SButton*)m_pControlContainer->FindControl(65763);
		if (pOpSP1Button->IsOver() == 1)
			btPressType = 65754;
		if (pOpSP2Button->IsOver() == 1)
			btPressType = 65757;
		if (pOpSP3Button->IsOver() == 1)
			btPressType = 65760;
		if (pOpSP4Button->IsOver() == 1)
			btPressType = 65763;

		if (!btPressType || !g_pEventTranslator->button[0])
		{
			m_bClbutton = 0;
			m_dwLastClbuttonTime = 0;
		}
		else if (m_bClbutton)
		{
			if (m_dwLastClbuttonTime + 100 < dwServerTime)
			{
				OnControlEvent(btPressType, 0);
				m_dwLastClbuttonTime = dwServerTime;
			}
		}
		else if (m_dwLastClbuttonTime)
		{
			if (m_dwLastClbuttonTime + 1500 < dwServerTime)
			{
				m_dwLastClbuttonTime = dwServerTime;
				m_bClbutton = 1;
			}
		}
		else
		{
			m_dwLastClbuttonTime = dwServerTime;
		}
	}

	if (dwServerTime - TMHouse::m_dwVisibleWaterFall > 1000)
	{
		auto pSoundManager = g_pSoundManager;
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(6);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}
	}

	if (m_bCriticalError == 1 && m_pMessagePanel)
	{
		m_pMessagePanel->SetMessage("Critical Data Error In Client", 0);
		m_pMessagePanel->SetVisible(1, 0);
		return 1;
	}

	if (m_pMiniMapDir)
		m_pMiniMapDir->GetGeomControl()->fAngle = m_pMyHuman->m_fAngle - 2.3561945f;

	if (m_pMiniMapPanel)
	{
		if (m_pMiniMapPanel->m_bVisible)
		{
			float fX = 0.0f;
			float fY = 0.0f;
			float fX2 = 0.0f;
			float fY2 = 0.0f;

			auto pFocused = g_pObjectManager->m_pCamera->m_pFocusedObject;
			if (pFocused)
			{
				int nCullSize = 400;
				if (TMGround::m_fMiniMapScale < 1.0)
					nCullSize = 130;

				int PlusX = 60;
				int PlusY = 10;
				for (int l = 0; l < 256; ++l)
				{
					fX = pFocused->m_vecPosition.x - (float)g_MinimapPos[l].nX;
					fY = pFocused->m_vecPosition.y - (float)g_MinimapPos[l].nY;

					// TODO: confirm this "-" latter.
					fX2 = ((-((0.70710701f * fX) + (-0.70710701f * fY))	* 2.0f) + 78.0f)
						+ ((signed int)TMGround::m_fMiniMapScale * 128.0f);
					fY2 = ((((0.70710701f * fX) + (0.70710701f * fY)) * 2.0f) + 78.0f)
						+ ((signed int)TMGround::m_fMiniMapScale * 128.0f);

					if (fX2 >= 0.0 && fX2 <= (float)nCullSize && fY2 >= 0.0 && fY2 <= (float)nCullSize)
						m_pInMiniMapPosPanel[l]->SetVisible(1);
					else
						m_pInMiniMapPosPanel[l]->SetVisible(0);

					if (((float)g_MinimapPos[l].nCX + fX2) >= 0.0f && ((float)g_MinimapPos[l].nCX + fX2) <= (float)(nCullSize - PlusX) &&
						((float)g_MinimapPos[l].nCY + fY2) >= 0.0f && ((float)g_MinimapPos[l].nCY + fY2) <= (float)(nCullSize - PlusY))
					{
						if (TMGround::m_fMiniMapScale >= 1.0f)
							m_pInMiniMapPosText[l]->SetVisible(1);
						else
							m_pInMiniMapPosText[l]->SetVisible(0);
					}
					else
					{
						m_pInMiniMapPosText[l]->SetVisible(0);
					}

					m_pInMiniMapPosPanel[l]->SetPos(BASE_ScreenResize(fX2), BASE_ScreenResize(fY2));
					if (TMGround::m_fMiniMapScale >= 1.0f)
					{
						m_pInMiniMapPosText[l]->SetPos(BASE_ScreenResize((float)g_MinimapPos[l].nCX + fX2),
							BASE_ScreenResize((float)g_MinimapPos[l].nCY + fY2));
					}
				}
			}
		}
	}
	if (m_pTarget1 && m_pTarget2 && m_pTargetBill)
	{
		if (m_pTarget1->m_bShow == 1)
		{
			m_pTarget1->m_fAngle = ((float)(dwServerTime % 1000) * 3.1415927f) / 500.0f;
			m_pTarget2->m_fAngle = m_pTarget1->m_fAngle;

			m_pTargetBill->m_vecScale.x = (float)(sinf(m_pTarget1->m_fAngle * 2.0f) * 0.1f) + 1.0f;
			m_pTargetBill->m_vecScale.y = m_pTargetBill->m_vecScale.x;
			m_pTargetBill->m_vecScale.z = m_pTargetBill->m_vecScale.x;
		}
		if ((m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK|| m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
			&& !m_bAutoRun && !m_pMyHuman->m_pMoveTargetHuman && !m_pMyHuman->m_pMoveSkillTargetHuman && !m_pTargetItem)
		{
			m_pTarget1->m_bShow = 1;
			m_pTarget2->m_bShow = 1;
			m_pTargetBill->m_bShow = 1;
		}
		else
		{
			m_pTarget1->m_bShow = 0;
			m_pTarget2->m_bShow = 0;
			m_pTargetBill->m_bShow = 0;
		}
	}

	bool bWarn = 0;
	for (int nZone = 0; nZone < 7; ++nZone)
	{
		POINT pt;
		pt.x = (int)m_pMyHuman->m_vecPosition.x;
		pt.y = (int)m_pMyHuman->m_vecPosition.y;
		if (PtInRect(&TMFieldScene::m_rectWarning[nZone], pt) == 1)
		{
			bWarn = 1;
			break;
		}
	}

	if (!m_bWarning && bWarn == 1)
	{
		m_bWarning = 1;
		m_pMessagePanel->SetMessage(g_pMessageStringTable[42], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	else if (!bWarn)
		m_bWarning = 0;

	int nDesert = (int)m_pMyHuman->m_vecPosition.x >> 7 >= 8 && (int)m_pMyHuman->m_vecPosition.x >> 7 <= 12 &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 >= 11&& (int)m_pMyHuman->m_vecPosition.y >> 7 <= 14;

	if (!RenderDevice::m_bDungeon && nDesert == 1)
	{
		RenderDevice::m_bDungeon = 3;
	}
	else if (RenderDevice::m_bDungeon == 3 && !nDesert)
	{
		RenderDevice::m_bDungeon = 0;
	}
	if (m_pRemainText && !m_bInstanceRemainOn && RenderDevice::m_bDungeon == 4)
	{
		if (dwServerTime - m_dwRemainTime > 900000)
			m_pRemainText->SetVisible(0);
	}
	else if (m_pRemainText && !m_bInstanceRemainOn && RenderDevice::m_bDungeon != 5 &&
		dwServerTime - m_dwRemainTime > 6000)
	{
		m_pRemainText->SetVisible(0);
	}
	if (RenderDevice::m_bDungeon >= 0)
	{
		UpdateQuestTime();
	}
	else if (m_pQuestRemainTime && m_pQuestRemainTime->IsVisible() == 1)
	{
		SetQuestStatus(0);
	}

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 6 && (int)m_pMyHuman->m_vecPosition.y >> 7 == 28 && m_pPositionText)
		m_pPositionText->SetVisible(0);

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 != 31 && (int)m_pMyHuman->m_vecPosition.y >> 7 != 31 &&
		m_pGambleStore && m_pGambleStore->m_bVisible == 1)
		SetVisibleGamble(0, 0);

	if (!g_bCastleWar2)
	{
		STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
		if (bInTown == 1)
		{
			POINT pt;
			pt.x = (int)m_pMyHuman->m_vecPosition.x;
			pt.y = (int)m_pMyHuman->m_vecPosition.y;

			int nMusicIndex = 0;
			if (PtInRect(&rectTownInCastle, pt) != 1)
			{
				int city = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
				if (city == 2)
					city = 0;

				nMusicIndex = 2 * city + 1;
			}
			else
				nMusicIndex = 8;

			if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
			{
				m_dwInTownTime = dwServerTime;
				if (g_pApp->m_pBGMManager)
				{
					SAFE_DELETE(g_pApp->m_pBGMManager);

					g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
					g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
					g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
				}

				DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;

				MSG_ChangeCity dst{};
				dst.Header.ID = g_pObjectManager->m_dwCharID;
				dst.Header.Type = MSG_ChangeCity_Opcode;
				dst.Village = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);

				if (dst.Village < 4)
				{
					SendOneMessage((char*)&dst, 16);
					g_pObjectManager->m_stSelCharData.HomeTownX[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.x;
					g_pObjectManager->m_stSelCharData.HomeTownY[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.y;
				}
			}
		}
		else if((int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
		{
			if ((int)m_pMyHuman->m_vecPosition.x <= 1664 || (int)m_pMyHuman->m_vecPosition.x >= 1792 ||
				(int)m_pMyHuman->m_vecPosition.y <= 1536 || (int)m_pMyHuman->m_vecPosition.y >= 1920)
			{
				if (RenderDevice::m_bDungeon == 3)
				{
					int nMusicIndex = 9;
					if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
					{
						m_dwFieldTime = dwServerTime;
						if (g_pApp->m_pBGMManager)
						{
							SAFE_DELETE(g_pApp->m_pBGMManager);

							g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
							g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
							g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
						}
						DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
					}
					if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
						SetVisibleAutoTrade(0, 0);
				}
				else if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26	&& (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
						 (int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
				{
					int nMusicIndex = 9;
					if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
					{
						m_dwFieldTime = dwServerTime;
						if (g_pApp->m_pBGMManager)
						{
							SAFE_DELETE(g_pApp->m_pBGMManager);

							g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
							g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
							g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
						}
						DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
					}
					if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
						SetVisibleAutoTrade(0, 0);
				}
				else
				{
					POINT pt;
					pt.x = (int)m_pMyHuman->m_vecPosition.x;
					pt.y = (int)m_pMyHuman->m_vecPosition.y;

					int m;
					for (m = 0; m < 2; ++m)
					{
						if (PtInRect(&g_rectField[m], pt) != 1)
							break;
					}

					if (m == 2)
						m = 1;

					int nMusicIndex = 2 * m + 2;
					if (RenderDevice::m_bDungeon == 4 && m_pRemainText &&
						m_pRemainText->m_bVisible == 1)
						nMusicIndex = 5;

					if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
					{
						m_dwFieldTime = dwServerTime;
						if (g_pApp->m_pBGMManager)
						{
							SAFE_DELETE(g_pApp->m_pBGMManager);

							g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
							g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
							g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
						}
						DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
					}
					if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
						SetVisibleAutoTrade(0, 0);
				}
			}
			else
			{
				int nMusicIndex = 6;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}

				if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
					SetVisibleAutoTrade(0, 0);
			}
		}
		else
		{
			if ((int)m_pMyHuman->m_vecPosition.x >> 7 < 16 && (int)m_pMyHuman->m_vecPosition.x >> 7 > 8 &&
				(int)m_pMyHuman->m_vecPosition.y >> 7 > 25)
			{
				int nMusicIndex = 7;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
			}
			else if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 18 && (int)m_pMyHuman->m_vecPosition.y >> 7 == 30)
			{
				int nMusicIndex = 12;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
				if (m_pMyHuman && m_pMyHuman->m_vecPosition.y >= 3918.0)
					SetVisibleKhepraPortal(m_dwKhepraID == 0);
				else
					SetVisibleKhepraPortal(0);

				FrameMove_KhepraDieEffect(dwServerTime);
			}
			else if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 16 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 > 29)
			{
				int nMusicIndex = 11;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
			}
			else
			{
				int nMusicIndex = 5;

				if ((int)m_pMyHuman->m_vecPosition.x >> 7 != 31 && (int)m_pMyHuman->m_vecPosition.y >> 7 != 31)
				{
					if (m_pGambleStore && m_pGambleStore->m_bVisible == 1)
						SetVisibleGamble(0, 0);
				}
				else
					nMusicIndex = 6;

				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
			}

			if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
				SetVisibleAutoTrade(0, 0);
		}
	}
	else
	{
		if (bInTown == 1)
		{
			int nVillage = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
			if (m_nVillage != nVillage)
			{
				MSG_ChangeCity stParam{};
				stParam.Header.ID = g_pObjectManager->m_dwCharID;
				stParam.Header.Type = MSG_ChangeCity_Opcode;
				stParam.Village = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
				m_nVillage = nVillage;

				if (stParam.Village < 4)
				{
					SendOneMessage((char*)&stParam, 16);
					g_pObjectManager->m_stSelCharData.HomeTownX[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.x;
					g_pObjectManager->m_stSelCharData.HomeTownY[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.y;
				}
			}
		}
		else
		{
			m_nVillage = -1;
		}

		int nMusicIndex = 10;
		if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
		{
			m_dwFieldTime = dwServerTime;
			if (g_pApp->m_pBGMManager)
			{
				SAFE_DELETE(g_pApp->m_pBGMManager);

				g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
				g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
				g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
			}
			DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
		}
	}
	if (m_pPositionText)
	{
		if (m_pPositionText->IsVisible() == 1)
		{
			char szPos[64]{};
			sprintf(szPos, "X: %d Y: %d", (int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
			m_pPositionText->SetText(szPos, 0);

			char szServer[64]{};
			int nServerGroupIndex = nServerGroupIndex = g_pObjectManager->m_nServerGroupIndex;
			sprintf(szServer, "%s-%d", g_szServerNameList[nServerGroupIndex], g_pObjectManager->m_nServerIndex);
			if (m_pMiniMapServerText)
				m_pMiniMapServerText->SetText(szServer, 0);
		}
	}

	for (int i = 0; i < 32; ++i)
	{
		if ((unsigned int)m_pMyHuman->m_stAffect[i].Type > 0)
		{
			char szVal[128]{};
			int sTime = 8 * m_pMyHuman->m_stAffect[i].Time - (dwServerTime - m_dwStartAffectTime[i]) / 1000;

			if (sTime <= 0)
			{
				if (m_pAffectL[i])
				{
					if (!i)
						m_pAffect[0]->SetTextColor(0xFF000000);
					m_pAffect[i]->SetText((char*)"    0", 0);
					m_pAffectL[i]->SetVisible(0);
				}
				continue;
			}

			if (m_pMyHuman->m_stAffect[i].Time >= 1000000)
			{
				int add = m_pMyHuman->m_stAffect[i].Time / 1000000;
				int Year = m_pMyHuman->m_stAffect[i].Time % 1000000 / 10000;
				int Day = m_pMyHuman->m_stAffect[i].Time % 10000;
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
				{
					int freeDay = nDafaultDay - nYearDay * (m_nYear - Year) - (m_nDays - Day);
					sprintf(szVal, "%dD", freeDay);
				}
			}
			else if (sTime > 86400)
			{
				int nDay = sTime / 86400;
				sprintf(szVal, "%dD", sTime / 86400);
			}
			else if (sTime > 3600)
			{
				int nHour = sTime / 3600;
				sprintf(szVal, "%dH", sTime / 3600);
			}
			else if (sTime > 600)
			{
				int nMin = sTime / 60;
				sprintf(szVal, "%dM", sTime / 60);
			}
			else
			{
				sprintf(szVal, "%5d", sTime);
			}

			if (m_pAffectL[i])
			{
				if (!i)
				{
					if (sTime < 7375)
					{
						for (int nJ = 0; nJ < 7; ++nJ)
						{
							if (sTime < g_sTimeTable[nJ])
							{
								m_pAffect[i]->SetTextColor(g_dwFoodColor[nJ]);
								break;
							}
						}
					}
					else
						m_pAffect[i]->SetTextColor(0xFFFFFFFF);
				}
				m_pAffect[i]->SetText(szVal, 0);
			}

			if (m_pAffectL[i])
				m_pAffectL[i]->SetVisible(0);
			if (m_pMiniPanel && m_pAffectIcon[i])
			{
				if ((unsigned char)m_pMyHuman->m_stAffect[i].Type < 50)
					m_pAffectIcon[i]->m_GCPanel.nTextureIndex = g_AffectSkillType[(unsigned char)m_pMyHuman->m_stAffect[i].Type];
				else
					m_pAffectIcon[i]->m_GCPanel.nTextureIndex = 0;
				m_pAffectIcon[i]->m_GCPanel.nLayer = 29;
			}
		}
		else
		{
			if (m_pAffectL[i])
				m_pAffectL[i]->SetVisible(0);
			m_dwAffectBlinkTime[i] = 0;
		}
	}
	if (g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0 && m_bAutoRun && !g_pCursor->m_pAttachedItem)
	{
		TMVector2 vec = m_pMyHuman->m_vecPosition;
		float fDir = 1.0f;

		if (m_bReverse)
			fDir = -1.0f;

		vec.x = (((fDir * g_pObjectManager->m_pCamera->m_vecCamDir.x)
			* m_pMyHuman->m_fMaxSpeed)
			* 1.5f)
			+ vec.x;
		vec.y = (((fDir * g_pObjectManager->m_pCamera->m_vecCamDir.z)
			* m_pMyHuman->m_fMaxSpeed)
			* 1.5f)
			+ vec.y;

		if (!BASE_IsInLowZone((int)vec.x, (int)vec.y))
			MobMove2(vec, dwServerTime);
	}
	if (!m_pMyHuman->m_cLastMoveStop)
	{
		if (m_pMyHuman->m_fMaxSpeed >= 5.0f)
		{
			if (m_pMyHuman->m_fProgressRate >= 0.8f || m_pMyHuman->m_fProgressRate == 0.0f)
			{
				if ((int)m_pMyHuman->m_vecPosition.x == m_vecMyNext.x && (int)m_pMyHuman->m_vecPosition.y == m_vecMyNext.y &&
					(m_pMyHuman->m_LastSendTargetPos.x != m_vecMyNext.x || m_pMyHuman->m_LastSendTargetPos.y != m_vecMyNext.y) &&
					dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000 && !m_pMyHuman->m_cDie)
				{
					m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

					MSG_Action Msg{};
					Msg.Header.ID = m_pMyHuman->m_dwID;
					Msg.PosX = m_stMoveStop.NextX;
					Msg.PosY = m_stMoveStop.NextY;
					Msg.Effect = 0;
					Msg.Header.Type = MSG_Action_Opcode;
					Msg.Speed = g_nMyHumanSpeed;
					Msg.TargetX = m_pMyHuman->m_LastSendTargetPos.x;
					Msg.TargetY = m_pMyHuman->m_LastSendTargetPos.y;

					for (int n = 0; n < 23; ++n)
						Msg.Route[n] = 0;

					g_bLastStop = Msg.Header.Type;
					m_stMoveStop.LastX = Msg.PosX;
					m_stMoveStop.LastY = Msg.PosY;
					m_stMoveStop.NextX = Msg.TargetX;
					m_stMoveStop.NextY = Msg.TargetY;
					SendOneMessage((char*)&Msg, 52);
					m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
				}
				else if (m_vecMyNext.x != (int)m_pMyHuman->m_vecPosition.x || m_vecMyNext.y != (int)m_pMyHuman->m_vecPosition.y)
				{
					m_pMyHuman->GetRoute(m_vecMyNext, 0, m_pMyHuman->m_cLastMoveStop);
				}
			}
		}
		else if (m_pMyHuman->m_fProgressRate >= 0.90f || m_pMyHuman->m_fProgressRate == 0.0f)
		{
			int nPosX = (int)m_pMyHuman->m_vecPosition.x;
			int nPosY = (int)m_pMyHuman->m_vecPosition.y;
			if (nPosX == m_vecMyNext.x && nPosY == m_vecMyNext.y && (m_pMyHuman->m_LastSendTargetPos.x != m_vecMyNext.x	||
				m_pMyHuman->m_LastSendTargetPos.y != m_vecMyNext.y) && dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000 && !m_pMyHuman->m_cDie)
			{
				m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

				MSG_Action stAction{};
				stAction.Header.ID = m_pMyHuman->m_dwID;
				stAction.PosX = m_pMyHuman->m_LastSendTargetPos.x;
				stAction.PosY = m_pMyHuman->m_LastSendTargetPos.y;
				stAction.Effect = 0;
				stAction.Header.Type = MSG_Action_Opcode;
				stAction.Speed = g_nMyHumanSpeed;
				stAction.TargetX = m_pMyHuman->m_LastSendTargetPos.x;
				stAction.TargetY = m_pMyHuman->m_LastSendTargetPos.y;

				for (int j = 0; j < 23; ++j)
					stAction.Route[j] = 0;

				g_bLastStop = stAction.Header.Type;
				m_stMoveStop.LastX = stAction.PosX;
				m_stMoveStop.LastY = stAction.PosY;
				m_stMoveStop.NextX = stAction.TargetX;
				m_stMoveStop.NextY = stAction.TargetY;
				SendOneMessage((char*)&stAction, 52);
				m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
			}
			else if (m_vecMyNext.x != (int)m_pMyHuman->m_vecPosition.x || m_vecMyNext.y != (int)m_pMyHuman->m_vecPosition.y)
				m_pMyHuman->GetRoute(m_vecMyNext, 0, m_pMyHuman->m_cLastMoveStop);
		}
	}
	if (m_pTargetItem)
		m_pMyHuman->MoveGet(m_pTargetItem);

	if (m_dwStartFlashTime)
	{
		if (m_fFlashTerm <= 0.0f)
			m_fFlashTerm = 1.0f;

		float fProgress = (float)(dwServerTime - m_dwStartFlashTime) / m_fFlashTerm;
		auto pFadePanel = m_pFadePanel;

		if (fProgress < 0.1f)
		{
			float fSin = sinf((float)(fProgress * 5.0f) * 3.1415927f);

			unsigned int dwAlpha = (unsigned int)(float)(fabsf(fSin) * 0.0f);
			pFadePanel->m_GCPanel.dwColor = dwAlpha | (dwAlpha << 8) | (dwAlpha << 16) | (dwAlpha << 24);
		}
		else if (fProgress < 0.90f)
			pFadePanel->m_GCPanel.dwColor = 0xDD000000;
		else if (fProgress < 1.0f)
		{
			float fSin = sinf((((fProgress - 0.90f) * 5.0f) + 0.5f) * 3.1415927f);

			unsigned int dwAlpha = (unsigned int)(float)(fabsf(fSin) * 0.0f);
			pFadePanel->m_GCPanel.dwColor = dwAlpha | (dwAlpha << 8) | (dwAlpha << 16) | (dwAlpha << 24);
		}
		else
		{
			pFadePanel->m_GCPanel.dwColor = 0;
			m_dwStartFlashTime = 0;
		}

		m_pTargetHuman = 0;
	}

	auto pSoundManager = g_pSoundManager;
	if (m_pGround->m_bDungeon == 0|| m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4)
	{
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(341);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}
	}
	else
	{
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(107);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}

		if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 18 && (int)m_pMyHuman->m_vecPosition.y >> 7 == 30)
		{
			if (pSoundManager)
			{
				auto pSoundData = pSoundManager->GetSoundData(341);
				if (pSoundData && !pSoundData->IsSoundPlaying())
					pSoundData->Play();
			}
		}
		else if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(341);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}
	}
	if (g_nWeather == 2 || g_nWeather == 3)
	{
		if ((m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4) && m_pSnow->m_bVisible == 1)
		{
			m_pSnow->m_bVisible = 0;
			m_pSnow2->m_bVisible = 0;
			m_pRain->m_bVisible = 1;
		}
		else if (!m_pGround->m_bDungeon && !m_pSnow->m_bVisible)
		{
			m_pSnow->m_bVisible = 1;
			if (g_nWeather == 3)
				m_pSnow2->m_bVisible = 1;
			m_pRain->m_bVisible = 0;
		}
	}
	if (!g_nWeather && (!m_pGround->m_bDungeon || m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4))
	{
		int nTime = (dwServerTime / 600000) % 12;
		if (m_nWTime)
			nTime = m_nWTime;

		if ((m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4) && m_pSnow->m_bVisible == 1)
		{
			m_pSnow->m_bVisible = 0;
			m_pSnow2->m_bVisible = 0;
			m_pRain->m_bVisible = 1;
		}
		if (nTime <= 7)
		{
			if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
				(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
			{
				if (m_pSky->m_nState != 1 && m_pSky->m_nState != 11)
				{
					m_pSky->SetWeatherState(11);
					m_pRain->m_bVisible = 0;
					m_pSnow->m_bVisible = 1;
					m_pSnow2->m_bVisible = 0;
				}
			}
			else if (m_pSky->m_nState && m_pSky->m_nState != 10)
			{
				m_pSky->SetWeatherState(10);
				m_pRain->m_bVisible = 0;
				m_pSnow->m_bVisible = 0;
				m_pSnow2->m_bVisible = 0;
			}
		}
		else if (nTime >= 8 && nTime <= 9)
		{
			if (m_pSky->m_nState != 2 && m_pSky->m_nState != 12)
			{
				m_pSky->SetWeatherState(12);
				m_pRain->m_bVisible = 0;
				m_pSnow->m_bVisible = 0;
				m_pSnow2->m_bVisible = 0;
			}
		}
		else if (nTime >= 10 && nTime <= 11)
		{
			if (m_pSky->m_nState != 3 && m_pSky->m_nState != 13)
			{
				m_pSky->SetWeatherState(13);
				m_pRain->m_bVisible = 0;
				m_pSnow->m_bVisible = 0;
				m_pSnow2->m_bVisible = 0;
			}
		}
		else if (nTime == 12)
		{
			if (m_pSky->m_nState != 1)
			{
				g_nWeather = 12;
				m_pSky->SetWeatherState(11);
			}
			if (!m_pRain->m_bVisible)
				m_pRain->m_bVisible = 1;
			if (m_pSnow->m_bVisible == 1)
				m_pSnow->m_bVisible = 0;
			if (m_pSnow2->m_bVisible == 1)
				m_pSnow2->m_bVisible = 0;
		}
		else if (nTime == 13)
		{
			if (m_pSky->m_nState != 1)
			{
				g_nWeather = 13;
				m_pSky->SetWeatherState(11);
			}
			if (m_pRain->m_bVisible == 1)
				m_pRain->m_bVisible = 0;
			if (!m_pSnow->m_bVisible)
				m_pSnow->m_bVisible = 1;
			if (m_pSnow2->m_bVisible == 1)
				m_pSnow2->m_bVisible = 0;
		}
		else if (nTime == 14)
		{
			if (m_pSky->m_nState != 1)
			{
				g_nWeather = 14;
				m_pSky->SetWeatherState(11);
			}
			if (m_pRain->m_bVisible == 1)
				m_pRain->m_bVisible = 0;
			if (!m_pSnow->m_bVisible)
				m_pSnow->m_bVisible = 1;
			if (!m_pSnow2->m_bVisible)
				m_pSnow2->m_bVisible = 1;
		}
		if (dwServerTime - m_dwWeatherTime > 1000)
		{
			if (!m_pGround->m_bDungeon)
			{
				int nRand = rand() % 7;
				int nTexture = 95;

				if (((int)m_pMyHuman->m_vecPosition.x >> 7 == 29 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 30) && (int)m_pMyHuman->m_vecPosition.y >> 7 == 22)
				{
					if (pSoundManager)
					{
						auto pSoundData = pSoundManager->GetSoundData(114);
						if (pSoundData && !pSoundData->IsSoundPlaying())
							pSoundData->Play();
					}

					nTexture = 2;
				}
				else if (pSoundManager)
				{
					auto pSoundData = pSoundManager->GetSoundData(114);
					if (pSoundData && pSoundData->IsSoundPlaying())
						pSoundData->Stop();
				}

				if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 28	&& (int)m_pMyHuman->m_vecPosition.y >> 7 == 24 ||
					(int)m_pMyHuman->m_vecPosition.x >> 7 == 19	&& (int)m_pMyHuman->m_vecPosition.y >> 7 == 12)
				{
					int nSoundRand = rand() % 7;
					if (nSoundRand >= 0 && nSoundRand <= 2)
					{
						if (pSoundManager)
						{
							auto pSoundData = pSoundManager->GetSoundData(367 + nSoundRand);
							if (pSoundData && !pSoundData->IsSoundPlaying())
								pSoundData->Play();
						}
					}
				}

				if (!g_bHideEffect)
				{
					auto pPappus = new TMEffectBillBoard(nTexture, 20000, 0.2f, 0.2f, 0.2f, 0.0f, 1, 80);
					if (pPappus)
					{
						pPappus->m_vecPosition = TMVector3((float)(m_pMyHuman->m_vecPosition.x + 2.0f) - (float)nRand,
							((float)nRand * 0.2f) + m_pMyHuman->m_fHeight,
							(float)(m_pMyHuman->m_vecPosition.y + 2.0f) - (float)nRand);

						pPappus->m_vecPosition.y += 1.0f;
						pPappus->m_vecStartPos = pPappus->m_vecPosition;
						pPappus->m_fCircleSpeed = ((float)nRand * 0.1f) + 1.0f;
						nRand = rand() % 7;
						pPappus->m_fParticleH = ((float)nRand * 0.5f) + 7.0f;
						nRand = rand() % 7;
						pPappus->m_fParticleV = ((float)nRand * 0.1f) + 1.0f;
						pPappus->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pPappus->m_nParticleType = nRand % 3 + 3;
						pPappus->SetColor(0xAAAAAAAA);
						if (m_pEffectContainer)
							m_pEffectContainer->AddChild(pPappus);
					}
				}
			}
			if (m_pSky->m_nState == 3 || m_pSky->m_nState == 13)
			{
				if (pSoundManager)
				{
					auto pSoundData = pSoundManager->GetSoundData(107);
					if (pSoundData && !pSoundData->IsSoundPlaying())
						pSoundData->Play();
				}

				if (((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
					 (int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25) == 0 &&
					!g_bHideEffect)
				{
					int nRand = rand() % 7;
					auto pGlow = new TMEffectBillBoard(56, 20000, 0.2f, 0.2f, 0.2f, 0.0f, 1, 80);
					if (pGlow)
					{
						pGlow->m_vecPosition = TMVector3((float)(m_pMyHuman->m_vecPosition.x + 2.0f) - (float)nRand,
							(float)(m_pMyHuman->m_fHeight + 1.5f) + ((float)nRand * 0.2f),
							(float)(m_pMyHuman->m_vecPosition.y + 2.0f) - (float)nRand);

						pGlow->m_vecStartPos = pGlow->m_vecPosition;
						pGlow->m_fCircleSpeed = (float)((float)nRand * 0.1f) + 1.5f;
						pGlow->m_fParticleH = (float)((float)nRand * 0.5f) + 5.0f;
						pGlow->m_fParticleV = (float)((float)nRand * 0.05f) + 0.2f;
						pGlow->m_nParticleType = nRand % 3 + 6;
						pGlow->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pGlow->SetColor(0x0FFFFFF00);
						if (m_pEffectContainer)
							m_pEffectContainer->AddChild(pGlow);
					}

					pGlow = new TMEffectBillBoard(60, 20000, 0.07f, 0.07f, 0.07f, 0.0f, 1, 80);
					if (pGlow)
					{
						pGlow->m_vecPosition = TMVector3((float)(m_pMyHuman->m_vecPosition.x + 2.0f) - (float)nRand,
							(float)(m_pMyHuman->m_fHeight + 1.5f) + ((float)nRand * 0.2f),
							(float)(m_pMyHuman->m_vecPosition.y + 2.0f) - (float)nRand);

						pGlow->m_vecStartPos = pGlow->m_vecPosition;
						pGlow->m_fCircleSpeed = ((float)nRand * 0.1f) + 1.5f;
						pGlow->m_fParticleH = ((float)nRand * 0.5f) + 5.0f;
						pGlow->m_fParticleV = ((float)nRand * 0.05f) + 0.2f;
						pGlow->m_nParticleType = nRand % 3 + 6;
						pGlow->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pGlow->SetColor(0x0FFFFFF00);
						if (m_pEffectContainer)
							m_pEffectContainer->AddChild(pGlow);
					}
				}
				if ((RenderDevice::m_bDungeon == 3 || RenderDevice::m_bDungeon == 4) && dwServerTime - m_dwLastWolfSound > 25000)
				{
					if (pSoundManager)
					{
						auto pSoundData = pSoundManager->GetSoundData(111);
						if (pSoundData && !pSoundData->IsSoundPlaying())
							pSoundData->Play();
					}

					m_dwLastWolfSound = dwServerTime;
				}
			}
			else if (pSoundManager)
			{
				auto pSoundData = pSoundManager->GetSoundData(107);
				if (pSoundData && !pSoundData->IsSoundPlaying())
					pSoundData->Stop();
			}

			if (RenderDevice::m_bDungeon == 3)
			{
				if (dwServerTime - m_dwLastDustTime > m_dwDustTerm && m_bSandWind == 1 && g_bCastleWar == 2 && !m_pSky->m_nState)
				{
					if (pSoundManager)
					{
						auto pSoundData = pSoundManager->GetSoundData(304);
						if (pSoundData && pSoundData->IsSoundPlaying())
							pSoundData->Play();
					}
					if (!g_bHideEffect)
					{
						for (int ii = 0; ii < 5; ++ii)
						{
							float fWidth = (float)(50 * (rand() % 5)) + 300.0f;
							auto pCamera = g_pObjectManager->m_pCamera;

							auto pBillEffect = new TMEffectBillBoard4(193, 800 * ii + 3000, fWidth, fWidth, 1.0f, 1, 80);
							if (pBillEffect)
							{
								pBillEffect->SetPosition(100 * ii - 900, 100 * ii);
								pBillEffect->SetParticle(1600.0, 0.0);
								m_pEffectContainer->AddChild(pBillEffect);
							}
						}
					}
					++m_nDustCount;
					m_nDustCount %= 10;
					if (m_nDustCount)
						m_dwDustTerm = 1000;
					else
						m_dwDustTerm = 3000 * (rand() % 10 + 5);

					m_dwLastDustTime = dwServerTime;
				}
				if (dwServerTime - m_dwLastPappusTime > 2000)
				{
					for (int jj = 0; jj < 8; ++jj)
					{
						int nRand = rand() % 5;
						int nRand2 = rand() % 5;

						TMVector3 vecStart = TMVector3(
							((float)(m_pMyHuman->m_vecPosition.x - 16.0f) + (float)jj) + (float)(4 * nRand),
							m_pMyHuman->m_fHeight + 8.0f,
							((float)(m_pMyHuman->m_vecPosition.y - 12.0f) + (float)jj) + (float)(2 * nRand2));

						TMVector3 vecEnd = TMVector3(
							(float)((float)(m_pMyHuman->m_vecPosition.x + 4.0f) + (float)jj) + (float)(2 * nRand),
							m_pMyHuman->m_fHeight + 0.5f,
							(float)((float)(m_pMyHuman->m_vecPosition.y + 4.0f) + (float)jj) + (float)nRand2
						);

						auto pStorm = new TMSkillMagicArrow(vecStart, vecEnd, 3, 0);
						if (pStorm)
							m_pEffectContainer->AddChild(pStorm);
					}
					m_dwLastPappusTime = dwServerTime;
				}
			}
			m_dwWeatherTime = g_pTimerManager->GetServerTime();
		}
	}
	if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
	{
		int bValue = BASE_GetAttr((int)m_pMyHuman->m_vecPosition.x,	(int)m_pMyHuman->m_vecPosition.y);
		if (!(bValue & 8) && dwServerTime - m_dwLastDustTime > m_dwDustTerm	&& m_bSandWind == 1
			&& !g_bCastleWar2 && m_pSky->m_nState == 1)
		{
			if (pSoundManager)
			{
				auto pSoundData = pSoundManager->GetSoundData(304);
				if (pSoundData && pSoundData->IsSoundPlaying())
					pSoundData->Play();
			}

			for (int kk = 0; kk < 5; ++kk)
			{
				float fScaleX = (float)(50 * (rand() % 5)) + 300.0f;
				auto pCamera = g_pObjectManager->m_pCamera;

				auto pBillEffect = new TMEffectBillBoard4(422, 800 * kk + 3000, fScaleX, fScaleX, 1.0f, 1, 80);
				if (pBillEffect)
				{
					pBillEffect->SetPosition(100 * kk - 900, 100 * kk);
					pBillEffect->SetParticle(1600.0, 0.0);
					m_pEffectContainer->AddChild(pBillEffect);
				}
			}

			++m_nDustCount;
			m_nDustCount %= 10;
			if (m_nDustCount)
				m_dwDustTerm = 1000;
			else
				m_dwDustTerm = 3000 * (rand() % 10 + 5);
			m_dwLastDustTime = dwServerTime;
		}
	}

	UpdateSkillCooldownUI(dwServerTime);
	if (dwServerTime - m_dwChatTime > 10000)
	{
		char szMsg[128]{};
		sprintf(szMsg, "");
		auto pItem = new SListBoxItem(szMsg, 0xFFAAFFAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (pItem)
		{
			m_pChatList->AddItem(pItem);
			m_dwChatTime = dwServerTime;
		}
	}
	if (m_pRankTimeText && m_bRankTimeOn)
	{
		// 0x3A1 is the authoritative instance-timer signal.  The old client
		// additionally gated this HUD on a fixed list of map offsets, which
		// excluded newer/alternate instance rooms and could dereference the
		// autotrade panel while changing scenes.  Once the server sends a timer,
		// keep it visible until the timer expires or a zero packet clears it.
		unsigned int dwElapsedRankTime = dwServerTime - m_dwStartRankTime;
		int nRemainTime = m_nLastTime - static_cast<int>(dwElapsedRankTime / 1000);
		if (nRemainTime > 0)
		{
			char szTime[128]{};
			sprintf(szTime, "%02d : %02d", nRemainTime / 60, nRemainTime % 60);
			m_pRankTimeText->SetText(szTime, 0);
			m_pRankTimeText->SetVisible(1);
		}
		else
		{
			m_pRankTimeText->SetVisible(0);
			m_bRankTimeOn = 0;
		}
	}
	if (m_cAutoAttack == 1 && m_pTargetHuman && !m_pTargetHuman->m_cDie && dwServerTime - m_dwLastAutoAttackTime > 200)
	{
		if (m_pMyHuman->m_usGuild != m_pTargetHuman->m_usGuild && g_pObjectManager->m_usAllyGuild != m_pTargetHuman->m_usGuild ||
			!m_pMyHuman->m_usGuild || !m_pTargetHuman->m_usGuild)
		{
			m_pMyHuman->MoveAttack(m_pTargetHuman);
			if (m_pTargetHuman->m_nClass == 66 && m_pTargetHuman->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
				m_pTargetHuman = 0;

			m_dwLastAutoAttackTime = dwServerTime;
		}
	}
	else if (m_cAutoAttack == 1 && m_pTargetHuman && m_pTargetHuman->m_cDie == 1)
		m_pTargetHuman = 0;

	FindAuto();

	for (int i = 0; i < m_pPartyList->m_nNumItem; ++i)
	{
		auto pPartyItem = (SListBoxPartyItem*)m_pPartyList->m_pItemList[i];
		auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
		if (pHuman)
		{
			if (pHuman->m_cCancel == 1)
			{
				pPartyItem->m_GCText.dwColor = 0xFFFF0000;
				pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString,	pPartyItem->m_GCText.dwColor, 0);
			}
			else if (pPartyItem->m_nState == 2)
			{
				pPartyItem->m_GCText.dwColor = 0xFFAAAAFF;
				pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
			}
			else if (!pPartyItem->m_nState)
			{
				pPartyItem->m_GCText.dwColor = 0xFFFFFFFF;
				pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
			}
		}
	}

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
			if (g_bActiveWB == 1)
				g_pApp->SwitchWebBrowserState(0);
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
			if (g_bActiveWB == 1)
				g_pApp->SwitchWebBrowserState(0);
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
			if (g_bActiveWB == 1)
				g_pApp->SwitchWebBrowserState(0);
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
		const int carryPages = m_bCompatFieldScene ? 1 : 4;
		for (int i = 0; i < carryPages; ++i)
		{
			SGridControl* pMyGrid = m_bCompatFieldScene ? m_pGridInv : m_pGridInvList[i];

			IVector2 vecGrid = pMyGrid->CanAddItemInEmpty(GridX, GridY);

			if (vecGrid.x > -1 && vecGrid.y > -1)
			{
				MSG_HellBuy stHellBuy{};

				stHellBuy.Header.ID = m_pMyHuman->m_dwID;
				stHellBuy.Header.Type = 701;
				stHellBuy.TargetID = m_pMessageBox->m_dwArg;
				stHellBuy.TargetCarryPos = static_cast<unsigned char>(Value);
				stHellBuy.MyCarryPos = m_bCompatFieldScene
					? vecGrid.x + 9 * vecGrid.y
					: 15 * i + vecGrid.x + 5 * vecGrid.y;

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
	case 81923:
		m_ItemMixClass.DoCombine(m_pMessagePanel, m_pGridInvList, m_Coin);
		break;
	case 86019:
		// Mission is a preserved later UI extension, but it has no client/server
		// contract yet. In particular, it must never alias native Tiny opcode 0x3C0.
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
			+ (m_bCompatFieldScene ? 9 : 5) * pSellItem->m_nCellIndexY;

		int DestPage = m_bCompatFieldScene
			? 0
			: 15 * (pSellItem->m_pGridControl->m_dwControlID - 67072);
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
				sDestPos = m_bCompatFieldScene && sDestType == 1
					? GetCarrySlotForCell(pSellItem->m_pGridControl,
						pSellItem->m_nCellIndexX, pSellItem->m_nCellIndexY)
					: pSellItem->m_nCellIndexX + 5 * pSellItem->m_nCellIndexY;
			}

			int DestPage = m_bCompatFieldScene
				? 0
				: 15 * (pSellItem->m_pGridControl->m_dwControlID - 67072);
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
			g_pObjectManager->m_stMobData.CurrentScore.Mastery[nSkill / 8 + 1]) <= g_pObjectManager->m_stMobData.CurrentScore.CurMP)
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
