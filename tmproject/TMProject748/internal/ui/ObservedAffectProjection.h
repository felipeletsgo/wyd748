#pragma once

#include <cstddef>
#include <cstdint>

namespace observed_affect_ui {
constexpr int kCapacity = 32;

inline float CenteredBarX(float screenWidth, float scaledBarWidth)
{
    return (screenWidth - scaledBarWidth) * 0.5f;
}

// Call from the active scene initializer; the compat path returns before the
// modern resource bindings. The factory transfers ownership to the scene tree.
template <typename Panel, std::size_t PartyRows, typename Factory>
void InitializePanels(Panel* (&target)[kCapacity],
    Panel* (&party)[PartyRows][kCapacity], Factory create)
{
    for (auto& panel : target) {
        panel = create();
        panel->SetVisible(0);
    }
    for (auto& row : party) {
        for (auto& panel : row) {
            panel = create();
            panel->SetVisible(0);
        }
    }
}

template <typename Human>
const std::uint16_t* InViewAffects(const Human* human)
{
    // RemoveMob marks m_nWillDie before the delayed-delete animation finishes.
    // That actor may still be found by ID, but no longer belongs to the view.
    return human && !human->m_cDeleted && !human->m_dwDelayDel
        && !human->m_cDie && human->m_nWillDie == -1 ? human->m_usAffect : nullptr;
}

// Public words contain a visual type in the high byte and duration in the low
// byte. They are server snapshots, not the owner's private STRUCT_AFFECT array.
template <typename Panel>
int Project(Panel* const (&panels)[kCapacity], const std::uint16_t* words,
    const int* textures, std::size_t textureCount, float x, float y,
    float size, int columns)
{
    int visible = 0;
    for (int i = 0; i < kCapacity; ++i) {
        Panel* panel = panels[i];
        if (!panel) continue;
        const unsigned int type = words ? words[i] >> 8 : 0;
        if (!words || !textures || type == 0 || type >= textureCount
            || (words[i] & 0xff) == 0 || textures[type] <= 0
            || columns <= 0 || size <= 0) {
            panel->SetVisible(0);
            continue;
        }
        panel->m_GCPanel.nTextureIndex = textures[type];
        panel->SetSize(size, size);
        panel->SetPos(x + static_cast<float>(visible % columns) * (size + 1.0f),
            y + static_cast<float>(visible / columns) * (size + 1.0f));
        panel->SetVisible(1);
        ++visible;
    }
    return visible;
}
} // namespace observed_affect_ui
