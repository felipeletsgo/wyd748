#include "SourceMethod.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
std::string ReadFieldUnit(const char* filename)
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / "tmproject/TMProject748/internal/app/scenes" / filename;
        std::ifstream input(candidate, std::ios::binary);
        if (input) {
            std::string source{std::istreambuf_iterator<char>(input), {}};
            source.erase(std::remove(source.begin(), source.end(), '\r'), source.end());
            return source;
        }
        current = current.parent_path();
    }
    return {};
}
}

int RunFieldSceneChatContractTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL field chat: %s\n", message);
        }
    };
    const auto chat = ReadFieldUnit("TMFieldSceneChat.cpp");
    const auto core = ReadFieldUnit("TMFieldScene.cpp");
    check(!chat.empty() && !core.empty(), "independent chat and core sources are available");
    const char* signatures[] = {
        "void TMFieldScene::SetWhisper(char cOn)",
        "void TMFieldScene::SetPartyChat(char cOn)",
        "void TMFieldScene::SetGuildChat(char cOn)",
        "void TMFieldScene::SetKingDomChat(char cOn)",
        "int TMFieldScene::OnPacketMessageChat(MSG_MessageChat* pStd)",
        "int TMFieldScene::OnPacketMessageChat_Index(MSG_MessageChat* pStd)",
        "int TMFieldScene::OnPacketMessageChat_Param(MSG_STANDARD* pStd)",
        "int TMFieldScene::OnPacketMessageWhisper(MSG_MessageWhisper* pMsg)",
        "void TMFieldScene::SysMsgChat(char* str)",
        "void TMFieldScene::InsertInChatList(SListBox* pChatList, STRUCT_MOB *pMobData, SEditableText* pEditChat, unsigned int dwColor, int colorId, unsigned int startId)",
    };
    for (const auto signature : signatures) {
        check(!source_contract::Method(chat, signature).empty(), "chat definition has exactly one owner");
        check(source_contract::Method(core, signature).empty(), "chat definition is absent from core");
    }
    for (int channel = 0; channel < 3; ++channel) {
        const auto body = source_contract::Method(chat, signatures[channel]);
        check(body.find("m_bCompatFieldScene ? cOn != 0 : cOn == 0") != std::string::npos,
            "native and imported channel selections retain their polarity");
    }
    const auto party = source_contract::Method(chat, signatures[4]);
    check(party.find("if (!pStd || !m_pChatList || !m_pPartyList || m_pPartyList->m_nNumItem <= 0)") <
        party.find("GetHumanByID"), "party controls are guarded before use");
    const auto indexed = source_contract::Method(chat, signatures[5]);
    check(indexed.find("if (!pStd || !m_pChatList || !m_pPartyList)") <
        indexed.find("pStd->String[0]"), "indexed chat guards partial scene initialization");
    const auto parameterized = source_contract::Method(chat, signatures[6]);
    check(parameterized == std::string(signatures[6]) + "\n{\n\treturn 0;\n}",
        "parameterized chat remains the existing no-op");
    const auto whisper = source_contract::Method(chat, signatures[7]);
    check(whisper.find("if (!m_pPartyList)\n\t\t\treturn 1;") <
        whisper.find("m_pPartyList->m_nNumItem"), "party whisper keeps its consumed early return");
    const auto dispatch = source_contract::Method(core,
        "int TMFieldScene::OnPacketEvent(unsigned int dwCode, char* buf)");
    for (const auto handler : {"OnPacketMessageChat(", "OnPacketMessageChat_Index(",
        "OnPacketMessageChat_Param(", "OnPacketMessageWhisper("})
        check(dispatch.find(handler) != std::string::npos, "chat routing remains in the single dispatcher");
    const auto controls = source_contract::Method(core,
        "int TMFieldScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)");
    const auto previousDomain = controls.find("if (idwControlID == B_CHAT_SELECT)");
    const auto chatDomain = controls.find("field_chat::Handle(m_bCompatFieldScene, idwControlID,");
    const auto consumption = controls.find("if (chatOutcome != field_chat::Outcome::Unhandled)", chatDomain);
    const auto nextDomain = controls.find("if (idwControlID >= B_CHAT_SELECT_NOMAL", consumption);
    check(previousDomain != std::string::npos && chatDomain != std::string::npos &&
        consumption != std::string::npos && nextDomain != std::string::npos &&
        previousDomain < chatDomain && chatDomain < consumption && consumption < nextDomain &&
        controls.substr(consumption, nextDomain - consumption).find("return 0;") != std::string::npos,
        "chat keeps dispatcher precedence and consumes both handled and rejected events");
    // Chat submission keeps the original decision order inside the chat edit.
    std::size_t cursor = controls.find("if (isChatEdit && !idwEvent)");
    bool ordered = cursor != std::string::npos;
    for (const auto step : {
        "chat_submit::RecordSubmission(m_dwLastChatTime, dwServerTime)",
        "if (chatFlood)",
        "chat_submit::Remember(m_szLastChatList, pEditChat->GetText())",
        "chat_submit::ClassifyLocal(pEditChat->GetText(), g_pMessageStringTable[191])",
        "if (!BASE_CheckChatValid(Chat))",
        "chat_submit::ClassifyPrefix(Chat)",
        "if (pPartyList->m_nNumItem <= 0)",
        "InsertInChatList(pChatList, pMobData, pEditChat, idwFontColor, route.colorId, route.startIndex)",
        "chat_submit::IsRelocate(str1, g_pMessageStringTable[234])",
        "sprintf(stMsgWhisper.MobName, \"%s\", str1);",
        "chat_submit::TruncateCommandName(str1)",
        "chat_submit::Remember(m_szWhisperList, str1)",
        "BASE_TransCurse(stMsgWhisper.String)",
        "chat_submit::ClassifyCommand(str1, commandNames)",
        "chat_submit::IsReplyAlias(str1)",
        "if (isChatEdit && (idwEvent == 2 || idwEvent == 3))"}) {
        const auto next = ordered ? controls.find(step, cursor) : std::string::npos;
        ordered = next != std::string::npos;
        cursor = next;
    }
    check(ordered, "chat submission decisions keep their original order");
    check(controls.find("m_dwLastChatTime[3] = m_dwLastChatTime[2]") == std::string::npos &&
        controls.find("memcpy(m_szWhisperList[l]") == std::string::npos,
        "chat submission history is updated only through the policy");
    const auto inventory = ReadFieldUnit("TMFieldSceneInventory.cpp");
    const auto ui = ReadFieldUnit("TMFieldSceneUI.cpp");
    const auto mix = ReadFieldUnit("TMFieldSceneMix.cpp");
    for (const auto& fixture : {
        std::pair<std::string, const char*>{inventory, "SGridControl* TMFieldScene::GetCarryGridForSlot(int slot) const"},
        {ui, "unsigned int TMFieldScene::GetLascDescParamId()"},
        {mix, "int TMFieldScene::TryStageNativeMixItem(SGridControl* sourceGrid,\n\tSGridControlItem* sourceItem, SGridControl* preferredTarget)"}})
        check(!source_contract::Method(fixture.first, fixture.second).empty(),
            "actual pointer/const, compound return and multiline signatures extract a complete definition");
    check(!source_contract::Method(core, "TMFieldScene::TMFieldScene()\n\t: TMScene()").empty() &&
        !source_contract::Method(core, "TMFieldScene::~TMFieldScene()").empty(),
        "actual construction and teardown extract without adjacent-method boundaries");
    return failures;
}
