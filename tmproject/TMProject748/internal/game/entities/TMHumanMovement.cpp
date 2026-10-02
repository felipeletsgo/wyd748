#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "TMScene.h"
#include "TMEffectSkinMesh.h"
#include "HumanAnglePolicy.h"
#include "TMFieldScene.h"
#include "TMUtil.h"
#include "TMObjectContainer.h"

void TMHuman::InitPosition(float fX, float fY, float fZ)
{
    if (m_dwDelayDel != 0)
        return;

    SetPosition(fX, fY, fZ);
    m_fWantHeight = fY;

    m_vecMoveToPos = m_vecPosition;
    m_vecFromPos = m_vecPosition;

    m_vecTargetPos.x = (int)m_vecPosition.x;
    m_vecTargetPos.y = (int)m_vecPosition.y;

    m_vecDPosition = TMVector2(0.0f, 0.0f);

    for (int i = 0; i < 48; i++)
        m_vecRouteBuffer[i] = m_vecPosition;

    m_LastSendTargetPos = m_vecTargetPos;
}

void TMHuman::InitAngle(float fYaw, float fPitch, float fRoll)
{
    if (m_dwDelayDel != 0)
        return;

    if (m_nClass == 44)
        fPitch = D3DXToRadian(180);

    TMObject::InitAngle(fYaw, fPitch, fRoll);
    m_fWantAngle = m_fAngle;
    m_fMoveToAngle = m_fAngle;
}

void TMHuman::SetAngle(float fYaw, float fPitch, float fRoll)
{
    if (m_dwDelayDel != 0)
        return;

    m_fAngle = fPitch;
    if (m_cMount == 0)
    {
        if (m_pSkinMesh)
            m_pSkinMesh->SetAngle(fYaw, human_angle::MeshPitch(m_nWeaponTypeL, fPitch, false), fRoll);
    }
    else
    {
        if (m_pMount)
            m_pMount->SetAngle(fYaw, human_angle::MeshPitch(m_nWeaponTypeL, fPitch, true), fRoll);
        if (m_pSkinMesh)
            m_pSkinMesh->SetAngle(0.0f, 0.0f, 0.0f);
    }
}

void TMHuman::SetPosition(float fX, float fY, float fZ)
{
    if (m_dwDelayDel != 0)
        return;

    m_vecPosition.x = fX;
    m_vecPosition.y = fZ;
    m_fHeight = fY;

    float fHSize = (TMHuman::m_vecPickSize[m_nSkinMeshType].x * m_fScale) / 2.0f;
    if (m_cMount)
        fHSize = (TMHuman::m_vecPickSize[m_nMountSkinMeshType].x * m_fScale) / 2.0f;

    float fDX = cosf(m_fAngle) * fHSize;
    float fDY = sinf(m_fAngle) * fHSize;

    if (m_cMount == 0)
    {
        if (m_nClass == 44)
            fDX = fDX + 0.5f;
        if (m_pSkinMesh)
            m_pSkinMesh->SetPosition(fX + fDX, fY, fZ - fDY);
    }
    else
    {
        if (m_pMount)
            m_pMount->SetPosition(fX + fDX, fY, fZ - fDY);
        if (m_pSkinMesh)
            m_pSkinMesh->SetPosition(0.0f, 0.0f, 0.0f);
    }
}

void TMHuman::MoveTo(TMVector2 vecPos)
{
    if (!m_dwDelayDel && (vecPos.x != m_vecPosition.x || vecPos.y != m_vecPosition.y))
    {
        TMVector2 dPosition = vecPos - m_vecPosition;
        m_vecMoveToPos = vecPos;

        m_fWantAngle = (float)atan2f(dPosition.x, dPosition.y) + D3DXToRadian(90);

        if (m_fAngle < 0.0f)
            m_fAngle = m_fAngle + D3DXToRadian(360);
        else if (m_fAngle > D3DXToRadian(360))
            m_fAngle = m_fAngle - D3DXToRadian(360);

        m_fMoveToAngle = m_fAngle;
        if ((float)(m_fWantAngle - m_fMoveToAngle) < 0.0f && (float)(m_fWantAngle - m_fMoveToAngle) < -D3DXToRadian(180))
            m_fWantAngle = m_fWantAngle + D3DXToRadian(360);
        else if ((float)(m_fWantAngle - m_fMoveToAngle) > 0.0f && (float)(m_fWantAngle - m_fMoveToAngle) > D3DXToRadian(180))
            m_fWantAngle = m_fWantAngle - D3DXToRadian(360);

        m_vecFromPos = m_vecPosition;
        m_vecDPosition = m_vecMoveToPos - m_vecPosition;
        float fDistance = m_vecDPosition.DistanceFrom(TMVector2(0.0f, 0.0f));
        m_dwMoveToTime = g_pTimerManager->GetServerTime();
    }
}

