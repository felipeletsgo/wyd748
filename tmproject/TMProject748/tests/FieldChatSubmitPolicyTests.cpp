#include "../internal/application/FieldChatSubmitPolicy.h"

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace
{
// Independent oracles copied from the original OnControlEvent chat-edit block.
bool LegacyFlood(unsigned int (&times)[4], unsigned int now)
{
    unsigned int last = times[3];
    times[3] = times[2];
    times[2] = times[1];
    times[1] = times[0];
    times[0] = now;
    return now - last < 4000;
}

template<std::size_t Width>
void LegacyRemember(char (&list)[5][Width], const char* text)
{
    if (list[0][0]) {
        if (std::strcmp(list[0], text)) {
            for (int j = 4; j > 0; --j)
                std::memcpy(list[j], list[j - 1], Width);
            std::memcpy(list[0], text, std::strlen(text) + 1);
        }
    } else {
        for (int k = 0; k < 5; ++k)
            std::memcpy(list[k], text, std::strlen(text) + 1);
    }
}

struct LegacyRoute { int kind; unsigned int color; int idx; int colorId; int start; };

LegacyRoute LegacyPrefix(const char* chat)
{
    unsigned int color = 0xFFFFAAAA;
    switch (chat[0]) {
    case '-': {
        color = 0xFFAAFFFF;
        int idx = 1, start = 1;
        if (chat[1] == '-') { color = 0xFF00FFFF; start = 2; idx = 2; }
        return {1, color, idx, 3, start};
    }
    case '=':
        return {2, 0xFFFF99FF, 2, 1, 1};
    case '@': {
        int idx = 0, start = 1;
        if (chat[1] == '@') { color = 0xF0F60AFF; idx = 3; start = 2; }
        else { color = 0xFF00AAFF; idx = 2; start = 1; }
        return {3, color, idx, 3, start};
    }
    case '/':
        return {4, color, 3, 0, 0};
    default:
        return {0, color, 0, 0, 0};
    }
}

int Kind(chat_submit::Prefix prefix)
{
    switch (prefix) {
    case chat_submit::Prefix::Dash:
    case chat_submit::Prefix::DoubleDash: return 1;
    case chat_submit::Prefix::Party: return 2;
    case chat_submit::Prefix::At:
    case chat_submit::Prefix::DoubleAt: return 3;
    case chat_submit::Prefix::Slash: return 4;
    default: return 0;
    }
}

const char* kTable[]{"spkx", "guildx", "handx", "outx", "warx", "lockx", "unlockx"};

// Returns the server keyword (or the original name) and early-return marker
// exactly as the original if/else chain would choose them.
std::string LegacyCommand(const char* str1, const char* const* table)
{
    if (!std::strcmp(str1, "summonguild")) return "summonguild";
    else if (!std::strcmp(str1, "king") || !std::strcmp(str1, "kingdom") ||
        !std::strcmp(str1, "King") || !std::strcmp(str1, "Kingdom")) return "kingdom";
    else if (!std::strcmp(str1, table[0])) return "spk";
    else if (!std::strcmp(str1, table[1])) return "create";
    else if (!std::strcmp(str1, table[2])) return "handover";
    else if (!std::strcmp(str1, table[3])) return "getout";
    else if (!std::strcmp(str1, table[4])) return "war";
    else if (!std::strcmp(str1, "srv")) return "srv";
    else if (!std::strcmp(str1, table[5])) return "item_lock";
    else if (!std::strcmp(str1, table[6])) return "item_unlock";
    else if (!std::strcmp(str1, "tab")) return "tab";
    return "whisper";
}

std::string Describe(chat_submit::Command command)
{
    if (const char* keyword = chat_submit::ServerKeyword(command))
        return keyword;
    switch (command) {
    case chat_submit::Command::SummonGuild: return "summonguild";
    case chat_submit::Command::Kingdom: return "kingdom";
    case chat_submit::Command::ServerQuery: return "srv";
    case chat_submit::Command::Nickname: return "tab";
    default: return "whisper";
    }
}

static_assert(chat_submit::CanHandOverGuild(9) && chat_submit::CanHandOverGuild(3) &&
    chat_submit::CanHandOverGuild(8) && !chat_submit::CanHandOverGuild(2) &&
    !chat_submit::CanHandOverGuild(10) && !chat_submit::CanHandOverGuild(0),
    "Guild handover level boundaries");
static_assert(chat_submit::ClassifyPrefix("--x").startIndex == 2, "Double dash skips two bytes");
}

int RunFieldChatSubmitPolicyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL field chat submit: %s\n", message);
        }
    };

    // Flood window, including unsigned wrap-around and the exact boundary.
    const unsigned int times[]{0u, 1u, 3999u, 4000u, 4001u, 0x7FFFFFFFu, 0xFFFFF000u, 0xFFFFFFFFu};
    for (unsigned int a : times)
        for (unsigned int b : times)
            for (unsigned int now : times) {
                unsigned int expected[4]{a, b, a ^ b, b + 7u};
                unsigned int actual[4]{a, b, a ^ b, b + 7u};
                const bool legacy = LegacyFlood(expected, now);
                check(chat_submit::RecordSubmission(actual, now) == legacy, "flood decision matches legacy");
                check(std::memcmp(actual, expected, sizeof(actual)) == 0, "flood history rotation matches legacy");
            }

    // Recall histories for both widths: first use, repeat, new entry and shifts.
    const char* entries[]{"hello", "hello", "world", "", "x", "world", "a much longer line"};
    char chat[5][128]{}, chatOracle[5][128]{};
    char whisper[5][16]{}, whisperOracle[5][16]{};
    for (const char* entry : entries) {
        chat_submit::Remember(chat, entry);
        LegacyRemember(chatOracle, entry);
        check(std::memcmp(chat, chatOracle, sizeof(chat)) == 0, "chat recall list matches legacy");
        char name[16]{};
        const std::size_t length = std::strlen(entry);
        std::memcpy(name, entry, length < 14 ? length : 14);
        chat_submit::Remember(whisper, name);
        LegacyRemember(whisperOracle, name);
        check(std::memcmp(whisper, whisperOracle, sizeof(whisper)) == 0, "whisper recall list matches legacy");
    }

    // Local commands: exact matches only, resource phrase first.
    const char* clear = "fps";
    check(chat_submit::ClassifyLocal("fps", clear) == chat_submit::LocalCommand::Clear,
        "resource phrase takes precedence over a built-in command");
    const std::pair<const char*, chat_submit::LocalCommand> locals[]{
        {"/help", chat_submit::LocalCommand::Help}, {"effects", chat_submit::LocalCommand::Effects},
        {"fps", chat_submit::LocalCommand::Fps}, {"effects2", chat_submit::LocalCommand::Effects2},
        {"effectold", chat_submit::LocalCommand::EffectOld}, {"exp", chat_submit::LocalCommand::Exp},
        {"/HELP", chat_submit::LocalCommand::None}, {"fps ", chat_submit::LocalCommand::None},
        {"effect", chat_submit::LocalCommand::None}, {"", chat_submit::LocalCommand::None}};
    for (const auto& local : locals)
        check(chat_submit::ClassifyLocal(local.first, "clear-phrase") == local.second,
            "local command classification is byte-exact");

    // Every first/second byte pair against the original prefix switch.
    for (int first = 1; first < 256; ++first)
        for (int second = 0; second < 256; ++second) {
            const char text[3]{static_cast<char>(first), static_cast<char>(second), 0};
            const auto legacy = LegacyPrefix(text);
            const auto route = chat_submit::ClassifyPrefix(text);
            check(Kind(route.prefix) == legacy.kind && route.color == legacy.color &&
                route.idx == legacy.idx && route.colorId == legacy.colorId &&
                route.startIndex == legacy.start, "prefix route matches legacy switch");
        }

    // Relocation aliases are checked before truncation.
    for (const char* name : {"relo", "Relocate", "relocate", "teleport-x"})
        check(chat_submit::IsRelocate(name, "teleport-x"), "relocation aliases");
    for (const char* name : {"Relo", "relo ", "RELOCATE", ""})
        check(!chat_submit::IsRelocate(name, "teleport-x"), "relocation rejects near matches");

    // Truncation keeps names under sixteen bytes and cuts longer ones to fourteen.
    for (std::size_t length = 0; length < 40; ++length) {
        char name[128]{};
        std::memset(name, 'n', length);
        chat_submit::TruncateCommandName(name);
        check(std::strlen(name) == (length >= 16 ? 14u : length), "command name truncation");
    }

    // Command routing over literals, table names and their collisions.
    const chat_submit::CommandNames names{kTable[0], kTable[1], kTable[2], kTable[3],
        kTable[4], kTable[5], kTable[6]};
    std::vector<std::string> inputs{"summonguild", "king", "kingdom", "King", "Kingdom",
        "KING", "srv", "tab", "r", "re", "someone", "", "summonguil"};
    for (const char* entry : kTable) inputs.push_back(entry);
    for (const auto& input : inputs)
        check(Describe(chat_submit::ClassifyCommand(input.c_str(), names)) ==
            LegacyCommand(input.c_str(), kTable), "command routing matches legacy order");
    const char* collisions[]{"srv", "tab", "king", "srv", "summonguild", "tab", "srv"};
    const chat_submit::CommandNames colliding{collisions[0], collisions[1], collisions[2],
        collisions[3], collisions[4], collisions[5], collisions[6]};
    for (const char* input : {"srv", "tab", "king", "summonguild"})
        check(Describe(chat_submit::ClassifyCommand(input, colliding)) ==
            LegacyCommand(input, collisions), "literal/table collisions keep legacy precedence");

    for (int level = -2; level < 12; ++level)
        check(chat_submit::CanHandOverGuild(level) == !(level != 9 && (level < 3 || level > 8)),
            "guild handover gate matches legacy");
    const float rates[]{-1.0f, 0.0f, 0.00001f, 0.5f, 0.89999998f, 0.8999999f, 0.9f, 1.0f};
    for (float rate : rates)
        check(chat_submit::IsBlockedByProgress(rate) == (rate < 0.89999998f && rate > 0.0f),
            "progress gate matches legacy");
    for (const char* name : {"r", "re", "R", "rr", "", "reply"})
        check(chat_submit::IsReplyAlias(name) == (!std::strcmp(name, "r") || !std::strcmp(name, "re")),
            "reply alias");

    return failures;
}
