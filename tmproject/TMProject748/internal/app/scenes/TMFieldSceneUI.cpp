#include "pch.h"
#include "TMFieldScene.h"
#include "FieldSceneWorldSupport.h"
#include "ItemEffect.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "TMFont3.h"
#include "TMSkinMesh.h"
#include "WYD748Assets.h"
#include "../../ui/ObservedAffectProjection.h"
#include "../../ui/ResourceBarProjection.h"
#include "TMLog.h"
#include "ClientDiagnostics.h"
#include "TMGround.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"
#include "TMCamera.h"
#include "TMSun.h"
#include "TMSky.h"
#include "TMSnow.h"
#include "TMRain.h"

namespace
{
	class ObservedAffectPanel final : public SPanel
	{
	public:
		ObservedAffectPanel() : SPanel(200, 0, 0, 16, 16, 0xFFFFFFFFu, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH) {}
		int OnMouseEvent(unsigned int, unsigned int, int, int) override { return 0; }
	};
}

void TMFieldScene::UpdateCompatScoreUI()
{
	// FieldScene2.bin 7.48 stores the compact HUD under the original low IDs;
	// assigning those controls here keeps STRUCT_MOB/STRUCT_SCORE byte-exact
	// while letting the newer source render the authoritative server values.
	if (!m_pControlContainer)
		return;

	auto pMobData = &g_pObjectManager->m_stMobData;
	m_pHPBar = static_cast<SProgressBar*>(m_pControlContainer->FindControl(TMP_HP_PROGRESS));
	m_pMPBar = static_cast<SProgressBar*>(m_pControlContainer->FindControl(TMP_MP_PROGRESS));
	m_pCHP = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CURRENT_HP));
	m_pCMP = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CURRENT_MP));
	// 0x181 shares these resource-owned texts with the initial score projector.
	m_pCurrentHPText = m_pCHP;
	m_pCurrentMPText = m_pCMP;
	m_pMaxHPText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_MAX_HP));
	m_pMaxMPText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_MAX_MP));
	m_pMainCharName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_MAIN_INFO2_NAME));
	m_pMainInfo2_Name = m_pMainCharName;
	m_pMainInfo2_Lv = static_cast<SText*>(m_pControlContainer->FindControl(TMT_MAIN_INFO2_LEVEL));
	// FUN_00435b13 binds the six native compact-HUD texts at 1029..1040.
	// IDs 5718..5720 belong to the imported 7.59 graph and overwrite unrelated
	// children in FieldScene2.bin, which is why ATT/DEF appeared displaced.
	m_pDamage = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATT));
	m_pSkillDam = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATT_ENC));
	m_pDefence = static_cast<SText*>(m_pControlContainer->FindControl(TMT_DEF));
	m_pMoney1 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_MONEY_TEXT));
	// FieldScene2.bin 7.48 uses the native low ID for the skill-point value.
	// 65600 belongs to the later graph and is absent here, leaving the label
	// visible but its value blank.
	m_pSkBonus = static_cast<SText*>(m_pControlContainer->FindControl(TMT_SKILL_BONUS));

	if (pMobData->CurrentScore.CurHP > pMobData->CurrentScore.MaxHP)
		pMobData->CurrentScore.CurHP = pMobData->CurrentScore.MaxHP;
	if (pMobData->CurrentScore.CurMP > pMobData->CurrentScore.MaxMP)
		pMobData->CurrentScore.CurMP = pMobData->CurrentScore.MaxMP;

	resource_ui::ProjectNativeHpVisual(m_pHPBar,
		pMobData->CurrentScore.CurHP, pMobData->CurrentScore.MaxHP);
	resource_ui::ProjectNativeHpVisual(
		static_cast<SProgressBar*>(m_pControlContainer->FindControl(TMP_HP_PROGRESS_TR)),
		pMobData->CurrentScore.CurHP, pMobData->CurrentScore.MaxHP);
	if (m_pMPBar)
	{
		m_pMPBar->SetMaxProgress(pMobData->CurrentScore.MaxMP);
		m_pMPBar->SetCurrentProgress(pMobData->CurrentScore.CurMP);
	}

	auto setNumber = [this](unsigned int controlID, long long value)
	{
		if (auto pText = static_cast<SText*>(m_pControlContainer->FindControl(controlID)))
		{
			char text[64]{};
			sprintf_s(text, "%I64d", value);
			pText->SetText(text, 0);
		}
	};

	setNumber(TMT_CURRENT_HP, pMobData->CurrentScore.CurHP);
	setNumber(TMT_MAX_HP, pMobData->CurrentScore.MaxHP);
	setNumber(TMT_CURRENT_MP, pMobData->CurrentScore.CurMP);
	setNumber(TMT_MAX_MP, pMobData->CurrentScore.MaxMP);
	if (m_pSkBonus)
	{
		char skillPoints[32]{};
		sprintf_s(skillPoints, "%u", pMobData->CurrentScore.SkillPts);
		m_pSkBonus->SetText(skillPoints, 0);
	}

	// FUN_004431e4 writes paired values into the resource-owned ATT and DEF
	// cells.  Keep that split intact instead of duplicating one value into the
	// later 7.59 labels: current attack/defense are the primary values, while
	// selected-skill damage and native base/ability defense are the companions.
	int selectedSkillDamage = 0;
	const int selectedShortSlot = g_pObjectManager->m_cSelectShortSkill;
	if (m_pMyHuman && selectedShortSlot >= 0 && selectedShortSlot < 20)
	{
		const int selectedSkill = g_pObjectManager->m_cShortSkill[selectedShortSlot];
		if (selectedSkill >= 0 && selectedSkill < 248)
		{
			SetMyHumanMagic();
			int weather = g_nWeather == 3 ? 2 : g_nWeather;
			const int mapX = static_cast<int>(m_pMyHuman->m_vecPosition.x) >> 7;
			const int mapY = static_cast<int>(m_pMyHuman->m_vecPosition.y) >> 7;
			if (mapX > 26 && mapX < 31 && mapY > 20 && mapY < 25)
				weather = 2;
			selectedSkillDamage = BASE_GetSkillDamage(selectedSkill, pMobData,
				weather, GetWeaponDamage(), pMobData->Equip[0].sIndex);
			if (selectedSkillDamage < 0)
				selectedSkillDamage = -selectedSkillDamage;
		}
	}

	const int nativeDefense = BASE_GetMobAbility(pMobData, 53)
		+ BASE_GetMobAbility(pMobData, 3) + pMobData->BaseScore.Defense;
	setNumber(TMT_ATT, pMobData->CurrentScore.Attack);
	setNumber(TMT_ATT_ENC, selectedSkillDamage);
	setNumber(TMT_DEF, pMobData->CurrentScore.Defense);
	setNumber(TMT_DEF_ENC, nativeDefense);

	// The EXP band is two absolute counters in 7.48 (current and next-level
	// threshold).  FUN_004431e4 updates 1031 and 1032 independently; leaving the
	// second control empty made the range look broken even when Exp was valid.
	const bool secondClass = m_pMyHuman && m_pMyHuman->Is2stClass() == 2;
	const long long* levelTable = secondClass ? g_pNextLevel_G2 : g_pNextLevel;
	const int levelTableCount = secondClass ? _countof(g_pNextLevel_G2) : _countof(g_pNextLevel);
	int currentLevel = pMobData->CurrentScore.Level;
	if (currentLevel < 0)
		currentLevel = 0;
	if (currentLevel >= levelTableCount - 1)
		currentLevel = levelTableCount - 2;
	setNumber(TMT_EXP, pMobData->Exp);
	setNumber(TMT_EXP_ENC, levelTable[currentLevel + 1]);

	// FUN_004431e4 divides the active 7.48 level interval into four quarters:
	// panels 1172..1174 mark completed quarters and progress 1171 renders only
	// the current quarter. Reproducing that contract restores the native EXP
	// strip instead of leaving a full-width but permanently empty decoration.
	const long long levelStart = levelTable[currentLevel];
	const long long levelEnd = levelTable[currentLevel + 1];
	long long levelProgress = static_cast<long long>(pMobData->Exp) - levelStart;
	const long long levelSpan = max(levelEnd - levelStart, 1LL);
	if (levelProgress < 0)
		levelProgress = 0;
	if (levelProgress > levelSpan)
		levelProgress = levelSpan;
	const long long quarterSpan = max(levelSpan >> 2, 1LL);
	int activeQuarter = 4;
	if (levelProgress < quarterSpan)
		activeQuarter = 1;
	else if (levelProgress < (quarterSpan << 1))
		activeQuarter = 2;
	else if (levelProgress < quarterSpan * 3)
		activeQuarter = 3;
	for (int lamp = 0; lamp < 3; ++lamp)
	{
		if (auto pQuarterLamp = static_cast<SPanel*>(
			m_pControlContainer->FindControl(1172 + lamp)))
		{
			pQuarterLamp->m_GCPanel.nTextureIndex = lamp < activeQuarter - 1 ? 1 : 0;
		}
	}
	long long quarterProgress = levelProgress - (activeQuarter - 1) * quarterSpan;
	if (quarterProgress < 0)
		quarterProgress = 0;
	if (quarterProgress > quarterSpan)
		quarterProgress = quarterSpan;
	if (auto pNativeExpProgress = static_cast<SProgressBar*>(
		m_pControlContainer->FindControl(1171)))
	{
		const int nativeQuarterMax = static_cast<int>(
			min(quarterSpan, 0x7FFFFFFFLL));
		const int nativeQuarterProgress = static_cast<int>(
			min(quarterProgress, static_cast<long long>(nativeQuarterMax)));
		pNativeExpProgress->SetMaxProgress(nativeQuarterMax);
		pNativeExpProgress->SetCurrentProgress(nativeQuarterProgress);
	}
	setNumber(TMT_MONEY_C, pMobData->Coin);
	setNumber(TMT_MONEY, pMobData->Coin);
	setNumber(TMT_MONEY_TEXT, pMobData->Coin);

	// The compact 7.48 character window uses the original 10xx control IDs,
	// whereas the imported 7.59 initializer only binds its replacement 65xxx
	// controls.  Project the same authoritative score into the legacy fields.
	setNumber(TMT_CI_LEVEL, pMobData->CurrentScore.Level + 1);
	setNumber(TMT_CI_EXP, pMobData->Exp);
	setNumber(TMT_CI_EXP_E, pMobData->Exp);
	setNumber(TMT_CI_STR, pMobData->CurrentScore.Str);
	setNumber(TMT_CI_INT, pMobData->CurrentScore.Int);
	setNumber(TMT_CI_DEX, pMobData->CurrentScore.Dex);
	setNumber(TMT_CI_CON, pMobData->CurrentScore.Con);
	setNumber(TMT_CI_SCOREPOINT, g_pObjectManager->m_stMobData.CurrentScore.StatusPts);
	setNumber(TMT_CI_HP, pMobData->CurrentScore.CurHP);
	setNumber(TMT_CI_MP, pMobData->CurrentScore.CurMP);
	setNumber(TMT_CI_ATT, pMobData->CurrentScore.Attack);
	setNumber(TMT_CI_DFE, pMobData->CurrentScore.Defense);
	setNumber(TMT_CI_SPEED, pMobData->CurrentScore.AttackRun & 0x0F);
	setNumber(TMT_CI_HOLY, pMobData->CurrentScore.ResistHoly);
	setNumber(TMT_CI_THUNDER, pMobData->CurrentScore.ResistThunder);
	setNumber(TMT_CI_FIRE, pMobData->CurrentScore.ResistFire);
	setNumber(TMT_CI_ICE, pMobData->CurrentScore.ResistIce);
	setNumber(TMT_CI_SPECIALPOINT, g_pObjectManager->m_stMobData.CurrentScore.MasterPts);

	auto setCompatText = [this](unsigned int controlID, const char* value)
	{
		if (auto pText = static_cast<SText*>(m_pControlContainer->FindControl(controlID)))
		{
			// SText keeps the historical mutable signature although SetText only
			// copies the input; keep compatibility without weakening the callers.
			pText->SetText(const_cast<char*>(value ? value : ""), 0);
		}
	};
	setCompatText(TMT_CI_NAME, pMobData->MobName);
	static const unsigned int classMessage[4] = { 121, 122, 123, 124 };
	const int characterClass = pMobData->Class >= 0 && pMobData->Class < 4 ? pMobData->Class : 0;
	setCompatText(TMT_CI_CLASS, g_pMessageStringTable[classMessage[characterClass]]);
	if (pMobData->Equip[0].sIndex < 40 && pMobData->Equip[0].sIndex % 10 >= 6)
	{
		const int bodyClass = pMobData->Equip[0].sIndex / 10;
		setCompatText(TMT_CI_CLASS2,
			bodyClass >= 0 && bodyClass < 4 ? g_pMessageStringTable[classMessage[bodyClass]] : "");
	}
	else
		setCompatText(TMT_CI_CLASS2, "");

	// WYD.exe 7.48 FUN_004431e4 formats every mastery as current/native-max.
	// Deliberately omit the 7.59 skill IDs 200/204/205/208/233/238: they change
	// these caps but have no representation in the target client's skill ABI.
	int masteryMax[4]{};
	masteryMax[0] = 3 * (pMobData->CurrentScore.Level + 1) / 2;
	if (masteryMax[0] > 200 || (m_pMyHuman && m_pMyHuman->Is2stClass() == 2))
		masteryMax[0] = 200;
	masteryMax[1] = masteryMax[0];
	masteryMax[2] = masteryMax[0];
	masteryMax[3] = masteryMax[0];
	if (IsValidSkill(31) == 1)
		masteryMax[1] = 255;
	if (IsValidSkill(39) == 1)
		masteryMax[2] = 255;
	if (IsValidSkill(47) == 1)
		masteryMax[3] = 255;
	// The fourth legacy row is intentionally non-contiguous (1153), so keep
	// the resource IDs explicit instead of projecting the 7.59 stride onto it.
	const unsigned int masteryValueIDs[4] = {
		TMT_CI_SPECIAL1, TMT_CI_SPECIAL2, TMT_CI_SPECIAL3, TMT_CI_SPECIAL4
	};
	for (unsigned int mastery = 0; mastery < _countof(masteryMax); ++mastery)
	{
		char masteryText[32]{};
		sprintf_s(masteryText, "%3d/%3d", pMobData->CurrentScore.Mastery[mastery], masteryMax[mastery]);
		setCompatText(masteryValueIDs[mastery], masteryText);
	}

	// The four captions are class-specific in the native 7.48 score updater.
	// Binding the original 5776..5779 controls avoids leaving the Trans Knight
	// labels baked into FieldScene2.bin visible for every other base class.
	for (unsigned int mastery = 0; mastery < 4; ++mastery)
		setCompatText(TMT_CI_SPECIAL1_C + mastery,
			g_pMessageStringTable[242 + characterClass * 4 + mastery]);

	// Control 1376 is the native C.POINT value. FUN_004431e4 derives the shown
	// percentage from the reserved hold counter and the active class EXP table;
	// it is not the character's chaos-level byte despite the abbreviated label.
	const long long holdPointRange =
		(levelTable[currentLevel + 1] - levelTable[currentLevel]) / 10;
	const int holdPointPercent = holdPointRange > 0
		? static_cast<int>(
			(static_cast<double>(g_pObjectManager->m_nFakeExp) * 100.0)
			/ holdPointRange)
		: 0;
	char holdPointText[64]{};
	const char* holdPointFormat = g_pMessageStringTable[304]
		? g_pMessageStringTable[304] : "%12d / %d%%";
	sprintf_s(holdPointText, holdPointFormat,
		g_pObjectManager->m_nFakeExp, holdPointPercent);
	if (auto pHoldPoint = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_FAKEEXPPOINT)))
	{
		pHoldPoint->SetText(holdPointText, 0);
		pHoldPoint->SetTextColor(
			holdPointPercent < 80 ? 0xFFFFFFFF : 0xFFFF0000);
	}
	if (auto pExpHold = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_EXP_HOLD)))
		pExpHold->SetVisible(g_pObjectManager->m_nFakeExp > 0);

	// Native control 1377 is the kingdom mantle emblem. Keep it hidden for a
	// character without a valid mantle, but preserve the native helmet override
	// and citizen-mantle normalization for characters that do belong to a realm.
	if (m_pKingDomFlag)
	{
		if (!m_pMyHuman || !m_pMyHuman->m_pMantua || m_pMyHuman->m_pMantua->m_Look.Skin0 == 19)
			m_pKingDomFlag->SetVisible(0);
		else
		{
			m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pMyHuman->m_pMantua->m_Look.Skin0;
			if ((m_pMyHuman->m_sHelmIndex == 3503 || m_pMyHuman->m_sHelmIndex == 3504 ||
				m_pMyHuman->m_sHelmIndex == 3505 || m_pMyHuman->m_sHelmIndex == 3506) &&
				m_pMyHuman->m_pMantua->m_Look.Skin0 == 2)
			{
				m_pKingDomFlag->m_GCPanel.nTextureIndex = 33;
			}

			m_pKingDomFlag->m_GCPanel.nTextureIndex =
				m_pMyHuman->UnSetCitizenMantle(m_pKingDomFlag->m_GCPanel.nTextureIndex);
			m_pKingDomFlag->SetVisible(1);
		}
	}

	// WYD.exe 7.48 FUN_004431e4 includes the native class bonuses and freeze
	// penalty in this presentation value. The server still authorizes cadence;
	// this restores only the number shown by control 1110 (Att Speed).
	int attackSpeedClass = 0;
	if (m_pMyHuman && m_pMyHuman->m_nClass == 26 && m_pMyHuman->m_stLookInfo.FaceMesh == 0)
		attackSpeedClass = 20;
	else if (m_pMyHuman && m_pMyHuman->m_nClass == 26 && m_pMyHuman->m_stLookInfo.FaceMesh == 1)
	{
		if ((pMobData->LearnedSkill[0] & 0x80000) == 0x80000)
			attackSpeedClass = 20;
	}
	else if (m_pMyHuman && m_pMyHuman->m_nClass == 33)
	{
		attackSpeedClass = 20;
		if ((pMobData->LearnedSkill[0] & 0x200000) == 0x200000)
			attackSpeedClass = 40;
	}
	else if (m_pMyHuman && m_pMyHuman->m_nClass == 40)
		attackSpeedClass = 20;
	else if (m_pMyHuman && m_pMyHuman->m_nClass == 63)
		attackSpeedClass = 30;

	int attackSpeed = (pMobData->CurrentScore.Mastery[2] / 10 + 10)
		* (m_pMyHuman ? m_pMyHuman->m_cSpeedUp - m_pMyHuman->m_cSpeedDown : 0)
		+ pMobData->CurrentScore.Dex / 5 + attackSpeedClass
		+ BASE_GetMobAbility(pMobData, 26) + 100;
	if (m_pMyHuman && m_pMyHuman->m_cFreeze == 1)
		attackSpeed -= 30;
	char percent[32]{};
	sprintf_s(percent, "%d%%", attackSpeed);
	setCompatText(TMT_CI_DFBT, percent);
	sprintf_s(percent, "%d.%d%%", (pMobData->CurrentScore.Critical * 4) / 10, (pMobData->CurrentScore.Critical * 4) % 10);
	setCompatText(TMT_CI_CRITICAL, percent);

	if (m_pMainCharName && m_pMyHuman)
		m_pMainCharName->SetText(m_pMyHuman->m_szName, 0);
	if (m_pMainInfo2_Lv)
	{
		char level[32]{};
		sprintf_s(level, "%d", pMobData->CurrentScore.Level + 1);
		m_pMainInfo2_Lv->SetText(level, 0);
	}
	m_Coin = pMobData->Coin;
}

