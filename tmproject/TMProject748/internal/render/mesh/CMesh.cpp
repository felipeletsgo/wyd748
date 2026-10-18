#include "pch.h"
#include "TMMesh.h"
#include "TMSkinMesh.h"
#include "CMesh.h"
#include "TMGlobal.h"
#include "TMEffectSWSwing.h"
#include "TMHuman.h"
#include "TMCamera.h"
#include "CFrame.h"
#include "DyeTextureStages.h"
#include "RefinementGlowTexture.h"
#include "NativeMountGlow.h"
#include "SkinMeshLoad.h"
#include <memory>
#include <new>

CMesh::CMesh(TMSkinMesh* pParentSkin)
{
	m_pMesh = nullptr;
    m_pBoneMatrix = nullptr;
    m_dwNames = nullptr;
    m_dwID = 0;
    m_numFaceInflunce = 0;
    m_bMeshGenerated = 0;
    memset(m_pBoneOffset, 0, sizeof(m_pBoneOffset));
    m_pParentSkin = pParentSkin;
    m_nSkinMeshIndex = -1;
    m_sLegendType = 0;
    m_sMultiType = 0;
    m_nTextureIndex = 0;
    m_numPalette = 0;
    m_numFaces = 0;
    m_bHead = 0;
    m_bMount = 0;
    m_god2cos = 0;
    m_bSheild = 0;
    m_god2Alpha = 255;
    m_god2Red = 50;
    m_god2Green = 50;
    m_god2Brue = 50;
}

CMesh::~CMesh()
{
}

