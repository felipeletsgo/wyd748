#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "SControl.h"
#include "TMScene.h"
#include "TMEffectBillBoard.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectMeshRotate.h"
#include "TMEffectSkinMesh.h"
#include "TMButterFly.h"
#include "ObjectManager.h"
#include "TMCamera.h"
#include "TMMesh.h"
#include "TMSky.h"
#include "TMObjectContainer.h"
#include "TMLight.h"
#include "ItemEffect.h"

int TMHuman::Render()
{
    if (m_dwDelayDel)
        return 0;
    if (m_cDeleted == 1)
        return 0;
    if (!m_pSkinMesh)
        return 0;

    if (!IsVisible() || m_cHide == 1 || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
    {
        for (int i = 0; i < 7; ++i)
        {
            if (m_pRotateBone[i])
                m_pRotateBone[i]->m_bVisible = 0;
        }
        for (int i = 0; i < 7; ++i)
        {
            if (m_pEyeFire[i])
                m_pEyeFire[i]->m_bVisible = 0;
        }
        for (int i = 0; i < 4; ++i)
        {
            if (m_pFly[i])
                m_pFly[i]->m_bVisible = 0;
        }
        for (int i = 0; i < 2; ++i)
        {
            if (m_pSoul[i])
            {
                m_pSoul[i]->m_bVisible = 0;
                m_pSoul[i]->m_pBillBoard->m_bShow = 0;
            }
        }

        HideLabel();
        return 1;
    }
    else if (m_nClass == 45)
    {
        return 1;
    }

    int nCommon = m_stLookInfo.LeftMesh;
    TMMesh* pMesh = g_pMeshManager->GetCommonMesh(nCommon, 0, 3_min);

    if (!pMesh || nCommon != 2888 && nCommon != 2889)
    {
        for (int j = 0; j < 7; ++j)
        {
            if (m_pRotateBone[j])
                m_pRotateBone[j]->m_bVisible = 1;
        }
        for (int j = 0; j < 7; ++j)
        {
            if (m_pEyeFire[j])
                m_pEyeFire[j]->m_bVisible = 1;
        }
        for (int j = 0; j < 4; ++j)
        {
            if (m_pFly[j])
                m_pFly[j]->m_bVisible = 1;
        }
        for (int j = 0; j < 2; ++j)
        {
            if (m_pSoul[j])
                m_pSoul[j]->m_bVisible = 1;
        }

        if (g_pDevice->m_dwBitCount == 32)
            g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xAAu);
        else
            g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFFu);

        if (m_cHide == 1 && (TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject != this
            || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this  && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        {
            return 1;
        }

        if (m_cHide == 1 || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 9u);
            m_pSkinMesh->m_cDefaultAlpha = 0;
        }
        else if (g_bHideSkillBuffEffect == 1 && m_cCoinArmor == 1)
        {
            D3DCOLORVALUE color{};
            color.r = 1.0f;
            color.g = 0.69f;
            color.b = 0.0;
            color.a = 1.0f;
            m_pSkinMesh->m_materials.Diffuse = color;

            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
            m_pSkinMesh->m_materials.Emissive.r = 1.0f;
            m_pSkinMesh->m_materials.Emissive.g = 0.69f;
            m_pSkinMesh->m_materials.Emissive.b = 0.0;
        }
        else
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4u);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6u);
            m_pSkinMesh->m_cDefaultAlpha = 1;
        }

        if (m_nClass == 32 && m_stLookInfo.FaceSkin == 2)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 8u);
        }
        else if (m_nClass == 59 || m_cShadow == 1)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
            g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
            g_pDevice->SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 2u);
        }
        else
        {
            g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 7u);
        }

        if (g_pCurrentScene && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && m_pSkinMesh && m_bPunchEffect == 1)
        {
            m_pSkinMesh->m_materials.Emissive.r = 0.69f;
            m_pSkinMesh->m_materials.Emissive.g = 0.69f;
            m_pSkinMesh->m_materials.Emissive.b = 0.89f;
        }

        if (!m_cMount || !m_pMount)
        {
            m_pSkinMesh->Render(0.0f, 1.0f, 0.0f);
            m_vecSkinPos = TMVector3(m_pSkinMesh->m_vPosition.x, m_pSkinMesh->m_vPosition.y, m_pSkinMesh->m_vPosition.z);
        }
        else
        {
            m_pMount->Render(0.0f, 1.0f, 0.0f);
            m_pSkinMesh->m_BaseMatrix = m_pMount->m_OutMatrix;

            TMVector3 vecPosScale = TMVector3(
                -0.37f * m_fMountScale,
                1.0f / m_fMountScale,
                0.2f * m_fMountScale);

            switch (m_nMountSkinMeshType)
            {
            case 25:
                vecPosScale.x = -0.34999999f * m_fMountScale;
                vecPosScale.y = 1.0f / m_fMountScale;
                vecPosScale.z = -0.1f * m_fMountScale;
                break;
            case 28:
                vecPosScale.x = -0.2f * m_fMountScale;
                vecPosScale.y = 1.0f / m_fMountScale;
                vecPosScale.z = 0.1f * m_fMountScale;
                break;

            case 29:
                if (m_pMount->m_Look.Mesh0 != 1 && m_pMount->m_Look.Mesh0 != 4)
                {
                    vecPosScale.x = -0.37f * m_fMountScale;
                    vecPosScale.y = 1.0f / m_fMountScale;
                    vecPosScale.z = -0.60000002f * m_fMountScale;
                }
                else
                {
                    vecPosScale.x = -0.1f * m_fMountScale;
                    vecPosScale.z = -0.60000002f * m_fMountScale;
                }
                break;

            case 20:
                if (m_pMount->m_Look.Mesh0 == 7)
                {
                    vecPosScale.x = 0.5f * m_fMountScale;
                    vecPosScale.y = 1.0f / m_fMountScale;
                    vecPosScale.z = -0.1f * m_fMountScale;
                }
                else
                {
                    vecPosScale.x = 0.25f * m_fMountScale;
                    vecPosScale.y = 1.0f / m_fMountScale;
                    vecPosScale.z = -0.1f * m_fMountScale;
                }
                break;
            default:
                break;
            }

            float fTemp = 0.0f;
            if (m_nClass == 40)
            {
                if (m_nMountSkinMeshType == 20 && m_stMountLook.Mesh0 != 7)
                {
                    vecPosScale.y = m_fScale / 1.3f;
                    vecPosScale.x = vecPosScale.x / 1.45f;
                }
                else if (m_nMountSkinMeshType == 20 && m_stMountLook.Mesh0 == 7)
                {
                    vecPosScale.y = m_fScale * 1.7f;
                }
                else
                {
                    vecPosScale.y = m_fScale;
                    vecPosScale.x = vecPosScale.x + 0.050000001f;
                }
            }

            if (m_nMountSkinMeshType == 29 && m_stMountLook.Mesh1 == 5)
            {
                vecPosScale.x = -0.2f;
                vecPosScale.y = 1.0f / m_fMountScale;
            }
            else if (m_nMountSkinMeshType == 48)
            {
                vecPosScale.x = vecPosScale.x - 0.80000001f;
                vecPosScale.z = vecPosScale.z + 0.2f;
            }

            else if (m_nMountSkinMeshType == 49 )
            {
                vecPosScale.x = vecPosScale.x - 0.2f;
                vecPosScale.z = vecPosScale.z - 0.2f;
            }
            else if (m_nMountSkinMeshType == 51)
            {
                vecPosScale.x = vecPosScale.x + 0.2f;
                vecPosScale.z = vecPosScale.z + 0.2f;
            }
            else if (m_nMountSkinMeshType == 59)//aranha
            {
                vecPosScale.x = vecPosScale.x + 0.9f;
                vecPosScale.z = vecPosScale.z - 0.8f;
            }
            else if (m_nMountSkinMeshType == 50)//balan�o
            {
                vecPosScale.x = vecPosScale.x - 0.5f;
                vecPosScale.z = vecPosScale.z - 0.3f;
            }
            else if (m_nMountSkinMeshType == 29)//tigre
            {
                vecPosScale.x = vecPosScale.x + 0.1f;
                vecPosScale.z = vecPosScale.z + 0.0f;
            }
            else if (m_nMountSkinMeshType == 20)
            {
                vecPosScale.x = vecPosScale.x - 0.1f;
                vecPosScale.z = vecPosScale.z - 0.5f;
            }
            m_pSkinMesh->Render(vecPosScale.x, vecPosScale.y, vecPosScale.z);
            D3DXVECTOR4 vecOut;
            D3DXVECTOR3 vecIn = D3DXVECTOR3(vecPosScale.z / m_fMountScale,
                vecPosScale.x / m_fMountScale,
                0.0f);

            D3DXVec3Transform(&vecOut, &vecIn, &m_pSkinMesh->m_BaseMatrix);
            m_vecSkinPos.x = vecOut.x;
            m_vecSkinPos.y = vecOut.y - (float)(0.40000001 * m_fScale);
            m_vecSkinPos.z = vecOut.z;

        }

        if (m_cHide == 1
            || m_cCoinArmor == 1
            || m_cShadow == 1
            && g_pCurrentScene->m_pMyHuman != this
            && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6u);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4u);
        }

        if (m_cMantua > 0 && m_pMantua)
        {
            float fScale = 1.0f;
            int nClass = 0;
            switch (m_nClass)
            {
            case 1:
                nClass = 0;
                break;
            case 2:
                nClass = 1;
                break;
            case 4:
                nClass = 2;
                break;
            case 8:
                nClass = 3;
                break;
            case 38:
                nClass = 1;
                break;
            case 25:
                nClass = 5;
                break;
            case 26:
                nClass = 6;
                break;
            case 33:
                nClass = 7;
                break;
            case 40:
                nClass = 8;
                break;
            default:
                nClass = 0;
                break;
            }

            if (m_sHeadIndex == 22 || m_sHeadIndex == 23)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.07f, 1.2f, -0.2f);
            }
            else if (m_sHeadIndex == 24)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.0f, 1.2f, 0.0f);
            }
            else if (m_sHeadIndex == 25)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.01f, 1.0f, 0.05f);
            }
            else if (m_sHeadIndex == 26)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.07f, 1.2f, 0.05f);
            }

            else
            {
                if (m_nSkinMeshType == 1)
                    fScale = 0.9f;

                int nMesh = m_stLookInfo.CoatMesh;
                float fLen = 0.0f;
                if (nMesh < 40)
                    fLen = fMantuaList[nClass][nMesh];
                else
                    fLen = fMantuaList[nClass][nMesh - 40]; // TODO: confirm this later

                float fLenUp = 0.0;
                if (m_sMantuaIndex >= 3197 && m_sMantuaIndex <= 3199)
                {
                    fLen = 0.11f;
                    fLenUp = 0.05f;
                }
                if (m_sMantuaIndex == 573 || m_sMantuaIndex == 1767 || m_sMantuaIndex == 1770)
                {
                    fLen = 0.15f;
                    fLenUp = 0.05f;
                }
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(fLen, fScale, fLenUp);
            }
        }
        RenderEffect();
        if (m_bVisible == 1)
            LabelPosition();
        return 1;
    }

    if (nCommon == 2888)
    {
        pMesh->m_fScaleH = 3.0f;
        pMesh->m_fScaleV = 3.0f;
    }
    if (nCommon == 2889)
    {
        pMesh->m_fScaleH = 1.0f;
        pMesh->m_fScaleV = 1.0f;
    }

    if ((int)m_vecPosition.x == 1224 && (int)m_vecPosition.y == 1975
     || (int)m_vecPosition.x == 1225 && (int)m_vecPosition.y == 1976)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, -0.08726646f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, -0.08726646f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1216 && (int)m_vecPosition.y == 1968
     || (int)m_vecPosition.x == 1217 && (int)m_vecPosition.y == 1969)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.2359877f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.2359877f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1207 && (int)m_vecPosition.y == 1988
        || (int)m_vecPosition.x == 1206 && (int)m_vecPosition.y == 1987)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.5850534f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.5850534f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1189 && (int)m_vecPosition.y == 1955
        || (int)m_vecPosition.x == 1188 && (int)m_vecPosition.y == 1954)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 4.8869219f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 4.8869219f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1240 && (int)m_vecPosition.y == 2006
        || (int)m_vecPosition.x == 1239 && (int)m_vecPosition.y == 2005)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1082 && (int)m_vecPosition.y == 2126
        || (int)m_vecPosition.x == 1081 && (int)m_vecPosition.y == 2125)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.9198622f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.9198622f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1074 && (int)m_vecPosition.y == 2119
        || (int)m_vecPosition.x == 1073 && (int)m_vecPosition.y == 2118)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.9670596f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.9670596f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1108 && (int)m_vecPosition.y == 2142
        || (int)m_vecPosition.x == 1107 && (int)m_vecPosition.y == 2141)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.7453293f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.7453293f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1095 && (int)m_vecPosition.y == 2107
        || (int)m_vecPosition.x == 1094 && (int)m_vecPosition.y == 2106)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.4434609f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.4434609f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1060 && (int)m_vecPosition.y == 2090
        || (int)m_vecPosition.x == 1059 && (int)m_vecPosition.y == 2089)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 3.3161256f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 3.3161256f, 0, 0, 0, 0);
    }
    else if (m_pTitleProgressBar->IsVisible())
    {
        pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
    }
    else
    {
        pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
    }

    LabelPosition2();
    return 1;
}

