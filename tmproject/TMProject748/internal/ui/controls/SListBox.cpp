#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "EventTranslator.h"
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMFieldScene.h"

SListBoxItem::SListBoxItem(const char* istrText, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight, int ibBorder, unsigned int idwBorderColor, unsigned int dwType, unsigned int dwAlignType)
	: SText(-1, //bordas do serverlist
		istrText,
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
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_LISTBOXITEM;
	// Ghidra FUN_00407203 proves that the native 7.48 list item owns no
	// selection panel: SText type/border state is the translucent highlight.
	m_bBGColor = 0;
}

SListBoxItem::~SListBoxItem()
{
}

void SListBoxItem::FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag)
{
	if (m_bBGColor == 0)
	{
		// Ghidra FUN_00407281 uses type 2 plus a one-pixel border for the
		// selected row and type 1 with no border for every other row.
		if (nFlag == 1)
		{
			m_cBorder = 1;
			SetType(2);
		}
		else
		{
			m_cBorder = 0;
			SetType(1);
		}
	}

	SText::FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
}

int SListBoxItem::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);
	Update();
	return 0;
}

SListBoxBoardItem::SListBoxBoardItem(char* szIndex, char* szVIndex, char* szTitle, char* szWriter, char* szCount, char* szDate, unsigned int dwColor, int bTitile)
	: SListBoxItem(szVIndex, 0xFFFFFFFF, 0.0f, 0.0f, 500.0f, 12.0f, 0, 0x77777777, 1, 0)
{
	unsigned int dwFontColor = 0xFFFFFFFF;

	if (strcmp(szVIndex, g_pMessageStringTable[176]) && (*szVIndex < '0' || *szVIndex > '9'))
		dwFontColor = 0xFFAAFFAA;

	m_pTitleText = new SText(-2, szTitle, dwFontColor, 44.0f, 0.0f, 260.0f, 16.0f, 1, dwColor, 1, bTitile == 1 ? 1 : 0);
	m_pWriterText = new SText(-2, szWriter, dwFontColor, 305.0f, 0.0f, 79.0f, 16.0f, 1, dwColor, 1, 1);
	m_pCountText = new SText(-2, szCount, dwFontColor, 385.0f, 0.0f, 34.0f, 16.0f, 1, dwColor, 1, 1);
	m_pDateText = new SText(-2, szDate, dwFontColor, 420.0f, 0.0f, 80.0f, 16.0f, 1, dwColor, 1, 1);

	sprintf(m_szIndex, "%s", szIndex);
}

SListBoxBoardItem::~SListBoxBoardItem()
{
	SAFE_DELETE(m_pTitleText);
	SAFE_DELETE(m_pDateText);
	SAFE_DELETE(m_pWriterText);
	SAFE_DELETE(m_pCountText);
}

void SListBoxBoardItem::FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag)
{
	m_pTitleText->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
	m_pWriterText->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
	m_pCountText->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
	m_pDateText->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);

	SListBoxItem::FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
}

void SListBoxBoardItem::SetPos(float nPosX, float nPosY)
{
	SControl::SetPos(nPosX, nPosY);
	m_pTitleText->SetPos(nPosX + 44.0f, nPosY);
	m_pWriterText->SetPos(nPosX + 305.0f, nPosY);
	m_pCountText->SetPos(nPosX + 385.0f, nPosY);
	m_pDateText->SetPos(nPosX + 420.0f, nPosY);
}

