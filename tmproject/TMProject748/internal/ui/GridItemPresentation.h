#pragma once

namespace grid_item_presentation {
inline const char* SplitCaption(const char* resource)
{
    if (resource) {
        for (const char* p = resource; *p; ++p)
            if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
                return resource;
    }
    return "Enter the quantity to split.";
}

// The item visual and its text are separate draw-list nodes in 7.48.
// The caller supplies a final centre for both sprite and mesh footprints.
template<class Node, class Queue>
inline void QueueLabel(Node& label, float x, float y, int layer, Queue queue)
{
    if (!label.strString[0])
        return;
    label.nPosX = static_cast<decltype(label.nPosX)>(static_cast<int>(x));
    label.nPosY = static_cast<decltype(label.nPosY)>(static_cast<int>(y));
    label.nLayer = layer;
    queue(&label, layer);
}
}
