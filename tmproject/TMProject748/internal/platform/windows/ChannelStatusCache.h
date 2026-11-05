#pragma once
#include "ChannelStatusRequest.h"
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace channel_status {
struct Completion { std::string url; bool success; double milliseconds; };
class Cache {
    using Clock = std::chrono::steady_clock;
    struct Entry { std::string text; Clock::time_point updated{}; bool pending = false; };
    std::mutex mutex_;
    std::condition_variable ready_;
    std::unordered_map<std::string, Entry> entries_;
    std::deque<std::string> queue_;
    std::vector<Completion> completed_;
    unsigned long long revision_ = 0;
    void Work() {
        for (;;) {
            std::unique_lock<std::mutex> lock(mutex_);
            ready_.wait(lock, [this] { return !queue_.empty(); });
            const std::string url = std::move(queue_.front());
            queue_.pop_front();
            lock.unlock();
            char text[1024]{};
            const auto start = Clock::now();
            const bool success = Fetch(url.c_str(), text, sizeof text) != 0;
            const auto end = Clock::now();
            lock.lock();
            auto& entry = entries_.at(url);
            entry.text = success ? text : "";
            entry.updated = end;
            entry.pending = false;
            ++revision_;
            if (completed_.size() < 64)
                completed_.push_back({url, success, std::chrono::duration<double, std::milli>(end - start).count()});
        }
    }
public:
    Cache() {
        // The process owns these workers. They never borrow scenes, controls,
        // timers, render state, or diagnostics. Shutdown never waits on DNS.
        std::thread([this] { Work(); }).detach();
        std::thread([this] { Work(); }).detach();
    }
    int Read(const char* url, char* output, int capacity) {
        if (!output || capacity <= 0) return 0;
        output[0] = 0;
        if (!url || !*url || std::strlen(url) > 2048) return 0;
        std::lock_guard<std::mutex> lock(mutex_);
        auto found = entries_.find(url);
        if (found == entries_.end()) {
            if (entries_.size() >= 32) return 0;
            found = entries_.emplace(url, Entry{}).first;
        }
        auto& entry = found->second;
        if (!entry.pending && Clock::now() - entry.updated >= std::chrono::seconds(5)) {
            entry.pending = true;
            queue_.emplace_back(url);
            ready_.notify_one();
        }
        const auto count = (std::min)(entry.text.size(), static_cast<std::size_t>(capacity - 1));
        std::memcpy(output, entry.text.data(), count);
        output[count] = 0;
        return count != 0;
    }
    template<class Record> unsigned long long Poll(Record record) {
        std::vector<Completion> completed;
        unsigned long long revision;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            completed.swap(completed_);
            revision = revision_;
        }
        // The UI thread alone records performance samples.
        for (const auto& result : completed) record(result);
        return revision;
    }
};
inline Cache& StatusCache() {
    // Process lifetime avoids static teardown while an Internet call is active.
    static Cache* cache = new Cache;
    return *cache;
}
}