SListBoxPartyItem::SListBoxPartyItem(char* iStrText, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight, unsigned int dwCharID, int nClass, int nLevel, int nHp, int nMaxHp)
	: SListBoxItem(iStrText,
		idwFontColor,
		inX,
		inY,
		inWidth,
		inHeight,
		0,
		0x77777777,
		1,
		0)
{
	m_dwCharID = dwCharID;
	m_nClass = nClass;
	m_nLevel = nLevel;
	m_nState = 0;
	m_pLevelText = 0;
	m_pHpProgress = 0;

	char szLevel[16]{};
	sprintf(szLevel, "%d", m_nLevel + 1);

	m_pLevelText = new SText(-1, szLevel, 0xFFFFFFAA, 74.0f, 3.0f, 8.0f, 12.0f, 0, 0x77777777, 1, 0);
	m_pHpProgress = new SProgressBar(7, nHp, nMaxHp, 0.0f, 15.0f, 110.0f, 8.0f, 0xFFFFFFFF, 0xFFFFFFFF, 1);
	m_pDirPanel = new SPanel(-1, 66.0f, 0.0f, 16.0f, 16.0f, 0, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

	char *pDest = strchr(iStrText, '^');
	if (pDest && !IsClearString2(iStrText, pDest - iStrText))
		pDest = 0;

	if (pDest != nullptr)
	{
		char szMyMob[64]{};
		memset(szMyMob, 0, 64);
		memcpy(szMyMob, iStrText, pDest - iStrText);
		SetText(szMyMob, 0);
	}
	else
		SetText(iStrText, 0);
}

SListBoxPartyItem::~SListBoxPartyItem()
{
	SAFE_DELETE(m_pLevelText);
	SAFE_DELETE(m_pHpProgress);
	SAFE_DELETE(m_pDirPanel);
}

void SListBoxPartyItem::FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag)
{
	m_pLevelText->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
	m_pHpProgress->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);

	ivItemPos.x -= 4.0f;
	ivItemPos.y -= 1.0f;

	if (m_nState == 1)
	{
		if ((g_pTimerManager->GetServerTime() % 1000) / 500)
			m_GCText.dwColor = 0xFFFF0000;
		else
			m_GCText.dwColor = 0xFFFFFFFF;

		m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
	}

	SListBoxItem::FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
}

SListBoxServerItem::SListBoxServerItem(int nTextureSet, char* iStrText, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight, int nCount, char cCastle, char cGoldBug, int Num)
	: SListBoxItem(iStrText,
		idwFontColor,
		inX,
		inY,
		inWidth,
		inHeight,
		0,
		0x77777777,
		1,
		0)
{
	m_pBusyProgress = 0;
	m_nCurrent = 0;
	m_cCastle = cCastle;
	m_nCurrent = nCount;
	m_cConnected = 1;
	m_pCrownPanel = 0;
	m_pGoldBugPanel = 0;
	m_pAgePanel = 0;
	m_cGoldBug = cGoldBug;

	unsigned int dwCol1 = -1;
	unsigned int dwCol2 = -1;

	if (m_cCastle == 1)
		m_pCrownPanel = new SPanel(151, 24.0f + 104.0f, 1.0f, 16.0f, 16.0f, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
	if (m_cGoldBug == 1)
		m_pGoldBugPanel = new SPanel(316, 24.0f + 122.0f, 1.0f, 16.0f, 16.0f, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

	if (nTextureSet <= -1)
	{
		dwCol1 = 0xFFFF0000;
		dwCol2 = 0xFF222222;
	}
	if (nTextureSet <= -2)
	{
		dwCol1 = 0xFF00FF99;
		dwCol2 = 0xFF222222;
	}

	m_pBusyProgress = new SProgressBar(nTextureSet, nCount, 600, inWidth - 58.0f, 6.0f, 20.0f, 6.0f, dwCol1, dwCol2, 1);
}

SListBoxServerItem::~SListBoxServerItem()
{
	SAFE_DELETE(m_pGoldBugPanel);
	SAFE_DELETE(m_pCrownPanel);
	SAFE_DELETE(m_pBusyProgress);
}

void SListBoxServerItem::FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag)
{
	if (m_pCrownPanel != nullptr)
		m_pCrownPanel->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
	if (m_pGoldBugPanel != nullptr)
		m_pGoldBugPanel->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
	if (m_pBusyProgress != nullptr)
		m_pBusyProgress->FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);

	SListBoxItem::FrameMove2(pDrawList, ivItemPos, inParentLayer, nFlag);
}

