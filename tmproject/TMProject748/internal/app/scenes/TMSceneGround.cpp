#include "pch.h"
// TMScene split by responsibility; the scene lifecycle and dispatch stay in TMScene.cpp.
#include "SControlContainer.h"
#include "SGrid.h"
#include "UIBinary.h"
#include "TMGround.h"
#include "TMSky.h"
#include "TMSun.h"
#include "TMLight.h"
#include "TMCamera.h"
#include "TMObjectContainer.h"
#include "TMItem.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMScene.h"
#include "TMFieldScene.h"
#include "TMEffectFirework.h"
#include "TMSnow.h"
#include "TMRain.h"
#include "WYD748Assets.h"

char heightMapData[128][128]{};
int TMScene::GroundNewAttach(EDirection eDir)
{
	memset(heightMapData, 0, sizeof(heightMapData));
	if (m_bCriticalError == 1 || !m_pGround)
		return 0;

	int x{};
	int y{};

	if (eDir == EDirection::EDIR_LEFT)
	{
		if (m_pGround->m_pLeftGround || m_pGround->m_cLeftEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x - 1;
		y = m_pGround->m_vecOffsetIndex.y;
	}
	else if (eDir == EDirection::EDIR_RIGHT)
	{
		if (m_pGround->m_pRightGround || m_pGround->m_cRightEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x + 1;
		y = m_pGround->m_vecOffsetIndex.y;
	}
	else if (eDir == EDirection::EDIR_UP)
	{
		if (m_pGround->m_pUpGround || m_pGround->m_cUpEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x;
		y = m_pGround->m_vecOffsetIndex.y - 1;
	}
	else if (eDir == EDirection::EDIR_DOWN)
	{
		if (m_pGround->m_pDownGround || m_pGround->m_cDownEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x;
		y = m_pGround->m_vecOffsetIndex.y + 1;
	}

	char fileNameTrn[128]{};
	char fileNameDat[128]{};

	sprintf_s(fileNameTrn, "Env\\Field%02d%02d.trn", x, y);
	sprintf_s(fileNameDat, "Env\\Field%02d%02d.dat", x, y);

	int gId = (m_nCurrentGroundIndex + 1) % 2;

	if (m_pGroundList[gId])
	{
		if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x + 1 &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], m_HeightMapData[i], 128);
		}
		else if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y + 1)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], m_HeightMapData[i], 128);
		}
		else if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x - 1 &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], &m_HeightMapData[i][128], 128);
		}
		else if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y - 1)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], m_HeightMapData[i + 128], 128);
		}
	}
	else
	{
		for (int i = 0; i < 128; ++i)
			memcpy(heightMapData[i], m_HeightMapData[i], 128);
	}

	auto pGround = new TMGround();

	if (!pGround->LoadTileMap(fileNameTrn))
	{
		LOG_WRITELOG("TerrainFile Not Found or Invalid : %s\r\n", fileNameTrn);
		if (!m_bCriticalError)
			LogMsgCriticalError(10, 0, 0, 0, 0);
		m_bCriticalError = 1;
		delete pGround;
		return 0;
	}
	if (pGround->m_vecOffsetIndex.x != x || pGround->m_vecOffsetIndex.y != y)
	{
		LOG_WRITELOG("TerrainFile Position Mismatch : %s\r\n", fileNameTrn);
		if (!m_bCriticalError)
			LogMsgCriticalError(10, 0, 0, 0, 0);
		m_bCriticalError = 1;
		delete pGround;
		return 0;
	}

	if (m_pObjectContainerList[gId])
	{
		delete m_pObjectContainerList[gId];

		m_pObjectContainerList[gId] = nullptr;
	}

	if (m_pGroundList[gId])
	{
		if (m_pGround->m_pLeftGround == m_pGroundList[gId])
			m_pGround->m_pLeftGround = nullptr;
		if (m_pGround->m_pRightGround == m_pGroundList[gId])
			m_pGround->m_pRightGround = nullptr;
		if (m_pGround->m_pUpGround == m_pGroundList[gId])
			m_pGround->m_pUpGround = nullptr;
		if (m_pGround->m_pDownGround == m_pGroundList[gId])
			m_pGround->m_pDownGround = nullptr;
		delete m_pGroundList[gId];

		m_pGroundList[gId] = nullptr;
	}

	m_pGroundList[gId] = pGround;

	m_pGroundObjectContainer->AddChild(m_pGroundList[gId]);

	// Attach must precede object creation: light effects in Field*.dat use
	// GroundGetColor/GroundSetColor through the scene's neighbor links.
	FileTileInfo previousBorder[64]{};
	TMVector3 previousNormals[64]{};
	const int previousMiniMapPos = m_pGround->m_nMiniMapPos;
	const bool changesCurrentBorder =
		eDir == EDirection::EDIR_LEFT || eDir == EDirection::EDIR_UP;
	if (changesCurrentBorder)
	{
		for (int i = 0; i < 64; ++i)
		{
			const int index = eDir == EDirection::EDIR_LEFT ? i * 64 : i;
			previousBorder[i] = m_pGround->m_TileMapData[index];
			previousNormals[i] = m_pGround->m_TileNormalVector[index];
		}
	}

	if (!m_pGroundList[m_nCurrentGroundIndex] ||
		!m_pGroundList[m_nCurrentGroundIndex]->Attach(m_pGroundList[gId]))
	{
		SAFE_DELETE(m_pGroundList[gId]);
		LOG_WRITELOG("TerrainFile Attach Failed : %s\r\n", fileNameTrn);
		if (!m_bCriticalError)
			LogMsgCriticalError(10, 0, 0, 0, 0);
		m_bCriticalError = 1;
		return 0;
	}

	m_pObjectContainerList[gId] = new TMObjectContainer(m_pGroundList[gId]);

	if (!m_pObjectContainerList[gId]->Load(fileNameDat))
	{
		SAFE_DELETE(m_pObjectContainerList[gId]);
		if (m_pGround->m_pLeftGround == m_pGroundList[gId])
			m_pGround->m_pLeftGround = nullptr;
		if (m_pGround->m_pRightGround == m_pGroundList[gId])
			m_pGround->m_pRightGround = nullptr;
		if (m_pGround->m_pUpGround == m_pGroundList[gId])
			m_pGround->m_pUpGround = nullptr;
		if (m_pGround->m_pDownGround == m_pGroundList[gId])
			m_pGround->m_pDownGround = nullptr;
		if (changesCurrentBorder)
		{
			for (int i = 0; i < 64; ++i)
			{
				const int index = eDir == EDirection::EDIR_LEFT ? i * 64 : i;
				m_pGround->m_TileMapData[index] = previousBorder[i];
				m_pGround->m_TileNormalVector[index] = previousNormals[i];
			}
		}
		m_pGround->m_nMiniMapPos = previousMiniMapPos;

		SAFE_DELETE(m_pGroundList[gId]);

		LOG_WRITELOG("DataFile Not Found : %s\r\n", fileNameDat);

		if (!m_bCriticalError)
			LogMsgCriticalError(12, 0, 0, 0, 0);

		m_bCriticalError = 1;
		return 0;
	}

	if (m_pObjectContainerList[m_nCurrentGroundIndex])
	{
		if (eDir == EDirection::EDIR_DOWN)
			m_pObjectContainerList[m_nCurrentGroundIndex]->SetPrevNode(m_pObjectContainerList[gId]);
		else
			m_pObjectContainerList[m_nCurrentGroundIndex]->SetNextNode(m_pObjectContainerList[gId]);
	}

	g_pTextureManager->ReleaseNotUsingTexture();

	memset(m_HeightMapData, 0, sizeof(m_HeightMapData));

	switch (eDir)
	{
	case EDirection::EDIR_LEFT:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(m_HeightMapData[i], m_pGround->m_pLeftGround->m_pMaskData[i], 128);
			memcpy(&m_HeightMapData[i][128], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_pLeftGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_pLeftGround->m_vecOffset.y;
		break;
	case EDirection::EDIR_RIGHT:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(&m_HeightMapData[i][128], m_pGround->m_pRightGround->m_pMaskData[i], 128);
			memcpy(m_HeightMapData[i], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_vecOffset.y;
		break;
	case EDirection::EDIR_UP:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(&m_HeightMapData[i], m_pGround->m_pUpGround->m_pMaskData[i], 128);
			memcpy(&m_HeightMapData[i + 128], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_pUpGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_pUpGround->m_vecOffset.y;
		break;
	case EDirection::EDIR_DOWN:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(&m_HeightMapData[i + 128], m_pGround->m_pDownGround->m_pMaskData[i], 128);
			memcpy(&m_HeightMapData[i], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_vecOffset.y;
		break;
	}

	BASE_ApplyAttribute((char*)m_HeightMapData, 256);

	memcpy(m_GateMapData, m_HeightMapData, sizeof(m_HeightMapData));

	SaveHeightMap(fileNameTrn);

	m_pGround->SetMiniMapData();

	m_nAdjustTime = 0;
	m_dwInitTime = g_pTimerManager->GetServerTime();

	return 1;
}

D3DXVECTOR3 TMScene::GroundGetPickPos()
{
	D3DXVECTOR3 vPickPos{ 0.0f, -10000.0f, 0.0f };
	D3DXVECTOR3 vPickTempPos{ 0.0f, -10000.0f, 0.0f };
	D3DXVECTOR3 vFocusePos{ 0.0f, -9000.0f, 0.0f };

	auto pFocusedObject = static_cast<TMObject*>(m_pMyHuman);

	if (pFocusedObject)
	{
		vFocusePos.x = pFocusedObject->m_vecPosition.x;
		vFocusePos.y = pFocusedObject->m_fHeight;
		vFocusePos.z = pFocusedObject->m_vecPosition.x;
	}

	if (!m_pGround)
		return vPickPos;

	vPickTempPos = m_pGround->GetPickPos();

	if (fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
		vPickPos = vPickTempPos;

	if (vPickPos.y >= -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f)
		return vPickPos;

	if (m_pGround->m_pLeftGround)
	{
		vPickTempPos = m_pGround->m_pLeftGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y) || fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f)
				return vPickPos;
		}
	}

	if (m_pGround->m_pRightGround)
	{
		vPickTempPos = m_pGround->m_pRightGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y))
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f || fabsf(vFocusePos.y - vPickTempPos.y < 4.0f))
				return vPickPos;
		}
	}

	if (m_pGround->m_pUpGround)
	{
		vPickTempPos = m_pGround->m_pUpGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y))
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f || fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
				return vPickPos;
		}
	}

	if (m_pGround->m_pDownGround)
	{
		vPickTempPos = m_pGround->m_pDownGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y))
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0 && fabsf(vFocusePos.y - vPickPos.y) < 2.0f || fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
				return vPickPos;

		}
	}

	return vPickPos;
}

