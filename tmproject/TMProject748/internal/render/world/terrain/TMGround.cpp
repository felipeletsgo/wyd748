#include "pch.h"
#include "TMSea.h"
#include "TMScene.h"
#include "TMGlobal.h"
#include "TMHuman.h"
#include "TMLog.h"
#include "TMGround.h"
#include "TerrainTileMapReader.h"
#include "TMCamera.h"
#include "TMEffectBillBoard.h"
#include "TMSkillFire.h"
#include "TMUtil.h"

int TMGround::m_bFirst = 1;
float TMGround::m_fMiniMapScale = 0.60f;

TMGround::TMGround()
    : TreeNode(0)
{
    m_vecOffsetIndex = IVector2{};
    m_vecEffset = TMVector2{};

    m_pLeftGround = 0;
    m_pRightGround = 0;
    m_pUpGround = 0;
    m_pDownGround = 0;
    m_nSeaIndex = 0;
    m_bVisible = 1;
    m_bDungeon = 0;
    m_bWire = 0;
    m_dwLastEffectTime = 0;
    m_dwServertime = 0;
    m_cLeftEnable = 0;
    m_cRightEnable = 0;
    m_cUpEnable = 0;
    m_cDownEnable = 0;
    m_fEffHeight = 2.0f;
    m_dwEffStart = 0;

    // this code is to supress warnings
    memset(&m_MapName, 0, sizeof m_MapName);
    memset(&m_TileMapData, 0, sizeof m_TileMapData);

    for (int i = 0; i < 4; ++i)
    {
        m_vertex[i].diffuse = 0x00FFFFFF;
        m_vertexVoodoo[i].diffuse = 0x00FFFFFF;
    }

    for (int i = 0; i < 128; ++i)
    {
        memset(&m_pMaskData[i], 0, sizeof m_pMaskData[i]);
        memset(&m_pVAttrData[i], 0, sizeof m_pVAttrData[i]);
    }

    for (int i = 0; i < 10; ++i)
        m_pSeaList[i] = 0;

    if (TMGround::m_bFirst == 1)
    {
        FILE* pFile = nullptr;
        fopen_s(&pFile, "cdata.bin", "rb");

        if (pFile)
        {
            fread(&TMGround::m_nCheckSum, sizeof m_nCheckSum, 1u, pFile);

            int nCDataCheckSum = 0;
            for (int k = 0; k < 64; ++k)
                for (int j = 0; j < 32; ++j)
                    nCDataCheckSum += 8 * k + 4 * m_nCheckSum[j][j] * 16 * j;

            if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && nCDataCheckSum != 5855606140)
            {
                fclose(pFile);

                LOG_WRITELOG("DataFile Error\r\n");
                MessageBoxA(g_pApp->m_hWnd, "DataFile Error.", "File Error", 0);
                PostMessageA(g_pApp->m_hWnd, WM_CLOSE, 0, 0);

                return;
            }

            fclose(pFile);
            m_bFirst = 0;
        }
        else
        {
            LOG_WRITELOG("DataFile NotFound");

            if (!g_pCurrentScene->m_bCriticalError)
                g_pCurrentScene->LogMsgCriticalError(4, 0, 0, 0, 0);

            g_pCurrentScene->m_bCriticalError = 1;
            return;
        }
    }

    SetPos(0, 0);
    return;
}

TMGround::~TMGround()
{
}

void TMGround::RestoreDeviceObjects()
{
    SetMiniMapData();
}

void TMGround::SetPos(int nX, int nY)
{
    m_vecOffsetIndex.x = nX;
    m_vecOffsetIndex.y = nY;

    m_vecOffset.x = (m_vecOffsetIndex.x * 2.0f) * 64.0f;
    m_vecOffset.y = (m_vecOffsetIndex.y * 2.0f) * 64.0f;

    if (nY <= 25)
    {
        if (nX >= 8 && nY <= 12 && nY >= 11 && nY <= 14)
        {
            m_bDungeon = 3;
            RenderDevice::m_bDungeon = 3;
        }
        else if (nX > 1 && nX < 11 && nY < 5)
        {
            m_bDungeon = 4;
            RenderDevice::m_bDungeon = 4;
        }
        else if (nX >= 26 && nX <= 30 && nY >= 8 && nY <= 12)
        {
            m_bDungeon = 5;
            RenderDevice::m_bDungeon = 5;
        }
        else
        {
            m_bDungeon = 0;
            RenderDevice::m_bDungeon = 0;
        }
    }
    else
    {
        if (nX < 16 && nX > 8 && nY > 25)
        {
            m_bDungeon = 2;
            RenderDevice::m_bDungeon = 2;
        }
        else
        {
            m_bDungeon = 1;
            RenderDevice::m_bDungeon = 1;
        }
    }
}

