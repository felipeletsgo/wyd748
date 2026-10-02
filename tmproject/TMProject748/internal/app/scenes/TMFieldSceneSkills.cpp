#include "pch.h"
#include "TMFieldScene.h"
#include "DirShow.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMUtil.h"
#include "TMItem.h"
#include "ItemEffect.h"
#include "TMSkillHolyTouch.h"
#include "TMSkillTownPortal.h"
#include "TMSkillMagicArrow.h"
#include "TMSkillJudgement.h"
#include "TMSkillPoison.h"
#include "TMSkillMeteorStorm.h"
#include "TMSkillThunderBolt.h"
#include "TMSkillSlowSlash.h"
#include "TMSkillMagicShield.h"
#include "TMSkillFreezeBlade.h"
#include "TMEffectMesh.h"
#include "TMEffectBillBoard.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectBillBoard4.h"
#include "TMEffectSkinMesh.h"
#include "TMEffectStart.h"
#include "TMEffectSWSwing.h"
#include "TMEffectSpark.h"
#include "TMEffectParticle.h"
#include "TMShade.h"
#include "TMArrow.h"
#include "TMSkillHeavenDust.h"
#include "TMSkillFlash.h"
#include "TMEffectCharge.h"
#include "TMSkillExplosion2.h"
#include "TMCannon.h"
#include "TMSkinMesh.h"
#include "../../application/SkillCooldownPolicy.h"
#include "../../application/FieldInteractionPolicy.h"
#include "../../application/SkillRequestPolicy.h"
#include "../../wire/SkillAttackRequest.h"
#include "ClientDiagnostics.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"

void TMFieldScene::InitializeCompatSkillBelts()
{
	if (!m_pControlContainer)
		return;

	for (int i = 0; i < 24; ++i)
		m_pSkillSecGrid[i] = static_cast<SGridControl*>(
			m_pControlContainer->FindControl(TMG_SKILL_SEC1_1 + i));

	for (int i = 0; i < 12; ++i)
		m_pSkillSecGrid2[i] = nullptr;

	// FieldScene2.bin serializes the three native 7.48 belt grids (571, 573 and
	// 586).  Ghidra FUN_00435b13 binds these exact controls, so never allocate
	// modern 655xx overlays that intercept field clicks and corrupt ownership.
	m_pGridSkillBelt = static_cast<SGridControl*>(m_pControlContainer->FindControl(TMG_SKILL_BELT));
	m_pGridSkillBelt2 = static_cast<SGridControl*>(m_pControlContainer->FindControl(TMG_SKILL_BELT2));
	m_pGridSkillBelt3 = static_cast<SGridControl*>(m_pControlContainer->FindControl(TMG_SKILL_BELT3));
	// FUN_00435b13 stores control 575 as the native auto-skill selection bar.
	// The serialized control is visible by default, so hide it until T enables
	// auto-attack and let SetAutoSkillNum apply the initial native geometry.
	m_pAutoSkillPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_SKILL_SEL));
	if (m_pAutoSkillPanel)
		m_pAutoSkillPanel->SetVisible(0);
	SetAutoSkillNum(m_nAutoSkillNum);
	// FUN_00435b13 also owns the page selectors 587/588. Binding their native
	// instances keeps keyboard Z and mouse clicks on the same 7.48 state path.
	m_pShortSkillTglBtn1 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SHORTSKILL_TGL1));
	m_pShortSkillTglBtn2 = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_SHORTSKILL_TGL2));
	if (m_pGridSkillBelt)
		m_pGridSkillBelt->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;
	if (m_pGridSkillBelt2)
		m_pGridSkillBelt2->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;
	if (m_pGridSkillBelt3)
		m_pGridSkillBelt3->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;
	// The resource starts with both overlapping pages visible. The full scene
	// hides page two later, but compatibility returns before that initializer.
	// Only the selected page may draw or update the shared hover description.
	if (m_pGridSkillBelt2)
		m_pGridSkillBelt2->SetVisible(!m_bSkillBeltSwitch);
	if (m_pGridSkillBelt3)
		m_pGridSkillBelt3->SetVisible(m_bSkillBeltSwitch != 0);
	if (m_pShortSkillTglBtn1)
		m_pShortSkillTglBtn1->SetSelected(!m_bSkillBeltSwitch);
	if (m_pShortSkillTglBtn2)
		m_pShortSkillTglBtn2->SetSelected(m_bSkillBeltSwitch != 0);

	WYD748_DiagnosticsLog("compat native skill grids bound learned=%p belt2=%p belt3=%p\r\n",
		m_pGridSkillBelt, m_pGridSkillBelt2, m_pGridSkillBelt3);
}

int TMFieldScene::GetSkillDelay(int skillIndex) const
{
	if (skillIndex < 0 || skillIndex >= 248)
		return 1;
	// Keep source-HUD exceptions local; never mutate the shared skill catalog.
	if (!m_bCompatFieldScene && m_pMyHuman && g_pObjectManager)
	{
		if (skillIndex == 102 && m_pMyHuman->Is2stClass() == 2 &&
			g_pObjectManager->m_stMobData.CurrentScore.Level >= 79)
			return 1000;
		if (skillIndex == 40 &&
			g_pObjectManager->m_stMobData.Equip[4].sIndex > 0 &&
			g_pObjectManager->m_stMobData.Equip[4].sIndex < MAX_ITEMLIST &&
			g_pItemList[g_pObjectManager->m_stMobData.Equip[4].sIndex].nPos == 16)
			return 1;
	}
	return skill_cooldown::DelaySeconds(g_pSpell[skillIndex].Delay,
		m_bCompatFieldScene != 0, m_nMySanc,
		m_pMyHuman && m_pMyHuman->m_DilpunchJewel == 1);
}

bool TMFieldScene::IsSkillCoolingDown(int skillIndex, unsigned int now) const
{
	if (skillIndex < 0 || skillIndex >= 248)
		return true;
	return skill_cooldown::Active(now, m_dwSkillLastTime[skillIndex], GetSkillDelay(skillIndex));
}

void TMFieldScene::UpdateSkillCooldownUI(unsigned int now)
{
	if (!g_pObjectManager)
		return;
	for (int slot = 0; slot < 20; ++slot)
	{
		auto grid = slot < 10 ? m_pGridSkillBelt2 : m_pGridSkillBelt3;
		SGridControlItem* item = nullptr;
		// GetItem(x, y) changes hover state; a timer tick must not change selection.
		if (grid)
			for (int i = 0; i < grid->m_nNumItem; ++i)
				if (grid->m_pItemList[i] && grid->m_pItemList[i]->PtAtItem(slot % 10, 0))
				{
					item = grid->m_pItemList[i];
					break;
				}
		if (!item)
			continue;
		item->m_fTimer = 1.0f;
		int skill = static_cast<unsigned char>(g_pObjectManager->m_cShortSkill[slot]);
		if (skill >= 105)
			skill += 95;
		if (skill >= 248 || !item->m_pItem)
			continue;
		const int icon = skill < 200 ? skill + 5000 : skill + 5200;
		if (item->m_pItem->sIndex == icon)
			item->m_fTimer = skill_cooldown::Progress(now, m_dwSkillLastTime[skill], GetSkillDelay(skill));
	}
}