int TMScene::GroundGetTileType(TMVector2 vecPosition)
{
	int nTileType{};

	if (m_pGround)
	{
		if (vecPosition.x >= m_pGround->m_vecOffset.x &&
			vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) &&
			vecPosition.y >= m_pGround->m_vecOffset.y &&
			vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pLeftGround
			&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pLeftGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pRightGround
			&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pRightGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pUpGround
			&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pUpGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pDownGround
			&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pDownGround->GetTileType(vecPosition);
		}
	}

	return nTileType;
}

int TMScene::GroundGetMask(TMVector2 vecPosition)
{
	int nXIndex = (int)vecPosition.x - g_HeightPosX;
	int nYIndex = (int)vecPosition.y - g_HeightPosY;

	if (nXIndex < 0)
		nXIndex = 0;

	if (nYIndex < 0)
		nYIndex = 0;

	if (nXIndex >= 256)
		nXIndex = 255;

	if (nYIndex >= 256)
		nYIndex = 255;

	int value = m_HeightMapData[nYIndex][nXIndex];
	return m_HeightMapData[nYIndex][nXIndex];
}

int TMScene::GroundGetMask(IVector2 vecPosition)
{
	int nXIndex = vecPosition.x - g_HeightPosX;
	int nYIndex = vecPosition.y - g_HeightPosY;

	if (nXIndex < 0)
		nXIndex = 0;

	if (nYIndex < 0)
		nYIndex = 0;

	if (nXIndex >= 256)
		nXIndex = 255;

	if (nYIndex >= 256)
		nYIndex = 255;

	return m_HeightMapData[nYIndex][nXIndex];
}

