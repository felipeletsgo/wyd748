#pragma once

#include <windows.h>
#include <cstddef>
#include <cstring>
#include "../app/scenes/GroundNeighborPolicy.h"
#include "../wire/CreateMobContract.h"

// Small scene decisions without controls, devices or transport. The scenes
// keep every side effect; these helpers only validate and bound their input.

// Countdowns and input gates use 32-bit server-time stamps, which wrap after
// about 49.7 days. Compare elapsed differences, never `stamp + delay`.
namespace scene_countdown
{
// A stamp up to this many ms ahead of `now` (server-time drift correction)
// counts as zero elapsed time. A stamp farther ahead is treated as old.
constexpr unsigned int kAheadTolerance = 6000;

inline unsigned int Elapsed(unsigned int now, unsigned int stamp)
{
    const unsigned int elapsed = now - stamp;
    return elapsed > 0u - kAheadTolerance ? 0u : elapsed;
}

// True when more than `delay` ms passed since `stamp`.
inline bool IsPast(unsigned int now, unsigned int stamp, unsigned int delay)
{
    return Elapsed(now, stamp) > delay;
}

// Whole `unit` periods left before `delay` ends; zero after it ends.
inline unsigned int Remaining(unsigned int now, unsigned int stamp, unsigned int delay, unsigned int unit)
{
    const unsigned int elapsed = Elapsed(now, stamp);
    return elapsed >= delay || unit == 0 ? 0u : (delay - elapsed) / unit;
}

// An input gate is closed while a set (nonzero) stamp is younger than `window`.
inline bool IsGateClosed(unsigned int now, unsigned int stamp, unsigned int window)
{
    return stamp != 0 && Elapsed(now, stamp) < window;
}
}

// Sub-guild name prompt: the target is captured when the prompt opens and is
// resolved again on OK. The interaction pointer can be cleared or moved to
// another player while the prompt is open.
namespace guild_prompt
{
// Returns the stored name while a player with the same ID and name is known;
// otherwise null. `findName(id)` returns the known player's name or null.
template <typename FindName>
const char* ResolveTarget(unsigned int targetId, const char* storedName, FindName findName)
{
    if (!targetId || !storedName || !storedName[0])
        return nullptr;
    const char* current = findName(targetId);
    return current && std::strcmp(current, storedName) == 0 ? storedName : nullptr;
}
}

// Camera path files: a 4-byte record count, then 28-byte records.
namespace camera_path
{
constexpr std::size_t kRecordBytes = 28;

// Records to read: the declared count limited to [0, capacity].
inline int RequestedRecords(int declared, int capacity)
{
    if (declared < 0)
        return 0;
    return declared > capacity ? capacity : declared;
}

// Records to use: only the complete records that were actually read.
inline int UsableRecords(int requested, std::size_t recordsRead)
{
    if (requested <= 0)
        return 0;
    return recordsRead < static_cast<std::size_t>(requested) ? static_cast<int>(recordsRead) : requested;
}
}

// 0x3A2 environment effect: one effect per grid point of a server rectangle.
// Only the 256 x 256 height window around the player has terrain data, so the
// grid is clipped to that window before effects are allocated.
namespace env_effect_area
{
constexpr int kHeightWindow = 256;

struct Span
{
    int first;
    int end; // positions first, first + step, ... below end
};

// Keeps the grid phase (begin + k * step) and limits it to [windowBegin, windowEnd).
inline Span Clip(int begin, int end, int step, int windowBegin, int windowEnd)
{
    long long first = begin;
    if (step > 0 && first < windowBegin)
        first += (static_cast<long long>(windowBegin) - first + step - 1) / step * step;
    const long long last = end < windowEnd ? end : windowEnd;
    if (first > last)
        first = last;
    return Span{static_cast<int>(first), static_cast<int>(last)};
}

inline int Count(const Span& span, int step)
{
    return step > 0 && span.first < span.end ? (span.end - span.first + step - 1) / step : 0;
}
}

