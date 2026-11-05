#include "pch.h"
#include "TMFieldScene.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMUtil.h"
#include "TMItem.h"
#include "ItemEffect.h"
#include "TMEffectMesh.h"
#include "TMEffectBillBoard.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectSkinMesh.h"
#include "TMEffectLevelUp.h"
#include "TMEffectParticle.h"
#include "TMEffectDust.h"
#include "../../application/FieldInteractionPolicy.h"
#include "../../application/CCModePolicy.h"
#include "../../application/SkillDamagePreview.h"
#include "FieldActionPolicy.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"
#include "TMSkinMesh.h"
#include "TMSkillThunderBolt.h"
#include "TMSkillPoison.h"
#include "TMFont3.h"

void TMFieldScene::UpdatePhysicalAttack(unsigned int dwServerTime)
{
	if (!m_pTargetHuman || !m_pMyHuman || !g_pObjectManager)
		return;

	const auto action = field_interaction::PhysicalAttackTick(
		m_pMyHuman->m_cDie == 0 && g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0,
		m_pTargetHuman->m_cDie == 0 && m_pTargetHuman->m_sDelayDel == 0 &&
			m_pTargetHuman->m_stScore.CurHP > 0,
		m_dwLastLogout || m_dwLastTown || m_dwLastTeleport || g_dwStartQuitGameTime ||
			m_bAirMove || m_pMyHuman->m_dwDelayDel,
		cc_mode::OwnsCombat(g_GameAuto),
		m_pMyHuman->m_cHide == 1 || m_pMyHuman->m_bMoveing != 0 || static_cast<int>(m_pMyHuman->m_eMotion) > 1,
		dwServerTime, m_dwLastAutoAttackTime);
	if (action == field_interaction::PhysicalAttackAction::Cancel)
	{
		m_pTargetHuman = nullptr;
		m_pMyHuman->m_pMoveTargetHuman = nullptr;
		return;
	}
	if (action != field_interaction::PhysicalAttackAction::Retry)
		return;

	m_dwLastAutoAttackTime = dwServerTime;
	auto target = m_pTargetHuman;
	D3DXVECTOR3 position(target->m_vecPosition.x, target->m_fHeight, target->m_vecPosition.y);
	// Reuse click validation and its existing packet/cooldown path, without
	// borrowing the mouse hover or selecting another enemy when this one dies.
	if (!MobAttack(0, position, dwServerTime, target))
	{
		m_pTargetHuman = nullptr;
		m_pMyHuman->m_pMoveTargetHuman = nullptr;
	}
}

