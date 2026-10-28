#pragma once
#include "Basedef.h"
#include "SGrid.h"
#include <cstdio>

namespace item_bonus_text
{
    // Shared with the native tooltip: preserve its units, refinement scaling
    // and resource labels when presenting catalog item additions in the browser.
    template<class Emit>
    void ForEach(STRUCT_ITEM* item, Emit emit)
    {
        const unsigned int position = BASE_GetItemAbility(item, 17);
        for (int i = 0; i < 49; ++i)
        {
            const int effect = dwEFParam[i];
            const int ability = effect == 45 ? 69 : effect == 46 ? 70 : effect;
            const bool total = ability == 42 || ability == 53 || (position == 32 && ability == 2);
            const int value = total ? BASE_GetItemAbilityNosanc(item, ability) : BASE_GetBonusItemAbilityNosanc(item, ability);
            const int scaled = total ? BASE_GetItemAbility(item, ability) : BASE_GetBonusItemAbility(item, ability);
            if (!scaled) continue;
            const char* label = SGridControl::m_szParamString[i];
            // Resolve mastery labels per item instead of inheriting the class
            // of the last hovered item from the grid's shared label pointers.
            if (i >= 23 && i <= 26)
            {
                const int classMask = BASE_GetItemAbility(item, 18);
                const int masteryLabels[5][4] = {
                    {95, 96, 97, 98}, {106, 107, 108, 109},
                    {106, 110, 111, 112}, {106, 113, 114, 115},
                    {106, 133, 134, 135}
                };
                const int row = classMask == 255 ? 0 :
                    (classMask & 1) ? 1 : (classMask & 2) ? 2 :
                    (classMask & 4) ? 3 : (classMask & 8) ? 4 : 0;
                label = g_pMessageStringTable[masteryLabels[row][i - 23]];
            }
            char text[512]{};
            if (effect == 42)
            {
                if (value == scaled) sprintf_s(text, "%s : %d.%d%%", label, scaled / 10, scaled % 10);
                else sprintf_s(text, "%s : %d.%d%% (%d.%d%%)", label, value / 10, value % 10, scaled / 10, scaled % 10);
            }
            else if (effect == 26 || effect == 60 || effect == 45 || effect == 46 || effect == 68)
            {
                if (value == scaled) sprintf_s(text, "%s : %d%%", label, scaled);
                else sprintf_s(text, "%s : %d%% (%d%%)", label, value, scaled);
            }
            else if ((effect != 73 || position != 32) && (effect != 67 || position == 64 || position == 192) && effect != 63)
            {
                if (value == scaled) sprintf_s(text, "%s : %d", label, scaled);
                else sprintf_s(text, "%s : %d (%d)", label, value, scaled);
            }
            else continue;
            emit(position, effect, value, text);
        }
    }
}
