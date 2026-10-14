#pragma once

namespace skill_requirement_748 {
// Native 7.48 books have eight entries per lineage. Advanced books retain
// the same twelve lineage captions with four entries per lineage.
constexpr int BookLineage(int itemId)
{
    if (itemId >= 5000 && itemId < 5096)
        return (itemId - 5000) / 8;
    if (itemId >= 5400 && itemId < 5448)
        return (itemId - 5400) / 4;
    return -1;
}

constexpr int RequirementCaption(int itemId, int masteryIndex)
{
    const int lineage = BookLineage(itemId);
    return lineage >= 0 && masteryIndex >= 1 && masteryIndex <= 3
        ? (lineage / 3) * 3 + masteryIndex - 1 : -1;
}

constexpr int ManaMastery(int skillId, int characterClass)
{
    return skillId >= 0 && skillId < 96 && characterClass >= 0 && characterClass < 4
        && skillId / 24 == characterClass ? (skillId % 24) / 8 + 1 : -1;
}

template <typename Score>
bool MeetsMastery(const Score& score, int masteryIndex, int required)
{
    return masteryIndex >= 1 && masteryIndex <= 3 && required >= 0
        && score.Mastery[masteryIndex] >= static_cast<unsigned int>(required);
}
}
