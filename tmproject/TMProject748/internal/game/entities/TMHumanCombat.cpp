#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "SControl.h"
#include "TMItem.h"
#include "TMScene.h"
#include "TMEffectBillBoard.h"
#include "TMEffectSkinMesh.h"
#include "TMShade.h"
#include "ObjectManager.h"
#include "TMFieldScene.h"
#include "TMEffectParticle.h"
#include "TMUtil.h"
#include "TMObjectContainer.h"
#include "TMSkillExplosion2.h"
#include "SGrid.h"
#include "TMFireEffect.h"
#include "TMEffectSpark.h"
#include "ItemEffect.h"

void TMHuman::MoveAttack(TMHuman* pTarget)
{
    if (m_dwDelayDel)
        return;

    if (!pTarget)
        return;

    if ((pTarget->m_dwID < 0 || pTarget->m_dwID >= 1000) && pTarget->IsMerchant())
        return;

    auto pFocused = g_pCurrentScene->m_pMyHuman;
    if ((m_stScore.Merchant & 0xF) == 15 && m_cMantua > 0 && pFocused->m_cMantua > 0 && m_cMantua == pFocused->m_cMantua)
        return;

    if (m_cCantAttk)
        return;

    if (pTarget->m_nClass == 66 && pTarget->m_cShadow == 1 && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        return;

    if (IsInTown() == 1 || pTarget->IsInTown() == 1)
        return;

    if ((int)m_vecPosition.x >= 2362 && (int)m_vecPosition.x <= 2370 && (int)m_vecPosition.y >= 3927 && (int)m_vecPosition.y <= 3935)
        return;

    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
        return;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pScene->m_dwLastTeleport != 0 || pScene->m_dwLastTown != 0 || pScene->m_dwLastLogout != 0 || g_dwStartQuitGameTime != 0)
        return;

    if (!TMFieldScene::m_bPK && pTarget->m_cSummons == 1 && g_pObjectManager->m_usWarGuild == pTarget->m_usGuild && pTarget->m_usGuild && g_bCastleWar)
    {
        // TODO: check this later
        int isInPos = (int)m_vecPosition.x >> 7 <= 16 || (int)m_vecPosition.x >> 7 >= 20 || (int)m_vecPosition.y >> 7 <= 29;
        if (isInPos || (m_cMantua > 0 && pTarget->m_cMantua == m_cMantua))
            return;
    }

    if (!TMFieldScene::m_bPK)
    {
        if (!g_bCastleWar && ((int)m_vecPosition.x >> 7 <= 16 || (int)m_vecPosition.x >> 7 >= 20 || (int)m_vecPosition.y >> 7 <= 29))
        {
            if (pTarget->m_usGuild && g_pObjectManager->m_usWarGuild != pTarget->m_usGuild)
                return;

            if (pScene->m_cAutoAttack == 1)
            {
                if (g_pObjectManager->m_usWarGuild > 0 && g_pObjectManager->m_usWarGuild != pTarget->m_usGuild)
                {
                    if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000)
                        return;
                }
            }
        }
        else if (m_cMantua > 0 && pTarget->m_cMantua == m_cMantua)
        {
            return;
        }
    }

    unsigned dwServerTime = g_pTimerManager->GetServerTime();
    pScene->m_pTargetHuman = pTarget;

    int nSpecForce = 0;
    if (g_pObjectManager->m_stMobData.LearnedSkill[0] & 0x20000000)
    {
        nSpecForce = 1;
    }

    if (dwServerTime <= pScene->m_dwOldAttackTime + 1000)
        return;

    int nSX = (int)m_vecPosition.x;
    int nSY = (int)m_vecPosition.y;
    int nTX = (int)pTarget->m_vecPosition.x;
    int nTY = (int)pTarget->m_vecPosition.y;
    int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
    int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);

    BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)pScene->m_HeightMapData, 8);
    if (pTarget->m_nClass == 56 && !pTarget->m_stLookInfo.FaceMesh)
    {
        nDistance -= 12;
        if (nDistance < 0)
            nDistance = 0;

        nTX = (int)pTarget->m_vecPosition.x;
        nTY = (int)pTarget->m_vecPosition.y;
    }

    auto pMobData = &g_pObjectManager->m_stMobData;
    static int nMotionIndex = 0;
    ++nMotionIndex;
    nMotionIndex %= 3;

    if (pTarget->m_cShadow == 1 && pTarget->m_nClass == 66 && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        return;

    if (nDistance <= nMobAttackRange && nTX == (int)pTarget->m_vecPosition.x && nTY == (int)pTarget->m_vecPosition.y)
    {
        MSG_Attack stAttack{};
        stAttack.Header.Type = MSG_Attack_One_Opcode;
        stAttack.Header.ID = m_dwID;
        stAttack.AttackerID = m_dwID;
        stAttack.PosX = (int)m_vecPosition.x;
        stAttack.PosY = (int)m_vecPosition.y;

        if (pScene->m_stMoveStop.NextX)
        {
            stAttack.PosX = pScene->m_stMoveStop.NextX;
            stAttack.PosY = pScene->m_stMoveStop.NextY;
        }

        stAttack.CurrentMp = -1;
        stAttack.SkillIndex = -1;
        stAttack.SkillParm = 0;
        stAttack.Motion = nMotionIndex + 4;

        if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 101)
            stAttack.SkillIndex = 151;
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

        stAttack.Dam[0].TargetID = pTarget->m_dwID;

        auto pHumanTarget = g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
        if (pHumanTarget)
        {
            int nCritical = (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.Critical;
            stAttack.Dam[0].Damage = -2;
            stAttack.Progress = TMFieldScene::m_usProgress;

            BASE_GetDoubleCritical(&g_pObjectManager->m_stMobData, 0, &TMFieldScene::m_usProgress, &stAttack.DoubleCritical);

            stAttack.TargetX = (int)pHumanTarget->m_vecPosition.x;
            stAttack.TargetY = (int)pHumanTarget->m_vecPosition.y;

            int nSize = sizeof(MSG_Attack);
            if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x200000)
            {
                stAttack.Header.Type = MSG_Attack_Two_Opcode;
                nSize = sizeof(MSG_AttackTwo);
            }
            if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x40)
            {
                int nDX = (int)(pHumanTarget->m_vecPosition.x - pScene->m_pMyHuman->m_vecPosition.x);
                int nDY = (int)(pHumanTarget->m_vecPosition.y - pScene->m_pMyHuman->m_vecPosition.y);
                if (nDX > 0)
                    nDX = 1;
                else if (nDX < 0)
                    nDX = -1;
                if (nDY > 0)
                    nDY = 1;
                else if (nDY < 0)
                    nDY = -1;

                int TX = nDX + (int)pHumanTarget->m_vecPosition.x;
                int TY = nDY + (int)pHumanTarget->m_vecPosition.y;

                auto pNode = (TMHuman*)pScene->m_pHumanContainer->m_pDown;

                while (pNode->m_pNextLink != nullptr)
                {
                    if (pNode == pScene->m_pMyHuman || pNode == pHumanTarget || (int)pNode->m_vecPosition.x != TX || (int)pNode->m_vecPosition.y != TY)
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
                        if (!TMFieldScene::m_bPK && pScene->m_pMyHuman->m_cMantua > 0 && pScene->m_pMyHuman->m_cMantua == pNode->m_cMantua && g_bCastleWar > 0)
                        {
                            pNode = (TMHuman*)pNode->m_pNextLink;
                            continue;
                        }
                        if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && pScene->m_pMyHuman->m_cMantua == 3
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

            SendOneMessage((char*)&stAttack, nSize);

            MSG_Attack stAttackLocal{};
            memcpy((char*)&stAttackLocal, (char*)&stAttack, nSize);
            stAttackLocal.Header.ID = m_dwID;
            stAttackLocal.FlagLocal = 1;
            if (nSpecForce)
                stAttackLocal.DoubleCritical |= 4;

            stAttackLocal.Progress = stAttack.Progress;
            pScene->OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
            pScene->m_dwOldAttackTime = dwServerTime;
            return;
        }
    }
    else if (pScene->m_cAutoAttack == 1 &&
        (m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_eMotion == ECHAR_MOTION::ECMOTION_STAND02) && !m_cCantMove &&
        (int)GetKeyState(16) >> 8 <= 0)
    {
        int nMoveSX = (int)m_vecPosition.x;
        int nMoveSY = (int)m_vecPosition.y;

        nTX = (int)pTarget->m_vecPosition.x;
        nTY = (int)pTarget->m_vecPosition.y;

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
                return;

            if (nMoveSX != nTX)
                nMoveSX += PlusX;

            if (nMoveSY != nTY)
                nMoveSY += PlusY;

            nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
            ++nBreak;
        }

        pScene->m_vecMyNext.x = nMoveSX;
        pScene->m_vecMyNext.y = nMoveSY;
        GetRoute(pScene->m_vecMyNext, 0, 0);
    }
}

