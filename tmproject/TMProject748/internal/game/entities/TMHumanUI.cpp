#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "SControl.h"
#include "TMScene.h"
#include "TMEffectMeshRotate.h"
#include "TMEffectSkinMesh.h"
#include "SControlContainer.h"
#include "ObjectManager.h"
#include "TMCamera.h"
#include "TMMesh.h"
#include "TMFieldScene.h"
#include "TMObjectContainer.h"

int TMHuman::IsMouseOver()
{
    if (m_dwDelayDel)
        return 0;

    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELECT_SERVER)
    {
        m_bMouseOver = 0;
        return 0;
    }
    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_DEMO)
    {
        m_bMouseOver = 0;
        return 0;
    }
    if (m_cHide || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
    {
        m_bMouseOver = 0;
        return 0;
    }

    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD || g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill] != 31)
    {
        if (m_eMotion == ECHAR_MOTION::ECMOTION_DEAD || m_eMotion == ECHAR_MOTION::ECMOTION_DIE || m_cDie == 1)
        {
            if (g_pCurrentScene->m_pMouseOverHuman == this)
                g_pCurrentScene->m_pMouseOverHuman = nullptr;

            m_bMouseOver = 0;
            return 0;
        }
    }
    else
    {
        if (m_eMotion != ECHAR_MOTION::ECMOTION_DEAD && m_eMotion != ECHAR_MOTION::ECMOTION_DIE && m_cDie != 1)
        {
            if (g_pCurrentScene->m_pMouseOverHuman == this)
                g_pCurrentScene->m_pMouseOverHuman = nullptr;

            m_bMouseOver = 0;
            return 0;
        }
    }

    D3DXVECTOR3 vPickRayDir{};
    D3DXVECTOR3 vPickRayOrig{};
    g_pDevice->GetPickRayVector(&vPickRayOrig, &vPickRayDir);

    TMVector3 vecCam = g_pObjectManager->m_pCamera->m_cameraPos;

    TMHuman* pOldOverHuman = g_pCurrentScene->m_pMouseOverHuman;
    TMHuman* pFocusedObject = g_pCurrentScene->m_pMyHuman;

    bool bMouseOver = 0;

    int nMeshType = m_nSkinMeshType;
    if (m_cMount > 0)
        nMeshType = m_nMountSkinMeshType;

    int nCommon = m_stLookInfo.LeftMesh;
    TMMesh* pMesh = g_pMeshManager->GetCommonMesh(nCommon, 0, 3_min);

    if (m_pNameLabel && m_pNameLabel->IsOver())
    {
        if (!pFocusedObject)
        {
            g_pCurrentScene->m_pMouseOverHuman = this;
            m_bMouseOver = 1;
        }

        bMouseOver = 1;
    }

    D3DXVECTOR3 v0{};
    D3DXVECTOR3 v1{};
    D3DXVECTOR3 v2{};
    D3DXVECTOR3 v3{};

    float fRadius = (TMHuman::m_vecPickSize[nMeshType].x * m_fScale) + 0.5f;

    if (pMesh && (nCommon == 2888 || nCommon == 2889))
    {
        if (nCommon == 2888)
        {
            fRadius = 3.0f;
        }
        else if (nCommon == 2889)
        {
            fRadius = 4.0f;
        }

        if (!bMouseOver)
        {
            v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y - fRadius);
            v1 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y + fRadius);
            v2 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
            v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

            if (D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
            {
                if (!pFocusedObject)
                {
                    g_pCurrentScene->m_pMouseOverHuman = this;
                    m_bMouseOver = 1;
                }
                bMouseOver = 1;
            }
            if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
            {
                if (!pFocusedObject)
                {
                    g_pCurrentScene->m_pMouseOverHuman = this;
                    m_bMouseOver = 1;
                }
                bMouseOver = 1;
            }
        }
    }
    if (IAmkhepra() == 1)
    {
        fRadius -= 0.2f;

        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y);
        v1 = D3DXVECTOR3(m_vecPosition.x + fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y);
        v2 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y);

        if (D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
    }
    if (!bMouseOver)
    {
        fRadius = (TMHuman::m_vecPickSize[nMeshType].x * m_fScale) * 1.0f;
        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y - fRadius);
        v1 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y + fRadius);
        v2 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

        if (D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }

        D3DXVECTOR3 v4{};
        D3DXVECTOR3 v5{};
        D3DXVECTOR3 v6{};
        D3DXVECTOR3 v7{};

        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y - fRadius);
        v1 = D3DXVECTOR3(m_vecPosition.x + fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y - fRadius);
        v2 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v4 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y + fRadius);
        v5 = D3DXVECTOR3(m_vecPosition.x + fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y + fRadius);
        v6 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y + fRadius);
        v7 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

        if (!bMouseOver && D3DXIntersectTri(&v4, &v0, &v2, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v6, &v4, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v0, &v1, &v3, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v3, &v2, &v0, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v1, &v5, &v7, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v7, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v5, &v4, &v6, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v6, &v7, &v5, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
    }
    if (!bMouseOver && m_nClass == 56 && !m_stLookInfo.FaceMesh)
    {
        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v1 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y + fRadius);
        v2 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

        if (!bMouseOver && D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
    }
    if (g_pObjectManager->m_pCamera && pFocusedObject != this)
    {
        if (bMouseOver == 1)
        {
            TMVector2 vec2{ vecCam.x, vecCam.z };
            if (!pOldOverHuman || vec2.DistanceFrom(pOldOverHuman->m_vecPosition) > vec2.DistanceFrom(m_vecPosition))
            {
                if (g_pCurrentScene->m_pMouseOverHuman)
                    g_pCurrentScene->m_pMouseOverHuman->m_bMouseOver = 0;

                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
        }
        else if (g_pCurrentScene->m_pMouseOverHuman == this)
        {
            g_pCurrentScene->m_pMouseOverHuman = 0;
            m_bMouseOver = 1;
        }
    }
    if (bMouseOver == 1)
    {
        float fDis = TMVector2(vecCam.x, vecCam.z).DistanceFrom(m_vecPosition);

        if (fDis < 1.5f)
        {
            bMouseOver = 0;
            m_bMouseOver = 0;
            g_pCurrentScene->m_pMouseOverHuman = pOldOverHuman;
        }
        else if (pOldOverHuman)
        {
            if (pFocusedObject != this)
            {
                if (m_dwID >= 0 && m_dwID < 1000)
                {
                    if ((pOldOverHuman->m_dwID < 0 || pOldOverHuman->m_dwID > 1000) && pOldOverHuman->m_cSummons == 1)
                    {
                        if (g_pCurrentScene->m_pMouseOverHuman)
                            g_pCurrentScene->m_pMouseOverHuman->m_bMouseOver = 0;

                        m_bMouseOver = 1;
                        g_pCurrentScene->m_pMouseOverHuman = this;
                    }
                }
            }
        }
    }
    if (!m_bMouseOver && bMouseOver == 1)
    {
        TMScene* pScene = g_pCurrentScene;
        if (pScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        {
            auto pSoundManager = g_pSoundManager;
            if (pSoundManager)
            {
                auto pSoundData = pSoundManager->GetSoundData(52);
                if (pSoundData->IsSoundPlaying())
                {
                    pSoundData->Play();
                }
            }
        }
    }

    m_bMouseOver = bMouseOver;
    return bMouseOver;
}

int TMHuman::OnCharEvent(char iCharCode, int lParam)
{
    if (m_dwDelayDel)
        return 0;

    return TreeNode::OnCharEvent(iCharCode, lParam);
}

void TMHuman::LabelPosition()
{
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (!pFScene || g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return;

    if (m_dwDelayDel)
        return;

    if (m_cHide == 1 || m_cShadow == 1 && pFScene->m_pMyHuman != this &&
        !pFScene->m_pMyHuman->m_JewelGlasses)
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pProgressBar1->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible( 0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pTitleProgressBar->SetVisible(0);
        m_pTitleNameLabel->SetVisible(0);
        return;
    }

    if (pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
    {
        if (m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 &&
            m_sMountIndex != 29 && m_sMountIndex != 30 && !pFScene->m_bAirMove)
        {
            pFScene->m_pMHPBar->SetVisible(1);
            pFScene->m_pMHPBarT->SetVisible(1);
        }
        else
        {
            pFScene->m_pMHPBar->SetVisible(0);
            pFScene->m_pMHPBarT->SetVisible(0);
        }
    }

    m_pTitleNameLabel->SetVisible(0);
    m_pTitleProgressBar->SetVisible(0);
    bool bTargetMob = false;
    if (pFScene->m_pMyHuman && m_dwID == pFScene->m_pMyHuman->m_nAttackDestID && !m_cDie)
    {
        bTargetMob = 1;
        m_pTitleProgressBar->SetVisible(1);
    }
    if (pFScene->m_pMouseOverHuman != this && !m_TradeDesc[0] &&
        (m_sHeadIndex == 216 || m_sHeadIndex == 226 || m_sHeadIndex == 298))
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pProgressBar1->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pNickNameLabel->SetVisible(0);
        return;
    }

    if (g_nUpdateGuildName > 0)
    {
        if (m_dwID >= 0 && m_dwID < 1000 && (int)m_usGuild > 0)
            UpdateGuildName();
    }

    if (m_pNameLabel)
    {
        if (g_bEvent == 1)
            return;

        // FUN_00504a80 evaluates TradeDesc before the generic non-hover cull;
        // an open 7.48 shop therefore keeps its title and panel in this path.
        if (pFScene->m_pMouseOverHuman != this && !m_TradeDesc[0]
            && (m_nClass != 1 && m_nClass != 2 && m_nClass != 4 && m_nClass != 8 && m_nClass != 26 &&
                (m_nClass != 33 || m_stLookInfo.FaceMesh) ||
                (m_dwID < 0 || m_dwID >= 1000)) &&
            !IsMerchant() &&
            m_bParty != 1 &&
            (int)m_usGuild <= 0 &&
            (m_dwID < 0 || m_dwID >= 1000) &&
            (m_sHeadIndex != 271 || !(m_stScore.Merchant & 0xF)) &&
            bTargetMob != 1)
        {
            m_pNameLabel->SetVisible(0);
            m_pKillLabel->SetVisible(0);
            m_pProgressBar->SetVisible(0);
            m_pProgressBar1->SetVisible(0);
            m_pMountHPBar->SetVisible(0);
            if (m_stGuildMark.pGuildMark)
                m_stGuildMark.pGuildMark->SetVisible(0);
            m_pAutoTradeDesc->SetVisible(0);
            m_pAutoTradePanel->SetVisible(0);
            m_pNickNameLabel->SetVisible(0);
            m_pTitleProgressBar->SetVisible( 0);
            m_pTitleNameLabel->SetVisible(0);
            if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
            {
                m_pTitleProgressBar->SetVisible(1);
                m_pTitleNameLabel->SetVisible(1);
            }
        }
        else
        {
            D3DXVECTOR3 vTemp;
            D3DXVECTOR3 vPosTransformed;
            D3DXVECTOR3 vecPos;

            vecPos.x = m_vecPosition.x;
            vecPos.z = m_vecPosition.y;
            vecPos.y = ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight) + 0.2f;
            D3DXVec3TransformCoord(&vTemp, &vecPos, &g_pDevice->m_matView);
            D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);
            if (vPosTransformed.z < 0.0f || vPosTransformed.z >= 1.0f)
            {
                m_pNameLabel->SetVisible(0);
                m_pKillLabel->SetVisible(0);
                m_pProgressBar->SetVisible(0);
                m_pProgressBar1->SetVisible(0);
                m_pMountHPBar->SetVisible(0);

                if (m_stGuildMark.pGuildMark)
                    m_stGuildMark.pGuildMark->SetVisible(0);

                m_pAutoTradeDesc->SetVisible(0);
                m_pAutoTradePanel->SetVisible(0);
                m_pNickNameLabel->SetVisible( 0);
                m_pChatMsg->SetVisible(0);
                m_pTitleProgressBar->SetVisible(0);
                m_pTitleNameLabel->SetVisible(0);
                if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                {
                    m_pTitleProgressBar->SetVisible(1);
                    m_pTitleNameLabel->SetVisible(1);
                }
            }
            else
            {
                int vPosInX = (int)(((vPosTransformed.x + 1.0f) * (float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)) / 2.0f);
                int vPosInY = 0;
                if (m_cMount)
                    vPosInY = (int)((-vPosTransformed.y + 0.76f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    + (int)(g_pObjectManager->m_pCamera->m_fSightLength * 3.5f);
                else
                    vPosInY = (int)((-vPosTransformed.y + 1.16f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    - 3 * (int)g_pObjectManager->m_pCamera->m_fSightLength;

                if (vPosInX <= 0 || vPosInX >= (int)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) ||
                    vPosInY <= 0 || vPosInY >= (int)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
                {
                    m_pNameLabel->SetVisible(0);
                    m_pKillLabel->SetVisible(0);
                    m_pProgressBar->SetVisible(0);
                    m_pProgressBar1->SetVisible(0);
                    m_pMountHPBar->SetVisible(0);
                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetVisible(0);
                    m_pAutoTradeDesc->SetVisible(0);
                    m_pAutoTradePanel->SetVisible(0);
                    m_pNickNameLabel->SetVisible(0);
                    m_pChatMsg->SetVisible(0);
                    m_pTitleProgressBar->SetVisible(0);
                    m_pTitleNameLabel->SetVisible(0);
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                }
                else
                {
                    if (!m_ucChaosLevel)
                    {
                        float fProgress = (float)(g_pTimerManager->GetServerTime() % 2000);
                        fProgress = sinf((float)(fProgress * D3DXToRadian(180)) / 2000.0f);
                        m_pNameLabel->SetTextColor(((unsigned int)(float)(fProgress * 255.0f) << 16) | 0xFF000000);
                    }

                    short sLevel = pFScene->m_pMyHuman->m_stScore.Level;
                    if (pFScene->m_pMyHuman->Is2stClass() == 2)
                        sLevel += 400;

                    if (sLevel - 40 >= m_stScore.Level &&
                        m_sHeadIndex > 40 && m_stScore.Level < 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF737373;
                    }
                    if (sLevel - 40 < m_stScore.Level &&
                        m_sHeadIndex > 40 || m_stScore.Level >= 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                    }
                    m_pNameLabel->SetVisible(1);
                    if (m_pTitleProgressBar->m_bVisible == 1)
                        m_pTitleNameLabel->SetVisible(1);

                    if (m_stGuildMark.pGuildMark && m_stGuildMark.nGuild != -1 &&
                        m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex >= 0)
                    {
                        int nMark = m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex;
                        if (nMark < 0 || nMark > 64 || m_stGuildMark.nGuild == (g_pTextureManager->m_stGuildMark[nMark].nGuild & 0xFFFF) &&
                            g_pTextureManager->m_stGuildMark[nMark].nGuild >> 16 == m_stGuildMark.nGuildChannel)
                        {
                            if (!m_stGuildMark.bHideGuildmark)
                                m_stGuildMark.pGuildMark->SetVisible(1);
                        }
                        else
                        {
                            m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex = -1;
                            m_stGuildMark.pGuildMark->SetVisible(0);
                        }
                    }
                    m_pNickNameLabel->SetVisible(1);
                    if (m_cDie == 1)
                        m_pNameLabel->SetVisible(0);

                    if (m_dwID >= 0 && m_dwID < 1000 && (m_nCurrentKill > 0 || (int)m_nTotalKill > 0))
                    {
                        m_pKillLabel->SetVisible(1);
                        if ((int)m_vecPosition.x >> 7 > 16 && (int)m_vecPosition.x >> 7 < 20 && (int)m_vecPosition.y >> 7 > 29 &&
                            pFScene->m_pMyHuman != this)
                        {
                            m_pKillLabel->SetVisible(0);
                        }

                        if ((int)m_vecPosition.x >> 7 == 17 && (int)m_vecPosition.y >> 7 == 28)
                            m_pKillLabel->SetVisible(0);
                    }

                    if (m_TradeDesc[0])
                    {
                        m_pAutoTradeDesc->SetVisible(1);
                        m_pAutoTradePanel->SetVisible(1);
                    }
                    else
                    {
                        m_pAutoTradeDesc->SetVisible(0);
                        m_pAutoTradePanel->SetVisible(0);
                    }
                    if (g_pCurrentScene->m_pMyHuman != this)
                    {

                        if (m_nClass != 56 || m_stLookInfo.FaceMesh)
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pProgressBar1->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pProgressBar1->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                        }
                    }

                    vPosInY = (int)((float)vPosInY + (RenderDevice::m_fHeightRatio * 13.0f));
                    float fWidthRatio = RenderDevice::m_fWidthRatio;
                    if (m_cMount)
                        vPosInY = vPosInY + (int)((RenderDevice::m_fHeightRatio * 2.0f) * 4.0f);
                    else
                        vPosInY = vPosInY - (int)((RenderDevice::m_fHeightRatio * 2.0f) * 16.0f);

                    if (1.0 == RenderDevice::m_fHeightRatio)
                    {
                        if (m_cMount)
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 8.0f);
                        else
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 10.0f);
                    }
                    else if (RenderDevice::m_fHeightRatio >= 1.7)
                    {
                        if (m_cMount)
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 15.0f);
                        else
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 6.0f);
                    }
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                    else
                    {
                        if (m_cMount)
                            m_pProgressBar->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f),
                                ((float)vPosInY - BASE_ScreenResize(13.5f)) - 12.5f);
                        else
                            m_pProgressBar->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f),
                                (float)vPosInY - BASE_ScreenResize(12.0f));

                        if (m_cMount)
                            m_pProgressBar1->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f),
                                ((float)vPosInY - BASE_ScreenResize(15.5f)) - 15.6f);
                        else
                            m_pProgressBar1->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f),
                                (float)vPosInY - BASE_ScreenResize(17.0f));

                        m_pMountHPBar->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f),
                            ((float)vPosInY - BASE_ScreenResize(18.0f)) - 18.0f);

                        m_pProgressBar->SetSize(BASE_ScreenResize(72.0f), BASE_ScreenResize(1.5f) + 6.0f);
                        m_pProgressBar1->SetSize(BASE_ScreenResize(72.0f), BASE_ScreenResize(1.5f) + 7.0f);
                        m_pMountHPBar->SetSize(BASE_ScreenResize(72.0f), BASE_ScreenResize(1.5f) + 6.0f);

                        m_pMountHPBar->Update();
                        m_pProgressBar->Update();
                        m_pProgressBar1->Update();
                        m_pProgressBar->m_GCProgress.nWidth = m_pProgressBar->m_nProgressWidth - 4.0f;
                        m_pProgressBar->m_GCProgress.nHeight = m_pProgressBar->m_nHeight - 4.0f;
                        m_pProgressBar1->m_GCProgress.nWidth = m_pProgressBar1->m_nProgressWidth - 3.5f;
                        m_pProgressBar1->m_GCProgress.nHeight = m_pProgressBar1->m_nHeight - 5.0f;
                        m_pMountHPBar->m_GCProgress.nWidth = m_pMountHPBar->m_nProgressWidth - 4.0f;
                        m_pMountHPBar->m_GCProgress.nHeight = m_pMountHPBar->m_nHeight - 4.0f;
                    }

                    int nLen2 = strlen(m_pNameLabel->GetText());
                    int nLen3 = strlen(m_TradeDesc);
                    int nLen4 = strlen(m_pNickNameLabel->GetText());

                    if (m_cMount)
                        m_pChatMsg->SetRealPos((float)vPosInX - (float)(m_pChatMsg->m_nWidth / 2.0f), (float)vPosInY - 85.0f);
                    else
                        m_pChatMsg->SetRealPos((float)vPosInX - (float)(m_pChatMsg->m_nWidth / 2.0f), (float)vPosInY - 75.0f);

                    // Native FUN_00504a80 anchors the title 13 scaled pixels below
                    // NewUI_AutoTrade_BG (140/150 centering widths respectively).
                    m_pAutoTradeDesc->SetRealPos((float)vPosInX - ((140.0f * fWidthRatio) / 2.0f), (float)vPosInY);
                    m_pAutoTradePanel->SetRealPos((float)vPosInX - ((150.0f * fWidthRatio) / 2.0f),
                        (float)vPosInY - (float)(13.0f * RenderDevice::m_fHeightRatio));

                    float nPosY = (float)vPosInY;
                    if (m_cMount && !m_pAutoTradeDesc->IsVisible())
                        nPosY = nPosY - 20.0f;

                    fWidthRatio = RenderDevice::m_fWidthRatio;
                    m_pNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen2 + 2)) * RenderDevice::m_fWidthRatio) / 2.0f),
                        nPosY);

                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetRealPos(((float)vPosInX - (((float)(6 * (nLen2 + 2)) * fWidthRatio) / 2.0f)) - 10.0f,
                            nPosY + 2.0f);

                    m_pNickNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen4 + 2)) * fWidthRatio) / 2.0f),
                        nPosY - (float)(28.0f * fWidthRatio));

                    if ((int)m_vecPosition.x >> 7 > 1 && (int)m_vecPosition.x >> 7 < 11 && (int)m_vecPosition.y >> 7 < 5)
                    {
                        m_pProgressBar->SetVisible(1);
                        m_pProgressBar1->SetVisible(1);
                        m_pMountHPBar->SetVisible(0);
                        m_pTitleProgressBar->SetVisible(0);
                        m_pTitleNameLabel->SetVisible(0);
                    }

                    else if (m_nClass == 1 || m_nClass == 2 || m_nClass == 4 || m_nClass == 8 || m_nClass == 26 || m_nClass == 33 &&
                        !m_stLookInfo.FaceMesh || m_sHeadIndex == 271 && m_stScore.Merchant & 0xF)
                    {
                        if (_locationCheck(m_vecPosition, 14, 28) && m_sHeadIndex == 51)
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pProgressBar1->SetVisible(1);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pProgressBar1->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                            m_pTitleProgressBar->SetVisible(0);
                            m_pTitleNameLabel->SetVisible(0);
                            m_pKillLabel->SetVisible(1);
                        }
                    }

                    auto pFocused = g_pCurrentScene->m_pMyHuman;
                    if (pFocused)
                    {

                        if (!IsMerchant() && m_sHeadIndex != 57 && !m_pAutoTradeDesc->IsVisible())
                        {
                            if (!IsMerchant() && m_sHeadIndex <= 43 && !m_pAutoTradeDesc->IsVisible()) // Added head-index visibility guard.
                            {
                                m_pProgressBar1->SetVisible(1);
                            }
                            m_pProgressBar->SetVisible(1);

                            if (g_pCurrentScene->m_pMyHuman != this && g_pCurrentScene->m_pMouseOverHuman == this)
                            {
                                if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                                    m_pProgressBar->SetVisible(0);

                                m_pTitleNameLabel->SetVisible(1);//
                                m_pTitleProgressBar->SetVisible(1);
                            }
                        }

                        if (pFocused == this && m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 && m_sMountIndex != 29 && m_sMountIndex != 30)
                            m_pMountHPBar->SetVisible(1);
                        else
                            m_pMountHPBar->SetVisible(0);

                        if (pFocused == this&& m_cMount == 1)
                        {
                            int MountIndex = g_pObjectManager->m_stMobData.Equip[14].sIndex;
                            if (MountIndex >= 3980 && MountIndex <= 3999)
                                m_pMountHPBar->SetVisible(0);
                        }

                        int blueblue = SlateBlue;
;
                        m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                        m_pProgressBar1->m_GCProgress.dwColor = blueblue;

                        if (g_bCastleWar)
                        {
                            if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pProgressBar1->m_GCProgress.dwColor = blueblue;
                        }
                        else if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                        {
                            if (m_dwID < 0 || m_dwID >= 1000 && !TMFieldScene::m_bPK)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pProgressBar1->m_GCProgress.dwColor = blueblue;
                        }
                        if (pFocused != this && m_citizen == pFocused->m_citizen && m_sHeadIndex < 40)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                            m_pProgressBar1->m_GCProgress.dwColor = blueblue;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                        }
                        if (IsInCastleZone2() && !g_bCastleWar)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                            m_pProgressBar1->m_GCProgress.dwColor = blueblue;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                        }
                        if (pFocused == this || m_bParty == 1 || m_usGuild &&
                            (m_usGuild == pFocused->m_usGuild || g_pObjectManager->m_usAllyGuild == m_usGuild))
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pProgressBar1->m_GCProgress.dwColor = blueblue;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF006400;
                        }
                    }
                }
            }
        }
    }
    if (IsInTown() == 1 && m_nClass == 16)
        m_pProgressBar->SetVisible(0);

    if (m_pAutoTradeDesc->IsVisible() == 1)
    {
        m_pProgressBar->SetVisible(0);
        m_pProgressBar1->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_stGuildMark.pGuildMark->SetVisible(0);
    }
    else if (!m_stGuildMark.bLoadedGuildmark)
    {
        if (!strlen(m_TradeDesc) && m_stGuildMark.sGuildIndex && m_stGuildMark.nGuild != -1 && !m_pAutoTradeDesc->IsVisible())
            pFScene->Guildmark_Create(&m_stGuildMark);
    }
    if (m_bIgnoreHeight == 1)
        m_pMountHPBar->SetVisible(0);



    if (!pFScene->m_bShowNameLabel && pFScene->m_pMouseOverHuman != this && !bTargetMob)
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pProgressBar1->SetVisible(0);
        m_pMountHPBar->SetVisible(0);
        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);
        // The global name-label toggle must not hide an active 7.48 shop sign;
        // native AutoTrade visibility is governed by TradeDesc, not mouse hover.
        if (!m_TradeDesc[0])
        {
            m_pAutoTradeDesc->SetVisible(0);
            m_pAutoTradePanel->SetVisible(0);
        }
        m_pKillLabel->SetVisible(0);
        m_pNickNameLabel->SetVisible(0);
    }
}