void TMFieldScene::UpdateCompatLearnedSkillUI()
{
	if (!m_bCompatFieldScene || !g_pObjectManager)
		return;

	auto pMobData = &g_pObjectManager->m_stMobData;
	const unsigned int learnedMask = pMobData->LearnedSkill[0];
	for (int i = 0; i < 24; ++i)
	{
		auto pGrid = m_pSkillSecGrid[i];
		const unsigned int controlID = TMG_SKILL_SEC1_1 + i;
		const int itemIndex = 5000 + 24 * pMobData->Class + i;
		if (!pGrid)
		{
			WYD748_DiagnosticsLog(
				"compat learned skill mask=0x%08X bit=%d control=%u item=%d result=missing-grid\r\n",
				learnedMask, i, controlID, itemIndex);
			continue;
		}

		auto pCurrent = pGrid->GetItem(0, 0);
		if ((learnedMask & (1u << i)) == 0)
		{
			if (pCurrent)
			{
				pGrid->Empty();
				WYD748_DiagnosticsLog(
					"compat learned skill mask=0x%08X bit=%d control=%u item=%d result=cleared\r\n",
					learnedMask, i, controlID, itemIndex);
			}
			continue;
		}

		if (pCurrent && pCurrent->GetItem() && pCurrent->GetItem()->sIndex == itemIndex)
			continue;

		pGrid->Empty();
		auto pStructItem = new STRUCT_ITEM;
		if (!pStructItem)
		{
			WYD748_DiagnosticsLog(
				"compat learned skill mask=0x%08X bit=%d control=%u item=%d result=item-allocation-failed\r\n",
				learnedMask, i, controlID, itemIndex);
			continue;
		}
		memset(pStructItem, 0, sizeof(STRUCT_ITEM));
		pStructItem->sIndex = itemIndex;

		auto pControlItem = new SGridControlItem(nullptr, pStructItem, 0.0f, 0.0f);
		if (!pControlItem)
		{
			SAFE_DELETE(pStructItem);
			WYD748_DiagnosticsLog(
				"compat learned skill mask=0x%08X bit=%d control=%u item=%d result=control-allocation-failed\r\n",
				learnedMask, i, controlID, itemIndex);
			continue;
		}
		// The 7.48 config selects the classic UI (UIVer=1), whose generic item
		// constructor points learned skills at the legacy Amulet atlas (set 1).
		// FieldScene2's native learned-skill surface uses the color atlas instead;
		// keep this override local so inventory and equipment retain their native
		// classic presentation.
		pControlItem->m_GCObj.nTextureSetIndex = 199;

		const IVector2 inserted = pGrid->AddItemInEmpty(pControlItem);
		WYD748_DiagnosticsLog(
			"compat learned skill mask=0x%08X bit=%d control=%u item=%d result=%d,%d\r\n",
			learnedMask, i, controlID, itemIndex, inserted.x, inserted.y);
		if (inserted.x < 0)
			SAFE_DELETE(pControlItem);
	}
}