void TMHuman::MoveGet(TMItem* pTarget)
{
    if (m_dwDelayDel)
        return;

    if (!pTarget)
        return;

    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
        return;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    unsigned int dwServerTime = g_pTimerManager->GetServerTime();
    pScene->m_pTargetItem = pTarget;

    if (dwServerTime <= pScene->m_dwGetItemTime + 1000)
        return;

    int nSX = (int)m_vecPosition.x;
    int nSY = (int)m_vecPosition.y;

    if (pScene->m_stMoveStop.NextX)
    {
        nSX = pScene->m_stMoveStop.NextX;
        nSY = pScene->m_stMoveStop.NextY;
    }

    int nTX = (int)pTarget->m_vecPosition.x;
    int nTY = (int)pTarget->m_vecPosition.y;
    int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
    BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)pScene->m_HeightMapData, 8);

    if ((pTarget->m_stItem.sIndex == 1727 && m_stScore.Level < 1000) || pTarget->m_stItem.sIndex == 359 || (pTarget->m_stItem.sIndex >= 1733 && pTarget->m_stItem.sIndex <= 1736))
        return;

    if (nDistance > 1 ||
        nTX != (int)pTarget->m_vecPosition.x || nTY != (int)pTarget->m_vecPosition.y || m_LastSendTargetPos.x != (int)m_vecPosition.x || m_LastSendTargetPos.y != (int)m_vecPosition.y)
    {
        if (m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_eMotion == ECHAR_MOTION::ECMOTION_STAND02)
        {
            short sVal = GetKeyState(16);
            if ((sVal >> 8) <= 0)
            {
                pScene->m_vecMyNext.x = (int)pTarget->m_vecPosition.x;
                pScene->m_vecMyNext.y = (int)pTarget->m_vecPosition.y;
                GetRoute(pScene->m_vecMyNext, 0, 0);
            }
        }

        return;
    }

    auto pGrid = pScene->m_pGridInv;
    int nGridIndex = BASE_GetItemAbility(&pTarget->m_stItem, 33);
    if (nGridIndex > 7 || nGridIndex < 0)
        nGridIndex = 0;

    auto vecGrid = pGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0],
        g_pItemGridXY[nGridIndex][1]);

    if (vecGrid.x <= -1 || vecGrid.y <= -1 && BASE_GetItemAbility(&pTarget->m_stItem, 38) != 2)
        return;

    if (pTarget->m_stItem.sIndex == 773 || pTarget->m_stItem.sIndex == 746)
        return;

    MSG_GetItem stGetItem{};
    stGetItem.Header.ID = m_dwID;
    stGetItem.Header.Type = MSG_GetItem_Opcode;
    stGetItem.ItemID = pTarget->m_dwID;
    stGetItem.DestType = 1;
    // Native FieldScene2 sends the destination in the single 9-column Carry
    // address space; TMProject's five-column value belongs to its paged UI.
    stGetItem.DestPos = pScene->m_bCompatFieldScene
        ? vecGrid.x + 9 * vecGrid.y
        : vecGrid.x + 5 * vecGrid.y;
    stGetItem.GridX = (int)pTarget->m_vecPosition.x;
    stGetItem.GridY = (int)pTarget->m_vecPosition.y;
    SendPacket({reinterpret_cast<MSG_STANDARD*>(&stGetItem)->Type, reinterpret_cast<char*>(&stGetItem), sizeof(stGetItem)});

    pScene->m_pTargetItem = 0;
    pScene->m_dwOldAttackTime = dwServerTime;
    pScene->m_dwGetItemTime = dwServerTime;
}

