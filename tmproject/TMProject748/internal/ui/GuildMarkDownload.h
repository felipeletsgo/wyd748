#pragma once

#include <mutex>
#include <vector>

// Guild marks are uploaded by guild leaders on the player panel and served by
// the WYD-Go website. WYD.exe 7.48 read this address from strdef row 377
// (a retired wydglobal.com host); the client requests
// b<group:02><channel:02><guild:04>.bmp below it, a 16x12 24-bit BMP.
namespace guild_mark_download
{
    constexpr const char* Address = "http://wyd.forjago.com/patch/guildmark/";

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
