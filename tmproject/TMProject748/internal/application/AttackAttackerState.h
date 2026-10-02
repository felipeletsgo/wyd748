#pragma once

namespace attack_attacker
{
// Decisions and synchronous state application for an already resolved attacker.
// No entity lookup, scene ownership, transport, timers or effect allocation.
constexpr int SwingForce(int meshType, int actorClass, int leftWeaponType)
{
    if (meshType == 3 || meshType == 8 || actorClass == 40) return 5;
    if (leftWeaponType == 101) return 2;
    if (leftWeaponType == 41) return 3;
    if (leftWeaponType == 103) return 4;
    return 1;
}

template<class Swing>
void ApplySwing(Swing* first, Swing* second, int force, unsigned int startTime)
{
    if (first) {
        first->m_cSForce = static_cast<decltype(first->m_cSForce)>(force);
        first->m_dwStartTime = startTime;
    }
    if (second) {
        second->m_cSForce = static_cast<decltype(second->m_cSForce)>(force);
        second->m_dwStartTime = startTime;
    }
}

constexpr bool ShouldApplyMana(bool localAttacker, int localFlag, int skillIndex)
{
    return (!localAttacker || localFlag == 0) && skillIndex >= 0 && skillIndex < 104;
}

// The callback borrows its inputs only until return. It updates the local player
// snapshot/UI after actor mana, and is never invoked for remote or rejected state.
template<class LocalApply>
bool ApplyMana(bool localAttacker, int localFlag, int skillIndex,
    unsigned short receivedMana, unsigned int& actorMana, LocalApply&& applyLocal)
{
    if (!ShouldApplyMana(localAttacker, localFlag, skillIndex)) return false;
    actorMana = receivedMana;
    if (localAttacker) applyLocal(receivedMana);
    return true;
}

constexpr bool ShouldDispatchVisuals(bool localAttacker, int localFlag,
    unsigned char motion)
{
    return !localAttacker || localFlag == 1 || (localFlag == 0 && motion == 254);
}
}
