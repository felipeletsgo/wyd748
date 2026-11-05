#pragma once

#include <algorithm>
#include <cstddef>

// Target collection for the party-wide skills 29 and 44.
//
// The legacy loop appended in-range party members to a 12-entry stack array
// and re-sorted it after every member. A thirteenth in-range member (WYD-Go
// parties hold 13) was written past the array. The loop still sent at most
// the first 12 in-range members, nearest first. No native 7.48 record shows a
// thirteenth target, so this collector keeps that observable limit.
namespace party_skill_targets
{
constexpr std::size_t kCandidateCapacity = 12;

struct Candidate
{
    unsigned int id;
    int distance;
};

class Collector
{
public:
    // Keeps the first kCandidateCapacity members in party-list order.
    bool Add(unsigned int id, int distance)
    {
        if (m_count >= kCandidateCapacity)
            return false;
        m_items[m_count++] = Candidate{id, distance};
        return true;
    }

    std::size_t Count() const { return m_count; }

    // Orders the kept members nearest first (equal distances keep list order)
    // and passes at most min(count, maxTargets, outputCapacity) IDs to write.
    template <class Write>
    std::size_t Emit(int maxTargets, std::size_t outputCapacity, Write write)
    {
        std::stable_sort(m_items, m_items + m_count, [](const Candidate& a, const Candidate& b) {
            return a.distance < b.distance;
        });
        std::size_t limit = m_count;
        const std::size_t allowed = maxTargets > 0 ? static_cast<std::size_t>(maxTargets) : 0;
        if (allowed < limit)
            limit = allowed;
        if (outputCapacity < limit)
            limit = outputCapacity;
        for (std::size_t i = 0; i < limit; ++i)
            write(i, m_items[i].id);
        return limit;
    }

private:
    Candidate m_items[kCandidateCapacity]{};
    std::size_t m_count = 0;
};
}
