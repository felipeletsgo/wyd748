#include "pch.h"
#include "../../application/BoundedText.h"
#include "TMFieldScene.h"
#include "../../game/entities/DeathMotionPolicy.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMUtil.h"
#include "TMCamera.h"
#include "TMEffectBillBoard2.h"
#include "TMSkillTownPortal.h"
#include "../../application/FieldInteractionPolicy.h"
#include "../../game/entities/AirMoveMotion.h"
#include "ClientDiagnostics.h"
#include "TMObjectContainer.h"
#include "TMSkinMesh.h"
#include "TMEffectParticle.h"

void TMFieldScene::UpdateAutoRun()
{
	if (!m_pMyHuman || !m_pGround || !g_pObjectManager || !g_pObjectManager->m_pCamera)
		return;
	if (!field_interaction::CanAutoRun(m_bAutoRun != 0,
		g_pObjectManager->m_stMobData.CurrentScore.CurHP > 0 && !m_pMyHuman->m_cDie,
		m_bAirMove != 0 || m_nAirMove_State != 0, m_pMyHuman->m_cCantMove != 0,
		m_stAutoTrade.TargetID == m_pMyHuman->m_dwID ||
			(m_pMessageBox && m_pMessageBox->IsVisible()), m_pMyHuman->m_fProgressRate))
		return;

	m_pMyHuman->SetSpeed(m_bMountDead);
	const auto direction = g_pObjectManager->m_pCamera->GetCameraLookatDir();
	const auto target = field_interaction::AutoRunDestination(m_pMyHuman->m_vecPosition.x,
		m_pMyHuman->m_vecPosition.y, direction.x, direction.z,
		m_pMyHuman->m_fMaxSpeed, m_bReverse != 0);
	// Check bounds before BASE_GetAttr, which assumes valid map coordinates.
	if (!field_interaction::AutoRunCellAllowed(target.x, target.y, 0) ||
		!field_interaction::AutoRunCellAllowed(target.x, target.y, BASE_GetAttr(target.x, target.y)))
		return;
	m_vecMyNext = {target.x, target.y};
	// Reuse normal route validation, transition gates and packet throttling.
	// The same movement intent remains subject to server validation.
	m_pMyHuman->GetRoute(m_vecMyNext, 0, 0);
	m_pTargetHuman = nullptr;
	m_pMyHuman->m_pMoveTargetHuman = nullptr;
	m_pMyHuman->m_pMoveSkillTargetHuman = nullptr;
	m_pTargetItem = nullptr;
	m_bMoveing = 1;
}

