#include "pch.h"
#include "UiLayout.h"
#include "WeatherPacket.h"
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
#include "../../ui/NativeScoreHud.h"
#include "../../application/NativeSpecialSkills.h"
#include "TMLog.h"
#include "ClientDiagnostics.h"
#include "TMGround.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"
#include "TMCamera.h"
#include "ServerEndpoint.h"
#include "ServerStatus.h"
#include "ServerChannelLabel.h"
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
	// FUN_004431e4 also feeds the translucent MP overlay 1159 (Field+0x27adc),
	// the MP counterpart of 1158; leaving it unfed froze it at its resource value.
	resource_ui::Project(
		static_cast<SProgressBar*>(m_pControlContainer->FindControl(TMP_MP_PROGRESS_TR)),
		pMobData->CurrentScore.CurMP, pMobData->CurrentScore.MaxMP);

	auto setNumber = [this](unsigned int controlID, long long value)
	{
		if (auto pText = static_cast<SText*>(m_pControlContainer->FindControl(controlID)))
		{
			char text[64]{};
			sprintf_s(text, "%I64d", value);
			pText->SetText(text, 0);
		}
	};

	// FUN_004431e4 totals the Q/W/E shortcut items of the 9x7 Carry into the
	// bottom-bar counters 1042/1043 (and 1044/5730/5731 where a resource has
	// them). FieldScene2 has no 1044; FindControl returns null for it.
	native_score_hud::QuickPotionCounts potionCounts;
	if (m_pGridInv)
	{
		for (int y = 0; y < 7; ++y)
		{
			for (int x = 0; x < 9; ++x)
			{
				auto* pCell = m_pGridInv->GetItem(x, y);
				if (!pCell || !pCell->m_pItem)
					continue;
				native_score_hud::AddCarryCell(potionCounts, pCell->m_pItem->sIndex,
					BASE_GetItemAmount(pCell->m_pItem), BASE_GetItemAbility(pCell->m_pItem, 38));
			}
		}
	}
	const bool shortcutCancelled = m_pMyHuman && m_pMyHuman->m_cCancel == 1;
	auto setShortcutCount = [this](unsigned int controlID, int count, bool cancelled)
	{
		if (auto pText = static_cast<SText*>(m_pControlContainer->FindControl(controlID)))
		{
			char text[16]{};
			native_score_hud::FormatShortcutCount(text, sizeof(text), count, cancelled);
			pText->SetText(text, 0);
		}
	};
	setShortcutCount(TMT_HPOTION, potionCounts.hp, shortcutCancelled);
	setShortcutCount(TMT_HPOTION2, potionCounts.hp, shortcutCancelled);
	setShortcutCount(TMT_MPOTION, potionCounts.mp, shortcutCancelled);
	setShortcutCount(TMT_MPOTION2, potionCounts.mp, shortcutCancelled);
	setShortcutCount(TMT_PPOTION, potionCounts.special, false);

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

	// FUN_004431e4 writes the selected skill's damage into HUD 1030 and
	// Character 1104 together. Skills outside the native 0..103 range show "0"
	// and keep their color; healing (negative) is shown absolute in green,
	// damage in blue.
	const int selectedShortSlot = g_pObjectManager->m_cSelectShortSkill;
	const int selectedSkill = selectedShortSlot >= 0 && selectedShortSlot < 20
		? g_pObjectManager->m_cShortSkill[selectedShortSlot] : -1;
	char skillDamageText[32]{};
	unsigned int skillDamageColor = 0;
	if (m_pMyHuman && selectedSkill >= 0 && selectedSkill <= 103)
	{
		SetMyHumanMagic();
		const int weather = SkillDamageWeather(g_nWeather,
			static_cast<int>(m_pMyHuman->m_vecPosition.x) >> 7,
			static_cast<int>(m_pMyHuman->m_vecPosition.y) >> 7);
		const int skillDamage = BASE_GetSkillDamage(selectedSkill, pMobData,
			weather, GetWeaponDamage(), pMobData->Equip[0].sIndex);
		sprintf_s(skillDamageText, "%d", skillDamage < 0 ? -skillDamage : skillDamage);
		skillDamageColor = skillDamage < 0 ? 0xFFAAFFAA : 0xFFBBBBFF;
	}
	else
		sprintf_s(skillDamageText, "%d", 0);
	for (unsigned int controlID : { TMT_ATT_ENC, TMT_CI_ATRT })
	{
		if (auto pText = static_cast<SText*>(m_pControlContainer->FindControl(controlID)))
		{
			pText->SetText(skillDamageText, 0);
			if (skillDamageColor)
				pText->SetTextColor(skillDamageColor);
		}
	}

	// HUD ATT 1029 and Character 1096 show the current attack. HUD DEF 1033
	// (white) is base defense plus abilities 3 and 53; the blue 1040 and
	// Character 1108 repeat the current defense (Field+0x27a80/0x27a84).
	const int nativeDefense = BASE_GetMobAbility(pMobData, 53)
		+ BASE_GetMobAbility(pMobData, 3) + pMobData->BaseScore.Defense;
	setNumber(TMT_ATT, pMobData->CurrentScore.Attack);
	setNumber(TMT_DEF, nativeDefense);
	setNumber(TMT_DEF_ENC, pMobData->CurrentScore.Defense);

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
	// FieldScene2 overlaps 1031 (x=40, w=130) and 1032 (x=104, w=64). The
	// native modern HUD hides 1032 and shows "current / next" in 1031 only.
	char expRange[64]{};
	native_score_hud::FormatExpRange(expRange, sizeof(expRange),
		pMobData->Exp, levelTable[currentLevel + 1]);
	if (auto pExpText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_EXP)))
		pExpText->SetText(expRange, 0);
	if (auto pNextExp = m_pControlContainer->FindControl(TMT_EXP_ENC))
		pNextExp->SetVisible(0);

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
	// FUN_004431e4 formats gold as "%10d" with SText comma grouping (+0xcdd = 1):
	// inventory 370/1041, gamble balance 6408 (gold minus the pending bet) and
	// cargo gold 1835/812 from the ObjectManager cargo counter.
	// The auto-trade copy 812 is written without touching its comma flag.
	auto setGold = [this](unsigned int controlID, long long value, bool comma)
	{
		if (auto pText = static_cast<SText*>(m_pControlContainer->FindControl(controlID)))
		{
			char text[32]{};
			sprintf_s(text, "%10lld", value);
			if (comma)
				pText->m_cComma = 1;
			pText->SetText(text, 0);
		}
	};
	setGold(TMT_MONEY_TEXT, pMobData->Coin, true);
	setGold(TMT_MONEY, pMobData->Coin, true);
	setGold(TMT_GAMBLE_MONEY, static_cast<long long>(pMobData->Coin) - m_nBet, true);
	if (m_pControlContainer->FindControl(TMT_CMONEY_TEXT))
	{
		setGold(TMT_CMONEY_TEXT, g_pObjectManager->m_nCargoCoin, true);
		setGold(TMT_ATRADE_COIN, g_pObjectManager->m_nCargoCoin, false);
	}

	// The compact 7.48 character window uses the original 10xx control IDs,
	// whereas the imported 7.59 initializer only binds its replacement 65xxx
	// controls.  Project the same authoritative score into the legacy fields.
	setNumber(TMT_CI_LEVEL, pMobData->CurrentScore.Level + 1);
	// FUN_004431e4 writes the current EXP into 1061 and the next-level
	// threshold into 1062; repeating the current value hid the target.
	setNumber(TMT_CI_EXP, pMobData->Exp);
	setNumber(TMT_CI_EXP_E, levelTable[currentLevel + 1]);
	setNumber(TMT_CI_STR, pMobData->CurrentScore.Str);
	setNumber(TMT_CI_INT, pMobData->CurrentScore.Int);
	setNumber(TMT_CI_DEX, pMobData->CurrentScore.Dex);
	setNumber(TMT_CI_CON, pMobData->CurrentScore.Con);
	setNumber(TMT_CI_SCOREPOINT, g_pObjectManager->m_stMobData.CurrentScore.StatusPts);
	setNumber(TMT_CI_HP, pMobData->CurrentScore.CurHP);
	setNumber(TMT_CI_MP, pMobData->CurrentScore.CurMP);
	setNumber(TMT_CI_ATT, pMobData->CurrentScore.Attack);
	setNumber(TMT_CI_DFE, pMobData->CurrentScore.Defense);
	// FUN_004431e4 prints (int)TMHuman::m_fMaxSpeed (+0x220), the clamped
	// 1..7 BASE_GetSpeed result, not the raw AttackRun nibble.
	setNumber(TMT_CI_SPEED, m_pMyHuman
		? static_cast<int>(m_pMyHuman->m_fMaxSpeed)
		: BASE_GetSpeed(&pMobData->CurrentScore));
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
	// Name 1057 is "[%s]" drawn with slashed zeros (SetText second argument 1).
	if (auto pName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_NAME)))
	{
		char nameText[64]{};
		native_score_hud::FormatCharacterName(nameText, sizeof(nameText), pMobData->MobName);
		pName->SetText(nameText, 1);
	}
	// Guild 5752: the slot's guild name in grey, or an empty white text.
	if (auto pGuild = static_cast<SText*>(m_pControlContainer->FindControl(TMT_CI_GUILD)))
	{
		const int slot = g_pObjectManager->m_cCharacterSlot;
		if (pMobData->Guild != 0 && slot >= 0 && slot < 4)
		{
			char guildText[16]{};
			sprintf_s(guildText, "%.11s", g_pObjectManager->m_strGuildName[slot]);
			pGuild->SetText(guildText, 0);
			pGuild->SetTextColor(0xFFAAAAAA);
		}
		else
		{
			pGuild->SetText(const_cast<char*>(""), 0);
			pGuild->SetTextColor(0xFFFFFFFF);
		}
	}
	static const unsigned int classMessage[4] = { 121, 122, 123, 124 };
	const int characterClass = pMobData->Class >= 0 && pMobData->Class < 4 ? pMobData->Class : 0;
	if (pMobData->Equip[0].sIndex >= 40)
	{
		// A non-player face shows the native "Monster" caption.
		setCompatText(TMT_CI_CLASS, "Monster");
		setCompatText(TMT_CI_CLASS2, "");
	}
	else
	{
		setCompatText(TMT_CI_CLASS, g_pMessageStringTable[classMessage[characterClass]]);
		const int bodyClass = pMobData->Equip[0].sIndex / 10;
		setCompatText(TMT_CI_CLASS2, pMobData->Equip[0].sIndex % 10 >= 6
			&& bodyClass >= 0 && bodyClass < 4 ? g_pMessageStringTable[classMessage[bodyClass]] : "");
	}

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
		// Native format "%3d/%-3d": the cap is left-aligned after the slash.
		native_score_hud::FormatMastery(masteryText, sizeof(masteryText),
			static_cast<int>(pMobData->CurrentScore.Mastery[mastery]), masteryMax[mastery]);
		setCompatText(masteryValueIDs[mastery], masteryText);
	}

	// The four captions are class-specific in the native 7.48 score updater.
	// Binding the original 5776..5779 controls avoids leaving the Trans Knight
	// labels baked into FieldScene2.bin visible for every other base class.
	for (unsigned int mastery = 0; mastery < 4; ++mastery)
		setCompatText(TMT_CI_SPECIAL1_C + mastery,
			g_pMessageStringTable[242 + characterClass * 4 + mastery]);
	// Skill-window section captions 5788..5790 follow the class as well
	// (strings 107..115 for the first three classes, 133..135 for Hunter).
	static const unsigned int skillSectionMessage[4] = { 107, 110, 113, 133 };
	for (unsigned int section = 0; section < 3; ++section)
		setCompatText(TMT_SKILL_SEC1_C + section,
			g_pMessageStringTable[skillSectionMessage[characterClass] + section]);

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
	// FUN_004431e4 shows the critical byte << 1 per mille, not << 2.
	native_score_hud::FormatCritical(percent, sizeof(percent), pMobData->CurrentScore.Critical);
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
	if (!g_pObjectManager)
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
		const IVector2 inserted = pGrid->AddItemInEmpty(pControlItem);
		WYD748_DiagnosticsLog(
			"compat learned skill mask=0x%08X bit=%d control=%u item=%d result=%d,%d\r\n",
			learnedMask, i, controlID, itemIndex, inserted.x, inserted.y);
		if (inserted.x < 0)
			SAFE_DELETE(pControlItem);
	}

	// FUN_004431e4 also projects the special bits 24..31 into grid 571; without
	// this a learned Sephira book never yields a skill that can be dragged or cast.
	// Unlike the native loop, a slot whose bit was removed is cleared.
	if (!m_pGridSkillBelt)
		return;
	for (int i = 0; i < native_special_skill::SlotCount; ++i)
	{
		const auto cell = native_special_skill::SlotCell(i);
		const short itemIndex = native_special_skill::ItemIndex(i);
		const bool learned = (learnedMask & native_special_skill::Bit(i)) != 0;
		auto pCurrent = m_pGridSkillBelt->GetItem(cell.x, cell.y);
		if (learned && pCurrent && pCurrent->GetItem() && pCurrent->GetItem()->sIndex == itemIndex)
			continue;
		if (pCurrent)
		{
			auto pOld = m_pGridSkillBelt->PickupItem(cell.x, cell.y);
			if (g_pCursor && g_pCursor->m_pAttachedItem == pOld)
				g_pCursor->m_pAttachedItem = nullptr;
			SAFE_DELETE(pOld);
		}
		if (!learned)
			continue;

		auto pStructItem = new STRUCT_ITEM;
		memset(pStructItem, 0, sizeof(STRUCT_ITEM));
		pStructItem->sIndex = itemIndex;
		auto pControlItem = new SGridControlItem(nullptr, pStructItem, 0.0f, 0.0f);
		const int added = m_pGridSkillBelt->AddItem(pControlItem, cell.x, cell.y);
		WYD748_DiagnosticsLog(
			"compat special skill mask=0x%08X slot=%d item=%d cell=%d,%d result=%d\r\n",
			learnedMask, i, itemIndex, cell.x, cell.y, added);
		if (!added)
			SAFE_DELETE(pControlItem);
	}
}