int TMFieldScene::MobAttack(unsigned int wParam, D3DXVECTOR3 vec, unsigned int dwServerTime, TMHuman* target)
{
	// A03-11: the local actor and object manager exist after field init;
	// a teardown frame must not dereference them.
	if (!m_pMyHuman || !g_pObjectManager)
		return 0;

	auto pOver = target ? target : m_pMouseOverHuman;
	int nSpecForce = 0;
	auto pMobData = &g_pObjectManager->m_stMobData;

	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		nSpecForce = 1;

	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 0;

	if (pOver)
	{
		if (pOver == m_pMyHuman || pOver->m_cDie || pOver->m_sDelayDel || pOver->m_stScore.CurHP <= 0)
			return 0;
		if (pOver->m_nClass == 66 && pOver->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
			return 0;
		if (pOver->m_bParty == 1)
			return 0;
		if (pOver->m_usGuild && (m_pMyHuman->m_usGuild == pOver->m_usGuild || g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
			return 0;
		if (pOver->m_dwID > 0 && pOver->m_dwID < 1000 && (!pOver->IsInPKZone() || !m_pMyHuman->IsInPKZone()))
			return 0;
		if (m_pMyHuman->m_cCantAttk)
			return 0;

		if (!TMFieldScene::m_bPK && pOver->m_cSummons == 1)
		{
			bool SameMantua = false;
			if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
				SameMantua = true;

			if (pOver->m_usGuild && g_pObjectManager->m_usWarGuild != pOver->m_usGuild && !g_bCastleWar)
			{
				if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild)
					return 0;

				if (m_pMyHuman->m_cMantua == pOver->m_cMantua && !pOver->IsInCastleZone())
					return 0;
			}

			if (!pOver->m_usGuild)
			{
				if (!g_bCastleWar)
					return 1;
				if (SameMantua)
					return 0;
			}
		}
	}
	if (pOver && (target || pOver->m_bMouseOver == 1))
	{
		int IsTargetUser = pOver->m_dwID > 0 && pOver->m_dwID < 1000;

		if (pOver->m_dwID >= 1000)
		{
			if (pOver->IsMerchant())
				return 0;

			if ((pOver->m_stScore.Merchant & 0xF) == 15)
			{
				if (!m_pMyHuman->m_cMantua || m_pMyHuman->m_cMantua == 3)
					return 0;
				if (pOver->m_cMantua > 0 && m_pMyHuman->m_cMantua > 0 && pOver->m_cMantua == m_pMyHuman->m_cMantua)
					return 0;
			}
		}

		if (pOver->m_TradeDesc[0])
			return 0;
		if (m_pMyHuman->IsInTown() == 1 || pOver->IsInTown() == 1)
			return 0;

		if (!IsTargetUser && !pOver->m_bParty && !pOver->m_cSummons || pOver->IsInPKZone())
		{
			if (!m_pMyHuman->m_cMantua)
			{
				if (IsTargetUser && pOver->IsInPKZone() && !TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pOver->m_usGuild)
					return 0;
			}
			if (!TMFieldScene::m_bPK)
			{
				if (IsTargetUser)
				{
					if (g_bCastleWar == 1 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
						return 0;
					if (!g_bCastleWar)
					{
						if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild)
							return 0;
						if (m_pMyHuman->m_cMantua == pOver->m_cMantua)
							return 0;
					}
				}
			}

			if (!TMFieldScene::m_bPK && pOver->m_cSummons == 1 && g_pObjectManager->m_usWarGuild != pOver->m_usGuild && pOver->m_usGuild && !pOver->IsInCastleZone())
				return 0;
			if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
				return 0;

			int isInPos = ((int)m_pMyHuman->m_vecPosition.x >> 7 == 8 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 9) &&
				((int)m_pMyHuman->m_vecPosition.y >> 7 == 15 || (int)m_pMyHuman->m_vecPosition.y >> 7 == 16);
			if (TMFieldScene::m_bPK == 1 && g_bCastleWar > 0 && m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua && !isInPos)
				return 0;
			if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
			{
				if (IsTargetUser || pOver->m_cMantua > 0 && pOver->m_cMantua != 4)
					return 0;
			}
			if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pOver->IsInCastleZone())
			{
				if (IsTargetUser && !isInPos)
					return 0;
			}

			// A valid physical click retains the enemy even during cooldown or
			// before reaching attack range. Manual movement/skills cancel it.
			m_pTargetHuman = pOver;
			unsigned int dwAddTime = 0;
			// A03-04: wrap-safe "now <= last attack + 1000".
			if (field_time::NotAfter(dwServerTime, m_dwOldAttackTime, 1000))
				return 1;

			int nSX = (int)m_pMyHuman->m_vecPosition.x;
			int nSY = (int)m_pMyHuman->m_vecPosition.y;
			int nDistance = BASE_GetDistance(nSX, nSY, nSX, nSY);
			int nTX = (int)pOver->m_vecPosition.x;
			int nTY = (int)pOver->m_vecPosition.y;
			nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
			if (nDistance > 1 && m_pMyHuman->m_cCantAttk == 1)
				return 0;

			int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);
			BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)m_HeightMapData, 8);

			if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
			{
				if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 &&
					(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
					return 0;

				nDistance -= 12;
				if (nDistance < 0)
					nDistance = 0;

				nTX = (int)pOver->m_vecPosition.x;
				nTY = (int)pOver->m_vecPosition.y;
			}

			static int nMotionIndex = 0;
			++nMotionIndex;
			nMotionIndex %= 3;

			if (pOver)
				m_pMyHuman->m_nAttackDestID = pOver->m_dwID;

			if (nDistance <= nMobAttackRange && nTX == (signed int)pOver->m_vecPosition.x && nTY == (signed int)pOver->m_vecPosition.y)
			{
				MSG_Attack stAttack{};
				stAttack.Header.Type = MSG_Attack_One_Opcode;
				stAttack.Header.ID = m_pMyHuman->m_dwID;
				stAttack.AttackerID = m_pMyHuman->m_dwID;
				stAttack.PosX = m_stMoveStop.NextX;
				stAttack.PosY = m_stMoveStop.NextY;
				stAttack.CurrentMp = -1;
				stAttack.SkillIndex = -1;
				stAttack.SkillParm = 0;
				stAttack.Motion = nMotionIndex + 4;

				if (m_pMyHuman->m_nClass == 33)
				{
					if (nDistance < 2)
					{
						stAttack.Motion = 4;
					}
					else
					{
						stAttack.Motion = 5;
						stAttack.SkillIndex = 105;
						stAttack.SkillParm = 2;
					}
				}
				if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 101)
				{
					stAttack.SkillIndex = 151;
					stAttack.SkillParm = 0;
				}
				else if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 102)
				{
					stAttack.SkillIndex = 152;
					if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 871)
						stAttack.SkillParm = 0;
					else if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 872)
						stAttack.SkillParm = 1;
				}
				else if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 103)
				{
					stAttack.SkillIndex = 153;
					switch (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh)
					{
					case 873:
						stAttack.SkillParm = 0;
						break;
					case 874:
						stAttack.SkillParm = 1;
						break;
					case 875:
						stAttack.SkillParm = 2;
						break;
					case 876:
						stAttack.SkillParm = 3;
						break;
					case 877:
						stAttack.SkillParm = 4;
						break;
					case 892:
						stAttack.SkillParm = 5;
						break;
					case 907:
						stAttack.SkillParm = 6;
						break;
					case 908:
						stAttack.SkillParm = 7;
						break;
					case 909:
						stAttack.SkillParm = 8;
						break;
					case 37:
						stAttack.SkillParm = 9;
						break;
					case 767:
						stAttack.SkillParm = 10;
						break;
					case 2814:
						stAttack.SkillParm = 11;
						break;
					}
				}
				else if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 104)
				{
					stAttack.SkillIndex = 104;
					if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 878)
						stAttack.SkillParm = 0;
					else if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 879)
						stAttack.SkillParm = 1;
				}

				stAttack.Dam[0].TargetID = pOver->m_dwID;
				auto pTarget = g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
				int nCritical = (unsigned char)pMobData->CurrentScore.Critical;
				stAttack.Dam[0].Damage = -2;
				stAttack.Progress = TMFieldScene::m_usProgress;

				BASE_GetDoubleCritical(pMobData, 0, &TMFieldScene::m_usProgress, &stAttack.DoubleCritical);
				stAttack.TargetX = (signed int)pOver->m_vecPosition.x;
				stAttack.TargetY = (signed int)pOver->m_vecPosition.y;
				// A03-11: an ID lookup can miss (ID 0); the clicked actor is the target.
				const TMHuman* pShadowCheck = pTarget ? pTarget : pOver;
				if (pShadowCheck->m_nClass == 66 && pShadowCheck->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
					return 0;

				int nSize = sizeof(MSG_AttackOne);
				if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x200000)
				{
					stAttack.Header.Type = MSG_Attack_Two_Opcode;
					nSize = sizeof(MSG_AttackTwo);
				}
				if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x40)
				{
					int nDX = (int)pOver->m_vecPosition.x - (int)m_pMyHuman->m_vecPosition.x;
					int nDY = (int)pOver->m_vecPosition.y - (int)m_pMyHuman->m_vecPosition.y;
					if (nDX > 0)
						nDX = 1;
					else if (nDX < 0)
						nDX = -1;
					if (nDY > 0)
						nDY = 1;
					else if (nDY < 0)
						nDY = -1;

					int TX = nDX + (int)pOver->m_vecPosition.x;
					int TY = nDY + (int)pOver->m_vecPosition.y;

					// The local human is the oldest child and therefore the last node;
					// the loop skips it either way.
					auto pNode = m_pHumanContainer ? (TMHuman*)m_pHumanContainer->m_pDown : nullptr;

					while (pNode && pNode->m_pNextLink != nullptr)
					{
						if (pNode == m_pMyHuman || pNode == pOver || (int)pNode->m_vecPosition.x != TX || (int)pNode->m_vecPosition.y != TY)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}

						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							if (!pNode->IsInPKZone() && pNode->m_bParty == 1)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !g_bCastleWar)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pNode->m_cMantua && g_bCastleWar > 0)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3
								&& ((pNode->m_dwID > 0 && pNode->m_dwID < 1000) || pNode->m_cMantua > 0 && pNode->m_cMantua != 4))
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && pNode->IsInCastleZone())
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}
						else
						{
							// A03-10: skip only summons outside the war guild, as the
							// primary-target filter does; ordinary monsters have no guild.
							if (!TMFieldScene::m_bPK && pNode->m_cSummons == 1 &&
								((g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild) || !pNode->m_usGuild))
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}

						stAttack.Dam[1].TargetID = pNode->m_dwID;
						stAttack.Dam[1].Damage = -2;
						break;
					}

					if (pMobData->LearnedSkill[0] & 0x200000 || stAttack.Dam[1].Damage == -2)
					{
						stAttack.Header.Type = MSG_Attack_Two_Opcode;
						nSize = sizeof(MSG_AttackTwo);
					}
					else
					{
						stAttack.Header.Type = MSG_Attack_One_Opcode;
						nSize = sizeof(MSG_AttackOne);
					}
				}

				if (m_pMyHuman->m_bMoveing && (stAttack.SkillIndex == -1 || stAttack.SkillIndex == 104 || stAttack.SkillIndex > 150))
				{
					m_stMoveStop.Header.ID = m_pMyHuman->m_dwID;
					m_stMoveStop.Header.Type = 0x2CB;
					m_stMoveStop.CurrentX = stAttack.PosX;
					m_stMoveStop.CurrentY = stAttack.PosY;
					SendOneMessage((char*)&m_stMoveStop, sizeof(m_stMoveStop));
				}

				if (!SendOneMessage((char*)&stAttack, nSize))
					return 0;

				MSG_Attack stAttackLocal{};
				memcpy((char*)&stAttackLocal, (char*)&stAttack, nSize);
				stAttackLocal.Header.ID = m_dwID;
				stAttackLocal.FlagLocal = 1;
				if (nSpecForce)
					stAttackLocal.DoubleCritical |= 8;
				stAttackLocal.Progress = stAttack.Progress;
				OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
				m_dwOldAttackTime = dwServerTime;
			}
			else if (nDistance >= nMobAttackRange && !(wParam & 4))
			{
				if (m_pMyHuman->m_cCantMove)
					return 0;
				if ((int)m_pMyHuman->m_eMotion >= 4 && (int)m_pMyHuman->m_eMotion <= 9 && m_pMyHuman->m_pSkinMesh)
				{
					unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
					if (dwMod > 2)
						dwMod -= 2;

					const unsigned int dwSwing = 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS;
					if (g_pEventTranslator->button[0] && dwSwing &&
						field_time::Before(dwServerTime, m_pMyHuman->m_dwStartAnimationTime, dwSwing))
						return 1;
				}

				if (m_pGround)
				{
					if (vec.y < -5000.0f && pOver)
					{
						vec.x = pOver->m_vecPosition.x;
						vec.z = pOver->m_vecPosition.y;
						vec.y = pOver->m_fHeight;
					}

					m_pMyHuman->SetSpeed(m_bMountDead);
					if (vec.x == m_pMyHuman->m_vecPosition.x && vec.z == m_pMyHuman->m_vecPosition.y)
						return 1;

					m_pTargetItem = 0;
					m_pMyHuman->m_pMoveTargetHuman = pOver;
					m_dwLastSetTargetHuman = dwServerTime;
					int nMoveSX = (int)m_pMyHuman->m_vecPosition.x;
					int nMoveSY = (int)m_pMyHuman->m_vecPosition.y;

					if (m_stMoveStop.NextX)
					{
						nMoveSX = m_stMoveStop.NextX;
						nMoveSY = m_stMoveStop.NextY;
					}
					if (pOver)
					{
						int nTX = (int)pOver->m_vecPosition.x;
						int nTY = (int)pOver->m_vecPosition.y;
						int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
						int PlusX = 1;
						int PlusY = 1;
						if (nMoveSX > nTX)
							PlusX = -1;
						if (nMoveSY > nTY)
							PlusY = -1;

						int nBreak = 0;
						while (nMoveDistance2 > nMobAttackRange)
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
						// Native FieldScene2 FUN_0051a939 stores this same in-range
						// destination and immediately invokes route constructor FUN_00520216.
						// Deferring GetRoute to FrameMove leaves a single attack click inert
						// whenever the progress gate is not revisited.
						m_pMyHuman->GetRoute(m_vecMyNext, 0, 0);
					}
				}
			}
		}

		return 1;
	}

	m_pMouseOverHuman = nullptr;
	return GetItemFromGround(dwServerTime) == 1;
}