int TMFieldScene::SkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)
{
	if (m_pPotalPanel && m_pPotalPanel->m_bVisible == 1)
		return 0;
	if (g_pEventTranslator->m_bAlt == 1)
		return 0;
	if (m_pMyHuman->m_cHide == 1)
		return 0;
	if (m_pMyHuman->m_cCantAttk)
		return 0;
	m_pTargetItem = 0;
	if (m_pMyHuman->m_bSkillBlack == 1)
		return 0;

	int nSpecForce = 0;
	auto pMobData = &g_pObjectManager->m_stMobData;
	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		nSpecForce = 1;

	TMHuman* pOver = pTarget ? pTarget : m_pMouseOverHuman;

	if (pOver && pOver->m_cShadow == 1 && pOver->m_nClass == 66 && !m_pMyHuman->m_JewelGlasses)
		return 0;

	m_pTargetHuman = nullptr;

	char cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
	if ((unsigned char)cSkillIndex > 104)
		cSkillIndex += 95;

	if (pOver && cSkillIndex == 6 && pOver->m_nClass == 66)
		return 0;

	/*if ( m_pMyHuman->m_sCostume >= 4150 && m_pMyHuman->m_sCostume < 4200 || m_pMyHuman->m_sCostume >= 4300 && m_pMyHuman->m_sCostume < 4320 && (cSkillIndex == 64 || cSkillIndex == 66 || cSkillIndex == 68 || cSkillIndex == 70 || cSkillIndex == 71))
		return 0;*/

	int nCannonIndex = -1;

	if (!IsValidClassSkill((unsigned char)cSkillIndex))
		return 0;

	int ItemType = BASE_GetItemAbility(&pMobData->Equip[6], 21);
	if (cSkillIndex == 79 && ItemType != 101)
		return 0;
	if (cSkillIndex == 75 && ItemType != 101)
		return 0;
	if (cSkillIndex == 92 && ItemType != 41)
		return 0;

	switch (cSkillIndex)
	{
	case 99:
		return 0;
	case 83:
		return 0;
	case 84:
		m_ItemMixClass.ResultItemListSet(0, 0, 0);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	case 97:
		bool bFind = false;
		for (int i = 0; i < 100; ++i)
		{
			auto pItem = (TMCannon*)g_pObjectManager->GetItemByID(i + 15001);
			if (pItem && pItem->m_stItem.sIndex == 746 &&
				m_pMyHuman->m_vecPosition.x == pItem->m_vecBasePosition.x &&
				m_pMyHuman->m_vecPosition.y == pItem->m_vecBasePosition.y)
			{
				bFind = 1;
				nCannonIndex = i + 15001;
				break;
			}
		}
		if (!bFind)
			return GetItemFromGround(dwServerTime);
		break;
	}

	if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK && m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
		return 0;
	if ((unsigned char)cSkillIndex >= 248)
		return GetItemFromGround(dwServerTime);
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return GetItemFromGround(dwServerTime);
	if (g_pSpell[(unsigned char)cSkillIndex].Passive == 1)
		return GetItemFromGround(dwServerTime);

	if (pOver)
	{
		if (g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && pOver && pOver->m_bParty == 1)
			return GetItemFromGround(dwServerTime);
		if (g_pSpell[(unsigned __int8)cSkillIndex].Aggressive == 1 && pOver && m_pMyHuman->m_usGuild && g_pObjectManager->m_usAllyGuild &&
			(m_pMyHuman->m_usGuild == pOver->m_usGuild || g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
		{
			return GetItemFromGround(dwServerTime);
		}
	}

	if (g_bCastleWar)
	{
		if (!TMFieldScene::m_bPK && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			if (pOver)
			{
				if (pOver->m_cMantua == m_pMyHuman->m_cMantua)
				{
					if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000) && pOver->IsInPKZone() == 1)
						return TMFieldScene::GetItemFromGround(dwServerTime);
				}
			}
		}
		if (!TMFieldScene::m_bPK
			&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
			&& pOver
			&& pOver->m_cSummons == 1
			&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
			&& pOver->m_usGuild
			&& !pOver->IsInCastleZone())
		{
			return GetItemFromGround(dwServerTime);
		}
	}
	else
	{
		if (!TMFieldScene::m_bPK && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && m_pMyHuman->m_cMantua > 0)
		{
			if (pOver)
			{
				if (!pOver->m_cMantua)
				{
					if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000) && pOver->IsInPKZone() == 1)
						return GetItemFromGround(dwServerTime);
				}
			}
		}
		if (!TMFieldScene::m_bPK
			&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
			&& pOver
			&& pOver->m_cSummons == 1
			&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
			&& pOver->m_usGuild)
		{
			return TMFieldScene::GetItemFromGround(dwServerTime);
		}
	}

	if (pOver && pOver->m_nClass == 66 && pOver->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
		return 0;
	if (IsSkillCoolingDown((unsigned char)cSkillIndex, dwServerTime))
		return 0;

	if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 &&
		(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
		return 0;

	if (dwServerTime > m_dwOldAttackTime + 1000 &&
		skill_request::UsesAreaRequest(g_pSpell[(unsigned char)cSkillIndex].TargetType))
	{
		int nSpecial = m_pMyHuman->m_stScore.Level;
		if ((unsigned char)cSkillIndex < 96)
			nSpecial = g_pObjectManager->m_stMobData.CurrentScore.Mastery[((unsigned char)cSkillIndex - 24 * (unsigned char)g_pObjectManager->m_stMobData.Class) / 8 + 1];

		auto pChatList = m_pChatList;
		if (BASE_GetManaSpent((unsigned char)cSkillIndex, (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.SaveMana, nSpecial) > g_pObjectManager->m_stMobData.CurrentScore.CurMP)
		{
			auto ipNewItem = new SListBoxItem(g_pMessageStringTable[30],
				0xFFFFAAAA,
				0.0f,
				0.0f,
				300.0f,
				16.0f,
				0,
				0x77777777,
				1,
				0);

			pChatList->AddItem(ipNewItem);

			GetSoundAndPlay(33, 0, 0);
			return 0;
		}
		if (cSkillIndex == 85 && (100 * m_pMyHuman->m_stScore.Mastery[2]) > g_pObjectManager->m_stMobData.Coin)
		{
			auto ipNewItem = new SListBoxItem(g_pMessageStringTable[155],
				0xFFFFAAAA,
				0.0f,
				0.0f,
				300.0f,
				16.0f,
				0,
				0x77777777,
				1,
				0);

			pChatList->AddItem(ipNewItem);

			GetSoundAndPlay(33, 0, 0);
			return 0;
		}
		if (m_pMyHuman->IsInTown() == 1	&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
			return 0;

		auto stAttack = skill_attack::Area<MSG_Attack>(m_pMyHuman->m_dwID,
			(unsigned char)cSkillIndex, (int)m_pMyHuman->m_vecPosition.x,
			(int)m_pMyHuman->m_vecPosition.y, m_stMoveStop.NextX, m_stMoveStop.NextY);

		if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 5)
		{
			stAttack.FlagLocal = 0;
			int nTargetIndex = 0;
			int nCritical = (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.Critical;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			if (!pNode)
				return 1;

			float fMyAngle = atan2f(vec.x - m_pMyHuman->m_vecPosition.x, vec.z - m_pMyHuman->m_vecPosition.y);

			int nMastery = m_pMyHuman->m_stScore.Mastery[3] / 75;
			if (nMastery > 3)
				nMastery = 3;

			int nMX = (int)m_pMyHuman->m_vecPosition.x;
			int nMY = (int)m_pMyHuman->m_vecPosition.y;
			int nMobCount = 0;

			while (pNode != nullptr && nMobCount <= 1000)
			{
				++nMobCount;
				if (pNode == m_pMyHuman)
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				int nDX = (int)pNode->m_vecPosition.x;
				int nDY = (int)pNode->m_vecPosition.y;
				int x1 = (int)m_pMyHuman->m_vecPosition.x;
				int y1 = (int)m_pMyHuman->m_vecPosition.y;

				if (m_stMoveStop.NextX)
				{
					x1 = m_stMoveStop.NextX;
					y1 = m_stMoveStop.NextY;
				}

				int nDistanceFromMe = BASE_GetDistance(x1, y1, nDX, nDY);
				if (pNode->m_nClass == 56 && !pNode->m_stLookInfo.FaceMesh)
				{
					nDistanceFromMe -= 12;
					if (nDistanceFromMe < 0)
						nDistanceFromMe = 0;
				}

				if (nDistanceFromMe <= nMastery + 3)
				{
					float fNodeAngle = atan2f(pNode->m_vecPosition.x - m_pMyHuman->m_vecPosition.x,	pNode->m_vecPosition.y - m_pMyHuman->m_vecPosition.y);
					if (fabsf(fNodeAngle - fMyAngle) > 0.7853982f)
					{
						int nDTX = nDX;
						int nDTY = nDY;
						BASE_GetHitPosition(x1, y1, &nDTX, &nDTY, (char*)m_HeightMapData, 8);
						if (pNode->m_nClass == 56 && !pNode->m_stLookInfo.FaceMesh)
						{
							nDTX = nDX;
							nDTY = nDY;
						}

						if (nDTX != nDX && nDTY != nDY)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}

						if (!pNode->IsInPKZone() && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 &&
							(pNode->m_dwID > 0 && pNode->m_dwID < 1000))
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}

						if (pNode->IsInPKZone() == 1 && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
						{
							if (pNode->m_bParty == 1)
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}

							if (!TMFieldScene::m_bPK)
							{
								if (pNode->m_dwID > 0 && pNode->m_dwID < 1000 && g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
								{
									if (!g_bCastleWar)
									{
										pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
										continue;
									}

									if (m_pMyHuman->m_cMantua && pNode->m_cMantua == m_pMyHuman->m_cMantua)
									{
										pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
										continue;
									}
								}

								if (pNode->m_cSummons == 1 && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild)
								{
									if (!g_bCastleWar)
									{
										pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
										continue;
									}

									if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
									{
										pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
										continue;
									}
								}

								if (pNode->m_cSummons == 1 && !pNode->m_usGuild)
								{
									if (!g_bCastleWar)
									{
										pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
										continue;
									}

									if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
									{
										pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
										continue;
									}
								}
							}

							if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua > 0 && m_pMyHuman->m_cMantua == pNode->m_cMantua &&
								!TMFieldScene::m_bPK)
							{
								if (!g_bCastleWar)
								{
									pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
									continue;
								}

								if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000)
								{
									pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
									continue;
								}
							}

							if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pNode->IsInCastleZone())
							{
								if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
								{
									pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
									continue;
								}
							}

							if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000 &&
								!pNode->m_bParty && !pNode->m_cSummons && pNode->IsMerchant())
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}
						}

						if (pNode->m_cDie == 1)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}

						stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
						stAttack.Dam[nTargetIndex++].Damage = -1;
						stAttack.TargetX = nDX;
						stAttack.TargetY = nDY;
					}
				}

				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				if (g_pSpell[(unsigned char)cSkillIndex].MaxTarget <= nTargetIndex || nTargetIndex >= 13)
					break;
			};
		}
		else if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 4 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 6)
		{
			stAttack.FlagLocal = 0;
			int nTargetIndex = 0;
			int nCritical = (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.Critical;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			if (!pNode)
				return 1;

			int nSX = (int)vec.x;
			int nSY = (int)vec.z;
			if (!(int)vec.x && !nSY && pOver)
			{
				nSX = (signed int)pOver->m_vecPosition.x;
				nSY = (signed int)pOver->m_vecPosition.y;
			}

			int nMobCount = 0;

			while (pNode != nullptr && nMobCount < 1000)
			{
				++nMobCount;
				int x2 = (int)pNode->m_vecPosition.x;
				int y2 = (int)pNode->m_vecPosition.y;

				if (skill_request::AimsAtPrimaryTarget(cSkillIndex))
				{
					if (!pOver)
						return 1;

					nSX = (int)pOver->m_vecPosition.x;
					nSY = (int)pOver->m_vecPosition.y;

					if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
					{
						int nMX = (int)m_pMyHuman->m_vecPosition.x;
						int nMY = (int)m_pMyHuman->m_vecPosition.y;
						if (m_stMoveStop.NextX)
						{
							nMX = m_stMoveStop.NextX;
							nMY = m_stMoveStop.NextY;
						}

						int nDistanceFromMe = BASE_GetDistance(nMX, nMY, (int)pOver->m_vecPosition.x, (int)pOver->m_vecPosition.y) - 12;
						if (nDistanceFromMe < 1)
						{
							nSX = (signed int)pOver->m_vecPosition.x;
							nSY = (signed int)pOver->m_vecPosition.y;
						}
					}
				}
				if (cSkillIndex == 95 && pNode != pOver)
				{
					if (!pOver)
						return 0;

					pNode = (TMHuman*)pNode->m_pNextLink;
					nSX = (int)pOver->m_vecPosition.x;
					nSY = (int)pOver->m_vecPosition.y;
					continue;
				}

				if (pNode && pNode->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 &&
					(m_pMyHuman->m_usGuild == pNode->m_usGuild || g_pObjectManager->m_usAllyGuild == pNode->m_usGuild))
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int nMX = (int)m_pMyHuman->m_vecPosition.x;
				int nMY = (int)m_pMyHuman->m_vecPosition.y;
				if (m_stMoveStop.NextX)
				{
					nMX = m_stMoveStop.NextX;
					nMY = m_stMoveStop.NextY;
				}

				int nDistanceFromMe = BASE_GetDistance(nMX, nMY, nSX, nSY);
				if (cSkillIndex == 0 && nDistanceFromMe == 2 && (nSX == (int)m_pMyHuman->m_vecPosition.x || nSY == (int)m_pMyHuman->m_vecPosition.y))
				{
					nDistanceFromMe = 1;
				}
				if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
				{
					nDistanceFromMe -= 12;
					if (nDistanceFromMe < 0)
						nDistanceFromMe = 0;
				}
				if (skill_request::ChecksPrimaryTargetRange(cSkillIndex, skill_request::Invocation::Manual))
				{
					if (nDistanceFromMe > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range && g_pSpell[(unsigned char)cSkillIndex].Range != -1)
					{
						if (cSkillIndex == 35 || cSkillIndex == 39 || cSkillIndex == 97)
							return 0;

						int tx = nSX;
						int ty = nSY;
						bool bHit = true;

						BASE_GetHitPosition((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y, &tx, &ty, (char*)m_HeightMapData, 8);

						if (tx != nSX || ty != nSY)
							bHit = false;

						if (!bMoving)
							return 0;

						if (nDistanceFromMe < nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
							return 0;

						if (!bHit)
							return 0;

						if ((int)m_pMyHuman->m_eMotion >= 4 && (int)m_pMyHuman->m_eMotion <= 9)
						{
							unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
							if (dwMod > 2)
								dwMod -= 2;

							if (g_pEventTranslator->button[0])
							{
								if (dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
									return 1;
							}
						}

						if (m_pGround == nullptr)
							return 0;

						if (vec.y < -5000.0f && pOver)
						{
							vec.x = pOver->m_vecPosition.x;
							vec.z = pOver->m_vecPosition.y;
							vec.y = pOver->m_fHeight;
						}

						m_pMyHuman->SetSpeed(m_bMountDead);
						if (vec.x == m_pMyHuman->m_vecPosition.x && vec.z == m_pMyHuman->m_vecPosition.y)
							return 1;

						m_pTargetItem = nullptr;
						m_pMyHuman->m_pMoveSkillTargetHuman = pOver;
						m_dwLastSetTargetHuman = dwServerTime;
						if (m_pMyHuman->m_cCantMove)
							return 0;

						int nMoveSX = (int)m_pMyHuman->m_vecPosition.x;
						int nMoveSY = (int)m_pMyHuman->m_vecPosition.y;
						if (m_stMoveStop.NextX)
						{
							nMoveSX = m_stMoveStop.NextX;
							nMoveSY = m_stMoveStop.NextY;
						}

						if (!pOver)
							return 0;

						tx = (int)pOver->m_vecPosition.x;
						ty = (int)pOver->m_vecPosition.y;

						int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, tx, ty);
						int PlusX = 1;
						int PlusY = 1;
						if (nMoveSX > tx)
							PlusX = -1;
						if (nMoveSY > ty)
							PlusY = -1;

						int nBreak = 0;
						while (nMoveDistance2 > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
						{
							if (nBreak > 500)
								return 1;

							if (nMoveSX != tx)
								nMoveSX += PlusX;
							if (nMoveSY != ty)
								nMoveSY += PlusY;

							nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, tx, ty);
							++nBreak;
						}

						m_vecMyNext.x = nMoveSX;
						m_vecMyNext.y = nMoveSY;
						return 0;
					}
					if (cSkillIndex == 97)
					{
						if (nDistanceFromMe < 4)
							return 1;

						auto pCannon = (TMItem*)g_pObjectManager->GetItemByID(nCannonIndex);
						float fAngle = atan2f((float)(nSX - (int)m_pMyHuman->m_vecPosition.x), (float)(nSY - (int)m_pMyHuman->m_vecPosition.y));

						if (fAngle < 0.0f)
							fAngle = fAngle + D3DXToRadian(360);
						if (pCannon->m_fAngle - D3DXToRadian(90) > fAngle)
							return 1;
						if (fAngle > pCannon->m_fAngle + D3DXToRadian(90))
							return 1;
					}
					if (cSkillIndex == 35)
					{
						if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_bParty == 1) &&
							g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && !pOver->IsInPKZone())
						{
							return 1;
						}
					}
					if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
					{
						if (nDistanceFromMe > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
							return 1;
					}

					int sx = (int)m_pMyHuman->m_vecPosition.x;
					int sy = (int)m_pMyHuman->m_vecPosition.y;
					int nDTX = nSX;
					int nDTY = nSY;
					BASE_GetHitPosition(sx, sy, &nDTX, &nDTY, (char*)m_HeightMapData, 8);
					if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
					{
						nDTX = nSX;
						nDTY = nSY;
					}

					if (nDTX != nSX || nDTY != nSY)
						return 1;
				}
				else
				{
					nSX = (int)m_pMyHuman->m_vecPosition.x;
					nSY = (int)m_pMyHuman->m_vecPosition.y;
					if (m_stMoveStop.NextX)
					{
						nSX = m_stMoveStop.NextX;
						nSY = m_stMoveStop.NextY;
					}
				}

				int nDistance = BASE_GetDistance(nSX, nSY, x2, y2);
				int nTX = x2;
				int nTY = y2;
				BASE_GetHitPosition2(nSX, nSY, &nTX, &nTY, (char*)m_HeightMapData, 8);
				const int nGridDistance = skill_request::AreaRadius(
					g_pSpell[(unsigned char)cSkillIndex].TargetType);

				if (pNode->IsInPKZone() == 1
					&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
					&& m_pMyHuman)
				{
					if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
					{
						if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
						{
							if (!g_bCastleWar)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}
					}

					if (TMFieldScene::m_bPK == 1
						&& g_bCastleWar > 0
						&& m_pMyHuman->m_cMantua > 0
						&& pOver
						&& pOver->m_cMantua > 0
						&& m_pMyHuman->m_cMantua == pOver->m_cMantua
						&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
					{
						int isInPos = (int)m_pMyHuman->m_vecPosition.x >> 7 != 8 && (int)m_pMyHuman->m_vecPosition.x >> 7 != 9 ||
									  (int)m_pMyHuman->m_vecPosition.y >> 7 != 15 && (int)m_pMyHuman->m_vecPosition.y >> 7 != 16 ? 0 : 1;

						if (isInPos)
							return 1;
					}

					if (!TMFieldScene::m_bPK
						&& pNode->m_cSummons == 1
						&& g_pObjectManager->m_usWarGuild != pNode->m_usGuild
						&& pNode->m_usGuild)
					{
						if (!g_bCastleWar)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}

						if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}
					}

					if (!TMFieldScene::m_bPK && pNode->m_cSummons == 1 && !pNode->m_usGuild)
					{
						if (!g_bCastleWar)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}

						if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}
					}
					if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua > 0 && m_pMyHuman->m_cMantua == pNode->m_cMantua && !TMFieldScene::m_bPK)
					{
						if (!g_bCastleWar)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}

						if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}
					}

					if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pNode->IsInCastleZone())
					{
						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}
					}

					if (!TMFieldScene::m_bPK
						&& g_bCastleWar > 0
						&& m_pMyHuman->m_cMantua == 3)
					{
						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}

						if (pNode->m_cMantua > 0 && pNode->m_cMantua != 4)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}
					}

					if (pNode->m_bParty == 1)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}

					if (!TMFieldScene::m_bPK)
					{
						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							if (!pNode->m_cMantua && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !g_bCastleWar)
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}
						}
					}
				}

				if ((pNode->m_dwID <= 0 || pNode->m_dwID >= 1000) && !pNode->m_bParty && !pNode->m_cSummons && pNode->IsMerchant())
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				if (pNode->m_cDie == 1)
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}
				if (pNode->IsInPKZone() == 1 && !m_pMyHuman->IsInPKZone() && !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				if (skill_request::ReachesAreaTarget(nDistance, nGridDistance, nTX, nTY, x2, y2)
					&& pNode != m_pMyHuman
					&& (!pNode->IsInTown() || !g_pSpell[(unsigned char)cSkillIndex].Aggressive))
				{
					if ((pNode->m_dwID <= 0 || pNode->m_dwID >= 1000 && !pNode->m_bParty && !pNode->m_cSummons) ||
						(pNode->m_dwID > 0 && pNode->m_dwID < 1000 && (!g_pSpell[(unsigned char)cSkillIndex].Aggressive || pNode->IsInPKZone())))
					{
						if (g_pSpell[(unsigned char)cSkillIndex].Range == -1 && !nTargetIndex)
						{
							stAttack.Dam[0].TargetID = pNode->m_dwID;
							stAttack.Dam[0].Damage = -2;
							stAttack.Progress = TMFieldScene::m_usProgress;
							nTargetIndex = 1;
						}

						stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
						stAttack.Dam[nTargetIndex].Damage = -1;
						if (pOver
							&& pOver == pNode
							&& nTargetIndex
							&& (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3
								|| g_pSpell[(unsigned char)cSkillIndex].TargetType == 4
								|| g_pSpell[(unsigned char)cSkillIndex].TargetType == 6))
						{
							unsigned int dwID = stAttack.Dam[0].TargetID;
							int nDamage = stAttack.Dam[0].Damage;
							stAttack.Dam[0].TargetID = stAttack.Dam[nTargetIndex].TargetID;
							stAttack.Dam[0].Damage = stAttack.Dam[nTargetIndex].Damage;
							stAttack.Dam[nTargetIndex].TargetID = dwID;
							stAttack.Dam[nTargetIndex].Damage = nDamage;
						}

						++nTargetIndex;
					}
				}

				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				if (skill_request::AreaTargetLimitReached(
					g_pSpell[(unsigned char)cSkillIndex].TargetType, nTargetIndex,
					g_pSpell[(unsigned char)cSkillIndex].MaxTarget))
				{
					break;
				}
			}

			stAttack.TargetX = nSX;
			stAttack.TargetY = nSY;
		}
		else if (cSkillIndex == 29 || cSkillIndex == 44)
		{
			if (m_pMyHuman->m_bParty == 1)
			{
				int nIndex = 0;
				int nIndexCount = 0;

				stIDDis stPartys[12]{};

				auto pPartyList = m_pPartyList;
				for (int k = 0; k < pPartyList->m_nNumItem; ++k)
				{
					auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[k];
					auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);

					if (pHuman && (m_pMyHuman->IsInPKZone() || pHuman->IsInPKZone() != 1))
					{
						int nMX = (signed int)m_pMyHuman->m_vecPosition.x;
						int nMY = (signed int)m_pMyHuman->m_vecPosition.y;
						if (m_stMoveStop.NextX)
						{
							nMX = m_stMoveStop.NextX;
							nMY = m_stMoveStop.NextY;
						}

						int nDistanceFromMe = BASE_GetDistance(nMX, nMY, (int)pHuman->m_vecPosition.x, (int)pHuman->m_vecPosition.y);
						if (nDistanceFromMe < nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
						{
							stPartys[nIndex].dwID = pPartyItem->m_dwCharID;
							stPartys[nIndex++].nLen = nDistanceFromMe;
						}
					}

					nIndexCount = nIndex;
					if (nIndex >= 12)
						nIndexCount = 12;

					int nMaxIndex = 0;

					for (int nIndex = 0; nIndex < nIndexCount; ++nIndex)
					{
						nMaxIndex = nIndex;

						stIDDis stMax{};
						stMax.dwID = stPartys[nIndex].dwID;
						stMax.nLen = stPartys[nIndex].nLen;
						for (int j = nIndex + 1; j < nIndexCount; ++j)
						{
							if (stMax.nLen > stPartys[j].nLen)
							{
								stMax.dwID = stPartys[j].dwID;
								stMax.nLen = stPartys[j].nLen;
								nMaxIndex = j;
							}
						}

						stPartys[nMaxIndex].dwID = stPartys[nIndex].dwID;
						stPartys[nMaxIndex].nLen = stPartys[nIndex].nLen;
						stPartys[nIndex].dwID = stMax.dwID;
						stPartys[nIndex].nLen = stMax.nLen;
					}
					int nTargetI = 0;
					for (nIndex = 0; nIndex < nIndexCount && g_pSpell[(unsigned char)cSkillIndex].MaxTarget > nTargetI && nTargetI < 13; ++nIndex)
					{
						stAttack.Dam[nTargetI].TargetID = stPartys[nIndex].dwID;
						stAttack.Dam[nTargetI++].Damage = -1;
					}
				}
			}
			else
			{
				stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
				stAttack.Dam[0].Damage = -1;
			}
		}
		else
		{
			if (cSkillIndex == 42)
			{
				if (m_pMyHuman->m_bParty == 1)
					m_pMessagePanel->SetMessage(g_pMessageStringTable[31], 1000);
				else
					m_pMessagePanel->SetMessage(g_pMessageStringTable[32], 1000);

				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
			if (cSkillIndex == 73)
			{
				if (g_pObjectManager->m_stMobData.CurrentScore.CurMP < g_pSpell[73].ManaSpent)
					return 1;

				if (dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000)
				{
					int targetx = (signed int)vec.x;
					int targety = (int)vec.z;
					int x = (int)m_pMyHuman->m_vecPosition.x;
					int y = (int)m_pMyHuman->m_vecPosition.y;

					char cRouteBuffer[48]{};
					BASE_GetRoute(x, y, &targetx, &targety, cRouteBuffer, 8, (char*)m_HeightMapData, 8);
					if (!strlen(cRouteBuffer))
						return 1;

					MSG_Action stAction{};
					stAction.Header.ID = m_pMyHuman->m_dwID;
					stAction.PosX = x;
					stAction.PosY = y;
					stAction.Effect = 6;
					stAction.Header.Type = MSG_Action2_Opcode;
					stAction.Speed = g_nMyHumanSpeed;
					stAction.TargetX = targetx;
					stAction.TargetY = targety;
					m_stMoveStop.LastX = x;
					m_stMoveStop.LastY = stAction.PosY;
					m_stMoveStop.NextX = stAction.TargetX;
					m_stMoveStop.NextY = stAction.TargetY;
					SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAction)->Type, reinterpret_cast<char*>(&stAction), sizeof(stAction)});
					IncSkillSel();
					m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
					m_pMyHuman->m_dwOldMovePacketTime = dwServerTime;
				}
				return 1;
			}

			stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
			stAttack.Dam[0].Damage = -1;
			stAttack.TargetX = (int)m_pMyHuman->m_vecPosition.x;
			stAttack.TargetY = (int)m_pMyHuman->m_vecPosition.y;

			if (m_stMoveStop.NextX)
			{
				stAttack.TargetX = m_stMoveStop.NextX;
				stAttack.TargetY = m_stMoveStop.NextY;
			}
			if (cSkillIndex == 56
				|| cSkillIndex == 57
				|| cSkillIndex == 58
				|| cSkillIndex == 59
				|| cSkillIndex == 60
				|| cSkillIndex == 61
				|| cSkillIndex == 62
				|| cSkillIndex == 63
				|| (unsigned char)cSkillIndex == 200
				|| (unsigned char)cSkillIndex == 216)
			{
				m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
				m_pMyHuman->m_dwOldMovePacketTime = dwServerTime;
			}
		}

		if (cSkillIndex == 98)
		{
			int nTX = (int)vec.x;
			int nTY = (int)vec.z;
			int nMX = (int)m_pMyHuman->m_vecPosition.x;
			int nMY = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				nMX = m_stMoveStop.NextX;
				nMY = m_stMoveStop.NextY;
			}

			int nDistance = BASE_GetDistance(nMX, nMY, nTX, nTY);
			int nMobAttackRange = nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range;
			int nDX = nTX;
			int nDY = nTY;
			BASE_GetHitPosition(nMX, nMY, &nDX, &nDY, (char*)m_HeightMapData, 8);
			if (nMobAttackRange == -1)
				nMobAttackRange = 1;

			if (BASE_GetItemAbility(&pMobData->Equip[6], 21) < 100)
			{
				if (nSpecForce + BASE_GetMobAbility(pMobData, 27) > nMobAttackRange)
					nMobAttackRange = nSpecForce + BASE_GetMobAbility(pMobData, 27);
			}

			BASE_GetDistance(nMX, nMY, nDX, nDY);
			if (nDistance > nMobAttackRange || nDX != nTX || nDY != nTY)
				return 1;

			if (BASE_GetDistance(nMX, nMY, (int)vec.x, (int)vec.z) > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
				return 1;
			stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
			stAttack.Dam[0].Damage = -1;
			stAttack.TargetX = (int)vec.x;
			stAttack.TargetY = (int)vec.z;
			int bValue = g_pAttribute[(int)stAttack.TargetY >> 2][(int)stAttack.TargetX >> 2];
			if (bValue & 1 && !(bValue & 0x40))
				return 1;
		}
		if (cSkillIndex == 102)
		{
			int nCls = m_pMyHuman->m_sHeadIndex % 10;
			if (!IsValidSkill(31) && !IsValidSkill(39) && !IsValidSkill(47))
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[357], 1000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
		}
		if ((int)stAttack.Dam[0].TargetID > 0 || cSkillIndex == 97 || cSkillIndex == 35 || cSkillIndex == 51)
		{
			for (int l = g_pSpell[(unsigned char)cSkillIndex].MaxTarget; l < 13; ++l)
			{
				stAttack.Dam[l].TargetID = 0;
				stAttack.Dam[l].Damage = 0;
			}

			const int nSize = skill_attack::SelectEnvelope(stAttack,
				g_pSpell[(unsigned char)cSkillIndex].MaxTarget);

			MSG_Attack stAttackLocal{};
			memcpy(&stAttackLocal, &stAttack, nSize);
			stAttackLocal.Header.ID = m_dwID;
			stAttackLocal.FlagLocal = 1;
			if (nSpecForce)
				stAttackLocal.DoubleCritical |= 8;

			OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
			SendOneMessage((char*)&stAttack, nSize);
			IncSkillSel();
			m_dwOldAttackTime = dwServerTime;
			m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
		}
		if (cSkillIndex != 85)
			return 1;
	}

	if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 2 && !pOver)
	{
		pOver = m_pMyHuman;
		pOver->m_bMouseOver = 1;
	}
	if (!pOver || pOver->m_bMouseOver != 1 && !pTarget || m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK || m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
	{
		m_pMouseOverHuman = 0;
		return GetItemFromGround(dwServerTime);
	}

	if (m_pMyHuman->m_cHide == 1)
		return 1;
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 1;

	int Special = m_pMyHuman->m_stScore.Level;
	if ((unsigned char)cSkillIndex < 96)
		Special = g_pObjectManager->m_stMobData.CurrentScore.Mastery[((unsigned char)cSkillIndex - 24 * (unsigned char)g_pObjectManager->m_stMobData.Class) / 8 + 1];

	if (BASE_GetManaSpent((unsigned char)cSkillIndex, (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.SaveMana, Special) > g_pObjectManager->m_stMobData.CurrentScore.CurMP)
	{
		auto ipNewItem = new SListBoxItem(g_pMessageStringTable[30],
			0xFFFFAAAA,
			0.0f,
			0.0f,
			300.0f,
			16.0f,
			0,
			0x77777777,
			1u,
			0);

		m_pChatList->AddItem(ipNewItem);

		GetSoundAndPlay(33, 0, 0);
		return 1;
	}

	if (pOver->IsInPKZone() == 1 && !m_pMyHuman->IsInPKZone())
		return 1;
	if ((m_pMyHuman->IsInTown() == 1 || pOver->IsInTown() == 1) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		return 1;

	if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_bParty == 1)
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& !pOver->IsInPKZone())
	{
		return 1;
	}
	if ((pOver->m_dwID && pOver->m_dwID < 1000 || pOver->m_bParty == 1)
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& !m_pMyHuman->IsInPKZone())
	{
		return 1;
	}
	if (pOver->IsInPKZone() == 1
		&& !m_pMyHuman->IsInPKZone()
		&& !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		return 1;
	}
	if (!pOver->IsInPKZone()
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (pOver->m_cSummons == 1 || pOver->m_bParty == 1))
	{
		return 1;
	}
	if (!TMFieldScene::m_bPK
		&& pOver->m_cSummons == 1
		&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
		&& pOver->m_usGuild)
	{
		if (!g_bCastleWar)
			return 1;
		if (m_pMyHuman->m_cMantua > 0
			&& m_pMyHuman->m_cMantua == pOver->m_cMantua
			&& !pOver->IsInCastleZone())
		{
			return 1;
		}
	}
	if (!TMFieldScene::m_bPK)
	{
		if (pOver->m_usGuild)
		{
			if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
			{
				if (pOver->m_dwID > 0 && pOver->m_dwID < 1000 && pOver->IsInPKZone() == 1)
				{
					if (!g_bCastleWar)
						return 1;
					if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
						return 1;
				}
			}
		}
	}

	if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
	{
		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cMantua > 0 && pOver->m_cMantua != 4) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			return 1;
		}
	}
	if (!g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (pOver->m_dwID <= 0 && pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons)
			return 1;
	}
	if (pOver
		&& pOver->m_usGuild
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (m_pMyHuman->m_usGuild == pOver->m_usGuild
			|| g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
	{
		return 1;
	}
	if (pOver && !TMFieldScene::m_bPK && pOver->m_cSummons == 1 && !pOver->m_usGuild)
	{
		if (!g_bCastleWar)
			return 1;
		if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
			return 1;
	}

	if (pOver->m_dwID <= 0 && pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons && pOver->IsMerchant()
		|| pOver->m_TradeDesc[0]
		|| dwServerTime <= m_dwOldAttackTime + 1000)
	{
		return GetItemFromGround(dwServerTime);
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 15 && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		if (!m_pMyHuman->m_cMantua)
			return 1;
		if (pOver->m_cMantua > 0
			&& m_pMyHuman->m_cMantua > 0
			&& pOver->m_cMantua == m_pMyHuman->m_cMantua)
		{
			return 1;
		}
	}

	int nTX = (int)pOver->m_vecPosition.x;
	int nTY = (int)pOver->m_vecPosition.y;
	int nSX = (int)m_pMyHuman->m_vecPosition.x;
	int nSY = (int)m_pMyHuman->m_vecPosition.y;
	if (m_stMoveStop.NextX)
	{
		nSX = m_stMoveStop.NextX;
		nSY = m_stMoveStop.NextY;
	}

	int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
	int nMobAttackRange = nSpecForce + g_pSpell[(unsigned __int8)cSkillIndex].Range;
	int nDX = nTX;
	int nDY = nTY;
	BASE_GetHitPosition(nSX, nSY, &nDX, &nDY, (char*)m_HeightMapData, 8);
	if (nMobAttackRange == -1)
		nMobAttackRange = 1;

	if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
	{
		nDistance -= 12;
		if (nDistance < 0)
			nDistance = 0;
		nDX = (signed int)pOver->m_vecPosition.x;
		nDY = (signed int)pOver->m_vecPosition.y;
	}
	if (BASE_GetItemAbility(&pMobData->Equip[6], 21) < 100)
	{
		if (nSpecForce + BASE_GetMobAbility(pMobData, 27) > nMobAttackRange)
			nMobAttackRange = nSpecForce + BASE_GetMobAbility(pMobData, 27);
	}

	if (nDistance > nMobAttackRange || nDX != nTX || nDY != nTY)
	{
		if (bMoving == 1 && nDistance >= nMobAttackRange && nDX == nTX && nDY == nTY)
		{
			if ((signed int)m_pMyHuman->m_eMotion >= 4 && (signed int)m_pMyHuman->m_eMotion <= 9)
			{
				unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
				if (dwMod > 2)
					dwMod -= 2;
				if (g_pEventTranslator->button[0] && dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
				{
					return 1;
				}
			}

			if (!m_pGround)
				return 0;
			if (vec.y < -5000.0f && pOver)
			{
				vec.x = pOver->m_vecPosition.x;
				vec.z = pOver->m_vecPosition.y;
				vec.y = pOver->m_fHeight;
			}

			m_pMyHuman->SetSpeed(m_bMountDead);
			if (vec.x == m_pMyHuman->m_vecPosition.x && vec.z == m_pMyHuman->m_vecPosition.y)
				return 1;

			m_pTargetItem = nullptr;
			m_pMyHuman->m_pMoveSkillTargetHuman = pOver;
			m_dwLastSetTargetHuman = dwServerTime;
			if (m_pMyHuman->m_cCantMove)
				return 0;

			int nMoveSX = (int)m_pMyHuman->m_vecPosition.x;
			int nMoveSY = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				nMoveSX = m_stMoveStop.NextX;
				nMoveSY = m_stMoveStop.NextY;
			}

			if (!pOver)
				return 0;

			nTX = (int)pOver->m_vecPosition.x;
			nTY = (int)pOver->m_vecPosition.y;

			int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
			int PlusX = 1;
			int PlusY = 1;
			if (nMoveSX > nTX)
				PlusX = -1;
			if (nMoveSY > nTY)
				PlusY = -1;

			int nBreak = 0;
			while (nMoveDistance2 > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
			{
				if (nBreak > 500)
					return 1;

				if (nMoveSX != nTX)
					nMoveSX += PlusX;
				if (nMoveSY != nTY)
					nMoveSY += PlusY;

				nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
				++nBreak;
			}

			m_vecMyNext.x = nMoveSX;
			m_vecMyNext.y = nMoveSY;
		}
		return GetItemFromGround(dwServerTime);
	}

	if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
	{
		if (nDistance > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
			return 1;
	}

	if (!TMFieldScene::m_bPK && !m_pMyHuman->m_cMantua)
	{
		if (pOver->m_dwID > 0 && pOver->m_dwID < 1000
			&& pOver->IsInPKZone() == 1
			&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
			&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			return 1;
		}
	}

	if (!TMFieldScene::m_bPK
		&& g_bCastleWar > 0
		&& m_pMyHuman->m_cMantua > 0
		&& pOver->m_cMantua > 0
		&& m_pMyHuman->m_cMantua == pOver->m_cMantua
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		return 1;
	}

	if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pOver->m_usGuild &&
		pOver->m_dwID > 0 && pOver->m_dwID < 1000 && g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (!g_bCastleWar)
			return 1;
		if (m_pMyHuman->m_cMantua == pOver->m_cMantua)
			return 1;
	}

	if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pOver->IsInCastleZone())
	{
		int isInPos = ((int)m_pMyHuman->m_vecPosition.x >> 7 == 8 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 9) &&
			((int)m_pMyHuman->m_vecPosition.y >> 7 == 15 || (int)m_pMyHuman->m_vecPosition.y >> 7 == 16);
		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cSummons == 1) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && !isInPos)
		{
			return 1;
		}
	}

	auto stAttack = skill_attack::Direct<MSG_Attack>(m_pMyHuman->m_dwID,
		(unsigned char)cSkillIndex, (int)m_pMyHuman->m_vecPosition.x,
		(int)m_pMyHuman->m_vecPosition.y, m_stMoveStop.NextX, m_stMoveStop.NextY,
		pOver->m_dwID);

	pTarget = (TMHuman*)g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
	if (!pTarget)
		return 1;

	int nTargetIndex = 0;
	if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
	{
		stAttack.Dam[nTargetIndex].TargetID = pTarget->m_dwID;
		stAttack.Dam[nTargetIndex].Damage = -2;
		stAttack.Progress = TMFieldScene::m_usProgress;
		++nTargetIndex;
	}
	stAttack.Dam[nTargetIndex].TargetID = pTarget->m_dwID;
	stAttack.Dam[nTargetIndex++].Damage = -1;
	if (cSkillIndex == 16 || cSkillIndex == 12 || cSkillIndex == 28)
	{
		nDX = (int)pTarget->m_vecPosition.x - (int)m_pMyHuman->m_vecPosition.x;
		nDY = (int)pTarget->m_vecPosition.y - (int)m_pMyHuman->m_vecPosition.y;
		if (nDX > 0)
			nDX = 1;
		else if (nDX < 0)
			nDX = -1;
		if (nDY > 0)
			nDY = 1;
		else if (nDY < 0)
			nDY = -1;

		int TX = nDX + (int)pTarget->m_vecPosition.x;
		int TY = nDY + (int)pTarget->m_vecPosition.y;
		TMHuman* pNode = (TMHuman*)m_pHumanContainer->m_pDown;

		while (pNode->m_pNextLink)
		{
			if (pNode == m_pMyHuman	|| pNode == pTarget
				|| (int)pNode->m_vecPosition.x != TX
				|| (int)pNode->m_vecPosition.y != TY)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
			{
				if (!pNode->IsInPKZone() || !TMFieldScene::m_bPK || pNode->m_bParty == 1)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !g_bCastleWar)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK
					&& m_pMyHuman->m_cMantua > 0
					&& m_pMyHuman->m_cMantua == pNode->m_cMantua
					&& g_bCastleWar > 0)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK
					&& g_bCastleWar > 0
					&& m_pMyHuman->m_cMantua == 3
					&& (pNode->m_dwID > 0 && pNode->m_dwID < 1000 || (pNode->m_cMantua > 0 && pNode->m_cMantua != 4)))
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pNode->IsInCastleZone())
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
			}
			else if (!TMFieldScene::m_bPK
				&& pNode->m_cSummons == 1
				&& (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild || !pNode->m_usGuild))
			{
				if (!pNode->IsInCastleZone() && g_bCastleWar > 0 || !g_bCastleWar)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
			}
			else if (pOver && !TMFieldScene::m_bPK && pNode->m_cSummons == 1 && pNode->m_usGuild)
			{
				if (!pNode->IsInCastleZone() && g_bCastleWar > 0 || !g_bCastleWar)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
			}

			stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
			stAttack.Dam[nTargetIndex++].Damage = -1;
			break;
		}
	}

	stAttack.TargetX = (int)pOver->m_vecPosition.x;
	stAttack.TargetY = (int)pOver->m_vecPosition.y;
	if (cSkillIndex)
	{
		stAttack.PosX = m_stMoveStop.NextX;
		stAttack.PosY = m_stMoveStop.NextY;
		stAttack.TargetX = m_stMoveStop.NextX;
		stAttack.TargetY = m_stMoveStop.NextY;
	}

	const int Size = skill_attack::SelectEnvelope(stAttack,
		g_pSpell[(unsigned char)cSkillIndex].MaxTarget);
	SendOneMessage((char*)&stAttack, Size);
	IncSkillSel();

	MSG_Attack stLocalAttack{};
	memcpy(&stLocalAttack, &stAttack, sizeof(stLocalAttack));

	stLocalAttack.Header.ID = m_dwID;
	stLocalAttack.FlagLocal = 1;

	if (nSpecForce)
		stLocalAttack.DoubleCritical |= 8u;

	OnPacketEvent(stAttack.Header.Type, (char*)&stLocalAttack);
	m_dwOldAttackTime = dwServerTime;
	m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
	m_pMyHuman->m_pMoveSkillTargetHuman = 0;

	return 1;
}

