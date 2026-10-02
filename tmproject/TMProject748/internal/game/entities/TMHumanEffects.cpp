#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "TMItem.h"
#include "TMScene.h"
#include "TMSkillMagicShield.h"
#include "TMEffectBillBoard.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectCharge.h"
#include "TMEffectMesh.h"
#include "TMEffectMeshRotate.h"
#include "TMEffectSkinMesh.h"
#include "TMButterFly.h"
#include "TMShade.h"
#include "TMEffectSWSwing.h"
#include "ObjectManager.h"
#include "TMCamera.h"
#include "TMGround.h"
#include "TMFieldScene.h"
#include "TMEffectParticle.h"
#include "TMUtil.h"
#include "TMObjectContainer.h"
#include "TMLight.h"
#include "TMSkillTownPortal.h"
#include "TMEffectLevelUp.h"
#include "ItemEffect.h"
#include "TMSkillJudgement.h"
#include "TMSkillSlowSlash.h"
#include "TMSkillHaste.h"
#include "TMSkillSpeedUp.h"
#include "TMSkillDoubleSwing.h"
#include "TMSkillFreezeBlade.h"
#include "TMSkillBash.h"
#include "TMSkillMagicArrow.h"
#include "TMSkillCure.h"
#include "TMSkillHeal.h"
#include "TMSkillFire.h"
#include "TMSkillIceSpear.h"
#include "TMSkillPoison.h"
#include "TMSkillSpChange.h"
#include "TMArrow.h"
#include "TMSkillThunderBolt.h"
#include "TMEffectDust.h"

void TMHuman::RenderEffect()
{
    if (m_dwDelayDel)
        return;

    unsigned int dwServerTime = g_pTimerManager->GetServerTime();

    if (m_pEyeFire[0] && (m_nClass == 36 || m_nClass == 37 || m_cCoinArmor == 1))
    {
        RenderEffect_Skull();
    }
    else if (m_nClass == 32)
    {
        RenderEffect_Golem(dwServerTime);
    }
    else if (m_nClass == 34 || m_nClass == 23 || m_nClass == 21 && m_stLookInfo.FaceMesh == 10)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[i];
                m_pEyeFire[i]->FrameMove(0);
            }
        }
    }
    else if (m_nClass == 16 && m_stLookInfo.FaceMesh == 6)
    {
        RenderEffect_BoneDragon(dwServerTime);
    }
    else if (m_nClass == 16 && !m_stLookInfo.FaceMesh && m_stLookInfo.FaceSkin == 1)
    {
        RenderEffect_EmeraldDragon(dwServerTime);
    }
    else if (m_nClass == 30 && (!m_stLookInfo.FaceMesh || m_stLookInfo.FaceMesh == 1 || m_stLookInfo.FaceMesh == 2))
    {
        RenderEffect_Minotauros(dwServerTime);
    }
    else if (m_nClass == 30 && m_stLookInfo.FaceMesh == 4 || m_nClass == 38 && m_stLookInfo.CoatMesh == 14 && !m_cMantua)
    {
        RenderEffect_DarkElf(dwServerTime);
    }
    else if (m_nClass == 25 && m_stLookInfo.FaceMesh == 3 && m_stLookInfo.FaceSkin == 8 || m_nClass == 25 && m_stLookInfo.FaceMesh == 12)
    {
        RenderEffect_DarkNightZombieTroll(dwServerTime);
    }
    else if (m_nClass == 23)
    {
        RenderEffect_Hydra(dwServerTime);
    }
    else if (m_nClass == 28 && m_stLookInfo.FaceMesh == 2)
    {
        RenderEffect_DungeonBear(dwServerTime);
    }
    else if (m_nClass == 22 || m_nClass == 27)
    {
        RenderEffect_Pig_Wolf(dwServerTime);
    }
    else if (m_nClass == 18 && m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK02)
    {
        for (int i = 0; i < 2; ++i)
        {
            auto pEffect = new TMEffectBillBoard(0, 400 * i + 1500, 0.1f, 0.1f, 0.1f, 0.001f, 1, 80);

            if (pEffect != nullptr)
            {
                pEffect->m_vecPosition = TMVector3{
                    ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i].x,
                    m_vecTempPos[i].y,
                    ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i].z };

                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pEffect->SetColor(0xFFFF6666);
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
    }
    else if (m_nClass == 25 && m_stLookInfo.FaceMesh == 3 && m_fScale > 1.1751f || m_nClass == 30)
    {
        unsigned int dwColor = 0xFFFF6666;

        if (m_nClass == 30)
            dwColor = 0xFF66FF66;

        if ((dwServerTime - m_dwGolemDustTime) > 100)
        {
            for (int i = 0; i < 2; ++i)
            {
                auto pEffect = new TMEffectBillBoard(0, 400 * i + 1500, 0.1f, 0.1f, 0.1f, 0.001f, 1, 80);

                if (pEffect != nullptr)
                {
                    pEffect->m_vecPosition = TMVector3{
                        ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i + 1].x,
                        m_vecTempPos[i + 1].y,
                        ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i + 1].z };

                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->SetColor(dwColor);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
            }
            m_dwGolemDustTime = dwServerTime;
        }
    }

    else if (m_nClass == 21 && m_stLookInfo.FaceMesh == 4)
    {
        if ((dwServerTime - m_dwGolemDustTime) > 100)
        {
            for (int i = 0; i < 2; ++i)
            {
                for (int j = 0; j < 2; ++j)
                {
                    auto pEffect = new TMEffectBillBoard(0, 400 * i + 1500, 0.1f, 0.1f, 0.1f, 0.001f, 1, 80);

                    if (pEffect != nullptr)
                    {
                        pEffect->m_vecPosition = TMVector3{
                            ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[j + 1].x,
                            m_vecTempPos[j + 1].y,
                            ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[j + 1].z };
                        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        pEffect->SetColor(0xFFFFAA66);
                        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                    }
                }
            }
            m_dwGolemDustTime = dwServerTime;
        }
    }
    else if (m_nClass == 29 && m_stLookInfo.FaceMesh == 1)
    {
        if (m_pEyeFire[0])
        {
            m_pEyeFire[0]->m_vecPosition = m_vecTempPos[0];
            m_pEyeFire[0]->FrameMove(0);
        }
    }
    else if (m_nClass == 38 && m_cMantua > 0)
    {
        for (int i = 1; i < 7; ++i)
        {
            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[i];
                m_pEyeFire[i]->FrameMove(0);
            }
        }
    }
    else if (m_nClass == 33 && m_stLookInfo.FaceMesh == 1 && RenderDevice::m_bDungeon == 2)
    {
        for (int i = 0; i < 7; ++i)
        {
            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->SetColor(0xFFFF5500);
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[i];
                m_pEyeFire[i]->m_vecPosition.y += (0.30000001f * m_fScale);
                m_pEyeFire[i]->FrameMove(0);
            }
        }
    }
    else if (m_nClass == 16 && m_stLookInfo.FaceMesh == 7)
    {
        if ((dwServerTime - m_dwGolemDustTime) > 100)
        {
            for (int i = 0; i < 2; ++i)
            {
                auto pEffect = new TMEffectBillBoard(0, 400 * i + 1500, 0.1f, 0.1f, 0.1f, 0.001f, 1, 80);

                if (pEffect != nullptr)
                {
                    pEffect->m_vecPosition = TMVector3{
                        ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecPosition.x,
                        m_fHeight + 0.2f,
                        ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecPosition.y };

                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->SetColor(0xFFFF8800);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
            }
            m_dwGolemDustTime = dwServerTime;
        }
    }
    else if (m_nClass == 39)
    {
        static const int nIndex[4]{ 6, 7, 2, 3 };

        for (int i = 0; i < 4; ++i)
        {
            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[nIndex[i]];
                m_pEyeFire[i]->FrameMove(0);
            }
        }
    }
    else if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
    {
        RenderEffect_Khepra(dwServerTime);
    }
    else if (m_nClass == 66 && !m_cShadow)
    {
        RenderEffect_LegendBeriel(dwServerTime);
    }
    else if (m_nClass == 67)
    {
        RenderEffect_LegendBerielKeeper(dwServerTime);
    }
    else if (m_sCostume == 4161 || m_sCostume == 4162)
    {
        RenderEffect_RudolphCostume(dwServerTime);
    }
    else if (m_pMount && m_pMount->m_nBoneAniIndex == 31 && m_pMount->m_Look.Mesh0 == 8)
    {
        m_pMount->m_bRenderEffect = 1;
    }

    if (m_cEnchant)
    {
        if (m_pSkinMesh->m_pSwingEffect[0] != nullptr && m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant)
        {
            m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant->m_vecPosition = m_vecTempPos[6];
            m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant->FrameMove(0);
        }
        if (m_pSkinMesh->m_pSwingEffect[1] != nullptr && m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant)
        {
            m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant->m_vecPosition = m_vecTempPos[7];
            m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant->FrameMove(0);
        }
    }
}