int CMesh::Render(int nBright)
{
    if (!m_pParentSkin)
        return 0;
    if (m_numPalette > sizeof(m_pBoneOffset) / sizeof(m_pBoneOffset[0]) ||
        (m_numPalette && !m_pBoneMatrix)) return 0;
    for (std::size_t i = 0; i < m_numPalette; ++i)
        if (!m_pBoneOffset[i]) return 0;

    LPDIRECT3DTEXTURE9 pTex = g_pTextureManager->GetModelTexture(m_nTextureIndex, 10000);
 
    char cAlpha = 'A';
    if (m_nTextureIndex >= 0)
        cAlpha = g_pTextureManager->m_stModelTextureList[m_nTextureIndex].cAlpha;

    g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, 1);

    if (cAlpha == 'C')
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, 0);

    g_pDevice->SetTexture(0, pTex);

    if (!nBright)
        SetMaterial(cAlpha);

    if (!m_pParentSkin->m_pOwner
        || m_pParentSkin->m_nBoneAniIndex
        && m_pParentSkin->m_nBoneAniIndex != 1
        && m_pParentSkin->m_nBoneAniIndex != 2
        && m_pParentSkin->m_nBoneAniIndex != 4
        && m_pParentSkin->m_nBoneAniIndex != 6
        && m_pParentSkin->m_nBoneAniIndex != 11
        && m_pParentSkin->m_nBoneAniIndex != 9
        && m_pParentSkin->m_nBoneAniIndex != 12
        && m_pParentSkin->m_nBoneAniIndex != 10
        && m_pParentSkin->m_nBoneAniIndex != 5
        && m_pParentSkin->m_nBoneAniIndex != 45
        && m_pParentSkin->m_nBoneAniIndex != 46
        && m_pParentSkin->m_nBoneAniIndex != 47
        || m_dwID != g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][0]
        && m_dwID != g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][1])
    {
        return RenderMesh(cAlpha);
    }

    int nPartIndex = 0;
    int checkpos = 0;

    if (m_dwID == g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][0])
    {
        if (m_pParentSkin->m_Look.Mesh6 <= 0)
            nPartIndex = -1;
        else
            nPartIndex = m_pParentSkin->m_Look.Mesh6;
        checkpos = 1;
    }
    else if (m_dwID == g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][1])
    {
        if (m_pParentSkin->m_Look.Mesh7 <= 0)
            nPartIndex = -1;
        else
            nPartIndex = m_pParentSkin->m_Look.Mesh7;
    }
    if (nPartIndex >= 2889 && nPartIndex <= 2901 || nPartIndex >= 2906 && nPartIndex <= 2909)
    {
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, 0);

        TMMesh* pMesh = g_pMeshManager->GetCommonMesh(nPartIndex, 0, 20_min);
        if (g_pDevice->m_pd3dDevice->SetStreamSource(0, pMesh->m_pVB, 0, pMesh->m_sizeVertex) < 0)
            return 1;
        if (g_pDevice->m_pd3dDevice->SetIndices(pMesh->m_pIB) < 0)
            return 1;
        if (g_pDevice->m_pd3dDevice->SetFVF(pMesh->m_dwFVF) < 0)
            return 1;

        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FOGVERTEXMODE, D3DFOGMODE::D3DFOG_LINEAR);
        g_pDevice->SetTexture(0, g_pTextureManager->GetModelTexture(pMesh->m_nTextureIndex[0], 10000));

        int nBaseIndex = refinement_glow::TextureBase(m_sLegendType);
        if (nPartIndex >= 2906 && nPartIndex <= 2909)
            nBaseIndex = 260;

        if (m_sMultiType > 12)
            m_sMultiType = 12;

        g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nBaseIndex + m_sMultiType, 10000));

        cAlpha = g_pTextureManager->m_stModelTextureList[pMesh->m_nTextureIndex[0]].cAlpha;
        if (cAlpha != 'C')
        {
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG1, D3DTEXTUREOP::D3DTOP_DISABLE);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, D3DTEXTUREOP::D3DTOP_SELECTARG1);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATEALPHA_ADDCOLOR);
            if (m_sMultiType > 9)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE4X);
            }
            else if (m_sMultiType <= 6)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
            }
            else
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE2X);
            }

            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADDSMOOTH);
        }

        unsigned int dwTime = g_pTimerManager->GetServerTime();
        float fProgress = (float)(dwTime % 10000) / 10000.0f;
        RDVERTEX2* pVertex = nullptr;

        if (pMesh->m_pVB->Lock(0, 0, (void**)&pVertex, 0) < 0)
            return 1;

        for (size_t k = 0; k < pMesh->m_dwAttCount; ++k)
        {
            int nStartIndex = pMesh->m_AttRange[k].VertexStart;
            int nCount = pMesh->m_AttRange[k].VertexCount;
            for (int j = nStartIndex; j < nCount + nStartIndex; ++j)
            {
                pVertex[j].tu2 = pVertex[j].tu1 + fProgress;
                pVertex[j].tv2 = pVertex[j].tv1 + fProgress;
            }
        }

        if (pMesh->m_pVB->Unlock() < 0)
            return 1;

        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_NONE);

        if (g_pDevice->m_pd3dDevice->DrawIndexedPrimitive(
            D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST,
            0,
            pMesh->m_AttRange[0].VertexStart,
            pMesh->m_AttRange[0].VertexCount,
            pMesh->m_AttRange[0].FaceStart,
            pMesh->m_AttRange[0].FaceCount) < 0)
            return 0;

        g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_CCW);
        return 1;
    }
    if (nPartIndex >= 2912 && nPartIndex <= 2923)
    {
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, 0);
        TMMesh* pMesh = g_pMeshManager->GetCommonMesh(nPartIndex, 0, 20_min);
        if (g_pDevice->m_pd3dDevice->SetStreamSource(0, pMesh->m_pVB, 0, pMesh->m_sizeVertex) < 0)
            return 1;
        if (g_pDevice->m_pd3dDevice->SetIndices(pMesh->m_pIB) < 0)
            return 1;
        if (g_pDevice->m_pd3dDevice->SetFVF(pMesh->m_dwFVF) < 0)
            return 1;

        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FOGVERTEXMODE, D3DFOGMODE::D3DFOG_LINEAR);
        g_pDevice->SetTexture(0, g_pTextureManager->GetModelTexture(pMesh->m_nTextureIndex[0], 10000));

        if (m_sMultiType > 12)
            m_sMultiType = 12;

        g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(m_sMultiType + 440, 10000));

        cAlpha = g_pTextureManager->m_stModelTextureList[pMesh->m_nTextureIndex[0]].cAlpha;
        if (cAlpha == 'C' && !g_pDevice->m_bVoodoo && !g_pDevice->m_bIntel && !g_pDevice->m_bG400)
        {
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG1, D3DTEXTUREOP::D3DTOP_DISABLE);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, D3DTEXTUREOP::D3DTOP_SELECTARG1);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATEALPHA_ADDCOLOR);        
        }
        else
        {
            g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_SELECTARG1);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_DISABLE);
            g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAARG1, D3DTEXTUREOP::D3DTOP_SELECTARG1);
        }
        g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 5u);

        unsigned int dwTime = g_pTimerManager->GetServerTime();
        float fProgress = (float)(dwTime % 10000) / 10000.0f;
        RDVERTEX2* pVertex = nullptr;

        if (pMesh->m_pVB->Lock(0, 0, (void**)&pVertex, 0) < 0)
            return 1;

        for (size_t k = 0; k < pMesh->m_dwAttCount; ++k)
        {
            int nStartIndex = pMesh->m_AttRange[k].VertexStart;
            int nCount = pMesh->m_AttRange[k].VertexCount;
            for (int j = nStartIndex; j < nCount + nStartIndex; ++j)
            {
                pVertex[j].tu2 = pVertex[j].tu1 + fProgress;
                pVertex[j].tv2 = pVertex[j].tv1 + fProgress;
            }
        }

        if (pMesh->m_pVB->Unlock() < 0)
            return 1;

        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_NONE);

        if (g_pDevice->m_pd3dDevice->DrawIndexedPrimitive(
            D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST,
            0,
            pMesh->m_AttRange[0].VertexStart,
            pMesh->m_AttRange[0].VertexCount,
            pMesh->m_AttRange[0].FaceStart,
            pMesh->m_AttRange[0].FaceCount) < 0)
            return 0;

        g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_CCW);
        return 1;
    }
    else
    {
        return RenderMesh(cAlpha);
    }

    return 0;
}

