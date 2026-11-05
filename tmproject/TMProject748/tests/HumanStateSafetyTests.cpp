#include "../internal/game/entities/GroundItemPresentation.h"
#include "../internal/game/entities/HumanMovementPolicy.h"
#include "../internal/game/entities/HumanPacketPolicy.h"
#include "../internal/game/entities/HumanVisualPolicy.h"
#include "SourceMethod.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <set>
#include <string>
#include <windows.h>

namespace
{
std::string ReadRepositoryFile(const char* relative)
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        std::ifstream input(current / relative, std::ios::binary);
        if (input) {
            std::string text{std::istreambuf_iterator<char>(input), {}};
            text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
            return text;
        }
        current = current.parent_path();
    }
    return {};
}

std::string ReadClientSource(const char* relative)
{
    return ReadRepositoryFile((std::string("tmproject/TMProject748/") + relative).c_str());
}

bool Contains(const std::string& text, const char* fragment)
{
    return !text.empty() && text.find(fragment) != std::string::npos;
}

// ---- ground items ---------------------------------------------------------

struct FakeVertex
{
    unsigned int diffuse;
    float pad[3];
};

struct FakeVertexBuffer
{
    long lockResult = 0;
    bool returnNull = false;
    FakeVertex* storage = nullptr;
    int locks = 0;
    int unlocks = 0;

    long Lock(unsigned int, unsigned int, void** data, unsigned long)
    {
        ++locks;
        if (lockResult >= 0 && !returnNull)
            *data = storage;
        return lockResult;
    }

    long Unlock()
    {
        ++unlocks;
        return 0;
    }
};