void TMHuman::Attack(ECHAR_MOTION eMotion, TMVector2 vecTarget, char cSkillIndex)
{
    if (m_dwDelayDel)
        return;

    m_vecAttTargetPos = TMVector2(0.0f, 0.0f);
    m_fTargetHeight = 0.5f;

    auto dPosition = vecTarget - m_vecPosition;

    if (m_nClass != 44)
        m_fWantAngle = atan2f(dPosition.x, dPosition.y) + D3DXToRadian(90);

    if (cSkillIndex >= 0 && cSkillIndex < 248)
    {
        m_bSkill = 1;
        m_nMotionIndex = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (m_nSkinMeshType == 1)
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act2[i + (m_cMount == 1 ? 3 : 0)] - 1);
            else
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act1[i + (m_cMount == 1 ? 3 : 0)] - 1);
        }

        m_eMotionBuffer[3] = ECHAR_MOTION::ECMOTION_NONE;
        SetAnimation(m_eMotionBuffer[m_nMotionIndex], 0);
        return;
    }

    if (eMotion != m_eMotion)
        SetAnimation(eMotion, 0);

    m_vecAttTargetPos = TMVector2((float)(m_vecPosition.x * 0.2f) + (float)(vecTarget.x * 0.80000001f),
        (float)(m_vecPosition.y * 0.2f) + (float)(vecTarget.y * 0.80000001f));
}

void TMHuman::Attack(ECHAR_MOTION eMotion, TMHuman* pTarget, short cSkillIndex)
{
    if (m_dwDelayDel)
        return;

    if (!pTarget)
        return;

    auto dPosition = pTarget->m_vecPosition - m_vecPosition;
    m_fTargetHeight = 0.5f;

    float fW = TMHuman::m_vecPickSize[pTarget->m_nSkinMeshType].x;
    float fH = TMHuman::m_vecPickSize[pTarget->m_nSkinMeshType].y;

    m_fTargetHeight = (float)(sqrtf((float)(fW * fW) + (float)(fH * fH)) * pTarget->m_fScale) * 0.30000001f;

    if (m_fTargetHeight > 2.0f)
        m_fTargetHeight = 2.0f;
    if (pTarget != this && m_nClass != 44)
        m_fWantAngle = atan2f(dPosition.x, dPosition.y) + D3DXToRadian(90);

    m_nSkillIndex = cSkillIndex;

    if (cSkillIndex >= 0 && cSkillIndex <= 103 || cSkillIndex >= 200 && cSkillIndex < 248)
    {
        m_bSkill = 1;
        m_nMotionIndex = 0;
        m_nMotionCount = 0;

        for (int i = 0; i < 4; ++i)
        {
            if (m_nSkinMeshType == 1)
            {
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act2[i + (m_cMount ? 3 : 0)] - 1);
                if ((int)m_eMotionBuffer[i] > 0)
                    ++m_nMotionCount;
            }
            else
            {
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act1[i + (m_cMount ? 3 : 0)] - 1);
                if ((int)m_eMotionBuffer[i] > 0)
                    ++m_nMotionCount;
            }
        }

        m_eMotionBuffer[3] = ECHAR_MOTION::ECMOTION_NONE;
        SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
        SetAnimation(m_eMotionBuffer[m_nMotionIndex], 0);
        m_nSkillIndex = -1;
        return;
    }

    if (eMotion != m_eMotion)
    {
        if (g_pCurrentScene->m_pMyHuman == this)
            SetAnimation(eMotion, 0);
        else if (m_nClass == 66 &&
            m_eMotion != ECHAR_MOTION::ECMOTION_ATTACK04 && m_eMotion != ECHAR_MOTION::ECMOTION_ATTACK05 && m_eMotion != ECHAR_MOTION::ECMOTION_ATTACK06 ||
            m_nClass != 66)
        {
            SetAnimation(eMotion, 0);
        }
        m_nSkillIndex = -1;
    }

    if (pTarget->m_nClass == 56 && !pTarget->m_stLookInfo.FaceMesh)
    {
        m_vecAttTargetPos = TMVector2((float)(m_vecPosition.x * 0.60000002f) + (float)(pTarget->m_vecPosition.x * 0.40000001f),
            (float)(m_vecPosition.y * 0.60000002f) + (float)(pTarget->m_vecPosition.y * 0.40000001f));
    }
    else
    {
        m_vecAttTargetPos = TMVector2((float)(m_vecPosition.x * 0.2f) + (float)(pTarget->m_vecPosition.x * 0.80000001f),
            (float)(m_vecPosition.y * 0.2f) + (float)(pTarget->m_vecPosition.y * 0.80000001f));
    }
}

