#include "../internal/application/ItemShortcutPolicy.h"
#include <cstdio>
#include <initializer_list>

int RunItemShortcutPolicyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", name); }
    };
    using namespace item_shortcuts;
    check(Resolve(0x205, 8, true, true, true, false, false) == Action::Equipment,
        "Ctrl right release equips from Carry");
    check(Resolve(0x205, 8, true, true, false, true, true) == Action::Equipment,
        "Ctrl right release unequips even with warehouse open");
    check(Resolve(0x205, 4, true, true, true, false, true) == Action::Warehouse,
        "Shift right release transfers storage items");
    check(Resolve(0x202, 8, true, true, true, false, true) == Action::Warehouse,
        "existing Ctrl left warehouse shortcut is retained");
    check(Resolve(0x201, 8, true, true, true, false, true) == Action::ReservePress,
        "Ctrl left press cannot start a drag before transfer");
    check(Resolve(0x205, 12, true, true, true, false, true) == Action::Equipment,
        "Ctrl wins over Shift without accidentally transferring equipment");
    for (unsigned int message : {0x201u, 0x202u, 0x204u, 0x205u})
    {
        check(Resolve(message, 4, true, true, false, true, true) != Action::Warehouse,
            "equipped items cannot transfer directly to warehouse");
        for (unsigned int modifiers : {0u, 4u, 8u, 12u})
        {
            check(Resolve(message, modifiers, false, true, true, false, true) == Action::None,
                "broadcast outside grid does not select a shortcut source");
            check(Resolve(message, modifiers, true, false, true, false, true) == Action::None,
                "attached cursor is not discarded by a shortcut");
            check(Resolve(message, modifiers, true, true, false, false, true) == Action::None,
                "shop trade mix and skill grids cannot emit item shortcuts");
            if (message == 0x204 && modifiers != 0)
                check(Resolve(message, modifiers, true, true, true, false, true) == Action::ReservePress,
                    "modified right press cannot consume an item");
        }
    }
    check(Resolve(0x201, 4, true, true, true, false, true) == Action::None,
        "Shift left press retains stack splitting");
    check(Resolve(0x202, 4, true, true, true, false, true) == Action::None,
        "Shift left release cannot transfer the whole stack");
    check(Resolve(0x205, 4, true, true, true, false, false) == Action::None,
        "Shift right without warehouse retains fairy sale handling");
    check(Resolve(0x202, 8, true, true, true, false, false) == Action::None,
        "closed warehouse prevents legacy quick transfer");
    for (int type = -1; type <= 3; ++type)
        for (int slot = -1; slot <= 129; ++slot)
        {
            const bool expected = (type == 0 && slot > 0 && slot < 16) ||
                (type == 1 && slot >= 0 && slot < 63) || (type == 2 && slot >= 0 && slot < 120);
            check(ValidSlot(type, slot) == expected,
                "shortcut rejects face modern equipment and reserved storage slots");
        }
    struct Control {
        Control* m_pTop = nullptr;
        bool m_bVisible = true;
        bool m_cDeleted = false;
        float m_nPosX = 0, m_nPosY = 0, m_nWidth = 0, m_nHeight = 0;
    };
    Control root;
    Control parent{&root, true, false, 100, 80, 300, 300};
    Control panel{&parent, true, false, 30, 40, 120, 160};
    check(VisibleHit(&panel, 130, 120) && VisibleHit(&panel, 249, 279),
        "item panel hit test includes all parent positions");
    check(!VisibleHit(&panel, 129, 120) && !VisibleHit(&panel, 250, 120) &&
        !VisibleHit(&panel, 130, 280), "outside panel retains world interaction menu");
    parent.m_bVisible = false;
    check(!VisibleHit(&panel, 130, 120), "hidden parent cannot intercept world Ctrl right click");
    parent.m_bVisible = true;
    panel.m_cDeleted = true;
    check(!VisibleHit(&panel, 130, 120) && !VisibleHit<Control>(nullptr, 130, 120),
        "deleted or absent panels cannot intercept world input");
    return failures;
}
