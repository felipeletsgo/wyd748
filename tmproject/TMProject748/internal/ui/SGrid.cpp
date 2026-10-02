#include "pch.h"
#include "SGrid.h"
#include "GridInsertion.h"
#include "../application/NativeVolatileRoutes.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMMesh.h"
#include "TMFieldScene.h"
#include "TMUtil.h"
#include "ItemEffect.h"
#include "SellConfirmationText.h"
#include "NativeSaleQuote.h"
#include "SGridSupport.h"


static_assert(EF_VOLATILE == native_sale_quote::VolatileEffect,
	"Native sale quote requires ability type 38");

// Local shortcut replacement: the caller retains the new visual if AddItem
// rejects it. Remove the old visual only after insertion succeeds; the caller
// destroys it after detaching the cursor, which may still point to it.
bool WYD748_ReplaceGridVisual(SGridControl* grid, SGridControlItem* replacement,
	int x, int y, SGridControlItem*& previous)
{
	previous = nullptr;
	if (grid->AddItem(replacement, x, y) != 1)
		return false;
	// Search like PickupItem, without GetItem's hover effect or selecting
	// the replacement just appended at the end of the list.
	for (int i = 0; i < grid->m_nNumItem - 1; ++i)
	{
		if (grid->m_pItemList[i] && grid->m_pItemList[i]->PtInItem(x, y) == 1)
		{
			previous = grid->PickupItem(x, y);
			break;
		}
	}
	if (previous)
	{
		// Pickup clears shared occupancy. AddItem already accepted the new
		// geometry, so restore only the replacement's footprint.
		grid_occupancy::FillClipped(grid->m_pbFilled, grid->m_nColumnGridCount,
			grid->m_nRowGridCount, x, y, replacement->m_nCellWidth,
			replacement->m_nCellHeight, 1);
	}
	return true;
}

bool WYD748_IsValidItemIndex(const short itemIndex)
{
	// ItemList.bin 7.48 contains exactly MAX_ITEMLIST rows.  Rejecting newer
	// TMProject indices prevents shop/inventory code from reading past it.
	return itemIndex >= 0 && itemIndex < MAX_ITEMLIST;
}

bool WYD748_FormatSellConfirmation(char (&message)[128], const SGridControlItem* item)
{
	if (!item || !item->m_pItem || !WYD748_IsValidItemIndex(item->m_pItem->sIndex))
		return false;

	return wyd748::ui::FormatSellConfirmation(message, sizeof(message),
		g_pMessageStringTable[342], sizeof(g_pMessageStringTable[342]),
		g_pItemList[item->m_pItem->sIndex].Name,
		sizeof(g_pItemList[0].Name));
}

int WYD748_ResolveWireSlot(TMFieldScene* scene, const SGridControl* grid,
	short wireType, int cellX, int cellY)
{
	// Central 7.48 interaction projection: Carry and Cargo are nine-column
	// linear aggregates on the native wire.  Non-aggregate feature grids keep
	// their own column count, so trade/shop/mix layouts are not accidentally
	// rewritten while removing every duplicated 7.59 inventory-page formula.
	if (scene && wireType == 1)
		return scene->GetCarrySlotForCell(grid, cellX, cellY);
	if (scene && wireType == 2)
		return scene->GetCargoSlotForCell(grid, cellX, cellY);
	return cellX + (grid ? grid->m_nColumnGridCount : 5) * cellY;
}

bool WYD748_IsCenteredSingleCellGrid(const TMEGRIDTYPE gridType)
{
	// FUN_0040fc3e treats trade/mix result controls as one visual receptacle,
	// even though their packet position is still a single logical cell.
	return gridType == TMEGRIDTYPE::GRID_TRADENONE
		|| gridType == TMEGRIDTYPE::GRID_TRADEOP
		|| gridType == TMEGRIDTYPE::GRID_TRADEMY
		|| gridType == TMEGRIDTYPE::GRID_TRADEMY2
		|| gridType == TMEGRIDTYPE::GRID_ITEMMIX
		|| gridType == TMEGRIDTYPE::GRID_ITEMMIX4
		|| gridType == TMEGRIDTYPE::GRID_ITEMMIXRESULT
		|| gridType == TMEGRIDTYPE::GRID_ITEMMIXNEED
		|| gridType == TMEGRIDTYPE::GRID_MISSION_RESULT
		|| gridType == TMEGRIDTYPE::GRID_MISSION_NEED
		|| gridType == TMEGRIDTYPE::GRID_MISSION_NEEDLIST;
}

float WYD748_GetGridMeshScale(const SGridControlItem* item)
{
	if (!item || !g_pMeshManager || item->m_GCObj.n3DObjIndex < 0)
		return 1.0f;

	TMMesh* mesh = g_pMeshManager->GetCommonMesh(item->m_GCObj.n3DObjIndex, 0, 180000);
	if (!mesh)
		return 1.0f;

	// Native FUN_0040e817 compares only MaxZ against 0.3 mesh units for each
	// occupied cell row.  It shrinks meshes that exceed that vertical budget
	// and never enlarges a mesh beyond its authored 1.0 scale.
	constexpr float NativeCellHeight = 0.3f;
	if (!std::isfinite(mesh->m_fMaxZ) || mesh->m_fMaxZ <= 0.0f
		|| item->m_nCellHeight <= 0)
		return 1.0f;

	const float targetHeight = static_cast<float>(item->m_nCellHeight) * NativeCellHeight;
	return mesh->m_fMaxZ > targetHeight ? targetHeight / mesh->m_fMaxZ : 1.0f;
}

