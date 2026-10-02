#include "pch.h"
// TMGround split by responsibility; terrain lifecycle and loading stay in TMGround.cpp.
#include "TMSea.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMCamera.h"
#include "TMUtil.h"

D3DXVECTOR3 TMGround::GetPickPos()
{
    static D3DXVECTOR3 vPickPos(0.0f, -10000.0f, 0.0f);

    D3DXVECTOR3 vPickRayDir;
    D3DXVECTOR3 vPickRayOrig;
    g_pDevice->GetPickRayVector(&vPickRayOrig, &vPickRayDir);
    D3DXVec3Normalize(&vPickRayDir, &vPickRayDir);

    float fU = 0.0f;
    float fV = 0.0f;
    float fDistance = 0.0f;
    TMCamera* pCamera = g_pObjectManager->m_pCamera;
    TMVector2 vecCam{};

    if (pCamera->m_pFocusedObject)
    {
        vecCam = pCamera->m_pFocusedObject->m_vecPosition;
    }
    else
    {
        vecCam.x = g_pObjectManager->m_pCamera->m_cameraPos.x;
        vecCam.y = g_pObjectManager->m_pCamera->m_cameraPos.z;
    }

    int nCamPosX = (int)(vecCam.x - m_vecOffset.x);
    int nCamPosY = (int)(vecCam.y - m_vecOffset.y);
    int nClipIndex = 25;
    int nMinClipIndex = 0;

    if(fabsf(g_pObjectManager->m_pCamera->m_fVerticalAngle) > 1.0f)
        nClipIndex = (int)((pCamera->m_fSightLength * 1.5f) + 8.0f);

    nMinClipIndex = nClipIndex / 2;

    for (int nY = nCamPosY - nClipIndex / 2; nY < nClipIndex + nCamPosY; ++nY)
    {
        if (nY >= 0 && nY <= 127)
        {
            for (int nX = nCamPosX - nClipIndex / 2; nX < nClipIndex + nCamPosX; ++nX)
            {
                if (nX >= 0 && nX <= 127)
                {
                    int nMaskHeight = m_pMaskData[nY][nX];
                    if (nMaskHeight > 127)
                        nMaskHeight = 0;
                    if (nMaskHeight == 127)
                        nMaskHeight = 400;

                    D3DXVECTOR3 vertex[4]{};
                    vertex[0] = D3DXVECTOR3((float)nX + m_vecOffset.x, (float)nMaskHeight * 0.1f, (float)nY + m_vecOffset.y);
                    vertex[1] = D3DXVECTOR3((float)nX + m_vecOffset.x, (float)nMaskHeight * 0.1f, ((float)nY + m_vecOffset.y) + 1.0f);
                    vertex[2] = D3DXVECTOR3(((float)nX + m_vecOffset.x) + 1.0f, (float)nMaskHeight * 0.1f, (float)nY + m_vecOffset.y);
                    vertex[3] = D3DXVECTOR3(((float)nX + m_vecOffset.x) + 1.0f, (float)nMaskHeight * 0.1f, ((float)nY + m_vecOffset.y) + 1.0f);
                    if (D3DXIntersectTri(&vertex[0], &vertex[1], &vertex[2], &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDistance) == 1)
                    {
                        vPickPos.y = vertex[0].y;
                        vPickPos.x = vertex[0].x + fV;
                        vPickPos.z = vertex[0].z + fU;
                        return vPickPos;
                    }
                    if (D3DXIntersectTri(&vertex[3], &vertex[2], &vertex[1], &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDistance) == 1)
                    {
                        vPickPos.y = vertex[3].y;
                        vPickPos.x = vertex[3].x - fV;
                        vPickPos.z = vertex[3].z - fU;
                        return vPickPos;
                    }
                }
            }
        }
    }

    if (pCamera->m_fVerticalAngle <= 0.2f && g_pCurrentScene->m_bAutoRun != 1)
    {
        int vPosInX = 0;
        int vPosInY = 0;
        D3DXVECTOR3 vTemp;
        D3DXVECTOR3 vPosTransformed;

        int nPY = (int)((float)nCamPosY / 2.0f);
        int nMinX = (int)((float)nCamPosX / 2.0f) - 12;
        int nMinY = nPY - 12;
        int nMaxX = (int)((float)nCamPosX / 2.0f) + 12;
        int nMaxY = nPY + 12;

        if (nMinX < 0)
            nMinX = 0;
        if (nMinY < 0)
            nMinY = 0;
        if (nMaxX > 64)
            nMaxX = 63;
        if (nMaxY > 64)
            nMaxY = 63;
        nCamPosX /= 2;
        nCamPosY /= 2;

        for (int j = nMinY; j < nMaxY; ++j)
        {
            if (j >= 0 && j <= 64)
            {
                for (int k = nMinX; k < nMaxX; ++k)
                {
                    if (k >= 0 && k <= 64)
                    {
                        D3DXVECTOR3 vec[4]{};
                        if (k < 64 && j < 64)
                        {
                            vec[0] = D3DXVECTOR3((float)((float)k * 2.0f) + m_vecOffset.x,
                                (float)m_TileMapData[k + (j << 6)].cHeight * 0.1f,
                                (float)((float)j * 2.0f) + m_vecOffset.y);
                            vec[1] = D3DXVECTOR3((float)((float)k * 2.0f) + m_vecOffset.x,
                                (float)m_TileMapData[k + ((j + 1) << 6)].cHeight * 0.1f,
                                (float)((float)(j + 1) * 2.0f) + m_vecOffset.y);
                            vec[2] = D3DXVECTOR3((float)((float)(k + 1) * 2.0f) + m_vecOffset.x,
                                (float)m_TileMapData[k + (j << 6) + 1].cHeight * 0.1f,
                                (float)((float)j * 2.0f) + m_vecOffset.y);
                            vec[3] = D3DXVECTOR3((float)((float)(k + 1) * 2.0f) + m_vecOffset.x,
                                (float)m_TileMapData[k + ((j + 1) << 6) + 1].cHeight * 0.1f,
                                (float)((float)(j + 1) * 2.0f) + m_vecOffset.y);

                            if (fabsf((((float)m_pMaskData[j][k] * 0.1f) - ((((vec[0].y + vec[1].y) + vec[2].y) + vec[3].y) / 4.0f))) > 1.0f)
                                continue;
                        }

                        if (D3DXIntersectTri(&vec[0], &vec[1], &vec[2], &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDistance) == 1)
                        {
                            int bVisible = 0;
                            for (int i = 0; i < 3; ++i)
                            {
                                D3DXVECTOR3 vecPos;
                                vecPos = vec[i];

                                D3DXVec3TransformCoord(&vTemp, &vecPos, &g_pDevice->m_matView);
                                D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);
                                if (vPosTransformed.z >= 0.0f && vPosTransformed.z < 1.0f)
                                {
                                    int vPosInX = g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift;
                                    vPosInX = (int)(((vPosTransformed.x + 1.0f) * (float)vPosInX) / 2.0f);
                                    int vPosInY = g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift;
                                    vPosInY = (int)(((-vPosTransformed.y + 1.0f) * (float)vPosInY) / 2.0f);

                                    if ((float)vPosInX > (float)(-100.0f * RenderDevice::m_fWidthRatio)
                                        && (float)((float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)
                                            + (float)(100.0f * RenderDevice::m_fWidthRatio)) > (float)vPosInX
                                        && (float)vPosInY > (float)(-100.0f * RenderDevice::m_fHeightRatio)
                                        && (float)((float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift)
                                            + (float)(100.0 * RenderDevice::m_fHeightRatio)) > (float)vPosInY)
                                    {
                                        bVisible = 1;
                                        break;
                                    }
                                }
                            }

                            if (bVisible)
                            {
                                vPickPos.y = vec[0].y;
                                vPickPos.x = (fV * 2.0f) + vec[0].x;
                                vPickPos.z = (fU * 2.0f) + vec[0].z;
                                return vPickPos;
                            }
                        }
                        if (D3DXIntersectTri(&vec[3], &vec[2], &vec[1], &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDistance) == 1)
                        {
                            int bVisible = 0;
                            for (int l = 1; l < 4; ++l)
                            {
                                D3DXVECTOR3 vecPos;
                                vecPos = vec[l];
                                D3DXVec3TransformCoord(&vTemp, &vecPos, &g_pDevice->m_matView);
                                D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);

                                if (vPosTransformed.z >= 0.0f && vPosTransformed.z < 1.0f)
                                {
                                    int vPosInX = g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift;
                                    vPosInX = (int)(((vPosTransformed.x + 1.0f) * (float)vPosInX) / 2.0f);
                                    int vPosInY = g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift;
                                    vPosInY = (int)(((-vPosTransformed.y + 1.0f) * (float)vPosInY) / 2.0f);


                                    if ((float)vPosInX > (float)(-100.0f * RenderDevice::m_fWidthRatio)
                                        && (float)((float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)
                                            + (float)(100.0f * RenderDevice::m_fWidthRatio)) > (float)vPosInX
                                        && (float)vPosInY > (float)(-100.0f * RenderDevice::m_fHeightRatio)
                                        && (float)((float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift)
                                            + (float)(100.0f * RenderDevice::m_fHeightRatio)) > (float)vPosInY)
                                    {
                                        bVisible = 1;
                                        break;
                                    }
                                }
                            }
                            if (bVisible)
                            {
                                vPickPos.y = vec[3].y;
                                vPickPos.x = vec[3].x - (fV * 2.0f);
                                vPickPos.z = vec[3].z - (fU * 2.0f);
                                return vPickPos;
                            }
                        }
                    }
                }
            }
        }

        return D3DXVECTOR3(0.0f, -10000.0f, 0.0f);
    }
    else
    {
        return vPickPos;
    }

    return D3DXVECTOR3();
}

