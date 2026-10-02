#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMFieldScene.h"

SEditableText::SEditableText(int inTextureSetIndex, const char* istrText, size_t inMaxStringLen, int ibPasswd, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight, int ibBorder, unsigned int idwBorderColor, unsigned int dwType, unsigned int dwAlignType)
	: SText(inTextureSetIndex,
		0,
		idwFontColor,
		inX,
		inY,
		inWidth,
		inHeight,
		ibBorder,
		idwBorderColor,
		dwType,
		dwAlignType)
{
	m_cTempChar = 0;
	m_cReserved = 0;
	m_nMaxStringLen = inMaxStringLen;
	m_nCursorVisible = 0;
	m_bPasswd = ibPasswd;
	m_bEncrypt = 0;
	m_bKorean = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT;
	m_nMaxStringLen = m_nMaxStringLen;

	if (istrText != nullptr)
	{
		BASE_UnderBarToSpace(const_cast<char*>(istrText));
		if (!strcmp(istrText, " "))
			istrText = nullptr;
	}

	int nCopyLen = 0;
	if (istrText != nullptr)
	{
		nCopyLen = strlen(istrText) <= inMaxStringLen ? strlen(istrText) : inMaxStringLen;
		strncpy(m_strText, istrText, nCopyLen);
	}
	else
		nCopyLen = 0;

	m_strText[nCopyLen] = 0;
	m_strComposeText[0] = 0;
}

SEditableText::~SEditableText()
{
}

void SEditableText::SetText(char* istrText)
{
	if (istrText[0] == 0)
	{
		memset(m_strText, 0, sizeof(m_strText));
		Update();
		return;
	}

	int nCopyLen = strlen(istrText) <= m_nMaxStringLen ? strlen(istrText) : m_nMaxStringLen;

	strncpy(m_strText, istrText, nCopyLen);
	m_strText[nCopyLen] = 0;
	Update();
}

char* SEditableText::GetText()
{
	return m_strText;
}

int SEditableText::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	int bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);

	if (dwFlags != WM_LBUTTONDOWN)
		return SText::OnMouseEvent(dwFlags, wParam, nX, nY);

	if (bOver != 1)
		return 0;

	m_bFocused = 1;
	return 1;
}

int SEditableText::OnCharEvent(char iCharCode, int lParam)
{
	if (m_bEnable == 0)
		return 0;

	if (m_bFocused != 1)
		return 0;

	if (iCharCode == VK_ESCAPE)
	{
		if (g_nKeyType == 0)
		{
			SetText((char*)"");
			m_bFocused = 0;
			g_pCurrentScene->m_pControlContainer->SetFocusedControl(nullptr);
		}
		return 1;
	}
	if (iCharCode == VK_RETURN)
	{
		if (m_pEventListener != nullptr)
			return m_pEventListener->OnControlEvent(m_dwControlID, 0);

		return 0;
	}
	if (iCharCode == VK_TAB)
	{
		if (m_pEventListener != nullptr)
			return m_pEventListener->OnControlEvent(m_dwControlID, 1);

		return 0;
	}

	// Native WYD 7.48 FUN_00406bd7 keeps text editing independent from the
	// field chat UI.  The imported chat-selector block required controls
	// 90114/90129..90136 that do not exist in the 7.48 RC and dereferenced a
	// null B_CHAT_SELECT while typing or deleting text.
	if (iCharCode == VK_BACK)
	{
		if (strlen(m_strComposeText))
			return 1;

		int nTextLen = strlen(m_strText);
		if (nTextLen == 0)
		{
			// Valid 7.48 edit controls own a listener; keep a defensive guard for
			// compatibility-created controls without changing the native event.
			return m_pEventListener != nullptr
				? m_pEventListener->OnControlEvent(m_dwControlID, TMEDIT_MSG_NO_STRING)
				: 0;
		}

		LPSTR szPrevText = CharPrev(m_strText, &m_strText[nTextLen]);

		int nLen = szPrevText - m_strText;
		m_strText[nLen] = 0;
		Update();

		return 1;
	}
	else if (m_nMaxStringLen > strlen(m_strText))
	{
		if (m_cTempChar)
			m_cTempChar = 0;

		LPSTR lpCurrent = &m_strText[strlen(m_strText)];
		lpCurrent[0] = iCharCode;
		lpCurrent++;
		lpCurrent[0] = 0;
		lpCurrent++;
		Update();

		return 1;
	}
	else
	{
		int nTextLen = strlen(m_strText);
		if (nTextLen == 0)
			return 1;

		if (!IsClearString(m_strText, nTextLen - 1))
		{
			m_strText[nTextLen - 1] = 0;
			m_strComposeText[0] = 0;
		}

		Update();
		m_cTempChar = iCharCode;
		return 1;
	}

	return 0;
}

int SEditableText::OnChangeIME()
{
	// No need for now
	return 0;
}

int SEditableText::OnIMEEvent(char* ipComposeString)
{
	if (ipComposeString == nullptr)
		return 0;

	if (m_bFocused != 1)
		return SText::OnIMEEvent(ipComposeString);

	strncpy_s(m_strComposeText, ipComposeString, _TRUNCATE);
	Update();
	return 1;
}

int SEditableText::OnKeyDownEvent(unsigned int iKeyCode)
{
	if (m_bEnable == 0)
		return 0;
	if (m_bFocused != 1)
		return 0;

	if (iKeyCode == VK_PRIOR)
		return m_pEventListener->OnControlEvent(m_dwControlID, 2);
	if (iKeyCode == VK_UP)
		return m_pEventListener->OnControlEvent(m_dwControlID, 3);
	if (iKeyCode == VK_NEXT)
		return m_pEventListener->OnControlEvent(m_dwControlID, 4);
	if (iKeyCode == VK_DOWN)
		return m_pEventListener->OnControlEvent(m_dwControlID, 5);
	if(iKeyCode == VK_DELETE)
		return m_pEventListener->OnControlEvent(m_dwControlID, 6);

	return 0;
}

int SEditableText::OnKeyUpEvent(unsigned int iKeyCode)
{
	return m_bFocused == 1;
}

void SEditableText::Update()
{
	int nStringLen = strlen(m_strText);
	int nComposeStringLen = strlen(m_strComposeText);

	if (m_bPasswd == 0)
	{
		strcpy(m_GCText.strString, m_strText);
	}
	else
	{
		for (int nIndex = 0; nIndex < nComposeStringLen + nStringLen; ++nIndex)
			m_GCText.strString[nIndex] = '•';

		m_GCText.strString[nStringLen] = 0;
	}

	m_GCText.pFont->SetText(m_GCText.strString, 0xFFFFFFFF, 0);
}

void SEditableText::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	int nStringLen = strlen(m_strText);
	if (m_bFocused == 1)
	{
		++m_nCursorVisible;
		m_nCursorVisible %= 20;

		if (m_nCursorVisible == 10)
		{
			m_GCText.strString[nStringLen] = '|';
			m_GCText.strString[nStringLen + 1] = 0;
			m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
		}
		else if (m_nCursorVisible == 0)
		{
			m_GCText.strString[nStringLen] = 0;
			m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
		}
	}
	else if (m_GCText.strString[nStringLen] == '|')
	{
		m_GCText.strString[nStringLen] = 0;
		m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
	}
	else
		m_GCText.strString[nStringLen] = 0;

	SText::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);
}

void SEditableText::SetFocused(int bFocused)
{
	m_bFocused = bFocused;
}

int SEditableText::IsIMENative()
{
	return 0;
}
