#include "pch.h"
#include "TMFieldScene.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMUtil.h"
#include "WYD748Assets.h"
#include "features/macro/MacroMsg.h"
#include "../../application/CCModePolicy.h"
#include "../../application/FieldInteractionPolicy.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"

void TMFieldScene::InitializeCompatCCControls()
{
	if (!m_pControlContainer || m_pccmode)
		return;

	// Recreate only the CC subtree from 7.59 FieldScene2, after ABI detection.
	// SControl scales logical coordinates once; the container owns every child.
	// Compact layout: about half the previous area, without scaling the font.
	m_pccmode = new SPanel(164, 0.0f, 0.0f, 180.0f, 64.0f,
		0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
	m_pccmode->SetControlID(T_CCMODE_DLG);
	m_pControlContainer->AddItem(m_pccmode);

	auto addText = [this](unsigned int id, const char* text, float x, float y, float width)
	{
		auto label = new SText(-1, text, 0xFFFFFFFF, x, y, width, 16.0f,
			0, 0xFF333333, SText::TEXT_TYPE_SHADOW, SText::TEXT_ALIGN_CENTER);
		label->SetControlID(id);
		label->m_bSelectEnable = 0;
		m_pccmode->AddChild(label);
		return label;
	};
	addText(0, "Combat Control", 4.0f, 2.0f, 150.0f);
	m_pCCModeHpSte = addText(T_CCMODE_HPSTE, "", 46.0f, 46.0f, 44.0f);
	m_pCCModeMountSte = addText(T_CCMODE_MOUNTSTE, "", 90.0f, 46.0f, 44.0f);
	addText(T_CCMODE_COMPAT_MODE, "", 2.0f, 46.0f, 44.0f);
	addText(T_CCMODE_COMPAT_MOVE, "", 134.0f, 46.0f, 44.0f);

	auto addButton = [this](unsigned int id, int texture, float x, const char* tooltip)
	{
		char caption[128]{};
		strcpy_s(caption, tooltip);
		auto button = new SButton(texture, x, 20.0f, 26.0f, 26.0f,
			0xFFFFFFFF, 1, caption);
		button->SetControlID(id);
		button->SetEventListener(m_pControlContainer);
		m_pccmode->AddChild(button);
		return button;
	};
	m_pMGameAutoBtn = addButton(B_CCMODE_DLG_MODE, 550, 11.0f, "Combat");
	m_pCCPotionBtn = addButton(B_CCMODE_DLG_HP, 554, 55.0f, "HP/MP potion: click to adjust the percentage");
	m_pCCFeedBtn = addButton(B_CCMODE_DLG_MOUNT, 555, 99.0f, "Mount feed: click to adjust or turn off");
	m_pSetType = addButton(P_CCMODE_DLG_PONT, 556, 143.0f, "Movement");
	char closeText[] = "X";
	auto close = new SButton(-2, 160.0f, 2.0f, 16.0f, 16.0f, 0xFFFFFFFF, 1, closeText);
	close->SetControlID(B_CCMODE_COMPAT_CLOSE);
	close->SetEventListener(m_pControlContainer);
	m_pccmode->AddChild(close);

	char caption[] = "C.C. - configure automatic combat";
	m_pCC_Btn = new SButton(559, 0.0f, 0.0f, 30.0f, 30.0f, 0xFFFFFFFF, 1, caption);
	m_pCC_Btn->SetControlID(B_CCMODE_SYSTEM);
	m_pCC_Btn->SetEventListener(m_pControlContainer);
	m_pControlContainer->AddItem(m_pCC_Btn);

	// The native EXP segments are children of the bottom HUD, so their stored
	// positions are relative to that hierarchy. Resolve their rendered bounds
	// and anchor C.C. immediately above the actual EXP strip instead of using a
	// resolution-dependent screen-height offset.
	for (int index = 0; index < 10; ++index)
	{
		if (!m_pExpProgress[index])
			m_pExpProgress[index] = static_cast<SProgressBar*>(
				m_pControlContainer->FindControl(P_EXP_PROGRESS1 + index));
	}

	auto getRootPos = [this](SControl* control)
	{
		TMVector2 pos(control->m_nPosX, control->m_nPosY);
		for (TreeNode* parent = control->m_pTop;
			parent && parent != m_pControlContainer;
			parent = parent->m_pTop)
		{
			// Resource-control parents between a leaf and SControlContainer are
			// SControl nodes; FrameMove2 adds these same offsets while rendering.
			auto parentControl = static_cast<SControl*>(parent);
			pos.x += parentControl->m_nPosX;
			pos.y += parentControl->m_nPosY;
		}
		return pos;
	};

	bool hasExpBounds = false;
	float expLeft = 0.0f;
	float expRight = 0.0f;
	float expTop = 0.0f;
	for (auto progress : m_pExpProgress)
	{
		if (!progress)
			continue;

		const TMVector2 pos = getRootPos(progress);
		const float right = pos.x + progress->m_nWidth;
		if (!hasExpBounds)
		{
			expLeft = pos.x;
			expRight = right;
			expTop = pos.y;
			hasExpBounds = true;
		}
		else
		{
			if (pos.x < expLeft) expLeft = pos.x;
			if (right > expRight) expRight = right;
			if (pos.y < expTop) expTop = pos.y;
		}
	}

	if (hasExpBounds)
	{
		const float gap = 3.0f * RenderDevice::m_fHeightRatio;
		m_pCC_Btn->SetPos(
			((expLeft + expRight) - m_pCC_Btn->m_nWidth) * 0.5f,
			expTop - m_pCC_Btn->m_nHeight - gap);
	}
	else
	{
		// Defensive fallback for malformed/custom resources lacking EXP controls.
		m_pCC_Btn->SetPos((g_pDevice->m_dwScreenWidth - m_pCC_Btn->m_nWidth) * 0.5f,
			g_pDevice->m_dwScreenHeight - 96.0f * RenderDevice::m_fHeightRatio);
	}
	m_pccmode->SetVisible(0);
	NewCCMode(false, true);
}

void TMFieldScene::FindAuto()
{
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (dwServerTime - LastSendTime > 250000)
	{
		MSG_STANDARD stStandard{};
		stStandard.ID = g_pObjectManager->m_dwCharID;
		stStandard.Type = MSG_Ping_Opcode;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stStandard)->Type, reinterpret_cast<char*>(&stStandard), sizeof(stStandard)});
	}

	// TODO: china stuffs here, not needed for now
}