void WYD748_ApplyGridMeshScale(SGridControlItem* item, const bool fitSingleCell)
{
	if (!item)
		return;

	// Equipment panels are irregular body slots and retain their stock 1.0
	// presentation.  Every cell-based grid and the cursor use the same fit.
	item->m_GCObj.fScale = fitSingleCell
		? WYD748_GetGridMeshScale(item)
		: 1.0f;
}

SGridControlItem* SGridControl::m_pLastMouseOverItem;
SGridControlItem* SGridControl::m_pLastAttachedItem;
SGridControlItem* SGridControl::m_pSellItem;
int SGridControl::m_bNeedUpdate = 1;
char* SGridControl::m_szParamString[49] = {
	g_pMessageStringTable[73],
	g_pMessageStringTable[74],
	g_pMessageStringTable[75],
	g_pMessageStringTable[76],
	g_pMessageStringTable[77],
	g_pMessageStringTable[78],
	g_pMessageStringTable[79],
	g_pMessageStringTable[80],
	g_pMessageStringTable[81],
	g_pMessageStringTable[82],
	g_pMessageStringTable[83],
	g_pMessageStringTable[84],
	g_pMessageStringTable[85],
	g_pMessageStringTable[86],
	g_pMessageStringTable[87],
	g_pMessageStringTable[88],
	g_pMessageStringTable[89],
	g_pMessageStringTable[174],
	g_pMessageStringTable[90],
	g_pMessageStringTable[91],
	g_pMessageStringTable[92],
	g_pMessageStringTable[93],
	g_pMessageStringTable[94],
	g_pMessageStringTable[95],
	g_pMessageStringTable[96],
	g_pMessageStringTable[97],
	g_pMessageStringTable[98],
	g_pMessageStringTable[99],
	g_pMessageStringTable[140],
	g_pMessageStringTable[100],
	g_pMessageStringTable[101],
	g_pMessageStringTable[102],
	g_pMessageStringTable[103],
	g_pMessageStringTable[104],
	g_pMessageStringTable[105],
	g_pMessageStringTable[128],
	g_pMessageStringTable[129],
	g_pMessageStringTable[130],
	g_pMessageStringTable[80],
	g_pMessageStringTable[79],
	g_pMessageStringTable[104],
	g_pMessageStringTable[79],
	g_pMessageStringTable[162],
	g_pMessageStringTable[163],
	g_pMessageStringTable[164],
	g_pMessageStringTable[165],
	g_pMessageStringTable[166],
	g_pMessageStringTable[169],
	g_pMessageStringTable[171]
};

SGridControl::SGridControl(unsigned int inTextureSetIndex, int inRowGridCount, int inColumnGridCount, float inX, float inY, float inWidth, float inHeight, TMEITEMTYPE type)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	// WYD 7.48 serializes grids as control type 16 in FieldScene2.bin.  SPanel's
	// constructor labels the object as a panel, so restore the concrete type here;
	// otherwise diagnostics and event routing mistake every native grid for a
	// passive panel even though TMScene::ReadRCBin constructed SGridControl.
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_GRID;
	m_eItemType = type;
	m_nRowGridCount = inRowGridCount;
	m_nColumnGridCount = inColumnGridCount;
	m_bDrawGrid = 0;
	m_nNumItem = 0;
	m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_dwMerchantID = 0;
	m_dwLastBuyTime = 0;
	m_GCEnable = GeomControl(RENDERCTRLTYPE::RENDER_IMAGE_STRETCH, -2, 0.0f, 0.0f, inWidth, inHeight, 0, 0xFFFF0000);
	m_dwEnableColor = 0;

	m_vecPickupedPos = IVector2(0, 0);
	m_vecPickupedSize = IVector2(0, 0);

	m_pbFilled = new int[m_nColumnGridCount * m_nRowGridCount];
	m_GCGrid = new GeomControl[inColumnGridCount * inRowGridCount];

	if (m_GCGrid)
	{
		for (int i = 0; i < inColumnGridCount * inRowGridCount; ++i)
		{
			m_GCGrid[i].nTextureSetIndex = -489;
			m_GCGrid[i].nTextureIndex = i;
			m_GCGrid[i].nWidth = inWidth / (float)m_nColumnGridCount;
			m_GCGrid[i].nHeight = inHeight / (float)m_nRowGridCount;
			m_GCGrid[i].dwColor = -1;
			m_GCGrid[i].eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
		}
	}

	memset(m_pbFilled, 0, m_nColumnGridCount * sizeof(int) * m_nRowGridCount);
	memset(m_pItemList, 0, sizeof(m_pItemList));
	m_nTradeMoney = 0;
	m_dwLastSortTime = 0;
}