void TMFieldScene::InitializeRuntimeCounterTexts()
{
	if (!m_pControlContainer || !g_pDevice)
		return;

	m_pRankTimeText = new SText(-2, "00 : 00", 0xFF00FF00,
		(((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fUIScaleX) / 2.0f) - 42.0f,
		30.0f, 200.0f, 16.0f, 0, 0x77777777, 1, 0);
	if (m_pRankTimeText)
	{
		m_pRankTimeText->m_Font.m_fSize = 2.0f;
		m_pRankTimeText->SetVisible(0);
		m_pRankTimeText->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - RenderDevice::UiPx(50.0f),
			RenderDevice::UiPx(3.0f + 55.0f));
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
		((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fUIScaleX) - 210.0f,
		30.0f, 100.0f, 16.0f, 0, 0x77777777, 1, 0);
	if (m_pRemainText)
	{
		m_pRemainText->m_Font.m_fSize = 2.0f;
		m_pRemainText->SetVisible(0);
		m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - RenderDevice::UiPx(200.0f),
			RenderDevice::UiPx(30.0f));
		m_pControlContainer->AddItem(m_pRemainText);
	}

	m_pQuestRemainTime = new SText(-2, "", 0xDDFFFF33,
		((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fUIScaleX) - 140.0f,
		5.0f, 100.0f, 16.0f,
		0, 0x77777777, 1, 0);
	if (m_pQuestRemainTime)
	{
		m_pQuestRemainTime->m_Font.m_fSize = 1.0f;
		m_pQuestRemainTime->SetVisible(1);
		// FUN_00435b13 (UI mode 2): top-right corner, W-140 from 800 pixels up.
		if (g_pDevice->m_dwScreenWidth < 800)
			m_pQuestRemainTime->SetPos((float)g_pDevice->m_dwScreenWidth + RenderDevice::UiPx(35.0f),
				RenderDevice::UiPx(5.0f));
		else
			m_pQuestRemainTime->SetPos((float)g_pDevice->m_dwScreenWidth - RenderDevice::UiPx(140.0f),
				RenderDevice::UiPx(5.0f));
		m_pControlContainer->AddItem(m_pQuestRemainTime);
	}
}

int TMFieldScene::InitializeCompatFieldScene()
{
	// This compatibility initializer deliberately contains only the objects
	// required to render the 7.48 world.  The imported source's full HUD setup
	// dereferences controls that do not exist in the original FieldScene2.bin;
	// entering the world must not depend on those optional controls.
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
		// Ghidra FUN_00435b13 (0x00440a35..0x00440e60): the in-world channel
		// selector is FieldScene2 panel 0x3000 with list 0x3001. The modern UI
		// sets the channel width to 140, adds the 7.48 header caps (-392..-394)
		// and the list body (-396; its height reuses a loop counter left at 128),
		// centers the panel, then hides panel and list until button 633.
		m_pServerPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_MOVE_SELSERVER));
		if (m_pServerPanel)
		{
			g_nChannelWidth = 140;
			const float middleWidth = static_cast<float>(g_nChannelWidth - 65);
			m_pServerPanel->AddChild(new SPanel(-392, 0.0f, 0.0f, 40.0f, 45.0f,
				0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH));
			m_pServerPanel->AddChild(new SPanel(-393, 23.0f, 0.0f, middleWidth, 45.0f,
				0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH));
			m_pServerPanel->AddChild(new SPanel(-394, middleWidth + 23.0f, 0.0f, 40.0f, 45.0f,
				0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH));
			m_pServerPanel->SetPos(
				static_cast<float>(g_pDevice->m_dwScreenWidth >> 1) - RenderDevice::UiPx(static_cast<float>(g_nChannelWidth + 15)) / 2.0f,
				static_cast<float>(g_pDevice->m_dwScreenHeight >> 1) - m_pServerPanel->m_nHeight / 2.0f);
			m_pServerPanel->AddChild(new SPanel(-396, -1.0f, 45.0f, static_cast<float>(g_nChannelWidth),
				128.0f + 63.0f, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH));
		}
		m_pServerList = static_cast<SListBox*>(m_pControlContainer->FindControl(TML_MOVE_SELECT_SERVER));
		if (m_pServerList)
		{
			m_pServerList->SetVisible(0);
			if (m_pServerPanel)
				m_pServerPanel->SetVisible(0);
			m_pServerList->SetEventListener(m_pControlContainer);
		}
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
		// Bottom HUD roots anchored below (FUN_00435b13, UI mode 2).
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
		// FUN_00435b13 pairs each tab with the 4x8 indicator buttons
		// 12835..12838. 5701..5704 are SText captions in FieldScene2.bin;
		// treating them as SButton wrote past the text object and left the
		// on/off indicators unbound, so a toggled filter showed no change.
		m_pChatGeneral_C = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_CHAT_GENERAL_C));
		m_pChatParty_C = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_CHAT_PARTY_C));
		m_pChatWhisper_C = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_CHAT_WHISPER_C));
		m_pChatGuild_C = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_CHAT_GUILD_C));
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
		// Show the initial "all channels on" state before the first mouse event.
		for (SButton* chatButton : { m_pChatGeneral, m_pChatParty, m_pChatWhisper, m_pChatGuild,
			m_pChatGeneral_C, m_pChatParty_C, m_pChatWhisper_C, m_pChatGuild_C })
		{
			if (chatButton)
				chatButton->Update();
		}
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
		// FUN_00435b13 (UI mode 2) anchors the bottom HUD in this order; the
		// helpers are FUN_00401304 (scale the resource position from 800x600),
		// FUN_004013c2/FUN_0040137c/FUN_00401367/FUN_004013ad (stick bottom,
		// right, left, top) and FUN_004013f3 (centre the 800x600 layout).
		const float viewportW = static_cast<float>(g_pDevice->m_dwScreenWidth);
		const float viewportH = static_cast<float>(g_pDevice->m_dwScreenHeight);
		// The 800x600 layout in screen pixels (ClientSettings UIScale).
		const float layoutW = RenderDevice::UiPx(WYD748_UI_BASE_WIDTH);
		const float layoutH = RenderDevice::UiPx(WYD748_UI_BASE_HEIGHT);
		auto scaleResourcePos = [viewportW, viewportH, layoutW, layoutH](SControl* control)
		{
			control->m_nPosX = control->m_nPosX / layoutW * viewportW;
			control->m_nPosY = control->m_nPosY / layoutH * viewportH;
		};
		if (pNativeMainInfo)
		{
			scaleResourcePos(pNativeMainInfo);
			pNativeMainInfo->SetStickBottom();
			pNativeMainInfo->m_nPosX = viewportW * 0.5f - pNativeMainInfo->m_nWidth * 0.5f + RenderDevice::UiPx(4.0f);
		}
		SControl* pNativeChatStrip = m_pControlContainer->FindControl(12624);
		if (pNativeChatStrip && pNativeMainInfo)
		{
			scaleResourcePos(pNativeChatStrip);
			pNativeChatStrip->SetStickBottom();
			pNativeChatStrip->m_nPosX = pNativeMainInfo->m_nPosX - pNativeChatStrip->m_nWidth;
			if (SControl* pChatList = m_pControlContainer->FindControl(5377))
				pChatList->m_nPosY = pNativeChatStrip->m_nPosY - pChatList->m_nHeight - RenderDevice::UiPx(6.0f);
		}
		// FUN_00435b13 also moves the hint text (SButton::m_pAltText, native
		// +0x1f0) of the HUD buttons relative to their button.
		auto buttonHint = [](SControl* control) -> SText*
		{
			if (!control || control->m_eCtrlType != CONTROL_TYPE::CTRL_TYPE_BUTTON)
				return nullptr;
			return static_cast<SButton*>(control)->m_pAltText;
		};
		if (SControl* pBottomCenter = m_pControlContainer->FindControl(5722))
		{
			pBottomCenter->SetPos(viewportW * 0.5f - pBottomCenter->m_nWidth * 0.5f,
				viewportH - pBottomCenter->m_nHeight);
			if (SText* hint = buttonHint(pBottomCenter); hint && pNativeMainInfo)
				hint->SetPos(pNativeMainInfo->m_nWidth * 0.5f - RenderDevice::UiPx(58.0f),
					hint->m_nPosY - RenderDevice::UiPx(15.0f));
		}
		if (SControl* pStatusOrb = m_pControlContainer->FindControl(5723))
		{
			scaleResourcePos(pStatusOrb);
			pStatusOrb->m_nPosX = 0.0f;
			pStatusOrb->m_nPosY = 0.0f;
		}
		if (SControl* pBottomLeft = m_pControlContainer->FindControl(5732))
		{
			pBottomLeft->SetPos(0.0f, viewportH - pBottomLeft->m_nHeight);
			SControl* pOrb = m_pControlContainer->FindControl(5723);
			if (SText* hint = buttonHint(pBottomLeft); hint && pOrb)
				hint->SetPos(hint->m_nPosX + RenderDevice::UiPx(5.0f),
					pOrb->m_nHeight * 0.5f - RenderDevice::UiPx(5.0f));
		}
		SControl* pBottomRight = m_pControlContainer->FindControl(5733);
		if (pBottomRight && pNativeMainInfo)
			pBottomRight->SetPos(viewportW - pBottomRight->m_nWidth,
				viewportH - pNativeMainInfo->m_nHeight - RenderDevice::UiPx(30.0f));
		if (SControl* pBottomRightTab = m_pControlContainer->FindControl(5735))
			if (pBottomRight)
			{
				pBottomRightTab->SetPos(viewportW - RenderDevice::UiPx(10.0f), pBottomRight->m_nPosY);
				if (SText* hint = buttonHint(pBottomRightTab))
					hint->SetPos(hint->m_nPosX - RenderDevice::UiPx(40.0f),
						pBottomRight->m_nHeight * 0.5f - RenderDevice::UiPx(5.0f));
			}
		if (pNativeChatBar && pNativeChatStrip)
			pNativeChatBar->SetPos(pNativeChatStrip->m_nPosX, pNativeChatStrip->m_nPosY);
		// The input-language marker (native scene +0x60) sits inside the chat bar.
		if (pNativeChatBar && m_pAlphaNative)
			m_pAlphaNative->SetPos(pNativeChatBar->m_nPosX + RenderDevice::UiPx(3.0f),
				pNativeChatBar->m_nPosY + RenderDevice::UiPx(5.0f));
		// HP/MP bars 1169/1170, their overlays 1158/1159 and the mount HP bars
		// 12804/12805 grow by 6 pixels and move up by 3 in UI mode 2.
		for (unsigned int barID : { 1169u, 1170u, 1158u, 1159u, 12804u, 12805u })
		{
			if (SControl* bar = m_pControlContainer->FindControl(barID))
			{
				bar->SetSize(bar->m_nWidth, bar->m_nHeight + RenderDevice::UiPx(6.0f));
				bar->SetPos(bar->m_nPosX, bar->m_nPosY - RenderDevice::UiPx(3.0f));
			}
		}
		// The event chat list 6135 opens at (300, 300).
		if (SControl* pChatEvent = m_pControlContainer->FindControl(TMT_CHAT_EVENT))
			pChatEvent->SetPos(RenderDevice::UiPx(300.0f), RenderDevice::UiPx(300.0f));
		if (pNativeMainMenuButton)
		{
			scaleResourcePos(pNativeMainMenuButton);
			pNativeMainMenuButton->SetStickRight();
			pNativeMainMenuButton->SetStickBottom();
		}
		if (pNativeMainMenuPanel && pNativeMainMenuButton)
		{
			scaleResourcePos(pNativeMainMenuPanel);
			pNativeMainMenuPanel->SetStickRight();
			pNativeMainMenuPanel->m_nPosY = pNativeMainMenuButton->m_nPosY - pNativeMainMenuPanel->m_nHeight;
			pNativeMainMenuPanel->m_nPosX -= RenderDevice::UiPx(2.0f);
			pNativeMainMenuPanel->SetVisible(0);
		}
		if (pNativeShortSkillBar && pNativeMainInfo)
		{
			pNativeShortSkillBar->m_nPosX += (viewportW - layoutW) * 0.5f;
			pNativeShortSkillBar->m_nPosY += (viewportH - layoutH) * 0.5f;
			pNativeShortSkillBar->SetStickBottom();
			pNativeShortSkillBar->m_nPosX = pNativeMainInfo->m_nPosX + pNativeMainInfo->m_nWidth;
			CreateCompatStatusTexts(pNativeShortSkillBar);
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
		PositionCompatNativeRoots();
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
	if (!g_pDevice)
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
			viewportCenterX - m_pCPanel->m_nWidth * 1.5f - RenderDevice::UiPx(10.0f),
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
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + RenderDevice::UiPx(10.0f),
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
	PositionCompatShopPanels();
	PositionCompatTradePanels();
}

