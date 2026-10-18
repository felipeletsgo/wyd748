#pragma once

namespace actor_lifetime
{
    template<class Actor, class Scene, class SceneType>
    bool MayDelayDelete(const Actor* actor, const Scene* scene, SceneType fieldType)
    {
        // The local field actor remains alive for HUD, resurrection and logout.
        return !scene || scene->m_eSceneType != fieldType || scene->m_pMyHuman != actor;
    }
}