int CMesh::RenderMesh(char cAlpha)
{
    int dyePaletteIndex = -1;
    if (m_sMultiType > 0 && m_pParentSkin->m_cEnableMultiTex == 1
        || m_sLegendType >= 4 && m_sLegendType <= 8)
    {
        g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_TEXCOORDINDEX, D3DTEXTUREOP::D3DTOP_DISABLE);
        if (m_sLegendType == 4)
        {
            if (m_sMultiType > 12)
                m_sMultiType = 12;

            g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(179 + m_sMultiType, 10000));
            if (cAlpha != 'C' || g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_SELECTARG1);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_DISABLE);
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAARG1, D3DTEXTUREOP::D3DTOP_SELECTARG1);
            }
            if (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
            }
            else
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE2X);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
            }
        }
        else if (m_sLegendType > 4 && m_sLegendType <= 8)
        {
            int nBaseIndex = 153;
            if (m_sLegendType == 6)
                nBaseIndex = 166;
            if (m_sLegendType == 7)
                nBaseIndex = 246;
            if (m_sLegendType == 8)
                nBaseIndex = 260;
            if (m_sMultiType > 12)
                m_sMultiType = 12;

            g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nBaseIndex + m_sMultiType, 10000));

            if (cAlpha != 'C' || g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_SELECTARG1);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_DISABLE);
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAARG1, D3DTEXTUREOP::D3DTOP_SELECTARG1);
            }
            else
            {
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG1, D3DTEXTUREOP::D3DTOP_DISABLE);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, D3DTEXTUREOP::D3DTOP_SELECTARG1);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATEALPHA_ADDCOLOR);
            }
            if (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADD);
            }
            else
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
                if(cAlpha != 'C')
                    g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADDSMOOTH);
            }
        }
        else if (m_sLegendType >= 116 && m_sLegendType <= 125 && m_sMultiType > 0)
        {
            int nBaseIndex = 0;
            switch (m_sLegendType - 116)
            {
            case 0:
                nBaseIndex = 275;
                break;
            case 1:
                nBaseIndex = 288;
                break;
            case 2:
                nBaseIndex = 301;
                break;
            case 3:
                nBaseIndex = 314;
                break;
            case 4:
                nBaseIndex = 327;
                break;
            case 5:
                nBaseIndex = 340;
                break;
            case 6:
                nBaseIndex = 353;
                break;
            case 7:
                nBaseIndex = 366;
                break;
            case 8:
                nBaseIndex = 425;
                break;
            case 9:
                nBaseIndex = 392;
                break;
            }

            if (m_nTextureIndex < 0)
                m_bSheild = 1;

            dyePaletteIndex = nBaseIndex;
            // Texture families stop at +12; retain the real refinement for
            // the shader, including later frames and grades +13 through +15.
            const int dyeTextureLevel = m_sMultiType > 12 ? 12 : m_sMultiType;
            if (m_nTextureIndex >= 0 || m_bSheild)
            {
                g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, 0);

                g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nBaseIndex + dyeTextureLevel - 1, 10000));

                dye_texture_stages::Apply(*g_pDevice, m_sLegendType, cAlpha,
                    g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400,
                    g_pDevice->m_bTNT != 0);
            }
            else
            {
                g_pDevice->SetTexture(2, g_pTextureManager->GetEffectTexture(nBaseIndex + dyeTextureLevel - 1, 10000));

                if (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
                {
                    g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
                    g_pDevice->SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
                    g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
                    g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4u);
                    g_pDevice->SetTextureStageState(1u, D3DTSS_COLOROP, 7u);
                }
                else
                {
                    g_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
                    g_pDevice->SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 0);
                    g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 2u);
                    g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
                    g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
                    g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
                    g_pDevice->SetTextureStageState(1u, D3DTSS_COLOROP, 8u);
                    g_pDevice->SetTextureStageState(1u, D3DTSS_COLORARG1, 2u);
                    g_pDevice->SetTextureStageState(1u, D3DTSS_COLORARG2, 1u);
                    g_pDevice->SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
                }
            }
        }
        else if (m_sLegendType >= 9 && m_sLegendType <= 12)
        {
            const int nBaseIndex = refinement_glow::TextureBase(m_sLegendType);

            if (m_sMultiType > 12)
                m_sMultiType = 12;

            g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nBaseIndex + m_sMultiType, 10000));
            if (cAlpha == 'C' && !g_pDevice->m_bVoodoo && !g_pDevice->m_bIntel && !g_pDevice->m_bG400)
            {
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG1, D3DTEXTUREOP::D3DTOP_DISABLE);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, D3DTEXTUREOP::D3DTOP_SELECTARG1);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATEALPHA_ADDCOLOR);
            }
            else
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_SELECTARG1);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_DISABLE);
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAARG1, D3DTEXTUREOP::D3DTOP_SELECTARG1);
            }
            if (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADD);
            }
            else if (m_sMultiType < 7)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
                if (cAlpha != 'C')
                    g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADDSMOOTH);
            }
            else
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE2X);  
                if (cAlpha != 'C')
                    g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADDSMOOTH);
            }
        }
        else if (!g_pDevice->m_bVoodoo && !g_pDevice->m_bIntel && !g_pDevice->m_bG400)
        {
            if (m_bMount)
            {
                g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(native_mount_glow::TextureIndex(m_sMultiType), 10000));
            }
            else
            {
                g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(m_sMultiType >= 13 ? 456 : m_sMultiType + 233, 10000));
            }

            if (cAlpha == 'C')
            {
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG1, D3DTEXTUREOP::D3DTOP_DISABLE);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, D3DTEXTUREOP::D3DTOP_SELECTARG1);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATEALPHA_ADDCOLOR);
            }
            else
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_SELECTARG1);
                g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, D3DTEXTUREOP::D3DTOP_DISABLE);
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAARG1, D3DTEXTUREOP::D3DTOP_SELECTARG1);
            }

            if (m_sMultiType < 7)
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
                if (cAlpha != 'C')
                    g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADDSMOOTH);
            }
            else
            {
                g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE2X);
                if (cAlpha != 'C')
                    g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADDSMOOTH);
            }

            g_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, 0);
        }
        else
        {
            g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(m_sMultiType + 233, 10000));
            g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_ADD);
        }
    }
    else
    {
        g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
        g_pDevice->SetTexture(1, nullptr);
    }

    if (m_pParentSkin->m_pOwner
        && (!m_pParentSkin->m_nBoneAniIndex
            || m_pParentSkin->m_nBoneAniIndex == 1
            || m_pParentSkin->m_nBoneAniIndex == 2
            || m_pParentSkin->m_nBoneAniIndex == 4
            || m_pParentSkin->m_nBoneAniIndex == 6
            || m_pParentSkin->m_nBoneAniIndex == 11
            || m_pParentSkin->m_nBoneAniIndex == 9
            || m_pParentSkin->m_nBoneAniIndex == 12
            || m_pParentSkin->m_nBoneAniIndex == 10
            || m_pParentSkin->m_nBoneAniIndex == 5
            || m_pParentSkin->m_nBoneAniIndex == 45
            || m_pParentSkin->m_nBoneAniIndex == 46)
            && (m_dwID == g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][0]
            || m_dwID == g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][1]))
    {
        int nPartIndex = 0;

        if (m_dwID == g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][0])
        {
            if (m_pParentSkin->m_Look.Mesh6 <= 0)
                nPartIndex = -1;
            else
                nPartIndex = m_pParentSkin->m_Look.Mesh6;
        }
        else if (m_dwID == g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][1])
        {
            if (m_pParentSkin->m_Look.Mesh7 <= 0)
                nPartIndex = -1;
            else
                nPartIndex = m_pParentSkin->m_Look.Mesh7;
        }

        if (nPartIndex >= 13 && nPartIndex < 1000 || nPartIndex > 2700 && nPartIndex <= 3000)
        {
            int bMulti = 0;
            if (m_sMultiType > 0)
                bMulti = 1;

            if (cAlpha == 'C')
                g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, 1);

            g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, 2);

            TMMesh* pMesh = g_pMeshManager->GetCommonMesh(nPartIndex, 0, 20_min);
            if (pMesh)
               pMesh->Render(bMulti, 0);

            g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, 3);
        }
        if (m_sMultiType > 0)
        {
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_TEXCOORDINDEX, 1);
            g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 1);
        }
        return 1;
    }
    else if (m_pMesh)
    {
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FOGVERTEXMODE, D3DFOGMODE::D3DFOG_NONE);
        if (g_pDevice->m_bUseSW)
            g_pDevice->m_pd3dDevice->SetSoftwareVertexProcessing(1);

        if (g_pDevice->m_pd3dDevice->SetStreamSource(0, m_pMesh->m_pVB, 0, m_pMesh->m_sizeVertex) < 0)
            return 0;
        if (g_pDevice->m_pd3dDevice->SetIndices(m_pMesh->m_pIB) < 0)
            return 0;        
        if (m_numFaceInflunce > 4 || m_numFaceInflunce < 1)
            return 0;
        if (m_pParentSkin->m_nBoneAniIndex == 61)
        {
            if (g_pDevice->m_pd3dDevice->SetVertexDeclaration(g_pDevice->m_pVertexDeclaration[m_numFaceInflunce + 3]) < 0)
                return 0;
            if (g_pDevice->m_pd3dDevice->SetVertexShader(g_pDevice->m_pVertexShader[m_numFaceInflunce + 3]) < 0)
                return 0;
        }
        else
        {
            if (g_pDevice->m_pd3dDevice->SetVertexDeclaration(g_pDevice->m_pVertexDeclaration[m_numFaceInflunce - 1]) < 0)
                return 0;
            if (g_pDevice->m_pd3dDevice->SetVertexShader(g_pDevice->m_pVertexShader[m_numFaceInflunce - 1]) < 0)
                return 0;
        }

        for (size_t i = 0; i < m_numPalette; ++i)
        {
            D3DXMATRIXA16 mat;
            D3DXMatrixMultiply(&mat, &m_pBoneMatrix[i], m_pBoneOffset[i]);
            D3DXMatrixMultiplyTranspose(&mat, &mat, &g_pDevice->m_matView);
            g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(3 * i + 9, (const float*)&mat, 3);
        }

        D3DXVECTOR3 vecDir(-1.0f, 1.0f, 1.0f);
        D3DXVec3Normalize(&vecDir, &vecDir);
        g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(1, (const float*)&vecDir, 1);

        D3DXMATRIXA16 matViewInv;
        D3DXMatrixInverse(&matViewInv, 0, &g_pDevice->m_matView);
        D3DXMatrixTranspose(&matViewInv, &matViewInv);
        g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(92, (const float*)&matViewInv, 4);

        if (m_pParentSkin && m_pParentSkin->m_nMeshType == 1)
        {
            g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_CW);
        }

        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_NONE);
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_NONE);

        {
            dye_pixel_shader::Binding dye(g_pDevice->m_pd3dDevice, g_pDevice->m_dyePixelShader,
                m_sLegendType, m_sMultiType, dyePaletteIndex >= 0
                    && m_sMultiType > 0 && m_pParentSkin->m_cEnableMultiTex == 1);
            // Keep the icon's grade-specific effect as the moving highlight,
            // but sample the unrefined palette separately for the armor color.
            // Yellow's native family is solid at every grade, so use the
            // silver family's moving pattern without changing its yellow hue.
            if (dye.Active())
            {
                const int maskIndex = m_sLegendType == 124 ? 314 : dyePaletteIndex;
                const int grade = m_sMultiType > 12 ? 12 : m_sMultiType;
                dye_pixel_shader::TextureBinding textures(g_pDevice->m_pd3dDevice,
                    g_pTextureManager->GetEffectTexture(maskIndex, 10000),
                    g_pTextureManager->GetEffectTexture(maskIndex + grade - 1, 10000),
                    g_pTextureManager->GetEffectTexture(dyePaletteIndex, 10000));
                if (!textures.Active()) return 0;
                if (g_pDevice->m_pd3dDevice->DrawIndexedPrimitive(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST,
                    0, 0, m_pMesh->m_AttRange[0].VertexCount, 0, m_numFaces) < 0)
                    return 0;
            }
            else if (g_pDevice->m_pd3dDevice->DrawIndexedPrimitive(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST,
                0, 0, m_pMesh->m_AttRange[0].VertexCount, 0, m_numFaces) < 0)
                return 0;
        }

        g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_DISABLE);
        g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_DISABLE);
        g_pDevice->SetTextureStageState(2, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_DISABLE);
        g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL::D3DCULL_CCW);

        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD
            && !g_pCurrentScene->m_pMouseOverHuman
            && !g_pObjectManager->m_bTvControl)
        {
            memset(g_pCurrentScene->m_TargetAffect, 0, sizeof(g_pCurrentScene->m_TargetAffect));
        }

        if(g_pCurrentScene->m_pMouseOverHuman == (TMHuman*)m_pParentSkin->m_pOwner
            && g_pCurrentScene->m_pMouseOverHuman
            && !g_pObjectManager->m_bTvControl
            && (g_pCurrentScene->m_pMouseOverHuman != (TMHuman*)m_pParentSkin->m_pOwner
                || g_pCurrentScene->m_pMouseOverHuman->m_cShadow != 1))
        {
            D3DXMATRIX matScale;
            D3DXMATRIX matMove;
            TMVector3 vecCamTarget = g_pObjectManager->m_pCamera->m_cameraPos;

            TMVector3 vecTargetPos;
            vecTargetPos.x = m_pBoneOffset[0]->_41;
            vecTargetPos.y = m_pBoneOffset[0]->_42;
            vecTargetPos.z = m_pBoneOffset[0]->_43;

            TMVector3 resultVec = vecTargetPos - vecCamTarget;

            float fDist = vecTargetPos.Length();
            fDist = fDist * 0.00002f;
            D3DXVec3Normalize((D3DXVECTOR3*)&resultVec, (D3DXVECTOR3*)&resultVec);
            D3DXMatrixTranslation(&matMove, resultVec.x * 0.1f, resultVec.y * 0.1f, resultVec.z * 0.1f);
            if (m_pParentSkin->m_nBoneAniIndex == 41 && !m_pParentSkin->m_Look.Mesh0)
                D3DXMatrixScaling(&matScale, fDist + 0.94f, fDist + 0.94f, fDist + 0.94f);
            else
                D3DXMatrixScaling(&matScale, fDist + 1.0f, fDist + 1.0f, fDist + 1.0f);

            for (size_t l = 0; l < m_numPalette; ++l)
            {
                D3DXMATRIX* fy = m_pBoneOffset[l];
                D3DXMATRIX rMatrix = (((m_pBoneMatrix[l] * matScale) * *fy) * matMove) * g_pDevice->m_matView;
                D3DXMatrixTranspose(&rMatrix, &rMatrix);
                g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(
                    3 * l + 9,
                    (const float*)&rMatrix,
                    3);
            }

            if (g_pCurrentScene->m_pMouseOverHuman == (TMHuman*)m_pParentSkin->m_pOwner)
            {
                float fvalue = 0.5f;
                m_pParentSkin->m_materials.Diffuse.r = fvalue;
                m_pParentSkin->m_materials.Diffuse.g = fvalue;
                m_pParentSkin->m_materials.Diffuse.b = fvalue;
                m_pParentSkin->m_materials.Specular.r = fvalue;
                m_pParentSkin->m_materials.Specular.g = fvalue;
                m_pParentSkin->m_materials.Specular.b = fvalue;
                m_pParentSkin->m_materials.Emissive.r = fvalue;
                m_pParentSkin->m_materials.Emissive.g = fvalue;
                m_pParentSkin->m_materials.Emissive.b = fvalue;

                if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
                {
                    if (g_pCurrentScene->m_pMouseOverHuman)
                        memcpy(g_pCurrentScene->m_TargetAffect, g_pCurrentScene->m_pMouseOverHuman->m_usAffect, 
                            sizeof(g_pCurrentScene->m_TargetAffect));
                }
            }

            g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_TEXTUREFACTOR, g_pCurrentScene->m_pMouseOverHuman->m_dwEdgeColor);
            g_pDevice->SetTexture(0, g_pTextureManager->GetModelTexture(m_nTextureIndex, 10000));
            g_pDevice->SetRenderState(D3DRS_SHADEMODE, 1u);
            g_pDevice->SetRenderState(D3DRS_SRCBLEND, 5u);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6u);
            g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 2u);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 3u);
            g_pDevice->SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
            g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
            g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
            g_pDevice->SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);

            if (g_pDevice->m_pd3dDevice->DrawIndexedPrimitive(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST,
                0,
                0,
                m_pMesh->m_AttRange[0].VertexCount,
                0,
                m_numFaces) < 0)
                return 0;

            g_pDevice->SetRenderState(D3DRS_CULLMODE, 3u);
            g_pDevice->SetRenderState(D3DRS_SHADEMODE, 2u);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4u);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
            g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
            g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
            g_pDevice->SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
            g_pDevice->SetRenderState(D3DRS_FOGENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
        }

        if (g_pDevice->m_bUseSW)
            g_pDevice->m_pd3dDevice->SetSoftwareVertexProcessing(0);

        g_pDevice->m_pd3dDevice->SetVertexShader(0);

        if (m_sMultiType > 0)
        {
            if (m_pParentSkin->m_cDefaultAlpha == 1)
                g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, D3DBLEND::D3DBLEND_INVSRCALPHA);

            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_TEXCOORDINDEX, D3DTEXTUREOP::D3DTOP_DISABLE);
            g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_DISABLE);
            g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHATESTENABLE, 1);
        }

        g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
        g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, D3DTEXTUREOP::D3DTOP_DISABLE);
        return 1;
    }
    else
    {
        if (m_sMultiType > 0)
        {
            if (m_pParentSkin->m_cDefaultAlpha == 1)
                g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, D3DBLEND::D3DBLEND_INVSRCALPHA);

            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_TEXCOORDINDEX, D3DTEXTUREOP::D3DTOP_DISABLE);
            g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_MODULATE);
            g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, D3DTEXTUREOP::D3DTOP_DISABLE);
        }

        return 1;
    }

    return 0;
}

