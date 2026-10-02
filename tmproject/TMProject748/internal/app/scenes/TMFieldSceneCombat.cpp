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
#include "TMHuman.h"
#include "TMObjectContainer.h"
#include "TMSkinMesh.h"
#include "TMSkillThunderBolt.h"
#include "TMSkillPoison.h"
#include "TMFont3.h"

int TMFieldScene::MobAttack(unsigned int wParam, D3DXVECTOR3 vec, unsigned int dwServerTime)
{
	auto pOver = m_pMouseOverHuman;
	int nSpecForce = 0;
	auto pMobData = &g_pObjectManager->m_stMobData;

	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		nSpecForce = 1;

	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 0;

	if (pOver)
	{
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
	if (pOver && pOver->m_bMouseOver == 1)
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

			unsigned int dwAddTime = 0;
			if (dwServerTime <= m_dwOldAttackTime + 1000)
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

				if (m_cAutoAttack == 1)
					m_pTargetHuman = pOver;
				stAttack.Dam[0].TargetID = pOver->m_dwID;
				auto pTarget = g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
				int nCritical = (unsigned char)pMobData->CurrentScore.Critical;
				stAttack.Dam[0].Damage = -2;
				stAttack.Progress = TMFieldScene::m_usProgress;

				BASE_GetDoubleCritical(pMobData, 0, &TMFieldScene::m_usProgress, &stAttack.DoubleCritical);
				stAttack.TargetX = (signed int)pOver->m_vecPosition.x;
				stAttack.TargetY = (signed int)pOver->m_vecPosition.y;
				if (pTarget->m_nClass == 66 && pTarget->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
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

					auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;

					while (pNode->m_pNextLink != nullptr)
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
							if (!TMFieldScene::m_bPK && pNode->m_cSummons == 1 && (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild)
								|| !pNode->m_usGuild)
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

				SendOneMessage((char*)&stAttack, nSize);

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
				if ((int)m_pMyHuman->m_eMotion >= 4 && (int)m_pMyHuman->m_eMotion <= 9)
				{
					unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
					if (dwMod > 2)
						dwMod -= 2;

					if (g_pEventTranslator->button[0] && dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
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

void TMFieldScene::FrameMove_KhepraDieEffect(unsigned int dwServerTime)
{
	if (m_nKhepraDieFlag < 1 || m_dwKhepraDieTime + 300 >= dwServerTime)
		return;

	int RndX = rand() % 9;
	int RndZ = rand() % 9;
	float X = (float)RndX + 2362.0f;
	float Y = -6.8000002f;
	float Z = (float)RndZ + 3927.0f;

	auto pThunder1 = new TMSkillThunderBolt(TMVector3(X, Y, Z), 3);
	m_pEffectContainer->AddChild(pThunder1);
	auto pDust1 = new TMEffectDust(TMVector3(X, Y, Z), 50.0f, 0);
	m_pEffectContainer->AddChild(pDust1);
	auto pDust2 = new TMEffectDust(TMVector3(X - 0.3f, Y, Z - 0.3f), 50.0f, 0);
	m_pEffectContainer->AddChild(pDust2);
	auto pDust3 = new TMEffectDust(TMVector3(X + 0.3f, Y, Z + 0.3f), 50.0f, 0);
	m_pEffectContainer->AddChild(pDust3);

	m_dwKhepraDieTime += 100 * (rand() % 10 + 1) + 300;
	m_nKhepraDieFlag++;

	if (m_nKhepraDieFlag >= 10)
	{
		auto pPoison = new TMSkillPoison(TMVector3(2365.0f, -9.8000002f, 3930.0f), 0xFFCC6666, 25, 1, 0);
		m_pEffectContainer->AddChild(pPoison);

		m_nKhepraDieFlag = 0;
		m_dwKhepraDieTime = 0;
	}
}

void TMFieldScene::SetMyHumanExp(long long unExp, int nFakeExp)
{
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
		sprintf(szStr, "Exp +%d", static_cast<unsigned int>(nExp));

		m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFF8866, 0.5f, 0, 1, 1200, 0, 1));

		sprintf(szStr, g_pMessageStringTable[148], nExp);
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

		m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFF8866, 1.0f, 0, 1, 1500, 0, 1));

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
	auto pMobData = &g_pObjectManager->m_stMobData;
	int w1 = BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[6], 2);
	int w2 = BASE_GetItemAbility(&pMobData->Equip[7], 2);
	int idx1 = pMobData->Equip[6].sIndex;
	int idx2 = pMobData->Equip[7].sIndex;
	int t1 = idx1 >= 0 && idx1 < MAX_ITEMLIST ? g_pItemList[idx1].nUnique : 0;
	int t2 = idx2 >= 0 && idx2 < MAX_ITEMLIST ? g_pItemList[idx2].nUnique : 0;

	int nWeaponDamage = 0;
	if (t1 == 47 && t2 == 45)
		w2 = 0;
	if (w1 <= w2)
		nWeaponDamage = w2 + w1 / 3;
	else
		nWeaponDamage = w1 + w2 / 3;

	if (idx1 >= 0 && idx1 < MAX_ITEMLIST)
	{
		int nPos1 = g_pItemList[idx1].nPos;

		if ((nPos1 == 64 || nPos1 == 192) && t1 != 44 && t2 != 47 && BASE_GetItemSanc(&pMobData->Equip[6]) >= 9)
		{
			int nu = g_pItemList[idx1].nUnique;
			if (nu != 47 && nu != 44)
				nWeaponDamage += 40;
		}
	}

	if (idx2 >= 0 && idx2 < MAX_ITEMLIST)
	{
		int nPos2 = g_pItemList[idx2].nPos;
		if ((nPos2 == 64 || nPos2 == 192) && t1 != 44 && t2 != 47 && BASE_GetItemSanc(&pMobData->Equip[7]) >= 9)
		{
			int nux = g_pItemList[idx2].nUnique;
			if (nux != 47 && nux != 44)
				nWeaponDamage += 40;
		}
	}

	return nWeaponDamage;
}

void TMFieldScene::SetPosPKRun()
{
	;
}