void TMHuman::LabelPosition2()
{
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (!pFScene || g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return;

    if (m_dwDelayDel)
        return;

    if (m_cHide == 1 || m_cShadow == 1 && pFScene->m_pMyHuman != this &&
        !pFScene->m_pMyHuman->m_JewelGlasses)
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pProgressBar1->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pTitleProgressBar->SetVisible(0);
        m_pTitleNameLabel->SetVisible(0);
        return;
    }

    if (pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
    {
        if (m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 &&
            m_sMountIndex != 29 && m_sMountIndex != 30 && !pFScene->m_bAirMove)
        {
            pFScene->m_pMHPBar->SetVisible(1);
            pFScene->m_pMHPBarT->SetVisible(1);
        }
        else
        {
            pFScene->m_pMHPBar->SetVisible(0);
            pFScene->m_pMHPBarT->SetVisible(0);
        }
    }

    m_pTitleNameLabel->SetVisible(0);
    m_pTitleProgressBar->SetVisible(0);
    bool bTargetMob = false;
    if (pFScene->m_pMyHuman && m_dwID == pFScene->m_pMyHuman->m_nAttackDestID && !m_cDie)
    {
        bTargetMob = 1;
        m_pTitleProgressBar->SetVisible(1);
    }
    // Carbunkle shop actors use these head indices, but an active 7.48
    // TradeDesc keeps their AutoTrade sign visible independently of hover.
    if (pFScene->m_pMouseOverHuman != this && !m_TradeDesc[0] &&
        (m_sHeadIndex == 216 || m_sHeadIndex == 226 || m_sHeadIndex == 298))
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pProgressBar1->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pNickNameLabel->SetVisible(0);
        return;
    }

    if (g_nUpdateGuildName > 0)
    {
        if (m_dwID >= 0 && m_dwID < 1000 && (int)m_usGuild > 0)
            UpdateGuildName();
    }

    if (m_pNameLabel)
    {
        if (g_bEvent == 1)
            return;

        // FUN_00504a80 lets a non-empty TradeDesc enter the positioning path;
        // the generic non-hover label cull must not hide the shop panel/title.
        if (pFScene->m_pMouseOverHuman != this && !m_TradeDesc[0]
            && (m_nClass != 1 && m_nClass != 2 && m_nClass != 4 && m_nClass != 8 && m_nClass != 26 &&
                (m_nClass != 33 || m_stLookInfo.FaceMesh) ||
                (m_dwID < 0 || m_dwID >= 1000)) &&
            !IsMerchant() &&
            m_bParty != 1 &&
            (int)m_usGuild <= 0 &&
            (m_dwID < 0 || m_dwID >= 1000) &&
            (m_sHeadIndex != 271 || !(m_stScore.Merchant & 0xF)) &&
            bTargetMob != 1)
        {
            m_pNameLabel->SetVisible(0);
            m_pKillLabel->SetVisible(0);
            m_pProgressBar->SetVisible(0);
            m_pProgressBar1->SetVisible(0);
            m_pMountHPBar->SetVisible(0);
            if (m_stGuildMark.pGuildMark)
                m_stGuildMark.pGuildMark->SetVisible(0);
            m_pAutoTradeDesc->SetVisible(0);
            m_pAutoTradePanel->SetVisible(0);
            m_pNickNameLabel->SetVisible(0);
            m_pTitleProgressBar->SetVisible(0);
            m_pTitleNameLabel->SetVisible(0);
            if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
            {
                m_pTitleProgressBar->SetVisible(1);
                m_pTitleNameLabel->SetVisible(1);
            }
        }
        else
        {
            D3DXVECTOR3 vTemp;
            D3DXVECTOR3 vPosTransformed;
            D3DXVECTOR3 vecPos;

            vecPos.x = m_vecPosition.x;
            vecPos.z = m_vecPosition.y;
            vecPos.y = ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight) + 0.2f;
            D3DXVec3TransformCoord(&vTemp, &vecPos, &g_pDevice->m_matView);
            D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);
            if (vPosTransformed.z < 0.0f || vPosTransformed.z >= 1.0f)
            {
                m_pNameLabel->SetVisible(0);
                m_pKillLabel->SetVisible(0);
                m_pProgressBar->SetVisible(0);
                m_pProgressBar1->SetVisible(0);
                m_pMountHPBar->SetVisible(0);

                if (m_stGuildMark.pGuildMark)
                    m_stGuildMark.pGuildMark->SetVisible(0);

                m_pAutoTradeDesc->SetVisible(0);
                m_pAutoTradePanel->SetVisible(0);
                m_pNickNameLabel->SetVisible(0);
                m_pChatMsg->SetVisible(0);
                m_pTitleProgressBar->SetVisible(0);
                m_pTitleNameLabel->SetVisible(0);
                if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                {
                    m_pTitleProgressBar->SetVisible(1);
                    m_pTitleNameLabel->SetVisible(1);
                }
            }
            else
            {
                int vPosInX = (int)(((vPosTransformed.x + 1.0f) * (float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)) / 2.0f);
                int vPosInY = 0;
                if (m_cMount)
                    vPosInY = (int)((-vPosTransformed.y + 0.76f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    + (int)(g_pObjectManager->m_pCamera->m_fSightLength * 3.5f);
                else
                    vPosInY = (int)((-vPosTransformed.y + 1.16f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    - 3 * (int)g_pObjectManager->m_pCamera->m_fSightLength;

                if (vPosInX <= 0 || vPosInX >= (int)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) ||
                    vPosInY <= 0 || vPosInY >= (int)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
                {
                    m_pNameLabel->SetVisible(0);
                    m_pKillLabel->SetVisible(0);
                    m_pProgressBar->SetVisible(0);
                    m_pProgressBar1->SetVisible(0);
                    m_pMountHPBar->SetVisible(0);
                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetVisible(0);
                    m_pAutoTradeDesc->SetVisible(0);
                    m_pAutoTradePanel->SetVisible(0);
                    m_pNickNameLabel->SetVisible(0);
                    m_pChatMsg->SetVisible(0);
                    m_pTitleProgressBar->SetVisible(0);
                    m_pTitleNameLabel->SetVisible(0);
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                }
                else
                {
                    if (!m_ucChaosLevel)
                    {
                        float fProgress = (float)(g_pTimerManager->GetServerTime() % 2000);
                        fProgress = sinf((float)(fProgress * D3DXToRadian(180)) / 2000.0f);
                        m_pNameLabel->SetTextColor(((unsigned int)(float)(fProgress * 255.0f) << 16) | 0xFF000000);
                    }

                    short sLevel = pFScene->m_pMyHuman->m_stScore.Level;
                    if (pFScene->m_pMyHuman->Is2stClass() == 2)
                        sLevel += 400;

                    if (sLevel - 40 >= m_stScore.Level &&
                        m_sHeadIndex > 40 && m_stScore.Level < 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF737373;
                    }
                    if (sLevel - 40 < m_stScore.Level &&
                        m_sHeadIndex > 40 || m_stScore.Level >= 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                    }
                    m_pNameLabel->SetVisible(1);
                    if (m_pTitleProgressBar->m_bVisible == 1)
                        m_pTitleNameLabel->SetVisible(1);

                    if (m_stGuildMark.pGuildMark && m_stGuildMark.nGuild != -1 &&
                        m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex >= 0)
                    {
                        int nMark = m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex;
                        if (nMark < 0 || nMark > 64 || m_stGuildMark.nGuild == (g_pTextureManager->m_stGuildMark[nMark].nGuild & 0xFFFF) &&
                            g_pTextureManager->m_stGuildMark[nMark].nGuild >> 16 == m_stGuildMark.nGuildChannel)
                        {
                            if (!m_stGuildMark.bHideGuildmark)
                                m_stGuildMark.pGuildMark->SetVisible(1);
                        }
                        else
                        {
                            m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex = -1;
                            m_stGuildMark.pGuildMark->SetVisible(0);
                        }
                    }
                    m_pNickNameLabel->SetVisible(1);
                    if (m_cDie == 1)
                        m_pNameLabel->SetVisible(0);

                    if (m_dwID >= 0 && m_dwID < 1000 && (m_nCurrentKill > 0 || (int)m_nTotalKill > 0))
                    {
                        m_pKillLabel->SetVisible(1);
                        if ((int)m_vecPosition.x >> 7 > 16 && (int)m_vecPosition.x >> 7 < 20 && (int)m_vecPosition.y >> 7 > 29 &&
                            pFScene->m_pMyHuman != this)
                        {
                            m_pKillLabel->SetVisible(0);
                        }

                        if ((int)m_vecPosition.x >> 7 == 17 && (int)m_vecPosition.y >> 7 == 28)
                            m_pKillLabel->SetVisible(0);
                    }

                    if (m_TradeDesc[0])
                    {
                        m_pAutoTradeDesc->SetVisible(1);
                        m_pAutoTradePanel->SetVisible(1);
                    }
                    else
                    {
                        m_pAutoTradeDesc->SetVisible(0);
                        m_pAutoTradePanel->SetVisible(0);
                    }
                    if (g_pCurrentScene->m_pMyHuman != this)
                    {
                        if (m_nClass != 56 || m_stLookInfo.FaceMesh)
                        {
                            m_pProgressBar->SetVisible(1);
                           m_pProgressBar1->SetVisible(1);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pProgressBar1->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                        }
                    }

                    vPosInY = (int)((float)vPosInY + (RenderDevice::m_fHeightRatio * 13.0f));
                    float fWidthRatio = RenderDevice::m_fWidthRatio;
                    if (m_cMount)
                        vPosInY = vPosInY + (int)((RenderDevice::m_fHeightRatio * 2.0f) * 4.0f);
                    else
                        vPosInY = vPosInY - (int)((RenderDevice::m_fHeightRatio * 2.0f) * 16.0f);

                    if (1.0 == RenderDevice::m_fHeightRatio)
                    {
                        if (m_cMount)
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 8.0f);
                        else
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 10.0f);
                    }
                    else if (RenderDevice::m_fHeightRatio >= 1.7)
                    {
                        if (m_cMount)
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 15.0f);
                        else
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 6.0f);
                    }
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                    else
                    {
                        if (m_cMount)
                            m_pProgressBar->SetRealPos((float)vPosInX - 34.0f,
                                ((float)vPosInY - 10.0f) + 5.0f);
                        else
                            m_pProgressBar->SetRealPos((float)vPosInX - 34.0f,
                                (float)vPosInY - 25.0f);

                        if (m_cMount)
                            m_pProgressBar1->SetRealPos((float)vPosInX - 34.0f,
                                ((float)vPosInY - 10.0f) + 5.0f);
                        else
                            m_pProgressBar1->SetRealPos((float)vPosInX - 34.0f,
                                (float)vPosInY - 25.0f);

                        m_pMountHPBar->SetRealPos((float)vPosInX - 34.0f,
                            ((float)vPosInY - 30.0f));

                        m_pProgressBar->SetSize(72.0f, 7.0f);
                        m_pProgressBar1->SetSize(75.0f, 8.0f);
                        m_pMountHPBar->SetSize(72.0f, 7.0f);

                        m_pMountHPBar->Update();
                        m_pProgressBar->Update();
                        m_pProgressBar->m_GCProgress.nWidth = m_pProgressBar->m_nProgressWidth;
                        m_pProgressBar->m_GCProgress.nHeight = m_pProgressBar->m_nHeight - 4.0f;

                        m_pProgressBar1->Update();
                        m_pProgressBar1->m_GCProgress.nWidth = m_pProgressBar1->m_nProgressWidth;
                        m_pProgressBar1->m_GCProgress.nHeight = m_pProgressBar1->m_nHeight - 5.0f;

                        m_pMountHPBar->m_GCProgress.nWidth = m_pMountHPBar->m_nProgressWidth;
                        m_pMountHPBar->m_GCProgress.nHeight = m_pMountHPBar->m_nHeight - 4.0f;
                    }

                    int nLen2 = strlen(m_pNameLabel->GetText());
                    int nLen3 = strlen(m_TradeDesc);
                    int nLen4 = strlen(m_pNickNameLabel->GetText());

                    m_pChatMsg->SetRealPos((float)vPosInX - (float)(m_pChatMsg->m_nWidth / 2.0f), (float)vPosInY - 120.0f);
                    // Keep the alternate render path byte-for-byte equivalent to
                    // FUN_00504a80's separate title/background screen anchors.
                    m_pAutoTradeDesc->SetRealPos((float)vPosInX - ((140.0f * fWidthRatio) / 2.0f), (float)vPosInY);
                    m_pAutoTradePanel->SetRealPos((float)vPosInX - ((150.0f * fWidthRatio) / 2.0f),
                        (float)vPosInY - (float)(13.0f * RenderDevice::m_fHeightRatio));

                    float nPosY = (float)vPosInY;
                    if (m_cMount && !m_pAutoTradeDesc->IsVisible())
                        nPosY = nPosY - 20.0f;

                    fWidthRatio = RenderDevice::m_fWidthRatio;
                    m_pNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen2 + 2)) * RenderDevice::m_fWidthRatio) / 2.0f),
                        nPosY);

                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetRealPos(((float)vPosInX - (((float)(6 * (nLen2 + 2)) * fWidthRatio) / 2.0f)) - 10.0f,
                            nPosY + 2.0f);

                    m_pNickNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen4 + 2)) * fWidthRatio) / 2.0f),
                        nPosY - (float)(28.0f * fWidthRatio));

                    if ((int)m_vecPosition.x >> 7 > 1 && (int)m_vecPosition.x >> 7 < 11 && (int)m_vecPosition.y >> 7 < 5)
                    {
                        m_pProgressBar->SetVisible(1);
                        m_pProgressBar1->SetVisible(1);
                        m_pMountHPBar->SetVisible(0);
                        m_pTitleProgressBar->SetVisible(0);
                        m_pTitleNameLabel->SetVisible(0);
                    }

                    else if (m_nClass == 1 || m_nClass == 2 || m_nClass == 4 || m_nClass == 8 || m_nClass == 26 || m_nClass == 33 &&
                        !m_stLookInfo.FaceMesh || m_sHeadIndex == 271 && m_stScore.Merchant & 0xF)
                    {
                        if (_locationCheck(m_vecPosition, 14, 28) && m_sHeadIndex == 51)
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pProgressBar1->SetVisible(1);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pProgressBar1->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                            m_pTitleProgressBar->SetVisible(0);
                            m_pTitleNameLabel->SetVisible(0);
                        }
                    }

                    auto pFocused = g_pCurrentScene->m_pMyHuman;
                    if (pFocused)
                    {
                        if ((m_bParty == 1
                            || IsInPKZone() == 1
                            || pFocused->m_cMantua > 0 && m_cMantua > 0
                            || pFocused == this && m_cMount == 1) &&
                            !IsMerchant() && m_sHeadIndex != 57 && !m_pAutoTradeDesc->IsVisible())
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pProgressBar1->SetVisible(1);
                            if (g_pCurrentScene->m_pMyHuman != this && g_pCurrentScene->m_pMouseOverHuman == this)
                            {
                                if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                                    m_pProgressBar->SetVisible(0);
                                m_pProgressBar1->SetVisible(0);

                                m_pTitleNameLabel->SetVisible(1);
                                m_pTitleProgressBar->SetVisible(1);
                            }
                        }

                        if (pFocused == this && m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 && m_sMountIndex != 29 && m_sMountIndex != 30)
                            m_pMountHPBar->SetVisible(1);
                        else
                            m_pMountHPBar->SetVisible(0);

                        if (pFocused == this && m_cMount == 1)
                        {
                            int MountIndex = g_pObjectManager->m_stMobData.Equip[14].sIndex;
                            if (MountIndex >= 3980 && MountIndex <= 3999)
                                m_pMountHPBar->SetVisible(0);
                        }

                        m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                        m_pProgressBar1->m_GCProgress.dwColor = Blue;

                        if (g_bCastleWar)
                        {
                            if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pProgressBar1->m_GCProgress.dwColor = Blue;
                        }
                        else if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                        {
                            if (m_dwID < 0 || m_dwID >= 1000 && !TMFieldScene::m_bPK)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pProgressBar1->m_GCProgress.dwColor = Blue;
                        }
                        if (pFocused != this && m_citizen == pFocused->m_citizen && m_sHeadIndex < 40)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                            m_pProgressBar1->m_GCProgress.dwColor = Blue;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                        }
                        if (IsInCastleZone2() && !g_bCastleWar)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                            m_pProgressBar1->m_GCProgress.dwColor = Blue;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                        }
                        if (pFocused == this || m_bParty == 1 || m_usGuild &&
                            (m_usGuild == pFocused->m_usGuild || g_pObjectManager->m_usAllyGuild == m_usGuild))
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pProgressBar1->m_GCProgress.dwColor = Blue;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF006400;
                        }
                    }
                }
            }
        }
    }
    if (m_pAutoTradeDesc->IsVisible() == 1)
    {
        m_pKillLabel->SetVisible(0);
        m_stGuildMark.pGuildMark->SetVisible(0);
    }
    else if (!m_stGuildMark.bLoadedGuildmark)
    {
        if (!strlen(m_TradeDesc) && m_stGuildMark.sGuildIndex && m_stGuildMark.nGuild != -1 && !m_pAutoTradeDesc->IsVisible())
            pFScene->Guildmark_Create(&m_stGuildMark);
    }
    if (m_bIgnoreHeight == 1)
        m_pMountHPBar->SetVisible(0);

}