int TMFieldScene::AutoSkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)
{
	if (!pTarget)
		return 0;
	if (m_pMyHuman->m_cHide == 1)
		return 0;
	if (m_pPotalPanel && m_pPotalPanel->m_bVisible == 1)
		return 0;
	if (g_pEventTranslator->m_bAlt == 1)
		return 0;
	if (pTarget->IsMerchant())
		return 0;
	if (pTarget->m_cSummons == 1)
		return 0;
	if (pTarget->m_cDie == 1)
		return 0;
	if (m_pMyHuman->IsInTown() == 1)
		return 0;
	if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK && m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
		return 0;
	if (m_pMyHuman->m_cCantAttk)
		return 0;
	if (pTarget->m_nClass == 44 && pTarget->m_sHeadIndex == 219)
		return 0;
	if (m_pMyHuman->m_bSkillBlack == 1)
		return 0;

	dwServerTime = g_pApp->m_pTimerManager->GetServerTime();
	m_pTargetItem = 0;

	int nSpecForce = 0;
	auto pMobData = &g_pObjectManager->m_stMobData;
	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		nSpecForce = 1;

	auto pOver = pTarget;
	if (!pTarget)
		return 0;
	if (pOver->m_cShadow == 1 && pOver->m_nClass == 66 && !m_pMyHuman->m_JewelGlasses)
		return 0;

	char cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
	if ((unsigned char)cSkillIndex > 104)
		cSkillIndex += 95;
	if (cSkillIndex == 86)
	{
		if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000)
			return 0;
	}
	if (cSkillIndex == 6 && pOver->m_nClass == 66)
		return 0;

	if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000 == 1 && cSkillIndex != 27 && cSkillIndex != 29 && cSkillIndex != 44)
		return 0;

	/*if ( m_pMyHuman->m_sCostume >= 4150 && m_pMyHuman->m_sCostume < 4200 || m_pMyHuman->m_sCostume >= 4300 && m_pMyHuman->m_sCostume < 4420 && (cSkillIndex == 64 || cSkillIndex == 66 || cSkillIndex == 68 || cSkillIndex == 70 || cSkillIndex == 71))
		return 0;*/

	if (!IsValidClassSkill((unsigned char)cSkillIndex))
		return 0;

	if (dwServerTime < m_dwOldAttackTime + 1000)
		return 0;
	if (IsSkillCoolingDown((unsigned char)cSkillIndex, dwServerTime))
		return 0;

	int TarType = g_pSpell[(unsigned __int8)cSkillIndex].TargetType;
	if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 &&
		(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
		return 0;

	if (cSkillIndex == 79 && BASE_GetItemAbility(&pMobData->Equip[6], 21) != 101)
		return 0;

	if (skill_request::UsesAreaRequest(TarType))
	{
		int nSpecial = m_pMyHuman->m_stScore.Level;
		if ((unsigned char)cSkillIndex < 96)
			nSpecial = g_pObjectManager->m_stMobData.CurrentScore.Mastery[((unsigned char)cSkillIndex - 24 * (unsigned char)g_pObjectManager->m_stMobData.Class) / 8 + 1];

		if (BASE_GetManaSpent((unsigned char)cSkillIndex, (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.SaveMana, nSpecial) > g_pObjectManager->m_stMobData.CurrentScore.CurMP)
		{
			auto pChatList = m_pChatList;

			pChatList->AddItem(new SListBoxItem(g_pMessageStringTable[30],
				0xFFFFAAAA,
				0.0f,
				0.0f,
				300.0f,
				16.0f,
				0,
				0x77777777,
				1,
				0));

			GetSoundAndPlay(33, 0, 0);
			return 0;
		}

		auto stAttack = skill_attack::Area<MSG_Attack>(m_pMyHuman->m_dwID,
			(unsigned char)cSkillIndex, (int)m_pMyHuman->m_vecPosition.x,
			(int)m_pMyHuman->m_vecPosition.y, m_stMoveStop.NextX, m_stMoveStop.NextY);

		if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 5)
		{
			stAttack.FlagLocal = 0;
			int nTargetIndex = 0;
			int nCritical = (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.Critical;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			if (!pNode)
				return 0;

			float fMyAngle = atan2f(vec.x - m_pMyHuman->m_vecPosition.x,
				vec.z - m_pMyHuman->m_vecPosition.y);

			int nMastery = m_pMyHuman->m_stScore.Mastery[3] / 75;
			if (nMastery > 3)
				nMastery = 3;

			int nMX = (int)m_pMyHuman->m_vecPosition.x;
			int nMY = (int)m_pMyHuman->m_vecPosition.y;
			int nMobCount = 0;

			while (pNode && nMobCount <= 1000)
			{
				++nMobCount;

				if (pNode == m_pMyHuman)
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				int nDX = (int)pNode->m_vecPosition.x;
				int nDY = (int)pNode->m_vecPosition.y;
				int x1 = (int)m_pMyHuman->m_vecPosition.x;
				int y1 = (int)m_pMyHuman->m_vecPosition.y;
				if (m_stMoveStop.NextX)
				{
					x1 = m_stMoveStop.NextX;
					y1 = m_stMoveStop.NextY;
				}

				int nDistanceFromMe = BASE_GetDistance(x1, y1, nDX, nDY);
				if (pNode->m_nClass == 56 && !pNode->m_stLookInfo.FaceMesh)
				{
					nDistanceFromMe -= 12;
					if (nDistanceFromMe < 0)
						nDistanceFromMe = 0;
				}

				if (nDistanceFromMe <= nMastery + 3)
				{
					float fNodeAngle = atan2f(pNode->m_vecPosition.x - m_pMyHuman->m_vecPosition.x,
						pNode->m_vecPosition.y - m_pMyHuman->m_vecPosition.y);

					if (fabsf(fNodeAngle - fMyAngle) >= 0.78539819f)
					{
						int nDTX = nDX;
						int nDTY = nDY;
						BASE_GetHitPosition(x1, y1, &nDTX, &nDTY, (char*)m_HeightMapData, 8);
						if (pNode->m_nClass == 56 && !pNode->m_stLookInfo.FaceMesh)
						{
							nDTX = nDX;
							nDTY = nDY;
						}

						if (nDTX != nDX || nDTY != nDY)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}
						if (pNode->m_cDie == 1)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}

						if (!TMFieldScene::m_bPK && pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}

						stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
						stAttack.Dam[nTargetIndex++].Damage = -1;
						stAttack.TargetX = nDX;
						stAttack.TargetY = nDY;
					}
				}

				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				if (g_pSpell[(unsigned char)cSkillIndex].MaxTarget <= nTargetIndex || nTargetIndex >= 13)
					break;
			}
		}

		else if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 4 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 6)
		{
			stAttack.FlagLocal = 0;
			int nTargetIndex = 0;
			int nCritical = (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.Critical;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			if (!pNode)
				return 0;

			int nSX = (int)vec.x;
			int nSY = (int)vec.z;
			if (!(int)vec.x && !nSY && pOver)
			{
				nSX = (int)pOver->m_vecPosition.x;
				nSY = (int)pOver->m_vecPosition.y;
			}

			int nMobCount = 0;
			while (pNode && nMobCount <= 1000)
			{
				++nMobCount;

				int x2 = (int)pNode->m_vecPosition.x;
				int y2 = (int)pNode->m_vecPosition.y;

				if (skill_request::AimsAtPrimaryTarget(cSkillIndex))
				{
					if (!pOver)
						return 0;

					int nSX = (int)pOver->m_vecPosition.x;
					int nSY = (int)pOver->m_vecPosition.y;
					if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
					{
						int nDTX = (int)m_pMyHuman->m_vecPosition.x;
						int nDTY = (int)m_pMyHuman->m_vecPosition.y;
						if (m_stMoveStop.NextX)
						{
							nDTX = m_stMoveStop.NextX;
							nDTY = m_stMoveStop.NextY;
						}
						int nDistanceFromMe = BASE_GetDistance(
							nDTX,
							nDTY,
							(int)pOver->m_vecPosition.x,
							(int)pOver->m_vecPosition.y)
							- 12;
						if (nDistanceFromMe < 1)
						{
							nSX = (int)pOver->m_vecPosition.x;
							nSY = (int)pOver->m_vecPosition.y;
						}
					}
				}

				if (cSkillIndex == 79 && pNode != pOver)
				{
					if (!pOver)
						return 0;

					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					nSX = (int)pOver->m_vecPosition.x;
					nSY = (int)pOver->m_vecPosition.y;
					continue;
				}

				if (pNode && pNode->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 &&
					(m_pMyHuman->m_usGuild == pNode->m_usGuild || g_pObjectManager->m_usAllyGuild == pNode->m_usGuild))
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				int x1 = (int)m_pMyHuman->m_vecPosition.x;
				int y1 = (int)m_pMyHuman->m_vecPosition.y;
				if (m_stMoveStop.NextX)
				{
					x1 = m_stMoveStop.NextX;
					y1 = m_stMoveStop.NextY;
				}

				int nDistanceFromMe = BASE_GetDistance(x1, y1, nSX, nSY);

				if (!cSkillIndex && nDistanceFromMe == 2 && (nSX == (int)m_pMyHuman->m_vecPosition.x || nSY == (int)m_pMyHuman->m_vecPosition.y))
					nDistanceFromMe = 1;
				if (pOver)
				{
					if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
					{
						nDistanceFromMe -= 12;
						if (nDistanceFromMe < 0)
							nDistanceFromMe = 0;
					}
				}
				if (skill_request::ChecksPrimaryTargetRange(cSkillIndex, skill_request::Invocation::Automatic))
				{
					if (nDistanceFromMe > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range && g_pSpell[(unsigned char)cSkillIndex].Range != -1)
						return 0;
					if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
					{
						if (nDistanceFromMe > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
							return 0;
					}

					int sx = (int)m_pMyHuman->m_vecPosition.x;
					int sy = (int)m_pMyHuman->m_vecPosition.y;
					int tx = nSX;
					int ty = nSY;
					BASE_GetHitPosition(sx, sy, &tx, &ty, (char*)m_HeightMapData, 8);
					if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
					{
						tx = nSX;
						ty = nSY;
					}
					if (tx != nSX || ty != nSY)
						return 0;
				}
				else
				{
					nSX = (int)m_pMyHuman->m_vecPosition.x;
					nSY = (int)m_pMyHuman->m_vecPosition.y;
					if (m_stMoveStop.NextX)
					{
						nSX = m_stMoveStop.NextX;
						nSY = m_stMoveStop.NextY;
					}
				}

				int nDistance = BASE_GetDistance(nSX, nSY, x2, y2);
				int nTX = x2;
				int nTY = y2;
				BASE_GetHitPosition2(nSX, nSY, &nTX, &nTY, (char*)m_HeightMapData, 8);
				const int nGridDistance = skill_request::AreaRadius(
					g_pSpell[(unsigned char)cSkillIndex].TargetType);

				if (pNode->IsInPKZone() && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
				{
					if (pNode->m_bParty == 1)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}

					if (!TMFieldScene::m_bPK)
					{
						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
							{
								if (!g_bCastleWar)
								{
									pNode = (TMHuman*)pNode->m_pNextLink;
									continue;
								}
								if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua)
								{
									pNode = (TMHuman*)pNode->m_pNextLink;
									continue;
								}
							}
							if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !pNode->m_cMantua && !g_bCastleWar)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}

						if (pNode->m_cSummons == 1)
						{
							if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild)
							{
								if (!g_bCastleWar)
								{
									pNode = (TMHuman*)pNode->m_pNextLink;
									continue;
								}
								if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
								{
									pNode = (TMHuman*)pNode->m_pNextLink;
									continue;
								}
							}
							if (!pNode->m_usGuild)
							{
								if (!g_bCastleWar)
								{
									pNode = (TMHuman*)pNode->m_pNextLink;
									continue;
								}

								if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
								{
									pNode = (TMHuman*)pNode->m_pNextLink;
									continue;
								}
							}
						}
						if (g_bCastleWar > 0 && !pNode->IsInCastleZone() && pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}

						if (g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
						{
							if ((pNode->m_dwID > 0 && pNode->m_dwID < 1000) ||
								(pNode->m_cMantua > 0 && pNode->m_cMantua != 4))
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}
					}

					if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua > 0 && m_pMyHuman->m_cMantua == pNode->m_cMantua && !TMFieldScene::m_bPK)
					{
						if (!g_bCastleWar)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}

						if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}
					}
				}
				if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000 && !pNode->m_bParty && !pNode->m_cSummons && pNode->IsMerchant())
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (pNode->m_cDie == 1)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (pNode->IsInPKZone() == 1 && !m_pMyHuman->IsInPKZone() && !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				if (skill_request::ReachesAreaTarget(nDistance, nGridDistance, nTX, nTY, x2, y2)
					&& pNode != m_pMyHuman
					&& (!pNode->IsInTown() || !g_pSpell[(unsigned char)cSkillIndex].Aggressive))
				{
					if ((pNode->m_dwID <= 0 || pNode->m_dwID >= 1000 && !pNode->m_bParty && !pNode->m_cSummons) ||
						(pNode->m_dwID > 0 && pNode->m_dwID < 1000 && (!g_pSpell[(unsigned char)cSkillIndex].Aggressive || pNode->IsInPKZone())))
					{
						if (g_pSpell[(unsigned char)cSkillIndex].Range == -1 && !nTargetIndex)
						{
							stAttack.Dam[0].TargetID = pNode->m_dwID;
							stAttack.Dam[0].Damage = -2;
							stAttack.Progress = TMFieldScene::m_usProgress;
							nTargetIndex = 1;
						}

						stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
						stAttack.Dam[nTargetIndex].Damage = -1;
						if (pOver
							&& pOver == pNode
							&& nTargetIndex
							&& (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3
								|| g_pSpell[(unsigned char)cSkillIndex].TargetType == 4
								|| g_pSpell[(unsigned char)cSkillIndex].TargetType == 6))
						{
							unsigned int dwID = stAttack.Dam[0].TargetID;
							int nDamage = stAttack.Dam[0].Damage;
							stAttack.Dam[0].TargetID = stAttack.Dam[nTargetIndex].TargetID;
							stAttack.Dam[0].Damage = stAttack.Dam[nTargetIndex].Damage;
							stAttack.Dam[nTargetIndex].TargetID = dwID;
							stAttack.Dam[nTargetIndex].Damage = nDamage;
						}

						++nTargetIndex;
					}
				}

				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				if (skill_request::AreaTargetLimitReached(
					g_pSpell[(unsigned char)cSkillIndex].TargetType, nTargetIndex,
					g_pSpell[(unsigned char)cSkillIndex].MaxTarget))
				{
					break;
				}
			}

			stAttack.TargetX = nSX;
			stAttack.TargetY = nSY;
		}

		if ((int)stAttack.Dam[0].TargetID > 0 || cSkillIndex == 97 || cSkillIndex == 35 || cSkillIndex == 51)
		{
			for (int i = g_pSpell[(unsigned char)cSkillIndex].MaxTarget; i < 13; ++i)
			{
				stAttack.Dam[i].TargetID = 0;
				stAttack.Dam[i].Damage = 0;
			}

			const int nSize = skill_attack::SelectEnvelope(stAttack,
				g_pSpell[(unsigned char)cSkillIndex].MaxTarget);

			SendOneMessage((char*)&stAttack, nSize);
			IncSkillSel();

			MSG_Attack stAttackLocal{};
			memcpy(&stAttackLocal, &stAttack, nSize);
			stAttackLocal.Header.ID = m_dwID;
			stAttackLocal.FlagLocal = 1;
			if (nSpecForce)
				stAttackLocal.DoubleCritical |= 8;

			OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
			m_dwOldAttackTime = dwServerTime;
			m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
			return 1;
		}
		if (cSkillIndex == 29)
		{
			if (m_pMyHuman->m_bParty == 1)
			{
				int nIndex = 0;
				int nIndexCount = 0;

				stIDDis stPartys[12]{};

				auto pPartyList = m_pPartyList;
				for (int k = 0; k < pPartyList->m_nNumItem; ++k)
				{
					auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[k];
					auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);

					if (pHuman && (m_pMyHuman->IsInPKZone() || pHuman->IsInPKZone() != 1))
					{
						int nMX = (signed int)m_pMyHuman->m_vecPosition.x;
						int nMY = (signed int)m_pMyHuman->m_vecPosition.y;
						if (m_stMoveStop.NextX)
						{
							nMX = m_stMoveStop.NextX;
							nMY = m_stMoveStop.NextY;
						}

						int nDistanceFromMe = BASE_GetDistance(nMX, nMY, (int)pHuman->m_vecPosition.x, (int)pHuman->m_vecPosition.y);
						if (nDistanceFromMe < nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
						{
							stPartys[nIndex].dwID = pPartyItem->m_dwCharID;
							stPartys[nIndex++].nLen = nDistanceFromMe;
						}
					}

					nIndexCount = nIndex;
					if (nIndex >= 12)
						nIndexCount = 12;

					int nMaxIndex = 0;

					for (int nIndex = 0; nIndex < nIndexCount; ++nIndex)
					{
						nMaxIndex = nIndex;

						stIDDis stMax{};
						stMax.dwID = stPartys[nIndex].dwID;
						stMax.nLen = stPartys[nIndex].nLen;
						for (int j = nIndex + 1; j < nIndexCount; ++j)
						{
							if (stMax.nLen > stPartys[j].nLen)
							{
								stMax.dwID = stPartys[j].dwID;
								stMax.nLen = stPartys[j].nLen;
								nMaxIndex = j;
							}
						}

						stPartys[nMaxIndex].dwID = stPartys[nIndex].dwID;
						stPartys[nMaxIndex].nLen = stPartys[nIndex].nLen;
						stPartys[nIndex].dwID = stMax.dwID;
						stPartys[nIndex].nLen = stMax.nLen;
					}
					int nTargetI = 0;
					for (nIndex = 0; nIndex < nIndexCount && g_pSpell[(unsigned char)cSkillIndex].MaxTarget > nTargetI && nTargetI < 13; ++nIndex)
					{
						stAttack.Dam[nTargetI].TargetID = stPartys[nIndex].dwID;
						stAttack.Dam[nTargetI++].Damage = -1;
					}
				}
			}
			else
			{
				stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
				stAttack.Dam[0].Damage = -1;
			}

			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAttack)->Type, reinterpret_cast<char*>(&stAttack), sizeof(stAttack)});

			MSG_Attack stAttackLocal{};
			memcpy(&stAttackLocal, &stAttack, sizeof(MSG_Attack));
			stAttackLocal.Header.ID = m_dwID;
			stAttackLocal.FlagLocal = 1;
			if (nSpecForce)
				stAttackLocal.DoubleCritical |= 8;

			OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
			m_dwOldAttackTime = dwServerTime;
			m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
			return 1;
		}
		if (cSkillIndex == 27)
		{
			int nMX = (int)m_pMyHuman->m_vecPosition.x;
			int nMY = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				nMX = m_stMoveStop.NextX;
				nMY = m_stMoveStop.NextY;
			}

			int nTX = (int)pOver->m_vecPosition.x;
			int nTY = (int)pOver->m_vecPosition.y;
			int nDistance = BASE_GetDistance(nMX, nMY, nTX, nTY);
			int nMobRange = nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range;
			int nDX = nTX;
			int nDY = nTY;

			BASE_GetHitPosition(nMX, nMY, &nDX, &nDY, (char*)m_HeightMapData, 8);
			if (nDistance > nMobRange || nDX != nTX || nDY != nTY)
				return 1;

			int bMyValue = g_pAttribute[nMY / 4][nMX / 4];
			int bNodeValue = g_pAttribute[nTY / 4][nTX / 4];
			if (!(bMyValue & 0x40) && bNodeValue & 0x40)
				return 1;

			MSG_Attack Msg{};
			Msg.Header.Type = MSG_Attack_Multi_Opcode;
			Msg.Header.ID = m_pMyHuman->m_dwID;
			Msg.AttackerID = m_pMyHuman->m_dwID;
			Msg.PosX = (int)m_pMyHuman->m_vecPosition.x;
			Msg.PosY = (int)m_pMyHuman->m_vecPosition.y;
			Msg.CurrentMp = -1;
			Msg.SkillIndex = (unsigned char)cSkillIndex;
			Msg.SkillParm = 0;
			Msg.Motion = -1;
			Msg.Dam[0].TargetID = pOver->m_dwID;
			Msg.Dam[0].Damage = -1;
			Msg.TargetX = (int)m_pMyHuman->m_vecPosition.x;
			Msg.TargetY = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				Msg.PosX = m_stMoveStop.NextX;
				Msg.TargetX = Msg.PosX;
				Msg.PosY = m_stMoveStop.NextY;
				Msg.TargetY = Msg.PosY;
			}

			SendOneMessage((char*)&Msg, sizeof(Msg));

			MSG_Attack stAttackLocal{};
			memcpy(&stAttackLocal, &stAttack, sizeof(MSG_Attack));
			stAttackLocal.Header.ID = m_dwID;
			stAttackLocal.FlagLocal = 1;
			if (nSpecForce)
				stAttackLocal.DoubleCritical |= 8;

			OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
			m_dwOldAttackTime = dwServerTime;
			m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
			return 1;
		}
		if (cSkillIndex != 85)
			return 0;
	}

	if (!pOver || pOver->m_bMouseOver != 1 && !pTarget)
	{
		m_pMouseOverHuman = 0;
		GetItemFromGround(dwServerTime);
		return 0;
	}

	if (m_pMyHuman->m_cHide == 1)
		return 0;
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 0;

	int Special = m_pMyHuman->m_stScore.Level;
	if ((unsigned char)cSkillIndex < 96)
		Special = g_pObjectManager->m_stMobData.CurrentScore.Mastery[((unsigned char)cSkillIndex - 24 * (unsigned char)g_pObjectManager->m_stMobData.Class) / 8 + 1];

	if (BASE_GetManaSpent((unsigned char)cSkillIndex, (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.SaveMana, Special) > g_pObjectManager->m_stMobData.CurrentScore.CurMP)
	{
		auto ipNewItem = new SListBoxItem(g_pMessageStringTable[30],
			0xFFFFAAAA,
			0.0f,
			0.0f,
			300.0f,
			16.0f,
			0,
			0x77777777,
			1u,
			0);

		m_pChatList->AddItem(ipNewItem);

		GetSoundAndPlay(33, 0, 0);
		return 1;
	}

	if (g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		if ((m_pMyHuman->IsInTown() == 1 || pOver->IsInTown() == 1))
			return 0;

		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_bParty == 1))
		{
			if (!pOver->IsInPKZone())
				return 0;

			if (!m_pMyHuman->IsInPKZone())
				return 0;
		}
	}

	if (pOver->IsInPKZone() == 1
		&& !m_pMyHuman->IsInPKZone()
		&& !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		return 0;
	}
	if (!pOver->IsInPKZone()
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (pOver->m_cSummons == 1 || pOver->m_bParty == 1))
	{
		return 0;
	}

	if (!TMFieldScene::m_bPK)
	{
		if (pOver->m_usGuild)
		{
			if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
			{
				if (pOver->m_dwID > 0 && pOver->m_dwID < 1000 && pOver->IsInPKZone() == 1)
				{
					if (!g_bCastleWar)
						return 0;
					if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
						return 0;
				}
			}
		}
		if (pOver->m_cSummons == 1)
		{
			if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild && pOver->m_usGuild)
			{
				if (!g_bCastleWar)
					return 0;
				if (m_pMyHuman->m_cMantua > 0
					&& m_pMyHuman->m_cMantua == pOver->m_cMantua
					&& !pOver->IsInCastleZone())
				{
					return 0;
				}
			}
			if (!pOver->m_usGuild)
			{
				if (!g_bCastleWar)
					return 0;
				if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
					return 0;
			}
		}
		if (g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
		{
			if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cMantua > 0 && pOver->m_cMantua != 4)
				&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
			{
				return 0;
			}
		}
	}

	if (!g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons)
			return 0;
	}

	if (pOver
		&& pOver->m_usGuild
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (m_pMyHuman->m_usGuild == pOver->m_usGuild
			|| g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
	{
		return 0;
	}

	if (pOver->m_dwID <= 0 && pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons && pOver->IsMerchant())
		return 0;
	if (pOver->m_TradeDesc[0])
		return 0;
	if ((pOver->m_stScore.Merchant & 0xF) == 15 && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		if (!m_pMyHuman->m_cMantua)
			return 0;
		if (pOver->m_cMantua > 0
			&& m_pMyHuman->m_cMantua > 0
			&& pOver->m_cMantua == m_pMyHuman->m_cMantua)
		{
			return 0;
		}
	}

	int nTX = (int)pOver->m_vecPosition.x;
	int nTY = (int)pOver->m_vecPosition.y;
	int nSX = (int)m_pMyHuman->m_vecPosition.x;
	int nSY = (int)m_pMyHuman->m_vecPosition.y;
	if (m_stMoveStop.NextX)
	{
		nSX = m_stMoveStop.NextX;
		nSY = m_stMoveStop.NextY;
	}

	int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
	int nMobAttackRange = nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range;
	int nDX = nTX;
	int nDY = nTY;
	BASE_GetHitPosition(nSX, nSY, &nDX, &nDY, (char*)m_HeightMapData, 8);
	if (nMobAttackRange == -1)
		nMobAttackRange = 1;

	if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
	{
		nDistance -= 12;
		if (nDistance < 0)
			nDistance = 0;
		nDX = (signed int)pOver->m_vecPosition.x;
		nDY = (signed int)pOver->m_vecPosition.y;
	}
	if (BASE_GetItemAbility(&pMobData->Equip[6], 21) < 100)
	{
		if (nSpecForce + BASE_GetMobAbility(pMobData, 27) > nMobAttackRange)
			nMobAttackRange = nSpecForce + BASE_GetMobAbility(pMobData, 27);
	}

	if (nDistance > nMobAttackRange || nDX != nTX || nDY != nTY)
	{
		GetItemFromGround(dwServerTime);
		return 0;
	}

	if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
	{
		if (nDistance > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
			return 0;
	}

	if (!TMFieldScene::m_bPK && !m_pMyHuman->m_cMantua)
	{
		if (pOver->m_dwID > 0 && pOver->m_dwID < 1000
			&& pOver->IsInPKZone() == 1
			&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
			&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			return 0;
		}
	}

	if (!TMFieldScene::m_bPK
		&& g_bCastleWar > 0
		&& m_pMyHuman->m_cMantua > 0
		&& pOver->m_cMantua > 0
		&& m_pMyHuman->m_cMantua == pOver->m_cMantua
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		return 0;
	}

	if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pOver->m_usGuild &&
		pOver->m_dwID > 0 && pOver->m_dwID < 1000 && g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (!g_bCastleWar)
			return 0;
		if (m_pMyHuman->m_cMantua == pOver->m_cMantua)
			return 0;
	}

	if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pOver->IsInCastleZone())
	{
		int isInPos = ((int)m_pMyHuman->m_vecPosition.x >> 7 == 8 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 9) &&
			((int)m_pMyHuman->m_vecPosition.y >> 7 == 15 || (int)m_pMyHuman->m_vecPosition.y >> 7 == 16);
		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cSummons == 1) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && !isInPos)
		{
			return 0;
		}
	}

	auto stAttack = skill_attack::Direct<MSG_Attack>(m_pMyHuman->m_dwID,
		(unsigned char)cSkillIndex, (int)m_pMyHuman->m_vecPosition.x,
		(int)m_pMyHuman->m_vecPosition.y, m_stMoveStop.NextX, m_stMoveStop.NextY,
		pOver->m_dwID);

	pTarget = (TMHuman*)g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
	if (!pTarget)
		return 0;

	int nTargetIndex = 0;
	if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
	{
		stAttack.Dam[nTargetIndex].TargetID = pTarget->m_dwID;
		stAttack.Dam[nTargetIndex].Damage = -2;
		stAttack.Progress = TMFieldScene::m_usProgress;
		++nTargetIndex;
	}
	stAttack.Dam[nTargetIndex].TargetID = pTarget->m_dwID;
	stAttack.Dam[nTargetIndex++].Damage = -1;
	if (cSkillIndex == 16 || cSkillIndex == 12 || cSkillIndex == 28)
	{
		nDX = (int)pTarget->m_vecPosition.x - (int)m_pMyHuman->m_vecPosition.x;
		nDY = (int)pTarget->m_vecPosition.y - (int)m_pMyHuman->m_vecPosition.y;
		if (nDX > 0)
			nDX = 1;
		else if (nDX < 0)
			nDX = -1;
		if (nDY > 0)
			nDY = 1;
		else if (nDY < 0)
			nDY = -1;

		int TX = nDX + (int)pTarget->m_vecPosition.x;
		int TY = nDY + (int)pTarget->m_vecPosition.y;
		TMHuman* pNode = (TMHuman*)m_pHumanContainer->m_pDown;

		while (pNode->m_pNextLink)
		{
			if (pNode == m_pMyHuman || pNode == pTarget
				|| (int)pNode->m_vecPosition.x != TX
				|| (int)pNode->m_vecPosition.y != TY)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
			{
				if (!pNode->IsInPKZone() || !TMFieldScene::m_bPK || pNode->m_bParty == 1)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !g_bCastleWar)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK
					&& m_pMyHuman->m_cMantua > 0
					&& m_pMyHuman->m_cMantua == pNode->m_cMantua
					&& g_bCastleWar > 0)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK
					&& g_bCastleWar > 0
					&& m_pMyHuman->m_cMantua == 3
					&& (pNode->m_dwID > 0 && pNode->m_dwID < 1000 || (pNode->m_cMantua > 0 && pNode->m_cMantua != 4)))
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
				if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pNode->IsInCastleZone())
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
			}
			else if (!TMFieldScene::m_bPK
				&& pNode->m_cSummons == 1
				&& (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild || !pNode->m_usGuild))
			{
				if (!pNode->IsInCastleZone() && g_bCastleWar > 0 || !g_bCastleWar)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
			}
			else if (pOver && !TMFieldScene::m_bPK && pNode->m_cSummons == 1 && pNode->m_usGuild)
			{
				if (!pNode->IsInCastleZone() && g_bCastleWar > 0 || !g_bCastleWar)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}
			}

			stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
			stAttack.Dam[nTargetIndex++].Damage = -1;
			break;
		}
	}

	stAttack.TargetX = (int)pOver->m_vecPosition.x;
	stAttack.TargetY = (int)pOver->m_vecPosition.y;
	if (cSkillIndex)
	{
		stAttack.PosX = m_stMoveStop.NextX;
		stAttack.PosY = m_stMoveStop.NextY;
		stAttack.TargetX = m_stMoveStop.NextX;
		stAttack.TargetY = m_stMoveStop.NextY;
	}

	const int Size = skill_attack::SelectEnvelope(stAttack,
		g_pSpell[(unsigned char)cSkillIndex].MaxTarget);
	SendOneMessage((char*)&stAttack, Size);
	IncSkillSel();

	MSG_Attack stLocalAttack{};
	memcpy(&stLocalAttack, &stAttack, sizeof(stLocalAttack));

	stLocalAttack.Header.ID = m_dwID;
	stLocalAttack.FlagLocal = 1;

	if (nSpecForce)
		stLocalAttack.DoubleCritical |= 8u;

	OnPacketEvent(stAttack.Header.Type, (char*)&stLocalAttack);
	m_dwOldAttackTime = dwServerTime;
	m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
	m_pMyHuman->m_pMoveSkillTargetHuman = 0;

	return 1;
}