float TMGround::GetHeight(TMVector2 vecPosition)
{
    float fHeight = 1.0f;

    int nX = static_cast<int>((vecPosition.x - m_vecOffset.x) / 2.0f);
    int nY = static_cast<int>((vecPosition.y - m_vecOffset.y) / 2.0f);

    if (nX < 0 || nY < 0 || nX > 64 || nY > 64)
        return -10000.0f;

    D3DXVECTOR3 vPickPos{ 0.0, -10000.0f, 0.0f };
    D3DXVECTOR3 vPickRayDir{ 0.0f, -1.0f, 0.0f };
    D3DXVECTOR3 vPickRayOrig{ vecPosition.x, 100.0f, vecPosition.y };
    D3DXVECTOR3 v0{};
    D3DXVECTOR3 v2{};
    D3DXVECTOR3 v6{};
    D3DXVECTOR3 v8{};

    if (nX < 63 && nY < 63 && nX >= 0 && nY >= 0)
    {
        v0 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + (nY << 6)].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v2 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + (nY << 6) + 1].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v6 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + ((nY + 1) << 6)].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);

        v8 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + ((nY + 1) << 6) + 1].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);
    }
    if (nX == 63)
    {
        v0 = D3DXVECTOR3((float)((float)63 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[(nY << 6) + 63].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v2 = D3DXVECTOR3((float)((float)64 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[(nY << 6) + 63].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v6 = D3DXVECTOR3((float)((float)63 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[((nY + 1) << 6) + 63].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);

        v8 = D3DXVECTOR3((float)((float)64 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[((nY + 1) << 6) + 63].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);
    }
    else if (nY == 63)
    {
        v0 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4032].cHeight * 0.1f,
            (float)((float)63 * 2.0f) + m_vecOffset.y);

        v2 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4033].cHeight * 0.1f,
            (float)((float)63 * 2.0f) + m_vecOffset.y);

        v6 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4032].cHeight * 0.1f,
            (float)((float)64 * 2.0f) + m_vecOffset.y);

        v8 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4033].cHeight * 0.1f,
            (float)((float)64 * 2.0f) + m_vecOffset.y);
    }

    float fU = 0.0;
    float fV = 0.0;
    float fDis = 0.0f;

    if (D3DXIntersectTri(&v0, &v2, &v6, &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDis) == 1)
        return (100.0f - fDis);

    if (D3DXIntersectTri(&v8, &v6, &v2, &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDis) == 1)
        return (100.0f - fDis);

    if (nX < 0 || nY < 0 || nX > 63 || nY > 63)
        return -10000.0f;

    if (nX >= 0 && nX < 63 && nY >= 0 && nY < 63)
    {
        fHeight += m_TileMapData[nX + (nY << 6)].cHeight;
        fHeight += m_TileMapData[nX + (nY << 6) + 1].cHeight;
        fHeight += m_TileMapData[nX + ((nY + 1) << 6)].cHeight;
        fHeight += m_TileMapData[nX + ((nY + 1) << 6) + 1].cHeight;
    }
    else if (nX == 63)
    {
        fHeight += m_TileMapData[(nY << 6) + 63].cHeight;
        fHeight += m_TileMapData[(nY << 6) + 63].cHeight;
        fHeight += m_TileMapData[((nY + 1) << 6) + 63].cHeight;
        fHeight += m_TileMapData[((nY + 1) << 6) + 63].cHeight;
    }
    else if (nY == 63)
    {
        fHeight += m_TileMapData[nX + 4032].cHeight;
        fHeight += m_TileMapData[nX + 4033].cHeight;
        fHeight += m_TileMapData[nX + 4032].cHeight;
        fHeight += m_TileMapData[nX + 4033].cHeight;
    }

    return (fHeight * 0.1f) / 4.0f;
}

