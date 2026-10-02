#include "pch.h"
#include "TMFieldScene.h"
#include "FieldSceneWorldSupport.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMObjectContainer.h"
#include "TMUtil.h"
#include "TMItem.h"
#include "TMHouse.h"
#include "TMCamera.h"
#include "TMSun.h"
#include "TMSky.h"
#include "TMSnow.h"
#include "TMRain.h"
#include "TMSkinMesh.h"
#include "TMEffectBillBoard.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectMesh.h"
#include "TMEffectDust.h"
#include "TMEffectParticle.h"
#include "TMShade.h"
#include "TMGate.h"
#include "../../ui/MiniMapLayout.h"
#include "TMHuman.h"
#include "TMEffectStart.h"
#include "TMSkillJudgement.h"
#include "TMSkillTownPortal.h"
#include "TMEffectLevelUp.h"
#include "TMCannon.h"
#include "TMSkillExplosion2.h"

void SetMinimapPos()
{
	FILE* fpMinimap = nullptr;
	fopen_s(&fpMinimap, "minimap.dat", "rt");

	memset(g_MinimapPos, 0, sizeof(g_MinimapPos));

	if (fpMinimap)
	{
		int Color = 0;
		for (int index = 0; index < 256; ++index)
		{
			if (fscanf(fpMinimap, "%d %d %d %d %d %s", &g_MinimapPos[index].nX, &g_MinimapPos[index].nY, &Color, &g_MinimapPos[index].nCX, &g_MinimapPos[index].nCY, g_MinimapPos[index].szTarget) == -1)
				break;

			switch (Color)
			{
			case 1:
				g_MinimapPos[index].dwColor = 0xFFFFFFFF;
				break;
			case 2:
				g_MinimapPos[index].dwColor = 0xFF44AA44;
				break;
			case 3:
				g_MinimapPos[index].dwColor = 0xFF5555FF;
				break;
			case 4:
				g_MinimapPos[index].dwColor = 0xFFAA00FF;
				break;
			}
		}

		fclose(fpMinimap);
	}
}

void TMFieldScene::SetVisiblePotal(int bShow, int nPos)
{
	if (m_bAirmove_ShowUI == 1)
		AirMove_ShowUI(0);

	auto pPotalPanel = m_pPotalPanel;
	if (!pPotalPanel)
		return;

	char strTmp1[32]{};
	strcpy(strTmp1, g_pMessageStringTable[382]);
	char strTmp2[32]{};
	strcpy(strTmp2, g_pMessageStringTable[383]);
	char strTmp3[32]{};
	strcpy(strTmp3, g_pMessageStringTable[384]);
	if (m_pPotalText1)
		m_pPotalText1->SetText(strTmp1, 0);
	if (m_pPotalText2)
		m_pPotalText2->SetText(strTmp2, 0);
	if (m_pPotalText3)
		m_pPotalText3->SetText(strTmp3, 0);

	pPotalPanel->SetVisible(bShow);
	char szPotalPos[64]{};
	sprintf(szPotalPos, "UI\\PotalPos.txt");
	if (bShow == 1)
		LoadMsgText2(m_pPotalList, szPotalPos,10 * nPos, 10 * (nPos + 1) - 1);
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(0);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(0);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);
	if (bShow == 1 && m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
		SetVisibleInventory();
	if (bShow == 1 && m_pShopPanel && m_pShopPanel->m_bVisible == 1)
		SetVisibleShop(0);
	if (bShow)
	{
		m_pSkillPanel->m_bVisible = 0;

		char szStr[128]{};
		sprintf(szStr, "%s", g_pMessageStringTable[310]);
		TMScene::LoadMsgText2(m_pPotalList, szPotalPos, 10 * nPos, 10 * (nPos + 1) - 1);
		switch (nPos)
		{
		case 0:
			sprintf(szStr, "%s", g_pMessageStringTable[310]);
			break;
		case 1:
			sprintf(szStr, "%s", g_pMessageStringTable[311]);
			break;
		case 2:
			sprintf(szStr, "%s", g_pMessageStringTable[312]);
			break;
		case 3:
			sprintf(szStr, "%s", g_pMessageStringTable[313]);
			break;
		case 4:
			sprintf(szStr, "%s", g_pMessageStringTable[314]);
			break;
		case 5:
			sprintf(szStr, "%s", g_pMessageStringTable[315]);
			break;
		}

		if (m_pPotalText)
		{
			m_pPotalText->SetText(szStr, 0);
			m_pPotalText->SetTextColor(0xFFFFFFFF);
		}
	}
	else
	{
		memset(&m_stPotalItem, 0, sizeof(m_stPotalItem));
	}
}