void TMHuman::FrameMoveEffect(unsigned int dwServerTime)
{
    if (this->m_dwDelayDel)
        return;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this)
        g_pObjectManager->DeleteObject(m_pFamiliar);

    if (m_cHide || m_cShadow == 1)
    {
        if (m_pRescue)
            m_pRescue->m_bShow = 0;
        if (m_pMagicShield)
            m_pMagicShield->m_bShow = 0;
        if (m_pCancelation)
            m_pCancelation->m_bShow = 0;
        if (m_pEleStream)
            m_pEleStream->m_bShow = 0;
        if (m_pEleStream2)
            m_pEleStream2->m_bShow = 0;
        if (m_pLightenStorm[0])
            m_pLightenStorm[0]->m_bShow = 0;
        if (m_pLightenStorm[1])
            m_pLightenStorm[1]->m_bShow = 0;
        if (m_pAurora)
            m_pAurora->m_bShow = 0;
        if (m_pSkillAmp)
            m_pSkillAmp->m_bShow = 0;
        if (m_pShadow)
            m_pShadow->m_bShow = 0;
        if (m_pHuntersVision)
            m_pHuntersVision->m_bShow = 0;
        if (m_pOverExp)
            m_pOverExp->m_bShow = 0;
        if (m_pBraveOverExp)
            m_pBraveOverExp->m_bShow = 0;
        if (m_pbomb)
            m_pbomb->m_bShow = 0;
        if (m_pLifeDrain)
            m_pLifeDrain->m_bShow = 0;
        if (m_pChargeEnergy)
            m_pChargeEnergy->m_bShow = 0;
        if (m_pCriticalArmor)
            m_pCriticalArmor->m_bShow = 0;
        if (m_pSoul[0])
        {
            m_pSoul[0]->m_pBillBoard->m_bShow = 0;
            m_pSoul[0]->m_bShow = 0;
        }
        if (m_pSoul[1])
        {
            m_pSoul[1]->m_pBillBoard->m_bShow = 0;
            m_pSoul[1]->m_bShow = 0;
        }
        if (m_pProtector)
            m_pProtector->m_bVisible = 0;
        if (m_pFamiliar)
            m_pFamiliar->m_bVisible = 0;
        for (int i = 0; i < 7; ++i)
        {
            if (m_pEyeFire[i])
                m_pEyeFire[i]->m_bShow = 0;
            if (m_pEyeFire2[i])
                m_pEyeFire2[i]->m_bShow = 0;
            if (m_pRotateBone[i])
                m_pRotateBone[i]->m_bShow = 0;
        }
        for (int i = 0; i < 4; ++i)
        {
            if (m_pFly[i])
                m_pFly[i]->m_bVisible = 0;
        }
        for (int i = 0; i < 5; ++i)
        {
            if (m_pImmunity[i])
                m_pImmunity[i]->m_bShow = 0;
        }
        if (m_pSkinMesh->m_pSwingEffect[0])
        {
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_cAssert = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_cArmorClass = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_bEnchant = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 0;
        }
        if (m_pSkinMesh->m_pSwingEffect[1])
        {
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_cAssert = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_cArmorClass = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_bEnchant = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 0;
        }

        return;
    }

    if (g_bHideSkillBuffEffect == 1 || g_bHideSkillBuffEffect2 == 1)
    {
        if (m_pAurora)
            m_pAurora->m_bShow = 0;
        if (m_pEleStream)
            m_pEleStream->m_bShow = 0;
        if (m_pEleStream2)
            m_pEleStream2->m_bShow = 0;
        if (m_pMagicShield)
            m_pMagicShield->m_bShow = 0;
        if (m_pCriticalArmor)
            m_pCriticalArmor->m_bShow = 0;
        if (m_pLightenStorm[0])
            m_pLightenStorm[0]->m_bShow = 0;
        if (m_pLightenStorm[1])
            m_pLightenStorm[1]->m_bShow = 0;
        if (m_pSkillAmp)
            m_pSkillAmp->m_bShow = 0;
        if (m_pChargeEnergy)
            m_pChargeEnergy->m_bShow = 0;
        if (m_pProtector)
            m_pProtector->m_bVisible = 0;
        if (m_pSoul[0])
            m_pSoul[0]->m_bShow = 0;
        if (m_pSoul[1])
            m_pSoul[1]->m_bShow = 0;
        for (int j = 0; j < 4; ++j)
        {
            if (m_pFly[j])
                m_pFly[j]->m_bVisible = 0;
        }
        if (m_pSkinMesh->m_pSwingEffect[0])
        {
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_cArmorClass = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_bEnchant = 0;
            m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 0;
        }
        if (m_pSkinMesh->m_pSwingEffect[1])
        {
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_cArmorClass = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_bEnchant = 0;
            m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 0;
        }
    }

    if (m_stEffectEvent.dwTime && dwServerTime > m_stEffectEvent.dwTime)
    {
        float fWantAngle = atan2f(m_stEffectEvent.vecTo.x - m_vecPosition.x, m_stEffectEvent.vecTo.z - m_vecPosition.y) + D3DXToRadian(90);
        // Native 7.48 also dispatches ground casts (pTarget == nullptr).
        // Keep their skill ID: vecTo carries the destination without an actor.

        // Remote actors' effects must not change the local player's skill timers.
        if (this == pScene->m_pMyHuman && m_stEffectEvent.sEffectIndex >= 0 && m_stEffectEvent.sEffectIndex < 248)
        {
            if (!pScene->m_dwSkillLastTime[m_stEffectEvent.sEffectIndex] || pScene->m_dwSkillLastTime[m_stEffectEvent.sEffectIndex] >= m_stEffectEvent.dwTime)
                pScene->m_dwSkillLastTime[m_stEffectEvent.sEffectIndex] = dwServerTime;
            else
                m_stEffectEvent.dwTime = pScene->m_dwSkillLastTime[m_stEffectEvent.sEffectIndex];
        }
        if (m_stEffectEvent.sEffectIndex >= 0 && m_stEffectEvent.sEffectIndex < 96 ||
            m_stEffectEvent.sEffectIndex >= 151 && m_stEffectEvent.sEffectIndex <= 153 ||
            m_stEffectEvent.sEffectIndex == 104 || m_stEffectEvent.sEffectIndex == 105)
        {
            TMVector3 vecStart{ m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y };
            TMVector3 vecDest{ m_stEffectEvent.vecTo.x, m_stEffectEvent.vecTo.y, m_stEffectEvent.vecTo.z };

            if (m_stEffectEvent.sEffectIndex == 2)
            {
                auto pSlow = new TMSkillSlowSlash(vecStart, vecStart, 0, m_stEffectEvent.pTarget);
                g_pCurrentScene->m_pEffectContainer->AddChild(pSlow);
            }
            else if (m_stEffectEvent.sEffectIndex == 3)
            {
                if (m_stEffectEvent.pTarget && m_stEffectEvent.pTarget->m_pRescue)
                    m_stEffectEvent.pTarget->m_pRescue->StartVisible(dwServerTime);
                GetSoundAndPlay(158, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 13)
            {
                auto pCrArmor = new TMEffectMesh(2838, 0xFF999999, m_fAngle, 4);
                pCrArmor->m_nTextureIndex = 413;
                pCrArmor->m_dwLifeTime = 500;
                pCrArmor->m_dwCycleTime = 500;

                if (m_cMount == 1)
                    pCrArmor->m_vecPosition = { m_vecSkinPos.x, (((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) / 2.0f) + m_vecSkinPos.y) - 0.3f, m_vecSkinPos.z };
                else
                    pCrArmor->m_vecPosition = { m_vecPosition.x, (((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) / 2.0f) + m_fHeight) + 0.3f, m_vecPosition.y };

                pCrArmor->m_fScaleH = 2.5f;
                pCrArmor->m_fScaleV = 2.5f;
                pCrArmor->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pCrArmor->m_cShine = 0;

                g_pCurrentScene->m_pEffectContainer->AddChild(pCrArmor);
            }
            else if (m_stEffectEvent.sEffectIndex == 8 || m_stEffectEvent.sEffectIndex == 10 || m_stEffectEvent.sEffectIndex == 18)
            {
                auto pCamera = g_pObjectManager->GetCamera();
                if (pCamera->m_pFocusedObject == this)
                    pCamera->EarthQuake(2);
                GetSoundAndPlay(160, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 11)
            {
                auto vecPos = vecStart;
                float fRand = (float)(rand() % 5);

                auto pFire = new TMEffectBillBoard(56, 700, 1.6f, 1.6f, 1.6f, 0.002f, 1, 80);
                pFire->SetColor(0xFF990000);
                pFire->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pFire->m_nFade = 1;
                pFire->m_vecPosition = { vecPos.x, vecPos.y + 0.2f, vecPos.z };
                g_pCurrentScene->m_pEffectContainer->AddChild(pFire);

                auto pFire2 = new TMEffectBillBoard(60, 1200, (float)(fRand * 0.3f) + 2.5999999f, (float)(fRand * 0.3f) + 2.3f,
                    (float)(fRand * 0.3f) + 2.5999999f, 0.0005f, 1, 80);

                pFire2->SetColor(0xFF994444);
                pFire2->m_fAxisAngle = (float)(D3DXToRadian(180) * fRand) / 3.0f;
                pFire2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pFire2->m_nFade = 1;
                pFire2->m_vecPosition = { vecPos.x, vecPos.y + 0.2f, vecPos.z };
                g_pCurrentScene->m_pEffectContainer->AddChild(pFire2);

                GetSoundAndPlay(168, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 9)
            {
                auto vecPos = vecStart;
                auto pSpeedUp = new TMSkillSpeedUp(vecStart, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pSpeedUp);
            }
            else if (m_stEffectEvent.sEffectIndex == 15)
            {
                auto pCrArmor = new TMEffectMesh(2838, 0xFF999999, m_fAngle, 4);
                pCrArmor->m_nTextureIndex = 413;
                pCrArmor->m_dwLifeTime = 500;
                pCrArmor->m_dwCycleTime = 500;

                if (m_cMount == 1)
                    pCrArmor->m_vecPosition = { m_vecSkinPos.x, (((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) / 2.0f) + m_vecSkinPos.y) - 0.3f, m_vecSkinPos.z };
                else
                    pCrArmor->m_vecPosition = { m_vecPosition.x, (((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) / 2.0f) + m_fHeight) + 0.3f, m_vecPosition.y };

                pCrArmor->m_fScaleH = 2.5f;
                pCrArmor->m_fScaleV = 2.5f;
                pCrArmor->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pCrArmor->m_cShine = 0;

                g_pCurrentScene->m_pEffectContainer->AddChild(pCrArmor);
            }
            else if (m_stEffectEvent.sEffectIndex == 16 || m_stEffectEvent.sEffectIndex == 12 || m_stEffectEvent.sEffectIndex == 28)
            {
                vecDest.y += 1.0f;
                int nLevel = 0;
                if (m_stEffectEvent.sEffectIndex == 12)
                    nLevel = 1;
                if (m_stEffectEvent.sEffectIndex == 28)
                    nLevel = 2;

                auto pDouble = new TMSkillDoubleSwing(vecStart, vecDest, nLevel, nullptr);
                g_pCurrentScene->m_pEffectContainer->AddChild(pDouble);
            }
            else if (m_stEffectEvent.sEffectIndex == 17 && m_pSkinMesh)
            {
                GetSoundAndPlay(155, 0, 0);
                if (m_bSwordShadow[0] == 1 && m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 1;
                if (m_bSwordShadow[1] == 1 && m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 1;
                if (!m_bSwordShadow[0] && !m_bSwordShadow[1])
                {
                    if (m_pSkinMesh->m_pSwingEffect[0])
                        m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 1;
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 1;
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 19 && m_pSkinMesh)
            {
                GetSoundAndPlay(160, 0, 0);
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = 4;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = 4;

                auto pFreeze = new TMSkillFreezeBlade(vecDest, 0, 0, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pFreeze);
            }
            else if (m_stEffectEvent.sEffectIndex == 20)
            {
                GetSoundAndPlay(153, 0, 0);
                if (m_stEffectEvent.pTarget)
                {
                    bool bExpand = false;
                    if (m_stEffectEvent.pTarget->m_nClass == 4 || m_stEffectEvent.pTarget->m_nClass == 8)
                        bExpand = true;

                    auto pEffect = new TMEffectSkinMesh(m_stEffectEvent.pTarget->m_nSkinMeshType,
                        { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0 }, 0, 0);

                    if (!m_stEffectEvent.pTarget || m_stEffectEvent.pTarget->m_cMount <= 0 || !m_stEffectEvent.pTarget->m_pMount)
                    {
                        memcpy(&pEffect->m_stLookInfo, &m_stEffectEvent.pTarget->m_stLookInfo, sizeof(pEffect->m_stLookInfo));
                    }
                    else
                    {
                        pEffect->m_nSkinMeshType = m_stEffectEvent.pTarget->m_nMountSkinMeshType;
                        memcpy(
                            &pEffect->m_stLookInfo, &m_stEffectEvent.pTarget->m_stMountLook, sizeof(pEffect->m_stLookInfo));
                        pEffect->m_nSkinMeshType2 = m_stEffectEvent.pTarget->m_nSkinMeshType;
                        memcpy(&pEffect->m_stLookInfo2, &m_stEffectEvent.pTarget->m_stLookInfo, sizeof(pEffect->m_stLookInfo2));
                    }

                    pEffect->m_StartColor.r = 0.5f;
                    pEffect->m_StartColor.g = 0.5f;
                    pEffect->m_StartColor.b = 0.5f;
                    pEffect->InitObject(bExpand);
                    pEffect->m_nFade = 1;
                    pEffect->m_dwLifeTime = 3000;
                    pEffect->InitPosition(m_stEffectEvent.pTarget->m_vecPosition.x,
                        m_stEffectEvent.pTarget->m_fHeight + 0.1f, m_stEffectEvent.pTarget->m_vecPosition.y);

                    if (m_cMount > 0 && m_pMount)
                    {
                        if (pEffect->m_pSkinMesh)
                            pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffect->m_nSkinMeshType].dwAniTable[0]);
                        if (pEffect->m_pSkinMesh2)
                            pEffect->m_pSkinMesh2->SetAnimation(g_MobAniTable[pEffect->m_nSkinMeshType2].dwAniTable[24]);
                    }
                    else
                        pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffect->m_nSkinMeshType].dwAniTable[0]);

                    pEffect->m_fStartAngle = m_stEffectEvent.pTarget->m_fAngle;
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->m_nMotionType = 1;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 22)
            {
                TMVector3 vec{ vecDest.x, vecDest.y + 1.0f, vecDest.z };
                auto pBash = new TMSkillBash(vec, 1);
                g_pCurrentScene->m_pEffectContainer->AddChild(pBash);
            }
            else if (m_stEffectEvent.sEffectIndex == 23)
            {
                auto vec = vecDest;
                for (int i = 0; i < 4; i++)
                {
                    vec = { ((float)(2 * (i % 2)) + vecDest.x) - 1.0f, vecDest.y, ((float)(2 * (i / 2)) + vecDest.z) - 1.0f };
                    auto pFreeze = new TMSkillFreezeBlade(vec, 1, 0, 0);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pFreeze);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 24)
            {
                vecDest.y += 1.0f;
                auto pMagic = new TMSkillMagicArrow(vecStart, vecDest, 0, nullptr);
                g_pCurrentScene->m_pEffectContainer->AddChild(pMagic);
            }
            else if (m_stEffectEvent.sEffectIndex == 25)
            {
                auto pCure = new TMSkillCure(vecDest, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pCure);
            }
            else if (m_stEffectEvent.sEffectIndex == 27 || m_stEffectEvent.sEffectIndex == 29)
            {
                int Type = 0;
                if (m_stEffectEvent.sEffectIndex == 27)
                    Type = 3;
                if (m_stEffectEvent.sEffectIndex == 29)
                    Type = 4;

                auto pHeal = new TMSkillHeal(vecDest, 0, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pHeal);
            }
            else if (m_stEffectEvent.sEffectIndex == 31)
            {
                if (m_stEffectEvent.pTarget)
                {
                    auto vec = vecDest;
                    vec.y = (float)pScene->GroundGetMask(TMVector2(vecDest.x, vecDest.z)) * 0.1f;

                    auto pHaste = new TMSkillHaste(vec, 4);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pHaste);

                    auto pHaste2 = new TMSkillHaste(vecStart, 4);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pHaste2);

                    bool bExpand = false;
                    if (m_stEffectEvent.pTarget->m_nClass == 4 || m_stEffectEvent.pTarget->m_nClass == 8)
                        bExpand = true;

                    auto pEffect = new TMEffectSkinMesh(m_stEffectEvent.pTarget->m_nSkinMeshType,
                        { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0 }, 0, 0);

                    if (!m_stEffectEvent.pTarget || m_stEffectEvent.pTarget->m_cMount <= 0 || !m_stEffectEvent.pTarget->m_pMount)
                    {
                        memcpy(&pEffect->m_stLookInfo, &m_stEffectEvent.pTarget->m_stLookInfo, sizeof(pEffect->m_stLookInfo));
                    }
                    else
                    {
                        pEffect->m_nSkinMeshType = m_stEffectEvent.pTarget->m_nMountSkinMeshType;
                        memcpy(
                            &pEffect->m_stLookInfo, &m_stEffectEvent.pTarget->m_stMountLook, sizeof(pEffect->m_stLookInfo));
                        pEffect->m_nSkinMeshType2 = m_stEffectEvent.pTarget->m_nSkinMeshType;
                        memcpy(&pEffect->m_stLookInfo2, &m_stEffectEvent.pTarget->m_stLookInfo, sizeof(pEffect->m_stLookInfo2));
                    }

                    pEffect->m_StartColor.r = 0.5f;
                    pEffect->m_StartColor.g = 0.5f;
                    pEffect->m_StartColor.b = 0.5f;
                    pEffect->InitObject(bExpand);
                    pEffect->m_nFade = 1;
                    pEffect->m_dwLifeTime = 1500;
                    pEffect->InitPosition(m_stEffectEvent.pTarget->m_vecPosition.x,
                        m_stEffectEvent.pTarget->m_fHeight + 0.1f, m_stEffectEvent.pTarget->m_vecPosition.y);

                    if (m_cMount > 0 && m_pMount)
                    {
                        if (pEffect->m_pSkinMesh)
                            pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffect->m_nSkinMeshType].dwAniTable[0]);
                        if (pEffect->m_pSkinMesh2)
                            pEffect->m_pSkinMesh2->SetAnimation(g_MobAniTable[pEffect->m_nSkinMeshType2].dwAniTable[24]);
                    }
                    else
                        pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffect->m_nSkinMeshType].dwAniTable[0]);

                    pEffect->m_fStartAngle = m_stEffectEvent.pTarget->m_fAngle;
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->m_nMotionType = 9;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 32)
            {
                auto pFire = new TMSkillFire(vecDest, 0, m_stEffectEvent.pTarget, 0xFFFFFFFF, 0x22331100);
                g_pCurrentScene->m_pEffectContainer->AddChild(pFire);
            }
            else if (m_stEffectEvent.sEffectIndex == 34)
            {
                vecDest.y += 1.0f;
                auto pSpear = new TMSkillIceSpear(vecStart, vecDest, 0, nullptr);
                g_pCurrentScene->m_pEffectContainer->AddChild(pSpear);
            }
            else if (m_stEffectEvent.sEffectIndex == 37)
            {
                TMEffectBillBoard* pLightenStorm[2]{};

                float fScale = 1.0f;
                if (m_cMount == 1)
                    fScale = 1.3f;

                for (int i = 0; i < 2; i++)
                {
                    float fRand = (float)(rand() % 5);
                    if (!pLightenStorm[i])
                    {
                        pLightenStorm[i] = new TMEffectBillBoard(109,
                            0,
                            (((0.2f * fRand) + 1.0f) * fScale) - ((float)i * 0.4f),
                            (((0.2f * fRand) + 1.0f) * fScale) - ((float)i * 0.4f),
                            (((0.2f * fRand) + 1.0f) * fScale) - ((float)i * 0.4f),
                            0.0f,
                            8,
                            80);

                        pLightenStorm[i]->SetColor(0xFFFFDD00);
                        pLightenStorm[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        pLightenStorm[i]->m_nFade = 0;
                        pLightenStorm[i]->m_dwLifeTime = 2000;
                        g_pCurrentScene->m_pEffectContainer->AddChild(pLightenStorm[i]);
                    }

                    if (pLightenStorm[i])
                    {
                        pLightenStorm[i]->m_fAxisAngle = (((float)((dwServerTime - pLightenStorm[i]->m_dwCreateTime) % 1000) * D3DXToRadian(360)) / 1000.0f) + (float)i;
                        pLightenStorm[i]->m_vecPosition = { m_vecPosition.x + 0.5f, m_fHeight + 1.8f, m_vecPosition.y + 0.5f };
                    }

                    auto pEffect = new TMEffectBillBoard(109,
                        500,
                        ((0.2f * fRand) + 1.0f) * fScale,
                        ((0.2f * fRand) + 1.0f) * fScale,
                        ((0.2f * fRand) + 1.0f) * fScale,
                        0.0f,
                        8,
                        80);
                    pEffect->SetColor(0xFFFFDD00);
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->m_nFade = 1;
                    pEffect->m_dwLifeTime = 2000;
                    pEffect->m_fAxisAngle = (((float)((dwServerTime - pEffect->m_dwCreateTime) % 1000) * D3DXToRadian(360)) / 1000.0f);
                    pEffect->m_vecPosition = { m_vecPosition.x + 0.5f, m_fHeight + 1.8f, m_vecPosition.y + 0.5f };
                    pEffect->m_pOwner = this;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
                GetSoundAndPlay(105, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 38)
            {
                vecDest.y += 1.0f;
                vecStart.y += 1.0f;

                int nType = 1;
                if (m_nClass == 38)
                    nType = 2;

                auto pMagicArrow = new TMSkillMagicArrow(vecStart, vecDest, nType, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pMagicArrow);

                vecDest.y += 1.0f;
                auto pDoubleSwing = new TMSkillDoubleSwing(vecStart, vecDest, 4, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pDoubleSwing);
            }
            else if (m_stEffectEvent.sEffectIndex == 40 || m_stEffectEvent.sEffectIndex == 21)
            {
                unsigned int dwColor = 0xFF33FF66;
                if (m_stEffectEvent.sEffectLevel == 1)
                    dwColor = 0xFF66FFAA;
                if (m_stEffectEvent.sEffectLevel == 2)
                    dwColor = 0xFF113388;
                if (m_stEffectEvent.sEffectLevel == 3)
                    dwColor = 0xFFFF8800;

                auto pPoison = new TMSkillPoison(vecDest, dwColor, 10, 1, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pPoison);
            }
            else if (m_stEffectEvent.sEffectIndex == 41 || m_stEffectEvent.sEffectIndex == 43 || m_stEffectEvent.sEffectIndex == 90 || m_stEffectEvent.sEffectIndex == 54)
            {
                auto vec = vecDest;
                vec.y = (float)pScene->GroundGetMask(TMVector2(vecDest.x, vecDest.z)) * 0.1f;

                int nType = 0;
                if (m_stEffectEvent.sEffectIndex == 43)
                    nType = 1;
                if (m_stEffectEvent.sEffectIndex == 90)
                    nType = 2;
                if (m_stEffectEvent.sEffectIndex == 54)
                    nType = 3;

                auto pHaste = new TMSkillHaste(vec, nType);
                g_pCurrentScene->m_pEffectContainer->AddChild(pHaste);
            }
            else if (m_stEffectEvent.sEffectIndex == 42)
            {
                auto pPortal = new TMSkillTownPortal({ vecStart.x, vecStart.y - 1.0f, vecStart.z }, 0);
                g_pCurrentScene->m_pEffectContainer->AddChild(pPortal);
            }
            else if (m_stEffectEvent.sEffectIndex == 44)
            {
                const static unsigned int dwColor[3] = { 0xFF550088, 0xFF555500, 0xFF005500 };
                for (int i = 0; i < 3; i++)
                {
                    auto pBill1 = new TMEffectBillBoard(56,
                        800,
                        0.4f - ((float)i * 0.0099999998f),
                        0.4f - ((float)i * 0.0099999998f),
                        0.4f - ((float)i * 0.0099999998f),
                        0.002f,
                        1,
                        80);

                    pBill1->SetColor(dwColor[i]);
                    pBill1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pBill1->m_nFade = 1;
                    pBill1->m_vecPosition = { vecStart.x, vecStart.y + 1.3f, vecStart.z };
                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill1);

                    float fRand = (float)(rand() % 5) + 5.0f;
                    auto pBill2 = new TMEffectBillBoard(60,
                        500,
                        (fRand * 0.0099999998f) + 0.4f,
                        (fRand * 0.0099999998f) + 0.4f,
                        (fRand * 0.0099999998f) + 0.4f,
                        0.0005f,
                        1,
                        80);
                    pBill2->SetColor(0xFF333355);
                    pBill2->m_fAxisAngle = (float)(D3DXToRadian(180) * fRand) / 3.0f;
                    pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pBill2->m_nFade = 1;
                    pBill2->m_vecPosition = { vecStart.x, vecStart.y + 1.3f, vecStart.z };
                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                }

                GetSoundAndPlay(159, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 46)
            {
                auto pParticle1 = new TMEffectParticle(vecStart, 3, 15, 1.0f, 0xFFFF3300, 0, 122, 1.0f, 1, { 0.0f, 0.0f, 0.0f }, 1000);
                g_pCurrentScene->m_pEffectContainer->AddChild(pParticle1);
                auto pParticle2 = new TMEffectParticle(vecStart, 1, 15, 0.3f, 0xFFFF3300, 0, 56, 1.0f, 1, { 0.0f, 0.0f, 0.0f }, 1000);
                g_pCurrentScene->m_pEffectContainer->AddChild(pParticle2);
                auto pParticle3 = new TMEffectParticle(vecStart, 2, 15, 1.0f, 0xFF0033FF, 0, 122, 1.0f, 1, { 0.0f, 0.0f, 0.0f }, 1000);
                g_pCurrentScene->m_pEffectContainer->AddChild(pParticle3);
                auto pParticle = new TMEffectParticle(vecStart, 2, 15, 0.3f, 0xFF0033FF, 0, 56, 1.0f, 1, { 0.0f, 0.0f, 0.0f }, 1000);
                g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);

                auto pLightMap = new TMShade(2, 7, 1.0f);
                pLightMap->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pLightMap->SetColor(0xFFAA3300);
                pLightMap->SetPosition({ vecStart.x, vecStart.y });
                pLightMap->m_dwLifeTime = 1500;
                g_pCurrentScene->m_pEffectContainer->AddChild(pLightMap);

                GetSoundAndPlay(36, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 48 || m_stEffectEvent.sEffectIndex == 49)
            {
                vecStart.y -= 1.0f;

                auto pJudgement = new TMSkillJudgement(vecStart, 3, 0.1f);
                g_pCurrentScene->m_pEffectContainer->AddChild(pJudgement);

                auto pEffect = new TMEffectSkinMesh(20, vecStart, vecDest, m_stEffectEvent.sEffectIndex - 48, m_stEffectEvent.pTarget);
                if (m_stEffectEvent.sEffectIndex == 48)
                {
                    pEffect->m_stLookInfo.Mesh1 = 4;
                    pEffect->m_stLookInfo.Mesh0 = 4;
                    pEffect->m_stLookInfo.Skin1 = 1;
                    pEffect->m_stLookInfo.Skin0 = 1;
                    pEffect->InitObject(0);
                    pEffect->m_pSkinMesh->m_vScale.x = 0.4f;
                    pEffect->m_pSkinMesh->m_vScale.y = 0.4f;
                    pEffect->m_pSkinMesh->m_vScale.z = 0.4f;
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                }
                else if (m_stEffectEvent.sEffectIndex == 49)
                {
                    pEffect->m_stLookInfo.Mesh1 = 2;
                    pEffect->m_stLookInfo.Mesh0 = 2;
                    pEffect->m_stLookInfo.Skin1 = 0;
                    pEffect->m_stLookInfo.Skin0 = 0;
                    pEffect->InitObject(0);
                    pEffect->m_pSkinMesh->m_vScale.x = 0.2f;
                    pEffect->m_pSkinMesh->m_vScale.y = 0.2f;
                    pEffect->m_pSkinMesh->m_vScale.z = 0.2f;
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_DEFAULT;
                }

                pEffect->m_StartColor.r = 1.0f;
                pEffect->m_StartColor.g = 1.0f;
                pEffect->m_StartColor.b = 1.0f;
                pEffect->m_nFade = 1;
                pEffect->m_fStartAngle = m_fAngle;
                pEffect->m_nMotionType = 2;
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
            else if (m_stEffectEvent.sEffectIndex == 50)
            {
                vecStart.x -= 0.5f;
                vecStart.y -= 1.0f;
                vecStart.z += 0.5f;

                for (int i = 0; i < 3; i++)
                {
                    vecStart.x = ((float)i * 0.60000002f) + vecStart.x;
                    vecStart.z = vecStart.z - ((float)i * 0.60000002f);

                    auto pEffect = new TMEffectSkinMesh(32, vecStart, vecDest, m_stEffectEvent.sEffectIndex - 48, m_stEffectEvent.pTarget);
                    pEffect->m_stLookInfo.Mesh1 = 0;
                    pEffect->m_stLookInfo.Mesh0 = 0;
                    pEffect->m_stLookInfo.Skin1 = 0;
                    pEffect->m_stLookInfo.Skin0 = 0;
                    pEffect->InitObject(0);
                    pEffect->m_pSkinMesh->m_vScale.x = 2.0f;
                    pEffect->m_pSkinMesh->m_vScale.y = 2.0f;
                    pEffect->m_pSkinMesh->m_vScale.z = 2.0f;
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->m_StartColor.r = 1.0f;
                    pEffect->m_StartColor.g = 1.0f;
                    pEffect->m_StartColor.b = 1.0f;
                    pEffect->m_nFade = 1;
                    pEffect->m_fStartAngle = m_fAngle;
                    pEffect->m_nMotionType = 4;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 51)
            {
                TMVector3 vec{ m_vecPosition.x, m_fHeight, m_vecPosition.y };
                auto pSlow = new TMSkillSlowSlash(vec, vec, 1, m_stEffectEvent.pTarget);
                g_pCurrentScene->m_pEffectContainer->AddChild(pSlow);

                GetSoundAndPlay(157, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 53)
            {
                for (int i = 0; i < 5; i++)
                {
                    int nRand = rand() % 10 + 10;
                    float fAddScale = (float)i * 0.3f;

                    TMEffectBillBoard* mpBill = new TMEffectBillBoard(0,
                        200 * i + 1200,
                        (((float)nRand * 0.1f) + 0.60000002f) + fAddScale,
                        (((float)nRand * 0.30000001f) + 0.60000002f) + fAddScale,
                        (((float)nRand * 0.1f) + 0.60000002f) + fAddScale,
                        0.000099999997f,
                        1,
                        80);

                    mpBill->m_vecPosition = { ((float)(rand() % 10 - 5) * 0.02f) + m_vecPosition.x, m_fHeight, ((float)(rand() % 10 - 5) * 0.02f) + m_vecPosition.y };
                    mpBill->m_vecStartPos = mpBill->m_vecPosition;
                    mpBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    mpBill->m_bStickGround = 0;
                    mpBill->m_nParticleType = 1;
                    mpBill->SetColor(0xFFFFFFFF);
                    g_pCurrentScene->m_pEffectContainer->AddChild(mpBill);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 72 || m_stEffectEvent.sEffectIndex == 80)
            {
                if (m_stEffectEvent.pTarget)
                {
                    int nType = 0;
                    if (m_stEffectEvent.sEffectIndex == 80)
                        nType = 1;

                    const static unsigned int dwCol[2][2]{ 0xFFFFFFFF, 0xFFFFAA55, 0xFFFFFFFF, 0xFFFF0000 };
                    auto pCamera = g_pObjectManager->GetCamera();
                    if (pCamera->m_pFocusedObject == this)
                        pCamera->EarthQuake(1);

                    auto pLightMap = new TMShade(nType + 2, 7, 1.0f);
                    pLightMap->SetColor(dwCol[nType][1]);
                    pLightMap->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pLightMap->SetPosition({ m_stEffectEvent.pTarget->m_vecPosition.x, m_stEffectEvent.pTarget->m_vecPosition.y });
                    pLightMap->m_dwLifeTime = 800;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pLightMap);

                    for (int i = 0; i < 2; i++)
                    {
                        auto pBill = new TMEffectBillBoard(4 * nType + 56,
                            700,
                            ((float)i * 0.2f) + 1.0f,
                            ((float)i * 0.2f) + 1.0f,
                            ((float)i * 0.2f) + 1.0f,
                            ((float)nType * 0.0049999999f) + 0.001f,
                            1,
                            80);

                        pBill->SetColor(dwCol[nType][i]);
                        pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        pBill->m_nFade = 1;
                        pBill->m_vecPosition = { m_stEffectEvent.pTarget->m_vecPosition.x, m_stEffectEvent.pTarget->m_fHeight + 1.0f,
                                                    m_stEffectEvent.pTarget->m_vecPosition.y };
                        g_pCurrentScene->m_pEffectContainer->AddChild(pBill);
                    }
                    GetSoundAndPlay(11 * nType + 160, 0, 0);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 74)
            {
                for (int i = 0; i < 5; i++)
                {
                    auto pEnchant = new TMEffectBillBoard(56,
                        200 * i + 500,
                        ((float)i * 0.69999999f) + 1.0f,
                        ((float)i * 0.5f) + 1.0f,
                        ((float)i * 0.69999999f) + 1.0f,
                        0.0,
                        1,
                        80);

                    pEnchant->SetColor(0xFF55AAFF);
                    pEnchant->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEnchant->m_nFade = 1;
                    pEnchant->m_vecPosition = { vecStart.x, (((float)i * 0.30000001f) + vecStart.y) - 0.5f, vecStart.z };
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEnchant);
                }
                GetSoundAndPlay(174, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 76)
            {
                const static unsigned int dwColor[5]{ 0xFF99BBFF, 0xFF00FFAA, 0xFFFFAA00, 0xFF880088, 0xFFCC8888 };

                for (int i = 0; i < 5; i++)
                {
                    TMEffectMesh* pImmunity = new TMEffectMesh(12, dwColor[i], 0.0f, 1);
                    pImmunity->m_nTextureIndex = 0;
                    pImmunity->m_vecPosition = m_vecSkinPos;
                    pImmunity->m_vecPosition.y += 1.1f;
                    pImmunity->m_dwLifeTime = 50 * i + 1300;
                    pImmunity->m_fScaleH = 1.0f;
                    pImmunity->m_fScaleV = 1.0f;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pImmunity);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 77)
            {
                for (int i = 0; i < 5; i++)
                {
                    auto pBill = new TMEffectBillBoard(101,
                        100 * i + 500,
                        0.12f - ((float)i * 0.0099999998f),
                        0.2f - ((float)i * 0.0099999998f),
                        0.2f - ((float)i * 0.0099999998f),
                        0.0020000001f,
                        8,
                        80);

                    pBill->SetColor(0xFF5500FF);
                    pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pBill->m_nFade = 1;
                    pBill->m_vecPosition = { m_vecPosition.x, (float)(m_fHeight + 0.2f) + ((float)i * 0.2f), m_vecPosition.y };
                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill);

                    float fRand = (float)(rand() % 5);
                    auto pBill2 = new TMEffectBillBoard(101,
                        100 * i + 500,
                        (fRand * 0.0099999998f) + 0.1f,
                        (fRand * 0.0099999998f) + 0.1f,
                        (fRand * 0.0099999998f) + 0.1f,
                        0.00050000002f,
                        8,
                        80);

                    pBill2->SetColor(0xFFFFFFFF);
                    pBill2->m_fAxisAngle = (D3DXToRadian(180) * fRand) / 3.0f;
                    pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pBill2->m_nFade = 1;
                    pBill2->m_vecPosition = { m_vecPosition.x, (float)(m_fHeight + 0.2f) + ((float)i * 0.2f), m_vecPosition.y };
                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 79)
            {
                for (int i = 0; i < 6; i++)
                {
                    auto pSoul = new TMEffectBillBoard(56,
                        50 * i + 800,
                        2.0,
                        ((float)i * 0.2f) + 1.0f,
                        2.0f,
                        0.0f,
                        1,
                        80);

                    pSoul->SetColor(0xFFFFFFFF);
                    pSoul->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pSoul->m_nFade = 1;
                    pSoul->m_vecPosition = { vecStart.x, vecStart.y + 0.2f, vecStart.z };
                    g_pCurrentScene->m_pEffectContainer->AddChild(pSoul);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 86)
            {
                auto pLevelUp = new TMEffectLevelUp({ m_vecPosition.x, m_fHeight, m_vecPosition.y }, 1);
                g_pCurrentScene->m_pEffectContainer->AddChild(pLevelUp);

                GetSoundAndPlay(37, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 87)
            {
                auto pSpChange = new TMSkillSpChange({ m_vecPosition.x, m_fHeight, m_vecPosition.y }, 0, this);
                g_pCurrentScene->m_pEffectContainer->AddChild(pSpChange);
            }
            else if (m_stEffectEvent.sEffectIndex == 88 && m_pSkinMesh)
            {
                if (m_stEffectEvent.pTarget)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        bool bExpand = false;
                        if (m_nClass == 4 || m_nClass == 8)
                            bExpand = true;

                        auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, { m_vecPosition.x, m_fHeight, m_vecPosition.y },
                            { m_stEffectEvent.pTarget->m_vecPosition.x, m_stEffectEvent.pTarget->m_fHeight,m_stEffectEvent.pTarget->m_vecPosition.y },
                            0, nullptr);

                        pEffect->m_fScale = m_fScale;
                        if (m_cMount > 0 && m_pMount)
                        {
                            bExpand = false;
                            memcpy(&pEffect->m_stLookInfo, &m_stMountLook, sizeof(pEffect->m_stLookInfo));
                            pEffect->m_nSkinMeshType = m_nMountSkinMeshType;
                        }
                        else
                        {
                            memcpy(
                                &pEffect->m_stLookInfo,
                                &m_stLookInfo,
                                sizeof(pEffect->m_stLookInfo));
                        }

                        pEffect->m_nFade = 1;
                        pEffect->m_StartColor.r = 0.5f;
                        pEffect->m_StartColor.g = 0.3f;
                        pEffect->m_StartColor.b = 0.2f;
                        pEffect->InitObject(bExpand);
                        pEffect->m_dwStartTime = dwServerTime;
                        pEffect->m_dwLifeTime = 200 * i + 100;
                        pEffect->InitPosition(m_vecPosition.x, m_fHeight + 0.1f, m_vecPosition.y);
                        pEffect->m_fStartAngle = m_fAngle;
                        pEffect->m_fAngle = pEffect->m_fStartAngle;
                        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion]]);
                        pEffect->m_pSkinMesh->m_dwFPS = m_pSkinMesh->m_dwFPS;
                        pEffect->m_nMotionType = 6;
                        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                    }

                    GetSoundAndPlay(160, 0, 0);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 89 && m_pSkinMesh)
            {
                for (int i = 0; i < 5; i++)
                {
                    bool bExpand = false;
                    if (m_nClass == 4 || m_nClass == 8)
                        bExpand = true;

                    auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0, nullptr);
                    if (m_cMount > 0 && m_pMount)
                    {
                        bExpand = false;
                        pEffect->m_nSkinMeshType = m_nMountSkinMeshType;
                        memcpy(&pEffect->m_stLookInfo, &m_stMountLook, sizeof(pEffect->m_stLookInfo));
                        pEffect->m_fScale = m_fScale;
                        pEffect->InitPosition(m_pMount->m_vPosition.x, m_fHeight + 0.1f, m_pMount->m_vPosition.z);
                    }
                    else
                    {
                        memcpy(
                            &pEffect->m_stLookInfo,
                            &m_stLookInfo,
                            sizeof(pEffect->m_stLookInfo));
                        pEffect->m_fScale = m_fScale;
                        pEffect->InitPosition(m_vecPosition.x, m_fHeight + 0.1f, m_vecPosition.y);
                    }

                    pEffect->m_nFade = 1;
                    pEffect->m_StartColor.r = 0.3f;
                    pEffect->m_StartColor.g = 0.3f;
                    pEffect->m_StartColor.b = 0.3f;
                    pEffect->InitObject(bExpand);
                    pEffect->m_dwStartTime = dwServerTime + 100 * i;
                    pEffect->m_dwLifeTime = 50 * i + 400;
                    pEffect->m_fStartAngle = m_fAngle;
                    pEffect->m_fAngle = pEffect->m_fStartAngle;
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion]]);
                    if (m_cMount > 0 && m_pMount && pEffect->m_pSkinMesh)
                    {
                        pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[3]);
                    }

                    pEffect->m_pSkinMesh->m_dwFPS = m_pSkinMesh->m_dwFPS;
                    pEffect->m_nMotionType = 0;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
            }
            else if (m_stEffectEvent.sEffectIndex == 91)
            {
                GetSoundAndPlay(169, 0, 0);
            }
            else if (m_stEffectEvent.sEffectIndex == 95)
            {
                vecDest.y += 1.2f;
                auto vecTempDest = vecDest;
                int nCount = 3;
                if (m_stEffectEvent.pTarget)
                {
                    vecTempDest = { m_stEffectEvent.pTarget->m_vecPosition.x,
                        (float)(m_stEffectEvent.pTarget->m_fScale / 1.5f)
                        + m_stEffectEvent.pTarget->m_fHeight,
                        m_stEffectEvent.pTarget->m_vecPosition.y };

                    nCount += m_stScore.Mastery[3] / 80;
                    if (nCount < 3)
                        nCount = 3;
                    if (nCount > 6)
                        nCount = 6;
                }

                auto vecTempStart = vecStart;
                int nColor = 0;

                for (int i = 0; i < nCount; i++)
                {
                    int nRand = rand() % 1000;
                    if (i > 1)
                        nColor = i + 3;

                    vecTempStart.x = (vecTempStart.x - 0.1f) + ((m_fScale * 1.0f) * (sinf((((float)i / 6.0f) * D3DXToRadian(180)) * 2.0f)));
                    vecTempStart.z = (vecTempStart.z - 0.1f) + ((m_fScale * 1.0f) * (cosf((((float)i / 6.0f) * D3DXToRadian(180)) * 2.0f)));
                    vecTempStart.y = m_fHeight + m_fScale;

                    unsigned int BillColor = 0x00777777;
                    switch (nColor)
                    {
                    case 8:
                        BillColor = 0x00883333;
                        break;
                    case 7:
                        BillColor = 0x00884388;
                        break;
                    case 6:
                        BillColor = 0x00338843;
                        break;
                    case 5:
                        BillColor = 0x00222288;
                        break;
                    }

                    auto pBillBoard = new TMEffectBillBoard(56,
                        100 * i + 400,
                        1.0f,
                        1.0f,
                        1.0f,
                        0.0f,
                        1,
                        80);

                    pBillBoard->SetColor(BillColor);
                    pBillBoard->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pBillBoard->m_nFade = 2;
                    pBillBoard->m_vecPosition = vecTempStart;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pBillBoard);

                    auto pArrow = new TMArrow(vecTempStart, vecTempDest, m_stEffectEvent.sEffectLevel, 151, 0, nColor, 0);
                    pArrow->m_dwStartTime += 100 * i;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pArrow);
                }
            }
            else if (m_stEffectEvent.sEffectIndex >= 151 && m_stEffectEvent.sEffectIndex <= 153 ||
                m_stEffectEvent.sEffectIndex == 104 || m_stEffectEvent.sEffectIndex == 105)
            {
                vecStart.y = ((float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) * 0.69999999f) + m_fHeight;
                if (m_nSkinMeshType == 8)
                    vecStart.y = (float)(1.0f * m_fScale) + vecStart.y;

                vecDest.y += 1.2f;
                auto vecTempDest = vecDest;
                if (m_stEffectEvent.pTarget)
                {
                    vecTempDest = { m_stEffectEvent.pTarget->m_vecPosition.x,
                        (float)(m_stEffectEvent.pTarget->m_fScale / 2.5f) + m_stEffectEvent.pTarget->m_fHeight,
                        m_stEffectEvent.pTarget->m_vecPosition.y };
                }

                ++m_nDoubleCount;

                if (m_cSoul == 1 && m_pSoul[0] && m_cDie != 1 && !m_cShadow)
                {
                    auto pArrow = new TMArrow(m_pSoul[0]->m_vecPosition, vecTempDest, m_stEffectEvent.sEffectLevel, 10000, m_cAvatar, 0, m_nAttackDestID);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pArrow);
                }
                if (m_stEffectEvent.sEffectIndex == 105 && m_stEffectEvent.sEffectLevel == 2)
                {
                    auto pSwing = new TMSkillDoubleSwing(vecStart, vecTempDest, 3, m_stEffectEvent.pTarget);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pSwing);
                    m_stEffectEvent.dwTime += 700;
                }
                else
                {
                    auto pMobData = &g_pObjectManager->m_stMobData;
                    int left = g_pObjectManager->m_stMobData.Equip[6].sIndex;
                    if (left == 1010)
                        m_stEffectEvent.sEffectLevel = 99;

                    auto pArrow = new TMArrow(vecStart, vecTempDest, m_stEffectEvent.sEffectLevel, m_stEffectEvent.sEffectIndex,
                        m_cAvatar, m_stSancInfo.Legend7, m_nAttackDestID);
                    g_pCurrentScene->m_pEffectContainer->AddChild(pArrow);
                    m_stEffectEvent.dwTime += 500;
                }
            }
        }
        if ((m_stEffectEvent.sEffectIndex < 151 || m_stEffectEvent.sEffectIndex > 153) &&
            m_stEffectEvent.sEffectIndex != 104 && m_stEffectEvent.sEffectIndex != 105 ||
            !m_bDoubleAttack || m_nDoubleCount >= 2)
        {
            memset(&m_stEffectEvent, 0, sizeof(m_stEffectEvent));
            m_nDoubleCount = 0;
        }
    }

    // FUN_00506f9d keeps two rotating texture-109 billboards alive while the
    // 7.48 Lighten flag is set and destroys them as soon as the affect ends.
    if (m_cLighten == 1 && !g_bHideEffect)
    {
        if (dwServerTime - m_dwLastLighten > 500)
        {
            float fRand = (float)(rand() % 5);

            float fScale = 0.6f;
            if (m_cMount == 1)
                fScale = 1.3f;

            for (int i = 0; i < 2; i++)
            {
                if (!m_pLightenStorm[i])
                {
                    m_pLightenStorm[i] = new TMEffectBillBoard(109,
                        0,
                        (((0.2f * fRand) + 1.0f) * fScale) - ((float)i * 0.40000001f),
                        (((0.2f * fRand) + 1.0f) * fScale) - ((float)i * 0.40000001f),
                        (((0.2f * fRand) + 1.0f) * fScale) - ((float)i * 0.40000001f),
                        0.0,
                        8,
                        80);

                    m_pLightenStorm[i]->SetColor(0xFFFFDD00);
                    m_pLightenStorm[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    m_pLightenStorm[i]->m_nFade = 0;
                    g_pCurrentScene->m_pEffectContainer->AddChild(m_pLightenStorm[i]);
                }
                if (m_pLightenStorm[i])
                {
                    m_pLightenStorm[i]->m_fAxisAngle = (((float)((dwServerTime - m_pLightenStorm[i]->m_dwCreateTime) % 1000) * D3DXToRadian(360)) / 1000.0f) + (float)i;
                    m_pLightenStorm[i]->m_vecPosition = { m_vecPosition.x + 0.5f, m_fHeight + 1.8f, m_vecPosition.y + 0.5f };
                }
            }
            m_dwLastLighten = dwServerTime;
        }
        for (int i = 0; i < 2; ++i)
        {
            if (m_pLightenStorm[i])
                m_pLightenStorm[i]->m_vecPosition = { m_vecSkinPos.x, m_vecSkinPos.y + 1.3f, m_vecSkinPos.z + 0.1f };
        }
    }
    else
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_pLightenStorm[i])
            {
                g_pObjectManager->DeleteObject(m_pLightenStorm[i]);
                m_pLightenStorm[i] = nullptr;
            }
        }
    }

    // FUN_00506f9d restarts the stock magic-shield object once per second
    // while affect flag 11 is active; CheckAffect already owns that flag.
    if (m_cShield == 1 && dwServerTime - m_dwLastMagicShield > 1000)
    {
        if (m_pMagicShield)
            m_pMagicShield->StartVisible(dwServerTime);
        m_dwLastMagicShield = dwServerTime;
    }

    if (m_cCancel == 1)
    {
        unsigned int CancelTime = 1000;
        if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
            CancelTime = 3000;
        if (dwServerTime - m_dwLastCancelTime > CancelTime && m_pCancelation)
        {
            if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                m_pCancelation->m_fVectorH = (float)-(rand() % 50) / 100.0f;

            m_pCancelation->StartVisible(dwServerTime);
            m_pCancelation->m_fCancelScale = TMHuman::m_vecPickSize[m_nSkinMeshType].x * m_fScale;
            m_dwLastCancelTime = dwServerTime;
        }
    }
    if (m_cWaste == 1 && dwServerTime - m_dwLastWaste > 1000)
    {
        TMVector3 vec{ m_vecPosition.x, m_fHeight, m_vecPosition.y };
        auto pSlow = new TMSkillSlowSlash(vec, vec, 1, m_stEffectEvent.pTarget);
        g_pCurrentScene->m_pEffectContainer->AddChild(pSlow);
        m_dwLastWaste = dwServerTime;
    }

    if ((m_cImmunity == 1 || m_nClass == 44) && m_cDie != 1 && !g_bHideEffect)
    {
        const static unsigned int dwColor[5]{ 0xFF99BBFF, 0xFF00FFAA, 0xFFFFAA00, 0xFF880088, 0xFFCC8888 };
        for (int i = 0; i < 2; i++)
        {
            if (m_pImmunity[i])
            {
                float fAngle = ((((float)(i % 5) * 1.0f) / 5.0f) + ((float)(dwServerTime % 5000) / 5000.0f)) * D3DXToRadian(360);
                if (!(i % 2))
                    m_pImmunity[i]->m_fAngle = fAngle;
                if (i % 3 == 1)
                    m_pImmunity[i]->m_fAngle2 = fAngle;
                if (m_cMount == 1)
                {
                    m_pImmunity[i]->m_vecPosition = { m_vecSkinPos.x, m_vecSkinPos.y, m_vecSkinPos.z };
                    m_pImmunity[i]->m_fScaleH = 1.5f;
                    m_pImmunity[i]->m_fScaleV = 1.5f;
                }
                else
                {
                    m_pImmunity[i]->m_vecPosition = { m_vecPosition.x, ((float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) / 2.0f)
                        + m_fHeight,
                        m_vecPosition.y };

                    m_pImmunity[i]->m_fScaleH = 1.0f;
                    m_pImmunity[i]->m_fScaleV = 1.0f;
                }

                if (m_nClass == 44)
                {
                    m_pImmunity[i]->m_nTextureIndex = 45;
                    m_pImmunity[i]->m_fScaleH = m_fScale;
                    m_pImmunity[i]->m_fScaleV = m_fScale;
                    m_pImmunity[i]->m_vecPosition.y = (float)(m_fScale * 5.0999999f) + m_pImmunity[i]->m_vecPosition.y;
                }
            }
            else
            {
                m_pImmunity[i] = new TMEffectMesh(12, dwColor[i], 0.0f, 1);
                m_pImmunity[i]->m_nTextureIndex = 0;
                m_pImmunity[i]->m_vecPosition = m_vecSkinPos;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pImmunity[i]);
            }
        }
    }
    else
    {
        for (int i = 0; i < 5; ++i)
        {
            if (m_pImmunity[i])
            {
                g_pObjectManager->DeleteObject(m_pImmunity[i]);
                m_pImmunity[i] = nullptr;
            }
        }
    }

    if (!g_bHideSkillBuffEffect2 && m_cElimental == 1 && dwServerTime - m_dwElimental > 250)
    {
        TMVector3 vec{ m_vecPosition.x, m_fHeight, m_vecPosition.y };

        auto pSlow = new TMSkillSlowSlash(vec, vec, 2, this);
        g_pCurrentScene->m_pEffectContainer->AddChild(pSlow);

        m_dwElimental = dwServerTime;
        if (m_pEleStream)
            m_pEleStream->StartVisible(dwServerTime);
    }
    if (m_cAurora == 1)
    {
        if (m_pAurora)
            m_pAurora->m_vecPosition = { m_vecPosition.x, m_fHeight + 0.30000001f, m_vecPosition.y };
        else
        {
            m_pAurora = new TMEffectBillBoard2(94, 0, 1.5f, 1.5f, 1.5f, 0.0, 3000);
            if (m_pAurora)
            {
                m_pAurora->m_nFade = 2;
                m_pAurora->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pAurora->m_vecPosition = { m_vecPosition.x,
                    m_fHeight + 0.30000001f,
                    m_vecPosition.y };
                m_pAurora->SetColor(0x33333333);
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pAurora);
            }
        }
    }
    else if (m_pAurora)
    {
        g_pObjectManager->DeleteObject(m_pAurora);
        m_pAurora = nullptr;
    }

    if (!g_bHideSkillBuffEffect && m_cDodge == 1 && (m_eMotion == ECHAR_MOTION::ECMOTION_WALK || m_eMotion == ECHAR_MOTION::ECMOTION_RUN) && dwServerTime - m_dwDodgeTime > 80)
    {
        bool bExpand = false;
        if (m_nClass == 4 || m_nClass == 8)
            bExpand = true;

        auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0, nullptr);
        if (m_cMount > 0 && m_pMount)
        {
            bExpand = false;
            pEffect->m_nSkinMeshType = m_nMountSkinMeshType;
            memcpy(&pEffect->m_stLookInfo, &m_stMountLook, sizeof(pEffect->m_stLookInfo));
            pEffect->m_fScale = m_fScale;
            pEffect->InitPosition(m_pMount->m_vPosition.x, m_fHeight + 0.1f, m_pMount->m_vPosition.z);
        }
        else
        {
            memcpy(
                &pEffect->m_stLookInfo,
                &m_stLookInfo,
                sizeof(pEffect->m_stLookInfo));
            pEffect->m_fScale = m_fScale;
            pEffect->InitPosition(m_vecPosition.x, m_fHeight + 0.1f, m_vecPosition.y);
        }

        pEffect->m_nFade = 1;
        pEffect->m_StartColor.r = 0.3f;
        pEffect->m_StartColor.g = 0.3f;
        pEffect->m_StartColor.b = 0.3f;
        pEffect->InitObject(bExpand);
        pEffect->m_dwStartTime = dwServerTime;
        pEffect->m_dwLifeTime = 200;
        pEffect->m_fStartAngle = m_fAngle;
        pEffect->m_fAngle = pEffect->m_fStartAngle;
        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion]]);
        if (m_cMount > 0 && m_pMount && pEffect->m_pSkinMesh)
        {
            pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[3]);
        }

        pEffect->m_pSkinMesh->m_dwFPS = m_pSkinMesh->m_dwFPS;
        pEffect->m_nMotionType = 0;
        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        m_dwDodgeTime = dwServerTime;
    }

    // FUN_00506f9d identifies affect 15 as the texture-93 Skill Amp plane.
    // Keep that plane just above the terrain so it follows the feet without
    // z-fighting; a character-height offset incorrectly draws it on the head.
    constexpr float kSkillAmpGroundOffset = 0.05f;
    if (m_cSKillAmp == 1)
    {
        if (m_pSkillAmp)
        {
            m_pSkillAmp->m_vecPosition = { m_vecPosition.x,
                m_fHeight + kSkillAmpGroundOffset,
                m_vecPosition.y };
        }
        else
        {
            m_pSkillAmp = new TMEffectBillBoard2(93, 0, 0.5f, 0.5f, 0.5f, 0.0f, 5000);

            if (m_pSkillAmp)
            {
                m_pSkillAmp->m_nFade = 3;
                m_pSkillAmp->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pSkillAmp->m_vecPosition = { m_vecPosition.x,
                    m_fHeight + kSkillAmpGroundOffset,
                    m_vecPosition.y };
                m_pSkillAmp->SetColor(0x88888800);
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pSkillAmp);
            }
        }
    }
    else if (m_pSkillAmp)
    {
        g_pObjectManager->DeleteObject(m_pSkillAmp);
        m_pSkillAmp = nullptr;
    }

    if (m_cShadow == 1 && g_pCurrentScene->m_pMyHuman == this)
    {
        if (m_pShadow)
        {
            m_pShadow->m_vecPosition = { m_vecPosition.x, m_fHeight + 0.5f, m_vecPosition.y };
        }
        else
        {
            m_pShadow = new TMEffectBillBoard2(94, 0, 1.2f, 1.2f, 1.2f, 0.0f, 5000);
            m_pShadow->m_nFade = 4;
            m_pShadow->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            m_pShadow->m_vecPosition = { m_vecPosition.x, m_fHeight + 0.5f, m_vecPosition.y };
            m_pShadow->SetColor(0xAAAA0000);
            g_pCurrentScene->m_pEffectContainer->AddChild(m_pShadow);
        }
    }
    else if (m_pShadow)
    {
        g_pObjectManager->DeleteObject(m_pShadow);
        m_pShadow = nullptr;
    }

    if ((m_nClass != 8 || m_cCoinArmor != 1) && m_nClass == 8 && m_pEyeFire[0])
    {
        for (int i = 0; i < 7; ++i)
        {
            if (m_pEyeFire[i])
            {
                g_pObjectManager->DeleteObject(m_pEyeFire[i]);
                m_pEyeFire[i] = nullptr;
            }
        }
    }
    if (m_cHuntersVision != 1 || m_cOverExp)
    {
        if (m_pHuntersVision)
        {
            g_pObjectManager->DeleteObject(m_pHuntersVision);
            m_pHuntersVision = 0;
        }
    }
    else if (!m_pHuntersVision)
    {
        m_pHuntersVision = new TMEffectBillBoard(438, 0, 0.4f, 0.4f, 0.4f, 0.0f, 1, 80);
        m_pHuntersVision->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        m_pHuntersVision->m_nFade = 2;

        if (m_cMount == 1)
            m_pHuntersVision->m_vecPosition = { m_vecSkinPos.x, ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
        else
            m_pHuntersVision->m_vecPosition = { m_vecPosition.x, (TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y };

        m_pHuntersVision->m_nParticleType = 10;
        m_pHuntersVision->m_fParticleH = 0.1f;
        m_pHuntersVision->m_fParticleV = 0.1f;
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pHuntersVision);
    }
    else
    {
        if (m_cMount == 1)
            m_pHuntersVision->m_vecPosition = { m_vecSkinPos.x, ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
        else
            m_pHuntersVision->m_vecPosition = { m_vecPosition.x, (TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y };
    }
    if (m_cOverExp != 1 || m_cHuntersVision)
    {
        if (m_pOverExp)
        {
            g_pObjectManager->DeleteObject(m_pOverExp);
            m_pOverExp = nullptr;
        }
    }
    else if (!m_pOverExp)
    {
        m_pOverExp = new TMEffectBillBoard(439, 0, 0.40000001f, 0.40000001f, 0.40000001f, 0.0f, 1, 80);
        m_pOverExp->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        m_pOverExp->m_nFade = 2;

        if (m_cMount == 1)
            m_pOverExp->m_vecPosition = { m_vecSkinPos.x, ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
        else
            m_pOverExp->m_vecPosition = { m_vecPosition.x, (TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y };

        m_pOverExp->m_nParticleType = 10;
        m_pOverExp->m_fParticleH = 0.1f;
        m_pOverExp->m_fParticleV = 0.1f;
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pOverExp);
    }
    else
    {
        if (m_cMount == 1)
            m_pOverExp->m_vecPosition = { m_vecSkinPos.x, ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
        else
            m_pOverExp->m_vecPosition = { m_vecPosition.x, (TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y };
    }

    if (m_cOverExp != 1 || m_cHuntersVision != 1)
    {
        if (m_pBraveOverExp)
        {
            g_pObjectManager->DeleteObject(m_pBraveOverExp);
            m_pBraveOverExp = nullptr;
        }
    }
    else if (!m_pBraveOverExp)
    {
        m_pBraveOverExp = new TMEffectBillBoard(440, 0, 0.4f, 0.4f, 0.4f, 0.0f, 1, 80);
        m_pBraveOverExp->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        m_pBraveOverExp->m_nFade = 2;

        if (m_cMount == 1)
            m_pBraveOverExp->m_vecPosition = { m_vecSkinPos.x, ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
        else
            m_pBraveOverExp->m_vecPosition = { m_vecPosition.x, (float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y };

        m_pBraveOverExp->m_nParticleType = 10;
        m_pBraveOverExp->m_fParticleH = 0.1f;
        m_pBraveOverExp->m_fParticleV = 0.1f;
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pBraveOverExp);
    }
    else
    {
        if (m_cMount == 1)
            m_pBraveOverExp->m_vecPosition = { m_vecSkinPos.x, ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
        else
            m_pBraveOverExp->m_vecPosition = { m_vecPosition.x, (float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y };
    }

    if (!g_bHideSkillBuffEffect && m_cManaControl == 1)
    {
        //int nRand = rand() % 5;
        //auto mpBill = new TMEffectBillBoard(56,
        //    1500,
        //    (float)((float)nRand * 0.02f) + 0.02f,
        //    (float)((float)nRand * 0.05f) + 0.02f,
        //    (float)((float)nRand * 0.02f) + 0.02f,
        //    0.0f,
        //    1,
        //    80);

        //float fY = ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale)
        //    + m_fHeight)
        //    - ((float)nRand * 0.05f);
        //float fZ = ((float)(rand() % 10 - 5) * 0.07f) + m_vecPosition.y;
        //mpBill->m_vecPosition = { ((float)(rand() % 10 - 5) * 0.07f) + m_vecPosition.x, fY, fZ };
        //mpBill->m_vecStartPos = mpBill->m_vecPosition;
        //mpBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        //mpBill->m_bStickGround = 0;
        //mpBill->m_nParticleType = nRand % 3;
        //mpBill->m_fParticleV = -1.2f;
        //mpBill->m_fParticleH = 0.1f;
        //mpBill->SetColor(0xFFFF3300);
        //g_pCurrentScene->m_pEffectContainer->AddChild(mpBill);

        //auto mpBill2 = new TMEffectBillBoard(0,
        //    1500,
        //    ((float)nRand * 0.1f) + 0.1f,
        //    ((float)nRand * 0.05f) + 0.80000001f,
        //    ((float)nRand * 0.1f) + 0.1f,
        //    0.000099999997f,
        //    1,
        //    80);
        //mpBill2->m_vecPosition = { ((float)(rand() % 10 - 5) * 0.01f) + m_vecPosition.x, m_fHeight, ((float)(rand() % 10 - 5) * 0.01f) + m_vecPosition.y };
        //mpBill2->m_vecStartPos = mpBill2->m_vecPosition;
        //mpBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        //mpBill2->m_bStickGround = 0;
        //mpBill2->m_nParticleType = 1;
        //mpBill2->m_fParticleV = 1.5f;
        //mpBill2->SetColor(0xFFFF0000);
        //g_pCurrentScene->m_pEffectContainer->AddChild(mpBill2);
    }

    if (!g_bHideSkillBuffEffect2 && m_cProtector == 1 && !m_pProtector)
    {
        TMVector3 vec{ m_vecPosition.x, m_fHeight + 2.5f, m_vecPosition.y };
        m_pProtector = new TMEffectSkinMesh(32, vec, vec, 3, this);
        m_pProtector->m_stLookInfo.Mesh1 = 0;
        m_pProtector->m_stLookInfo.Mesh0 = 0;
        m_pProtector->m_stLookInfo.Skin1 = 0;
        m_pProtector->m_stLookInfo.Skin0 = 0;
        m_pProtector->InitObject(0);
        m_pProtector->m_dwLifeTime = 0;
        m_pProtector->m_pSkinMesh->m_vScale.x = 0.3f;
        m_pProtector->m_pSkinMesh->m_vScale.y = 0.3f;
        m_pProtector->m_pSkinMesh->m_vScale.z = 0.3f;
        m_pProtector->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        m_pProtector->m_StartColor.r = 1.0f;
        m_pProtector->m_StartColor.g = 1.0f;
        m_pProtector->m_StartColor.b = 1.0f;
        m_pProtector->m_nFade = 1;
        m_pProtector->m_fStartAngle = m_fAngle;
        m_pProtector->m_nMotionType = 5;
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pProtector);
    }
    else if (!m_cProtector && m_pProtector)
    {
        g_pObjectManager->DeleteObject(m_pProtector);
        m_pProtector = nullptr;
    }

    if (!g_bHideSkillBuffEffect && m_cCriticalArmor == 1)
    {
        if (!m_pCriticalArmor)
        {
            m_pCriticalArmor = new TMEffectMesh(2838, 0xFF999999, m_fAngle, 0);
            if (m_pCriticalArmor)
            {
                m_pCriticalArmor->m_nTextureIndex = 413;
                m_pCriticalArmor->m_dwCycleTime = 1500;
                if (m_cMount == 1)
                    m_pCriticalArmor->m_vecPosition = {
                        m_vecSkinPos.x,
                        ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale)
                            + m_vecSkinPos.y)
                        + 1.7f,
                        m_vecSkinPos.z };
                else
                    m_pCriticalArmor->m_vecPosition = {
                        m_vecPosition.x,
                        ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale)
                            + m_fHeight)
                        + 1.3f,
                        m_vecPosition.y };

                m_pCriticalArmor->m_fScaleH = 0.0f;
                m_pCriticalArmor->m_fScaleV = 0.5f;
                m_pCriticalArmor->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pCriticalArmor->m_cShine = 1;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pCriticalArmor);
            }
        }
        else
        {
            if (m_cMount == 1)
                m_pCriticalArmor->m_vecPosition = {
                    m_vecSkinPos.x,
                    ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale)
                        + m_vecSkinPos.y)
                    + 0.7f,
                    m_vecSkinPos.z };
            else
                m_pCriticalArmor->m_vecPosition = {
                    m_vecPosition.x,
                    ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale)
                        + m_fHeight)
                    + 0.3f,
                    m_vecPosition.y };
            m_pCriticalArmor->m_fAngle = m_fAngle;
        }
    }
    else if (!m_cCriticalArmor && m_pCriticalArmor)
    {
        g_pObjectManager->DeleteObject(m_pCriticalArmor);
        m_pCriticalArmor = nullptr;
    }

    if (m_cSoul == 1 && m_cDie != 1 && !m_cShadow)
    {
        for (int i = 0; i < 2; i++)
        {
            if (!m_pSoul[i])
            {
                m_pSoul[i] = new TMEffectMeshRotate({ m_vecPosition.x, (float)(m_fHeight + 0.80000001f) + (float)((float)i * 0.2f), m_vecPosition.y }, 1, this, 1, 0);

                if (m_cMount == 1)
                    m_pSoul[i]->m_vecPosition = { m_vecSkinPos.x, ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
                else
                    m_pSoul[i]->m_vecPosition = { m_vecPosition.x, (TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y };

                m_pSoul[i]->m_dwStartTime = (unsigned int)((float)dwServerTime - (((float)i * 2000.0f) / 2.0f));
                m_pSoul[i]->m_nMeshIndex = 2839;
                m_pSoul[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pSoul[i]->m_fScale = 1.4f;
                m_pSoul[i]->SetColor(0xFF999999);
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pSoul[i]);
            }
        }
    }
    else
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_pSoul[i])
            {
                g_pObjectManager->DeleteObject(m_pSoul[i]);
                m_pSoul[i] = nullptr;
            }
        }
    }

    if (m_sFamiliar == 753 && !m_pFamiliar)
    {
        TMVector3 vec{ m_vecPosition.x, (float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y + 0.3f) * m_fScale, m_vecPosition.y };
        if (!m_pFamiliar)
        {
            m_pFamiliar = new TMEffectSkinMesh(20, vec, vec, 3, this);
        }
        if (m_pFamiliar)
        {
            m_pFamiliar->m_stLookInfo.Mesh1 = 2;
            m_pFamiliar->m_stLookInfo.Mesh0 = 2;
            m_pFamiliar->m_stLookInfo.Skin1 = 0;
            m_pFamiliar->m_stLookInfo.Skin0 = 0;
            m_pFamiliar->InitObject(0);
            m_pFamiliar->m_dwLifeTime = 0;
            m_pFamiliar->m_pSkinMesh->m_vScale.x = 0.090000004f;
            m_pFamiliar->m_pSkinMesh->m_vScale.y = 0.090000004f;
            m_pFamiliar->m_pSkinMesh->m_vScale.z = 0.090000004f;
            m_pFamiliar->m_efAlphaType = EEFFECT_ALPHATYPE::EF_DEFAULT;
            m_pFamiliar->m_StartColor.r = 1.0f;
            m_pFamiliar->m_StartColor.g = 1.0f;
            m_pFamiliar->m_StartColor.b = 1.0f;
            m_pFamiliar->m_nFade = 1;
            m_pFamiliar->m_fStartAngle = m_fAngle;
            m_pFamiliar->m_nMotionType = 7;
            g_pCurrentScene->m_pEffectContainer->AddChild(m_pFamiliar);
        }
    }
    else if (m_sFamiliar == 769 && !m_pFamiliar)
    {
        TMVector3 vec{ m_vecPosition.x, (float)((TMHuman::m_vecPickSize[m_nSkinMeshType].y + 0.3f) * m_fScale) + m_fHeight, m_vecPosition.y };
        if (!m_pFamiliar)
        {
            m_pFamiliar = new TMEffectSkinMesh(32, vec, vec, 4, this);
        }
        if (m_pFamiliar)
        {
            m_pFamiliar->m_stLookInfo.Mesh1 = 1;
            m_pFamiliar->m_stLookInfo.Mesh0 = 1;
            m_pFamiliar->m_stLookInfo.Skin1 = 0;
            m_pFamiliar->m_stLookInfo.Skin0 = 0;
            m_pFamiliar->InitObject(0);
            m_pFamiliar->m_dwLifeTime = 0;
            m_pFamiliar->m_pSkinMesh->m_vScale.x = 1.2f;
            m_pFamiliar->m_pSkinMesh->m_vScale.y = 1.2f;
            m_pFamiliar->m_pSkinMesh->m_vScale.z = 1.2f;
            m_pFamiliar->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            m_pFamiliar->m_StartColor.r = 1.0f;
            m_pFamiliar->m_StartColor.g = 1.0f;
            m_pFamiliar->m_StartColor.b = 1.0f;
            m_pFamiliar->m_nFade = 1;
            m_pFamiliar->m_fStartAngle = m_fAngle;
            m_pFamiliar->m_nMotionType = 5;
            g_pCurrentScene->m_pEffectContainer->AddChild(m_pFamiliar);
        }
    }
    else if (m_sFamiliar == 1726 && !m_pFamiliar)
    {
        TMVector3 vec{ m_vecPosition.x, (float)((TMHuman::m_vecPickSize[m_nSkinMeshType].y + 0.3f) * m_fScale) + m_fHeight, m_vecPosition.y };
        if (!m_pFamiliar)
        {
            m_pFamiliar = new TMEffectSkinMesh(32, vec, vec, 5, this);
        }
        if (m_pFamiliar)
        {
            m_pFamiliar->m_stLookInfo.Mesh1 = 2;
            m_pFamiliar->m_stLookInfo.Mesh0 = 2;
            m_pFamiliar->m_stLookInfo.Skin1 = 0;
            m_pFamiliar->m_stLookInfo.Skin0 = 0;
            m_pFamiliar->InitObject(0);
            m_pFamiliar->m_dwLifeTime = 0;
            m_pFamiliar->m_pSkinMesh->m_vScale.x = 1.0f;
            m_pFamiliar->m_pSkinMesh->m_vScale.y = 1.0f;
            m_pFamiliar->m_pSkinMesh->m_vScale.z = 1.0f;
            m_pFamiliar->m_efAlphaType = EEFFECT_ALPHATYPE::EF_DEFAULT;
            m_pFamiliar->m_StartColor.r = 1.0f;
            m_pFamiliar->m_StartColor.g = 1.0f;
            m_pFamiliar->m_StartColor.b = 1.0f;
            m_pFamiliar->m_nFade = 1;
            m_pFamiliar->m_fStartAngle = m_fAngle;
            m_pFamiliar->m_nMotionType = 5;
            g_pCurrentScene->m_pEffectContainer->AddChild(m_pFamiliar);
        }
    }
    else if ((m_sFamiliar >= 3900 && m_sFamiliar <= 3908 || m_sFamiliar >= 3911 && m_sFamiliar <= 3916 || m_sFamiliar == 4060) && !m_pFamiliar)
    {
        TMVector3 vec{ m_vecPosition.x, (float)((TMHuman::m_vecPickSize[m_nSkinMeshType].y + 0.3f) * m_fScale) + m_fHeight, m_vecPosition.y };
        if (!m_pFamiliar)
        {
            m_pFamiliar = new TMEffectSkinMesh(32, vec, vec, 6, this);
        }
        if (m_pFamiliar)
        {
            m_pFamiliar->m_stLookInfo.Mesh1 = 3;
            m_pFamiliar->m_stLookInfo.Mesh0 = 3;
            if (m_sFamiliar == 3900 || m_sFamiliar == 3903 || m_sFamiliar == 3906 || m_sFamiliar >= 3911 && m_sFamiliar <= 3913)
            {
                m_pFamiliar->m_stLookInfo.Skin1 = 0;
                m_pFamiliar->m_stLookInfo.Skin0 = 0;
            }
            else if (m_sFamiliar == 3901 || m_sFamiliar == 3904 || m_sFamiliar == 3907)
            {
                m_pFamiliar->m_stLookInfo.Skin1 = 1;
                m_pFamiliar->m_stLookInfo.Skin0 = 1;
            }
            else if (m_sFamiliar == 3902 || m_sFamiliar == 3905 || m_sFamiliar == 3908)
            {
                m_pFamiliar->m_stLookInfo.Skin1 = 2;
                m_pFamiliar->m_stLookInfo.Skin0 = 2;
            }
            else if (m_sFamiliar == 3914)
            {
                m_pFamiliar->m_stLookInfo.Skin1 = 3;
                m_pFamiliar->m_stLookInfo.Skin0 = 3;
            }
            else if (m_sFamiliar == 3915 || m_sFamiliar == 3916)
            {
                m_pFamiliar->m_stLookInfo.Skin1 = 4;
                m_pFamiliar->m_stLookInfo.Skin0 = 4;
            }
            else if (m_sFamiliar == 4060)
            {
                m_pFamiliar->m_stLookInfo.Skin1 = 1;
                m_pFamiliar->m_stLookInfo.Skin0 = 1;
            }
            m_pFamiliar->InitObject(0);
            m_pFamiliar->m_dwLifeTime = 0;
            m_pFamiliar->m_pSkinMesh->m_vScale.x = 1.0f;
            m_pFamiliar->m_pSkinMesh->m_vScale.y = 1.0f;
            m_pFamiliar->m_pSkinMesh->m_vScale.z = 1.0f;
            m_pFamiliar->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            m_pFamiliar->m_StartColor.r = 1.0f;
            m_pFamiliar->m_StartColor.g = 1.0f;
            m_pFamiliar->m_StartColor.b = 1.0f;
            m_pFamiliar->m_nFade = 1;
            m_pFamiliar->m_fStartAngle = m_fAngle;
            m_pFamiliar->m_nMotionType = 5;
            g_pCurrentScene->m_pEffectContainer->AddChild(m_pFamiliar);
        }
    }
    else if (m_sFamiliar != 753 && m_sFamiliar != 769 && m_sFamiliar != 1726 &&
        (m_sFamiliar < 3900 || m_sFamiliar > 3908) && (m_sFamiliar < 3911 || m_sFamiliar > 3916) && m_sFamiliar != 4060 && m_pFamiliar)
    {
        g_pObjectManager->DeleteObject(m_pFamiliar);
        m_pFamiliar = nullptr;
    }

    if (dwServerTime - m_dwPunishedTime > 1500 && m_cPunish == 1)
        m_cPunish = 0;
    if (m_cPunish == 1 && dwServerTime - m_dwLastDummyTime > 300)
    {
        bool bExpand = false;
        if (m_nClass == 4 || m_nClass == 8)
            bExpand = true;

        auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 0, nullptr);
        if (m_cMount > 0 && m_pMount)
        {
            pEffect->m_nSkinMeshType = m_nMountSkinMeshType;
            memcpy(&pEffect->m_stLookInfo, &m_stMountLook, sizeof(pEffect->m_stLookInfo));
            pEffect->m_nSkinMeshType2 = m_nSkinMeshType;
            memcpy(&pEffect->m_stLookInfo2, &m_stLookInfo, sizeof(pEffect->m_stLookInfo));

        }
        else
        {
            memcpy(&pEffect->m_stLookInfo, &m_stLookInfo, sizeof(pEffect->m_stLookInfo));
        }

        pEffect->m_nFade = 1;
        pEffect->m_StartColor.r = 0.5f;
        pEffect->m_StartColor.g = 0.5f;
        pEffect->m_StartColor.b = 0.5f;
        pEffect->InitObject(bExpand);
        pEffect->m_dwStartTime = dwServerTime;
        pEffect->m_dwLifeTime = 700;
        pEffect->InitPosition(m_vecPosition.x, m_fHeight + 0.1f, m_vecPosition.y);
        pEffect->m_fStartAngle = m_fAngle;
        pEffect->m_fAngle = pEffect->m_fStartAngle;
        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;

        if (m_cMount > 0 && m_pMount)
        {
            if (pEffect->m_pSkinMesh)
                pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nMountSkinMeshType].dwAniTable[(int)m_eMotion]]);
            if (pEffect->m_pSkinMesh2)
                pEffect->m_pSkinMesh2->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion + 24]]);
        }
        else
        {
            if (pEffect->m_pSkinMesh)
                pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion]]);
        }
        if (pEffect->m_pSkinMesh)
            pEffect->m_pSkinMesh->m_dwFPS = m_pSkinMesh->m_dwFPS;
        pEffect->m_nMotionType = 0;

        GetSoundAndPlayIfNot(160, 0, 0);

        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        m_dwLastDummyTime = dwServerTime;
    }

    if (m_stEffectEvent.sEffectIndex == 102 && !m_bStartAvatarEffect)
    {
        int nCls = m_sHeadIndex % 10;
        if (nCls == 6)
            FrameMoveEffect_AvatarTrans();
        else if (nCls == 7)
            FrameMoveEffect_AvatarFoema();
        else if (nCls == 8 || m_sHeadIndex >= 22 && m_sHeadIndex <= 25 || m_sHeadIndex == 32)
            FrameMoveEffect_AvatarBMaster();
        else if (nCls == 9)
            FrameMoveEffect_AvatarHunter();

        m_bStartAvatarEffect = 1;
        m_dwAvatarEffTime = g_pTimerManager->GetServerTime();
    }

    if (m_bStartAvatarEffect == 1 && m_cAvatar == 1 && dwServerTime > m_dwAvatarEffTime + 1250)
    {
        TMVector3 vPos{ m_vecPosition.x, m_fHeight + 2.0f, m_vecPosition.y };
        int nCls = m_sHeadIndex / 10;
        for (int i = 0; i < 3; ++i)
        {
            auto pEffect1 = new TMEffectBillBoard(
                56,
                700,
                ((float)i * 1.5f) + 3.0f,
                ((float)i * 1.5f) + 3.0f,
                ((float)i * 0.5f) + 3.0f,
                0.001f,
                1,
                80);

            pEffect1->SetColor(0xFFFFFFFF);
            pEffect1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect1->m_nFade = 1;
            pEffect1->m_vecPosition = vPos;
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect1);
        }
        m_bStartAvatarEffect = 0;
    }
}