int TMFieldScene::UpdateTeleportPrompt()
{
	if (!m_pMyHuman)
		return 0;

	// This is the native 7.48 FUN_004776C3 portal slice: sample the cell,
	// normalize it to the four-unit portal table grid, then gate the prompt on
	// the stand motion and the 0x10 terrain attribute.
	const auto bAttr = BASE_GetAttr((int)m_pMyHuman->m_vecPosition.x,
		(int)m_pMyHuman->m_vecPosition.y);
	int nPosIndex = -1;
	const int x = (int)m_pMyHuman->m_vecPosition.x & 0xFFFC;
	const int y = (int)m_pMyHuman->m_vecPosition.y & 0xFFFC;

	for (int ll = 0; ll < _countof(g_TeleportTable); ++ll)
	{
		if (g_TeleportTable[ll].nX == x && g_TeleportTable[ll].nY == y)
		{
			nPosIndex = ll;
			break;
		}
	}
	const bool hasPortalIndex = nPosIndex >= 0 &&
		nPosIndex < static_cast<int>(_countof(g_TeleportTable));
	const bool hasLastPortal = m_nLastPotal >= 0 &&
		m_nLastPotal < static_cast<int>(_countof(g_TeleportTable));
	const auto messageAt = [](int index) -> char*
	{
		if (index < 0 || index >= static_cast<int>(_countof(g_pMessageStringTable)))
			return nullptr;
		return g_pMessageStringTable[index];
	};

	const bool isStanding = m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 ||
		m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND02;
	if (isStanding)
	{
		const int bLastAttr = m_bLastMyAttr;
		m_bLastMyAttr = bAttr;

		if ((bAttr & 0x10) && !m_bTeleportMsg)
		{
			if (!m_pMessageBox)
			{
				WYD748_DiagnosticsLog("teleport blocked x=%d y=%d attr=0x%02X index=%d messagebox=null\r\n",
					x, y, bAttr, nPosIndex);
			}
			else
			{
				bool deferPrompt = false;
				if (hasPortalIndex && hasLastPortal)
				{
					int GridX = g_TeleportTable[nPosIndex].nX / 64;
					int GridY = g_TeleportTable[nPosIndex].nY / 64;
					int LstGridX = g_TeleportTable[m_nLastPotal].nX / 64;
					int LstGridY = g_TeleportTable[m_nLastPotal].nY / 64;
					if ((GridX != LstGridX || GridY != LstGridY) && bLastAttr == bAttr)
					{
						m_nLastPotal = nPosIndex;
						deferPrompt = true;
					}
				}

				if (!deferPrompt && (int)m_pMyHuman->m_vecPosition.x > 1963 &&
					(int)m_pMyHuman->m_vecPosition.x < 1970 &&
					(int)m_pMyHuman->m_vecPosition.y > 1770 &&
					(int)m_pMyHuman->m_vecPosition.y < 1777)
				{
					char* pMessage = messageAt(160);
					if (m_pMessagePanel && pMessage)
					{
						m_pMessagePanel->SetMessage(pMessage, 3000);
						m_pMessagePanel->SetVisible(1, 1);
						m_bTeleportMsg = 1;
					}
				}
				else if (!deferPrompt)
				{
					if ((int)m_pMyHuman->m_vecPosition.x >= 2370 &&
						(int)m_pMyHuman->m_vecPosition.x <= 2411 &&
						(int)m_pMyHuman->m_vecPosition.y >= 1728 &&
						(int)m_pMyHuman->m_vecPosition.y <= 1759)
					{
						char* pMessage = messageAt(258);
						if (m_pMessagePanel && pMessage)
						{
							m_pMessagePanel->SetMessage(pMessage, 3000);
							m_pMessagePanel->SetVisible(1, 1);
							m_bTeleportMsg = 1;
						}
						return 1;
					}

					if (!m_pMessageBox->IsVisible())
					{
						if (m_bAirMove == 1)
							return 1;

						if (m_pMyHuman->m_vecPosition.x > 1366.5f &&
							m_pMyHuman->m_vecPosition.x < 3366.5f &&
							m_pMyHuman->m_vecPosition.y > 2926.5f &&
							m_pMyHuman->m_vecPosition.y < 4926.5f && m_dwKhepraID)
							return 1;

						m_nLastPotal = nPosIndex;

						char szGoto[256]{};
						char* pPrompt = messageAt(43);
						if (hasPortalIndex)
						{
							char* pNamedFormat = messageAt(208);
							char* pTarget = g_TeleportTable[nPosIndex].szTarget;
							if (pNamedFormat && pTarget &&
								sprintf_s(szGoto, _countof(szGoto), pNamedFormat, pTarget) >= 0)
								pPrompt = szGoto;
						}

						if (!pPrompt)
						{
							WYD748_DiagnosticsLog("teleport prompt unavailable x=%d y=%d attr=0x%02X index=%d\r\n",
								x, y, bAttr, nPosIndex);
						}
						else
						{
							int nShowPrice = 0;
							if (hasPortalIndex)
							{
								if (!m_cWarClan && m_pMyHuman->m_cMantua &&
									m_pMyHuman->m_cMantua != 3)
									nShowPrice = 1;
								if (m_cWarClan == 7 && m_pMyHuman->m_cMantua != 1)
									nShowPrice = 1;
								if (m_cWarClan == 8 && m_pMyHuman->m_cMantua != 2)
									nShowPrice = 1;
								if (m_cWarClan == -1)
									nShowPrice = 1;
							}

							if (hasPortalIndex && g_TeleportTable[nPosIndex].nPrice > 0 &&
								nShowPrice == 1)
							{
								char* pPriceFormat = messageAt(207);
								char szPrice[256]{};
								if (pPriceFormat &&
									sprintf_s(szPrice, _countof(szPrice), pPriceFormat,
										g_TeleportTable[nPosIndex].nPrice) >= 0)
									m_pMessageBox->SetMessage(pPrompt, 16, szPrice);
								else
									m_pMessageBox->SetMessage(pPrompt, 16, nullptr);
							}
							else
							{
								m_pMessageBox->SetMessage(pPrompt, 16, nullptr);
							}
						}
						if (pPrompt)
						{
							m_pMessageBox->SetVisible(1);
							m_bTeleportMsg = 1;
							WYD748_DiagnosticsLog("teleport prompt x=%d y=%d attr=0x%02X motion=%d index=%d visible=%d\r\n",
								x, y, bAttr, static_cast<int>(m_pMyHuman->m_eMotion),
								nPosIndex, m_pMessageBox->IsVisible());
						}
					}
				}
			}
		}
	}

	if (!(bAttr & 0x10))
	{
		// Leaving the portal tile rearms the native prompt for the next entry.
		m_bTeleportMsg = 0;
		if (m_pMessageBox && m_pMessageBox->IsVisible() == 1 &&
			m_pMessageBox->m_dwMessage == 16)
			m_pMessageBox->SetVisible(0);
	}

	return 0;
}

