#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>
#include "../internal/wire/MessageHeader.h"
#include "../CommonFiles/SharedStructs.h"
#include "SourceMethod.h"
#include "../internal/application/AccountLoginAttempt.h"

namespace {
std::filesystem::path FindSource(const char* relativePath)
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};

    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / relativePath;
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
        current = current.parent_path();
    }
    return {};
}

std::string LoadSource(const char* relativePath)
{
    const auto path = FindSource(relativePath);
    if (path.empty())
        return {};

    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
}

// The native 7.48 login handler rejects passwords longer than ten characters
// before opening the socket. Keep that UI policy ahead of connection and of the
// bounded copy into the fixed 12-byte wire field.
int RunLoginCredentialContractTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL login credentials: %s\n", name);
        }
    };

    const std::string source = LoadSource(
        "TMProject748/internal/app/scenes/TMSelectServerScene.cpp");
    check(!source.empty(), "select-server source is available");
    AccountLoginAttempt attempt;
    check(!attempt.Resolve() && attempt.Begin() && attempt.Pending() && !attempt.Begin(),
        "A07-03 one connection accepts only one outstanding login");
    check(attempt.Resolve() && !attempt.Pending() && !attempt.Resolve(),
        "A07-03 cancellation or failure consumes the pending attempt and rejects a late response");
    check(attempt.Begin() && attempt.Resolve(), "A07-03 a resolved attempt permits a later login");
    const auto response = source_contract::Method(source,
        "int TMSelectServerScene::OnPacketEvent(unsigned int dwCode, char* buf)");
    check(response.find("if (!m_loginAttempt.Resolve())") < response.find("TM_SELECTCHAR_STATE"),
        "A07-03 a login response needs a pending attempt before entering character selection");
    check(source.find("m_szAccountPass") == std::string::npos &&
        source.find("SecureZeroMemory(&stAccountLogin, sizeof(stAccountLogin))") != std::string::npos,
        "A07-04 the obsolete password proof is removed and the stack packet is wiped after send");

    // OnControlEvent routes B_LOGIN_OK to OnLoginOk; check that helper by its own boundaries.
    const auto routeStart = source.find("case B_LOGIN_OK:");
    const auto routeEnd = source.find("case B_CREATE_ID:", routeStart);
    check(routeStart != std::string::npos && routeEnd != std::string::npos &&
        source.substr(routeStart, routeEnd - routeStart).find("= OnLoginOk(extractedResult);") != std::string::npos,
        "login handler is delimited");
    const std::string handler = source_contract::Method(source, "ExtractedFlow TMSelectServerScene::OnLoginOk(int& extractedResult)");
    // A renamed handler must fail the gate instead of skipping its checks.
    check(!handler.empty(), "login handler method is found");

    if (!handler.empty()) {
        const auto minimum = handler.find("strlen(pEditPassword->GetText()) < 4");
        const auto maximum = handler.find("strlen(pEditPassword->GetText()) > 10");
        const auto maximumMessage = handler.find("g_pMessageStringTable[6]", maximum);
        const auto connect = handler.find("ConnectServer");
        const auto copy = handler.find(
            "strncpy_s(stAccountLogin.AccountPassword, pEditPassword->GetText(), _TRUNCATE)");

        check(minimum != std::string::npos && maximum != std::string::npos,
            "native four-to-ten password limits are explicit");
        check(maximumMessage != std::string::npos,
            "native password-too-long message is preserved");
        check(connect != std::string::npos && copy != std::string::npos,
            "connection and bounded packet copy remain present");
        check(source_contract::InOrder(minimum, maximum) && source_contract::InOrder(maximum, maximumMessage) &&
            source_contract::InOrder(maximumMessage, connect) && source_contract::InOrder(connect, copy),
            "invalid password is rejected before network and serialization");
    }

    const std::string fieldSource = source_contract::Method(LoadSource(
        "TMProject748/internal/app/scenes/TMFieldSceneSessionPackets.cpp"),
        "int TMFieldScene::OnPacketCNFRemoveServer(MSG_CNFRemoveServer* pStd)");
    for (const auto* loginSource : {&handler, &fieldSource}) {
        check(loginSource->find("MSG_AccountLogin stAccountLogin{}") != std::string::npos &&
            loginSource->find(".AdapterName") == std::string::npos,
            "login path leaves the reserved adapter field zero");
    }

    const auto identify = handler.find("client_build::IdentifyLogin(stAccountLogin)");
    const auto send = handler.find("SendPacket(");
    check(identify != std::string::npos && send != std::string::npos && identify < send,
        "initial login identifies compiled build before sending");
    const auto migrateIdentify = fieldSource.find("client_build::IdentifyLogin(stAccountLogin)");
    check(migrateIdentify != std::string::npos && source_contract::InOrder(migrateIdentify, fieldSource.find("SendPacket(")),
        "migration reconnect identifies the same compiled build");
    MSG_AccountLogin login{};
    constexpr char ticket[] = "migration-ticket";
    std::memcpy(login.Zero, ticket, sizeof(ticket));
    client_build::IdentifyLogin(login);
    check(login.ClientVersion == 748 && login.ClientBuild ==
        (client_build::Prefix | client_build::Revision), "compiled identity fields match contract");
    const int reserved[4]{};
    check(std::strcmp(login.Zero, ticket) == 0 &&
        std::memcmp(login.AdapterName, reserved, sizeof(reserved)) == 0,
        "build identification preserves the migration ticket and zeroed adapter field");
    const std::string disconnectSource = source_contract::Method(LoadSource(
        "TMProject748/internal/app/scenes/TMScene.cpp"),
        "int TMScene::OnPacketEvent(unsigned int dwCode, char* pSBuffer)");
    check(disconnectSource.find("m_pMessagePanel && !m_pMessagePanel->IsVisible()") !=
        std::string::npos, "disconnect does not replace a visible update-required message");

    // Password fields render an ASCII asterisk per character. Byte 0xA2, left
    // from an imported bullet literal, drew a stray glyph in the 7.48 font.
    const std::string editSource = source_contract::Method(LoadSource(
        "TMProject748/internal/ui/controls/SEditableText.cpp"), "void SEditableText::Update()");
    check(editSource.find("editable_text_bounds::WriteMask(m_GCText.strString, nStringLen, '*')") != std::string::npos &&
        editSource.find("\\242") == std::string::npos,
        "password fields mask each character with an asterisk");

    return failures;
}
