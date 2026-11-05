#include "../internal/game/entities/EntitySafetyPolicy.h"
#include "../internal/application/ActorLifetime.h"
#include "SourceMethod.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <new>
#include <regex>
#include <string>
#include <vector>
#include <windows.h>

// Lane ENTA: TMHuman core, appearance, combat, TMObject, TMCamera and the
// entity helpers. Behavior tests exercise EntitySafetyPolicy.h; source
// contracts pin that production code calls those decisions.
namespace
{
// Searches upward from the test executable, as the other contract tests do,
// so the result does not depend on the working directory.
std::string ReadSource(const char* relativePath)
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        std::ifstream input(current / "tmproject/TMProject748" / relativePath, std::ios::binary);
        if (input) {
            std::string text{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
            text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
            return text;
        }
        current = current.parent_path();
    }
    return {};
}

// True when the declaration "<anything> name<init>;" carries an initializer.
bool MemberHasInitializer(const std::string& header, const std::string& name)
{
    const std::regex declaration("[\\w:\\*\\s]+\\b" + name + "(\\[\\d+\\])?\\s*(=[^;]+|\\{\\})\\s*;");
    return std::regex_search(header, declaration);
}

int CountOf(const std::string& text, const std::string& needle)
{
    int count = 0;
    for (auto at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size()))
        ++count;
    return count;
}
}

int RunEntitySafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    // A03-03: retirement clears borrowed targets before their storage is reused.
    {
        struct Node { Node* m_pDown = nullptr; Node* m_pNextLink = nullptr; };
        struct Actor : Node {
            struct Event {
                Actor* pTarget = nullptr;
                unsigned int dwTime = 1500;
                float destination = 12.5f;
            } m_stEffectEvent;
        };
        Node humans;
        Actor attacker, observer, unrelated;
        alignas(Actor) unsigned char storage[sizeof(Actor)];
        Actor* target = new (storage) Actor;
        humans.m_pDown = &attacker;
        attacker.m_pNextLink = &observer;
        observer.m_pNextLink = &unrelated;
        attacker.m_stEffectEvent.pTarget = target;
        observer.m_stEffectEvent.pTarget = target;
        unrelated.m_stEffectEvent.pTarget = &attacker;
        actor_lifetime::ClearPendingEffectTargets(target, &humans);
        target->~Actor();
        Actor* replacement = new (storage) Actor;
        check(attacker.m_stEffectEvent.pTarget == nullptr &&
            observer.m_stEffectEvent.pTarget == nullptr &&
            unrelated.m_stEffectEvent.pTarget == &attacker,
            "A03-03 retirement clears all matching targets and preserves unrelated targets");
        check(attacker.m_stEffectEvent.dwTime == 1500 &&
            attacker.m_stEffectEvent.destination == 12.5f &&
            replacement == reinterpret_cast<Actor*>(storage),
            "A03-03 address reuse cannot retarget an event; ground position and due time survive");
        actor_lifetime::ClearPendingEffectTargets(replacement, static_cast<Node*>(nullptr));
        actor_lifetime::ClearPendingEffectTargets(static_cast<Actor*>(nullptr), &humans);
        replacement->~Actor();

        const auto manager = ReadSource("internal/core/ObjectManager.cpp");
        const auto retire = source_contract::Method(manager,
            "void ObjectManager::DisconnectEffectFromMob(TMHuman* pMob)");
        const auto remove = source_contract::Method(manager,
            "void ObjectManager::DeleteObject(TreeNode* pNode)");
        check(retire.find("actor_lifetime::ClearPendingEffectTargets(pMob,") != std::string::npos &&
            source_contract::InOrder(remove.find("actor_lifetime::ClearPendingEffectTargets("),
                remove.find("pNode->m_cDeleted = 1")),
            "A03-03 delayed and immediate retirement both clear pending targets before deletion");
    }

    // ---- A13-06: wrap-safe timers -------------------------------------------
    {
        using human_timer::Elapsed;
        // Ordinary interval: exclusive bound, as the legacy "now > start + n".
        check(!Elapsed(11000u, 1000u, 10000u) && Elapsed(11001u, 1000u, 10000u),
            "timer keeps the exclusive legacy bound without a wrap");
        // A13-06 trigger: the deadline sum stays below 2^32, then the tick wraps.
        const unsigned int start = 0xFFFFFFFFu - 4999u;
        const unsigned int legacyDeadline = start + 1000u;
        const unsigned int afterWrap = 100u;
        check(!(afterWrap > legacyDeadline), "legacy deadline stays false after the wrap (trigger reproduced)");
        check(Elapsed(afterWrap, start, 1000u), "elapsed time crosses the 32-bit wrap");
        // Pre-wrap window where the legacy sum overflowed and fired early.
        const unsigned int nearWrap = 0xFFFFFFFFu - 99u;
        check(!Elapsed(nearWrap + 50u, nearWrap, 10000u) && !Elapsed(9898u, nearWrap, 10000u) &&
            Elapsed(9901u, nearWrap, 10000u), "start near the wrap waits the full interval");
        // Backward resynchronization and "never set" stamps.
        check(!Elapsed(4950u, 5000u, 0u) && !Elapsed(5000u - human_timer::kBackwardTolerance, 5000u, 0u),
            "a stamp slightly ahead of now has not elapsed");
        check(Elapsed(3000000000u, 0u, 1000u) && Elapsed(0x80000001u, 0u, 2000u),
            "a zero stamp elapses at every tick, also above 2^31");
        // Sweep: for every true elapsed delta inside the valid range the
        // answer is delta > interval, for starts on both sides of the wrap.
        bool sweep = true;
        for (const unsigned int base : {0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFF000u, 0xFFFFFFFFu})
            for (const unsigned int interval : {0u, 100u, 999u, 10000u, 600000u})
                for (const unsigned int delta : {0u, 1u, 99u, 100u, 101u, 999u, 1000u, 9999u, 10000u, 10001u,
                         599999u, 600000u, 600001u, 0x7FFFFFFFu, 0xFFFFFFFFu - human_timer::kBackwardTolerance})
                    sweep = sweep && Elapsed(base + delta, base, interval) == (delta > interval);
        check(sweep, "elapsed comparison matches the true delta across the wrap");
    }

    // ---- A13-07: unit time never zero ---------------------------------------
    {
        using human_motion::UnitTime;
        check(UnitTime(0.0f) == 1000u && UnitTime(-3.0f) == 1000u &&
            UnitTime(std::numeric_limits<float>::quiet_NaN()) == 1000u, "no speed uses the legacy 1000 ms cell");
        check(UnitTime(1.0f) == 1000u && UnitTime(2.0f) == 500u && UnitTime(6.0f) == 166u && UnitTime(7.0f) == 142u,
            "native speeds 1..7 keep the legacy cell time");
        check(UnitTime(1001.0f) == 1u && UnitTime(1.0e9f) == 1u &&
            UnitTime(std::numeric_limits<float>::infinity()) == 1u, "out-of-range speeds no longer yield zero");
        bool positive = true;
        for (int speed = 1; speed <= 5000; ++speed)
            positive = positive && UnitTime(static_cast<float>(speed)) >= 1u;
        check(positive, "unit time is positive for every speed");
    }

    // ---- A13-02: animation table bounds -------------------------------------
    {
        using namespace human_animation;
        check(!IsMotion(-1) && !IsMotion(-32768) && IsMotion(0) && IsMotion(27) && !IsMotion(28) && !IsMotion(255),
            "motion domain is ECMOTION_STAND01..ECMOTION_PUNEND");
        bool columns = true;
        for (int motion = -32768; motion <= 32767; ++motion)
            if (IsMotion(motion))
                columns = columns && motion + kMotionCount < kTableColumns;
        check(columns, "accepted motions keep the mounted column inside the 56-column tables");
        check(IsSkinType(0) && IsSkinType(59) && !IsSkinType(60) && !IsSkinType(69) && !IsSkinType(-1),
            "skin domain is the 60 MAX_ANI_TYPE rows (class 41 skin 69 rejected)");

        bool extendedBounds = true;
        bool extendedLegacy = true;
        for (int head = -200; head <= 200; ++head)
        {
            for (int skin = -2; skin <= 70; ++skin)
            {
                const int row = ExtendedClass(head, skin);
                extendedBounds = extendedBounds && (row == -1 ||
                    (row >= 0 && row < kExtendedClasses && skin >= 0 && skin < kExtendedSkins));
                // Legacy row selection, kept for skins 0 and 1 only.
                const bool legacyHead = head < 40 && (head % 10 == 1 || head % 10 > 5);
                if (skin == 0 || skin == 1)
                {
                    int legacyRow = head % 10 == 1 ? head / 10 : head % 10 - 6;
                    if (legacyRow > 3)
                        legacyRow = 0;
                    extendedLegacy = extendedLegacy && (legacyHead ? row == legacyRow : row == -1);
                }
            }
        }
        check(extendedBounds, "extended table row and column stay inside g_MobAniTableEx[4][2]");
        check(extendedLegacy, "skins 0 and 1 keep the legacy class-row selection");
        check(ExtendedClass(11, 0) == 1 && ExtendedClass(16, 1) == 0 && ExtendedClass(39, 0) == 3 &&
            ExtendedClass(21, 2) == -1 && ExtendedClass(21, 5) == -1 && ExtendedClass(40, 0) == -1 &&
            ExtendedClass(5, 0) == -1, "heads 1/6..9 select a class row only for skins 0 and 1");

        static short clips[6][60][56]{};
        for (int s = 0; s < 6; ++s)
            for (int w = 0; w < 60; ++w)
                for (int c = 0; c < 56; ++c)
                    clips[s][w][c] = static_cast<short>(s * 1000 + w * 10 + c % 10);
        check(HumanClip(clips, 2, 0, 55) == 2005 && HumanClip(clips, 5, 59, 0) == 5590,
            "valid human clip lookups return the array value");
        check(HumanClip(clips, 0, 0, 56) == -1 && HumanClip(clips, 0, 0, 0xFFFFFFFFu) == -1 &&
            HumanClip(clips, 6, 0, 0) == -1 && HumanClip(clips, -1, 0, 0) == -1 &&
            HumanClip(clips, 0, 60, 0) == -1 && HumanClip(clips, 0, -1, 0) == -1,
            "AniSound4 clip values outside m_sAnimationArray are rejected");
    }

    // ---- A13-03: cape predicate ---------------------------------------------
    {
        bool predicate = true;
        for (int mantua : {-1, 0, 1, 2, 127})
            for (int skin = -1; skin <= 100; ++skin)
            {
                const bool expected = mantua > 0 && (skin == 0 || skin == 1 || skin == 2 || skin == 3 || skin == 8);
                predicate = predicate && human_cape::MayCarryCape(mantua, skin) == expected;
            }
        check(predicate, "a cape needs a cape item and skin 0/1/2/3/8");
        // The legacy restore predicate let skins 1/2/3/8 through without a cape.
        const auto legacyRestore = [](int mantua, int skin) {
            return (mantua > 0 && !skin) || skin == 1 || skin == 8 || skin == 3 || skin == 2;
        };
        check(legacyRestore(0, 1) && !human_cape::MayCarryCape(0, 1) &&
            legacyRestore(0, 8) && !human_cape::MayCarryCape(0, 8),
            "an actor without a cape no longer gets an empty cape on device restore");
        check(human_cape::CostumeKeepsCape(0) && human_cape::CostumeKeepsCape(100) &&
            !human_cape::CostumeKeepsCape(5) && !human_cape::CostumeKeepsCape(130),
            "costume renderers other than 100 replace the cape");
    }

    // ---- A14-01 / A14-02 / A14-04: equipment indices ------------------------
    {
        using namespace human_equipment;
        check(IsCatalogIndex(0) && IsCatalogIndex(6499) && !IsCatalogIndex(6500) && !IsCatalogIndex(-1) &&
            !IsCatalogIndex(32767) && !IsCatalogIndex(-32768), "catalog domain is 0..MAX_ITEMLIST-1");
        check(CatalogIndex(-1) == 0 && CatalogIndex(6500) == 0 && CatalogIndex(32767) == 0 &&
            CatalogIndex(3500) == 3500 && CatalogIndex(6499) == 6499, "malformed equipment shows an empty slot");

        bool circlets = true;
        for (int item = 0; item <= 0xFFF; ++item)
        {
            const bool mobPath = item >= 3500 && (item <= 3502 || item == 3507);
            const bool packedKeepsHelm = item < 3500 || (item > 3502 && item != 3507);
            circlets = circlets && IsCirclet(item) == mobPath && IsCirclet(item) == !packedKeepsHelm;
        }
        check(circlets, "both equipment projections share the circlet set");
        check(IsCirclet(3500) && IsCirclet(3501) && IsCirclet(3502) && IsCirclet(3507) &&
            !IsCirclet(3503) && !IsCirclet(3506) && !IsCirclet(3508), "Silver/Gold/Mystic/Arcana circlets");

        const int mountTable[28] = {7000, 8000, 8000, 7500, 9000, 9000, 10000, 10000, 11000, 11000, 12000, 10000, 10000,
            11000, 11000, 12000, 12000, 15000, 12000, 23000, 15000, 25000, 10000, 15000, 20000, 20000, 20000, 0};
        check(TableValue(mountTable, 21) == 25000 && TableValue(mountTable, 27) == 0, "in-range mount rows");
        // Catalog horses 3981/3984/3987 map to 28, 3982/3985/3988/2960 to 29, 3989/2961 to 30.
        check(TableValue(mountTable, 28) == 0 && TableValue(mountTable, 29) == 0 && TableValue(mountTable, 30) == 0 &&
            TableValue(mountTable, -1) == 0 && TableValue(mountTable, 1638) == 0, "mount rows past the table have no maximum");
    }

    // ---- A14-03 / A14-09: combat input and skill range ----------------------
    {
        using namespace human_combat;
        const short held = static_cast<short>(0xFF80);
        const short heldToggled = static_cast<short>(0xFF81);
        const short released = 0;
        const short releasedToggled = 1;
        // The legacy tests could not distinguish the states.
        check(((held >> 8) <= 0) && ((released >> 8) <= 0) && !((held >> 8) > 0),
            "signed SHORT shift is constant (trigger reproduced)");
        check(IsKeyDown(held) && IsKeyDown(heldToggled) && IsKeyDown(static_cast<short>(0x8000)) &&
            !IsKeyDown(released) && !IsKeyDown(releasedToggled) && !IsKeyDown(0x7FFF),
            "Shift held is the GetKeyState sign bit");

        check(!UsesSpellMotion(-1) && UsesSpellMotion(0) && UsesSpellMotion(103) && !UsesSpellMotion(104) &&
            !UsesSpellMotion(127) && !UsesSpellMotion(151) && !UsesSpellMotion(199) && UsesSpellMotion(200) &&
            UsesSpellMotion(247) && !UsesSpellMotion(248), "spell motion range of both Attack overloads");
        bool narrowed = true;
        for (int skill = 200; skill < 248; ++skill)
            narrowed = narrowed && static_cast<char>(skill) < 0 && UsesSpellMotion(static_cast<short>(skill));
        check(narrowed, "skills 200..247 are accepted once the index is not narrowed to char");
    }

    // ---- C-23: arrow color index ---------------------------------------------
    {
        bool colors = true;
        for (int level = 1; level <= 1000; ++level)
        {
            const int index = entity_arrow::EffectColorIndex(level);
            colors = colors && index >= 0 && index < 3;
            if (level % 3 != 0)
                colors = colors && index == (level % 3) - 1;
        }
        check(colors, "arrow colors stay in the 3-entry table and keep the defined legacy entries");
        check(entity_arrow::EffectColorIndex(1) == 0 && entity_arrow::EffectColorIndex(2) == 1 &&
            entity_arrow::EffectColorIndex(3) == 2 && entity_arrow::EffectColorIndex(6) == 2,
            "levels that are multiples of three use the third color, not entry -1");
    }

    // ---- Source contracts: production code uses the tested decisions --------
    const auto human = ReadSource("internal/game/entities/TMHuman.cpp");
    const auto humanHeader = ReadSource("internal/game/entities/TMHuman.h");
    const auto animation = ReadSource("internal/game/entities/TMHumanAnimation.cpp");
    const auto appearance = ReadSource("internal/game/entities/TMHumanAppearance.cpp");
    const auto combat = ReadSource("internal/game/entities/TMHumanCombat.cpp");
    check(!human.empty() && !humanHeader.empty() && !animation.empty() && !appearance.empty() && !combat.empty(),
        "TMHuman sources located");

    {
        // C-03: no index past the seven-entry eye-fire arrays.
        const auto initObject = source_contract::Method(human, "int TMHuman::InitObject()");
        check(source_contract::Lacks(initObject, "for (int i = 8; i < 10; ++i)") &&
            initObject.find("m_pEyeFire2[slot] = new TMEffectBillBoard(") != std::string::npos &&
            initObject.find("m_vecTempPos[slot + 7]") != std::string::npos,
            "Emerald Dragon eye fires are written to m_pEyeFire2[1..2]");
        bool literalBounds = true;
        const std::regex literalIndex("m_pEyeFire2?\\[(\\d+)\\]");
        for (const auto* source : {&human, &animation, &appearance, &combat})
            for (std::sregex_iterator it(source->begin(), source->end(), literalIndex), end; it != end; ++it)
                literalBounds = literalBounds && std::stoi((*it)[1].str()) < 7;
        check(literalBounds, "every literal eye-fire index is below seven");
        const auto emerald = source_contract::Method(ReadSource("internal/game/entities/TMHumanEffects.cpp"),
            "void TMHuman::RenderEffect_EmeraldDragon(unsigned int dwServerTime)");
        check(emerald.find("m_pEyeFire2[i]->m_vecPosition = m_vecTempPos[i + 7];") != std::string::npos &&
            emerald.find("for (int i = 1; i <= 2; ++i)") != std::string::npos,
            "the renderer reads the same m_pEyeFire2 slots that InitObject creates");
    }
    {
        // A13-03 / A14-08: one cape rule for creation, restore and refresh.
        const auto create = source_contract::Method(human, "void TMHuman::CreateMantua(int nCos)");
        const auto restore = source_contract::Method(human, "void TMHuman::RestoreDeviceObjects()");
        const auto refresh = source_contract::Method(human, "void TMHuman::RefreshMantua()");
        const auto initObject = source_contract::Method(human, "int TMHuman::InitObject()");
        check(create.find("human_cape::MayCarryCape(m_cMantua, m_nSkinMeshType)") != std::string::npos &&
            create.find("human_cape::CostumeKeepsCape(nCos)") != std::string::npos,
            "cape creation checks the cape item, skin and costume");
        check(restore.find("human_cape::MayCarryCape(m_cMantua, m_nSkinMeshType)") != std::string::npos &&
            restore.find("CreateMantua(m_nCapeCostume)") != std::string::npos &&
            source_contract::Lacks(restore, "new TMSkinMesh(&stLook"),
            "device restore reuses the InitObject cape rule");
        check(source_contract::Before(initObject, "m_nCapeCostume = nCos;", "CreateMantua(nCos);") &&
            source_contract::Lacks(initObject, "new TMSkinMesh(&stLook"),
            "InitObject records the costume used by restore and refresh");
        check(source_contract::Before(refresh, "SAFE_DELETE(m_pMantua);", "CreateMantua(m_nCapeCostume);"),
            "cape refresh rebuilds only the cape mesh");
    }
    {
        // A13-04 / A14-06: members read before their first writer.
        bool initialized = true;
        for (const char* member : {"m_BigMp", "m_MaxBigMp", "m_usMP", "m_bShield2", "m_cCantMove", "m_cCantAttk",
                 "_dwAttackDelay", "m_citizen", "m_dwLastLighten", "m_fTargetHeight", "m_stOldSancInfo",
                 "m_stOldColorInfo", "m_stOldMountSanc", "m_stGuildMark", "TempCarry", "m_nCapeCostume"})
        {
            const bool has = MemberHasInitializer(humanHeader, member);
            if (!has)
                std::fprintf(stderr, "  missing initializer: %s\n", member);
            initialized = initialized && has;
        }
        check(initialized, "TMHuman members read before assignment have default initializers");
        // m_dwDelayDel is assigned in the constructor; the scan must not accept it.
        check(!MemberHasInitializer(humanHeader, "m_dwDelayDel"), "the member scan rejects a member without an initializer");
    }
    {
        // A13-06: no absolute deadline comparisons remain in the timer paths.
        const std::regex absoluteDeadline("(>|<|<=|>=)\\s*\\(?\\s*(pScene->)?m_dw\\w+\\s*\\+|m_dw\\w+\\s*\\+\\s*\\d+\\s*(<|>)");
        bool none = true;
        for (const auto* source : {&human, &animation, &combat})
            none = none && !std::regex_search(*source, absoluteDeadline);
        check(none, "TMHuman timers compare elapsed time instead of absolute deadlines");
        check(CountOf(human, "human_timer::Elapsed(") >= 8 && CountOf(animation, "human_timer::Elapsed(") == 2 &&
            CountOf(combat, "human_timer::Elapsed(") >= 6, "timer sites use the wrap-safe helper");
    }
    {
        // A13-07 / A13-08 / A13-09: frame loop.
        const auto frame = source_contract::WithHelpers(human, "int TMHuman::FrameMove(unsigned int dwServerTime)", "TMHuman");
        check(frame.find("human_motion::UnitTime(m_fMaxSpeed)") != std::string::npos &&
            source_contract::Lacks(frame, "Crashed Character Unit Time"),
            "frame movement never divides by a zero cell time or prints the debug notice");
        check(source_contract::Lacks(frame, "pPartyList") && source_contract::Lacks(frame, "SListBoxPartyItem"),
            "the dead party scan with its null local-actor read is gone");
        check(source_contract::Lacks(human, "skill da foema"), "Portuguese engineering comment translated");
    }
    {
        // A13-02: SetAnimation rejects table indices before any lookup.
        const auto setAnimation = source_contract::Method(animation, "void TMHuman::SetAnimation(ECHAR_MOTION eMotion, int nLoop)");
        check(source_contract::Before(setAnimation,
                "human_animation::IsMotion(static_cast<int>(eMotion)) || !human_animation::IsSkinType(m_nSkinMeshType)",
                "g_MobAniTable[") &&
            source_contract::Lacks(setAnimation, "MeshManager::m_sAnimationArray[m_nSkinMeshType]") &&
            source_contract::Lacks(setAnimation, "m_sHeadIndex % 10") &&
            CountOf(setAnimation, "human_animation::HumanClip(") == 4,
            "SetAnimation bounds motion, skin, extended row and clip lookups");
        const auto frame = source_contract::Method(animation, "void TMHuman::AnimationFrame(int nWalkSndIndex)");
        check(frame.find("human_animation::IsSkinType(m_nSkinMeshType)") != std::string::npos,
            "frame sounds index g_MobAniTable only with a table skin");
    }
    {
        // A14-01 / A14-02 / A14-04: appearance.
        const auto checkWeapon = source_contract::Method(appearance, "void TMHuman::CheckWeapon(short sIndexL, short sIndexR)");
        check(checkWeapon.find("m_sLeftIndex = human_equipment::CatalogIndex(sIndexL);") != std::string::npos &&
            checkWeapon.find("m_sRightIndex = human_equipment::CatalogIndex(sIndexR);") != std::string::npos &&
            source_contract::Lacks(checkWeapon, "m_sLeftIndex = sIndexL;"),
            "weapon indices are validated before PlayAttackSound can read them");
        const auto mobItem = source_contract::Method(appearance, "void TMHuman::SetPacketMOBItem(STRUCT_MOB* pMobData)");
        check(source_contract::Lacks(mobItem, "g_pItemList[pMobData->Equip[") &&
            mobItem.find("human_equipment::CatalogIndex(pMobData->Equip[i].sIndex)") != std::string::npos &&
            mobItem.find("human_equipment::CatalogIndex(pMobData->Equip[15].sIndex)") != std::string::npos,
            "local equipment reads the catalog only through validated indices");
        check(source_contract::Lacks(mobItem, "g_nMountHPTable[nMountMaxHPIndex]") &&
            mobItem.find("mount_health::Maximum(tempIndex)") != std::string::npos,
            "mount maximum HP never indexes past g_nMountHPTable");
        const auto equipItem = source_contract::Method(appearance, "void TMHuman::SetPacketEquipItem(unsigned short* sEquip)");
        const auto circlet = equipItem.find("if (!human_equipment::IsCirclet(sEquip[1] & 0xFFF))");
        const auto clear = equipItem.find("m_stLookInfo.HelmMesh = 0;", circlet);
        check(source_contract::InOrder(circlet, clear) && equipItem.find("m_stLookInfo.HelmSkin = 0;", clear) != std::string::npos,
            "a circlet clears the previous helmet for observers");
    }
    {
        // A14-03 / A14-09: combat.
        const std::regex shiftedKeyState("GetKeyState\\([^)]*\\)\\s*>>|sVal\\s*>>");
        check(!std::regex_search(combat, shiftedKeyState) &&
            CountOf(combat, "human_combat::IsKeyDown(GetKeyState(VK_SHIFT))") == 3,
            "the three Shift tests read the key-down bit");
        const auto positionAttack = source_contract::Method(combat,
            "void TMHuman::Attack(ECHAR_MOTION eMotion, TMVector2 vecTarget, short sSkillIndex)");
        const auto targetAttack = source_contract::Method(combat,
            "void TMHuman::Attack(ECHAR_MOTION eMotion, TMHuman* pTarget, short cSkillIndex)");
        check(positionAttack.find("human_combat::UsesSpellMotion(sSkillIndex)") != std::string::npos &&
            targetAttack.find("human_combat::UsesSpellMotion(cSkillIndex)") != std::string::npos,
            "both Attack overloads share the spell-motion range");
        const auto sound = source_contract::Method(combat, "void TMHuman::PlayAttackSound(ECHAR_MOTION eMotion, int nLR)");
        check(sound.find("g_pItemList[m_sRightIndex].nReqLvl > 90 && nLR == 1") != std::string::npos,
            "the right-hand weapon selects the right swing sound");
        const auto autoAttack = source_contract::Method(combat, "int TMHuman::MAutoAttack(TMHuman* pTarget, int mode)");
        check(autoAttack.find("human_timer::Elapsed(g_pTimerManager->GetServerTime(), _dwAttackDelay, 2000)") != std::string::npos,
            "the post-portal C.C. pause compares server times");
    }
    {
        // C-19: camera members exist before InitCamera reads them.
        const auto camera = ReadSource("internal/render/camera/TMCamera.h");
        bool initialized = !camera.empty();
        int scalars = 0;
        const std::regex scalarMember("\\s*(int|float|unsigned int|TMObject\\*)\\s+(\\w+)\\s*([^;(]*);\\s*");
        std::size_t lineStart = 0;
        while (lineStart < camera.size())
        {
            auto lineEnd = camera.find('\n', lineStart);
            if (lineEnd == std::string::npos)
                lineEnd = camera.size();
            const auto line = camera.substr(lineStart, lineEnd - lineStart);
            std::smatch match;
            if (std::regex_match(line, match, scalarMember))
            {
                ++scalars;
                const auto init = match[3].str();
                const bool has = init.find('=') != std::string::npos || init.find('{') != std::string::npos;
                if (!has)
                    std::fprintf(stderr, "  camera member without initializer: %s\n", match[2].str().c_str());
                initialized = initialized && has;
            }
            lineStart = lineEnd + 1;
        }
        check(initialized && scalars >= 30, "every TMCamera scalar has a default initializer");
        check(camera.find("int m_nQuaterView = -1;") != std::string::npos &&
            camera.find("float m_fMaxCamLen = 20.0f;") != std::string::npos,
            "the constructor's SetViewMode(1) runs with the 20-unit maximum length");
    }
    {
        // C-09: TMObject focus and temporary positions.
        const auto object = ReadSource("internal/game/entities/TMObject.cpp");
        const auto inTown = source_contract::Method(object, "int TMObject::IsInTown()");
        const auto camPos = source_contract::Method(object, "int TMObject::isCamPos()");
        const auto reset = source_contract::Method(object, "void TMObject::ResetObject()");
        check(source_contract::Lacks(inTown, "GetFocusedObject()->m_vecPosition") &&
            source_contract::Before(inTown, "if (pFocused == nullptr)", "pFocused->m_vecPosition"),
            "IsInTown tolerates a camera without focus");
        check(source_contract::Before(camPos, "pCamera->m_pFocusedObject == nullptr", "pCamera->m_pFocusedObject->m_vecPosition"),
            "isCamPos tolerates a camera without focus");
        check(reset.find("for (auto& vecTempPos : m_vecTempPos)") != std::string::npos &&
            source_contract::Lacks(reset, "i < 10;"), "ResetObject clears all eleven temporary positions");
    }
    {
        // C-23: entity helpers.
        const auto arrow = ReadSource("internal/game/entities/TMArrow.cpp");
        // The constructor has an initializer list, so take the text up to the destructor.
        const auto constructorStart = arrow.find(
            "TMArrow::TMArrow(TMVector3 vecStart, TMVector3 vecTarget, int nLevel, int nType, char cAvatar, int nColor, int nDestID)");
        const auto constructorEnd = arrow.find("TMArrow::~TMArrow()");
        const auto constructor = source_contract::InOrder(constructorStart, constructorEnd)
            ? arrow.substr(constructorStart, constructorEnd - constructorStart) : std::string{};
        check(source_contract::Lacks(arrow, "(nLevel % 3) - 1") &&
            arrow.find("dwColor[entity_arrow::EffectColorIndex(nLevel)]") != std::string::npos,
            "bow trail color never reads entry -1");
        check(source_contract::Before(constructor, "m_nMeshIndex = -1;", "switch (m_nType)") &&
            source_contract::Before(constructor, "m_dwLifeTime = 0;", "switch (m_nType)"),
            "arrow mesh and lifetime have defaults for unlisted types and levels");
        check(source_contract::Method(arrow, "TMArrow::~TMArrow()").find("SAFE_DELETE(m_pEffectMesh);") != std::string::npos,
            "an arrow destroyed by teardown frees its owned effect mesh");
        check(source_contract::Lacks(arrow, "if (g_pCurrentScene->m_pMyHuman->m_bCritical)") &&
            arrow.find("g_pCurrentScene->m_pMyHuman && g_pCurrentScene->m_pMyHuman->m_bCritical") != std::string::npos,
            "arrow impact tolerates a scene without a local actor");
        const auto butterfly = source_contract::Method(ReadSource("internal/game/entities/TMButterFly.cpp"),
            "TMButterFly::TMButterFly(int nType, int nBD, TMObject* pOwner)");
        check(butterfly.find("m_fParticleV = ") != std::string::npos && CountOf(butterfly, "m_fParticleH = ") == 1,
            "butterfly initializes the vertical amplitude it reads every frame");
        const auto gate = source_contract::Method(ReadSource("internal/game/entities/TMGate.cpp"),
            "void TMGate::InitGate(STRUCT_ITEM stItem)");
        check(gate.find("human_equipment::IsCatalogIndex(stItem.sIndex)") != std::string::npos &&
            source_contract::Lacks(gate, "m_stItem.sIndex <= 6500"), "gate validates the new item index");
        bool focus = true;
        for (const char* path : {"internal/game/entities/TMBike.cpp", "internal/game/entities/TMDrop.cpp"})
            focus = focus && source_contract::Lacks(ReadSource(path), "m_pCamera->m_pFocusedObject->m_vecPosition");
        check(focus, "bike and rain sounds tolerate a camera without focus");
    }

    {
        const auto movement = ReadSource("internal/game/entities/TMHumanMovement.cpp");
        const auto route = source_contract::Method(movement, "void TMHuman::GetRoute(IVector2 vecTarget, int nCount, int bStop)");
        check(source_contract::Before(route, "m_pendingMoveTarget.Set(", "BASE_GetRoute(") &&
            source_contract::Before(route, "if (!SendOneMessage(", "g_pCurrentScene->OnPacketEvent(MSG_Action_Opcode"),
            "movement defers local prediction and stop application until send acceptance");
        const auto init = source_contract::Method(movement, "void TMHuman::InitPosition(float fX, float fY, float fZ)");
        check(init.find("m_pendingMoveTarget.Clear();") != std::string::npos,
            "teleport and authoritative position reset cancel old movement intent");
        const auto scene = source_contract::Method(ReadSource("internal/app/scenes/TMScene.cpp"), "TMScene::~TMScene()");
        const auto manager = source_contract::Method(ReadSource("internal/core/ObjectManager.cpp"), "ObjectManager::~ObjectManager()");
        check(source_contract::Lacks(scene, "CleanUp(") &&
            source_contract::Before(manager, "g_pCurrentScene = nullptr;", "SAFE_DELETE(m_pRoot);"),
            "scene destruction cannot reenter cleanup and shutdown clears the borrowed scene first");
        const auto container = ReadSource("internal/ui/SControlContainer.cpp");
        const auto destroy = source_contract::Method(container, "SControlContainer::~SControlContainer()");
        const auto discard = source_contract::Method(container, "void SControlContainer::DiscardFrameReferences()");
        const auto transition = source_contract::Method(ReadSource("internal/core/ObjectManager.cpp"),
            "void ObjectManager::SetCurrentScene(TMScene* pScene)");
        check(discard.find("render_control_list::Discard(list)") != std::string::npos &&
            source_contract::Before(destroy, "DiscardFrameReferences();", "SAFE_DELETE(m_pControlRoot)") &&
            source_contract::Lacks(destroy, "SetFocused(") &&
            source_contract::Before(transition, "DiscardFrameReferences();", "g_pCurrentScene = pScene;"),
            "scene retirement detaches geometry and terminal disposal cannot read dead focus or draw nodes");
        const auto render = source_contract::Method(ReadSource("internal/app/scenes/NewApp.cpp"), "HRESULT NewApp::RenderScene()");
        check(render.find("clearControls([frameScene]") != std::string::npos,
            "frame cleanup retains its original scene until deferred retirement");
        const auto item = source_contract::Method(ReadSource("internal/game/entities/TMItem.cpp"), "TMItem::~TMItem()");
        const auto cursor = source_contract::Method(ReadSource("internal/ui/controls/SCursor.cpp"), "SCursor::~SCursor()");
        check(item.find("if (g_pCurrentScene &&") != std::string::npos &&
            cursor.find("g_pCurrentScene ? g_pCurrentScene->m_pControlContainer : nullptr") != std::string::npos,
            "item and pickup cursor teardown accept a cleared current scene");
    }
    return failures;
}