void TMFieldScene::PositionCompatNativeRoots()
{
	if (!g_pDevice || !m_pControlContainer)
		return;

	const float viewportW = static_cast<float>(g_pDevice->m_dwScreenWidth);
	const float viewportH = static_cast<float>(g_pDevice->m_dwScreenHeight);
	auto centre = [&](unsigned int controlID)
	{
		if (SControl* control = m_pControlContainer->FindControl(controlID))
			control->SetPos(viewportW * 0.5f - control->m_nWidth * 0.5f,
				viewportH * 0.5f - control->m_nHeight * 0.5f);
	};
	// Auto trade 646, input boxes 640/626, skill master 1889, 669 and 6185.
	for (unsigned int controlID : { 646u, 640u, 626u, 1889u, 669u, 6185u })
		centre(controlID);
	// 8705 and 8961 use (W - w) / 2 and (H - h) / 2.
	for (unsigned int controlID : { 8705u, 8961u })
	{
		if (SControl* control = m_pControlContainer->FindControl(controlID))
			control->SetPos((viewportW - control->m_nWidth) * 0.5f,
				(viewportH - control->m_nHeight) * 0.5f);
	}
	// 2048 centres its width plus a 40-pixel margin.
	if (SControl* control = m_pControlContainer->FindControl(2048))
		control->SetPos(viewportW * 0.5f - (control->m_nWidth + RenderDevice::UiPx(40.0f)) * 0.5f,
			viewportH * 0.5f - control->m_nHeight * 0.5f);
	// 880 sits 200 pixels above the viewport centre.
	if (SControl* control = m_pControlContainer->FindControl(880))
		control->SetPos(viewportW * 0.5f - control->m_nWidth * 0.5f, viewportH * 0.5f - RenderDevice::UiPx(200.0f));
	// Cargo 1825 sits immediately left of Inventory 257's native position.
	if (m_pCargoPanel && m_pInvenPanel)
		m_pCargoPanel->SetPos(
			viewportW * 0.5f + m_pInvenPanel->m_nWidth * 0.5f + RenderDevice::UiPx(10.0f) - m_pCargoPanel->m_nWidth,
			viewportH * 0.5f - m_pCargoPanel->m_nHeight * 0.5f);
}