SGridControl::~SGridControl()
{
	Empty();

	// A grid can outlive its field scene during disconnect or scene changes;
	// release render nodes only while the owning 7.48 container still exists.
	auto pControlContainer =
		g_pCurrentScene != nullptr ? g_pCurrentScene->m_pControlContainer : nullptr;
	if (pControlContainer)
	{
		if (m_bDrawGrid == 1)
		{
			for (int nY = 0; nY < m_nRowGridCount; ++nY)
			{
				for (int nX = 0; nX < m_nColumnGridCount; ++nX)
				{
					if (!m_pbFilled[nX + m_nColumnGridCount * nY]
						&& m_GCGrid[nX + m_nColumnGridCount * nY].nLayer >= 0)
					{
						RemoveRenderControlItem(
							pControlContainer->m_pDrawControl,
							&m_GCGrid[nX + m_nColumnGridCount * nY],
							m_GCGrid[nX + m_nColumnGridCount * nY].nLayer);
					}
				}
			}
		}
		if (m_GCEnable.nLayer >= 0)
			RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCEnable, m_GCEnable.nLayer);
	}

	SAFE_DELETE_ARRAY(m_pbFilled);
	SAFE_DELETE_ARRAY(m_GCGrid);
}

void SGridControl::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	m_GCPanel.nPosX = ivParentPos.x + m_nPosX;
	m_GCPanel.nPosY = ivParentPos.y + m_nPosY;
	if (m_GCPanel.nTextureSetIndex >= 0)
		SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);

	if (m_bDrawGrid == 1)
	{
		for (int nY = 0; nY < m_nRowGridCount; ++nY)
		{
			for (int nX = 0; nX < m_nColumnGridCount; ++nX)
			{
				if (m_pbFilled[nX + m_nColumnGridCount * nY])
					continue;

				m_GCGrid[nX + m_nColumnGridCount * nY].nPosX = (float)(ivParentPos.x + m_nPosX)
					+ (float)((float)nX
						* m_GCGrid[nX + m_nColumnGridCount * nY].nWidth);
				m_GCGrid[nX + m_nColumnGridCount * nY].nPosY = (float)(ivParentPos.y + m_nPosY)
					+ (float)((float)nY
						* m_GCGrid[nX + m_nColumnGridCount * nY].nHeight);
				m_GCGrid[nX + m_nColumnGridCount * nY].nLayer = inParentLayer;
				AddRenderControlItem(pDrawList, &m_GCGrid[nX + m_nColumnGridCount * nY], inParentLayer);
			}
		}
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pGridCurrent = m_pItemList[i];
			if (!pGridCurrent)
				continue;

			TMVector2 vecPos = TMVector2((ivParentPos.x + m_nPosX) + (((float)pGridCurrent->m_nCellIndexX * m_nWidth) / (float)m_nColumnGridCount),
				(ivParentPos.y + m_nPosY) + (((float)pGridCurrent->m_nCellIndexY * m_nHeight) / (float)m_nRowGridCount));

			pGridCurrent->FrameMove2(pDrawList, vecPos, inParentLayer, 0);

			if (pGridCurrent->m_fTimer <= 0.0f || pGridCurrent->m_fTimer >= 1.0f)
				continue;

			pGridCurrent->m_GCEnable.nPosX = ivParentPos.x + m_nPosX + (float)((float)pGridCurrent->m_nCellIndexX * BASE_ScreenResize(m_GCGrid->nWidth));
			pGridCurrent->m_GCEnable.nPosY = ivParentPos.y + m_nPosY + (float)((float)pGridCurrent->m_nCellIndexY * BASE_ScreenResize(m_GCGrid->nHeight));
			pGridCurrent->m_GCEnable.nWidth = (float)((float)(24 * pGridCurrent->m_nCellWidth)
				* RenderDevice::m_fWidthRatio)
				* (float)(1.0f - pGridCurrent->m_fTimer);

			if (pGridCurrent->m_GCEnable.nWidth > 0.0099999998f && pGridCurrent->m_GCEnable.nWidth < 2.0f)
				pGridCurrent->m_GCEnable.nWidth = 2.0f;

			pGridCurrent->m_GCEnable.nHeight = (float)(24 * pGridCurrent->m_nCellHeight)
				* RenderDevice::m_fHeightRatio;

			pGridCurrent->m_GCEnable.nLayer = inParentLayer;
			pGridCurrent->m_GCEnable.dwColor = 0xAA000000;
			AddRenderControlItem(pDrawList, &pGridCurrent->m_GCEnable, inParentLayer);
		}
		return;
	}
	// These native single-cell panels center their item instead of applying the
	// regular multi-cell grid transform.  The imported source joined mutually
	// exclusive grid kinds with &&, making this 7.48 rendering path unreachable.
	const bool isCenteredSingleCellGrid = WYD748_IsCenteredSingleCellGrid(m_eGridType);
	const bool isEquipmentGrid =
		m_eItemType != TMEITEMTYPE::ITEMTYPE_NONE;
	if (isEquipmentGrid)
	{
		// Equipment controls are irregular panels, not 24-pixel Carry cells.  The
		// native 7.48 path first centers the item's box origin inside that receptacle
		// and then lets the common item frame convert the origin to the renderer
		// centre. Keeping those two stages preserves both axes for every footprint.
		if (m_nNumItem > 0 && m_pItemList[0])
		{
			auto pItem = m_pItemList[0];
			TMVector2 vecItemOrigin = TMVector2(
				(ivParentPos.x + m_nPosX) + ((m_nWidth - pItem->m_nWidth) * 0.5f),
				(ivParentPos.y + m_nPosY) + ((m_nHeight - pItem->m_nHeight) * 0.5f));

			pItem->FrameMove2(pDrawList, vecItemOrigin, inParentLayer, 0);
		}

		if (m_dwEnableColor != 0)
		{
			// The equip target is the whole native body slot.  A one-cell rectangle
			// here produced the small red/blue square seen over armour slots.
			m_GCEnable.nPosX = ivParentPos.x + m_nPosX;
			m_GCEnable.nPosY = ivParentPos.y + m_nPosY;
			m_GCEnable.nWidth = m_nWidth;
			m_GCEnable.nHeight = m_nHeight;
			m_GCEnable.nLayer = inParentLayer;
			m_GCEnable.dwColor = m_dwEnableColor;
			AddRenderControlItem(pDrawList, &m_GCEnable, inParentLayer);
		}
		return;
	}
	if (isCenteredSingleCellGrid)
	{
		if (!m_pItemList[0])
			return;

		// GRID_TRADEMY2 is the buyer's display slot: FieldScene2 already supplies
		// its native receptacle, so the seller-side dark selection mask must not
		// cover the item artwork in the customer window.
		if (m_eGridType == TMEGRIDTYPE::GRID_TRADEOP || m_eGridType == TMEGRIDTYPE::GRID_TRADEMY)
		{
			m_pItemList[0]->m_GCEnable.nPosX = ivParentPos.x + m_nPosX;
			m_pItemList[0]->m_GCEnable.nPosY = ivParentPos.y + m_nPosY;
			m_pItemList[0]->m_GCEnable.nWidth = m_nWidth - 4.0f;
			m_pItemList[0]->m_GCEnable.nHeight = m_nHeight - 3.0f;
			m_pItemList[0]->m_GCEnable.nLayer = inParentLayer;
			m_pItemList[0]->m_GCEnable.dwColor = 0x0FF000000;
			AddRenderControlItem(pDrawList, &m_pItemList[0]->m_GCEnable, inParentLayer);
		}
		// Trade/mix receptacles follow the same native centre contract as equipment;
		// item-local offsets remain available for the few stock mix arrangements.
		TMVector2 vecCenter = TMVector2((ivParentPos.x + m_nPosX) + (m_nWidth * 0.5f),
			(ivParentPos.y + m_nPosY) + (m_nHeight * 0.5f));
		m_pItemList[0]->FrameMoveAtCenter748(pDrawList, vecCenter, inParentLayer);
		return;
	}

	for (int j = 0; j < m_nNumItem; ++j)
	{
		auto pGridCurrent = m_pItemList[j];
		if (!pGridCurrent)
			continue;

		const float cellWidth = m_nWidth / static_cast<float>(m_nColumnGridCount);
		const float cellHeight = m_nHeight / static_cast<float>(m_nRowGridCount);
		// FUN_0040fc3e proves that regular grids derive one origin per occupied
		// cell. FUN_0040dd00 then converts that box origin to the renderer centre by
		// adding half of the item's width and height in the common child frame.
		TMVector2 vecItemOrigin = TMVector2((ivParentPos.x + m_nPosX)
			+ (pGridCurrent->m_nCellIndexX * cellWidth),
			(ivParentPos.y + m_nPosY)
			+ (pGridCurrent->m_nCellIndexY * cellHeight));

		pGridCurrent->FrameMove2(pDrawList, vecItemOrigin, inParentLayer, 0);
		if (pGridCurrent->m_GCObj.dwColor != 0xFFFF0000)
			continue;

		pGridCurrent->m_GCEnable.nPosX = (ivParentPos.x + m_nPosX)
			+ (pGridCurrent->m_nCellIndexX * cellWidth);
		pGridCurrent->m_GCEnable.nPosY = (ivParentPos.y + m_nPosY)
			+ (pGridCurrent->m_nCellIndexY * cellHeight);

		if (m_bDrawGrid)
		{
			// The blocked-item overlay must match the same one-cell rectangle used by
			// rendering and hit-testing, not the unrelated legacy 35-pixel constant.
			pGridCurrent->m_GCEnable.nWidth = cellWidth * pGridCurrent->m_nCellWidth;
			pGridCurrent->m_GCEnable.nHeight = cellHeight * pGridCurrent->m_nCellHeight;
		}
		else
		{
			pGridCurrent->m_GCEnable.nWidth = (float)(SControl::m_nGridCellSize * pGridCurrent->m_nCellWidth);
			pGridCurrent->m_GCEnable.nHeight = (float)(SControl::m_nGridCellSize * pGridCurrent->m_nCellHeight);
		}

		pGridCurrent->m_GCEnable.nLayer = inParentLayer;
		pGridCurrent->m_GCEnable.dwColor = 0x33FF0000;
		AddRenderControlItem(pDrawList, &pGridCurrent->m_GCEnable, inParentLayer);
	}

	if (m_dwEnableColor != 0 &&
		m_vecPickupedPos.x >= 0 && m_vecPickupedPos.x < m_nColumnGridCount &&
		m_vecPickupedPos.y >= 0 && m_vecPickupedPos.y < m_nRowGridCount)
	{
		// MouseOver already applies the native 7.48 placement rules and records
		// blue for a legal target or red for a blocked one. ItemList.bin provides
		// the canonical 1x1 footprint, so the preview covers exactly one cell.
		const float cellWidth = m_nWidth / static_cast<float>(m_nColumnGridCount);
		const float cellHeight = m_nHeight / static_cast<float>(m_nRowGridCount);
		const int visibleWidth = min(m_vecPickupedSize.x, m_nColumnGridCount - m_vecPickupedPos.x);
		const int visibleHeight = min(m_vecPickupedSize.y, m_nRowGridCount - m_vecPickupedPos.y);
		if (visibleWidth > 0 && visibleHeight > 0)
		{
			m_GCEnable.nPosX = ivParentPos.x + m_nPosX + cellWidth * m_vecPickupedPos.x;
			m_GCEnable.nPosY = ivParentPos.y + m_nPosY + cellHeight * m_vecPickupedPos.y;
			m_GCEnable.nWidth = cellWidth * visibleWidth;
			m_GCEnable.nHeight = cellHeight * visibleHeight;
			m_GCEnable.nLayer = inParentLayer;
			m_GCEnable.dwColor = m_dwEnableColor;
			AddRenderControlItem(pDrawList, &m_GCEnable, inParentLayer);
		}
	}
}