int CMesh::LoadMesh(char* file)
{
    if (!file || !g_pMeshManager || !g_pDevice || !g_pDevice->m_pd3dDevice)
        return 0;
    const std::size_t pathCapacity = sizeof(g_pMeshManager->m_stSkinMeshList[0].szFileName);
    if (strnlen_s(file, pathCapacity) >= pathCapacity || !file[0])
        return 0;
    m_nSkinMeshIndex = g_pMeshManager->GetSkinMeshIndex(file);
    if (m_nSkinMeshIndex == -1)
    {
        m_nSkinMeshIndex = g_pMeshManager->GetAvailableSkinMeshIndex();

        if (m_nSkinMeshIndex < 0 || m_nSkinMeshIndex >= MAX_SKIN_MESH_LIST)
            return 0;

        FILE* handle = nullptr;
        fopen_s(&handle, file, "rb");
        if (handle == nullptr)
            return 0;
        std::unique_ptr<FILE, decltype(&fclose)> input(handle, &fclose);
        if (_fseeki64(handle, 0, SEEK_END) != 0) return 0;
        const auto fileBytes = _ftelli64(handle);
        if (fileBytes < 0 || _fseeki64(handle, 0, SEEK_SET) != 0) return 0;
        skin_mesh_load::Header header{};
        skin_mesh_load::Payload payload;
        if (!skin_mesh_load::ReadExact(handle, &header, sizeof(header)) ||
            !skin_mesh_load::Plan(header, static_cast<std::uint64_t>(fileBytes),
                sizeof(m_pBoneOffset) / sizeof(m_pBoneOffset[0]), payload))
            return 0;

        std::unique_ptr<TMMesh> pending(new (std::nothrow) TMMesh());
        if (!pending) return 0;
        auto* mesh = pending.get();
        mesh->m_dwFVF = header.fvf;
        mesh->m_sizeVertex = header.stride;
        mesh->m_AttRange[0].VertexCount = header.vertices;
        mesh->m_AttRange[0].FaceCount = header.indices;
        std::unique_ptr<D3DXMATRIX, decltype(&free)> matrices(
            payload.matrices ? static_cast<D3DXMATRIX*>(malloc(payload.matrices)) : nullptr, &free);
        std::unique_ptr<DWORD, decltype(&free)> names(
            payload.names ? static_cast<DWORD*>(malloc(payload.names)) : nullptr, &free);
        if (!skin_mesh_load::ReadExact(handle, matrices.get(), payload.matrices) ||
            !skin_mesh_load::ReadExact(handle, names.get(), payload.names)) return 0;

        const auto createVertices = [&](IDirect3DVertexBuffer9** buffer) {
            return g_pDevice->m_pd3dDevice->CreateVertexBuffer(payload.vertices, 0,
                header.fvf, D3DPOOL_MANAGED, buffer, nullptr);
        };
        const auto readVertices = [&](void* data) {
            auto* pVertex = static_cast<float*>(data);
            const std::size_t nFloatCount = header.stride / sizeof(float);
            const std::size_t nNumVertex = header.vertices;
            if (!skin_mesh_load::Positions(data, header)) return false;

            // Preserve the established bounds calculation; this batch changes safety only.
            for (std::size_t i = 0; i < nNumVertex - 1; ++i)
            {
                if (pVertex[nFloatCount * i] > mesh->m_fMaxX)
                    mesh->m_fMaxX = pVertex[nFloatCount * i];
                if (mesh->m_fMinX > pVertex[nFloatCount * i])
                    mesh->m_fMinX = pVertex[nFloatCount * i];
                if (pVertex[nFloatCount * i + 1] > mesh->m_fMaxY)
                    mesh->m_fMaxY = pVertex[nFloatCount * i + 1];
                if (mesh->m_fMinY > pVertex[nFloatCount * i + 1])
                    mesh->m_fMinY = pVertex[nFloatCount * i + 1];
                if (pVertex[nFloatCount * i + 2] > mesh->m_fMaxZ)
                    mesh->m_fMaxZ = pVertex[nFloatCount * i + 2];
                if (mesh->m_fMinZ > pVertex[nFloatCount * i + 2])
                    mesh->m_fMinZ = pVertex[nFloatCount * i + 2];
            }

            for (const float bound : {mesh->m_fMaxX, mesh->m_fMinX, mesh->m_fMaxY,
                mesh->m_fMinY, mesh->m_fMaxZ, mesh->m_fMinZ})
                if (fabsf(bound) > mesh->m_fRadius) mesh->m_fRadius = fabsf(bound);
            return true;
        };
        if (!skin_mesh_load::ReadBuffer(handle, payload.vertices, mesh->m_pVB,
            createVertices, readVertices)) return 0;
        const auto createIndices = [&](IDirect3DIndexBuffer9** buffer) {
            return g_pDevice->m_pd3dDevice->CreateIndexBuffer(payload.indices, 0,
                D3DFMT_INDEX16, D3DPOOL_MANAGED, buffer, nullptr);
        };
        const auto readIndices = [&](void* data) {
            return skin_mesh_load::Indices(data, header);
        };
        if (!skin_mesh_load::ReadBuffer(handle, payload.indices, mesh->m_pIB,
            createIndices, readIndices)) return 0;

        // Publish only a completely initialized entry. MeshManager owns these resources.
        auto& entry = g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex];
        strcpy_s(entry.szFileName, file);
        entry.m_dwID = header.id;
        entry.m_numFaceInflunce = header.influences;
        entry.m_numPalette = header.palette;
        entry.m_pBoneMatrix = matrices.release();
        entry.m_dwNames = names.release();
        entry.pMesh = pending.release();
    }

    {
        m_pMesh = g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex].pMesh;
        if (!m_pMesh || g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex].m_numPalette >
            sizeof(m_pBoneOffset) / sizeof(m_pBoneOffset[0]))
            return 0;

        if (m_pMesh->m_bDummy == 1)
            return 0;

        m_pBoneMatrix = g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex].m_pBoneMatrix;
        m_dwID = g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex].m_dwID;
        m_numFaceInflunce = g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex].m_numFaceInflunce;
        m_numPalette = g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex].m_numPalette;
        m_dwNames = g_pMeshManager->m_stSkinMeshList[m_nSkinMeshIndex].m_dwNames;
        m_numFaces = m_pMesh->m_AttRange[0].FaceCount / 3;
        m_bMeshGenerated = 1;
    }

    return 1;
}

