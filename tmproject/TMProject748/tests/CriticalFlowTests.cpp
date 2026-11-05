#include "../internal/application/FieldChatSubmitPolicy.h"
#include "../internal/app/scenes/InventoryCellSlots.h"
#include "../internal/app/scenes/PartySkillTargets.h"
#include "../internal/core/WYD748Assets.h"
#include "../internal/application/SceneSafetyPolicy.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>
#include <windows.h>

namespace {

std::string ReadProductFile(const char* relative)
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    std::vector<std::filesystem::path> roots{std::filesystem::current_path()};
    if (length && length < MAX_PATH)
        roots.push_back(std::filesystem::path(executable).parent_path());
    for (auto current : roots) {
        for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
            std::ifstream input(current / "tmproject/TMProject748" / relative, std::ios::binary);
            if (input)
                return std::string{std::istreambuf_iterator<char>(input), {}};
            current = current.parent_path();
        }
    }
    return {};
}

// Returns the depth-1 data members of `className` that have no default member
// initializer. Function declarations, statics, typedefs and access labels are
// ignored. A missing class is reported as a single "<class not found>" entry.
std::vector<std::string> MembersWithoutInitializer(const std::string& source, const char* className)
{
    std::vector<std::string> missing;
    const std::regex head(std::string("class\\s+") + className + "\\b[^;{]*\\{");
    std::smatch match;
    if (!std::regex_search(source, match, head)) {
        missing.push_back("<class not found>");
        return missing;
    }
    const std::regex skip("^(static|typedef|using|friend|enum|public|private|protected|struct|class|union)\\b");
    int depth = 1;
    std::string line;
    for (auto it = source.begin() + match.position(0) + match.length(0); it != source.end() && depth > 0; ++it) {
        if (*it != '\n') {
            line += *it;
            continue;
        }
        std::string code = line.substr(0, line.find("//"));
        line.clear();
        const int opens = static_cast<int>(std::count(code.begin(), code.end(), '{'));
        const int closes = static_cast<int>(std::count(code.begin(), code.end(), '}'));
        const int memberDepth = depth;
        depth += opens - closes;
        if (memberDepth != 1 || opens || closes)
            continue;
        const auto first = code.find_first_not_of(" \t\r");
        const auto last = code.find_last_not_of(" \t\r");
        if (first == std::string::npos)
            continue;
        code = code.substr(first, last - first + 1);
        if (code.back() != ';' || code.find_first_of("()") != std::string::npos || std::regex_search(code, skip))
            continue;
        if (code.find('=') == std::string::npos && code.find('{') == std::string::npos)
            missing.push_back(code);
    }
    return missing;
}

// A02-02/A03-02: TMFieldScene is allocated with `new TMFieldScene()` and its
// user-provided constructors do not zero storage. Every data member of the
// scene and its base must carry a default initializer so that a recycled heap
// block cannot leave pointers, indices or timers indeterminate.
int RunSceneMemberInitializerChecks(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const std::string& name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name.c_str());
        }
    };
    const struct { const char* path; const char* name; } classes[] = {
        {"internal/app/scenes/TMFieldScene.h", "TMFieldScene"},
        {"internal/app/scenes/TMScene.h", "TMScene"},
    };
    for (const auto& target : classes) {
        const auto source = ReadProductFile(target.path);
        check(!source.empty(), std::string(target.path) + " is readable");
        if (source.empty())
            continue;
        const auto missing = MembersWithoutInitializer(source, target.name);
        for (const auto& member : missing)
            check(false, std::string(target.name) + " member without initializer: " + member);
        check(missing.empty(), std::string(target.name) + " data members all have initializers");
    }

    const std::string synthetic =
        "class Sample : public Base\n{\npublic:\n    Sample();\n    static int s_count;\n"
        "    int m_ready{};\n    char m_text[4];\n    struct Nested\n    {\n        int inner;\n    };\n"
        "    void* m_pointer = nullptr;\n    int Method(int first,\n        int second);\n};\n";
    const auto sample = MembersWithoutInitializer(synthetic, "Sample");
    check(sample.size() == 1 && sample[0] == "char m_text[4];",
        "the member scanner reports only the uninitialized depth-1 member");
    check(MembersWithoutInitializer(synthetic, "Missing").size() == 1,
        "the member scanner reports a missing class instead of passing");
    return failures;
}

