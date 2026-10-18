#pragma once

namespace tree_attachment
{
    // Walk descendants without consuming call stack; do not leave this subtree.
    // The visitor may update payload, but must not change the tree links.
    template<class Node, class Visit>
    void ForEachDescendant(Node* root, Visit visit)
    {
        Node* node = root ? root->m_pDown : nullptr;
        while (node)
        {
            visit(node);
            if (node->m_pDown) { node = node->m_pDown; continue; }
            while (node && node != root && !node->m_pNextLink) node = node->m_pTop;
            if (!node || node == root) break;
            node = node->m_pNextLink;
        }
    }

    template<class Node, class Owner>
    void ClearOwner(Node* root, Owner* owner)
    {
        ForEachDescendant(root, [owner](Node* node) {
            if (!owner || node->m_pOwner == owner) node->m_pOwner = nullptr;
        });
    }

    template<class Node>
    bool CanAdopt(const Node* parent, const Node* child)
    {
        if (!child || parent == child) return false;
        const Node* slow = parent;
        const Node* fast = parent;
        for (const Node* ancestor = parent; ancestor; ancestor = ancestor->m_pTop)
        {
            if (ancestor == child) return false;
            slow = slow ? slow->m_pTop : nullptr;
            fast = fast && fast->m_pTop ? fast->m_pTop->m_pTop : nullptr;
            if (slow && slow == fast) return false;
        }
        return true;
    }
}