bool TMFieldScene::OfferRespawnPrompt(bool playerAction)
{
	if (!m_pMyHuman || !m_pMessageBox || !g_pObjectManager ||
		!death_motion::MayOfferRespawnPrompt(m_bRespawnPromptOffered, playerAction,
			m_dwLastTown != 0 || m_dwLastResurrect != 0))
		return false;
	POINT position{static_cast<LONG>(m_pMyHuman->m_vecPosition.x),
		static_cast<LONG>(m_pMyHuman->m_vecPosition.y)};
	if (!death_motion::CanOfferRespawnPrompt(g_pObjectManager->m_stMobData.CurrentScore.CurHP,
		m_pMyHuman->m_cDie == 1, m_pMyHuman->m_sFamCount != 0,
		m_pMyHuman->IsInTown() != 0, PtInRect(&rectTownInCastle, position) == 1,
		m_pMessageBox->IsVisible() != 0))
		return false;
	m_bRespawnPromptOffered = true;
	m_pMessageBox->SetMessage(g_pMessageStringTable[27], 11u, 0);
	m_pMessageBox->SetVisible(1);
	return true;
}

void TMFieldScene::SetRunMode()
{
	if (m_pMyHuman->m_cMount == 1 &&
		(m_pMyHuman->m_nMountSkinMeshType == 31 || m_pMyHuman->m_nMountSkinMeshType == 40 || m_pMyHuman->m_nMountSkinMeshType == 20 &&
		 m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
	{
		g_bRunning = g_bRunning == 0;
		UpdateScoreUI(0);
	}
}

void TMFieldScene::MobStop(D3DXVECTOR3 vec)
{
	int nTX = static_cast<int>(vec.x);
	int nTY = static_cast<int>(vec.z);
	int nX = static_cast<int>(m_pMyHuman->m_vecPosition.x);
	int nY = static_cast<int>(m_pMyHuman->m_vecPosition.y);

	if (nTX >= nX + 1)
		++nX;

	else if (nTX <= nX - 1)
		--nX;

	if (nTY >= nY + 1)
		++nY;

	else if (nTY <= nY - 1)
		--nY;

	int nSX = m_pMyHuman->m_LastSendTargetPos.x;
	int nSY = m_pMyHuman->m_LastSendTargetPos.y;

	char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;

	char cRouteBuffer[48]{};

	int nRet = BASE_GetRoute(nSX, nSY, &nX, &nY, cRouteBuffer, 12, pHeightMapData, 8);
	if (!nRet)
	{
		nX = nSX;
		nY = nSY;
	}

	int nStart = GroundGetMask(TMVector2{ (float)nSX, (float)nSY });
	int nEnd = GroundGetMask(TMVector2{ (float)nX, (float)nY });

	int nHeight{};

	if ((nEnd - nStart) <= 0)
		nHeight = nStart - nEnd;
	else
		nHeight = nEnd - nStart;

	if (nHeight > 30)
	{
		nRet = BASE_GetRoute(nSX, nSY, &nX, &nY, cRouteBuffer, 6, pHeightMapData, 8);

		if (!nRet)
		{
			nX = nSX;
			nY = nSY;
		}
	}
	if (nRet && g_bLastStop != MSG_Action_Stop_Opcode && !m_pMyHuman->m_cLastMoveStop)
	{
		m_vecMyNext.x = nX;
		m_vecMyNext.y = nY;
		m_pMyHuman->m_vecTargetPos.x = nX;
		m_pMyHuman->m_vecTargetPos.y = nY;

		m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

		MSG_Action stAction{};

		stAction.Header.ID = m_pMyHuman->m_dwID;
		stAction.PosX = nSX;
		stAction.PosY = nSY;
		stAction.Effect = 0;
		stAction.Header.Type = MSG_Action_Stop_Opcode;
		stAction.Speed = g_nMyHumanSpeed;
		stAction.TargetX = m_pMyHuman->m_LastSendTargetPos.x;
		stAction.TargetY = m_pMyHuman->m_LastSendTargetPos.y;

		for (int j = 0; j < 23; ++j)
			stAction.Route[j] = 0;

		m_stMoveStop.LastX = stAction.PosX;
		m_stMoveStop.LastY = stAction.PosY;
		m_stMoveStop.NextX = stAction.TargetX;
		m_stMoveStop.NextY = stAction.TargetY;

		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAction)->Type, reinterpret_cast<char*>(&stAction), sizeof(stAction)});

		m_pMyHuman->OnPacketEvent(MSG_Action_Stop_Opcode, (char*)&stAction);

		g_bLastStop = stAction.Header.Type;
	}

	m_pMyHuman->m_cLastMoveStop = 0;
	m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
}

