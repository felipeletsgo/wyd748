#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "EventTranslator.h"
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"

SScrollBar::SScrollBar(int inCurrent, int inMax, float inX, float inY, float inWidth, float inHeight, unsigned int dwStyle, unsigned int idwBarColor, unsigned int idwColor, int bChat)
	: SControl(inX, inY, inWidth, inHeight)
{
	m_dwStyle = dwStyle;
	m_nCurrent = inCurrent;
	m_nMax = inMax;
	m_nBtnSize = 17.0f;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_SCROLLBAR;

	if (m_dwStyle == 0)
	{
		m_nBarSize = inWidth;
		float nStartXPos = 0;

		if (bChat)
			nStartXPos = -5;
		m_pBar = new SPanel(176, (float)nStartXPos + 1.0f, 0.0f, 12.0f - 2.0f, 17.0f, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pUpPanel = new SPanel(174, (float)nStartXPos, 0.0f, 13.0f, m_nBtnSize, 0xAAAA00FF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pDownPanel = new SPanel(175, (float)nStartXPos, 0.0f, 13.0f, m_nBtnSize, 0xAAAA00FF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_nScrollLength = (int)(float)(m_nHeight - (m_nBtnSize * 0.0));
		m_pBackground1 = new SPanel(510, (float)nStartXPos, 0.0f, 10.0f, inHeight, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pBackground1->GetGeomControl()->eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
	}
	else
	{
		m_pBar = new SPanel(-2, 0.0f, 0.0f, m_nBarSize, m_nBarSize, 0, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pUpPanel = new SPanel(-2, 0.0, 0.0, m_nBtnSize, m_nHeight, 0x44444444, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pDownPanel = new SPanel(-2, m_nWidth - m_nBtnSize, 0.0f, m_nBtnSize, m_nHeight, 0x44444444, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_nScrollLength = (int)(m_nWidth - (m_nBtnSize * 1.0f));

		m_pBackground1 = new SPanel(-2, 0.0f, 0.0f, m_nWidth, inHeight, 0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

		if(m_pBackground1 != nullptr)
			m_pBackground1->GetGeomControl()->nTextureIndex = 0;
	}
	if (m_pBar != nullptr)
	{
		m_pBar->SetControlID(2);
		AddChild(m_pBar);
	}
	if (m_pUpPanel != nullptr)
	{
		m_pUpPanel->SetControlID(0);
		AddChild(m_pUpPanel);
	}
	if (m_pDownPanel != nullptr)
	{
		m_pDownPanel->SetControlID(1);
		AddChild(m_pDownPanel);
	}
	if (m_pBackground1 != nullptr)
	{
		m_pBackground1->SetControlID(3);
		AddChild(m_pBackground1);
	}

	Update();
}

SScrollBar::~SScrollBar()
{
}

void SScrollBar::SetCurrentPos(int inCurrent)
{
	m_nCurrent = inCurrent;
	Update();
}

int SScrollBar::GetCurrentPos()
{
	return m_nCurrent;
}

void SScrollBar::SetMaxValue(int inMax)
{
	m_nMax = inMax;
	Update();
}

int SScrollBar::GetMaxValue()
{
	return m_nMax;
}

void SScrollBar::SetSize(float nWidth, float nHeight)
{
	SControl::SetSize(nWidth, nHeight);
	if (m_dwStyle == 0)
	{
		m_pDownPanel->SetPos(m_pDownPanel->m_nPosX, (float)(m_nHeight - m_nBtnSize) / 1.0f);
		m_nScrollLength = (int)(nHeight - (m_nBtnSize * 2.0f));
		Update();
	}
}

void SScrollBar::Up()
{
	if (m_nCurrent > 5)
	{
		m_nCurrent -= 5;
	}
	else if (m_nCurrent > 0)
	{
		m_nCurrent = 0;
	}

	Update();
}

void SScrollBar::Down()
{
	if (m_nCurrent < m_nMax - 5)
	{
		m_nCurrent += 5;
	}
	else if (m_nCurrent < m_nMax)
	{
		m_nCurrent = m_nMax;
	}

	Update();
}



void SScrollBar::Update()
{
	if (m_nCurrent < 0)
		m_nCurrent = 0;
	if (m_nCurrent > m_nMax)
		m_nCurrent = m_nMax;

	if (m_nMax > 0)
	{
		m_nScrollPos = (int)((((float)m_nScrollLength - m_nBarSize) * (float)m_nCurrent) / (float)m_nMax);
		if (m_dwStyle == 0)
			m_pBar->SetPos(m_pBar->m_nPosX, (float)m_nScrollPos + m_nBtnSize);
		else
			m_pBar->SetPos((float)m_nScrollPos + m_nBtnSize, 0.0f);

		if (m_pEventListener != nullptr)
			m_pEventListener->OnControlEvent(m_dwControlID, m_nCurrent);
	}
}

int SScrollBar::OnControlEvent(DWORD idwControlID, DWORD idwEvent)
{
	return 1;
}

void SScrollBar::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	;
}

int SScrollBar::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	int bInUp = m_pUpPanel->PtInControl((int)((float)nX - m_nPosX), (int)((float)nY - m_nPosY));
	int bInDown = m_pDownPanel->PtInControl((int)((float)nX - m_nPosX), (int)((float)nY - m_nPosY));
	int bInBar = PointInRect(nX, nY, (float)(m_nPosX + m_nWidth) - 5.0f, m_nPosY + m_nBtnSize, 15.0f, m_nHeight - (float)(2.0f * m_nBtnSize));



	if (bInUp && dwFlags == WM_LBUTTONUP)
		Up();
	if (bInDown && dwFlags == WM_LBUTTONUP)
		Down();

	if (bInBar && dwFlags == WM_MOUSEMOVE && wParam & 1)
	{
		if (m_dwStyle != 0)
			m_nScrollPos = static_cast<int>((float)nX - (float)(m_nPosX + m_nBtnSize));
		else
			m_nScrollPos = static_cast<int>((float)nY - (float)(m_nPosY + m_nBtnSize));

		m_nCurrent = (int)((float)(m_nScrollPos * m_nMax) / (float)((float)m_nScrollLength - m_nBarSize));
		if (m_nCurrent > 0 && m_nCurrent < m_nMax)
			Update();
	}

	return 0;
}

void SScrollBar::upbarSetPos(float x, float y)
{
	m_pUpPanel->SetPos(x, y);
}

void SScrollBar::downbarSetPos(float x, float y)
{
	m_pDownPanel->SetPos(x, y);
}

void SScrollBar::upbarSetsize(float x, float y)
{
	m_pUpPanel->SetRealSize(x, y);
}

void SScrollBar::downbarSetsize(float x, float y)
{
	m_pDownPanel->SetRealSize(x, y);
}

void SScrollBar::upbarSetvisible(bool bSet)
{
	;
}

void SScrollBar::downbarSetvisible(bool bSet)
{
	;
}

void SScrollBar::scrollbarSetvisible(bool bSet)
{
	m_pBar->SetVisible(bSet);
}

void SScrollBar::scrollbarbackSetvisible(bool bSet)
{
	m_pBackground1->SetVisible(bSet);
}