int SGridControl::CanItAdd(int* bFilledBuffer, int inCellIndexX, int inCellIndexY, int inCellWidth, int inCellHeight)
{
	// Local hardening of FUN_0040E604: preserve occupancy == 1 for valid
	// rectangles and reject invalid bounds before reading memory.
	return grid_occupancy::CanPlace(bFilledBuffer, m_nColumnGridCount,
		m_nRowGridCount, inCellIndexX, inCellIndexY, inCellWidth, inCellHeight) ? 1 : 0;
}

int SGridControl::AddItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY)
{
	return grid_insertion::Execute(m_pItemList, m_nNumItem, ipNewItem, [&]()
	{

	// Validate before mutation; preserve legacy clipping and overlap behavior.
	if (!grid_occupancy::FillClipped(m_pbFilled, m_nColumnGridCount, m_nRowGridCount,
		inCellIndexX, inCellIndexY, ipNewItem->m_nCellWidth, ipNewItem->m_nCellHeight, 1))
		return 0;

	ipNewItem->SetGridControl(this);
	ipNewItem->m_nCellIndexX = inCellIndexX;
	ipNewItem->m_nCellIndexY = inCellIndexY;
	m_pItemList[m_nNumItem++] = ipNewItem;

	const bool isEquipmentGrid = m_eItemType != TMEITEMTYPE::ITEMTYPE_NONE;
	WYD748_ApplyGridMeshScale(ipNewItem, !isEquipmentGrid);

	return 1;
	});
}

