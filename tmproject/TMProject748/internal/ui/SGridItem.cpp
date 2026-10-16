#include "pch.h"
// SGridControl split by responsibility; see SGrid.cpp for the grid model.
#include "SGrid.h"
#include "GridInsertion.h"
#include "GridItemPresentation.h"
#include "NativeInventoryGlow.h"
#include "../application/NativeVolatileRoutes.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMMesh.h"
#include "TMFieldScene.h"
#include "TMUtil.h"
#include "ItemEffect.h"
#include "ClientDiagnostics.h"
#include "SellConfirmationText.h"
#include "NativeSaleQuote.h"
#include "SGridSupport.h"


SGridControlItem::SGridControlItem(SGridControl* pParent, STRUCT_ITEM* pItem, float inX, float inY)
	: S3DObj(0, inX, inY, 24.0f, 24.0f)
{
	m_GCEnable = GeomControl(RENDERCTRLTYPE::RENDER_IMAGE_STRETCH, -2, 0.0f, 0.0f, 1.0f, 1.0f, 0, 0x33FF0000);
	m_GCText = GeomControl(RENDERCTRLTYPE::RENDER_TEXT, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0xFFFFFFFF);

	// Initialize the complete interaction state even for malformed wire items;
	// this keeps destruction safe without consulting an invalid catalog row.
	m_pItem = pItem;
	m_pGridControl = pParent;
	m_bSelect = 0;
	m_fTimer = 1.0f;
	m_nCellIndexX = 0;
	m_nCellIndexY = 0;
	m_nCellWidth = 1;
	m_nCellHeight = 1;
	m_GCObj.n3DObjIndex = -1;

	if (m_pItem && WYD748_IsValidItemIndex(pItem->sIndex))
	{
		const auto& itemDef = g_pItemList[pItem->sIndex];
		if (itemDef.nIndexMesh < 0)
		{
			// FUN_0040d13e keeps catalog rows without a mesh in the native 2D item
			// path. Skill books (5000..5095) use the UI skill atlas instead of a
			// translated model index, so they must never reach BASE_GetMeshIndex.
			m_GCObj.eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
			m_GCObj.nTextureSetIndex = g_UIVer == 2 ? 199 : 1;
			m_GCObj.nTextureIndex = itemDef.nIndexTexture;
			m_GCObj.n3DObjIndex = -1;
		}
		else
		{
			// FUN_0040d13e resolves mesh-backed rows through FUN_0040cea0. The
			// source-side equivalent is BASE_GetMeshIndex; using nIndexMesh raw
			// produced generic spheres/rocks in the active client.
			m_GCObj.n3DObjIndex = BASE_GetMeshIndex(pItem->sIndex);
			m_GCObj.nTextureSetIndex = grid_item_presentation::IconTexture(
				pItem->sIndex, itemDef.nIndexTexture);
		}
		m_GCText.strString[0] = 0;
		m_GCText.pFont = &m_Font;

		int nSizeIndex = BASE_GetItemAbility(pItem, 33);
		int nType = BASE_GetItemAbility(pItem, 38);
		if (itemDef.nIndexMesh >= 0)
			m_GCObj.dwBGColor = native_inventory_glow::Background(
				pItem->sIndex, nType, pItem->stEffect[0].cEffect);
		if (nSizeIndex > 7 || nSizeIndex < 0)
			nSizeIndex = 0;

		m_nCellWidth = g_pItemGridXY[nSizeIndex][0];
		m_nCellHeight = g_pItemGridXY[nSizeIndex][1];
		// FUN_0040d13e uses a distinct UI2 box for image-backed catalog rows:
		// skill books 5000..5102 are 23 logical pixels per cell and the remaining
		// sprites are 32. Meshes and the legacy UI keep the historical 24-pixel
		// item box. The parent grid still owns viewport coordinate conversion.
		float itemBoxSize = 24.0f;
		if (itemDef.nIndexMesh < 0 && g_UIVer == 2)
			itemBoxSize = pItem->sIndex >= 5000 && pItem->sIndex <= 5102 ? 23.0f : 32.0f;

		// UI mode 2 keeps grids at resource pixel size, so the item box uses the
		// same 2D UI scale as its parent grid.
		m_nWidth = itemBoxSize * static_cast<float>(m_nCellWidth)
			* RenderDevice::m_fUIScaleX;
		m_nHeight = itemBoxSize * static_cast<float>(m_nCellHeight)
			* RenderDevice::m_fUIScaleY;
		m_GCObj.m_fWidth = m_nWidth;
		m_GCObj.m_fHeight = m_nHeight;

		int nAmount = BASE_GetItemAmount(pItem);
		if (pItem->sIndex >= 2330 && pItem->sIndex < 2390)
			nAmount = 0;

		if (nAmount > 0)
		{
			sprintf(m_GCText.strString, "%2d", nAmount);
			m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
		}

		m_GCObj.sLegend = g_pItemList[pItem->sIndex].nGrade;
		int sMultiTexture = BASE_GetItemSanc(pItem);

		if (sMultiTexture > 0 && BASE_GetItemAbility(pItem, 17) > 0)
		{
			sprintf(m_GCText.strString, "+%d", sMultiTexture);
			if (sMultiTexture < 1 || sMultiTexture > 9)
				m_GCText.pFont->SetText(m_GCText.strString, 0xFFFFFF55, 0);
			else
				m_GCText.pFont->SetText(m_GCText.strString, 0xFFFFFFFF, 0);
		}
		if (sMultiTexture > 12)
			sMultiTexture = 12;

		// Native FUN_0040d13e stores the refinement texture in the 3D control's
		// nTextureIndex. RenderForUI consumes that field for refinement and dye
		// overlays; sSanc alone only preserves the source-side label state.
		if (itemDef.nIndexMesh >= 0)
			m_GCObj.nTextureIndex = sMultiTexture;

		if ((g_pItemList[pItem->sIndex].nUnique == 51 || m_GCObj.sLegend) && m_GCObj.sLegend <= 4 && sMultiTexture > 9)
			m_GCObj.sLegend = (unsigned char)BASE_GetItemTenColor(pItem) + 4;

		if (sMultiTexture > 9)
			sMultiTexture = 9;

		if (BASE_GetItemAbility(pItem, 17) > 0)
			m_GCObj.sSanc = sMultiTexture;

		switch (BASE_GetItemColorEffect(pItem))
		{
		case 116:
			m_GCObj.sLegend = 116;
			break;
		case 117:
			m_GCObj.sLegend = 117;
			break;
		case 118:
			m_GCObj.sLegend = 118;
			break;
		case 119:
			m_GCObj.sLegend = 119;
			break;
		case 120:
			m_GCObj.sLegend = 120;
			break;
		case 121:
			m_GCObj.sLegend = 121;
			break;
		case 122:
			m_GCObj.sLegend = 122;
			break;
		case 123:
			m_GCObj.sLegend = 123;
			break;
		case 124:
			m_GCObj.sLegend = 124;
			break;
		case 125:
			m_GCObj.sLegend = 125;
			break;
		}
		if (pItem->sIndex >= 2330 && pItem->sIndex < 2390)
			m_GCObj.sLegend = 0;
	}
}

