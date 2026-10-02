#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "EventTranslator.h"
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMFieldScene.h"

SMessageBox::SMessageBox(const char* istrMessage, char ibyMessageBoxType, float inX, float inY)
	: SPanel(-2, inX, inY, 256.0f, 128.0f,
		0x1010101, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	// WYD 7.48's NewUI message box is not the older 256x172 composition kept by
	// TMProject. The original constructor at VA 0x00403EB8 uses texture sets
	// 164/165 and a 232x107 visible frame; preserving that exact tree also keeps
	// the rendered buttons and their hit boxes on the same coordinates.
	// Keep the classic global mode for the 7.48 HUD and hit-test geometry, but
	// confirmations still use the native NewUI composition requested in-game.
	// Type 0 is the shared Yes/No box used by AutoTrade purchase confirmation.
	const bool useWYD748NewUI =
		ibyMessageBoxType == TMC_MESSAGEBOX_MESSAGE || g_UIVer == 2;
	m_pPanel1 = nullptr;
	m_pPanel2 = nullptr;
	m_pMessage = nullptr;
	m_pMessage2 = nullptr;
	m_pCaption = nullptr;
	m_pOKButton = nullptr;
	m_pCancelButton = nullptr;
	m_pPanelBtn1 = nullptr;
	m_pPanelBtn2 = nullptr;

	m_dwArg = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_MESSAGEBOX;
	m_dwControlID = 4617;
	m_dwMessage = -1;

	const float centeredWidth = useWYD748NewUI ? 232.0f : 256.0f;
	const float centeredHeight = 128.0f;
	SetCenterPos(m_dwControlID, inX, inY, centeredWidth * RenderDevice::m_fWidthRatio,
		centeredHeight * RenderDevice::m_fHeightRatio);

	if (useWYD748NewUI)
	{
		m_pPanel1 = new SPanel(164, 0.0f, 0.0f, 232.0f, 107.0f,
			0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pMessage = new SText(-1, istrMessage, 0xFFFFFFFF, 10.0f, 36.0f,
			(float)strlen(istrMessage) * 6.0f, 12.0f, -1, 1, 0, 0);
		m_pMessage2 = new SText(-1, istrMessage, 0xFFFFFFFF, 10.0f, 52.0f,
			(float)strlen(istrMessage) * 6.0f, 12.0f, -1, 1, 0, 0);
		m_pCaption = new SText(-1, g_pMessageStringTable[237], 0xFFFFFFFF, 96.0f, 8.0f,
			(float)strlen(g_pMessageStringTable[237]) * 6.0f, 12.0f, -1, 1, 0, 0);
	}
	else
	{
		// Classic FUN_00403eb8 uses the compact MessageBox texture set 9.  Sets
		// 501+ and the 172-pixel layout belong to the later client and render as
		// missing/bare UI with the distributed 7.48 atlas.
		m_pPanel1 = new SPanel(9, 0.0f, 0.0f, 256.0f, 97.0f,
			0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pMessage = new SText(-1, istrMessage, 0xFFFFFFFF, 20.0f, 17.0f,
			(float)strlen(istrMessage) * 6.0f, 14.0f, -1, 1, 0, 0);
		m_pMessage2 = new SText(-1, istrMessage, 0xFFFFFFFF, 20.0f, 32.0f,
			(float)strlen(istrMessage) * 6.0f, 14.0f, -1, 1, 0, 0);
	}

	m_bPickable = 1;

	if (m_pCaption != nullptr)
		AddChild(m_pCaption);
	if (m_pMessage != nullptr)
		AddChild(m_pMessage);
	if (m_pMessage2 != nullptr)
		AddChild(m_pMessage2);

	m_pMessage->SetText((char*)"", 0);
	m_pMessage2->SetText((char*)"", 0);
	m_byMessageBoxType = ibyMessageBoxType;

	if (ibyMessageBoxType == TMC_MESSAGEBOX_MESSAGE)
	{
		if (useWYD748NewUI)
		{
			// The 7.48 renderer walks children in reverse order, so these skins are
			// inserted after the clickable controls and before the main background.
			m_pPanelBtn1 = new SPanel(165, 20.0f, 76.0f, 88.0f, 23.0f,
				0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
			m_pPanelBtn2 = new SPanel(165, 124.0f, 76.0f, 88.0f, 23.0f,
				0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
			m_pOKButton = new SButton(-2, 12.0f, 77.0f, 103.0f, 21.0f,
				0, 1, g_pMessageStringTable[238]);
		}
		else
			m_pOKButton = new SButton(14, 25.0f, 56.0f, 103.0f, 21.0f,
				0, 1, g_pMessageStringTable[238]);

		if (m_pOKButton != nullptr)
		{
			m_pOKButton->SetControlID(1);
			m_pOKButton->SetEventListener(this);
			AddChild(m_pOKButton);
		}

		if (useWYD748NewUI)
			m_pCancelButton = new SButton(-2, 117.0f, 77.0f, 103.0f, 21.0f,
				0, 1, g_pMessageStringTable[239]);
		else
			m_pCancelButton = new SButton(15, 128.0f, 56.0f, 103.0f, 21.0f,
				0, 1, g_pMessageStringTable[239]);
		if (m_pCancelButton != nullptr)
		{
			m_pCancelButton->SetControlID(2);
			m_pCancelButton->SetEventListener(this);
			AddChild(m_pCancelButton);
		}
	}
	else if (ibyMessageBoxType == TMC_MESSAGEBOX_OK)
	{
		if (useWYD748NewUI)
		{
			m_pPanelBtn1 = new SPanel(165, 70.0f, 76.0f, 88.0f, 23.0f,
				0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
			m_pOKButton = new SButton(-2, 70.0f, 76.0f, 88.0f, 23.0f,
				0, 1, g_pMessageStringTable[238]);
		}
		else
			m_pOKButton = new SButton(14, 79.0f, 56.0f, 103.0f, 21.0f,
				0, 1, g_pMessageStringTable[238]);

		if (m_pOKButton != nullptr)
		{
			m_pOKButton->SetControlID(1);
			m_pOKButton->SetEventListener(this);
			AddChild(m_pOKButton);
		}
	}

	if (m_pPanelBtn1 != nullptr)
		AddChild(m_pPanelBtn1);
	if (m_pPanelBtn2 != nullptr)
		AddChild(m_pPanelBtn2);

	if (m_pPanel1 != nullptr)
		AddChild(m_pPanel1);
}

SMessageBox::~SMessageBox()
{
}

int SMessageBox::OnControlEvent(DWORD idwControlID, DWORD idwEvent)
{
	if (idwControlID == m_pOKButton->GetControlID()	&&
		!idwEvent &&
		m_pEventListener != nullptr)
	{
		SetVisible(0);

		m_pEventListener->OnControlEvent(m_dwControlID, 0);
		return 1;
	}
	else if (m_pCancelButton != nullptr &&
		idwControlID == m_pCancelButton->GetControlID()	&&
		!idwEvent &&
		m_pEventListener != nullptr)
	{
		SetVisible(0);

		m_pEventListener->OnControlEvent(m_dwControlID, 1);
		return 1;
	}

	return 0;
}

void SMessageBox::SetMessage(char* istrMessage, unsigned int dwMessageValue, char* istrMessage2)
{
	m_dwMessage = dwMessageValue;

	if (strlen(istrMessage) > 34 && !istrMessage2)
	{
		LPSTR pPrev = CharPrev(istrMessage, &istrMessage[34]);
		int nLen = pPrev - istrMessage;

		char szMessage[128]{};
		strncpy(szMessage, istrMessage, nLen);

		m_pMessage->SetText(szMessage, 0);
		m_pMessage2->SetText(&istrMessage[nLen], 0);
	}
	else
	{
		m_pMessage->SetText(istrMessage, 0);
		m_pMessage2->SetText(istrMessage2, 0);
	}
}

void SMessageBox::SetMessage(unsigned int dwMessageValue)
{
	m_dwMessage = dwMessageValue;
}

unsigned int SMessageBox::GetMessageA()
{
	return m_dwMessage;
}

int SMessageBox::OnCharEvent(char iCharCode, int lParam)
{
	if (m_pEventListener == nullptr)
		return 0;

	if (iCharCode == VK_RETURN || iCharCode == 'Y' || iCharCode == 'y')
	{
		SetVisible(0);
		m_pEventListener->OnControlEvent(m_dwControlID, 0);
		return 1;
	}

	if (iCharCode != 'N' && iCharCode != 'n')
		return 0;

	SetVisible(0);
	m_pEventListener->OnControlEvent(m_dwControlID, 1);
	return 1;
}

void SMessageBox::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	SControl::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);
}

void SMessageBox::SetVisible(int bVisible)
{
	SControl::SetVisible(bVisible);

	if (g_pCurrentScene != nullptr)
	{
		if (bVisible == 1)
		{
			SControlContainer* Ctrl = g_pCurrentScene->GetCtrlContainer();
			if (Ctrl != nullptr)
				Ctrl->SetFocusedControl(this);
		}
		else
		{
			SControlContainer* Ctrl = g_pCurrentScene->GetCtrlContainer();
			if (Ctrl != nullptr)
				Ctrl->SetFocusedControl(nullptr);
		}
	}
}

int SMessageBox::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);

	if (dwFlags != WM_LBUTTONDOWN || m_bOver != 1)
		return SPanel::OnMouseEvent(dwFlags, wParam, nX, nY);

	if (g_pCurrentScene == nullptr)
		return 1;

	TMScene* pScene = g_pCurrentScene;
	pScene->GetCtrlContainer()->SetFocusedControl(this);

	return SPanel::OnMouseEvent(0x201, wParam, nX, nY);
}