SListBox::SListBox(int inTextureSetIndex, int inMaxCount, int inVisibleCount, float inX, float inY, float inWidth, float inHeight, unsigned int idwColor, RENDERCTRLTYPE eRenderType, int bSelectEnable, int bScrollBar, int bEditable)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, idwColor, eRenderType)
{
	m_cScrollBar = bScrollBar;
	m_cEditable = bEditable;
	m_sEditLine = 0;
	m_nMaxCount = inMaxCount;
	m_nVisibleCount = inVisibleCount;
	m_nStartItemIndex = 0;
	m_nSelectedItem = -1;
	m_pEditLine = 0;
	m_pScrollBar = 0;
	m_nNumItem = 0;

	if (m_nMaxCount > 1000)
		m_nMaxCount = 1000;

	m_bSelectEnable = bSelectEnable;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_LISTBOX;
	m_bRButton = 0;

	if (m_cScrollBar == 1)
	{
		m_pScrollBar = new SScrollBar(0, 0, 0.0, 0.0, (float)12, inHeight - 1.0f, 0, 0, 0xFFAAAAAA, 1);

		if (m_pScrollBar != nullptr)
		{
			m_pScrollBar->SetVisible(1);
			m_pScrollBar->SetControlID(1);
			m_pScrollBar->SetEventListener(this);
			AddChild(m_pScrollBar);
		}
	}
	else if (m_cScrollBar == 2)
	{
		m_pScrollBar = new SScrollBar(0, 0, (float)(inWidth - 12.0f) + 2.0f, 0.0f, 12.0f, inHeight - 1.0f, 0, 0, 0xFFAAAAAA, 0);

		if (m_pScrollBar != nullptr)
		{
			m_pScrollBar->SetVisible(1);
			m_pScrollBar->SetControlID(1);
			m_pScrollBar->SetEventListener(this);
			AddChild(m_pScrollBar);
		}
	}

	if (m_cEditable == 1)
		SetEditable();

	memset(m_pItemList, 0, sizeof(m_pItemList));

	m_fPickWidth = m_nWidth;
	m_fPickHeight = m_nHeight;
}

SListBox::~SListBox()
{
	Empty();
}

int SListBox::AddItem(SListBoxItem* ipNewItem)
{
	if (m_nNumItem >= m_nMaxCount - m_cEditable)
	{
		SAFE_DELETE(m_pItemList[0]);

		for (int i = 1; i < m_nNumItem; ++i)
			m_pItemList[i - 1] = m_pItemList[i];

		m_pItemList[m_nNumItem - 1] = ipNewItem;
		m_pItemList[m_nNumItem - 1]->m_dwID = m_nNumItem - 1;

		return 1;
	}

	m_pItemList[m_nNumItem] = ipNewItem;
	m_pItemList[m_nNumItem]->m_dwID = m_nNumItem;

	++m_nNumItem;

	if (m_cScrollBar != 0 && m_pScrollBar != nullptr)
	{
		m_pScrollBar->SetMaxValue(m_nNumItem);
		if (m_nNumItem >= m_nVisibleCount)
		{
			SetStartItemIndex(m_cEditable + m_nNumItem - m_nVisibleCount);
			m_pScrollBar->SetMaxValue(m_nNumItem);
			m_pScrollBar->SetCurrentPos(m_cEditable + m_nNumItem - m_nVisibleCount);
		}
	}

	return 1;
}

int SListBox::DeleteItem(int inItemIndex)
{
	if (m_nNumItem <= inItemIndex || inItemIndex < 0)
		return 0;

	SAFE_DELETE(m_pItemList[inItemIndex]);

	for (int i = inItemIndex + 1; i < m_nNumItem; ++i)
		m_pItemList[i - 1] = m_pItemList[i];

	m_pItemList[m_nNumItem - 1] = 0;
	m_nNumItem--;

	if (m_cScrollBar != 0)
		m_pScrollBar->SetMaxValue(m_nNumItem);

	return 1;
}

int SListBox::DeleteItem(SListBoxItem* ipItem)
{
	int i = 0;
	for (i = 0; ; ++i)
	{
		if (i >= m_nNumItem)
			return 0;

		if (m_pItemList[i] == ipItem)
			break;
	}

	if (m_pItemList[i] != nullptr)
	{
		SAFE_DELETE(m_pItemList[i]);
	}

	for (int j = i + 1; j < m_nNumItem; ++j)
		m_pItemList[j - 1] = m_pItemList[j];

	--m_nNumItem;

	if (m_cScrollBar != 0)
		m_pScrollBar->SetMaxValue(m_nNumItem);

	return 1;
}

