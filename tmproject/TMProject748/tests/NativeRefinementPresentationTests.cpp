#include "../internal/core/Basedef.h"
#include "../internal/ui/NativeAylinPreview.h"
#include "../internal/render/mesh/RefinementGlowTexture.h"
#include "../internal/render/mesh/NativeMountGlow.h"

#include <cstdio>
#include <cstring>

namespace
{
STRUCT_ITEM Item(short index, unsigned char first, unsigned char second,
    unsigned char third)
{
    STRUCT_ITEM item{};
    item.sIndex = index;
    item.stEffect[0].cEffect = 43;
    item.stEffect[0].cValue = 9;
    item.stEffect[1].cEffect = first;
    item.stEffect[1].cValue = second;
    item.stEffect[2].cEffect = third;
    item.stEffect[2].cValue = 17;
    return item;
}
}

int RunNativeRefinementPresentationTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* description) {
        ++checks;
        if (!condition)
        {
            ++failures;
            std::fprintf(stderr, "FAIL refinement presentation: %s\n", description);
        }
    };
    const auto first = Item(1000, 2, 30, 3);
    const auto second = Item(1001, 4, 40, 5);
    std::array<STRUCT_ITEM, 4> results{};
    check(native_aylin::Build(first, second, 2441, 9, results), "complete preview");
    for (int slot = 0; slot < 4; ++slot)
    {
        check(results[slot].sIndex == first.sIndex, "first item identity");
        check(results[slot].stEffect[0].cValue == 230, "Diamond +10 raw encoding");
        const bool useSecondAdditional = slot == 1 || slot == 2;
        check(results[slot].stEffect[1].sValue ==
            (useSecondAdditional ? second : first).stEffect[1].sValue,
            "four additional combinations");
        check(results[slot].stEffect[2].sValue ==
            (slot >= 2 ? second : first).stEffect[2].sValue, "base effect preserved");
    }
    check(first.stEffect[0].cValue == 9 && second.stEffect[0].cValue == 9,
        "staged inputs unchanged");
    results[0].stEffect[1].cValue = 99;
    check(results[1].stEffect[1].cValue == 40 && first.stEffect[1].cValue == 30,
        "independent preview copies");

    for (int level = 9; level <= 11; ++level)
        for (int gem = 2441; gem <= 2450; ++gem)
        {
            check(native_aylin::Build(first, second, gem, level, results),
                "native accepted gem range");
            const int expected = 230 + (level - 9) * 4 + gem - 2441;
            for (const auto& result : results)
                check(result.stEffect[0].cValue == (expected >= 246 ? 9 : expected),
                    "gem/level encoding and native >=26 rejection");
        }
    for (int position = 1; position <= 2; ++position)
    {
        auto unordered = first;
        std::swap(unordered.stEffect[0], unordered.stEffect[position]);
        check(native_aylin::Build(unordered, second, 2444, 9, results), "normalize");
        check(results[0].stEffect[0].cEffect == 43 &&
            results[0].stEffect[0].cValue == 233, "refinement moved to front");
        check(results[0].stEffect[1].sValue == first.stEffect[1].sValue,
            "normalized additional");
    }
    for (int tint = 115; tint <= 126; ++tint)
    {
        auto dyed = first;
        dyed.stEffect[0].cEffect = static_cast<unsigned char>(tint);
        check(native_aylin::Build(dyed, second, 2443, 9, results), "tinted preview");
        check(results[0].stEffect[0].cEffect == tint &&
            results[0].stEffect[0].cValue == 232, "tint retained");
        dyed.stEffect[2].cEffect = 43;
        check(native_aylin::SetRefinement(dyed, 10), "set explicit refinement");
        check(dyed.stEffect[0].cValue == 9 && dyed.stEffect[2].cValue == 230,
            "explicit refinement precedes earlier tint");
    }
    for (int level = -1; level <= 26; ++level)
    {
        auto item = first;
        const auto previous = item;
        check(native_aylin::SetRefinement(item, level) == (level < 26),
            "setter accepted range");
        const int expected = level < 0 ? 0 : level <= 9 ? level : level + 220;
        check(level >= 26 ? std::memcmp(&item, &previous, sizeof(item)) == 0 :
            item.stEffect[0].cValue == expected, "setter boundary encoding");
    }
    auto noRefinement = first;
    noRefinement.stEffect[0].cEffect = 0;
    check(native_aylin::SetRefinement(noRefinement, 10) &&
        noRefinement.stEffect[0].cEffect == 43 &&
        noRefinement.stEffect[0].cValue == 230, "first empty pair");
    noRefinement.stEffect[0].cEffect = 1;
    const auto unchanged = noRefinement;
    check(!native_aylin::SetRefinement(noRefinement, 10) &&
        std::memcmp(&noRefinement, &unchanged, sizeof(unchanged)) == 0,
        "full effects unchanged");
    for (int gem : {0, 2440, 2451})
    {
        check(!native_aylin::Build(first, second, gem, 9, results), "invalid gem");
        for (const auto& result : results)
            check(result.sIndex == 0, "clear stale results on rejection");
    }
    STRUCT_ITEM missing{};
    check(!native_aylin::Build(missing, second, 2441, 9, results), "missing first");
    check(!native_aylin::Build(first, missing, 2441, 9, results), "missing second");

    // Independent native texture oracle, not derived from the production helper.
    constexpr int expectedTextures[] = {153, 166, 246, 260};
    for (int variant = 0; variant < 4; ++variant)
    {
        check(refinement_glow::TextureBase(5 + variant) == expectedTextures[variant],
            "Ancient gem texture");
        check(refinement_glow::TextureBase(9 + variant) == expectedTextures[variant],
            "refined gem texture");
    }
    for (int legend : {-1, 0, 4, 13, 119})
        check(refinement_glow::TextureBase(legend) == 153, "legacy fallback");
    constexpr int mountTextures[] = {243, 234, 235, 236, 237, 239, 240,
        240, 241, 242, 243, 244, 245};
    for (int grade = 0; grade <= 12; ++grade)
        check(native_mount_glow::TextureIndex(grade) == mountTextures[grade],
            "native mount refinement texture lookup");
    check(native_mount_glow::TextureIndex(-1) == 121 &&
        native_mount_glow::TextureIndex(13) == 121 &&
        native_mount_glow::TextureIndex(15) == 121, "native mount overflow fallback");
    check(native_mount_glow::TextureIndex(7) != native_mount_glow::TextureIndex(12),
        "level 70 and level 120 cannot request the same texture");
    return failures;
}
