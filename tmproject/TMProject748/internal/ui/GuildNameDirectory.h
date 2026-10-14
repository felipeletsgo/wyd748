#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace guild_name_748 {
// Guilds.txt: group, channel, local guild ID, name. Native FUN_00557593
// uses a 12-byte name cell and a channel/12-bit-guild lookup.
class Directory {
public:
    void Load(std::istream& input)
    {
        names_.clear();
        std::string line;
        while (std::getline(input, line)) {
            std::istringstream fields(line);
            int group = -1, channel = -1, guild = -1;
            std::string name;
            if (!(fields >> group >> channel >> guild >> name) || group < 0 || group >= 10
                || channel < 0 || channel >= 16 || guild <= 0 || guild >= 4096)
                continue;
            names_[Key(group, (channel << 12) | guild)] = name.substr(0, 11);
        }
    }

    std::string Find(int group, unsigned int packedGuild) const
    {
        if (group < 0 || group >= 10 || packedGuild > 65535 || !(packedGuild & 4095))
            return {};
        const auto entry = names_.find(Key(group, packedGuild));
        return entry == names_.end() ? std::string{} : entry->second;
    }

private:
    static unsigned int Key(int group, unsigned int packedGuild)
    {
        return (static_cast<unsigned int>(group) << 16) | packedGuild;
    }
    std::unordered_map<unsigned int, std::string> names_;
};

class FileDirectory {
public:
    void Refresh(const std::filesystem::path& path)
    {
        std::error_code error;
        const auto stamp = std::filesystem::last_write_time(path, error);
        const auto size = error ? 0 : std::filesystem::file_size(path, error);
        if (error) {
            std::istringstream empty;
            directory_.Load(empty);
            loaded_ = false;
            return;
        }
        if (loaded_ && path == path_ && stamp == stamp_ && size == size_)
            return;
        std::ifstream input(path);
        directory_.Load(input); // Missing/unreadable files never retain another server's names.
        loaded_ = static_cast<bool>(input) || input.eof();
        path_ = path;
        stamp_ = stamp;
        size_ = size;
    }

    std::string Find(int group, unsigned int packedGuild) const
    {
        return directory_.Find(group, packedGuild);
    }

private:
    Directory directory_;
    bool loaded_ = false;
    std::filesystem::path path_;
    std::filesystem::file_time_type stamp_{};
    std::uintmax_t size_ = 0;
};
}