void TMHuman::HideLabel()
{
    m_pProgressBar->SetVisible(0);
    m_pProgressBar1->SetVisible(0);
    m_pMountHPBar->SetVisible(0);
    m_pNameLabel->SetVisible(0);
    if (m_stGuildMark.pGuildMark)
        m_stGuildMark.pGuildMark->SetVisible(0);

    m_pNickNameLabel->SetVisible(0);
    m_pAutoTradeDesc->SetVisible(0);
    m_pAutoTradePanel->SetVisible(0);
    m_pKillLabel->SetVisible(0);
    m_pChatMsg->SetVisible(0);
    m_pTitleProgressBar->SetVisible(0);
    m_pTitleNameLabel->SetVisible(0);
}

void TMHuman::SetChatMessage(const char* szString)
{
    if (!m_dwDelayDel)
    {
        int nHeight = 35;
        m_dwStartChatMsgTime = g_pTimerManager->GetServerTime();

        char temp[256]{ 0 };
        sprintf_s(temp, "%s", m_pNameLabel->GetText());

        GetChatLen(temp, &nHeight);
        sprintf_s(temp, "%s", szString);

        m_pChatMsg->SetText(temp, 0);

        float width = (float)GetChatLen(temp, &nHeight) * RenderDevice::m_fWidthRatio;
        m_pChatMsg->SetSize(width, (float)nHeight * RenderDevice::m_fHeightRatio);
    }
}