void GroundItemTests(const std::function<void(bool, const char*)>& check)
{
    using namespace ground_item_view;
    // A16-01: the hover range keeps the former 18-cell boundary semantics.
    check(!IsOutsideTargetHoverRange(100, 100, 82.0f, 100.0f), "A16-01 hover range includes x = target - 18");
    check(IsOutsideTargetHoverRange(100, 100, 81.9f, 100.0f), "A16-01 hover range excludes x < target - 18");
    check(!IsOutsideTargetHoverRange(100, 100, 118.0f, 118.0f), "A16-01 hover range includes target + 18");
    check(IsOutsideTargetHoverRange(100, 100, 100.0f, 118.5f), "A16-01 hover range excludes y > target + 18");

    const auto item = ReadClientSource("internal/game/entities/TMItem.cpp");
    const auto frameMove = source_contract::MaskCpp(
        source_contract::Method(item, "int TMItem::FrameMove(unsigned int dwServerTime)"));
    check(!frameMove.empty() && frameMove.find("DeleteObject(") == std::string::npos,
        "A16-01 TMItem::FrameMove never deletes a server-visible item");
    check(Contains(frameMove, "ground_item_view::IsOutsideTargetHoverRange("),
        "A16-01 a distant movement target only clears the hover state");

    // A16-06: EF_HWORDCOIN * 256 + EF_LWORDCOIN, as WYD-Go computes it.
    check(CoinAmount(117, 48) == 30000, "A16-06 coin bundle 117/48 shows 30000");
    check(CoinAmount(0, 48) == 48 && CoinAmount(1, 0) == 256 && CoinAmount(255, 255) == 65535,
        "A16-06 coin amount keeps the high word");
    const auto initObject = source_contract::MaskCpp(source_contract::Method(item, "int TMItem::InitObject()"));
    check(Contains(initObject, "ground_item_view::CoinAmount(") &&
        initObject.find("unsigned char coin") == std::string::npos,
        "A16-06 the gold label no longer uses a byte accumulator");

    // A16-07: unattached special effect, replaced guild panel, member init.
    check(Contains(initObject, "if (m_pEffectSpecial && !m_pEffectSpecial->m_pTop)"),
        "A16-07 InitObject releases a special effect that no branch attached");
    const auto initItem = source_contract::MaskCpp(source_contract::Method(item, "void TMItem::InitItem(STRUCT_ITEM stItem)"));
    const auto panelRelease = initItem.find("SAFE_DELETE(m_stGuildMark.pGuildMark);");
    const auto panelCreate = initItem.find("new SPanel(");
    check(panelRelease != std::string::npos && panelCreate != std::string::npos && panelRelease < panelCreate,
        "A16-07 InitItem releases the previous guild panel before creating one");
    check(Contains(initItem, "m_stItem.sIndex < MAX_ITEMLIST") && initItem.find("<= 6500") == std::string::npos,
        "A16-07 InitItem bounds the item index by the item-list size");
    const auto ctorStart = item.find("TMItem::TMItem()");
    const auto ctorEnd = item.find("TMItem::~TMItem()");
    check(ctorStart != std::string::npos && ctorEnd != std::string::npos &&
        item.substr(ctorStart, ctorEnd - ctorStart).find("m_Materials = {};") != std::string::npos,
        "A16-07 the constructor value-initializes the material");

    // A16-08 / C-41 (TMItem site): checked lock helper.
    FakeVertex vertices[4]{};
    for (auto& vertex : vertices)
        vertex.diffuse = 0x11111111u;
    int writes = 0;
    const auto tint = [&](FakeVertex* data, int count) {
        ++writes;
        for (int i = 0; i < count; ++i)
            data[i].diffuse = 0xFF00FF00u;
    };
    FakeVertexBuffer failed;
    failed.lockResult = -2147467259L; // E_FAIL
    failed.storage = vertices;
    check(!WithLockedVertices<FakeVertex>(&failed, sizeof(vertices), tint) && writes == 0 &&
        failed.unlocks == 0 && vertices[0].diffuse == 0x11111111u,
        "C-41 a failed Lock leaves the vertices untouched and does not unlock");
    FakeVertexBuffer nullData;
    nullData.returnNull = true;
    check(!WithLockedVertices<FakeVertex>(&nullData, sizeof(vertices), tint) && writes == 0 && nullData.unlocks == 1,
        "C-41 a null locked pointer is not written and the lock is released");
    check(!WithLockedVertices<FakeVertex>(static_cast<FakeVertexBuffer*>(nullptr), sizeof(vertices), tint) && writes == 0,
        "C-41 a missing vertex buffer is skipped");
    FakeVertexBuffer valid;
    valid.storage = vertices;
    check(WithLockedVertices<FakeVertex>(&valid, sizeof(vertices), tint) && writes == 1 && valid.unlocks == 1 &&
        vertices[0].diffuse == 0xFF00FF00u && vertices[3].diffuse == 0xFF00FF00u,
        "C-41 a successful Lock writes every vertex once and unlocks");
    const auto render = source_contract::Method(item, "int TMItem::Render()");
    check(Contains(render, "if (pMesh && pMesh->m_pVB)") && Contains(render, "SUCCEEDED(pMesh->m_pVB->GetDesc(&vDesc))") &&
        Contains(render, "WithLockedVertices<RDLVERTEX>(pMesh->m_pVB, vDesc.Size") &&
        Contains(render, "pVertex[i].diffuse = dwColor[nMeshIndex - 737];"),
        "A16-08 the ankh tint keeps the native colour through a checked lock");

    // A16-05 (ground item): show the mark only for the owning guild slot.
    const auto label = source_contract::MaskCpp(source_contract::Method(item, "void TMItem::LabelPosition()"));
    check(Contains(label, "guild_mark_label::Shows(") && label.find("nMarkIndex == -1") == std::string::npos,
        "A16-05 ground guild items compare the slot owner before showing a mark");
}

// ---- movement -------------------------------------------------------------

struct Cell
{
    float x;
    float y;
};

bool SameCell(const Cell& a, const Cell& b)
{
    return a.x == b.x && a.y == b.y;
}

// Mirrors GenerateRouteTable's layout: [0] start, then one cell per step,
// then the last cell repeated. Returns step count + 2.
int BuildTable(int x, int y, int dx, int dy, int steps, Cell (&table)[route_bridge::kRouteCells])
{
    Cell current{ static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f };
    table[0] = current;
    for (int i = 1; i < route_bridge::kRouteCells; ++i) {
        if (i <= steps)
            current = Cell{ current.x + static_cast<float>(dx), current.y + static_cast<float>(dy) };
        table[i] = current;
    }
    return steps + 2;
}