void TMFieldScene::PositionCompatNativeMixPanels()
{
	if (!g_pDevice)
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
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + RenderDevice::UiPx(10.0f),
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
}

void TMFieldScene::PositionCompatShopPanels()
{
	if (!g_pDevice)
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
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + RenderDevice::UiPx(10.0f),
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
}

void TMFieldScene::PositionCompatTradePanels()
{
	if (!g_pDevice)
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
			viewportCenterX + m_pInvenPanel->m_nWidth * 0.5f + RenderDevice::UiPx(10.0f),
			viewportCenterY - m_pInvenPanel->m_nHeight * 0.5f);
	}
}

void TMFieldScene::PositionCompatGamblePanel()
{
	if (!g_pDevice || !m_pGambleStore)
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
	if (!g_pDevice || !m_pPartyPanel)
		return;

	// Native WYD 7.48 FUN_00435b13 places both the Party root 1857 and its
	// toggle 5742 at the same lower-left anchor. FieldScene2.bin alone does
	// not account for the active viewport height.
	const float partyY = static_cast<float>(g_pDevice->m_dwScreenHeight)
		- m_pPartyPanel->m_nHeight - RenderDevice::UiPx(165.0f);
	m_pPartyPanel->SetPos(0.0f, partyY);
	if (m_pPartyBtn)
	{
		m_pPartyBtn->SetPos(0.0f, partyY);
		if (m_pPartyBtn->m_pAltText)
			m_pPartyBtn->m_pAltText->SetPos(PartyHintOffsetX(m_pPartyBtn->m_nWidth, RenderDevice::UiPx(1.0f)),
				m_pPartyPanel->m_nHeight * 0.5f - RenderDevice::UiPx(13.0f));
	}
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
	if (m_pCPanel)
		m_pCPanel->SetVisible(m_pCPanel->IsVisible() == 0);
	GetSoundAndPlay(51, 0, 0);
	return;
}