void TMHuman::Punched(int nDamage, TMVector2 vecFrom, short sSkillIndex)
{
    if (m_dwDelayDel)
        return;

    if (m_eMotion != ECHAR_MOTION::ECMOTION_STRIKE)
    {
        int nBlood = GetBloodColor();
        if (m_eMotion != ECHAR_MOTION::ECMOTION_DIE)
        {
            float Tvalue = 0.2f;
            int Trand = 10;
            if (m_nClass == 66)
            {
                Tvalue = 0.8f;
                Trand = 30;
            }

            float fDam = 0.0f;
            if (m_stScore.CurHP <= 0)
                fDam = (float)nDamage;
            else
                fDam = (float)nDamage / (float)m_stScore.CurHP;

            if ((fDam > Tvalue || !(rand() % Trand) && (int)fDam > 0) && m_stScore.CurHP > 0)
            {
                if (g_pCurrentScene->m_pMyHuman == this)
                {
                    unsigned int dwTime = g_pTimerManager->GetServerTime();
                    if (((int)m_eMotion < 4 || (int)m_eMotion > 9 || !(rand() % 5)) && dwTime - m_dwStartAnimationTime > 0x258)
                    {
                        TMHuman::SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
                    }
                }
                else
                {
                    TMHuman::SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
                }
            }
            if (fDam > 0.1f)
            {
                float fSize = (float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + 1.01f;

                int nDamageTexture = (sSkillIndex >= 0 && sSkillIndex < 248) ? 120 : 119;
                auto pDamageEffect = new TMEffectBillBoard(nDamageTexture,
                    500,
                    1.5f * fSize,
                    1.5f * fSize,
                    1.5f * fSize,
                    0.002f,
                    1,
                    80);

                pDamageEffect->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y);

                if (g_pDevice->m_bSavage == 1 || g_pDevice->m_bIntel == 1)
                    pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                if (nBlood == 56)
                    pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;

                g_pCurrentScene->AddChild(pDamageEffect);
            }
        }
    }

    if (m_stScore.CurHP < 0)
        m_stScore.CurHP = 0;
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            if (pFScene->m_pMainCharName)
                pFScene->m_pMainCharName->SetText(m_szName, 0);
            if (pFScene->m_pCurrentHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "%d", m_stScore.CurHP);
                pFScene->m_pCurrentHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pMaxHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "/ %d", m_stScore.MaxHP);
                pFScene->m_pMaxHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pCurrentMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "%d", m_stScore.CurMP);
                pFScene->m_pCurrentMPText->SetText(szMP, 0);
            }
            if (pFScene->m_pMaxMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "/ %d", m_stScore.MaxMP);
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
            if (pFScene->m_pHPBar)
                pFScene->m_pHPBar->SetMaxProgress(m_stScore.MaxHP);
            if (pFScene->m_pMPBar)
            {
                pFScene->m_pMPBar->SetMaxProgress(m_stScore.MaxMP);
                pFScene->m_pMPBar->SetCurrentProgress(m_stScore.CurMP);
            }
            if (m_pMountHPBar && pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
            {
                pFScene->m_pMHPBar->SetMaxProgress(m_pMountHPBar->GetMaxProgress());
                pFScene->m_pMHPBar->SetCurrentProgress(m_pMountHPBar->GetCurrentProgress());
            }
        }
        return;
    }

    if (m_MaxBigHp)
        m_pProgressBar->SetCurrentProgress(m_BigHp);
    else
        m_pProgressBar->SetCurrentProgress(m_stScore.CurHP);

    if (m_MaxBigMp)
        m_pProgressBar1->SetCurrentProgress(m_BigMp);
    else
        m_pProgressBar1->SetCurrentProgress(m_stScore.CurMP);
    SetGuildBattleHPBar(m_stScore.CurMP);
    SetGuildBattleHPBar(m_stScore.CurHP);
    SetGuildBattleLifeCount();

    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
    {
        if (pFScene->m_pInfoText)
            pFScene->m_pInfoText->SetText(m_szName, 1);
    }
}