void MovementTests(const std::function<void(bool, const char*)>& check)
{
    using transition_gate::HasElapsed;
    // A15-09: wrap-safe transition gates.
    check(HasElapsed(0x10u, 0xFFF00000u, 6000), "A15-09 a stamp before the server-tick wrap opens the gate");
    check(HasElapsed(10u, 0u, 6000) && HasElapsed(0xFFFFFFF0u, 0u, 6000), "A15-09 an unset stamp never blocks");
    check(!HasElapsed(1000u + 5999u, 1000u, 6000), "A15-09 a fresh stamp blocks for 6000 ms");
    check(HasElapsed(1000u + 6000u, 1000u, 6000), "A15-09 the gate opens after 6000 ms");
    check(!HasElapsed(1000u, 1085u, 6000), "A15-09 a stamp slightly ahead after drift correction still blocks");
    check(!HasElapsed(5000u, 5000u, 6000), "A15-09 a stamp equal to now blocks");

    const auto movement = ReadClientSource("internal/game/entities/TMHumanMovement.cpp");
    const auto getRoute = source_contract::MaskCpp(
        source_contract::Method(movement, "void TMHuman::GetRoute(IVector2 vecTarget, int nCount, int bStop)"));
    check(!getRoute.empty() && getRoute.find("+ 6000") == std::string::npos &&
        Contains(getRoute, "transition_gate::HasElapsed(dwTime, pFScene->m_dwLastTeleport, 6000)"),
        "A15-09 GetRoute uses elapsed transition gates");

    // A15-06: keep the bridge route instead of an empty buffer.
    Cell route[route_bridge::kRouteCells]{};
    const int routeMax = BuildTable(20, 10, 1, 0, 8, route);   // (20,10) -> (28,10)
    Cell bridge[route_bridge::kRouteCells]{};
    const int join = 3;                                        // route cell (23,10)
    const int bridgeMax = BuildTable(23, 15, 0, -1, 5, bridge); // actor (23,15) -> (23,10)
    check(SameCell(bridge[bridgeMax - 2], route[join]), "A15-06 fixture bridge ends on the join cell");
    Cell out[route_bridge::kRouteCells + 1]{};
    const Cell guard{ -7.0f, -7.0f };
    out[route_bridge::kRouteCells] = guard;
    const int joinedMax = route_bridge::Compose(bridge, bridgeMax, route, join, routeMax, out);
    check(SameCell(out[0], bridge[0]), "A15-06 the joined route starts at the actor's cell, not at the join cell");
    bool bridgeKept = true;
    for (int i = 0; i <= 5; ++i)
        bridgeKept = bridgeKept && SameCell(out[i], bridge[i]);
    check(bridgeKept, "A15-06 every bridge step is kept");
    bool tailKept = true;
    for (int i = 5; i < route_bridge::kRouteCells; ++i)
        tailKept = tailKept && SameCell(out[i], route[(std::min)(i - 5 + join, route_bridge::kRouteCells - 1)]);
    check(tailKept, "A15-06 the server route tail follows the bridge");
    check(joinedMax == routeMax - join + 5, "A15-06 the max route index covers bridge and tail");
    check(SameCell(out[route_bridge::kRouteCells], guard), "A15-06 composition stays inside 48 cells");
    check(route_bridge::Compose(bridge, 1, route, join, routeMax, out) == 0 &&
        route_bridge::Compose(bridge, bridgeMax, route, route_bridge::kRouteCells, routeMax, out) == 0,
        "A15-06 invalid bridge or join indices are rejected");
    Cell longBridge[route_bridge::kRouteCells]{};
    const int longMax = BuildTable(0, 0, 1, 1, 46, longBridge);
    check(route_bridge::Compose(longBridge, longMax, route, 40, 47, out) == route_bridge::kRouteCells - 1,
        "A15-06 the joined max index is clamped to the table");
    const auto change = source_contract::MaskCpp(source_contract::Method(movement,
        "int TMHuman::ChangeRouteBuffer(int nSX, int nSY, TMVector2* pRouteTable, int* pMaxRouteIndex)"));
    check(Contains(change, "GenerateRouteTable(nSX, nSY, szBridge, vecBridge, &nBridgeMaxIndex);") &&
        Contains(change, "route_bridge::Compose(") && Contains(change, "*pMaxRouteIndex = nMaxRouteIndex;"),
        "A15-06 ChangeRouteBuffer generates the kept bridge and updates the max index");
}

// ---- visuals --------------------------------------------------------------

struct FakeEffect
{
    int m_bShow = 0;
};