int TMGround::GetMask(TMVector2 vecPosition)
{
    int nMaskX = (int)(vecPosition.x - m_vecOffset.x);
    int nMaskY = (int)(vecPosition.y - m_vecOffset.y);

    if (nMaskX >= 0 && nMaskY >= 0 && nMaskX < 128 && nMaskY < 128)
        return m_pMaskData[nMaskY][nMaskX];

    return -10000;
}

D3DCOLORVALUE TMGround::GetColor(TMVector2 vecPosition)
{
    int nX = static_cast<int>((vecPosition.x - m_vecOffset.x) / 2);
    int nY = static_cast<int>((vecPosition.y - m_vecOffset.y) / 2);

    int dwColor[4]{ 0 };
    if (nX >= 0 && nX < 63 && nY >= 0 && nY < 63)
    {
        dwColor[0] = m_TileMapData[nX + (nY << 6)].dwColor;
        dwColor[1] = m_TileMapData[nX + (nY << 6) + 1].dwColor;
        dwColor[2] = m_TileMapData[nX + ((nY + 1) << 6)].dwColor;
        dwColor[3] = m_TileMapData[nX + ((nY + 1) << 6) + 1].dwColor;
    }
    else if (nX == 63)
    {
        dwColor[0] = m_TileMapData[(nY << 6) + 63].dwColor;
        dwColor[1] = m_TileMapData[(nY << 6) + 63].dwColor;
        dwColor[2] = m_TileMapData[((nY + 1) << 6) + 63].dwColor;
        dwColor[3] = m_TileMapData[((nY + 1) << 6) + 63].dwColor;
    }
    else if (nX == 63)
    {
        dwColor[0] = m_TileMapData[nX + 4032].dwColor;
        dwColor[1] = m_TileMapData[nX + 4033].dwColor;
        dwColor[2] = m_TileMapData[nX + 4032].dwColor;
        dwColor[3] = m_TileMapData[nX + 4033].dwColor;
    }

    D3DCOLORVALUE color[4]{};
    for (int i = 0; i < 4; ++i)
    {
        color[i].r = ((0xFF0000 & dwColor[i]) >> 16) / 256.0f;
        color[i].g = ((dwColor[i] & 0xFF00) >> 8) / 256.0f;
        color[i].b = (dwColor[i] & 0xFF) / 256.0f;
    }

    float fDX = ((float)nX * 2.0f) - (vecPosition.x - m_vecOffset.x);
    float fDY = ((float)nY * 2.0f) - (vecPosition.y - m_vecOffset.y);

    D3DCOLORVALUE result{};

    result.g = (((((fDX + fDY) * color[3].g) + (((4.0f - fDX) - fDY) * color[0].g)) + (((fDX + 2.0f) - fDY) * color[1].g)) + (((2.0f - fDX) + fDY) * color[2].g)) / 12.0f;
    result.b = (((((fDX + fDY) * color[3].b) + (((4.0f - fDX) - fDY) * color[0].b)) + (((fDX + 2.0f) - fDY) * color[1].b)) + (((2.0f - fDX) + fDY) * color[2].b)) / 12.0f;
    result.r = (((((fDX + fDY) * color[3].r) + (((4.0f - fDX) - fDY) * color[0].r)) + (((fDX + 2.0f) - fDY) * color[1].r)) + (((2.0f - fDX) + fDY) * color[2].r)) / 12.0f;
    result.a = 1.0f;

	return result;
}

