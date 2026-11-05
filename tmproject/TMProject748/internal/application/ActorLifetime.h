#pragma once

namespace actor_lifetime
{
    // Human containers own only direct human children. Clear borrowed targets
    // before retirement, while every sibling and its event remain allocated.
    template<class Actor, class Node>
    void ClearPendingEffectTargets(Actor* target, Node* container)
    {
        if (!target || !container)
            return;
        for (Node* node = container->m_pDown; node; node = node->m_pNextLink)
        {
            auto* actor = static_cast<Actor*>(node);
            if (actor->m_stEffectEvent.pTarget == target)
                actor->m_stEffectEvent.pTarget = nullptr;
        }
    }

    template<class Actor, class Scene, class SceneType>
    bool MayDelayDelete(const Actor* actor, const Scene* scene, SceneType fieldType)
    {
        // The local field actor remains alive for HUD, resurrection and logout.
        return !scene || scene->m_eSceneType != fieldType || scene->m_pMyHuman != actor;
    }
}