void TMFieldScene::SetVisibleMiniMap()
{
	if (m_pMiniMapPanel == nullptr)
		return;

	SGridControl::m_sLastMouseOverIndex = -1;

	if (m_bCompatFieldScene)
	{
		// Follow the native layout matching the loaded graph, not the INI flag.
		SPanel* pMiniMapBorder = static_cast<SPanel*>(m_pControlContainer->FindControl(290));
		const bool wasVisible = m_pMiniMapPanel->m_bVisible != 0;
		const bool ui2 = m_pMiniMapZoomIn != nullptr;
		const auto layout = mini_map_layout::Next(wasVisible, TMGround::m_fMiniMapScale, ui2,
			static_cast<float>(g_pDevice->m_dwScreenWidth), static_cast<float>(g_pDevice->m_dwScreenHeight));
		for (int markerIndex = 0; markerIndex < 256; ++markerIndex)
		{
			if (m_pInMiniMapPosPanel[markerIndex])
				m_pInMiniMapPosPanel[markerIndex]->SetVisible(0);
			if (m_pInMiniMapPosText[markerIndex])
				m_pInMiniMapPosText[markerIndex]->SetVisible(0);
		}

		TMGround::m_fMiniMapScale = layout.scale;
		m_pMiniMapPanel->SetPos(layout.x, layout.y);
		m_pMiniMapPanel->SetSize(layout.size, layout.size);
		m_pMiniMapPanel->SetVisible(layout.visible);
		if (pMiniMapBorder)
		{
			pMiniMapBorder->SetPos(ui2 ? 0.0f : -4.0f, ui2 ? 0.0f : -4.0f);
			pMiniMapBorder->SetSize(layout.size + (ui2 ? 0.0f : 8.0f), layout.size + (ui2 ? 0.0f : 8.0f));
			pMiniMapBorder->SetVisible(layout.visible && (!ui2 || !layout.expanded));
		}
		// UI2 has eight separate frame pieces, not a scalable root border.
		const float edge = 8.0f;
		const float last = layout.size - edge;
		const float middle = layout.size - 2.0f * edge;
		const float frame[8][4] = {
			{0, 0, edge, edge}, {edge, 0, middle, edge}, {last, 0, edge, edge},
			{0, edge, edge, middle}, {last, edge, edge, middle},
			{0, last, edge, edge}, {edge, last, middle, edge}, {last, last, edge, edge}};
		for (int i = 0; i < 8; ++i)
		{
			if (auto part = m_pControlContainer->FindControl(5705 + i))
			{
				part->SetPos(frame[i][0], frame[i][1]);
				part->SetSize(frame[i][2], frame[i][3]);
				part->SetVisible(layout.visible && layout.expanded);
			}
		}
		if (auto bottom = m_pControlContainer->FindControl(5713))
		{
			bottom->SetPos(0, layout.size);
			bottom->SetSize(layout.size, 16.0f);
			bottom->SetVisible(layout.visible && !layout.expanded);
		}
		// Do not show the resource's placeholder server label in the compact HUD.
		if (m_pMiniMapServerPanel)
			m_pMiniMapServerPanel->SetVisible(0);
		// The compact header is a separate child and must not cover the expanded map.
		if (auto header = m_pControlContainer->FindControl(12841u))
			header->SetVisible(layout.visible && !layout.expanded);
		if (m_pMiniMapZoomIn)
		{
			m_pMiniMapZoomIn->SetPos(layout.size - 19.0f, -18.0f);
			m_pMiniMapZoomIn->SetSize(13.0f, 13.0f);
			m_pMiniMapZoomIn->SetVisible(layout.visible && !layout.expanded);
		}
		if (m_pMiniMapZoomOut)
		{
			m_pMiniMapZoomOut->SetPos(layout.size - 40.0f, 0);
			m_pMiniMapZoomOut->SetSize(40.0f, 38.0f);
			m_pMiniMapZoomOut->SetVisible(layout.visible && layout.expanded);
		}
		if (m_pMiniMapDir)
			m_pMiniMapDir->SetPos(layout.size * 0.5f - m_pMiniMapDir->m_nWidth * 0.5f,
				layout.size * 0.5f - m_pMiniMapDir->m_nHeight * 0.5f);
		if (m_pPositionText)
		{
			m_pPositionText->SetPos(0, layout.size + 2.0f);
			m_pPositionText->SetSize(layout.size, 20.0f);
			m_pPositionText->m_dwAlignType = SText::TEXT_ALIGN_CENTER;
		}

		const bool isVisible = m_pMiniMapPanel->m_bVisible != 0;
		if (isVisible && m_pGround)
			m_pGround->RestoreDeviceObjects();
		if (auto pMiniMapButton = static_cast<SButton*>(m_pControlContainer->FindControl(296)))
			pMiniMapButton->SetSelected(isVisible ? 1 : 0);
		if (m_pPositionText)
			m_pPositionText->SetVisible(isVisible ? 1 : 0);
		if (g_pSoundManager)
		{
			if (auto pSoundData = g_pSoundManager->GetSoundData(51))
				pSoundData->Play(0, 0);
		}
		return;
	}

	int bVisible = m_pMiniMapPanel->m_bVisible;

	if (bVisible)
	{
		for (int j = 0; j < 256; ++j)
		{
			m_pInMiniMapPosPanel[j]->SetVisible(0);
			m_pInMiniMapPosText[j]->SetVisible(0);
		}
	}
	else
	{
		for (int i = 0; i < 256; ++i)
		{
			m_pInMiniMapPosPanel[i]->SetVisible(0);
			m_pInMiniMapPosText[i]->SetVisible(0);
		}
	}

	if (bVisible == 1)
	{
		auto pBGPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(12841u));

		if (pBGPanel)
			pBGPanel->SetVisible(0);

		if (m_pMiniMapServerPanel)
			m_pMiniMapServerPanel->SetVisible(0);

		if (m_pMiniMapZoomOut)
			m_pMiniMapZoomOut->SetVisible(1);

		if (m_pMiniMapZoomIn)
			m_pMiniMapZoomIn->SetVisible(0);

		if (TMGround::m_fMiniMapScale >= 1.0f)
		{
			if (m_pMiniMapPanel)
			{
				m_pMiniMapPanel->SetPos((float)g_pDevice->m_dwScreenWidth - m_pMiniMapPanel->m_nWidth, 0);
				m_pMiniMapPanel->SetVisible(0);
			}
		}
		else
		{
			TMGround::m_fMiniMapScale = 1.5f;

			float fHeight = BASE_ScreenResize(400.0f);

			if (m_pMiniMapPanel)
			{
				m_pMiniMapPanel->SetSize(fHeight, fHeight);

				m_pMiniMapPanel->SetPos(
					((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pMiniMapPanel->m_nWidth * 0.5f),
					(((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pMiniMapPanel->m_nHeight * 0.5f)) - 15.0f);
			}

			if (m_pMiniMapDir)
				m_pMiniMapDir->SetPos(fHeight * 0.5f, fHeight * 0.5f);

			if (m_pPositionText)
				m_pPositionText->SetPos(10.0f, 0);

			if (m_pMiniMapZoomIn)
				m_pMiniMapZoomIn->SetPos(fHeight - 134.0f, fHeight + 5.0f);

			if (m_pMiniMapZoomOut)
				m_pMiniMapZoomOut->SetPos(fHeight - 40.0f, 0);

			if (m_pMiniMapServerPanel)
				m_pMiniMapServerPanel->SetPos(fHeight - 274.0f, fHeight);

			if (m_pMiniMapZoomIn)
				m_pMiniMapZoomIn->m_bSelected = 1;

			if (m_pMiniMapZoomOut)
				m_pMiniMapZoomOut->m_bSelected = 0;
		}
	}
	else
	{
		TMGround::m_fMiniMapScale = 0.60000002f;

		auto pBGPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(12841u));

		if (pBGPanel)
			pBGPanel->SetVisible(1);

		if (m_pMiniMapZoomOut)
			m_pMiniMapZoomOut->SetVisible(0);

		if (m_pMiniMapZoomIn)
		{
			m_pMiniMapZoomIn->SetVisible(1);

			m_pMiniMapZoomIn->SetPos(BASE_ScreenResize(118.0f), -BASE_ScreenResize(18.0f));
		}

		if (m_pMiniMapPanel)
		{
			m_pMiniMapPanel->SetSize(BASE_ScreenResize(137.0f), BASE_ScreenResize(137.0f));

			m_pMiniMapPanel->SetPos(((float)g_pDevice->m_dwScreenWidth - m_pMiniMapPanel->m_nWidth) - 2.0f, 33.0f);
		}

		m_pMiniMapServerPanel->SetVisible(1);

		if (m_pMiniMapServerPanel)
		{
			m_pMiniMapServerPanel->SetPos(0, BASE_ScreenResize(130.0f));
		}

		if (m_pMiniMapPanel)
			m_pMiniMapPanel->SetVisible(bVisible == 0);

		if (m_pMiniMapDir)
		{
			m_pMiniMapDir->SetPos(BASE_ScreenResize(68.0f), BASE_ScreenResize(68.0f));
		}

		if (m_pPositionText)
		{
			m_pPositionText->SetPos(BASE_ScreenResize(13.0f), BASE_ScreenResize(120.0f));
		}

		if (m_pMiniMapZoomIn && m_pMiniMapZoomOut)
		{
			m_pMiniMapZoomOut->SetPos(BASE_ScreenResize(119.0f), BASE_ScreenResize(142.0f));
			m_pMiniMapZoomOut->m_bSelected = 1;
		}
	}

	bVisible = m_pMiniMapPanel->m_bVisible;

	if (bVisible == 1 && m_pGround)
		m_pGround->RestoreDeviceObjects();

	auto pBtnMinimap = static_cast<SButton*>(m_pControlContainer->FindControl(296u));

	if (pBtnMinimap)
		pBtnMinimap->SetSelected(m_pMiniMapPanel->m_bVisible);

	m_pPositionText->SetVisible(bVisible);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetCameraView()
{
	auto pCamera = g_pObjectManager->m_pCamera;
	if (pCamera->m_dwSetTime)
		return;

	m_nCameraMode += 1;
	if (m_nCameraMode > 2)
		m_nCameraMode = 0;

	if (m_bIsDungeon == 1 && m_nCameraMode == 1)
		m_nCameraMode = 2;

	if (m_nCameraMode == 0)
	{
		TMVector2 Pos = g_pObjectManager->m_pCamera->GetFocusedObject()->m_vecPosition;
		if ((int)Pos.x >> 7 > 26 && (int)Pos.x >> 7 < 31 &&
			(int)Pos.y >> 7 > 20 && (int)Pos.y >> 7 < 25)
		{
			pCamera->m_fVerticalAngle = -0.64114136f;
			pCamera->m_fHorizonAngle = 1.013417f;
		}
		else
		{
			pCamera->m_fVerticalAngle = -0.76999825f;
			pCamera->m_fHorizonAngle = 0.80142671f;
		}

		pCamera->m_nQuaterView = 0;
		pCamera->m_fSightLength = pCamera->m_fMaxCamLen;
		pCamera->m_fWantLength = pCamera->m_fMaxCamLen;
		pCamera->m_fCamHeight = 0.33f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}
	else if (m_nCameraMode == 1)
	{
		TMVector2 Pos = g_pObjectManager->m_pCamera->GetFocusedObject()->m_vecPosition;
		if ((int)Pos.x >> 7 > 26 && (int)Pos.x >> 7 < 31 &&
			(int)Pos.y >> 7 > 20 && (int)Pos.y >> 7 < 25)
		{
			pCamera->m_fVerticalAngle = -0.64114136f;
			pCamera->m_fHorizonAngle = 1.013417f;
		}
		else
		{
			pCamera->m_fVerticalAngle = -0.76999825f;
			pCamera->m_fHorizonAngle = 0.80142671f;
		}

		pCamera->m_nQuaterView = 0;
		pCamera->m_fSightLength = 11.0f;
		pCamera->m_fWantLength = 11.0f;
		pCamera->m_fCamHeight = 0.33f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}
	else if (m_nCameraMode == 2)
	{
		pCamera->m_nQuaterView = 0;
		pCamera->m_fVerticalAngle = 0.1f;
		pCamera->m_fSightLength = 3.5f;
		pCamera->m_fWantLength = 3.5f;
		pCamera->m_fCamHeight = 0.33f;
		pCamera->m_fHorizonAngle = 3.1415927f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}

	auto pBtnCamera = (SButton*)m_pControlContainer->FindControl(308);
	if (pBtnCamera)
		pBtnCamera->SetSelected(m_bQuater);
}

void TMFieldScene::InitCameraView()
{
	auto pCamera = g_pObjectManager->m_pCamera;
	pCamera->m_nQuaterView = 0;
	pCamera->m_fVerticalAngle = -0.78539819f;
	pCamera->m_fSightLength = pCamera->m_fMaxCamLen;
	pCamera->m_fWantLength = pCamera->m_fMaxCamLen;
	pCamera->m_fCamHeight = 0.33000001f;
	pCamera->m_fHorizonAngle = 0.78539819f;
	g_pDevice->m_bFog = 1;
	m_pSky->m_bVisible = 0;
	m_bQuater = 0;
	m_bTab = 0;
	pCamera->EarthQuake(10);
}

void TMFieldScene::SetVisibleNameLabel()
{
	m_bShowNameLabel = m_bShowNameLabel == 0;

	auto pBtnName = static_cast<SButton*>(m_pControlContainer->FindControl(307u));

	if (pBtnName)
		pBtnName->SetSelected(m_bShowNameLabel == 0);
}

void TMFieldScene::SetWeather(int nWeather)
{
	g_nWeather = nWeather;

	if (((int)m_pMyHuman->m_vecPosition.x >> 7 == 29 ||
		(int)m_pMyHuman->m_vecPosition.x >> 7 == 30) &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 == 22 == 1)
	{
		nWeather = 0;
	}
	else if ((nWeather == 2 || nWeather == 3) && (RenderDevice::m_bDungeon == 3 || RenderDevice::m_bDungeon == 4))
	{
		nWeather = 1;
	}
	else
	{
		if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 &&
			(int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
			(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 &&
			(int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
		{
			if (nWeather == 1)
				nWeather = 3;
			else
				nWeather = 2;
		}
	}

	switch (nWeather)
	{
	case 1:
		m_pRain->m_bVisible = 1;
		m_pSnow->m_bVisible = 0;
		m_pSnow2->m_bVisible = 0;
		m_pSky->SetWeatherState(11);
		break;
	case 2:
		m_pRain->m_bVisible = 0;
		m_pSnow->m_bVisible = 1;
		m_pSnow2->m_bVisible = 0;
		m_pSky->SetWeatherState(11);
		break;
	case 3:
		m_pRain->m_bVisible = 0;
		m_pSnow->m_bVisible = 1;
		m_pSnow2->m_bVisible = 1;
		m_pSky->SetWeatherState(11);
		break;
	}
}

void TMFieldScene::SetVisibleKhepraPortal(bool bVisible)
{
	if (m_pKhepraPortal && m_pKhepraPortalEff1 && m_pKhepraPortalEff2)
	{
		if (bVisible == 1)
		{
			m_pKhepraPortal->m_fHeight = -8.3000002f;
			m_pKhepraPortalEff1->m_vecPosition.y = -4.73f;
			m_pKhepraPortalEff2->m_vecPosition.y = -7.8000002f;
		}
		else
		{
			m_pKhepraPortal->m_fHeight = -20.0f;
			m_pKhepraPortalEff1->m_vecPosition.y = -20.0f;
			m_pKhepraPortalEff2->m_vecPosition.y = -20.0f;
		}
	}
}

int TMFieldScene::OnPacketCreateMobCompat(MSG_STANDARD* pStd)
{
	// The compact 7.48 field path materializes only renderable entities.  The
	// full source handler also updates party, trade, target and HUD controls;
	// those controls are absent in the legacy resource and must not be touched
	// while the character/world synchronization is being established.
	if (!pStd || !m_pHumanContainer || !m_pGround)
		return 1;

	auto pCreateMob = reinterpret_cast<MSG_CreateMob*>(pStd);
	if (pCreateMob->MobID == g_pObjectManager->m_dwCharID)
	{
		// The authoritative self was built from the 0x114 MOB before this packet;
		// creating a second TMHuman would duplicate labels and skin resources.
		return 1;
	}

	if (g_pObjectManager->GetHumanByID(pCreateMob->MobID))
		return 1;

	auto pHuman = new TMHuman(this);
	if (!pHuman)
		return 1;

	pHuman->m_dwID = pCreateMob->MobID;
	pHuman->m_usGuild = pCreateMob->Guild;
	pHuman->m_citizen = pCreateMob->Server;
	pHuman->m_nCurrentKill = 0;
	pHuman->m_nTotalKill = 0;
	pHuman->m_ucChaosLevel = 75;
	pCreateMob->MobName[15] = 0;
	pCreateMob->Nick[25] = 0;
	sprintf_s(pHuman->m_szName, "%s", pCreateMob->MobName);
	sprintf_s(pHuman->m_szNickName, "%s", pCreateMob->Nick);

	// Equipment and affects are copied before InitObject because the skin
	// builder derives the visible mesh, mount and weapon state from these fields.
	memcpy(pHuman->m_usAffect, pCreateMob->Affect, sizeof(pHuman->m_usAffect));
	pHuman->SetPacketEquipItem(pCreateMob->Equip);
	pHuman->SetColorItem(reinterpret_cast<char*>(pCreateMob->Equip2));
	memcpy(&pHuman->m_stScore, &pCreateMob->Score, sizeof(pHuman->m_stScore));
	pHuman->SetCharHeight(static_cast<float>(pHuman->m_stScore.Con));
	pHuman->SetRace(pCreateMob->Equip[0] & 0x0FFF);
	pHuman->InitObject();
	if (pStd->Type == MSG_CreateMobTrade_Opcode)
	{
		auto pCreateMobTrade = reinterpret_cast<MSG_CreateMobTrade*>(pStd);

		// The native 7.48 lifecycle uses this same field to render the shop title
		// and to enable the 0x39A click request, so the compat spawn must retain it.
		memcpy(pHuman->m_TradeDesc, pCreateMobTrade->Desc, sizeof(pHuman->m_TradeDesc));
		pHuman->m_TradeDesc[sizeof(pHuman->m_TradeDesc) - 1] = 0;
		if (pHuman->m_pAutoTradeDesc)
			pHuman->m_pAutoTradeDesc->SetText(pHuman->m_TradeDesc, 0);
	}
	else
	{
		memset(pHuman->m_TradeDesc, 0, sizeof(pHuman->m_TradeDesc));
		if (pHuman->m_pAutoTradeDesc)
			pHuman->m_pAutoTradeDesc->SetText(pHuman->m_TradeDesc, 0);
	}
	pHuman->CheckAffect();
	pHuman->CheckWeapon(pCreateMob->Equip[6] & 0x0FFF, pCreateMob->Equip[7] & 0x0FFF);

	// Reserved's high nibble is the native direction field; preserving it makes
	// observers see the same initial facing as the authoritative 7.48 packet.
	float fAngle = 0.0f;
	switch ((static_cast<unsigned char>(pHuman->m_stScore.Merchant) >> 4) & 0x0F)
	{
	case 6: fAngle = D3DXToRadian(180.0f); break;
	case 9: fAngle = D3DXToRadian(135.0f); break;
	case 8: fAngle = D3DXToRadian(90.0f); break;
	case 7: fAngle = D3DXToRadian(45.0f); break;
	case 1: fAngle = D3DXToRadian(315.0f); break;
	case 2: fAngle = D3DXToRadian(270.0f); break;
	case 3: fAngle = D3DXToRadian(225.0f); break;
	default: break;
	}
	pHuman->InitAngle(0.0f, fAngle, 0.0f);

	const TMVector2 vecPosition{
		static_cast<float>(pCreateMob->PosX) + 0.5f,
		static_cast<float>(pCreateMob->PosY) + 0.5f};
	pHuman->InitPosition(vecPosition.x, m_pGround->GetHeight(vecPosition), vecPosition.y);
	pHuman->m_cHide = (pHuman->m_dwID < 1000) && ((pHuman->m_stScore.Merchant & 1) != 0);
	m_pHumanContainer->AddChild(pHuman);
	return 1;
}

int TMFieldScene::OnPacketSoundEffect(MSG_STANDARDPARM* pStd)
{
	int nSoundIndex = pStd->Parm;

	GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
	return 1;
}

int TMFieldScene::OnPacketCreateMob(MSG_STANDARD* pStd)
{
	if (m_pHumanContainer == nullptr)
		return 1;

	auto pCreateMob = (MSG_CreateMob*)pStd;
	auto pCreateMobTrade = (MSG_CreateMobTrade*)pStd;

	auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pCreateMob->MobID);

	std::string qwe{ pCreateMob->MobName };

	STRUCT_AFFECT tempAffect[32]{};

	TMHuman* pHuman = nullptr;
	if (pCreateMob->MobID == m_pMyHuman->m_dwID)
	{
		for (int l = 0; l < 32; ++l)
			memcpy(&tempAffect[l], &m_pMyHuman->m_stAffect[l], sizeof(STRUCT_AFFECT));

		g_nTempArray[2] = (int)&m_pMyHuman->m_vecPosition;
		g_nTempArray2[2] = (int)&m_pMyHuman->m_vecPosition;
		if (!m_stMoveStop.NextX)
		{
			m_stMoveStop.NextX = pCreateMob->PosX;
			m_stMoveStop.NextY = pCreateMob->PosY;
		}
	}
	if (pStd->Type == MSG_CreateMobTrade_Opcode && pCreateMobTrade->Desc[0])
	{
		int len = strlen(pCreateMobTrade->Desc);
		if (len > 0)
		{
			for (int i = 1; i < MAX_EQUIPITEM; ++i)
			{
				pCreateMob->Equip[i] &= 0xFFF;
				pCreateMob->Equip2[i] = 0;
			}

			for (int i = 1; i < 32; ++i)
				pCreateMob->Affect[i] = 0;

			pCreateMob->Equip[0] = 230;
			pCreateMob->Equip[6] = 0;
			pCreateMob->Equip[7] = 0;
			pCreateMob->Equip[14] = 0;
			pCreateMob->Score.Con = 15000;
		}
	}
	if (!pNode)
	{
		pHuman = new TMHuman(this);
		if (pHuman == nullptr)
			return 1;

		pHuman->m_dwID = pCreateMob->MobID;
		pHuman->m_usGuild = pCreateMob->Guild;

		if (pCreateMob->MobID > 0 && pCreateMob->MobID < 1000)
		{
			char cCurrent = pCreateMob->MobName[13];
			short* sTotal = (short*)&pCreateMob->MobName[14];
			pHuman->m_nCurrentKill = (unsigned char)cCurrent;
			pHuman->m_nTotalKill = *sTotal;
			pHuman->m_ucChaosLevel = pCreateMob->MobName[12];
			pCreateMob->MobName[12] = 0;
			pCreateMob->MobName[15] = 0;
			pCreateMob->Nick[25] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);
			sprintf(pHuman->m_szNickName, "%s", pCreateMob->Nick);
			pHuman->m_citizen = pCreateMob->Server;
		}
		else
		{
			pHuman->m_nCurrentKill = 0;
			pHuman->m_nTotalKill = 0;
			pHuman->m_ucChaosLevel = 75;
			pCreateMob->MobName[15] = 0;
			pCreateMob->Nick[25] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);
			sprintf(pHuman->m_szNickName, "%s", pCreateMob->Nick);
			pHuman->m_citizen = 0;
		}

		memcpy(pHuman->m_usAffect, pCreateMob->Affect, sizeof(pHuman->m_usAffect));

		pHuman->SetPacketEquipItem(pCreateMob->Equip);

		// CreateMob carries imported KR mounts in the full slot-14 word. Keep the
		// legacy Equip2 selector only when that full ID was not handled.
		if (!pHuman->SetImportedMountCostume(pCreateMob->Equip[14])
			&& (pCreateMob->Equip[14] & 0xFFF) && pCreateMob->Equip2[14] &&
			((pCreateMob->Equip2[14] & 0xFFF)< 3980 || (pCreateMob->Equip2[14] & 0xFFF) >= 3999))
		{
			pHuman->SetMountCostume((unsigned char)pCreateMob->Equip2[14]);
		}

		pHuman->SetColorItem(pCreateMob->Equip2);
		pHuman->m_fMaxSpeed = 2.0f;

		memcpy(&pHuman->m_stScore, &pCreateMob->Score, sizeof(pHuman->m_stScore));

		float fCon = (float)pHuman->m_stScore.Con;
		pHuman->SetCharHeight(fCon);
		if (pCreateMob->Equip[0] < 40)
			pHuman->m_fScale = pHuman->m_fScale * 0.89999998f;

		pHuman->SetRace(pCreateMob->Equip[0] & 0xFFF);

		STRUCT_ITEM tepFace{};
		memcpy(&tepFace, &pCreateMob->Equip[0], sizeof(tepFace));
		tepFace.sIndex = pCreateMob->Equip[0];

		STRUCT_ITEM itemL{};
		itemL.sIndex = pCreateMob->Equip[6] & 0xFFF;
		int nWeaponTypeL = BASE_GetItemAbility(&itemL, 21);

		if (nWeaponTypeL == 41)
		{
			pHuman->m_stLookInfo.RightMesh = pHuman->m_stLookInfo.LeftMesh;
			pHuman->m_stLookInfo.RightSkin = pHuman->m_stLookInfo.LeftSkin;
			pHuman->m_stSancInfo.Sanc6 = pHuman->m_stSancInfo.Sanc7;
			pHuman->m_stSancInfo.Legend6 = pHuman->m_stSancInfo.Legend7;
		}

		pHuman->InitObject();

		if (pHuman == m_pMyHuman)
		{
			for (int k = 0; k < 32; ++k)
				memcpy(&m_pMyHuman->m_stAffect[k], &tempAffect[k], sizeof(STRUCT_AFFECT));
		}

		pHuman->CheckAffect();
		pHuman->CheckWeapon(pCreateMob->Equip[6] & 0xFFF, pCreateMob->Equip[7] & 0xFFF);

		pHuman->m_sGuildLevel = (unsigned char)pCreateMob->GuildLevel;
		unsigned short usGuild = pCreateMob->Guild;

		if (usGuild)
		{
			int nSubGuild = BASE_GetSubGuild(pHuman->m_sGuildLevel);
			pHuman->m_stGuildMark.nGuild = usGuild & 0xFFF;
			pHuman->m_stGuildMark.nSubGuild = nSubGuild;
			pHuman->m_stGuildMark.nGuildChannel = ((signed int)usGuild >> 12) & 0xF;

			switch (pHuman->m_sGuildLevel)
			{
			case 0:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 1:
				pHuman->m_stGuildMark.sGuildIndex = 535;
				break;
			case 2:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 3:
				pHuman->m_stGuildMark.sGuildIndex = 526;
				break;
			case 4:
				pHuman->m_stGuildMark.sGuildIndex = 527;
				break;
			case 5:
				pHuman->m_stGuildMark.sGuildIndex = 528;
				break;
			case 6:
				pHuman->m_stGuildMark.sGuildIndex = 529;
				break;
			case 7:
				pHuman->m_stGuildMark.sGuildIndex = 530;
				break;
			case 8:
				pHuman->m_stGuildMark.sGuildIndex = 531;
				break;
			case 9:
				pHuman->m_stGuildMark.sGuildIndex = 509;
				break;
			}

			if (pStd->Type != MSG_CreateMobTrade_Opcode && !pHuman->m_pAutoTradeDesc->IsVisible())
				Guildmark_Create(&pHuman->m_stGuildMark);
		}

		float fAngle = 0.0f;
		unsigned char nDir = ((unsigned char)pHuman->m_stScore.Merchant >> 4);
		if (nDir == 6)
			fAngle = D3DXToRadian(180);
		if (nDir == 9)
			fAngle = D3DXToRadian(135);
		if (nDir == 8)
			fAngle = D3DXToRadian(90);
		if (nDir == 7)
			fAngle = D3DXToRadian(45);
		if (nDir == 4)
			fAngle = D3DXToRadian(0);
		if (nDir == 1)
			fAngle = D3DXToRadian(315);
		if (nDir == 2)
			fAngle = D3DXToRadian(270);
		if (nDir == 3)
			fAngle = D3DXToRadian(225);

		pHuman->InitAngle(0.0f, fAngle, 0.0f);

		TMVector2 vecPosition{ (float)pCreateMob->PosX + 0.5f, (float)pCreateMob->PosY + 0.5f };

		pHuman->InitPosition(vecPosition.x, GroundGetMask(vecPosition) * 0.1f, vecPosition.y);

		pHuman->m_cHide = (pHuman->m_dwID >= 0 && pHuman->m_dwID < 1000) && pHuman->m_stScore.Merchant & 1;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && (pHuman->IsMerchant() || (pHuman->m_stScore.Merchant & 0xF) == 15))
			pHuman->m_pNameLabel->SetTextColor(0xFFAAFFAA);

		int nItemCode = pCreateMob->Equip[13] & 0xFFF;
		int nHP = 0;
		if (nItemCode == 786 || nItemCode == 1936 || nItemCode == 1937)
		{
			int nHp = 0;
			int nSanc = (int)pCreateMob->Equip[13] >> 12;
			if (nSanc < 2)
				nSanc = 2;
			if (nItemCode == 1936)
			{
				nSanc *= 10;
			}
			else if (nItemCode == 1937)
			{
				nSanc *= 1000;
			}

			// The canonical Score renamed the ambiguous Hp member to CurHP.
			int tmp = pCreateMob->Score.CurHP * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;

			pHuman->m_BigHp = tmp;
			tmp = pCreateMob->Score.MaxHP * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;
			pHuman->m_MaxBigHp = tmp;
			pHuman->m_usHP = pHuman->m_MaxBigHp;
		}

		if (pHuman->m_nClass == 56 && !pHuman->m_stLookInfo.FaceMesh)
			m_dwKhepraID = pHuman->m_dwID;

		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) &&
			(int)vecPosition.x >> 7 >= 28 && (int)vecPosition.x >> 7 <= 30 &&
			(int)vecPosition.y >> 7 >= 27 && (int)vecPosition.y >> 7 <= 28)
		{
			pHuman->SetInMiniMap(0xAAFF0000);
		}

		m_pHumanContainer->AddChild(pHuman);
	}
	else if (pNode->m_nWillDie)
	{
		unsigned int dwOldMoveTime = pNode->m_dwOldMovePacketTime;
		pNode->m_nWillDie = -1;
		pNode->m_dwDeadTime = 0;

		pHuman = pNode;
		pHuman->Init();

		pHuman->m_dwID = pCreateMob->MobID;
		pHuman->m_usGuild = pCreateMob->Guild;

		if (pCreateMob->MobID > 0 && pCreateMob->MobID < 1000)
		{
			char cCurrent = pCreateMob->MobName[13];
			short* sTotal = (short*)&pCreateMob->MobName[14];
			pHuman->m_nCurrentKill = (unsigned char)cCurrent;
			pHuman->m_nTotalKill = *sTotal;
			pHuman->m_ucChaosLevel = pCreateMob->MobName[12];
			pCreateMob->MobName[12] = 0;
			pCreateMob->MobName[15] = 0;
			pCreateMob->MobName[14] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);
		}
		else
		{
			pHuman->m_nCurrentKill = 0;
			pHuman->m_nTotalKill = 0;
			pHuman->m_ucChaosLevel = 75;
			pCreateMob->MobName[15] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);

			int nItemCode = pCreateMob->Equip[13] & 0xFFF;
			int nHP = 0;
			if (nItemCode == 786 || nItemCode == 1936 || nItemCode == 1937)
			{
				int nHp = 0;
				int nSanc = (int)pCreateMob->Equip[13] >> 12;
				if (nSanc < 2)
					nSanc = 2;
				if (nItemCode == 1936)
				{
					nSanc *= 10;
				}
				else if (nItemCode == 1937)
				{
					nSanc *= 1000;
				}

				// Big-HP display scales the canonical current HP value.
				int tmp = pCreateMob->Score.CurHP * nSanc;
				if (tmp > 2000000000)
					tmp = 2000000000;

				pHuman->m_BigHp = tmp;
				tmp = pCreateMob->Score.MaxHP * nSanc;
				if (tmp > 2000000000)
					tmp = 2000000000;
				pHuman->m_MaxBigHp = tmp;
				pHuman->m_usHP = pHuman->m_MaxBigHp;
			}
		}

		pCreateMob->Nick[25] = 0;

		sprintf(pHuman->m_szNickName, "%s", pCreateMob->Nick);
		memcpy(pHuman->m_usAffect, pCreateMob->Affect, sizeof(pHuman->m_usAffect));

		pHuman->SetPacketEquipItem(pCreateMob->Equip);

		// This second native spawn path must obey the same full-ID precedence or
		// nearby entities regress to the truncated costume byte after creation.
		if (!pHuman->SetImportedMountCostume(pCreateMob->Equip[14])
			&& (pCreateMob->Equip[14] & 0xFFF) && pCreateMob->Equip2[14] &&
			((pCreateMob->Equip2[14] & 0xFFF) < 3980 || (pCreateMob->Equip2[14] & 0xFFF) >= 3999))
		{
			pHuman->SetMountCostume((unsigned char)pCreateMob->Equip2[14]);
		}

		pHuman->SetColorItem(pCreateMob->Equip2);
		pHuman->m_fMaxSpeed = 2.0f;

		memcpy(&pHuman->m_stScore, &pCreateMob->Score, sizeof(pHuman->m_stScore));

		float fCon = (float)pHuman->m_stScore.Con;
		pHuman->SetCharHeight(fCon);

		int SkinMeshType = pHuman->m_nSkinMeshType;
		bool bWasMobTrade = false;
		if (pHuman->m_nClass == 29)
			bWasMobTrade = true;

		pHuman->SetRace(pCreateMob->Equip[0] & 0xFFF);

		if (pStd->Type != MSG_CreateMobTrade_Opcode && !bWasMobTrade)
			pHuman->m_nSkinMeshType = SkinMeshType;

		STRUCT_ITEM itemL{};
		itemL.sIndex = pCreateMob->Equip[6] & 0xFFF;
		int nWeaponTypeL = BASE_GetItemAbility(&itemL, 21);

		if (nWeaponTypeL == 41)
		{
			pHuman->m_stLookInfo.RightMesh = pHuman->m_stLookInfo.LeftMesh;
			pHuman->m_stLookInfo.RightSkin = pHuman->m_stLookInfo.LeftSkin;
			pHuman->m_stSancInfo.Sanc6 = pHuman->m_stSancInfo.Sanc7;
			pHuman->m_stSancInfo.Legend6 = pHuman->m_stSancInfo.Legend7;
		}
		if (pHuman == m_pMyHuman)
		{
			for (int k = 0; k < 32; ++k)
				memcpy(&m_pMyHuman->m_stAffect[k], &tempAffect[k], sizeof(STRUCT_AFFECT));
		}

		pHuman->CheckAffect();
		pHuman->CheckWeapon(pCreateMob->Equip[6] & 0xFFF, pCreateMob->Equip[7] & 0xFFF);
		pHuman->InitObject();

		float fAngle = 0.0f;
		unsigned char nDir = ((unsigned char)pHuman->m_stScore.Merchant >> 4);
		if (nDir == 6)
			fAngle = D3DXToRadian(180);
		if (nDir == 9)
			fAngle = D3DXToRadian(135);
		if (nDir == 8)
			fAngle = D3DXToRadian(90);
		if (nDir == 7)
			fAngle = D3DXToRadian(45);
		if (nDir == 4)
			fAngle = D3DXToRadian(0);
		if (nDir == 1)
			fAngle = D3DXToRadian(315);
		if (nDir == 2)
			fAngle = D3DXToRadian(270);
		if (nDir == 3)
			fAngle = D3DXToRadian(225);

		pHuman->InitAngle(0.0f, fAngle, 0.0f);

		TMVector2 vecPosition{ (float)pCreateMob->PosX + 0.5f, (float)pCreateMob->PosY + 0.5f };

		pHuman->InitPosition(vecPosition.x, GroundGetMask(vecPosition) * 0.1f, vecPosition.y);

		pHuman->m_sGuildLevel = (unsigned char)pCreateMob->GuildLevel;
		unsigned short usGuild = pCreateMob->Guild;

		if (usGuild)
		{
			int nSubGuild = BASE_GetSubGuild(pHuman->m_sGuildLevel);
			pHuman->m_stGuildMark.nGuild = usGuild & 0xFFF;
			pHuman->m_stGuildMark.nSubGuild = nSubGuild;
			pHuman->m_stGuildMark.nGuildChannel = ((signed int)usGuild >> 12) & 0xF;

			switch (pHuman->m_sGuildLevel)
			{
			case 0:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 1:
				pHuman->m_stGuildMark.sGuildIndex = 535;
				break;
			case 2:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 3:
				pHuman->m_stGuildMark.sGuildIndex = 526;
				break;
			case 4:
				pHuman->m_stGuildMark.sGuildIndex = 527;
				break;
			case 5:
				pHuman->m_stGuildMark.sGuildIndex = 528;
				break;
			case 6:
				pHuman->m_stGuildMark.sGuildIndex = 529;
				break;
			case 7:
				pHuman->m_stGuildMark.sGuildIndex = 530;
				break;
			case 8:
				pHuman->m_stGuildMark.sGuildIndex = 531;
				break;
			case 9:
				pHuman->m_stGuildMark.sGuildIndex = 509;
				break;
			}

			if (pStd->Type != MSG_CreateMobTrade_Opcode && !pHuman->m_pAutoTradeDesc->IsVisible())
				Guildmark_Create(&pHuman->m_stGuildMark);
		}

		pHuman->m_cHide = (pHuman->m_dwID >= 0 && pHuman->m_dwID < 1000) && pHuman->m_stScore.Merchant & 1;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && (pHuman->m_stScore.Merchant & 0xF) >= 1 && (pHuman->m_stScore.Merchant & 0xF) <= 15)
			pHuman->m_pNameLabel->SetTextColor(0xFFAAFFAA);

		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && pHuman->m_sHeadIndex == 54)
			pHuman->m_pNameLabel->SetTextColor(0xFFAAFFAA);

		if (pHuman == m_pMyHuman)
			UpdateScoreUI(0);

		pNode->m_dwOldMovePacketTime = dwOldMoveTime;
	}
	else
	{
		pNode->m_nWillDie = -1;
		pNode->m_dwDeadTime = 0;

		int nItemCode = pCreateMob->Equip[13] & 0xFFF;
		int nHP = 0;
		if (nItemCode == 786 || nItemCode == 1936 || nItemCode == 1937)
		{
			int nHp = 0;
			int nSanc = (int)pCreateMob->Equip[13] >> 12;
			if (nSanc < 2)
				nSanc = 2;
			if (nItemCode == 1936)
			{
				nSanc *= 10;
			}
			else if (nItemCode == 1937)
			{
				nSanc *= 1000;
			}

			// Big-HP display scales the canonical current HP value.
			int tmp = pCreateMob->Score.CurHP * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;

			pNode->m_BigHp = tmp;
			tmp = pCreateMob->Score.MaxHP * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;
			pNode->m_MaxBigHp = tmp;
			pNode->m_usHP = pNode->m_MaxBigHp;
			pNode->UpdateScore(0);
		}
	}
	if (pHuman)
	{
		pHuman->m_pNameLabel->m_GCBorder.dwColor = 0x55AA0000;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && !pHuman->m_stScore.Defense)
		{
			pHuman->m_pNameLabel->m_GCBorder.dwColor = 0x5500AA00;
			pHuman->m_pNameLabel->m_cBorder = 1;
			pHuman->m_cSummons = 1;
		}
		if (pStd->Type == MSG_CreateMobTrade_Opcode)
		{
			pCreateMobTrade->Desc[22] = 0;
			pCreateMobTrade->Desc[21] = 0;

			sprintf(pHuman->m_TradeDesc, pCreateMobTrade->Desc);
			pHuman->m_pAutoTradeDesc->SetText(pHuman->m_TradeDesc, 0);
		}
		else
		{
			memset(pHuman->m_TradeDesc, 0, sizeof(pHuman->m_TradeDesc));
			pHuman->m_pAutoTradeDesc->SetText((char*)"", 0);

			auto pPanel = m_pAutoTrade;
			if (pPanel && pPanel->IsVisible() == 1 && pHuman->m_dwID == m_stAutoTrade.TargetID)
			{
				SetVisibleAutoTrade(0, 0);
			}
		}
	}

	if (pHuman && !pHuman->m_cHide)
	{
		if (!pHuman->m_cHide && ((pCreateMob->CreateType & 0x7FFF) == 2 || (pCreateMob->CreateType & 0x7FFF) == 3))
		{
			TMVector3 vecEffectPos = TMVector3((float)pCreateMob->PosX + 0.5f,
				GroundGetMask(pHuman->m_vecPosition) * 0.1f + 0.05000000f,
				(float)pCreateMob->PosY + 0.5f);

			if (!pHuman->m_nSkinMeshType || pHuman->m_nSkinMeshType == 1)
			{
				auto pEffect = new TMEffectStart(vecEffectPos, 0, nullptr);

				if (pEffect && m_pEffectContainer)
					m_pEffectContainer->AddChild(pEffect);
			}
			else if ((pCreateMob->CreateType & 0x7FFF) != 3 && pHuman->m_nSkinMeshType != 35 && pHuman->m_nSkinMeshType != 36)
			{
				auto pChild = new TMEffectStart(vecEffectPos, 1, nullptr);

				if (pChild && m_pEffectContainer)
					m_pEffectContainer->AddChild(pChild);

				auto pSoundManager = g_pSoundManager;
				if (pSoundManager && m_pMyHuman == pHuman)
				{
					auto pSoundData = pSoundManager->GetSoundData(151);
					pSoundData->Play();
				}
			}
			if ((pCreateMob->CreateType & 0x7FFF) == 3)
			{
				if (pHuman->m_nClass == 62 && pHuman->m_stLookInfo.FaceMesh == 2)
				{
					TMVector2 effectPos{ (float)pCreateMob->PosX + 0.5f, (float)pCreateMob->PosY + 0.5f };
					pHuman->InitPosition(effectPos.x,
						(GroundGetMask(effectPos) * 0.1f) - 2.0f,
						effectPos.y);

					TMVector3 vecPos = TMVector3(effectPos.x, (GroundGetMask(effectPos) * 0.1f) + 0.2f, effectPos.y);
					auto pJudgement = new TMSkillJudgement(vecPos, 4, 0.1f);

					if (pJudgement && m_pEffectContainer)
						m_pEffectContainer->AddChild(pJudgement);
				}
				else
				{
					int nType = 1;
					if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000))
						nType = 3;

					auto pPortal = new TMSkillTownPortal(vecEffectPos, nType);

					if (pPortal && m_pEffectContainer)
						m_pEffectContainer->AddChild(pPortal);
				}
			}
			pHuman->SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);

			if (pHuman->m_nSkinMeshType == 35 || pHuman->m_nSkinMeshType == 36)
			{
				auto pSoundManager = g_pSoundManager;
				if (pSoundManager && m_pMyHuman == pHuman)
				{
					auto pSoundData = pSoundManager->GetSoundData(303);
					pSoundData->Play();
				}

				if (!g_bHideEffect)
				{
					for (int m = -3; m < 3; ++m)
					{
						auto pBillEffect = new TMEffectBillBoard(193, 4000, 1.0f, 1.0f, 1.0f, 0.001f, 1, 80);

						if (pBillEffect)
						{
							pBillEffect->m_bStickGround = m % 2;
							pBillEffect->m_vecPosition = TMVector3(((float)m * 0.5f) + pHuman->m_vecPosition.x,
								pHuman->m_fHeight,
								((float)m * 0.5f) + pHuman->m_vecPosition.y);

							m_pEffectContainer->AddChild(pBillEffect);
						}

						auto pBillEffect2 = new TMEffectBillBoard(193, 4000, 1.0f, 1.0f, 1.0f, 0.001f, 1, 80);
						if (pBillEffect2)
						{
							pBillEffect2->m_bStickGround = m % 2;
							pBillEffect2->m_vecPosition = TMVector3(pHuman->m_vecPosition.x - ((float)m * 0.5f),
								pHuman->m_fHeight,
								pHuman->m_vecPosition.y - ((float)m * 0.5f));

							m_pEffectContainer->AddChild(pBillEffect2);
						}
					}
				}
			}
			else
			{
				auto pLevelUp = new TMEffectLevelUp(vecEffectPos, 0);

				if (pLevelUp)
					m_pEffectContainer->AddChild(pLevelUp);
			}
		}
		if (!pHuman->m_nSkinMeshType || pHuman->m_nSkinMeshType == 1)
		{
			if ((pCreateMob->CreateType & 0xF0) == 16)
				pHuman->SetAnimation(ECHAR_MOTION::ECMOTION_PUNISHING, 1);
			else if ((pCreateMob->CreateType & 0xF0) == 32)
				pHuman->SetAnimation(ECHAR_MOTION::ECMOTION_SEATING, 1);
		}
	}
	if (pHuman == m_pMyHuman)
	{
		m_vecMyNext.x = (int)m_pMyHuman->m_vecPosition.x;
		m_vecMyNext.y = (int)m_pMyHuman->m_vecPosition.y;
		Bag_View();
	}

	auto pPartyList = m_pPartyList;
	if (pPartyList)
	{
		for (int n = 0; n < pPartyList->m_nNumItem; ++n)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[n];
			if (pPartyItem->m_dwCharID == pCreateMob->MobID)
			{
				auto pPartyHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
				if (pPartyHuman && pPartyItem->m_nState != 1)
				{
					pPartyHuman->m_bParty = 1;
					pPartyHuman->SetInMiniMap(0xAAFFFF00);
				}
				if (pPartyItem->m_nState == 4)
				{
					pPartyItem->m_nState = 2;
					pPartyItem->m_GCText.dwColor = 0x0FFAAAAFF;
					pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString,
						pPartyItem->m_GCText.dwColor,
						0);
				}
				else if (pPartyItem->m_nState == 3)
				{
					pPartyItem->m_nState = 0;
					pPartyItem->m_GCText.dwColor = 0x0FFFFFFFF;
					pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString,
						pPartyItem->m_GCText.dwColor,
						0);
				}
				break;
			}
		}
	}

	if (pHuman &&
		(_locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 8, 15) || _locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 8, 16)	||
		 _locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 9, 15) || _locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 9, 16)))
	{
		if (pHuman->m_cMantua == 1)
			pHuman->SetInMiniMap(0xAA0000FF);
		if (pHuman->m_cMantua == 2)
			pHuman->SetInMiniMap(0xAAFF0000);
	}

	return 1;
}

