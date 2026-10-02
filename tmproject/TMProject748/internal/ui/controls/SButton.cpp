#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMFieldScene.h"

SButton::SButton(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int idwColor, int bSound, char* istrText)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, idwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_bMouseOver = 0;
	m_bSelected = 0;
	m_cAlwaysAlt = 0;
	m_cBlink = 0;
	m_pAltText = 0;
	m_dwColor = idwColor;
	m_bSound = bSound;
	m_dwOldTime = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_BUTTON;

	if (istrText != nullptr)
	{
		BASE_UnderBarToSpace(istrText);
		if (!strcmp(istrText, " "))
			istrText = nullptr;
	}

	if (istrText != nullptr)
	{
		if (inTextureSetIndex >= -2 && inTextureSetIndex < 0)
		{
			m_GCPanel.pFont = nullptr;
			m_GCPanel.pFont = new TMFont2();

			sprintf(m_GCPanel.strString, "%s", istrText);

			if (m_GCPanel.pFont != nullptr)
				m_GCPanel.pFont->SetText(m_GCPanel.strString, 0xFFFFFFFF, 0);
		}

		m_pAltText = new SText(-1, istrText, 0xFFFFFFFF, inWidth - 10.0f, 0.0f, (float)strlen(istrText) * 8.0f, (float)RenderDevice::m_nFontSize,
			0, 0xFF333333, 1u, 0);
		m_pAltText->SetVisible(0);
		m_pAltText->SetAlwaysOnTop(1);

		if (inTextureSetIndex == -1 || inTextureSetIndex == -2)
		{
			m_nPosX = m_nPosX
				- (float)((float)(RenderDevice::m_nFontSize - 12) * (float)((float)strlen(istrText) * 0.3f));
			m_nPosY = m_nPosY - (float)((float)(RenderDevice::m_nFontSize - 12) * 0.6f);
		}
	}

	m_GrayType = 0;
	Update();
}

SButton::~SButton()
{
	SAFE_DELETE(m_GCPanel.pFont);
	SAFE_DELETE(m_pAltText);
}

void SButton::SetText(char* istrText)
{
	if (m_pAltText != nullptr)
		m_pAltText->SetText(istrText, 0);

	sprintf(m_GCPanel.strString, "%s", istrText);
	m_GCPanel.pFont->SetText(m_GCPanel.strString, 0xFFFFFFFF, 0);
}

int SButton::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);
	if (m_pAltText != nullptr)
	{
		if (m_bOver == 1)
		{
			if (!m_pAltText->IsVisible())
				m_pAltText->SetVisible(1);
			if (m_GCPanel.nTextureSetIndex > -3 && m_GCPanel.nTextureSetIndex < 3)
			{
				m_GCPanel.eRenderType = RENDERCTRLTYPE::RENDER_TEXT_FOCUS;
				if (m_GCPanel.pFont != nullptr)
					m_GCPanel.pFont->m_dwShadeColor = 0x66FFFFFF;
			}
		}
		else
		{
			m_pAltText->SetVisible(0);
			m_GCPanel.eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
			if (m_GCPanel.pFont != nullptr)
				m_GCPanel.pFont->m_dwShadeColor = 0xFF000000;
		}
	}

	switch (dwFlags)
	{
	case WM_MOUSEMOVE:
		if (m_bMouseOver == 1 && m_pEventListener != nullptr)
			m_pEventListener->OnControlEvent(m_dwControlID, 2);
		break;
	case WM_LBUTTONDOWN:
		if (m_bVisible == 0)
			return 1;

		if (m_bOver == 1)
		{
			m_bPressed = 1;
			m_bFocused = 1;
			Update();
			return 1;
		}
		break;
	case WM_LBUTTONUP:
		if (m_bVisible == 0)
			return 1;

		if (m_bEnable == 1 && m_bOver == 1 && m_bPressed == 1 && m_pEventListener != nullptr)
		{
			unsigned int dwServerTime = g_pTimerManager->GetServerTime();
			if (dwServerTime > m_dwOldTime + 500)
			{
				if (m_bSound == 1 && g_pSoundManager != nullptr)
				{
					if (g_pSoundManager->GetSoundData(53))
						g_pSoundManager->GetSoundData(53)->Play(0, 0);
				}

				Update();
				m_pEventListener->OnControlEvent(m_dwControlID, 0);
				m_dwOldTime = g_pTimerManager->GetServerTime();
				m_bPressed = 0;
			}
			return 1;
		}
		m_bPressed = 0;
		break;
	default:
		return 0;
	}

	if (m_cAlwaysAlt != 0 && m_pAltText->IsVisible())
		m_pAltText->SetVisible(1);

	Update();
	return 0;
}

void SButton::SetSelected(int bSelected)
{
	m_bSelected = bSelected;
}

void SButton::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	if (m_dwControlID == 4628)
		m_GCPanel.dwColor = 0xAA000077;

	if (m_cBlink != 0)
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();
		if ((dwServerTime / 750) % 2)
			m_GCPanel.nTextureIndex = 2;
		else if ((dwServerTime / 750) % 3)
			m_GCPanel.nTextureIndex = 1;
		else
			m_GCPanel.nTextureIndex = 0;
	}


	SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);
	if (m_pAltText != nullptr &&
		m_pAltText->IsVisible()	&&
		(m_GCPanel.nTextureSetIndex > 0 || m_GCPanel.nTextureSetIndex < -3))
	{
		ivParentPos = ivParentPos + TMVector2(m_nPosX, m_nPosY);

		if (((ivParentPos.x + m_pAltText->m_nPosX) + m_pAltText->m_nWidth) > (float)g_pDevice->m_dwScreenWidth)
			ivParentPos.x = ((float)g_pDevice->m_dwScreenWidth - (m_pAltText->m_nPosX * 2.0f)) - m_pAltText->m_nWidth;

		m_pAltText->FrameMove2(pDrawList, ivParentPos, 28, nFlag);
	}
}

void SButton::Update()
{
	if (m_bEnable == 0)
	{
		m_GCPanel.nTextureIndex = 4;
		if (m_GrayType != 0)
			m_GCPanel.dwColor = 0xFF7F7F7F;
	}
	else if (m_bSelected == 1)
	{
		m_GCPanel.nTextureIndex = 3;
	}
	else if (m_bOver == 1)
	{
		if (m_bPressed == 1)
			m_GCPanel.nTextureIndex = 2;
		else
			m_GCPanel.nTextureIndex = 1;
	}
	else
	{
		m_GCPanel.nTextureIndex = 0;
		if (m_GCPanel.pFont != nullptr)
			m_GCPanel.dwColor = 0x1010101;
		else
			m_GCPanel.dwColor = m_dwColor;
	}
}