void TMHuman::SetColorMaterial()
{
    if (m_dwDelayDel)
        return;

    TMScene* pScene = g_pCurrentScene;
    if (!m_pSkinMesh)
        return;

    float fAlpha = 1.0f;

    if (pScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
    {
        TMHuman *pFocused = (TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject;

        D3DCOLORVALUE color = pScene->GroundGetColor(m_vecPosition);
        bool bLight = false;
        TMSky* pSky = g_pCurrentScene->m_pSky;

        if (pSky->m_nState && pSky->m_nState != 2 && pSky->m_nState != 12)
        {
            for (int i = 0; i < 2; ++i)
            {
                TMObjectContainer* pContainer = g_pCurrentScene->m_pObjectContainerList[i];
                if (pContainer == nullptr)
                    continue;

                int nLightIndex = pContainer->m_nLightIndex;
                for (int j = 0; j < nLightIndex; ++j)
                {
                    if (pContainer->m_pLightContainer[j]->m_bEnable != 1)
                        continue;

                    float fLen = pContainer->m_pLightContainer[j]->m_vecPosition.DistanceFrom(m_vecPosition);

                    if (fLen >= 5.0f)
                        continue;

                    color.r = (((5.0f - fLen) * pContainer->m_pLightContainer[j]->m_Light.Ambient.r)
                        / 3.0f)
                        + (g_pDevice->m_colorLight.r * 0.2f);
                    color.g = (((5.0f - fLen) * pContainer->m_pLightContainer[j]->m_Light.Ambient.g)
                        / 3.0f)
                        + (g_pDevice->m_colorLight.g * 0.2f);
                    color.b = (((5.0f - fLen) * pContainer->m_pLightContainer[j]->m_Light.Ambient.b)
                        / 3.0f)
                        + (g_pDevice->m_colorLight.b * 0.2f);
                    bLight = 1;
                    break;
                }
            }
        }

        if (!bLight)
        {
            color.r = (color.r * 0.5f) + (g_pDevice->m_colorLight.r * 0.5f);
            color.g = (color.g * 0.5f) + (g_pDevice->m_colorLight.g * 0.5f);
            color.b = (color.b * 0.5f) + (g_pDevice->m_colorLight.b * 0.5f);
            color.a = fAlpha;
        }

        m_pSkinMesh->m_materials.Specular.a = fAlpha;
        m_pSkinMesh->m_materials.Emissive.a = fAlpha;
        m_pSkinMesh->m_materials.Diffuse = color;
        m_pSkinMesh->m_materials.Specular.r = 1.0f;
        m_pSkinMesh->m_materials.Specular.g = 1.0f;
        m_pSkinMesh->m_materials.Specular.b = 1.0f;

        if (g_pCurrentScene->m_pMouseOverHuman == this && pFocused != this)
        {
            if ((m_dwID >= 0 && m_dwID < 1000) || (m_stScore.Merchant & 0xF) != 15)
            {
                if (m_dwID < 0 || m_dwID >= 1000 && IsMerchant()) // NPC outline color
                    m_dwEdgeColor = 0x8800FF00;
                else if (m_TradeDesc[0])
                    m_dwEdgeColor = 0x8800FF00;
                else if (m_bParty == 1)
                    m_dwEdgeColor = 0x88FF00FF;
                else if (m_usGuild && (m_usGuild == pFocused->m_usGuild || g_pObjectManager->m_usAllyGuild == m_usGuild))
                    m_dwEdgeColor = 0x8800FFFF;
                else
                {
                    int bTown = IsInTown(); // Character outline color
                    if (bTown || m_pProgressBar->m_GCProgress.dwColor != 0xFFFF0000)
                        m_dwEdgeColor = 0x88FFFFFF;
                    else
                        m_dwEdgeColor = 0x88FF0000;
                }
            }
            else
            {
                m_dwEdgeColor = 0x8800FF00;
            }

            if ((m_stScore.Merchant & 0xF) == 15 && pFocused->m_cMantua && pFocused->m_cMantua != 3 && m_pMantua && pFocused->m_pMantua
                && m_cMantua != pFocused->m_cMantua)
            {
                m_dwEdgeColor = 0x88FF0000;
            }
        }
        else
        {
            if (!g_bHideSkillBuffEffect && m_cHaste == 1)
            {
                unsigned int dwServerTime = g_pTimerManager->GetServerTime();
                if (m_eMotion == ECHAR_MOTION::ECMOTION_RUN && dwServerTime - m_dwLastHaste > (unsigned int)(600.0f / m_fMaxSpeed))
                {
                    TMEffectBillBoard2* pBill2 = new TMEffectBillBoard2(43, 1000, 0.5f, 0.5f, 0.5f, 0.00050000002f, 0);

                    if (pBill2)
                    {
                        pBill2->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight + 0.2f, m_vecPosition.y);
                        pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                        m_dwLastHaste = dwServerTime;
                    }
                }
            }
            if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
            {
                color.a = fAlpha;
                color.r = 0.6f;
                color.g = 0.4f;
                color.b = 0.4f;
                m_pSkinMesh->m_materials.Emissive.r = (g_pDevice->m_colorBackLight.r * 0.4f)
                    + (0.6f * 0.5f);
                m_pSkinMesh->m_materials.Emissive.g = (g_pDevice->m_colorBackLight.g * 0.4f)
                    + (color.r * 0.5f);
                m_pSkinMesh->m_materials.Emissive.b = (g_pDevice->m_colorBackLight.b * 0.4f)
                    + (color.r * 0.5f);
                return;
            }
            if (m_cCancel == 1)
            {
                color.a = fAlpha;
                color.r = (color.r * 0.4f) + 0.2f;
                color.g = color.g * 0.4f;
                color.b = color.b * 0.4f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else if (m_cFreeze == 1)
            {
                color.a = fAlpha;
                color.r = 0.0f;
                color.g = 0.4f;
                color.b = 0.9f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else if (m_cPoison == 1)
            {
                color.a = fAlpha;
                color.r = 0.0f;
                color.g = 0.3f;
                color.b = 0.0f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;

                color.g = 0.8f;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else if(m_cSlowSlash == 1)
            {
                float fVel = (float)((g_pTimerManager->GetServerTime() + (300 * m_dwID)) % 1000) / 1000.0f;
                float fR = (sinf((fVel * 3.1415927f) * 2.0f) * 0.5f) + 0.5f;
                color.a = fAlpha;
                color.r = fR / 2.0f;
                color.g = fR / 2.0f;
                color.b = 0.0f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else
            {
                m_pSkinMesh->m_materials.Emissive.r = (g_pDevice->m_colorBackLight.r * 0.3f) + (color.r * 0.4f);
                m_pSkinMesh->m_materials.Emissive.g = (g_pDevice->m_colorBackLight.g * 0.3f) + (color.r * 0.4f);
                m_pSkinMesh->m_materials.Emissive.b = (g_pDevice->m_colorBackLight.b * 0.3f) + (color.r * 0.4f);

                if (m_nClass == 32 && m_stLookInfo.FaceMesh == 2)
                {
                    m_pSkinMesh->m_materials.Emissive.r = 0.65f;
                    m_pSkinMesh->m_materials.Emissive.g = 0.0;
                    m_pSkinMesh->m_materials.Emissive.b = 0.0;
                }
            }
        }
    }
    else
    {
        if (m_bSelected)
        {
            m_pSkinMesh->m_materials.Diffuse.r = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.g = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.b = 1.0f;
            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
            m_pSkinMesh->m_materials.Emissive.r = 1.0f;
            m_pSkinMesh->m_materials.Emissive.g = 1.0f;
            m_pSkinMesh->m_materials.Emissive.b = 1.0f;
        }
        else if (m_bMouseOver == 1)
        {
            m_pSkinMesh->m_materials.Diffuse.r = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.g = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.b = 1.0f;
            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
            m_pSkinMesh->m_materials.Emissive.r = 0.3f;
            m_pSkinMesh->m_materials.Emissive.g = 0.5f;
            m_pSkinMesh->m_materials.Emissive.b = 0.0;
        }
        else
        {
            D3DCOLORVALUE color{};
            color.r = (g_pDevice->m_colorLight.r * 0.30000001f) + 0.2f;
            color.g = (g_pDevice->m_colorLight.g * 0.30000001f) + 0.2f;
            color.b = (g_pDevice->m_colorLight.b * 0.30000001f) + 0.2f;

            m_pSkinMesh->m_materials.Diffuse = color;
            m_pSkinMesh->m_materials.Emissive = color;
            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
        }
    }

    if (m_cMantua > 0 && m_pMantua)
        memcpy(&m_pMantua->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMantua->m_materials));
    if (m_cMount > 0 && m_pMount)
        memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
}
