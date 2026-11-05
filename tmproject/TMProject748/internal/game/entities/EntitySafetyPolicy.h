#pragma once

#include <cstddef>
#include <type_traits>

// Bounds, timers and predicates of TMHuman and the entity helpers.
// No scene, Win32 or Direct3D dependency, so each decision is testable.

namespace human_timer
{
    // NewAppWindow resynchronizes server time by small steps in both
    // directions. A stamp up to this far ahead of "now" has not elapsed.
    constexpr unsigned int kBackwardTolerance = 60000;

    // Server time is a 32-bit tick that wraps. Compare the elapsed time, not an
    // absolute deadline, so a sum computed before the wrap cannot hold the
    // condition false for 49.7 days. A zero "never set" stamp still elapses at
    // any tick. The interval stays exclusive, as in "now > start + interval".
    constexpr bool Elapsed(unsigned int now, unsigned int start, unsigned int interval)
    {
        const unsigned int elapsed = now - start;
        return elapsed <= 0xFFFFFFFFu - kBackwardTolerance && elapsed > interval;
    }
}

namespace human_motion
{
    // Milliseconds per route cell. The legacy frame stopped the actor and
    // printed a debug notice when the speed made this zero.
    inline unsigned int UnitTime(float maxSpeed)
    {
        if (!(maxSpeed > 0.0f))
            return 1000;
        const float unitTime = 1000.0f / maxSpeed;
        return unitTime < 1.0f ? 1u : static_cast<unsigned int>(unitTime);
    }
}

namespace human_animation
{
    constexpr int kMotionCount = 28;   // ECMOTION_STAND01..ECMOTION_PUNEND per table half
    constexpr int kTableColumns = 56;  // dwAniTable/dwSpeed width, MAX_ANI_MOTION
    constexpr int kSkinTypes = 60;     // g_MobAniTable rows, MAX_ANI_TYPE
    constexpr int kExtendedClasses = 4; // g_MobAniTableEx rows
    constexpr int kExtendedSkins = 2;   // g_MobAniTableEx columns (skins 0 and 1)
    static_assert(kTableColumns == 2 * kMotionCount, "mounted clips use the second half");

    constexpr bool IsMotion(int motion)
    {
        return motion >= 0 && motion < kMotionCount;
    }

    constexpr bool IsSkinType(int skinType)
    {
        return skinType >= 0 && skinType < kSkinTypes;
    }

    // Heads below 40 that end in 1 or 6..9 use a class-specific table. That
    // table has columns only for skins 0 and 1; other skins use g_MobAniTable.
    // Returns the table row, or -1 when the regular table applies.
    constexpr int ExtendedClass(int headIndex, int skinType)
    {
        if (headIndex >= 40 || skinType < 0 || skinType >= kExtendedSkins)
            return -1;
        const int unit = headIndex % 10;
        if (unit != 1 && unit <= 5)
            return -1;
        const int row = unit == 1 ? headIndex / 10 : unit - 6;
        return row >= 0 && row < kExtendedClasses ? row : 0;
    }

    // Resolves MeshManager::m_sAnimationArray[skin][weapon][clip]. A clip read
    // from AniSound4.txt can exceed the array; -1 is rejected by
    // TMSkinMesh::SetAnimation, as an unknown clip index.
    template <class Clips>
    int HumanClip(const Clips& clips, int skin, int weapon, unsigned int clip)
    {
        using Array = std::remove_reference_t<decltype(clips)>;
        constexpr std::size_t skins = std::extent<Array, 0>::value;
        constexpr std::size_t weapons = std::extent<Array, 1>::value;
        constexpr std::size_t columns = std::extent<Array, 2>::value;
        if (skin < 0 || static_cast<std::size_t>(skin) >= skins || weapon < 0 ||
            static_cast<std::size_t>(weapon) >= weapons || clip >= columns)
            return -1;
        return clips[skin][weapon][clip];
    }
}

namespace human_cape
{
    // InitObject and RestoreDeviceObjects create a cape only for these skins.
    constexpr bool MayCarryCape(int mantua, int skinMeshType)
    {
        return mantua > 0 && (skinMeshType == 0 || skinMeshType == 1 || skinMeshType == 2 ||
            skinMeshType == 3 || skinMeshType == 8);
    }

    // Costume selector 0 draws the item cape; 100 draws the costume cape.
    // Other costume renderers replace the cape.
    constexpr bool CostumeKeepsCape(int costume)
    {
        return costume == 0 || costume == 100;
    }
}

namespace human_equipment
{
    constexpr int kCatalogItems = 6500; // MAX_ITEMLIST

    constexpr bool IsCatalogIndex(int itemIndex)
    {
        return itemIndex >= 0 && itemIndex < kCatalogItems;
    }

    // A malformed equipment index shows as an empty slot (catalog row 0).
    constexpr short CatalogIndex(int itemIndex)
    {
        return IsCatalogIndex(itemIndex) ? static_cast<short>(itemIndex) : static_cast<short>(0);
    }

    // Silver, Gold, Mystic and Arcana circlets hide the helmet mesh.
    constexpr bool IsCirclet(int itemIndex)
    {
        return itemIndex >= 3500 && (itemIndex <= 3502 || itemIndex == 3507);
    }

    template <class T, std::size_t N>
    constexpr T TableValue(const T (&table)[N], int index)
    {
        return index >= 0 && static_cast<std::size_t>(index) < N ? table[index] : T{};
    }
}

namespace human_combat
{
    // GetKeyState sets the sign bit while a key is down. Shifting the signed
    // SHORT keeps the sign, so "> 0" never matched and "<= 0" always matched.
    constexpr bool IsKeyDown(short keyState)
    {
        return (static_cast<unsigned short>(keyState) & 0x8000u) != 0;
    }

    // Skills that play their spell Act1/Act2 motion chain instead of the
    // packet motion. Both Attack overloads use this range.
    constexpr bool UsesSpellMotion(int skillIndex)
    {
        return (skillIndex >= 0 && skillIndex <= 103) || (skillIndex >= 200 && skillIndex < 248);
    }
}

namespace entity_arrow
{
    // Bow effect color for levels 1, 2, 3, 4, ...: entries 0, 1, 2, 0, ...
    // The legacy "(level % 3) - 1" read entry -1 for multiples of three.
    constexpr int EffectColorIndex(int level)
    {
        return level > 0 ? (level - 1) % 3 : 0;
    }
}
