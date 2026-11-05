#pragma once
#include <cstddef>

namespace tree_cleanup {
// The root and ancestors outlive the sweep. Destructors can remove peers.
// A peer removal changes the destruction counter and invalidates the saved next.
template<class Node, class Counter, class Dispose>
std::size_t Sweep(Node* root, Counter destroyed, Dispose dispose)
{
    if (!root) return 0;
    std::size_t visits = 0;
    Node* node = root->m_pDown;
    while (node && node != root) {
        ++visits;
        if (node->m_cDeleted) {
            Node* parent = node->m_pTop;
            Node* next = node->m_pNextLink;
            const auto before = destroyed();
            dispose(node);
            if (destroyed() - before != 1) {
                // Re-read live links after a subtree or peer destructor ran.
                node = parent;
                if (node == root) node = root->m_pDown;
                continue;
            }
            if (next) { node = next; continue; }
            node = parent;
        } else if (node->m_pDown) {
            node = node->m_pDown;
            continue;
        }
        while (node && node != root && !node->m_pNextLink) node = node->m_pTop;
        if (node && node != root) node = node->m_pNextLink;
    }
    return visits;
}
}
