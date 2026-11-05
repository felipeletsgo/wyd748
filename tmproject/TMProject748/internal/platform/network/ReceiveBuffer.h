#pragma once

#include <cstddef>

namespace receive_buffer
{
    // Bytes after the receive window that recv never fills. A legacy consumer
    // can read a larger structure than its frame; for a frame that ends at the
    // window end, the zeroed slack keeps that access inside the heap block.
    // The slack covers the largest 16-bit frame size.
    constexpr int kConsumerSlack = 0x10000;

    // Allocation size for a receive window of the given capacity, or zero.
    inline std::size_t AllocationSize(int capacity)
    {
        return capacity > 0
            ? static_cast<std::size_t>(capacity) + static_cast<std::size_t>(kConsumerSlack)
            : 0;
    }

    inline bool HasValidWindow(int processed, int queued, int capacity)
    {
        return capacity > 0 && processed >= 0 && queued >= processed &&
            queued <= capacity;
    }

    // Validate a frame before advancing the cursor, without accessing the buffer.
    inline bool CanReadFrame(int size, int available, int minimumSize)
    {
        return size >= minimumSize && minimumSize > 0 && available >= 0 &&
            size <= available;
    }
}
