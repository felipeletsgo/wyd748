#pragma once

// TMObjectContainer::Load publishes the Kefra hall portal nodes to borrowed
// TMFieldScene slots. The container owns those nodes through its subtree, so
// it must clear each slot that still names one of them before the subtree is
// deleted (A15-03). Only pointer values are compared; no slot is dereferenced.
namespace hall_portal_links
{
template<class Slot, class Node>
bool ClearIfPublished(Slot*& sceneSlot, Node* published)
{
    if (!published || sceneSlot != published)
        return false;
    sceneSlot = nullptr;
    return true;
}
}