void TMFieldScene::SetMyHumanExp(long long unExp, int nFakeExp)
{
	// A03-11: a party kill can arrive while the local actor is not built.
	if (!m_pMyHuman || !g_pObjectManager)
		return;
	auto pMobData = &g_pObjectManager->m_stMobData;

	long long nExp = unExp - pMobData->Exp;
	int nFExp = g_pObjectManager->m_nFakeExp - nFakeExp;

	if (nExp == 0 && nFExp == 0)
		return;

	pMobData->Exp = unExp;
	g_pObjectManager->m_nFakeExp = nFakeExp;

	memcpy(&m_pMyHuman->m_stScore, &pMobData->CurrentScore, sizeof(pMobData->CurrentScore));

	UpdateScoreUI(0);

	int nTX = 0;
	int nTY = 0;
	if (nExp >= 0 && BASE_Get3DTo2DPos(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y, &nTX, &nTY))
	{
		char szStr[128]{};
		// EXP is 32-bit; the message-table format takes an int, never the
		// 64-bit difference itself.
		const unsigned int gainedExp = static_cast<unsigned int>(nExp);
		sprintf(szStr, "Exp +%u", gainedExp);

		if (m_pExtraContainer)
			m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fUIScaleY * 80.0f), 0xFFFF8866, 0.5f, 0, 1, 1200, 0, 1));

		sprintf(szStr, g_pMessageStringTable[148], static_cast<int>(gainedExp));
		if (!m_bShowExp)
		{
			if (m_pChatListnotice)
				m_pChatListnotice->AddItem(new SListBoxItem(szStr, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		}
	}
	if (nFExp > 0 && BASE_Get3DTo2DPos(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y, &nTX, &nTY))
	{
		char szStr[128]{};
		sprintf(szStr, "Cp -%d", nFExp);

		if (m_pExtraContainer)
			m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fUIScaleY * 80.0f), 0xFFFF8866, 1.0f, 0, 1, 1500, 0, 1));

		sprintf(szStr, g_pMessageStringTable[305], nFExp);
		if (!m_bShowExp)
		{
			if (m_pChatListnotice)
				m_pChatListnotice->AddItem(new SListBoxItem(szStr, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		}
	}
}