int TMGround::GetTileType(TMVector2 vecPosition)
{
    int nX = static_cast<int>(vecPosition.x - m_vecOffset.x);
    int nY = static_cast<int>(vecPosition.y - m_vecOffset.y);

    if (m_pVAttrData[nY][nX] == 1)
        return 1;

    int nIndex = m_TileMapData[nX / 2 + (nY / 2 << 6)].byTileIndex + 10;

    if (nIndex >= 14 && nIndex <= 17 && nIndex >= 38 && nIndex <= 77 && nIndex >= 86 && nIndex <= 101 && nIndex >= 130 && nIndex <= 149)
        return 0;

    if (nIndex >= 186 && nIndex <= 193)
        return 11;

    if (nIndex >= 202 && nIndex <= 205)
        return 8;

    if (nIndex < 230 || nIndex > 231)
        return 3;

    return 9;
}

void TMGround::SetColor(TMVector2 vecPosition, unsigned int dwColor)
{
    int nX = static_cast<int>(vecPosition.x - m_vecOffset.x) / 2;
    int nY = static_cast<int>(vecPosition.y - m_vecOffset.y) / 2;

    if (nX >= 0 && nX <= 63 && nY >= 0 && nY <= 63)
        m_TileMapData[nX + (nY << 6)].dwColor = dwColor;
}