int SGridControl::AddSkillItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY)
{
	return grid_insertion::Execute(m_pItemList, m_nNumItem, ipNewItem, [&]()
	{

	// Validate before mutation; preserve legacy clipping and overlap behavior.
	if (!grid_occupancy::FillClipped(m_pbFilled, m_nColumnGridCount, m_nRowGridCount,
		inCellIndexX, inCellIndexY, ipNewItem->m_nCellWidth, ipNewItem->m_nCellHeight, 1))
		return 0;

	ipNewItem->SetGridControl(this);
	ipNewItem->m_nCellIndexX = inCellIndexX;
	ipNewItem->m_nCellIndexY = inCellIndexY;
	ipNewItem->m_nWidth = 23.0f * RenderDevice::m_fWidthRatio;
	ipNewItem->m_nHeight = 24.0f * RenderDevice::m_fHeightRatio;
	ipNewItem->m_GCObj.m_fWidth = ipNewItem->m_nWidth;
	ipNewItem->m_GCObj.m_fHeight = ipNewItem->m_nHeight;
	m_pItemList[m_nNumItem++] = ipNewItem;

	const bool isEquipmentGrid = m_eItemType != TMEITEMTYPE::ITEMTYPE_NONE;
	WYD748_ApplyGridMeshScale(ipNewItem, !isEquipmentGrid);

	return 1;
	});
}

