#include "../internal/render/mesh/CostumeSelection.h"
#include <cstdio>
#include <cstring>
#include <iterator>

int RunCostumeSelectionTests(int& checks)
{
    using namespace costume748;
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL costume: %s\n", name); }
    };
    check(std::size(Items) == 135 && std::size(Renderers) == 129, "validated collection size");
    check(!FindItem(0) && !FindItem(-1) && !FindItem(65535), "empty/invalid slots remain empty");
    check(!FindItem(4349) && Select(4349, 0) == 0, "unavailable entry is not imported");
    check(Select(4307 & 0xFFF, 0) == 0, "low bits are not a full costume ID");
    check(Select(4366, 0) == (ImportedTag | 173), "King male renderer");
    check(Select(4366, 1) == (ImportedTag | 171), "King female renderer");
    check(Select(4367, 42) == (ImportedTag | 180), "TopRanker even body variant");
    check(Select(4367, 43) == (ImportedTag | 181), "TopRanker odd body variant");
    check(Select(4307, 0) == (ImportedTag | 108), "manifest type differs from upstream paths");
    check(Select(4490, 0) != 0, "available item beyond old 4420 admission limit");
    int tableItems = 0;
    for (const auto& item : Items) {
        check(FindItem(item.index) == &item, "exact item lookup");
        for (int skeleton = 0; skeleton < 44; ++skeleton) {
            const int type = item.skin == -1 && (skeleton & 1) ? item.femaleType : item.maleType;
            const int selected = Select(item.index, skeleton);
            if (type >= 1 && type <= 5) {
                check(selected == 0, "base costume delegates to existing renderer");
            } else {
                const auto* renderer = FindRenderer(selected);
                check(renderer && renderer->type == type, "every body variant resolves its exact renderer");
            }
        }
        if (Select(item.index, 0)) ++tableItems;
    }
    check(tableItems == 130, "130 table-selected items plus five base costumes");
    check(!FindRenderer(-1) && !FindRenderer(0) && !FindRenderer(100) &&
        !FindRenderer(ImportedTag) && !FindRenderer(ImportedTag * 2), "invalid/legacy selectors stay isolated");
    int preservedFaces = 0, replacements = 0;
    for (const auto& renderer : Renderers) {
        check(!FindRenderer(renderer.type), "untagged NPC skin never enters costume table");
        check(FindRenderer(ImportedTag | renderer.type) == &renderer, "tag selects exact table");
        for (unsigned int index = 0; index < 6; ++index) {
            const Part* part = nullptr;
            const auto action = ResolvePart(&renderer, index, part);
            if (renderer.parts[index].mesh[0]) {
                ++replacements;
                check(action == PartAction::Replace && part == &renderer.parts[index], "part order is stable");
                check(part && part->texture[0] && std::strlen(part->mesh) < 32 &&
                    std::strlen(part->texture) < 64, "paths fit production loader buffers");
            } else {
                if (index == 0) ++preservedFaces;
                check(!part && action == (index == 0 ? PartAction::Preserve : PartAction::Skip),
                    "empty face preserves native geometry; empty body skips only that part");
            }
        }
        const Part* stale = &renderer.parts[0];
        check(ResolvePart(&renderer, 6, stale) == PartAction::Preserve && !stale,
            "non-body part clears pointer and preserves original path");
        check(ResolvePart(&renderer, 0xFFFFFFFFu, stale) == PartAction::Preserve && !stale,
            "out-of-range index never reads table");
        check(ResolvePart(nullptr, 0, stale) == PartAction::Preserve && !stale, "no costume preserves original");
    }
    check(preservedFaces > 0 && replacements > 0, "both native-face and replacement paths covered");
    const Part* part = nullptr;
    check(ResolvePart(FindRenderer(Select(4307, 0)), 2, part) == PartAction::Replace && part &&
        std::strcmp(part->mesh, "mesh\\ch0203102.msh") == 0 &&
        std::strcmp(part->texture, "mesh\\ch0201102.wys") == 0,
        "4307 uses the catalogued part rather than the upstream costume switch");
    check(ResolvePart(FindRenderer(ImportedTag | 131), 0, part) == PartAction::Preserve && !part,
        "corrected costume keeps native face without shifting other parts");
    check(ResolvePart(FindRenderer(ImportedTag | 200), 0, part) == PartAction::Replace && part,
        "real part-zero geometry is not discarded");
    Renderer sparse{999, {{"", ""}, {"", ""}}};
    check(ResolvePart(&sparse, 1, part) == PartAction::Skip && !part, "empty body never falls into legacy loader");
    // Repeated loads and equip/dequip are stateless; no m_Cos cursor survives.
    for (int pass = 0; pass < 2; ++pass) {
        const auto* renderer = FindRenderer(Select(4307, 0));
        check(ResolvePart(renderer, 2, part) == PartAction::Replace && part == &renderer->parts[2],
            "reloading chooses the same explicit part");
        check(ResolvePart(FindRenderer(Select(0, 0)), 2, part) == PartAction::Preserve && !part,
            "unequip leaves no imported part state");
    }
    return failures;
}