void TMHuman::Punched(int nDamage, TMHuman* pFrom)
{
    if (m_dwDelayDel || !pFrom)
        return;

    float fDam = 0.0f;
    if (m_stScore.CurHP <= 0)
        fDam = (float)nDamage;
    else
        fDam = (float)nDamage / (float)m_stScore.CurHP;

    int nBlood = GetBloodColor();
    if ((fDam > 0.2f || !(rand() % 10) && (int)fDam > 0) && m_stScore.CurHP > 0)
    {
        if (g_pCurrentScene->m_pMyHuman == this)
        {
            unsigned int dwTime = g_pTimerManager->GetServerTime();
            if (((int)m_eMotion < 4 || (int)m_eMotion > 9 || !(rand() % 5)) && dwTime - m_dwStartAnimationTime > 600)
            {
                SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
            }
        }
        else
        {
            SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
        }
    }
    if (fDam > 0.1f)
    {
        float fSize = (float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + 0.0099999998f;

        auto pDamageEffect = new TMEffectBillBoard(nBlood,
            600,
            0.1f * fSize,
            0.1f * fSize,
            0.1f * fSize,
            0.0049999999f,
            1,
            80);

        pDamageEffect->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y);

        if (g_pDevice->m_bSavage == 1 || g_pDevice->m_bIntel == 1)
            pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        if (nBlood == 56)
            pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;

        g_pCurrentScene->AddChild(pDamageEffect);
    }

    if (m_stScore.CurHP < 0)
        m_stScore.CurHP = 0;
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            if (pFScene->m_pMainCharName)
                pFScene->m_pMainCharName->SetText(m_szName, 0);
            if (pFScene->m_pCurrentHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "%d", m_stScore.CurHP);
                pFScene->m_pCurrentHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pMaxHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "/ %d", m_stScore.MaxHP);
                pFScene->m_pMaxHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pCurrentMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "%d", m_stScore.CurMP);
                pFScene->m_pCurrentMPText->SetText(szMP, 0);
            }
            if (pFScene->m_pMaxMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "/ %d", m_stScore.MaxMP);
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
            if (pFScene->m_pHPBar)
                pFScene->m_pHPBar->SetMaxProgress(m_stScore.MaxHP);
            if (pFScene->m_pMPBar)
            {
                pFScene->m_pMPBar->SetMaxProgress(m_stScore.MaxMP);
                pFScene->m_pMPBar->SetCurrentProgress(m_stScore.CurMP);
            }
            if (m_pMountHPBar && pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
            {
                pFScene->m_pMHPBar->SetMaxProgress(m_pMountHPBar->GetMaxProgress());
                pFScene->m_pMHPBar->SetCurrentProgress(m_pMountHPBar->GetCurrentProgress());
            }
        }
        return;
    }

    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
    {
        m_pProgressBar->SetCurrentProgress(m_stScore.CurHP);
        m_pProgressBar1->SetCurrentProgress(m_stScore.CurMP);
        SetGuildBattleHPBar(m_stScore.CurHP);
        SetGuildBattleHPBar(m_stScore.CurMP);
        SetGuildBattleLifeCount();

        if (pFScene->m_pInfoText)
            pFScene->m_pInfoText->SetText(m_szName, 1);
    }
}

void TMHuman::Fire(TMObject* pTarget, int nSkill)
{
    if (m_dwDelayDel)
        return;

    if (m_vecTempPos[0].x == 0.0f && m_vecTempPos[0].y == 0.0f && m_vecTempPos[0].z == 0.0f)
    {
        m_vecTempPos[0].x = m_vecPosition.x;
        m_vecTempPos[0].y = m_fHeight + 1.5f;
        m_vecTempPos[0].z = m_vecPosition.y;
    }

    if (nSkill == 0)
    {
        if (m_nSkinMeshType == 20 && (!m_stLookInfo.HelmMesh || m_stLookInfo.HelmMesh == 2) || m_nClass == 35)
        {
            auto pBreathFire = new TMFireEffect(m_vecTempPos[0], pTarget, 42);

            m_dwBreathStartTime = pBreathFire->m_dwCreateTime;
            m_dwBreathLifeTime = pBreathFire->m_dwLifeTime;

            g_pCurrentScene->AddChild(pBreathFire);
        }
    }
    else if (nSkill == 1)
    {
        auto pEffect = new TMEffectSpark(m_vecTempPos[0], pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFFFF3300, 0xFF551100, 1000, 1.0f, 5, 0.0);
        pEffect->m_fRange = 0.5f;

        g_pCurrentScene->AddChild(pEffect);
    }
}