int SGridControl::SetItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY)
{
	return grid_insertion::Execute(m_pItemList, m_nNumItem, ipNewItem, [&]()
	{

	// Validate before mutation; preserve legacy clipping and overlap behavior.
	if (!grid_occupancy::FillClipped(m_pbFilled, m_nColumnGridCount, m_nRowGridCount,
		inCellIndexX, inCellIndexY, ipNewItem->m_nCellWidth, ipNewItem->m_nCellHeight, 1))
		return 0;

	ipNewItem->SetGridControl(this);
	ipNewItem->m_nCellIndexX = inCellIndexX;
	ipNewItem->m_nCellIndexY = inCellIndexY;
	m_pItemList[m_nNumItem++] = ipNewItem;

	const bool isEquipmentGrid = m_eItemType != TMEITEMTYPE::ITEMTYPE_NONE;
	WYD748_ApplyGridMeshScale(ipNewItem, !isEquipmentGrid);

	return 1;
	});
}

IVector2 SGridControl::AddItemInEmpty(SGridControlItem* ipNewItem)
{
	IVector2 vec{ -1, -1 };
	// Do not read a null item's dimensions or search a full list.
	if (!grid_insertion::CanAppend(m_pItemList, m_nNumItem, ipNewItem))
		return vec;
	// Invalid dimensions must not overflow the search bounds. Do not add an
	// occupancy check: AddItem preserves its legacy insertion policy.
	if (!grid_occupancy::ContainsRectangle(m_nColumnGridCount,
		m_nRowGridCount, 0, 0, ipNewItem->m_nCellWidth, ipNewItem->m_nCellHeight))
		return vec;
	for (int nY = 0; nY <= m_nRowGridCount - ipNewItem->m_nCellHeight; ++nY)
	{
		for (int nX = 0; nX <= m_nColumnGridCount - ipNewItem->m_nCellWidth; ++nX)
		{
			if (AddItem(ipNewItem, nX, nY) == 1)
			{
				vec.x = nX;
				vec.y = nY;				
				return vec;
			}
		}
	}

	return vec;
}

IVector2 SGridControl::CanAddItemInEmpty(int nWidth, int nHeight)
{
	IVector2 vec{ -1, -1 };
	// Validate before subtracting dimensions in the search bounds.
	if (!m_pbFilled || !grid_occupancy::ContainsRectangle(m_nColumnGridCount,
		m_nRowGridCount, 0, 0, nWidth, nHeight))
		return vec;

	for (int nX = 0; nX <= m_nColumnGridCount - nWidth; ++nX)
	{
		for (int nY = 0; nY <= m_nRowGridCount - nHeight; ++nY)
		{
			if (CanItAdd(m_pbFilled, nX, nY, nWidth, nHeight) == 1)
			{
				vec.x = nX;
				vec.y = nY;				
				return vec;
			}
		}
	}

	return vec;
}

int SGridControl::CanChangeItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY, int bOnlyCheck)
{
	// WYD748: slot validation needs the active scene's native Carry/Cargo
	// topology before it emits an automatic equipment swap.
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (IsSkill(ipNewItem->m_pItem->sIndex) == 1)
		return 0;

	auto pMobData = &g_pObjectManager->m_stMobData;
	short sType = CheckType(m_eItemType, m_eGridType);
	short sPos = CheckPos(m_eItemType);

	if (!sType)
	{
		unsigned int nItemType = BASE_GetItemAbility(ipNewItem->m_pItem, 17);
		if (m_dwControlID == 65556)
		{
			if (nItemType == 128)
			{
				return BASE_CanEquip(
					ipNewItem->m_pItem,
					&pMobData->CurrentScore,
					sPos,
					pMobData->Equip[0].sIndex,
					pMobData->Equip,
					g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex,
					pMobData->HasSoulSkill());
			}
			if (nItemType == 64 || nItemType == 192)
			{
				if (pMobData->Equip[6].sIndex <= 0)
				{
					if (BASE_CanEquip(
						ipNewItem->m_pItem,
						&pMobData->CurrentScore,
						6,
						pMobData->Equip[0].sIndex,
						pMobData->Equip,
						g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex,
						pMobData->HasSoulSkill())
						&& !bOnlyCheck)
					{
						short sDestType = CheckType(ipNewItem->m_pGridControl->m_eItemType, ipNewItem->m_pGridControl->m_eGridType);
						short sDestPos = CheckPos(ipNewItem->m_pGridControl->m_eItemType);
						if (sDestPos == -1)
							sDestPos = WYD748_ResolveWireSlot(pFScene, ipNewItem->m_pGridControl,
								sDestType, ipNewItem->m_nCellIndexX, ipNewItem->m_nCellIndexY);

						MSG_SwapItem stSwapItem{};
						stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
						stSwapItem.Header.Type = MSG_SwapItem_Opcode;
						stSwapItem.SourType = 0;
						stSwapItem.SourPos = 6;
						stSwapItem.DestType = static_cast<char>(sDestType);
						stSwapItem.DestPos = static_cast<char>(sDestPos);
						stSwapItem.TargetID = TMFieldScene::m_dwCargoID;
						SendOneMessage((char*)&stSwapItem, 20);
					}

					return 0;
				}

				short sDestType = CheckType(ipNewItem->m_pGridControl->m_eItemType, ipNewItem->m_pGridControl->m_eGridType);
				short sDestPos = CheckPos(ipNewItem->m_pGridControl->m_eItemType);
				if (sDestType == 0 && sDestPos == 6)
					return 0;
			}
		}

		return BASE_CanEquip(
			ipNewItem->m_pItem,
			&pMobData->CurrentScore,
			sPos,
			pMobData->Equip[0].sIndex,
			pMobData->Equip,
			g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex,
			pMobData->HasSoulSkill());
	}
	else if (sType == 1)
		return 1;
	else
	{
		// WYD748: cargo uses the native 9-column topology; this position is
		// later used to clear the simulated source item during placement checks.
		int nPos = pFScene->GetCargoSlotForCell(ipNewItem->m_pGridControl,
			ipNewItem->m_nCellIndexX, ipNewItem->m_nCellIndexY);

		STRUCT_ITEM stCargo[128]{};
		memcpy(stCargo, g_pObjectManager->m_stItemCargo, sizeof(stCargo));
		if (ipNewItem->m_pGridControl->m_eGridType == TMEGRIDTYPE::GRID_CARGO)
			memset(&stCargo[nPos], 0, sizeof(STRUCT_ITEM));

		return 1;
	}

	return 0;
}

