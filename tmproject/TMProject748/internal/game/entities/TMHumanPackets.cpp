#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "SControl.h"
#include "TMScene.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectMesh.h"
#include "TMEffectSkinMesh.h"
#include "TMShade.h"
#include "DeathMotionPolicy.h"
#include "../../ui/ResourceBarProjection.h"
#include "SControlContainer.h"
#include "ObjectManager.h"
#include "TMCamera.h"
#include "TMFieldScene.h"
#include "TMEffectParticle.h"
#include "TMLog.h"
#include "TMUtil.h"
#include "TMObjectContainer.h"
#include "SGrid.h"
#include "TMEffectStart.h"
#include "TMSkillTownPortal.h"
#include "TMEffectLevelUp.h"
#include "ItemEffect.h"
#include "TMFont3.h"
#include "TMSkillJudgement.h"
#include "TMEffectFirework.h"

int TMHuman::OnPacketEvent(unsigned int dwCode, char* buf)
{
    if (m_dwDelayDel)
        return 0;

    if (buf == nullptr)
        return 0;

    MSG_STANDARD* pStandard = (MSG_STANDARD*)buf;

    if (pStandard->ID != m_dwID)
        return 0;

    if (pStandard->Type == MSG_Action_Opcode || pStandard->Type == MSG_Action_Stop_Opcode || pStandard->Type == MSG_Action2_Opcode)
    {
        MSG_Action* pAction = (MSG_Action*)buf;
        if (pAction->TargetX < 0 || pAction->TargetX > 5000 ||
            pAction->TargetY < 0 || pAction->TargetY > 5000)
        {
            LOG_WRITELOG("\nError Position [X:%d Y:%d] MSG Type : 0x%X\n", pAction->TargetX, pAction->TargetY, pAction->Header.Type);
        }

        if (IsRouteCorrectionAction(pStandard->Type, pAction->Effect))
            return OnPacketRouteCorrection(pAction);
        else if (pAction->Effect == 0 || pAction->Effect == 2)
            return OnPacketMove(pAction);
        else if (pAction->Effect == 7)
            return OnPacketChaosCube(pAction);
        else if (pAction->Effect >= 1)
            return OnPacketIllusion(pStandard);
        else
            return 1;
    }

    switch (pStandard->Type)
    {
    case MSG_PremiumFirework_Opcode:
        return OnPacketPremiumFireWork(reinterpret_cast<MSG_PremiumFirework*>(buf));
        break;
    case MSG_Motion_Opcode:
        return OnPacketFireWork(reinterpret_cast<MSG_Motion*>(buf));
        break;
    case MSG_RemoveMob_Opcode:
        return OnPacketRemoveMob((MSG_STANDARD*)buf);
        break;
    case 0x182:
        return OnPacketSendItem((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateEquip_Opcode:
        return OnPacketUpdateEquip((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateAffect_Opcode:
        return OnPacketUpdateAffect((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateScore_Opcode:
        return OnPacketUpdateScore((MSG_STANDARD*)buf);
        break;
    case MSG_SetHpMp_Opcode:
        return OnPacketSetHpMp(reinterpret_cast<MSG_SetHpMp*>(buf));
        break;
    case 0x18A:
        return OnPacketSetHpDam((MSG_STANDARD*)buf);
        break;
    case 0x333:
        return OnPacketMessageChat((MSG_STANDARD*)buf);
        break;
    case 0x105:
        return OnPacketMessageChat_Index((MSG_STANDARD*)buf);
        break;
    case 0x106:
        return OnPacketMessageChat_Param((MSG_STANDARD*)buf);
        break;
    case 0x334:
        return OnPacketMessageWhisper(reinterpret_cast<MSG_MessageWhisper*>(buf));
        break;
    case MSG_UpdateEtc_Opcode:
        return OnPacketUpdateEtc((MSG_STANDARD*)buf);
        break;
    case 0x3AF:
        return OnPacketUpdateCoin(reinterpret_cast<MSG_STANDARDPARM*>(buf));
        break;
    case 0x1CF:
        return OnPacketUpdateRMB(reinterpret_cast<MSG_STANDARDPARM*>(buf));
        break;
    case MSG_Trade_Opcode:
        return OnPacketTrade(reinterpret_cast<MSG_Trade*>(buf));
        break;
    case MSG_CloseTrade_Opcode:
        return OnPacketQuitTrade((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateCarry_Opcode:
        return OnPacketCarry(reinterpret_cast<MSG_Carry*>(buf));
        break;
    case MSG_CNFTradeCheck_Opcode:
        return OnPacketCNFCheck((MSG_STANDARD*)buf);
        break;
    case 0x193:
        return OnPacketSetClan(reinterpret_cast<MSG_STANDARDPARM*>(buf));
        break;
    case MSG_PlayerChallenge_Opcode:
        return OnPacketReqRanking(reinterpret_cast<MSG_STANDARDPARM2*>(buf));
        break;
    case 0x3AD:
        return OnPacketVisualEffect((MSG_STANDARD*)buf);
        break;
    default:
        break;
    }

    return 0;
}

int TMHuman::OnPacketMove(MSG_Action* pAction)
{
    if (pAction == nullptr)
        return 0;

    // NOTE: there's a strange code in the beginnig, that not make sense...
    // and is not used aparently.

    if (m_cDie == 1)
        return 1;
    if (pAction->Effect == 2)
        m_bSliding = 1;

    m_fMaxSpeed = (float)pAction->Speed;
    if (pAction->Speed < 1)
        m_fMaxSpeed = 1.0f;

    char szBuffer[48]{};

    int nStartRouteIndex = m_nLastRouteIndex;
    if (m_fProgressRate > 0.5f)
        nStartRouteIndex = m_nLastRouteIndex + 1;

    int nX = (int)m_vecRouteBuffer[nStartRouteIndex].x;
    int nY = (int)m_vecRouteBuffer[nStartRouteIndex].y;

    if (strlen(pAction->Route) == 0)
        m_cOnlyMove = 1;

    int tX = 0;
    int tY = 0;

    if (nX == pAction->PosX && nY == pAction->PosY && m_cOnlyMove != 1)
    {
        memcpy(m_cRouteBuffer, pAction->Route, 24);

        m_vecTargetPos.x = pAction->TargetX;
        m_vecTargetPos.y = pAction->TargetY;

        m_cSameHeight = 0;
        GenerateRouteTable(pAction->PosX, pAction->PosY, m_cRouteBuffer, m_vecRouteBuffer, &m_nMaxRouteIndex);
    }
    else
    {
        if (std::abs(pAction->PosX - nX) > 33 || std::abs(pAction->PosY - nY) > 33)
        {
            nX = pAction->PosX;
            nY = pAction->PosY;
        }

        int tX = pAction->TargetX;
        int tY = pAction->TargetY;

        int bRoute = 0;
        char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;
        BASE_GetRoute(nX, nY, &tX, &tY, szBuffer, 12, pHeightMapData, 8);

        if (strlen(szBuffer) == 0)
            return 1;

        if (tX == pAction->TargetX && tY == pAction->TargetY)
        {
            memcpy(m_cRouteBuffer, szBuffer, 48);
            m_vecTargetPos.x = pAction->TargetX;
            m_vecTargetPos.y = pAction->TargetY;
            m_cSameHeight = 0;

            GenerateRouteTable(nX, nY, m_cRouteBuffer, m_vecRouteBuffer, &m_nMaxRouteIndex);
        }
        else if (g_pCurrentScene)
        {
            if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && g_pCurrentScene->m_pMyHuman != this)
            {
                int nStartX = nX;
                int nStartY = nY;
                nX = pAction->PosX;
                nY = pAction->PosY;
                tX = pAction->TargetX;
                tY = pAction->TargetY;
                char* pHeight = (char*)g_pCurrentScene->m_HeightMapData;
                BASE_GetRoute(nX, nY, &tX, &tY, szBuffer, 12, pHeight, 8);

                if (strlen(szBuffer))
                {
                    if (tX == pAction->TargetX && tY == pAction->TargetY)
                    {
                        memcpy(m_cRouteBuffer, szBuffer, 48);

                        m_vecTargetPos.x = pAction->TargetX;
                        m_vecTargetPos.y = pAction->TargetY;
                        m_cSameHeight = 0;

                        GenerateRouteTable(nX, nY, m_cRouteBuffer, m_vecRouteBuffer, &m_nMaxRouteIndex);
                        ChangeRouteBuffer(nStartX, nStartY, m_vecRouteBuffer, &m_nMaxRouteIndex);
                    }
                }
            }
        }
    }

    if (m_cOnlyMove)
    {
        if (g_pCurrentScene && g_pCurrentScene->m_pMyHuman == this)
        {
            TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;
            pFScene->m_stMoveStop.LastX = pAction->PosX;
            pFScene->m_stMoveStop.LastY = pAction->PosY;
            pFScene->m_stMoveStop.NextX = pAction->TargetX;
            pFScene->m_stMoveStop.NextY = pAction->TargetY;
        }
    }

    MoveTo(m_vecRouteBuffer[1]);
    m_fMoveToAngle = m_fAngle;
    m_dwStartMoveTime = g_pTimerManager->GetServerTime();

    return 1;
}

int TMHuman::OnPacketChaosCube(MSG_Action* pAction)
{
    m_cSameHeight = 1;
    m_vecStartPos.x = (int)m_vecPosition.x;
    m_vecStartPos.y = (int)m_vecPosition.y;
    m_vecTargetPos.x = pAction->TargetX;
    m_vecTargetPos.y = pAction->TargetY;

    for (int i = 0; i < 48; ++i)
    {
        m_vecRouteBuffer[i].x = (float)pAction->TargetX + 0.5f;
        m_vecRouteBuffer[i].y = (float)pAction->TargetY + 0.5f;
    }

    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        {
            pScene->m_vecMyNext.x = pAction->TargetX;
            pScene->m_vecMyNext.y = pAction->TargetY;
            pScene->m_stMoveStop.NextX = pAction->TargetX;
            pScene->m_stMoveStop.NextY = pAction->TargetY;
        }

        m_LastSendTargetPos.x = pAction->TargetX;
        m_LastSendTargetPos.y = pAction->TargetY;
    }

    m_dwStartMoveTime = g_pTimerManager->GetServerTime();
    return 1;

}

int TMHuman::OnPacketIllusion(MSG_STANDARD* pStd)
{
    auto pAction = (MSG_Action*)pStd;
    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (!m_cHide && pStd->Type == MSG_Action2_Opcode)
    {
        g_pObjectManager->m_stMobData.CurrentScore.CurMP -= g_pSpell[73].ManaSpent;
        bool bExpand = false;
        if (m_nClass == 4 || m_nClass == 8)
            bExpand = true;

        auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, TMVector3(0.0f, 0.0f, 0.0f), TMVector3(0.0f, 0.0f, 0.0f), 0, nullptr);

        if (m_cMount > 0 && m_pMount)
        {
            pEffect->m_nSkinMeshType = m_nMountSkinMeshType;
            memcpy(&pEffect->m_stLookInfo, &m_stMountLook, sizeof(m_stMountLook));
            pEffect->m_nSkinMeshType2 = m_nSkinMeshType;
            memcpy(&pEffect->m_stLookInfo2, &m_stLookInfo, sizeof(m_stLookInfo));
        }
        else
        {
            memcpy(&pEffect->m_stLookInfo, &m_stLookInfo, sizeof(m_stLookInfo));
        }

        pEffect->m_StartColor.r = 1.0f;
        pEffect->m_StartColor.g = 1.0f;
        pEffect->m_StartColor.b = 1.0f;
        pEffect->m_fScale = m_fScale;
        pEffect->InitObject(bExpand);
        pEffect->m_nFade = 1;
        pEffect->m_dwLifeTime = 3000;
        pEffect->InitPosition(m_vecPosition.x, m_fHeight + 0.1f, m_vecPosition.y);

        if (m_cMount > 0 && m_pMount)
        {
            if (pEffect->m_pSkinMesh)
                pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nMountSkinMeshType].dwAniTable[(int)m_eMotion]]);
            if (pEffect->m_pSkinMesh2)
                pEffect->m_pSkinMesh2->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion + 28]]);
        }
        else
        {
            pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion]]);
        }

        pEffect->m_fStartAngle = m_fAngle;
        pEffect->m_fAngle = pEffect->m_fStartAngle;
        if (m_nWeaponTypeL != 101)
            pEffect->m_fAngle = m_fAngle + D3DXToRadian(360);
        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pEffect->m_nMotionType = 0;

        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
    }
    else if (!m_cHide && pAction->Effect != 5)
    {
        int nType = 1;
        if (pAction->Effect == 6)
            nType = 2;

        auto pPortal = new TMSkillTownPortal(TMVector3(m_vecPosition.x,
            m_fHeight + 0.050000001f, m_vecPosition.y), nType);

        g_pCurrentScene->m_pEffectContainer->AddChild(pPortal);
        _dwAttackDelay = g_pTimerManager->GetRealTime();
    }
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pCamera = g_pObjectManager->m_pCamera;
        bool bDestIsDungeon = false;
        bool bNowIsDungeon = false;

        if ((int)pAction->TargetY >> 7 > 25)
        {
            bDestIsDungeon = true;
            pScene->m_bIsDungeon = 1;
        }
        else
            pScene->m_bIsDungeon = 0;

        if ((int)m_vecPosition.y >> 7 > 25)
            bNowIsDungeon = true;

        if (bDestIsDungeon == 1 && !bNowIsDungeon)//camera
        {
            pCamera->m_fMaxCamLen = 20.0f;
            if (pCamera->m_fSightLength > 20.0f)
                pCamera->m_fSightLength = 20.0f;
        }
        else if (bNowIsDungeon == 1 && !bDestIsDungeon)//camera
        {
            pCamera->m_fMaxCamLen = 20.0f;
            pCamera->m_fSightLength = pCamera->m_fSightLength + 4.0f;
            if (pCamera->m_fSightLength > 20.0f)
                pCamera->m_fSightLength = 20.0f;
        }
        if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        {
            int nNowX = (int)m_vecPosition.x;
            int nNowY = (int)m_vecPosition.y;
            bool bQuestEnd = false;
            bool bQuestStart = false;

            bQuestStart = nNowX >> 7 == 25
                && nNowY >> 7 == 13
                && (int)pAction->TargetX >> 7 >= 26
                && (int)pAction->TargetX >> 7 <= 30
                && (int)pAction->TargetY >> 7 >= 8
                && (int)pAction->TargetY >> 7 <= 12;

            if (bQuestStart)
                pScene->SetQuestStatus(1);
            else if (bQuestEnd == 1)
                pScene->SetQuestStatus(0);

            pScene->m_vecMyNext.x = pAction->TargetX;
            pScene->m_vecMyNext.y = pAction->TargetY;
            pScene->m_stMoveStop.NextX = pAction->TargetX;
            pScene->m_stMoveStop.NextY = pAction->TargetY;
        }
        m_LastSendTargetPos.x = pAction->TargetX;
        m_LastSendTargetPos.y = pAction->TargetY;
    }

    TMVector2 vecPosition{ (float)pAction->TargetX + 0.5f, (float)pAction->TargetY + 0.5f };

    InitPosition(vecPosition.x, static_cast<float>(pScene->GroundGetMask(vecPosition)), vecPosition.y);

    if (pScene->m_pMyHuman != this)
    {
        m_LastSendTargetPos.x = pAction->TargetX;
        m_LastSendTargetPos.y = pAction->TargetY;
    }
    else if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        pScene->m_vecMyNext.x = pAction->TargetX;
        pScene->m_vecMyNext.y = pAction->TargetY;
        pScene->m_stMoveStop.NextX = pAction->TargetX;
        pScene->m_stMoveStop.NextY = pAction->TargetY;
    }

    m_cDie = 0;
    if (!m_cHide && pAction->Effect != 5 && pStd->Type != 872)
    {
        if (m_nClass != 1 && m_nClass != 2 && m_nClass != 4 && m_nClass != 8 && m_nClass != 26)
        {
            auto pEffect = new TMEffectStart(
                TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                1, nullptr);

            pScene->m_pEffectContainer->AddChild(pEffect);
        }
        else
        {
            auto pEffect = new TMEffectStart(
                TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                0, nullptr);

            pScene->m_pEffectContainer->AddChild(pEffect);
        }

        if (g_pObjectManager->m_pCamera->m_pFocusedObject == this)
            GetSoundAndPlay(151, 0, 0);

        int nType = 1;
        if (pAction->Effect == 6)
            nType = 2;

        if (pAction->Effect == 6 && m_nClass == 62 && m_stLookInfo.FaceMesh == 2)
        {
            InitPosition(m_vecPosition.x, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) - 2.0f, m_vecPosition.y);
            TMVector3 vecPos{ m_vecPosition.x, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.2f, m_vecPosition.y };

            auto pJudgement = new TMSkillJudgement(vecPos, 4, 0.1f);

            pScene->m_pEffectContainer->AddChild(pJudgement);
        }
        else
        {
            auto pPartal = new TMSkillTownPortal(
                TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                nType);

            pScene->m_pEffectContainer->AddChild(pPartal);
        }
    }

    if (pScene->m_pMyHuman == this)
    {
        auto pCamera = g_pObjectManager->m_pCamera;
        auto fHAngle = pCamera->m_fHorizonAngle;

        pScene->Warp();
        if (pStd->Type == MSG_Action2_Opcode)
            pCamera->m_fHorizonAngle = fHAngle;
        if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
            pScene->m_bLastMyAttr = BASE_GetAttr(pAction->TargetX, pAction->TargetY);

        InitPosition(vecPosition.x,
            (float)pScene->GroundGetMask(vecPosition) * 0.1f,
            vecPosition.y);
    }
    if (pStd->Type == MSG_Action2_Opcode)
    {
        SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 0);
        if (g_pObjectManager->m_pCamera->m_pFocusedObject == this)
            GetSoundAndPlay(175, 0, 0);
    }
    else
    {
        SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);
        if (!m_cHide)
        {
            auto pLevelUp = new TMEffectLevelUp(TMVector3(m_vecPosition.x, m_fHeight, m_vecPosition.y), 0);
            pScene->m_pEffectContainer->AddChild(pLevelUp);

            if (!m_cSummons)
            {
                auto pPortal = new TMSkillTownPortal(
                    TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                    1);

                pScene->m_pEffectContainer->AddChild(pPortal);
            }
        }
    }

    m_bIgnoreHeight = 0;
    if (pScene && pScene->m_pMyHuman == this && pScene->m_bAirMove == 1)
        pScene->AirMove_End(TMFieldScene::AirMoveEndReason::ExternalTeleport);

    return 1;
}

