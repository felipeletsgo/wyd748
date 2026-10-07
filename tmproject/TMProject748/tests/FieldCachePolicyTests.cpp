#include "../internal/app/scenes/FieldCachePolicy.h"
#include <cstdio>

namespace {
struct FakeGround {};
struct FakeObjects {};
using Entry = field_cache::Entry<FakeGround, FakeObjects>;
}

int RunFieldCachePolicyTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL field cache policy: %s\n", name); }
    };

    FakeGround grounds[4];
    Entry entries[field_cache::kMaxEntries];
    check(field_cache::ChooseSlot(entries, 4) == 0, "an empty cache fills the first slot");
    check(field_cache::Find(entries, 4, 16, 16) == -1, "an empty cache finds nothing");

    unsigned int clock = 0;
    for (int i = 0; i < 4; ++i) {
        const int slot = field_cache::ChooseSlot(entries, 4);
        entries[slot] = { 10 + i, 20, &grounds[i], nullptr, ++clock };
    }
    check(field_cache::Find(entries, 4, 12, 20) == 2, "a cached field is found by position");
    check(field_cache::ChooseSlot(entries, 4) == 0, "a full cache evicts the least recently used field");

    entries[0].lastUse = ++clock; // field 10/20 used again
    check(field_cache::ChooseSlot(entries, 4) == 1, "a reused field is no longer the eviction candidate");

    entries[2] = {};
    check(field_cache::ChooseSlot(entries, 4) == 2, "a taken field frees its slot");
    check(field_cache::Find(entries, 2, 13, 20) == -1, "slots beyond the capacity are ignored");
    check(field_cache::ChooseSlot(entries, 99) >= 0, "an oversized capacity is clamped");
    return failures;
}
