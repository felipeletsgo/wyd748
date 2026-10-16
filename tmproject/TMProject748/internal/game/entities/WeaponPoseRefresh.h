#pragma once

namespace weapon_pose {

// InitObject selects an animation before CheckWeapon updates the equipment
// stance. Rebind that animation when the stance changes on the rebuilt skin.
template <typename Human>
void Select(Human& human, int weaponType)
{
    if (human.m_nWeaponTypeIndex == weaponType)
        return;

    human.m_nWeaponTypeIndex = weaponType;
    if (human.m_pSkinMesh)
        human.SetAnimation(human.m_eMotion, human.m_nLoop);
}

} // namespace weapon_pose