int TMFieldScene::FindProcess(unsigned int processID)
{
	return 0;
}

void TMFieldScene::SetAutoOption(int nIndex, char* szString)
{
}

void TMFieldScene::SetAutoSkillNum(int nCount)
{
	if (nCount < 1)
		nCount = 1;
	else if (nCount > 10)
		nCount = 10;

	m_nAutoSkillNum = nCount;

	if (m_bCompatFieldScene && m_pAutoSkillPanel && m_pGridSkillBelt2 &&
		m_pGridSkillBelt2->m_nColumnGridCount > 0)
	{
		const float slotWidth = m_pGridSkillBelt2->m_nWidth /
			static_cast<float>(m_pGridSkillBelt2->m_nColumnGridCount);
		const float width = slotWidth * static_cast<float>(m_nAutoSkillNum);
		// Controls 575 and 573/586 share parent 5745 and scaled local coordinates.
		// The serialized bar is wider than the belt; anchor to the actual slots,
		// not that placeholder width (which extends beyond the C.C button).
		const float rightEdge = m_pGridSkillBelt2->m_nPosX + m_pGridSkillBelt2->m_nWidth;
		m_pAutoSkillPanel->SetPos(rightEdge - width, m_pAutoSkillPanel->m_nPosY);
		m_pAutoSkillPanel->SetSize(width, 4.0f * RenderDevice::m_fHeightRatio);
	}

	if (m_pAutoSkillPanel)
	{
		for (int i = 1; i <= 10; ++i)
		{
			auto pSkill = m_pAutoSkillPanelChild[i - 1];
			if (pSkill)
				pSkill->SetVisible(i <= nCount);
		}
	}
}

void TMFieldScene::SetAutoTarget()
{
	m_cAutoAttack = m_cAutoAttack == 0;

	auto pBtnAuto = static_cast<SButton*>(m_pControlContainer->FindControl(312u));

	if (m_cAutoAttack == 0)
		m_pTargetHuman = nullptr;

	if (pBtnAuto)
		pBtnAuto->SetSelected(m_cAutoAttack);

	if (m_pAutoSkillPanel)
		m_pAutoSkillPanel->SetVisible(m_cAutoAttack);
}

