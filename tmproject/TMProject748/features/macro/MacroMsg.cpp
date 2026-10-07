#include "pch.h"
#include "MacroLevel.h"
#include "MacroFactory.h"
#include "MacroMsg.h"

// Routes a use/move message to the strategy of the water tier.
// posX/posY are world coordinates and Param is the item ID received by the
// macro flow; none of these values carries ownership.
void MacroMsg::onEvent(const int posX, const int posY, const int Param)
{
	int Level = 0;

	// The three ID families belong to the three difficulty tiers. Param 0
	// is the legacy marker for a move without item use.
	if (Param >= 3173 && Param <= 3181)
		Level = 1;

	else if (Param >= 777 && Param <= 785)
		Level = 2;

	else if (Param >= 3182 && Param <= 3190)
		Level = 3;

	if (Param == 0)
		Level = 1;

	if (Level == 0)
		return;

	auto macroLevel = MacroFactory::instance().getMacroLevel(Level);

	if (!macroLevel)
		return;

	// Inside this area the macro moves; outside it, it consumes the item
	// following the rule of the selected level.
	if (posX >= 1954 && posY >= 1764 && posX <= 1976 && posY <= 1773)
		macroLevel->DoMove(posX, posY);
	else
		macroLevel->UseItem(posX, posY, Param);
}
