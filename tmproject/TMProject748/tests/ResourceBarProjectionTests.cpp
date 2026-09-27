#include "../internal/ui/ResourceBarProjection.h"
#include "../internal/ui/MiniMapLayout.h"
#include "../internal/game/entities/DeathMotionPolicy.h"
#include <cstdio>
#include <cstring>
#include <utility>
#include <initializer_list>

namespace {
// Mirror SProgressBar's current-to-max clamp to expose ordering regressions.
struct Bar {
    int maximum = 100;
    int current = 50;
    int calls = 0;
    int order = 0;
    bool visible = true;
    void SetMaxProgress(int value) {
        maximum = value;
        if (current > maximum) current = maximum;
        ++calls; order = order * 10 + 1;
    }
    void SetCurrentProgress(int value) {
        current = value > maximum ? maximum : value;
        ++calls; order = order * 10 + 2;
    }
    void SetVisible(int value) { visible = value != 0; }
};
}

int RunResourceBarProjectionTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL resource UI: %s\n", name); }
    };
    for (const auto& viewport : { std::pair<float, float>{800.0f, 600.0f}, {1024.0f, 768.0f}, {1280.0f, 960.0f}, {1920.0f, 1080.0f} }) {
        for (bool ui2 : {false, true}) {
            const auto compact = mini_map_layout::Next(false, 1.5f, ui2, viewport.first, viewport.second);
            check(compact.visible && !compact.expanded && compact.scale == 0.6f, "hidden map opens compact");
            check(compact.x + compact.size <= viewport.first && compact.y >= 0, "compact map fits viewport");
            check(compact.size == (ui2 ? 137.0f : 160.0f), "compact size matches selected resource");
            const auto expanded = mini_map_layout::Next(compact.visible, compact.scale, ui2, viewport.first, viewport.second);
            check(expanded.visible && expanded.expanded && expanded.size == 400, "compact map opens expanded");
            check(expanded.x * 2 + expanded.size == viewport.first && expanded.y * 2 + expanded.size == viewport.second,
                "expanded map is centered without applying UI scale twice");
            const auto hidden = mini_map_layout::Next(expanded.visible, expanded.scale, ui2, viewport.first, viewport.second);
            check(!hidden.visible, "expanded map closes");
            const auto reopened = mini_map_layout::Next(hidden.visible, hidden.scale, ui2, viewport.first, viewport.second);
            check(reopened.visible && !reopened.expanded, "closed map reopens compact");
        }
    }
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
    Bar overlay;
    resource_ui::ProjectNativeHpVisual(&overlay, 0, 82);
    check(overlay.maximum == 82 && overlay.current == 0 && !overlay.visible,
        "zero HP hides a native HP visual even when its panel remains textured");
    resource_ui::ProjectNativeHpVisual(&overlay, 41, 82);
    check(overlay.current == 41 && overlay.visible,
        "revival restores the native HP visual");
    resource_ui::ProjectNativeHpVisual(static_cast<Bar*>(nullptr), 0, 82);
    check(true, "missing native HP visual is safe");
    resource_ui::Project(static_cast<Bar*>(nullptr), 10, 20);
    check(true, "missing optional control is safe");
    char text[32]{};
    std::snprintf(text, sizeof(text), resource_ui::MaximumTextFormat(true), 2000000000u);
    check(std::strcmp(text, "2000000000") == 0, "native max cell contains only full numeric value");
    std::snprintf(text, sizeof(text), resource_ui::MaximumTextFormat(false), 2000000000u);
    check(std::strcmp(text, "/ 2000000000") == 0, "combined layout retains max separator");
    check(death_motion::MayEnterTravelAnimation(82, false, false), "living character may enter travel animation");
    check(!death_motion::MayEnterTravelAnimation(0, false, false), "zero HP prevents travel before death confirmation");
    check(!death_motion::MayEnterTravelAnimation(0, true, false), "death confirmation cannot resume a running route");
    check(!death_motion::MayEnterTravelAnimation(82, true, false), "death flag prevents travel until revival clears it");
    check(!death_motion::MayEnterTravelAnimation(82, false, true), "sliding motion retains its own animation");
    check(death_motion::ShouldEnterDeath(0, false), "zero HP starts death without a kill confirmation");
    check(!death_motion::ShouldEnterDeath(0, true), "kill confirmation does not replay the death transition");
    check(!death_motion::ShouldEnterDeath(82, false), "positive HP cannot start death");
    check(death_motion::ShouldOpenRespawnPrompt(0, false, true, false, false, false, false),
        "dead player click opens respawn prompt before kill confirmation");
    check(death_motion::ShouldOpenRespawnPrompt(82, true, true, false, false, false, false),
        "death flag opens respawn prompt before vitals update");
    check(!death_motion::ShouldOpenRespawnPrompt(0, false, false, false, false, false, false),
        "other dead-player events do not open the click fallback");
    check(!death_motion::ShouldOpenRespawnPrompt(0, false, true, true, false, false, false),
        "familiar state retains the native respawn restriction");
    check(!death_motion::ShouldOpenRespawnPrompt(0, false, true, false, true, false, false),
        "ordinary town does not open the field respawn prompt");
    check(death_motion::ShouldOpenRespawnPrompt(0, false, true, false, true, true, false),
        "castle town retains the native respawn exception");
    check(!death_motion::ShouldOpenRespawnPrompt(0, false, true, false, false, false, true),
        "visible prompt is not reopened");
    check(!death_motion::ShouldOfferTimedRespawnPrompt(1000, 3999, 0, true,
        false, false, false, false),
        "timed respawn prompt waits for the death animation interval");
    check(death_motion::ShouldOfferTimedRespawnPrompt(1000, 4000, 0, true,
        false, false, false, false),
        "timed respawn prompt recovers when the death animation stalls");
    check(!death_motion::ShouldOfferTimedRespawnPrompt(1000, 4000, 0, true,
        false, false, false, true),
        "timed respawn prompt does not reopen a visible prompt");
    check(!death_motion::ShouldOfferTimedRespawnPrompt(0, 4000, 0, true,
        false, false, false, false),
        "timed respawn prompt requires a recorded death time");
    check(death_motion::MayOfferRespawnPrompt(false, false, false),
        "first death animation or timeout may offer the prompt");
    check(!death_motion::MayOfferRespawnPrompt(true, false, false),
        "dismissed prompt stays hidden across later frames and animation completion");
    check(death_motion::MayOfferRespawnPrompt(true, true, false),
        "a new player action can reopen a dismissed prompt");
    check(!death_motion::MayOfferRespawnPrompt(true, true, true) &&
        !death_motion::MayOfferRespawnPrompt(false, false, true),
        "recall channel suppresses both automatic and player-action prompts");
    check(death_motion::MayOfferRespawnPrompt(false, false, false),
        "resetting the offer after revival permits the next death prompt");
    check(!death_motion::ShouldAutoRecallDeadPlayer(1000, 181000, true, 3647, 3112),
        "automatic recall waits beyond three minutes");
    check(death_motion::ShouldAutoRecallDeadPlayer(1000, 181001, true, 3647, 3112),
        "automatic recall requests server revival after three minutes");
    check(!death_motion::ShouldAutoRecallDeadPlayer(0, 181001, true, 3647, 3112),
        "automatic recall requires a recorded death time");
    check(!death_motion::ShouldAutoRecallDeadPlayer(1000, 181001, false, 3647, 3112),
        "automatic recall does not move a revived character");
    check(!death_motion::ShouldAutoRecallDeadPlayer(1000, 181001, true, 128, 3112),
        "automatic recall preserves the native tile restriction");
    check(death_motion::ShouldAdvanceRespawnRecallCountdown(1000, 5999, true) &&
        death_motion::RespawnRecallSecondsRemaining(1000, 5999) == 0,
        "respawn recall countdown reaches its final second using its own timer");
    check(death_motion::ShouldAdvanceRespawnRecallCountdown(1000, 6000, true),
        "respawn recall countdown retains the exact five-second boundary");
    check(!death_motion::ShouldAdvanceRespawnRecallCountdown(1000, 6001, true),
        "respawn recall countdown stops when the request is due");
    check(!death_motion::ShouldAdvanceRespawnRecallCountdown(1000, 6000, false),
        "respawn recall countdown does not replay effects after the request");
    check(!death_motion::ShouldAdvanceRespawnRecallCountdown(0, 6000, true),
        "respawn recall countdown requires its own start time");
    return failures;
}