void TMFieldScene::InitializeRuntimeCounterTexts()
{
	if (!m_pControlContainer || !g_pDevice)
		return;

	m_pRankTimeText = new SText(-2, "00 : 00", 0xFF00FF00,
		(((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) / 2.0f) - 42.0f,
		30.0f, 200.0f, 16.0f, 0, 0x77777777, 1, 0);
	if (m_pRankTimeText)
	{
		m_pRankTimeText->m_Font.m_fSize = 2.0f;
		m_pRankTimeText->SetVisible(0);
		m_pRankTimeText->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - 50.0f,
			(3.0f * RenderDevice::m_fHeightRatio) + 55.0f);
		m_pControlContainer->AddItem(m_pRankTimeText);
	}
	m_bRankTimeOn = 0;
	m_bInstanceRemainOn = 0;

	char szTempLeft[128]{};
	// The English 7.48 strdef keeps the native Korean entry 230 at a
	// different index.  Using it here renders the counter as "12345 100"
	// instead of a mob label.  Keep this compatibility text local to the
	// instance-mob path rather than changing the shared string table ABI.
	sprintf(szTempLeft, "Monsters 100");
	m_pRemainText = new SText(-2, szTempLeft, 0xFFFFAA00,
		((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) - 210.0f,
		30.0f, 100.0f, 16.0f, 0, 0x77777777, 1, 0);
	if (m_pRemainText)
	{
		m_pRemainText->m_Font.m_fSize = 2.0f;
		m_pRemainText->SetVisible(0);
		m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 200.0f,
			30.0f * RenderDevice::m_fHeightRatio);
		m_pControlContainer->AddItem(m_pRemainText);
	}

	m_pQuestRemainTime = new SText(-2, "", 0xDDFFFF33,
		((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) - 140.0f,
		5.0f * RenderDevice::m_fHeightRatio, 100.0f, 16.0f,
		0, 0x77777777, 1, 0);
	if (m_pQuestRemainTime)
	{
		m_pQuestRemainTime->m_Font.m_fSize = 1.0f;
		m_pQuestRemainTime->SetVisible(1);
		if (g_pDevice->m_dwScreenWidth < 800)
			m_pQuestRemainTime->SetPos((float)g_pDevice->m_dwScreenWidth + 35.0f,
				5.0f * RenderDevice::m_fHeightRatio);
		else
			m_pQuestRemainTime->SetPos(276.0f * RenderDevice::m_fWidthRatio,
				525.0f * RenderDevice::m_fHeightRatio);
		m_pControlContainer->AddItem(m_pQuestRemainTime);
	}
}

int TMFieldScene::InitializeCompatFieldScene()
{
	// This compatibility initializer deliberately contains only the objects
	// required to render the 7.48 world.  The imported source's full HUD setup
	// dereferences controls that do not exist in the original FieldScene2.bin;
	// entering the world must not depend on those optional controls.
	m_bCompatFieldScene = true;
	// Record the exact legacy control tree before changing visibility.  This
	// gives us the real 7.48 IDs and prevents future ports from guessing IDs
	// from the newer TMProject resource.
	WYD748_DumpControlTree(m_pControlContainer, "compat-field-before-hide");
	// FieldScene2.bin stores feature panels as visible resources, while the
	// native startup path hides them and opens each one only after a command or
	// server response.  Reapply that initial state in compatibility mode so the
	// character, shop, party and jackpot windows do not appear over the world.
	if (m_pControlContainer)
	{
		const unsigned int hiddenFeaturePanels[] =
		{
			258u,    // item-description popup
			259u,    // native Character auxiliary bar, hidden by FUN_00435b13
			289u,    // native minimap foreground
			290u,    // native minimap border/background
			292u,    // native 7.48 main-menu flyout (opened only by button 5744)
			320u,    // quest window
			332u,    // new-quest notification button
			257u,    // TMP_INV_PANEL: inventory
			513u,    // TMP_CHAR_PANEL: character information
			576u,    // player-to-player trade
			626u,    // gold amount prompt
			632u,    // system menu
			640u,    // party/guild/trade interaction menu
			646u,    // TMP_ATRADE_PANEL: auto-trade window
			669u,    // notices window
			819u,    // guild board and notice editor
			864u,    // helper, memo and message window
			875u,    // message-arrived notification button
			878u,    // summon-request notification button
			880u,    // quiz window
			1360u,   // composition window
			1793u,   // TMP_SHOP_PANEL: merchant shop
			1825u,   // TMP_CARGO_PANEL: warehouse/cargo
			1857u,   // legacy FieldScene2 party panel
			1889u,   // TMP_SKILLM_PANEL: skill mastery
			1905u,   // TMP_SKILL_PANEL: skill list
			2048u,   // TMP_LOTTO: jackpot/lottery
			5749u,   // inactive party/member status list
			6110u,   // advanced composition window
			6145u,   // advanced composition window
			6185u,   // special item store
			6400u,   // TMP_GAMBLE_PANEL: jackpot/gamble
			6432u,   // second-job weapon composition
			6481u,   // advanced composition window
			6512u,   // restore system
			8705u,   // emote selector
			8961u,   // TMP_TOTO: toto/game-time panel
			12288u,  // server/channel selector while already in world
			12544u   // coordinate teleport window
		};

		for (const unsigned int controlID : hiddenFeaturePanels)
		{
			if (auto pControl = m_pControlContainer->FindControl(controlID))
			{
				pControl->SetVisible(0);
				// Visibility alone removes the panel from hit testing.  Do not disable
				// selection permanently: native handlers later reopen the same 7.48
				// control and expect its buttons to work immediately.
				WYD748_DiagnosticsLog("compat hide control id=%u\r\n", controlID);
			}
			else
				WYD748_DiagnosticsLog("compat hide control missing id=%u\r\n", controlID);
		}
		// The native 7.48 handlers keep the teleport list bound even while its
		// panel is closed.  Generic button handling consults this pointer, so the
		// compact bootstrap must restore the binding rather than only hiding UI.
		m_pPotalPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(12544));
		m_pPotalList = static_cast<SListBox*>(m_pControlContainer->FindControl(12545));
		// FieldScene2.bin has title 12549 and three column labels 12550..12552.
		// The imported InitBoard uses a different resource's 12551/52/53/60.
		m_pPotalText = static_cast<SText*>(m_pControlContainer->FindControl(12549));
		m_pPotalText1 = static_cast<SText*>(m_pControlContainer->FindControl(12550));
		m_pPotalText2 = static_cast<SText*>(m_pControlContainer->FindControl(12551));
		m_pPotalText3 = static_cast<SText*>(m_pControlContainer->FindControl(12552));
		if (m_pPotalList)
			m_pPotalList->SetEventListener(m_pControlContainer);
		// Native input and mouse-over code consults these panels even while they are
		// closed. Ghidra FUN_00435b13 stores panel 626 as modal slot 3 and keeps its
		// dimming background 574 separately, so preserve that ownership contract.
		m_pAutoTrade = static_cast<SPanel*>(m_pControlContainer->FindControl(646));
		m_pInputGoldPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_INPUT_GOLD));
		m_pInputBG2 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_INPUT_BG2));
		m_pSystemPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(632));
		if (m_pSystemPanel)
		{
			// FieldScene2.bin stores the native System panel at (0,0), while
			// FUN_0044df53 presents it as a modal viewport-centred menu.  Reapply
			// that runtime placement after RC scaling so its hitbox follows the UI.
			m_pSystemPanel->SetPos(
				static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f - m_pSystemPanel->m_nWidth * 0.5f,
				static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f - m_pSystemPanel->m_nHeight * 0.5f);
			m_pSystemPanel->SetVisible(0);
		}
		// FUN_00435b13 keeps these native 7.48 panels in the scene even while
		// hidden.  ESC and close-button dispatch require the real resource IDs;
		// leaving their 7.59 member aliases null silently disables those paths.
		m_pMsgPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_MSG_PANEL));
		m_pHelpPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_HELP_PANEL));
		if (m_pHelpPanel)
		{
			// Native 7.48 FUN_0052d2c8 writes whisper notices beginning with '!'
			// directly into list 874.  The compatibility initializer must therefore
			// bind the complete resource-owned Help group before network packets can
			// arrive; leaving only panel 864 bound caused SListBox::AddItem to run
			// with a null this pointer while entering the world.
			m_pHelpText = static_cast<SText*>(m_pControlContainer->FindControl(865));
			m_pHelpButton[0] = static_cast<SButton*>(m_pControlContainer->FindControl(867));
			m_pHelpList[0] = static_cast<SListBox*>(m_pControlContainer->FindControl(868));
			m_pHelpButton[1] = static_cast<SButton*>(m_pControlContainer->FindControl(869));
			m_pHelpList[1] = static_cast<SListBox*>(m_pControlContainer->FindControl(870));
			m_pHelpButton[2] = static_cast<SButton*>(m_pControlContainer->FindControl(871));
			m_pHelpList[2] = static_cast<SListBox*>(m_pControlContainer->FindControl(872));
			m_pHelpButton[3] = static_cast<SButton*>(m_pControlContainer->FindControl(873));
			m_pHelpList[3] = static_cast<SListBox*>(m_pControlContainer->FindControl(874));
			m_pHelpMemo = static_cast<SButton*>(m_pControlContainer->FindControl(875));
			m_pHelpSummon = static_cast<SButton*>(m_pControlContainer->FindControl(878));
			m_pHelpInterface = static_cast<SPanel*>(m_pControlContainer->FindControl(6067));
			for (int i = 0; i < 3; ++i)
			{
				m_pHelpInterfacePanel[i] = static_cast<SPanel*>(m_pControlContainer->FindControl(6064 + i));
				m_pHelpInterfaceList[i] = static_cast<SListBox*>(m_pControlContainer->FindControl(6074 + i));
			}

			if (m_pHelpList[0])
				m_pHelpList[0]->SetVisible(0);
			if (m_pHelpList[1])
				m_pHelpList[1]->SetVisible(0);
			if (m_pHelpList[2])
				m_pHelpList[2]->SetVisible(0);
			if (m_pHelpList[3])
				m_pHelpList[3]->SetVisible(0);
			if (m_pHelpMemo)
				m_pHelpMemo->SetVisible(0);
			if (m_pHelpSummon)
				m_pHelpSummon->SetVisible(0);
			if (m_pHelpList[0])
				LoadMsgText(m_pHelpList[0], (char*)"UI\\interface.txt");
			if (m_pHelpList[1])
				LoadMsgText(m_pHelpList[1], (char*)"UI\\command.txt");
			if (m_pHelpList[2])
				LoadMsgText(m_pHelpList[2], (char*)"UI\\etc.txt");
			if (m_pHelpInterfaceList[0])
				LoadMsgText(m_pHelpInterfaceList[0], (char*)"UI\\interface1.txt");
			if (m_pHelpInterfaceList[1])
				LoadMsgText(m_pHelpInterfaceList[1], (char*)"UI\\interface2.txt");
			if (m_pHelpInterfaceList[2])
				LoadMsgText(m_pHelpInterfaceList[2], (char*)"UI\\interface3.txt");
			m_nCurrInterfacePanelIndex = 0;
			SelectHelpTab(0);
		}
		// FUN_00435b13 binds classic buttons 314/315/316 before the help,
		// quest and autorun handlers can be reached. The imported 6579x IDs are
		// from the newer UI and leave H/Esc dereferencing a null button in 7.48.
		m_pHelpBtn = static_cast<SButton*>(m_pControlContainer->FindControl(314));
		m_pQuestBtn = static_cast<SButton*>(m_pControlContainer->FindControl(315));
		m_pAutoRunBtn = static_cast<SButton*>(m_pControlContainer->FindControl(316));
		// Native FUN_00441823 owns the complete Quest group. Binding only button
		// 315 left the 7.48 window without tabs, list callbacks or close lifecycle.
		m_pQuestPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_QUEST_PANEL));
		m_pQuestQuitBtn = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_QUEST_QUIT));
		m_pQuestButton[0] = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_QUEST_BUTTON));
		m_pQuestButton[1] = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_QUEST_BUTTON2));
		m_pQuestButton[2] = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_QUEST_BUTTON3));
		m_pQuestButton[3] = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_QUEST_BUTTON4));
		m_pQuestList[0] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_LIST));
		m_pQuestList[1] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_LIST2));
		m_pQuestList[2] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_LIST3));
		m_pQuestList[3] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_LIST4));
		m_pQuestContentList[0] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_CONTENT));
		m_pQuestContentList[1] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_CONTENT2));
		m_pQuestContentList[2] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_CONTENT3));
		m_pQuestContentList[3] = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_QUEST_CONTENT4));
		m_pQuestMemo = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_QUEST_MEMO));
		for (int questIndex = 0; questIndex < 4; ++questIndex)
		{
			if (m_pQuestList[questIndex])
			{
				m_pQuestList[questIndex]->SetEventListener(m_pControlContainer);
				m_pQuestList[questIndex]->SetVisible(0);
				m_pQuestList[questIndex]->m_bSelectEnable = 0;
			}
			if (m_pQuestContentList[questIndex])
				m_pQuestContentList[questIndex]->SetVisible(0);
		}
		if (m_pQuestMemo)
		{
			m_pQuestMemo->m_cAlwaysAlt = 1;
			m_pQuestMemo->m_cBlink = 1;
			m_pQuestMemo->SetVisible(0);
		}
		if (m_pQuestPanel)
		{
			PositionCompatQuestPanel();
			m_pQuestPanel->SetVisible(0);
		}
		if (m_pQuestBtn)
			m_pQuestBtn->SetSelected(0);
		memset(m_pLevelQuest, 0, sizeof(m_pLevelQuest));
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects.txt", 97);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects2.txt", 98);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects3.txt", 99);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects4.txt", 101);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestMessage.txt", 100);
		m_pNativeCCPhysicalBtn = static_cast<SButton*>(m_pControlContainer->FindControl(318));
		m_pNativeCCMagicBtn = static_cast<SButton*>(m_pControlContainer->FindControl(319));
		if (m_pNativeCCPhysicalBtn)
		{
			m_pNativeCCPhysicalBtn->SetVisible(0);
			m_pNativeCCPhysicalBtn->SetEnable(0);
			m_pNativeCCPhysicalBtn = nullptr;
		}
		if (m_pNativeCCMagicBtn)
		{
			m_pNativeCCMagicBtn->SetVisible(0);
			m_pNativeCCMagicBtn->SetEnable(0);
			m_pNativeCCMagicBtn = nullptr;
		}
		m_pServerPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(12288));
		m_pPartyPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1857));
		m_pPartyList = static_cast<SListBox*>(m_pControlContainer->FindControl(1863));
		m_pPartyBtn = static_cast<SButton*>(m_pControlContainer->FindControl(5742));
		PositionCompatPartyPanel();
		if (m_pPartyPanel && m_pPartyBtn)
			m_pPartyBtn->SetSelected(m_pPartyPanel->m_bVisible == 0);
		if (m_pInputGoldPanel)
		{
			m_pInputGoldPanel->SetVisible(0);
			m_pInputGoldPanel->m_bModal = 1;
			m_pControlContainer->m_pModalControl[3] = m_pInputGoldPanel;
		}
		if (m_pInputBG2)
			m_pInputBG2->SetVisible(0);
		// Ghidra FUN_00435b13 anchors the native bottom controls to the viewport,
		// places the 292 flyout immediately above/left of button 5744 and hides it.
		// FieldScene2.bin contains absolute 800x600 coordinates, so this must run
		// after RC scaling to make the bar reach the actual window edge.
		SControl* pNativeMainMenuButton = m_pControlContainer->FindControl(5744);
		SControl* pNativeShortSkillBar = m_pControlContainer->FindControl(5745);
		SControl* pNativeChatBar = m_pControlContainer->FindControl(5739);
		SControl* pNativeMainInfo = m_pControlContainer->FindControl(5716);
		SControl* pNativeMainMenuPanel = m_pControlContainer->FindControl(292);
		// Native 7.48 FUN_00435b13 uses control 292 only as the presence gate and
		// allocates exactly sixteen 23x23 affect icons (IDs 12806..12821).  The
		// imported 7.59 initializer instead expects IDs 90400+, which do not exist
		// in FieldScene2.bin and made Affect_Main return before drawing any buff.
		m_pMiniPanel = static_cast<SPanel*>(pNativeMainMenuPanel);
		for (int affectIndex = 0; affectIndex < 16; ++affectIndex)
		{
			m_pAffectIcon[affectIndex] = new SPanel(200, 0.0f, 0.0f, 23.0f, 23.0f,
				0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
			if (m_pAffectIcon[affectIndex])
			{
				m_pAffectIcon[affectIndex]->SetControlID(12806u + affectIndex);
				m_pAffectIcon[affectIndex]->SetEventListener(m_pControlContainer);
				m_pAffectIcon[affectIndex]->SetVisible(0);
				m_pControlContainer->AddItem(m_pAffectIcon[affectIndex]);
			}
		}
		m_pAffectDesc = static_cast<SText*>(m_pControlContainer->FindControl(12834));
		if (m_pAffectDesc)
			m_pAffectDesc->SetVisible(0);
		// InitializeScene returns immediately after this compat initializer.
		// Target/party icons must be created here, not in the modern HUD branch.
		observed_affect_ui::InitializePanels(m_pTargetAffectIcon, m_pPartyAffectIcon,
			[this]() -> SPanel* {
				auto* panel = new ObservedAffectPanel();
				m_pControlContainer->AddItem(panel);
				return panel;
			});
		// FUN_00435b13 binds the only 7.48 chat list (5377), its edit (5123)
		// and the edit panel (5739).  The imported source split system and normal
		// messages into two newer lists, so both logical aliases must target the
		// single resource-owned legacy list instead of controls absent from 7.48.
		m_pChatList = static_cast<SListBox*>(m_pControlContainer->FindControl(5377));
		m_pChatListnotice = m_pChatList;
		m_pEditChat = static_cast<SEditableText*>(m_pControlContainer->FindControl(5123));
		m_pEditChatPanel = static_cast<SPanel*>(pNativeChatBar);
		m_pChatPanel = static_cast<SPanel*>(pNativeChatBar);
		// The native scene uses the 5696-5704 channel controls as the gate for
		// TMHuman::OnPacketMessageChat. Without the general-channel binding the
		// packet is rejected before the list/bubble update.
		m_pChatGeneral = static_cast<SButton*>(m_pControlContainer->FindControl(5697));
		m_pChatParty = static_cast<SButton*>(m_pControlContainer->FindControl(5698));
		m_pChatWhisper = static_cast<SButton*>(m_pControlContainer->FindControl(5699));
		m_pChatGuild = static_cast<SButton*>(m_pControlContainer->FindControl(5700));
		m_pChatGeneral_C = static_cast<SButton*>(m_pControlContainer->FindControl(5701));
		m_pChatParty_C = static_cast<SButton*>(m_pControlContainer->FindControl(5702));
		m_pChatWhisper_C = static_cast<SButton*>(m_pControlContainer->FindControl(5703));
		m_pChatGuild_C = static_cast<SButton*>(m_pControlContainer->FindControl(5704));
		// The primary buttons are the receive gates. The native 7.48 scene keeps
		// each paired *_C control selected in the same state as its primary.
		if (m_pChatGeneral)
			m_pChatGeneral->m_bSelected = 1;
		if (m_pChatParty)
			m_pChatParty->m_bSelected = 1;
		if (m_pChatWhisper)
			m_pChatWhisper->m_bSelected = 1;
		if (m_pChatGuild)
			m_pChatGuild->m_bSelected = 1;
		if (m_pChatGeneral_C)
			m_pChatGeneral_C->m_bSelected = 1;
		if (m_pChatParty_C)
			m_pChatParty_C->m_bSelected = 1;
		if (m_pChatWhisper_C)
			m_pChatWhisper_C->m_bSelected = 1;
		if (m_pChatGuild_C)
			m_pChatGuild_C->m_bSelected = 1;
		// Bind the graph actually loaded from FieldScene2.bin. The CLASSIC
		// option does not remove UI2 children from that resource.
		m_pPositionText = static_cast<SText*>(m_pControlContainer->FindControl(771));
		m_pMiniMapPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(289));
		m_pMiniMapDir = static_cast<SPanel*>(m_pControlContainer->FindControl(291));
		m_pMiniMapZoomIn = static_cast<SButton*>(m_pControlContainer->FindControl(5714));
		m_pMiniMapZoomOut = static_cast<SButton*>(m_pControlContainer->FindControl(5715));
		m_pMiniMapServerPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(6136));
		m_pMiniMapServerText = static_cast<SText*>(m_pControlContainer->FindControl(6137));
		SButton* pNativeMiniMapButton = static_cast<SButton*>(m_pControlContainer->FindControl(296));
		if (m_pPositionText)
			m_pPositionText->SetVisible(0);
		if (m_pMiniMapPanel)
		{
			// The native 7.48 control stores -45 degrees, but the current source
			// renderer applies that angle directly to the stretched map texture.
			// Keep the compat minimap axis-aligned; the direction arrow rotates
			// independently in FrameMove.
			m_pMiniMapPanel->GetGeomControl()->fAngle = 0.0f;
			m_pMiniMapPanel->m_bSelectEnable = 0;
			m_pMiniMapPanel->m_GCPanel.dwColor = 0x80FFFFFF;
			m_pMiniMapPanel->SetVisible(0);
			if (m_pMiniMapDir)
				m_pMiniMapDir->m_bSelectEnable = 0;
			if (pNativeMiniMapButton)
				pNativeMiniMapButton->SetSelected(0);

			// The native initializer allocates one marker panel and one label for
			// every g_MinimapPos entry before M can expose the map. FrameMove and
			// FUN_0044ca65 both assume this complete 256-pair ownership contract.
			for (int markerIndex = 0; markerIndex < 256; ++markerIndex)
			{
				m_pInMiniMapPosPanel[markerIndex] = new SPanel(-2, 0.0f, 0.0f, 4.0f, 4.0f,
					g_MinimapPos[markerIndex].dwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
				if (m_pInMiniMapPosPanel[markerIndex])
				{
					m_pInMiniMapPosPanel[markerIndex]->m_bSelectEnable = 0;
					m_pInMiniMapPosPanel[markerIndex]->SetVisible(0);
					m_pMiniMapPanel->AddChild(m_pInMiniMapPosPanel[markerIndex]);
				}

				m_pInMiniMapPosText[markerIndex] = new SText(-2,
					g_MinimapPos[markerIndex].szTarget,
					g_MinimapPos[markerIndex].dwColor,
					0.0f, 0.0f, 8.0f, 12.0f, 0, 0x77777777u, 1u, 0);
				if (m_pInMiniMapPosText[markerIndex])
				{
					m_pInMiniMapPosText[markerIndex]->m_bSelectEnable = 0;
					m_pInMiniMapPosText[markerIndex]->SetVisible(0);
					m_pMiniMapPanel->AddChild(m_pInMiniMapPosText[markerIndex]);
				}
			}
		}
		if (pNativeMainMenuButton)
		{
			pNativeMainMenuButton->SetStickRight();
			pNativeMainMenuButton->SetStickBottom();
		}
		if (pNativeShortSkillBar && pNativeMainMenuButton)
		{
			pNativeShortSkillBar->SetStickBottom();
			pNativeShortSkillBar->SetPos(
				pNativeMainMenuButton->m_nPosX - pNativeShortSkillBar->m_nWidth,
				pNativeShortSkillBar->m_nPosY);
		}
		if (pNativeChatBar)
		{
			pNativeChatBar->SetStickLeft();
			pNativeChatBar->SetStickBottom();
		}
		if (pNativeMainInfo)
			pNativeMainInfo->SetStickBottom();
		if (pNativeMainMenuPanel && pNativeMainMenuButton)
		{
			// FUN_00435b13 preserves root 292's resource-authored X and subtracts
			// _DAT_005A365C (2.0f); only Y is anchored above button 5744.
			pNativeMainMenuPanel->SetPos(
				pNativeMainMenuPanel->m_nPosX - 2.0f,
				pNativeMainMenuButton->m_nPosY - pNativeMainMenuPanel->m_nHeight);
			pNativeMainMenuPanel->SetVisible(0);
		}
		// Bind the native 7.48 feature-panel IDs as real scene members.  Keyboard
		// dispatch runs before the requested panel is opened, so leaving the newer
		// 7.59 pointers null makes an unrelated shortcut (for example Inventory)
		// crash while OnKeySkill probes the closed skill window.
		m_pInvenPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_INV_PANEL));
		m_pCPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_CHAR_PANEL));
		m_pShopPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1793));
		m_pCargoPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1825));
		m_pSkillMPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1889));
		// FUN_00435b13 binds the 7.48 Skill Apprentice children independently
		// from root 1889.  The imported 7.59 IDs (65604..65608) do not exist in
		// FieldScene2.bin; leaving these members null crashes on the first 0x3C4
		// shop list when OnPacketShopList writes the section captions.
		m_pSkillMDesc = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_SKILLM_DESC));
		m_pSkillMSec1 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_SKILLM_SEC1_C));
		m_pSkillMSec2 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_SKILLM_SEC2_C));
		m_pSkillMSec3 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_SKILLM_SEC3_C));
		if (auto pSkillMPanel1 = static_cast<SPanel*>(
			m_pControlContainer->FindControl(TMP_SKILLM_PANEL1)))
		{
			// The stock 7.48 initializer makes this decorative child non-selectable
			// so clicks continue to reach the skill grid and close control.
			pSkillMPanel1->m_bSelectEnable = 0;
		}
		m_pSkillPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(1905));
		m_pTradePanel = static_cast<SPanel*>(m_pControlContainer->FindControl(576));
		// FieldScene2.bin is the native 7.48-compatible resource.  It does not
		// necessarily materialize all ten pages from the newer trade layout, so
		// configure only the grids that are actually present.
		int tradeOpGridCount = 0;
		int tradeMyGridCount = 0;
		for (int slot = 0; slot < 15; ++slot)
		{
			if (auto pGrid = static_cast<SGridControl*>(m_pControlContainer->FindControl(8192 + slot)))
			{
				pGrid->m_bSelectEnable = 1;
				pGrid->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
				++tradeOpGridCount;
			}
			if (auto pGrid = static_cast<SGridControl*>(m_pControlContainer->FindControl(8448 + slot)))
			{
				pGrid->m_bSelectEnable = 1;
				pGrid->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
				++tradeMyGridCount;
			}
		}
		WYD748_DiagnosticsLog(
			"compat trade controls panel=%p opcheck=%p mycheck=%p grids=%d/%d\r\n",
			m_pTradePanel, m_pControlContainer->FindControl(TMB_TRADE_OPCHECK),
			m_pControlContainer->FindControl(TMB_TRADE_MYCHECK),
			tradeOpGridCount, tradeMyGridCount);
		// FUN_00435b13 binds the complete native player-interaction menu even
		// though it starts hidden.  InitializeCompatFieldScene returns before the
		// newer initializer reaches these assignments, so PGTVisible would
		// otherwise dereference null controls after Ctrl+right-click.
		m_pPGTPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_PGT_MENU));
		m_pPGTText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_PGT_TEXT));
		m_pBtnPGTParty = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_PARTY));
		m_pBtnPGTGuild = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GUILD));
		m_pBtnPGTTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_TRADE));
		m_pBtnPGTChallenge = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_CHALLENGE));
		m_pBtnPGT1_V_1 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_1_V_1));
		m_pBtnPGT5_V_5 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_5_V_5));
		m_pBtnPGT10_V_10 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_10_V_10));
		m_pBtnPGTAll_V_All = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_ALL_V_ALL));
		m_pBtnPGTGuildDrop = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GDROP));
		m_pBtnPGTGuildWar = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GWAR));
		m_pBtnPGTGuildAlly = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GALLY));
		m_pBtnPGTGuildInvite = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GINVITE));
		m_pBtnPGTGICommon = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GICOMMON));
		m_pBtnPGTGIChief1 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GICHIEF1));
		m_pBtnPGTGIChief2 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GICHIEF2));
		m_pBtnPGTGIChief3 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_PGT_GICHIEF3));
		if (m_pPGTPanel)
		{
			m_pPGTPanel->SetPos(
				static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f - m_pPGTPanel->m_nWidth * 0.5f,
				static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f - m_pPGTPanel->m_nHeight * 0.5f);
			m_pPGTPanel->SetVisible(0);
			m_pPGTPanel->m_bModal = 1;
			m_pControlContainer->m_pModalControl[2] = m_pPGTPanel;
		}
		WYD748_DiagnosticsLog("compat PGT controls panel=%p text=%p party=%p guild=%p trade=%p challenge=%p\r\n",
			m_pPGTPanel, m_pPGTText, m_pBtnPGTParty, m_pBtnPGTGuild, m_pBtnPGTTrade,
			m_pBtnPGTChallenge);
		PositionCompatFeaturePanels();
		// FUN_00435b13 binds all six stock artisan panels.  Numeric grid values
		// 14..23 were later reused by TMProject for unrelated controls, so retain
		// the native values only on these exact FieldScene2 control IDs and let the
		// interaction layer below resolve them by panel/control identity.
		m_pItemMixPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ITEMMIX_PANEL));
		m_pItemMixPanel2 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ITEMMIX2_PANEL));
		m_pItemMixPanel3 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ITEMMIX3_PANEL));
		m_pItemMixPanel4 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ITEMMIX4_PANEL));
		m_pItemMixPanel5 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ITEMMIX5_PANEL));
		m_pItemMixPanel6 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ITEMMIX6_PANEL));
		PositionCompatNativeMixPanels();
		for (int slot = 0; slot < 8; ++slot)
		{
			m_pGridItemMix[slot] = static_cast<SGridControl*>(
				m_pControlContainer->FindControl(TMG_ITEMMIX_MY1 + slot));
			m_pGridItemMix2[slot] = static_cast<SGridControl*>(
				m_pControlContainer->FindControl(TMG_ITEMMIX2_MY1 + slot));
			if (m_pGridItemMix[slot])
				m_pGridItemMix[slot]->m_eGridType = static_cast<TMEGRIDTYPE>(12);
			if (m_pGridItemMix2[slot])
				m_pGridItemMix2[slot]->m_eGridType = static_cast<TMEGRIDTYPE>(14);
		}
		for (int slot = 0; slot < 6; ++slot)
		{
			m_pGridItemMix3[slot] = static_cast<SGridControl*>(
				m_pControlContainer->FindControl(TMG_ITEMMIX3_MY1 + slot));
			if (m_pGridItemMix3[slot])
				m_pGridItemMix3[slot]->m_eGridType = static_cast<TMEGRIDTYPE>(16);
		}
		for (int slot = 0; slot < 3; ++slot)
		{
			m_pGridItemMix4[slot] = static_cast<SGridControl*>(
				m_pControlContainer->FindControl(TMG_ITEMMIX4_MY1 + slot));
			m_pGridItemMix6[slot] = static_cast<SGridControl*>(
				m_pControlContainer->FindControl(TMG_ITEMMIX6_MY1 + slot));
			if (m_pGridItemMix4[slot])
				m_pGridItemMix4[slot]->m_eGridType = static_cast<TMEGRIDTYPE>(18);
			if (m_pGridItemMix6[slot])
				m_pGridItemMix6[slot]->m_eGridType = static_cast<TMEGRIDTYPE>(22);
		}
		for (int slot = 0; slot < 7; ++slot)
		{
			m_pGridItemMix5[slot] = static_cast<SGridControl*>(
				m_pControlContainer->FindControl(TMG_ITEMMIX5_MY1 + slot));
			if (m_pGridItemMix5[slot])
				m_pGridItemMix5[slot]->m_eGridType = static_cast<TMEGRIDTYPE>(20);
		}
		for (int slot = 0; slot < 4; ++slot)
		{
			m_pGridMixResult[slot] = static_cast<SGridControl*>(
				m_pControlContainer->FindControl(TMG_ITEMMIX2_RESULT1 + slot));
			if (m_pGridMixResult[slot])
				m_pGridMixResult[slot]->m_eGridType = TMEGRIDTYPE::GRID_ITEMMIXRESULT;
		}
		// FUN_0044df53 includes panel 6400 in the native 7.48 ESC cascade.
		m_pGambleStore = static_cast<SPanel*>(m_pControlContainer->FindControl(6400));
		if (m_pGambleStore)
		{
			m_pReelPanel = new SReelPanel(330, 21.0f, 79.0f, 62.0f, 62.0f, 1.0f);
			m_pReelPanel2 = new SReelPanel(340, 21.0f, 79.0f, 62.0f, 62.0f, 1.0f);
			m_pGambleStore->AddChild(m_pReelPanel);
			m_pGambleStore->AddChild(m_pReelPanel2);
			PositionCompatGamblePanel();
			m_pGambleStore->SetVisible(0);
		}
		// These are the original Character values consumed by FUN_004431e4. The
		// imported initializer otherwise leaves only its 65xxx replacement aliases,
		// which makes C.POINT and mastery values disappear in FieldScene2.bin.
		m_pCIFakeExp = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_FAKEEXPPOINT));
		m_pCISpecial1 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_SPECIAL1));
		m_pCISpecial2 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_SPECIAL2));
		m_pCISpecial3 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_SPECIAL3));
		m_pCISpecial4 = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_SPECIAL4));
		m_pKingDomFlag = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_KINGDOMFLAG));
		if (m_pKingDomFlag)
			m_pKingDomFlag->SetVisible(0);
		if (auto pExpHold = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_EXP_HOLD)))
			pExpHold->SetVisible(0);
		// Grid hover writes the native 7.48 item description into this hidden panel;
		// binding it restores tooltips without importing the 7.59 tooltip resource.
		m_pDescPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(258));
		// WYD748 Ghidra FUN_00435b13 binds the tooltip name at 772 and exactly
		// twelve parameter rows at 773..799 (with the native ID gaps below).
		m_pDescNameText = static_cast<SText*>(m_pControlContainer->FindControl(772));
		for (auto& pParamText : m_pParamText)
			pParamText = nullptr;
		const unsigned int native748DescParamIDs[] = {
			773, 774, 775, 776, 777, 784,
			794, 795, 796, 797, 798, 799
		};
		for (unsigned int i = 0; i < _countof(native748DescParamIDs); ++i)
			m_pParamText[i] = static_cast<SText*>(m_pControlContainer->FindControl(native748DescParamIDs[i]));
		if (m_pDescPanel)
		{
			// The native initializer keeps the tooltip hidden and non-selectable
			// until MouseOver has materialized a valid item description.
			m_pDescPanel->m_bSelectEnable = 0;
			m_pDescPanel->SetVisible(0);
		}
		// TMP_FADEOUT is a full-screen legacy panel.  The native 7.48 transition
		// eventually disables it, but the compact frame path has no fade state
		// machine; leaving it selectable intercepts every world click invisibly.
		m_pFadePanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_FADEOUT));
		if (m_pFadePanel)
		{
			m_pFadePanel->m_bSelectEnable = 0;
			m_pFadePanel->SetVisible(0);
		}
		WYD748_DumpControlTree(m_pControlContainer, "compat-field-after-hide");
	}
	g_pDevice->m_nHeightShift = 0;
	// Keep bootstrap checkpoints in the client log; this separates resource,
	// mesh and packet failures when a 7.48 client exits before the first frame.
	LOG_WRITELOG("Compat Field Scene: bootstrap start\r\n");

	STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
	const int nTownX = static_cast<int>(pMobData->HomeTownX) >> 7;
	const int nTownY = static_cast<int>(pMobData->HomeTownY) >> 7;
	char szMapPath[128]{};
	char szDataPath[128]{};
	sprintf_s(szMapPath, "env\\Field%02d%02d.trn", nTownX, nTownY);
	sprintf_s(szDataPath, "env\\Field%02d%02d.dat", nTownX, nTownY);

	m_pGroundList[0] = new TMGround();
	if (!m_pGroundList[0] || !m_pGroundList[0]->LoadTileMap(szMapPath))
	{
		LOG_WRITELOG("Compat Field Scene: terrain load failed (%s)\\r\\n", szMapPath);
		m_bCriticalError = 1;
		return 0;
	}

	m_pGround = m_pGroundList[0];
	m_pGround->SetMiniMapData();
	LOG_WRITELOG("Compat Field Scene: terrain ready\r\n");
	m_pObjectContainerList[0] = new TMObjectContainer(m_pGround);
	if (!m_pObjectContainerList[0])
	{
		LOG_WRITELOG("Compat Field Scene: object container allocation failed\\r\\n");
		m_bCriticalError = 1;
		return 0;
	}
	if (!m_pObjectContainerList[0]->Load(szDataPath))
	{
		LOG_WRITELOG("Compat Field Scene: object data invalid (%s)\\r\\n", szDataPath);
		m_bCriticalError = 1;
		SAFE_DELETE(m_pObjectContainerList[0]);
		SAFE_DELETE(m_pGroundList[0]);
		m_pGround = nullptr;
		return 0;
	}
	if (m_pGroundObjectContainer)
	{
		m_pGroundObjectContainer->AddChild(m_pObjectContainerList[0]);
		m_pGroundObjectContainer->AddChild(m_pGroundList[0]);
	}

	for (int nY = 0; nY < 128; ++nY)
		memcpy(m_HeightMapData[nY], m_pGround->m_pMaskData[nY], 128);
	g_HeightPosX = static_cast<int>(m_pGround->m_vecOffset.x);
	g_HeightPosY = static_cast<int>(m_pGround->m_vecOffset.y);
	BASE_ApplyAttribute(reinterpret_cast<char*>(m_HeightMapData), 256);
	memcpy(m_GateMapData, m_HeightMapData, sizeof(m_HeightMapData));
	SetMinimapPos();
	LOG_WRITELOG("Compat Field Scene: height data ready\r\n");

	m_pItemContainer = new TreeNode(0);
	// Ground items belong to the same render subtree as the terrain in the
	// native field scene; keeping that ownership also preserves depth ordering
	// when the compact HUD path is active.
	if (m_pItemContainer && m_pGroundObjectContainer)
		m_pGroundObjectContainer->AddChild(m_pItemContainer);
	m_pSun = new TMSun();
	if (m_pSun && m_pEffectContainer)
	{
		m_pSun->InitObject();
		m_pEffectContainer->AddChild(m_pSun);
	}
	m_pSky = new TMSky();
	if (m_pSky)
	{
		m_pSky->m_bVisible = 0;
		AddChild(m_pSky);
	}
	LOG_WRITELOG("Compat Field Scene: environment ready\r\n");

	// Copy the authoritative score before InitObject so the skin and bars are
	// built from the server character, not from zeroed constructor defaults.
	m_pMyHuman = new TMHuman(this);
	if (!m_pMyHuman)
	{
		LOG_WRITELOG("Compat Field Scene: player allocation failed\\r\\n");
		m_bCriticalError = 1;
		return 0;
	}
	m_pMyHuman->m_dwID = g_pObjectManager->m_dwCharID;
	m_pMyHuman->m_stScore = pMobData->CurrentScore;
	m_pMyHuman->m_nTotalKill = (static_cast<unsigned char>(pMobData->MobName[15]) << 8)
		+ static_cast<unsigned char>(pMobData->MobName[14]);
	m_pMyHuman->m_nCurrentKill = m_pMyHuman->m_nTotalKill;
	m_pMyHuman->m_ucChaosLevel = static_cast<unsigned char>(pMobData->MobName[12]);
	char szName[sizeof(pMobData->MobName)]{};
	memcpy(szName, pMobData->MobName, sizeof(szName));
	szName[sizeof(szName) - 1] = 0;
	szName[12] = 0;
	szName[14] = 0;
	szName[15] = 0;
	sprintf_s(m_pMyHuman->m_szName, "%s", szName);
	m_pMyHuman->m_sGuildLevel = static_cast<unsigned char>(pMobData->GuildLevel);
	m_pMyHuman->m_usGuild = g_pObjectManager->m_stSelCharData.Guild[g_pObjectManager->m_cCharacterSlot];

	LOG_WRITELOG("Compat Field Scene: player object allocated\r\n");
	m_pMyHuman->SetPacketMOBItem(pMobData);
	LOG_WRITELOG("Compat Field Scene: player equipment materialized\r\n");
	m_pMyHuman->SetCharHeight(static_cast<float>(m_pMyHuman->m_stScore.Con));
	m_pMyHuman->SetRace(pMobData->Equip[0].sIndex);
	m_pMyHuman->InitObject();
	LOG_WRITELOG("Compat Field Scene: player render object initialized\r\n");
	m_pMyHuman->CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
	m_pMyHuman->InitAngle(0.0f, 0.39269909f, 0.0f);
	// The server sends map coordinates, not a world-space height.  Resolving
	// the terrain height here keeps the local character on the same surface as
	// observers created by OnPacketCreateMobCompat instead of leaving it at the
	// origin height (which can hide the mesh below raised terrain).
	const TMVector2 selfPosition{
		static_cast<float>(pMobData->HomeTownX) + 0.5f,
		static_cast<float>(pMobData->HomeTownY) + 0.5f};
	const float selfHeight = m_pGround->GetHeight(selfPosition);
	m_pMyHuman->InitPosition(selfPosition.x, selfHeight, selfPosition.y);
	m_pMyHuman->m_cHide = (m_pMyHuman->m_dwID < 1000) && (m_pMyHuman->m_stScore.Merchant & 1);
	if (m_pHumanContainer)
		m_pHumanContainer->AddChild(m_pMyHuman);
	if (g_pObjectManager->m_pCamera)
		g_pObjectManager->m_pCamera->SetFocusedObject(m_pMyHuman);
	m_bLastMyAttr = BASE_GetAttr(pMobData->HomeTownX, pMobData->HomeTownY);

	// Weather/effect containers are created because incoming 7.48 packets can
	// arrive immediately after the self CreateMob packet.
	m_pRain = new TMRain();
	m_pSnow = new TMSnow(1.0f);
	m_pSnow2 = new TMSnow(2.0f);
	if (m_pRain && m_pEffectContainer) m_pEffectContainer->AddChild(m_pRain);
	if (m_pSnow && m_pEffectContainer) m_pEffectContainer->AddChild(m_pSnow);
	if (m_pSnow2 && m_pEffectContainer) m_pEffectContainer->AddChild(m_pSnow2);

	LOG_WRITELOG("Compat Field Scene: player %s at (%d,%d)\\r\\n", m_pMyHuman->m_szName,
		pMobData->HomeTownX, pMobData->HomeTownY);
	// Populate the two legacy presentation adapters only after the character
	// exists, because name/level and item icon construction depend on that state.
	InitializeCompatInventory();
	// Bootstrap must not run the interactive cleanup path: these packet members
	// have no valid CarryPos state yet, and cleanup also changes live UI grids.
	for (int mixIndex = 1; mixIndex <= 6; ++mixIndex)
		ResetNativeMixPacket(mixIndex);
	// Ghidra FUN_00489023 immediately calls the native shortcut-grid updater for
	// opcode 0x378, so both runtime belts must exist before queued world packets
	// are dispatched after this initializer returns.
	InitializeCompatSkillBelts();
	InitializeCompatCCControls();
	InitializeFireWorkControls();
	InitializeQuizEventControls();
	UpdateCompatLearnedSkillUI();
	// Counters are created at runtime on the full path and are not part of
	// FieldScene2.bin. The compatible early return skipped these allocations,
	// although WYD-Go sends 0x3A1/0x3B0 during instances and quests.
	InitializeRuntimeCounterTexts();
	// The full field initializer runs this exact sequence after focusing the local
	// human.  Without it the compact 7.48 path leaves TMCamera in quarter-view 1,
	// and native FUN_004aec3d deliberately rejects mouse rotation in that mode.
	InitCameraView();
	SetCameraView();
	UpdateCompatScoreUI();
	return 1;
}

