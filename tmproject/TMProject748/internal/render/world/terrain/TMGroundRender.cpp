#include "pch.h"
// TMGround split by responsibility; terrain lifecycle and loading stay in TMGround.cpp.
#include "TMSea.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMCamera.h"
#include "TMEffectBillBoard.h"
#include "TMSkillFire.h"

int TMGround::Render()
{
    if (g_bHideBackground)
        return 0;

    if (!m_bVisible)
        return 1;

    TMCamera* pCamera = g_pObjectManager->m_pCamera;
    if (pCamera->m_fVerticalAngle > 0.4f)
        return 1;

    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELCHAR)
        return 1;

    int nXList[3]{};
    int nYList[3]{};
    unsigned int dwColor[4]{};
    float fX[4]{};
    float fY[4]{};

    D3DXVECTOR3 vTemp;
    D3DXVECTOR3 vPosTransformed;
    D3DXMATRIX matScale;
    D3DXMATRIX matPos;

    D3DXMatrixScaling(&matScale, 2.0f, 0.1f, 2.0f);
    D3DXMatrixTranslation(&matPos, m_vecOffset.x, 0, m_vecOffset.y);
    D3DXMatrixMultiply(&matScale, &g_pDevice->m_matWorld, &matScale);
    D3DXMatrixMultiply(&matScale, &matScale, &matPos);

    g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLD, &matScale);

    if (m_bWire == 1)
        g_pDevice->SetRenderState(D3DRS_FILLMODE, 2);

    TMVector3 vecCam = pCamera->m_cameraPos;

    int nCamPosX = ((int)(vecCam.x - m_vecOffset.x) / 2);
    int nCamPosY = ((int)(vecCam.z - m_vecOffset.y) / 2);

    g_pDevice->SetRenderState(D3DRS_RANGEFOGENABLE, 0);
    g_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, 3);

    if (g_pDevice->m_bVoodoo == 1)
    {
        g_pDevice->SetRenderState(D3DRS_COLORVERTEX, 1);
        g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
        g_pDevice->SetRenderState(D3DRS_LIGHTING, 0);
        g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6);
    }
    else
    {
        g_pDevice->SetRenderState(D3DRS_COLORVERTEX, 1);
        g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6);
        g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);

        if (m_bDungeon && m_bDungeon != 3 && m_bDungeon != 4)
            g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
        else
            g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 4);

        g_pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
    }

    g_pDevice->m_pd3dDevice->SetMaterial(&m_materials);

    int nClipIndex = 15;
    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_DEMO)
        nClipIndex = 18;

    int nMinClipIndex = 0;

    if (fabsf(g_pObjectManager->m_pCamera->m_fVerticalAngle) > 1.0f)
        nClipIndex = (int)((pCamera->m_fSightLength * 1.5f) + 8.0f);

    nMinClipIndex = nClipIndex / 3;

    int nMinX = nCamPosX - nMinClipIndex;
    int nMinY = nCamPosY - nMinClipIndex;
    int nMaxX = nClipIndex + nCamPosX;
    int nMaxY = nClipIndex + nCamPosY;

    if (RenderDevice::m_bDungeon >= 0 || g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELECT_SERVER || g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_DEMO)
    {
        int nLen = (int)(pCamera->m_fSightLength + 1.0f);

        if ((RenderDevice::m_bDungeon == 2
            || RenderDevice::m_bDungeon == 3
            || RenderDevice::m_bDungeon == 4
            || !RenderDevice::m_bDungeon
            || RenderDevice::m_bDungeon == 4)
            && g_pObjectManager->m_pCamera->m_fVerticalAngle > -0.69999999f)
        {
            nLen = 13;
        }

        if (nLen < 6)
            nLen = 6;

        nLen += 2; //??? pqp

        if ((int)vecCam.x >> 7 > 26
            && (int)vecCam.x >> 7 < 31
            && (int)vecCam.z >> 7 > 20
            && (int)vecCam.z >> 7 < 25)
        {
            if (g_pObjectManager->m_pCamera->m_fVerticalAngle > -0.60000002f)
            {
                nLen += 10;
            }
            else if (g_pObjectManager->m_pCamera->m_fVerticalAngle > -0.66000003f)
            {
                nLen += 4;
            }
            else if (g_pObjectManager->m_pCamera->m_fVerticalAngle > -0.68000001f)
            {
                nLen += 3;
            }
        }
        else if (g_pObjectManager->m_pCamera->m_fVerticalAngle > -0.60000002f)
            nLen += 3;

        float fcos = cosf(pCamera->m_fHorizonAngle);
        float fsin = sinf(pCamera->m_fHorizonAngle);
        nXList[0] = nCamPosX;
        nYList[0] = nCamPosY;
        nXList[1] = nCamPosX + (int)(nLen * fcos);
        nYList[1] = nCamPosY + (int)(nLen * fsin);
        nXList[2] = (nXList[1] + nCamPosX) / 2;
        nYList[2] = (nYList[1] + nCamPosY) / 2;
        nMinX = nXList[2] - nLen;
        nMinY = nYList[2] - nLen;
        nMaxX = nLen + nXList[2];
        nMaxY = nLen + nYList[2];
    }

    int nTickX = 1, nTickY = 1;
    for (int nY = nMinY; nY < nMaxY; ++nY)
    {
        if (nY >= 0 && nY <= 63)
        {
            for (int nX = nMinX; nX < nMaxX; ++nX)
            {
                if (nX < 0 || nX > 63)
                    continue;

                nTickX = 1;
                nTickY = 1;

                if (nCamPosX % 2)
                {
                    if (std::abs(nX - nCamPosX) > 17)
                    {
                        if (std::abs(nX - nCamPosX) % 2)
                            continue;

                        nTickX = 2;
                    }
                }
                else
                {
                    if (std::abs(nX - nCamPosX) > 16)
                    {
                        if (!(std::abs(nX - nCamPosX) % 2))
                            continue;

                        nTickX = 2;
                    }
                }

                if (nCamPosY % 2)
                {
                    if (std::abs(nY - nCamPosY) > 17)
                    {
                        if (std::abs(nY - nCamPosY) % 2 == 1)
                            continue;

                        nTickY = 2;
                    }
                }
                else
                {
                    if (std::abs(nY - nCamPosY) > 16)
                    {
                        if (!(std::abs(nY - nCamPosY) % 2))
                            continue;

                        nTickY = 2;
                    }
                }

                char bCoordIndex = m_TileMapData[nX + (nY << 6)].byTileCoord;
                char bCoordBackIndex = m_TileMapData[nX + (nY << 6)].byBackTileCoord;
                int nTexIndex = (unsigned char)m_TileMapData[nX + (nY << 6)].byTileIndex + 10;

                if (g_pDevice->m_bVoodoo == 1)
                {
                	const ExtractedFlow extractedFlow = BuildVoodooTileVertices(bCoordIndex, dwColor, nTexIndex, nTickX, nTickY, nX, nY);
                	if (extractedFlow == ExtractedFlow::Continue)
                		continue;
                }

                int nIndex = m_TileMapData[nX + (nY << 6)].byTileIndex + 10;
                for (int k = 0; k < 4; k++)
                {
                	const ExtractedFlow extractedFlow = SetTileTextureCoordinates(bCoordBackIndex, bCoordIndex, fX, fY, k, nIndex, nTexIndex, nX, nY);
                	if (extractedFlow == ExtractedFlow::Continue)
                		continue;
                }

                BuildTileVertices(nTickX, nTickY, nX, nY);
                if (m_dwEffStart && m_dwServertime < (m_dwEffStart + 2000))
                {
                    float Height = (float)((4.0f * m_fEffHeight) * (float)(m_dwServertime - m_dwEffStart - 2000) / 2000.0f);

                    for (int j = 0; j < 4; j++)
                    {
                        auto vecCalc = (float)((float)(m_vecEffset.x - m_vertex[j].position.x) * (float)(m_vecEffset.y - m_vertex[j].position.z)) / 10.0f;

                        float fCos = cosf(((m_vecEffset.x - m_vertex[j].position.x)
                            * (m_vecEffset.y - m_vertex[j].position.z))
                            / 10.0f + (float)((float)m_dwServertime / 300.0f));

                        m_vertex[j].position.y = (float)(fCos * Height) + m_vertex[j].position.y;
                    }
                }

                g_pDevice->SetTexture(0, g_pTextureManager->GetEnvTexture(nTexIndex, 5000));

                if (!m_bDungeon || m_bDungeon == 3 || m_bDungeon == 4)
                    g_pDevice->SetTexture(1, g_pTextureManager->GetEnvTexture(((unsigned char)m_TileMapData[nX + (nY << 6)].byBackTileIndex + 256), 5000));

                g_pDevice->m_pd3dDevice->SetFVF(594);

                int bVisible = 0;

                for (int m = 0; m < 4; ++m)
                {
                    D3DXVECTOR3 tempVec;
                    tempVec.x = (m_vertex[m].position.x * 2.0f) + m_vecOffset.x;
                    tempVec.y = m_vertex[m].position.y * 0.1f;
                    tempVec.z = (m_vertex[m].position.z * 2.0f) + m_vecOffset.y;

                    D3DXVec3TransformCoord(&vTemp, &tempVec, &g_pDevice->m_matView);
                    D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);

                    if (vPosTransformed.z >= 0.0f && vPosTransformed.z < 1.0f)
                    {
                        int vPosInX = (int)((((vPosTransformed.x + 1.0f) * (float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)) / 2.0f));
                        int vPosInY = (int)((((-vPosTransformed.y + 1.0f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nWidthShift)) / 2.0f));

                        if ((float)vPosInX > (float)(-50.0f * RenderDevice::m_fWidthRatio)
                            && ((float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)
                                + (50.0f * RenderDevice::m_fWidthRatio)) > (float)vPosInX
                            && (float)vPosInY > (float)(-50.0f * RenderDevice::m_fHeightRatio)
                            && ((float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift)
                                + (50.0f * RenderDevice::m_fHeightRatio)) > (float)vPosInY)
                        {
                            bVisible = 1;
                            break;
                        }
                    }
                }
                if (bVisible == 1)
                    g_pDevice->m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_vertex, 44u);
            }
        }
    }

    if (m_dwServertime - m_dwLastEffectTime > 2000)
        m_dwLastEffectTime = m_dwServertime;

    if (g_pDevice->m_bVoodoo == 1)
        g_pDevice->SetRenderState(D3DRS_LIGHTING, 1);

    g_pDevice->SetRenderState(D3DRS_COLORVERTEX, 0);
    g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
    g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
    g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
    g_pDevice->SetTextureStageState(2, D3DTSS_COLOROP, 1);

    if (m_bWire == 1)
        g_pDevice->SetRenderState(D3DRS_FILLMODE, 3);

    return 1;
}

