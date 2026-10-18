#include "../internal/core/Basedef.h"
#include "../internal/ui/NativeAylinPreview.h"
#include "../internal/ui/OdinComposition.h"
#include "../internal/ui/NativeInventoryGlow.h"
#include "../internal/render/mesh/RefinementGlowTexture.h"
#include "../internal/render/mesh/NativeMountGlow.h"
#include "../internal/game/entities/WeaponPoseRefresh.h"

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
    STRUCT_ITEM odin[8]{};
    constexpr short unlock[] = {4127, 4127, 5135, 5113, 5129, 5112, 5110};
    for (int slot = 0; slot < 7; ++slot)
        odin[slot].sIndex = unlock[slot];
    check(odin_composition::AcceptsSealRecipe(odin), "Odin accepts the ordered level-40 recipe");
    for (int slot = 0; slot < 8; ++slot) {
        const auto original = odin[slot];
        odin[slot].sIndex = 0;
        check(odin_composition::AcceptsSealRecipe(odin) == (slot == 7),
            "all seven ingredients are required and the unused slot is empty");
        odin[slot].sIndex = 413;
        check(!odin_composition::AcceptsSealRecipe(odin), "partial powder substitutions are rejected");
        odin[slot] = original;
    }
    for (int left = 3; left < 7; ++left)
        for (int right = left + 1; right < 7; ++right) {
            std::swap(odin[left], odin[right]);
            check(!odin_composition::AcceptsSealRecipe(odin), "unlock rune order matches the server");
            std::swap(odin[left], odin[right]);
        }
    for (int slot = 3; slot < 7; ++slot)
        odin[slot].sIndex = 413;
    check(odin_composition::AcceptsSealRecipe(odin), "Odin cape recipe remains accepted");
    const auto first = Item(1000, 2, 30, 3);
    using native_inventory_glow::Background;
    using native_inventory_glow::PulsedColor;
    check(Background(3140, 0, 0) == 0xFFFFFFFFu, "Glittering Stone native halo");
    check(Background(4141, 0, 0) == 0xFF8800BBu &&
        Background(4142, 0, 0) == 0xFF444488u, "native purple halo variants");
    check(Background(1739, 0, 0) == 0xFFAA8888u &&
        Background(4145, 0, 0) == 0xFFAA8888u &&
        Background(1000, 206, 59) == 0xFFAA8888u, "native effect halo");
    check(Background(1000, 206, 58) == 0 && Background(1000, 205, 59) == 0,
        "halo type and first-effect gates");
    constexpr unsigned int palette[] = {0xFF777777u, 0xFF666677u, 0xFF555599u,
        0xFF995555u, 0xFF559955u, 0xFF999900u, 0xFF998866u, 0xFF775599u};
    for (int item = 1; item < 6000; ++item)
    {
        unsigned int expected = 0;
        if (item == 4141) expected = 0xFF8800BBu;
        if (item == 4142) expected = 0xFF444488u;
        if (item == 1739 || item == 4145) expected = 0xFFAA8888u;
        if (item >= 1744 && item <= 1751) expected = palette[item - 1744];
        if ((item >= 1760 && item <= 1763) || item == 4032 ||
            (item >= 5334 && item <= 5338)) expected = 0xFFFFFFAAu;
        if (item == 3140 || (item >= 3407 && item <= 3416)) expected = 0xFFFFFFFFu;
        check(Background(item, 0, 0) == expected, "native halo catalog boundaries");
    }
    check(Background(1744, 206, 59) == palette[0] &&
        Background(3140, 206, 59) == 0xFFFFFFFFu, "native halo precedence");
    check(PulsedColor(0xFFFFFFFFu, 500) == 0xFF4C4C4Cu &&
        PulsedColor(0xFFFFFFFFu, 1500) == 0xFFFFFFFFu,
        "native sine pulse minimum and maximum");
    for (unsigned int elapsed = 0; elapsed < 2000; ++elapsed)
    {
        const auto pulsed = PulsedColor(0x7F804020u, elapsed);
        check((pulsed >> 24) == 0x7Fu, "pulse preserves alpha");
        check(((pulsed >> 16) & 0xFFu) >= 38 && ((pulsed >> 16) & 0xFFu) <= 128,
            "pulse remains positive and within native range");
        check(pulsed == PulsedColor(0x7F804020u, elapsed + 2000), "pulse period");
    }
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
    for (int grade = 0; grade < 13; ++grade)
    {
        check(native_mount_glow::InventoryTextureIndex(grade, false) == mountTextures[grade],
            "native inventory shares the grade streak table");
        check(native_mount_glow::InventoryTextureIndex(grade, true) == 233 + grade,
            "native inventory legacy hardware retains linear textures");
    }
    check(native_mount_glow::TextureIndex(-1) == 121 &&
        native_mount_glow::TextureIndex(13) == 121 &&
        native_mount_glow::TextureIndex(15) == 121, "native mount overflow fallback");
    check(native_mount_glow::TextureIndex(7) != native_mount_glow::TextureIndex(12),
        "level 70 and level 120 cannot request the same texture");

    struct PoseHuman
    {
        int m_nWeaponTypeIndex = 4;
        void* m_pSkinMesh = this;
        int m_eMotion = 2;
        int m_nLoop = 1;
        int boundWeaponType = 4;
        int calls = 0;
        int boundMotion = -1;
        int boundLoop = -1;
        void SetAnimation(int motion, int loop)
        {
            ++calls;
            boundWeaponType = m_nWeaponTypeIndex;
            boundMotion = motion;
            boundLoop = loop;
        }
    };
    // Cover both single-hand selector results and mounted dual-wield stance.
    for (int dualType : {2, 4})
        for (int remainingType : {0, 1, 5})
        {
            PoseHuman human;
            human.m_nWeaponTypeIndex = dualType;
            human.boundWeaponType = dualType;
            weapon_pose::Select(human, remainingType);
            check(human.m_nWeaponTypeIndex == remainingType &&
                human.boundWeaponType == remainingType && human.calls == 1,
                "dual-to-single weapon pose rebinds after the new stance is stored");
            check(human.boundMotion == 2 && human.boundLoop == 1,
                "weapon pose refresh preserves the current motion and loop");
            weapon_pose::Select(human, remainingType);
            check(human.calls == 1, "unchanged stance does not restart animation");
        }
    PoseHuman noSkin;
    noSkin.m_pSkinMesh = nullptr;
    weapon_pose::Select(noSkin, 1);
    check(noSkin.m_nWeaponTypeIndex == 1 && noSkin.calls == 0,
        "weapon selection before skin creation does not animate a missing mesh");
    return failures;
}