float TMScene::GroundGetHeight(TMVector2 vecPosition)
{
	float fHeight{ -10000.0f };

	if (m_pGround != nullptr)
	{
		if (vecPosition.x >= m_pGround->m_vecOffset.x &&
			vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) &&
			vecPosition.y >= m_pGround->m_vecOffset.y &&
			vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pLeftGround
			&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pLeftGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pRightGround
			&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pRightGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pUpGround
			&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pUpGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pDownGround
			&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pDownGround->GetHeight(vecPosition);
		}
	}

	return fHeight;
}

D3DCOLORVALUE TMScene::GroundGetColor(TMVector2 vecPosition)
{
	D3DCOLORVALUE color{ 1.0f, 1.0f, 1.0f, 1.0f };

	if (m_pGround != nullptr)
	{
		if (vecPosition.x >= m_pGround->m_vecOffset.x &&
			vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) &&
			vecPosition.y >= m_pGround->m_vecOffset.y &&
			vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pLeftGround
			&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pLeftGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pRightGround
			&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pRightGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pUpGround
			&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pUpGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pDownGround
			&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pDownGround->GetColor(vecPosition);
		}
	}

	return color;
}

void TMScene::GroundSetColor(TMVector2 vecPosition, unsigned int dwColor)
{
	if (!m_pGround)
		return;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) &&
		vecPosition.y >= m_pGround->m_vecOffset.y && vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pLeftGround
		&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pLeftGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pRightGround
		&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pRightGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pUpGround
		&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pUpGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pDownGround
		&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pDownGround->SetColor(vecPosition, dwColor);
	}
}