int TMHuman::GetChatLen(const char* szString, int* pHeight)
{
    if (m_dwDelayDel)
        return 0;

    int len = strlen(szString);
    int nLen = 0;
    if (len >= 41)
        nLen = 256;
    else
        nLen = 6 * len;

    if (len >= 41)
    {
        nLen = (int)((float)nLen * 1.0f);
        *pHeight = 50;
    }
    else
    {
        nLen = (int)((float)nLen * 1.0f) + 20;
        *pHeight = 40;
    }

	return nLen;
}

void TMHuman::SetInMiniMap(unsigned int dwCol)
{
    if (!m_pInMiniMap)
    {
        if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        {
            auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

            if (g_pCurrentScene->m_pMyHuman != this)
            {
                m_pInMiniMap = new SPanel(-2, 0.0f, 0.0f, 4.0f, 4.0f, dwCol, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

                if (m_pInMiniMap)
                {
                    m_pInMiniMap->m_bSelectEnable = 0;
                    pFScene->m_pMiniMapPanel->AddChild(m_pInMiniMap);
                }
            }
        }
    }
    else
        m_pInMiniMap->m_GCPanel.dwColor = dwCol;
}

void TMHuman::UpdateGuildName()
{
    if (!m_dwDelayDel)
    {
        if (!m_usGuild)
        {
            m_stGuildMark.bHideGuildmark = 1;
            m_stGuildMark.pGuildMark->SetVisible(0);
        }
        else
        {
            m_stGuildMark.nSubGuild = BASE_GetSubGuild(m_sGuildLevel);
            m_stGuildMark.nGuild = m_usGuild % 0xFFF;
            m_stGuildMark.nGuildChannel = (m_usGuild >> 12) & 0xF;
            m_stGuildMark.sGuildIndex = m_sGuildLevel;

            TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
            if (g_pCurrentScene)
            {
                if (!m_pAutoTradeDesc->IsVisible())
                    pFScene->Guildmark_Create(&m_stGuildMark);
            }

        }
    }
}

void TMHuman::SetGuildBattleHPColor()
{
    // It's an empty function... yeah!
}

void TMHuman::SetGuildBattleHPBar(int nHP)
{
    // It's an empty function... yeah!
}

void TMHuman::SetGuildBattleMPBar(int nMP)
{
    // It's an empty function... yeah!
}

void TMHuman::SetGuildBattleLifeCount()
{
    // It's an empty function... yeah!
}

void TMHuman::CreateControl()
{
    DestroyControl();

    m_pNameLabel = new SText(-1, "MoName", 0xFFFFFFAA, 0.0f, 650.0f, 128.0f, 16.0f, 0, 0x55AA0000u, 1u, 0);
    m_pKillLabel = new SText(-1, "", 0xFF1E821E, 0.0f, 650.0f, 128.0, 16.0f, 0, 0x55AA0000u, 1u, 0);
    m_pAutoTradeDesc = new SText(-1, "", 0xFFFFFFFF, 0.0f, 650.0f, 143.0f, 50.0f, 0, 0xFFFFFFFF, 1u, 0);
    // Recreated controls must preserve the native 7.48 shop sign asset and dimensions used by the primary constructor.
    m_pAutoTradePanel = new SPanel(446, -10.0f, 635.0f, 143.0f, 50.0f, 0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
    m_pAutoTradePanel->m_bSelectEnable = 0;

    m_pChatMsg = new SText(-2, "", 0xFFFFFFFF, 0.0, 650.0, (float)256, 64.0, 1, 0x77000000u, 1u, 0);
    m_pChatMsg->m_Font.m_bMultiLine = 1;

    m_stGuildMark.pGuildMark = nullptr;

    m_stGuildMark.pGuildMark = new SPanel(-2, 0.0, 0.0, 12.0, 16.0, 0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
    m_pProgressBar = new SProgressBar(-2, 30, 30, 0.0f, 0.0f, 60.0f, 7.0f, 0xFFFF0000, 0xFF333333, 1u);
    m_pProgressBar1 = new SProgressBar(-2, 30, 30, 0.0f, 0.0f, 60.0f, 7.0f, 0xFFFF0000, 0xFF333333, 1u);
    m_pMountHPBar = new SProgressBar(-2, 30, 30, 0.0f, 0.0f, 60.0f, 7.0f, 0xFFFFAA00, 0xFF333333, 1u);

    m_pChatMsg->SetVisible(0);
    m_pNameLabel->m_GCBorder.nTextureSetIndex = -2;
    m_pNameLabel->SetVisible(0);
    m_pKillLabel->SetVisible(0);

    if (m_stGuildMark.pGuildMark)
        m_stGuildMark.pGuildMark->SetVisible(0);

    m_pAutoTradeDesc->SetVisible(0);

    m_pNickNameLabel = new SText(-1, "", 0xFFFFFFAA, 0.0f, 650.0f, 128.0f, 16.0f, 0, 0x55AA0000u, 1u, 0);
    m_pNickNameLabel->m_GCBorder.nTextureSetIndex = -2;
    m_pNickNameLabel->SetVisible(0);
    m_pAutoTradePanel->SetVisible(0);
    m_pMountHPBar->SetVisible(0);

    if (g_pCurrentScene)
    {
        g_pCurrentScene->m_pControlContainer->AddItem(m_pNameLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pKillLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_stGuildMark.pGuildMark);
		// Recreated controls must keep FUN_004f7ea6's title-before-panel insertion
		// because reverse traversal renders the title above the dark background.
		g_pCurrentScene->m_pControlContainer->AddItem(m_pAutoTradeDesc);
		g_pCurrentScene->m_pControlContainer->AddItem(m_pAutoTradePanel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pNickNameLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pChatMsg);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pProgressBar);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pProgressBar1);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pMountHPBar);
    }
}

