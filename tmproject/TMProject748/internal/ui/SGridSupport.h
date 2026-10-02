#pragma once

#include "SGrid.h"

class TMFieldScene;

// Grid helpers shared by the SGrid*.cpp units. Defined in SGrid.cpp; private
// to the grid control, not a general UI utility.
bool WYD748_ReplaceGridVisual(SGridControl* grid, SGridControlItem* replacement,
	int x, int y, SGridControlItem*& previous);
bool WYD748_IsValidItemIndex(const short itemIndex);
bool WYD748_FormatSellConfirmation(char (&message)[128], const SGridControlItem* item);
int WYD748_ResolveWireSlot(TMFieldScene* scene, const SGridControl* grid,
	short wireType, int cellX, int cellY);
bool WYD748_IsCenteredSingleCellGrid(const TMEGRIDTYPE gridType);
float WYD748_GetGridMeshScale(const SGridControlItem* item);
void WYD748_ApplyGridMeshScale(SGridControlItem* item, const bool fitSingleCell);
