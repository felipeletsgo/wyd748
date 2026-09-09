#pragma once

namespace receive_buffer
{
    inline bool HasValidWindow(int processed, int queued, int capacity)
    {
        return capacity > 0 && processed >= 0 && queued >= processed &&
            queued <= capacity;
    }

    // Valida um frame antes de avancar o cursor; nao acessa o buffer.
    inline bool CanReadFrame(int size, int available, int minimumSize)
    {
        return size >= minimumSize && minimumSize > 0 && available >= 0 &&
            size <= available;
    }
}
