#include "pch.h"
#include "WeatherPacket.h"
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
	// Record the server weather even while a BerialGuard aura overrides it.
	m_berielAura.SetServer(nWeather);
	g_nWeather = m_berielAura.Effective();

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

int TMFieldScene::OnPacketCreateMobCompat(MSG_STANDARD* pStd)
{
	// The compact 7.48 field path materializes only renderable entities.  The
	// full source handler also updates party, trade, target and HUD controls;
	// those controls are absent in the legacy resource and must not be touched
	// while the character/world synchronization is being established.
	if (!pStd || !m_pHumanContainer || !m_pGround)
		return 1;

	auto pCreateMob = reinterpret_cast<MSG_CreateMob*>(pStd);
	auto pExisting = pCreateMob->MobID == g_pObjectManager->m_dwCharID
		? m_pMyHuman : g_pObjectManager->GetHumanByID(pCreateMob->MobID);
	if (pExisting)
	{
		// A CP refresh must not respawn the entity or interrupt movement.
		if (pCreateMob->MobID < 1000)
			pExisting->m_ucChaosLevel = static_cast<unsigned char>(pCreateMob->MobName[12]);
		pExisting->UpdateNameColor748();
		return 1;
	}
	if (pCreateMob->MobID == g_pObjectManager->m_dwCharID)
		return 1; // Login owns the authoritative self's lifetime.

	auto pHuman = new TMHuman(this);
	if (!pHuman)
		return 1;

	pHuman->m_dwID = pCreateMob->MobID;
	pHuman->m_usGuild = pCreateMob->Guild;
	pHuman->m_citizen = pCreateMob->Server;
	pHuman->m_nCurrentKill = 0;
	pHuman->m_nTotalKill = 0;
	pHuman->m_ucChaosLevel = pCreateMob->MobID < 1000
		? static_cast<unsigned char>(pCreateMob->MobName[12]) : 75;
	pCreateMob->MobName[15] = 0;
	pCreateMob->Nick[25] = 0;
	// Byte 12 carries player CP, not a name character. NPC names retain 15 bytes.
	strncpy_s(pHuman->m_szName, pCreateMob->MobName, pCreateMob->MobID < 1000 ? 12 : 15);
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

int TMFieldScene::OnPacketWeather(MSG_STANDARDPARM* pStd)
{
	// The dispatcher validated the 16-byte frame; ignore unknown values.
	if (!pStd || !IsWeatherValue(pStd->Parm))
		return 1;
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
