#pragma once
#include <cstddef>

// Non-owning per-frame geometry lists. Unlink before destroying a control.
namespace render_control_list
{
    template<class List>
    void Clear(List& list)
    {
        auto* current = list.pHeadGeom;
        list.pHeadGeom = list.pTailGeom = nullptr;
        while (current)
        {
            auto* next = current->m_pNextGeom;
            current->m_pNextGeom = nullptr;
            current = next;
        }
    }

    template<class Cleanup>
    class FrameCleanup
    {
        Cleanup cleanup;
    public:
        explicit FrameCleanup(Cleanup action) : cleanup(action) {}
        FrameCleanup(const FrameCleanup&) = delete;
        FrameCleanup& operator=(const FrameCleanup&) = delete;
        ~FrameCleanup() { cleanup(); }
    };

    template<class List, class Node>
    bool Remove(List& list, Node* node)
    {
        if (!node) return false;
        Node* previous = nullptr;
        for (Node* current = list.pHeadGeom; current; current = current->m_pNextGeom)
        {
            if (current == node)
            {
                if (previous) previous->m_pNextGeom = current->m_pNextGeom;
                else list.pHeadGeom = current->m_pNextGeom;
                if (list.pTailGeom == current) list.pTailGeom = previous;
                current->m_pNextGeom = nullptr;
                return true;
            }
            previous = current;
        }
        return false;
    }

    template<class List, class Node>
    bool Append(List& list, Node* node)
    {
        if (!node) return false;
        // Reject duplicate insertion instead of creating a self-link/cycle.
        for (Node* current = list.pHeadGeom; current; current = current->m_pNextGeom)
            if (current == node) return false;
        node->m_pNextGeom = nullptr;
        if (list.pHeadGeom) list.pTailGeom->m_pNextGeom = node;
        else list.pHeadGeom = node;
        list.pTailGeom = node;
        return true;
    }

    template<class List, class Node>
    bool AppendUnique(List* lists, std::size_t count, std::size_t layer, Node* node)
    {
        if (!lists || !node || layer >= count) return false;
        // A geometry node has one link, so it may belong to only one layer.
        for (std::size_t i = 0; i < count; ++i)
            for (Node* current = lists[i].pHeadGeom; current; current = current->m_pNextGeom)
                if (current == node) return false;
        return Append(lists[layer], node);
    }

    template<class List, class Node>
    bool RemoveAny(List* lists, std::size_t count, Node* node)
    {
        if (!lists || !node) return false;
        for (std::size_t i = 0; i < count; ++i)
            if (Remove(lists[i], node)) return true;
        return false;
    }
}
