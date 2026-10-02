#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"

int SCursor::m_nCursorType{ 0 };
HCURSOR SCursor::m_hCursor1{};
HCURSOR SCursor::m_hCursor2{};

SCursor::SCursor(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, 0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_eStyle = ECursorStyle::TMC_CURSOR_HAND;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_CURSOR;
	m_GCPanel.nTextureIndex = 0;
	m_nPosX = inX;
	m_nPosY = inY;
	g_pCursor = this;
	m_pAttachedItem = nullptr;
}

SCursor::~SCursor()
{
	if (m_eStyle == ECursorStyle::TMC_CURSOR_PICKUP)
	{
		SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;
		if (pControlContainer != nullptr)
		{
			if(m_GeomItem.nLayer >= 0)
				RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GeomItem, m_GeomItem.nLayer);
		}
	}
}

int SCursor::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (dwFlags == WM_MOUSEMOVE)
		SetPosition(nX, nY);

	return SControl::OnMouseEvent(dwFlags, wParam, nX, nY);
}

void SCursor::FrameMove2(stGeomList* pDrawList, TMVector2 ivParenPos, int inParentLayer, int nFlag)
{
	m_GCPanel.nPosX = m_nPosX;
	m_GCPanel.nPosY = m_nPosY;

	if (SCursor::m_nCursorType == 2)
	{
		m_GCPanel.nPosX = -100.0f;
		m_GCPanel.nPosY = -100.0f;
	}

	m_GCPanel.nWidth = m_nWidth;
	m_GCPanel.nHeight = m_nHeight;
	m_GCPanel.nLayer = inParentLayer;
	m_bAlwaysOnTop = 1;

	if (m_eStyle == ECursorStyle::TMC_CURSOR_PICKUP)
	{
		if (m_pAttachedItem == nullptr)
		{
			m_eStyle = ECursorStyle::TMC_CURSOR_HAND;
			return;
		}

		m_GeomItem = *m_pAttachedItem->GetGeomControl();

		if (m_GeomItem.eRenderType == RENDERCTRLTYPE::RENDER_3DOBJ)
		{
			// The attached geometry is a local copy: anchor it to the live cursor
			// before adding the item's pickup offset.  The imported 7.59 branch
			// reused the grid's absolute coordinates, so 3D inventory icons stayed
			// behind while the cursor moved.
			m_GeomItem.nPosX = m_nPosX + m_pAttachedItem->GetPos().x;
			m_GeomItem.nPosY = m_nPosY + m_pAttachedItem->GetPos().y;

			m_GeomItem.nLayer = m_GCPanel.nLayer;
			// PickupItem applies the same complete-AABB containment used by every 1x1
			// receptacle. Copying the geometry preserves that fScale on the cursor;
			// replacing it with 1.5 made dragged models invade adjacent cells.

			unsigned int dwServerTime = g_pTimerManager->GetServerTime() % 3000;
			m_GeomItem.fAngle = ((float)dwServerTime * 6.28f) / 3000.0f;
		}
		else
		{
			m_GeomItem.nPosX = (float)(m_nPosX + m_pAttachedItem->GetPos().x) - 12.0f;
			m_GeomItem.nPosY = (float)(m_nPosY + m_pAttachedItem->GetPos().y) - 12.0f;
			m_GeomItem.nLayer = m_GCPanel.nLayer;
		}

		AddRenderControlItem(pDrawList, &m_GeomItem, inParentLayer);
		AddRenderControlItem(pDrawList, &m_GCPanel, inParentLayer);
	}
	else
	{
		AddRenderControlItem(pDrawList, &m_GCPanel, inParentLayer);
	}
}

void SCursor::SetPosition(int iX, int iY)
{
	m_nPosX = (float)iX;
	m_nPosY = (float)iY;
}

void SCursor::SetVisible(int bVisible)
{
	m_bVisible = bVisible;
}

void SCursor::SetStyle(ECursorStyle eStyle)
{
	m_eStyle = eStyle;
}

ECursorStyle SCursor::GetStyle()
{
	return m_eStyle;
}

GeomControl* SCursor::GetGeomControl()
{
	return &m_GCPanel;
}

int SCursor::AttachItem(SGridControlItem* pItem)
{
	if (pItem == nullptr)
		return 0;
	if (m_eStyle != ECursorStyle::TMC_CURSOR_HAND)
		return 0;

	m_eStyle = ECursorStyle::TMC_CURSOR_PICKUP;
	m_pAttachedItem = pItem;
	return 1;
}

//int SGridControl::SetItemOnGrid(STRUCT_ITEM* item, int CellIndexX, int CellIndexY)//adicionado
//{
//	auto setItem = new STRUCT_ITEM();
//
//	if (setItem)
//	{
//		memcpy(setItem, item, sizeof(STRUCT_ITEM));
//
//		auto pItem = new SGridControlItem(0, setItem, 0.0f, 0.0f);
//
//		if (pItem)
//		{
//			AddItem(pItem, CellIndexX, CellIndexY);
//			return true;
//		}
//	}
//
//	return false;
//}

SGridControlItem* SCursor::DetachItem()
{
	SGridControlItem* pItem = m_pAttachedItem;
	m_pAttachedItem = nullptr;

	if (m_eStyle == ECursorStyle::TMC_CURSOR_PICKUP)
		m_eStyle = ECursorStyle::TMC_CURSOR_HAND;

	return pItem;
}