// A02-01: the chat edit recalls history rows. A row is user text and must be
// copied literally; the old path passed it to sprintf_s as the format.
int RunChatRecallChecks(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    char history[5][128]{};
    chat_submit::Remember(history, "100%");
    chat_submit::Remember(history, "%n%s%s%d");
    char out[128]{};
    int next = chat_submit::Recall(history, 0, out);
    check(std::strcmp(out, "%n%s%s%d") == 0 && next == 1,
        "recall copies a row with conversion specifiers literally");
    next = chat_submit::Recall(history, next, out);
    check(std::strcmp(out, "100%") == 0 && next == 2,
        "recall copies a trailing percent literally");
    next = chat_submit::Recall(history, 4, out);
    check(next == 0, "recall index wraps after the last row");

    std::memcpy(out, "stale", 6);
    next = chat_submit::Recall(history, -7, out);
    check(std::strcmp(out, "%n%s%s%d") == 0 && next == 1,
        "a negative recall index restarts at row zero");
    next = chat_submit::Recall(history, 5, out);
    check(std::strcmp(out, "%n%s%s%d") == 0 && next == 1,
        "an index past the list restarts at row zero");

    char unterminated[5][128];
    std::memset(unterminated, 'x', sizeof(unterminated));
    char narrow[16];
    std::memset(narrow, 'q', sizeof(narrow));
    chat_submit::Recall(unterminated, 0, narrow);
    check(narrow[15] == '\0' && std::strlen(narrow) == 15,
        "an unterminated row is truncated to the output buffer");
    chat_submit::Recall(unterminated, 0, out);
    check(out[127] == '\0' && std::strlen(out) == 127,
        "a full unterminated row stays inside the row and the output");

    char longText[300];
    std::memset(longText, 'y', sizeof(longText) - 1);
    longText[sizeof(longText) - 1] = '\0';
    char bounded[3][8]{};
    chat_submit::Remember(bounded, longText);
    check(bounded[0][7] == '\0' && bounded[2][7] == '\0' && std::strlen(bounded[1]) == 7,
        "a text longer than a row is truncated on first use");
    chat_submit::Remember(bounded, "abc");
    check(std::strcmp(bounded[0], "abc") == 0 && std::strlen(bounded[1]) == 7,
        "a new entry shifts the rows down");
    chat_submit::Remember(bounded, "abc");
    check(std::strcmp(bounded[1], "yyyyyyy") == 0,
        "repeating the newest entry does not shift the rows");
    chat_submit::Remember(bounded, longText);
    check(std::strlen(bounded[0]) == 7 && std::strcmp(bounded[1], "abc") == 0,
        "a long entry is stored truncated after a short one");
    return failures;
}