void TMFieldScene::PositionCompatFeaturePanels()
{
	if (!m_bCompatFieldScene || !g_pDevice)
		return;

	const float viewportCenterX = static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f;
	const float viewportCenterY = static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f;

	// Native WYD 7.48 FUN_00435b13 resolves roots 513, 1905 and 257 after the
	// resource is loaded. The constants at 005A34A0, 005A430C and 005A3670 are
	// 0.5f, 1.5f and 10.0f respectively. At 800x600 this yields the original
	// Character (49.5, 89.5), Skill (286.5, 89.5), Inventory (523.5, 89.5)
	// composition instead of the shared (530, 0) serialized root position.
	if (m_pCPanel)
	{
		m_pCPanel->SetPos(
			viewportCenterX - m_pCPanel->m_nWidth * 1.5f - 10.0f,
			viewportCenterY - m_pCPanel->m_nHeight * 0.5f);
	}
	if (m_pSkillPanel)
	{
		m_pSkillPanel->SetPos(
			viewportCenterX - m_pSkillPanel->m_nWidth * 0.5f,
			viewportCenterY - m_pSkillPanel->m_nHeight * 0.5f);
	}
	if (m_pInvenPanel)
	{
		m_pInvenPanel->SetPos(
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + 10.0f,
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
	PositionCompatShopPanels();
	PositionCompatTradePanels();
}

void TMFieldScene::PositionCompatNativeMixPanels()
{
	if (!m_bCompatFieldScene || !g_pDevice)
		return;

	const float viewportCenterX = static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f;
	const float viewportCenterY = static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f;

	// FUN_00435b13 centers each independent ItemMix root (1360, 6110, 6145,
	// 6432, 6481 and 6512) and places the shared Inventory root 257 at the
	// native right-hand position with a ten-pixel logical gap.
	for (int mixIndex = 1; mixIndex <= 6; ++mixIndex)
	{
		auto panel = GetNativeMixPanel(mixIndex);
		if (panel)
		{
			panel->SetPos(
				viewportCenterX - panel->m_nWidth * 0.5f,
				viewportCenterY - panel->m_nHeight * 0.5f);
		}
	}
	if (m_pInvenPanel)
	{
		m_pInvenPanel->SetPos(
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + 10.0f,
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
}

void TMFieldScene::PositionCompatShopPanels()
{
	if (!m_bCompatFieldScene || !g_pDevice)
		return;

	const float viewportCenterX = static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f;
	const float viewportCenterY = static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f;

	// FUN_00435b13 centers root 1793 (Shop) and places root 257 (Inventory)
	// immediately to its right using the native 10-pixel logical gap.
	if (m_pShopPanel)
	{
		m_pShopPanel->SetPos(
			viewportCenterX - m_pShopPanel->m_nWidth * 0.5f,
			viewportCenterY - m_pShopPanel->m_nHeight * 0.5f);
	}
	if (m_pInvenPanel)
	{
		m_pInvenPanel->SetPos(
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + 10.0f,
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
}

void TMFieldScene::PositionCompatTradePanels()
{
	if (!m_bCompatFieldScene || !g_pDevice)
		return;

	const float viewportCenterX = static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f;
	const float viewportCenterY = static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f;

	// FUN_00435b13 centers root 576 (Trade) and places root 257 (Inventory)
	// immediately to its right using the same native 10-pixel logical gap.
	if (m_pTradePanel)
	{
		m_pTradePanel->SetPos(
			viewportCenterX - m_pTradePanel->m_nWidth * 0.5f,
			viewportCenterY - m_pTradePanel->m_nHeight * 0.5f);
	}
	if (m_pInvenPanel)
	{
		m_pInvenPanel->SetPos(
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + 10.0f,
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
}

void TMFieldScene::PositionCompatGamblePanel()
{
	if (!m_bCompatFieldScene || !g_pDevice || !m_pGambleStore)
		return;

	const float viewportCenterX = static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f;
	const float viewportCenterY = static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f;

	// Native WYD 7.48 FUN_00435b13 positions root 6400 ten logical pixels to
	// the right of the viewport center while preserving vertical centering.
	m_pGambleStore->SetPos(
		viewportCenterX - m_pGambleStore->m_nWidth * 0.5f + 10.0f,
		viewportCenterY - m_pGambleStore->m_nHeight * 0.5f);
}

void TMFieldScene::PositionCompatPartyPanel()
{
	if (!m_bCompatFieldScene || !g_pDevice || !m_pPartyPanel)
		return;

	// Native WYD 7.48 FUN_00435b13 places both the Party root 1857 and its
	// toggle 5742 at the same lower-left anchor. FieldScene2.bin alone does
	// not account for the active viewport height.
	const float partyY = static_cast<float>(g_pDevice->m_dwScreenHeight)
		- m_pPartyPanel->m_nHeight - 165.0f;
	m_pPartyPanel->SetPos(0.0f, partyY);
	if (m_pPartyBtn)
		m_pPartyBtn->SetPos(0.0f, partyY);
}

void TMFieldScene::PositionCompatQuestPanel()
{
	if (!g_pDevice || !m_pQuestPanel)
		return;

	// Native FUN_00441823 centers root 320 on both axes. The imported 7.59
	// initializer used 0.6 of the panel height and shifted the whole hitbox up.
	m_pQuestPanel->SetPos(
		(static_cast<float>(g_pDevice->m_dwScreenWidth) - m_pQuestPanel->m_nWidth) * 0.5f,
		(static_cast<float>(g_pDevice->m_dwScreenHeight) - m_pQuestPanel->m_nHeight) * 0.5f);
}

void TMFieldScene::SelectQuestTab(int tabIndex)
{
	if (tabIndex < 0 || tabIndex >= 4)
		return;

	for (int questIndex = 0; questIndex < 4; ++questIndex)
	{
		const int selected = questIndex == tabIndex;
		if (m_pQuestButton[questIndex])
			m_pQuestButton[questIndex]->SetSelected(selected);
		if (m_pQuestList[questIndex])
		{
			m_pQuestList[questIndex]->SetVisible(selected);
			m_pQuestList[questIndex]->m_bSelectEnable = selected;
		}
		if (m_pQuestContentList[questIndex])
			m_pQuestContentList[questIndex]->SetVisible(selected);
	}

	if (m_pQuestMemo)
		m_pQuestMemo->SetVisible(0);
}

void TMFieldScene::SelectHelpTab(int tabIndex)
{
	if (tabIndex < 0 || tabIndex >= 5)
		return;

	// FieldScene2.bin can materialize the native Help controls as siblings of
	// the 6067 panel instead of as one strict child tree.  FindControl() only
	// returns the first matching node, so hiding that one pointer can leave a
	// second copy of the Interface page drawable over Command/Message.  Walk
	// the complete RC tree and apply the page state to every exact native ID.
	auto forEachControl = [&](auto&& self, TreeNode* first, auto&& callback) -> void
	{
		for (TreeNode* current = first; current != nullptr; current = current->m_pNextLink)
		{
			if (!current->m_cDeleted)
				callback(static_cast<SControl*>(current));
			if (current->m_pDown)
				self(self, current->m_pDown, callback);
		}
	};
	auto setControlsVisible = [&](unsigned int controlID, int visible)
	{
		if (!m_pControlContainer || !m_pControlContainer->m_pControlRoot)
			return;
		forEachControl(forEachControl, m_pControlContainer->m_pControlRoot,
			[&](SControl* control)
			{
				if (control->GetControlID() == controlID)
					control->SetVisible(visible);
			});
	};

	WYD748_DiagnosticsLog("help tab begin tab=%d root=%p rootId=%u rootVisible=%d panels=%p/%p/%p lists=%p/%p/%p\r\n",
		tabIndex, m_pHelpInterface,
		m_pHelpInterface ? m_pHelpInterface->GetControlID() : 0u,
		m_pHelpInterface ? m_pHelpInterface->IsVisible() : -1,
		m_pHelpInterfacePanel[0], m_pHelpInterfacePanel[1], m_pHelpInterfacePanel[2],
		m_pHelpInterfaceList[0], m_pHelpInterfaceList[1], m_pHelpInterfaceList[2]);

	// The resource does not make the four top-level lists children of one
	// another. Clear every list and every Help-page control before selecting the
	// requested tab; otherwise the previously visible page remains drawable.
	for (int i = 0; i < 4; ++i)
	{
		if (m_pHelpButton[i])
			m_pHelpButton[i]->SetSelected(tabIndex == i);
		if (m_pHelpList[i])
		{
			m_pHelpList[i]->SetVisible(0);
			m_pHelpList[i]->m_bSelectEnable = 0;
		}
	}

	// Clear all native Interface-page copies, including controls that are not
	// returned by the first-match bindings above.
	static const unsigned int interfaceControlIDs[] =
	{
		6067, 6064, 6065, 6066, 6071, 6072, 6073, 6074, 6075, 6076
	};
	for (const unsigned int controlID : interfaceControlIDs)
		setControlsVisible(controlID, 0);

	if (auto pTalkButton = m_pControlContainer
		? static_cast<SButton*>(m_pControlContainer->FindControl(TMB_HELP_BUTTON5))
		: nullptr)
		pTalkButton->SetSelected(tabIndex == 4);

	for (int i = 0; i < 3; ++i)
	{
		if (m_pHelpInterfacePanel[i])
			m_pHelpInterfacePanel[i]->SetVisible(0);
		if (m_pHelpInterfaceList[i])
		{
			m_pHelpInterfaceList[i]->SetVisible(0);
			m_pHelpInterfaceList[i]->m_bSelectEnable = 0;
		}
		// FieldScene2.bin materializes each keyboard-page background as a
		// separate panel (6071..6073).  Clear it explicitly as well: the
		// resource/runtime can render these siblings even after the page/list
		// bindings have been hidden.
	}

	if (tabIndex == 0)
	{
		if (m_nCurrInterfacePanelIndex < 0 || m_nCurrInterfacePanelIndex >= 3)
			m_nCurrInterfacePanelIndex = 0;
		setControlsVisible(6067, 1);
		setControlsVisible(6064 + m_nCurrInterfacePanelIndex, 1);
		setControlsVisible(6071 + m_nCurrInterfacePanelIndex, 1);
		setControlsVisible(6074 + m_nCurrInterfacePanelIndex, 1);
		if (m_pHelpInterfaceList[m_nCurrInterfacePanelIndex])
			m_pHelpInterfaceList[m_nCurrInterfacePanelIndex]->m_bSelectEnable = 1;
		WYD748_DiagnosticsLog("help tab end tab=%d rootVisible=%d p0=%d/%d/%d b0=%d/%d/%d l0=%d/%d/%d\r\n",
			tabIndex,
			m_pHelpInterface ? m_pHelpInterface->IsVisible() : -1,
			m_pHelpInterfacePanel[0] ? m_pHelpInterfacePanel[0]->GetControlID() : 0u,
			m_pHelpInterfacePanel[1] ? m_pHelpInterfacePanel[1]->GetControlID() : 0u,
			m_pHelpInterfacePanel[2] ? m_pHelpInterfacePanel[2]->GetControlID() : 0u,
			m_pHelpInterfacePanel[0] ? m_pHelpInterfacePanel[0]->IsVisible() : -1,
			m_pHelpInterfacePanel[1] ? m_pHelpInterfacePanel[1]->IsVisible() : -1,
			m_pHelpInterfacePanel[2] ? m_pHelpInterfacePanel[2]->IsVisible() : -1,
			m_pHelpInterfaceList[0] ? m_pHelpInterfaceList[0]->IsVisible() : -1,
			m_pHelpInterfaceList[1] ? m_pHelpInterfaceList[1]->IsVisible() : -1,
			m_pHelpInterfaceList[2] ? m_pHelpInterfaceList[2]->IsVisible() : -1);
		return;
	}

	// Command, Others and Message use lists 1..3; WYD Talk is the original
	// interface.txt list (868), exposed by the fifth tab (879).
	const int listIndex = tabIndex == 4 ? 0 : tabIndex;
	setControlsVisible(868 + (listIndex * 2), 1);
	if (m_pHelpList[listIndex])
	{
		m_pHelpList[listIndex]->SetVisible(1);
		m_pHelpList[listIndex]->m_bSelectEnable = 1;
	}

	WYD748_DiagnosticsLog("help tab end tab=%d rootVisible=%d p0=%d/%d/%d b0=%d/%d/%d l0=%d/%d/%d\r\n",
		tabIndex,
		m_pHelpInterface ? m_pHelpInterface->IsVisible() : -1,
		m_pHelpInterfacePanel[0] ? m_pHelpInterfacePanel[0]->GetControlID() : 0u,
		m_pHelpInterfacePanel[1] ? m_pHelpInterfacePanel[1]->GetControlID() : 0u,
		m_pHelpInterfacePanel[2] ? m_pHelpInterfacePanel[2]->GetControlID() : 0u,
		m_pHelpInterfacePanel[0] ? m_pHelpInterfacePanel[0]->IsVisible() : -1,
		m_pHelpInterfacePanel[1] ? m_pHelpInterfacePanel[1]->IsVisible() : -1,
		m_pHelpInterfacePanel[2] ? m_pHelpInterfacePanel[2]->IsVisible() : -1,
		m_pHelpInterfaceList[0] ? m_pHelpInterfaceList[0]->IsVisible() : -1,
		m_pHelpInterfaceList[1] ? m_pHelpInterfaceList[1]->IsVisible() : -1,
		m_pHelpInterfaceList[2] ? m_pHelpInterfaceList[2]->IsVisible() : -1);
}

void TMFieldScene::SetQuestPanelVisible(bool visible)
{
	if (!m_pQuestPanel)
		return;

	const bool changed = (m_pQuestPanel->IsVisible() != 0) != visible;
	if (visible)
	{
		PositionCompatQuestPanel();
		if (g_pObjectManager)
		{
			static const char* const questSubjectFiles[4] =
			{
				"UI\\QuestSubjects.txt",
				"UI\\QuestSubjects2.txt",
				"UI\\QuestSubjects3.txt",
				"UI\\QuestSubjects4.txt"
			};
			const STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
			for (int questIndex = 0; questIndex < 4; ++questIndex)
			{
				if (m_pQuestList[questIndex])
				{
					TMScene::LoadMsgText3(
						m_pQuestList[questIndex],
						questSubjectFiles[questIndex],
						pMobData->CurrentScore.Level + 1,
						pMobData->Equip[0].sIndex % 10);
				}
			}
		}
		SelectQuestTab(0);
	}

	m_pQuestPanel->SetVisible(visible);
	if (m_pQuestBtn)
		m_pQuestBtn->SetSelected(visible);
	if (changed)
		GetSoundAndPlay(51, 0, 0);
}

void TMFieldScene::SetVisibleCharInfo()
{
	SGridControl::m_sLastMouseOverIndex = -1;
	// The native 7.48 character panel is self-contained; toggling it must not
	// dereference the cargo/gamble extensions introduced by later clients.
	if (m_bCompatFieldScene)
	{
		if (m_pCPanel)
			m_pCPanel->SetVisible(m_pCPanel->IsVisible() == 0);
		GetSoundAndPlay(51, 0, 0);
		return;
	}

	auto pCPanel = m_pCPanel;
	auto pCargoPanel = m_pCargoPanel;
	auto pCargoPanel1 = m_pCargoPanel1;
	auto pSkillMPanel = m_pSkillMPanel;
	auto pSkillPanel = m_pSkillPanel;
	auto pTradePanel = m_pTradePanel;
	auto pATradePanel = m_pAutoTrade;
	auto pHellgateStore = m_pHellgateStore;
	auto pGambleStore = m_pGambleStore;

	bool bInv = m_pInvenPanel->m_bVisible;
	bool bVisible = pCPanel->m_bVisible == 0;

	pCPanel->SetVisible(bVisible);
	pCPanel->SetVisible(bVisible);

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(51);
		if (pSoundData)
		{
			pSoundData->Play();
		}
	}
}

void TMFieldScene::UpdateScoreUI(unsigned int unFlag)
{
	// The 7.48 HUD has a different control graph.  Its dedicated projector
	// updates the same authoritative score without dereferencing 7.59-only UI.
	if (m_bCompatFieldScene)
	{
		UpdateCompatScoreUI();
		UpdateCompatLearnedSkillUI();
		return;
	}

	auto pMobData = &g_pObjectManager->m_stMobData;
	if (g_pObjectManager->m_stMobData.Equip[13].sIndex == 769)
	{
		int sanc = BASE_GetItemSanc(&pMobData->Equip[13]);
		int hpprogress = 0;

		if (sanc > 6)
			hpprogress = 545;
		else if (sanc > 3)
			hpprogress = 546;
		else if (sanc > 0)
			hpprogress = 547;
		else
			hpprogress = 538;
		m_pHPBar->m_GCProgress.nTextureSetIndex = hpprogress;
		m_pHPBar->Update();
	}
	else
	{
		m_pHPBar->m_GCProgress.nTextureSetIndex = 538;
		m_pHPBar->Update();
	}

	if (pMobData->CurrentScore.CurHP > pMobData->CurrentScore.MaxHP)
		pMobData->CurrentScore.CurHP = pMobData->CurrentScore.MaxHP;

	if (m_pHPBar)
		m_pHPBar->SetMaxProgress(pMobData->CurrentScore.MaxHP);

	if (m_pHPBar)
		m_pHPBar->SetCurrentProgress(pMobData->CurrentScore.CurHP);

	char szStr[128]{};
	sprintf(szStr, "%d", pMobData->CurrentScore.CurHP);
	if (m_pCHP)
		m_pCHP->SetText(szStr, 0);

	// HP on "C"
	sprintf(szStr, "%d", pMobData->CurrentScore.CurHP);
	if (m_pCIHP)
		m_pCIHP->SetText(szStr, 0);
	if (m_pMPBar)
		m_pMPBar->SetMaxProgress(pMobData->CurrentScore.MaxMP);
	if (m_pMPBar)
		m_pMPBar->SetCurrentProgress(pMobData->CurrentScore.CurMP);

	// MP on "C"
	sprintf(szStr, "%d", pMobData->CurrentScore.CurMP);
	if (m_pCMP)
		m_pCMP->SetText(szStr, 0);
	if (m_pCIMP)
		m_pCIMP->SetText(szStr, 0);

	if (m_pMainCharName)
		m_pMainCharName->SetText(pMobData->MobName, 0);

	if (!(unFlag & 0x10))
	{
		sprintf(szStr, "%I64d", pMobData->Exp);
		if (m_pCIEXP)
		{
			m_pCIEXP->m_cComma = 1;
			m_pCIEXP->SetText(szStr, 0);
		}

		long long nLevelCount = 0;
		int ckind = m_pMyHuman->Is2stClass();
		int CurLevel = pMobData->CurrentScore.Level;
		if (ckind == 2)
			nLevelCount = (g_pNextLevel_G2[CurLevel + 1] - g_pNextLevel_G2[CurLevel]) / 10;
		else
			nLevelCount = (g_pNextLevel[CurLevel + 1] - g_pNextLevel[CurLevel]) / 10;

		nLevelCount = nLevelCount > 0
			? static_cast<int>((((float)g_pObjectManager->m_nFakeExp * 100.0f) / (float)nLevelCount))
			: 0;
		sprintf(szStr, g_pMessageStringTable[304], g_pObjectManager->m_nFakeExp, nLevelCount);
		if (m_pGridCharFace)
		{
			m_pGridCharFace->m_GCPanel.nTextureSetIndex = 541;
			m_pGridCharFace->m_GCPanel.nTextureIndex = ckind + 3 * pMobData->Class;
		}
		if (m_pCIFakeExp)
		{
			m_pCIFakeExp->SetText(szStr, 0);
			if (nLevelCount < 80)
				m_pCIFakeExp->SetTextColor(0xFFFFFFFF);
			else
				m_pCIFakeExp->SetTextColor(0xFFFF0000);
		}
		if (m_pExpHold)
		{
			if (g_pObjectManager->m_nFakeExp <= 0)
				m_pExpHold->SetVisible(0);
			else
				m_pExpHold->SetVisible(1);
		}

		sprintf(szStr, "%d", pMobData->CurrentScore.Level + 1);
		m_Level = pMobData->CurrentScore.Level + 1;
		if (m_pCLevel)
			m_pCLevel->SetText(szStr, 0);
		if (m_pMainInfo2_Lv)
			m_pMainInfo2_Lv->SetText(szStr, 0);

		ckind = m_pMyHuman->Is2stClass();
		CurLevel = pMobData->CurrentScore.Level;
		if (ckind == 2)
			sprintf(szStr, "%I64d", g_pNextLevel_G2[CurLevel + 1]);
		else
			sprintf(szStr, "%I64d", g_pNextLevel[CurLevel + 1]);

		char strEXP[256]{};
		if (m_pCIEXPE)
		{
			m_pCIEXPE->m_cComma = 1;
			m_pCIEXPE->SetText(szStr, 0);
		}

		// The UI renders authoritative counters, not their signed-short
		// compatibility projection stored inside STRUCT_MOB.
		sprintf(szStr, "%u", g_pObjectManager->m_stMobData.CurrentScore.StatusPts);
		if (m_pScBonus)
			m_pScBonus->SetText(szStr, 0);

		sprintf(szStr, "%u", g_pObjectManager->m_stMobData.CurrentScore.MasterPts);
		if (m_pSpBonus)
			m_pSpBonus->SetText(szStr, 0);

		sprintf(szStr, "%u", g_pObjectManager->m_stMobData.CurrentScore.SkillPts);
		if (m_pSkBonus)
			m_pSkBonus->SetText(szStr, 0);

		char szMoeny[64]{};
		char szRMoeny[64]{};
		sprintf(szMoeny, "%10d", pMobData->Coin);
		sprintf(szRMoeny, "%10d", g_pObjectManager->m_RMBCount);
		m_Coin = pMobData->Coin;
		m_RMB = g_pObjectManager->m_RMBCount;
		if (m_pMoney1)
		{
			m_pMoney1->m_cComma = 1;
			m_pMoney1->SetText(szMoeny, 0);
		}
		if (m_pMoney4)
		{
			m_pMoney4->m_cComma = 1;
			m_pMoney4->SetText(szRMoeny, 0);
		}
		if (m_pMoney2)
		{
			m_pMoney2->m_cComma = 1;
			m_pMoney2->SetText(szMoeny, 0);
		}

		sprintf(szMoeny, "%10d", pMobData->Coin);
		if (m_pMoney3)
		{
			m_pMoney3->m_cComma = 1;
			m_pMoney3->SetText(szMoeny, 0);
		}

		char szNameTemp[128]{};
		sprintf(szNameTemp, "[%s]:%d", pMobData->MobName, strlen(pMobData->MobName));
		if (m_pCIName)
			m_pCIName->SetText(szNameTemp, 1);

		static const char* szClass[4] = {
			g_pMessageStringTable[121],
			g_pMessageStringTable[122],
			g_pMessageStringTable[123],
			g_pMessageStringTable[124]
		};

		char szValue[256]{};
		sprintf(szValue, "%d", m_pMyHuman->m_nTotalKill);
		m_pMyHuman->m_pKillLabel->SetText(szValue, 0);

		m_pCIGuild->SetText(szValue, 0);
		m_pCIGuild->SetTextColor(0xFFAAAAAA);
		if (pMobData->Equip[0].sIndex >= 40)
		{
			m_pCIClass->SetText((char*)"Monster", 0);
			m_pCIClass2->SetText((char*)" ", 0);
		}
		else if (pMobData->Equip[0].sIndex % 10 >= 6)
		{
			m_pCIClass->SetText((char*)szClass[pMobData->Class], 0);
			m_pCIClass2->SetText((char*)szClass[pMobData->Equip[0].sIndex / 10], 0);
		}
		else
		{
			m_pCIClass->SetText((char*)szClass[pMobData->Class], 0);
			m_pCIClass2->SetText((char*)" ", 0);
		}

		sprintf(szStr, "%d", pMobData->CurrentScore.Str);
		if (m_pCIStr)
			m_pCIStr->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Int);
		if (m_pCIInt)
			m_pCIInt->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Dex);
		if (m_pCIDex)
			m_pCIDex->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Con);
		if (m_pCICon)
			m_pCICon->SetText(szStr, 0);

		int nMaxLevel = 0;
		if (m_pMyHuman->Is2stClass() == 2)
		{
			nMaxLevel = 200;
		}
		else
		{
			nMaxLevel = 3 * (pMobData->CurrentScore.Level + 1) / 2;
			if (nMaxLevel > 200)
				nMaxLevel = 200;
		}

		int nMax3 = nMaxLevel;
		int nMax2 = nMaxLevel;
		int nMax1 = nMaxLevel;
		if (!pMobData->Class && IsValidSkill(205) == 1)
			nMaxLevel = 280;
		else if (pMobData->Class == 2 && IsValidSkill(233) == 1)
			nMaxLevel = 230;
		if (pMobData->Class == 3 && IsValidSkill(238) == 1)
			nMax1 = 400;
		else if (IsValidSkill(200) == 1)
			nMax1 = 320;
		else if (IsValidSkill(31) == 1)
			nMax1 = 255;
		if (IsValidSkill(204) == 1)
			nMax2 = 320;
		else if (IsValidSkill(39) == 1)
			nMax2 = 255;
		if (IsValidSkill(208) == 1)
			nMax3 = 320;
		else if (IsValidSkill(47) == 1)
			nMax3 = 255;

		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Mastery[0], nMaxLevel);
		if (m_pCISpecial1)
			m_pCISpecial1->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Mastery[1], nMax1);
		if (m_pCISpecial2)
			m_pCISpecial2->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Mastery[2], nMax2);
		if (m_pCISpecial3)
			m_pCISpecial3->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Mastery[3], nMax3);
		if (m_pCISpecial4)
			m_pCISpecial4->SetText(szStr, 0);

		long long nMax = 0;
		long long nCur = 0;
		if (ckind == 2)
		{
			nMax = (g_pNextLevel_G2[CurLevel + 1] - g_pNextLevel_G2[CurLevel]);
			nCur = pMobData->Exp - g_pNextLevel_G2[CurLevel];
		}
		else
		{
			nMax = g_pNextLevel[CurLevel + 1] -g_pNextLevel[CurLevel];
			nCur = pMobData->Exp - g_pNextLevel[CurLevel];
		}

		int nLamp = static_cast<int>(nMax / 4);
		int nGrid = 4;

		if (nCur >= nMax / 4)
		{
			if (nCur < static_cast<long long>(2 * static_cast<long long>(nLamp)))
			{
				nGrid = 2;
			}
			else
			{
				if (nCur < static_cast<long long>(3 * static_cast<long long>(nLamp)))
					nGrid = 3;
			}
		}
		else
		{
			nGrid = 1;
		}

		for (int l = 0; l < 3; ++l)
		{
			if (m_pExpLamp[l])
				m_pExpLamp[l]->m_GCPanel.nTextureIndex = l < nGrid - 1;
		}

		nCur -= (static_cast<long long>(nGrid) - 1) * static_cast<long long>(nLamp);
		nMax = static_cast<long long>((double)nLamp * 0.1);

		for (int n = 0; n < 10; ++n)
		{
			if (nCur > nMax)
			{
				nCur -= nMax;
				if (m_pExpProgress[n])
				{
					m_pExpProgress[n]->SetMaxProgress(static_cast<int>(nMax));
					m_pExpProgress[n]->SetCurrentProgress(static_cast<int>(nMax));
				}
			}
			else if (m_pExpProgress[n])
			{
				m_pExpProgress[n]->SetMaxProgress(static_cast<int>(nMax));
				m_pExpProgress[n]->SetCurrentProgress(static_cast<int>(nCur));
				nCur = 0;
			}
		}

		int face = pMobData->Equip[0].sIndex;
		int cls = 0;
		if (face < 40)
		{
			if (face == 32)
			{
				cls = 2;
			}
			else
			{
				int mod = face % 10;
				int div = face / 10;
				if (face % 10 < 6 || mod > 9)
					cls = div;
				else
					cls = mod - 6;
			}
			pMobData->Class = cls;
		}

		if (!(unFlag & 1))
		{
			int sIndex = 0;
			int ItemIndex = 0;
			// Skill list on "S"
			for (int i = 0; i < 24; ++i)
			{
				unsigned int dwBit = 1 << i;

				if (!m_pSkillSecGrid[i])
					continue;

				if ((dwBit & pMobData->LearnedSkill[0]) != dwBit)
				{
					m_pSkillSecGrid[i]->Empty();
					continue;
				}

				auto pSkillItem = m_pSkillSecGrid[i]->GetItem(0, 0);
				sIndex = 24 * pMobData->Class + i + 5000;

				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					ItemIndex = pstOldItem->sIndex;

					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = 0;
						continue;
					}
				}

				m_pSkillSecGrid[i]->Empty();

				auto pStructItem = new STRUCT_ITEM;
				memset(pStructItem, 0, sizeof(STRUCT_ITEM));

				pStructItem->sIndex = sIndex;

				auto ipCtrlItem = new SGridControlItem(0, pStructItem, 0.0f, 0.0f);
				if (ipCtrlItem && m_pSkillSecGrid[i]->AddItemInEmpty(ipCtrlItem).x < 0)
					SAFE_DELETE(ipCtrlItem);
			}
			// Skill bar 2
			for (int i = 0; i < 12; ++i)
			{
				if (!m_pSkillSecGrid2[i])
					continue;

				if (((1 << i) & pMobData->LearnedSkill[1]) != 1 << i)
				{
					m_pSkillSecGrid2[i]->Empty();
					continue;
				}

				SGridControlItem* pSkillItem = m_pSkillSecGrid2[i]->GetItem(0, 0);
				sIndex = 12 * pMobData->Class + i + 5400;

				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = nullptr;
						continue;
					}
				}

				m_pSkillSecGrid2[i]->Empty();

				auto pStructItem = new STRUCT_ITEM;
				memset(pStructItem, 0, sizeof(STRUCT_ITEM));

				pStructItem->sIndex = sIndex;

				auto ipCtrlItem = new SGridControlItem(0, pStructItem, 0.0f, 0.0f);
				if (ipCtrlItem && m_pSkillSecGrid2[i]->AddItemInEmpty(ipCtrlItem).x < 0)
					SAFE_DELETE(ipCtrlItem);
			}
			// Skill bar 1
			for (int i = 0; i < 8; ++i)
			{
				unsigned int dwBit = 1 << (i + 24);
				if (!m_pGridSkillBelt)
					continue;

				auto pSkillItem = m_pGridSkillBelt->GetItem(i % 4, i / 4);
				sIndex = i + 5096;
				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					ItemIndex = pstOldItem->sIndex;
					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = 0;
						continue;
					}
				}

				auto pOldItem = m_pGridSkillBelt->PickupItem(i % 4, i / 4);
				if ((dwBit & pMobData->LearnedSkill[0]) == dwBit)
				{
				auto pItem = new STRUCT_ITEM;
				memset(pItem, 0, sizeof(STRUCT_ITEM));
				pItem->sIndex = i + 5096;

				auto pControlItem = new SGridControlItem(0, pItem, 0.0f, 0.0f);
				if (pControlItem)
				{
					if (!m_pGridSkillBelt->AddItem(pControlItem, i % 4, i / 4))
						SAFE_DELETE(pControlItem);
				}
				else
					SAFE_DELETE(pItem);
				}
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pOldItem)
					g_pCursor->m_pAttachedItem = 0;

				//TODO: check this strange bOld
				bool bOld = false;
				if (bOld != 1 && pOldItem)
				{
					SAFE_DELETE(pOldItem);
				}
			}
		}
		if (m_pSLPanel1 && !pMobData->Class)
			m_pSLPanel1->SetVisible(0);
		else if (m_pSLPanel1)
		{
			m_pSLPanel1->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSLPanel2 && !pMobData->Class)
			m_pSLPanel2->SetVisible(0);
		else if (m_pSLPanel2)
		{
			m_pSLPanel2->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSLPanel3 && !pMobData->Class)
			m_pSLPanel3->SetVisible(0);
		else if (m_pSLPanel3)
		{
			m_pSLPanel3->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSkillCover && !pMobData->Class)
			m_pSkillCover->SetVisible( 0);
		else if (m_pSkillCover)
		{
			m_pSkillCover->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSkillCover->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSkillCover->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSkillCover->m_GCPanel.nTextureIndex = 2;
		}

		switch (pMobData->Class)
		{
		case 0:
			m_pCISp1Caption->SetText(g_pMessageStringTable[242], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[243], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[244], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[245], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[107], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[108], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[109], 0);
			break;
		case 1:
			m_pCISp1Caption->SetText(g_pMessageStringTable[246], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[247], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[248], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[249], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[110], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[111], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[112], 0);
			break;
		case 2:
			m_pCISp1Caption->SetText(g_pMessageStringTable[250], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[251], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[252], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[253], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[113], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[114], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[115], 0);
			break;
		case 3:
			m_pCISp1Caption->SetText(g_pMessageStringTable[254], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[255], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[256], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[257], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[133], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[134], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[135], 0);
			break;
		}

		int nRate = pMobData->CurrentScore.Mastery[0];
		int nDamageValue = pMobData->CurrentScore.Attack;
		if (nRate > 100)
			nRate = 100;

		sprintf(szStr, "%d", nDamageValue);
		if (m_pDamage)
			m_pDamage->SetText(szStr, 0);

		int cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
		if (cSkillIndex >= 0 && cSkillIndex < 248)
		{
			SetMyHumanMagic();
			int nIdx = g_pObjectManager->m_stMobData.Equip[6].sIndex;
			int nWeather = g_nWeather;
			if (g_nWeather == 3)
				nWeather = 2;

			if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
				(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
				nWeather = 2;

			int nSkillDam = BASE_GetSkillDamage(cSkillIndex, pMobData, nWeather, GetWeaponDamage(),
				g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex);
			if (nSkillDam >= 0)
			{
				sprintf(szStr, "%d", nSkillDam);
				if (m_pSkillDam)
					m_pSkillDam->SetText(szStr, 0);
				if (m_pSkillDam)
					m_pSkillDam->SetTextColor(0xFFBBBBFF);
			}
			else
			{
				sprintf(szStr, "%d", -nSkillDam);
				if (m_pSkillDam)
					m_pSkillDam->SetText(szStr, 0);
				if (m_pSkillDam)
					m_pSkillDam->SetTextColor(0xFFBBBBFF);
			}
		}
		else
		{
			sprintf(szStr, "%d", 0);
			if (m_pSkillDam)
				m_pSkillDam->SetText(szStr, 0);
		}

		if (!m_pMyHuman->m_cOnlyMove)
			m_pMyHuman->SetSpeed(m_bMountDead);

		int nSpeedValue = (int)m_pMyHuman->m_fMaxSpeed;

		sprintf(szStr, "%d", nSpeedValue);
		if (m_pSpeed)
			m_pSpeed->SetText(szStr, 0);

		int nACValue = pMobData->CurrentScore.Defense;
		sprintf(szStr, "%d", nACValue);
		if (m_pDefence)
			m_pDefence->SetText(szStr, 0);

		int nDef = BASE_GetMobAbility(pMobData, 53) + (BASE_GetMobAbility(pMobData, 3) + pMobData->BaseScore.Defense);
		sprintf(szStr, "%d", nDef);
		int nSpeedClass = 0;

		if (m_pMyHuman->m_nClass == 26 && !m_pMyHuman->m_stLookInfo.FaceMesh)
		{
			nSpeedClass = 10;
			if (pMobData->LearnedSkill[0] & 0x20000)
				nSpeedClass = 20;
		}
		else if (m_pMyHuman->m_nClass == 26 && m_pMyHuman->m_stLookInfo.FaceMesh == 1)
			nSpeedClass = 0;
		else if (m_pMyHuman->m_nClass == 33)
		{
			nSpeedClass = 20;
			if (pMobData->LearnedSkill[0] & 0x200000)
				nSpeedClass = 40;
		}
		else if(m_pMyHuman->m_nClass == 40)
			nSpeedClass = 20;
		else if (m_pMyHuman->m_nClass == 63)
			nSpeedClass = 30;

		int nAttSpeedValue = (pMobData->CurrentScore.Mastery[2] / 10 + 10)
			* (m_pMyHuman->m_cSpeedUp - m_pMyHuman->m_cSpeedDown)
			+ pMobData->CurrentScore.Dex / 5
			+ nSpeedClass
			+ BASE_GetMobAbility(pMobData, 26)
			+ 100;

		if (m_pMyHuman->m_cFreeze == 1)
			nAttSpeedValue -= 30;

		sprintf(szStr, "%d%%", nAttSpeedValue);
		if (m_pAttackSpeed)
			m_pAttackSpeed->SetText(szStr, 0);
		int nCriticalValue = pMobData->CurrentScore.Critical;
		nCriticalValue *= 4;

		sprintf(szStr, "%d.%d%%", nCriticalValue / 10, nCriticalValue % 10);
		if (m_pCritical)
			m_pCritical->SetText(szStr, 0);

		sprintf(szStr, "%d", pMobData->CurrentScore.ResistHoly);
		if (m_pRegist1)
			m_pRegist1->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.ResistThunder);
		if (m_pRegist2)
			m_pRegist2->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.ResistFire);
		if (m_pRegist3)
			m_pRegist3->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.ResistIce);
		if (m_pRegist4)
			m_pRegist4->SetText(szStr, 0);

		//new char

		int PvpDam = BASE_GetMobAbility(pMobData, EF_HWORDGUILD);
		sprintf(szStr, "%d.%d%%", PvpDam / 10, PvpDam % 10);

		if (m_pPvpDamage)
			m_pPvpDamage->SetText(szStr, 0);

		int PvpAc = BASE_GetMobAbility(pMobData, EF_LWORDGUILD);
		sprintf(szStr, "%d.%d%%", PvpAc / 10, PvpAc % 10);

		if (m_pPvpAc)
			m_pPvpAc->SetText(szStr, 0);

		sprintf(szStr, "%d%%", ValueEffectSrver[0]);
		if (m_pPerfuDamage)
			m_pPerfuDamage->SetText(szStr, 0);

		sprintf(szStr, "%d%%", ValueEffectSrver[1]);
		if (m_pAbsDamage)
			m_pAbsDamage->SetText(szStr, 0);

		int Precision = BASE_GetMobAbility(pMobData, 103);
		sprintf(szStr, "%d.%d%%", Precision / 10, Precision % 10);

		if (m_pPrecision)
			m_pPrecision->SetText(szStr, 0);

		int ParryRate = BASE_GetMobAbility(pMobData, EF_PARRY);
		sprintf(szStr, "%d.%d%%", ParryRate / 10, ParryRate % 10);

		if (m_pParryRate)
			m_pParryRate->SetText(szStr, 0);

		sprintf(szStr, "%d", pMobData->CurrentScore.SaveMana);
		if (m_pSaveMana)
			m_pSaveMana->SetText(szStr, 0);

		sprintf(szStr, "%d", pMobData->CurrentScore.RegenHP);
		m_pRegenHP->SetText(szStr, 0);

		if (m_pRegenHP)
			m_pRegenHP->SetText(szStr, 0);

		sprintf(szStr, "%d", pMobData->CurrentScore.RegenMP);
		m_pRegenMP->SetText(szStr, 0);

		if (m_pRegenMP)
			m_pRegenMP->SetText(szStr, 0);

		sprintf(szStr, "%d.%d%%", nCriticalValue / 10, nCriticalValue % 10);
		if (m_pCriticalDamInc)
			m_pCriticalDamInc->SetText(szStr, 0);

		sprintf(szStr, "%d", BASE_GetMobAbility(pMobData, EF_RANGE));
		if (m_pAttackRange)
			m_pAttackRange->SetText(szStr, 0);

		sprintf(szStr, "%d", BASE_GetMobAbility(pMobData, 98)); //new
		if (m_pSkillDelayDec)
			m_pSkillDelayDec->SetText(szStr, 0);

		sprintf(szStr, "%d.%d%%", nCriticalValue / 10, nCriticalValue % 10);
		if (m_pCriticalNew)
			m_pCriticalNew->SetText(szStr, 0);

		sprintf(szStr, "%d%%", pMobData->CurrentScore.MagicAmp * 4);
		if (m_pAtkMagic)
			m_pAtkMagic->SetText(szStr, 0);

		sprintf(szStr, "%d%%", Exp);
		if (m_pBonusEXP)
			m_pBonusEXP->SetText(szStr, 0);

		sprintf(szStr, "%d%%", Drop);
		if (m_pBonusDROP)
			m_pBonusDROP->SetText(szStr, 0);

		auto labelcash = (SText*)m_pControlContainer->FindControl(3000069); // Cash label.
		sprintf(szStr, "%d", Cash);
		labelcash->SetText(szStr, 0);

		for (int i = 0; i < 32; ++i)
		{
			int nType = (signed int)m_pMyHuman->m_usAffect[i] >> 8;
			if (m_pAffectL[i])
			{
				if (nType >= 0 && nType < 50)
					m_pAffectL[i]->SetText(g_pAffectTable[nType], 0);
				else
					m_pAffectL[i]->SetText((char*)" ", 0);
			}
		}

		char szVal[32]{};

		for (int i = 0; i < 32; ++i)
		{
			if (m_pMyHuman->m_stAffect[i].Type > 0)
			{
				if (m_pMyHuman->m_stAffect[i].Time < 1000000)
				{
					sprintf(szVal, "%5d", m_pMyHuman->m_stAffect[i].Time);
				}
				else
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
						sprintf(szVal, g_pMessageStringTable[291], freeDay);
					}
				}
				if (m_pAffectL[i] && !i)
				{
					int sTime = m_pMyHuman->m_stAffect[0].Time;
					if (sTime >= 7375)
					{
						m_pAffect[i]->SetTextColor(0xFFFFFFFF);
					}
					else
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
				}
				if (m_pMiniPanel && m_pAffectIcon[i])
				{
					if (m_pMyHuman->m_stAffect[i].Type < 50)
						m_pAffectIcon[i]->m_GCPanel.nTextureIndex = g_AffectSkillType[m_pMyHuman->m_stAffect[i].Type];
					else
						m_pAffectIcon[i]->m_GCPanel.nTextureIndex = 0;
					m_pAffectIcon[i]->m_GCPanel.nLayer = 29;
				}
			}
			else
			{
				if (m_pAffectL[i])
					m_pAffectL[i]->SetVisible(0);
				if (m_pAffectIcon[i] && m_pMiniPanel)
				{
					m_pAffectIcon[i]->SetVisible(0);
					m_dwAffectBlinkTime[i] = 0;
				}
			}
		}
		if (m_pCargoCoin)
		{
			sprintf(szVal, "%10d", g_pObjectManager->m_nCargoCoin);
			m_pCargoCoin->m_cComma = 1;
			m_pCargoCoin->SetText(szVal, 0);
			if (m_pMyCargoCoin)
				m_pMyCargoCoin->SetText(szVal, 0);
		}

		if (m_pKingDomFlag)
		{
			if (!m_pMyHuman->m_pMantua || m_pMyHuman->m_pMantua->m_Look.Skin0 == 19)
				m_pKingDomFlag->SetVisible(0);
			else
			{
				char szTemp[256]{};
				m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pMyHuman->m_pMantua->m_Look.Skin0;
				if ((m_pMyHuman->m_sHelmIndex == 3503 || m_pMyHuman->m_sHelmIndex == 3504 ||
					m_pMyHuman->m_sHelmIndex == 3505 || m_pMyHuman->m_sHelmIndex == 3506) &&
					m_pMyHuman->m_pMantua->m_Look.Skin0 == 2)
				{
					m_pKingDomFlag->m_GCPanel.nTextureIndex = 33;
				}

				m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pMyHuman->UnSetCitizenMantle(m_pKingDomFlag->m_GCPanel.nTextureIndex);
				if (m_pFlagDescText[0])
				{
					const int capeIndex = pMobData->Equip[15].sIndex;
					m_pFlagDescText[0]->SetText(capeIndex > 0 && capeIndex < MAX_ITEMLIST
						? g_pItemList[capeIndex].Name : nullptr, 0);
				}

				sprintf(szTemp, "%s %d", g_pMessageStringTable[80], BASE_GetItemAbility(&pMobData->Equip[15], 3));
				if (m_pFlagDescText[1])
					m_pFlagDescText[1]->SetText(szTemp, 0);

				sprintf(szTemp, "%s %d", g_pMessageStringTable[81], BASE_GetItemAbility(&pMobData->Equip[15], 4));
				if (m_pFlagDescText[2])
					m_pFlagDescText[2]->SetText(szTemp, 0);
				m_pKingDomFlag->SetVisible(1);
			}
		}

		if (m_pMyHuman->m_cMount == 1 &&
			(m_pMyHuman->m_nMountSkinMeshType == 31	|| m_pMyHuman->m_nMountSkinMeshType == 40 ||
			 m_pMyHuman->m_nMountSkinMeshType == 20 && m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
		{
			if (m_pBtnMountRun)
			{
				m_pBtnMountRun->m_bEnable = 1;
				m_pBtnMountRun->m_bSelected = g_bRunning;
			}
		}
		else if (m_pBtnMountRun)
		{
			m_pBtnMountRun->m_bEnable = 0;
		}

		SetPosPKRun();
	}
}

