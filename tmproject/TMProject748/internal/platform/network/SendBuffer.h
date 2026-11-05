#pragma once

#include <cstring>

// Local output-queue operations without WinSock or encryption dependencies.
// Buffers are borrowed; the caller guarantees the specified capacity.
namespace send_buffer
{
    // The raw path accepts exactly the remaining space and has no header.
    inline bool CanAppendRaw(int size, int queued, int capacity)
    {
        return size >= 0 && queued >= 0 && queued <= capacity &&
            size <= capacity - queued;
    }
// Preserve the strict legacy queue limit and WORD frame size.
// Validate indices before subtraction to avoid signed overflow.
inline bool CanAppendPacket(int size, int queued, int capacity, int headerSize)
{
    return headerSize > 0 && size >= headerSize && size <= 0xFFFF &&
        queued >= 0 && queued < capacity && size < capacity - queued;
}

// Discard only the sent prefix. memmove permits overlap. Remaining bytes are encrypted; never read them from the receive queue.
// Invalid state returns false without accessing memory or changing indices.
inline bool Compact(char* buffer, int capacity, int& queued, int& sent)
{
    if (!buffer || capacity <= 0 || queued < 0 || queued > capacity ||
        sent < 0 || sent > queued)
        return false;

    const int remaining = queued - sent;
    if (sent > 0 && remaining > 0)
        std::memmove(buffer, buffer + sent, static_cast<unsigned int>(remaining));
    queued = remaining;
    sent = 0;
    return true;
}

// Attempt the final flush even when enqueue fails, so earlier messages can proceed.
// Success requires both enqueue and flush acceptance, not remote delivery. Call each callback once, synchronously.
template <typename Enqueue, typename Flush>
bool EnqueueAndFlush(Enqueue enqueue, Flush flush)
{
    const bool accepted = enqueue() != 0;
    const bool flushed = flush() != 0;
    return accepted && flushed;
}
}