void TMHuman::FrameMoveEffect_AvatarTrans()
{
    if (m_c8thSkill == 1)
    {
        for (int i = 0; i < 4; ++i)
        {
            TMVector3 vecPos{ m_vecPosition.x, ((float)i * 0.5f) + m_fHeight, m_vecPosition.y };

            auto pAvaTrans1 = new TMArrow(vecPos, vecPos, 0, 10002, 0, 0, 0);
            if (!pAvaTrans1)
                break;

            pAvaTrans1->m_dwStartTime += 150 * i;

            if (g_pCurrentScene != nullptr)
            {
                if (pAvaTrans1)
                    g_pCurrentScene->m_pEffectContainer->AddChild(pAvaTrans1);
            }
        }
    }
    else if (m_c8thSkill == 2)
    {
        for (int i = 0; i < 15; ++i)
        {
            auto pCrArmor = new TMEffectMesh(2838, 0x33555555, m_fAngle, 3);
            if (!pCrArmor)
                break;

            pCrArmor->m_nTextureIndex = 413;
            pCrArmor->m_dwLifeTime = 30 * i + 1000;
            pCrArmor->m_dwCycleTime = 1000 - 30 * i;

            if (m_cMount == 1)
                pCrArmor->m_vecPosition = { m_vecSkinPos.x, (((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) / 2.0f) + m_vecSkinPos.y) - 0.30000001f, m_vecSkinPos.z };
            else
                pCrArmor->m_vecPosition = { m_vecPosition.x, (((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) / 2.0f) + m_fHeight) + 0.30000001f, m_vecPosition.y };

            pCrArmor->m_fScaleH = ((float)i * 0.30000001f) + 1.0f;
            pCrArmor->m_fScaleV = ((float)i * 0.30000001f) + 1.0f;
            pCrArmor->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pCrArmor->m_cShine = 1;
            g_pCurrentScene->m_pEffectContainer->AddChild(pCrArmor);
        }
    }
    else if (m_c8thSkill == 3)
    {
        for (int i = 0; i < 3; ++i)
        {
            auto pFreeze = new TMSkillFreezeBlade({ m_stEffectEvent.vecTo.x, m_stEffectEvent.vecTo.y, m_stEffectEvent.vecTo.z }, 1, 0, 1);
            if (!pFreeze)
                return;

            g_pCurrentScene->m_pEffectContainer->AddChild(pFreeze);
            pFreeze->m_dwLifeTime = 500 * i + 1000;
        }

        auto pFreeze = new TMSkillFreezeBlade({ m_stEffectEvent.vecTo.x, m_stEffectEvent.vecTo.y, m_stEffectEvent.vecTo.z }, 1, 0, 1);
        if (pFreeze != nullptr)
        {
            g_pCurrentScene->m_pEffectContainer->AddChild(pFreeze);
            pFreeze->m_dwLifeTime = 1500;
        }
    }
}