void TMHuman::OnlyMove(int nX, int nY, int nLocal)
{
    if (m_dwDelayDel)
        return;

    if (m_vecTargetPos.x == nX && m_vecTargetPos.y == nY)
        return;

    MSG_Action stAction{};
    stAction.Header.ID = m_dwID;
    stAction.PosX = nX;
    stAction.PosY = nY;
    stAction.Effect = 0;
    stAction.Header.Type = MSG_Action_Opcode;
    if (g_pCurrentScene->m_pMyHuman == this)
        stAction.Speed = g_nMyHumanSpeed;
    else
        stAction.Speed = (int)m_fMaxSpeed;

    stAction.TargetX = nX;
    stAction.TargetY = nY;

    if (!nLocal || nLocal == 2)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pFScene->m_stMoveStop.LastX = stAction.PosX;
        pFScene->m_stMoveStop.LastY = stAction.PosY;
        pFScene->m_stMoveStop.NextX = stAction.TargetX;
        pFScene->m_stMoveStop.NextY = stAction.TargetY;
        SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAction)->Type, reinterpret_cast<char*>(&stAction), sizeof(stAction)});
    }
    if (!nLocal || nLocal == 1)
        OnPacketEvent(876, (char*)&stAction);
}

int TMHuman::IsGoMore()
{
    if (m_dwDelayDel)
        return 0;

    if (m_vecRouteBuffer[m_nLastRouteIndex].x != m_vecRouteBuffer[m_nLastRouteIndex + 1].x ||
        m_vecRouteBuffer[m_nLastRouteIndex].y != m_vecRouteBuffer[m_nLastRouteIndex + 1].y)
    {
        return 1;
    }

    return 0;
}

void TMHuman::SetWantAngle(float fAngle)
{
    if (m_dwDelayDel)
        return;
    if (m_nClass == 44)
        fAngle = D3DXToRadian(180);

    m_fWantAngle = fAngle;
    m_fMoveToAngle = m_fAngle;
}