SListBoxItem* SListBox::GetItem(int inItemIndex)
{
	// The valid range is [0, m_nNumItem); accepting the end index leaks the
	// uninitialized sentinel into server selection and confirmation handlers.
	if (inItemIndex < 0 || inItemIndex >= m_nNumItem)
		return nullptr;

	return m_pItemList[inItemIndex];
}


void SListBox::Empty()
{
	for (int i = 0; i < m_nNumItem; ++i)
	{
		SAFE_DELETE(m_pItemList[i]);
	}

	m_nNumItem = 0;
	if (m_cScrollBar != 0 && m_pScrollBar != nullptr)
		m_pScrollBar->SetMaxValue(m_nNumItem);
}

int SListBox::GetSelectedIndex()
{
	return m_nSelectedItem;
}

void SListBox::SetSelectedIndex(int nIndex)
{
	m_nSelectedItem = nIndex;
}

void SListBox::SetStartItemIndex(int nIndex)
{
	if (nIndex <= m_nNumItem - m_nVisibleCount)
	{
		m_nStartItemIndex = nIndex;
		return;
	}

	m_nStartItemIndex = m_nNumItem - m_nVisibleCount + 1;
	if (m_nStartItemIndex < 0)
		m_nStartItemIndex = 0;
}

void SListBox::SetSize(float nWidth, float nHeight)
{
	SControl::SetSize(nWidth, nHeight);

	m_fPickWidth = m_nWidth;
	m_fPickHeight = m_nHeight;

	if (m_pScrollBar != nullptr)
		m_pScrollBar->SetSize(m_nWidth, nHeight - 1.0f);
}

void SListBox::SetPickSize(float nWidth, float nHeight)
{
	float fWidthRatio = g_UIVer == 2 ? 1.0f : RenderDevice::m_fWidthRatio;
	float fHeightRatio = g_UIVer == 2 ? 1.0f : RenderDevice::m_fHeightRatio;

	m_fPickWidth = nWidth * fWidthRatio;
	m_fPickHeight = nHeight * fHeightRatio;
}

void SListBox::SetEditable()
{
	if (m_pEditLine == nullptr)
	{
		m_cEditable = 1;
		m_sEditLine = 0;

		m_pEditLine = new SEditableText(-2,	"", 79,	0, 0xFFFFFFFF, 0.0f, 0.0f, m_nWidth, 16.0f, 0, 0, 1, 0);
		if (m_pEditLine != nullptr)
		{
			m_pEditLine->SetVisible(1);
			m_pEditLine->SetControlID(2);

			m_pEditLine->SetEventListener(this);
			AddChild(m_pEditLine);
		}
	}
}