void TMFieldScene::SetVisibleSkillMaster()
{
	SGridControl::m_sLastMouseOverIndex = -1;

	// FieldScene2.bin 7.48 materializes the Skill Apprentice roots (1889/1905),
	// but newer auxiliary store controls may legitimately be absent.  Keep the
	// native paired lifecycle and never dereference a control the active layout
	// did not create.
	if (!m_pSkillMPanel || !m_pSkillPanel)
		return;

	const int bVisible = m_pSkillMPanel->IsVisible() == 0;
	const float viewportCenterX = g_pDevice
		? static_cast<float>(g_pDevice->m_dwScreenWidth) * 0.5f
		: 0.0f;
	const float viewportCenterY = g_pDevice
		? static_cast<float>(g_pDevice->m_dwScreenHeight) * 0.5f
		: 0.0f;

	if (bVisible == 1)
	{
		if (m_pSystemPanel)
			m_pSystemPanel->SetVisible(0);

		if (m_pCargoPanel)
			m_pCargoPanel->SetVisible(0);

		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);

		if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);

		if (m_pInvenPanel)
			m_pInvenPanel->SetVisible(0);

		if (m_pCPanel)
			m_pCPanel->SetVisible(0);

		if (m_pShopPanel)
			m_pShopPanel->SetVisible(0);

		if (g_pDevice)
		{
			// Native FUN_0044c15c centers root 1889 and moves root 1905 to its
			// right, leaving a 10-pixel logical gap between the two panels.
			m_pSkillMPanel->SetPos(
				viewportCenterX - m_pSkillMPanel->m_nWidth * 0.5f,
				viewportCenterY - m_pSkillMPanel->m_nHeight * 0.5f);
			m_pSkillPanel->SetPos(
				viewportCenterX + m_pSkillPanel->m_nWidth * 0.5f + 10.0f,
				viewportCenterY - m_pSkillPanel->m_nHeight * 0.5f);
		}

		m_pSkillMPanel->SetVisible(1);
		m_pSkillPanel->SetVisible(1);

		if (m_pHellgateStore)
			m_pHellgateStore->SetVisible(0);

		if (m_pGambleStore)
			SetVisibleGamble(0, 0);

	}
	else
	{
		m_pSkillMPanel->SetVisible(0);
		m_pSkillPanel->SetVisible(0);

		if (g_pDevice)
		{
			// Closing the paired NPC view restores root 1905 to the normal Skill
			// position used by its independent keyboard/button toggle.
			m_pSkillPanel->SetPos(
				viewportCenterX - m_pSkillPanel->m_nWidth * 0.5f,
				viewportCenterY - m_pSkillPanel->m_nHeight * 0.5f);
		}
	}

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleSkill()
{
	SGridControl::m_sLastMouseOverIndex = -1;
	// FUN_00435b13 binds root 1905 (the character Skill window) separately from
	// root 1889 (the NPC Skill Apprentice window) and hides 1889 at startup. The
	// player hotkey/button must therefore toggle only 1905; coupling both roots
	// resurrects the trainer shop whenever the ordinary Skill UI is opened.
	if (m_bCompatFieldScene)
	{
		const int visible = m_pSkillPanel && m_pSkillPanel->IsVisible() == 0;
		if (m_pSkillPanel)
			m_pSkillPanel->SetVisible(visible);
		// Rebind the counter when the panel is opened.  Login and UpdateEtc can
		// arrive before the lazy skill panel controls are resolved, leaving the
		// authoritative value correct but the text widget blank until the next
		// full field refresh.
		if (visible && m_pSkBonus && g_pObjectManager)
		{
			char skillPoints[32]{};
			sprintf(skillPoints, "%u", g_pObjectManager->m_stMobData.CurrentScore.SkillPts);
			m_pSkBonus->SetText(skillPoints, 0);
		}
		if (!visible && m_pDescPanel)
			m_pDescPanel->SetVisible(0);
		GetSoundAndPlay(51, 0, 0);
		return;
	}

	int bVisible = m_pSkillPanel->m_bVisible == 0;

	m_pSkillPanel->SetVisible(bVisible);

	if (bVisible == 1)
	{
		m_pSkillPanel->SetPos(RenderDevice::m_fWidthRatio * 380.0f, RenderDevice::m_fHeightRatio * 35.0f);
	}
	else
	{
		m_pShopPanel->SetVisible(0);
		m_pCargoPanel->SetVisible(0);
		m_pHellgateStore->SetVisible(0);
		if (m_pGambleStore)
			SetVisibleGamble(0, 0);
		m_pSkillMPanel->SetVisible(0);
		m_pDescPanel->SetVisible(0);
		g_pCursor->DetachItem();

	}

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}

	if (m_pCargoPanel->IsVisible())
		m_pCargoPanel->SetVisible(0);
}