void TMFieldScene::InitBoard()
{
	if (m_pGMsgPanel)
	{
		SListBoxBoardItem* pBoardItem = new SListBoxBoardItem((char*)"0",
			g_pMessageStringTable[176],
			g_pMessageStringTable[177],
			g_pMessageStringTable[178],
			g_pMessageStringTable[179],
			g_pMessageStringTable[180],
			0x5588FFFFu,
			1);

		pBoardItem->SetControlID(824);
		pBoardItem->SetPos(4.0f, 28.0f);
		m_pGMsgListPanel->AddChild(pBoardItem);

		m_pButtonBox = new SButtonBox(4.0f, 320.0f, 500.0f, 16.0f, 1, 10, 1, 1, 10, 1, 10);
		m_pButtonBox->SetControlID(826);
		m_pButtonBox->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);

		m_pGMsgListPanel->AddChild(m_pButtonBox);
		m_pGMsgPanel->SetVisible(0);
	}

	m_pHelpPanel = (SPanel*)m_pControlContainer->FindControl(864);

	if (m_pHelpPanel)
	{
		m_pHelpText = (SText*)m_pControlContainer->FindControl(865);
		m_pHelpButton[0] = (SButton*)m_pControlContainer->FindControl(867);
		m_pHelpList[0] = (SListBox*)m_pControlContainer->FindControl(868);
		m_pHelpButton[1] = (SButton*)m_pControlContainer->FindControl(869);
		m_pHelpList[1] = (SListBox*)m_pControlContainer->FindControl(870);
		m_pHelpButton[2] = (SButton*)m_pControlContainer->FindControl(871);
		m_pHelpList[2] = (SListBox*)m_pControlContainer->FindControl(872);
		m_pHelpButton[3] = (SButton*)m_pControlContainer->FindControl(873);
		m_pHelpList[3] = (SListBox*)m_pControlContainer->FindControl(874);
		m_pHelpMemo = (SButton*)m_pControlContainer->FindControl(875);
		m_pHelpSummon = (SButton*)m_pControlContainer->FindControl(878);

		m_pHelpSummon->m_cAlwaysAlt = 1;
		m_pHelpSummon->m_cBlink = 1;

		m_pHelpSummon->m_pAltText->SetPos(-26.0f * RenderDevice::m_fWidthRatio,
			-17.0f * RenderDevice::m_fHeightRatio);
		m_pHelpMemo->m_pAltText->SetPos(-26.0f * RenderDevice::m_fWidthRatio,
			-17.0f * RenderDevice::m_fHeightRatio);

		m_pHelpSummon->m_cAlwaysAlt = 0;
		m_pHelpMemo->m_nPosX = (float)(665.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pHelpMemo->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(105.0f * RenderDevice::m_fHeightRatio);
		m_pHelpMemo->m_cBlink = 1;
		m_pHelpSummon->m_nPosX = (float)(616.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pHelpSummon->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(90.0f * RenderDevice::m_fHeightRatio);

		LoadMsgText(m_pHelpList[0], (char*)"UI\\interface.txt");
		LoadMsgText(m_pHelpList[1], (char*)"UI\\command.txt");
		LoadMsgText(m_pHelpList[2], (char*)"UI\\etc.txt");

		if (m_pHelpMemo)
			m_pHelpMemo->SetVisible(0);
		if (m_pHelpList[0])
			m_pHelpList[0]->SetVisible(0);
		if (m_pHelpList[1])
			m_pHelpList[1]->SetVisible(0);
		if (m_pHelpList[2])
			m_pHelpList[2]->SetVisible(0);
		if (m_pHelpList[3])
			m_pHelpList[3]->SetVisible(0);

		if (m_pHelpSummon)
			m_pHelpSummon->SetVisible(0);

		m_pHelpPanel->SetVisible(0);
		m_pHelpPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pHelpPanel->m_nWidth * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pHelpPanel->m_nHeight * 0.6f));

		m_pHelpInterface = (SPanel*)m_pControlContainer->FindControl(6067);
		for (int i = 0; i < 3; ++i)
		{
			m_pHelpInterfacePanel[i] = (SPanel*)m_pControlContainer->FindControl(i + 6064);
			m_pHelpInterfaceList[i] = (SListBox*)m_pControlContainer->FindControl(i + 6074);
			if (m_pHelpInterfacePanel[i])
				m_pHelpInterfacePanel[i]->SetVisible(0);
			if (m_pHelpInterfaceList[i])
				m_pHelpInterfaceList[i]->SetVisible(0);
		}
		if (m_pHelpInterfaceList[0])
			LoadMsgText(m_pHelpInterfaceList[0], (char*)"UI\\interface1.txt");
		if (m_pHelpInterfaceList[1])
			LoadMsgText(m_pHelpInterfaceList[1], (char*)"UI\\interface2.txt");
		if (m_pHelpInterfaceList[2])
			LoadMsgText(m_pHelpInterfaceList[2], (char*)"UI\\interface3.txt");
		m_nCurrInterfacePanelIndex = 0;
		SelectHelpTab(0);
	}

	m_pQuestPanel = (SPanel*)m_pControlContainer->FindControl(1054256);
	if (m_pQuestPanel)
	{
		m_pQuestQuitBtn = (SButton*)m_pControlContainer->FindControl(B_QUEST_QUIT);
		m_pQuestButton[0] = (SButton*)m_pControlContainer->FindControl(1054259);
		m_pQuestList[0] = (SListBox*)m_pControlContainer->FindControl(1054260);
		m_pQuestContentList[0] = (SListBox*)m_pControlContainer->FindControl(1054261);
		m_pQuestButton[1] = (SButton*)m_pControlContainer->FindControl(1054262);
		m_pQuestList[1] = (SListBox*)m_pControlContainer->FindControl(1054263);
		m_pQuestContentList[1] = (SListBox*)m_pControlContainer->FindControl(1054264);
		m_pQuestButton[2] = (SButton*)m_pControlContainer->FindControl(1054265);
		m_pQuestList[2] = (SListBox*)m_pControlContainer->FindControl(1054266);
		m_pQuestContentList[2] = (SListBox*)m_pControlContainer->FindControl(1054267);
		m_pQuestButton[3] = (SButton*)m_pControlContainer->FindControl(1054268);
		m_pQuestList[3] = (SListBox*)m_pControlContainer->FindControl(1054269);
		m_pQuestContentList[3] = (SListBox*)m_pControlContainer->FindControl(1054271);

		for (int questIndex = 0; questIndex < 4; ++questIndex)
		{
			if (m_pQuestList[questIndex])
			{
				m_pQuestList[questIndex]->SetVisible(0);
				m_pQuestList[questIndex]->m_bSelectEnable = 0;
				m_pQuestList[questIndex]->SetEventListener(m_pControlContainer);
			}
			if (m_pQuestContentList[questIndex])
				m_pQuestContentList[questIndex]->SetVisible(0);
		}

		m_pQuestPanel->SetVisible(0);
		PositionCompatQuestPanel();

		memset(m_pLevelQuest, 0, sizeof(m_pLevelQuest));

		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects.txt", 97);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects2.txt", 98);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects3.txt", 99);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects4.txt", 101);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestMessage.txt", 100);

		m_pQuestMemo = (SButton*)m_pControlContainer->FindControl(1054273);
		if (m_pQuestMemo)
		{
			m_pQuestMemo->m_cAlwaysAlt = 1;// blinking button
			m_pQuestMemo->m_cBlink = 1;
			if (m_pQuestMemo->m_pAltText)
				m_pQuestMemo->m_pAltText->SetPos(-20.0f, -10.0f);

			if (m_pMainInfo1)
			{
				m_pQuestMemo->m_nPosX = (float)(300.0f * RenderDevice::m_fWidthRatio)
					+ m_pMainInfo1->m_nPosX;//adicionado
				m_pQuestMemo->m_nPosY = m_pMainInfo1->m_nPosY
					- (float)(480.0f * RenderDevice::m_fHeightRatio);
			}
		}

		if (g_pObjectManager && !g_pObjectManager->m_stMobData.CurrentScore.Level)
		{
			const STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
			static const char* const questSubjectFiles[4] =
			{
				"UI\\QuestSubjects.txt",
				"UI\\QuestSubjects2.txt",
				"UI\\QuestSubjects3.txt",
				"UI\\QuestSubjects4.txt"
			};
			for (int questIndex = 0; questIndex < 4; ++questIndex)
			{
				if (m_pQuestList[questIndex])
				{
					LoadMsgText3(
						m_pQuestList[questIndex],
						questSubjectFiles[questIndex],
						pMobData->CurrentScore.Level + 1,
						pMobData->Equip[0].sIndex % 10);
				}
			}

			if (m_pQuestMemo)
				m_pQuestMemo->SetVisible(1);

			char szStr[128]{};
			unsigned int dwCol = 0xFFAAAAFF;
			if (m_pLevelQuest[pMobData->CurrentScore.Level] == 100)
				dwCol = LoadMsgText4(
					szStr,
					sizeof szStr,
					(char*)"UI\\QuestMessage.txt",
					pMobData->CurrentScore.Level + 1,
					pMobData->Equip[0].sIndex % 10);
			else
				sprintf(szStr, g_pMessageStringTable[307]);

			SListBoxItem* pChatItem = new SListBoxItem(szStr, dwCol, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);

			if (pChatItem && m_pChatList)
				m_pChatList->AddItem(pChatItem);
		}
		else if (m_pQuestMemo)
		{

			m_pQuestMemo->SetVisible(0);
		}
	}

	m_pPotalPanel = (SPanel*)m_pControlContainer->FindControl(12544);
	m_pPotalList = (SListBox*)m_pControlContainer->FindControl(12545);
	m_pPotalText = (SText*)m_pControlContainer->FindControl(12551);
	m_pPotalText1 = (SText*)m_pControlContainer->FindControl(12552);
	m_pPotalText2 = (SText*)m_pControlContainer->FindControl(12553);
	m_pPotalText3 = (SText*)m_pControlContainer->FindControl(12560);

	m_pPotalPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pQuestPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pQuestPanel->m_nHeight * 0.6f));

	if (g_pApp->m_dwScreenWidth <= 1024)
	{//adicionado
	m_pPotalList->m_nWidth = BASE_ScreenResize(312.0f);
	}

	if (m_pPotalList)
	{
		m_pPotalList->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
	}

	if (m_pPotalPanel)
		m_pPotalPanel->SetVisible(0);

	for (int j = 0; j < 4; ++j)
	{
		m_pQuestList[j]->SetSize(200.0f, m_pQuestList[j]->m_nHeight);
		m_pQuestContentList[j]->SetSize(300.0f, m_pQuestContentList[j]->m_nHeight);
	}
}