int CMesh::InitEffect()
{
    if (!m_pParentSkin->m_pOwner)
        return 1;

    for (int i = 0; i < 2; i++)
    {
        if (m_pParentSkin && 
            m_pParentSkin->m_nBoneAniIndex < 19 && 
            m_dwID == g_dwHandIndex[m_pParentSkin->m_nBoneAniIndex][i])
        {
            TMEffectSWSwing* eff = new TMEffectSWSwing();
            if (eff != nullptr)
            {
                eff->m_pParentSkin = m_pParentSkin;
                if (m_pParentSkin->m_pSwingEffect[i])
                {
                    g_pObjectManager->DeleteObject(m_pParentSkin->m_pSwingEffect[i]);
                    m_pParentSkin->m_pSwingEffect[i] = 0;
                }

                m_pParentSkin->m_pSwingEffect[i] = eff;
                CFrame* pFrame = m_pParentSkin->m_pRoot->FindFrame(m_dwID);

                eff->m_dwNumIndex = 0;
                for (int j = 0; j < 48; j++)
                {
                    if (!pFrame)
                        break;

                    eff->m_dwIndices[j] = pFrame->m_dwParentID;
                    pFrame = m_pParentSkin->m_pRoot->FindFrame(pFrame->m_dwParentID);
                    ++eff->m_dwNumIndex;

                    if (!eff->m_dwIndices[j])
                        break;
                }

                if (g_pCurrentScene != nullptr && g_pCurrentScene->m_pEffectContainer != nullptr)
                {
                    g_pCurrentScene->m_pEffectContainer->AddChild(eff);
                }
            }
        }
    }

    return 1;
}

