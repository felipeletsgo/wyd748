#pragma once

#include <cstddef>

// Presentation decisions of TMHuman without scene or Direct3D dependencies.
namespace effect_visibility
{
    // Hidden actors and observed Shadow actors hide their effects. The local
    // player keeps them, so its own Shadow indicator plane can be shown.
    inline bool HidesActorEffects(int hide, int shadow, bool localActor)
    {
        return hide != 0 || (shadow == 1 && !localActor);
    }

    // The chat Effects toggles hide persistent buff visuals.
    inline bool HidesBuffEffects(int hideSkillBuff, int hideSkillBuff2)
    {
        return hideSkillBuff == 1 || hideSkillBuff2 == 1;
    }

    // Hide switches only clear m_bShow; constructors are its only other writer.
    template <class Effect>
    void Show(Effect* effect)
    {
        if (effect)
            effect->m_bShow = 1;
    }
}

namespace ground_buff_visual
{
    // Lift horizontal buff planes just enough to clear the terrain surface.
    inline constexpr float Lift = 0.05f;
}

namespace afterimage_clip
{
    constexpr int kHumanSkins = 6;      // MeshManager MAX_ANI_CLASS
    constexpr int kAnimationTypes = 60; // g_MobAniTable rows, MAX_ANI_TYPE
    constexpr int kMotionColumns = 56;  // dwAniTable width, MAX_ANI_MOTION
    constexpr int kWolfBearSkin = 3;    // never filled in m_sAnimationArray

    inline bool IsTableCell(int skin, int column)
    {
        return skin >= 0 && skin < kAnimationTypes && column >= 0 && column < kMotionColumns;
    }

    // TMHuman::SetAnimation resolves skins 0..5, except the wolf/bear
    // skeleton, through m_sAnimationArray; other skins use the table value.
    inline bool UsesHumanArray(int skin)
    {
        return skin >= 0 && skin < kHumanSkins && skin != kWolfBearSkin;
    }

    // Body clip of an after-image, selected as SetAnimation selects it.
    template <class Table, class HumanArray>
    int Body(const Table& table, const HumanArray& humanArray, int skin, int weapon, int column)
    {
        if (!IsTableCell(skin, column))
            return 0;
        const unsigned int animation = table[skin].dwAniTable[column];
        if (!UsesHumanArray(skin))
            return static_cast<int>(animation);
        if (weapon < 0 || weapon >= kAnimationTypes || animation >= static_cast<unsigned int>(kMotionColumns))
            return 0;
        return static_cast<int>(humanArray[skin][weapon][animation]);
    }

    // Mount meshes use their own table value, as SetAnimation's mount path does.
    template <class Table>
    int Mount(const Table& table, int mountSkin, int column)
    {
        return IsTableCell(mountSkin, column) ? static_cast<int>(table[mountSkin].dwAniTable[column]) : 0;
    }
}

namespace mount_costume
{
    // Selectors with a visual in TMHuman::SetMountCostume. Other selectors
    // (empty cases and the default branch) must leave the mount unchanged.
    inline bool HasVisual(unsigned int index)
    {
        if (index < 11 || index > 118)
            return false;
        if (index == 54 || index == 55)
            return false;
        if (index >= 63 && index <= 69)
            return false;
        if (index >= 74 && index <= 90)
            return false;
        return true;
    }
}

namespace guild_mark_label
{
    constexpr int kSlots = 64; // TextureManager::m_stGuildMark, MAX_GUILD_MARK

    // A slot key is guild + (channel << 16), written by Guildmark_Create.
    template <class Slots>
    bool Shows(const Slots& slots, int markIndex, int guild, int channel)
    {
        if (markIndex < 0 || markIndex >= kSlots)
            return false;
        const int key = slots[markIndex].nGuild;
        return guild == (key & 0xFFFF) && (key >> 16) == channel;
    }
}

namespace challenge_label
{
    // UIString rows 64..67 hold the native challenge-mode captions in each
    // language folder ("1_:_1" .. "All:All"); '_' renders as a space.
    constexpr int kFirstUIString = 64;
    constexpr unsigned int kModes = 4;

    inline const char* Fallback(unsigned int mode)
    {
        static const char* const labels[kModes] = { "1 : 1", "5 : 5", "10 : 10", "All : All" };
        return labels[mode % kModes];
    }

    template <std::size_t N>
    void Format(const char* resource, unsigned int mode, char (&output)[N])
    {
        static_assert(N > 0, "output buffer");
        const char* source = resource && resource[0] ? resource : Fallback(mode);
        std::size_t i = 0;
        for (; i + 1 < N && source[i]; ++i)
            output[i] = source[i] == '_' ? ' ' : source[i];
        output[i] = '\0';
    }
}