void TMHuman::GetRoute(IVector2 vecTarget, int nCount, int bStop)
{
    if (!m_dwDelayDel && (vecTarget.x != m_LastSendTargetPos.x || vecTarget.y != m_LastSendTargetPos.y))
    {
        int nStartRouteIndex = m_nLastRouteIndex;
        if (m_fProgressRate > 0.5f)
            nStartRouteIndex = m_nLastRouteIndex + 1;
        int nSX = (int)m_vecRouteBuffer[nStartRouteIndex].x;
        int nSY = (int)m_vecRouteBuffer[nStartRouteIndex].y;
        unsigned int dwDealyTime = 1000;
        unsigned int dwServerTime = g_pTimerManager->GetServerTime();

        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 4)
            dwDealyTime = 500;
        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 5)
            dwDealyTime = 500;
        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 6)
            dwDealyTime = 100;
        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 7)
            dwDealyTime = 100;

        unsigned int dwTime = g_pTimerManager->GetServerTime();
        TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;

        if ((this != g_pCurrentScene->m_pMyHuman || g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD ||
            (dwTime >= g_dwStartQuitGameTime + 6000
            && dwTime >= pFScene->m_dwLastLogout + 6000
            && dwTime >= pFScene->m_dwLastSelServer + 6000
            && dwTime >= pFScene->m_dwLastTown + 6000
            && dwTime >= pFScene->m_dwLastTeleport + 6000)))
        {
            if (dwServerTime - m_dwOldMovePacketTime > dwDealyTime || bStop)
            {
                char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;
                int nTX = vecTarget.x;
                int nTY = vecTarget.y;

                char cRouteBuffer[48]{};

                TMScene* pScene = g_pCurrentScene;
                float fHeight = (float)pScene->GroundGetMask(m_vecPosition);
                int nMaxRoute = 12;

                if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_DEMO)
                    nMaxRoute = 16;

                BASE_GetRoute(nSX, nSY, &vecTarget.x, &vecTarget.y, cRouteBuffer, nMaxRoute, pHeightMapData, 8);

                int nStart2 = pScene->GroundGetMask(TMVector2((float)m_LastSendTargetPos.x, (float)m_LastSendTargetPos.y));
                int nStart = pScene->GroundGetMask(TMVector2((float)nSX, (float)nSY));
                int nEnd = pScene->GroundGetMask(TMVector2((float)vecTarget.x, (float)vecTarget.y));

                int nHeight = abs(nEnd - nStart);
                int nHeight2 = abs(nEnd - nStart2);

                if (nHeight2 <= 30 || !m_LastSendTargetPos.x || !m_LastSendTargetPos.y)
                {
                    if (nHeight > 30)
                    {
                        memset(cRouteBuffer, 0, sizeof(cRouteBuffer));
                        BASE_GetRoute(nSX, nSY, &vecTarget.x, &vecTarget.y, cRouteBuffer, nMaxRoute / 2, pHeightMapData, 8);

                        nEnd = pScene->GroundGetMask(TMVector2((float)vecTarget.x, (float)vecTarget.y));
                        nHeight = abs(nEnd - nStart);
                        if (nHeight > 30)
                        {
                            memset(cRouteBuffer, 0, sizeof(cRouteBuffer));
                            BASE_GetRoute(nSX, nSY, &vecTarget.x, &vecTarget.y, cRouteBuffer, nMaxRoute / 4, pHeightMapData, 8);

                            nEnd = pScene->GroundGetMask(TMVector2((float)vecTarget.x, (float)vecTarget.y));
                            nHeight = abs(nEnd - nStart);
                        }
                    }
                    if ((int)m_vecPosition.x == vecTarget.x
                     && (int)m_vecPosition.y == vecTarget.y)
                    {
                        if (bStop && (m_LastSendTargetPos.x != vecTarget.x || m_LastSendTargetPos.y != vecTarget.y))
                        {
                            m_cLastMoveStop = 1;
                            TMHuman* pObj = g_pCurrentScene->m_pMyHuman;

                            if (pObj == this
                                && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
                                && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO)
                            {
                                m_LastSendTargetPos.x = vecTarget.x;
                                m_LastSendTargetPos.y = vecTarget.y;

                                MSG_Action dst{};

                                dst.Header.ID = m_dwID;
                                dst.PosX = nSX;
                                dst.PosY = nSY;
                                dst.Effect = 0;
                                dst.Header.Type = MSG_Action_Opcode;
                                dst.Speed = g_nMyHumanSpeed;
                                dst.TargetX = vecTarget.x;
                                dst.TargetY = vecTarget.y;

                                g_pCurrentScene->OnPacketEvent(MSG_Action_Opcode, (char*)&dst);

                                if (bStop != 2)
                                {
                                    pFScene->m_stMoveStop.LastX = dst.PosX;
                                    pFScene->m_stMoveStop.LastY = dst.PosY;
                                    pFScene->m_stMoveStop.NextX = dst.TargetX;
                                    pFScene->m_stMoveStop.NextY = dst.TargetY;
                                    SendOneMessage((char*)&dst, 52);
                                    g_bLastStop = dst.Header.Type;
                                }

                                for (int i = 0; i < 48; ++i)
                                {
                                    m_vecRouteBuffer[i].x = (float)nSX + 0.5f;
                                    m_vecRouteBuffer[i].y = (float)nSY + 0.5f;
                                }

                                m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
                            }
                        }
                    }
                    else if (cRouteBuffer[0] != 0)
                    {
                        TMVector2 vecRouteTable[48]{};
                        int nRouteLen;
                        GenerateRouteTable(nSX, nSY, cRouteBuffer, vecRouteTable, &nRouteLen);

                        bool bFind = false;
                        int nRouteIndex = nRouteLen;
                        if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO)
                        {
                            for (nRouteIndex = nRouteLen; nRouteIndex > 0; --nRouteIndex)
                            {
                                bFind = false;
                                for (TMHuman* pNode = (TMHuman*)pScene->m_pHumanContainer->m_pDown; pNode && pNode->m_pNextLink; pNode = (TMHuman*)pNode->m_pNextLink)
                                {
                                    int nX = (int)pNode->m_vecRouteBuffer[47].x;
                                    int nY = (int)pNode->m_vecRouteBuffer[47].y;

                                    if ((int)vecRouteTable[nRouteIndex].x == nX && (int)vecRouteTable[nRouteIndex].y == nY && !pNode->m_cDie)
                                    {
                                        bFind = true;
                                        break;
                                    }
                                }

                                if (!bFind)
                                    break;
                            }
                        }

                        vecTarget.x = (int)vecRouteTable[nRouteIndex].x;
                        vecTarget.y = (int)vecRouteTable[nRouteIndex].y;

                        for (int k = nRouteIndex + 1; k < 48; ++k)
                        {
                            vecRouteTable[k].x = 0.0f;
                            vecRouteTable[k].y = 0.0f;
                        }

                        if (g_pCurrentScene->m_pMyHuman == this && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
                            pFScene->m_vecMyNext = vecTarget;

                        if ((vecTarget.x != nSX || vecTarget.y != nSY) && (vecTarget.x != m_vecTargetPos.x || vecTarget.y != m_vecTargetPos.y || bStop))
                        {
                            memset(m_cRouteBuffer, 0, sizeof(m_cRouteBuffer));
                            memcpy(m_cRouteBuffer, cRouteBuffer, nRouteIndex);

                            MSG_Action stAction{};
                            stAction.Header.ID = m_dwID;
                            stAction.PosX = nSX;
                            stAction.PosY = nSY;
                            stAction.Effect = 0;

                            if (bStop)
                                stAction.Header.Type = MSG_Action_Stop_Opcode;
                            else
                                stAction.Header.Type = MSG_Action_Opcode;

                            if (g_pCurrentScene->m_pMyHuman == this)
                                stAction.Speed = g_nMyHumanSpeed;
                            else
                                stAction.Speed = (int)m_fMaxSpeed;

                            stAction.TargetX = vecTarget.x;
                            stAction.TargetY = vecTarget.y;

                            for (int j = 0; j < 23; ++j)
                                stAction.Route[j] = m_cRouteBuffer[j % 24];

                            if (m_LastSendTargetPos.x != vecTarget.x || m_LastSendTargetPos.y != vecTarget.y)
                            {
                                if (m_cLastMoveStop == bStop && bStop == 1)
                                    return;

                                m_cLastMoveStop = bStop;
                                if (g_pCurrentScene->m_pMyHuman == this
                                    && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
                                    && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO
                                    && dwServerTime - m_dwOldMovePacketTime > 1000
                                    && bStop != 2)
                                {
                                    pFScene->m_stMoveStop.LastX = stAction.PosX;
                                    pFScene->m_stMoveStop.LastY = stAction.PosY;
                                    pFScene->m_stMoveStop.NextX = stAction.TargetX;
                                    pFScene->m_stMoveStop.NextY = stAction.TargetY;
                                    SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAction)->Type, reinterpret_cast<char*>(&stAction), sizeof(stAction)});
                                    g_bLastStop = stAction.Header.Type;

                                    m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
                                    m_LastSendTargetPos.x = stAction.TargetX;
                                    m_LastSendTargetPos.y = stAction.TargetY;
                                }
                            }

                            OnPacketEvent(MSG_Action_Opcode, (char*)&stAction);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void TMHuman::GenerateRouteTable(int nSX, int nSY, char* pRouteBuffer, TMVector2* pRouteTable, int* pMaxRouteIndex)
{
    if (m_dwDelayDel != 0)
        return;

    TMVector2 vecCurrent = TMVector2((float)nSX + 0.5f, (float)nSY + 0.5f);
    pRouteTable[0] = vecCurrent;

    if (pMaxRouteIndex)
        *pMaxRouteIndex = strlen(pRouteBuffer) + 2;

    for (int i = 1; i < 48; ++i)
    {
        pRouteTable[i] = vecCurrent;
        switch (pRouteBuffer[i - 1])
        {
        case '6':
            pRouteTable[i].x = pRouteTable[i].x + 1.0f;
            break;
        case '4':
            pRouteTable[i].x = pRouteTable[i].x - 1.0f;
            break;
        case '8':
            pRouteTable[i].y = pRouteTable[i].y + 1.0f;
            break;
        case '2':
            pRouteTable[i].y = pRouteTable[i].y - 1.0f;
            break;
        case '3':
            pRouteTable[i].x = pRouteTable[i].x + 1.0f;
            pRouteTable[i].y = pRouteTable[i].y - 1.0f;
            break;
        case '1':
            pRouteTable[i].x = pRouteTable[i].x - 1.0f;
            pRouteTable[i].y = pRouteTable[i].y - 1.0f;
            break;
        case '9':
            pRouteTable[i].x = pRouteTable[i].x + 1.0f;
            pRouteTable[i].y = pRouteTable[i].y + 1.0f;
            break;
        case '7':
            pRouteTable[i].x = pRouteTable[i].x - 1.0f;
            pRouteTable[i].y = pRouteTable[i].y + 1.0f;
            break;
        }
        vecCurrent = pRouteTable[i];
    }

    //std::cout << "Current " << vecCurrent.x << " " << vecCurrent.y << '\n';
}

int TMHuman::StraightRouteTable(int nSX, int nSY, int nTargetX, int nTargetY, TMVector2* pRouteTable, int* pMaxRouteIndex, int distance, char* pHeight, int MH)
{
    if (m_dwDelayDel)
        return 0;

    if (nSX == nTargetX && nSY == nTargetY)
        return 0;

    TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;

    // This is probably e ifndef of debug
    int a;
    if (g_pCurrentScene->m_pMyHuman == this)
        a = 0;

    int nDis = BASE_GetDistance(nSX, nSY, nTargetX, nTargetY);

    TMVector2 vecCurrent;
    // TODO: verify this vecposition.y
    if ((int)m_vecPosition.y == nTargetX && (int)m_vecPosition.y == nTargetY)
        return 0;

    vecCurrent.x = m_vecPosition.x;
    vecCurrent.y = m_vecPosition.y;

    D3DXVECTOR2 NorVec2;
    auto pV = D3DXVECTOR2((float)(nTargetX - nSX), (float)(nTargetY - nSY));
    D3DXVec2Normalize(&NorVec2, &pV);

    auto pV2 = D3DXVECTOR2((float)((float)nTargetX + 0.5f) - vecCurrent.x,
        (float)((float)nTargetY + 0.5f) - vecCurrent.y);
    float TargetLen = D3DXVec2Length(&pV2);

    pRouteTable[0] = vecCurrent;

    float NowLen = 0.0f;
    int nMax = 48;
    int nSXa = (int)pRouteTable->x;
    int nSYa = (int)pRouteTable->y;

    int nPlusX = 1;
    if (nTargetX - nSXa < 0)
        nPlusX = -1;

    int nPlusY = 1;
    if (nTargetY - nSYa < 0)
        nPlusY = -1;

    int nMinX = nSXa <= nTargetX ? nSXa : nTargetX;
    int nMinY = nSYa <= nTargetY ? nSYa : nTargetY;
    int nMaxX = nSXa <= nTargetX ? nTargetX : nSXa;
    int nMaxY = nSYa <= nTargetY ? nTargetY : nSYa;

    int Cul = pHeight[nSXa + g_HeightWidth * (nSYa - g_HeightPosY) - g_HeightPosX];

    for (int nY = nMinY; nY < nMaxY; ++nY)
    {
        for (int nX = nMinX; nX < nMaxX; ++nX)
        {
            int nH = pHeight[nX + g_HeightWidth * (nY - g_HeightPosY) - g_HeightPosX];
            if (abs(nH - Cul) > MH)
                return 0;
        }
    }

    for (int i = 1; i < 48; ++i)
    {
        pRouteTable[i] = vecCurrent;
        if (nMax > i)
        {
            float fy = (float)nDis;
            float value = TargetLen;

            pRouteTable[i] += ((TMVector2(NorVec2.x, NorVec2.y) * value) / fy);

            int nSXb = (int)pRouteTable[i].x;
            int nSYb = (int)pRouteTable[i].y;

            if (nSXb != (int)pRouteTable[i - 1].x || pRouteTable[i].y != pRouteTable[i - 1].y)
            {
                Cul = pHeight[nSXb + g_HeightWidth * (nSYb - g_HeightPosY) - g_HeightPosX];
                if ((int)pRouteTable[0].x != nSXb - nPlusX)
                {
                    int CulX = pHeight[nSXb + g_HeightWidth * (nSYb - g_HeightPosY) - nPlusX - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulX))
                        return 0;
                }
                if ((int)pRouteTable[0].y != nSYb - nPlusY)
                {
                    int CulY = pHeight[nSXb + g_HeightWidth * (nSYb - nPlusY - g_HeightPosY) - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulY))
                        return 0;
                }
                if ((int)pRouteTable[0].y != nSYb - nPlusY && (int)pRouteTable[0].x != nSXb - nPlusX)
                {
                    int CulXY = pHeight[nSXb + g_HeightWidth * (nSYb - nPlusY - g_HeightPosY) - nPlusX - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulXY))
                        return 0;
                }

                if (i)
                {
                    int CulBack = pHeight[(int)pRouteTable[i - 1].x
                        + g_HeightWidth * ((int)pRouteTable[i - 1].y - g_HeightPosY)
                        - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulBack))
                        return 0;
                }
            }

            if (i >= nDis)
            {
                nMax = i;
                pRouteTable[i].x = (float)nTargetX + 0.5f;
                pRouteTable[i].y = (float)nTargetY + 0.5f;
            }
        }

        vecCurrent = pRouteTable[i];
    }

    if (pMaxRouteIndex)
        *pMaxRouteIndex = nMax;

    m_cSameHeight = 2;
    return 1;
}