int TMFieldScene::OnPacketMacroWater(stWaterScrollMacro* pStd)
{
	MacroMsg::instance().onEvent(pStd->PosX, pStd->PosY, pStd->Parm);

	return 1;
}

void TMFieldScene::GameAuto()
{
	if (!g_GameAuto)
		return;

	if (!g_pCurrentScene || g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
	{
		g_GameAuto = 0;
		return;
	}

	// CC settings survive scene reconstruction; world dependencies may not yet
	// exist on the first frame (or may already be gone during teardown).
	if (g_pCurrentScene != this || !m_pMyHuman || !m_pHumanContainer || !m_pGround ||
		!g_pObjectManager || !g_pTimerManager || !g_pEventTranslator)
	{
		m_pAutoTarget = nullptr;
		return;
	}

	if (m_pMyHuman->m_cHide == 1)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwAttackDelay)
	{
		if (dwServerTime - m_dwAttackDelay > 2000)
			m_dwAttackDelay = 0;
		m_pAutoTarget = nullptr;
		return;
	}
	if (m_dwLastLogout || m_dwLastTown || m_dwLastTeleport || m_pMyHuman->m_dwDelayDel)
	{
		m_dwAttackDelay = dwServerTime;
		m_pAutoTarget = nullptr;
		return;
	}

	int CharHp = g_pObjectManager->m_stMobData.CurrentScore.CurHP;
	if (m_pMyHuman->m_cDie)
	{
		m_pAutoTarget = nullptr;
		return;
	}

	int nSX = (int)m_pMyHuman->m_vecPosition.x;
	int nSY = (int)m_pMyHuman->m_vecPosition.y;
	int CharMaxHp = g_pObjectManager->m_stMobData.CurrentScore.MaxHP;
	int CharMp = g_pObjectManager->m_stMobData.CurrentScore.CurMP;
	int CharMaxMp = g_pObjectManager->m_stMobData.CurrentScore.MaxMP;
	int nMountHP = BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[14], 80);
	int nMountFeed = BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[14], 82);
	int sIndex = g_pObjectManager->m_stMobData.Equip[14].sIndex - 2045;
	int _nEquipIdx = g_pObjectManager->m_stMobData.Equip[14].sIndex;
	if (_nEquipIdx >= 2387 && _nEquipIdx <= 2388)
		sIndex = 336;
	else if (_nEquipIdx >= 3980 && _nEquipIdx <= 3982)
		sIndex = _nEquipIdx - 3638;
	else if (_nEquipIdx >= 3983 && _nEquipIdx <= 3985)
		sIndex = _nEquipIdx - 3641;
	else if (_nEquipIdx >= 3986 && _nEquipIdx <= 3988)
		sIndex = _nEquipIdx - 3644;

	int nMountMaxHPIndex = sIndex - 315;
	if (sIndex - 315 < 0)
		nMountMaxHPIndex = 0;

	switch (g_pObjectManager->m_stMobData.Equip[14].sIndex - 2378)
	{
	case 0:
		nMountMaxHPIndex = 18;
		break;
	case 1:
		nMountMaxHPIndex = 19;
		break;
	case 2:
		nMountMaxHPIndex = 21;
		break;
	case 3:
		nMountMaxHPIndex = 20;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		break;
	case 9:
		nMountMaxHPIndex = 20;
		break;
	case 10:
		nMountMaxHPIndex = 21;
		break;
	case 11:
		nMountMaxHPIndex = 22;
		break;
	}

	int nMountMaxHp = nMountMaxHPIndex >= 0 && nMountMaxHPIndex < _countof(g_nMountHPTable)
		? g_nMountHPTable[nMountMaxHPIndex] : 0;
	int CheckMountHp = 0;

	if (g_GameAuto_mountValue)
		CheckMountHp = g_GameAuto_mountValue * nMountMaxHp / 100;
	else
		CheckMountHp = 0;

	CharMaxHp = g_GameAuto_hpValue * CharMaxHp / 100;
	CharMaxMp = g_GameAuto_hpValue * CharMaxMp / 100;

	if (!(dwServerTime % 3))
	{
		if (m_bCompatFieldScene && cc_mode::ShouldFeed(nMountHP, nMountMaxHp, nMountFeed, g_GameAuto_mountValue) && FeedMount())
			return;

		if (!m_bCompatFieldScene && nMountHP > 0 && nMountHP < CheckMountHp && FeedMount())
			return;

		if (CharHp < CharMaxHp && UseHPotion())
			return;

		if (m_AutoHpMp != 3 && CharMp < CharMaxMp && UseMPotion())
			return;

		if (!m_bCompatFieldScene && nMountFeed > 0 && nMountFeed < 6 && FeedMount())
			return;
	}

	if ((int)m_pMyHuman->m_eMotion > 1)
		return;

	if (m_bSkillBeltSwitch && m_bSkillBeltSwitch != 1)
		m_bSkillBeltSwitch = 0;

	for (int i = 10 * m_bSkillBeltSwitch; i < 10 * m_bSkillBeltSwitch + 10; ++i)
	{
		int idxSkill = (char)g_pObjectManager->m_cShortSkill[i];
		if (idxSkill == -1)
			continue;

		if (idxSkill == 3 || idxSkill == 5 || idxSkill == 53 || idxSkill == 54 || idxSkill == 9 || idxSkill == 11 || idxSkill == 37 ||
			idxSkill == 41 || idxSkill == 43 || idxSkill == 44 || idxSkill == 45 || idxSkill == 46 || idxSkill == 64 || idxSkill == 66 ||
			idxSkill == 68 || idxSkill == 70 || idxSkill == 71 || idxSkill == 87 || idxSkill == 75 || idxSkill == 76 || idxSkill == 77 ||
			idxSkill == 81 || idxSkill == 85 || idxSkill == 89 || idxSkill == 92)
		{
			int useSkill = 1;
			for (int j = 0; j < 32; ++j)
			{
				if ((unsigned char)m_pMyHuman->m_stAffect[j].Type <= 0)
					continue;

				if (idxSkill == 64 || idxSkill == 66 || idxSkill == 68 || idxSkill == 70 || idxSkill == 71)
				{
					if (g_AffectSkillType[(unsigned char)m_pMyHuman->m_stAffect[j].Type] == 71 && m_pMyHuman->m_stAffect[j].Time > 3)
					{
						useSkill = 0;
						break;
					}
				}
				else if (g_AffectSkillType[(unsigned char)m_pMyHuman->m_stAffect[j].Type] == idxSkill && m_pMyHuman->m_stAffect[j].Time > 3)
				{
					useSkill = 0;
					break;
				}
			}
			if (useSkill)
			{
				int DelayTime = 0;
				if (idxSkill == 3 || idxSkill == 5 || idxSkill == 53 || idxSkill == 54 || idxSkill == 74 || idxSkill == 76 || idxSkill == 77)
					DelayTime = 100 * g_pSpell[idxSkill].AffectTime * g_pObjectManager->m_stMobData.CurrentScore.Mastery[1];
				else if (idxSkill == 9 || idxSkill == 11 || idxSkill == 13 || idxSkill == 15 || idxSkill == 37 || idxSkill == 86 || idxSkill == 87)
					DelayTime = 100 * g_pSpell[idxSkill].AffectTime * g_pObjectManager->m_stMobData.CurrentScore.Mastery[2];
				else if (idxSkill == 41 || idxSkill == 43 || idxSkill == 44 || idxSkill == 45 || idxSkill == 46 || idxSkill == 64 || idxSkill == 66 ||
					idxSkill == 68 || idxSkill == 70 || idxSkill == 71 || idxSkill == 89 || idxSkill == 90)
				{
					DelayTime = 100 * g_pSpell[idxSkill].AffectTime * g_pObjectManager->m_stMobData.CurrentScore.Mastery[3];
				}

				if (DelayTime + m_dwSkillLastTime[idxSkill] <= dwServerTime)
				{
					const char selectedSkill = g_pObjectManager->m_cSelectShortSkill;
					g_pObjectManager->m_cSelectShortSkill = i;
					const int used = SkillUse(nSX, nSY, GroundGetPickPos(), dwServerTime, 1, 0);
					g_pObjectManager->m_cSelectShortSkill = selectedSkill;
					if (used == 1)
						return;
				}
			}
			continue;
		}
		if ((idxSkill == 56 || idxSkill == 57 || idxSkill == 58 || idxSkill == 59 || idxSkill == 60 || idxSkill == 61 || idxSkill == 62 || idxSkill == 63) &&
			m_dwSkillLastTime[idxSkill] + 80000 <= dwServerTime)
		{
			const char selectedSkill = g_pObjectManager->m_cSelectShortSkill;
			g_pObjectManager->m_cSelectShortSkill = i;
			SkillUse(nSX, nSY, GroundGetPickPos(), dwServerTime, 1, 0);
			g_pObjectManager->m_cSelectShortSkill = selectedSkill;
			return;
		}
	}

	if (g_GameAuto == 3)
		return;

	if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 &&
		(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
		return;

	if (m_AutoPostionUse == 1 && !m_pAutoTarget)
	{
		if ((nSX != m_AutoStartPointX || nSY != m_AutoStartPointY) && BASE_GetDistance(nSX, nSY, m_AutoStartPointX, m_AutoStartPointY) <= 10)
		{
			if (dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000 && !m_pMyHuman->m_cDie)
			{
				m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

				MSG_Action stAction{};
				stAction.Header.ID = m_pMyHuman->m_dwID;
				stAction.PosX = m_pMyHuman->m_LastSendTargetPos.x;
				stAction.PosY = m_pMyHuman->m_LastSendTargetPos.y;
				stAction.Effect = 0;
				stAction.Header.Type = MSG_Action_Opcode;
				stAction.Speed = g_nMyHumanSpeed;
				stAction.TargetX = m_AutoStartPointX;
				stAction.TargetY = m_AutoStartPointY;

				for (int k = 0; k < 23; ++k)
					stAction.Route[k] = 0;

				g_bLastStop = stAction.Header.Type;
				m_stMoveStop.LastX = stAction.PosX;
				m_stMoveStop.LastY = stAction.PosY;
				m_stMoveStop.NextX = stAction.TargetX;
				m_stMoveStop.NextY = stAction.TargetY;
				SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAction)->Type, reinterpret_cast<char*>(&stAction), sizeof(stAction)});
				m_pMyHuman->OnPacketEvent(MSG_Action_Opcode, (char*)&stAction);
				m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
				return;
			}
		}
	}

	if (m_pMyHuman->IsInTown() == 1)
		return;

	if (g_GameAuto == 1)
	{
		if (!m_pAutoTarget)
		{
			m_pAutoTarget = nullptr;

			auto pTarget = (TMHuman*)m_pHumanContainer->m_pDown;
			while (pTarget && pTarget->m_pNextLink)
			{
				if (pTarget == m_pMyHuman)
					pTarget = (TMHuman*)pTarget->m_pNextLink;
				else
				{
					if (m_pMyHuman->MAutoAttack(pTarget, 0) == 1)
					{
						m_pAutoTarget = pTarget;
						return;
					}

					pTarget = (TMHuman*)pTarget->m_pNextLink;
				}
			}

			if (m_pAutoTarget || m_AutoPostionUse == 2)
				return;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			while (pNode && pNode->m_pNextLink)
			{
				if (pNode == m_pMyHuman)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int nTX = (int)pNode->m_vecPosition.x;
				int nTY = (int)pNode->m_vecPosition.y;

				if (BASE_GetDistance(nSX, nSY, nTX, nTY) > 8)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int nSpecForce = 0;
				if (g_pObjectManager->m_stMobData.LearnedSkill[0] & 0x20000000)
					nSpecForce = 1;

				int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);
				BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)g_pCurrentScene->m_HeightMapData, 8);

				if (nTX != (int)pNode->m_vecPosition.x || nTY != (int)pNode->m_vecPosition.y)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int attack = 0;
				if (pNode)
					attack = m_pMyHuman->MAutoAttack(pNode, 1);
				if (attack == 1)
				{
					m_pAutoTarget = pNode;
					return;
				}
				if (attack == 2)
					return;

				pNode = (TMHuman*)pNode->m_pNextLink;
			}

			return;
		}

		int rnt = 0;
		auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
		while (pNode && pNode->m_pNextLink)
		{
			if (pNode == m_pMyHuman)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			if (pNode == m_pAutoTarget)
			{
				rnt = m_pMyHuman->MAutoAttack(m_pAutoTarget, m_AutoPostionUse == 2 ? 0 : 1);
				if (m_pAutoTarget->m_cShadow == 1 && m_pAutoTarget->m_nClass == 66)
					m_pAutoTarget = nullptr;
			}

			pNode = (TMHuman*)pNode->m_pNextLink;
			if (!rnt)
				m_pAutoTarget = nullptr;
		}

		return;
	}

	if (g_GameAuto == 2)
	{
		if (m_pMyHuman->m_bSkillBlack == 1)
			return;

		if (!m_pAutoTarget)
		{
			m_pAutoTarget = nullptr;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			while (pNode && pNode->m_pNextLink)
			{
				if (pNode == m_pMyHuman)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int nX = (int)pNode->m_vecPosition.x;
				int nY = (int)pNode->m_vecPosition.y;

				if (AutoSkillUse(nX, nY, D3DXVECTOR3(pNode->m_vecPosition.x, pNode->m_fHeight, pNode->m_vecPosition.y), 0, 0, pNode) == 1)
				{
					m_pAutoTarget = pNode;
					return;
				}

				pNode = (TMHuman*)pNode->m_pNextLink;
			}

			return;
		}

		int CheckAtt = 0;
		auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
		while (pNode && pNode->m_pNextLink)
		{
			if (pNode == m_pMyHuman)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			if (pNode == m_pAutoTarget)
			{
				int itx = (int)m_pAutoTarget->m_vecPosition.x;
				int ity = (int)m_pAutoTarget->m_vecPosition.y;

				if (!AutoSkillUse(itx, ity, D3DXVECTOR3(m_pAutoTarget->m_vecPosition.x, m_pAutoTarget->m_fHeight, m_pAutoTarget->m_vecPosition.y), 0, 0, m_pAutoTarget))
				{
					m_pAutoTarget = 0;
					CheckAtt = 1;
					break;
				}
			}

			pNode = (TMHuman*)pNode->m_pNextLink;
		}

		if (!CheckAtt)
			m_pAutoTarget = 0;
	}
}

