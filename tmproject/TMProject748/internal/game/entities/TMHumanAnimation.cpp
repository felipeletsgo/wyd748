#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "TMScene.h"
#include "TMEffectBillBoard.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectMesh.h"
#include "TMEffectMeshRotate.h"
#include "TMEffectSkinMesh.h"
#include "TMShade.h"
#include "SkinMotionPolicy.h"
#include "TMEffectSWSwing.h"
#include "ObjectManager.h"
#include "TMCamera.h"
#include "TMFieldScene.h"
#include "TMEffectParticle.h"
#include "TMUtil.h"
#include "TMObjectContainer.h"
#include "ItemEffect.h"

void TMHuman::SetAnimation(ECHAR_MOTION eMotion, int nLoop)
{
    if (!m_dwDelayDel && eMotion != ECHAR_MOTION::ECMOTION_NONE && m_pSkinMesh)
    {
        if (eMotion == ECHAR_MOTION::ECMOTION_LEVELUP && (m_nClass == 34 || m_nClass == 38))
            eMotion = ECHAR_MOTION::ECMOTION_STAND02;

        if ((eMotion == ECHAR_MOTION::ECMOTION_HOLYTOUCH || eMotion == ECHAR_MOTION::ECMOTION_RELAX)
            && (m_nSkinMeshType == 3 || m_nSkinMeshType == 7 || m_nSkinMeshType == 25 || m_nSkinMeshType == 28))
        {
            eMotion = ECHAR_MOTION::ECMOTION_LEVELUP;
        }
        if (m_nSkinMeshType == 8 && eMotion == ECHAR_MOTION::ECMOTION_HOLYTOUCH)
        {
            eMotion = ECHAR_MOTION::ECMOTION_ATTACK01;
            nLoop = 0;
        }
        if (IsInTown() == 1 && eMotion == ECHAR_MOTION::ECMOTION_STAND02 && !m_nWeaponTypeIndex)
            eMotion = ECHAR_MOTION::ECMOTION_STAND01;

        if (eMotion == ECHAR_MOTION::ECMOTION_STAND01 || eMotion == ECHAR_MOTION::ECMOTION_STAND02)
        {
            m_bSliding = 0;
            if (m_cMantua > 0 && m_pMantua)
            {
                if (m_cMount <= 0)
                    m_pMantua->SetAnimation(0);
                else
                    m_pMantua->SetAnimation(3);
            }
        }

        eMotion = skin_motion::Remap(m_nSkinMeshType, eMotion);
        if (m_nSkinMeshType == 21 && (int)m_stLookInfo.FaceMesh > 1 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
            eMotion = ECHAR_MOTION::ECMOTION_RUN;
        if (m_nSkinMeshType == 24 && m_bParty == 1 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
            eMotion = ECHAR_MOTION::ECMOTION_RUN;
        if (m_nSkinMeshType == 20 && m_stLookInfo.FaceMesh && m_stLookInfo.HelmMesh != 2 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
            eMotion = ECHAR_MOTION::ECMOTION_WALK;

        if (m_nSkinMeshType == 20 && m_stLookInfo.FaceMesh == 4 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
        {
            m_pSkinMesh->m_dwFPS = (int)(float)(34.0f - m_fMaxSpeed) / 2;
            m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fScale);
        }
        if (m_nSkinMeshType == 20 && (eMotion == ECHAR_MOTION::ECMOTION_WALK || eMotion == ECHAR_MOTION::ECMOTION_RUN)
            && m_stLookInfo.FaceMesh == 7)
            m_pSkinMesh->m_dwFPS = 6;

        if (m_nSkinMeshType == 0 || m_nSkinMeshType == 1 || m_nSkinMeshType == 2 || m_nSkinMeshType == 3
         || m_nSkinMeshType == 4 || m_nSkinMeshType == 5)
        {
            if (m_cMount == 1)
            {
                if (m_pMount)
                {
                    if (m_nMountSkinMeshType == 20 && (eMotion == ECHAR_MOTION::ECMOTION_WALK || eMotion == ECHAR_MOTION::ECMOTION_RUN) && m_pMount->m_Look.Mesh0 == 7)
                        m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[2]);
                    else if ((signed int)eMotion >= 4 && (signed int)eMotion <= 9)
                        m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[1]);
                    else
                        m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[(int)eMotion]);
                }
                if (m_sHeadIndex < 40 && (m_sHeadIndex % 10 == 1 || m_sHeadIndex % 10 > 5))
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTableEx[nClass][m_nSkinMeshType].dwAniTable[(int)eMotion + 28]]);
                }
                else if (m_nSkinMeshType == 3)
                    m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion + 28]);
                else
                    m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion + 28]]);
            }
            else
            {
                int bExt = 0;
                if (m_sHeadIndex < 40 && (m_sHeadIndex % 10 == 1 || m_sHeadIndex % 10 > 5))
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    if (!m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTableEx[nClass][m_nSkinMeshType].dwAniTable[(int)eMotion]])
                        && !g_MobAniTableEx[nClass][m_nSkinMeshType].dwAniTable[(int)eMotion])
                    {
                        m_eMotion = eMotion;
                        return;
                    }
                }
                else if (!m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion]])
                    && !g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion])
                {
                    m_eMotion = eMotion;
                    return;
                }
            }

            m_eMotion = eMotion;

            if (eMotion == ECHAR_MOTION::ECMOTION_RUN)
            {
                m_pSkinMesh->m_dwFPS = (int)(27.0f - m_fMaxSpeed) / 2;
                m_pSkinMesh->m_dwFPS = (unsigned int)((float)m_pSkinMesh->m_dwFPS * m_fScale);
                if (m_cMantua > 0 && m_pMantua)
                {
                    if (m_cMount <= 0)
                        m_pMantua->SetAnimation(2);
                    else
                        m_pMantua->SetAnimation(3);
                }

                if (m_cMount == 1 && m_pMount)
                {
                    if (m_nMountSkinMeshType != 40)
                        m_pMount->m_dwFPS = (int)(float)(27.0f - m_fMaxSpeed) / 2;

                    switch (m_nMountSkinMeshType)
                    {
                    case 20:
                        m_pMount->m_dwFPS = 20;
                        break;
                    case 39:
                        m_pMount->m_dwFPS = 20;
                        break;
                    case 48:
                        m_pMount->m_dwFPS = 3;
                        break;
                    case 49:
                        m_pMount->m_dwFPS = 6;
                        break;
                    case 52:
                        m_pMount->m_dwFPS = 6;
                        break;
                    case 50:
                        m_pMount->m_dwFPS = 6;
                        break;
                    }
                }

                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 0;
            }
            else if (eMotion == ECHAR_MOTION::ECMOTION_WALK)
            {
                if (m_cMantua > 0 && m_pMantua)
                {
                    if (m_cMount <= 0)
                        m_pMantua->SetAnimation(1);
                    else
                        m_pMantua->SetAnimation(3);
                }

                if (m_nSkinMeshType == 2)
                    m_pSkinMesh->m_dwFPS = (signed int)(float)(40.0f - (float)(m_fMaxSpeed * 3.0f)) / 2;
                else
                    m_pSkinMesh->m_dwFPS = (signed int)(float)(36.0f - (float)(m_fMaxSpeed * 3.0f)) / 2;

                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fScale);

                if (m_cMount == 1 && m_pMount)
                {
                    m_pMount->m_dwFPS = (signed int)(float)(32.0f - (float)(m_fMaxSpeed * 3.0f)) / 2;
                    m_pMount->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fMountScale);
                }
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 0;
            }
            else
            {
                if (m_cMantua > 0 && m_pMantua)
                {
                    if (m_cMount <= 0)
                        m_pMantua->SetAnimation(1);
                    else
                        m_pMantua->SetAnimation(3);
                }

                unsigned int dwSpeedTemp = 0;
                if (m_sHeadIndex >= 40 || m_sHeadIndex % 10 != 1 && m_sHeadIndex % 10 <= 5)
                    dwSpeedTemp = g_MobAniTable[m_nSkinMeshType].dwSpeed[(int)eMotion];
                else
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    dwSpeedTemp = g_MobAniTableEx[nClass][m_nSkinMeshType].dwSpeed[(int)eMotion];
                }

                m_pSkinMesh->m_dwFPS = (unsigned int)((float)dwSpeedTemp * 1.0f);
                if (m_cMount == 1 && m_pMount)
                    m_pMount->m_dwFPS = (unsigned int)((float)g_MobAniTable[m_nMountSkinMeshType].dwSpeed[(int)eMotion] * 1.0f);
            }

            if ((int)eMotion >= 4 && (int)eMotion <= 9)
            {
                float fEffectLen = 1.0f;
                if (m_cMount == 1)
                    fEffectLen = 1.2f;

                unsigned int dwSpeedTemp = 0;
                if (m_sHeadIndex >= 40 || m_sHeadIndex % 10 != 1 && m_sHeadIndex % 10 <= 5)
                    dwSpeedTemp = g_MobAniTable[m_nSkinMeshType].dwSpeed[(int)eMotion];
                else
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    dwSpeedTemp = g_MobAniTableEx[nClass][m_nSkinMeshType].dwSpeed[(int)eMotion];
                }

                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)dwSpeedTemp * 1.0f);
                if (m_bSwordShadow[0] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[0])
                    {
                        if (m_nWeaponTypeL == 41 && m_pSkinMesh->m_pSwingEffect[1])
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                        else
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[0] * fEffectLen;
                    }
                    if (m_pSkinMesh->m_pSwingEffect[0])
                        m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }
                if (m_bSwordShadow[1] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }

                float fSpeed = 1.0f;
                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 1.0f);
                if (m_cFreeze == 1)
                    m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * 1.15f);
                if (m_nMotionCount > 1)
                    m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 1.2f);
                else if (m_bDoubleAttack == 1)
                {
                    if (m_nWeaponTypeL == 101)
                        m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 2.0f);
                    else
                        m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 1.7f);
                }
            }
            else
            {
                m_nMotionCount = 0;
            }
            if (eMotion == ECHAR_MOTION::ECMOTION_LEVELUP)
            {
                if (m_nSkinMeshType == 2 && m_nWeaponTypeIndex == 4)
                     m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * 1.6f);

                if ((TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject == this
                    && m_cMount == 1
                    && m_nMountSkinMeshType == 31
                    && g_pSoundManager
                    && g_pSoundManager->GetSoundData(279))
                {
                    g_pSoundManager->GetSoundData(279)->Play();
                }
                if ((TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject == this)
                {
                    int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[14];
                    if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                        nSoundIndex = 225;
                    if (g_pSoundManager && g_pSoundManager->GetSoundData(nSoundIndex))
                        g_pSoundManager->GetSoundData(nSoundIndex)->Play();
                }
                else if (g_pSoundManager != nullptr && g_pSoundManager->GetSoundData(158))
                {
                    g_pSoundManager->GetSoundData(158)->Play();
                }
            }
        }
        else
        {
            int nBaseValue = 30;
            if (m_nSkinMeshType == 31)
                nBaseValue = 38;
            else if (m_nSkinMeshType == 39)
                nBaseValue = 40;
            else if (m_nSkinMeshType == 38)
                nBaseValue = 38;
            else if (m_nSkinMeshType == 30)
                nBaseValue = 40;
            else if (m_nSkinMeshType == 21 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
                nBaseValue = 24;
            else if (m_nSkinMeshType == 24 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
                nBaseValue = 20;
            else if (m_nSkinMeshType == 22)
                nBaseValue = 26;
            else if (m_nSkinMeshType == 21)
                nBaseValue = 28;
            else if (m_nSkinMeshType == 29 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
                nBaseValue = 36;
            else if (m_nSkinMeshType == 7)
                nBaseValue = 34;
            else if (m_nSkinMeshType == 28)
                nBaseValue = 31;
            else if (m_nSkinMeshType == 2)
                nBaseValue = 26;
            else if (m_nSkinMeshType == 11)
                nBaseValue = 50;
            else if (m_nSkinMeshType == 35)
                nBaseValue = 28;
            else if (m_nSkinMeshType == 44)
                nBaseValue = 50;

            if ((signed int)eMotion >= 4 && (signed int)eMotion <= 9)
            {
                if (m_bSwordShadow[0] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[0])
                    {
                        if (m_nWeaponTypeL == 41 && m_pSkinMesh->m_pSwingEffect[1])
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[1] * 1.0f;
                        else
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[0] * 1.0f;
                    }
                    if (m_pSkinMesh->m_pSwingEffect[0])
                        m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }
                if (m_bSwordShadow[1] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = m_fSowrdLength[1] * 1.0f;
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }
            }
            if (m_nSkinMeshType == 37 && eMotion == ECHAR_MOTION::ECMOTION_LEVELUP && g_pSoundManager && g_pSoundManager->GetSoundData(294))
                g_pSoundManager->GetSoundData(294)->Play();

            if (eMotion == ECHAR_MOTION::ECMOTION_RUN || eMotion == ECHAR_MOTION::ECMOTION_WALK)
            {
                m_pSkinMesh->m_dwFPS = (int)((float)nBaseValue - (float)(m_fMaxSpeed * 3.0f)) / 2;
                if (m_nSkinMeshType == 40)
                    m_pSkinMesh->m_dwFPS = 20;
                if (m_nSkinMeshType == 20 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
                    m_pSkinMesh->m_dwFPS = 20;
                if (m_nSkinMeshType == 39 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
                    m_pSkinMesh->m_dwFPS = 20;

                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fScale);
            }
            if (m_cMount == 1)
            {
                if (m_pMount)
                {
                    m_pMount->m_dwFPS = m_pSkinMesh->m_dwFPS;
                    m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[(int)eMotion]);
                }

                m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion + 28 * m_cMount]);
            }
            else
            {
                if (m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion]) == 0)
                {
                    if (!g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion])
                        m_eMotion = eMotion;
                    return;
                }
            }

            m_eMotion = eMotion;
            m_pSkinMesh->m_dwFPS = g_MobAniTable[m_nSkinMeshType].dwSpeed[(int)eMotion];
            if (m_cMount == 1 && m_pMount)
                m_pMount->m_dwFPS = g_MobAniTable[m_nMountSkinMeshType].dwSpeed[(int)eMotion];
        }

        m_nLoop = nLoop;
        m_dwStartAnimationTime = g_pTimerManager->GetServerTime();

        if (m_cPunish == 1)
            m_dwLastDummyTime = g_pTimerManager->GetServerTime();
    }
}