// Message-text files: each line has a fixed prefix (6-byte color, or a
// 10-byte "level color" field), one space, then the text.
namespace message_text
{
// Removes the line end and returns the text after the prefix (starting at the
// separating space), or null when the line has no such text. The length is
// measured first, so bytes left in the buffer by a longer earlier line are
// never used.
inline char* LineBody(char* line, std::size_t prefixLength)
{
    if (!line)
        return nullptr;
    if (char* lineEnd = std::strchr(line, '\n'))
        *lineEnd = '\0';
    if (std::strlen(line) <= prefixLength || line[prefixLength] != ' ')
        return nullptr;
    return line + prefixLength;
}
}

namespace create_mob_view
{
// Citizenship (cape emblem) byte of a create-mob frame. 0x364 keeps it at
// byte 326. The coordinated 0x363 trade frame keeps the shop title at bytes
// 326..349 and the citizenship at byte 350.
inline unsigned char Citizen(const unsigned char* frame, std::size_t size, unsigned int type)
{
    const std::size_t offset = type == MSG_CreateMobTrade_Opcode
        ? kCreateMobTradeServerOffset : kCreateMobServerOffset;
    return frame && offset < size ? frame[offset] : 0;
}
}

namespace mastery_index
{
// Mastery[] slot (1..3) for one of the character's own 24 class skills, or -1
// for a skill of another class or a skill outside the class range.
inline int ForSkill(int skillId, int characterClass)
{
    if (characterClass < 0 || characterClass > 3 || skillId < 0 || skillId >= 96)
        return -1;
    const int classSkill = skillId - 24 * characterClass;
    if (classSkill < 0 || classSkill >= 24)
        return -1;
    return classSkill / 8 + 1;
}
}

namespace diagnostic_switch
{
// Opt-in values of a diagnostic environment variable.
inline bool IsOnValue(const char* value)
{
    return value && (!_stricmp(value, "1") || !_stricmp(value, "true") ||
        !_stricmp(value, "yes") || !_stricmp(value, "on"));
}

inline bool IsEnabled(const char* name)
{
    char value[8]{};
    const DWORD length = GetEnvironmentVariableA(name, value, static_cast<DWORD>(sizeof(value)));
    return length > 0 && length < sizeof(value) && IsOnValue(value);
}
}

namespace external_link
{
// The client opens only HTTPS web addresses. Plain HTTP and other schemes are
// refused; spaces, quotes and control bytes are refused too.
inline bool IsHttps(const char* url)
{
    if (!url || _strnicmp(url, "https://", 8) != 0)
        return false;
    const char* host = url + 8;
    if (!host[0] || host[0] == '/' || host[0] == '.')
        return false;
    for (const char* p = url; *p; ++p)
    {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c <= ' ' || c >= 0x7F || c == '"')
            return false;
    }
    return true;
}
}

namespace ui_text
{
// A locale text row, or the English fallback when the row is absent or empty.
template <std::size_t Rows, std::size_t Width>
const char* RowOr(const char (&table)[Rows][Width], int index, const char* fallback)
{
    return index >= 0 && static_cast<std::size_t>(index) < Rows && table[index][0] ? table[index] : fallback;
}

// Locale rows (UI/UIString.txt in the root, EN and PT packs) for client text
// that had no locale resource. An empty row uses the English fallback.
// Rows 240-242 hold the kill label and item prompts.
constexpr int kShowExpOn = 243;
constexpr int kShowExpOff = 244;
constexpr int kNetStatsShown = 245;
constexpr int kNetStatsHidden = 246;
constexpr int kMonsterClass = 247;
constexpr int kCriticalDataError = 248;
constexpr int kDataFileNotFound = 249;
constexpr int kFileLostTitle = 250;
constexpr int kCombatOff = 229;
constexpr int kCombatSupport = 230;
constexpr int kMoveFixed = 232;
constexpr int kMoveFree = 233;
constexpr int kMoveCycle = 234;
constexpr int kCombatTitle = 235;
constexpr int kCombatHint = 236;
constexpr int kPotionHint = 237;
constexpr int kMountFeedHint = 238;
constexpr int kMovementHint = 239;
constexpr int kCombatConfigHint = 251;
constexpr int kCombatPhysical = 252;
constexpr int kCombatMagic = 253;
constexpr int kCharacterNameTooShort = 254;
constexpr int kCharacterNameTooLong = 255;
constexpr int kCharacterNameLetters = 256;
constexpr int kQuizCountdown = 257;
constexpr int kTotoMatch = 258;
constexpr int kTotoTeamA = 259;
constexpr int kTotoTeamB = 260;
constexpr int kTotoRemaining = 261;
constexpr int kCatalogItemFallback = 262;
constexpr int kNellComplete = 263;
constexpr int kNellRejected = 264;
constexpr int kNellWaiting = 265;
constexpr int kNellUnavailable = 266;
}