SGridControlItem::~SGridControlItem()
{
	// Render controls belong to the current scene, but the item allocation always
	// belongs to this grid object and must be released even for an invalid index.
	auto pControlContainer =
		g_pCurrentScene != nullptr ? g_pCurrentScene->m_pControlContainer : nullptr;
	// Text can belong to sprites too, and clearing its caption does not detach it.
	if (pControlContainer && m_GCText.nLayer >= 0)
	{
		RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCText, m_GCText.nLayer);
		m_GCText.nLayer = -1;
	}
	if (m_pItem && WYD748_IsValidItemIndex(m_pItem->sIndex))
	{
		if (g_pItemList[m_pItem->sIndex].nIndexMesh < 0)
		{
			if (pControlContainer && m_GCObj.nLayer >= 0)
			{
				RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCObj, m_GCObj.nLayer);
				// The base S3DObj destructor runs next; mark this native 7.48 node as
				// detached so it cannot attempt a second removal from the draw list.
				m_GCObj.nLayer = -1;
			}
		}
		if (pControlContainer && m_GCEnable.nLayer >= 0)
			RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCEnable, m_GCEnable.nLayer);

	}

	// S3DObj owns m_GCObj and releases it in its base destructor.  Keeping that
	// ownership in one place avoids removing the same 7.48 render node twice.
	delete m_pItem;
	m_pItem = nullptr;
}