int TMHuman::ChangeRouteBuffer(int nSX, int nSY, TMVector2* pRouteTable, int* pMaxRouteIndex)
{
    char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;
    int nRoutCount = 0;

    int i;
    for (i = 46; i >= 0; --i)
    {
        if (pRouteTable[i].x != pRouteTable[i + 1].x ||
            pRouteTable[i].y != pRouteTable[i + 1].y)
        {
            int tX = (int)pRouteTable[i].x;
            int tY = (int)pRouteTable[i].y;

            char szBuffer[48]{};

            BASE_GetRoute(nSX, nSY, &tX, &tY, szBuffer, 12, pHeightMapData, 8);
            if (tX == (int)pRouteTable[i].x && tY == (int)pRouteTable[i].y)
            {
                nRoutCount = i;
                break;
            }
        }
    }

    if (i < 0 || i > 46)
        return 0;

    if (nRoutCount < 0 || nRoutCount > 46)
        return 0;

    TMVector2 vecRouteBuffer[48]{};
    TMVector2 vecCurrent;
    int nMaxRouteIndex;
    char szBuffer[48]{};
    GenerateRouteTable(nSX, nSY, szBuffer, vecRouteBuffer, &nMaxRouteIndex);

    if (nMaxRouteIndex - 2 < 0)
        return 0;

    vecCurrent = vecRouteBuffer[nMaxRouteIndex - 2];
    for (i = nMaxRouteIndex - 2; i < 48; ++i)
    {
        vecRouteBuffer[i] = vecCurrent;
        if (i + nRoutCount - (nMaxRouteIndex - 2) < 48)
           vecRouteBuffer[i] = pRouteTable[i + nRoutCount - (nMaxRouteIndex - 2)];

        vecCurrent = vecRouteBuffer[i];
    }

    memcpy(pRouteTable, vecRouteBuffer, sizeof(vecRouteBuffer));
    return 1;
}

void TMHuman::SetSpeed(int bMountDead)
{
    m_fMaxSpeed = (float)BASE_GetSpeed(&m_stScore);
    if (g_pCurrentScene->m_pMyHuman == this)
        g_nMyHumanSpeed = (int)m_fMaxSpeed;
}