void TMHuman::FrameMoveEffect_AvatarFoema()
{
    if (m_c8thSkill == 1)
    {
        auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, TMVector3{}, TMVector3{}, 0, 0);
        if (pEffect != nullptr)
        {
            bool bExpand{ false };

            if (m_nClass == 4 || m_nClass == 8)
                bExpand = true;

            memcpy(&pEffect->m_stLookInfo, &m_stLookInfo, sizeof(pEffect->m_stLookInfo));

            ECHAR_MOTION eMotion{ ECHAR_MOTION::ECMOTION_LEVELUP };

            pEffect->m_StartColor.r = 0.30000001f;
            pEffect->m_StartColor.g = 0.30000001f;
            pEffect->m_StartColor.b = 0.30000001f;
            pEffect->InitObject(bExpand);
            pEffect->m_nFade = 1;
            pEffect->m_dwLifeTime = 1400;

            float fHeight = m_fHeight;
            if (m_cMount > 0 && m_pMount)
                fHeight += 0.5f;

            pEffect->InitPosition(m_vecPosition.x, fHeight, m_vecPosition.y);
            pEffect->m_pSkinMesh->m_vScale.x = 2.0f;
            pEffect->m_pSkinMesh->m_vScale.y = 2.0f;
            pEffect->m_pSkinMesh->m_vScale.z = 2.0f;
            pEffect->m_fAngle = m_fAngle;
            pEffect->m_pSkinMesh->m_dwFPS = m_pSkinMesh->m_dwFPS;
            pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[static_cast<int>(eMotion)]);
            pEffect->m_fStartAngle = 1.0f;
            pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);

            auto mpBill = new TMEffectBillBoard(0, 1300, 7.0f, 7.0f, 7.0f, 0.000099999997f, 1, 80);
            if (mpBill != nullptr)
            {
                mpBill->m_vecStartPos = mpBill->m_vecPosition = TMVector3{ m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y };
                mpBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                mpBill->m_bStickGround = 1;
                mpBill->m_nParticleType = 2;
                mpBill->m_fParticleV = 0.050000001f;
                mpBill->m_fParticleH = 0.050000001f;
                mpBill->SetColor(0xFFFFFFFF);
                g_pCurrentScene->m_pEffectContainer->AddChild(mpBill);
            }
        }
    }
    else if (m_c8thSkill == 2)
    {
        for (int i = 0; i < 3; ++i)
        {
            TMVector3 vecDest{ m_stEffectEvent.vecTo.x, m_stEffectEvent.vecTo.y + 2.5f, m_stEffectEvent.vecTo.z };

            auto pEffect1 = new TMEffectBillBoard(33, 1000, ((float)i * 1.5f) + 2.0f, ((float)i * 1.5f) + 2.0f, ((float)i * 1.5f) + 2.0f, 0.0f, 9, 110);

            if (pEffect1 == nullptr)
                break;

            pEffect1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect1->m_nFade = 0;
            pEffect1->m_vecPosition = vecDest;
            pEffect1->m_vecPosition.y -= 0.5f;
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect1);

            auto pEffect2 = new TMEffectBillBoard(0, 1500, ((float)i * 1.0f) + 4.0f, ((float)i * 2.0f) + 4.0f,  ((float)i * 1.0f) + 4.0f, 0.000099999997f, 1, 80);

            if (pEffect2 == nullptr)
                break;

            pEffect2->m_vecStartPos = pEffect2->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight + 2.0f, m_vecPosition.y);
            pEffect2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect2->m_bStickGround = 1;
            pEffect2->m_nParticleType = 1;
            pEffect2->m_fParticleV = 0.079999998f;
            pEffect2->m_fParticleH = 0.079999998f;
            pEffect2->SetColor(0xFFFF5555);
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect2);

            auto pEffect3 = new TMEffectBillBoard2(8, 1500, 0.00050000002f, 0.00050000002f, 0.00050000002f, 0.0049999999f, 0);

            if (pEffect3 == nullptr)
                break;

            pEffect3->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect3->m_vecPosition = vecDest;
            pEffect3->m_vecPosition.y -= (2.0f - ((float)i * 0.1f));
            pEffect3->SetColor(0xFFFF5555);
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect3);
        }
    }
    else if (m_c8thSkill == 3)
    {
        for (int j = 0; j < 8; ++j)
        {
            auto pEffect1 = new TMEffectBillBoard(56, 100 * j + 1000, ((float)j * 0.5f) + 1.5f, ((float)j * 0.40000001f) + 1.5f, ((float)j * 0.5f) + 1.5f, 0.0f, 1,  80);

            if (pEffect1 == nullptr)
                break;

            pEffect1->m_vecStartPos = pEffect1->m_vecPosition = TMVector3{ m_vecPosition.x, (m_fHeight + 5.0f) - ((float)j * 0.89999998f), m_vecPosition.y };
            pEffect1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect1->m_bStickGround = 1;
            pEffect1->m_nParticleType = 9;
            pEffect1->m_fParticleV = ((float)j * 2.0f) + 1.0f;
            pEffect1->m_fParticleH = ((float)j * 2.0f) + 1.0f;
            pEffect1->SetColor(0xFFFF3300);
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect1);

            auto pEffect2 = new TMEffectBillBoard(0, 1000, 5.0f, 5.0f, 5.0f, 0.000099999997f, 1, 80);

            if (pEffect2 == nullptr)
                break;

            pEffect2->m_vecStartPos = pEffect2->m_vecPosition = TMVector3{ m_vecPosition.x, m_fHeight + 2.0f, m_vecPosition.y };
            pEffect2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect2->m_bStickGround = 0;
            pEffect2->m_nParticleType = 1;
            pEffect2->m_fParticleV = 5.0f;
            pEffect2->m_fParticleH = 5.0f;
            pEffect2->SetColor(0xFFFF0000);
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect2);
        }
    }
}