// A05-01: party skills 29/44 collected in-range members into a 12-entry stack
// array; a 13-member party wrote the 13th entry past it. The collector keeps
// the observable limit of the legacy loop (first 12 in-range members, nearest
// first) without writing outside its storage.
int RunPartySkillTargetChecks(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };
    struct Dam { unsigned int id; int damage; };

    for (const unsigned int members : {12u, 13u, 14u}) {
        party_skill_targets::Collector targets;
        unsigned int accepted = 0;
        for (unsigned int i = 0; i < members; ++i)
            accepted += targets.Add(1000 + i, static_cast<int>(members - i)) ? 1 : 0;
        Dam dam[14]{};
        for (auto& d : dam)
            d.id = 0xDEADu;
        const auto written = targets.Emit(13, 13, [&](std::size_t i, unsigned int id) {
            dam[i].id = id;
            dam[i].damage = -1;
        });
        check(accepted == 12 && targets.Count() == 12, "at most 12 party members are kept");
        check(written == 12 && dam[12].id == 0xDEADu && dam[13].id == 0xDEADu,
            "no target is written past the twelfth even with MaxTarget 13");
        check(dam[0].id == 1011 && dam[11].id == 1000 && dam[0].damage == -1,
            "the kept members are ordered nearest first");
    }

    party_skill_targets::Collector ties;
    ties.Add(7, 3);
    ties.Add(5, 1);
    ties.Add(9, 3);
    ties.Add(4, 1);
    unsigned int order[4]{};
    check(ties.Emit(13, 4, [&](std::size_t i, unsigned int id) { order[i] = id; }) == 4 &&
        order[0] == 5 && order[1] == 4 && order[2] == 7 && order[3] == 9,
        "equal distances keep party-list order");

    party_skill_targets::Collector limited;
    for (unsigned int i = 0; i < 6; ++i)
        limited.Add(i + 1, static_cast<int>(i));
    unsigned int out[6]{};
    check(limited.Emit(2, 6, [&](std::size_t i, unsigned int id) { out[i] = id; }) == 2 &&
        out[0] == 1 && out[1] == 2 && out[2] == 0, "MaxTarget limits the targets");
    check(limited.Emit(13, 3, [&](std::size_t i, unsigned int id) { out[i] = id; }) == 3,
        "the output capacity limits the targets");
    check(limited.Emit(-4, 6, [&](std::size_t, unsigned int) {}) == 0,
        "a negative MaxTarget writes no target");

    party_skill_targets::Collector empty;
    check(empty.Emit(13, 13, [&](std::size_t, unsigned int) {}) == 0,
        "no in-range member writes no target");
    return failures;
}

// A26-01/A04-04: every default-item grid reported wire type 1, and its cell
// was projected with the Carry formula. A skill icon at cell (0,0) therefore
// named Carry slot 0 in sale, extraction, swap and mix requests.
int RunInventoryCellSlotChecks(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };
    using namespace inventory_cell_slots;
    check(CarrySlot(true, 0, 0) == 0 && CarrySlot(true, 8, 0) == 8 &&
        CarrySlot(true, 0, 1) == 9 && CarrySlot(true, 8, 6) == 62,
        "Carry cells map row-major to the 63 visible slots");
    check(CarrySlot(false, 0, 0) == -1 && CarrySlot(false, 3, 2) == -1,
        "a presentation grid cell has no Carry slot");
    check(CarrySlot(true, 9, 0) == -1 && CarrySlot(true, 0, 7) == -1 &&
        CarrySlot(true, -1, 0) == -1 && CarrySlot(true, 0, -1) == -1,
        "cells outside the 9x7 Carry surface have no slot");
    check(CargoSlot(true, 0, 0) == 0 && CargoSlot(true, 8, 12) == 116 &&
        CargoSlot(true, 2, 13) == 119,
        "Cargo cells map row-major to the 120 usable slots");
    check(CargoSlot(true, 3, 13) == -1 && CargoSlot(true, 0, 14) == -1 &&
        CargoSlot(true, 9, 0) == -1 && CargoSlot(true, -1, 0) == -1,
        "cells past the usable Cargo surface have no slot");
    check(CargoSlot(false, 0, 0) == -1,
        "a non-Cargo grid cell has no Cargo slot (mix staging from Cargo, A04-04)");
    int mapped = 0;
    for (int y = 0; y < kCarryRows; ++y)
        for (int x = 0; x < kColumns; ++x)
            mapped += CarrySlot(true, x, y) == x + kColumns * y ? 1 : 0;
    check(mapped == kVisibleCarrySlots, "every visible Carry cell keeps its native slot");
    return failures;
}