void TMHuman::Die()
{
    if (m_dwDelayDel)
        return;

    if (g_pCurrentScene->m_pMyHuman == this && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pScene->m_pTargetHuman = 0;
        // Discard the visual flight before freezing the death position.
        if (pScene->m_bAirMove)
            pScene->AirMove_End(TMFieldScene::AirMoveEndReason::Death);
    }

    if ((int)m_wAttackerID > 0)
    {
        auto pAttacker = g_pObjectManager->GetHumanByID(m_wAttackerID);
        if (pAttacker)
        {
            if (pAttacker->m_cLifeDrain == 1)
            {
                // Dead code aparently
                /*
                 TMVector3 vecStart{ m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y };
                 TMEffectBillBoard* pFire = nullptr;
                 */
            }
        }
    }

    if (m_cDie == 1)
        return;

    // A late emote response is ignored while dead. Clear the outstanding
    // request now so revival cannot inherit a permanently blocked input.
    m_SendeMotion = ECHAR_MOTION::ECMOTION_NONE;

    // Death freezes the current position. Leaving an unfinished route active
    // lets FrameMove advance it and can restore movement over the death clip.
    for (auto& routePoint : m_vecRouteBuffer)
        routePoint = m_vecPosition;
    m_nMaxRouteIndex = 0;
    m_nLastRouteIndex = 0;
    m_fProgressRate = 0.0f;
    m_bMoveing = 0;
    m_cOnlyMove = 0;
    m_bSliding = 0;
    m_vecStartPos.x = static_cast<int>(m_vecPosition.x);
    m_vecStartPos.y = static_cast<int>(m_vecPosition.y);
    m_pMoveTargetHuman = nullptr;
    m_pMoveSkillTargetHuman = nullptr;
    m_dwStartMoveTime = g_pTimerManager->GetServerTime();

    SetAnimation(ECHAR_MOTION::ECMOTION_DIE, 0);
    // A rejected mesh clip must not leave the logical state in RUN. The
    // one-shot completion path also owns the respawn prompt.
    m_eMotion = ECHAR_MOTION::ECMOTION_DIE;
    m_nLoop = 0;
    m_dwStartAnimationTime = g_pTimerManager->GetServerTime();

    if (m_nClass == 44)
    {
        if (auto* effectContainer = g_pCurrentScene->m_pEffectContainer)
        {
            TMVector3 vecPos{ m_vecPosition.x, m_fHeight + 2.0f, m_vecPosition.y };
            effectContainer->AddChild(new TMEffectParticle(vecPos, 4, 12, 0.05f,
                0xFFFFAA00, 0, 56, 1.0, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000));

            constexpr unsigned int dwColor = 0x44444444;
            effectContainer->AddChild(new TMSkillExplosion2(vecPos, 0, 1.0f, 210, dwColor));

            auto pBill = new TMEffectBillBoard(59, 2500, 0.2f, 0.2f, 0.2f, 0.003f, 1, 80);
            pBill->m_vecStartPos = pBill->m_vecPosition = vecPos;
            pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_DEFAULT;
            if (g_pDevice->m_bSavage == 1 || g_pDevice->m_bIntel == 1)
                pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pBill->SetColor(0xFFFFFFFF);
            effectContainer->AddChild(pBill);
        }

        GetSoundAndPlay(309, 0, 0);
    }

    m_cPoison = 0;
    m_cHaste = 0;
    m_cAssert = 0;
    m_cFreeze = 0;
    m_cSlowSlash = 0;
    m_cSpeedUp = 0;
    m_cSpeedDown = 0;
    m_cShield = 0;
    m_cCancel = 0;
    m_cAurora = 0;
    m_cWeapon = 0;
    m_cSKillAmp = 0;
    m_cLighten = 0;
    m_cWaste = 0;

    SetAvatar(0);
    if (m_pInMiniMap)
        m_pInMiniMap->SetVisible(0);

    if (g_pCurrentScene->m_pMyHuman == this && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pFScene->m_dwLastDeadTime = g_pTimerManager->GetServerTime();
    }

    m_cSummons = 0;
    m_cDie = 1;
    m_vecTempPos[0].x = m_vecPosition.x;
    m_vecTempPos[0].y = m_fHeight + 1.5f;
    m_vecTempPos[0].z = m_vecPosition.y;

    g_pObjectManager->DeleteObject(m_pShade);
    m_pShade = nullptr;

    if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
        StartKhepraDieEffect();
}

void TMHuman::Stand()
{
    if (m_dwDelayDel)
        return;

    char szBuffer[48]{};
    int nStartRouteIndex = m_nLastRouteIndex + 1;
    if (m_fProgressRate > 0.5f)
        nStartRouteIndex = m_nLastRouteIndex + 2;

    int nX = (int)m_vecRouteBuffer[nStartRouteIndex].x;
    int nY = (int)m_vecRouteBuffer[nStartRouteIndex].y;
    for (int i = nStartRouteIndex; i < 48; ++i)
    {
        m_vecRouteBuffer[i].x = (float)nX;
        m_vecRouteBuffer[i].y = (float)nY;
    }
}

void TMHuman::PlayAttackSound(ECHAR_MOTION eMotion, int nLR)
{
    if (m_dwDelayDel)
        return;

    STRUCT_ITEM itemL{};
    STRUCT_ITEM itemR{};

    itemL.sIndex = m_sLeftIndex;
    itemR.sIndex = m_sRightIndex;
    BASE_GetItemAbility(&itemL, 17);

	unsigned int nWeaponPosR = BASE_GetItemAbility(&itemR, 17);
    int nSoundIndex = 121;

    if (m_nSkinMeshType == 3 || m_nClass == 40)
    {
        if (!nLR && m_nWeaponTypeL || nLR == 1 && nWeaponPosR && nWeaponPosR != 128)
        {
            if (g_pSoundManager)
            {
                auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
                if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sAttackLR != nLR))
                {
                    pSoundData->Play(0, 0);
                }
            }
        }
        m_sAttackLR = nLR;
        return;
    }

    if (m_nWeaponTypeL == 1 && !nLR || m_nWeaponTypeR == 1 && nLR == 1)
    {
        if (g_pItemList[m_sLeftIndex].nReqLvl > 90 && !nLR || g_pItemList[m_sRightIndex].nReqLvl > 90 && !nLR)
        {
            if (!((int)eMotion % 2))
                nSoundIndex = 124;
            else if ((int)eMotion % 2 == 1)
                nSoundIndex = 125;
        }
        else if (!((signed int)eMotion % 2))
            nSoundIndex = 121;
        else if ((signed int)eMotion % 2 == 1)
            nSoundIndex = 122;
    }
    else if (m_nWeaponTypeL == 2 && !nLR || m_nWeaponTypeR == 2 && nLR == 1)
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 124;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 125;
    }
    else if (m_nWeaponTypeL == 3 && !nLR || m_nWeaponTypeR == 3 && nLR == 1)
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 126;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 127;
    }
    else if (!nLR
        && (m_nWeaponTypeL == 11
            || m_nWeaponTypeL == 12
            || m_nWeaponTypeL == 31
            || m_nWeaponTypeL == 32
            || m_nWeaponTypeL == 61
            || m_nWeaponTypeL == 62)
        || nLR == 1
        && (m_nWeaponTypeR == 11
            || m_nWeaponTypeR == 12
            || m_nWeaponTypeR == 31
            || m_nWeaponTypeR == 32
            || m_nWeaponTypeR == 61
            || m_nWeaponTypeR == 62))
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 128;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 129;
    }
    else if (m_nWeaponTypeL == 13 || m_nWeaponTypeL == 33 || m_nWeaponTypeL == 63)
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 131;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 132;
    }
    else
    {
        if (m_nWeaponTypeL == 101)
            return;
        if (m_nWeaponTypeL == 41)
        {
            if (!((int)eMotion % 2))
                nSoundIndex = 135;
            else if ((int)eMotion % 2 == 1)
                nSoundIndex = 136;
        }
        else if (m_nWeaponTypeL == 21 || m_nWeaponTypeL == 22 || m_nWeaponTypeL == 23)
        {
            if (!((int)eMotion % 2))
                nSoundIndex = 137;
            else if ((int)eMotion % 2 == 1)
                nSoundIndex = 138;
        }
        else if (m_nWeaponTypeL == 102)
            nSoundIndex = 139;
        else if (m_nWeaponTypeL != 103 && m_nWeaponTypeL != 104)
            nSoundIndex = 121;
        else
            nSoundIndex = 140;
    }

    if (!nLR && m_nWeaponTypeL || nLR == 1 && nWeaponPosR && nWeaponPosR != 128)
    {
        if (g_pSoundManager)
        {
            auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
            if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sAttackLR != nLR))
            {
                pSoundData->Play(0, 0);
            }
        }
    }

    m_sAttackLR = nLR;
}