int TMFieldScene::OnPacketReqSummon(MSG_ReqSummon* pStd)
{
	if (!pStd || !m_pHelpSummon)
		return 0;
	pStd->Name[15] = 0;

	m_pHelpSummon->SetVisible(1);

	bounded_text::Copy(m_szSummoner, pStd->Name, sizeof(pStd->Name));
	return 1;
}

int TMFieldScene::OnPacketCancelSummon(MSG_STANDARD* pStd)
{
	if (m_pHelpSummon)
		m_pHelpSummon->SetVisible(0);
	return 1;
}

void TMFieldScene::AirMove_Main(unsigned int dwServerTime)
{
	// TODO: change the state to enum
	if (m_nAirMove_State && m_pMyHuman)
	{
		if (m_pMyHuman->m_cDie == 1 || m_pMyHuman->m_stScore.CurHP <= 0)
		{
			if (m_bAirMove)
				AirMove_End(AirMoveEndReason::Death);
			else
				m_nAirMove_State = 0;
			return;
		}
		if (m_bAirMove)
			m_pMyHuman->m_cHide = 0;

		switch (m_nAirMove_State)
		{
		case 1:
		{
			AirMove_Start(0);
		}
		break;
		case 2:
		{
			if (dwServerTime > m_dwAirMove_TickTime + 2800)
			{
				m_nAirMove_State = 3;
				m_dwAirMove_TickTime = dwServerTime;
				return;
			}

			float fVal = sinf((D3DXToRadian(180) * ((float)(dwServerTime - m_dwAirMove_TickTime) / 15000.0f)) / 6.0f);
			if (fVal >= 0.3f)
				fVal = 0.3f;
			if (fVal < 0.0f)
				fVal = 0.0f;

			if (m_pMyHuman->m_pMount)
				m_pMyHuman->m_pMount->m_dwFPS = 15;
			m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight + fVal;
		}
		break;
		case 3:
		{
			int nRnd = rand() % 300;
			if (dwServerTime >= m_dwAirMove_TickTime + nRnd + 1000)
			{
				m_dwAirMove_TickTime = dwServerTime;
				m_bAirMove_Wing = m_bAirMove_Wing == 0;
			}

			float fVal = sinf((D3DXToRadian(180) * ((float)(dwServerTime - m_dwAirMove_TickTime) / 10000.0f)) / 6.0f) + ((float)nRnd / 8000.0f);
			if (m_bAirMove_Wing || m_pMyHuman->m_fHeight >= 14.0)
			{
				if (m_pMyHuman->m_fHeight > 8.0)
					m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight - fVal;
			}
			else
				m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight + fVal;

			float AddY = 0.0f;
			float AddX = 0.0f;
			float MyX = m_pMyHuman->m_vecPosition.x;
			float MyY = m_pMyHuman->m_vecPosition.y;
			if (MyX >= (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX)
				AddX = -m_fAirMove_Speed;
			else if ((float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX >= MyX)
				AddX = +m_fAirMove_Speed;
			if (MyY >= (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY)
				AddY = -m_fAirMove_Speed;
			else if ((float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY >= MyY)
				AddY = +m_fAirMove_Speed;

			float DiffX = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX - (float)(MyX + AddX);
			float DiffY = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY - (float)(MyY + AddY);
			if (DiffX <= 0.5f && DiffX >= -0.5f)
				AddX = 0.0f;
			if (DiffY <= 0.5f && DiffY >= -0.5f)
				AddY = 0.0f;

			if (fabsf(AddX) == m_fAirMove_Speed && fabsf(AddY) == m_fAirMove_Speed)
			{
				AddX = AddX / 2.0f;
				AddY = AddY / 2.0f;
			}
			if (AddX != 0.0f || AddY != 0.0f)
			{
				m_pMyHuman->m_vecAirMove.x = m_pMyHuman->m_vecAirMove.x + AddX;
				m_pMyHuman->m_vecAirMove.y = m_pMyHuman->m_vecAirMove.y + AddY;

				float dPosX = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX - m_pMyHuman->m_vecPosition.x;
				float dPosY = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY - m_pMyHuman->m_vecPosition.y;

				if (fabsf(dPosX) + fabsf(dPosY) < 70.0f
					&& !HasNextAirMoveWaypoint(g_pAirMoveRoute[m_nAirMove_Index], m_nAirMove_RouteIndex))
				{
					if (m_fAirMove_Speed >= 0.2f)
						m_fAirMove_Speed = m_fAirMove_Speed - sinf((m_fAirMove_Speed * 0.0023f) * D3DXToRadian(180));
				}
				else if (m_fAirMove_Speed <= 0.69999999f)
					m_fAirMove_Speed = sinf((m_fAirMove_Speed * 0.0049999999f) * D3DXToRadian(180)) + m_fAirMove_Speed;

				if (m_pMyHuman->m_pMount)
					m_pMyHuman->m_pMount->m_dwFPS = 15 - (int)(m_fAirMove_Speed * 2.0f);

				m_pMyHuman->m_fWantAngle = atan2f(dPosX, dPosY) + D3DXToRadian(90);
			}
			else if (HasNextAirMoveWaypoint(g_pAirMoveRoute[m_nAirMove_Index], m_nAirMove_RouteIndex))
			{
				++m_nAirMove_RouteIndex;
				m_vecAirMove_Dest.x = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX;
				m_vecAirMove_Dest.y = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY;
				m_dwAirMove_TickTime = dwServerTime;
			}
			else
			{
				m_nAirMove_State = 4;
				m_dwAirMove_TickTime = dwServerTime;
			}
		}
		break;
		case 4:
		{
			float fHeight = m_pMyHuman->m_fWantHeight;
			float fDiff = m_pMyHuman->m_fHeight - fHeight;
			if (dwServerTime > m_dwAirMove_TickTime + 2800)
			{
				m_pMyHuman->m_fHeight = fHeight;
				m_nAirMove_State = 5;
				m_dwAirMove_TickTime = dwServerTime;
				return;
			}

			float fVal = sinf((D3DXToRadian(180) * ((float)(dwServerTime - m_dwAirMove_TickTime) / 15000.0f)) / 6.0f);
			if (m_pMyHuman->m_pMount)
				m_pMyHuman->m_pMount->m_dwFPS = 15;
			if ((float)(m_pMyHuman->m_fHeight - fVal) > 0.0f)
				m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight - fVal;
		}
		break;
		case 5:
		{
			AirMove_End();
		}
		break;
		}
	}
}

void TMFieldScene::AirMove_Start(int nIndex)
{
	if (!m_pMyHuman || !m_pEffectContainer || !IsValidAirMoveRouteIndex(nIndex) ||
		m_pMyHuman->m_cDie == 1 || m_pMyHuman->m_stScore.CurHP <= 0)
		return;

	m_vecAirMove_Origin = m_pMyHuman->m_vecPosition;
	m_nAirMove_State = 2;
	m_bAirMove = 1;
	m_eOldMotion = m_pMyHuman->m_eMotion;
	m_nOldMountSkinMeshType = m_pMyHuman->m_nMountSkinMeshType;
	m_stOldAirMoveMountLook = m_pMyHuman->m_stMountLook;

	m_pMyHuman->UpdateMount();
	m_pMyHuman->m_bIgnoreHeight = 1;
	m_dwAirMove_TickTime = g_pTimerManager->GetServerTime();
	m_pMyHuman->m_vecAirMove.x = 0.0f;
	m_pMyHuman->m_vecAirMove.y = 0.0f;

	auto pParticle = new TMEffectParticle(TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y),
		1, 10, 3.0f, 0, 1, 56, 1.0f, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000);

	m_pEffectContainer->AddChild(pParticle);

	m_vecAirMove_Dest.x = (float)g_pAirMoveRoute[nIndex][0].nX;
	m_vecAirMove_Dest.y = (float)g_pAirMoveRoute[nIndex][0].nY;

	MSG_STANDARDPARM2 stAirmoveStart{};
	stAirmoveStart.Header.Type = MSG_AirMove_Start_Opcode;
	stAirmoveStart.Header.ID = m_pMyHuman->m_dwID;
	stAirmoveStart.Parm1 = nIndex;
	stAirmoveStart.Parm2 = kAirMoveStartMode;
	SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAirmoveStart)->Type, reinterpret_cast<char*>(&stAirmoveStart), sizeof(stAirmoveStart)});
	m_nAirMove_Index = nIndex;
	m_nAirMove_RouteIndex = 0;
	m_fAirMove_Speed = 0.2f;
}

