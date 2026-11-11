#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "TMScene.h"
#include "TMEffectSkinMesh.h"
#include "HumanAnglePolicy.h"
#include "HumanMovementPolicy.h"
#include "EntitySafetyPolicy.h"
#include "FieldActionPolicy.h"
#include "TMFieldScene.h"
#include "TMUtil.h"
#include "TMObjectContainer.h"

void TMHuman::InitPosition(float fX, float fY, float fZ)
{
    m_pendingMoveTarget.Clear();
    ClearGroundMoveTarget();
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

void TMHuman::SetWantAngle(float fAngle)
{
    if (m_dwDelayDel)
        return;
    if (m_nClass == 44)
        fAngle = D3DXToRadian(180);

    m_fWantAngle = fAngle;
    m_fMoveToAngle = m_fAngle;
}

void TMHuman::SetGroundMoveTarget(IVector2 target)
{
    m_pendingMoveTarget.Clear();
    if (m_groundMovePending && m_groundMoveTarget.x == target.x && m_groundMoveTarget.y == target.y)
        return;
    m_groundMoveTarget = target;
    m_groundMovePending = true;
    m_groundMoveRetarget = true;
}

void TMHuman::ClearGroundMoveTarget()
{
    m_groundMovePending = false;
}

void TMHuman::UpdateGroundMove(unsigned int now)
{
    if (!m_groundMovePending)
        return;

    const auto* scene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (m_cDie || m_cCantMove || m_cSameHeight ||
        m_pMoveTargetHuman || m_pMoveSkillTargetHuman || scene->m_pTargetItem ||
        scene->m_bAutoRun || scene->m_stAutoTrade.TargetID == m_dwID ||
        field_action::AirMoveBlocksAutoRun(scene->m_bAirMove != 0, scene->m_nAirMove_State))
    {
        ClearGroundMoveTarget();
        return;
    }

    const auto target = m_groundMoveTarget;
    const bool atDestination = static_cast<int>(m_vecPosition.x) == target.x &&
        static_cast<int>(m_vecPosition.y) == target.y;
    if (!m_bMoveing && atDestination)
    {
        ClearGroundMoveTarget();
        return;
    }

    const bool reachesDestination = m_vecTargetPos.x == target.x && m_vecTargetPos.y == target.y;
    if (!ground_route::MayReplan(m_bMoveing != 0, m_fProgressRate, m_groundMoveRetarget,
        m_nLastRouteIndex >= m_nMaxRouteIndex - 1, reachesDestination))
        return;

    if ((!m_dwOldMovePacketTime ||
         human_timer::Elapsed(now, m_dwOldMovePacketTime, client_send_cadence::GroundActionInterval)) &&
        !GetRoute(target, 0, 0, true))
        ClearGroundMoveTarget();
}

bool TMHuman::GetRoute(IVector2 vecTarget, int nCount, int bStop, bool groundMove)
{
    if (!g_pCurrentScene)
        return false;
    const bool localField = this == g_pCurrentScene->m_pMyHuman &&
        g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD;
    if (groundMove && !localField)
        return false;
    if (localField && !groundMove)
        ClearGroundMoveTarget();

    if (g_pCurrentScene && this == g_pCurrentScene->m_pMyHuman &&
        (bStop == 2 || (vecTarget.x == m_LastSendTargetPos.x && vecTarget.y == m_LastSendTargetPos.y)))
        m_pendingMoveTarget.Clear();
    if (!m_dwDelayDel && (vecTarget.x != m_LastSendTargetPos.x || vecTarget.y != m_LastSendTargetPos.y))
    {
        int nStartRouteIndex = m_nLastRouteIndex;
        if (m_fProgressRate > 0.5f)
            nStartRouteIndex = m_nLastRouteIndex + 1;
        int nSX = (int)m_vecRouteBuffer[nStartRouteIndex].x;
        int nSY = (int)m_vecRouteBuffer[nStartRouteIndex].y;
        const unsigned int actionInterval = groundMove ? client_send_cadence::GroundActionInterval :
            client_send_cadence::LegacyActionInterval;
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

        // Elapsed comparisons keep these transition gates working across a
        // server-tick wrap; absolute `stamp + 6000` deadlines did not.
        if ((this != g_pCurrentScene->m_pMyHuman || g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD ||
            (transition_gate::HasElapsed(dwTime, g_dwStartQuitGameTime, 6000)
            && transition_gate::HasElapsed(dwTime, pFScene->m_dwLastLogout, 6000)
            && transition_gate::HasElapsed(dwTime, pFScene->m_dwLastSelServer, 6000)
            && transition_gate::HasElapsed(dwTime, pFScene->m_dwLastTown, 6000)
            && transition_gate::HasElapsed(dwTime, pFScene->m_dwLastTeleport, 6000))))
        {
            if (localField && bStop != 2 && m_dwOldMovePacketTime &&
                !human_timer::Elapsed(dwServerTime, m_dwOldMovePacketTime, actionInterval))
            {
                // Ground input owns its destination across segments.
                if (!groundMove)
                    m_pendingMoveTarget.Set(vecTarget.x, vecTarget.y, nCount, bStop);
                return true;
            }
            if (groundMove || dwServerTime - m_dwOldMovePacketTime > dwDealyTime || bStop)
            {
                if (localField) m_pendingMoveTarget.Clear();
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
                            TMHuman* pObj = g_pCurrentScene->m_pMyHuman;

                            if (pObj == this
                                && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
                                && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO)
                            {
                                MSG_Action dst{};

                                dst.Header.ID = m_dwID;
                                dst.PosX = nSX;
                                dst.PosY = nSY;
                                dst.Effect = 0;
                                dst.Header.Type = MSG_Action_Opcode;
                                dst.Speed = g_nMyHumanSpeed;
                                dst.TargetX = vecTarget.x;
                                dst.TargetY = vecTarget.y;

                                if (bStop != 2)
                                {
                                    if (!SendOneMessage(reinterpret_cast<char*>(&dst), sizeof(dst)))
                                    {
                                        m_pendingMoveTarget.Set(vecTarget.x, vecTarget.y, nCount, bStop);
                                        return true;
                                    }
                                    pFScene->m_stMoveStop.LastX = dst.PosX;
                                    pFScene->m_stMoveStop.LastY = dst.PosY;
                                    pFScene->m_stMoveStop.NextX = dst.TargetX;
                                    pFScene->m_stMoveStop.NextY = dst.TargetY;
                                    g_bLastStop = dst.Header.Type;
                                }
                                m_cLastMoveStop = 1;
                                m_LastSendTargetPos.x = vecTarget.x;
                                m_LastSendTargetPos.y = vecTarget.y;
                                g_pCurrentScene->OnPacketEvent(MSG_Action_Opcode, (char*)&dst);

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

                        if (groundMove &&
                            ((m_LastSendTargetPos.x == vecTarget.x && m_LastSendTargetPos.y == vecTarget.y) ||
                             (std::max)(abs(nTX - vecTarget.x), abs(nTY - vecTarget.y)) >=
                             (std::max)(abs(nTX - nSX), abs(nTY - nSY))))
                            return false;

                        for (int k = nRouteIndex + 1; k < 48; ++k)
                        {
                            vecRouteTable[k].x = 0.0f;
                            vecRouteTable[k].y = 0.0f;
                        }

                        if ((vecTarget.x != nSX || vecTarget.y != nSY) && (vecTarget.x != m_vecTargetPos.x || vecTarget.y != m_vecTargetPos.y || bStop))
                        {
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
                                stAction.Route[j] = j < nRouteIndex ? cRouteBuffer[j] : 0;

                            if (m_LastSendTargetPos.x != vecTarget.x || m_LastSendTargetPos.y != vecTarget.y)
                            {
                                if (g_pCurrentScene->m_pMyHuman == this
                                    && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
                                    && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO
                                    && (localField || dwServerTime - m_dwOldMovePacketTime > actionInterval)
                                    && bStop != 2)
                                {
                                    if (!SendOneMessage(reinterpret_cast<char*>(&stAction), sizeof(stAction), actionInterval))
                                    {
                                        if (!groundMove)
                                            m_pendingMoveTarget.Set(vecTarget.x, vecTarget.y, nCount, bStop);
                                        return true;
                                    }
                                    pFScene->m_stMoveStop.LastX = stAction.PosX;
                                    pFScene->m_stMoveStop.LastY = stAction.PosY;
                                    pFScene->m_stMoveStop.NextX = stAction.TargetX;
                                    pFScene->m_stMoveStop.NextY = stAction.TargetY;
                                    g_bLastStop = stAction.Header.Type;

                                    m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
                                    m_LastSendTargetPos.x = stAction.TargetX;
                                    m_LastSendTargetPos.y = stAction.TargetY;
                                }
                                m_cLastMoveStop = bStop;
                            }
                            memset(m_cRouteBuffer, 0, sizeof(m_cRouteBuffer));
                            memcpy(m_cRouteBuffer, cRouteBuffer, nRouteIndex);
                            if (localField) pFScene->m_vecMyNext = vecTarget;
                            if (groundMove) m_cOnlyMove = 0;
                            OnPacketEvent(MSG_Action_Opcode, (char*)&stAction);
                            if (groundMove)
                            {
                                // The route table pads with copies of its endpoint.
                                // End the segment at its last real step.
                                m_nMaxRouteIndex = static_cast<int>(strlen(stAction.Route));
                                m_nLastRouteIndex = 0;
                                m_fProgressRate = 0.0f;
                                m_groundMoveRetarget = false;
                            }
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
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

int TMHuman::ChangeRouteBuffer(int nSX, int nSY, TMVector2* pRouteTable, int* pMaxRouteIndex)
{
    if (!g_pCurrentScene || !pRouteTable)
        return 0;

    char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;
    int nRoutCount = -1;
    // Keep the bridge from the actor's cell to the furthest reachable route
    // cell; a second, empty buffer made the actor jump to that cell.
    char szBridge[48]{};

    for (int i = 46; i >= 0; --i)
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
                memcpy(szBridge, szBuffer, sizeof(szBridge));
                break;
            }
        }
    }

    if (nRoutCount < 0 || nRoutCount > 46)
        return 0;

    TMVector2 vecBridge[48]{};
    int nBridgeMaxIndex = 0;
    GenerateRouteTable(nSX, nSY, szBridge, vecBridge, &nBridgeMaxIndex);

    TMVector2 vecRouteBuffer[48]{};
    const int nMaxRouteIndex = route_bridge::Compose(vecBridge, nBridgeMaxIndex, pRouteTable, nRoutCount,
        pMaxRouteIndex ? *pMaxRouteIndex : 0, vecRouteBuffer);
    if (nMaxRouteIndex <= 0)
        return 0;

    memcpy(pRouteTable, vecRouteBuffer, sizeof(vecRouteBuffer));
    if (pMaxRouteIndex)
        *pMaxRouteIndex = nMaxRouteIndex;
    return 1;
}

void TMHuman::SetSpeed(int bMountDead)
{
    m_fMaxSpeed = (float)BASE_GetSpeed(&m_stScore);
    if (g_pCurrentScene->m_pMyHuman == this)
        g_nMyHumanSpeed = (int)m_fMaxSpeed;
}