// Extracted from TMGround::Render; behavior is unchanged.
ExtractedFlow TMGround::BuildVoodooTileVertices(char& bCoordIndex, unsigned int (&dwColor)[4], int& nTexIndex, int& nTickX, int& nTickY, int& nX, int& nY)
{
                    // TODO: THIS CODE NEED TO BE REVIEW =)
                    for (int nVertexIndex = 0; nVertexIndex < 4; ++nVertexIndex)
                    {
                        m_vertexVoodoo[nVertexIndex].tu = TMGround::TileCoordList[(unsigned char)bCoordIndex][nVertexIndex][0];
                        m_vertexVoodoo[nVertexIndex].tv = TMGround::TileCoordList[(unsigned char)bCoordIndex][nVertexIndex][1];
                    }

                    dwColor[0] = m_TileMapData[nX + (nY << 6)].dwColor;
                    dwColor[1] = m_TileMapData[nX + ((nTickY + nY) << 6)].dwColor;
                    dwColor[2] = m_TileMapData[(nY << 6) + nTickX + nX].dwColor;
                    dwColor[3] = m_TileMapData[((nTickY + nY) << 6) + nTickX + nX].dwColor;

                    for (int i = 0; i < 4; i++)
                    {
                        auto fR = static_cast<float>(WYDCOLOR_RED(dwColor[i])) / 256.0f;
                        auto fG = static_cast<float>(WYDCOLOR_GREEN(dwColor[i])) / 256.0f;
                        auto fB = static_cast<float>(WYDCOLOR_BLUE(dwColor[i])) / 256.0f;

                        D3DXCOLOR color1 = D3DXCOLOR();
                        color1.r = fR;
                        color1.g = fG;
                        color1.b = fB;

                        D3DXCOLOR color2 = D3DXCOLOR();
                        color2.r = g_pDevice->m_colorLight.r * 0.40000001f;
                        color2.g = g_pDevice->m_colorLight.g * 0.40000001f;
                        color2.b = g_pDevice->m_colorLight.b * 0.40000001f;

                        D3DXCOLOR RetColor = D3DXCOLOR();
                        D3DXColorLerp(&RetColor, &color1, &color2, 0.69999999f);

                        m_vertexVoodoo[i].diffuse = (unsigned int)(RetColor.b * 256.0f) | ((unsigned int)(RetColor.g * 256.0f) << 8) | ((unsigned int)(RetColor.r * 256.0f) << 16);
                    }
                    if (nX < 63 && nY < 63)
                    {
                        m_vertex[0].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                        m_vertex[1].diffuse = m_TileMapData[nX + ((nTickY + nY) << 6)].dwColor;
                        m_vertex[2].diffuse = m_TileMapData[(nY << 6) + nTickX + nX].dwColor;
                        m_vertex[3].diffuse = m_TileMapData[((nTickY + nY) << 6) + nTickX + nX].dwColor;

                        m_vertexVoodoo[0].position = TMVector3((float)nX, (float)m_TileMapData[nX + (nY << 6)].cHeight, (float)nY);
                        m_vertexVoodoo[1].position = TMVector3((float)nX, (float)m_TileMapData[nX + ((nTickY + nY) << 6)].cHeight, (float)(nTickY + nY));
                        m_vertexVoodoo[2].position = TMVector3((float)(nTickX + nX), (float)m_TileMapData[(nY << 6) + nTickX + nX].cHeight, (float)nY);
                        m_vertexVoodoo[3].position = TMVector3((float)(nTickX + nX), (float)m_TileMapData[((nTickY + nY) << 6) + nTickX + nX].cHeight, (float)(nTickY + nY));
                    }
                    if (nX == 63 && nY < 63)
                    {
                        m_vertex[0].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                        m_vertex[1].diffuse = m_TileMapData[nX + ((nTickY + nY) << 6)].dwColor;
                        m_vertex[2].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                        m_vertex[3].diffuse = m_TileMapData[nX + ((nTickY + nY) << 6)].dwColor;

                        m_vertexVoodoo[0].position = TMVector3((float)nX, (float)m_TileMapData[nX + (nY << 6)].cHeight, (float)nY);
                        m_vertexVoodoo[1].position = TMVector3((float)nX, (float)m_TileMapData[nX + ((nTickY + nY) << 6)].cHeight, (float)(nTickY + nY));
                        m_vertexVoodoo[2].position = TMVector3((float)(nTickX + nX), (float)m_TileMapData[nX + (nY << 6)].cHeight, (float)nY);
                        m_vertexVoodoo[3].position = TMVector3((float)(nTickX + nX), (float)m_TileMapData[nX + ((nTickY + nY) << 6)].cHeight, (float)(nTickY + nY));
                    }
                    if (nY == 63 && nX < 63)
                    {
                        m_vertex[0].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                        m_vertex[1].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                        m_vertex[2].diffuse = m_TileMapData[(nY << 6) + nTickX + nX].dwColor;
                        m_vertex[3].diffuse = m_TileMapData[(nY << 6) + nTickX + nX].dwColor;

                        m_vertexVoodoo[0].position = TMVector3((float)nX, m_TileMapData[nX + (nY << 6)].cHeight, (float)nY);
                        m_vertexVoodoo[1].position = TMVector3((float)nX, m_TileMapData[nX + (nY << 6)].cHeight, (float)(nTickY + nY));
                        m_vertexVoodoo[2].position = TMVector3((float)(nTickX + nX), m_TileMapData[(nY << 6) + nTickX + nX].cHeight, (float)nY);
                        m_vertexVoodoo[3].position = TMVector3((float)(nTickX + nX), m_TileMapData[(nY << 6) + nTickX + nX].cHeight, (float)(nTickY + nY));
                    }

                    if (m_dwEffStart && m_dwServertime < (m_dwEffStart + 2000))
                    {
                        auto Height = (float)(((4.0f * m_fEffHeight) * (float)(m_dwServertime - m_dwEffStart - 2000)) / 2000.0f);

                        for (int j = 0; j < 4; j++)
                        {
                            auto vecCalc = (float)((float)(m_vecEffset.x - m_vertexVoodoo[j].position.x) * (float)(m_vecEffset.y - m_vertexVoodoo[j].position.z)) / 10.0f;

                            float fCos = cosf(((m_vecEffset.x - m_vertexVoodoo[j].position.x) * (m_vecEffset.y - m_vertexVoodoo[j].position.z))
                                / 10.0f + ((float)m_dwServertime / 300.0f));
                            m_vertexVoodoo[j].position.y = (float)(fCos * Height) + m_vertexVoodoo[j].position.y;
                        }
                    }

                    g_pDevice->SetTexture(0, g_pTextureManager->GetEnvTexture(nTexIndex, 5000));
                    g_pDevice->m_pd3dDevice->SetFVF(322);
                    g_pDevice->m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_vertexVoodoo, 24);
                    return ExtractedFlow::Continue;
	return ExtractedFlow::Next;
}