void CMesh::SetMaterial(char cAlpha)
{
    if (m_pParentSkin != nullptr)
    {
        D3DMATERIAL9 materials;
        memcpy(&materials, &m_pParentSkin->m_materials, sizeof(materials));
        g_pDevice->m_pd3dDevice->SetMaterial(&materials);

        unsigned int dwTime = g_pTimerManager->GetServerTime();
        float fProgress = (float)(dwTime % 10000) / 10000.0f;

        D3DXVECTOR4 vConst(1.0f, 0.0f, fProgress, 765.01001f);
        D3DXCOLOR ambEmm;      
        
        auto pC1 = D3DXCOLOR(materials.Ambient);
        auto pC2 = D3DXCOLOR(0.25f, 0.25f, 0.25f, 1.0f);
        D3DXColorModulate(&ambEmm, &pC1, &pC2);

        if (m_sLegendType >= 116 && m_sLegendType <= 125 && m_sMultiType > 0)
        {
            D3DXCOLOR Emi(materials.Emissive);

            Emi.a = 0.89f;
            if (materials.Emissive.r < 0.55000001f)
                Emi.r = 0.55000001f;
            if (materials.Emissive.g < 0.55000001f)
                Emi.g = 0.55000001f;
            if (materials.Emissive.b < 0.55000001f)
                Emi.b = 0.55000001f;

            materials.Diffuse.a = 0.89f;
            if (materials.Diffuse.r > 0.1f)
                materials.Diffuse.r = 0.1f;
            if (materials.Diffuse.g > 0.1f)
                materials.Diffuse.g = 0.1f;
            if (materials.Diffuse.b > 0.1f)
                materials.Diffuse.b = 0.1f;

            ambEmm += Emi;
        }
        else
        {
            ambEmm += materials.Emissive;
        }

        vConst.y = materials.Power;
        g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(
            8, (const float*)&materials, 1
        );
        g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(
            7, (const float*)&ambEmm, 1
        );
        g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(
            0, (const float*)&vConst, 1
        );

        float fEnd = g_pDevice->m_fFogEnd;
        if (m_pParentSkin->m_nBoneAniIndex == 61)
            fEnd = fEnd * 1.4f;

        float fDistance = fEnd - g_pDevice->m_fFogStart;
        if (fDistance == 0.0f)
            fDistance = 0.01f;

        float fFog[4] = {1.0f, fEnd, 1.0f / fDistance, 0.0f};
        g_pDevice->m_pd3dDevice->SetVertexShaderConstantF(
            6, (const float*)&fFog, 1
        );
    }
}

