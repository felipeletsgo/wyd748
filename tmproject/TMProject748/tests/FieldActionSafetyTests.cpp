#include "../internal/app/scenes/FieldActionPolicy.h"
#include "../internal/application/FieldInteractionPolicy.h"
#include "../internal/application/CCModePolicy.h"
#include "../internal/application/ClientSendCadence.h"
#include "../internal/application/SlotItemConfirmation.h"
#include "../internal/ui/GuildMarkDownload.h"

#include <cstdio>
#include <vector>
#include <algorithm>
#include <cstring>

namespace
{
struct FakeItem
{
    static int destroyed;
    int value = 0;
    ~FakeItem() { ++destroyed; }
};

int FakeItem::destroyed = 0;

struct FakeGrid
{
    static FakeItem* m_pLastMouseOverItem;
    static FakeItem* m_pLastAttachedItem;
    static FakeItem* m_pSellItem;
};

FakeItem* FakeGrid::m_pLastMouseOverItem = nullptr;
FakeItem* FakeGrid::m_pLastAttachedItem = nullptr;
FakeItem* FakeGrid::m_pSellItem = nullptr;

struct FakeCursor
{
    FakeItem* m_pAttachedItem = nullptr;
};

struct FakeOffer
{
    int sIndex = 0;
};
}

int RunFieldActionSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    // A03-09: unrelated nodes and a last-node target cannot reset a valid target.
    {
        struct Actor { Actor* m_pNextLink = nullptr; bool usable = true; } first, last, absent;
        first.m_pNextLink = &last;
        Actor* target = &last;
        int attacks = 0;
        const auto usable = [](Actor* actor) { return actor->usable; };
        const auto attack = [&](Actor*) { ++attacks; return 1; };
        check(cc_mode::UseRetainedTarget(&first, target, usable, attack) && target == &last && attacks == 1,
            "A03-09 a later target survives preceding unrelated actors and is visited once");
        target = &last;
        check(cc_mode::UseRetainedTarget(&last, target, usable, attack) && target == &last,
            "A03-09 a single actor is not skipped as the last node");
        target = &absent;
        attacks = 0;
        check(!cc_mode::UseRetainedTarget(&first, target, usable, attack) && !target && attacks == 0,
            "A03-09 an absent target is cleared without dereference or attack");
        target = &last;
        last.usable = false;
        check(!cc_mode::UseRetainedTarget(&first, target, usable, attack) && !target && attacks == 0,
            "A03-09 an unusable target is cleared before attack");
        last.usable = true;
        target = &last;
        check(cc_mode::UseRetainedTarget(&first, target, usable, [](Actor*) { return 2; }) && target == &last,
            "A03-09 a retry result retains the target");
        check(!cc_mode::UseRetainedTarget(&first, target, usable, [](Actor*) { return 0; }) && !target,
            "A03-09 a rejected attack clears the matched target");
    }

    // A03-03: bind only an entity that is live when the event is queued.
    check(field_action::MayBindEffectTarget(0, 0, 0) &&
        !field_action::MayBindEffectTarget(1, 0, 0) &&
        !field_action::MayBindEffectTarget(0, 1, 0) &&
        !field_action::MayBindEffectTarget(0, 0, 1),
        "A03-03 queued visuals reject actors already retiring");

    // A03-04: elapsed-time comparisons remain correct at the uint32 wrap.
    check(field_time::NotAfter(0x00000020u, 0xFFFFFFF0u, 1000u) &&
        field_time::After(0x00000020u, 0xFFFFFFF0u, 40u) &&
        !field_time::NotAfter(0x00000020u, 0xFFFFF000u, 1000u),
        "A03-04 attack gate uses elapsed ticks across wrap");
    check(field_interaction::PhysicalAttackTick(true, true, false, false, false,
        0x00000020u, 0xFFFFFFF0u) == field_interaction::PhysicalAttackAction::Wait,
        "A03-04 retained target waits only through its short retry interval");

    // A03-06: every untrusted Motion byte maps to an in-table reaction slot.
    bool allMotionIndicesInRange = true;
    for (int actorClass = -2; actorClass < 9; ++actorClass)
        for (int motion = 0; motion <= 255; ++motion)
        {
            const int index = field_action::PunchDelayIndex(actorClass,
                static_cast<unsigned char>(motion));
            if (index < 0 || index >= 36)
                allMotionIndicesInRange = false;
        }
    check(allMotionIndicesInRange && field_action::PunchDelayIndex(0, 0) == 0 &&
        field_action::PunchDelayIndex(0, 9) == 5 &&
        field_action::PunchDelayIndex(5, 255) == 35,
        "A03-06 motions clamp to the six valid rows per class");

    // A03-10: target-number delay is a fixed 100 ms step, not an accumulated
    // per-target delay. Other skills keep their original base delay.
    check(field_action::HitNumberDelay(250, 95, 0) == 250 &&
        field_action::HitNumberDelay(250, 95, 1) == 350 &&
        field_action::HitNumberDelay(250, 95, 3) == 550 &&
        field_action::HitNumberDelay(250, 94, 3) == 250,
        "A03-10 Rapid Hit numbers keep the intended target cadence");

    // A04-01/A05-11: deletion clears every UI alias before the object dies.
    {
        FakeItem::destroyed = 0;
        auto* item = new FakeItem{};
        FakeCursor cursor{ item };
        FakeGrid::m_pLastMouseOverItem = item;
        FakeGrid::m_pLastAttachedItem = item;
        FakeGrid::m_pSellItem = item;
        grid_item_release::Release<FakeGrid>(item, &cursor);
        check(item == nullptr && cursor.m_pAttachedItem == nullptr &&
            FakeGrid::m_pLastMouseOverItem == nullptr &&
            FakeGrid::m_pLastAttachedItem == nullptr && FakeGrid::m_pSellItem == nullptr &&
            FakeItem::destroyed == 1,
            "A04-01 released item leaves no dangling UI alias");

        auto* unrelated = new FakeItem{};
        auto* selected = new FakeItem{};
        FakeGrid::m_pSellItem = selected;
        FakeCursor otherCursor{ unrelated };
        grid_item_release::Release<FakeGrid>(selected, &otherCursor);
        const bool unrelatedSurvives = FakeGrid::m_pSellItem == nullptr &&
            otherCursor.m_pAttachedItem == unrelated && FakeItem::destroyed == 2;
        delete unrelated;
        FakeGrid::m_pLastMouseOverItem = nullptr;
        FakeGrid::m_pLastAttachedItem = nullptr;
        FakeGrid::m_pSellItem = nullptr;
        check(unrelatedSurvives, "A04-01 releasing one item preserves other aliases");
    }

    // A04-05/A04-09: catalog and footprint inputs stay inside native arrays.
    check(field_action::IsCatalogItem(1, 6500) &&
        field_action::IsCatalogItem(6499, 6500) &&
        !field_action::IsCatalogItem(0, 6500) &&
        !field_action::IsCatalogItem(6500, 6500) &&
        !field_action::IsCatalogItem(-1, 6500),
        "A04-05 only valid catalog rows reach shop rendering");
    check(field_action::ItemGridShape(0) == 0 && field_action::ItemGridShape(7) == 7 &&
        field_action::ItemGridShape(-1) == 0 && field_action::ItemGridShape(8) == 0,
        "A04-09 malformed item footprints use the established fallback");

    // A05-02: empty and invalid shortcut bytes cannot wrap into Huntress 94.
    check(field_action::ValidShortSkillSlot(0) && field_action::ValidShortSkillSlot(19) &&
        !field_action::ValidShortSkillSlot(-1) && !field_action::ValidShortSkillSlot(20) &&
        field_action::ResolveShortSkill(104) == 104 &&
        field_action::ResolveShortSkill(105) == 200 &&
        field_action::ResolveShortSkill(152) == 247 &&
        field_action::ResolveShortSkill(153) == -1 &&
        field_action::ResolveShortSkill(255) == -1,
        "A05-02 only populated, in-range shortcut values resolve to skills");

    check(field_action::CanPrioritizeGroundPickup(413, 0) &&
        field_action::CanPrioritizeGroundPickup(454, 0) &&
        !field_action::CanPrioritizeGroundPickup(0, 0) &&
        !field_action::CanPrioritizeGroundPickup(746, 0) &&
        !field_action::CanPrioritizeGroundPickup(773, 1),
        "right-click pickup priority admits powder and gold but excludes the cannon and gate");

    // A05-03: support skills cannot target ordinary monsters or missing IDs.
    check(field_action::IsNonPlayerTarget(0, false, false) &&
        field_action::IsNonPlayerTarget(1000, false, false) &&
        !field_action::IsNonPlayerTarget(1, false, false) &&
        !field_action::IsNonPlayerTarget(1000, true, false) &&
        !field_action::IsNonPlayerTarget(1000, false, true),
        "A05-03 support targeting accepts players, party members and summons only");

    // A05-04: a completed route (-1) no longer blocks the auto-run toggle.
    check(!field_action::AirMoveBlocksAutoRun(false, -1) &&
        !field_action::AirMoveBlocksAutoRun(false, 0) &&
        field_action::AirMoveBlocksAutoRun(true, -1) &&
        field_action::AirMoveBlocksAutoRun(false, 1),
        "A05-04 only active AirMove state blocks auto-run");

    // A05-07: manual reports are throttled; the longer automatic cadence remains.
    check(field_action::ShouldReportCastFailure(false, false, 500, 0) &&
        !field_action::ShouldReportCastFailure(false, true, 1499, 500) &&
        field_action::ShouldReportCastFailure(false, true, 1500, 500) &&
        !field_action::ShouldReportCastFailure(true, true, 3499, 500) &&
        field_action::ShouldReportCastFailure(true, true, 3500, 500),
        "A05-07 repeated cast failures obey manual and automatic limits");

    // Guild mark transport must reject truncation, excess bytes and failed reads.
    {
        std::vector<char> bitmap(630, 0);
        const auto put32 = [&bitmap](std::size_t offset, unsigned int value) {
            for (int byte = 0; byte < 4; ++byte)
                bitmap[offset + byte] = static_cast<char>(value >> (8 * byte));
        };
        bitmap[0] = 'B';
        bitmap[1] = 'M';
        put32(2, 630);
        put32(10, 54);
        put32(14, 40);
        put32(18, 16);
        put32(22, 12);
        bitmap[26] = 1;
        bitmap[28] = 24;
        char output[guild_mark_download::MaxBytes + 1]{};
        const auto download = [&bitmap, &output](int failureAt) {
            std::size_t offset = 0;
            return guild_mark_download::ReadBitmap(output, sizeof(output),
                [&bitmap, &offset, failureAt](char* target, std::size_t capacity) {
                    if (failureAt >= 0 && offset >= static_cast<std::size_t>(failureAt))
                        return -1;
                    const auto size = std::min({capacity, bitmap.size() - offset, std::size_t(17)});
                    std::memcpy(target, bitmap.data() + offset, size);
                    offset += size;
                    return static_cast<int>(size);
                });
        };
        check(download(-1) == 630 && std::memcmp(output, bitmap.data(), 630) == 0,
            "guild marks preserve a complete BMP across fragmented reads");
        check(download(630) == 0, "a read error after the BMP prefix rejects the download");
        bitmap.resize(632);
        put32(2, 632);
        check(download(-1) == 632, "guild marks accept the supported 632-byte BMP");
        bitmap.pop_back();
        check(download(-1) == 0, "guild marks reject a truncated body");
        bitmap.resize(633);
        check(download(-1) == 0, "guild marks reject excess response bytes");
        bitmap.resize(630);
        put32(2, 630);
        const std::size_t headerFields[] = {0, 2, 10, 14, 18, 22, 26, 28, 30};
        for (const auto offset : headerFields)
        {
            const char saved = bitmap[offset];
            bitmap[offset] ^= 1;
            check(download(-1) == 0, "guild marks reject inconsistent BMP headers");
            bitmap[offset] = saved;
        }
        check(!guild_mark_download::IsValidBitmap(nullptr, 630) &&
            !guild_mark_download::IsValidBitmap(output, 53),
            "guild mark header reads reject null and short buffers");
    }

    // C-12: the detached downloader hands each completed request to the scene
    // exactly once. Network transport and worker-count limits stay external.
    {
        guild_mark_download::CompletionQueue<int> queue;
        int first = 1;
        int second = 2;
        queue.Push(&first);
        queue.Push(&second);
        const auto completed = queue.Drain();
        const auto empty = queue.Drain();
        check(completed.size() == 2 && completed[0] == &first &&
            completed[1] == &second && empty.empty(),
            "C-12 download completion queue drains each result once");
    }

    check(!client_send_cadence::Allows(1999, 1000, false, false, true, true) &&
        client_send_cadence::Allows(2000, 1000, false, false, true, true) &&
        !client_send_cadence::Allows(2000, 1000, true, true, false, false) &&
        client_send_cadence::Allows(2001, 1000, true, true, false, false),
        "A03-08 the limiter preserves distinct action and attack boundaries");
    check(client_send_cadence::Allows(0xFFFFFF00u + 1000u, 0xFFFFFF00u,
        false, false, true, true) &&
        !client_send_cadence::Allows(2004, 1005, false, false, true, true),
        "A03-08 tick wrap permits an attack, while send-time jitter can still reject it");

    struct ValueItem { short index; unsigned char effects[6]; };
    SlotItemConfirmation<ValueItem> prompt;
    ValueItem original{100, {1, 2, 3, 4, 5, 6}};
    ValueItem replacement = original;
    replacement.effects[3] = 7;
    prompt.Capture(4, &original);
    check(!prompt.Take(4, &replacement) && !prompt.Take(4, &original),
        "A02-08 a changed item invalidates the prompt even when its icon index matches");
    prompt.Capture(4, &original);
    check(!prompt.Take(5, &original), "A02-08 the prompt cannot act on another slot");
    prompt.Capture(4, &original);
    check(prompt.Take(4, &original) && !prompt.Take(4, &original),
        "A02-08 an unchanged item can be confirmed once");
    prompt.Capture(4, &original);
    check(!prompt.Take(4, nullptr), "A02-08 a removed item cancels a deferred use");
    return failures;
}