void SGridControlItem::SelectThis(int bSelect)
{
	m_bSelect = bSelect;
}

int SGridControlItem::IsSelect()
{
	return m_bSelect;
}

SGridControl* SGridControlItem::GetGridControl()
{
	return m_pGridControl;
}

void SGridControlItem::SetGridControl(SGridControl* pGridControl)
{
	m_pGridControl = pGridControl;
}

STRUCT_ITEM* SGridControlItem::GetItem()
{
	return m_pItem;
}

void SGridControlItem::FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag)
{
	// Only catalog-backed 7.48 items may enter the render list; this prevents a
	// malformed shop row from becoming an out-of-bounds mesh lookup.
	if (m_pItem && WYD748_IsValidItemIndex(m_pItem->sIndex))
	{
		const bool isSprite = g_pItemList[m_pItem->sIndex].nIndexMesh < 0;
		// FUN_0040dd00 leaves native sprites at the item-box origin. Only meshes
		// convert that origin to the renderer centre by adding half their footprint.
		m_GCObj.nPosX = ivItemPos.x + m_nPosX + (isSprite ? 0.0f : m_nWidth * 0.5f);
		m_GCObj.nPosY = ivItemPos.y + m_nPosY + (isSprite ? 0.0f : m_nHeight * 0.5f);
		m_GCObj.nWidth = m_nWidth;
		m_GCObj.nHeight = m_nHeight;
		m_GCObj.nLayer = inParentLayer;
		AddRenderControlItem(pDrawList, &m_GCObj, inParentLayer);
		grid_item_presentation::QueueLabel(m_GCText,
			ivItemPos.x + m_nPosX + m_nWidth * 0.5f,
			ivItemPos.y + m_nPosY + m_nHeight * 0.5f, inParentLayer,
			[&](GeomControl* node, int layer) { AddRenderControlItem(pDrawList, node, layer); });
	}
}

void SGridControlItem::FrameMoveAtCenter748(stGeomList* pDrawList, TMVector2 ivItemCenter, int inParentLayer)
{
	// Trade/mix controls already provide the final visual centre and must bypass
	// FrameMove2's vertical origin conversion. This helper changes no mesh offset;
	// it only preserves the scale selected for those native centered receptacles.
	if (!m_pItem || !WYD748_IsValidItemIndex(m_pItem->sIndex))
		return;

	const bool isSprite = g_pItemList[m_pItem->sIndex].nIndexMesh < 0;
	// The parent supplied a final visual centre. A 2D control is origin-based,
	// while a 3D control consumes that centre directly.
	m_GCObj.nPosX = ivItemCenter.x + m_nPosX - (isSprite ? m_nWidth * 0.5f : 0.0f);
	m_GCObj.nPosY = ivItemCenter.y + m_nPosY - (isSprite ? m_nHeight * 0.5f : 0.0f);
	m_GCObj.nWidth = m_nWidth;
	m_GCObj.nHeight = m_nHeight;
	m_GCObj.nLayer = inParentLayer;
	AddRenderControlItem(pDrawList, &m_GCObj, inParentLayer);
	grid_item_presentation::QueueLabel(m_GCText,
		ivItemCenter.x + m_nPosX, ivItemCenter.y + m_nPosY, inParentLayer,
		[&](GeomControl* node, int layer) { AddRenderControlItem(pDrawList, node, layer); });
}

int SGridControlItem::PtInItem(int inPosX, int inPosY)
{
	return inPosX >= m_nCellIndexX
		&& inPosY >= m_nCellIndexY
		&& inPosX < m_nCellWidth + m_nCellIndexX
		&& inPosY < m_nCellHeight + m_nCellIndexY;
}

int SGridControlItem::PtAtItem(int inPosX, int inPosY)
{
	return inPosX == m_nCellIndexX && inPosY == m_nCellIndexY;
}