void TMFieldScene::UpdateSkillBelt()
{
	// Native 7.48 reaches this routine only after FUN_00441823 has constructed
	// both ten-slot pages.  Fail closed in the source port if scene setup ever
	// regresses; accepting opcode 0x378 with either pointer null previously
	// crashed inside SGridControl::PickupItem before diagnostics could flush.
	if (!m_pGridSkillBelt2 || !m_pGridSkillBelt3)
	{
		WYD748_DiagnosticsLog("UpdateSkillBelt skipped: belt2=%p belt3=%p\r\n",
			m_pGridSkillBelt2, m_pGridSkillBelt3);
		return;
	}

	auto pSkillBelt2 = m_pGridSkillBelt2;
	auto releaseReturnedSkill = [](SGridControlItem*& item)
	{
		if (g_pCursor && g_pCursor->m_pAttachedItem == item)
			g_pCursor->m_pAttachedItem = nullptr;
		SAFE_DELETE(item);
	};
	for (int i = 0; i < 10; ++i)
	{
		auto pReturnItem2 = pSkillBelt2->PickupItem(i, 0);
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 248)
		{
			auto pStructItem2 = new STRUCT_ITEM;
			if (!pStructItem2)
			{
				releaseReturnedSkill(pReturnItem2);
				continue;
			}
			memset(pStructItem2, 0, sizeof(STRUCT_ITEM));

			pStructItem2->sIndex = (unsigned char)g_pObjectManager->m_cShortSkill[i] < 105 ? (unsigned char)g_pObjectManager->m_cShortSkill[i] + 5000 :
				(unsigned char)g_pObjectManager->m_cShortSkill[i] + 5295;

			auto pSkillItem = new SGridControlItem(0, pStructItem2, 0.0f, 0.0f);

			if (!pSkillItem)
			{
				delete pStructItem2;
				releaseReturnedSkill(pReturnItem2);
				continue;
			}
			if (pSkillBelt2->AddSkillItem(pSkillItem, i, 0) != 1)
			{
				SAFE_DELETE(pSkillItem);
				releaseReturnedSkill(pReturnItem2);
				continue;
			}

			if (g_pObjectManager->m_cSelectShortSkill == i)
				pSkillItem->m_GCObj.nTextureSetIndex = 200;
		}
		releaseReturnedSkill(pReturnItem2);
	}

	auto pSkillBelt3 = m_pGridSkillBelt3;
	for (int i = 0; i < 10; ++i)
	{
		auto pReturnItem3 = pSkillBelt3->PickupItem(i, 0);
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i + 10] < 248)
		{
			auto pStructItem3 = new STRUCT_ITEM;
			if (!pStructItem3)
			{
				releaseReturnedSkill(pReturnItem3);
				continue;
			}
			memset(pStructItem3, 0, sizeof(STRUCT_ITEM));

			pStructItem3->sIndex = (unsigned char)g_pObjectManager->m_cShortSkill[i + 10] < 105 ? (unsigned char)g_pObjectManager->m_cShortSkill[i + 10] + 5000 :
				(unsigned char)g_pObjectManager->m_cShortSkill[i + 10] + 5295;

			auto pSkillItem = new SGridControlItem(0, pStructItem3, 0.0f, 0.0f);

			if (!pSkillItem)
			{
				delete pStructItem3;
				releaseReturnedSkill(pReturnItem3);
				continue;
			}
			if (pSkillBelt3->AddSkillItem(pSkillItem, i, 0) != 1)
			{
				SAFE_DELETE(pSkillItem);
				releaseReturnedSkill(pReturnItem3);
				continue;
			}

			if (g_pObjectManager->m_cSelectShortSkill == i + 10)
				pSkillItem->m_GCObj.nTextureSetIndex = 200;
		}
		releaseReturnedSkill(pReturnItem3);
	}
}

