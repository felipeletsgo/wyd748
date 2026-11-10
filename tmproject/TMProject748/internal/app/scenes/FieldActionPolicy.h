#pragma once

#include "../../game/entities/EntitySafetyPolicy.h"

// Scene-independent decisions of the field action handlers (combat, skills,
// inventory, movement). No Win32, Direct3D or scene dependency, so the lane
// tests exercise each decision directly.

namespace field_time
{
// Server time is a 32-bit tick that wraps after 49.7 days. Gates written as
// "now < stamp + interval" stay closed after the wrap when the stamp was taken
// before it (A03-04, A02-11 family). Compare the elapsed time instead. The
// backward-resync tolerance of human_timer applies: a stamp slightly ahead of
// now has not elapsed. intervalMs must be positive.

// now < stamp + intervalMs
constexpr bool Before(unsigned int now, unsigned int stamp, unsigned int intervalMs)
{
    return !human_timer::Elapsed(now, stamp, intervalMs - 1);
}

// now <= stamp + intervalMs
constexpr bool NotAfter(unsigned int now, unsigned int stamp, unsigned int intervalMs)
{
    return !human_timer::Elapsed(now, stamp, intervalMs);
}

// now > stamp + intervalMs
constexpr bool After(unsigned int now, unsigned int stamp, unsigned int intervalMs)
{
    return human_timer::Elapsed(now, stamp, intervalMs);
}
}

namespace field_action
{
// A05-02: the belt stores raw 7.48 short-skill bytes. 255 (char -1) is an
// empty slot; 0..104 are catalog skills; 105..152 are newer-base indices that
// the client shifts by 95. Every other byte wrapped to an unrelated skill
// (255 + 95 became Huntress skill 94). Returns -1 for no cast.
constexpr int kShortSkillSlots = 20;

constexpr int ResolveShortSkill(unsigned char raw)
{
    if (raw <= 104)
        return raw;
    if (raw <= 152)
        return raw + 95;
    return -1;
}

constexpr bool ValidShortSkillSlot(int slot)
{
    return slot >= 0 && slot < kShortSkillSlots;
}

constexpr bool CanPrioritizeGroundPickup(int itemIndex, int groundCells)
{
    // Ground fixtures have their own interaction. The cannon uses skill 97.
    return itemIndex > 0 && groundCells <= 0 && itemIndex != 746;
}

// A03-06: the punch-reaction delay table holds six motions (4..9) per class.
// The server relays the client's Motion byte, so clamp both ends.
constexpr int kPunchMotionsPerClass = 6;
constexpr int kPunchDelayClasses = 6;

constexpr int PunchDelayIndex(int nClass, unsigned char motion)
{
    const int cls = nClass < 0 ? 0 : (nClass >= kPunchDelayClasses ? kPunchDelayClasses - 1 : nClass);
    const int clamped = motion < 4 ? 4 : (motion > 9 ? 9 : motion);
    return kPunchMotionsPerClass * cls + (clamped - 4);
}

// A03-10: Rapid Hit (95) shows the number of target i 100 ms after target
// i - 1 (DOCS/current-state.md). The old loop accumulated the offset.
constexpr unsigned int kRapidHitSkill = 95;
constexpr unsigned int kRapidHitStepMs = 100;

constexpr unsigned int HitNumberDelay(unsigned int baseDelay, int skillIndex, int targetIndex)
{
    return skillIndex == static_cast<int>(kRapidHitSkill) && targetIndex > 0
        ? baseDelay + kRapidHitStepMs * static_cast<unsigned int>(targetIndex)
        : baseDelay;
}

// A03-10: the Kefra arena push stays inside the arena rectangle. The old
// code clamped the Y underflow into the X coordinate.
struct Cell
{
    int x;
    int y;
};

constexpr Cell ClampKefraPush(int x, int y)
{
    return Cell{x < 2341 ? 2341 : (x > 2391 ? 2391 : x),
        y < 3907 ? 3907 : (y > 3952 ? 3952 : y)};
}

// A03-03: an attack effect event keeps a raw target pointer for 200-500 ms.
// A delay-deleted or deleted actor can be freed inside that window, so such
// an actor is never bound; the event keeps its target position instead.
constexpr bool MayBindEffectTarget(unsigned int delayDeleteTime, int delayDeleted, int deleted)
{
    return delayDeleteTime == 0 && delayDeleted == 0 && deleted == 0;
}

// A05-03: a support (non-aggressive) skill needs a player, party member or
// summon. IDs 1000 and above are monsters and NPCs; ID 0 is not an actor.
constexpr bool IsNonPlayerTarget(unsigned int id, bool party, bool summon)
{
    return (id == 0 || id >= 1000) && !party && !summon;
}

// A05-04: a completed flight leaves state -1 (no pending phase). Only an
// active flight or a positive phase blocks auto-run.
constexpr bool AirMoveBlocksAutoRun(bool airMove, int state)
{
    return airMove || state > 0;
}

// A05-07: a held right button re-sends the cast every frame. Automatic (C.C.)
// casts report at most every 3 s (cc_mode::ManaWarningIntervalMs); manual
// casts report at most once per second instead of once per frame.
constexpr unsigned int kAutomaticCastWarningMs = 3000;
constexpr unsigned int kManualCastWarningMs = 1000;

constexpr bool ShouldReportCastFailure(bool automatic, bool reported,
    unsigned int now, unsigned int last)
{
    return !reported ||
        now - last >= (automatic ? kAutomaticCastWarningMs : kManualCastWarningMs);
}

// A04-09: EF_GRID selects one of eight item footprints (g_pItemGridXY[8]).
// SGridControlItem treats every other value as footprint 0.
constexpr int kItemGridShapes = 8;

constexpr int ItemGridShape(int effectValue)
{
    return effectValue < 0 || effectValue >= kItemGridShapes ? 0 : effectValue;
}

// A04-05: a shop row names an item only inside the loaded catalog.
constexpr bool IsCatalogItem(int itemIndex, int catalogSize)
{
    return itemIndex > 0 && itemIndex < catalogSize;
}
}

namespace grid_item_release
{
// The 7.48 UI keeps four process-wide aliases of a grid item: hover, the last
// drag source, the sell/delete confirmation item and the cursor attachment.
// Clear every alias before an item is deleted so a pending prompt cannot read
// freed memory (A04-01, A05-11). SGridControl::Empty applies the same rule.
template <class Grid, class Item, class Cursor>
void ClearAliases(const Item* item, Cursor* cursor)
{
    if (!item)
        return;
    if (Grid::m_pLastMouseOverItem == item)
        Grid::m_pLastMouseOverItem = nullptr;
    if (Grid::m_pLastAttachedItem == item)
        Grid::m_pLastAttachedItem = nullptr;
    if (Grid::m_pSellItem == item)
        Grid::m_pSellItem = nullptr;
    if (cursor && cursor->m_pAttachedItem == item)
        cursor->m_pAttachedItem = nullptr;
}

template <class Grid, class Item, class Cursor>
void Release(Item*& item, Cursor* cursor)
{
    ClearAliases<Grid>(item, cursor);
    delete item;
    item = nullptr;
}
}
