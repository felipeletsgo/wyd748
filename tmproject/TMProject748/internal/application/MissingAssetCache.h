#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace missing_asset {
class Cache {
    std::unordered_map<std::string, std::uint64_t> expires_;
    static std::string Key(const char* path) {
        std::string key(path ? path : "");
        for (char& c : key) c = c == '\\' ? '/' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return key;
    }
public:
    bool ShouldTry(const char* path, std::uint64_t now) {
        const auto found = expires_.find(Key(path));
        if (found == expires_.end()) return true;
        if (now < found->second) return false;
        expires_.erase(found);
        return true;
    }
    void Missing(const char* path, std::uint64_t now) {
        if (!path || !*path) return;
        if (expires_.size() >= 1024) {
            const auto oldest = std::min_element(expires_.begin(), expires_.end(),
                [](const auto& a, const auto& b) { return a.second < b.second; });
            expires_.erase(oldest);
        }
        expires_[Key(path)] = now + 5000;
    }
};
}