struct FakeAniRow
{
    unsigned int dwAniTable[afterimage_clip::kMotionColumns];
};

struct FakeGuildSlot
{
    int nGuild;
};

// Collects `m_pName` identifiers of statements `...->m_bShow = 0` in `body`.
std::set<std::string> HiddenMembers(const std::string& body)
{
    std::set<std::string> names;
    const std::string marker = "m_bShow = 0";
    for (std::size_t at = 0; (at = body.find(marker, at)) != std::string::npos; at += marker.size()) {
        const auto lineStart = body.rfind('\n', at);
        const auto line = body.substr(lineStart == std::string::npos ? 0 : lineStart + 1,
            at - (lineStart == std::string::npos ? 0 : lineStart + 1));
        const auto name = line.find("m_p");
        if (name == std::string::npos)
            continue;
        auto end = name + 3;
        while (end < line.size() && (std::isalnum(static_cast<unsigned char>(line[end])) || line[end] == '_'))
            ++end;
        names.insert(line.substr(name, end - name));
        if (line.find("->m_pBillBoard->") != std::string::npos)
            names.insert("m_pBillBoard");
    }
    return names;
}

// Parses SetMountCostume's switch: a label is mapped when its body assigns.
std::set<unsigned int> MappedMountSelectors(const std::string& method)
{
    std::set<unsigned int> mapped;
    const auto code = source_contract::MaskCpp(method);
    const auto start = code.find("switch (curIndex)");
    const auto end = code.find("default:", start);
    if (start == std::string::npos || end == std::string::npos)
        return mapped;
    std::set<unsigned int> pending;
    std::size_t at = start;
    while (true) {
        const auto label = code.find("case ", at);
        if (label == std::string::npos || label > end)
            break;
        const auto colon = code.find(':', label);
        const auto value = static_cast<unsigned int>(std::stoul(code.substr(label + 5, colon - label - 5)));
        auto next = code.find("case ", colon);
        if (next == std::string::npos || next > end)
            next = end;
        const auto body = code.substr(colon + 1, next - colon - 1);
        pending.insert(value);
        if (body.find_first_not_of(" \t\n") != std::string::npos) {
            if (body.find('=') != std::string::npos)
                mapped.insert(pending.begin(), pending.end());
            pending.clear();
        }
        at = next;
    }
    return mapped;
}