int TMHuman::OnPacketFireWork(MSG_Motion* pStd)
{
	if (pStd->Motion == 100)
	{
		if (g_pCurrentScene->m_pEffectContainer)
			g_pCurrentScene->m_pEffectContainer->AddChild(new TMEffectFireWork(
				{ m_vecPosition.x, m_fHeight + 5.0f, m_vecPosition.y }, pStd->Parm));
		return 1;
	}
	// A delayed motion packet must not replace the death animation. Parm 2 is
	// the explicit revival motion and is allowed to clear the death state.
	if ((m_cDie == 1 || m_stScore.CurHP <= 0) && pStd->Parm != 2)
		return 1;
	if (pStd->Parm == 1)
	{
		if (m_nClass == 36 || m_nClass == 37)
			return 1;
		if (m_nClass == 39)
		{
			if (!(rand() % 2))
			{
				pStd->Motion = 16;
				if (m_bVisible == 1)
					GetSoundAndPlay(267, 0, 0);
			}

			SetMotion((ECHAR_MOTION)pStd->Motion, pStd->Direction);
			return 1;
		}
		if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
		{
			SetMotion((ECHAR_MOTION)pStd->Motion, pStd->Direction);
			return 1;
		}
		if (m_nSkinMeshType == 0 || m_nSkinMeshType == 1 || m_nSkinMeshType == 21 || m_nSkinMeshType == 3)
		{
			SetMotion((ECHAR_MOTION)pStd->Motion, pStd->Direction);
			if (m_nClass == 4 && m_stLookInfo.FaceMesh == 15 && m_bVisible == 1)
				GetSoundAndPlay(300, 0, 0);

			return 1;
		}

		return 1;
	}

	if (pStd->Parm == 2)
		m_cDie = 0;
	if (pStd->Parm == 3 && g_pCurrentScene->m_pEffectContainer)
	{
		auto pLevelUp = new TMEffectLevelUp({ m_vecPosition.x, m_fHeight, m_vecPosition.y }, 0);
		g_pCurrentScene->m_pEffectContainer->AddChild(pLevelUp);
	}
	if (pStd->Motion < 256)
	{
		if (g_pObjectManager->m_dwCharID == pStd->Header.ID)
			m_SendeMotion = ECHAR_MOTION::ECMOTION_NONE;

		auto eMotion = pStd->Motion;
		if ((m_dwID > 0 && m_dwID < 1000) &&
			(m_nSkinMeshType == 3 || m_nSkinMeshType == 8 || m_nSkinMeshType == 7 || m_nSkinMeshType == 25 || m_nSkinMeshType == 28) &&
			((int)eMotion > 14 && (int)eMotion < 25 || eMotion == 13))
		{
			eMotion -= 14;
		}

		SetMotion((ECHAR_MOTION)eMotion, pStd->Direction);
	}

	return 1;
}

int TMHuman::OnPacketPremiumFireWork(MSG_PremiumFirework* pFirework)
{
	if (!pFirework || !g_pCurrentScene || !g_pCurrentScene->m_pEffectContainer)
		return 1;

	auto pEffect = new TMEffectFireWork(
		{ m_vecPosition.x, m_fHeight + 5.0f, m_vecPosition.y },
		6);
	pEffect->SetCustomFireWork(pFirework->Bitmap);
	g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
	return 1;
}