void TMHuman::DestroyControl()
{
    SAFE_DELETE(m_pChatMsg);
    SAFE_DELETE(m_pNameLabel);
    SAFE_DELETE(m_pKillLabel);
    SAFE_DELETE(m_stGuildMark.pGuildMark);
    SAFE_DELETE(m_pAutoTradeDesc);
    SAFE_DELETE(m_pAutoTradePanel);
    SAFE_DELETE(m_pNickNameLabel);
    SAFE_DELETE(m_pProgressBar);
    SAFE_DELETE(m_pProgressBar1);
    SAFE_DELETE(m_pMountHPBar);
    SAFE_DELETE(m_pInMiniMap);
    SAFE_DELETE(m_pSkinMesh);
    SAFE_DELETE(m_pMantua);
    SAFE_DELETE(m_pMount);
    SAFE_DELETE(m_pMount);
}

int TMHuman::StrByteCheck(const char* szString)
{
    int value = 0;
    bool byteCheck = false;

    int len = strlen(szString);
    for (int i = 0;i < len ; ++i)
    {
        if (szString[i] >= 'A' && szString[i] <= 'z')
            ++value;
        else if (byteCheck)
        {
            ++value;
            byteCheck = false;
        }
        else
            byteCheck = true;
    }

    return value;
}

bool TMHuman::_locationCheck(TMVector2 vec2, int mapX, int mapY)
{
    return mapY == (int)(vec2.y * 0.0078125f) && mapX == (int)(vec2.x * 0.0078125f);
}