void TMFieldScene::IncSkillSel()
{
	int nMax = 10;
	if (m_pGridSkillBelt3->m_bVisible)
		nMax = 20;

	int nBase = nMax - m_nAutoSkillNum;

	if (m_cAutoAttack == 1)
	{
		int nBack = g_pObjectManager->m_cSelectShortSkill;
		if (g_pObjectManager->m_cSelectShortSkill < nBase)
			g_pObjectManager->m_cSelectShortSkill = nBase;
		else if (++g_pObjectManager->m_cSelectShortSkill >= nMax)
			g_pObjectManager->m_cSelectShortSkill = nBase;

		auto pBeltGrid = m_pGridSkillBelt2;

		if (nMax == 20)
			pBeltGrid = m_pGridSkillBelt3;

		auto pItem = pBeltGrid->GetItem(g_pObjectManager->m_cSelectShortSkill - 10 * m_pGridSkillBelt3->m_bVisible, 0);

		if (!pItem)
		{
			int nStart = g_pObjectManager->m_cSelectShortSkill + 1;
			if (nStart >= nMax)
				nStart = nBase;
			for (int i = nStart; i < nMax; ++i)
			{
				pItem = pBeltGrid->GetItem(i - 10 * m_pGridSkillBelt3->m_bVisible, 0);
				if (pItem)
				{
					g_pObjectManager->m_cSelectShortSkill = i;
					break;
				}
			}
		}
		if (!pItem)
		{
			g_pObjectManager->m_cSelectShortSkill = nBack;
			if (!pBeltGrid->GetItem(g_pObjectManager->m_cSelectShortSkill - 10 * m_pGridSkillBelt3->m_bVisible, 0))
				return;
		}

		for (int j = 0; j < 10; ++j)
		{
			auto ipCtrlItem = m_pGridSkillBelt2->GetItem(j, 0);
			if (j == g_pObjectManager->m_cSelectShortSkill && ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 200;
			else if (ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 199;
		}

		for (int j = 0; j < 10; ++j)
		{
			auto ipCtrlItem = m_pGridSkillBelt3->GetItem(j, 0);
			if (j + 10 == g_pObjectManager->m_cSelectShortSkill && ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 200;
			else if (ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 199;
		}
	}
}

void TMFieldScene::SetShortSkill(int nIndex, SGridControlItem* pGridItem)
{
	if (!pGridItem || !pGridItem->m_pItem || nIndex < 0 || nIndex >= 20 ||
		pGridItem->m_pItem->sIndex <= 0 || pGridItem->m_pItem->sIndex >= MAX_ITEMLIST ||
		IsPassiveSkill(pGridItem->m_pItem->sIndex))
		return;

	// Allocation or insertion failure must not erase the previous shortcut.
	// Since rejection does not transfer ownership, rollback restores the old
	// visual before returning control to the caller.
	auto restoreOld = [](SGridControl* belt, SGridControlItem*& oldItem, int cell)
	{
		if (oldItem && (!belt || belt->AddItem(oldItem, cell, 0) != 1))
			SAFE_DELETE(oldItem);
	};

	if (nIndex < 10)
	{
		auto pBelt2 = m_pGridSkillBelt2;
		if (!pBelt2)
			return;
		auto pReturnItem2 = pBelt2->PickupItem(nIndex, 0);

		auto pNewItem2 = new STRUCT_ITEM;
		if (!pNewItem2)
		{
			restoreOld(pBelt2, pReturnItem2, nIndex);
			return;
		}
		memcpy(pNewItem2, pGridItem->m_pItem, sizeof(STRUCT_ITEM));

		auto pNewGridItem = new SGridControlItem(nullptr, pNewItem2, 0.0f, 0.0f);
		if (!pNewGridItem)
		{
			delete pNewItem2;
			restoreOld(pBelt2, pReturnItem2, nIndex);
			return;
		}
		if (pBelt2->AddItem(pNewGridItem, nIndex, 0) != 1)
		{
			SAFE_DELETE(pNewGridItem);
			restoreOld(pBelt2, pReturnItem2, nIndex);
			return;
		}

		if (g_pObjectManager->m_cSelectShortSkill == nIndex)
			pNewGridItem->m_GCObj.nTextureSetIndex = 200;

		g_pCursor->DetachItem();

		SAFE_DELETE(pReturnItem2);
	}
	else
	{
		auto pBelt3 = m_pGridSkillBelt3;
		if (!pBelt3)
			return;
		auto pReturnItem3 = pBelt3->PickupItem(nIndex - 10, 0);
		auto pNewItem3 = new STRUCT_ITEM;
		if (!pNewItem3)
		{
			restoreOld(pBelt3, pReturnItem3, nIndex - 10);
			return;
		}
		memcpy(pNewItem3, pGridItem->m_pItem, sizeof(STRUCT_ITEM));

		auto pNewGridItem = new SGridControlItem(nullptr, pNewItem3, 0.0f, 0.0f);
		if (!pNewGridItem)
		{
			delete pNewItem3;
			restoreOld(pBelt3, pReturnItem3, nIndex - 10);
			return;
		}
		if (pBelt3->AddItem(pNewGridItem, nIndex - 10, 0) != 1)
		{
			SAFE_DELETE(pNewGridItem);
			restoreOld(pBelt3, pReturnItem3, nIndex - 10);
			return;
		}

		if (g_pObjectManager->m_cSelectShortSkill == nIndex)
			pNewGridItem->m_GCObj.nTextureSetIndex = 200;

		g_pCursor->DetachItem();

		SAFE_DELETE(pReturnItem3);
	}

	g_pObjectManager->m_cShortSkill[nIndex] = static_cast<char>(g_pItemList[pGridItem->m_pItem->sIndex].nIndexTexture);
	MSG_SetShortSkill stSetShortSkill{};
	stSetShortSkill.Header.ID = m_pMyHuman->m_dwID;
	stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;
	memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));
	for (int i = 0; i < 20; ++i)
	{
		if (stSetShortSkill.Skill[i] >= 0 && stSetShortSkill.Skill[i] < 96)
		{
			stSetShortSkill.Skill[i] -= 24 * g_pObjectManager->m_stMobData.Class;
		}
		else if (stSetShortSkill.Skill[i] >= 105 && stSetShortSkill.Skill[i] < 153)
		{
			stSetShortSkill.Skill[i] -= 12 * g_pObjectManager->m_stMobData.Class;
		}
	}

	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSetShortSkill)->Type, reinterpret_cast<char*>(&stSetShortSkill), sizeof(stSetShortSkill)});

	GetSoundAndPlay(31, 0, 0);
}

