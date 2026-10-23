#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>
#include "../internal/platform/windows/AdapterIdentity.h"
#include "../internal/wire/ClientBuildContract.h"
#include "SourceMethod.h"

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

    // OnControlEvent routes B_LOGIN_OK to OnLoginOk; check that helper by its own boundaries.
    const auto routeStart = source.find("case B_LOGIN_OK:");
    const auto routeEnd = source.find("case B_CREATE_ID:", routeStart);
    check(routeStart != std::string::npos && routeEnd != std::string::npos &&
        source.substr(routeStart, routeEnd - routeStart).find("= OnLoginOk(extractedResult);") != std::string::npos,
        "login handler is delimited");
    const std::string handler = source_contract::Method(source, "ExtractedFlow TMSelectServerScene::OnLoginOk(int& extractedResult)");

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
        check(minimum < maximum && maximum < maximumMessage &&
            maximumMessage < connect && connect < copy,
            "invalid password is rejected before network and serialization");
    }

    int identity[4]{};
    constexpr char guid[] = "{01234567-89AB-CDEF-0123-456789abcdef}";
    check(ParseAdapterIdentity(guid, sizeof(guid), identity),
        "valid adapter GUID is accepted");
    check(static_cast<unsigned int>(identity[0]) == 0x01234567u &&
        static_cast<unsigned int>(identity[1]) == 0x89ABCDEFu &&
        static_cast<unsigned int>(identity[2]) == 0x01234567u &&
        static_cast<unsigned int>(identity[3]) == 0x89ABCDEFu,
        "adapter GUID retains the four legacy wire words");

    int invalidIdentity[4]{1, 2, 3, 4};
    constexpr char malformed[] = "{01234567-89AB-CDEF-0123-456789abcdeZ}";
    check(!ParseAdapterIdentity(malformed, sizeof(malformed), invalidIdentity) &&
        invalidIdentity[0] == 1 && invalidIdentity[3] == 4,
        "malformed GUID does not partially change packet identity");
    check(!ParseAdapterIdentity(guid, sizeof(guid) - 1, invalidIdentity),
        "unterminated adapter name is rejected within its capacity");
    check(!ParseAdapterIdentity(guid, 20, invalidIdentity),
        "truncated adapter name is rejected");

    check(source.find("ReadFirstAdapterIdentity(stAccountLogin.AdapterName)") !=
        std::string::npos, "initial login uses bounded adapter identity");
    const std::string fieldSource = source_contract::Method(LoadSource(
        "TMProject748/internal/app/scenes/TMFieldSceneSessionPackets.cpp"),
        "int TMFieldScene::OnPacketCNFRemoveServer(MSG_CNFRemoveServer* pStd)");
    check(fieldSource.find("ReadFirstAdapterIdentity(stAccountLogin.AdapterName)") !=
        std::string::npos, "migration login uses bounded adapter identity");

    const auto identify = handler.find("client_build::IdentifyLogin(stAccountLogin)");
    const auto send = handler.find("SendPacket(");
    check(identify != std::string::npos && send != std::string::npos && identify < send,
        "initial login identifies compiled build before sending");
    const auto migrateIdentify = fieldSource.find("client_build::IdentifyLogin(stAccountLogin)");
    check(migrateIdentify != std::string::npos && migrateIdentify < fieldSource.find("SendPacket("),
        "migration reconnect identifies the same compiled build");
    struct LoginIdentity {
        int ClientVersion = 0;
        unsigned int ClientBuild = 0;
        char ticket[52] = "migration-ticket";
        int adapter[4] = {1, 2, 3, 4};
    } login;
    client_build::IdentifyLogin(login);
    check(login.ClientVersion == 748 && login.ClientBuild ==
        (client_build::Prefix | client_build::Revision), "compiled identity fields match contract");
    check(std::string(login.ticket) == "migration-ticket" && login.adapter[0] == 1 &&
        login.adapter[3] == 4, "build identification preserves ticket and adapter identity");
    const std::string disconnectSource = source_contract::Method(LoadSource(
        "TMProject748/internal/app/scenes/TMScene.cpp"),
        "int TMScene::OnPacketEvent(unsigned int dwCode, char* pSBuffer)");
    check(disconnectSource.find("m_pMessagePanel && !m_pMessagePanel->IsVisible()") !=
        std::string::npos, "disconnect does not replace a visible update-required message");

    // Password fields render an ASCII asterisk per character. Byte 0xA2, left
    // from an imported bullet literal, drew a stray glyph in the 7.48 font.
    const std::string editSource = source_contract::Method(LoadSource(
        "TMProject748/internal/ui/controls/SEditableText.cpp"), "void SEditableText::Update()");
    check(editSource.find("m_GCText.strString[nIndex] = '*';") != std::string::npos &&
        editSource.find("\\242") == std::string::npos,
        "password fields mask each character with an asterisk");

    return failures;
}