int TMScene::GroundIsInWater(TMVector2 vecPosition, float fHeight, float* pfWaterHeight)
{
	if (!m_pGround)
		return 0;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && (float)(m_pGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_vecOffset.y && (float)(m_pGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pLeftGround &&
		vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x && (float)(m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y && (float)(m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pLeftGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pRightGround
		&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x && (float)(m_pGround->m_pRightGround->m_vecOffset.x + 128.0f) > vecPosition.x
		&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y && (float)(m_pGround->m_pRightGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pRightGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pUpGround
		&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x && (float)(m_pGround->m_pUpGround->m_vecOffset.x + 128.0f) > vecPosition.x
		&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y && (float)(m_pGround->m_pUpGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pUpGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pDownGround
		&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x && (float)(m_pGround->m_pDownGround->m_vecOffset.x + 128.0f) > vecPosition.x
		&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y && (float)(m_pGround->m_pDownGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pDownGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}

	return 0;
}

int TMScene::GroundIsInWater2(TMVector2 vecPosition, float* pfWaterHeight)
{
	if (!m_pGround)
		return 0;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && (float)(m_pGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_vecOffset.y && (float)(m_pGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (m_pGround->m_pLeftGround &&
		vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x && (float)(m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y && (float)(m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pLeftGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (m_pGround->m_pRightGround &&
		vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x && (float)(m_pGround->m_pRightGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y && (float)(m_pGround->m_pRightGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pRightGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (m_pGround->m_pUpGround &&
		vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x && (float)(m_pGround->m_pUpGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y && (float)(m_pGround->m_pUpGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pUpGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (!m_pGround->m_pDownGround ||
		vecPosition.x < m_pGround->m_pDownGround->m_vecOffset.x || (float)(m_pGround->m_pDownGround->m_vecOffset.x + 128.0f) <= vecPosition.x ||
		vecPosition.y < m_pGround->m_pDownGround->m_vecOffset.y || (float)(m_pGround->m_pDownGround->m_vecOffset.y + 128.0f) <= vecPosition.y)
	{
		return 0;
	}

	return m_pGround->m_pDownGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
}

float TMScene::GroundGetWaterHeight(TMVector2 vecPosition, float* pfWaterHeight)
{
	if (!m_pGround)
		return -100.0f;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && (float)(m_pGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_vecOffset.y && (float)(m_pGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pLeftGround &&
		vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x && (float)(m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y && (float)(m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pLeftGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pRightGround &&
		vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x && (float)(m_pGround->m_pRightGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y && (float)(m_pGround->m_pRightGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pRightGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pUpGround &&
		vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x && (float)(m_pGround->m_pUpGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y && (float)(m_pGround->m_pUpGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pUpGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pDownGround &&
		vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x && (float)(m_pGround->m_pDownGround->m_vecOffset.x + 128.0f) > vecPosition.x &&
		vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y && (float)(m_pGround->m_pDownGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
	    return m_pGround->m_pDownGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}

	return -100.0f;
}

int TMScene::GetMask2(TMVector2 vecPosition)
{
	int nMaskX = (int)(vecPosition.x - (float)g_HeightPosX);
	int nMaskY = (int)(vecPosition.y - (float)g_HeightPosY);

	if (nMaskX >= 0 && nMaskY >= 0 && nMaskX < 256 && nMaskY < 256)
		return m_GateMapData[nMaskY][nMaskX];

	return -10000;
}

void TMScene::Warp()
{
	if (m_bCriticalError == 1)
		return;

	auto pFocusedObject = static_cast<TMObject*>(m_pMyHuman);

	if (pFocusedObject)
	{
		if (m_pGround)
			Warp2((int)(pFocusedObject->m_vecPosition.x / 128.0f), (int)(pFocusedObject->m_vecPosition.y / 128.0f));
	}
}

void TMScene::Warp2(int nZoneX, int nZoneY)
{
	if (m_bCriticalError == 1 || !m_pGround)
		return;

	m_bAutoRun = 0;
	if (m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && static_cast<TMFieldScene*>(this)->m_pAutoRunBtn)
		static_cast<TMFieldScene*>(this)->m_pAutoRunBtn->SetSelected(m_bAutoRun);

	TMGround* pNeighbor = nullptr;
	if (m_pGround->m_pLeftGround)
		pNeighbor = m_pGround->m_pLeftGround;
	if (m_pGround->m_pRightGround)
		pNeighbor = m_pGround->m_pRightGround;
	if (m_pGround->m_pUpGround)
		pNeighbor = m_pGround->m_pUpGround;
	if (m_pGround->m_pDownGround)
		pNeighbor = m_pGround->m_pDownGround;

	if (!pNeighbor &&
		(nZoneX != m_pGround->m_vecOffsetIndex.x || nZoneY != m_pGround->m_vecOffsetIndex.y) ||
		pNeighbor &&
		(nZoneX != m_pGround->m_vecOffsetIndex.x && nZoneX != pNeighbor->m_vecOffsetIndex.x ||
		 nZoneY != m_pGround->m_vecOffsetIndex.y && nZoneY != pNeighbor->m_vecOffsetIndex.y))
	{
		char szMapPath[128]{};
		char szDataPath[128]{};
		sprintf(szMapPath, "env\\Field%02d%02d.trn", nZoneX, nZoneY);
		sprintf(szDataPath, "env\\Field%02d%02d.dat", nZoneX, nZoneY);

		TMGround* pGround = new TMGround();

		if (!pGround->LoadTileMap(szMapPath))
		{
			if (!m_bCriticalError)
				LogMsgCriticalError(10, 0, 0, 0, 0);

			m_bCriticalError = 1;
			delete pGround;
			return;
		}
		if (pGround->m_vecOffsetIndex.x != nZoneX || pGround->m_vecOffsetIndex.y != nZoneY)
		{
			LOG_WRITELOG("TerrainFile Position Mismatch : %s\r\n", szMapPath);
			if (!m_bCriticalError)
				LogMsgCriticalError(10, 0, 0, 0, 0);
			m_bCriticalError = 1;
			delete pGround;
			return;
		}

		TMGround* pOldGround = m_pGround;
		m_pGround = pGround;

		for (int i = 0; i < 2; ++i)
		{
			SAFE_DELETE(m_pObjectContainerList[i]);
			SAFE_DELETE(m_pGroundList[i]);
		}

		g_HeightPosX = nZoneX << 7;
		g_HeightPosY = nZoneY << 7;

		m_nCurrentGroundIndex = 0;
		m_pGroundList[0] = m_pGround;

		m_pObjectContainerList[0] = new TMObjectContainer(m_pGround);

		if (!m_pObjectContainerList[0]->Load(szDataPath))
		{
			LOG_WRITELOG("DataFile Not Found : %s\r\n", szDataPath);
			if (!m_bCriticalError)
				LogMsgCriticalError(11, 0, 0, 0, 0);

			m_bCriticalError = 1;
			SAFE_DELETE(m_pObjectContainerList[0]);
			SAFE_DELETE(m_pGroundList[0]);
			m_pGround = nullptr;
			return;
		}

		memset(m_HeightMapData, 0, sizeof(m_HeightMapData));

		for (int nY = 0; nY < 128; ++nY)
			memcpy(m_HeightMapData[nY], m_pGround->m_pMaskData[nY], 128);

		m_pGround->SetMiniMapData();
		m_pGroundObjectContainer->AddChild(m_pObjectContainerList[0]);
		m_pGroundObjectContainer->AddChild(m_pGroundList[0]);

		BASE_ApplyAttribute((char*)m_HeightMapData, 256);

		memcpy(m_GateMapData, m_HeightMapData, sizeof(m_GateMapData));

		SaveHeightMap(szMapPath);
		m_nAdjustTime = 0;
		m_dwInitTime = g_pTimerManager->GetServerTime();
	}
	else if (pNeighbor && nZoneX == pNeighbor->m_vecOffsetIndex.x && nZoneY == pNeighbor->m_vecOffsetIndex.y)
	{
		m_pGround = pNeighbor;
		m_nCurrentGroundIndex = (m_nCurrentGroundIndex + 1) % 2;
		m_pGround->SetMiniMapData();
	}
	if (m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
	{
		auto pFieldScene = static_cast<TMFieldScene*>(this);
		auto pSoundManager = g_pSoundManager;
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(6);
			if (pSoundData && pSoundData->IsSoundPlaying())
			{
				pSoundData->Stop();
			}
		}
		if (RenderDevice::m_bDungeon && RenderDevice::m_bDungeon != 3 && RenderDevice::m_bDungeon != 4)
		{
			g_nWeather = 0;
			if (pFieldScene->m_pRain)
				pFieldScene->m_pRain->m_bVisible = 0;
			if (pFieldScene->m_pSnow)
				pFieldScene->m_pSnow->m_bVisible = 0;
			if (pFieldScene->m_pSnow2)
				pFieldScene->m_pSnow2->m_bVisible = 0;
			pFieldScene->m_bQuater = 1;
			pFieldScene->UpdateScoreUI(0);

			// WYD-Go 7.48 compatibility: terrain warps are gameplay state, while the
			// 7.59 minimap panel is optional and absent from the compact 7.48 UI.
			if (pFieldScene->m_pMiniMapPanel)
				pFieldScene->m_pMiniMapPanel->m_GCPanel.dwColor = 0x80FFFFFF;
		}
		else
		{
			if (RenderDevice::m_bDungeon == 3 || RenderDevice::m_bDungeon == 4)
			{
				if (pFieldScene->m_pSnow)
					pFieldScene->m_pSnow->m_bVisible = 0;
				if (pFieldScene->m_pSnow2)
					pFieldScene->m_pSnow2->m_bVisible = 0;
			}

			pFieldScene->SetWeather(g_nWeather);
			if (g_pObjectManager && g_pObjectManager->m_pCamera)
				g_pObjectManager->m_pCamera->m_fHorizonAngle = 0.78539819f;

			// WYD-Go 7.48 compatibility: do not make a successful world warp depend
			// on a presentation-only control that does not exist in Interface 7.48.
			if (pFieldScene->m_pMiniMapPanel)
				pFieldScene->m_pMiniMapPanel->m_GCPanel.dwColor = 0x80FFFFFF;
		}
	}
}

void TMScene::SaveHeightMap(char* szFileName)
{
	;
}
