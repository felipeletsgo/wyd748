#include "SourceMethod.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
std::string ReadShortcutSource(const char* relative)
{
    wchar_t executable[MAX_PATH]{};
    if (!GetModuleFileNameW(nullptr, executable, MAX_PATH)) return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        std::ifstream input(current / relative, std::ios::binary);
        if (input) {
            std::string result{std::istreambuf_iterator<char>(input), {}};
            result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
            return result;
        }
        current = current.parent_path();
    }
    return {};
}
}

int RunFieldShortcutContractTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL field shortcuts: %s\n", message);
        }
    };
    const auto read = [](const char* unit) {
        const auto path = std::string("tmproject/TMProject748/internal/app/scenes/") + unit;
        return ReadShortcutSource(path.c_str());
    };
    const auto resource = ReadShortcutSource("tmproject/TMProject748/TMProject.rc");
    for (int digit = 0; digit < 10; ++digit) {
        const auto key = "\"" + std::to_string(digit) + "\", ";
        check(resource.find(key + std::to_string(40064 + digit) +
            ", VIRTKEY, CONTROL, NOINVERT") != std::string::npos,
            "native Ctrl number selection is installed");
        check(resource.find(key + std::to_string(40074 + digit) +
            ", VIRTKEY, CONTROL, SHIFT, NOINVERT") != std::string::npos,
            "native Ctrl Shift assignment is installed");
        check(resource.find("VK_NUMPAD" + std::to_string(digit) + ", " +
            std::to_string(40104 + digit) + ", VIRTKEY, CONTROL, NOINVERT") != std::string::npos,
            "native Ctrl numpad entry is installed");
    }
    for (const auto entry : {"\"Q\", 40096", "\"W\", 40101", "\"E\", 40088", "\"F\", 40089"})
        check(resource.find(std::string(entry) + ", VIRTKEY, CONTROL, NOINVERT") != std::string::npos,
            "potion and consumable accelerators are installed");
    for (const auto entry : {"\"'\", 40029", "\"]\", 40028", "\"+\", 40047", "\"-\", 40048"})
        check(resource.find(std::string(entry) + ", ASCII, ALT, NOINVERT") != std::string::npos,
            "native punctuation uses Alt rather than Ctrl");
    const auto app = read("NewApp.cpp");
    check(app.find("LoadAccelerators(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDC_TMPROJECT))") != std::string::npos &&
        app.find("g_nKeyType == 1 || GetFocus() != m_hWnd") != std::string::npos,
        "resource belongs to the executable and preserves mode 0 plus editing");
    const auto window = read("NewAppWindow.cpp");
    check(window.find("int msg = LOWORD(wParam)") != std::string::npos,
        "accelerator high word is not mistaken for the command ID");
    const auto input = read("TMFieldSceneInput.cpp");
    const auto accel = source_contract::Method(input, "int TMFieldScene::OnAccel(int nMsg)");
    check(!accel.empty() && accel.find("g_nKeyType != 1") < accel.find("switch (nMsg)"),
        "gameplay accelerator dispatch requires mode 1");
    for (const auto entry : {"OnKeyHPotion('q', 0)", "OnKeyMPotion('w', 0)", "OnKeyPPotion('e', 0)"})
        check(accel.find("CurrentScore.CurHP > 0") < accel.find(entry),
            "potion accelerator retains the native alive gate");
    check(accel.find("OnKeyAuto('f', 0)") < accel.find("CurrentScore.CurHP > 0"),
        "F consumable retains its original dispatch position");
    const auto core = read("TMFieldScene.cpp");
    const auto frame = source_contract::Method(core, "int TMFieldScene::FrameMove(unsigned int dwServerTime)");
    check(!frame.empty() && frame.find("UpdateAutoRun();") != std::string::npos,
        "the frame loop consumes the autorun flag");
    const auto movement = source_contract::Method(read("TMFieldSceneMovement.cpp"), "void TMFieldScene::UpdateAutoRun()");
    check(!movement.empty() && movement.find("CanAutoRun(") < movement.find("GetRoute(") &&
        movement.find("AutoRunCellAllowed(target.x, target.y, 0)") < movement.find("BASE_GetAttr(") &&
        movement.find("GetCameraLookatDir()") != std::string::npos,
        "autorun checks state and coordinates before using the normal route");
    check(core.find("m_pBtnGuildOnOff = nullptr") != std::string::npos &&
        core.find("m_cGuildOnOff ? \"guildoff\" : \"guildon\"") != std::string::npos &&
        core.find("m_cGuildOnOff = m_cGuildOnOff == 0") == std::string::npos,
        "guild intent uses confirmed state and has no uninitialized button");
    const auto guildAck = source_contract::Method(read("TMFieldSceneSocial.cpp"),
        "int TMFieldScene::OnPacketGuildDisable(MSG_STANDARDPARM* pStd)");
    check(guildAck.find("!pStd") < guildAck.find("m_cGuildOnOff = pStd->Parm == 0") &&
        guildAck.find("SetSelected(pStd->Parm == 1 ? 1 : 0)") != std::string::npos,
        "guild acknowledgement updates both keyboard state and button polarity");
    const auto ui = read("TMFieldSceneUI.cpp");
    check(ui.find("if (m_nCurrInterfacePanelIndex == 0)") != std::string::npos &&
        ui.find("setControlsVisible(868, 1)") != std::string::npos,
        "keyboard Help replaces stale artwork with the current text guide");
    const auto help = ReadShortcutSource("tmproject/client748/UI/interface.txt");
    const auto other = ReadShortcutSource("tmproject/client748/UI/etc.txt");
    for (const auto entry : {"Ctrl+Shift+1-0", "Ctrl+Right-click", "Shift+Right-click",
        "Ctrl+Left-click", "Shift+Left-click", "Alt for +, -, ], and apostrophe", "FFFFAA K"})
        check(help.find(entry) != std::string::npos, "Help documents the active bindings");
    for (const auto entry : {"P or D", "M or A", "Open Guild Board", "Shift + F1", "(xxxx,xxxx)"})
        check(help.find(entry) == std::string::npos && other.find(entry) == std::string::npos,
            "Help does not advertise obsolete bindings or placeholders");
    for (const auto* text : {&help, &other}) {
        check(!text->empty(), "Help content is present");
        std::size_t start = 0;
        while (start < text->size()) {
            const auto end = text->find('\n', start);
            const auto line = text->substr(start, end == std::string::npos ? end : end - start);
            check(line.size() >= 7 && line[6] == ' ' &&
                line.find_first_not_of("0123456789ABCDEF", 0) == 6,
                "Help rows retain the six-digit color prefix");
            if (end == std::string::npos) break;
            start = end + 1;
        }
    }
    return failures;
}