// A13-09, A02-10, A03-12 and A26-07: load each shipped locale with the
// production loader. Keep the assigned scene rows separate from item rows.
int RunKillLabelLocaleChecks(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const std::string& name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name.c_str());
        }
    };
    const struct { const char* relative; const char* label; const char* split; const char* entrance; } packs[] = {
        {"../client748/UI/UIString.txt", "Kills", "Enter the quantity to split.", "Use this Entrance Ticket to enter?"},
        {"../client748/EN/UI/UIString.txt", "Kills", "Enter the quantity to split.", "Use this Entrance Ticket to enter?"},
        {"../client748/PT/UI/UIString.txt", "Abates", "Digite a quantidade para dividir.", "Usar este ingresso para entrar?"},
    };
    for (const auto& pack : packs) {
        static char strings[500][64];
        std::memset(strings, 0, sizeof(strings));
        std::filesystem::path path;
        for (auto current = std::filesystem::current_path(); !current.empty(); current = current.parent_path()) {
            if (std::filesystem::exists(current / "tmproject/TMProject748" / pack.relative)) {
                path = current / "tmproject/TMProject748" / pack.relative;
                break;
            }
            if (current == current.parent_path())
                break;
        }
        check(!path.empty(), std::string(pack.relative) + " is present");
        if (path.empty())
            continue;
        const bool loaded = WYD748_LoadUIStrings(path.string().c_str(), &strings[0][0], 500, 64);
        check(loaded, std::string(pack.relative) + " loads through the client loader");
        check(loaded && std::strcmp(strings[240], pack.label) == 0,
            std::string(pack.relative) + " row 240 holds the kill label");
        check(loaded && std::strcmp(strings[241], pack.split) == 0 &&
            std::strcmp(strings[242], pack.entrance) == 0,
            std::string(pack.relative) + " rows 241-242 hold the item prompts in the selected locale");
        const int sceneRows[] = {ui_text::kShowExpOn, ui_text::kShowExpOff,
            ui_text::kNetStatsShown, ui_text::kNetStatsHidden, ui_text::kMonsterClass,
            ui_text::kCombatOff, ui_text::kCombatSupport, ui_text::kCombatPhysical,
            ui_text::kCombatMagic, ui_text::kMoveFree, ui_text::kMoveCycle, ui_text::kMoveFixed,
            ui_text::kCombatTitle, ui_text::kCombatHint, ui_text::kPotionHint,
            ui_text::kMountFeedHint, ui_text::kMovementHint, ui_text::kCombatConfigHint,
            ui_text::kCharacterNameTooShort, ui_text::kCharacterNameTooLong,
            ui_text::kCharacterNameLetters, ui_text::kQuizCountdown, ui_text::kTotoMatch,
            ui_text::kTotoTeamA, ui_text::kTotoTeamB, ui_text::kTotoRemaining,
            ui_text::kCatalogItemFallback, ui_text::kNellComplete, ui_text::kNellRejected,
            ui_text::kNellWaiting, ui_text::kNellUnavailable};
        bool present = loaded;
        bool unique = true;
        for (std::size_t index = 0; index < sizeof(sceneRows) / sizeof(sceneRows[0]); ++index)
        {
            const int row = sceneRows[index];
            present = present && strings[row][0] != '\0';
            unique = unique && (row < 240 || row > 242);
            for (std::size_t earlier = 0; earlier < index; ++earlier)
                unique = unique && row != sceneRows[earlier];
        }
        check(present && unique, std::string(pack.relative) + " supplies distinct scene, combat and item rows");
        const bool portuguese = std::string(pack.relative).find("/PT/") != std::string::npos;
        check(std::strcmp(strings[ui_text::kQuizCountdown], portuguese ? "%s  (10 segundos)" : "%s  (10 seconds)") == 0 &&
            std::strcmp(strings[ui_text::kTotoRemaining], portuguese ? "Restam %d" : "%d Left") == 0 &&
            std::strcmp(strings[ui_text::kCatalogItemFallback], "Item %d") == 0,
            std::string(pack.relative) + " preserves countdown and item format arguments");
        check(std::strcmp(strings[ui_text::kShowExpOn], portuguese ? "Exibir EXP: LIGADO" : "Show Exp : ON") == 0 &&
            std::strcmp(strings[ui_text::kShowExpOff], portuguese ? "Exibir EXP: DESLIGADO" : "Show Exp : OFF") == 0,
            std::string(pack.relative) + " binds both EXP states without the kill or item captions");
    }
    return failures;
}

} // namespace

int RunCriticalFlowTests(int& checks)
{
    int failures = 0;
    failures += RunKillLabelLocaleChecks(checks);
    failures += RunInventoryCellSlotChecks(checks);
    failures += RunPartySkillTargetChecks(checks);
    failures += RunChatRecallChecks(checks);
    failures += RunSceneMemberInitializerChecks(checks);
    return failures;
}