int SListBox::OnControlEvent(DWORD idwControlID, DWORD idwEvent)
{
	if (idwControlID == 1)
	{
		SetStartItemIndex(idwEvent);
		return 1;
	}

	if (idwControlID != 2)
		return 0;

	if (idwEvent == 0|| idwEvent == 8)
	{
		if (m_sEditLine >= m_nMaxCount - 1)
			return 1;

		char *szText = m_pEditLine->GetText();

		SListBoxItem* pItem = new SListBoxItem(szText, 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
		AddItem(pItem);

		m_pEditLine->SetText((char*)"");
		++m_sEditLine;

		if (idwEvent == 8)
			m_pEditLine->OnCharEvent(m_pEditLine->m_cTempChar, 0);
	}
	else if (idwEvent == 7)
	{
		if (m_sEditLine > 0)
		{
			SListBoxItem* ipItem = GetItem(--m_sEditLine);
			if (ipItem == nullptr)
				return 1;

			m_pEditLine->SetText(ipItem->GetText());
			DeleteItem(ipItem);
		}
	}
	else if (idwEvent == 2)
		m_pScrollBar->Up();
	else if (idwEvent == 4)
		m_pScrollBar->Down();

	return 1;
}

void SListBox::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);

	int nCurrentVisibleCount = 0;
	if (m_nVisibleCount <= m_nNumItem - m_nStartItemIndex)
		nCurrentVisibleCount = m_nVisibleCount;
	else
		nCurrentVisibleCount = m_nNumItem - m_nStartItemIndex;

	int bStr = 0;
	for (int nIndex = 0; nIndex < nCurrentVisibleCount; ++nIndex)
	{
		int nFlag = nIndex + m_nStartItemIndex == m_nSelectedItem ? 1 : 0;
		m_pItemList[nIndex + m_nStartItemIndex]->FrameMove2(pDrawList,
			TMVector2(ivParentPos.x + m_nPosX, ((ivParentPos.y + m_nPosY) + (((float)nIndex * m_nHeight) / (float)m_nVisibleCount))),
			inParentLayer,
			m_bSelectEnable & nFlag);

		if (!bStr)
		{
			SListBoxItem* pItem = m_pItemList[nIndex + m_nStartItemIndex];
			if (pItem != nullptr)
			{
				if (strlen(pItem->GetText()))
					bStr = 1;
			}
		}
	}
	if (m_cEditable == 1 && m_pEditLine != nullptr)
	{
		m_pEditLine->SetPos(0.0f, ((float)(m_sEditLine - m_nStartItemIndex) * m_nHeight) / (float)m_nVisibleCount);

		if (m_sEditLine - m_nStartItemIndex > nCurrentVisibleCount || m_sEditLine - m_nStartItemIndex < 0)
			m_pEditLine->SetVisible(0);
		else
			m_pEditLine->SetVisible(1);
	}
}

int SListBox::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (!m_bSelectEnable && !m_cEditable)
		return 0;

	int bInListBox = 0;
	if (m_cScrollBar)
		bInListBox = PointInRect(nX, nY, m_nPosX, m_nPosY, m_fPickWidth - 14.0f, m_fPickHeight);
	else
		bInListBox = PointInRect(nX, nY, m_nPosX, m_nPosY, m_fPickWidth, m_fPickHeight);

	if (bInListBox == 1 && dwFlags == WM_LBUTTONUP && m_cEditable == 1 && m_pEditLine != nullptr)
	{
		g_pCurrentScene->m_pControlContainer->SetFocusedControl(m_pEditLine);
	}

	if (dwFlags == 512)
	{
		m_nHoverItem = -1;
		int nLocalIndex = (int)(((float)nY - m_nPosY) / (float)(m_nHeight / (float)m_nVisibleCount));
		if (m_nStartItemIndex + nLocalIndex < m_nNumItem)
			m_nHoverItem = m_nStartItemIndex + nLocalIndex;
	}

	if (bInListBox == 1 && (dwFlags == WM_LBUTTONDOWN || dwFlags == WM_RBUTTONDOWN))
		return 1;

	if (bInListBox == 1 && dwFlags == WM_LBUTTONUP)
	{
		int nLocalIndex = (int)(((float)nY - m_nPosY) / (float)(m_nHeight / (float)m_nVisibleCount));
		if (m_nStartItemIndex + nLocalIndex < m_nNumItem)
		{
			m_nSelectedItem = m_nStartItemIndex + nLocalIndex;
			if (m_pEventListener != nullptr)
			{
				m_bRButton = 0;
				m_pEventListener->OnControlEvent(m_dwControlID, m_nSelectedItem);
			}
		}

		if (m_bSelectEnable == 1)
			return 1;
	}



	if (bInListBox != 1 || dwFlags != WM_RBUTTONUP)
		return 0;

	int nLocalIndex = (int)(((float)nY - m_nPosY) / (float)(m_nHeight / (float)m_nVisibleCount));
	if (m_nStartItemIndex + nLocalIndex < m_nNumItem)
	{
		m_nSelectedItem = m_nStartItemIndex + nLocalIndex;
		if (m_pEventListener != nullptr)
		{
			m_bRButton = 1;
			m_pEventListener->OnControlEvent(m_dwControlID, m_nSelectedItem);
		}
	}

	if (m_bSelectEnable != 1)
		return 0;

	return 1;
}