int TMGround::LoadTileMap(const char* szFileName)
{
    FILE* fp = nullptr;
    fopen_s(&fp, szFileName, "rb");

    if (fp)
    {
        int bPosX = 0;
        int bPosY = 0;
        const bool validRecord = ReadTerrainTileMapRecord(fp, m_MapName,
            bPosX, bPosY, m_TileMapData);
        fclose(fp);
        if (!validRecord)
        {
            LOG_WRITELOG("Contact Support MAPERROR: invalid terrain record %s\r\n", szFileName);
            return 0;
        }

        SetPos(bPosX, bPosY);
        SetAttatchEnable(bPosX, bPosY);

        // .trn checksum
        /*
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            int nCheckSum = 0;
            int nCheckSize = 49152;
            auto pCheck = (char*)m_TileMapData;

            for (int i = 0; i < nCheckSize; nCheckSum += pCheck[i++])
                ;

            if (m_nCheckSum[bPosY][bPosX] != nCheckSum + bPosX * bPosY)
            {
                LOG_WRITELOG("Contact Support MAPERROR: %d,%d m_nCheckSum=%d nCheckSum= %d\r\n",
                    bPosX, bPosY, m_nCheckSum[bPosY][bPosX], nCheckSum + bPosX * bPosY);

                if (!g_pCurrentScene->m_bCriticalError)
                    g_pCurrentScene->LogMsgCriticalError(5, 0, 0, 0, 0);

                g_pCurrentScene->m_bCriticalError = 1;
                return 0;
            }
        }
        */

        for (int nY = 1; nY < 63; ++nY)
        {
            for (int nX = 1; nX < 63; ++nX)
            {
                m_TileNormalVector[nX + (nY << 6)] = GetNormalInGround(nX, nY);
            }
        }

        for (int nNoIndex = 0; nNoIndex < 64; ++nNoIndex)
        {
            m_TileNormalVector[64 * nNoIndex] = m_TileNormalVector[64 * nNoIndex + 1];
            m_TileNormalVector[(nNoIndex << 6) + 63] = m_TileNormalVector[64 * nNoIndex + 62];
        }

        for (int nNoIndex = 0; nNoIndex < 64; ++nNoIndex)
        {
            m_TileNormalVector[nNoIndex] = m_TileNormalVector[nNoIndex + 64];
            m_TileNormalVector[nNoIndex + 4032] = m_TileNormalVector[nNoIndex + 3968];
        }

        memset(&m_materials, 0, sizeof m_materials);
        m_materials.Diffuse.r = 1.0f;
        m_materials.Diffuse.g = 1.0f;
        m_materials.Diffuse.b = 1.0f;

        m_materials.Specular = m_materials.Diffuse;
        m_materials.Power = 0.0f;

        m_materials.Emissive.r = 0.3f;
        m_materials.Emissive.g = 0.3f;
        m_materials.Emissive.b = 0.3f;

        if (g_pDevice->m_bVoodoo == 1)
        {
            m_materials.Emissive.r = 0.2f;
            m_materials.Emissive.g = 0.2f;
            m_materials.Emissive.b = 0.2f;
        }

        for (int nY = 0; nY < 64; ++nY)
            m_TileMapData[64 * nY].cHeight = m_TileMapData[(nY << 6) + 1].cHeight;

        for (int j = 0; j < 64; ++j)
            m_TileMapData[j].cHeight = m_TileMapData[j + 64].cHeight;

        for (int nY = 0; nY < 64; ++nY)
        {
            for (int j = 0; j < 64; ++j)
            {
                float f1 = 0.0f;
                float f2 = 0.0f;
                float f3 = 0.0f;
                float f4 = 0.0f;

                f1 = m_TileMapData[j + (nY << 6)].cHeight;
                if (nY < 62)
                    f3 = m_TileMapData[j + ((nY + 1) << 6)].cHeight;
                else
                    f3 = m_TileMapData[j + (nY << 6)].cHeight;

                if (j < 62)
                {
                    f2 = m_TileMapData[j + (nY << 6) + 1].cHeight;

                    if (nY >= 62)
                        f4 = m_TileMapData[j + (nY << 6) + 1].cHeight;
                    else
                        f4 = m_TileMapData[j + ((nY + 1) << 6) + 1].cHeight;
                }
                else
                {
                    f2 = m_TileMapData[j + (nY << 6)].cHeight;

                    if (nY >= 62)
                        f4 = m_TileMapData[j + (nY << 6)].cHeight;
                    else
                        f4 = m_TileMapData[j + ((nY + 1) << 6)].cHeight;
                }

                float fCenter = (((f1 + f2) + f3) + f4) / 4.0f;
                m_pMaskData[2 * nY][2 * j] = static_cast<char>((f1 + fCenter) / 2.0f);
                m_pMaskData[2 * nY][2 * j + 1] = static_cast<char>((f2 + fCenter) / 2.0f);
                m_pMaskData[2 * nY + 1][2 * j] = static_cast<char>((f3 + fCenter) / 2.0f);
                m_pMaskData[2 * nY + 1][2 * j + 1] = static_cast<char>((f4 + fCenter) / 2.0f);
            }
        }

        if (!m_cUpEnable)
        {
            for (int X = 0; X < 128; ++X)
            {
                for (int Y = 0; Y < 15; ++Y)
                    m_pMaskData[Y][X] = 127;
            }
        }

        if (!m_cDownEnable)
        {
            for (int k = 0; k < 128; ++k)
            {
                for (int l = 114; l < 128; ++l)
                    m_pMaskData[l][k] = 127;
            }
        }

        if (!m_cLeftEnable)
        {
            for (int m = 0; m < 128; ++m)
            {
                for (int n = 0; n < 15; ++n)
                    m_pMaskData[m][n] = 127;
            }
        }

        if (!m_cRightEnable)
        {
            for (int ii = 0; ii < 128; ++ii)
            {
                for (int jj = 114; jj < 128; ++jj)
                    m_pMaskData[ii][jj] = 127;
            }
        }

        if (m_cLeftEnable == 1 && m_cDownEnable == 1)
        {
            for (int kk = 0; kk < 16; ++kk)
            {
                for (int ll = 113; ll < 128; ++ll)
                    m_pMaskData[ll][kk] = 127;
            }
        }

        if (m_cLeftEnable == 1 && m_cUpEnable == 1)
        {
            for (int mm = 0; mm < 16; ++mm)
            {
                for (int nn = 0; nn < 16; ++nn)
                    m_pMaskData[nn][mm] = 127;
            }
        }

        if (m_cRightEnable == 1 && m_cDownEnable == 1)
        {
            for (int i1 = 113; i1 < 128; ++i1)
            {
                for (int i2 = 113; i2 < 128; ++i2)
                    m_pMaskData[i2][i1] = 127;
            }
        }

        if (m_cRightEnable == 1 && m_cUpEnable == 1)
        {
            for (int i3 = 113; i3 < 128; ++i3)
            {
                for (int i4 = 0; i4 < 16; ++i4)
                    m_pMaskData[i4][i3] = 127;
            }
        }

        return true;
    }
    else
        LOG_WRITELOG("Contact Support MAPERROR: %s\r\n", szFileName);

    return 0;
}