void TMFieldScene::UpdateScoreUI(unsigned int unFlag)
{
	// The 7.48 HUD has a different control graph.  Its dedicated projector
	// updates the same authoritative score without dereferencing 7.59-only UI.
	UpdateCompatScoreUI();
	UpdateCompatLearnedSkillUI();
	return;
}

void TMFieldScene::UpdateScoreButtonRepeat(unsigned int dwServerTime)
{
	// FUN_004776c3 polls the eight native '+' buttons only while Character
	// panel 513 is visible. The last hovered ID wins, and the held control is
	// re-dispatched through the regular control event so every server gate
	// (dead, no points, mastery cap) still applies to each repeat.
	if (!m_pCPanel || !m_pCPanel->IsVisible() || !m_pControlContainer)
	{
		m_scoreButtonRepeat.Reset();
		return;
	}
	static const unsigned int scoreButtons[] = {
		TMB_CI_STR_E, TMB_CI_INT_E, TMB_CI_DEX_E, TMB_CI_CON_E,
		TMB_CI_SPECIAL1_E, TMB_CI_SPECIAL2_E, TMB_CI_SPECIAL3_E, TMB_CI_SPECIAL4_E };
	unsigned int heldButton = 0;
	for (unsigned int controlID : scoreButtons)
	{
		auto* pButton = m_pControlContainer->FindControl(controlID);
		if (pButton && pButton->m_bOver == 1)
			heldButton = controlID;
	}
	const bool held = heldButton != 0 && g_pEventTranslator && g_pEventTranslator->button[0] != 0;
	if (m_scoreButtonRepeat.Tick(held, dwServerTime))
		OnControlEvent(heldButton, 0);
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

void TMFieldScene::UpdateCompatObservedAffects()
{
	// Reproject every frame, including empty/hidden states. Never borrow the
	// render-mesh m_TargetAffect cache, which can still describe the previous mob.
	TMHuman* target = m_pMouseOverHuman;
	SProgressBar* bar = target ? target->m_pTitleProgressBar : nullptr;
	const auto* targetWords = bar && bar->IsVisible() ? observed_affect_ui::InViewAffects(target) : nullptr;
	const float targetSize = 18.0f * RenderDevice::m_fUIScaleY;
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
	const float partySize = min(18.0f * RenderDevice::m_fUIScaleY, max(1.0f, rowHeight - 2.0f));
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

	// FUN_004770ad defines the stock row geometry: only the first sixteen
	// STRUCT_AFFECT entries are visualized, types 0..40 are accepted, and the
	// row begins three pixels after HUD anchor 5723 at y=5.  The source's
	// compatibility scene explicitly recreates these panels over the
	// FieldScene2 tree; they do not depend on an imported UI2-only gate.

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

unsigned int TMFieldScene::GetLascDescParamId()
{
	for (int i = NUM_ITEM_DESC_PARAMS - 1; i >= 0; i--)
	{
		if (m_pParamText[i] && m_pParamText[i]->GetText()[0] != 0)
			return i;
	}

	return 0;
}

// Ghidra FUN_004662c5, control 0x279 (633): with panel 0x3000 present the
// native client lists the channels of the current group instead of quitting;
// only the panel's own 0x3003 starts the 0x3AE countdown. Without the panel it
// falls back to the immediate 0x3AE request.
int TMFieldScene::OpenNativeServerPanel()
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

	if (m_pSystemPanel)
		m_pSystemPanel->SetVisible(0);
	if (auto button = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SYSTEM)))
		button->SetSelected(0);
	m_pMessagePanel->SetMessage(g_pMessageStringTable[23], 0);
	m_pMessagePanel->SetVisible(1, 0);

	const int serverGroup = LastConfiguredServerGroup(g_pServerList);
	const int currentGroup = g_pObjectManager->m_nServerGroupIndex;
	if (serverGroup < 0 || currentGroup < 0 || currentGroup >= MAX_SERVERGROUP)
		return 1;

	// Each group rotates its channel numbering by the local day of month.
	_SYSTEMTIME time{};
	GetLocalTime(&time);
	int nDay[MAX_SERVERGROUP]{};
	for (int i = 0; i < MAX_SERVERGROUP; ++i)
	{
		for (int k = 1; k < MAX_SERVERNUMBER; ++k)
			if (g_pServerList[i][k][0] != 0)
				++nDay[i];
		if (nDay[i])
			nDay[i] = !(time.wDay % nDay[i]) ? nDay[i] : time.wDay % nDay[i];
	}

	char szUserCount[1024]{};
	int nUserCount[MAX_SERVERNUMBER]{};
	int nUserCount2[MAX_SERVERNUMBER]{};
	if (currentGroup == serverGroup)
	{
		// The last configured group aggregates the selected channel of every
		// other group.
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
			nUserCount[nDay[serverGroup - i]] = nUserCount2[nDay[serverGroup - i]];
			CopyServerEndpointAt(g_pServerList[currentGroup][i + 1], g_pServerList,
				serverGroup - i - 1, nDay[serverGroup - i] + 1);
		}
	}
	else
	{
		char szStatusEndpoint[64]{};
		if (CopyServerEndpoint(szStatusEndpoint, g_pServerList[currentGroup][0]))
			BASE_GetHttpRequest(szStatusEndpoint, szUserCount, sizeof szUserCount);
		ParseServerStatus(szUserCount, nUserCount, 10);
	}

	m_pMessagePanel->SetVisible(0, 1);
	if (!m_pServerList)
		return 1;

	m_pServerList->Empty();
	m_pServerList->SetSelectedIndex(-1);
	for (int num = 1; num < MAX_SERVERNUMBER; ++num)
	{
		char szText[32]{};
		if (g_pServerList[currentGroup][num][0])
		{
			if (currentGroup == serverGroup)
			{
				const int group = currentGroup - num;
				if (group < 0)
					continue;
				const char* selectedName = ServerChannelNameAt(g_szServerName, group, nDay[group]);
				const char* displayName = ServerChannelNameAt(g_szServerName, group, nDay[group] - 1);
				if (selectedName && displayName)
					sprintf_s(szText, "%s-%s", g_szServerNameList[group], displayName);
				else
				{
					sprintf_s(szText, "%s-%d", g_szServerNameList[group], nDay[group]);
					if (nUserCount[num] > 700)
						AppendFullChannelLabel(szText, sizeof(szText));
				}
			}
			else if (g_szServerNameList[currentGroup][0])
			{
				if (const char* channelName = ServerChannelNameAt(g_szServerName, currentGroup, num))
					sprintf_s(szText, "%s-%s", g_szServerNameList[currentGroup], channelName);
				else
				{
					sprintf_s(szText, "%s-%d", g_szServerNameList[currentGroup], num);
					if (nUserCount[num] > 700)
						AppendFullChannelLabel(szText, sizeof(szText));
				}
			}
			else
				sprintf_s(szText, g_pMessageStringTable[68], num + 1, num);

			// The current channel and the aggregate group use the selected skin.
			const int nTextureSet = (nDay[currentGroup] == num || currentGroup == serverGroup) ? -2 : 6;
			auto server = new SListBoxServerItem(nTextureSet, szText, 0xFFFFFFFF, 0.0f, 0.0f,
				static_cast<float>(g_nChannelWidth), 16.0f, nUserCount[num] < 0 ? 0 : nUserCount[num], 0, 0, num);
			if (nUserCount[num] < 0)
				server->m_cConnected = 0;
			m_pServerList->AddItem(server);
		}
		else if (currentGroup == serverGroup && num < serverGroup)
		{
			sprintf_s(szText, "%s", g_pMessageStringTable[70]);
			auto server = new SListBoxServerItem(6, szText, 0xFFFFFFFF, 0.0f, 0.0f,
				static_cast<float>(g_nChannelWidth), 16.0f, nUserCount2[num] < 0 ? 0 : nUserCount2[num], 0, 0, 0);
			if (nUserCount[num] < 0)
				server->m_cConnected = 0;
			m_pServerList->AddItem(server);
		}
	}
	m_pServerList->SetVisible(1);
	m_pServerPanel->SetVisible(1);
	return 1;
}

