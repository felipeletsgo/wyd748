#pragma once

#include "SGrid.h"
#include "../wire/QuotedSalePacket.h"

class TMFieldScene;
int WYD748_MerchantSalePrice(TMFieldScene* scene, const STRUCT_ITEM& item, unsigned int merchantID);
bool WYD748_BuildQuotedSale(TMFieldScene* scene, const SGridControlItem* item,
    unsigned int merchantID, int place, int slot, MSG_QuotedSale& packet);

// Grid helpers shared by the SGrid*.cpp units. Defined in SGrid.cpp; private
// to the grid control, not a general UI utility.
bool WYD748_ReplaceGridVisual(SGridControl* grid, SGridControlItem* replacement,
	int x, int y, SGridControlItem*& previous);
bool WYD748_IsValidItemIndex(const short itemIndex);
bool WYD748_FormatSellConfirmation(char (&message)[128], const SGridControlItem* item);
int WYD748_ResolveWireSlot(TMFieldScene* scene, const SGridControl* grid,
	short wireType, int cellX, int cellY);
// Wire endpoint of an item held by a grid: the equipment slot of its item type,
// otherwise the Carry/Cargo slot of its cell. Returns false when the item's grid
// has no wire slot (skill, trade, shop and mix presentation grids).
bool WYD748_ResolveItemEndpoint(TMFieldScene* scene, SGridControlItem* item,
	short& wireType, short& wirePos);
bool WYD748_IsCenteredSingleCellGrid(const TMEGRIDTYPE gridType);
float WYD748_GetCursorMeshScale(const SGridControlItem* item);
void WYD748_ApplyGridMeshScale(SGridControlItem* item);