int TMHuman::OnPacketRouteCorrection(MSG_Action* pAction)
{
    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD ||
        g_pCurrentScene->m_pMyHuman != this || m_cDie == 1 || m_stScore.CurHP <= 0 ||
        pAction->TargetX < 1 || pAction->TargetX > 5000 ||
        pAction->TargetY < 1 || pAction->TargetY > 5000)
        return 1;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    const TMVector2 position{static_cast<float>(pAction->TargetX) + 0.5f,
                             static_cast<float>(pAction->TargetY) + 0.5f};
    InitPosition(position.x, static_cast<float>(pScene->GroundGetMask(position)) * 0.1f, position.y);
    m_nMaxRouteIndex = 0;
    m_nLastRouteIndex = 0;
    m_fProgressRate = 0.0f;
    m_bMoveing = 0;
    m_cOnlyMove = 0;
    m_bSliding = 0;
    memset(m_cRouteBuffer, 0, sizeof(m_cRouteBuffer));
    m_vecStartPos.x = pAction->TargetX;
    m_vecStartPos.y = pAction->TargetY;
    m_pMoveTargetHuman = nullptr;
    m_pMoveSkillTargetHuman = nullptr;
    pScene->m_vecMyNext.x = pAction->TargetX;
    pScene->m_vecMyNext.y = pAction->TargetY;
    pScene->m_stMoveStop.LastX = pAction->TargetX;
    pScene->m_stMoveStop.LastY = pAction->TargetY;
    pScene->m_stMoveStop.NextX = pAction->TargetX;
    pScene->m_stMoveStop.NextY = pAction->TargetY;
    m_dwStartMoveTime = g_pTimerManager->GetServerTime();
    SetAnimation(ECHAR_MOTION::ECMOTION_STAND01, 0);
    return 1;
}

int TMHuman::OnPacketRemoveMob(MSG_STANDARD* pStd)
{
    auto pRemoveMob = (MSG_RemoveMob*)pStd;
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        if (m_stScore.CurHP <= 0 && !m_sFamCount)
            Die();

        return 1;
    }

    auto pScene = g_pCurrentScene;
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    // Native WYD 7.48 FUN_00529bf8 first proves that both the tracked target
    // and its panel exist.  Removal packets also arrive for ordinary NPCs,
    // where these optional UI pointers are null.
    if (pFScene && pFScene->m_pPGTOver && pFScene->m_pPGTPanel &&
        m_dwID == pFScene->m_pPGTOver->m_dwID &&
        pFScene->m_pPGTPanel->IsVisible() == 1)
        pFScene->m_pPGTPanel->SetVisible(0);

    if (pFScene && pFScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        // The 7.48 field can receive an NPC removal while optional shop panels
        // are not constructed; only close a shop whose controls still exist.
        if (pFScene->m_pGridShop && pFScene->m_pShopPanel &&
            m_dwID == pFScene->m_pGridShop->m_dwMerchantID &&
            pFScene->m_pShopPanel->m_bVisible == 1)
            pFScene->SetVisibleShop(0);
        if (pFScene->m_pHellgateStore && m_dwID == pFScene->m_dwHellStoreID &&
            pFScene->m_pHellgateStore->m_bVisible == 1)
            pFScene->SetVisibleHellGateStore(0);

        if (m_dwID == pFScene->m_stAutoTrade.TargetID)
        {
            auto pPanel = (SPanel*)pFScene->m_pControlContainer->FindControl(646);
            if (pPanel)
            {
                if (pPanel->IsVisible() == 1)
                    pFScene->SetVisibleAutoTrade(0, 0);
            }
        }

        auto pPartyList = pFScene->m_pPartyList;
        if (pPartyList && pPartyList->m_nNumItem < 2 && m_bParty == 1)
        {
            pPartyList->Empty();
            m_bParty = 0;

            SAFE_DELETE(m_pInMiniMap);
            pFScene->m_pMyHuman->m_bParty = 0;
        }
        else if (pPartyList)
        {
            for (int i = 0; i < pPartyList->m_nNumItem; ++i)
            {
                auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
                if (pPartyItem->m_dwCharID == m_dwID)
                {
                    auto pNode = g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
                    if (pNode)
                    {
                        pNode->m_bParty = 0;
                        SAFE_DELETE(m_pInMiniMap);
                    }
                    if (pPartyItem->m_nState == 2)
                    {
                        pPartyItem->m_nState = 4;
                        pPartyItem->m_GCText.dwColor = 0xFF777777;
                        pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
                    }
                    else if (!pPartyItem->m_nState)
                    {
                        pPartyItem->m_nState = 3;
                        pPartyItem->m_GCText.dwColor = 0xFF777777;
                        pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
                    }
                    break;
                }
            }
        }
    }
    if (m_nWillDie == -1)
    {
        if (!pRemoveMob->RemoveType)
        {
            if (g_pCurrentScene->m_pMyHuman != this)
                m_nWillDie = pRemoveMob->RemoveType;

            // RemoveType 0 keeps the actor in DelayDelete briefly, but its native
            // AutoTrade overlay belongs to the removed shop and must disappear now.
            memset(m_TradeDesc, 0, sizeof(m_TradeDesc));
            if (m_pAutoTradeDesc)
            {
                m_pAutoTradeDesc->SetText((char*)"", 0);
                m_pAutoTradeDesc->SetVisible(0);
            }
            if (m_pAutoTradePanel)
                m_pAutoTradePanel->SetVisible(0);

            return 1;
        }
        if (pRemoveMob->RemoveType == 1 || m_nSkinMeshType == 37)
        {
            unsigned int dwServerTime = g_pTimerManager->GetServerTime();
            m_nWillDie = pRemoveMob->RemoveType;
            if (!m_dwDeadTime)
                m_dwDeadTime = dwServerTime;
            m_stScore.CurHP = 0;
            Die();
            return 1;
        }
        if (pRemoveMob->RemoveType == 2)
        {
            m_nWillDie = pRemoveMob->RemoveType;

            TMVector3 vecStart{ m_vecPosition.x, (float)((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, m_vecPosition.y };
            if (!m_cHide)
            {
                if (m_nClass == 1 || m_nClass == 2 || m_nClass == 4 || m_nClass == 8 || m_nClass == 26)
                {
                    auto pEffect = new TMEffectStart(vecStart, 0, 0);
                    pScene->m_pEffectContainer->AddChild(pEffect);

                    auto pEffect2 = new TMEffectBillBoard2(1, 2000, 0.5f, 0.5f, 0.5f, 0.002f, 0);
                    pEffect2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect2->m_vecPosition = vecStart;
                    pScene->m_pEffectContainer->AddChild(pEffect2);
                }
                else if (m_nSkinMeshType == 21 || m_nSkinMeshType == 22 || m_nSkinMeshType == 23 ||
                    m_nSkinMeshType == 24 || m_nSkinMeshType == 2 || m_nSkinMeshType == 3 || m_nSkinMeshType == 4)
                {
                    auto pEffect = new TMEffectStart(vecStart, 1, 0);
                    pScene->m_pEffectContainer->AddChild(pEffect);
                }
                else if (m_nSkinMeshType == 20)
                {
                    auto pEffect = new TMEffectStart(vecStart, 1, 0);
                    pScene->m_pEffectContainer->AddChild(pEffect);
                }
            }

            if (!m_dwDeadTime)
                m_dwDeadTime = g_pTimerManager->GetServerTime() + 1000;

            if (static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject) == this)
            {
                GetSoundAndPlay(4, 0, 0);
            }

            return 1;
        }

        if (m_nWillDie == 3)
        {
            if (g_pCurrentScene->m_pMyHuman != this)
                DelayDelete();

            return 1;
        }

        int nType = 1;

        if (m_dwID <= 0 || m_dwID >= 1000)
            nType = 3;

        if (!m_cHide)
        {
            auto pPortal = new TMSkillTownPortal(TMVector3(m_vecPosition.x, m_fHeight, m_vecPosition.y), nType);
            pScene->m_pEffectContainer->AddChild(pPortal);
        }

        if (pScene->m_pMyHuman != this)
            DelayDelete();
        return 1;
    }

    if (m_nWillDie == 1)
    {
        if (m_nWillDie == 1 && !m_dwDeadTime)
        {
            m_cDie = 0;
            Die();
            if (!m_dwDeadTime)
                m_dwDeadTime = g_pTimerManager->GetServerTime() + 1000;
        }
        return 1;
    }

    if (g_pCurrentScene->m_pMyHuman != this)
        DelayDelete();

	return 1;
}

