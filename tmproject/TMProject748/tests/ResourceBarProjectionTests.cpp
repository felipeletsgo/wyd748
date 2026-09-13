#include "../internal/ui/ResourceBarProjection.h"
#include <cstdio>
#include <cstring>

namespace {
// Mirror SProgressBar's current-to-max clamp to expose ordering regressions.
struct Bar {
    int maximum = 100;
    int current = 50;
    int calls = 0;
    int order = 0;
    void SetMaxProgress(int value) {
        maximum = value;
        if (current > maximum) current = maximum;
        ++calls; order = order * 10 + 1;
    }
    void SetCurrentProgress(int value) {
        current = value > maximum ? maximum : value;
        ++calls; order = order * 10 + 2;
    }
};
}

int RunResourceBarProjectionTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL resource UI: %s\n", name); }
    };
    Bar stale;
    stale.SetCurrentProgress(150);
    check(stale.current == 100 && stale.maximum == 100, "old current-only path loses increased resource");
    const int snapshots[][2] = {
        {150, 200}, {15, 20}, {0, 20}, {20, 20}, {32768, 65536},
        {65536, 131072}, {1000000000, 2000000000}, {2000000000, 2000000000}
    };
    for (const auto& snapshot : snapshots) {
        Bar entity, hud;
        resource_ui::Project(&entity, snapshot[0], snapshot[1]);
        resource_ui::Project(&hud, snapshot[0], snapshot[1]);
        check(entity.maximum == snapshot[1] && entity.current == snapshot[0], "entity receives complete snapshot");
        check(hud.maximum == entity.maximum && hud.current == entity.current, "owner and entity use same scale");
        check(entity.order == 12 && hud.order == 12, "maximum precedes clamping current");
        entity.calls = entity.order = 0;
        resource_ui::Project(&entity, snapshot[0], snapshot[1]);
        check(entity.maximum == snapshot[1] && entity.current == snapshot[0], "repeated snapshot is stable");
    }
    Bar transition;
    resource_ui::Project(&transition, 150, 200);
    resource_ui::Project(&transition, 15, 20);
    check(transition.maximum == 20 && transition.current == 15, "maximum reduction replaces old scale");
    resource_ui::Project(&transition, 0, 20);
    check(transition.current == 0, "death empties bar");
    resource_ui::Project(&transition, 20, 20);
    check(transition.current == 20, "revival restores bar");
    resource_ui::Project(static_cast<Bar*>(nullptr), 10, 20);
    check(true, "missing optional control is safe");
    char text[32]{};
    std::snprintf(text, sizeof(text), resource_ui::MaximumTextFormat(true), 2000000000u);
    check(std::strcmp(text, "2000000000") == 0, "native max cell contains only full numeric value");
    std::snprintf(text, sizeof(text), resource_ui::MaximumTextFormat(false), 2000000000u);
    check(std::strcmp(text, "/ 2000000000") == 0, "combined layout retains max separator");
    return failures;
}
