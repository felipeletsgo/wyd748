#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

namespace wydpatch {

// Shared between the update thread and the window; the window polls it.
struct UpdateProgress {
    std::atomic<std::uint64_t> done{0};
    std::atomic<std::uint64_t> total{0};

    void SetStatus(std::wstring text)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        status_ = std::move(text);
    }
    std::wstring Status()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return status_;
    }

private:
    std::mutex mutex_;
    std::wstring status_;
};

enum class UpdateResult {
    UpToDate,     // every manifest file is verified; the game may start
    SelfUpdated,  // the launcher replaced itself and must restart
    Failed,
    Cancelled,
};

struct UpdateOptions {
    std::wstring clientDirectory;  // directory that holds the launcher and the game
    std::wstring launcherPath;     // full path of the running launcher
    bool repair = false;           // ignore patch.cache and rehash every file
    bool allowSelfUpdate = true;   // false right after a self-update restart
};

// Downloads /patch/manifest.txt from the release host, verifies the client
// directory against it and installs every missing or changed file. Every
// download is checked against its SHA-256 before it replaces anything.
UpdateResult RunUpdate(const UpdateOptions& options, UpdateProgress& progress,
    const std::atomic<bool>& cancel, std::wstring& error);

} // namespace wydpatch