void TMHuman::PlayPunchedSound(int nType, int nLR)
{
    if (m_dwDelayDel)
        return;

    int nSoundIndex = 27;
    unsigned int dwServerTime = g_pTimerManager->GetServerTime();
    if (m_nSkinMeshType == 3 || m_nClass == 40)
    {
        if (g_pSoundManager)
        {
            auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
            if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sPunchLR != nLR || dwServerTime > m_dwLastPlayPunchedTime + 400))
            {
                pSoundData->Play(0, 0);
                m_dwLastPlayPunchedTime = dwServerTime;
            }
        }

        m_sPunchLR = nLR;
        return;
    }

    if (nType == 1)
        nSoundIndex = 21;
    if (nType == 2)
        nSoundIndex = 22;
    if (nType == 3)
        nSoundIndex = 23;
    if (nType == 11 || nType == 31 || nType == 61)
        nSoundIndex = 25;
    if (nType == 12 || nType == 32 || nType == 62)
        nSoundIndex = 26;
    if (nType == 13 || nType == 33 || nType == 63)
        nSoundIndex = 27;
    if (nType == 21 || nType == 22 || nType == 23 || nType == 41)
        nSoundIndex = 28;
    if (nType > 100 && nType < 105)
        nSoundIndex = 24;

    if (g_pSoundManager)
    {
        if (nSoundIndex != 24)
        {
            auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
            if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sPunchLR != nLR || dwServerTime > m_dwLastPlayPunchedTime + 400))
            {
                pSoundData->Play(0, 0);
                m_dwLastPlayPunchedTime = dwServerTime;
            }
        }
        m_sPunchLR = nLR;
    }
}