void TMHuman::FrameMoveEffect_AvatarBMaster()
{
    if (m_c8thSkill == 1)
    {
        for (int i = 0; i < 10; ++i)
        {
            auto pJudgement = new TMSkillJudgement({ m_vecPosition.x, (m_fHeight - 2.0f) + ((float)i * 0.5f), m_vecPosition.y }, 7, (float)i * 0.1f);
            if (!pJudgement)
                break;
            g_pCurrentScene->m_pEffectContainer->AddChild(pJudgement);
        }
    }
    else if (m_c8thSkill == 2)
    {
        for (int i = 0; i < 5; ++i)
        {
            auto pJudgement = new TMSkillJudgement({ m_vecPosition.x, (m_fHeight + 0.5f) - ((float)i * 0.1f), m_vecPosition.y }, 8, (float)i * 0.40000001f);
            if (!pJudgement)
                break;
            g_pCurrentScene->m_pEffectContainer->AddChild(pJudgement);

            auto pEffect = new TMEffectBillBoard(56, 2300, ((float)i * 1.7f) + 0.30000001f, ((float)i * 0.5f) + 0.30000001f, ((float)i * 1.7f) + 0.30000001f, 0.0f, 1, 80);
            if (!pEffect)
                break;

            pEffect->m_vecStartPos = pEffect->m_vecPosition = { m_vecPosition.x, (m_fHeight - 0.5f) + ((float)i * 0.69999999f), m_vecPosition.y };
            pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect->m_bStickGround = 1;
            pEffect->m_nParticleType = 9;
            pEffect->m_fParticleV = ((float)i * 1.0f) + 1.0f;
            pEffect->m_fParticleH = ((float)i * 1.0f) + 1.0f;
            pEffect->SetColor(0x33555555);
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        }
    }
    else if (m_c8thSkill == 3)
    {
        for (int i = 0; i < 6; ++i)
        {
            auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, TMVector3{}, TMVector3{}, 0, 0);
            if (!pEffect)
                break;

            bool bExpand{ false };

            if (m_nClass == 4 || m_nClass == 8)
                bExpand = true;

            memcpy(&pEffect->m_stLookInfo, &m_stLookInfo, sizeof(pEffect->m_stLookInfo));

            ECHAR_MOTION eMotion{ ECHAR_MOTION::ECMOTION_ATTACK01 };

            pEffect->m_StartColor.r = ((float)i * 0.80000001f) + 0.30000001f;
            pEffect->m_StartColor.g = ((float)i * 0.80000001f) + 0.30000001f;
            pEffect->m_StartColor.b = ((float)i * 0.80000001f) + 0.30000001f;
            pEffect->InitObject(bExpand);
            pEffect->m_nFade = 1;
            pEffect->m_dwLifeTime = 300 * i + 100;

            float fHeight = m_fHeight;
            if (m_cMount > 0 && m_pMount)
                fHeight += 0.5f;

            pEffect->InitPosition(m_vecPosition.x, fHeight, m_vecPosition.y);
            pEffect->m_pSkinMesh->m_vScale.x = 1.9f;
            pEffect->m_pSkinMesh->m_vScale.y = 1.9f;
            pEffect->m_pSkinMesh->m_vScale.z = 1.9f;
            pEffect->m_nMotionType = 12;
            pEffect->m_fAngle = ((float)i * 1.5f) + m_fAngle;
            pEffect->m_pSkinMesh->m_dwFPS = m_pSkinMesh->m_dwFPS - 4 * i;
            pEffect->m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[static_cast<int>(eMotion)]);
            pEffect->m_fStartAngle = 1.0f;
            pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        }
    }
}

