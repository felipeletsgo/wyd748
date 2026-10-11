#pragma once

#include <memory>
#include <vector>

template<typename Node>
struct RCChildCheckpoint
{
    Node* parent;
    Node* firstChild;
};

// RC construction only prepends new children; it never reparents or removes
// existing controls. Snapshot the old child heads before the first mutation.
// Deleting a new subtree unlinks it through TreeNode's virtual destructor.
template<typename Node, typename Allocator = std::allocator<RCChildCheckpoint<Node>>>
class RCControlTransaction
{
public:
    explicit RCControlTransaction(Node* root, const Allocator& allocator = Allocator())
        : checkpoints_(allocator)
    {
        for (Node* node = root; node;)
        {
            checkpoints_.push_back({node, static_cast<Node*>(node->m_pDown)});
            if (node->m_pDown)
            {
                node = static_cast<Node*>(node->m_pDown);
                continue;
            }
            while (node != root && !node->m_pNextLink)
                node = static_cast<Node*>(node->m_pTop);
            node = node == root ? nullptr : static_cast<Node*>(node->m_pNextLink);
        }
    }

    RCControlTransaction(const RCControlTransaction&) = delete;
    RCControlTransaction& operator=(const RCControlTransaction&) = delete;

    ~RCControlTransaction() noexcept
    {
        if (committed_)
            return;
        // Rollback allocates nothing, even when construction failed for lack
        // of memory. Old parents remain alive throughout this traversal.
        for (const auto& checkpoint : checkpoints_)
            while (checkpoint.parent->m_pDown != checkpoint.firstChild)
                delete static_cast<Node*>(checkpoint.parent->m_pDown);
    }

    void Commit() noexcept { committed_ = true; }

private:
    std::vector<RCChildCheckpoint<Node>, Allocator> checkpoints_;
    bool committed_ = false;
};
