#include "../internal/application/SceneSafetyPolicy.h"
#include "../internal/application/ChatListRows.h"
#include "../internal/ui/UIBinary.h"

#include <climits>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{
std::filesystem::path RepositoryFile(const char* relative)
{
    wchar_t executable[MAX_PATH]{};
    if (!GetModuleFileNameW(nullptr, executable, MAX_PATH))
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 9 && !current.empty(); ++depth) {
        const auto file = current / relative;
        if (std::filesystem::exists(file))
            return file;
        current = current.parent_path();
    }
    return {};
}

std::string Source(const char* relative)
{
    std::ifstream file(RepositoryFile(relative), std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
}

int RunFieldSceneSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };
    // A02-11: short scene gates and countdowns retain their exact boundaries.
    check(!scene_countdown::IsPast(5000, 0, 5000) && scene_countdown::IsPast(5001, 0, 5000),
        "A02-11 the five-second deadline remains exclusive");
    check(scene_countdown::Remaining(1000, 1000, 5000, 1000) == 5 &&
        scene_countdown::Remaining(7000, 1000, 5000, 1000) == 0 &&
        scene_countdown::Remaining(1001, 1000, 5000, 0) == 0,
        "A02-11 completed countdowns and a zero period cannot underflow");
    const unsigned int beforeWrap = 0xFFFFFF00u;
    check(scene_countdown::Remaining(beforeWrap + 4000u, beforeWrap, 5000, 1000) == 1 &&
        scene_countdown::IsPast(beforeWrap + 6001u, beforeWrap, 6000),
        "A02-11 the same countdown completes across tick wrap");
    check(scene_countdown::Elapsed(1000, 1100) == 0 &&
        !scene_countdown::IsGateClosed(10, 0, 6000) &&
        scene_countdown::IsGateClosed(1000, 1100, 6000),
        "A02-11 an unset gate stays open and a small clock correction waits");

    // A02-04: resolve both identity fields on confirmation, without a raw alias.
    const auto known = [](unsigned int id) -> const char* { return id == 7 ? "Player" : nullptr; };
    check(guild_prompt::ResolveTarget(7, "Player", known) != nullptr &&
        guild_prompt::ResolveTarget(8, "Player", known) == nullptr &&
        guild_prompt::ResolveTarget(7, "Renamed", known) == nullptr &&
        guild_prompt::ResolveTarget(0, "Player", known) == nullptr,
        "A02-04 confirmation rejects a missing, renamed or unset guild target");

    // A02-03/A03-07/A16-03: a list receives exactly one item per nonempty row.
    struct Row { std::string text; };
    struct List {
        std::vector<Row> rows;
        void AddItem(Row row) { rows.push_back(row); }
    } list;
    int created = 0;
    const auto make = [&](const char* row) { ++created; return Row{row}; };
    char head[8]{};
    const char* tail = chat_rows::Split("abcdefghijkl", chat_rows::CutAt(7, false), head);
    check(std::strcmp(head, "abcdef") == 0 && std::strcmp(tail, "ghijkl") == 0 &&
        chat_rows::Add(&list, head, tail, make) == 2 && created == 2 && list.rows.size() == 2,
        "chat wrapping keeps both rows without duplicating an item or abandoning a row");
    check(chat_rows::Add(static_cast<List*>(nullptr), "a", "b", make) == 0 && created == 2 &&
        chat_rows::Add(&list, "short", "", make) == 1 && created == 3,
        "chat wrapping creates no second item for an empty tail or a null list");

    // A06-07: counts use only complete camera records and never exceed storage.
    check(camera_path::RequestedRecords(-1, 10) == 0 &&
        camera_path::RequestedRecords(INT_MAX, 10) == 10 &&
        camera_path::UsableRecords(10, 3) == 3 && camera_path::UsableRecords(0, 10) == 0,
        "A06-07 empty, oversized and short camera paths cannot expose missing records");

    // A06-08: an oversized rectangle is clipped while preserving grid phase.
    const auto span = env_effect_area::Clip(-1000, INT_MAX, 4, 1920, 2176);
    check(span.first >= 1920 && span.end == 2176 && (span.first + 1000) % 4 == 0 &&
        env_effect_area::Count(span, 4) <= 64 &&
        env_effect_area::Count(env_effect_area::Clip(2200, 3000, 4, 1920, 2176), 4) == 0,
        "A06-08 allocation counts stay bounded by the visible height window");

    // A07-07: a short line must not expose bytes from a previous long line.
    char line[64] = "FFFFFF example\n";
    check(message_text::LineBody(line, 6) && std::strcmp(line + 6, " example") == 0,
        "A07-07 text starts at a validated color separator");
    strcpy_s(line, "A");
    check(message_text::LineBody(line, 6) == nullptr,
        "A07-07 a reused short line does not return stale text past its terminator");

    bool masteryBounded = true;
    for (int cls = -1; cls <= 4; ++cls)
        for (int skill = -1; skill <= 248; ++skill)
        {
            const int index = mastery_index::ForSkill(skill, cls);
            const bool ownClass = cls >= 0 && cls <= 3 && skill >= 24 * cls && skill < 24 * cls + 24;
            masteryBounded = masteryBounded && (ownClass ? index >= 1 && index <= 3 : index == -1);
        }
    check(masteryBounded, "A02-12 only the actor's class skills index Mastery 1..3");

    const char notice[] = "Kingdom Chatting : Off";
    const auto toggle = channel_notice::Classify(notice);
    check(toggle.channel == channel_notice::Channel::Kingdom && !toggle.on &&
        std::strcmp(notice, "Kingdom Chatting : Off") == 0 &&
        channel_notice::Classify("Kingdom Chatting : Off extra").channel == channel_notice::Channel::None,
        "A06-01 channel toggles classify exact text without rewriting a panel buffer");
    nightmare_clock::Time nightmare{};
    check(nightmare_clock::Parse("!!235959", nightmare) && nightmare.hour == 23 &&
        nightmare.minute == 59 && nightmare.second == 59 &&
        nightmare_clock::Parse("!!000000", nightmare) && nightmare.hour == 0,
        "A06-10 exact server clock replies preserve midnight and the last second of the day");
    check(!nightmare_clock::Parse("!test", nightmare) &&
        !nightmare_clock::Parse("x!test", nightmare) &&
        !nightmare_clock::Parse("!!246000", nightmare) &&
        !nightmare_clock::Parse("!!120000 extra", nightmare) &&
        !nightmare_clock::Parse("!!12a000", nightmare),
        "A06-10 ordinary exclamation notices cannot be consumed as clock replies");

    unsigned char create[351]{};
    create[kCreateMobServerOffset] = 9;
    create[kCreateMobTradeServerOffset] = 3;
    check(create_mob_view::Citizen(create, sizeof(create), MSG_CreateMobTrade_Opcode) == 3 &&
        create_mob_view::Citizen(create, sizeof(create), MSG_CreateMob_Opcode) == 9 &&
        create_mob_view::Citizen(create, kCreateMobTradeServerOffset, MSG_CreateMobTrade_Opcode) == 0,
        "A06-02 trade spawn reads citizenship after the shop title and rejects a short frame");

    const auto disabled = ground_side::Demand(0.0f, false, false);
    const auto enabled = ground_side::Demand(1.0f, false, true);
    check(!disabled.Near() && ground_neighbor::ShouldAttach(enabled, disabled),
        "A06-04 a disabled border cannot starve an enabled neighboring field");
    check(character_name::HasOnlyAsciiLetters("Player") &&
        !character_name::HasOnlyAsciiLetters("Player1") && !character_name::HasOnlyAsciiLetters(""),
        "A07-09 character naming uses the current server's ASCII letter contract");

    // A07-06: use the actual shipped RC, then remove each required control.
    const auto resource = RepositoryFile("tmproject/client748/UI/SelCharScene2.bin");
    FILE* stream = nullptr;
    _wfopen_s(&stream, resource.c_str(), L"rb");
    std::vector<unsigned int> controls;
    bool complete = stream != nullptr;
    while (stream) {
        int type = 0;
        const auto tag = ReadRCControlType(stream, type);
        if (tag == RCControlTypeReadResult::End)
            break;
        const auto bytes = RCRecordSize(type, false);
        unsigned int id = 0;
        if (tag != RCControlTypeReadResult::Record || bytes < sizeof(id) ||
            fread(&id, sizeof(id), 1, stream) != 1 ||
            fseek(stream, static_cast<long>(bytes - sizeof(id)), SEEK_CUR) != 0) {
            complete = false;
            break;
        }
        controls.push_back(id);
    }
    if (stream)
        fclose(stream);
    const auto found = [&](unsigned int id) {
        return std::find(controls.begin(), controls.end(), id) != controls.end();
    };
    check(complete && !controls.empty() && required_controls::FirstMissing(
        required_controls::kCharacterSelection, found) == 0,
        "A07-06 the required controls exist in the shipped character selection resource");
    for (const unsigned int missing : required_controls::kCharacterSelection)
        check(required_controls::FirstMissing(required_controls::kCharacterSelection,
            [&](unsigned int id) { return id != missing && found(id); }) == missing,
            "A07-06 each missing required control rejects the scene before use");

    const auto packets = Source("tmproject/TMProject748/internal/app/scenes/TMSelectCharScenePackets.cpp");
    const auto snapshot = packets.find("&login->MOB, sizeof(login->MOB)");
    const auto equipment = packets.find("ClearOutOfCatalog(g_pObjectManager->m_stMobData.Equip", snapshot);
    const auto carry = packets.find("ClearOutOfCatalog(g_pObjectManager->m_stMobData.Carry", snapshot);
    const auto transition = packets.find("TM_FIELD_STATE", snapshot);
    check(snapshot != std::string::npos && equipment < transition && carry < transition,
        "A09-10 login clears invalid catalog indices before entering the field");
    const auto deleted = packets.find("void TMSelectCharScene::HandleCharacterDeleted");
    check(packets.find("m_cCharacterSlot = -1", deleted) < packets.find("OnControlEvent(4616", deleted),
        "A07-05 deletion clears selection before the controls and models reload");
    const auto scene = Source("tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp");
    const auto portal = scene.find("ExtractedFlow TMFieldScene::OnPortalButton");
    const auto nextMethod = scene.find("int TMFieldScene::OnCharEvent", portal);
    const auto portalBody = scene.substr(portal, nextMethod - portal);
    check(portal != std::string::npos && nextMethod != std::string::npos &&
        portalBody.find("PickupItem") == std::string::npos &&
        portalBody.find("BASE_SetItemAmount") == std::string::npos,
        "A02-08 opening a portal countdown cannot consume an inventory item");
    const auto delayedUse = scene.find("m_pendingPortalItem.Take(slot,");
    check(delayedUse != std::string::npos &&
        scene.find("SendOneMessage((char*)&m_stUseItem", delayedUse) <
        scene.find("BASE_SetItemAmount", delayedUse),
        "A02-08 delayed consumption follows snapshot validation and queue acceptance");
    return failures;
}