int TMHuman::OnPacketSendItem(MSG_STANDARD* pStd)
{
    auto pSendItem = reinterpret_cast<MSG_SendItem*>(pStd);

    // Ownership transfers only after PickupItem/PickupAtItem removes the item
    // from its grid. Apply the same cleanup to Equip, Carry, and Cargo so hover,
    // sale, and attachment pointers cannot reference freed memory. This is a
    // local policy equivalent to Empty, not a claim of native cleanup parity.
    const auto releaseReplacedItem = [](SGridControlItem* item)
    {
        if (!item)
            return;
        if (SGridControl::m_pLastMouseOverItem == item)
            SGridControl::m_pLastMouseOverItem = nullptr;
        if (SGridControl::m_pLastAttachedItem == item)
            SGridControl::m_pLastAttachedItem = nullptr;
        if (SGridControl::m_pSellItem == item)
            SGridControl::m_pSellItem = nullptr;
        if (g_pCursor && g_pCursor->m_pAttachedItem == item)
            g_pCursor->m_pAttachedItem = nullptr;
        delete item;
    };

    TMFieldScene* pFScene{};

    // ObjectManager validates the frame size on entry. Local array capacity
    // bounds each index before any copy or visual effect. Additional source
    // slots remain preserved.
    auto pMobData = &g_pObjectManager->m_stMobData;
    if (!IsSendItemDestination(pSendItem->DestType, pSendItem->DestPos,
        sizeof(pMobData->Equip) / sizeof(pMobData->Equip[0]),
        sizeof(pMobData->Carry) / sizeof(pMobData->Carry[0]),
        sizeof(g_pObjectManager->m_stItemCargo) / sizeof(g_pObjectManager->m_stItemCargo[0])))
        return 1;

    // Every materialized item must resolve inside the loaded 7.48 catalog.
    // In particular, SetPacketMOBItem indexes g_pItemList for equipment.
    // Zero is the wire representation of an empty slot.
    if (pSendItem->Item.sIndex < 0 || pSendItem->Item.sIndex >= MAX_ITEMLIST)
        return 1;

    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    // 0x182 resynchronizes only the local character. Applying the global
    // inventory to another human would replace that human's appearance with
    // our equipment even when no grid is updated.
    if (g_pCurrentScene->m_pMyHuman != this)
        return 1;

    if (pFScene)
        pFScene->Bag_View();

    if (pFScene && g_pCurrentScene->m_pMyHuman == this)
    {
        if (pSendItem->DestType == 0)
        {
            if (pSendItem->DestPos == 6 && BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[6], EF_WTYPE) == 41)
            {
                m_stLookInfo.RightMesh = 0;
                m_stLookInfo.RightSkin = 0;
            }

            memcpy(&pMobData->Equip[pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));

            // The selection cache is auxiliary: a character sentinel must not
            // prevent the authoritative equipment update in the world.
            const int characterSlot = g_pObjectManager->m_cCharacterSlot;
            if (pSendItem->DestPos && characterSlot >= 0 && characterSlot < 4)
                memcpy(&g_pObjectManager->m_stSelCharData.Equip[characterSlot][pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));

            SGridControl* pGridEquip[MAX_EQUIPITEM]{};

            pGridEquip[1] = pFScene->m_pGridHelm;
            pGridEquip[2] = pFScene->m_pGridCoat;
            pGridEquip[3] = pFScene->m_pGridPants;
            pGridEquip[4] = pFScene->m_pGridGloves;
            pGridEquip[5] = pFScene->m_pGridBoots;
            pGridEquip[6] = pFScene->m_pGridLeft;
            pGridEquip[7] = pFScene->m_pGridRight;
            pGridEquip[8] = pFScene->m_pGridRing;
            pGridEquip[9] = pFScene->m_pGridNecklace;
            pGridEquip[10] = pFScene->m_pGridOrb;
            pGridEquip[11] = pFScene->m_pGridCabuncle;
            pGridEquip[12] = pFScene->m_pGridGuild;
            pGridEquip[13] = pFScene->m_pGridEvent;
            pGridEquip[14] = pFScene->m_pGridDRing;
            pGridEquip[15] = pFScene->m_pGridMantua;
            pGridEquip[16] = pFScene->m_pGridNewSlot1;
            pGridEquip[17] = pFScene->m_pGridNewSlot2;
            if (pSendItem->DestPos > 0 && pSendItem->DestPos < MAX_EQUIPITEM)
            {
                if (pGridEquip[pSendItem->DestPos] != nullptr)
                {
                    SGridControlItem* pItem = pGridEquip[pSendItem->DestPos]->PickupItem(0, 0);
                    releaseReplacedItem(pItem);
                }

                if (pSendItem->Item.sIndex > 0 && pGridEquip[pSendItem->DestPos])
                {
                    auto pstItem = new STRUCT_ITEM();

                    if (pstItem)
                    {
                        memcpy(pstItem, &pMobData->Equip[pSendItem->DestPos], sizeof(STRUCT_ITEM));

                        auto pItem = new SGridControlItem(0, pstItem, 0.0f, 0.0f);

                        if (pItem)
                        {
                            if (pGridEquip[pSendItem->DestPos])
                            {
                                pGridEquip[pSendItem->DestPos]->Empty();
                                if (!pGridEquip[pSendItem->DestPos]->AddItem(pItem, 0, 0))
                                    releaseReplacedItem(pItem);
                            }
                        }
                        else
                            SAFE_DELETE(pstItem);
                    }
                }
            }
        }
        else if (pSendItem->DestType == 1)
        {
            memcpy(&pMobData->Carry[pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));
            memcpy(g_pObjectManager->m_stMobData.Carry, pMobData->Carry, sizeof(g_pObjectManager->m_stMobData.Carry));

            // Project the server's structural Carry slot through the active
            // 7.48/7.59 UI topology instead of assuming 15-slot pages.
            int CellIndexX = 0;
            int CellIndexY = 0;
            pFScene->GetCarryCellForSlot(pSendItem->DestPos, CellIndexX, CellIndexY);
            SGridControl* pGrid = pFScene->GetCarryGridForSlot(pSendItem->DestPos);

            if (pGrid)
            {
                SGridControlItem* pOldGridItem = pGrid->PickupAtItem(CellIndexX, CellIndexY);
                releaseReplacedItem(pOldGridItem);

                if (pSendItem->Item.sIndex > 0)
                {
                    auto pstItem = new STRUCT_ITEM();

                    if (pstItem)
                    {
                        memcpy(pstItem, &pSendItem->Item, sizeof(STRUCT_ITEM));

                        auto pItem = new SGridControlItem(0, pstItem, 0.0f, 0.0f);

                        if (pItem && !pGrid->AddItem(pItem, CellIndexX, CellIndexY))
                            releaseReplacedItem(pItem);
                        else if (!pItem)
                            SAFE_DELETE(pstItem);
                    }
                }
            }
        }
        else if (pSendItem->DestType == 2)
        {
            memcpy(&g_pObjectManager->m_stItemCargo[pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));

            // Cargo is one nine-column registry in native 7.48 and paged only
            // by newer TMProject resources.
            int CellIndexX = 0;
            int CellIndexY = 0;
            pFScene->GetCargoCellForSlot(pSendItem->DestPos, CellIndexX, CellIndexY);
            auto pGrid = pFScene->GetCargoGridForSlot(pSendItem->DestPos);

            // An unbound Cargo grid suppresses only its visual projection.
            // Keep the committed slot and common appearance/HUD finalization.
            if (pGrid)
            {
                SGridControlItem* pOldGridItem = pGrid->PickupAtItem(CellIndexX, CellIndexY);
                releaseReplacedItem(pOldGridItem);

                if (pSendItem->Item.sIndex > 0)
                {
                    auto pstItem = new STRUCT_ITEM();

                    if (pstItem)
                    {
                        memcpy(pstItem, &pSendItem->Item, sizeof(STRUCT_ITEM));

                        auto pItem = new SGridControlItem(0, pstItem, 0.0f, 0.0f);

                        if (pItem && !pGrid->AddItem(pItem, CellIndexX, CellIndexY))
                            releaseReplacedItem(pItem);
                        else if (!pItem)
                            SAFE_DELETE(pstItem);
                    }
                }
            }
        }
    }

    SetPacketMOBItem(pMobData);
    SetCharHeight(static_cast<float>(m_stScore.Con));
    SetRace(pMobData->Equip[0].sIndex);

    if (m_nWeaponTypeL == 41)
    {
        m_stLookInfo.RightMesh = m_stLookInfo.LeftMesh;
        m_stLookInfo.RightSkin = m_stLookInfo.LeftSkin;
        m_stSancInfo.Sanc6 = m_stSancInfo.Sanc7;
        m_stSancInfo.Legend6 = m_stSancInfo.Legend7;
    }

    InitObject();
    CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
    InitAngle(0, m_fAngle, 0);

    if (m_cMount == 1)
    {
        int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], EF_MOUNTHP);

        if (m_pMountHPBar)
            m_pMountHPBar->SetCurrentProgress(nMountHP);

        if (pFScene)
        {
            // The 7.48 field resource legitimately omits the 7.59 mount-HUD
            // controls. Mount materialization must not require those widgets.
            if (pFScene->m_pMHPBar)
                pFScene->m_pMHPBar->SetCurrentProgress(nMountHP);

            char szMHP[32]{};
            sprintf_s(szMHP, "%d", nMountHP);

            if (pFScene->m_pCurrentMHPText)
                pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
        }
    }

    if (pFScene)
        pFScene->UpdateScoreUI(0);

    SGridControl::m_sLastMouseOverIndex = -1;
    return 1;
}

int TMHuman::OnPacketUpdateEquip(MSG_STANDARD* pStd)
{
    auto pEquip = reinterpret_cast<MSG_UpdateEquip*>(pStd);
    STRUCT_ITEM item{};
    item.sIndex = pEquip->sEquip[0] & 0xFFF;

    TMEffectParticle* pParticle = nullptr;
    if (m_stLookInfo.FaceMesh != g_pItemList[item.sIndex].nIndexMesh || BASE_GetItemAbility(&item, EF_CLASS) != m_nClass)
        pParticle = new TMEffectParticle(TMVector3{ m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y }, 1, 3, 3.0f, 0, 1, 56, 1.0f, 1, TMVector3{}, 1000u);

    if (pParticle)
        g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);

    if (g_pSoundManager && g_pSoundManager->GetSoundData(158))
        g_pSoundManager->GetSoundData(158)->Play();

    auto pLightMap = new TMShade(4, 7, 1.0f);

    if (pLightMap)
    {
        pLightMap->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pLightMap->SetPosition(m_vecPosition);
        pLightMap->m_dwLifeTime = 3000;
        pLightMap->SetColor(0xAAAAAAAA);
        g_pCurrentScene->m_pEffectContainer->AddChild(pLightMap);
    }

    SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);

    if (!m_cHide)
    {
        auto pLevelUp = new TMEffectLevelUp(TMVector3{ m_vecPosition.x, m_fHeight, m_vecPosition.y }, 0);

        if (pLevelUp)
            g_pCurrentScene->m_pEffectContainer->AddChild(pLevelUp);
    }



    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pMobData = &g_pObjectManager->m_stMobData;
        g_pObjectManager->m_stMobData.Equip[0].sIndex = pEquip->sEquip[0] & 0xFFF;

        if (m_cMount == 1)
        {
            int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], EF_MOUNTHP);
            if (m_pMountHPBar)
                m_pMountHPBar->SetCurrentProgress(nMountHP);
            if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            {
                auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
                // Equip refresh can arrive before optional 7.59 HUD controls;
                // the 7.48 character and mount state remain authoritative.
                if (pFScene->m_pMHPBar)
                    pFScene->m_pMHPBar->SetCurrentProgress(nMountHP);
                char szMHP[32] = { 0 };
                sprintf_s(szMHP, "%d", nMountHP);

                if (pFScene->m_pCurrentMHPText)
                    pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
            }
        }
    }

    SetPacketEquipItem(pEquip->sEquip);

    // The full-ID helper also acts as a handled guard: legacy Equip2 must not
    // overwrite a KR visual selected from slot 14 with an unrelated byte case.
    if (!SetImportedMountCostume(pEquip->sEquip[14])
        && ((pEquip->sEquip[14] & 0xFFF) && ((pEquip->sEquip[14] & 0xFFF) < 3980)
            || (pEquip->sEquip[14] & 0xFFF) >= 3999))
        SetMountCostume(pEquip->Equip2[14]);

    SetColorItem(pEquip->Equip2);
    float fCon = static_cast<float>(m_stScore.Con);

    SetCharHeight(fCon);
    SetRace(pEquip->sEquip[0] & 0xFFF);

    STRUCT_ITEM itemL{};
    itemL.sIndex = pEquip->sEquip[6] & 0xFFF;

    int nWeaponTypeL = BASE_GetItemAbility(&itemL, EF_WTYPE);
    if (nWeaponTypeL == 41)
    {
        m_stLookInfo.RightMesh = m_stLookInfo.LeftMesh;
        m_stLookInfo.RightSkin = m_stLookInfo.LeftSkin;

        m_stSancInfo.Sanc6 = m_stSancInfo.Sanc7;
        m_stSancInfo.Legend6 = m_stSancInfo.Legend7;
    }

    InitObject();

    CheckWeapon(pEquip->sEquip[6] & 0xFFF, pEquip->sEquip[7] & 0xFFF);
    InitAngle(0.0f, m_fAngle, 0.0f);

    TMFieldScene* pScene = nullptr;
    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pScene)
        pScene->UpdateScoreUI(0);

    SGridControl::m_sLastMouseOverIndex = -1;
	return 1;
}