int TMFieldScene::OnPacketWeather(MSG_STANDARDPARM* pStd)
{
	SetWeather(pStd->Parm);
	return 1;
}

int TMFieldScene::OnPacketCreateItem(MSG_CreateItem* pMsg)
{
	if (!pMsg || !g_pObjectManager || !m_pGround || !m_pItemContainer ||
		!IsGroundItemDefinitionIndex(pMsg->Item.sIndex))
		return 0;

	auto pOldItem = (TMItem*)g_pObjectManager->GetItemByID(pMsg->ItemID);
	const bool isGate = BASE_GetItemAbility(&pMsg->Item, 34) > 0;
	const bool isCannon = !isGate && g_pItemList[pMsg->Item.sIndex].nIndexMesh == 1607;
	// An existing ID may be updated in place only when its concrete renderer
	// still matches the incoming item. Casting a regular item to TMGate would
	// write gate fields beyond the allocated object.
	if (pOldItem &&
		(isGate != (dynamic_cast<TMGate*>(pOldItem) != nullptr) ||
		 isCannon != (dynamic_cast<TMCannon*>(pOldItem) != nullptr)))
		return 0;

	if (isGate)
	{
		TMGate* pItem = nullptr;

		if (pOldItem)
			pItem = static_cast<TMGate*>(pOldItem);
		else
		{
			pItem = new TMGate();
		}

		if (!pItem)
			return 1;

		pItem->InitItem(pMsg->Item);
		pItem->InitGate(pMsg->Item);

		pItem->m_dwID = pMsg->ItemID;
		pItem->m_nMaskIndex = 0;
		pItem->InitObject();
		pItem->InitAngle(0.0f, ((float)pMsg->Rotate * D3DXToRadian(180)) / 2.0f, 0.0f);

		float fX = (float)pMsg->GridX + 0.5f;
		float fY = (float)pMsg->GridY + 0.5f;

		float fHeight = m_pGround->GetHeight(TMVector2(fX, fY));

		if (fHeight < -500.0f)
		{
			auto pOtherGround = m_pGroundList[(m_nCurrentGroundIndex + 1) % 2];
			if (pOtherGround)
				fHeight = pOtherGround->GetHeight(TMVector2(fX, fY));
			else
			{
				TMScene::FrameMove(0);
				pOtherGround = m_pGroundList[(m_nCurrentGroundIndex + 1) % 2];
				if (pOtherGround)
					fHeight = pOtherGround->GetHeight(TMVector2(fX, fY));
			}
		}

		pItem->InitPosition(fX, fHeight, fY);
		pItem->m_sAuth = 1;

		int nMaskIndex = BASE_GetItemAbility(&pMsg->Item, 34);
		BASE_UpdateItem2(nMaskIndex, 1, (unsigned char)pMsg->State, pMsg->GridX, pMsg->GridY, (char*)m_HeightMapData,
			(int)(pItem->m_fAngle / D3DXToRadian(90)), pMsg->Height);

		pItem->SetState((EGATE_STATE)pMsg->State);
		if (!pOldItem)
			m_pItemContainer->AddChild(pItem);

		return 1;
	}

	TMItem* pItem = nullptr;
	if (pOldItem)
		pItem = pOldItem;
	else if (isCannon)
	{
		pItem = new TMCannon();
		pItem->m_dwObjType = 1607;
	}
	else
	{
		pItem = new TMItem();
	}

	if (!pItem)
		return 1;

	pItem->InitItem(pMsg->Item);
	pItem->m_dwID = pMsg->ItemID;
	pItem->m_nMaskIndex = 0;
	pItem->InitObject();
	pItem->InitAngle(0.0f, ((float)pMsg->Rotate * D3DXToRadian(180)) / 2.0f, 0.0f);

	float fX = (float)pMsg->GridX + 0.5f;
	float fY = (float)pMsg->GridY + 0.5f;

	float fHeight = GroundGetMask(TMVector2(fX, fY)) * 0.1f;
	pItem->InitPosition(fX, fHeight + 0.1f, fY);

	if (!pOldItem)
		m_pItemContainer->AddChild(pItem);

	if (pMsg->Create == 1)
	{
		if (BASE_GetItemAbility(&pMsg->Item, 38) == 2)
			GetSoundAndPlay(44, 0, 0);
		else if (pMsg->Item.sIndex == 412 || pMsg->Item.sIndex == 413 || pMsg->Item.sIndex == 4141 ||
			pMsg->Item.sIndex == 419 || pMsg->Item.sIndex == 420)
		{
			GetSoundAndPlay(48, 0, 0);
		}
		else if (pMsg->Item.sIndex == 747)
		{
			GetSoundAndPlay(306, 0, 0);
		}
		else
			GetSoundAndPlay(45, 0, 0);
	}

	return 1;
}

int TMFieldScene::OnPacketEnvEffect(MSG_STANDARD* pStd)
{
	auto pEnvEffect = reinterpret_cast<MSG_EnvEffect*>(pStd);
	if (pEnvEffect->x1 > pEnvEffect->x2 || pEnvEffect->y1 > pEnvEffect->y2)
		return 1;

	for (int nY = pEnvEffect->y1; nY < pEnvEffect->y2; nY += 3)
	{
		for (int nX = pEnvEffect->x1 + 1; nX < pEnvEffect->x2; nX += 4)
		{
			unsigned int dwColor = 0x44444444;

			TMVector2 vec{ (float)nX, (float)nY + 1.0f };
			float fHeight = (float)GroundGetMask(vec) * 0.1f;
			if (pEnvEffect->Effect == 32)
			{
				auto pExplosion = new TMSkillExplosion2(TMVector3(vec.x, fHeight, vec.y), 0, 1.5f, 210, dwColor);
				m_pEffectContainer->AddChild(pExplosion);
			}
		}
	}

	return 1;
}