SGridControlItem* SGridControl::PickupItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItem = nullptr;
	int nIndex = -1;

	for (int i = 0; i < m_nNumItem; ++i)
	{
		pItem = m_pItemList[i];
		if (pItem && pItem->PtInItem(inCellIndexX, inCellIndexY) == 1)
		{
			nIndex = i;
			break;
		}
		pItem = nullptr;
	}
	if (!pItem)
		return nullptr;

	// Removal uses the same clipping as insertion. Even an inconsistent visual
	// must leave the list without reading outside the occupancy buffer.
	grid_occupancy::FillClipped(m_pbFilled, m_nColumnGridCount, m_nRowGridCount,
		pItem->m_nCellIndexX, pItem->m_nCellIndexY, pItem->m_nCellWidth, pItem->m_nCellHeight, 0);
	if (nIndex != -1 && m_nNumItem > nIndex && nIndex >= 0)
	{
		for (int j = nIndex + 1; j < m_nNumItem; ++j)
			m_pItemList[j - 1] = m_pItemList[j];

		m_pItemList[m_nNumItem - 1] = nullptr;
		m_nNumItem--;
	}

	// A dragged item always uses the cell-safe presentation, including when it
	// originated in an irregular equipment panel. AddItem restores scale 1.0
	// if the item is placed back into equipment.
	WYD748_ApplyGridMeshScale(pItem, true);
	return pItem;
}

SGridControlItem* SGridControl::PickupAtItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItem = nullptr;
	int nIndex = -1;
	for (int i = 0; i < m_nNumItem; ++i)
	{
		pItem = m_pItemList[i];
		if (pItem && pItem->PtAtItem(inCellIndexX, inCellIndexY) == 1)
		{
			nIndex = i;
			break;
		}
		pItem = nullptr;
	}
	if (!pItem)
		return nullptr;

	// Removal uses the same clipping as insertion. Even an inconsistent visual
	// must leave the list without reading outside the occupancy buffer.
	grid_occupancy::FillClipped(m_pbFilled, m_nColumnGridCount, m_nRowGridCount,
		pItem->m_nCellIndexX, pItem->m_nCellIndexY, pItem->m_nCellWidth, pItem->m_nCellHeight, 0);
	if (nIndex != -1 && m_nNumItem > nIndex && nIndex >= 0)
	{
		for (int j = nIndex + 1; j < m_nNumItem; ++j)
			m_pItemList[j - 1] = m_pItemList[j];

		m_pItemList[m_nNumItem - 1] = nullptr;
		m_nNumItem--;
	}

	// PickupAtItem feeds the same cursor path as PickupItem and must not preserve
	// equipment scale while the mesh is hovering over the inventory grid.
	WYD748_ApplyGridMeshScale(pItem, true);
	return pItem;
}

SGridControlItem* SGridControl::PickupItem(SGridControlItem* ipItem)
{
	for (int i = 0; i < this->m_nNumItem; ++i)
	{
		auto pItem = m_pItemList[i];
		if (pItem == ipItem)
			return PickupItem(pItem->m_nCellIndexX, pItem->m_nCellIndexY);
	}

	return nullptr;
}

SGridControlItem* SGridControl::SelectItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItemReturn = nullptr;
	if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADENONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEOP
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX4
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXRESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXNEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_RESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pItem = m_pItemList[i];
			if (m_pItemList[i]->PtInItem(inCellIndexX, inCellIndexY) == 1)
			{
				pItem->SelectThis(1);
				pItemReturn = pItem;
			}
			else
				pItem->SelectThis(0);
		}
	}
	else if (m_pItemList[0])
	{
		m_pItemList[0]->SelectThis(1);
		pItemReturn = m_pItemList[0];
	}

	return pItemReturn;
}

