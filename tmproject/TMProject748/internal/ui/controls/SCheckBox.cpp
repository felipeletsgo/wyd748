#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"

SCheckBox::SCheckBox(unsigned int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int dwColor)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, dwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_bValue = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_CHECKBOX;
	m_dwSelectedColor = -16777216;
	m_dwUnSelectedColor = dwColor;
}

SCheckBox::~SCheckBox()
{
}

void SCheckBox::SetValue(int ibValue)
{
	m_bValue = ibValue;
	Update();
}

int SCheckBox::GetValue()
{
	return m_bValue;
}

int SCheckBox::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (!m_bSelectEnable)
		return 0;

	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);

	if (dwFlags == 512)
		return 0;

	if (dwFlags != 513)
		return SPanel::OnMouseEvent(dwFlags, wParam, nX, nY);

	m_bValue = m_bValue == 0;
	m_bFocused = 1;
	Update();

	if (m_pEventListener)
		m_pEventListener->OnControlEvent(m_dwControlID, 0);

	return 1;
}

void SCheckBox::Update()
{
	if (m_bValue == 1)
	{
		m_GCPanel.nTextureIndex = 1;
		m_GCPanel.dwColor = m_dwSelectedColor;
	}
	else
	{
		m_GCPanel.nTextureIndex = 0;
		m_GCPanel.dwColor = m_dwUnSelectedColor;
	}
}