int TMHuman::OnPacketUpdateAffect(MSG_STANDARD* pStd)
{
    auto pUpdateAffect = reinterpret_cast<MSG_UpdateAffect*>(pStd);

    TMFieldScene* pFScene{};

    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pFScene && g_pCurrentScene->m_pMyHuman == this)
    {
        m_DilpunchJewel = 0;
        m_MoonlightJewel = 0;
        m_BloodJewel = 0;
        m_JewelGlasses = 0;
        m_RedJewel = 0;

        unsigned int dwServerTime = g_pTimerManager->GetServerTime();

        // Opcode 0x3B9 is the native 7.48 16-entry icon/timer snapshot.  Use
        // the packet member count so this handler cannot read a newer ABI tail.
        for (int i = 0; i < _countof(pUpdateAffect->Affect); ++i)
        {
            if ((8 * pUpdateAffect->Affect[i].Time - 8 - (dwServerTime - pFScene->m_dwStartAffectTime[i]) / 1000) / 8 != pUpdateAffect->Affect[i].Time
                || m_stAffect[i].Type != pUpdateAffect->Affect[i].Type)
            {
                memcpy(&m_stAffect[i], &pUpdateAffect->Affect[i], sizeof(m_stAffect[i]));
                pFScene->m_dwStartAffectTime[i] = g_pTimerManager->GetServerTime();
            }

            if (pUpdateAffect->Affect[i].Type == 8)
            {
                if (pUpdateAffect->Affect[i].Value & 0x1)
                    m_DilpunchJewel = 1;
                if (pUpdateAffect->Affect[i].Value & 0x2)
                    m_MoonlightJewel = 1;
                if (pUpdateAffect->Affect[i].Value & 0x4)
                    m_JewelGlasses = 1;
                if (pUpdateAffect->Affect[i].Value & 0x8)
                    m_BloodJewel = 1;
                if (pUpdateAffect->Affect[i].Value & 0x10)
                    m_RedJewel = 1;
            }
        }

        if (!pFScene->m_nYear && !pFScene->m_nDays || pFScene->m_dwEventTime && dwServerTime > (pFScene->m_dwEventTime + 3600000))
        {
            pFScene->m_dwEventTime = dwServerTime;
            MSG_MessageWhisper stWhisper{};
            stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
            stWhisper.Header.Type = MSG_MessageWhisper_Opcode;
            sprintf_s(stWhisper.MobName, "day");
            SendPacket({reinterpret_cast<MSG_STANDARD*>(&stWhisper)->Type, reinterpret_cast<char*>(&stWhisper), sizeof(stWhisper)});
        }

        if (pFScene)
            pFScene->UpdateScoreUI(1u);
    }
    return 1;
}

int TMHuman::OnPacketUpdateScore(MSG_STANDARD* pStd)
{
    auto pUpdateScore = reinterpret_cast<MSG_UpdateScore*>(pStd);

    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        static_cast<TMFieldScene*>(g_pCurrentScene)->Bag_View();

    m_cHide = m_dwID < 1000 == 1 && pUpdateScore->Score.Merchant & 1;

    if (m_dwID >= 1000 && pUpdateScore->ReqHp == 1)
    {
        TMHuman* pHuman = g_pObjectManager->GetHumanByID(m_dwID);

        if (pHuman != nullptr)
        {
            pHuman->m_BigHp = pHuman->m_usHP;
            pHuman->m_MaxBigHp = pHuman->m_usHP;
        }
    }

    TMFieldScene* pFScene{};

    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pFScene == nullptr)
        return 1;

    SListBox* pPartyList = pFScene->m_pPartyList;

    if (pPartyList != nullptr)
    {
        for (int i = 0; i < pPartyList->m_nNumItem; ++i)
        {
            auto pPartyItem = static_cast<SListBoxPartyItem*>(pPartyList->m_pItemList[i]);
            if (pPartyItem->m_dwCharID == m_dwID)
            {
                char szVal[32]{};

                sprintf_s(szVal, "%d", pUpdateScore->Score.Level + 1);

                pPartyItem->m_pLevelText->SetText(szVal, 0);

                if (pUpdateScore->Score.CurHP > pUpdateScore->Score.MaxHP)
                    pUpdateScore->Score.CurHP = pUpdateScore->Score.MaxHP;

                pPartyItem->m_pHpProgress->SetMaxProgress(pUpdateScore->Score.MaxHP);
                pPartyItem->m_pHpProgress->SetCurrentProgress(pUpdateScore->Score.CurHP);
                break;
            }
        }
    }

    memcpy(&m_stScore, &pUpdateScore->Score, sizeof(m_stScore));

    if (g_pCurrentScene && g_pCurrentScene->m_pMyHuman == this)
    {
        if (g_pObjectManager->m_stMobData.CurrentScore.Level < pUpdateScore->Score.Level && Is2stClass() == 2)
        {
            auto pMobData = &g_pObjectManager->m_stMobData;
            if (pFScene->m_pLevelQuest[pMobData->CurrentScore.Level + 1])
            {
                if (g_nBattleMaster % 10 >= 6 && pUpdateScore->Score.Level >= 350 || g_nBattleMaster % 10 < 6 && pUpdateScore->Score.Level < 350)
                {
                    pFScene->m_pQuestMemo->SetVisible(1);

                    DWORD dwCol = 0xFFAAAAFF;

                    char szStr[128]{};

                    if (pFScene->m_pLevelQuest[pMobData->CurrentScore.Level + 1] == 100)
                        dwCol = pFScene->LoadMsgText4(
                            szStr,
                            sizeof szStr,
                            "UI\\QuestMessage.txt",
                            pMobData->CurrentScore.Level + 2,
                            pMobData->Equip[0].sIndex % 10);
                    else
                        sprintf_s(szStr, g_pMessageStringTable[307]);

                    if (dwCol != 0xFF000000)
                    {
                        pFScene->m_pMessagePanel->SetMessage(szStr, 3000);

                        auto pItem = new SListBoxItem(szStr, dwCol, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0);

                        if (pItem)
                            pFScene->m_pChatListnotice->AddItem(pItem);
                    }
                }
            }
        }
        memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pUpdateScore->Score, sizeof(STRUCT_SCORE));
        m_sGuildLevel = static_cast<unsigned char>(g_pObjectManager->m_stMobData.GuildLevel);
    }

    unsigned short usGuild = pUpdateScore->Guild;

    m_usGuild = usGuild;

    if (usGuild)
    {
        m_stGuildMark.bHideGuildmark = 0;
        m_stGuildMark.nGuild = usGuild & 0xFFF;
        m_stGuildMark.nSubGuild = BASE_GetSubGuild(m_sGuildLevel);
        m_stGuildMark.nGuildChannel = ((int)usGuild >> 12) & 0xF;
        m_stGuildMark.sGuildIndex = m_sGuildLevel;

        if (pFScene && !m_pAutoTradeDesc->IsVisible())
            pFScene->Guildmark_Create(&m_stGuildMark);
    }
    else
    {
        m_stGuildMark.bHideGuildmark = 1;
        m_stGuildMark.pGuildMark->SetVisible(0);//guild
    }

    SetCharHeight(static_cast<float>(m_stScore.Con));

    if (m_nClass == 40)
        m_fScale *= (((float)m_stScore.Mastery[3] * 0.003f) + 1.0f);

    if (pFScene && g_pCurrentScene->m_pMyHuman == this)
    {
        DWORD dwServerTime = g_pTimerManager->GetServerTime();

        for (int l = 0; l < 32; ++l)
        {
            if ((((m_usAffect[l] & 0xFF) - 1) * 8
                + 4
                - (dwServerTime - pFScene->m_dwStartAffectTime[l]) / 1000)
                / 8 != (pUpdateScore->Affect[l] & 0xFF)
                || (int)m_usAffect[l] >> 8 != (int)pUpdateScore->Affect[l] >> 8)
            {
                memcpy(&m_usAffect[l], &pUpdateScore->Affect[l], sizeof(m_usAffect[l]));
                pFScene->m_dwStartAffectTime[l] = dwServerTime;
            }
        }
    }
    else
    {
        memcpy(m_usAffect, pUpdateScore->Affect, sizeof(m_usAffect));
    }

    if (this == g_pCurrentScene->m_pMyHuman)
    {
    }

    char oldShaow = m_cShadow;
    CheckAffect();
    UpdateScore(pUpdateScore->GuildLevel);

    if (this == g_pCurrentScene->m_pMyHuman && pFScene)
    {
        if (!m_cOnlyMove)
            SetSpeed(pFScene->m_bMountDead);
    }
    else if (!m_cOnlyMove)
    {
        SetSpeed(0);
    }

    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pMobData = &g_pObjectManager->m_stMobData;


        g_pObjectManager->m_stSelCharData.Guild[g_pObjectManager->m_cCharacterSlot] = usGuild;

        if (!usGuild)
            g_pObjectManager->m_usWarGuild = -1;

        memcpy(&pMobData->CurrentScore, &pUpdateScore->Score, sizeof(pMobData->CurrentScore));

        if (m_cMount == 1)
        {
            int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], EF_MOUNTHP);
            if (m_pMountHPBar)
                m_pMountHPBar->SetCurrentProgress(nMountHP);

            if (pFScene)
            {
                // Score packets also precede optional modern HUD binding on a
                // 7.48 field scene, so update only controls that actually exist.
                if (pFScene->m_pMHPBar)
                    pFScene->m_pMHPBar->SetCurrentProgress(nMountHP);

                if (nMountHP < 0)
                    nMountHP = 0;

                char szMHP[32]{};
                sprintf(szMHP, "%d", nMountHP);
                if (pFScene->m_pCurrentMHPText)
                    pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
            }
        }

        if (pFScene)
        {
            pFScene->SetSanc();
            pFScene->m_nReqHP = pUpdateScore->ReqHp;
            pFScene->m_nReqMP = pUpdateScore->ReqMp;
        }

        if (pFScene)
            pFScene->UpdateScoreUI(0);
    }

    if (m_cMount && oldShaow != m_cShadow)
    {
        D3DXVECTOR3 m_vOldAngle{ m_pMount->m_vAngle };

        if (m_cShadow == 1)
        {
            memset(&m_stMountSanc, 0, sizeof(m_stMountSanc));
        }
        else
        {
            m_stMountSanc.Sanc0 = m_stOldMountSanc.Sanc0;
            m_stMountSanc.Sanc4 = m_stOldMountSanc.Sanc4;
            m_stMountSanc.Legend0 = m_stOldMountSanc.Legend0;
            m_stMountSanc.Legend4 = m_stOldMountSanc.Legend4;
        }
        UpdateMount();
        m_pMount->SetAngle(m_vOldAngle);
    }

    m_c8thSkill = pUpdateScore->LearnedSkill;

    if (oldShaow == 1 && !m_cShadow)
    {
        m_stSancInfo.Sanc0 = m_stOldSancInfo.Sanc0;
        m_stSancInfo.Sanc4 = m_stOldSancInfo.Sanc4;
        m_stSancInfo.Legend0 = m_stOldSancInfo.Legend0;
        m_stSancInfo.Legend4 = m_stOldSancInfo.Legend4;

        m_stColorInfo.Sanc0 = m_stOldColorInfo.Sanc0;
        m_stColorInfo.Sanc4 = m_stOldColorInfo.Sanc4;
        m_stColorInfo.Legend0 = m_stOldColorInfo.Legend0;
        m_stColorInfo.Legend4 = m_stOldColorInfo.Legend4;

        InitObject();

        if (g_pCurrentScene->m_pMyHuman == this)
        {
            CheckWeapon(
                g_pObjectManager->m_stMobData.Equip[6].sIndex,
                g_pObjectManager->m_stMobData.Equip[7].sIndex);

            InitAngle(0, m_fAngle, 0);
        }
    }

    return 1;
}