// Extracted from TMGround::Render; behavior is unchanged.
ExtractedFlow TMGround::SetTileTextureCoordinates(char& bCoordBackIndex, char& bCoordIndex, float (&fX)[4], float (&fY)[4], int& k, int& nIndex, int& nTexIndex, int& nX, int& nY)
{
                    m_vertex[k].tu1 = TMGround::TileCoordList[(unsigned char)bCoordIndex][k][0];
                    m_vertex[k].tv1 = TMGround::TileCoordList[(unsigned char)bCoordIndex][k][1];

                    if (!m_bDungeon || m_bDungeon == 3 || m_bDungeon == 4)
                    {
                        m_vertex[k].tu2 = TMGround::BackTileCoordList[(unsigned char)bCoordBackIndex][k][0];
                        m_vertex[k].tv2 = TMGround::BackTileCoordList[(unsigned char)bCoordBackIndex][k][1];
                       return ExtractedFlow::Continue;
                    }

                    if (m_vecOffsetIndex.x < 26 || m_vecOffsetIndex.x > 30 || m_vecOffsetIndex.y < 8 || m_vecOffsetIndex.y > 12)
                    {
                        if (nIndex == 170 || nIndex == 171)
                        {
                            m_vertex[k].tu1 = (float)((float)(m_dwServertime % 10000) / 10000.0f) + m_vertex[k].tu1;

                            if (g_bHideEffect)
                                return ExtractedFlow::Continue;

                            int nRandV = rand();

                            TMVector3 vecPos = TMVector3((float)((float)((float)nX * 2.0f) + m_vecOffset.x) + 0.5f,
                                (float)((float)m_TileMapData[nX + (nY << 6)].cHeight * 0.1f) + 1.5f,
                                (float)((float)((float)nY * 2.0f) + m_vecOffset.y) + 0.5f);

                            if (nRandV % 200 < 2)
                            {
                                int nRand = rand() % 10;

                                TMEffectBillBoard* mpBill = new TMEffectBillBoard(0, 1000,
                                    (float)((float)nRand * 0.19f) + 0.02f,
                                    (float)((float)nRand * 0.60000002f) + 0.02f,
                                    (float)((float)nRand * 0.19f) + 0.02f,
                                    0.000099999997f, 1, 80);

                                if (mpBill)
                                {
                                    vecPos.x = (float)((float)(rand() % 10 - 5) * 0.02f) + vecPos.x;
                                    vecPos.z = (float)((float)(rand() % 10 - 5) * 0.02f) + vecPos.z;

                                    mpBill->m_vecPosition = vecPos;
                                    mpBill->m_vecStartPos = vecPos;

                                    mpBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                    mpBill->m_bStickGround = 0;
                                    mpBill->m_nParticleType = 1;
                                    mpBill->m_fParticleV = 0.69999999f;
                                    mpBill->SetColor(0xFFFFAA00);

                                    g_pCurrentScene->m_pEffectContainer->AddChild(mpBill);
                                }
                            }
                            if (m_dwServertime - m_dwLastEffectTime > 2000
                                && !(nX % 2)
                                && !(nY % 3)
                                && (nRandV % 100 < 1))
                            {
                                int glowRand = (rand() % 7);

                                auto pGlow = new TMEffectBillBoard(56, 20000, 0.2f, 0.2f, 0.2f, 0.0f, 1, 80);

                                if (pGlow)
                                {
                                    pGlow->m_vecPosition = TMVector3(
                                        vecPos.x,
                                        (float)(vecPos.y + 1.5f) + (float)((float)glowRand * 0.2f),
                                        vecPos.z);

                                    pGlow->m_vecStartPos = pGlow->m_vecPosition;

                                    pGlow->m_fCircleSpeed = (float)((float)glowRand * 0.1f) + 1.5f;
                                    pGlow->m_fParticleH = (float)((float)glowRand * 0.30000001f) + 3.0f;
                                    pGlow->m_fParticleV = (float)((float)glowRand * 0.050000001f) + 0.2f;
                                    pGlow->m_nParticleType = glowRand % 3 + 6;
                                    pGlow->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                    pGlow->SetColor(0xFFFFAA00);

                                    g_pCurrentScene->m_pEffectContainer->AddChild(pGlow);
                                }

                                pGlow = new TMEffectBillBoard(60, 20000, 0.07f, 0.07f, 0.07f, 0.0f, 1, 80);

                                if (pGlow)
                                {
                                    pGlow->m_vecPosition = TMVector3(
                                        vecPos.x,
                                        (float)(vecPos.y + 1.5f) + (float)((float)glowRand * 0.2f),
                                        vecPos.z);

                                    pGlow->m_vecStartPos = pGlow->m_vecPosition;

                                    pGlow->m_fCircleSpeed = (float)((float)glowRand * 0.1f) + 1.5f;
                                    pGlow->m_fParticleH = (float)((float)glowRand * 0.30000001f) + 3.0f;
                                    pGlow->m_fParticleV = (float)((float)glowRand * 0.050000001f) + 0.2f;
                                    pGlow->m_nParticleType = glowRand % 3 + 6;
                                    pGlow->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                    pGlow->SetColor(0xFFFFFF00);

                                    g_pCurrentScene->m_pEffectContainer->AddChild((TreeNode*)pGlow);
                                }

                                if (glowRand < 3)
                                {
                                    auto pFire = new TMSkillFire(vecPos, 1, 0, 0xFFFFFFFF, 0x22331100);

                                    if (pFire)
                                        g_pCurrentScene->m_pEffectContainer->AddChild(pFire);
                                }
                            }
                        }
                        else if (nIndex == 38 || nIndex == 39)
                        {
                            fX[0] = 0.0f;
                            fX[1] = 0.0f;
                            fX[2] = 1.0f;
                            fX[3] = 1.0f;
                            fY[0] = 0.0f;
                            fY[1] = 1.0f;
                            fY[2] = 0.0f;
                            fY[3] = 1.0f;

                            g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 5);

                            g_pDevice->SetTexture(1, g_pTextureManager->GetEnvTexture(344, 5000));

                            float fAngle = (float)(m_dwServertime % 10000) / 10000.0f;

                            m_vertex[k].tu2 = fX[k];
                            m_vertex[k].tv2 = fY[k] + fAngle;

                            nTexIndex = nIndex + 92;
                        }
                        else if (nIndex >= 62 && nIndex <= 65)
                        {
                            g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 5);
                            g_pDevice->SetTexture(1, g_pTextureManager->GetEnvTexture(nIndex + 286, 5000));

                            m_vertex[k].tu2 = m_vertex[k].tu1;
                            m_vertex[k].tv2 = m_vertex[k].tv1;

                            nTexIndex = nIndex % 2 + 130;
                        }
                        else
                        {
                            g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
                        }
                    }
	return ExtractedFlow::Next;
}


