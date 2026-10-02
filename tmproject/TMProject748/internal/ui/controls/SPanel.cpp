#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"

SPanel::SPanel(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int idwColor, RENDERCTRLTYPE eRenderType)
	: SControl(inX, inY, inWidth, inHeight)
{
	m_GCPanel = GeomControl(eRenderType, inTextureSetIndex, 0.0f, 0.0f, inWidth, inHeight, 0, idwColor);
	m_pDescPanel = 0;
	m_bPickable = 0;
	m_bPicked = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_PANEL;
}

SPanel::~SPanel()
{
	SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;

	if (pControlContainer != nullptr && pControlContainer->m_pPickedControl == this)
		pControlContainer->m_pPickedControl = nullptr;

	if (pControlContainer != nullptr && m_GCPanel.nLayer >= 0)
		RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCPanel, m_GCPanel.nLayer);
}

int SPanel::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	if (m_dwControlID == 65943 || m_dwControlID == 65947)
		return 0;



	int bInCaption = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, 24.0f);
	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);
	m_cOver = m_bOver;
	if (m_bOver == 0 && m_pDescPanel != nullptr)
		m_pDescPanel->SetVisible(0);

	switch (dwFlags)
	{
	case WM_MOUSEMOVE:
	{
		if (m_bPicked != 0 && m_bPickable != 0)
		{
			m_nPosX = (float)(nX - m_nPickPosX) + m_nPosX;
			m_nPosY = (float)(nY - m_nPickPosY) + m_nPosY;
			if (m_nPosX < 0.0)
				m_nPosX = 0.0;
			if (m_nPosY < 0.0)
				m_nPosY = 0.0;

			if ((float)(m_nPosX + m_nWidth) > (float)g_pApp->m_dwScreenWidth)
				m_nPosX = (float)g_pApp->m_dwScreenWidth - m_nWidth;
			if ((float)(m_nPosY + m_nHeight) > (float)g_pApp->m_dwScreenHeight)
				m_nPosY = (float)g_pApp->m_dwScreenHeight - m_nHeight;

			m_nPickPosX = nX;
			m_nPickPosY = nY;
		}
		if (m_bOver == 1 && (wParam & 1))
			return 1;

		if (m_bOver == 1 && m_pDescPanel != nullptr)
			m_pDescPanel->SetVisible(1);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (m_bPickable != 0 && bInCaption != 0 && g_pCurrentScene->m_pControlContainer->m_pPickedControl == nullptr)
		{
			g_pCurrentScene->m_pControlContainer->m_pPickedControl = this;
			m_bPicked = 1;
			m_nPickPosX = nX;
			m_nPickPosY = nY;
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if (g_pCurrentScene->m_pControlContainer->m_pPickedControl == this)
			g_pCurrentScene->m_pControlContainer->m_pPickedControl = nullptr;

		if (m_bPickable != 0)
			m_bPicked = 0;
	}
	break;
	case WM_LBUTTONDBLCLK:
	{
		if (m_bModal == 1 && m_bOver == 1)
			return 1;

		return SControl::OnMouseEvent(dwFlags, wParam, nX, nY);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (m_bOver == 1)
			return 1;
	}
	break;
	}

	if (m_bOver == 1)
		return 1;

	return m_bModal == 1 && m_bOver == 1;
}

void SPanel::SetTextureSetIndex(int inTextureSetIndex)
{
	m_GCPanel.nTextureSetIndex = inTextureSetIndex;
}

GeomControl* SPanel::GetGeomControl()
{
	return &m_GCPanel;
}

void SPanel::SetVisible(int bVisible)
{
	m_bVisible = bVisible;

	if (m_bVisible == 0 && g_pCurrentScene != nullptr && g_pCurrentScene->m_pControlContainer != nullptr)
	{
		if (g_pCurrentScene->m_pControlContainer->m_pPickedControl == this)
		{
			// Native 7.48 FUN_004015dd releases only the hidden panel captured by
			// the mouse.  Destroying the scene container made every later X/ESC
			// lookup fail after a panel was closed once.
			g_pCurrentScene->m_pControlContainer->m_pPickedControl = nullptr;
		}
	}
}

void SPanel::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	if (m_GCPanel.nTextureSetIndex >= 0 || (m_GCPanel.dwColor & 0xFF000000))
	{
		m_GCPanel.nPosX = ivParentPos.x + m_nPosX;
		m_GCPanel.nPosY = ivParentPos.y + m_nPosY;
		m_GCPanel.nWidth = m_nWidth;
		m_GCPanel.nHeight = m_nHeight;
		m_GCPanel.nLayer = inParentLayer;
		if ((float)(m_GCPanel.nPosX + m_GCPanel.nWidth) >= 0.0f &&
			(float)(m_GCPanel.nPosY + m_GCPanel.nHeight) >= 0.0f &&
			m_GCPanel.nPosX <= WYD748_UI_BASE_WIDTH * RenderDevice::m_fWidthRatio &&
			m_GCPanel.nPosY <= WYD748_UI_BASE_HEIGHT * RenderDevice::m_fHeightRatio)
		{
			AddRenderControlItem(pDrawList, &m_GCPanel, inParentLayer);
		}
	}
}
