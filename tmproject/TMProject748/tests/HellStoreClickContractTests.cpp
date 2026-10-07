#include "SourceMethod.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
std::string ReadSceneUnit(const char* filename)
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

// FUN_0045ee28 opens the Hell Gate store for a head-51 NPC inside the Hell Gate
// (x/128 13-14, y/128 28) before any merchant-type branch, and that NPC becomes
// the TargetID of the 0x2BD purchase (FUN_004640e5).
int RunHellStoreClickContractTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL hell store click: %s\n", message);
        }
    };

    const auto input = ReadSceneUnit("TMFieldSceneInput.cpp");
    const auto click = source_contract::Method(input,
        "int TMFieldScene::MouseClick_NPC(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime)");
    check(!click.empty(), "MouseClick_NPC is defined");
    const auto store = click.find("pOver->m_sHeadIndex == 51 &&");
    const auto shop = click.find("(pOver->m_stScore.Merchant & 0xF) == 1");
    check(store != std::string::npos && shop != std::string::npos && store < shop,
        "the store check precedes the merchant branches");
    const auto branch = click.substr(store, shop == std::string::npos ? std::string::npos : shop - store);
    check(branch.find(">> 7 == 13") != std::string::npos && branch.find(">> 7 == 14") != std::string::npos &&
        branch.find(">> 7 == 28") != std::string::npos, "the native region gates the store");
    check(branch.find("m_dwHellStoreID = pOver->m_dwID;") != std::string::npos,
        "the clicked NPC becomes the purchase target");
    check(branch.find("SetVisibleHellGateStore(m_pHellgateStore->IsVisible() == 0);") != std::string::npos,
        "the click toggles the store panel");

    const auto scene = ReadSceneUnit("TMFieldScene.cpp");
    check(scene.find("stHellBuy.Header.Type = 701;") != std::string::npos &&
        scene.find("stHellBuy.TargetID = m_pMessageBox->m_dwArg;") != std::string::npos,
        "the confirmation sends 0x2BD to the stored target");
    check(scene.find("m_pMessageBox->m_dwArg = m_dwHellStoreID;") != std::string::npos,
        "the confirmation box carries the store target");
    return failures;
}
