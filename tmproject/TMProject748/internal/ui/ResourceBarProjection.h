#pragma once

namespace resource_ui {
// Progress bars clamp current to their stored maximum. Apply the resolved
// snapshot maximum first, including when a packet retains the previous max.
template <typename Bar>
void Project(Bar* bar, int current, int maximum)
{
    if (!bar) return;
    bar->SetMaxProgress(maximum);
    bar->SetCurrentProgress(current);
}

// Native HP controls draw their panel art even when their progress is zero.
// Hide both the main bar and its translucent overlay at zero HP, then restore
// them together on revival.
template <typename Bar>
void ProjectNativeHpVisual(Bar* bar, int current, int maximum)
{
    if (!bar) return;
    Project(bar, current, maximum);
    bar->SetVisible(current > 0);
}

inline const char* MaximumTextFormat(bool nativeLayout)
{
    // Native HUD has independent numeric cells, not a combined "cur / max".
    return nativeLayout ? "%u" : "/ %u";
}
} // namespace resource_ui