SGridControlItem* SGridControl::GetItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItemReturn = nullptr;
	if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADENONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEOP
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX4
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXRESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXNEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_RESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pItem = m_pItemList[i];
			if (m_pItemList[i]->PtInItem(inCellIndexX, inCellIndexY) == 1)
			{
				pItem->m_bOver = 1;
				pItemReturn = pItem;
			}
			else
				pItem->m_bOver = 0;
		}
	}
	else if (m_pItemList[0])
	{
		m_pItemList[0]->m_bOver = 1;;
		pItemReturn = m_pItemList[0];
	}

	return pItemReturn;
}

SGridControlItem* SGridControl::GetAtItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItemReturn = nullptr;
	if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADENONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEOP
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX4
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXRESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXNEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_RESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pItem = m_pItemList[i];
			if (m_pItemList[i]->PtAtItem(inCellIndexX, inCellIndexY) == 1)
			{
				pItem->m_bOver = 1;
				pItemReturn = pItem;
			}
			else
				pItem->m_bOver = 0;
		}
	}
	else if (m_pItemList[0])
	{
		m_pItemList[0]->m_bOver = 1;;
		pItemReturn = m_pItemList[0];
	}

	return pItemReturn;
}

SGridControlItem* SGridControl::GetItem(int nCount)
{
	return m_pItemList[nCount];
}

void SGridControl::Empty()
{
	memset(m_pbFilled, 0, m_nColumnGridCount * sizeof(int) * m_nRowGridCount);
	for (int i = 0; i < m_nNumItem; ++i)
	{
		SGridControlItem* item = m_pItemList[i];
		if (item == nullptr)
			continue;

		// The native 7.48 UI keeps these as process-wide interaction pointers.
		// Clear every alias before deleting an item so shop close/reopen cannot
		// dispatch hover, sell or attachment events through freed memory.
		if (m_pLastMouseOverItem == item)
			m_pLastMouseOverItem = nullptr;
		if (m_pLastAttachedItem == item)
			m_pLastAttachedItem = nullptr;
		if (m_pSellItem == item)
			m_pSellItem = nullptr;
		if (g_pCursor != nullptr && g_pCursor->m_pAttachedItem == item)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(m_pItemList[i]);
	}

	// Keep the complete fixed-size table deterministic after a shop or inventory
	// closes; stale entries beyond m_nNumItem must never be rediscovered later.
	memset(m_pItemList, 0, sizeof(m_pItemList));
	m_sLastMouseOverIndex = -1;
	m_nNumItem = 0;
}

short SGridControl::CheckType(TMEITEMTYPE eType, TMEGRIDTYPE eGridType)
{
	if (eType != TMEITEMTYPE::ITEMTYPE_NONE)
		return 0;

	if (eGridType == TMEGRIDTYPE::GRID_CARGO)
		return 2;

	return 1;
}

int SGridControl::SetItemOnGrid(STRUCT_ITEM* item, int CellIndexX, int CellIndexY)
{
	if (item == nullptr)
		return false;

	auto setItem = new STRUCT_ITEM();

	if (setItem)
	{
		memcpy(setItem, item, sizeof(STRUCT_ITEM));

		auto pItem = new SGridControlItem(0, setItem, 0.0f, 0.0f);

		if (pItem)
		{
			if (AddItem(pItem, CellIndexX, CellIndexY))
				return true;
			SAFE_DELETE(pItem);
			return false;
		}
		SAFE_DELETE(setItem);
	}

	return false;

}

short SGridControl::CheckPos(TMEITEMTYPE eType)
{
	switch (eType)
	{
	case TMEITEMTYPE::ITEMTYPE_HELM:
		return 1;
	case TMEITEMTYPE::ITEMTYPE_COAT:
		return 2;
	case TMEITEMTYPE::ITEMTYPE_PANTS:
		return 3;
	case TMEITEMTYPE::ITEMTYPE_GLOVES:
		return 4;
	case TMEITEMTYPE::ITEMTYPE_BOOTS:
		return 5;
	case TMEITEMTYPE::ITEMTYPE_RIGHT:
		return 7;
	case TMEITEMTYPE::ITEMTYPE_LEFT:
		return 6;
	case TMEITEMTYPE::ITEMTYPE_RING:
		return 8;
	case TMEITEMTYPE::ITEMTYPE_NECKLACE:
		return 9;
	case TMEITEMTYPE::ITEMTYPE_ORB:
		return 10;
	case TMEITEMTYPE::ITEMTYPE_CABUNCLE:
		return 11;
	case TMEITEMTYPE::ITEMTYPE_GUILD:
		return 12;
	case TMEITEMTYPE::ITEMTYPE_EVENT:
		return 13;
	case TMEITEMTYPE::ITEMTYPE_MOUNT:
		return 14;
	case TMEITEMTYPE::ITEMTYPE_MANTUA:
		return 15;
	case TMEITEMTYPE::ITEMTYPE_NEWSLOT1:
		return 16;
	case TMEITEMTYPE::ITEMTYPE_NEWSLOT2:
		return 17;
		break;
	}

	return -1;
}
