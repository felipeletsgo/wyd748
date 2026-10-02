#pragma once

// Shared includes and fixture helpers for the ReceivedPacketDispatch*Tests.cpp units.
#include "../internal/wire/ReceivedPacketDispatch.h"
#include "../internal/wire/CharacterLogoutRequestPacket.h"
#include "../internal/wire/TotoPurchasePacket.h"
#include "../internal/wire/ApplyBonusPacket.h"
#include "../internal/wire/UseItemPacket.h"
#include "../internal/wire/PKModePacket.h"
#include "../internal/wire/PremiumFireworkPacket.h"
#include "../internal/wire/PremiumFireworkUsePacket.h"
#include "../internal/wire/GamblePacket.h"
#include "../internal/wire/MobKillConfirmPacket.h"
#include "../internal/wire/UpdateEtcPacket.h"
#include "../internal/wire/MessagePanelPacket.h"
#include "../internal/wire/DelayStartPacket.h"
#include "../internal/wire/BillingNoticePacket.h"
#include "../internal/wire/PartyAddPacket.h"
#include "../internal/wire/PartyRemovePacket.h"
#include "../internal/wire/PartyRequestPacket.h"
#include "../internal/wire/MotionPacket.h"
#include "../internal/core/ServerEndpoint.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

namespace received_packet_tests {
inline std::filesystem::path FindFixture(const char* relativePath)
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

inline std::vector<char> LoadHexFixture(const char* relativePath)
{
    const auto path = FindFixture(relativePath);
    if (path.empty())
        return {};
    std::ifstream input(path);
    if (!input)
        return {};
    std::vector<char> bytes;
    std::string line;
    while (std::getline(input, line)) {
        if (const auto comment = line.find('#'); comment != std::string::npos)
            line.erase(comment);
        std::istringstream tokens(line);
        std::string token;
        while (tokens >> token) {
            try {
                const auto value = std::stoul(token, nullptr, 16);
                if (value > 0xFF)
                    return {};
                bytes.push_back(static_cast<char>(value));
            }
            catch (...) {
                return {};
            }
        }
    }
    return bytes;
}
}

using received_packet_tests::FindFixture;
using received_packet_tests::LoadHexFixture;