TMVector3 TMGround::GetNormalInGround(int nX, int nY)
{
    TMVector3 vRetNormal(0.0f, 1.0f, 0.0f);
    TMVector3 vNormal[4]{};
    TMVector3 vAround[4]{};

    float fCurHeight = (float)m_TileMapData[nX + (nY << 6)].cHeight;

    if (nX > 0 && nX < 64 && nY > 0 && nY < 64)
    {
        vAround[0] = TMVector3(-1.0f, (float)m_TileMapData[nX + (nY << 6) - 1].cHeight, 0.0f);
        vAround[1] = TMVector3(0.0f, (float)m_TileMapData[nX + ((nY + 1) << 6)].cHeight, 1.0f);
        vAround[2] = TMVector3(1.0f, (float)m_TileMapData[nX + (nY << 6) + 1].cHeight, 0.0f);
        vAround[3] = TMVector3(0.0f, (float)m_TileMapData[nX + ((nY - 1) << 6)].cHeight, -1.0f);

        vNormal[0] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[0], vAround[1]);
        vNormal[1] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[1], vAround[2]);
        vNormal[2] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[2], vAround[3]);
        vNormal[3] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[3], vAround[0]);

        vRetNormal = (vNormal[0] + vNormal[1] + vNormal[2] + vNormal[3]) / 4.0f;
    }

    return vRetNormal;
}

int TMGround::IsInWater(TMVector2 vecPosition, float fHeight, float* pfWaterHeight)
{
    int i = 0;
    for (i = 0; ; ++i)
    {
        if (i >= 10)
            return 0;

        if (m_pSeaList[i])
        {
            POINT ptPos{};
            ptPos.x = static_cast<LONG>(vecPosition.x);
            ptPos.y = static_cast<LONG>(vecPosition.y);

            if (PtInRect(&m_pSeaList[i]->m_rectRange, ptPos) == 1)
                break;
        }
    }

    if (m_pSeaList[i]->m_fHeight <= fHeight)
        return 0;

    *pfWaterHeight = m_pSeaList[i]->m_fHeight;
	return 1;
}

float TMGround::GetWaterHeight(TMVector2 vecPosition, float* pfWaterHeight)
{
    for (int i = 0; i < 10; ++i)
    {
        if (m_pSeaList[i])
        {
            float fHeight = m_pSeaList[i]->GetHeight(vecPosition.x, vecPosition.y);

            if (fHeight > -100.0f)
            {
                *pfWaterHeight = fHeight + m_pSeaList[i]->m_fHeight;

                return *pfWaterHeight;
            }
        }
    }

    return -100.0f;
}