int TMFieldScene::LoadMsgText(SListBox* pListBox, char* szFileName)
{
	FILE* fp = nullptr;
	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;
	if (!pListBox)
		return 0;

	char szCol[7]{};

	char szTemp[256]{};
	char szText[256]{};

	unsigned int dwCol = 0;

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		strncpy(szCol, szTemp, 6);
		sscanf(szCol, "%x", &dwCol);

		auto szRet = strstr(szTemp, "\n");
		if (szRet)
			szRet[0] = 0;

		if (szTemp[6] == ' ')
		{
			sprintf(szText, "%s", &szTemp[6]);
			pListBox->AddItem(new SListBoxItem(szText, dwCol | 0xFF000000, 0.0f, 0.0f, 400.0f, 16.0f, 0, 0x77777777, 1u, 0));
		}
	}
	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

void TMFieldScene::VisibleInputPass()
{
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pText = (SText*)m_pControlContainer->FindControl(65888);
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889);
	if (pText && pEdit)
	{
		m_nCoinMsgType = 11;
		pText->SetText(g_pMessageStringTable[405], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		pInputGoldPanel->SetVisible(1);

		auto pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);
		if (pInputBG2)
			pInputBG2->SetVisible(1);

		pEdit->m_nMaxStringLen = 12;
	}
}