// Extracted from TMGround::Render; behavior is unchanged.
void TMGround::BuildTileVertices(int& nTickX, int& nTickY, int& nX, int& nY)
{
                if (nX < 63 && nY < 63)
                {
                    m_vertex[0].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                    m_vertex[1].diffuse = m_TileMapData[nX + ((nTickY + nY) << 6)].dwColor;
                    m_vertex[2].diffuse = m_TileMapData[(nY << 6) + nTickX + nX].dwColor;
                    m_vertex[3].diffuse = m_TileMapData[((nTickY + nY) << 6) + nTickX + nX].dwColor;

                    m_vertex[0].normal = m_TileNormalVector[64 * nY + nX];
                    m_vertex[1].normal = m_TileNormalVector[64 * (nTickY + nY) + nX];
                    m_vertex[2].normal = m_TileNormalVector[64 * nY + nTickX + nX];
                    m_vertex[3].normal = m_TileNormalVector[64 * (nTickY + nY) + nTickX + nX];

                    m_vertex[0].position = TMVector3((float)nX, (float)m_TileMapData[nX + (nY << 6)].cHeight, (float)nY);
                    m_vertex[1].position = TMVector3((float)nX,
                        (float)m_TileMapData[nX + ((nTickY + nY) << 6)].cHeight,
                        (float)(nTickY + nY));
                    m_vertex[2].position = TMVector3((float)(nTickX + nX),
                        (float)m_TileMapData[(nY << 6) + nTickX + nX].cHeight,
                        (float)nY);
                    m_vertex[3].position = TMVector3((float)(nTickX + nX),
                        (float)m_TileMapData[((nTickY + nY) << 6) + nTickX + nX].cHeight,
                        (float)(nTickY + nY));
                }
                else if (nX == 63 && nY < 63)
                {
                    m_vertex[0].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                    m_vertex[1].diffuse = m_TileMapData[nX + ((nTickY + nY) << 6)].dwColor;
                    m_vertex[2].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                    m_vertex[3].diffuse = m_TileMapData[nX + ((nTickY + nY) << 6)].dwColor;

                    m_vertex[0].normal = m_TileNormalVector[64 * nY + nX];
                    m_vertex[1].normal = m_TileNormalVector[64 * (nTickY + nY) + nX];
                    m_vertex[2].normal = m_TileNormalVector[64 * nY + nX];
                    m_vertex[3].normal = m_TileNormalVector[64 * (nTickY + nY) + nX];

                    m_vertex[0].position = TMVector3((float)nX, (float)m_TileMapData[nX + (nY << 6)].cHeight, (float)nY);
                    m_vertex[1].position = TMVector3((float)nX,
                        (float)m_TileMapData[nX + ((nTickY + nY) << 6)].cHeight,
                        (float)(nTickY + nY));
                    m_vertex[2].position = TMVector3((float)(nTickX + nX),
                        (float)m_TileMapData[nX + (nY << 6)].cHeight,
                        (float)nY);
                    m_vertex[3].position = TMVector3((float)(nTickX + nX),
                        (float)m_TileMapData[nX + ((nTickY + nY) << 6)].cHeight,
                        (float)(nTickY + nY));
                }
                else if (nY == 63 && nX < 63)
                {
                    m_vertex[0].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                    m_vertex[1].diffuse = m_TileMapData[nX + (nY << 6)].dwColor;
                    m_vertex[2].diffuse = m_TileMapData[(nY << 6) + nTickX + nX].dwColor;
                    m_vertex[3].diffuse = m_TileMapData[(nY << 6) + nTickX + nX].dwColor;

                    m_vertex[0].normal = m_TileNormalVector[64 * nY + nX];
                    m_vertex[1].normal = m_TileNormalVector[64 * nY + nX];
                    m_vertex[2].normal = m_TileNormalVector[64 * nY + nTickX + nX];
                    m_vertex[3].normal = m_TileNormalVector[64 * nY + nTickX + nX];

                    m_vertex[0].position = TMVector3((float)nX, (float)m_TileMapData[nX + (nY << 6)].cHeight, (float)nY);
                    m_vertex[1].position = TMVector3((float)nX,
                        (float)m_TileMapData[nX + (nY << 6)].cHeight,
                        (float)(nTickY + nY));
                    m_vertex[2].position = TMVector3((float)(nTickX + nX),
                        (float)m_TileMapData[(nY << 6) + nTickX + nX].cHeight,
                        (float)nY);
                    m_vertex[3].position = TMVector3((float)(nTickX + nX),
                        (float)m_TileMapData[(nY << 6) + nTickX + nX].cHeight,
                        (float)(nTickY + nY));
                }
}