namespace character_name
{
// WYD-Go accepts only ASCII letters in character names (validCharacterName);
// the length limits stay with the caller.
inline bool HasOnlyAsciiLetters(const char* name)
{
    if (!name || !name[0])
        return false;
    for (const char* p = name; *p; ++p)
        if ((*p < 'A' || *p > 'Z') && (*p < 'a' || *p > 'z'))
            return false;
    return true;
}
}

namespace required_controls
{
// Controls dereferenced by the shipped character selection flow. Optional
// controls from later resource versions stay outside this set.
constexpr unsigned int kCharacterSelection[] = {
    1282, 1296, 1312, 1542, 1545, 1562, 1563, 1564, 1565,
    4612, 4613, 4615, 4616, 4626, 5654, 5666, 5667, 5668,
    5669, 5670, 5671
};

// The first ID that `find(id)` cannot resolve, or 0 when every ID resolves.
template <std::size_t Count, typename Find>
unsigned int FirstMissing(const unsigned int (&ids)[Count], Find find)
{
    for (const unsigned int id : ids)
        if (!find(id))
            return id;
    return 0;
}
}

namespace ground_side
{
// Neighbor demand for one border. A border whose neighbor field is disabled
// in the attach table cannot be attached, so it must not compete with the
// other axis (otherwise it starves an enabled neighbor near a corner).
inline ground_neighbor::Need Demand(float distance, bool attached, bool enabled)
{
    ground_neighbor::Need need;
    if (enabled)
    {
        need.distance = distance;
        need.attached = attached;
    }
    return need;
}
}

namespace channel_notice
{
// Exact 0x101 notices that toggle a chat channel. The panel shows the text;
// only the channel state changes. The text is never rewritten in place.
enum class Channel { None, Whisper, Citizen, Guild, Kingdom };

struct Toggle
{
    Channel channel;
    bool on;
};

inline Toggle Classify(const char* text)
{
    struct Row { const char* text; Channel channel; bool on; };
    static const Row rows[] = {
        {"Whisper : Off", Channel::Whisper, false}, {"Whisper : On", Channel::Whisper, true},
        {"Citizen Chatting : Off", Channel::Citizen, false}, {"Citizen Chatting : On", Channel::Citizen, true},
        {"Guild Chatting : Off", Channel::Guild, false}, {"Guild Chatting : On", Channel::Guild, true},
        {"Kingdom Chatting : On", Channel::Kingdom, true}, {"Kingdom Chatting : Off", Channel::Kingdom, false},
    };
    if (text)
        for (const Row& row : rows)
            if (!std::strcmp(text, row.text))
                return Toggle{row.channel, row.on};
    return Toggle{Channel::None, false};
}
}

namespace nightmare_clock
{
struct Time { unsigned short hour; unsigned short minute; unsigned short second; };

// WYD-Go nightmareTimeMessage emits exactly "!!HHMMSS". Other punctuation
// belongs to ordinary notice text and cannot reset the ticket clock.
inline bool Parse(const char* text, Time& time)
{
    if (!text || std::strlen(text) != 8 || text[0] != '!' || text[1] != '!')
        return false;
    for (int i = 2; i < 8; ++i)
        if (text[i] < '0' || text[i] > '9')
            return false;
    Time value{static_cast<unsigned short>((text[2] - '0') * 10 + text[3] - '0'),
        static_cast<unsigned short>((text[4] - '0') * 10 + text[5] - '0'),
        static_cast<unsigned short>((text[6] - '0') * 10 + text[7] - '0')};
    if (value.hour > 23 || value.minute > 59 || value.second > 59)
        return false;
    time = value;
    return true;
}
}
