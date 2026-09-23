#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>
#include "../internal/platform/windows/AdapterIdentity.h"

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

    const auto handlerStart = source.find("case B_LOGIN_OK:");
    const auto handlerEnd = source.find("case B_CREATE_ID:", handlerStart);
    check(handlerStart != std::string::npos && handlerEnd != std::string::npos,
        "login handler is delimited");

    if (handlerStart != std::string::npos && handlerEnd != std::string::npos) {
        const std::string handler = source.substr(handlerStart, handlerEnd - handlerStart);
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
    const std::string fieldSource = LoadSource(
        "TMProject748/internal/app/scenes/TMFieldScene.cpp");
    check(fieldSource.find("ReadFirstAdapterIdentity(stAccountLogin.AdapterName)") !=
        std::string::npos, "migration login uses bounded adapter identity");

    return failures;
}