void TMHuman::FrameMoveEffect_AvatarHunter()
{
    if (m_c8thSkill == 1)
    {
        for (int i = 0; i < 2; ++i)
        {
            auto pSoul = new TMEffectMeshRotate({ m_vecPosition.x, (m_fHeight - 3.0f) - ((float)i * 1.5f), m_vecPosition.y }, 2, this, 1, 0);
            if (!pSoul)
                break;

            if (m_cMount == 1)
                pSoul->m_vecPosition = { m_vecSkinPos.x, m_fHeight - 3.0f, m_vecSkinPos.z };
            else
                pSoul->m_vecPosition = { m_vecPosition.x, m_fHeight - 3.0f, m_vecPosition.y };

            pSoul->m_dwLifeTime = 1000 - 100 * i;
            pSoul->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pSoul->m_fScale = 6.5f;
            pSoul->SetColor(0xFF5555AA);
            g_pCurrentScene->m_pEffectContainer->AddChild(pSoul);
        }
    }
    else if (m_c8thSkill == 2)
    {
        for (int i = 0; i < 3; ++i)
        {
            auto pSpChange = new TMSkillSpChange({ m_vecPosition.x, ((float)i * 0.69999999f) + m_fHeight, m_vecPosition.y }, 1, this);
            if (!pSpChange)
                break;

            pSpChange->m_dwLifeTime = 300 * i + 700;
            g_pCurrentScene->m_pEffectContainer->AddChild(pSpChange);
        }
    }
    else if (m_c8thSkill == 3)
    {
        for (int i = 0; i < 7; ++i)
        {
            TMVector3 vecPos{ m_vecPosition.x, (m_fHeight + 0.5f) + ((float)i * 0.30000001f), m_vecPosition.y };

            auto pEffect = new TMArrow(vecPos, vecPos, m_stEffectEvent.sEffectLevel, 10003, 0, 0, 0);
            if (!pEffect)
                break;

            pEffect->m_dwStartTime += 100 * i;
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        }
    }
}