void VisualTests(const std::function<void(bool, const char*)>& check)
{
    using namespace effect_visibility;
    // A15-05: the local player keeps its effects in Shadow; observers hide them.
    check(!HidesActorEffects(0, 1, true), "A15-05 local Shadow does not take the hidden early return");
    check(HidesActorEffects(0, 1, false), "A15-05 an observed Shadow actor still hides its effects");
    check(HidesActorEffects(1, 0, true) && !HidesActorEffects(0, 0, false), "A15-05 the hide affect still hides");
    check(HidesBuffEffects(1, 0) && HidesBuffEffects(0, 1) && !HidesBuffEffects(0, 0), "A15-04 buff toggle states");

    // A15-04: a hidden effect is shown again after the hide condition ends.
    FakeEffect effect;
    effect.m_bShow = 0;
    Show(&effect);
    Show(static_cast<FakeEffect*>(nullptr));
    check(effect.m_bShow == 1, "A15-04 Show restores m_bShow and ignores null members");

    const auto effects = ReadClientSource("internal/game/entities/TMHumanEffects.cpp");
    const auto restore = source_contract::MaskCpp(
        source_contract::Method(effects, "void RestoreShownEffects(TMHuman& human, bool buffsHidden)"));
    const auto hidden = source_contract::MaskCpp(
        source_contract::Method(effects, "ExtractedFlow TMHuman::UpdateHiddenEffects()"));
    const auto frame = source_contract::MaskCpp(
        source_contract::Method(effects, "void TMHuman::FrameMoveEffect(unsigned int dwServerTime)"));
    const auto buffBlockStart = frame.find("if (buffsHidden)");
    const auto buffBlockEnd = frame.find("if (m_stEffectEvent.dwTime", buffBlockStart);
    check(!restore.empty() && !hidden.empty() && buffBlockStart != std::string::npos &&
        buffBlockEnd != std::string::npos, "A15-04 hide and restore paths located");
    const auto restoreBuffGate = restore.find("if (buffsHidden)");
    bool everyHiddenRestored = !restore.empty() && restoreBuffGate != std::string::npos;
    for (const auto& name : HiddenMembers(hidden)) {
        const auto use = name == "m_pBillBoard" ? std::string("->m_pBillBoard") : "human." + name;
        if (restore.find(use) == std::string::npos) {
            everyHiddenRestored = false;
            std::fprintf(stderr, "  not restored after actor hide: %s\n", name.c_str());
        }
    }
    check(HiddenMembers(hidden).size() >= 21, "A15-04 the actor-hide census finds the hidden members");
    check(everyHiddenRestored, "A15-04 every member UpdateHiddenEffects hides is restored");
    bool everyBuffRestored = everyHiddenRestored;
    if (buffBlockStart != std::string::npos && buffBlockEnd != std::string::npos) {
        for (const auto& name : HiddenMembers(frame.substr(buffBlockStart, buffBlockEnd - buffBlockStart))) {
            const auto use = restore.find("human." + name, restoreBuffGate);
            if (use == std::string::npos) {
                everyBuffRestored = false;
                std::fprintf(stderr, "  not restored after buff toggle: %s\n", name.c_str());
            }
        }
    }
    check(buffBlockStart != std::string::npos && buffBlockEnd != std::string::npos &&
        HiddenMembers(frame.substr(buffBlockStart, buffBlockEnd - buffBlockStart)).size() >= 9,
        "A15-04 the Effects-toggle census finds the hidden members");
    check(everyBuffRestored, "A15-04 every member the Effects toggle hides is restored only while buffs show");
    const auto actorGate = frame.find("effect_visibility::HidesActorEffects(m_cHide, m_cShadow, g_pCurrentScene->m_pMyHuman == this)");
    const auto restoreCall = frame.find("RestoreShownEffects(*this, buffsHidden);");
    const auto shadowPlane = frame.find("if (m_cShadow == 1 && g_pCurrentScene->m_pMyHuman == this)");
    check(actorGate != std::string::npos && restoreCall > actorGate && buffBlockStart > restoreCall &&
        shadowPlane > buffBlockStart, "A15-04/A15-05 gate, restore, buff hide and local Shadow plane are ordered");

    // A15-07 / A16-13: after-image clips follow SetAnimation.
    static FakeAniRow table[afterimage_clip::kAnimationTypes]{};
    static short humanArray[afterimage_clip::kHumanSkins][afterimage_clip::kAnimationTypes][afterimage_clip::kMotionColumns]{};
    for (int skin = 0; skin < afterimage_clip::kAnimationTypes; ++skin)
        for (int column = 0; column < afterimage_clip::kMotionColumns; ++column)
            table[skin].dwAniTable[column] = static_cast<unsigned int>((skin * 7 + column) % afterimage_clip::kMotionColumns);
    for (int skin = 0; skin < afterimage_clip::kHumanSkins; ++skin)
        for (int weapon = 0; weapon < afterimage_clip::kAnimationTypes; ++weapon)
            for (int motion = 0; motion < afterimage_clip::kMotionColumns; ++motion)
                humanArray[skin][weapon][motion] = static_cast<short>(1000 + skin * 100 + motion);
    bool matchesSetAnimation = true;
    for (int skin = 0; skin < afterimage_clip::kAnimationTypes; ++skin) {
        const int column = 2;
        const auto value = static_cast<int>(table[skin].dwAniTable[column]);
        // SetAnimation: skins 0..5 except 3 through the human array, others direct.
        const int expected = skin < 6 && skin != 3 ? 1000 + skin * 100 + value : value;
        matchesSetAnimation = matchesSetAnimation &&
            afterimage_clip::Body(table, humanArray, skin, 4, column) == expected;
    }
    check(matchesSetAnimation, "A15-07 every skin 0..59 selects the SetAnimation clip");
    check(afterimage_clip::Body(table, humanArray, 8, 4, 2) == static_cast<int>(table[8].dwAniTable[2]),
        "A15-07 skin 8 (Astaroth) no longer indexes past the 6-row human array");
    check(afterimage_clip::Body(table, humanArray, 60, 4, 2) == 0 && afterimage_clip::Body(table, humanArray, 0, 4, 56) == 0 &&
        afterimage_clip::Body(table, humanArray, 0, 60, 2) == 0 && afterimage_clip::Body(table, humanArray, -1, 0, 0) == 0,
        "A15-07 skin, column and weapon outside the tables yield clip 0");
    table[1].dwAniTable[5] = 56;
    check(afterimage_clip::Body(table, humanArray, 1, 0, 5) == 0, "A15-07 a table value outside the human array yields clip 0");
    check(afterimage_clip::Mount(table, 20, 3) == static_cast<int>(table[20].dwAniTable[3]) &&
        afterimage_clip::Mount(table, 60, 3) == 0, "A16-13 mount after-images use the mount table value");
    const auto packets = ReadClientSource("internal/game/entities/TMHumanPackets.cpp");
    check(!effects.empty() && !packets.empty() &&
        source_contract::MaskCpp(effects).find("m_sAnimationArray[") == std::string::npos &&
        source_contract::MaskCpp(packets).find("m_sAnimationArray[") == std::string::npos,
        "A15-07 no after-image site indexes m_sAnimationArray directly");

    // A15-10: unmapped selectors leave the mount unchanged.
    const auto mounts = ReadClientSource("internal/game/entities/TMHumanMounts.cpp");
    const auto costume = source_contract::Method(mounts, "void  TMHuman::SetMountCostume(unsigned int  index)");
    const auto mapped = MappedMountSelectors(costume);
    bool agrees = mapped.size() > 50;
    for (unsigned int index = 0; index < 256; ++index)
        agrees = agrees && mount_costume::HasVisual(index) == (mapped.count(index) == 1);
    check(agrees, "A15-10 HasVisual agrees with every mapped SetMountCostume case for bytes 0..255");
    check(!mount_costume::HasVisual(54) && !mount_costume::HasVisual(119) && !mount_costume::HasVisual(150) &&
        mount_costume::HasVisual(11) && mount_costume::HasVisual(118), "A15-10 empty and default selectors are unmapped");
    const auto costumeCode = source_contract::MaskCpp(costume);
    const auto early = costumeCode.find("if (!mount_costume::HasVisual(index))");
    const auto reset = costumeCode.find("m_stMountLook.Mesh2 = 0;");
    check(early != std::string::npos && reset != std::string::npos && early < reset,
        "A15-10 SetMountCostume returns before clearing the look for unmapped selectors");

    // A15-13: the field-scene member is read after the scene-type test.
    const auto updateMount = source_contract::MaskCpp(source_contract::Method(mounts, "void TMHuman::UpdateMount()"));
    const auto typeTest = updateMount.find("ESCENE_TYPE::ESCENE_FIELD && pScene->m_bAirMove == 1");
    check(typeTest != std::string::npos && updateMount.find("m_bAirMove") > updateMount.find("ESCENE_TYPE::ESCENE_FIELD"),
        "A15-13 UpdateMount tests owner and scene type before reading m_bAirMove");

    // A16-05: guild-mark slot range and ownership.
    FakeGuildSlot slots[guild_mark_label::kSlots]{};
    slots[0].nGuild = 10 + (1 << 16);
    slots[63].nGuild = 10 + (1 << 16);
    slots[5].nGuild = 12 + (2 << 16);
    check(guild_mark_label::Shows(slots, 0, 10, 1) && guild_mark_label::Shows(slots, 63, 10, 1),
        "A16-05 indices 0 and 63 show the owning guild's mark");
    check(!guild_mark_label::Shows(slots, -1, 10, 1) && !guild_mark_label::Shows(slots, 64, 10, 1) &&
        !guild_mark_label::Shows(slots, 65, 10, 1), "A16-05 indices -1, 64 and 65 are hidden");
    check(guild_mark_label::Shows(slots, 5, 12, 2) && !guild_mark_label::Shows(slots, 5, 12, 1), "A16-05 the channel is part of the key");
    slots[5].nGuild = 99 + (2 << 16);
    check(!guild_mark_label::Shows(slots, 5, 12, 2), "A16-05 a reused slot hides another guild's mark");
    const auto ui = ReadClientSource("internal/game/entities/TMHumanUI.cpp");
    check(!ui.empty() && ui.find("nMark > 64") == std::string::npos &&
        ui.find("guild_mark_label::Shows(") != ui.rfind("guild_mark_label::Shows("),
        "A16-05 both human label paths use the bounded owner test");

    // A16-12: localized challenge-mode captions.
    char mode[64]{};
    challenge_label::Format("Todos:Todos", 3, mode);
    check(std::strcmp(mode, "Todos:Todos") == 0, "A16-12 the PT caption is used for All : All");
    challenge_label::Format("1_:_1", 0, mode);
    check(std::strcmp(mode, "1 : 1") == 0, "A16-12 underscores render as spaces");
    challenge_label::Format("", 3, mode);
    check(std::strcmp(mode, "All : All") == 0, "A16-12 an empty caption falls back to English");
    challenge_label::Format(nullptr, 6, mode);
    check(std::strcmp(mode, "10 : 10") == 0, "A16-12 a missing caption falls back by mode");
    char shortBuffer[4]{};
    challenge_label::Format("All:All", 3, shortBuffer);
    check(std::strcmp(shortBuffer, "All") == 0, "A16-12 the caption is truncated to the buffer");
    bool localized = true;
    for (const char* language : { "EN", "PT" }) {
        const auto strings = ReadRepositoryFile((std::string("tmproject/client748/") + language + "/UI/UIString.txt").c_str());
        for (int row = challenge_label::kFirstUIString; row < challenge_label::kFirstUIString + 4; ++row) {
            const auto key = "\n" + std::to_string(row) + "\t";
            const auto at = strings.find(key);
            localized = localized && at != std::string::npos && strings.size() > at + key.size() &&
                strings[at + key.size()] != '\n';
        }
    }
    check(localized, "A16-12 EN and PT UIString provide rows 64..67");
    const auto ranking = source_contract::MaskCpp(
        source_contract::Method(packets, "int TMHuman::OnPacketReqRanking(MSG_STANDARDPARM2* pStd)"));
    check(Contains(ranking, "challenge_label::Format(g_UIString[challenge_label::kFirstUIString + challengeMode]") &&
        ranking.find("\"All : All\"") == std::string::npos, "A16-12 OnPacketReqRanking uses the localized captions");
    check(ranking.find("GetSceneType() != ESCENE_TYPE::ESCENE_FIELD") < ranking.find("static_cast<TMFieldScene*>"),
        "A16-09 OnPacketReqRanking tests the scene type before the cast");

    // A15-12: remote visuals use the actor's own state.
    const auto play = source_contract::MaskCpp(
        source_contract::Method(effects, "void TMHuman::PlayEffectEvent(unsigned int& dwServerTime, TMFieldScene*& pScene)"));
    check(Contains(play, "if (m_sLeftIndex == 1010)") && play.find("m_stMobData.Equip[6]") == std::string::npos &&
        play.find("m_stEffectEvent.sEffectLevel = 99") == std::string::npos,
        "A15-12 arrow level 99 comes from the archer's own left hand");
    std::size_t targetMounted = 0;
    for (std::size_t at = 0; (at = play.find("if (bTargetMounted)", at)) != std::string::npos; ++at)
        ++targetMounted;
    check(targetMounted == 2, "A15-12 effects 20 and 31 animate the target after-image by the target's mount");
    check(Contains(frame, "new TMSkillSlowSlash(vec, vec, 1, this)") &&
        frame.find("TMSkillSlowSlash(vec, vec, 1, m_stEffectEvent.pTarget)") == std::string::npos,
        "A15-12 Waste follows its owner");
}

