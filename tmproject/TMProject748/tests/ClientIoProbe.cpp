#include "../internal/platform/windows/ChannelStatusCache.h"
#include "../internal/application/MissingAssetCache.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstdlib>

int main(int argc, char** argv)
{
    if ((argc == 3 || argc == 4) && std::strcmp(argv[1], "--async") == 0) {
        char body[1024]{};
        auto& cache = channel_status::StatusCache();
        const auto start = std::chrono::steady_clock::now();
        double maximum = 0;
        for (int i = 0; i < 10000; ++i) {
            const auto before = std::chrono::steady_clock::now();
            cache.Read(argv[2], body, sizeof body);
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - before).count();
            if (ms > maximum) maximum = ms;
        }
        std::printf("async-read,10000,max_ms=%.3f,total_ms=%.3f\n", maximum,
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
        if (maximum > 100) return 1;
        unsigned int completions = 0;
        bool success = false;
        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(15)) {
            cache.Poll([&](const channel_status::Completion& result) {
                ++completions;
                success = result.success;
                std::printf("async-complete,%d,%.3f\n", result.success, result.milliseconds);
            });
            if (completions) {
                cache.Read(argv[2], body, sizeof body);
                struct Buffer { char text[4]; char canary; } bounded{{1, 1, 1, 1}, 'Z'};
                cache.Read(argv[2], bounded.text, sizeof bounded.text);
                if (bounded.canary != 'Z' || !std::memchr(bounded.text, 0, sizeof bounded.text)) return 1;
                std::printf("async-cached,%s\n", body);
                if (argc == 4 && (success != (std::strcmp(argv[3], "-") != 0) ||
                    (success && std::strcmp(body, argv[3]) != 0))) return 1;
                return completions == 1 ? 0 : 1;
            }
            Sleep(10);
        }
        return 1;
    }
    if (argc == 3 && std::strcmp(argv[1], "--missing-cache") == 0) {
        missing_asset::Cache cache;
        FILE* file = nullptr;
        if (fopen_s(&file, argv[2], "rb") == 0) { fclose(file); return 2; }
        unsigned int opens = 0;
        for (int i = 0; i < 10000; ++i) {
            if (!cache.ShouldTry(argv[2], 0)) continue;
            ++opens;
            if (fopen_s(&file, argv[2], "rb") == 0) { fclose(file); return 1; }
            cache.Missing(argv[2], 0);
        }
        if (opens != 1 || fopen_s(&file, argv[2], "wb") != 0) return 1;
        std::fputs("recovered", file);
        std::fclose(file);
        const bool expired = cache.ShouldTry(argv[2], 5000);
        if (!expired || fopen_s(&file, argv[2], "rb") != 0) return 1;
        std::fclose(file);
        std::printf("missing-cache,attempts=10000,opens=%u,recovered=%d\n", opens, expired);
        return 0;
    }
    if (argc != 4) return 2;
    char body[1024]{};
    const auto start = std::chrono::steady_clock::now();
    const int result = channel_status::Fetch(argv[1], body, sizeof body);
    const double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    std::printf("channel,%d,%.3f,%zu\n", result, elapsed, std::strlen(body));
    const int expected = std::atoi(argv[3]);
    if (expected >= 0 && result != expected) return 1;
    struct Buffer { char text[4]; char canary; } bounded{{1, 1, 1, 1}, 'Z'};
    channel_status::Fetch(argv[1], bounded.text, sizeof bounded.text);
    if (bounded.canary != 'Z' || !std::memchr(bounded.text, 0, sizeof bounded.text)) return 1;
    if (channel_status::Fetch(nullptr, body, sizeof body) || channel_status::Fetch(argv[1], body, 0)) return 1;
    constexpr int attempts = 10000;
    int misses = 0;
    const auto ioStart = std::chrono::steady_clock::now();
    for (int i = 0; i < attempts; ++i) {
        FILE* file = nullptr;
        fopen_s(&file, argv[2], "rb");
        if (file) { fclose(file); return 1; }
        ++misses;
    }
    const double ioMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - ioStart).count();
    std::printf("missing-open,%d,%.3f,%.6f\n", misses, ioMs, ioMs / attempts);
    return 0;
}