void TMHuman::SetHandEffect(int nHandEffect)
{
    if (m_dwDelayDel)
        return;

    m_nHandEffect = nHandEffect;
    if (m_pSkinMesh)
    {
        if (m_bSwordShadow[0] == 1 && m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = m_nHandEffect;
        else if (m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = 0;
        if (m_bSwordShadow[1] == 1 && m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = m_nHandEffect;
        else if (m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = 0;
    }
}

int TMHuman::StartKhepraDieEffect()
{
    if (!g_pCurrentScene)
        return 0;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (!pScene->m_pKhepraPortalEff1)
        return 0;

    float PtX = pScene->m_pKhepraPortalEff1->m_vecPosition.x;
    float PtY = -4.73f;
    float PtZ = pScene->m_pKhepraPortalEff1->m_vecPosition.z;
    float MyX = m_vecPosition.x;
    float MyY = m_fHeight + 1.5f;
    float MyZ = m_vecPosition.y;
    pScene->m_pKhepraPortalEff1->m_nAnimationType = 2;

    auto pEffect1 = new TMEffectBillBoard(423, 0, 4.0f, 4.0f, 2.7f, 0.0f, 1, 80);
    pEffect1->m_nFade = 1;
    pEffect1->SetColor(0x88FFFFFF);
    pEffect1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
    pEffect1->m_vecPosition = { PtX - 0.8f, PtY, PtZ - 0.3f };
    pEffect1->m_nAnimationType = 3;
    pEffect1->SetLifeTime(3800);
    pScene->m_pEffectContainer->AddChild(pEffect1);

    auto pEffect2 = new TMEffectBillBoard(423, 0, 4.0f, 4.0f, 2.7f, 0.0f, 1, 80);
    pEffect2->m_nFade = 1;
    pEffect2->SetColor(0x88FFFFFF);
    pEffect2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
    pEffect2->m_vecPosition = { PtX + 0.8f, PtY, PtZ + 0.3f };
    pEffect2->m_nAnimationType = 3;
    pEffect2->SetLifeTime(3800);
    pScene->m_pEffectContainer->AddChild(pEffect2);

    auto pEffect3 = new TMEffectBillBoard(423, 0, 4.0f, 4.0f, 2.7f, 0.0f, 1, 80);
    pEffect3->m_nFade = 1;
    pEffect3->SetColor(0x88FFFFFF);
    pEffect3->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
    pEffect3->m_vecPosition = { PtX, PtY + 0.8f, PtZ };
    pEffect3->m_nAnimationType = 3;
    pEffect3->SetLifeTime(3800);
    pScene->m_pEffectContainer->AddChild(pEffect3);

    auto pThunder1 = new TMSkillThunderBolt({ MyX, MyY, MyZ }, 5);
    pScene->m_pEffectContainer->AddChild(pThunder1);

    auto pDust1 = new TMEffectDust({ MyX, MyY, MyZ }, 50.0f, 0);;
    pScene->m_pEffectContainer->AddChild(pDust1);

    auto pDust2 = new TMEffectDust({ MyX - 0.3f, MyY, MyZ - 0.3f }, 50.0f, 0);
    pScene->m_pEffectContainer->AddChild(pDust2);

    auto pDust3 = new TMEffectDust({ MyX + 0.3f, MyY, MyZ + 0.3f }, 50.0f, 0);
    pScene->m_pEffectContainer->AddChild(pDust3);

    if ((int)pScene->m_pMyHuman->m_vecPosition.x >> 7 == 18 && (int)pScene->m_pMyHuman->m_vecPosition.y >> 7 == 30)
    {
        auto pGround = pScene->m_pGround;
        if (pGround)
        {
            pGround->m_dwEffStart = g_pTimerManager->GetServerTime() - 100;
            pGround->m_vecEffset = { MyX, MyZ };
        }
    }

    auto pJudgement = new TMSkillJudgement({ MyX, MyY - 2.0f, MyZ }, 5, 0.1f);
    pScene->m_pEffectContainer->AddChild(pJudgement);

    pScene->m_dwKhepraDieTime = g_pTimerManager->GetServerTime();
    pScene->m_nKhepraDieFlag = 1;
    return 1;
}

void TMHuman::RenderEffect_RudolphCostume(unsigned int dwServerTime)
{
    if ((dwServerTime - m_dwGolemDustTime) > 800)
    {
        auto pEffect = new TMEffectBillBoard(56, 200, 0.1f, 0.1f, 0.1f, 0.0f, 1, 80);

        if (pEffect != nullptr)
        {
            pEffect->m_nFade = 0;
            pEffect->SetColor(0xFFFF5500);
            pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect->m_vecPosition = TMVector3{ m_vecTempPos[10].x, m_vecTempPos[10].y, m_vecTempPos[10].z };
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        }

        m_dwGolemDustTime = dwServerTime;
    }
}

void TMHuman::RenderEffect_Khepra(unsigned int dwServerTime)
{
    if (m_pCancelation && !m_cCancel)
    {
        m_cCancel = 1;
        m_pCancelation->m_dwVisibleTime = 3000;
        m_pCancelation->m_fVectorH = (float)-(rand() % 50) / 100.0f;
        m_pCancelation->SetColor(0xFFFFFFFF);
    }

    int nRand = rand() % 5;

    auto pEffect = new TMEffectBillBoard(
        0,
        1500,
        ((float)nRand * 0.40000001f) + 0.2f,
        ((float)nRand * 0.69999999f) + 0.89999998f,
        ((float)nRand * 0.5f) + 0.5f,
        0.000099999997f,
        1,
        80);

    if (pEffect != nullptr)
    {
        pEffect->m_vecPosition = TMVector3{ ((float)(rand() % 40 - 5) * 0.02f) + m_vecTempPos[0].x, m_vecTempPos[0].y, ((float)(rand() % 10 - 5) * 0.02f) + m_vecTempPos[0].z };
        pEffect->m_vecStartPos = pEffect->m_vecPosition;
        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pEffect->m_bStickGround = 0;
        pEffect->m_nParticleType = 1;
        pEffect->m_fParticleV = -3.0f;
        pEffect->SetColor(0xFFFF7777);
        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
    }
}

void TMHuman::RenderEffect_LegendBerielKeeper(unsigned int dwServerTime)
{
    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        static_cast<TMFieldScene*>(g_pCurrentScene)->m_bShowBoss = 1;

    if ((dwServerTime - m_dwGolemDustTime) > 100)
    {
        static_cast<TMFieldScene*>(g_pCurrentScene)->m_nWTime = 12;
        g_nWeather = 0;
        RenderDevice::m_bDungeon = 1;

        auto pEffect1 = new TMEffectBillBoard(56, 500, 0.1f, 0.1f, 0.1f, 0.0f, 1, 80);
        if (pEffect1 != nullptr)
        {
            pEffect1->m_nFade = 0;
            pEffect1->SetColor(0xFF0088FF);
            pEffect1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;

            pEffect1->m_vecPosition = TMVector3{ m_vecTempPos[8].x, m_vecTempPos[8].y, m_vecTempPos[8].z };
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect1);
        }

        auto pEffect2 = new TMEffectBillBoard(56, 500, 0.1f, 0.1f, 0.1f, 0.0f, 1, 80);
        if (pEffect2 != nullptr)
        {
            pEffect2->m_nFade = 0;
            pEffect2->SetColor(0xFF0088FF);
            pEffect2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;

            pEffect2->m_vecPosition = TMVector3{ m_vecTempPos[9].x, m_vecTempPos[9].y, m_vecTempPos[9].z };
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect2);
        }
        m_dwGolemDustTime = dwServerTime;
    }

    int nRand = rand() % 20;
    float fSize = 2.0f;

    auto mpBill = new TMEffectBillBoard(
        0,
        1500,
        ((float)nRand * 0.0099999998f) + (fSize * 0.0099999998f),
        ((float)nRand * 0.1f) + (fSize * 0.0099999998f),
        ((float)nRand * 0.0099999998f) + (fSize * 0.029999999f),
        0.000099999997f,
        1,
        80);

    if (mpBill != nullptr)
    {
        mpBill->m_vecPosition = TMVector3{ ((float)(rand() % 6 - 3) * 0.02f) + m_vecTempPos[10].x, m_vecTempPos[10].y, ((float)(rand() % 40 - 20) * 0.02f) + m_vecTempPos[10].z };
        mpBill->m_vecStartPos = mpBill->m_vecPosition;
        mpBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        mpBill->m_bStickGround = 0;
        mpBill->m_nParticleType = 1;
        mpBill->m_fParticleV = -0.5f;
        mpBill->SetColor(0xFFAAAAFF);
        g_pCurrentScene->m_pEffectContainer->AddChild(mpBill);
    }
}

void TMHuman::RenderEffect_LegendBeriel(unsigned int dwServerTime)
{
    int nRand = rand() % 20;
    float fSize = 0.40000001f;

    auto mpBill1 = new TMEffectBillBoard(
        0,
        500,
        fSize,
        ((float)nRand * 0.1f) + (fSize * 0.0099999998f),
        ((float)nRand * 0.0099999998f) + (fSize * 0.0f),
        0.000099999997f,
        1,
        80);

    if (mpBill1 != nullptr)
    {
        mpBill1->m_vecPosition = TMVector3{ ((float)(rand() % 6 - 3) * 0.02f) + m_vecTempPos[8].x, m_vecTempPos[8].y + 0.60000002f, ((float)(rand() % 40 - 20) * 0.02f) + m_vecTempPos[8].z };
        mpBill1->m_vecStartPos = mpBill1->m_vecPosition;
        mpBill1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        mpBill1->m_bStickGround = 0;
        mpBill1->m_nParticleType = 1;
        mpBill1->m_fParticleV = 0.89999998f;
        mpBill1->SetColor(0xFFAAAACC);
        g_pCurrentScene->m_pEffectContainer->AddChild(mpBill1);
    }

    nRand = rand() % 20;
    fSize = 0.30000001f;

    auto mpBill2 = new TMEffectBillBoard(
        0,
        500,
        fSize,
        ((float)nRand * 0.1f) + (fSize * 0.0099999998f),
        ((float)nRand * 0.0099999998f) + (fSize * 0.30000001f),
        0.000099999997f,
        1,
        80);

    if (mpBill2 != nullptr)
    {
        mpBill2->m_vecPosition = TMVector3{ ((float)(rand() % 6 - 3) * 0.02f) + m_vecTempPos[8].x, m_vecTempPos[8].y + 0.60000002f, ((float)(rand() % 40 - 20) * 0.02f) + m_vecTempPos[8].z };
        mpBill2->m_vecStartPos = mpBill2->m_vecPosition;
        mpBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        mpBill2->m_bStickGround = 0;
        mpBill2->m_nParticleType = 1;
        mpBill2->m_fParticleV = 0.5f;
        mpBill2->SetColor(0xFF9999AA);
        g_pCurrentScene->m_pEffectContainer->AddChild(mpBill2);
    }

    nRand = rand() % 2;
    fSize = 1.5f;

    auto mpBill3 = new TMEffectBillBoard(0, 500, fSize, fSize, 0.30000001f * fSize, 0.000099999997f, 1, 80);
    if (mpBill3 != nullptr)
    {
        mpBill3->m_vecPosition = TMVector3{ ((float)(rand() % 6 - 3) * 0.02f) + m_vecTempPos[8].x, m_vecTempPos[8].y + 0.60000002f, ((float)(rand() % 40 - 20) * 0.02f) + m_vecTempPos[8].z };
        mpBill3->m_vecStartPos = mpBill3->m_vecPosition;
        mpBill3->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        mpBill3->m_bStickGround = 0;
        mpBill3->m_nParticleType = 1;
        mpBill3->m_fParticleV = 0.5f;
        mpBill3->SetColor(0xFF1111AA);
        g_pCurrentScene->m_pEffectContainer->AddChild(mpBill3);
    }

    fSize = 0.1f;
    nRand = rand() % 20;

    auto mpBill4 = new TMEffectBillBoard(
        0,
        700,
        ((float)nRand * 0.0099999998f) + fSize,
        ((float)nRand * 0.1f) + (fSize * 0.0099999998f),
        ((float)nRand * 0.0099999998f) + (fSize * 0.30000001f),
        0.000099999997f,
        1,
        80);

    if (mpBill4 != nullptr)
    {
        mpBill4->m_vecPosition = TMVector3{ ((float)(rand() % 6 - 3) * 0.02f) + m_vecTempPos[9].x, m_vecTempPos[9].y, ((float)(rand() % 40 - 20) * 0.02f) + m_vecTempPos[9].z };
        mpBill4->m_vecStartPos = mpBill4->m_vecPosition;
        mpBill4->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        mpBill4->m_bStickGround = 0;
        mpBill4->m_nParticleType = 1;
        mpBill4->m_fParticleV = -0.5f;
        mpBill4->SetColor(0xFFFFAAAA);
        g_pCurrentScene->m_pEffectContainer->AddChild(mpBill4);
    }

    nRand = rand() % 20;

    auto mpBill5 = new TMEffectBillBoard(
        0,
        700,
        ((float)nRand * 0.0099999998f) + fSize,
        ((float)nRand * 0.1f) + (fSize * 0.0099999998f),
        ((float)nRand * 0.0099999998f) + (fSize * 0.3f),
        0.000099999997f,
        1,
        80);

    if (mpBill5 != nullptr)
    {
        mpBill5->m_vecPosition = TMVector3{ ((float)(rand() % 6 - 3) * 0.02f) + m_vecTempPos[10].x, m_vecTempPos[10].y, ((float)(rand() % 40 - 20) * 0.02f) + m_vecTempPos[10].z };
        mpBill5->m_vecStartPos = mpBill5->m_vecPosition;
        mpBill5->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        mpBill5->m_bStickGround = 0;
        mpBill5->m_nParticleType = 1;
        mpBill5->m_fParticleV = -0.5f;
        mpBill5->SetColor(0xFFFFAAAA);
        g_pCurrentScene->m_pEffectContainer->AddChild(mpBill5);
    }
}

void TMHuman::RenderEffect_Pig_Wolf(unsigned int dwServerTime)
{
    int nRand = rand() % 5;

    auto pEffect = new TMEffectBillBoard(
        0,
        1500,
        ((float)nRand * 0.0099999998f) + 0.0099999998f,
        ((float)nRand * 0.029999999f) + 0.0099999998f,
        ((float)nRand * 0.0099999998f) + 0.0099999998f,
        0.000099999997f,
        1,
        80);

    if (pEffect != nullptr)
    {
        pEffect->m_vecPosition = TMVector3{ ((float)(rand() % 10 - 5) * 0.02f) + m_vecTempPos[0].x, m_vecTempPos[0].y, ((float)(rand() % 10 - 5) * 0.02f) + m_vecTempPos[0].z };
        pEffect->m_vecStartPos = pEffect->m_vecPosition;
        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pEffect->m_bStickGround = 0;
        pEffect->m_nParticleType = 1;
        pEffect->m_fParticleV = -1.0f;
        pEffect->SetColor(0xFFFFFFCC);

        if (m_nClass == 27)
            pEffect->SetColor(0xFFFFDD88);

        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
    }
}

void TMHuman::RenderEffect_DungeonBear(unsigned int dwServerTime)
{
    if ((dwServerTime - m_dwGolemDustTime) > 1000)
    {
        TMVector3 vec[2]
        {
            { m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y },
            { m_vecTempPos[0] }
        };

        for (int i = 0; i < 6; ++i)
        {
            int nRand = rand() % 5;

            auto pEffect = new TMEffectBillBoard(
                0,
                400 * i + 1500,
                ((float)nRand * 0.1f) + (0.2f * m_fScale),
                ((float)nRand * 0.1f) + (0.2f * m_fScale),
                ((float)nRand * 0.1f) + (0.2f * m_fScale),
                0.001f,
                1,
                80);

            if (pEffect != nullptr)
            {
                pEffect->m_vecPosition = vec[i % 2];
                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_DEFAULT;
                pEffect->m_fParticleV = -1.0f;
                pEffect->SetColor(0xFF00FF00);
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
        m_dwGolemDustTime = dwServerTime;
    }
}

void TMHuman::RenderEffect_Hydra(unsigned int dwServerTime)
{
    unsigned int dwTermTemp = 100;

    if (g_pDevice->m_fFPS < 10.0f)
        dwTermTemp = 1000;
    else if (g_pDevice->m_fFPS < 20.0f)
        dwTermTemp = 600;
    else if (g_pDevice->m_fFPS < 30.0f)
        dwTermTemp = 300;

    if ((dwServerTime - m_dwGolemDustTime) > dwTermTemp)
    {
        for (int i = 0; i < 2; ++i)
        {
            auto pEffect = new TMEffectBillBoard(0, 400 * i + 1500, 0.1f, 0.1f, 0.1f, 0.001f, 1, 80);
            if (pEffect != nullptr)
            {
                pEffect->m_vecPosition = TMVector3{ ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i].x, m_vecTempPos[i].y, ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i].z };
                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pEffect->SetColor(0xFF33FF66);
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
        m_dwGolemDustTime = dwServerTime;
    }
}

void TMHuman::RenderEffect_DarkNightZombieTroll(unsigned int dwServerTime)
{
    unsigned int dwTerm = 400;

    if (m_nClass == 25)
        dwTerm = 1000;

    if ((dwServerTime - m_dwLiquidTime) > dwTerm)
    {
        int nCount = 3;
        float fHeight = 1.0f;
        int nTexIndex = 119;

        if (m_nClass == 25)
        {
            nCount = 2;
            nTexIndex = 89;
            fHeight = 1.6f;
        }

        for (int i = 0; i < nCount; ++i)
        {
            int nRand = rand() % 3;

            auto pEffect = new TMEffectBillBoard(
                nTexIndex,
                600 * i + 2400,
                ((float)nRand * 0.1f) + (1.2f * m_fScale),
                (1.0f * m_fScale) + 0.30000001f,
                ((float)nRand * 0.1f) + (1.2f * m_fScale),
                0.00050000002f,
                1,
                80);

            if (pEffect != nullptr)
            {
                pEffect->m_vecPosition = TMVector3{
                    (((float)i * 0.30000001f) * (float)nRand) + m_vecPosition.x,
                    ((fHeight * m_fScale) + m_fHeight) - (((float)i * 0.30000001f) * (float)nRand),
                    (((float)i * 0.30000001f) * (float)nRand) + m_vecPosition.y };

                pEffect->m_vecStartPos = pEffect->m_vecPosition;
                pEffect->m_nParticleType = 1;
                pEffect->m_fParticleV = -2.0f;
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
        }

        if (m_nClass == 25)
        {
            int nRand = rand() % 3;

            auto pCrater = new TMShade(2, nTexIndex, 1.0f);

            if (pCrater != nullptr)
            {
                pCrater->m_bFI = 0;
                pCrater->m_dwLifeTime = 3000;
                pCrater->m_fAngle = ((float)nRand * 3.1415927f) / 6.0f;
                pCrater->SetColor(0xCCCCCCCC);

                TMVector2 vec
                {
                    ((cosf(m_fAngle - 3.1415927f) * 0.5f) + m_vecPosition.x) + ((float)nRand * 0.2f),
                    (m_vecPosition.y - (sinf(m_fAngle - 3.1415927f) * 0.5f)) + ((float)nRand * 0.2f)
                };

                pCrater->SetPosition(vec);
                g_pCurrentScene->m_pShadeContainer->AddChild(pCrater);
            }
        }
        m_dwLiquidTime = dwServerTime;
    }
}

void TMHuman::RenderEffect_DarkElf(unsigned int dwServerTime)
{
    if ((dwServerTime - m_dwGolemDustTime) > 100)
    {
        int nBase = 1;

        if (m_nClass == 38)
            nBase = 2;

        for (int i = 0; i < 2; ++i)
        {
            auto pEffect = new TMEffectBillBoard(0, 400 * i + 1500, 0.1f, 0.1f, 0.1f, 0.001f, 1, 80);

            if (pEffect != nullptr)
            {
                pEffect->m_vecPosition = TMVector3{ ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i + nBase].x, m_vecTempPos[i + nBase].y, ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i + nBase].z };
                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pEffect->SetColor(0xFFFF6666);
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
        m_dwGolemDustTime = dwServerTime;
    }
}

void TMHuman::RenderEffect_Minotauros(unsigned int dwServerTime)
{
    if ((dwServerTime - m_dwGolemDustTime) > 300)
    {
        auto pEffect = new TMEffectBillBoard(0, 2500, 0.0049999999f * m_fScale, 0.0049999999f * m_fScale, 0.0049999999f * m_fScale, 0.001f, 1, 80);

        if (pEffect != nullptr)
        {
            pEffect->m_vecPosition = TMVector3{ ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[0].x, m_vecTempPos[0].y, ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[0].z };
            pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect->SetColor(0xFFAAAAAA);
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        }

        m_dwGolemDustTime = dwServerTime;
    }
}

void TMHuman::RenderEffect_EmeraldDragon(unsigned int dwServerTime)
{
    if ((dwServerTime - m_dwGolemDustTime) > 300)
    {
        auto pEffect = new TMEffectBillBoard(0, 2500, 0.1f * m_fScale, 0.1f * m_fScale, 0.1f * m_fScale, 0.001f, 1, 80);

        if (pEffect != nullptr)
        {
            pEffect->m_vecPosition = TMVector3{ ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[0].x, m_vecTempPos[0].y, ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[0].z };
            pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pEffect->SetColor(0xFFAAAAAA);
            g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
        }

        m_dwGolemDustTime = dwServerTime;
    }

    for (int i = 1; i <= 2; ++i)
    {
        if (m_pEyeFire2[i] != nullptr)
        {
            m_pEyeFire2[i]->m_vecPosition = m_vecTempPos[i + 7];
            m_pEyeFire2[i]->FrameMove(0);
        }
    }
}

void TMHuman::RenderEffect_BoneDragon(unsigned int dwServerTime)
{
    if ((dwServerTime - m_dwGolemDustTime) > 300)
    {
        for (int i = 1; i < 8; ++i)
        {
            auto pEffect = new TMEffectBillBoard(0, 2500, 1.5f * m_fScale, 1.5f * m_fScale, 1.5f * m_fScale, 0.001f, 1, 80);
            if (pEffect != nullptr)
            {
                pEffect->m_vecPosition = TMVector3{ ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i].x, m_vecTempPos[i].y, ((float)(rand() % 10 - 5) * 0.050000001f) + m_vecTempPos[i].z };
                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pEffect->SetColor(0xFF00AA66);
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
        m_dwGolemDustTime = dwServerTime;
    }
}

void TMHuman::RenderEffect_Golem(unsigned int dwServerTime)
{
    if (m_stLookInfo.FaceMesh == 2)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (m_pEyeFire[i] != nullptr)
            {
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[i];
                m_pEyeFire[i]->m_vecPosition.y += (0.5f * m_fScale);
                m_pEyeFire[i]->FrameMove(0);
            }
        }
    }
    else if (m_stLookInfo.FaceMesh == 1 && !m_stLookInfo.FaceSkin)
    {
        unsigned int dwTerm = 500;
        float fSpeed = 0.000099999997f;
        float fBaseSize = 1.0f;
        int nTextureIndex = 119;

        if (m_stLookInfo.FaceSkin == 2)
        {
            nTextureIndex = 152;
            fSpeed = 0.00019999999f;
            fBaseSize = 0.30000001f;
            dwTerm = 600;
        }

        if ((dwServerTime - m_dwGolemDustTime) > dwTerm)
        {
            for (int j = 0; j < 6; ++j)
            {
                auto pEffect = new TMEffectBillBoard(
                    nTextureIndex,
                    3000,
                    (fBaseSize * m_fScale) + 0.1f,
                    (fBaseSize * m_fScale) + 0.30000001f,
                    (fBaseSize * m_fScale) + 0.1f,
                    fSpeed,
                    1,
                    80);

                if (pEffect != nullptr)
                {
                    if (m_stLookInfo.FaceSkin == 2)
                    {
                        pEffect->SetColor(0xAAFFFFFF);
                        pEffect->m_nParticleType = 1;
                        pEffect->m_fParticleV = -1.0f;
                    }
                    pEffect->m_nFade = 3;
                    pEffect->m_vecStartPos = pEffect->m_vecPosition = m_vecTempPos[j];
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
            }
            m_dwGolemDustTime = dwServerTime;
        }
    }
}

void TMHuman::RenderEffect_Skull()
{
    static const int nPosIndex[7]{ 8, 9, 1, 6, 7, 2, 3 };

    for (int i = 0; i < 7; ++i)
    {
        if (m_pEyeFire[i] != nullptr)
        {
            m_pEyeFire[i]->m_vecPosition = m_vecTempPos[nPosIndex[i]];

            if (i >= 3 && i < 5)
                m_pEyeFire[i]->m_vecPosition.y += (m_fScale * 0.1f);
            if (i >= 5)
                m_pEyeFire[i]->m_vecPosition.y += (m_fScale * 0.30000001f);

            m_pEyeFire[i]->FrameMove(0);
        }
    }
}
