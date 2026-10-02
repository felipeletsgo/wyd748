#include "pch.h"
#include "TMFieldScene.h"
#include "TMCamera.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMObjectContainer.h"
#include "TMUtil.h"
#include "TMSkinMesh.h"
#include "TMItem.h"
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
#include "TMSkillHolyTouch.h"
#include "../../game/entities/DeathMotionPolicy.h"
#include "../../wire/AttackVisualDamage.h"
#include "../../application/AttackAttackerState.h"
#include "../../application/AttackTargetState.h"
#include "TMHuman.h"
#include "TMEffectLevelUp.h"
#include "TMFont3.h"

namespace
{
static int GetWYD748AttackVisualDamage(const MSG_Attack* pAttack, int index)
{
	if (!pAttack || index < 0 || index >= 13)
		return 0;

	const int legacyDamage = pAttack->Dam[index].Damage;
	if (legacyDamage < 0)
		return legacyDamage;

	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(pAttack);
	return attack_visual::ReadDamage(bytes, pAttack->Header.Size,
		pAttack->Header.Type, index, legacyDamage);
}
}

int TMFieldScene::OnPacketAction(MSG_STANDARD* pStd)
{
	if (g_pObjectManager->GetHumanByID(pStd->ID))
		return 0;

	MSG_REQMobByID stReqMobById{};

	stReqMobById.Header.ID = g_pObjectManager->m_dwCharID;
	stReqMobById.Header.Type = MSG_REQMobByID_Opcode;
	stReqMobById.MobID = pStd->ID;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stReqMobById)->Type, reinterpret_cast<char*>(&stReqMobById), sizeof(stReqMobById)});
	return 1;
}

int TMFieldScene::OnPacketCNFMobKill(MSG_CNFMobKill* pStd)
{
	auto pAttacker = g_pObjectManager->GetHumanByID(pStd->Killer);
	if (pAttacker)
	{
		if (pAttacker->m_bParty == 1 || m_pMyHuman == pAttacker)
		{
			SetMyHumanExp(pStd->Exp, pStd->FakeExp);
			if (m_cAutoAttack == 1 && m_pMyHuman == pAttacker)
				m_pTargetHuman = 0;
		}

		m_pEffectContainer->AddChild(new TMEffectCharge(pAttacker, 0, 0xFFFFFFFF));
	}

	auto pKilled = g_pObjectManager->GetHumanByID(pStd->KilledMob);
	if (pKilled)
	{
		pKilled->m_stScore.CurHP = 0;
		pKilled->Die();
	}

	if (m_pMyHuman == pKilled)
	{
		bool bFind = false;
		// Search resurrection items in the one native 9x7 grid when the 7.48
		// FieldScene2 contract is active.
		const int carryPages = m_bCompatFieldScene ? 1 : 4;
		const int carryRows = m_bCompatFieldScene ? 7 : 3;
		const int carryColumns = m_bCompatFieldScene ? 9 : 5;
		for (int i = 0; i < carryPages; ++i)
		{
			auto pGridInv = m_bCompatFieldScene ? m_pGridInv : m_pGridInvList[i];
			if (!pGridInv)
				continue;
			for (int nY = 0; nY < carryRows; ++nY)
			{
				for (int nX = 0; nX < carryColumns; ++nX)
				{
					auto pItem = pGridInv->GetItem(nX, nY);
					if (pItem && pItem->m_pItem && pItem->m_pItem->sIndex == 3463)
					{
						bFind = true;
						break;
					}
				}
				if (bFind == true)
					break;
			}

			if (bFind == true)
				break;
		}

		if (bFind && m_pHelpList[3])
		{
			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);

			char szTime[128]{};

			const char* killerName = pAttacker ? pAttacker->m_szName : "Unknown";
			sprintf(szTime,	"[%02d:%02d:%02d] Killer[%s] ",	sysTime.wHour, sysTime.wMinute,	sysTime.wSecond, killerName);

			m_pHelpList[3]->AddItem(new SListBoxItem(szTime, 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

			if (m_pHelpMemo)
				m_pHelpMemo->SetVisible(0);
		}
	}

	return 1;
}

int TMFieldScene::OnPacketSetHpMode(MSG_SetHpMode* pStd)
{
	if (pStd->Mode / 10 == 1)
	{
		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTCHAR_STATE);
		return 1;
	}
	else if (pStd->Mode != 22)
	{
		if (!m_pMessagePanel->m_bVisible)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[13], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}

		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
		return 1;
	}
	else if (pStd->Mode == 22)
	{
		if (m_pMyHuman->m_stScore.Level < 1000)
			m_pMyHuman->Die();

		return 1;
	}

	return 1;
}