int TMGround::FrameMove(unsigned int dwServerTime)
{
    m_dwServertime = g_pTimerManager->GetServerTime();
    TMObject* pFocusedObject = g_pCurrentScene->m_pMyHuman;

    if (pFocusedObject)
    {
        if (g_pCurrentScene->m_pGround == this)
        {
            int nPosX = static_cast<int>((pFocusedObject->m_vecPosition.x - m_vecOffset.x) + 128.0f) / (512 / TextureManager::DYNAMIC_TEXTURE_WIDTH);
            int nPosY = static_cast<int>((pFocusedObject->m_vecPosition.y - m_vecOffset.y) + 256.0f) / (512 / TextureManager::DYNAMIC_TEXTURE_WIDTH);

            auto pUISet = g_pTextureManager->GetUITextureSet(11);
            if (pUISet)
            {
                pUISet->pTextureCoord->nStartX = nPosX - static_cast<int>(((static_cast<float>(TextureManager::DYNAMIC_TEXTURE_WIDTH) / 8.0f) * m_fMiniMapScale));
                pUISet->pTextureCoord->nStartY = static_cast<int>(((static_cast<float>(TextureManager::DYNAMIC_TEXTURE_HEIGHT - nPosY)) - ((static_cast<float>(TextureManager::DYNAMIC_TEXTURE_HEIGHT) / 8.0f) * TMGround::m_fMiniMapScale)));
                pUISet->pTextureCoord->nWidth = static_cast<int>((static_cast<float>(TextureManager::DYNAMIC_TEXTURE_WIDTH) / 4.0f) * m_fMiniMapScale);
                pUISet->pTextureCoord->nHeight = static_cast<int>((static_cast<float>(TextureManager::DYNAMIC_TEXTURE_HEIGHT) / 4.0f) * m_fMiniMapScale);

                pUISet->pTextureCoord->nStartY -= 4;
            }
        }
    }

    return 1;
}

int TMGround::SetMiniMapData()
{
    for (int nY = 0; nY < 3; ++nY)
    {
        for (int nX = 0; nX < 3; ++nX)
        {
            char szMapName[32] = { 0 };

            sprintf_s(szMapName, "UI\\m%02d%02d.wyt",
                m_vecOffsetIndex.x + nX - 1,
                m_vecOffsetIndex.y + nY - 1);

            int nSrcIndex = g_pTextureManager->GetUITextureIndex(szMapName);
            if (nSrcIndex < 0)
                nSrcIndex = 13;

            g_pTextureManager->GenerateTexture(4, nSrcIndex, nX << 7, (2 - nY) << 7, 0, 0, 128, 128);
        }
    }

    return 1;
}