void TMFieldScene::SetPK()
{
	TMFieldScene::m_bPK = TMFieldScene::m_bPK == 0;

	if (m_PkButton)
	{
		m_PkButton->SetSelected(TMFieldScene::m_bPK == 0);
		m_PkButton->Update();
	}

	auto pBtnPK = m_pControlContainer
		? (SButton*)m_pControlContainer->FindControl(306)
		: nullptr;
	MSG_STANDARDPARM stParam{};
	stParam.Header.ID = g_pObjectManager->m_dwCharID;
	stParam.Header.Type = MSG_SetPKMode_Opcode;
	stParam.Parm = TMFieldScene::m_bPK;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stParam)->Type, reinterpret_cast<char*>(&stParam), sizeof(stParam)});

	if (pBtnPK)
		pBtnPK->SetSelected(TMFieldScene::m_bPK);
}

int TMFieldScene::GetWeaponDamage()
{
	if (!g_pObjectManager) return 0;
	auto& mob = g_pObjectManager->m_stMobData;
	int damage[16]{};
	int unique[16]{};
	long long other = 0;
	for (int slot = 0; slot < 16; ++slot)
	{
		if (slot == 14) continue; // The server applies mount bonuses separately.
		auto& item = mob.Equip[slot];
		const int index = item.sIndex;
		if (index <= 0 || index >= MAX_ITEMLIST) continue;
		const auto& definition = g_pItemList[index];
		unique[slot] = definition.nUnique;
		int refinement = index <= 40 ? 0 : BASE_GetItemSanc(&item);
		if (refinement >= 9 && (definition.nPos & 0xF00)) ++refinement;
		// Sum both raw effects as Go itemAbility does. The generic native item
		// helper substitutes EF_DAMAGE2 on gloves and is unsuitable here.
		for (const int effect : {EF_DAMAGE, EF_DAMAGE2})
		{
			int value = 0;
			for (const auto& entry : definition.stEffect)
				if (entry.sEffect == effect) value += entry.sValue;
			for (const auto& entry : item.stEffect)
				if (entry.cEffect == effect) value += entry.cValue;
			damage[slot] = skill_damage_preview::Saturate(static_cast<long long>(damage[slot]) +
				skill_damage_preview::RefinedDamage(value, refinement));
		}
		if (slot != 6 && slot != 7) other += damage[slot];
	}
	return skill_damage_preview::EquipmentDamage(skill_damage_preview::Saturate(other),
		damage[6], damage[7], unique[6], unique[7], mob.Class, mob.LearnedSkill[0]);
}

void TMFieldScene::SetPosPKRun()
{
	;
}