int TMHuman::MAutoAttack(TMHuman* pTarget, int mode)
{
    if (_dwAttackDelay)
    {
        if (g_pTimerManager->GetServerTime() - _dwAttackDelay > 2000)
            _dwAttackDelay = 0;
        return 0;
    }
    if (!pTarget)
        return 0;
    if (pTarget->m_nClass == 44 && pTarget->m_sHeadIndex == 219)
        return 0;
    if (m_cMantua && pTarget->m_cMantua == m_cMantua)
        return 0;
    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
    {
        g_GameAuto = 0;
        return 0;
    }
    if (pTarget->m_cDie == 1)
        return 0;
    if (pTarget->m_cHide == 1)
        return 0;
    if (m_dwDelayDel)
        return 0;
    if (!pTarget)
        return 0;
    if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000 == 1)
        return 0;
    if (pTarget->IsMerchant())
        return 0;
    if (pTarget->m_cSummons == 1)
        return 0;
    if ((m_stScore.Merchant & 0xF) == 15)
    {
        auto pFocused = g_pCurrentScene->m_pMyHuman;
        if (m_cMantua > 0 && pFocused->m_cMantua > 0 && m_cMantua == pFocused->m_cMantua)
            return 0;
    }

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    unsigned int dwServerTime = g_pTimerManager->GetServerTime();
    auto pMobData = &g_pObjectManager->m_stMobData;
    int nSpecForce = 0;
    if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
        nSpecForce = 1;
    if (dwServerTime < pScene->m_dwOldAttackTime + 1000)
        return 0;

    int nSX = (int)m_vecPosition.x;
    int nSY = (int)m_vecPosition.y;
    int nTX = (int)pTarget->m_vecPosition.x;
    int nTY = (int)pTarget->m_vecPosition.y;
    int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
    int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);
    BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)pScene->m_HeightMapData, 8);
    m_cMount;

    static int nMotionIndex = 0;
    ++nMotionIndex;
    nMotionIndex %= 3;

    if (nDistance < 0 || nDistance > 15)
        return 0;
    if (!nTX || !nTY)
        return 0;

    if (nDistance <= nMobAttackRange &&
        nTX == (int)pTarget->m_vecPosition.x && nTY == (int)pTarget->m_vecPosition.y &&
        m_LastSendTargetPos.x == (int)m_vecPosition.x && m_LastSendTargetPos.y == (int)m_vecPosition.y)
    {
        if (pTarget->m_cShadow == 1 && pTarget->m_nClass == 66 && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
            return 0;

        MSG_Attack stAttack{};
        stAttack.Header.Type = MSG_Attack_One_Opcode;
        stAttack.Header.ID = m_dwID;
        stAttack.AttackerID = m_dwID;
        stAttack.PosX = (int)m_vecPosition.x;
        stAttack.PosY = (int)m_vecPosition.y;
        if (pScene->m_stMoveStop.NextX)
        {
            stAttack.PosX = pScene->m_stMoveStop.NextX;
            stAttack.PosY = pScene->m_stMoveStop.NextY;
        }
        stAttack.CurrentMp = -1;
        stAttack.SkillIndex = -1;
        stAttack.SkillParm = 0;
        stAttack.Motion = nMotionIndex + 4;

        if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 101)
            stAttack.SkillIndex = 151;
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
        stAttack.Dam[0].TargetID = pTarget->m_dwID;

        auto pTargetHuman = (TMHuman*)g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
        if (!pTargetHuman)
            return 0;

        int nCritical = (unsigned char)g_pObjectManager->m_stMobData.CurrentScore.Critical;
        stAttack.Dam[0].Damage = -2;
        stAttack.Progress = TMFieldScene::m_usProgress;
        BASE_GetDoubleCritical(&g_pObjectManager->m_stMobData, 0, &TMFieldScene::m_usProgress, &stAttack.DoubleCritical);
        stAttack.TargetX = (int)pTargetHuman->m_vecPosition.x;
        stAttack.TargetY = (int)pTargetHuman->m_vecPosition.y;

        int nSize = sizeof(MSG_AttackOne);
        if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x200000)
        {
            stAttack.Header.Type = MSG_Attack_Two_Opcode;
            nSize = sizeof(MSG_AttackTwo);
        }

        if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x40)
        {
            int nDX = (int)(pTargetHuman->m_vecPosition.x - pScene->m_pMyHuman->m_vecPosition.x);
            int nDY = (int)(pTargetHuman->m_vecPosition.y - pScene->m_pMyHuman->m_vecPosition.y);
            if (nDX > 0)
                nDX = 1;
            else if (nDX < 0)
                nDX = -1;
            if (nDY > 0)
                nDY = 1;
            else if (nDY < 0)
                nDY = -1;

            int TX = nDX + (int)pTargetHuman->m_vecPosition.x;
            int TY = nDY + (int)pTargetHuman->m_vecPosition.y;
            auto pNode = (TMHuman*)pScene->m_pHumanContainer->m_pDown;

            while (pNode->m_pNextLink)
            {
                if (pNode == pScene->m_pMyHuman || pNode == pTargetHuman || (int)pNode->m_vecPosition.x != TX || (int)pNode->m_vecPosition.y != TY)
                {
                    pNode = (TMHuman*)pNode->m_pNextLink;
                    continue;
                }

                if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
                {
                    if (!pNode->IsInPKZone() || pNode->m_bParty == 1)
                    {
                        pNode = (TMHuman*)pNode->m_pNextLink;
                        continue;
                    }

                    if (!TMFieldScene::m_bPK)
                    {
                        if (!g_bCastleWar)
                        {
                            if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }
                        }
                        else
                        {
                            if (m_cMantua > 0 && m_cMantua == pNode->m_cMantua)
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }
                            if (m_cMantua == 3 && (pNode->m_dwID > 0 && pNode->m_dwID < 1000) || pNode->m_cMantua > 0 && pNode->m_cMantua != 4)
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }

                            if (!pNode->IsInCastleZone())
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }
                        }
                    }
                }
                else
                {
                    if (!TMFieldScene::m_bPK && pNode->m_cSummons == 1 && (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild) || !pNode->m_usGuild)
                    {
                        pNode = (TMHuman*)pNode->m_pNextLink;
                        continue;
                    }
                }

                stAttack.Dam[1].TargetID = pNode->m_dwID;
                stAttack.Dam[1].Damage = -2;
                break;
            }

            if (!(pMobData->LearnedSkill[0] & 0x200000) && stAttack.Dam[1].Damage != -2)
            {
                stAttack.Header.Type = MSG_Attack_One_Opcode;
                nSize = sizeof(MSG_AttackOne);
            }
            else
            {
                stAttack.Header.Type = MSG_Attack_Two_Opcode;
                nSize = sizeof(MSG_AttackTwo);
            }
        }

        SendOneMessage((char*)&stAttack, nSize);

        MSG_Attack stAttackLocal{};
        memcpy((char*)&stAttackLocal, (char*)&stAttack, nSize);
        stAttackLocal.Header.ID = pScene->m_dwID;
        stAttackLocal.FlagLocal = 1;
        if (pMobData->LearnedSkill[0] & 0x20000000)
            stAttackLocal.DoubleCritical |= 4;
        pScene->OnPacketEvent(MSG_Attack_One_Opcode, (char*)&stAttackLocal);
        pScene->m_dwOldAttackTime = dwServerTime;
        return 1;
    }

    if (!mode)
        return 0;
    if ((GetKeyState(VK_SHIFT) >> 8) > 0)
        return 0;

    int nMoveSX = (int)m_vecPosition.x;
    int nMoveSY = (int)m_vecPosition.y;

    nTX = (int)pTarget->m_vecPosition.x;
    nTY = (int)pTarget->m_vecPosition.y;

    int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);

    int PlusX = 1;
    int PlusY = 1;
    if (nSX > nTX)
        PlusX = -1;
    if (nSY > nTY)
        PlusY = -1;

    while (nMoveDistance2 > nMobAttackRange)
    {
        if (nMoveSX != nTX)
            nMoveSX += PlusX;
        if (nMoveSY != nTY)
            nMoveSY += PlusY;
        nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
    }

    pScene->m_vecMyNext.x = nMoveSX;
    pScene->m_vecMyNext.y = nMoveSY;
    GetRoute(pScene->m_vecMyNext, 0, 0);
    return 2;
}
