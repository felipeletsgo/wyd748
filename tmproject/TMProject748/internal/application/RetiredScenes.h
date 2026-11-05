#pragma once

#include <cstddef>
#include <vector>

// Scenes replaced by a transition and not yet deleted. The scene tree root owns
// them until ObjectManager::CleanUp deletes them. C-13: the manager kept only
// one previous scene, so a second transition before CleanUp left the older
// scene linked under the root until shutdown.
template<class Scene>
class RetiredScenes
{
public:
    void Retire(Scene* scene)
    {
        if (scene)
            m_scenes.push_back(scene);
    }

    // Detaches the whole list. A scene destructor that re-enters CleanUp then
    // sees an empty list, so each retired scene is deleted exactly once.
    std::vector<Scene*> TakeAll()
    {
        std::vector<Scene*> taken;
        taken.swap(m_scenes);
        return taken;
    }

    // At shutdown the root deletes every child; drop the borrowed pointers first.
    void Forget() { m_scenes.clear(); }

    std::size_t Size() const { return m_scenes.size(); }

private:
    std::vector<Scene*> m_scenes;
};