void TMFieldScene::CreateCompatStatusTexts(SControl* pShortSkillBar)
{
	if (!m_pControlContainer || !pShortSkillBar || m_pStatusPKText)
		return;

	// FUN_00435b13 (UI mode 2) creates three bordered labels from strdef
	// 157/232/172 (" %s"), 18 pixels high, and places them in a row starting
	// 61 pixels right of and 20 pixels above the short skill bar (root 5745).
	auto makeLabel = [this](int messageIndex, unsigned int textColor, unsigned int borderColor,
		float width)
	{
		char szLabel[128]{};
		sprintf_s(szLabel, " %s", g_pMessageStringTable[messageIndex]);
		auto label = new SText(-2, szLabel, textColor, 0.0f, 0.0f, width, 18.0f, 1, borderColor, 1, 0);
		label->SetVisible(0);
		m_pControlContainer->AddItem(label);
		return label;
	};
	m_pStatusPKText = makeLabel(157, 0xFFFFFFBB, 0x88770000, 60.0f);
	m_pStatusAutoRunText = makeLabel(232, 0xFFFFFFFF, 0x88000077, 80.0f);
	m_pStatusRunText = makeLabel(172, 0xFFFFFFFF, 0x88007700, 60.0f);

	m_pStatusPKText->SetPos(pShortSkillBar->m_nPosX + RenderDevice::UiPx(61.0f),
		pShortSkillBar->m_nPosY - RenderDevice::UiPx(20.0f));
	m_pStatusAutoRunText->SetPos(m_pStatusPKText->m_nPosX + m_pStatusPKText->m_nWidth,
		m_pStatusPKText->m_nPosY);
	m_pStatusRunText->SetPos(m_pStatusAutoRunText->m_nPosX + m_pStatusAutoRunText->m_nWidth,
		m_pStatusPKText->m_nPosY);

	// The affect description 12834 closes FUN_00435b13: right edge minus 40,
	// 20 pixels above the auto-run label.
	if (m_pAffectDesc)
	{
		m_pAffectDesc->SetStickRight();
		m_pAffectDesc->m_nPosX -= RenderDevice::UiPx(40.0f);
		m_pAffectDesc->m_nPosY = m_pStatusAutoRunText->m_nPosY - RenderDevice::UiPx(20.0f);
	}
}

void TMFieldScene::UpdateCompatStatusTexts()
{
	// FUN_0044ecae (PK), FUN_00452737/FUN_00499f0e (auto run) and the score
	// update FUN_004431e4 (run mode while mounted) drive the same labels.
	if (m_pStatusPKText)
		m_pStatusPKText->SetVisible(TMFieldScene::m_bPK != 0);
	if (m_pStatusAutoRunText)
		m_pStatusAutoRunText->SetVisible(m_bAutoRun != 0);
	if (m_pStatusRunText)
		m_pStatusRunText->SetVisible(g_bRunning == 1 && m_pMyHuman && m_pMyHuman->m_cMount == 1);
}
