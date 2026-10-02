#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"

namespace
{
constexpr int kReelTextureOffset[3][22] = {
	{1, 6, 2, 3, 1, 4, 1, 8, 2, 5, 1, 9, 1, 6, 1, 4, 3, 0, 2, 5, 1, 7},
	{0, 1, 3, 5, 0, 3, 2, 4, 0, 1, 3, 6, 0, 3, 9, 2, 0, 3, 7, 3, 0, 8},
	{3, 2, 0, 4, 2, 6, 1, 0, 2, 0, 1, 8, 2, 3, 0, 5, 2, 0, 7, 0, 2, 9},
};
}

SReelPanel::SReelPanel(unsigned int inTextureSetIndex, float inX, float inY, float inSizeX, float inSizeY, float inPitch)
	: SPanel(-2, inX, inY, 186.0f, 180.0f, 0x1010101, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	memset(m_pGamBleReel, 0, sizeof(m_pGamBleReel));
	memset(m_pGamBleBox, 0, sizeof(m_pGamBleBox));
	memset(m_pJACKPOTCOUNT, 0, sizeof(m_pJACKPOTCOUNT));
	memset(m_bGamBleBox, 0, sizeof(m_bGamBleBox));
	memset(m_RollPos, 0, sizeof(m_RollPos));
	memset(m_StopPos, 0, sizeof(m_StopPos));
	m_pJACKPOTBG = nullptr;
	m_bRoling = false;
	m_dwOldServerTime = g_pTimerManager ? g_pTimerManager->GetServerTime() : 0;
	m_dwStopTime = 0;
	m_bResult = 0;
	m_dwJackpot = 0;
	m_dwJackPotView = 0;
	m_dwAutoTime = 0;
	m_dwBoxAniTime = 0;
	m_nPresent = 0;
	m_vSize.x = inSizeX;
	m_vSize.y = inSizeY;
	m_nPitch = static_cast<int>(inPitch);
	m_dwBatCoin = 0;

	for (int reel = 0; reel < 3; ++reel)
	{
		for (int symbol = 0; symbol < 22; ++symbol)
		{
			m_pGamBleReel[reel][symbol] = new SPanel(
				static_cast<int>(inTextureSetIndex) + kReelTextureOffset[reel][symbol],
				(inSizeX + inPitch) * reel,
				inSizeY * symbol,
				inSizeX,
				inSizeY,
				0xFFFFFFFF,
				RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
			if (m_pGamBleReel[reel][symbol])
				AddChild(m_pGamBleReel[reel][symbol]);
		}
	}

	for (int row = 0; row < 3; ++row)
	{
		for (int column = 0; column < 3; ++column)
		{
			const int index = row * 3 + column;
			const float x = (inSizeX + inPitch) * column;
			const float y = inSizeY * row;
			m_pGamBleBox[0][index] = new SPanel(350, x, y, inSizeX, inSizeY, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
			m_pGamBleBox[1][index] = new SPanel(351, x, y, inSizeX, inSizeY, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
			if (m_pGamBleBox[0][index])
			{
				m_pGamBleBox[0][index]->SetVisible(0);
				AddChild(m_pGamBleBox[0][index]);
			}
			if (m_pGamBleBox[1][index])
			{
				m_pGamBleBox[1][index]->SetVisible(0);
				AddChild(m_pGamBleBox[1][index]);
			}
		}
	}

	for (int digit = 0; digit < 10; ++digit)
	{
		m_pJACKPOTCOUNT[digit] = new SPanel(355, 41.0f + 15.0f * digit, -49.0f, 14.0f, 25.0f, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		if (m_pJACKPOTCOUNT[digit])
			AddChild(m_pJACKPOTCOUNT[digit]);
	}

	m_pJACKPOTBG = new SPanel(356, 30.0f, -49.0f, 160.0f, 27.0f, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
	if (m_pJACKPOTBG)
		AddChild(m_pJACKPOTBG);

	SetVisible(1);
	UpDateJackpot();
}

SReelPanel::~SReelPanel()
{
}

void SReelPanel::SetRoll(bool bRoll, int StopPos1, int StopPos2, int StopPos3, unsigned int StopTime)
{
	if (!m_bVisible)
		return;

	m_bRoling = bRoll;
	m_StopPos[0] = StopPos1;
	m_StopPos[1] = StopPos2;
	m_StopPos[2] = StopPos3;
	m_dwStopTime = StopTime;
	m_dwOldServerTime = g_pTimerManager ? g_pTimerManager->GetServerTime() : 0;

	if (bRoll)
	{
		for (int index = 0; index < 9; ++index)
		{
			m_bGamBleBox[index] = false;
			if (m_pGamBleBox[0][index])
				m_pGamBleBox[0][index]->SetVisible(0);
			if (m_pGamBleBox[1][index])
				m_pGamBleBox[1][index]->SetVisible(0);
		}
		m_bResult = 0;
		m_dwJackpot = 0;
		m_nPresent = 0;
	}
}

void SReelPanel::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);
	if (!g_pTimerManager)
		return;

	const unsigned int serverTime = g_pTimerManager->GetServerTime();
	const unsigned int elapsed = serverTime - m_dwOldServerTime;
	const int reelHeight = static_cast<int>(m_vSize.y);
	const int totalHeight = reelHeight * 22;

	if ((m_bRoling || m_dwStopTime) && reelHeight > 0)
	{
		if (elapsed >= m_dwStopTime)
			m_dwStopTime = 0;
		else
			m_dwStopTime -= elapsed;

		for (int reel = 0; reel < 3; ++reel)
		{
			const unsigned int stopThreshold = reel == 0 ? 1500u : (reel == 1 ? 900u : 300u);
			const int finalPosition = (23 - m_StopPos[reel]) * reelHeight;
			if (!m_bRoling && m_dwStopTime < stopThreshold)
				m_RollPos[reel] = finalPosition;
			else
				m_RollPos[reel] = (m_RollPos[reel] + static_cast<int>(elapsed)) % totalHeight;

			for (int symbol = 0; symbol < 22; ++symbol)
			{
				SPanel* panel = m_pGamBleReel[reel][symbol];
				if (!panel)
					continue;
				const int y = (m_RollPos[reel] + reelHeight * symbol) % totalHeight - reelHeight;
				panel->SetPos((m_vSize.x + static_cast<float>(m_nPitch)) * reel, static_cast<float>(y));
				panel->SetVisible(y <= reelHeight * 3);
			}
		}

		if (!m_bRoling && m_dwStopTime == 0)
		{
			for (int index = 0; index < 9; ++index)
				if (m_pGamBleBox[0][index])
					m_pGamBleBox[0][index]->SetVisible(m_bGamBleBox[index]);
		}
	}

	if (m_dwJackpot == 0 || m_dwJackPotView >= m_dwJackpot)
	{
		m_dwJackPotView = m_dwJackpot;
	}
	else
	{
		const unsigned int difference = m_dwJackpot - m_dwJackPotView;
		const unsigned int stepBase = difference / 500;
		const unsigned long long step = 1ull + static_cast<unsigned long long>(stepBase) * stepBase;
		m_dwJackPotView = step >= difference ? m_dwJackpot : m_dwJackPotView + static_cast<unsigned int>(step);
	}
	UpDateJackpot();
	m_dwOldServerTime = serverTime;
}

void SReelPanel::SetVisible(int bVisible)
{
	SPanel::SetVisible(bVisible);
	const int reelHeight = static_cast<int>(m_vSize.y);
	for (int reel = 0; reel < 3; ++reel)
	{
		for (int symbol = 0; symbol < 22; ++symbol)
		{
			SPanel* panel = m_pGamBleReel[reel][symbol];
			if (!panel)
				continue;
			panel->SetPos((m_vSize.x + static_cast<float>(m_nPitch)) * reel, static_cast<float>((symbol - 1) * reelHeight));
			panel->SetVisible(bVisible && symbol < 4);
		}
	}
	if (!bVisible)
	{
		for (int index = 0; index < 9; ++index)
		{
			if (m_pGamBleBox[0][index])
				m_pGamBleBox[0][index]->SetVisible(0);
			if (m_pGamBleBox[1][index])
				m_pGamBleBox[1][index]->SetVisible(0);
		}
	}
}

void SReelPanel::SetResult(char cResult)
{
	if (cResult < 0 || cResult > 4)
		return;
	m_bResult = 1;
	if (cResult < 3)
	{
		for (int column = 0; column < 3; ++column)
			m_bGamBleBox[cResult * 3 + column] = true;
	}
	else if (cResult == 3)
	{
		m_bGamBleBox[0] = true;
		m_bGamBleBox[4] = true;
		m_bGamBleBox[8] = true;
	}
	else
	{
		m_bGamBleBox[2] = true;
		m_bGamBleBox[4] = true;
		m_bGamBleBox[6] = true;
	}
}

void SReelPanel::UpDateJackpot()
{
	static constexpr unsigned int divisor[10] = {
		1000000000u, 100000000u, 10000000u, 1000000u, 100000u,
		10000u, 1000u, 100u, 10u, 1u,
	};
	for (int digit = 0; digit < 10; ++digit)
	{
		if (!m_pJACKPOTCOUNT[digit])
			continue;
		const bool leadingZero = digit < 9 && m_dwJackPotView < divisor[digit];
		m_pJACKPOTCOUNT[digit]->m_GCPanel.nTextureIndex = leadingZero ? 10 : (m_dwJackPotView / divisor[digit]) % 10;
	}
}