int TMHuman::OnPacketSetHpMp(MSG_SetHpMp* pStd)
{
    if (g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return 1;

    // The coordinated client/server contract carries one uint32 layout; it is
    // distinct from the historical 20/36-byte forms of opcode 0x181.
    const unsigned int hp = pStd->Hp;
    const unsigned int mp = pStd->Mp;
    const unsigned int maxHp = pStd->MaxHp;
    const unsigned int maxMp = pStd->MaxMp;

    m_stScore.CurHP = hp;
    m_stScore.CurMP = mp;
    if (maxHp)
        m_stScore.MaxHP = maxHp;
    if (maxMp)
        m_stScore.MaxMP = maxMp;

    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pFScene->m_cAutoAttack == 1 && pFScene->m_pTargetHuman == this && hp == 0)
        pFScene->m_pTargetHuman = 0;
    auto pFScene1 = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pFScene1->m_cAutoAttack == 1 && pFScene->m_pTargetHuman == this && mp == 0)
        pFScene1->m_pTargetHuman = 0;
    if (m_stScore.CurHP >= m_stScore.MaxHP)
        m_stScore.CurHP = m_stScore.MaxHP;
    if (m_stScore.CurMP >= m_stScore.MaxMP)
        m_stScore.CurMP = m_stScore.MaxMP;

    resource_ui::Project(m_pProgressBar, m_stScore.CurHP, m_stScore.MaxHP);
    SetGuildBattleHPBar(m_stScore.CurHP);

    resource_ui::Project(m_pProgressBar1, m_stScore.CurMP, m_stScore.MaxMP);
    SetGuildBattleMPBar(m_stScore.CurMP);

    SetGuildBattleLifeCount();

    const bool isLocalHuman = pFScene->m_pMyHuman == this;
    if (isLocalHuman)
    {
        auto& localScore = g_pObjectManager->m_stMobData.CurrentScore;
        localScore.CurHP = m_stScore.CurHP;
        localScore.CurMP = m_stScore.CurMP;
        localScore.MaxHP = m_stScore.MaxHP;
        localScore.MaxMP = m_stScore.MaxMP;
    }

    // A lethal snapshot must cancel flight before its visual gate is checked;
    // otherwise the HP bar keeps the last in-flight value until another packet.
    if (death_motion::ShouldEnterDeath(m_stScore.CurHP, m_cDie == 1))
        Die();

    if (isLocalHuman && !pFScene->m_bAirMove)
    {
        // Zero maxima mean retain the entity maxima; render that resolved
        // snapshot rather than keeping the progress bars' previous scale.
        pFScene->m_nReqHP = maxHp;
        pFScene->m_nReqMP = maxMp;
        if (pFScene->m_bCompatFieldScene)
            resource_ui::ProjectNativeHpVisual(pFScene->m_pHPBar, m_stScore.CurHP, m_stScore.MaxHP);
        else
            resource_ui::Project(pFScene->m_pHPBar, m_stScore.CurHP, m_stScore.MaxHP);
        if (pFScene->m_bCompatFieldScene && pFScene->m_pControlContainer)
            resource_ui::ProjectNativeHpVisual(
                static_cast<SProgressBar*>(pFScene->m_pControlContainer->FindControl(TMP_HP_PROGRESS_TR)),
                m_stScore.CurHP, m_stScore.MaxHP);
        resource_ui::Project(pFScene->m_pMPBar, m_stScore.CurMP, m_stScore.MaxMP);

        if (m_pMountHPBar && pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
            pFScene->m_pMHPBar->SetCurrentProgress(m_pMountHPBar->GetCurrentProgress());
        if (pFScene->m_pMainCharName)
            pFScene->m_pMainCharName->SetText(m_szName, 0);
        if (pFScene->m_pCurrentHPText)
        {
            char szHP[32]{};
            sprintf(szHP, "%d", m_stScore.CurHP);
            if (pFScene->m_pCurrentHPText)
                pFScene->m_pCurrentHPText->SetText(szHP, 0);
        }
        if (pFScene->m_pMaxHPText)
        {
            char szHP[32]{};
            sprintf(szHP, resource_ui::MaximumTextFormat(pFScene->m_bCompatFieldScene), m_stScore.MaxHP);
            if (pFScene->m_pMaxHPText)
                pFScene->m_pMaxHPText->SetText(szHP, 0);
        }
        if (pFScene->m_pCurrentMPText)
        {
            char szMP[32]{};
            sprintf(szMP, "%d", m_stScore.CurMP);
            if (pFScene->m_pCurrentMPText)
                pFScene->m_pCurrentMPText->SetText(szMP, 0);
        }
        if (pFScene->m_pMaxMPText)
        {
            char szMP[32]{};
            sprintf(szMP, resource_ui::MaximumTextFormat(pFScene->m_bCompatFieldScene), m_stScore.MaxMP);
            if (pFScene->m_pMaxMPText)
                pFScene->m_pMaxMPText->SetText(szMP, 0);
        }
        if (pFScene->m_pCurrentMHPText && m_pMountHPBar)
        {
            int nHP = m_pMountHPBar->GetCurrentProgress();
            if (!m_cMount)
                nHP = 0;

            char szMHP[32]{};
            sprintf(szMHP, "%d", nHP);
            pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
        }
        if (pFScene->m_pMaxMHPText && m_pMountHPBar)
        {
            int nMaxHP = m_pMountHPBar->GetMaxProgress();
            if (!m_cMount)
                nMaxHP = 0;

            char szMHP[32]{};
            sprintf(szMHP, "%d", nMaxHP);
            pFScene->m_pMaxMHPText->SetText(szMHP, 0);
        }
    }
    if (m_stScore.CurHP > 0 && (m_cDie == 1 || m_eMotion == ECHAR_MOTION::ECMOTION_DEAD))
    {
        m_cDie = 0;
        SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);
    }

    return 1;
}

int TMHuman::OnPacketSetHpDam(MSG_STANDARD* pStd)
{
    auto pSetHpDam = (MSG_SetHpDam*)pStd;
    if (g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return 1;

    m_stScore.CurHP = pSetHpDam->Hp;
    m_pProgressBar->SetCurrentProgress(pSetHpDam->Hp);
    SetGuildBattleHPBar(m_stScore.CurHP);
    SetGuildBattleLifeCount();

    TMFont3* pFont = nullptr;
    unsigned int dwColor = 0xFFFFFF00;

    int nTX = 0;
    int nTY = 0;
    if (BASE_Get3DTo2DPos(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y, &nTX, &nTY))
    {
        char szVal[128]{};

        if (pSetHpDam->Dam < 0)
        {
            if (g_pCurrentScene->m_pMyHuman == this)
            {
                dwColor = 0xFFFF0000;

                pFont = new TMFont3(szVal,
                    nTX,
                    nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f),
                    dwColor,
                    2.0,
                    500,
                    1,
                    1500,
                    0,
                    4);
            }
            else
            {
                pFont = new TMFont3(szVal,
                    nTX,
                    nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f),
                    dwColor,
                    2.0,
                    500,
                    1,
                    1500,
                    0,
                    3);
            }
        }
        else
        {
            dwColor = 0xFF5555FF;
            if (pSetHpDam->Dam)
            {
                sprintf(szVal, "+ %d", pSetHpDam->Dam);
                pFont = new TMFont3(szVal,
                    nTX,
                    nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f),
                    dwColor,
                    2.0f,
                    500,
                    1,
                    1500,
                    0,
                    2);
            }
        }
    }

    auto pScene = g_pCurrentScene;

    BASE_Get3DTo2DPos(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y, &nTX, &nTY);
    if (pFont)
        pScene->m_pExtraContainer->AddChild(pFont);
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &m_stScore, sizeof(m_stScore));
        auto pCurrentHPText = (SText*)pScene->m_pControlContainer->FindControl(65614);
        auto pHPBar = (SProgressBar*)pScene->m_pControlContainer->FindControl(65621);
        pHPBar->SetCurrentProgress(m_stScore.CurHP);
        if (pCurrentHPText)
        {
            char szHP[32]{};
            sprintf(szHP, "%d", m_stScore.CurHP);
            pCurrentHPText->SetText(szHP, 0);
        }
    }

    return 1;
}

int TMHuman::OnPacketMessageChat(MSG_STANDARD* pStd)
{
    auto pMsgChat = reinterpret_cast<MSG_MessageChat*>(pStd);

    pMsgChat->String[sizeof(pMsgChat->String) - 2] = 0;
    pMsgChat->String[sizeof(pMsgChat->String) - 1] = 0;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (!pScene->m_pChatGeneral)
        return 0;
    if (pScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && !pScene->m_pChatGeneral->m_bSelected)
        return 1;

    float nStartXPos = 0;
    if (pStd)
        nStartXPos = 0;

    auto pChatList = pScene->m_pChatList;
    int nIndex = 0;

    if (pMsgChat->String[0] == '*')
    {

        nIndex = 1;
        m_dwChatDelayTime = 10000;
    }
    else
        m_dwChatDelayTime = 3000;

    char szMsg[128]{};

    if (strlen(pMsgChat->String) + strlen(m_szName) <= 50)
    {
        sprintf(szMsg, "[%s]> %s", m_szName, &pMsgChat->String[nIndex]);

        auto ipNewItem = new SListBoxItem(szMsg, 0xFFFFFFFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0);

        if (ipNewItem && pChatList)
            pChatList->AddItem(ipNewItem);
    }
    else
    {
        char szMsg3[128]{};
        char szMsg2[128]{};
        if (IsClearString(pMsgChat->String, 39))
        {
            strncpy(szMsg3, pMsgChat->String, 40);
            sprintf(szMsg2, "%s", &pMsgChat->String[40]);
        }
        else
        {
            strncpy(szMsg3, pMsgChat->String, 39);
            sprintf(szMsg2, "%s", &pMsgChat->String[39]);
        }

        sprintf(szMsg, "[%s]> %s", m_szName, &szMsg3[nIndex]);

        auto ipNewItem = new SListBoxItem(szMsg, 0xFFFFFFFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);

        if (ipNewItem && pChatList)
            pChatList->AddItem(ipNewItem);

        auto ipNewItem2 = new SListBoxItem(szMsg2, 0xFFFFFFFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);

        if (strlen(pMsgChat->String) > 40 && ipNewItem2 && pChatList)
            pChatList->AddItem(ipNewItem2);
    }

    sprintf(szMsg, "[%s]> %s", m_szName, &pMsgChat->String[nIndex]);
    pScene->m_dwChatTime = g_pTimerManager->GetServerTime();

    if (pMsgChat->String[0] != '=' && pMsgChat->String[0] != '-' && pMsgChat->String[0] != '@')
        SetChatMessage(&pMsgChat->String[nIndex]);

    return 1;
}