void TMHuman::AnimationFrame(int nWalkSndIndex)
{
    if (m_dwDelayDel || !m_pSkinMesh)
        return;

    if (m_pShade)
    {
        if (m_cHide == 1)
            m_pShade->m_bShow = 0;
        if (m_cShadow == 1)
            m_pShade->m_bShow = 0;
        if (m_nClass == 45)
            m_pShade->m_bShow = 0;
    }

    if (m_nClass != 45 && (m_eMotion == ECHAR_MOTION::ECMOTION_WALK || m_eMotion == ECHAR_MOTION::ECMOTION_RUN))
    {
        float fWaterHeight = 0.0f;

        if (g_pCurrentScene->GroundIsInWater(m_vecPosition, m_fHeight, &fWaterHeight) == 1)
        {
            nWalkSndIndex = 4;
            unsigned int dwServerTime = g_pTimerManager->GetServerTime();
            unsigned int nWaterTime = 80;
            if (m_eMotion == ECHAR_MOTION::ECMOTION_WALK)
                nWaterTime = 120;
            if ((dwServerTime - m_dwWaterTime) > nWaterTime)
            {
                float fSpeed = (m_fScale * TMHuman::m_vecPickSize[m_nSkinMeshType].x) * 0.0020000001f;

                auto pEffect = new TMEffectBillBoard2(10, 700, 0.5f, 0.5f, 0.5f, fSpeed, 0);
                if (pEffect)
                {
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->m_vecPosition.x = m_vecPosition.x;
                    pEffect->m_vecPosition.z = m_vecPosition.y;
                    pEffect->m_vecPosition.y = fWaterHeight + 0.25f;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
                m_dwWaterTime = dwServerTime;
            }
        }

        unsigned int dwOffset = m_pSkinMesh->m_dwOffset;
        int nSkinMeshType = m_nSkinMeshType;
        int nHeadMesh = m_stLookInfo.HelmMesh;
        if (m_cMount == 1 && m_pMount)
        {
            dwOffset = m_pMount->m_dwOffset;
            nSkinMeshType = m_nMountSkinMeshType;
            nHeadMesh = m_stMountLook.Mesh0;
        }

        if (nWalkSndIndex == 8 &&
            (int)m_vecPosition.x >> 7 > 26 &&
            (int)m_vecPosition.x >> 7 < 31 &&
            (int)m_vecPosition.y >> 7 > 20 &&
            (int)m_vecPosition.y >> 7 < 25)
        {
            nWalkSndIndex = 83;
        }

        if (nWalkSndIndex == 9 &&
            (int)m_vecPosition.x >> 7 > 26 &&
            (int)m_vecPosition.x >> 7 < 31 &&
            (int)m_vecPosition.y >> 7 > 20 &&
            (int)m_vecPosition.y >> 7 < 25)
        {
            nWalkSndIndex = 82;
        }

        if (m_cHide || nWalkSndIndex == 4 || nSkinMeshType != 31 || this != static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject))
        {
            if (!m_cHide)
            {
                if (dwOffset >= 2)
                {
                    if (dwOffset >= 7 && dwOffset < 9)
                    {
                        if (g_pSoundManager == nullptr)
                            return;

                        auto pSoundData = g_pSoundManager->GetSoundData(2 * nWalkSndIndex + 183);

                        if (pSoundData == nullptr || pSoundData->IsSoundPlaying())
                            return;

                        if (nSkinMeshType == 20 && (!nHeadMesh || nHeadMesh == 2) && m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
                        {
                            GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                        }
                        else if (nSkinMeshType != 39 || m_eMotion != ECHAR_MOTION::ECMOTION_RUN)
                        {
                            if (m_nClass == 33)
                            {
                                GetSoundAndPlayIfNot(148, 0, 0);
                            }
                            else if (nSkinMeshType == 40)
                            {
                                GetSoundAndPlayIfNot(g_MobAniTable[40].dwSoundTable[3], 0, 0);
                            }
                            else if (this == static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject))
                            {
                                if (nWalkSndIndex == 9)
                                    nWalkSndIndex = 8;

                                pSoundData->Play(0, 0);
                            }
                        }
                        else
                        {
                            GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                        }
                    }
                }
                else if (g_pSoundManager != nullptr)
                {
                    auto pSoundData = g_pSoundManager->GetSoundData(2 * nWalkSndIndex + 182);

                    if (pSoundData == nullptr || pSoundData->IsSoundPlaying())
                        return;

                    if (nSkinMeshType == 20 && (!nHeadMesh || nHeadMesh == 2) && m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
                    {
                        GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                    }
                    else if (nSkinMeshType != 39 || m_eMotion != ECHAR_MOTION::ECMOTION_RUN)
                    {
                        if (m_nClass == 33)
                        {
                            GetSoundAndPlayIfNot(148, 0, 0);
                        }
                        else if (nSkinMeshType == 40)
                        {
                            GetSoundAndPlayIfNot(g_MobAniTable[40].dwSoundTable[3], 0, 0);
                        }
                        else if (this == static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject))
                        {
                            if (nWalkSndIndex == 9)
                                nWalkSndIndex = 8;

                            pSoundData->Play(0, 0);
                        }
                    }
                    else
                    {
                        GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                    }
                }
            }
        }
        else
        {
            if (dwOffset < 2)
            {
                int nSoundIndex = 192;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 184;
                    break;
                case 8:
                    nSoundIndex = 198;
                    break;
                case 82:
                    nSoundIndex = 354;
                    break;
                case 83:
                    nSoundIndex = 350;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }

            if (dwOffset >= 5 && dwOffset < 7)
            {
                int nSoundIndex = 193;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 185;
                    break;
                case 8:
                    nSoundIndex = 199;
                    break;
                case 82:
                    nSoundIndex = 355;
                    break;
                case 83:
                    nSoundIndex = 351;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }

            if (dwOffset >= 11 && dwOffset < 13)
            {
                int nSoundIndex = 194;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 196;
                    break;
                case 8:
                    nSoundIndex = 180;
                    break;
                case 82:
                    nSoundIndex = 356;
                    break;
                case 83:
                    nSoundIndex = 352;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }

            if (dwOffset >= 13 && dwOffset < 15)
            {
                int nSoundIndex = 195;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 197;
                    break;
                case 8:
                    nSoundIndex = 181;
                    break;
                case 82:
                    nSoundIndex = 357;
                    break;
                case 83:
                    nSoundIndex = 353;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }
        }
        return;
    }

    if ((int)m_eMotion < 4 || (int)m_eMotion > 9)
    {
        if (m_eMotion == ECHAR_MOTION::ECMOTION_STRIKE)
        {
            if (m_pSkinMesh->m_dwOffset < 2)
            {
                int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[10];
                if (m_nSkinMeshType == 2 && m_nClass == 25)
                    nSoundIndex += 10;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                    nSoundIndex = 223;
                else if (m_nSkinMeshType == 4 && m_stLookInfo.FaceMesh == 3)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 21 && m_stLookInfo.FaceMesh >= 2)
                    nSoundIndex += 171;
                else if (m_nSkinMeshType == 3 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 20 && (m_stLookInfo.FaceMesh == 4 || m_stLookInfo.FaceMesh == 7) && m_fScale < 0.60000002f)
                    nSoundIndex = 208;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 7 || m_stLookInfo.FaceMesh == 9))
                    nSoundIndex = 261;
                else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 8)
                    nSoundIndex = 261;
                else if (m_nClass == 35)
                    nSoundIndex -= 89;
                else if (m_nClass == 34)
                    nSoundIndex = 258;
                else if (m_nClass == 30 && m_stLookInfo.FaceMesh == 4 || m_nClass == 33 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex = 265;
                else if (m_nClass == 60)
                    nSoundIndex = 364;
                if (m_nClass == 4 && m_stLookInfo.FaceMesh == 15)
                    nSoundIndex = 312;

                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }
        }
        else if (m_eMotion == ECHAR_MOTION::ECMOTION_DIE && m_pSkinMesh->m_dwOffset >= 5 && m_pSkinMesh->m_dwOffset < 7)
        {
            int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[11];
            if (m_nSkinMeshType == 2 && m_nClass == 25)
                nSoundIndex += 10;
            else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                nSoundIndex = 224;
            else if (m_nSkinMeshType == 4 && m_stLookInfo.FaceMesh == 3)
                nSoundIndex += 4;
            else if (m_nSkinMeshType == 21 && (signed int)m_stLookInfo.FaceMesh >= 2)
                nSoundIndex += 171;
            else if (m_nSkinMeshType == 3 && m_stLookInfo.FaceMesh == 1)
                nSoundIndex += 4;
            else if (m_nSkinMeshType == 20 && (m_stLookInfo.FaceMesh == 4 || m_stLookInfo.FaceMesh == 7) && m_fScale < 0.60000002f)
                nSoundIndex = 209;
            else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 7 || m_stLookInfo.FaceMesh == 9))
                nSoundIndex = 262;
            else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 8)
                nSoundIndex = 263;
            else if (m_nClass == 34)
                nSoundIndex = 259;
            else if (m_nClass == 35)
                nSoundIndex -= 89;
            else if (m_nClass == 36 || m_nClass == 37)
                nSoundIndex = 264;
            else if (m_nClass == 39 || m_nClass == 30 && m_stLookInfo.FaceMesh == 4 || m_nClass == 33 && m_stLookInfo.FaceMesh == 1)
                nSoundIndex = 268;
            else if (m_nClass == 60)
                nSoundIndex = 365;
            if (m_nClass == 4 && m_stLookInfo.FaceMesh == 15)
                nSoundIndex = 299;

            GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
        }
    }
    else
    {
        if (m_pSkinMesh->m_dwOffset >= 6 && m_pSkinMesh->m_dwOffset < 8)
        {
            if (m_nSkinMeshType == 4 || m_nSkinMeshType == 2)
                PlayAttackSound(m_eMotion, 0);
            if (!m_nSkinMeshType || m_nSkinMeshType == 1)
            {
                if (m_nClass != 60)
                    PlayAttackSound(m_eMotion, 0);
                else
                    GetSoundAndPlayIfNot(363, 0, 0);
            }
            else
            {
                int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[static_cast<int>(m_eMotion)];
                if (m_nSkinMeshType == 2 && m_nClass == 25)
                    nSoundIndex += 10;
                else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 4)
                    nSoundIndex = 226;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                    nSoundIndex = 222;
                else if (m_nSkinMeshType == 4 && m_stLookInfo.FaceMesh == 3)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 21 && (signed int)m_stLookInfo.FaceMesh >= 2)
                    nSoundIndex += 171;
                else if (m_nSkinMeshType == 3 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 20 && (m_stLookInfo.FaceMesh == 4 || m_stLookInfo.FaceMesh == 7) && m_fScale < 0.60000002f)
                    nSoundIndex = 208;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 7 || m_stLookInfo.FaceMesh == 9))
                    nSoundIndex = 201;
                else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 8)
                    nSoundIndex = 201;
                else if (m_nClass == 35)
                    nSoundIndex -= 89;
                else if (m_nClass == 30 && m_stLookInfo.FaceMesh == 4 || m_nClass == 33 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex = 265;

                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }
        }

        if (m_pSkinMesh->m_dwOffset >= 0xD && m_pSkinMesh->m_dwOffset < 0xF && (!m_nSkinMeshType ||
            m_nSkinMeshType == 1 ||
            m_nSkinMeshType == 3 ||
            m_nSkinMeshType == 4 ||
            m_nSkinMeshType == 2))
        {
            PlayAttackSound(m_eMotion, 1);
        }

        if (m_pSkinMesh->m_dwOffset >= 8 && m_pSkinMesh->m_dwOffset < 11)
        {
            if (m_nClass == 68 && (m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK02 || m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK05))
            {
                for (int i = -3; i < 3; ++i)
                {
                    auto pBillEffect = new TMEffectBillBoard(193, 700, 0.5f, 1.0f, 0.5f, 0.0049999999f, 1, 80);;

                    if (pBillEffect != nullptr)
                    {
                        pBillEffect->m_bStickGround = i % 2;
                        pBillEffect->m_vecPosition = TMVector3{ ((float)i * 0.5f) + m_vecPosition.x, m_fHeight, ((float)i * 0.02f) + m_vecPosition.y };
                        g_pCurrentScene->m_pEffectContainer->AddChild(pBillEffect);
                    }

                    auto pBillEffect2 = new TMEffectBillBoard(193, 700, 0.5f, 1.0f, 0.5f, 0.0049999999f, 1, 80);

                    if (pBillEffect2 != nullptr)
                    {
                        pBillEffect2->m_bStickGround = i % 2;
                        pBillEffect2->m_vecPosition = TMVector3{ m_vecPosition.x - ((float)i * 0.5f), m_fHeight, m_vecPosition.y - ((float)i * 0.02f) };
                        g_pCurrentScene->m_pEffectContainer->AddChild(pBillEffect2);
                    }
                }
            }
        }

        if (m_nClass == 2 || (int)m_eMotion < 7)
        {
            STRUCT_ITEM itemL{};
            itemL.sIndex = m_sLeftIndex;

            int nWeaponTypeL = m_nWeaponTypeL;
            if (nWeaponTypeL == 1 && g_pItemList[m_sLeftIndex].nReqLvl > 90)
                nWeaponTypeL = 2;

            PlayPunchedSound(nWeaponTypeL, 0);

            if (!g_pCurrentScene->m_pMyHuman || BASE_GetDistance(
                (int)g_pCurrentScene->m_pMyHuman->m_vecPosition.x,
                (int)g_pCurrentScene->m_pMyHuman->m_vecPosition.y,
                (int)m_vecAttTargetPos.x,
                (int)m_vecAttTargetPos.y) <= 20)
            {
                if (nWeaponTypeL != 101 && nWeaponTypeL != 102 && nWeaponTypeL != 103)
                {
                    float fWeaponLen = 1.0f;
                    float fLevel = (float)m_stScore.Level / 300.0f;
                    if (fLevel > 1.0f)
                        fLevel = 1.0f;
                    TMVector3 vec{ m_vecAttTargetPos.x, (m_fHeight + 0.89999998f) + (0.30000001f * fLevel), m_vecAttTargetPos.y };

                    if (m_dwAttackEffectTime && g_pTimerManager->GetServerTime() > (m_dwAttackEffectTime + 100))
                    {
                        D3DXVECTOR3 vecAxis{ 0.0f, 0.0f, -1.0f };
                        D3DXVECTOR3 vecMyToTarget{ m_vecAttTargetPos.x - m_vecPosition.x, 0.0f, m_vecAttTargetPos.y - m_vecPosition.y };
                        D3DXVec3Normalize(&vecMyToTarget, &vecMyToTarget);
                        D3DXVECTOR3 vecCross;
                        D3DXVec3Cross(&vecCross, &vecAxis, &vecMyToTarget);

                        float fDot = -D3DXVec3Dot(&vecAxis, &vecMyToTarget);
                        float fAngle = ((fDot * 0.5f) + 0.5f) * 3.1415927f;

                        if (vecCross.y < 0.0f)
                            fAngle = -fAngle;

                        unsigned int dwCenterColor = 0x80FFFFFF;
                        unsigned int dwOtherColor = 0x52A9E5;
                        unsigned int dwLightColor = 0x334388;

                        switch (m_stSancInfo.Legend7)
                        {
                        case 8:
                            dwCenterColor = 0x80FFCCCC;
                            dwOtherColor = 0xE57777;
                            dwLightColor = 0x883333;
                            break;
                        case 7:
                            dwCenterColor = 0x80FFCCFF;
                            dwOtherColor = 0xCC88CC;
                            dwLightColor = 0x884388;
                            break;
                        case 6:
                            dwCenterColor = 0x80CCFFCC;
                            dwOtherColor = 0x88E588;
                            dwLightColor = 0x338843;
                            break;
                        case 5:
                            dwCenterColor = 0x80CCCCFF;
                            dwOtherColor = 0x5253E5;
                            dwLightColor = 0x222288;
                            break;
                        }

                        TMFieldScene* pScene{};
                        if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
                            pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

                        auto pMeshEffect = new TMEffectMesh(531, dwCenterColor, m_fAngle, 4);
                        if (pMeshEffect)
                        {
                            if (m_fTargetHeight > 1.5f && pScene && this != pScene->m_pMyHuman)
                            {
                                pMeshEffect->m_fScaleH = 1.8f * m_fTargetHeight;
                                pMeshEffect->m_fScaleV = 1.8f * m_fTargetHeight;
                                pMeshEffect->m_dwLifeTime = 200;
                            }
                            else
                            {
                                pMeshEffect->m_fScaleH = 2.0f * m_fTargetHeight;
                                pMeshEffect->m_fScaleV = 2.0f * m_fTargetHeight;
                                pMeshEffect->m_dwLifeTime = 200;
                            }
                            pMeshEffect->m_nTextureIndex = 229;
                            pMeshEffect->m_dwCycleTime = 200;
                            pMeshEffect->m_vecPosition = vec;
                            pMeshEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pMeshEffect->m_cShine = 0;
                            g_pCurrentScene->m_pEffectContainer->AddChild(pMeshEffect);
                        }

                        if (m_fTargetHeight <= 1.5f || m_nClass != 56)
                        {
                            auto pLightMap = new TMShade(7, 118, 1.0f);
                            if (pLightMap)
                            {
                                if (m_bCritical)
                                    pLightMap->SetColor(0x883333);
                                else
                                    pLightMap->SetColor(dwLightColor);

                                pLightMap->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                pLightMap->SetPosition(TMVector2{ (vecMyToTarget.x * 0.5f) + vec.x, (vecMyToTarget.z * 0.5f) + vec.z });
                                pLightMap->m_dwLifeTime = 250;
                                g_pCurrentScene->m_pEffectContainer->AddChild(pLightMap);
                            }
                        }

                        auto vecDir = TMVector3{ vecMyToTarget.x, vecMyToTarget.y, vecMyToTarget.z };

                        if (!g_bHideEffect)
                        {
                            TMHuman* pAttackDest = g_pObjectManager->GetHumanByID(m_nAttackDestID);
                            if (pAttackDest)
                            {
                                pAttackDest->m_bPunchEffect = 1;
                                pAttackDest->m_dwPunchEffectTime = g_pTimerManager->GetServerTime();
                            }

                            TMEffectParticle* pParticle{};

                            if (m_fTargetHeight <= 1.5f || m_nClass != 56)
                            {
                                if (m_cAvatar == 1)
                                {
                                    if (m_bCritical)
                                    {
                                        pParticle = new TMEffectParticle(vec + (vecDir * 0.30000001f), 5, 20, 1.0f, 0x883333, 0, 231, 1.0f, 1, vecDir, 300);
                                    }
                                }
                                else if (m_bCritical)
                                {
                                    pParticle = new TMEffectParticle(vec + (vecDir * 0.30000001f), 5, 10, 0.80000001f, 0x883333, 0, 231, 1.0f, 1, vecDir, 300);
                                }

                                if (pParticle)
                                    g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);
                            }

                            if (m_bCritical)
                            {
                                TMEffectBillBoard* pBill1{};

                                TMVector3 vecTargetPos = vec;

                                if (m_fTargetHeight <= 1.5f)
                                {
                                    pBill1 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);
                                }
                                else
                                {
                                    pBill1 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);
                                }

                                if (pBill1)
                                {
                                    pBill1->m_bLookCam = 0;
                                    pBill1->m_vecPosition = vecTargetPos + (vecDir * 0.5f);
                                    pBill1->m_vecRotAxis = vecDir;
                                    pBill1->m_fAxisAngle = (m_fAngle + 1.5707964f) + 0.050000001f;
                                    pBill1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                    pBill1->m_nParticleType = 15;
                                    pBill1->m_fScaleVelX = 0.003f;
                                    pBill1->m_fScaleVelY = 0.003f;
                                    pBill1->m_fScaleVelZ = 0.003f;
                                    pBill1->SetColor(0x883333);
                                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill1);
                                }

                                TMEffectBillBoard* pBill2{};

                                if (m_fTargetHeight <= 1.5f)
                                {
                                    pBill2 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);
                                }
                                else
                                {
                                    pBill2 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);;
                                }

                                if (pBill2)
                                {
                                    pBill2->m_bLookCam = 0;
                                    pBill2->m_vecPosition = vecTargetPos + (vecDir * 0.5f);
                                    pBill2->m_vecRotAxis = vecDir;
                                    pBill2->m_fAxisAngle = (m_fAngle + 1.5707964f) - 0.050000001f;
                                    pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                    pBill2->m_nParticleType = 15;
                                    pBill2->m_fScaleVelX = 0.0049999999f;
                                    pBill2->m_fScaleVelY = 0.0049999999f;
                                    pBill2->m_fScaleVelZ = 0.0049999999f;
                                    pBill2->SetColor(0x883333);
                                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                                }
                            }
                        }
                    }
                    m_dwAttackEffectTime = g_pTimerManager->GetServerTime();
                }
            }
        }

        if (m_pSkinMesh->m_dwOffset >= 0xF && m_pSkinMesh->m_dwOffset < 0x11 && (m_nClass == 2 || (int)m_eMotion < 7))
        {
            STRUCT_ITEM itemL{};
            STRUCT_ITEM itemR{};
            itemL.sIndex = m_sLeftIndex;
            itemR.sIndex = m_sRightIndex;
            int nWeaponTypeR = m_nWeaponTypeR;
            int nWeaponPosL = BASE_GetItemAbility(&itemL, EF_POS);
            int nWeaponPosR = BASE_GetItemAbility(&itemR, EF_POS);

            if (nWeaponPosR != 0 && nWeaponPosR != 128 || !nWeaponPosR && !nWeaponPosL)
            {
                if (nWeaponTypeR == 1 && g_pItemList[m_sRightIndex].nReqLvl > 90)
                    nWeaponTypeR = 2;
                PlayPunchedSound(nWeaponTypeR, 1);

                float fLevel = (float)m_stScore.Level / 300.0f;
                if (fLevel > 1.0f)
                    fLevel = 1.0f;

                unsigned int dwColor = 0x80FFFFFF;
                unsigned int dwOtherColor = 0x52A9E5;
                unsigned int dwLightColor = 0x334388;
                switch (m_stSancInfo.Legend7)
                {
                case 8:
                    dwColor = 0x80FFCCCC;
                    dwOtherColor = 0xE57777;
                    dwLightColor = 0x883333;
                    break;
                case 7:
                    dwColor = 0x80FFCCFF;
                    dwOtherColor = 0xCC88CC;
                    dwLightColor = 0x884388;
                    break;
                case 6:
                    dwColor = 0x80CCFFCC;
                    dwOtherColor = 0x88E588;
                    dwLightColor = 0x338843;
                    break;
                case 5:
                    dwColor = 0x80CCCCFF;
                    dwOtherColor = 0x5253E5;
                    dwLightColor = 0x222288;
                    break;
                }

                TMVector3 other{ m_vecAttTargetPos.x, (m_fHeight + 0.89999998f) + (0.30000001f * fLevel), m_vecAttTargetPos.y };

                if (m_dwAttackEffectTime && g_pTimerManager->GetServerTime() > (m_dwAttackEffectTime + 100))
                {
                    D3DXVECTOR3 pV1{ 0.0f, 0.0f, -1.0f };
                    D3DXVECTOR3 pV2{
                        m_vecAttTargetPos.x - m_vecPosition.x,
                        0.0f,
                        m_vecAttTargetPos.y - m_vecPosition.y };

                    D3DXVec3Normalize(&pV2, &pV2);
                    D3DXVECTOR3 pOut;
                    D3DXVec3Cross(&pOut, &pV1, &pV2);

                    auto pChild = new TMEffectMesh(531, dwColor, m_fAngle, 4);
                    if (pChild)
                    {
                        pChild->m_nTextureIndex = 229;
                        pChild->m_dwLifeTime = 150;
                        pChild->m_dwCycleTime = 500;
                        pChild->m_vecPosition = other;
                        pChild->m_fScaleH = 1.3f * m_fTargetHeight;
                        pChild->m_fScaleV = 1.3f * m_fTargetHeight;
                        pChild->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        pChild->m_cShine = 0;
                        g_pCurrentScene->m_pEffectContainer->AddChild(pChild);

                        auto pShade = new TMShade(7, 118, 1.0f);
                        if (pShade)
                        {
                            if (m_bCritical)
                                pShade->SetColor(0x883333);
                            else
                                pShade->SetColor(dwLightColor);

                            pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pShade->SetPosition(TMVector2{ (pV2.x * 0.5f) + other.x, (pV2.z * 0.5f) + other.z });
                            pShade->m_dwLifeTime = 500;
                            g_pCurrentScene->m_pEffectContainer->AddChild(pShade);
                        }
                    }

                    if (!g_bHideEffect)
                    {
                        TMVector3 vecDir{ pV2.x, pV2.y, pV2.z };

                        TMEffectParticle* pParticle{};

                        if (m_bCritical)
                        {
                            pParticle = new TMEffectParticle(other + (vecDir * 0.30000001f), 5, 10, 1.3f, 0x883333, 0, 231, 1.0f, 1, vecDir, 300);
                        }
                        else
                        {
                            pParticle = new TMEffectParticle(other + (vecDir * 0.30000001f), 5, 5, 0.1f, 0xFFEEAA, 0, 231, 1.0f, 1, vecDir, 800);
                        }

                        if (pParticle)
                            g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);

                        auto pBill1 = new TMEffectBillBoard(
                            230,
                            300,
                            4.0f * m_fTargetHeight,
                            4.0f * m_fTargetHeight,
                            4.5f * m_fTargetHeight,
                            0.00050000002f,
                            1,
                            80);

                        if (pBill1)
                        {
                            pBill1->m_bLookCam = 0;

                            pBill1->m_vecPosition = other + (vecDir * 0.5f);
                            pBill1->m_vecRotAxis = vecDir;
                            pBill1->m_fAxisAngle = (m_fAngle + 1.5707964f) + 0.050000001f;
                            pBill1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pBill1->m_nParticleType = 14;
                            pBill1->m_fScaleVelX = 0.0049999999f;
                            pBill1->m_fScaleVelY = 0.0049999999f;
                            pBill1->m_fScaleVelZ = 0.0049999999f;
                            if (m_bCritical)
                                pBill1->SetColor(0x883333);
                            else
                                pBill1->SetColor(dwLightColor);
                            g_pCurrentScene->m_pEffectContainer->AddChild(pBill1);
                        }

                        auto pBill2 = new TMEffectBillBoard(
                            230,
                            300,
                            4.0f * m_fTargetHeight,
                            4.0f * m_fTargetHeight,
                            4.5f * m_fTargetHeight,
                            0.00050000002f,
                            1,
                            80);

                        if (pBill2)
                        {
                            pBill2->m_bLookCam = 0;

                            pBill2->m_vecPosition = other + (vecDir * 0.5f);
                            pBill2->m_vecRotAxis = vecDir;
                            pBill2->m_fAxisAngle = (m_fAngle + 1.5707964f) - 0.050000001f;
                            pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pBill2->m_nParticleType = 15;
                            pBill2->m_fScaleVelX = 0.0049999999f;
                            pBill2->m_fScaleVelY = 0.0049999999f;
                            pBill2->m_fScaleVelZ = 0.0049999999f;
                            if (m_bCritical)
                                pBill2->SetColor(0x883333);
                            else
                                pBill2->SetColor(dwLightColor);
                            g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                        }
                    }
                }

                m_dwAttackEffectTime = g_pTimerManager->GetServerTime();
            }
        }

        if (m_nSkinMeshType == 20 && (!m_stLookInfo.HelmMesh || m_stLookInfo.HelmMesh == 2) && !(m_pSkinMesh->m_dwOffset % 5) &&
            m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK02 &&
            (m_vecOldFire.x != m_vecTempPos[0].x || m_vecOldFire.y != m_vecTempPos[0].y || m_vecOldFire.z != m_vecTempPos[0].z))
        {
            auto pBill = new TMEffectBillBoard(44, 2000, 0.5f, 0.5f, 0.5f, 0.00050000002f, 1, 80);
            if (pBill)
            {
                pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pBill->m_vecPosition = m_vecTempPos[0];
                g_pCurrentScene->m_pEffectContainer->AddChild(pBill);
            }
            m_vecOldFire = m_vecTempPos[0];
        }
    }
}

void TMHuman::SetMotion(ECHAR_MOTION eMotion, float fAngle)
{
    if (m_dwDelayDel)
        return;

    SetAnimation(eMotion, 0);
}