// ---- packets --------------------------------------------------------------

void PacketTests(const std::function<void(bool, const char*)>& check)
{
    using illusion_mana::Apply;
    // A16-02: remote casters never change the local MP; the local cost saturates.
    bool remoteUnchanged = true;
    for (const unsigned int mp : { 0u, 44u, 45u, 1000u })
        remoteUnchanged = remoteUnchanged && Apply(mp, 45, false) == mp;
    check(remoteUnchanged, "A16-02 an observed Illusion leaves the local MP unchanged");
    check(Apply(0u, 45, true) == 0u && Apply(44u, 45, true) == 0u && Apply(45u, 45, true) == 0u &&
        Apply(1000u, 45, true) == 955u, "A16-02 the local caster's MP saturates at zero");
    check(Apply(30u, 0, true) == 30u && Apply(30u, -5, true) == 30u, "A16-02 a non-positive cost changes nothing");
    const auto packets = ReadClientSource("internal/game/entities/TMHumanPackets.cpp");
    const auto illusion = source_contract::MaskCpp(
        source_contract::Method(packets, "int TMHuman::OnPacketIllusion(MSG_STANDARD* pStd)"));
    check(Contains(illusion, "illusion_mana::Apply(") && illusion.find("CurMP -=") == std::string::npos,
        "A16-02 OnPacketIllusion uses the owner-checked mana rule");

    // A16-10: selection-cache writes are bounded.
    check(!selection_slot::IsValid(-1) && selection_slot::IsValid(0) && selection_slot::IsValid(3) &&
        !selection_slot::IsValid(4), "A16-10 only slots 0..3 index the selection cache");
    const auto code = source_contract::MaskCpp(packets);
    bool guarded = !code.empty();
    std::size_t writes = 0;
    const std::string slotIndex = "[g_pObjectManager->m_cCharacterSlot]";
    for (std::size_t at = 0; (at = code.find(slotIndex, at)) != std::string::npos; at += slotIndex.size()) {
        ++writes;
        const auto lineStart = code.rfind('\n', at);
        const auto previousStart = code.rfind('\n', lineStart - 1);
        const auto previous = code.substr(previousStart + 1, lineStart - previousStart - 1);
        guarded = guarded && previous.find("selection_slot::IsValid(g_pObjectManager->m_cCharacterSlot)") != std::string::npos;
    }
    check(guarded && writes == 2, "A16-10 every selection-cache write by the selected slot is guarded");

    // A16-04: whole-cache restore and guarded mount angle.
    const auto score = source_contract::MaskCpp(
        source_contract::Method(packets, "int TMHuman::OnPacketUpdateScore(MSG_STANDARD* pStd)"));
    check(Contains(score, "m_stSancInfo = m_stOldSancInfo;") && Contains(score, "m_stMountSanc = m_stOldMountSanc;") &&
        score.find("m_stSancInfo.Sanc0 = m_stOldSancInfo.Sanc0") == std::string::npos &&
        score.find("m_stMountSanc.Sanc0 = m_stOldMountSanc.Sanc0") == std::string::npos,
        "A16-04 shadow end restores all 16 bytes of the sanc and mount caches");
    const auto angle = score.find("m_vOldAngle = m_pMount->m_vAngle;");
    check(angle != std::string::npos && score.rfind("if (bHadMount)", angle) != std::string::npos &&
        Contains(score, "if (bHadMount && m_pMount)"), "A16-04 the mount angle is read and restored only with a mount");

    // A16-09: field members are read after the scene type test.
    const auto remove = source_contract::MaskCpp(
        source_contract::Method(packets, "int TMHuman::OnPacketRemoveMob(MSG_STANDARD* pStd)"));
    check(remove.find("ESCENE_TYPE::ESCENE_FIELD") < remove.find("m_pPGTOver"),
        "A16-09 OnPacketRemoveMob tests the field scene before its members");
    const auto render = ReadClientSource("internal/game/entities/TMHumanRender.cpp");
    const auto ui = ReadClientSource("internal/game/entities/TMHumanUI.cpp");
    check(!render.empty() && !ui.empty() &&
        source_contract::MaskCpp(render).find("&& !g_pCurrentScene->m_pMyHuman->m_JewelGlasses") == std::string::npos &&
        source_contract::MaskCpp(ui).find("&& !g_pCurrentScene->m_pMyHuman->m_JewelGlasses") == std::string::npos &&
        source_contract::MaskCpp(ui).find("\n        !pFScene->m_pMyHuman->m_JewelGlasses") == std::string::npos,
        "A16-09 Shadow visibility tests a missing local human before its glasses");
    check(Contains(render, "(pFocused && m_usGuild == pFocused->m_usGuild)") &&
        Contains(render, "== 15 && pFocused && pFocused->m_cMantua"),
        "A16-09 SetColorMaterial tests the camera focus before reading it");
    check(Contains(ui, "if (pFScene->m_pMiniMapPanel)") && Contains(ui, "m_stGuildMark.nGuild = m_usGuild & 0xFFF;"),
        "A16-09/A16-13 minimap owner test and the guild mask");
}
}

int RunHumanStateSafetyTests(int& checks)
{
    int failures = 0;
    const std::function<void(bool, const char*)> check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };
    GroundItemTests(check);
    MovementTests(check);
    VisualTests(check);
    PacketTests(check);
    return failures;
}