void TMFieldScene::SetSkillColor(TMHuman* pAttacker, char cSkillIndex)
{
	if (!pAttacker)
		return;

	unsigned int dwIndex = 221;
	if (pAttacker->m_dwID > 0 && pAttacker->m_dwID < 1000)
	{
		const static unsigned int dwTextureIndex[MAX_SPELL_LIST] =
		{
			1, 5, 3, 5, 1, 5, 5, 5, 4, 0, 5, 0, 0, 5,
			5, 5, 2, 2, 4, 1, 3, 2, 4, 5, 1, 5, 3, 1,
			5, 5, 5, 5, 3, 1, 1, 3, 1, 0, 0, 5, 2, 0,
			5, 1, 1, 5, 1, 5, 0, 0, 1, 1, 0, 1, 5, 5,
			1, 0, 3, 0, 0, 0, 5, 5, 2, 5, 2, 5, 0, 5,
			5, 5, 0, 2, 1, 0, 2, 1, 5, 5, 5, 2, 3, 2,
			3, 5, 5, 5, 0, 5, 1, 2, 5, 5, 5, 5, 1, 5,
			5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		if (cSkillIndex >= 0 && cSkillIndex < 248)
			dwIndex = dwTextureIndex[cSkillIndex] + 216;
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = dwIndex;
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_bHide = cSkillIndex == 75;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
			{
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = dwIndex;
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_bHide = cSkillIndex == 75;
			}
		}
		return;
	}

	switch (DS_SOUND_MANAGER::m_nMusicIndex)
	{
	case 2:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 222;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 222;
		}
		break;
	case 3:
	case 6:
	case 8:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 221;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 221;
		}
		break;
	case 4:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 223;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 223;
		}
		break;
	case 5:
	case 7:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 224;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
			{
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 224;
				if (pAttacker->m_sHeadIndex == 287)
				{
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[0])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 226;
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[1])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 226;
				}
				if (pAttacker->m_sHeadIndex == 283)
				{
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[0])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 227;
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[1])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 227;
				}
				if (pAttacker->m_sHeadIndex == 175)
				{
					if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
					{
						pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 228;
						if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
							pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 228;
					}
				}
			}
		}
		break;
	case 9:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 225;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
			{
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 225;
				if (pAttacker->m_sHeadIndex == 175)
				{
					if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
					{
						pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 228;
						if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
							pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 228;
					}
				}
			}
		}
		break;
	}
}