void TMFieldScene::SetButtonTextXY(SButton* pButton)
{
	if (m_pSystemPanel && pButton)
	{
		TMVector2 vecXY{};
		auto vec = pButton->GetPos();

		int nLen = strlen(pButton->m_GCPanel.pFont->m_szString);
		if (RenderDevice::m_fWidthRatio == 0.80000001f)
		{
			vecXY.x = 43.0f - ((float)(nLen - 1) * 3.0f);
			vecXY.y = 2.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 1.28f)
		{
			vecXY.x = 66.0f - ((float)(nLen - 1) * 4.0f);
			vecXY.y = 5.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 1.6f)
		{
			vecXY.x = 75.0f - ((float)(nLen - 1) * 5.0f);
			vecXY.y = 5.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 2.0f)
		{
			vecXY.x = 90.0f - ((float)(nLen - 1) * 5.0f);
			vecXY.y = 5.0f;
		}

		pButton->m_GCPanel.pFont->m_nPosX = (int)((m_pSystemPanel->m_nPosX + vec.x) + vecXY.x);
		pButton->m_GCPanel.pFont->m_nPosY = (int)((m_pSystemPanel->m_nPosY + vec.y) + vecXY.y);
	}
}

void TMFieldScene::UpdateCompatObservedAffects()
{
	// Reproject every frame, including empty/hidden states. Never borrow the
	// render-mesh m_TargetAffect cache, which can still describe the previous mob.
	TMHuman* target = m_pMouseOverHuman;
	SProgressBar* bar = target ? target->m_pTitleProgressBar : nullptr;
	const auto* targetWords = bar && bar->IsVisible() ? observed_affect_ui::InViewAffects(target) : nullptr;
	const float targetSize = 18.0f * RenderDevice::m_fHeightRatio;
	const int targetColumns = bar ? max(1, static_cast<int>(bar->m_nWidth / (targetSize + 1.0f))) : 1;
	observed_affect_ui::Project(m_pTargetAffectIcon, targetWords,
		g_AffectSkillType, 41, bar ? bar->m_nPosX : 0.0f,
		bar ? bar->m_nPosY + bar->m_nHeight + 2.0f : 0.0f, targetSize, targetColumns);

	const bool showParty = m_pPartyPanel && m_pPartyPanel->IsVisible()
		&& m_pPartyList && m_pPartyList->IsVisible() && m_pPartyList->m_nVisibleCount > 0;
	float listX = 0.0f, listY = 0.0f;
	if (showParty)
	{
		// Include all resource-tree parents; list coordinates are not screen coordinates.
		for (auto* control = static_cast<SControl*>(m_pPartyList); control;
			control = static_cast<SControl*>(control->m_pTop))
		{
			listX += control->m_nPosX;
			listY += control->m_nPosY;
		}
	}
	const float rowHeight = showParty ? m_pPartyList->m_nHeight / m_pPartyList->m_nVisibleCount : 0.0f;
	// One readable row aligned with the member name, using the row's full height.
	const float partySize = min(18.0f * RenderDevice::m_fHeightRatio, max(1.0f, rowHeight - 2.0f));
	for (int row = 0; row < 13; ++row)
	{
		const unsigned short* words = nullptr;
		if (showParty && row < m_pPartyList->m_nVisibleCount)
		{
			const int index = m_pPartyList->m_nStartItemIndex + row;
			if (index >= 0 && index < m_pPartyList->m_nNumItem)
			{
				auto* item = static_cast<SListBoxPartyItem*>(m_pPartyList->m_pItemList[index]);
				// Never retain a party snapshot after its entity leaves the view.
				auto* human = item ? g_pObjectManager->GetHumanByID(item->m_dwCharID) : nullptr;
				words = observed_affect_ui::InViewAffects(human);
			}
		}
		observed_affect_ui::Project(m_pPartyAffectIcon[row], words, g_AffectSkillType, 41,
			listX + (showParty ? m_pPartyList->m_nWidth : 0.0f) + 3.0f,
			listY + static_cast<float>(row) * rowHeight, partySize, 32);
	}
}