void TMFieldScene::AirMove_End(AirMoveEndReason reason)
{
	if (m_pMyHuman && m_bAirMove)
	{
		const bool externalTeleport = reason == AirMoveEndReason::ExternalTeleport;
		const bool interruptedByDeath = !externalTeleport &&
			(reason == AirMoveEndReason::Death || m_pMyHuman->m_cDie == 1 ||
				m_pMyHuman->m_stScore.CurHP <= 0);
		if (interruptedByDeath)
			CancelAirMoveAtOrigin(m_pMyHuman->m_vecPosition,
				m_pMyHuman->m_vecAirMove, m_vecAirMove_Origin);
		else if (externalTeleport)
			DiscardAirMoveDelta(m_pMyHuman->m_vecAirMove);
		else
			ConsumeAirMoveDelta(m_pMyHuman->m_vecPosition, m_pMyHuman->m_vecAirMove);
		m_nAirMove_State = interruptedByDeath || externalTeleport ? 0 : -1;
		m_bAirMove = 0;

		if (m_nOldMountSkinMeshType <= 0)
		{
			SAFE_DELETE(m_pMyHuman->m_pMount);
			m_pMyHuman->m_cMount = 0;
			m_pMyHuman->m_nMountSkinMeshType = 0;
		}
		else
		{
			RestoreAirMoveMountVisual(m_pMyHuman->m_nMountSkinMeshType,
				m_pMyHuman->m_stMountLook, m_nOldMountSkinMeshType,
				m_stOldAirMoveMountLook);
			m_nOldMountSkinMeshType = -1;
			m_pMyHuman->UpdateMount();
		}

		m_pMyHuman->m_bIgnoreHeight = 0;
		m_dwAirMove_TickTime = 0;
		if (interruptedByDeath)
		{
			const auto& origin = m_pMyHuman->m_vecPosition;
			m_pMyHuman->InitPosition(origin.x,
				static_cast<float>(GroundGetMask(origin)) * 0.1f, origin.y);
			return;
		}
		if (externalTeleport)
			return;
		UpdateMyHuman();
		if (m_pMyHuman->m_cDie != 1 && m_pMyHuman->m_stScore.CurHP > 0)
			m_pMyHuman->SetAnimation(m_eOldMotion, 1);

		if (m_pEffectContainer)
		{
			auto pParticle = new TMEffectParticle(TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y),
				1, 10, 3.0f, 0, 1, 56, 1.0f, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000);
			m_pEffectContainer->AddChild(pParticle);
		}

		if (!IsValidAirMoveRouteIndex(m_nAirMove_Index))
			m_nAirMove_Index = 0;

		MSG_STANDARDPARM2 stAirmoveStart{};
		stAirmoveStart.Header.Type = MSG_AirMove_Start_Opcode;
		stAirmoveStart.Header.ID = m_pMyHuman->m_dwID;
		stAirmoveStart.Parm1 = m_nAirMove_Index;
		stAirmoveStart.Parm2 = kAirMoveEndMode;
		SendPacket({reinterpret_cast<MSG_STANDARD*>(&stAirmoveStart)->Type, reinterpret_cast<char*>(&stAirmoveStart), sizeof(stAirmoveStart)});
	}
}

