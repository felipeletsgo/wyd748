#pragma once
#include <cstdint>

namespace corrupted_crystal
{
inline constexpr int FaceIndex = 4095;
inline constexpr int MediumFaceIndex = 4094;
inline constexpr int SmallFaceIndex = 4093;
inline constexpr int SkinType = 52;
inline constexpr int MonsterClass = 58;
inline constexpr int ThunderSkill = 33;
inline constexpr unsigned char PulseMotion = 254;
inline constexpr unsigned int CorpseLifetimeMs = 1200;
inline constexpr unsigned int AnimationQuarterMs = 20;
inline constexpr int CriticalCracks = 3;

// Apply only to full-width HP from CreateMob and SetHpMp.
constexpr int CrackStage(std::uint32_t hp, std::uint32_t maxHp)
{
    if (!maxHp)
        return 0;
    const auto quarters = std::uint64_t{hp} * 4;
    if (quarters <= maxHp)
        return CriticalCracks;
    if (quarters <= std::uint64_t{maxHp} * 2)
        return 2;
    return quarters <= std::uint64_t{maxHp} * 3 ? 1 : 0;
}

constexpr bool IsFace(unsigned int actorId, int faceIndex)
{
    return actorId >= 1000 && faceIndex >= SmallFaceIndex && faceIndex <= FaceIndex;
}

constexpr float Scale(int faceIndex)
{
    switch (faceIndex)
    {
    case MediumFaceIndex: return 0.65f;
    case SmallFaceIndex: return 0.4f;
    default: return 1.0f;
    }
}
}