int TMFieldScene::Affect_Main(unsigned int dwServerTime)
{
	if (!m_pMyHuman || !m_pMiniPanel)
		return 0;

	if (m_bCompatFieldScene)
	{
		// FUN_004770ad defines the stock row geometry: only the first sixteen
		// STRUCT_AFFECT entries are visualized, types 0..40 are accepted, and the
		// row begins three pixels after HUD anchor 5723 at y=5.  The source's
		// compatibility scene explicitly recreates these panels over the classic
		// FieldScene2 tree, so applying the imported UI2-only gate here would make
		// the original 7.48 CLASSIC=1 configuration hide every buff icon.

		SControl* pNativeAffectAnchor = m_pControlContainer
			? m_pControlContainer->FindControl(5723)
			: nullptr;
		if (!pNativeAffectAnchor)
			return 0;

		if (m_pAffectDesc)
		{
			m_pAffectDesc->SetVisible(0);
			m_pAffectDesc->SetText((char*)"", 0);
		}

		int visibleCount = 0;
		for (int affectIndex = 0; affectIndex < 16; ++affectIndex)
		{
			SPanel* pIcon = m_pAffectIcon[affectIndex];
			if (!pIcon)
				continue;

			const unsigned char affectType =
				static_cast<unsigned char>(m_pMyHuman->m_stAffect[affectIndex].Type);
			const int nativeTime = m_pMyHuman->m_stAffect[affectIndex].Time;
			const bool active = affectType < 41
				&& ((affectIndex != 0 && affectType != 0) || (affectIndex == 0 && nativeTime > 0));
			if (!active)
			{
				pIcon->SetVisible(0);
				m_dwAffectBlinkTime[affectIndex] = 0;
				continue;
			}

			const int secondsLeft = nativeTime * 8
				- static_cast<int>((dwServerTime - m_dwStartAffectTime[affectIndex]) / 1000);
			pIcon->m_GCPanel.nTextureIndex = g_AffectSkillType[affectType];
			pIcon->m_GCPanel.nLayer = 29;
			pIcon->SetPos(pNativeAffectAnchor->m_nPosX + pNativeAffectAnchor->m_nWidth
				+ 3.0f + static_cast<float>(visibleCount) * (pIcon->m_nWidth + 3.0f), 5.0f);
			++visibleCount;

			if (pIcon->m_bOver == 1 && m_pAffectDesc)
			{
				char timeText[128]{};
				char affectText[128]{};
				GetTimeString(timeText, secondsLeft, nativeTime, affectIndex);
				sprintf_s(affectText, "%s : %s", g_pAffectTable[affectType], timeText);
				m_pAffectDesc->SetText(affectText, 0);
				m_pAffectDesc->SetPos(pIcon->m_nPosX, pIcon->m_nPosY + 40.0f);
				m_pAffectDesc->SetVisible(1);
			}

			if (!m_dwAffectBlinkTime[affectIndex] && secondsLeft <= 10 && secondsLeft > 1)
				m_dwAffectBlinkTime[affectIndex] = dwServerTime;
			else if (!m_dwAffectBlinkTime[affectIndex] || secondsLeft >= 10)
				pIcon->SetVisible(1);
			else
			{
				const unsigned int blinkDelay = 3 * nativeTime * nativeTime + 200;
				if (dwServerTime - m_dwAffectBlinkTime[affectIndex] >= blinkDelay)
				{
					pIcon->SetVisible(pIcon->m_bVisible == 0);
					m_dwAffectBlinkTime[affectIndex] = dwServerTime;
				}
			}
			if (nativeTime >= 10)
			{
				pIcon->SetVisible(1);
				m_dwAffectBlinkTime[affectIndex] = 0;
			}
		}
		return 1;
	}

	int i = 0;
	int nAvailCount = 0;
	m_pAffectDesc->SetVisible(0);
	m_pAffectDesc->SetText((char*)"", 0);
	bool bomb = false;
	for (int i = 0; i < 32; ++i)
	{
		if ((unsigned char)m_pMyHuman->m_stAffect[i].Type <= 50)
		{
			if (!m_pAffectIcon[i]->IsVisible())
				m_pAffect[i]->SetVisible(0);

			int sTime = 8 * m_pMyHuman->m_stAffect[i].Time - (dwServerTime - m_dwStartAffectTime[i]) / 1000;

			char szVal[128]{};
			GetTimeString(szVal, sTime, m_pMyHuman->m_stAffect[i].Time, i);
			int nTime = m_pMyHuman->m_stAffect[i].Time;

			if (m_pAffectIcon[i] && i && (unsigned char)m_pMyHuman->m_stAffect[i].Type > 0 || !i && nTime > 0)
			{
				m_pAffectIcon[i]->m_nPosX = BASE_ScreenResize(30.0f);
				m_pAffectIcon[i]->m_nPosX = ((float)(BASE_ScreenResize(1.0f) + m_pAffectIcon[i]->m_nWidth) * (float)(nAvailCount + 1))
					+ m_pAffectIcon[i]->m_nPosX;
				m_pAffectIcon[i]->m_nPosY = BASE_ScreenResize(80.0f);

				m_pAffectIcon[i]->m_nPosY = 60.0f * RenderDevice::m_fWidthRatio;// skill position adjustment
				m_pAffectIcon[i]->m_nPosY = 80.0f * RenderDevice::m_fHeightRatio;

				if (m_pAffectIcon[i]->m_bOver == 1)
				{
					if (m_pAffectL[i])
					{
						m_pAffect[i]->SetVisible(0);
						m_pAffectL[i]->SetVisible(0);
					}

					char strAffectString[128]{};
					if (m_pMyHuman->m_stAffect[i].Type == 8)
					{
						auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
						int efvalue = m_pMyHuman->m_stAffect[i].Value;
						int CheckBit = 0;
						int Cnt = 0;
						for (int k = 0; k < 10; ++k)
						{
							CheckBit = 1 << k;
							if ((1 << k) & efvalue)
							{
								sprintf(strAffectString, "%s %s", strAffectString, g_pAffectSubTable[k]);
								++Cnt;
							}
						}

						int slen = strlen(strAffectString) + 4;
						sprintf(strAffectString, "%s : %s", strAffectString, szVal);
						m_pAffectDesc->SetText(strAffectString, 0);
						m_pAffectDesc->m_nPosX = m_pAffectIcon[i]->m_nPosX;
						if ((float)(m_pAffectDesc->m_nWidth + 60.0f) + m_pAffectDesc->m_nPosX > (float)g_pDevice->m_dwScreenWidth)
							m_pAffectDesc->m_nPosX = (float)g_pDevice->m_dwScreenWidth - (float)(m_pAffectDesc->m_nWidth + 60.0f);

						m_pAffectDesc->m_nPosY = m_pAffectIcon[i]->m_nPosY + 40.0f;
						m_pAffectDesc->SetVisible(1);
					}
					else
					{
						sprintf(strAffectString, "%s : %s", g_pAffectTable[(unsigned char)m_pMyHuman->m_stAffect[i].Type], szVal);
						m_pAffectDesc->SetText(strAffectString, 0);
						m_pAffectDesc->m_nPosX = m_pAffectIcon[i]->m_nPosX;
						if ((float)(m_pAffectDesc->m_nWidth + 60.0f) + m_pAffectDesc->m_nPosX > (float)g_pDevice->m_dwScreenWidth)
							m_pAffectDesc->m_nPosX = (float)g_pDevice->m_dwScreenWidth - (float)(m_pAffectDesc->m_nWidth + 60.0f);
						m_pAffectDesc->m_nPosY = m_pAffectIcon[i]->m_nPosY + 40.0f;
						m_pAffectDesc->SetVisible(1);
					}
				}
				else
				{
					int len = strlen(m_pAffect[i]->m_Font.m_szString);
					int test = 1;
					for (int l = 0; l < len; ++l)
					{
						if (!isdigit(m_pAffect[i]->m_Font.m_szString[l]) && m_pAffect[i]->m_Font.m_szString[l] != ' ')
							test = 0;
					}

					m_pAffect[i]->m_nPosX = m_pAffectIcon[i]->m_nPosX + (test ? -8.0f : 3.0f);
					if ((float)(m_pAffect[i]->m_nWidth + 60.0f) + m_pAffect[i]->m_nPosX > (float)g_pDevice->m_dwScreenWidth)
						m_pAffect[i]->m_nPosX = (float)g_pDevice->m_dwScreenWidth - (float)(m_pAffect[i]->m_nWidth + 60.0f);

					m_pAffect[i]->m_nPosY = m_pAffectIcon[i]->m_nPosY + 24.0f;
					m_pAffect[i]->SetVisible(1);
				}

				++nAvailCount;
				if (!m_dwAffectBlinkTime[i] && sTime <= 10 && sTime > 1)
					m_dwAffectBlinkTime[i] = dwServerTime;
				else if (!m_dwAffectBlinkTime[i] || sTime >= 10)
					m_pAffectIcon[i]->SetVisible(1);
				else
				{
					unsigned int dwDelay = 3 * nTime * nTime + 200;
					if (dwServerTime - m_dwAffectBlinkTime[i] >= dwDelay)
					{
						m_pAffectIcon[i]->SetVisible(m_pAffectIcon[i]->m_bVisible == 0);
						m_dwAffectBlinkTime[i] = dwServerTime;
					}
				}
				if (nTime >= 10)
				{
					m_pAffectIcon[i]->SetVisible(1);
					m_dwAffectBlinkTime[i] = 0;
				}
			}
		}
	}

	nAvailCount = 0;
	int YnAvailCount = 0;
	int index = 0;

	if (m_pMyHuman->m_pTitleProgressBar)
	{
		for (int i = 0; i < 32; ++i)
		{
			m_pTargetAffectIcon[i]->m_GCPanel.nTextureIndex = -1;
			index = (int)m_TargetAffect[i] >> 8;
			if (index == 0)
			{
				m_pTargetAffectIcon[i]->m_GCPanel.nTextureIndex = -1;
			}
			else
			{
				m_pTargetAffectIcon[i]->m_GCPanel.nTextureIndex = g_AffectSkillType[index];
				++nAvailCount;

				auto vecTitleProgressBar = m_pMyHuman->m_pTitleProgressBar->GetPos();
				m_pTargetAffectIcon[i]->m_nPosX = vecTitleProgressBar.x
					- (float)(m_pTargetAffectIcon[i]->m_nWidth * 2.0f);
				m_pTargetAffectIcon[i]->m_nPosX = ((float)(BASE_ScreenResize(1.0f) + m_pTargetAffectIcon[i]->m_nWidth) * (float)(nAvailCount + 1))
					+ m_pTargetAffectIcon[i]->m_nPosX;
				m_pTargetAffectIcon[i]->m_nPosY = ((float)(m_pMyHuman->m_pTitleProgressBar->m_nHeight * 2.0f) / 3.0f) + vecTitleProgressBar.y;

				if (nAvailCount > 9)
				{
					m_pTargetAffectIcon[i]->m_nPosY = m_pTargetAffectIcon[i]->m_nPosY + m_pTargetAffectIcon[i]->m_nHeight;
					m_pTargetAffectIcon[i]->m_nPosX = vecTitleProgressBar.x - (float)(m_pTargetAffectIcon[i]->m_nWidth * 2.0f);
					m_pTargetAffectIcon[i]->m_nPosX = ((float)(BASE_ScreenResize(1.0f)
						+ m_pTargetAffectIcon[i]->m_nWidth)
						* (float)(++YnAvailCount + 1))
						+ m_pTargetAffectIcon[i]->m_nPosX;
				}
			}

			m_pTargetAffectIcon[i]->m_GCPanel.nLayer = 29;
		}
	}

	int j = 0;
	index = 0;
	m_pPartyAffectText->SetVisible(0);
	auto PartyExit = (SButton*)m_pControlContainer->FindControl(475139);
	if (PartyExit)
	{
		PartyExit->SetVisible(1);
		float size = (((float)(m_pPartyList->m_nNumItem - m_pPartyList->m_nStartItemIndex)
			* m_pPartyList->m_nHeight)
			/ (float)m_pPartyList->m_nVisibleCount)
			+ 30.0f;

		auto vecPartyExitPos = PartyExit->GetPos();
		PartyExit->SetPos(vecPartyExitPos.x, size);

		m_pPartyAutoButton->SetVisible(0);
		m_pPartyAutoText->SetVisible(0);

		auto partyback = (SPanel*)m_pControlContainer->FindControl(7602196);
		if (partyback)
			partyback->SetVisible(0);
		if (m_pPartyList->m_nNumItem >= 1)
		{
			m_pPartyPanel->SetSize(m_pPartyPanel->m_nWidth,	(float)m_pPartyList->m_nNumItem * 20.0f);
		}
		else
		{
			PartyExit->SetVisible(0);
			m_pPartyAutoButton->SetVisible(1);
			m_pPartyAutoText->SetVisible(1);
			if (partyback)
				partyback->SetVisible(1);
			if (m_bAutoParty)
				m_pPartyAutoButton->m_GCPanel.bVisible = 1;
			else
				m_pPartyAutoButton->m_GCPanel.bVisible = 0;

			m_pPartyPanel->SetSize(m_pPartyPanel->m_nWidth, 50.0f);
		}
	}
	for (i = 0; i < 13; ++i)
	{
		for (j = 0; j < 32; ++j)
		{
			m_pPartyAffectIcon[i][j]->m_GCPanel.nTextureIndex = -1;
			m_pPartyAffectIcon[i][j]->m_GCPanel.nLayer = 29;
		}
	}

	for (i = 0; i < m_pPartyList->m_nNumItem; ++i)
	{
		auto pPartyItem = (SListBoxPartyItem*)m_pPartyList->m_pItemList[i];
		auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
		nAvailCount = 0;
		YnAvailCount = 0;

		for (j = 0; j < 32; ++j)
		{
			if (pHuman)
			{
				index = (signed int)pHuman->m_usAffect[j] >> 8;

				if (!index)
				{
					m_pPartyAffectIcon[i][j]->m_GCPanel.nTextureIndex = -1;
				}
				else
				{
					m_pPartyAffectIcon[i][j]->m_GCPanel.nTextureIndex = g_AffectSkillType[index];
					++nAvailCount;

					auto vecPartyListPos = m_pPartyList->GetPos();
					m_pPartyAffectIcon[i][j]->m_nPosX = (float)(vecPartyListPos.x + m_pPartyList->m_nWidth)
						- 20.0f;
					m_pPartyAffectIcon[i][j]->m_nPosX = ((BASE_ScreenResize(1.0f) + m_pPartyAffectIcon[i][j]->m_nWidth)
						* (float)(nAvailCount + 1))
						+ m_pPartyAffectIcon[i][j]->m_nPosX;

					m_pPartyAffectIcon[i][j]->m_nPosY = (float)((float)((float)(i
						- m_pPartyList->m_nStartItemIndex)
						* m_pPartyList->m_nHeight)
						/ (float)m_pPartyList->m_nVisibleCount)
						+ 12.0f;

					if (nAvailCount > 10)
					{
						m_pPartyAffectIcon[i][j]->m_nPosX = (float)(vecPartyListPos.x + m_pPartyList->m_nWidth)
							- 20.0f;
						m_pPartyAffectIcon[i][j]->m_nPosX = ((BASE_ScreenResize(1.0f)
							+ m_pPartyAffectIcon[i][j]->m_nWidth)
							* (float)(++YnAvailCount + 1))
							+ m_pPartyAffectIcon[i][j]->m_nPosX;
						m_pPartyAffectIcon[i][j]->m_nPosY = (((float)(i
							- m_pPartyList->m_nStartItemIndex)
							* m_pPartyList->m_nHeight)
							/ (float)m_pPartyList->m_nVisibleCount)
							+ 22.0f;
					}
					if (m_pPartyAffectIcon[i][j]->IsOver() == 1)
					{
						m_pPartyAffectText->SetRealPos(m_pPartyAffectIcon[i][j]->m_nPosX - m_pPartyAffectIcon[i][j]->m_nWidth,
							m_pPartyAffectIcon[i][j]->m_nPosY - (float)(m_pPartyAffectIcon[i][j]->m_nHeight * 1.5f));

						m_pPartyAffectText->SetText(g_pAffectTable[index], 0);
						m_pPartyAffectText->SetVisible(1);
					}
				}
			}
		}
	}

	return 1;
}

unsigned int TMFieldScene::GetLascDescParamId()
{
	for (int i = NUM_ITEM_DESC_PARAMS - 1; i >= 0; i--)
	{
		if (m_pParamText[i] && m_pParamText[i]->GetText()[0] != 0)
			return i;
	}

	return 0;
}

int TMFieldScene::StrByteCheck(char* szString)
{
	int value = 0;
	int byteCheck = 0;
	for (size_t i = 0; i < strlen(szString); ++i)
	{
		if (szString[i] >= 'A' && szString[i] <= 'z')
			++value;
		else if (byteCheck == 1)
		{
			++value;
			byteCheck = 0;
		}
		else
			byteCheck = 1;
	}

	return value;
}