int TMFieldScene::AirMove_ShowUI(bool bShow)
{
	auto pPotalPanel = m_pPotalPanel;
	if (!pPotalPanel)
		return 0;

	if (bShow == 1)
	{
		// The compact 7.48 bootstrap does not run InitBoard.  A partially
		// populated resource must not open an empty modal or dereference text.
		if (!m_pPotalList || !m_pPotalText || !m_pQuestList[0] || !m_pQuestList[1])
		{
			pPotalPanel->SetVisible(0);
			m_bAirmove_ShowUI = false;
			return 0;
		}
		LoadMsgText3(m_pQuestList[0], (char*)"UI\\QuestSubjects.txt", 400, 0);
		LoadMsgText3(m_pQuestList[1], (char*)"UI\\QuestSubjects2.txt", 400, 0);
		if (m_pQuestList[2])
			LoadMsgText3(m_pQuestList[2], (char*)"UI\\QuestSubjects3.txt", 400, 0);
		if (m_pQuestList[3])
			LoadMsgText3(m_pQuestList[3], (char*)"UI\\QuestSubjects4.txt", 400, 0);
		if (!m_pQuestList[0]->m_pItemList[0] ||
			!m_pQuestList[0]->m_pItemList[1] ||
			!m_pQuestList[0]->m_pItemList[6])
		{
			pPotalPanel->SetVisible(0);
			m_bAirmove_ShowUI = false;
			return 0;
		}
		const char* place0 = m_pQuestList[0]->m_pItemList[0]->GetText();
		const char* place1 = m_pQuestList[0]->m_pItemList[1]->GetText();
		const char* place2 = m_pQuestList[0]->m_pItemList[6]->GetText();
		if (!place0 || !*place0 || !place1 || !*place1 || !place2 || !*place2)
		{
			pPotalPanel->SetVisible(0);
			m_bAirmove_ShowUI = false;
			return 0;
		}

		pPotalPanel->SetVisible(1);
		if (m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
			SetVisibleSkill();
		if (m_pCPanel && m_pCPanel->m_bVisible == 1)
			SetVisibleCharInfo();
		if (m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
			SetVisibleCargo(0);
		if (m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
			SetVisibleCargo(0);
		if (m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
			SetVisibleAutoTrade(0, 0);
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();
		if (m_pShopPanel && m_pShopPanel->m_bVisible == 1)
			SetVisibleShop(0);

		m_pPotalText->SetText(g_pMessageStringTable[378], 0);
		m_pPotalText->SetTextColor(0xFFFFFFFF);
		if (m_pPotalText1)
			m_pPotalText1->SetText(g_pMessageStringTable[380], 0);
		if (m_pPotalText2)
			m_pPotalText2->SetText(g_pMessageStringTable[379], 0);
		if (m_pPotalText3)
			m_pPotalText3->SetText(g_pMessageStringTable[381], 0);

		char strAirMoveList[10][256]{};

		int i = 0;
		char strAirMovePlaceName[10][64]{};
		sprintf_s(strAirMovePlaceName[0], "%.18s", place0 + 1);
		sprintf_s(strAirMovePlaceName[1], "%.18s", place1 + 1);
		sprintf_s(strAirMovePlaceName[2], "%.18s", place2 + 1);
		sprintf_s(strAirMovePlaceName[3], "%.18s", g_pMessageStringTable[212]);
		sprintf_s(strAirMovePlaceName[4], "%.18s", g_pMessageStringTable[218]);

		for (int i = 0; i < 5; ++i)
		{
			while (1)
			{
				if (strlen(strAirMovePlaceName[i]) >= 18)
					break;

				strcat(strAirMovePlaceName[i], " ");
			}
		}

		for (i = 0; i < 5; ++i)
			sprintf(strAirMoveList[i], "FFFFFF    %04d %04d  %s               %d", g_pAirMoveList[i].nX, g_pAirMoveList[i].nY, strAirMovePlaceName[i], 0);

		int nCount = 0;
		m_pPotalList->Empty();
		for (int j = 0; j < 5; ++j)
		{
			char szTemp[256]{};
			sprintf(szTemp, "%s", strAirMoveList[j]);

			char szCol[7]{};
			strncpy(szCol, szTemp, 6u);

			unsigned int dwCol = 0;
			sscanf(szCol, "%x", &dwCol);

			auto szRet = strstr(szTemp, "\n");
			if (szRet)
				szRet[0] = 0;

			char szText[256]{};
			sprintf(szText, "%s", &szTemp[6]);

			auto pItem = new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				m_pPotalList->m_nWidth,
				16.0f,
				0,
				0x77777777,
				1,
				0);

			m_pPotalList->AddItem(pItem);
			if (++nCount > 100)
				break;
		}

		if (m_pPotalList->m_pScrollBar)
			m_pPotalList->m_pScrollBar->SetCurrentPos(0);
	}
	else
		pPotalPanel->SetVisible(0);

	m_bAirmove_ShowUI = bShow;
	if (!bShow)
	{
		auto pCharPanel = m_pCPanel;
		auto pInvPanel = m_pInvenPanel;
		auto pSkillPanel = m_pSkillPanel;
		auto pPartyPanel = m_pPartyPanel;
		auto pTradePanel = m_pTradePanel;
		auto pAutoTradePanel = m_pAutoTrade;
		auto pShopPanel = m_pShopPanel;
		auto pCargoPanel = m_pCargoPanel;
		auto pCargoPanel1 = m_pCargoPanel1;
		auto pMinimapPanel = m_pMiniMapPanel;
		auto pInputGoldPanel = m_pInputGoldPanel;
		auto pPGTPanel = m_pPGTPanel;
		auto pSystemPanel = m_pSystemPanel;
		auto pGambleStore = m_pGambleStore;
		auto pServerPanel = m_pServerPanel;
		auto pPotalPanel = m_pPotalPanel;
		if (pAutoTradePanel && pAutoTradePanel->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);
		else if (pPGTPanel && pPGTPanel->IsVisible() == 1)
			pPGTPanel->SetVisible(0);
		else if (m_pQuestPanel && m_pQuestPanel->IsVisible() == 1)
		{
			SetQuestPanelVisible(false);
		}
		else if (m_pFireWorkPanel && m_pFireWorkPanel->IsVisible() == 1)
			m_pFireWorkPanel->SetVisible(0);
		else if (m_pTotoPanel && m_pTotoPanel->IsVisible() == 1)
			TotoClose();
		else if (pInvPanel && pInvPanel->IsVisible() == 1)
			OnControlEvent(65562, 0);
		else if (pSkillPanel && pSkillPanel->IsVisible() == 1)
			OnControlEvent(65568, 0);
		else if (pCharPanel && pCharPanel->IsVisible() == 1)
			OnControlEvent(65769, 0);
		else if (pTradePanel && pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);
		else if (pPartyPanel && pPartyPanel->IsVisible() == 1)
			SetVisibleParty();
		else if (pShopPanel && pShopPanel->IsVisible()== 1)
			SetVisibleShop(0);
		else if (pCargoPanel && pCargoPanel->IsVisible() == 1)
			SetVisibleCargo(0);
		else if (pCargoPanel1 && pCargoPanel1->IsVisible() == 1)
			SetVisibleCargo(0);
		else if (pGambleStore && pGambleStore->IsVisible() == 1)
			SetVisibleGamble(0, 0);
		else if (pInputGoldPanel && pInputGoldPanel->IsVisible() == 1)
			pInputGoldPanel->SetVisible(0);
		else if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 1)
		{
			m_pMsgPanel->SetVisible(0);
			if (m_pControlContainer)
				m_pControlContainer->SetFocusedControl(0);
		}
		else if (m_pHelpPanel && m_pHelpPanel->IsVisible() == 1)
		{
			m_pHelpPanel->SetVisible(0);
			// Keep Esc usable even if the 314 Help button was omitted by a damaged
			// resource; the 7.48 panel itself is still safe to close independently.
			if (m_pHelpBtn)
				m_pHelpBtn->SetSelected(0);
			GetSoundAndPlay(51, 0, 0);
		}
		else if (pServerPanel && pServerPanel->IsVisible() == 1)
			pServerPanel->SetVisible(0);
		else if (pPotalPanel && pPotalPanel->IsVisible() == 1)
			pPotalPanel->SetVisible(0);
		else if (m_pMessageBox && m_pMessageBox->IsVisible() == 1)
			m_pMessageBox->SetVisible(0);
	}

	return 1;
}