void TMFieldScene::SetMyHumanMagic()
{
	;
}

int TMFieldScene::OnPacketSetShortSkill(MSG_SetShortSkill* pStd)
{
	memcpy(g_pObjectManager->m_cShortSkill, pStd->Skill, sizeof(pStd->Skill));
	for (int i = 0; i < 20; ++i)
	{
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 105 || (unsigned char)g_pObjectManager->m_cShortSkill[i] >= 117)
		{
			if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 24)
				g_pObjectManager->m_cShortSkill[i] += 24 * g_pObjectManager->m_stMobData.Class;
		}
		else
		{
			g_pObjectManager->m_cShortSkill[i] += 12 * g_pObjectManager->m_stMobData.Class;
		}
	}

	UpdateSkillBelt();
	return 1;
}

int TMFieldScene::MouseClick_SkillMasterNPC(unsigned int dwServerTime, TMHuman* pOver)
{
	if (g_pObjectManager->m_stMobData.Equip[10].sIndex == 1742 && !m_pMessageBox->IsVisible())
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[152], 1742, 0);
		m_pMessageBox->m_dwArg = pOver->m_dwID;
		m_pMessageBox->SetVisible(1);

		MSG_UseNPC stQuest{};

		stQuest.Header.Type = MSG_UseNPC_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.TargetID = pOver->m_dwID;
		stQuest.ClickOk = 0;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stQuest)->Type, reinterpret_cast<char*>(&stQuest), sizeof(stQuest)});

		m_dwNPCClickTime = dwServerTime;
		return 1;
	}

	if (!m_pShopPanel->IsVisible())
	{
		MSG_REQShopList stReqShopList{};

		stReqShopList.Header.Type = MSG_REQShopList_Opcode;
		stReqShopList.Header.ID = m_pMyHuman->m_dwID;
		stReqShopList.TargetID = pOver->m_dwID;

		m_pGridSkillMaster->m_dwMerchantID = pOver->m_dwID;
		// MSG_ApplyBonus (0x277) confirms the selected 7.48 skill master through
		// TargetID. Keep the same target captured by the shop-list request.
		m_sShopTarget = pOver->m_dwID;

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stReqShopList)->Type, reinterpret_cast<char*>(&stReqShopList), sizeof(stReqShopList)});
		m_dwNPCClickTime = dwServerTime;
	}
	return 1;
}
