#pragma once

#include <cstddef>
#include <cstring>

// Local chat-submission decisions from the native field-scene chat edit.
// No transport, controls, entities or ownership: the scene keeps every side
// effect, its order and its return value. Text matching is byte-exact, as in
// the original strcmp/strncmp chains; it is not server-side command validation.
namespace chat_submit
{
constexpr unsigned int kFloodWindow = 4000;

// Rotates the four-submission history before the check, as the scene did.
inline bool RecordSubmission(unsigned int (&times)[4], unsigned int now)
{
    const unsigned int oldest = times[3];
    times[3] = times[2];
    times[2] = times[1];
    times[1] = times[0];
    times[0] = now;
    return now - oldest < kFloodWindow;
}

// Recent-entry recall list. Fills every row on first use; otherwise shifts
// complete rows down and stores the entry only when it differs from row zero.
// The caller guarantees that the text fits the row, as the original copy did.
template<std::size_t Rows, std::size_t Width>
void Remember(char (&history)[Rows][Width], const char* text)
{
    const std::size_t size = std::strlen(text) + 1;
    if (history[0][0]) {
        if (std::strcmp(history[0], text)) {
            for (std::size_t row = Rows - 1; row > 0; --row)
                std::memcpy(history[row], history[row - 1], Width);
            std::memcpy(history[0], text, size);
        }
        return;
    }
    for (std::size_t row = 0; row < Rows; ++row)
        std::memcpy(history[row], text, size);
}

enum class LocalCommand { None, Clear, Help, Effects, Fps, Effects2, EffectOld, Exp };

// The resource phrase is compared first, preserving the original precedence.
inline LocalCommand ClassifyLocal(const char* text, const char* clearPhrase)
{
    if (!std::strcmp(text, clearPhrase)) return LocalCommand::Clear;
    if (!std::strcmp(text, "/help")) return LocalCommand::Help;
    if (!std::strcmp(text, "effects")) return LocalCommand::Effects;
    if (!std::strcmp(text, "fps")) return LocalCommand::Fps;
    if (!std::strcmp(text, "effects2")) return LocalCommand::Effects2;
    if (!std::strcmp(text, "effectold")) return LocalCommand::EffectOld;
    if (!std::strcmp(text, "exp")) return LocalCommand::Exp;
    return LocalCommand::None;
}

enum class Prefix { Plain, Dash, DoubleDash, Party, At, DoubleAt, Slash };

struct Route
{
    Prefix prefix;
    unsigned int color;
    int idx;
    int colorId;
    int startIndex;
};

constexpr unsigned int kPlainColor = 0xFFFFAAAA;
constexpr unsigned int kCommandColor = 0xFFFFFF00;

// Prefix routing of the first one or two bytes. Party membership, list
// insertion and packet construction remain caller decisions.
constexpr Route ClassifyPrefix(const char* chat)
{
    switch (chat[0]) {
    case '-':
        return chat[1] == '-' ? Route{Prefix::DoubleDash, 0xFF00FFFF, 2, 3, 2}
                              : Route{Prefix::Dash, 0xFFAAFFFF, 1, 3, 1};
    case '=':
        return Route{Prefix::Party, 0xFFFF99FF, 2, 1, 1};
    case '@':
        return chat[1] == '@' ? Route{Prefix::DoubleAt, 0xF0F60AFF, 3, 3, 2}
                              : Route{Prefix::At, 0xFF00AAFF, 2, 3, 1};
    case '/':
        return Route{Prefix::Slash, kPlainColor, 3, 0, 0};
    default:
        return Route{Prefix::Plain, kPlainColor, 0, 0, 0};
    }
}

// Evaluated on the untruncated command name, before whisper preparation.
inline bool IsRelocate(const char* name, const char* localizedRelocate)
{
    return !std::strcmp(name, "relo") || !std::strcmp(name, localizedRelocate) ||
        !std::strcmp(name, "Relocate") || !std::strcmp(name, "relocate");
}

// Names of sixteen or more bytes are cut to fourteen, matching the scene.
inline void TruncateCommandName(char* name)
{
    if (std::strlen(name) >= 16) {
        name[15] = 0;
        name[14] = 0;
    }
}

enum class Command
{
    Whisper, SummonGuild, Kingdom, Speaker, GuildCreate, GuildHandover,
    GuildExpel, GuildWar, ServerQuery, ItemLock, ItemUnlock, Nickname
};

// Localized command names from the message-string table, by original index.
struct CommandNames
{
    const char* speaker;       // 389
    const char* guildCreate;   // 386
    const char* guildHandover; // 387
    const char* guildExpel;    // 391
    const char* guildWar;      // 390
    const char* itemLock;      // 496
    const char* itemUnlock;    // 497
};

// Evaluated on the truncated name, in the original if/else order.
inline Command ClassifyCommand(const char* name, const CommandNames& names)
{
    if (!std::strcmp(name, "summonguild")) return Command::SummonGuild;
    if (!std::strcmp(name, "king") || !std::strcmp(name, "kingdom") ||
        !std::strcmp(name, "King") || !std::strcmp(name, "Kingdom"))
        return Command::Kingdom;
    if (!std::strcmp(name, names.speaker)) return Command::Speaker;
    if (!std::strcmp(name, names.guildCreate)) return Command::GuildCreate;
    if (!std::strcmp(name, names.guildHandover)) return Command::GuildHandover;
    if (!std::strcmp(name, names.guildExpel)) return Command::GuildExpel;
    if (!std::strcmp(name, names.guildWar)) return Command::GuildWar;
    if (!std::strcmp(name, "srv")) return Command::ServerQuery;
    if (!std::strcmp(name, names.itemLock)) return Command::ItemLock;
    if (!std::strcmp(name, names.itemUnlock)) return Command::ItemUnlock;
    if (!std::strcmp(name, "tab")) return Command::Nickname;
    return Command::Whisper;
}

// Server-side command keyword written into the whisper target, or null when
// the typed name is sent unchanged. These bytes are the existing wire values.
constexpr const char* ServerKeyword(Command command)
{
    switch (command) {
    case Command::Speaker: return "spk";
    case Command::GuildCreate: return "create";
    case Command::GuildHandover: return "handover";
    case Command::GuildExpel: return "getout";
    case Command::GuildWar: return "war";
    case Command::ItemLock: return "item_lock";
    case Command::ItemUnlock: return "item_unlock";
    default: return nullptr;
    }
}

// Guild handover is allowed only for level 9 or levels 3 through 8.
constexpr bool CanHandOverGuild(int guildLevel)
{
    return guildLevel == 9 || (guildLevel >= 3 && guildLevel <= 8);
}

// Commands that wait while a cast or action progress bar is partially filled.
constexpr bool IsBlockedByProgress(float progressRate)
{
    return progressRate < 0.89999998f && progressRate > 0.0f;
}

inline bool IsReplyAlias(const char* name)
{
    return !std::strcmp(name, "r") || !std::strcmp(name, "re");
}
}