int TMHuman::OnPacketMessageChat_Index(MSG_STANDARD* pStd)
{
	return 0;
}

int TMHuman::OnPacketMessageChat_Param(MSG_STANDARD* pStd)
{
	return 0;
}

int TMHuman::OnPacketMessageWhisper(MSG_MessageWhisper* pMsg)
{
    if (!pMsg || !g_pCurrentScene || g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return 1;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    SListBox* pChatList = pScene->m_pChatList;

    pMsg->MobName[15] = 0;
    pMsg->String[sizeof(pMsg->String) - 2] = 0;
    pMsg->String[sizeof(pMsg->String) - 1] = 0;

    int nIndex = 0;
    unsigned int dwColor = 0xFFFFFF00;
    int bDrawText = 1;

    char szMsg[128]{};

    if (pMsg->String[0] == '-' && pMsg->Color == 3)
    {
        if (pScene->m_pChatGuild && !pScene->m_pChatGuild->m_bSelected)
            bDrawText = 0;

        dwColor = 0xFFAAFFFF;

        nIndex = 1;
        if (pMsg->String[1] == '-')
        {
            dwColor = 0xFF00FFFF;
            nIndex = 2;
        }

        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
    }
    else if (pMsg->Color == 7)
    {
        dwColor = 0xFFBBBBBB;
        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, pMsg->String);
    }
    else if (pMsg->String[0] == '=')
    {
        if (pScene->m_pChatParty && !pScene->m_pChatParty->m_bSelected)
            bDrawText = 0;

        // A party packet can arrive before the compatible Field resource
        // binds its list. No UI owns this line yet.
        if (!pScene->m_pPartyList)
            return 1;

        if (pScene->m_pPartyList->m_nNumItem > 1)
        {
            dwColor = 0xFFFF99FF;
            nIndex = 1;
        }
        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
    }
    else if (pMsg->String[0] == '@')
    {
        if (pMsg->String[1] == '@')
        {
            dwColor = 0xF0F60AFF;
            nIndex = 2;
        }
        else
        {
            dwColor = 0xFF00AAFF;
            nIndex = 1;
        }
        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
    }
    else if (pMsg->String[0] == '!')
    {
        if (pScene->m_pChatWhisper && !pScene->m_pChatWhisper->m_bSelected)
            bDrawText = 0;

        sprintf_s(szMsg, "[%s] : %s> %s", pMsg->MobName, m_szName, pMsg->String);

        _SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);

        if (pScene->m_pHelpList[3] != nullptr)
            pScene->m_pHelpList[3]->AddItem(new SListBoxItem(" ", 0xFFFFFFFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        sprintf_s(szMsg, g_pMessageStringTable[226], pMsg->MobName);

        char szTime[128]{};
        sprintf_s(szTime, "%s [%02d:%02d:%02d]", szMsg, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

        if (pScene->m_pHelpList[3] != nullptr)
            pScene->m_pHelpList[3]->AddItem(new SListBoxItem(szTime, 0xFFFFFFFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        sprintf_s(szMsg, "%s", &pMsg->String[1]);

        if (pScene->m_pHelpList[3] != nullptr)
            pScene->m_pHelpList[3]->AddItem(new SListBoxItem(szMsg, 0xFFFFFFCC, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        if (pScene->m_pHelpMemo)
            pScene->m_pHelpMemo->SetVisible(1);
    }
    else
    {
        if (pScene->m_pChatWhisper && !pScene->m_pChatWhisper->m_bSelected)
            bDrawText = 0;

        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, pMsg->String);
    }

    if (!bDrawText)
        return 1;

    if (strlen(pMsg->MobName) + strlen(szMsg) <= 55)
    {
        if (pChatList)
            pChatList->AddItem(new SListBoxItem(szMsg, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));
    }
    else
    {
        char szMsg2[128]{};
        char szMsg3[128]{};

        if (IsClearString(szMsg, 54))
        {
            strncpy(szMsg3, szMsg, 55);
            sprintf(szMsg2, "%s", &szMsg[55]);
        }
        else
        {
            strncpy(szMsg3, szMsg, 54);
            sprintf(szMsg2, "%s", &szMsg[54]);
        }

        if (pChatList)
            pChatList->AddItem(new SListBoxItem(szMsg3, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        if (strlen(szMsg) > 55)
        {
            if (pChatList)
                pChatList->AddItem(new SListBoxItem(szMsg2, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));
        }
    }
    return 1;
}

int TMHuman::OnPacketUpdateEtc(MSG_STANDARD* pStd)
{
    auto pUpdateEtc = reinterpret_cast<MSG_UpdateEtc*>(pStd);

    if (g_pCurrentScene->m_pMyHuman == this)
    {

        if (g_pObjectManager->m_stMobData.LearnedSkill[0] != (long)pUpdateEtc->LearnedSkill)
        {
            if (g_pSoundManager)
            {
                auto pSoundData = g_pSoundManager->GetSoundData(31);

                if (pSoundData)
                    pSoundData->Play(0, 0);
            }

            bool bChange{ false };

            for (int i = 0; i < 20; ++i)
            {
                int nBit = (unsigned char)g_pObjectManager->m_cShortSkill[i] - 24 * g_pObjectManager->m_stMobData.Class;

                if ((unsigned char)g_pObjectManager->m_cShortSkill[i] >= 96)
                    nBit = g_pObjectManager->m_cShortSkill[i] - 72;

                if (((1 << nBit) & pUpdateEtc->LearnedSkill) != 1 << nBit)
                {
                    if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 24)
                    {
                        g_pObjectManager->m_cShortSkill[i] = -1;
                        bChange = true;
                    }
                }
            }

            if (bChange)
            {
                MSG_SetShortSkill stSetShortSkill{};
                stSetShortSkill.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
                stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;

                memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));

                for (int j = 0; j < 20; ++j)
                {
                    if (stSetShortSkill.Skill[j] >= 0 && stSetShortSkill.Skill[j] < 96)
                        stSetShortSkill.Skill[j] -= 24 * g_pObjectManager->m_stMobData.Class;

                    else if (stSetShortSkill.Skill[j] >= 105 && stSetShortSkill.Skill[j] < 153)
                        stSetShortSkill.Skill[j] -= 12 * g_pObjectManager->m_stMobData.Class;
                }
                SendPacket({reinterpret_cast<MSG_STANDARD*>(&stSetShortSkill)->Type, reinterpret_cast<char*>(&stSetShortSkill), sizeof(stSetShortSkill)});
            }
        }
        // The 7.48 server has one learned-skill DWORD. The imported second
        // mask occupied the same bytes as point counters and corrupted both.
        g_pObjectManager->m_stMobData.LearnedSkill[0] = (int)pUpdateEtc->LearnedSkill;
        g_pObjectManager->m_stMobData.LearnedSkill[1] = 0;
        g_pObjectManager->m_stMobData.Exp = pUpdateEtc->Exp;
        // FUN_0052d93d updates only the three point counters from compact
        // 0x337.  Replacing the complete Score here destroyed combat fields
        // that belong exclusively to UpdateScore (0x336).
        // STRUCT_SCORE keeps the canonical server names (*Pts), while the
        // compact native packet names its WORD fields *Point.  Assign fields
        // explicitly so no alias or layout-dependent memcpy is introduced.
        m_stScore.StatusPts = pUpdateEtc->StatusPoint;
        m_stScore.MasterPts = pUpdateEtc->MasterPoint;
        m_stScore.SkillPts = pUpdateEtc->SkillPoint;
        g_pObjectManager->m_stMobData.CurrentScore.StatusPts = pUpdateEtc->StatusPoint;
        g_pObjectManager->m_stMobData.CurrentScore.MasterPts = pUpdateEtc->MasterPoint;
        g_pObjectManager->m_stMobData.CurrentScore.SkillPts = pUpdateEtc->SkillPoint;
        g_pObjectManager->m_stMobData.Coin = pUpdateEtc->Coin;
        // Hold is the native PvP death EXP debt and is paid by combat EXP.
        g_pObjectManager->m_nFakeExp = pUpdateEtc->Hold;
        g_pObjectManager->m_stSelCharData.Coin[g_pObjectManager->m_cCharacterSlot] = pUpdateEtc->Coin;

        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            static_cast<TMFieldScene*>(g_pCurrentScene)->UpdateScoreUI(0);
    }
    return 1;
}

int TMHuman::OnPacketUpdateCoin(MSG_STANDARDPARM* pStd)
{
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        g_pObjectManager->m_stMobData.Coin = pStd->Parm;
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            static_cast<TMFieldScene*>(g_pCurrentScene)->UpdateScoreUI(0);
    }

    return 1;
}

int TMHuman::OnPacketUpdateRMB(MSG_STANDARDPARM* pStd)
{
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        g_pObjectManager->m_RMBCount = pStd->Parm;
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            static_cast<TMFieldScene*>(g_pCurrentScene)->UpdateScoreUI(0);
    }

    return 1;
}

int TMHuman::OnPacketTrade(MSG_Trade* pStd)
{
	if (!pStd || !g_pObjectManager || !g_pApp || !g_pApp->m_pTimerManager ||
		!g_pCurrentScene || !g_pCurrentScene->m_pControlContainer ||
		g_pCurrentScene->m_pMyHuman != this ||
		g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
		return 1;

	auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	auto pOpCheckButton = (SButton*)g_pCurrentScene->m_pControlContainer->FindControl(TMB_TRADE_OPCHECK);
	if (pOpCheckButton)
		pOpCheckButton->SetSelected((unsigned char)pStd->MyCheck);

    if (g_pObjectManager->m_stTrade.OpponentID || !pStd->OpponentID)
    {
        bool bChanged = false;

		SGridControl* pGridOp[15]{};
		for (int i = 0; i < 15; ++i)
		{
			pGridOp[i] = (SGridControl*)pScene->m_pControlContainer->FindControl(i + TMG_TRADE_OP1);
			if (!pGridOp[i])
			{
				if (pStd->Item[i].sIndex > 0)
					bChanged = 1;
				continue;
			}
			auto pPickedItem = pGridOp[i]->PickupItem(0, 0);
			if (pPickedItem && (!pPickedItem->m_pItem ||
				memcmp(pPickedItem->m_pItem, &pStd->Item[i], sizeof(STRUCT_ITEM))))
				bChanged = 1;
            if (!pPickedItem && pStd->Item[i].sIndex > 0)
                bChanged = 1;
			if (g_pCursor && g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = 0;

            SAFE_DELETE(pPickedItem);

            if (pStd->Item[i].sIndex > 0)
            {
                auto pstItem = new STRUCT_ITEM;
                if (pstItem)
                {
                    memcpy(pstItem, &pStd->Item[i], sizeof(STRUCT_ITEM));
					auto pGridItem = new SGridControlItem(nullptr, pstItem, 0.0f, 0.0f);
					if (pGridItem)
					{
						if (!pGridOp[i]->AddItem(pGridItem, 0, 0))
							SAFE_DELETE(pGridItem);
					}
					else
						delete pstItem;
                }
            }
        }

		auto pOPGold = (SText*)pScene->m_pControlContainer->FindControl(TMT_TRADE_OPGOLD);

        char szGold[128]{};
        sprintf(szGold, "%10d", pStd->TradeMoney);
		if (bChanged == 1 || !pOPGold || strcmp(szGold, pOPGold->GetText()))
		{
			auto pMyCheck = (SButton*)pScene->m_pControlContainer->FindControl(TMB_TRADE_MYCHECK);
			auto pOtherCheck = (SButton*)pScene->m_pControlContainer->FindControl(TMB_TRADE_OPCHECK);
			if (pMyCheck)
				pMyCheck->m_bSelected = 0;
			if (pOtherCheck)
				pOtherCheck->m_bSelected = 0;

			pScene->m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
			g_pObjectManager->m_stTrade.MyCheck = pMyCheck ? pMyCheck->m_bSelected : 0;
		}

		if (pOPGold)
			pOPGold->SetText(szGold, 0);
		auto pTradePanel = pScene->m_pControlContainer->FindControl(TMP_TRADE_PANEL);
		if (pTradePanel && !pTradePanel->IsVisible())
		{
			auto pTextMyName = (SText*)pScene->m_pControlContainer->FindControl(TMT_TRADE_MYNAME);
			auto pTextOPName = (SText*)pScene->m_pControlContainer->FindControl(TMT_TRADE_OPNAME);
            auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->OpponentID);
            if (pNode)
            {
                char szMyName[128]{};
                sprintf(szMyName, "[%s]:%d", m_szName, strlen(m_szName));
                char szOPName[128]{};
                sprintf(szOPName, "[%s]:%d", pNode->m_szName, strlen(pNode->m_szName));
				if (pTextMyName)
					pTextMyName->SetText(szMyName, 1);
				if (pTextOPName)
					pTextOPName->SetText(szOPName, 1);
				pScene->SetVisibleTrade(1);
            }
        }
        return 1;
    }

    auto pOpp = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->OpponentID);
    if (pOpp)
    {
        char szMessage[128]{};
        sprintf(szMessage, g_pMessageStringTable[64], pOpp->m_szName, pOpp->m_stScore.Level + 1);
		if (pScene->m_pMessageBox)
		{
			pScene->m_pMessageBox->SetMessage(szMessage, 601, g_pMessageStringTable[28]);
			pScene->m_pMessageBox->m_dwArg = pStd->OpponentID;
			pScene->m_pMessageBox->SetVisible(1);
		}
		if (g_pCursor)
			g_pCursor->DetachItem();

		// pStd describes the remote invitation. It must never become the reusable
		// local offer, because zero-initialized CarryPos bytes are valid-looking
		// inventory slot 0 references to the authoritative server.
		memset(&g_pObjectManager->m_stTrade, 0, sizeof(g_pObjectManager->m_stTrade));
		for (int i = 0; i < 15; ++i)
			g_pObjectManager->m_stTrade.CarryPos[i] = -1;
	}

    return 1;
}

int TMHuman::OnPacketQuitTrade(MSG_STANDARD* pStd)
{
	if (!g_pCurrentScene || !g_pObjectManager)
		return 1;

	if (g_pCurrentScene->m_pMyHuman == this)
    {
        g_pObjectManager->m_stTrade.OpponentID = 0;
        g_pObjectManager->m_stTrade.MyCheck = 0;
        SGridControl::m_sLastMouseOverIndex = -1;
        // Native closure clears the model before optional field-scene UI cleanup.
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD &&
            g_pCurrentScene->m_pControlContainer)
        {
            auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
            auto pTradePanel = g_pCurrentScene->m_pControlContainer->FindControl(576);
            auto pATradePanel = pScene->m_pControlContainer->FindControl(646);
            if (pTradePanel && pTradePanel->IsVisible() == 1)
                pScene->SetVisibleTrade(0);
            if (pATradePanel && pATradePanel->IsVisible() == 1)
                pScene->SetVisibleAutoTrade(0, 0);
        }
    }
    return 1;
}

int TMHuman::OnPacketCarry(MSG_Carry* pStd)
{
	if (!g_pCurrentScene || g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
		return 1;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    // Only the local human receives the authoritative Carry cache.
	if (!pStd || !g_pObjectManager || pScene->m_pMyHuman != this)
        return 1;

	memcpy(g_pObjectManager->m_stMobData.Carry, pStd->Carry, sizeof(pStd->Carry));
	g_pObjectManager->m_stMobData.Coin = pStd->Coin;
	// Native FUN_0052E3C8 invalidates the trade before closing its UI. Clear
	// these flags even without controls, and before SetVisibleTrade can send
	// a cancellation for an otherwise active local opponent.
	g_pObjectManager->m_stTrade.OpponentID = 0;
	g_pObjectManager->m_stTrade.MyCheck = 0;
	if (pScene->m_pControlContainer)
	{
		// The native consumer closes Trade even when hidden, then invokes the
		// inventory toggle. Closure hides Carry, so the normal result is open.
		pScene->SetVisibleTrade(0);
		pScene->SetVisibleInventory();
		pScene->UpdateScoreUI(0);
	}

	// Missing presentation must not discard items, Coin, or trade invalidation.
	if (!pScene->m_pGridInv)
		return 1;

	// Empty() also detaches a cursor-owned item before deleting the old
	// presentation objects, so rebuilding the projection cannot leave a
	// dangling drag-and-drop pointer behind.
	pScene->m_pGridInv->Empty();

	// Ghidra FUN_0052a737 proves the native row-major slot formula is
	// x=slot%9, y=slot/9; keep presentation dimensions on that exact grid.
	const float cellWidth = pScene->m_pGridInv->m_nWidth / 9.0f;
	const float cellHeight = pScene->m_pGridInv->m_nHeight / 7.0f;
	for (int nCarryIndex = 0; nCarryIndex < 63; ++nCarryIndex)
	{
		if (pStd->Carry[nCarryIndex].sIndex <= 40)
			continue;

		auto pItemCarry = new STRUCT_ITEM;
		if (!pItemCarry)
			continue;

		memcpy(pItemCarry, &pStd->Carry[nCarryIndex], sizeof(STRUCT_ITEM));
		auto pGridItem = new SGridControlItem(pScene->m_pGridInv, pItemCarry, 0.0f, 0.0f);
		if (!pGridItem)
		{
			delete pItemCarry;
			continue;
		}

		// The 7.48 inventory treats every item as one structural slot and
		// scales its 3D presentation to the corresponding legacy cell.
		pGridItem->m_nCellWidth = 1;
		pGridItem->m_nCellHeight = 1;
		pGridItem->m_nWidth = cellWidth;
		pGridItem->m_nHeight = cellHeight;
		pGridItem->m_GCObj.m_fWidth = cellWidth;
		pGridItem->m_GCObj.m_fHeight = cellHeight;
		// The grid takes ownership only after accepting the cell. The logical
		// snapshot remains authoritative if the visual projection has no capacity.
		if (pScene->m_pGridInv->AddItem(pGridItem,
			nCarryIndex % 9, nCarryIndex / 9) != 1)
			SAFE_DELETE(pGridItem);
	}

	// Outside trade, snapshots remain state synchronization rather than an
	// inventory toggle; ordinary visibility stays controlled by I/menu input.
	return 1;
}

int TMHuman::OnPacketCNFCheck(MSG_STANDARD* pStd)
{
	if (!pStd || !g_pCurrentScene || !g_pCurrentScene->m_pControlContainer ||
		g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
		return 1;

	auto pMyCheckButton = (SButton*)g_pCurrentScene->m_pControlContainer->FindControl(TMB_TRADE_MYCHECK);
	if (pMyCheckButton)
		pMyCheckButton->m_bSelected = 1;
	return 1;
}

int TMHuman::OnPacketSetClan(MSG_STANDARDPARM* pStd)
{
    if (pStd->Parm == 4)
    {
        m_pNameLabel->m_GCBorder.dwColor = 0x5500AA00;
        m_pNameLabel->m_cBorder = 1;
        m_cSummons = 1;
    }
    else
    {
       m_cSummons = 0;
    }

    return 1;
}

int TMHuman::OnPacketReqRanking(MSG_STANDARDPARM2* pStd)
{
    auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->Parm1);
    if (pHuman)
    {
        const static char szVS[4][16] = {
            "1 : 1",
            "5 : 5",
            "10 : 10",
            "All : All"
        };

        char szTemp[128]{};
        const auto challengeMode = static_cast<unsigned int>(pStd->Parm2) % 4;
        sprintf(szTemp, g_pMessageStringTable[153], pHuman->m_szName, szVS[challengeMode]);

        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pScene->m_pMessageBox->SetMessage(szTemp, MSG_PlayerChallenge_Opcode, g_pMessageStringTable[154]);
        pScene->m_pMessageBox->SetVisible(1);
        pScene->m_pMessageBox->m_dwArg = pStd->Parm1;
    }

    return 1;
}

int TMHuman::OnPacketVisualEffect(MSG_STANDARD* pStd)
{
    auto pParam = reinterpret_cast<MSG_STANDARDPARM*>(pStd);
    if (pParam->Parm == 1)
    {
        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        unsigned dwLightColor = 0xFFFFFFFF;
        for (int i = -1; i < 2; ++i)
        {
            auto pEffect = new TMEffectMesh(506, dwLightColor, 0.0f, 0);
            if (pEffect)
            {
                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pEffect->m_cShine = 1;
                pEffect->m_dwCycleTime = 3000;
                pEffect->m_dwLifeTime = 8000;
                pEffect->m_fScaleH = 1.0f;
                pEffect->m_fScaleV = 2.5f;
                pEffect->m_vecPosition = TMVector3(((float)i * 0.2f) + m_vecPosition.x,
                    m_fHeight,
                    ((float)i * 0.2f) + m_vecPosition.y);

                pScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
        auto pLightShade = new TMShade(3, 118, 1.0);
        if (pLightShade)
        {
            pLightShade->m_dwLifeTime = 8000;
            pLightShade->SetColor(dwLightColor);
            pLightShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pLightShade->SetPosition(TMVector2(m_vecPosition.x, m_vecPosition.y));
            pScene->m_pEffectContainer->AddChild(pLightShade);
        }
        GetSoundAndPlay(1, 0, 0);
    }

    return 1;
}
