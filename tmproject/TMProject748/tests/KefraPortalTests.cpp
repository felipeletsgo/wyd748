#include "ReceivedPacketDispatchTestSupport.h"
#include "../internal/render/world/objects/KefraPortalVisual.h"
#include <regex>

namespace
{
    kefra_portal::ClientState portal;
    bool IsPortalOpen() { return portal.IsOpen(); }

    // Exercise the actual visibility adapter without a Direct3D device.
    class FakeVisual
    {
    public:
        virtual ~FakeVisual() = default;
        virtual int IsVisible() { ++visibilityCalls; return m_bVisible = inView ? 1 : 0; }
        virtual int Render() { ++renderCalls; return 7; }
        int m_bVisible = 1;
        bool inView = true;
        int visibilityCalls = 0;
        int renderCalls = 0;
    };
    using GatedVisual = kefra_portal::VisualGate<FakeVisual, IsPortalOpen>;

    std::string ReadSource(const char* relativePath)
    {
        std::ifstream input(FindFixture(relativePath));
        std::ostringstream contents;
        contents << input.rdbuf();
        return contents.str();
    }
}

int RunKefraPortalTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL Kefra portal: %s\n", name); }
    };
    auto bytes = LoadHexFixture("testdata/protocol/kefra_portal_0x7f30_defeated.hex");
    const auto alive = LoadHexFixture("testdata/protocol/kefra_portal_0x7f30_alive.hex");
    check(bytes.size() == sizeof(kefra_portal::State) && alive.size() == bytes.size(),
        "both canonical frames are loaded from the shared client/server corpus");
    if (bytes.size() != sizeof(kefra_portal::State) || alive.size() != bytes.size())
        return failures;
    kefra_portal::State parsed{};
    const auto dispatchable = [&](const void* data, std::size_t size, unsigned int opcode) {
        return received_packet::CanDispatch({opcode, static_cast<const char*>(data), size});
    };
    check(kefra_portal::Parse(bytes.data(), bytes.size(), parsed) && parsed.Defeated == 1,
        "canonical server open frame parses");
    check(dispatchable(bytes.data(), bytes.size(), kefra_portal::StateOpcode),
        "canonical open frame reaches the scene");
    check(received_packet::ExpectedSize(kefra_portal::StateOpcode) == 16,
        "dispatch recognizes the exact v1 size");
    for (std::size_t size = 0; size < bytes.size(); ++size)
    {
        check(!kefra_portal::Parse(bytes.data(), size, parsed), "every truncation is rejected");
        check(!dispatchable(bytes.data(), size, kefra_portal::StateOpcode),
            "truncated frames never reach a legacy cast");
    }
    std::array<unsigned char, 17> unaligned{};
    std::memcpy(unaligned.data() + 1, bytes.data(), bytes.size());
    check(kefra_portal::Parse(unaligned.data() + 1, bytes.size(), parsed) &&
        dispatchable(unaligned.data() + 1, bytes.size(), kefra_portal::StateOpcode),
        "unaligned frames are copied safely");
    check(!kefra_portal::Parse(unaligned.data(), unaligned.size(), parsed) &&
        !dispatchable(unaligned.data(), unaligned.size(), kefra_portal::StateOpcode),
        "an extra payload byte is rejected");
    check(!kefra_portal::Parse(nullptr, bytes.size(), parsed), "null frame is rejected");
    check(!dispatchable(bytes.data(), bytes.size(), 0), "metadata cannot disguise the opcode");

    for (const std::size_t offset : {std::size_t{0}, std::size_t{4}, std::size_t{6},
        std::size_t{12}, std::size_t{14}})
    {
        auto bad = bytes;
        bad[offset] = 2;
        const auto before = parsed;
        check(!kefra_portal::Parse(bad.data(), bad.size(), parsed) &&
            std::memcmp(&before, &parsed, sizeof(parsed)) == 0,
            "invalid size, opcode, recipient, version or flag cannot mutate parsed state");
        check(!dispatchable(bad.data(), bad.size(), kefra_portal::StateOpcode),
            "invalid discriminants are rejected at dispatch");
    }
    auto disguised = bytes;
    disguised[4] = static_cast<char>(0xfe);
    disguised[5] = 0x7e;
    check(!dispatchable(disguised.data(), disguised.size(), kefra_portal::StateOpcode),
        "portal metadata cannot disguise an unknown header opcode");

    portal.Reset();
    check(!portal.IsOpen(), "a fresh field scene fails closed before its snapshot");
    std::array<GatedVisual, 3> visuals; // The model and both billboards.
    for (auto& visual : visuals)
    {
        check(visual.IsVisible() == 0 && visual.Render() == 1 && visual.m_bVisible == 0 &&
            visual.visibilityCalls == 0 && visual.renderCalls == 0,
            "closed portal never enters model or billboard drawing");
    }
    check(portal.Apply(bytes.data(), bytes.size()) && portal.IsOpen() &&
        portal.Apply(bytes.data(), bytes.size()), "persisted defeat opens idempotently");
    for (auto& visual : visuals)
    {
        check(visual.IsVisible() == 1 && visual.Render() == 7 &&
            visual.visibilityCalls == 1 && visual.renderCalls == 1,
            "defeat enables all three normal rendering paths");
        visual.inView = false;
        check(visual.IsVisible() == 0, "open portal preserves the base visibility policy");
    }
    GatedVisual streamed;
    check(streamed.IsVisible() == 1, "an object streamed after the snapshot inherits its state");
    auto invalid = bytes;
    invalid[14] = 2;
    check(!portal.Apply(invalid.data(), invalid.size()) && portal.IsOpen(),
        "an invalid snapshot never changes a valid state");
    bytes = alive;
    check(portal.Apply(bytes.data(), bytes.size()) && !portal.IsOpen(),
        "weekly boss return closes the portal without reloading the map");
    for (auto& visual : visuals)
    {
        visual.m_bVisible = 1;
        check(visual.Render() == 1 && visual.m_bVisible == 0 && visual.renderCalls == 1,
            "closing clears visibility cached by an earlier frame");
    }
    bytes[14] = 1;
    portal.Apply(bytes.data(), bytes.size());
    portal.Reset();
    kefra_portal::ClientState nextSession;
    check(!portal.IsOpen() && !nextSession.IsOpen() && streamed.IsVisible() == 0,
        "scene reuse and relogin cannot retain the previous session's defeat");
    check(kefra_portal::IsHallPortal(2035, 2366.5f, 3926.5f) &&
        !kefra_portal::IsHallPortal(2034, 2366.5f, 3926.5f) &&
        !kefra_portal::IsHallPortal(2035, 2367.5f, 3926.5f) &&
        !kefra_portal::IsHallPortal(2035, 2366.5f, 3925.5f),
        "only the Kefra hall instance is gated, not other instances of model 2035");

    const auto objects = ReadSource("tmproject/TMProject748/internal/render/world/objects/TMObjectContainer.cpp");
    check(objects.find("new KefraPortalEffect(423,") != std::string::npos &&
        objects.find("new KefraPortalEffect(424,") != std::string::npos &&
        objects.find("new KefraPortalObject()") != std::string::npos &&
        objects.find("m_kefraPortal.IsOpen()") != std::string::npos,
        "the map loader wires the real model and both effects to the scene snapshot");
    const auto scene = ReadSource("tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp");
    check(scene.find("m_kefraPortal.Reset();") != std::string::npos &&
        scene.find("case kefra_portal::StateOpcode:") != std::string::npos &&
        scene.find("OnPacketKefraPortalState(") != std::string::npos,
        "scene initialization resets state and packet dispatch reaches its handler");
    const auto world = ReadSource("tmproject/TMProject748/internal/app/scenes/TMFieldSceneWorld.cpp");
    check(world.find("m_kefraPortal.Apply(packet, sizeof(kefra_portal::State))") != std::string::npos,
        "the field handler uses the validated versioned state");

    // Visibility alone does not activate a portal: the movement prompt must
    // resolve its four-unit block in the production table before sending 0x290.
    const auto globals = ReadSource("tmproject/TMProject748/internal/core/TMGlobal.h");
    const auto tableStart = globals.find("g_TeleportTable[]");
    const auto tableEnd = globals.find("};", tableStart);
    check(tableStart != std::string::npos && tableEnd != std::string::npos,
        "the production teleport table is available");
    if (tableStart != std::string::npos && tableEnd != std::string::npos)
    {
        const auto table = globals.substr(tableStart, tableEnd - tableStart);
        const std::regex row(R"(\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,([^\r\n]*))");
        int matches = 0;
        for (auto it = std::sregex_iterator(table.begin(), table.end(), row);
            it != std::sregex_iterator(); ++it)
        {
            if (std::stoi((*it)[1]) != 2364 || std::stoi((*it)[2]) != 3924)
                continue;
            ++matches;
            check(std::stoi((*it)[3]) == 700 &&
                (*it)[4].str().find("\"Uxmal\"") != std::string::npos,
                "the hall passage prompts for Uxmal with the existing 700 gold fee");
        }
        check(matches == 1, "the hall passage has exactly one client prompt entry");
    }
    const auto movement = ReadSource("tmproject/TMProject748/internal/app/scenes/TMFieldSceneMovement.cpp");
    check(movement.find("m_vecPosition.x & 0xFFFC") != std::string::npos &&
        movement.find("m_vecPosition.y & 0xFFFC") != std::string::npos &&
        movement.find("g_TeleportTable[ll].nX == x && g_TeleportTable[ll].nY == y") != std::string::npos,
        "the actual movement prompt resolves coordinates using the tested four-unit grid");
    for (const char* path : {"tmproject/client748/Env/AttributeMap.dat",
        "wydgo748/data/maps/AttributeMap.dat"})
    {
        std::ifstream attributes(FindFixture(path), std::ios::binary);
        for (int x = 2364; x < 2368; ++x)
        {
            for (int y = 3924; y < 3928; ++y)
            {
                check((x & 0xFFFC) == 2364 && (y & 0xFFFC) == 3924,
                    "every tile under the mirror resolves to the registered block");
                attributes.seekg((y / 4) * 1024 + x / 4);
                char attr = 0;
                attributes.get(attr);
                check(attributes.good() && (attr & 0x10) != 0,
                    "client and server maps mark every mirror tile as a portal");
            }
        }
    }
    std::istringstream routes(ReadSource("wydgo748/data/teleports.ini"));
    int routesFound = 0;
    for (std::string line; std::getline(routes, line);)
    {
        int x = 0, y = 0, destinationX = 0, destinationY = 0, price = 0;
        if (sscanf_s(line.c_str(), "%d,%d,%d,%d,%d", &x, &y,
            &destinationX, &destinationY, &price) != 5 ||
            (x & 0xFFFC) != 2364 || (y & 0xFFFC) != 3924)
            continue;
        ++routesFound;
        check(destinationX == 3250 && destinationY == 1703 && price == 700,
            "the client prompt matches the existing authoritative Uxmal route and fee");
    }
    check(routesFound == 1, "the mirror block has exactly one authoritative server route");
    return failures;
}