int TMFieldScene::ToggleNativeCCMode(int mode)
{
	if (mode != 1 && mode != 2)
		return 0;

	g_GameAuto = g_GameAuto == mode ? 0 : mode;
	NewCCMode(true, m_bCompatFieldScene && g_GameAuto != 0);
	return 1;
}

void TMFieldScene::NewCCMode(bool bResetCombat, bool bCapturePosition)
{
	if (g_GameAuto < 0 || g_GameAuto > 3)
		g_GameAuto = 0;
	if (m_AutoHpMp < 0 || m_AutoHpMp > 3)
		m_AutoHpMp = 0;
	if (m_AutoPostionUse < 0 || m_AutoPostionUse > 2)
		m_AutoPostionUse = 2;

	if (g_GameAuto_hpValue < 0)
		g_GameAuto_hpValue = 0;
	else if (g_GameAuto_hpValue > 90)
		g_GameAuto_hpValue = 90;
	g_GameAuto_hpValue -= g_GameAuto_hpValue % 10;

	if (m_bCompatFieldScene)
		g_GameAuto_mountValue = cc_mode::NormalizeThreshold(g_GameAuto_mountValue);
	else if (g_GameAuto_mountValue < 30)
		g_GameAuto_mountValue = 30;
	else if (g_GameAuto_mountValue > 90)
		g_GameAuto_mountValue = 90;
	g_GameAuto_mountValue -= g_GameAuto_mountValue % 10;

	if (bResetCombat)
	{
		m_pAutoTarget = nullptr;
		m_dwAttackDelay = 0;
		if (m_pMyHuman)
			m_pMyHuman->_dwAttackDelay = 0;
	}

	if (bCapturePosition && m_AutoPostionUse == 1 && m_pMyHuman)
	{
		m_AutoStartPointX = static_cast<int>(m_pMyHuman->m_vecPosition.x);
		m_AutoStartPointY = static_cast<int>(m_pMyHuman->m_vecPosition.y);
	}
	else if (bCapturePosition && m_AutoPostionUse != 1)
	{
		m_AutoStartPointX = 0;
		m_AutoStartPointY = 0;
	}

	auto SetButtonState = [](SButton* pButton, int textureSet, char* pAltText)
	{
		if (!pButton)
			return;
		pButton->SetTextureSetIndex(textureSet);
		if (pButton->m_pAltText && pAltText)
			pButton->m_pAltText->SetText(pAltText, 0);
	};

	if (m_pNativeCCPhysicalBtn)
		m_pNativeCCPhysicalBtn->SetSelected(g_GameAuto == 1);
	if (m_pNativeCCMagicBtn)
		m_pNativeCCMagicBtn->SetSelected(g_GameAuto == 2);

	if (m_bCompatFieldScene && m_pccmode)
	{
		const char* modes[] = {"Off", "Physical", "Magic", "Support"};
		const char* positions[] = {"Free", "Cycle", "Fixed"};
		const int positionTextures[] = {556, 557, 558};
		SetButtonState(m_pMGameAutoBtn, 550 + g_GameAuto, const_cast<char*>(modes[g_GameAuto]));
		SetButtonState(m_pSetType, positionTextures[m_AutoPostionUse], const_cast<char*>(positions[m_AutoPostionUse]));
		if (auto modeText = static_cast<SText*>(m_pControlContainer->FindControl(T_CCMODE_COMPAT_MODE)))
			modeText->SetText(const_cast<char*>(g_GameAuto == 0 ? "Off" : modes[g_GameAuto]), 0);
		if (auto moveText = static_cast<SText*>(m_pControlContainer->FindControl(T_CCMODE_COMPAT_MOVE)))
			moveText->SetText(const_cast<char*>(positions[m_AutoPostionUse]), 0);
		char threshold[16]{};
		sprintf_s(threshold, "%d%%", g_GameAuto_hpValue);
		m_pCCModeHpSte->SetText(threshold, 0);
		sprintf_s(threshold, "%d%%", g_GameAuto_mountValue);
		m_pCCModeMountSte->SetText(threshold, 0);
		if (m_pCCPotionBtn)
			m_pCCPotionBtn->SetSelected(g_GameAuto_hpValue != 0);
		if (m_pCCFeedBtn)
			m_pCCFeedBtn->SetSelected(g_GameAuto_mountValue != 0);
		return;
	}

	SButton* pLegacyMode = !m_bCompatFieldScene && m_pControlContainer
		? static_cast<SButton*>(m_pControlContainer->FindControl(B_CCATTACK))
		: nullptr;
	if (!m_bCompatFieldScene || m_pMGameAutoBtn)
	{
		int modeTexture = 458;
		char* pModeText = g_UIString[229];
		if (g_GameAuto == 1)
		{
			modeTexture = 455;
			pModeText = g_UIString[226];
		}
		else if (g_GameAuto == 2)
		{
			modeTexture = 456;
			pModeText = g_UIString[227];
		}
		else if (g_GameAuto == 3)
		{
			modeTexture = 459;
			pModeText = g_UIString[230];
		}

		SetButtonState(m_pMGameAutoBtn, modeTexture, pModeText);
		if (pLegacyMode != m_pMGameAutoBtn)
			SetButtonState(pLegacyMode, modeTexture, pModeText);
	}

	int potionTexture = 462;
	if (m_AutoHpMp == 1)
		potionTexture = 461;
	else if (m_AutoHpMp == 2)
		potionTexture = 460;
	else if (m_AutoHpMp == 3)
		potionTexture = 466;
	if (m_pSGameAutoBtn)
	{
		m_pSGameAutoBtn->SetTextureSetIndex(potionTexture);
		m_pSGameAutoBtn->SetVisible(g_GameAuto != 0);
	}

	SButton* pLegacyMove = !m_bCompatFieldScene && m_pControlContainer
		? static_cast<SButton*>(m_pControlContainer->FindControl(B_CCMOVE))
		: nullptr;
	if (!m_bCompatFieldScene || m_pSetType)
	{
		int positionTexture = 465;
		char* pPositionText = g_UIString[232];
		if (m_AutoPostionUse == 0)
		{
			positionTexture = 463;
			pPositionText = g_UIString[233];
		}
		else if (m_AutoPostionUse == 1)
		{
			positionTexture = 464;
			pPositionText = g_UIString[234];
		}

		SetButtonState(m_pSetType, positionTexture, pPositionText);
		if (pLegacyMove != m_pSetType)
		{
			SetButtonState(pLegacyMove, positionTexture, pPositionText);
			if (pLegacyMove)
				pLegacyMove->SetVisible(g_GameAuto != 0);
		}
	}

	char szThreshold[16]{};
	if (m_pCCModeHpSte)
	{
		sprintf_s(szThreshold, "%d%%", g_GameAuto_hpValue);
		m_pCCModeHpSte->SetText(szThreshold, 0);
	}
	if (m_pCCModeMountSte)
	{
		sprintf_s(szThreshold, "%d%%", g_GameAuto_mountValue);
		m_pCCModeMountSte->SetText(szThreshold, 0);
	}
}
