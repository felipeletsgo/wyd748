#include "../internal/ui/ObservedAffectProjection.h"
#include <cstdio>

namespace {
struct Panel {
    struct { int nTextureIndex = -1; } m_GCPanel;
    int visible = 0;
    float x = 0, y = 0, width = 0, height = 0;
    void SetVisible(int value) { visible = value; }
    void SetPos(float px, float py) { x = px; y = py; }
    void SetSize(float w, float h) { width = w; height = h; }
};
struct Human {
    int m_cDeleted = 0, m_dwDelayDel = 0, m_cDie = 0, m_nWillDie = -1;
    std::uint16_t m_usAffect[32]{};
};
}

int RunObservedAffectProjectionTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL observed affects: %s\n", name); }
    };
    Panel owned[32 * 14];
    Panel* target[32]{};
    Panel* party[13][32]{};
    int created = 0;
    observed_affect_ui::InitializePanels(target, party, [&]() {
        Panel* panel = &owned[created++];
        panel->SetVisible(1);
        return panel;
    });
    check(created == 32 * 14, "compat initialization creates all target and party panels");
    for (int i = 0; i < 32; ++i) {
        check(target[i] == &owned[i] && !target[i]->visible, "target bound and initially hidden");
        for (int row = 0; row < 13; ++row)
            check(party[row][i] == &owned[32 * (row + 1) + i] && !party[row][i]->visible,
                "party slot bound to its own initially hidden control");
    }
    const float screenWidths[] = {800.0f, 1024.0f, 1280.0f, 1920.0f};
    for (float screenWidth : screenWidths) {
        const float scaledWidth = 250.0f * screenWidth / 800.0f;
        const float left = observed_affect_ui::CenteredBarX(screenWidth, scaledWidth);
        check(left + scaledWidth * 0.5f == screenWidth * 0.5f,
            "target bar centered using its already-scaled width");
    }
    Panel storage[32];
    Panel* panels[32];
    for (int i = 0; i < 32; ++i) panels[i] = &storage[i];
    int textures[41]{};
    for (int i = 1; i <= 40; ++i) textures[i] = i + 100;
    std::uint16_t words[32]{};
    // First and last native visual type, high byte is unsigned; gaps compact.
    words[0] = 0x0101; words[3] = 0x28ff; words[31] = 0x1202;
    auto project = [&](const std::uint16_t* snapshot, int columns = 16) {
        return observed_affect_ui::Project(panels, snapshot, textures, 41, 100, 60, 10, columns);
    };
    Human member;
    member.m_usAffect[0] = 0x0101;
    check(project(observed_affect_ui::InViewAffects(&member)) == 1, "visible member has icons");
    member.m_nWillDie = 0;
    check(project(observed_affect_ui::InViewAffects(&member)) == 0,
        "RemoveMob hides icons before delayed deletion starts");
    member.m_nWillDie = -1;
    member.m_dwDelayDel = 1;
    check(observed_affect_ui::InViewAffects(&member) == nullptr, "delayed actor is not in view");
    member.m_dwDelayDel = 0;
    member.m_cDeleted = 1;
    check(observed_affect_ui::InViewAffects(&member) == nullptr, "deleted actor is not in view");
    member.m_cDeleted = 0;
    member.m_cDie = 1;
    check(observed_affect_ui::InViewAffects(&member) == nullptr, "dead actor has no icons");
    check(observed_affect_ui::InViewAffects<Human>(nullptr) == nullptr, "absent member is not in view");
    member.m_cDie = 0;
    member.m_usAffect[0] = 0x0201;
    check(project(observed_affect_ui::InViewAffects(&member)) == 1
        && storage[0].m_GCPanel.nTextureIndex == 102, "reentry uses fresh entity affects");
    check(project(words) == 3, "all occupied slots including slot 31 are projected");
    check(storage[0].x == 100 && storage[3].x == 111 && storage[31].x == 122,
        "sparse slots have no visual gaps");
    check(storage[3].m_GCPanel.nTextureIndex == 140, "correct visual type, not duration");
    words[0] = 0xff01; words[1] = 0x2901; words[2] = 0x00ff;
    words[3] = 0x0100; words[31] = 0;
    check(project(words) == 0, "unknown types, empty types and expired words are safe");
    check(storage[0].visible == 0 && storage[31].visible == 0, "new target clears old icons");
    for (int i = 0; i < 32; ++i) words[i] = 0x0101;
    check(project(words) == 32, "full snapshot is not truncated");
    check(storage[15].x == 265 && storage[16].x == 100 && storage[16].y == 71,
        "party wraps after sixteen without invading next row");
    check(storage[31].y + storage[31].height <= 82, "all slots fit a 22 pixel party row");
    check(project(words, 12) == 32 && storage[12].x == 100 && storage[12].y == 71,
        "target wraps to the HP bar width");
    check(project(nullptr) == 0, "no hover, hidden group or member out of view clears projection");
    for (const auto& panel : storage) check(!panel.visible, "no leftover icons");
    std::uint16_t cleared[32]{};
    check(project(cleared) == 0, "cleanse/death empty snapshot clears icons");
    check(project(words) == 32, "reentering view displays the new entity snapshot");
    panels[0] = nullptr;
    check(project(words) == 31 && storage[1].x == 100, "missing control is safe and compacts");
    check(project(words, 0) == 0, "invalid layout does not divide by zero");
    textures[1] = -1;
    check(project(words) == 0, "unmapped textures never select an unrelated icon");
    return failures;
}
