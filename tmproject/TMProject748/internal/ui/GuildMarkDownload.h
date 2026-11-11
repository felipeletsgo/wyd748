#pragma once

#include <mutex>
#include <vector>
#include <cstddef>
#include <cstdint>

// Guild marks are uploaded by guild leaders on the player panel and served by
// the WYD-Go website. WYD.exe 7.48 read this address from strdef row 377
// (a retired wydglobal.com host); the client requests
// b<group:02><channel:02><guild:04>.bmp below it, a 16x12 24-bit BMP.
namespace guild_mark_download
{
    constexpr const char* Address = "https://wyd.forjago.com/patch/guildmark/";
    constexpr unsigned int TimeoutMs = 5000;
    constexpr std::size_t MaxBytes = 632;

    inline std::uint32_t Read32(const unsigned char* data)
    {
        return std::uint32_t(data[0]) | (std::uint32_t(data[1]) << 8) |
            (std::uint32_t(data[2]) << 16) | (std::uint32_t(data[3]) << 24);
    }

    // Match the ready-to-use BMP contract enforced by the player portal.
    inline bool IsValidBitmap(const char* data, std::size_t size)
    {
        if (!data || (size != 630 && size != MaxBytes))
            return false;
        const auto bytes = reinterpret_cast<const unsigned char*>(data);
        return bytes[0] == 'B' && bytes[1] == 'M' && Read32(bytes + 2) == size &&
            Read32(bytes + 10) == 54 && Read32(bytes + 14) == 40 &&
            Read32(bytes + 18) == 16 && Read32(bytes + 22) == 12 &&
            bytes[26] == 1 && bytes[27] == 0 && bytes[28] == 24 && bytes[29] == 0 &&
            Read32(bytes + 30) == 0;
    }

    // Read one extra byte to reject oversized bodies, including chunked responses.
    // A transport error after a valid prefix must still reject the whole response.
    template <typename Reader>
    std::size_t ReadBitmap(char* data, std::size_t capacity, Reader read)
    {
        if (!data || capacity < MaxBytes + 1)
            return 0;
        std::size_t size = 0;
        while (size < MaxBytes + 1)
        {
            const auto remaining = MaxBytes + 1 - size;
            const int received = read(data + size, remaining);
            if (received < 0 || static_cast<std::size_t>(received) > remaining)
                return 0;
            if (received == 0)
                return IsValidBitmap(data, size) ? size : 0;
            size += static_cast<std::size_t>(received);
        }
        return 0;
    }

    // A guild without a mark, or a failed download, is requested again after
    // five minutes, so a newly uploaded mark appears without a restart.
    constexpr unsigned int RetryMs = 300000;

    // Hands finished downloads from detached worker threads to the render
    // thread. Shared() is never destroyed, so a worker that finishes during
    // process exit cannot lock a destroyed mutex or append to a destroyed vector.
    template <typename Request>
    class CompletionQueue
    {
    public:
        // Takes ownership of request until Drain returns it.
        void Push(Request* request)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            done_.push_back(request);
        }

        // Returns every pushed request once, in push order.
        std::vector<Request*> Drain()
        {
            std::vector<Request*> done;
            std::lock_guard<std::mutex> lock(mutex_);
            done.swap(done_);
            return done;
        }

        static CompletionQueue& Shared()
        {
            static CompletionQueue* const queue = new CompletionQueue();
            return *queue;
        }

    private:
        std::mutex mutex_;
        std::vector<Request*> done_;
    };
}