int TMFieldScene::OnPacketAttack(MSG_STANDARD* pStd)
{
	auto pAttack = reinterpret_cast<MSG_Attack*>(pStd);
	const int targetCount = static_cast<int>(AttackTargetCapacity(pAttack->Header.Type));

	auto pAttacker = (TMHuman*)g_pObjectManager->GetHumanByID(pAttack->AttackerID);
	auto pTarget = (TMHuman*)g_pObjectManager->GetHumanByID(pAttack->Dam[0].TargetID);

	bool bomb = false;
	if (pAttack->SkillIndex == 104)
	{
		pAttack->SkillIndex = 39;
		bomb = true;
	}

	TMVector2 vecAttackerPos{};
	int nClass = 0;
	float fHeight = 1.5f;

	if (pAttacker)
	{
		if (pAttack->DoubleCritical & 8)
		{
			unsigned int dwStartTime = g_pTimerManager->GetServerTime();
			const int force = attack_attacker::SwingForce(pAttacker->m_nSkinMeshType,
				pAttacker->m_nClass, pAttacker->m_nWeaponTypeL);
			attack_attacker::ApplySwing(pAttacker->m_pSkinMesh->m_pSwingEffect[0],
				pAttacker->m_pSkinMesh->m_pSwingEffect[1], force, dwStartTime);
		}
		if (pAttacker->m_nClass == 32 && pAttack->Motion == 4 && pTarget == m_pMyHuman)
			pAttacker->m_dwEarthQuakeTime = g_pTimerManager->GetServerTime();

		for (int i = 0; i < targetCount; ++i)
		{
			if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
				break;
			if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
				break;

			pAttacker->m_usTargetID[i] = pAttack->Dam[i].TargetID;
		}

		SetSkillColor(pAttacker, static_cast<char>(pAttack->SkillIndex));

		vecAttackerPos = pAttacker->m_vecPosition;
		if (pAttacker->m_nSkinMeshType == 20 && pAttacker->m_stLookInfo.HelmMesh == 0)
			nClass = 1;

		attack_attacker::ApplyMana(pAttacker == m_pMyHuman, pAttack->FlagLocal,
			pAttack->SkillIndex, pAttack->CurrentMp, pAttacker->m_stScore.CurMP,
			[&](unsigned short receivedMana) {
				g_pObjectManager->m_stMobData.CurrentScore.CurMP = receivedMana;
				auto pMPBar = (SProgressBar*)m_pControlContainer->FindControl(1170);
				auto pCurrentMPText = (SText*)m_pControlContainer->FindControl(65616);

				if (pMPBar)
					pMPBar->SetCurrentProgress(pAttacker->m_stScore.CurMP);
				if (pCurrentMPText)
				{
					char szText[128]{};
					sprintf(szText, "%d", pAttacker->m_stScore.CurMP);
					pCurrentMPText->SetText(szText, 0);
				}
			});
		// Legacy skill labels may not match the catalog.
		if (attack_attacker::ShouldDispatchVisuals(pAttacker == m_pMyHuman,
			pAttack->FlagLocal, (unsigned char)pAttack->Motion))
		{
			if (pAttack->SkillIndex == 4) // Possessed
			{
				pAttacker->m_cPunish = 1;
				pAttacker->m_dwPunishedTime = g_pTimerManager->GetServerTime();
			}

			float fAngle = pAttacker->m_fWantAngle;

			if (!pTarget)
				pAttacker->Attack((ECHAR_MOTION)pAttack->Motion, TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY), static_cast<char>(pAttack->SkillIndex));
			else if ((unsigned char)pAttack->Motion != 254)
			{
				if (pAttacker->m_sHeadIndex <= 50)
					pAttacker->Attack((ECHAR_MOTION)pAttack->Motion, pTarget, pAttack->SkillIndex);
				else
					pAttacker->Attack((ECHAR_MOTION)pAttack->Motion, pTarget, *(unsigned char*)&pAttack->SkillIndex);

				fHeight = pTarget->m_fHeight + 1.5f;

				if (pAttacker != pTarget)
					fAngle = atan2f(pTarget->m_vecPosition.x - pAttacker->m_vecPosition.x, pTarget->m_vecPosition.y - pAttacker->m_vecPosition.y) + D3DXToRadian(90);
			}

			if (pAttack->SkillIndex == 98) // Superior Cannon
				fAngle = atan2f((float)pAttack->TargetX - pAttacker->m_vecPosition.x, (float)pAttack->TargetY - pAttacker->m_vecPosition.y) + D3DXToRadian(90);
			if (pAttack->DoubleCritical & 1)
				pAttacker->m_bDoubleAttack = 1;
			if (pAttacker->m_nClass != 44)
				pAttacker->SetWantAngle(fAngle);

			if (pTarget && pTarget != pAttacker && pTarget != m_pMyHuman && (pTarget->m_nClass != 56 || pTarget->m_stLookInfo.FaceMesh)	&&
				pAttack->SkillIndex != 27)
			{
				pTarget->SetWantAngle(fAngle + D3DXToRadian(180));
			}

			if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104 ||
				pAttack->SkillIndex >= 151 && pAttack->SkillIndex <= 155 ||
				pAttack->SkillIndex == 104 || pAttack->SkillIndex == 105 || pAttack->SkillIndex == 111)
			{
				pAttacker->m_stEffectEvent.sEffectIndex = pAttack->SkillIndex;
				pAttacker->m_stEffectEvent.sEffectLevel = (unsigned char)pAttack->SkillParm;
				if ((unsigned char)pAttack->Motion == 254)
					pAttacker->m_stEffectEvent.sEffectLevel = 1;

				if (!pTarget)
				{
					float iY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY)) * 0.1f;
					pAttacker->m_stEffectEvent.vecTo = TMVector3((float)pAttack->TargetX, iY, (float)pAttack->TargetY);
					pAttacker->m_stEffectEvent.pTarget = 0;
				}
				else
				{
					pAttacker->m_stEffectEvent.pTarget = pTarget;
					if (pTarget && pAttacker && pTarget->m_nClass == 56 && !pTarget->m_stLookInfo.FaceMesh)
					{
						TMVector3 Len{ pTarget->m_vecPosition.x - pAttacker->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y - pAttacker->m_vecPosition.y };
						pAttacker->m_stEffectEvent.vecTo = TMVector3((float)(Len.x / 2.0f) + pAttacker->m_vecPosition.x, pTarget->m_fHeight, (float)(Len.z / 2.0f) + pAttacker->m_vecPosition.y);
					}
					else
						pAttacker->m_stEffectEvent.vecTo = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y);
				}

				if (pAttack->SkillIndex >= 151 && pAttack->SkillIndex <= 153 ||
					pAttack->SkillIndex == 104 || pAttack->SkillIndex == 105)
				{
					pAttacker->m_stEffectEvent.dwTime = g_pTimerManager->GetServerTime() + 200;
				}
				else
				{
					pAttacker->m_stEffectEvent.dwTime = g_pTimerManager->GetServerTime() + 500;
				}
			}
			if (pAttacker->m_nClass == 62 && pAttacker->m_stLookInfo.FaceMesh == 2 && pAttack->SkillIndex == 108 && pTarget)
			{
				TMVector3 vecStart{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
				TMVector3 vecDest{ pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y };

				vecDest.y += 1.0f;
				vecStart.y += 1.0f;

				auto pMagic = new TMSkillMagicArrow(vecStart, vecDest, 5, nullptr);
				if (pMagic && m_pEffectContainer)
					m_pEffectContainer->AddChild(pMagic);
			}
			else if (pAttack->SkillIndex == 6) // Life Aura
			{
				TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };

				auto pEffect = new TMEffectSpark(vecPos, pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFF5555FF, 0xFF222299, 1000, 1.0f, 5, 0.0f);
				if (pEffect && m_pEffectContainer)
					m_pEffectContainer->AddChild(pEffect);
			}
			else if (pAttack->SkillIndex == 3) // Pursuit
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					vecPos.y -= 0.5f;

					auto pHoly = new TMSkillHolyTouch(vecPos, 1);

					if (pHoly && m_pEffectContainer)
						m_pEffectContainer->AddChild(pHoly);
				}
			}
			else if (pAttack->SkillIndex == 5) // Fanaticism
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					vecPos.y -= 0.5f;

					auto pEffect = new TMEffectStart(vecPos, 2, nullptr);
					if (pEffect && m_pEffectContainer)
						m_pEffectContainer->AddChild(pEffect);

					GetSoundAndPlay(151, 0, 0);
				}
			}
			else if (pAttack->SkillIndex == 45) // Magic Weapon
			{
				float fY = (float)pAttack->TargetY + 0.5f;
				TMVector3 vecTarget{ (float)pAttack->TargetX + 0.5f, (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, fY)) * 0.1f, fY };

				if (pTarget)
					vecTarget = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight + 1.0f, pTarget->m_vecPosition.y);

				auto pParticle = new TMEffectParticle(vecTarget, 0, 20, 0.1f, 0, 1, 56, 1.0f, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000);
				if (pParticle && m_pEffectContainer)
					m_pEffectContainer->AddChild(pParticle);

				GetSoundAndPlay(158, 0, 0);
			}
			else if (pAttack->SkillIndex == 34 && (unsigned char)pAttack->Motion == 254) // Lightning
			{
				if (pAttacker && pAttacker->m_pFamiliar)
				{
					auto pEffect = new TMEffectBillBoard(1, 700, 0.1f, 0.1f, 0.1f, 0.002f, 1, 80);
					if (pEffect)
					{
						pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pEffect->m_vecStartPos = TMVector3(pAttacker->m_pFamiliar->m_vecPosition.x, pAttacker->m_pFamiliar->m_fHeight + 0.1f, pAttacker->m_pFamiliar->m_vecPosition.y);
						pEffect->SetColor(0xFFAAEEFF);
						m_pEffectContainer->AddChild(pEffect);
					}
				}
			}
			else if (pAttack->SkillIndex == 32 && (unsigned char)pAttack->Motion == 254) // Rebirth
			{
				if (pAttacker && pAttacker->m_pFamiliar)
				{
					auto pEffect = new TMEffectBillBoard(1, 700, 0.1f, 0.1f, 0.1f, 0.002f, 1, 80);
					if (pEffect)
					{
						pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pEffect->m_vecStartPos = TMVector3(pAttacker->m_pFamiliar->m_vecPosition.x, pAttacker->m_pFamiliar->m_fHeight + 0.1f, pAttacker->m_pFamiliar->m_vecPosition.y);
						pEffect->SetColor(0xFFFFAA00);
						m_pEffectContainer->AddChild(pEffect);
					}
				}
			}
			else if (pAttack->SkillIndex == 36) // Meteor Storm
			{
				if ((unsigned char)pAttack->Motion == 254 && pAttacker && pAttacker->m_pFamiliar)
				{
					auto pEffect = new TMEffectBillBoard(1, 700, 0.1f, 0.1f, 0.1f, 0.002f, 1, 80);
					if (pEffect)
					{
						pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pEffect->m_vecStartPos = TMVector3(pAttacker->m_pFamiliar->m_vecPosition.x, pAttacker->m_pFamiliar->m_fHeight + 0.1f, pAttacker->m_pFamiliar->m_vecPosition.y);
						pEffect->SetColor(0xFFAAEEFF);
						m_pEffectContainer->AddChild(pEffect);
					}
				}

				TMVector3 vecPos{};
				if (pTarget)
					vecPos = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y);
				else
				{
					float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f)) * 0.1f;
					vecPos = TMVector3((float)pAttack->TargetX + 0.5f, fY, (float)pAttack->TargetY + 0.5f);
				}

				for (int i = 0; i < 6; i++)
				{
					auto pMeteor = new TMSkillMeteorStorm(TMVector3(0.0f, 0.0f, 0.0f), TMVector3(((float)(i % 3) * 0.3f) + vecPos.x,
						vecPos.y, ((float)((i + 3) % 5) * 0.3f) + vecPos.z), 1, 0);

					if (pMeteor && m_pEffectContainer)
						m_pEffectContainer->AddChild(pMeteor);
				}
			}
			// Poison Mist, Divine Shock, Fire Attack, Holy Touch
			else if (pAttack->SkillIndex == 41 || pAttack->SkillIndex == 29	|| pAttack->SkillIndex == 33 || pAttack->SkillIndex == 2)
			{
				for (int i = 0; i < targetCount; i++)
				{
					if ((unsigned char)pAttack->Motion == 254)
					{
						if (i >= 4)
							break;
					}
					else if (g_pSpell[pAttack->SkillIndex].MaxTarget <= i)
						break;

					auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pTargetHuman && pTargetHuman != pAttacker)
					{
						if (pAttack->SkillIndex == 33)
						{
							if (!pAttack->FlagLocal || pAttack->FlagLocal == 1 && pAttacker && pAttacker == m_pMyHuman)
							{
								int nValue = 0;
								if ((unsigned char)pAttack->Motion == 254)
									nValue = 1;
								if ((unsigned char)pAttack->Motion == 253)
									nValue = 2;
								if ((unsigned char)pAttack->Motion == 252)
								{
									auto vecPos = TMVector3((float)pAttack->PosX + 0.5f, 1.0f, (float)pAttack->PosY + 0.5f);
									if (pAttacker)
										vecPos = TMVector3(pAttacker->m_vecPosition.x, (float)(pAttacker->m_fScale * 5.5999999f) + pAttacker->m_fHeight, pAttacker->m_vecPosition.y);

									auto pEffect = new TMEffectSpark(vecPos, pTargetHuman, TMVector3(0.0f, 0.0f, 0.0f),
										0xFF5555FF, 0xFF222299, 1000, 1.0f, 5, 0.0f);

									if (pEffect && m_pEffectContainer)
										m_pEffectContainer->AddChild(pEffect);
								}
								else
								{
									auto pThunder = new TMSkillThunderBolt(TMVector3(pTargetHuman->m_vecPosition.x,
										pTargetHuman->m_fHeight,
										pTargetHuman->m_vecPosition.y), nValue);

									if (pThunder && m_pEffectContainer)
										m_pEffectContainer->AddChild(pThunder);
								}
							}
						}
						else
						{
							pTargetHuman->m_stEffectEvent.sEffectIndex = pAttack->SkillIndex;
							pTargetHuman->m_stEffectEvent.sEffectLevel = (unsigned char)pAttack->SkillParm;
							pTargetHuman->m_stEffectEvent.pTarget = pTargetHuman;

							if (pTargetHuman && pAttacker && pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh)
							{
								TMVector3 Len{ pTargetHuman->m_vecPosition.x - pAttacker->m_vecPosition.x, pTargetHuman->m_fHeight,
									pTargetHuman->m_vecPosition.y - pAttacker->m_vecPosition.y };

								pAttacker->m_stEffectEvent.vecTo = TMVector3((float)(Len.x / 2.0) + pAttacker->m_vecPosition.x,
									pTargetHuman->m_fHeight,
									(float)(Len.z / 2.0) + pAttacker->m_vecPosition.y);
							}
							else
							{
								pTargetHuman->m_stEffectEvent.vecTo = TMVector3(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight, pTargetHuman->m_vecPosition.y);
							}

							pTargetHuman->m_stEffectEvent.dwTime = g_pTimerManager->GetServerTime() + 500;
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 52) // Weaken
			{
				if ((unsigned char)pAttack->Motion == 254)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					if (pTarget)
					{
						TMVector3 vecTarget{ pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y };

						auto pSlowSlash = new TMSkillSlowSlash(vecPos, vecTarget, 2, pTarget);
						if (pSlowSlash)
							m_pEffectContainer->AddChild(pSlowSlash);

						unsigned int dwServerTime = g_pTimerManager->GetServerTime();

						auto pLevelUp = new TMEffectLevelUp(vecTarget, 2);
						if (pLevelUp)
							m_pEffectContainer->AddChild(pLevelUp);

						if (pTarget->m_pEleStream2)
							pTarget->m_pEleStream2->StartVisible(dwServerTime);

						GetSoundAndPlay(156, 0, 0);
					}
				}
				else
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					TMVector3 vecTarget;
					if (pTarget)
						vecTarget = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y);
					else
					{
						float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f)) * 0.1f;
						vecTarget = TMVector3((float)pAttack->TargetX + 0.5f, fY, (float)pAttack->TargetY + 0.5f);
					}
					auto result = ((vecTarget - vecPos) / 7.0f);
					vecTarget = vecPos + result;

					auto pFreeze = new TMSkillFreezeBlade(vecTarget, 2, 0, 0);
					if (pFreeze)
						pFreeze->m_vecNextD = TMVector2(result.x, result.z);

					if (m_pEffectContainer && pFreeze)
						m_pEffectContainer->AddChild(pFreeze);
				}
			}
			// Recover, Exterminate
			else if (pAttack->SkillIndex == 30 || pAttack->SkillIndex == 23)
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 0.5f, pAttacker->m_vecPosition.y };
					int nType = 0;
					if (pAttack->SkillIndex == 23)
					{
						nType = 2;

						if (pTarget)
							vecPos = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight + 0.5f, pTarget->m_vecPosition.y);
						else
							vecPos = TMVector3((float)pAttack->TargetX + 0.5f, fHeight + 0.5f, (float)pAttack->TargetY + 0.5f);
					}

					auto pJudgement = new TMSkillJudgement(vecPos, nType, 0.1f);
					if (pJudgement)
						m_pEffectContainer->AddChild(pJudgement);
				}
			}
			else if (pAttack->SkillIndex == 55) // Beast Aura
			{
				TMVector3 vecPos;
				if (pTarget)
				{
					vecPos = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight + 0.2f, pTarget->m_vecPosition.y);

					auto pJudgement = new TMSkillJudgement(vecPos, 3, 0.1f);
					if (pJudgement)
						m_pEffectContainer->AddChild(pJudgement);
				}
				else
				{
					float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f)) * 0.1f;
					vecPos = TMVector3((float)pAttack->TargetX + 0.5f, fY + 0.2f, (float)pAttack->TargetY + 0.5f);
				}

				for (int i = 0; i < targetCount; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					auto pOwner = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pOwner)
					{
						auto pEffect = new TMEffectStart(TMVector3(pOwner->m_vecPosition.x, pOwner->m_fHeight + 0.5f, pOwner->m_vecPosition.y), 5, pOwner);

						if (pEffect)
						{
							pEffect->m_dwLifeTime = 2000;
							m_pEffectContainer->AddChild(pEffect);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 76) // Immunity
			{
				GetSoundAndPlay(34, 0, 0);
			}
			else if (pAttack->SkillIndex == 77) // Meditation
			{
				GetSoundAndPlay(36, 0, 0);
			}
			else if (pAttack->SkillIndex == 87) // Golden Shield
			{
				if (!g_bHideEffect)
				{
					for (int i = 0; i < 10; i++)
					{
						int nRand = rand() % 5;

						auto pBill = new TMEffectBillBoard(
							56,
							150 * i + 1000,
							((float)(i % 2) * 0.1f) + 0.2f,
							((float)(i % 2) * 0.1f) + 0.2f,
							((float)(i % 2) * 0.1f) + 0.2f,
							0.0f,
							1,
							80);

						if (pBill)
						{
							pBill->m_fParticleH = (float)((float)(i % 3) * 0.15f) + 1.1f;
							pBill->m_fParticleV = -2.0f;
							pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
							pBill->m_nParticleType = 3;

							pBill->m_vecStartPos = TMVector3((pAttacker->m_vecPosition.x + 0.5f) + ((float)(nRand - 3) * 0.1f),
								((pAttacker->m_fHeight + 0.5f) + ((float)(nRand - 3) * 0.1f)) + 2.0f,
								(pAttacker->m_vecPosition.y + 0.5f) + ((float)(nRand - 3) * 0.1f));

							pBill->SetColor(0xFF4444FF);

							m_pEffectContainer->AddChild(pBill);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 86) // Ethereal Explosion
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 0.5f, pAttacker->m_vecPosition.y };
					TMVector2 vecSum{};

					int nCount = 0;
					for (int i = 0; i < targetCount; i++)
					{
						if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
							break;

						if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
							break;

						auto pOwner = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
						if (pOwner)
						{
							TMVector3 vecDest{ pOwner->m_vecPosition.x, pAttacker->m_fHeight + 0.5f, pOwner->m_vecPosition.y };

							vecSum.x += pOwner->m_vecPosition.x;
							vecSum.y += pOwner->m_vecPosition.y;
							++nCount;

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
					}

					TMVector2 vecDNormal{};
					if (nCount <= 0)
						vecDNormal = TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f);
					else
						vecDNormal = vecSum / static_cast<const float>(nCount);

					float fAng = atan2f(vecDNormal.x - pAttacker->m_vecPosition.x, -vecDNormal.y + pAttacker->m_vecPosition.y);
					fAng -= D3DXToRadian(90);

					if (fAng < 0.0f)
						fAng += D3DXToRadian(360);
					if (fAng > D3DXToRadian(360))
						fAng -= D3DXToRadian(360);

					int nIndex = (int)((fAng * 12.0f) / D3DXToRadian(360));
					int nMin = nIndex - 3;
					int nMax = nIndex + 3;

					if (nMin < 0 || nMax > 12)
					{
						int nLow = 0;
						int nHigh = 0;

						if (nMin >= 0)
						{
							nLow = nMin;
							nHigh = nMax - 12;
						}
						else
						{
							nLow = nMin + 12;
							nHigh = nMax;
						}

						for (int i = 0; i < nHigh; i++)
						{
							TMVector2 vecCDest{ vecDNormal.x + m_vecKnifePos[i].x, vecDNormal.y + m_vecKnifePos[i].y };
							TMVector3 vecDest{ vecCDest.x, pAttacker->m_fHeight + 0.5f, vecCDest.y };

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
						for (int i = nLow; i < 12; i++)
						{
							TMVector2 vecCDest{ vecDNormal.x + m_vecKnifePos[i].x, vecDNormal.y + m_vecKnifePos[i].y };
							TMVector3 vecDest{ vecCDest.x, pAttacker->m_fHeight + 0.5f, vecCDest.y };

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
					}
					else
					{
						for (int i = nMin; i < nMax; i++)
						{
							TMVector2 vecCDest{ vecDNormal.x + m_vecKnifePos[i].x, vecDNormal.y + m_vecKnifePos[i].y };
							TMVector3 vecDest{ vecCDest.x, pAttacker->m_fHeight + 0.5f, vecCDest.y };

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 99) // Wall of Thorns
			{
				if (pTarget)
				{
					bool bExpand = false;
					if (pTarget->m_nClass == 4 || pTarget->m_nClass == 8)
						bExpand = 1;

					auto pEffectSkinMesh = new TMEffectSkinMesh(
						pTarget->m_nSkinMeshType,
						TMVector3(0.0f, 0.0f, 0.0f),
						TMVector3(0.0f, 0.0f, 0.0f),
						0,
						nullptr);

					if (pTarget && pTarget->m_cMount > 0 && pTarget->m_pMount)
					{
						pEffectSkinMesh->m_nSkinMeshType = pTarget->m_nMountSkinMeshType;
						memcpy(&pEffectSkinMesh->m_stLookInfo, &pTarget->m_stMountLook, sizeof(pTarget->m_stMountLook));
						pEffectSkinMesh->m_nSkinMeshType2 = pTarget->m_nSkinMeshType;
						memcpy(&pEffectSkinMesh->m_stLookInfo2, &pTarget->m_stLookInfo, sizeof(pTarget->m_stLookInfo));
					}
					else
					{
						memcpy(&pEffectSkinMesh->m_stLookInfo, &pTarget->m_stLookInfo, sizeof(pTarget->m_stLookInfo));
					}

					pEffectSkinMesh->m_StartColor.r = 0.5f;
					pEffectSkinMesh->m_StartColor.g = 0.5f;
					pEffectSkinMesh->m_StartColor.b = 0.5f;
					pEffectSkinMesh->InitObject(bExpand);
					pEffectSkinMesh->m_nFade = 1;
					pEffectSkinMesh->m_dwLifeTime = 3000;
					pEffectSkinMesh->InitPosition(pTarget->m_vecPosition.x, pTarget->m_fHeight + 6.0f, pTarget->m_vecPosition.y);

					if (pTarget->m_cMount > 0 && pTarget->m_pMount)
					{
						if (pEffectSkinMesh->m_pSkinMesh)
							pEffectSkinMesh->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffectSkinMesh->m_nSkinMeshType].dwAniTable[0]);
						if (pEffectSkinMesh->m_pSkinMesh2)
							pEffectSkinMesh->m_pSkinMesh2->SetAnimation(g_MobAniTable[pEffectSkinMesh->m_nSkinMeshType2].dwAniTable[24]);
					}
					else
					{
						pEffectSkinMesh->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffectSkinMesh->m_nSkinMeshType].dwAniTable[0]);
					}

					pEffectSkinMesh->m_fStartAngle = pTarget->m_fAngle;
					pEffectSkinMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
					pEffectSkinMesh->m_nMotionType = 8;

					m_pEffectContainer->AddChild(pEffectSkinMesh);
				}

				unsigned int dwColor = 0xAAAAAAAA;

				for(int i = -1; i < 2; i++)
				{
					auto pEffectMesh = new TMEffectMesh(506, dwColor, 0.0f, 0);

					pEffectMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
					pEffectMesh->m_cShine = 1;
					pEffectMesh->m_dwCycleTime = 3000;
					pEffectMesh->m_dwLifeTime = 3000;
					pEffectMesh->m_fScaleH = 1.5f;
					pEffectMesh->m_fScaleV = 3.0f;
					pEffectMesh->m_vecPosition = TMVector3(((float)i * 0.2f) + pTarget->m_vecPosition.x, pTarget->m_fHeight - 0.5f, ((float)i * 0.2f) + pTarget->m_vecPosition.y);

					m_pEffectContainer->AddChild(pEffectMesh);
				}

				auto pShade = new TMShade(4, 118, 1.0f);

				pShade->m_dwLifeTime = 8000;
				pShade->SetColor(dwColor);
				pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
				pShade->SetPosition(TMVector2(pTarget->m_vecPosition.x, pTarget->m_vecPosition.y));

				m_pEffectContainer->AddChild(pShade);

				GetSoundAndPlay(1, 0, 0);
			}
			else if (pAttack->SkillIndex == 100) // Resurrection
			{
				GetSoundAndPlay(156, 0, 0);
			}
			else if (pAttack->SkillIndex == 106 || pAttack->SkillIndex == 108)
			{
				int nSkill = 0;
				if (pAttack->SkillIndex == 108)
					nSkill = 1;

				if (pTarget && pAttacker)
					pAttacker->Fire(pTarget, nSkill);
			}
			else if (pAttack->SkillIndex == 107)
			{
				if (pTarget && pAttacker)
				{
					fHeight = ((float)(TMHuman::m_vecPickSize[pAttacker->m_nSkinMeshType].y	* pAttacker->m_fScale) * 0.69999999f) + pAttacker->m_fHeight;
					if(pAttacker->m_nSkinMeshType == 8)
						fHeight = (float)(1.0f * pAttacker->m_fScale) + fHeight;

					TMVector3 vecPos{ pAttacker->m_vecPosition.x, fHeight, pAttacker->m_vecPosition.y };

					auto pEffectSpark = new TMEffectSpark(vecPos, pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFFFFAA00, 0xFF554411, 1000, 1.0f, 5, 0.0f);

					m_pEffectContainer->AddChild(pEffectSpark);
				}
			}
			else if (pAttack->SkillIndex == 109)
			{
				if (pTarget && pAttacker)
				{
					fHeight = ((float)(TMHuman::m_vecPickSize[pAttacker->m_nSkinMeshType].y * pAttacker->m_fScale) * 0.69999999f) + pAttacker->m_fHeight;
					if (pAttacker->m_nSkinMeshType == 42)
						fHeight = (float)(1.0f * pAttacker->m_fScale) + fHeight;

					TMVector3 vecPos{ pAttacker->m_vecPosition.x, fHeight, pAttacker->m_vecPosition.y };

					auto pEffectSpark = new TMEffectSpark(vecPos, pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFF550000, 0xAA663311, 1000, 1.0f, 5, 0.0f);

					m_pEffectContainer->AddChild(pEffectSpark);
				}
			}
			else if (pAttack->SkillIndex == 110) // Skill Kefra
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};
				bool bMyAttack = false;
				for (int i = 0; i < targetCount; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					int rndx = rand() % 18;
					int rndy = rand() % 18;

					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);

					if (pMultiTarget)
					{
						if (pMultiTarget == m_pMyHuman)
							bMyAttack = true;

						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);

						vecStart = vecTarget;
						vecStart.y = 1.0f;

						auto pMeteor = new TMSkillMeteorStorm(vecStart, vecTarget, 3, nullptr);
						pMeteor->m_dwStartTime += rand() % 1000;

						m_pEffectContainer->AddChild(pMeteor);
					}
					else if (vecTarget.x != 0.0f && vecTarget.y != 0.0f && vecTarget.z != 0.0f)
					{
						auto pMeteor = new TMSkillMeteorStorm(
							TMVector3(((float)rndx + vecStart.x) - 9.0f, vecStart.y, ((float)rndy + vecStart.z) - 9.0f),
							TMVector3(((float)rndx + vecTarget.x) - 9.0f, vecTarget.y, ((float)rndy + vecTarget.z) - 9.0f),
							3, nullptr);

						pMeteor->m_dwStartTime += rand() % 1000;

						m_pEffectContainer->AddChild(pMeteor);
					}
				}

				if (pAttacker->m_dwEarthQuakeTime + 1700 < g_pTimerManager->GetServerTime())
				{
					pAttacker->m_dwEarthQuakeTime = g_pTimerManager->GetServerTime() - 1000;

					for (int i = -3; i < 3; i++)
					{
						for (int j = -3; i < 3; i++)
						{
							auto pBill = new TMEffectBillBoard(193, 4000, 2.0f, 2.0f, 2.0f, 0.001f, 1, 80);

							pBill->m_bStickGround = i % 2;
							pBill->m_vecPosition = TMVector3(((float)i * 2.0f) + pAttacker->m_vecPosition.x, pAttacker->m_fHeight, ((float)j * 2.0f) + pAttacker->m_vecPosition.y);
							pBill->SetColor(0xFF777799);

							m_pEffectContainer->AddChild(pBill);
						}
					}

					auto pGround = m_pGround;
					if (pGround)
					{
						pGround->m_dwEffStart = g_pTimerManager->GetServerTime();
						pGround->m_vecEffset = pAttacker->m_vecPosition;
					}
				}

				int nDistance = BASE_GetDistance((int)pAttacker->m_vecPosition.x, (int)pAttacker->m_vecPosition.y,
					(int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);

				unsigned int Now = g_pTimerManager->GetServerTime();

				unsigned int dwKhepraDelay = 2000;
				if (!bMyAttack)
					dwKhepraDelay = 1000;
				if (nDistance < 18)
				{
					if (dwKhepraDelay + m_pMyHuman->m_dwOldMovePacketTime < Now && !m_pMyHuman->m_cDie &&
						m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_DEAD &&
						m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_DIE)
					{
						int nRan = 4;
						int nLength = 5;

						if (!bMyAttack)
						{
							nRan = 8;
							nLength = 9;
						}

						if (rand() % 10 < nRan)
						{
							m_pTargetHuman = nullptr;
							if (!g_bRunning)
								SetRunMode();

							int x = (int)m_pMyHuman->m_vecPosition.x;
							int y = (int)m_pMyHuman->m_vecPosition.y;

							int nMoveX = 0;
							int nMoveY = 0;
							if (x <= 2362)
								nMoveX = -nLength;
							else if (x >= 2370)
								nMoveX = nLength;
							if (y <= 3927)
								nMoveY = -nLength;
							else if (y >= 3935)
								nMoveY = nLength;

							int targetx = (int)((float)nMoveX + m_pMyHuman->m_vecPosition.x);
							int targety = (int)((float)nMoveY + m_pMyHuman->m_vecPosition.y);
							if (targetx < 2341)
								targetx = 2341;
							else if (targetx > 2391)
								targetx = 2391;
							if (targety < 3907)
								targetx = 3907;
							else if (targety > 3952)
								targety = 3952;

							char Route[48]{};
							BASE_GetRoute(x, y, &targetx, &targety, Route, 12, (char*)m_HeightMapData, 8);
							if (!strlen(Route))
								return 1;

							MSG_Action Msg{};
							Msg.Header.ID = m_pMyHuman->m_dwID;
							Msg.Header.Type = MSG_Action_Opcode;
							Msg.PosX = targetx;
							Msg.PosY = targety;
							Msg.Effect = 2;
							Msg.TargetX = targetx;
							Msg.TargetY = targety;

							g_bLastStop = MSG_Action_Opcode;

							m_stMoveStop.LastX = targetx;
							m_stMoveStop.LastY = targety;
							m_stMoveStop.NextX = targetx;
							m_stMoveStop.NextY = targety;

							SendOneMessage((char*)&Msg, sizeof(Msg));

							m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
							m_pMyHuman->OnPacketEvent(MSG_Action_Opcode, (char*)&Msg);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 111)
			{
				TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };

				auto pShade = new TMShade(70, 118, 1.0f);

				pShade->SetColor(0xAA660000);
				pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
				pShade->SetPosition(TMVector2(pAttacker->m_vecPosition.x, pAttacker->m_vecPosition.y));
				pShade->m_dwLifeTime = 3200;
				m_pEffectContainer->AddChild(pShade);

				TMVector3 vecTarget{};
				TMVector3 vecTarget2{};
				TMVector3 vecD{};
				TMVector3 vecPos2{};
				D3DXVECTOR3 vecDNormal{};

				for (int i = 0; i < targetCount; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					int rndx = rand() % 18;
					int rndy = rand() % 18;
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x, pMultiTarget->m_fHeight, pMultiTarget->m_vecPosition.y);
						vecTarget2 = vecTarget;

						auto pFreeze = new TMSkillFreezeBlade(vecTarget2, 8, 1, 0);
						pFreeze->m_vecNextD = TMVector2(vecD.x, vecD.z);

						m_pEffectContainer->AddChild(pFreeze);
					}
					else if (vecTarget.x != 0.0f && vecTarget.y != 0.0f && vecTarget.z != 0.0f)
					{
						vecTarget2 = TMVector3(((float)rndx + vecTarget.x) - 9.0f, vecTarget.y, ((float)rndy + vecTarget.z) - 9.0f);

						auto pFreeze = new TMSkillFreezeBlade(vecTarget2, 8, 1, 0);
						pFreeze->m_vecNextD = TMVector2(vecD.x, vecD.z);

						m_pEffectContainer->AddChild(pFreeze);
					}
				}
			}
			else if (pAttacker->m_nClass == 56 && !pAttacker->m_stLookInfo.FaceMesh)
			{
				TMVector3 vecSpTarget = pAttacker->m_vecTempPos[2];
				TMVector3 vecSpStart = pAttacker->m_vecTempPos[1];
				TMVector3 vecStart{};
				TMVector3 vecTarget{};
				for (int i = 0; i < targetCount; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					int rndx = rand() % 18;
					int rndy = rand() % 18;

					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						TMVector3 vecStart = pAttacker->m_vecTempPos[i % 2 + 1];
						TMVector3 vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight + 1.0f,
							pMultiTarget->m_vecPosition.y);

						auto pEffectSpark = new TMEffectSpark(vecStart,
							pMultiTarget,
							TMVector3(0.0f, 0.0f, 0.0f),
							0xFFFF0000,
							0xFF222299,
							1000,
							1.0f,
							5,
							0.0);

						pEffectSpark->m_fRange = 0.2f;
						m_pEffectContainer->AddChild(pEffectSpark);
					}
					else if (vecTarget.x != 0.0f && vecTarget.y != 0.0f && vecTarget.z != 0.0f)
					{
						vecStart = pAttacker->m_vecTempPos[i % 2 + 1];

						auto pEffectSpark = new TMEffectSpark(vecStart,
							nullptr,
							TMVector3(((float)rndx + vecTarget.x) - 9.0f,
								vecTarget.y,
								(float)((float)rndy + vecTarget.z) - 9.0f),
							0xFFFF0000,
							0xFF222299,
							1000,
							1.0f,
							5,
							0.0);

						pEffectSpark->m_fRange = 0.4f;
						m_pEffectContainer->AddChild(pEffectSpark);
					}
				}
			}
			else if (pAttack->SkillIndex == 113)
			{
				pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);
				unsigned int dwColor = 0xFF33FF66;

				TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };

				auto pShade = new TMShade(70, 118, 1.0f);
				pShade->SetColor(0xAA660000);
				pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
				pShade->SetPosition(TMVector2(pAttacker->m_vecPosition.x, pAttacker->m_vecPosition.y));
				pShade->m_dwLifeTime = 3200;
				m_pEffectContainer->AddChild(pShade);

				TMVector3 vecTarget{};
				TMVector3 vecD{};
				TMVector3 vecPos2{};
				D3DXVECTOR3 vecDNormal{};

				for (int i = 0; i < targetCount; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight,
							pMultiTarget->m_vecPosition.y);

						auto pFreeze = new TMSkillFreezeBlade(vecTarget, 8, 1, 0);
						m_pEffectContainer->AddChild(pFreeze);

						auto pPoison = new TMSkillPoison(vecTarget, dwColor, 10, 1, 0);
						m_pEffectContainer->AddChild(pPoison);
					}
				}
			}
			else if (pAttack->SkillIndex == 114)
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};

				bool bMyAttack = false;
				pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);

				for (int i = 0; i < targetCount; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);
						vecTarget = vecStart;
						vecStart.y = 1.0f;

						auto pJudgement = new TMSkillJudgement(vecStart, 2, 0.1f);

						m_pEffectContainer->AddChild(pJudgement);

						TMVector3 vec{};
						for (int j = 0; j < 4; j++)
						{
							vec = TMVector3(((float)(2 * (j % 2)) + vecTarget.x) - 1.0f,
								vecTarget.y,
								((float)(2 * (j / 2)) + vecTarget.z) - 1.0f);

							auto pFreeze = new TMSkillFreezeBlade(
								vec, 1, 0, 0
							);

							m_pEffectContainer->AddChild(pFreeze);
						}

						TMVector3 vecDest{ pMultiTarget->m_vecPosition.x,
								  pMultiTarget->m_fHeight + 0.5f,
								  pMultiTarget->m_vecPosition.y };

						vecStart = TMVector3(vecDest.x + 3.0f, vecDest.y + 5.0f, vecDest.z - 3.0f);

						auto pArrow = new TMArrow(vecStart, vecDest, 0, 10001, 0, 0, 0);
						pArrow->m_nColor = 0xFFFFFF00;

						m_pEffectContainer->AddChild(pArrow);
					}
				}
			}
			else if (pAttack->SkillIndex == 115)
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};
				if (pAttacker->m_nClass != 73)
					pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);

				for (int i = 0; i < targetCount; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);

						vecStart = vecTarget;
						vecStart.y = 10.0f;

						auto pMeteor = new TMSkillMeteorStorm(vecStart,
							vecTarget,
							3,
							nullptr);

						m_pEffectContainer->AddChild(pMeteor);
					}
				}

				int ran = rand() % 10;

				if (ran >= 6)
				{
					if (pAttacker->m_dwEarthQuakeTime + 4000 < g_pTimerManager->GetServerTime())
					{
						pAttacker->m_dwEarthQuakeTime = g_pTimerManager->GetServerTime() - 2000;

						for (int i = -3; i < 3; i++)
						{
							for (int j = -3; j < 3; j++)
							{
								auto pBill = new TMEffectBillBoard(193,
									4000,
									2.0f,
									2.0f,
									2.0f,
									0.001f,
									1,
									80);

								pBill->m_bStickGround = i % 2;
								pBill->m_vecPosition = TMVector3(((float)i * 2.0f) + pAttacker->m_vecPosition.x,
									pAttacker->m_fHeight,
									(float)((float)j * 2.0f) + pAttacker->m_vecPosition.y);
								pBill->SetColor(0xFF777799);

								m_pEffectContainer->AddChild(pBill);
							}
						}

						auto pGround = m_pGround;
						if (pGround)
						{
							pGround->m_dwEffStart = g_pTimerManager->GetServerTime();
							pGround->m_vecOffset = pAttacker->m_vecPosition;
						}

						auto pCamera = g_pObjectManager->GetCamera();
						pCamera->EarthQuake(11);

						GetSoundAndPlay(160, 0, 0);
					}
				}
			}
			else if (pAttack->SkillIndex == 116)
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};
				pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);

				for (int i = 0; i < targetCount; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);

						vecStart = vecTarget;
						vecStart.y = 10.0f;

						auto pMeteor = new TMSkillMeteorStorm(vecStart,
							vecTarget,
							3,
							0);

						m_pEffectContainer->AddChild(pMeteor);
					}
				}

				for (int i = -3; i < 3; i++)
				{
					for (int j = -3; j < 3; j++)
					{
						auto pBill = new TMEffectBillBoard(193,
							4000,
							2.0f,
							2.0f,
							2.0f,
							0.001f,
							1,
							80);

						pBill->m_bStickGround = i % 2;
						pBill->m_vecPosition = TMVector3(((float)i * 2.0f) + pAttacker->m_vecPosition.x,
							pAttacker->m_fHeight,
							(float)((float)j * 2.0f) + pAttacker->m_vecPosition.y);
						pBill->SetColor(0xFF777799);

						m_pEffectContainer->AddChild(pBill);
					}
				}
			}
		}
	}
	else
	{
		vecAttackerPos = TMVector2((float)pAttack->PosX + 0.5f, (float)pAttack->PosY + 0.5f);
		bool bFind = false;
		for (int i = 0; i < targetCount; i++)
		{
			if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
				break;

			if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
				break;

			if (pAttack->Dam[i].TargetID == m_pMyHuman->m_dwID)
			{
				bFind = true;
				break;
			}
		}

		if (bFind || pAttack->SkillIndex > 55 && pAttack->SkillIndex > 63)
		{
			MSG_REQMobByID stReqMobById{};
			stReqMobById.Header.ID = g_pObjectManager->m_dwCharID;
			stReqMobById.Header.Type = MSG_REQMobByID_Opcode;
			stReqMobById.MobID = static_cast<short>(pAttack->AttackerID);
			SendPacket({reinterpret_cast<MSG_STANDARD*>(&stReqMobById)->Type, reinterpret_cast<char*>(&stReqMobById), sizeof(stReqMobById)});
		}
	}

	TMVector3 vecStart{};
	TMVector3 vecTarget{};

	if (pAttacker)
		vecStart = TMVector3(pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y);
	else
	{
		float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY)) * 0.1f;
		vecStart = TMVector3(vecAttackerPos.x, fY + 1.0f, vecAttackerPos.y);
	}

	unsigned int dwDelay = 0;
	if (g_pSpell[pAttack->SkillIndex].TargetType != 3 && g_pSpell[pAttack->SkillIndex].TargetType != 4 &&
		g_pSpell[pAttack->SkillIndex].TargetType != 5 && g_pSpell[pAttack->SkillIndex].TargetType != 6)
	{
		for (int i = 0; i < targetCount; i++)
		{
			if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
				break;

			if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
				break;

			auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
			if (pTargetHuman)
			{
				int nDamageRate = pTargetHuman->m_cDamageRate;
				if (nDamageRate == 0)
					nDamageRate = 1;

				if (m_pMyHuman)
				{
					int nDistance = BASE_GetDistance((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y,
						(int)pTargetHuman->m_vecPosition.x,	(int)pTargetHuman->m_vecPosition.y);

					if (nDistance > 20)
					{
						OutputDebugString(">>> damage font error distance : %d\n");
						return 0;
					}
				}
				if (pAttacker != m_pMyHuman || pAttack->FlagLocal == 0 && pAttacker == m_pMyHuman)
				{
					if (pAttack->Dam[i].Damage == -3 || pAttack->Dam[i].Damage == -4)
					{
						int nTX = 0;
						int nTY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nTX, &nTY))
						{
							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								char szStr[128]{};
								sprintf(szStr, "miss");

								auto pFont = new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFFFFFF, 2.0f, dwDelay, 1, 1500, 0, 4);
								m_pExtraContainer->AddChild(pFont);
							}
						}
					}
					else if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104 &&
						g_pSpell[pAttack->SkillIndex].InstanceType == 6 &&
						(pAttack->Dam[i].Damage >= 0 || pAttack->Dam[i].Damage <= -6))
					{
						if (!pAttack->FlagLocal)
						{
							if (!pTargetHuman->m_MaxBigHp)
								attack_target::HealPool(pTargetHuman->m_stScore.CurHP,
									pTargetHuman->m_stScore.MaxHP, pAttack->Dam[i].Damage, nDamageRate);
							else
								attack_target::HealBigPool(pTargetHuman->m_stScore.CurHP,
									pTargetHuman->m_BigHp, pAttack->Dam[i].Damage, nDamageRate);

							if (pTargetHuman == m_pMyHuman)
							{
								int HealDam = pAttack->Dam[i].Damage / nDamageRate;
								int Dam = HealDam + m_nReqHP;
								// TODO: change to define
								if (Dam > 100000)
									Dam = 100000;
								if (Dam <= 0)
									m_nReqHP = 0;
								else
									m_nReqHP = Dam;

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}
						int nTX = 0;
						int nTY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y,	&nTX, &nTY))
						{
							char szStr[128]{};
							if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
								(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
								&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
							{
								sprintf(szStr, "?");
							}
							else
								sprintf(szStr, "+ %d", -pAttack->Dam[i].Damage / ((pAttack->DoubleCritical & 1) + 1));

							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								auto pFont = new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFF5555FF, 2.0f, dwDelay, 1, 1500, 0, 2);
								m_pExtraContainer->AddChild(pFont);
							}
						}
						if (pAttack->AttackerID == m_pMyHuman->m_dwID)
							SetMyHumanExp(pAttack->CurrentExp, pAttack->FakeExp);
					}
					else
					{
						if (!pAttack->FlagLocal)
						{
							int nValue = attack_target::Damage(pTargetHuman->m_stScore.CurHP,
								pTargetHuman->m_BigHp, pTargetHuman->m_MaxBigHp != 0,
								pAttack->Dam[i].Damage, nDamageRate);
							if (pTargetHuman->m_stScore.CurHP < 0)
								pTargetHuman->m_stScore.CurHP = 0;
							if (pTargetHuman == m_pMyHuman)
							{
								if (m_nReqHP - pAttack->Dam[i].Damage > 0)
									m_nReqHP -= nValue;
								else
									m_nReqHP = 0;

								if ((int)pAttack->AttackerID > 1000)
								{
									if (pAttack->ReqMp > 0)
										m_nReqMP -= pAttack->ReqMp;

									pTargetHuman->m_stScore.CurMP = m_nReqMP;
								}

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}

						pTargetHuman->m_wAttackerID = pAttack->AttackerID;

						int bInScreen = 0;
						int nTX = 0;
						int nTY = 0;
						if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pTargetHuman && pAttacker)
							bInScreen = BASE_Get3DTo2DPos(
								(float)(pTargetHuman->m_vecPosition.x * 0.5f) + (float)(pAttacker->m_vecPosition.x * 0.5f),
								pTargetHuman->m_fHeight - 1.0f,
								(float)(pTargetHuman->m_vecPosition.y * 0.5f) + (float)(pAttacker->m_vecPosition.y * 0.5f),
								&nTX,
								&nTY);
						else
							bInScreen = BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nTX, &nTY);

						if (bInScreen)
						{
							if (!pTargetHuman || !pAttacker)
								return 1;

							for (int bViewHalf = 0; bViewHalf < (pAttack->DoubleCritical & 1) + 1; bViewHalf++)
							{
								int nFlank = 0;
								if (pAttack->DoubleCritical & 4)
									nFlank = GetWYD748AttackVisualDamage(pAttack, 1);

								// The HP mutation above deliberately used the projected WORD; only
								// the floating number consumes the optional uint32 WYD-Go tail.
								int nValue = (GetWYD748AttackVisualDamage(pAttack, i) - nFlank) / ((pAttack->DoubleCritical & 1) + 1);
								if (nValue > 1000000)
									nValue = 0;

								if (nValue > 0)
								{
									char szStr[128]{};
									if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
										((int)pTargetHuman->m_vecPosition.x >> 7 != 18 || (int)pTargetHuman->m_vecPosition.y >> 7 != 30) &&
										(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
										&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
									{
										sprintf(szStr, "?");
									}
									else if (!(pAttack->DoubleCritical & 4) || bViewHalf)
									{
										sprintf(szStr, "%d", nValue);
									}
									else if (nFlank > 0)
									{
										sprintf(szStr, "%d + %d", nValue, nFlank);
									}

									bool bDrawFront = false;
									if (g_bHideEffect)
									{
										if (pAttacker == m_pMyHuman)
											bDrawFront = true;
										if (pTargetHuman == m_pMyHuman)
											bDrawFront = true;
									}
									else
									{
										bDrawFront = true;
									}

									if (pAttack->TargetX == 1129 && pAttack->TargetY == 1707 ||
										pAttack->TargetX == 1116 && pAttack->TargetY == 1707 ||
										pAttack->TargetX == 1094 && pAttack->TargetY == 1690)
									{
										bDrawFront = false;
									}

									if (bDrawFront)
									{
										TMFont3* pFont = nullptr;
										unsigned int dwColor = 0xFFFFFFFF;
										float fSize = 1.0f;
										if (i > 0 && pAttack->Dam[i].TargetID == pAttack->Dam[i - 1].TargetID)
										{
											m_pMyHuman->m_bCritical = 0;
											if (!(pAttack->DoubleCritical & 2))
											{
												if (pTargetHuman == m_pMyHuman)
												{
													if (bInScreen)
													{
														pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
															(int)(RenderDevice::m_fHeightRatio * 80.0f) +
															(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) -
																((float)(20 * bViewHalf) * RenderDevice::m_fHeightRatio)),
															dwColor,
															fSize,
															dwDelay,
															1,
															1200,
															bViewHalf,
															4);
													}
												}
												else if (bInScreen)
												{
													if (pAttack->SkillIndex < 0 || pAttack->SkillIndex > 150) // physical attack
													{
														pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
															(int)(RenderDevice::m_fHeightRatio * 80.0f) +
															(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) -
																((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
															dwColor,
															fSize,
															dwDelay,
															1,
															1200,
															bViewHalf,
															3);
													}
													else
													{
														pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
															(int)(RenderDevice::m_fHeightRatio * 80.0f) +
															(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) -
																((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
															dwColor,
															fSize,
															dwDelay,
															1,
															1200,
															bViewHalf,
															7);
													}
												}
											}
											else
											{
												m_pMyHuman->m_bCritical = 1;
												if (bInScreen)
												{
													if (pAttack->SkillIndex < 0 || pAttack->SkillIndex > 150) // physical attack
													{

														pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
															(int)(RenderDevice::m_fHeightRatio * 80.0f) +
															(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
																((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
															dwColor,
															fSize,
															dwDelay,
															9,
															500 * (pAttack->DoubleCritical & 2) + 1100 * bViewHalf + 1100,
															bViewHalf,
															5);

													}
													else
													{

														pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
															(int)(RenderDevice::m_fHeightRatio * 80.0f) +
															(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
																((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
															dwColor,
															fSize,
															dwDelay,
															9,
															300 * (pAttack->DoubleCritical & 2) + 1200,
															bViewHalf,
															2);
													}
												}
											}
										}
										else
										{
											m_pMyHuman->m_bCritical = 0;
											if (pAttack->DoubleCritical & 2)
											{
												m_pMyHuman->m_bCritical = 1;
												if (bInScreen)
												{

													if (pAttack->SkillIndex < 0 || pAttack->SkillIndex > 150) // physical attack
													{
														//pFont = new TMFont3(szStr, nTX,
														//	(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														//	(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) -
														//		((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														//	dwColor,
														//	fSize,
														//	dwDelay,
														//	9,
														//	550 * (pAttack->DoubleCritical & 2) + 1000 * bViewHalf + 1000,
														//	bViewHalf,
														//	5); //Physical critical hit

														pFont = new TMFont3(szStr, nTX,
															(int)(RenderDevice::m_fHeightRatio * 120.0f) +
															(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
																((float)(-60 * bViewHalf) * RenderDevice::m_fHeightRatio)) ,
															dwColor,
															fSize,
															dwDelay,
															9,
															550* (pAttack->DoubleCritical & 2) + 1100 + bViewHalf * 1100,
															bViewHalf,
															5); //Physical critical hit

													}
													else
													{
														pFont = new TMFont3(szStr, nTX,
															(int)(RenderDevice::m_fHeightRatio * 80.0f) +
															(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) -
																((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
															dwColor,
															fSize,
															dwDelay,
															9,
															550 * (pAttack->AttackerID & 2) + 1000 * bViewHalf + 1000,
															bViewHalf,
															8); //Magic critical hit
													}
												}
											}
											else if (pTargetHuman == m_pMyHuman)
											{
												if (bInScreen)
												{
													pFont = new TMFont3(szStr, nTX,
														(int)(RenderDevice::m_fHeightRatio * 40.0f) +
														(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
															((float)(20 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														dwColor,
														fSize,
														dwDelay,
														1,
														1000 * bViewHalf + 1000,
														bViewHalf,
														4);
												}
											}
											else if (bInScreen)
											{
												if (pAttack->SkillIndex < 0 || pAttack->SkillIndex > 150) // physical attack
												{
													pFont = new TMFont3(szStr, nTX,
														(int)(RenderDevice::m_fHeightRatio * 40.0f) +
														(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
															((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														dwColor,
														fSize,
														dwDelay,
														1,
														300 + 1100 * bViewHalf + 1100,
														bViewHalf,
														3); //Physical attack
												}
												else
												{

													pFont = new TMFont3(szStr, nTX,
														(int)(RenderDevice::m_fHeightRatio * 40.0f) +
														(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
															((float)(-50 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														dwColor,
														fSize,
														dwDelay,
														1,
														1000 * bViewHalf + 1000,
														bViewHalf,
														7); //Magic attack
												}
											}
										}

										if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pAttacker != m_pMyHuman)
										{
											if (pFont)
											{
												pFont->m_fScale = 0.5f;
												if (pFont->m_nType == 5)
													pFont->m_nType = 6;
											}
										}

										m_pExtraContainer->AddChild(pFont);
									}

									if (pTargetHuman == m_pMyHuman && pAttacker)
										sprintf(m_szLastAttackerName, "%s", pAttacker->m_szName);

									if (pTargetHuman->m_cCriticalArmor == 1)
									{
										auto pEffectMesh = new TMEffectMesh(2838, 0xFF999999, pTargetHuman->m_fAngle, 4);

										pEffectMesh->m_nTextureIndex = 413;
										pEffectMesh->m_dwLifeTime = 500;
										pEffectMesh->m_dwCycleTime = 500;

										if (pTargetHuman->m_cMount == 1)
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecSkinPos.x,
												(((TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_vecSkinPos.y)
												- 0.30000001f,
												pTargetHuman->m_vecSkinPos.z);
										}
										else
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecPosition.x,
												(((TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_fHeight)
												+ 0.30000001f,
												pTargetHuman->m_vecPosition.y);
										}

										pEffectMesh->m_fScaleH = 2.5f;
										pEffectMesh->m_fScaleV = 2.5f;
										pEffectMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
										pEffectMesh->m_cShine = 0;

										m_pEffectContainer->AddChild(pEffectMesh);
									}
								}
							}
						}
					}

					if (pTargetHuman->m_stScore.CurHP < 0 && !pAttack->FlagLocal)
					{
						pTargetHuman->m_stScore.CurHP = 0;
						m_nReqHP = 0;
					}
					if (pTargetHuman->m_stScore.CurHP > pTargetHuman->m_stScore.MaxHP && !pAttack->FlagLocal)
						pTargetHuman->m_stScore.CurHP = pTargetHuman->m_stScore.MaxHP;
					if (pTargetHuman == m_pMyHuman && !pAttack->FlagLocal)
						g_pObjectManager->m_stMobData.CurrentScore.CurHP = pTargetHuman->m_stScore.CurHP;

					pTargetHuman->UpdateScore(0);

					auto pPartyList = m_pPartyList;
					if (pPartyList)
					{
						for (int i = 0; i < pPartyList->m_nNumItem; i++)
						{
							auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
							if (pPartyItem->m_dwCharID == pTargetHuman->m_dwID)
							{
								pPartyItem->m_pHpProgress->SetMaxProgress(pTargetHuman->m_stScore.MaxHP);
								pPartyItem->m_pHpProgress->SetCurrentProgress(pTargetHuman->m_stScore.CurHP);
								break;
							}
						}
					}
				}
				if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104 && pAttack->SkillIndex != 29)
				{
					if (g_pSpell[pAttack->SkillIndex].InstanceType == 8)
						return 1;
				}

				if (pAttacker && pAttacker->m_nSkinMeshType == 11)
					nClass = 4;

				static const unsigned int dwDelayTable[36] =
				{
					0, 0, 0, 0, 0, 0, 0, 0, 0, 1000, 1000, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 700, 700, 700,
					1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000
				};

				if ((unsigned char)pAttack->Motion > 9)
					pAttack->Motion = 9;

				int nMotion = (unsigned char)pAttack->Motion - 4;
				if (pAttack->SkillIndex == 151)
					nClass = 5;
				if (pAttacker != m_pMyHuman || !pAttack->FlagLocal && pAttacker == m_pMyHuman)
				{
					pTargetHuman->m_stPunchEvent.nDamage = pAttack->Dam[i].Damage / nDamageRate;
					pTargetHuman->m_stPunchEvent.vecFrom = vecAttackerPos;
					pTargetHuman->m_stPunchEvent.SkillIndex = pAttack->SkillIndex;
					pTargetHuman->m_stPunchEvent.dwTime = 0 + dwDelayTable[6 * nClass + nMotion] + g_pTimerManager->GetServerTime();
				}
			}
		}
	}
	else
	{
		float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY)) * 0.1f;
		vecTarget = TMVector3((float)pAttack->TargetX, fY, (float)pAttack->TargetY);

		if (pAttacker != m_pMyHuman || !pAttack->FlagLocal && pAttacker == m_pMyHuman)
		{
			for (int i = 0; i < targetCount; i++)
			{
				if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
					break;

				if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
					break;

				auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
				if (pTargetHuman)
				{
					int nDamageRate = pTargetHuman->m_cDamageRate;
					if (nDamageRate == 0)
						nDamageRate = 1;

					if (pAttack->Dam[i].Damage == -3 || pAttack->Dam[i].Damage == -4)
					{
						int pX = 0;
						int pY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &pX, &pY))
						{
							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								char szStr[128]{};
								sprintf(szStr, "miss");
								auto pFont = new TMFont3(szStr, pX, pY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFFFFFF, 2.0f, dwDelay, 1, 1500, 0, 4);
								m_pExtraContainer->AddChild(pFont);
							}
						}
					}
					else if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104
						&& g_pSpell[pAttack->SkillIndex].InstanceType == 6
						&& (pAttack->Dam[i].Damage >= 0 || pAttack->Dam[i].Damage <= -6))
					{
						if (!pAttack->FlagLocal)
						{
							attack_target::SubtractPool(pTargetHuman->m_stScore.CurHP,
								pAttack->Dam[i].Damage, nDamageRate);
							if (pTargetHuman == m_pMyHuman)
							{
								if (m_nReqHP - pAttack->Dam[i].Damage <= 0)
									m_nReqHP = 0;
								else
									m_nReqHP -= pAttack->Dam[i].Damage / nDamageRate;

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}

						int nStartX = 0;
						int nStartY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nStartX, &nStartY))
						{
							char szStr[128]{};
							if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
								(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
								&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
							{
								sprintf(szStr, "?");
							}
							else
								sprintf(szStr, "+ %d", -pAttack->Dam[i].Damage / ((pAttack->DoubleCritical & 1) + 1));

							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								auto pFont = new TMFont3(szStr, nStartX, nStartY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFF5555FF, 2.0f, dwDelay, 1, 1500, 0, 2);
								m_pExtraContainer->AddChild(pFont);
							}
						}
						if (pAttack->AttackerID == m_pMyHuman->m_dwID)
							SetMyHumanExp(pAttack->CurrentExp, pAttack->FakeExp);
					}
					else
					{
						if (!pAttack->FlagLocal)
						{
							if (!pTargetHuman->m_MaxBigHp)
								attack_target::SubtractPool(pTargetHuman->m_stScore.CurHP,
									pAttack->Dam[i].Damage, nDamageRate);
							else
								attack_target::SubtractBigPool(pTargetHuman->m_stScore.CurHP,
									pTargetHuman->m_BigHp, pAttack->Dam[i].Damage);
							if (pTargetHuman == m_pMyHuman)
							{
								if (m_nReqHP - pAttack->Dam[i].Damage <= 0)
									m_nReqHP = 0;
								else
									m_nReqHP -= pAttack->Dam[i].Damage / nDamageRate;

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}

						pTargetHuman->m_wAttackerID = pAttack->AttackerID;

						int bInScreen = 0;
						int nTX = 0;
						int nTY = 0;
						if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pTargetHuman && pAttacker)
							bInScreen = BASE_Get3DTo2DPos(
								(float)(pTargetHuman->m_vecPosition.x * 0.5f) + (float)(pAttacker->m_vecPosition.x * 0.5f),
								pTargetHuman->m_fHeight - 1.0f,
								(float)(pTargetHuman->m_vecPosition.y * 0.5f) + (float)(pAttacker->m_vecPosition.y * 0.5f),
								&nTX,
								&nTY);
						else
							bInScreen = BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nTX, &nTY);

						if (bInScreen)
						{
							if (!pTargetHuman || !pAttacker)
								return 1;

							for (int bViewHalf = 0; bViewHalf < (pAttack->DoubleCritical & 1) + 1; bViewHalf++)
							{
								// Preserve legacy HP arithmetic while rendering the authoritative
								// wide damage when the server supplied a validated DMGX tail.
								int nValue = GetWYD748AttackVisualDamage(pAttack, i) / ((pAttack->DoubleCritical & 1) + 1);

								if (nValue > 0)
								{
									char szStr[128]{};
									if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
										((int)pTargetHuman->m_vecPosition.x >> 7 != 18 || (int)pTargetHuman->m_vecPosition.y >> 7 != 30) &&
										(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
										&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
									{
										sprintf(szStr, "?");
									}
									else
									{
										sprintf(szStr, "%d", nValue);
									}

									if (pAttack->SkillIndex == 79)
										dwDelay += 100 * i;

									bool bDrawFront = false;
									if (g_bHideEffect)
									{
										if (pAttacker == m_pMyHuman)
											bDrawFront = true;
										if (pTargetHuman == m_pMyHuman)
											bDrawFront = true;
									}
									else
									{
										bDrawFront = true;
									}

									TMFont3* pFont = nullptr;
									unsigned int dwColor = 0xFFFFFFFF;
									float fSize = 1.0f;
									if (bDrawFront)
									{
										if (!(pAttack->DoubleCritical & 2))
										{
											if (pTargetHuman == m_pMyHuman)
											{
												if (bInScreen)
												{
													pFont = new TMFont3(szStr, nTX - 10 * i,
														(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														(int)(((float)nTY - (float)(20.0f * i)) * RenderDevice::m_fHeightRatio),
														dwColor,
														fSize,
														dwDelay,
														1,
														1200,
														0,
														4);
												}
											}
											else if (bInScreen)
											{
												if (pAttack->SkillIndex < 0 || pAttack->SkillIndex > 150) // physical attack
												{
													pFont = new TMFont3(szStr, nTX - 10 * i,
														(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														(int)(((float)nTY - (float)(20.0f * i)) * RenderDevice::m_fHeightRatio),
														dwColor,
														fSize,
														dwDelay,
														1,
														1200,
														0,
														3);
												}
												else
												{
													pFont = new TMFont3(szStr, nTX - 10 * i,
														(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														(int)(((float)nTY - (float)(20.0f * i)) * RenderDevice::m_fHeightRatio),
														dwColor,
														fSize,
														dwDelay,
														1,
														1200,
														0,
														7);
												}
											}
										}
										else if (bInScreen)
										{
											if (pAttack->SkillIndex < 0 || pAttack->SkillIndex > 150) // physical attack
											{
												pFont = new TMFont3(szStr, nTX - 10 * i,
													(int)(RenderDevice::m_fHeightRatio * 80.0f) +
													(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) - ((float)(40 * i) * RenderDevice::m_fHeightRatio) +
														(float)(RenderDevice::m_fHeightRatio * 80.0f)),
													dwColor,
													fSize,
													dwDelay,
													1,
													1200,
													0,
													5);
											}
											else
											{
												pFont = new TMFont3(szStr, nTX - 10 * i,
													(int)(RenderDevice::m_fHeightRatio * 80.0f) +
													(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) - ((float)(40 * i) * RenderDevice::m_fHeightRatio) +
														(float)(RenderDevice::m_fHeightRatio * 80.0f)),
													dwColor,
													fSize,
													dwDelay,
													1,
													1200,
													0,
													8); //test
											}
										}

										if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pAttacker != m_pMyHuman)
										{
											if (pFont)
											{
												pFont->m_fScale = 0.5f;
												if (pFont->m_nType == 5)
													pFont->m_nType = 6;
											}
										}

										m_pExtraContainer->AddChild(pFont);
									}

									if (pTargetHuman == m_pMyHuman && pAttacker)
										sprintf(m_szLastAttackerName, "%s", pAttacker->m_szName);

									if (pTargetHuman->m_cCriticalArmor == 1)
									{
										auto pEffectMesh = new TMEffectMesh(2838, 0xFF999999, pTargetHuman->m_fAngle, 4);

										pEffectMesh->m_nTextureIndex = 413;
										pEffectMesh->m_dwLifeTime = 500;
										pEffectMesh->m_dwCycleTime = 500;

										if (pTargetHuman->m_cMount == 1)
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecSkinPos.x,
												(((TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_vecSkinPos.y)
												- 0.30000001f,
												pTargetHuman->m_vecSkinPos.z);
										}
										else
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecPosition.x,
												(float)((float)((float)(TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_fHeight)
												+ 0.30000001f,
												pTargetHuman->m_vecPosition.y);
										}

										pEffectMesh->m_fScaleH = 2.5f;
										pEffectMesh->m_fScaleV = 2.5f;
										pEffectMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
										pEffectMesh->m_cShine = 0;

										m_pEffectContainer->AddChild(pEffectMesh);
									}
								}
							}
						}
					}
					if (pTargetHuman->m_stScore.CurHP < 0 && !pAttack->FlagLocal)
					{
						pTargetHuman->m_stScore.CurHP = 0;
						if (pTargetHuman == m_pMyHuman)
							memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));

						m_nReqHP = 0;
					}
					if (pTargetHuman->m_stScore.CurHP > pTargetHuman->m_stScore.MaxHP && !pAttack->FlagLocal)
					{
						pTargetHuman->m_stScore.CurHP = pTargetHuman->m_stScore.MaxHP;
						if (pTargetHuman == m_pMyHuman)
							memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
					}
					if (pTargetHuman == m_pMyHuman && !pAttack->FlagLocal)
					{
						g_pObjectManager->m_stMobData.CurrentScore.CurHP = pTargetHuman->m_stScore.CurHP;
						if (pTargetHuman == m_pMyHuman)
							memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
					}
					pTargetHuman->UpdateScore(0);
				}
			}
		}
		if (pAttacker != m_pMyHuman || (pAttack->FlagLocal == 1 && pAttacker == m_pMyHuman))
		{
			if (pAttack->SkillIndex == 0)
			{
				auto pHeavensDust = new TMSkillHeavensDust(vecTarget, 0);

				m_pEffectContainer->AddChild(pHeavensDust);
			}
			else if (pAttack->SkillIndex == 1)
			{
				TMVector3 vecPos{vecStart.x, vecStart.y - 0.5f, vecStart.z};

				auto pHollyTouch = new TMSkillHolyTouch(vecPos, 0);

				m_pEffectContainer->AddChild(pHollyTouch);
			}
			else if (pAttack->SkillIndex == 26)
			{
				TMVector3 vecPos{ vecStart.x, vecStart.y - 0.5f, vecStart.z };

				auto pFlash = new TMSkillFlash(vecPos, 0);

				m_pEffectContainer->AddChild(pFlash);

				if (!pAttacker)
					return 1;
				if (m_pMyHuman->IsInTown() == 1)
					return 1;
				if (pAttacker->m_usGuild && m_pMyHuman->m_usGuild && (m_pMyHuman->m_usGuild == pAttacker->m_usGuild || g_pObjectManager->m_usAllyGuild == pAttacker->m_usGuild))
					return 1;

				bool bFound = false;
				for (int i = 0; i < targetCount; i++)
				{
					if (pAttack->Dam[i].TargetID == m_pMyHuman->m_dwID)
					{
						bFound = true;
						break;
					}
				}

				if (bFound && pAttacker != m_pMyHuman && !pAttacker->m_bParty)
				{
					float fFlashTerm = 0.0f;
					if (m_pMyHuman->m_stScore.Level <= 0)
						fFlashTerm = (((float)m_pMyHuman->m_stScore.Level * 4000.0f)
							* (float)pAttacker->m_stScore.Mastery[1])
						/ 100.0f;
					else
						fFlashTerm = (((float)(pAttacker->m_stScore.Level / m_pMyHuman->m_stScore.Level)
							* 4000.0f)
							* (float)pAttacker->m_stScore.Mastery[1])
						/ 100.0f;
					if (fFlashTerm < 2000.0)
						fFlashTerm = 2000.0;
					if (fFlashTerm > 4000.0)
						fFlashTerm = 4000.0;

					m_fFlashTerm = fFlashTerm;
					m_dwStartFlashTime = g_pTimerManager->GetServerTime();
				}
			}
			else if (pAttack->SkillIndex == 7)
			{
				for (int i = 0; i < targetCount; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pTargetHuman)
					{
						float fTarget = 0.0f;
						if (pAttacker)
							fTarget = pAttacker->m_fHeight;

						vecStart = TMVector3(pTargetHuman->m_vecPosition.x,
							fTarget + 0.5f,
							pTargetHuman->m_vecPosition.y);

						vecTarget = TMVector3(vecStart.x + 3.0f, vecStart.y + 5.0f, vecStart.z - 3.0f);

						auto pArrow = new TMArrow(vecTarget, vecStart, 0, 10001, 0, 0, 0);

						m_pEffectContainer->AddChild(pArrow);
					}
				}
			}
			else if (pAttack->SkillIndex == 35)
			{
				vecTarget.y += 1.0f;

				auto pMeteor = new TMSkillMeteorStorm(TMVector3(0.0f, 0.0f, 0.0f), vecTarget, 0, nullptr);

				m_pEffectContainer->AddChild(pMeteor);
			}
			else if (pAttack->SkillIndex == 39 && !bomb)
			{
				vecTarget.y += 1.0f;

				TMVector3 vecSt{};

				for (int i = 0; i < 4; i++)
				{
					auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3((float)(vecTarget.x - 1.8f) + ((float)(i % 2) * 3.5999999f),
						vecTarget.y,
						(float)(vecTarget.z - 1.8f) + ((float)(i / 2) * 3.5999999f)), 4, nullptr);

					pMeteor->m_dwStartTime += 200 * i;
					m_pEffectContainer->AddChild(pMeteor);
				}

				auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3(vecTarget.x, vecTarget.y, vecTarget.z), 4, nullptr);

				pMeteor->m_dwStartTime += 270;
				m_pEffectContainer->AddChild(pMeteor);
			}
			else if (pAttack->SkillIndex == 39 && bomb)
			{
				vecTarget.y += 2.0f;

				TMVector3 vecSt{};

				for (int i = 0; i < 4; i++)
				{
					auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3((float)(vecTarget.x - 0.89999998f) + ((float)(i % 2) * 1.8f),
						vecTarget.y,
						(float)(vecTarget.z - 0.89999998f) + ((float)(i / 2) * 1.8f)), 4, nullptr);

					m_pEffectContainer->AddChild(pMeteor);
				}

				vecTarget.y += 0.30000001f;

				auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3(vecTarget.x, vecTarget.y, vecTarget.z), 10, nullptr);
				m_pEffectContainer->AddChild(pMeteor);
			}
			else if (pAttack->SkillIndex == 97)
			{
				vecTarget.y += 1.0f;
				for (int i = 0; i < 100; i++)
				{
					auto pItem = (TMCannon*)g_pObjectManager->GetItemByID(i + 15001);
					if (pItem)
					{
						if (pItem->m_stItem.sIndex == 746)
						{
							if (vecStart.x == pItem->m_vecBasePosition.x && vecStart.z == pItem->m_vecBasePosition.y)
							{
								pItem->m_cFire = 1;
								vecStart.x = (float)(pItem->m_fCosF * pItem->m_fCannonLen) + vecStart.x;
								vecStart.z = vecStart.z - (float)(pItem->m_fSinF * pItem->m_fCannonLen);
								break;
							}
						}
					}
				}

				float fx = vecTarget.x - m_pMyHuman->m_vecPosition.x;
				float fy = vecTarget.z - m_pMyHuman->m_vecPosition.y;

				auto pMeteor = new TMSkillMeteorStorm(vecStart, vecTarget, 2, 0);

				pMeteor->m_fDestLength = sqrt((float)(fx * fx) + (float)(fy * fy));

				m_pEffectContainer->AddChild(pMeteor);
				return 1;
			}
		}
	}

	if (!pAttack->FlagLocal)
	{
		if (pAttacker == m_pMyHuman)
		{
			m_nReqMP = pAttack->ReqMp;
			if (m_nReqHP < static_cast<unsigned short>(m_pMyHuman->m_stScore.CurHP))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.CurHP);
			if (m_nReqMP < static_cast<unsigned int>(m_pMyHuman->m_stScore.CurMP))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.CurMP);
			if (m_nReqHP > static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHP))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHP);
			if (m_nReqMP > static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMP))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMP);
			UpdateScoreUI(0);
		}
		else if (pTarget == m_pMyHuman)
		{
			if (m_nReqHP < static_cast<unsigned short>(m_pMyHuman->m_stScore.CurHP))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.CurHP);
			if (m_nReqMP < static_cast<unsigned int>(m_pMyHuman->m_stScore.CurMP))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.CurMP);
			if (m_nReqHP > static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHP))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHP);
			if (m_nReqMP > static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMP))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMP);
			UpdateScoreUI(16);
		}
	}

	return 1;
}

int TMFieldScene::OnPacketNuke(MSG_STANDARD* pStd)
{
	// just that
	return 1;
}
