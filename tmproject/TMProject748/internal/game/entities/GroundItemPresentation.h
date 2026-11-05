#pragma once

// Ground-item presentation decisions without Direct3D or scene dependencies.
namespace ground_item_view
{
    // WYD-Go owns ground-item visibility. It removes an item with 0x16F when
    // the player's position leaves its view and recreates it only after that
    // removal. A distant movement target therefore only clears the hover state.
    constexpr int kTargetHoverRange = 18;

    inline bool IsOutsideTargetHoverRange(int targetX, int targetY, float itemX, float itemY)
    {
        return static_cast<float>(targetX - kTargetHoverRange) > itemX ||
            itemX > static_cast<float>(targetX + kTargetHoverRange) ||
            static_cast<float>(targetY - kTargetHoverRange) > itemY ||
            itemY > static_cast<float>(targetY + kTargetHoverRange);
    }

    // Coin bundles store the amount as EF_HWORDCOIN * 256 + EF_LWORDCOIN.
    inline int CoinAmount(int highWord, int lowWord)
    {
        return highWord * 256 + lowWord;
    }

    // Lock a vertex buffer and pass its vertices to `write`. A failed lock or a
    // null pointer leaves memory unchanged; a successful lock is always released.
    template <class Vertex, class Buffer, class Write>
    bool WithLockedVertices(Buffer* buffer, unsigned int bytes, Write&& write)
    {
        if (!buffer)
            return false;
        Vertex* vertices = nullptr;
        if (buffer->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0) < 0)
            return false;
        if (!vertices)
        {
            buffer->Unlock();
            return false;
        }
        write(vertices, static_cast<int>(bytes / sizeof(Vertex)));
        buffer->Unlock();
        return true;
    }
}
