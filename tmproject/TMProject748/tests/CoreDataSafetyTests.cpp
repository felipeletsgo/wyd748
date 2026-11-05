// ArchitectureTests.vcxproj disables C4201 for the nameless unions in core structs.
#pragma warning(push)
#pragma warning(disable: 4201)
#include "../internal/core/Basedef.h"
#pragma warning(pop)
#include "../internal/core/Structures.h"
#include "../internal/core/BaseChecksum.h"
#include "../internal/application/CatalogItemSlots.h"
#include "../internal/application/FieldCoinInputPolicy.h"
#include "../internal/application/ImeContextBinding.h"
#include "../internal/application/ItemPriceOverride.h"
#include "../internal/application/OwnedSoundBuffers.h"
#include "../internal/application/RetiredScenes.h"
#include "../internal/application/SkillDamagePreview.h"
#include "SkillPredictionFixtures.h"
#include "EquipmentDamageFixtures.h"
#include "../internal/application/SkillValidityPolicy.h"
#include "../internal/wire/AttackVisualDamage.h"

#include "../internal/application/CDataIntegrity.h"
#include "../internal/application/MissingAssetCache.h"
#include "../internal/application/PendingMoveTarget.h"
#include "../internal/application/EffectEmissionClock.h"
#include "../internal/application/EquipmentAppearanceContract.h"
#include "../internal/application/MountHealthContract.h"
#include <array>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
struct FakeBuffer
{
    void Release() {}
};
static_assert(!std::is_copy_constructible_v<OwnedSoundBuffers<FakeBuffer>> &&
        !std::is_copy_assignable_v<OwnedSoundBuffers<FakeBuffer>> &&
        !std::is_move_constructible_v<OwnedSoundBuffers<FakeBuffer>> &&
        !std::is_move_assignable_v<OwnedSoundBuffers<FakeBuffer>>,
    "A08-05: COM buffer ownership must not be duplicated");

// Writes a DMGX tail with one value per target after the opcode's prefix.
std::array<unsigned char, 160> DmgxFrame(unsigned int base, const std::vector<std::uint32_t>& values)
{
    std::array<unsigned char, 160> bytes{};
    const std::uint32_t signature = kAttackWideSignature;
    const std::uint32_t count = static_cast<std::uint32_t>(values.size());
    std::memcpy(bytes.data() + base, &signature, sizeof(signature));
    std::memcpy(bytes.data() + base + 4, &count, sizeof(count));
    for (std::size_t i = 0; i < values.size(); ++i)
        std::memcpy(bytes.data() + base + 8 + i * 4, &values[i], sizeof(values[i]));
    return bytes;
}

struct CountedScene
{
    static int liveCount;
    static int destroyed;
    static RetiredScenes<CountedScene>* reenter;
    CountedScene() { ++liveCount; }
    ~CountedScene()
    {
        --liveCount;
        ++destroyed;
        // A scene destructor re-enters CleanUp; model the nested drain.
        if (reenter)
            for (CountedScene* scene : reenter->TakeAll())
                delete scene;
    }
};
int CountedScene::liveCount = 0;
int CountedScene::destroyed = 0;
RetiredScenes<CountedScene>* CountedScene::reenter = nullptr;

struct FakeImeApi
{
    int nextContext = 100;
    int creates = 0;
    std::vector<std::pair<int, int>> associations; // window, context
    std::vector<int> destroyed;
    int associated = 7; // context associated before the client installs its own
    bool failCreate = false;

    int Create()
    {
        ++creates;
        return failCreate ? 0 : nextContext++;
    }
    int Associate(int window, int context)
    {
        associations.emplace_back(window, context);
        const int previous = associated;
        associated = context;
        return previous;
    }
    void Destroy(int context) { destroyed.push_back(context); }
};
}

int RunCoreDataSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    // A31-03: asset loader tests use the same implementation as BASE_GetSum.
    const char checksumFixture[] = { 7, 7, 7, 7, 7, 7, 7 };
    check(base_checksum::Sum(checksumFixture, 7) == 290 &&
        base_checksum::Sum(checksumFixture, 0) == 0 &&
        base_checksum::Sum(checksumFixture, -1) == 0,
        "legacy checksum preserves the seven arithmetic branches and empty input");

    // A08-01: currency prompts bound amounts to the 32-bit wire domain.
    {
        const auto coin = [] { return 2147483647; };
        const auto validate = [&](int mode, const char* text) {
            char buffer[32]{};
            std::memcpy(buffer, text, std::strlen(text) + 1);
            const int length = static_cast<int>(std::strlen(buffer));
            const bool nonDigit = coin_input::ScanAndSanitize(buffer, length);
            return coin_input::Validate(mode, std::strlen(buffer), nonDigit, _atoi64(buffer), coin);
        };
        for (int mode : {1, 2, 7}) {
            check(validate(mode, "2147483647") == coin_input::Rejection::None,
                "A08-01: INT32_MAX remains a valid amount");
            check(validate(mode, "2147483648") == coin_input::Rejection::InvalidAmount,
                "A08-01: INT32_MAX + 1 is rejected");
            check(validate(mode, "9999999999") == coin_input::Rejection::InvalidAmount,
                "A08-01: ten digits above the wire domain are rejected");
            check(validate(mode, "12345678901") == coin_input::Rejection::InvalidAmount,
                "A08-01: eleven digits are rejected before the display format");
            check(validate(mode, "99999999999999999999") == coin_input::Rejection::InvalidAmount,
                "A08-01: twenty digits (saturated parse) are rejected");
            check(validate(mode, "12a") == coin_input::Rejection::InvalidAmount,
                "A08-01: non-digits stay rejected");
            check(validate(mode, "0") == coin_input::Rejection::None &&
                validate(mode, "1500") == coin_input::Rejection::None,
                "A08-01: ordinary amounts stay valid");
        }
        check(validate(0, "2147483648") == coin_input::Rejection::InvalidAmount,
            "A08-01: deposit stays bounded by the carried coin");
        check(validate(4, "3000000000") == coin_input::Rejection::AutoTradeAboveLimit,
            "A08-01: store price keeps its own limit message");
        check(validate(3, "12345678901234567890") == coin_input::Rejection::None &&
            validate(8, "12345678901") == coin_input::Rejection::None,
            "A08-01: text prompts are not currency amounts");
        // Every accepted trade amount fits the scene's 11-byte "%10lld" field.
        for (long long accepted : {0LL, 9LL, 999999999LL, 2147483647LL}) {
            char field[11]{};
            const int written = std::snprintf(field, sizeof(field), "%10lld", accepted);
            check(written > 0 && written < static_cast<int>(sizeof(field)) &&
                coin_input::Validate(1, 10, false, accepted, coin) == coin_input::Rejection::None,
                "A08-01: accepted amounts fit the trade display field");
        }
    }

    // A08-02: a slot beyond the opcode's target capacity projects no damage.
    {
        std::array<unsigned char, 52> physical{};
        const std::uint32_t hit = 70000;
        std::memcpy(physical.data() + 48, &hit, sizeof(hit));
        check(attack_visual::ReadDamage(physical.data(), 52, MSG_Attack_One_Opcode, 0, 32767) == 70000,
            "A08-02: physical hit slot 0 keeps its wide value");
        check(attack_visual::ReadDamage(physical.data(), 52, MSG_Attack_One_Opcode, 1, 32767) == 0,
            "A08-02: one-target flank slot does not repeat the hit");
        const auto one = DmgxFrame(48, {12345});
        check(attack_visual::ReadDamage(one.data(), 60, MSG_Attack_One_Opcode, 1, 5) == 0,
            "A08-02: one-target skill frame has no second slot");
        const auto two = DmgxFrame(52, {111, 222});
        check(attack_visual::ReadDamage(two.data(), 68, MSG_Attack_Two_Opcode, 1, 5) == 222 &&
            attack_visual::ReadDamage(two.data(), 68, MSG_Attack_Two_Opcode, 2, 5) == 0,
            "A08-02: two-target frames keep slot 1 and reject slot 2");
        std::vector<std::uint32_t> thirteen(13, 9);
        thirteen[12] = 4321;
        const auto multi = DmgxFrame(96, thirteen);
        check(attack_visual::ReadDamage(multi.data(), 156, MSG_Attack_Multi_Opcode, 12, 5) == 4321,
            "A08-02: multi-target frames keep all thirteen slots");
    }

    // A08-03: a DMGX slot equal to the negated WORD identifies a 1..5 HP heal.
    {
        for (int heal = 1; heal <= 5; ++heal) {
            const auto frame = DmgxFrame(48, {static_cast<std::uint32_t>(heal)});
            check(attack_visual::IsTailConfirmedSmallHeal(frame.data(), 60, MSG_Attack_One_Opcode, 0, -heal),
                "A08-03: small heal with matching tail is a heal, not a sentinel");
        }
        const auto miss = DmgxFrame(48, {0});
        check(!attack_visual::IsTailConfirmedSmallHeal(miss.data(), 60, MSG_Attack_One_Opcode, 0, -3),
            "A08-03: WYD-Go miss (zero tail) stays a miss");
        const auto heal3 = DmgxFrame(48, {3});
        check(!attack_visual::IsTailConfirmedSmallHeal(heal3.data(), 48, MSG_Attack_One_Opcode, 0, -3) &&
            !attack_visual::IsTailConfirmedSmallHeal(heal3.data(), 0, MSG_Attack_One_Opcode, 0, -3),
            "A08-03: frames without a tail and local previews keep the sentinel");
        check(!attack_visual::IsTailConfirmedSmallHeal(heal3.data(), 60, MSG_Attack_One_Opcode, 0, -4) &&
            !attack_visual::IsTailConfirmedSmallHeal(heal3.data(), 60, MSG_Attack_One_Opcode, 0, 3) &&
            !attack_visual::IsTailConfirmedSmallHeal(heal3.data(), 60, MSG_Attack_One_Opcode, 0, -6),
            "A08-03: mismatched tails, damage and ordinary heals are not small heals");
        check(!attack_visual::IsTailConfirmedSmallHeal(heal3.data(), 60, MSG_Attack_One_Opcode, 1, -3) &&
            !attack_visual::IsTailConfirmedSmallHeal(nullptr, 60, MSG_Attack_One_Opcode, 0, -3),
            "A08-03: absent slots and packets are not heals");
        check(attack_visual::ReadHeal(heal3.data(), 60, MSG_Attack_One_Opcode, 0, -3) == -3,
            "A08-03: ReadHeal keeps sentinel values unchanged");
    }

    // A09-02: ItemPrice.bin is validated as a whole before any catalog write.
    {
        const auto file = [](std::initializer_list<std::int32_t> words) {
            std::vector<unsigned char> bytes(words.size() * 4);
            std::size_t offset = 0;
            for (std::int32_t word : words) {
                std::memcpy(bytes.data() + offset, &word, 4);
                offset += 4;
            }
            return bytes;
        };
        item_price_override::Entry entries[MAX_ITEM_PRICE_REPLACE]{};
        const auto parse = [&](const std::vector<unsigned char>& bytes) {
            return item_price_override::Parse(bytes.data(), bytes.size(), MAX_ITEM_PRICE_REPLACE,
                MAX_ITEMLIST, entries);
        };
        check(item_price_override::Parse(nullptr, 0, MAX_ITEM_PRICE_REPLACE, MAX_ITEMLIST, entries) == 0,
            "A09-02: empty file has no overrides");
        check(parse(file({412, 100, 6499, 7, 0, 0, -1, 5})) == 2 &&
            entries[0].index == 412 && entries[0].price == 100 && entries[1].index == 6499,
            "A09-02: rows up to the zero terminator are applied; later rows are ignored");
        check(parse(file({412, 100, -1, 5})) == -1, "A09-02: negative index rejects the file");
        check(parse(file({412, 100, MAX_ITEMLIST, 5})) == -1, "A09-02: index 6500 rejects the file");
        std::vector<unsigned char> partial = file({412, 100, 413});
        check(parse(partial) == -1, "A09-02: a truncated row rejects the file");
        std::vector<std::int32_t> many;
        for (int i = 0; i <= MAX_ITEM_PRICE_REPLACE; ++i) {
            many.push_back(1 + i);
            many.push_back(10);
        }
        std::vector<unsigned char> oversize(many.size() * 4);
        std::memcpy(oversize.data(), many.data(), oversize.size());
        check(parse(oversize) == -1, "A09-02: more than 100 rows rejects the file");
        oversize.resize(MAX_ITEM_PRICE_REPLACE * item_price_override::kRowBytes);
        check(parse(oversize) == MAX_ITEM_PRICE_REPLACE, "A09-02: exactly 100 rows are accepted");
    }

    // A09-03: Celestial thresholds follow the server's level * 20,000,000.
    {
        bool linear = true;
        for (int level = 0; level < static_cast<int>(_countof(g_pNextLevel_G2)); ++level)
            linear = linear && g_pNextLevel_G2[level] == level * 20000000LL;
        check(linear, "A09-03: every Celestial threshold is level * 20,000,000");
        check(g_pNextLevel_G2[199] == 3980000000LL && g_pNextLevel_G2[200] == 4000000000LL,
            "A09-03: the server's last two Celestial thresholds");
    }

    // A09-05: class-skill gate checks the spell table, class tree and learned bits.
    {
        for (int index = -2; index < 260; ++index)
            for (int mobClass = 0; mobClass < 4; ++mobClass)
                for (unsigned int learned : {0u, 0xFFFFFFFFu, 1u << 5, 1u << 25}) {
                    bool expected = false;
                    if (index >= 0 && index < 96)
                        expected = index / 24 == mobClass && ((learned >> (index % 24)) & 1u);
                    else if (index >= 96 && index < 104)
                        expected = ((learned >> (index - 72)) & 1u) != 0;
                    else if (index >= 104 && index < MAX_SPELL_LIST)
                        expected = true;
                    check(skill_validity::IsValidClassSkill(index, mobClass, learned) == expected,
                        "A09-05: class-skill decision table");
                }
        check(!skill_validity::IsValidClassSkill(97, 0, 0) &&
            skill_validity::IsValidClassSkill(97, 0, 1u << 25),
            "A09-05: special skill requires its learned bit");
        check(!skill_validity::IsValidClassSkill(250, 0, 0xFFFFFFFFu) &&
            !skill_validity::IsValidClassSkill(-1, 0, 0xFFFFFFFFu),
            "A09-05: indexes outside g_pSpell are rejected");
        // IsValidSkill keeps its decisions for every defined shift.
        for (int index = 200; index < 247; ++index) {
            if (index == 205 || index == 233 || index == 238)
                continue;
            const int shift = 4 * ((index - 200) / 4);
            const bool expected = shift < 32 && ((0xFFFFFFFFu >> shift) & 1u);
            check(skill_validity::IsValidSkill(index, 0, 0xFFFFFFFFu) == expected,
                "A09-05: newer-base skill bits use bounded shifts");
        }
        check(skill_validity::IsValidSkill(205, 0, 0x20) && skill_validity::IsValidSkill(233, 0, 0x200) &&
            skill_validity::IsValidSkill(238, 0, 4) && !skill_validity::IsValidSkill(205, 0, 0),
            "A09-05: special newer-base bits are unchanged");
        check(skill_validity::IsValidSkill(31, 1u << 7, 0) && !skill_validity::IsValidSkill(31, 0, 0) &&
            skill_validity::IsValidSkill(103, 1u << 31, 0) && !skill_validity::IsValidSkill(247, 0, 0xFFFFFFFFu),
            "A09-05: class and special skills keep their learned-bit tests");
    }

    // A09-01: preview corrections that align with the coordinated score contract.
    {
        for (const auto& v : equipment_damage_fixture::Cases)
            check(skill_damage_preview::EquipmentDamage(v[5], v[3], v[4], v[1], v[2], v[0], v[6]) == v[7],
                "shared equipment fixture: damage input matches Go authority");
        for (int refinement = 0; refinement <= 17; ++refinement) {
            const int expected[] = {100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 220, 250, 280, 320, 370, 400, 7};
            check(skill_damage_preview::RefinedDamage(100, refinement) == expected[refinement],
                "equipment damage retains native refinement and effective +17 fallback");
        }
        check(skill_damage_preview::InstanceBase(10, 30, true, {0, 400, INT_MAX, INT_MAX, INT_MAX, 0, INT_MAX}) == INT_MAX,
            "wide instance products saturate without signed overflow");
        for (const auto& v : skill_prediction_fixture::Cases) {
            const skill_damage_preview::Caster caster{v[0], v[5], v[6], v[8], v[9], v[11], v[10]};
            check(skill_damage_preview::Damage(v[1], v[2], v[3], v[4], v[15], v[7],
                caster, v[12], v[13], v[14], v[16]) == v[20],
                "shared skill fixture: damage matches Go authority");
            check(skill_damage_preview::ManaCost(v[17], v[1] >= 96 ? v[5] : v[6], v[18]) == v[21],
                "shared skill fixture: mana matches Go authority");
            check(skill_damage_preview::CooldownSeconds(v[1], v[19]) == v[22],
                "shared skill fixture: cooldown matches Go authority");
        }
        const skill_damage_preview::Caster casters[] = {
            {0, 120, 50, 200, 300, 900, 400}, {1, 120, 50, 200, 300, 900, 400},
            {2, 120, 50, 200, 300, 900, 400}, {3, 120, 50, 200, 300, 900, 400}};
        for (const auto& caster : casters)
            for (bool transformed : {false, true})
                check(skill_damage_preview::InstanceBase(97, 30, transformed, caster) == 30 + 15 * 120,
                    "A09-01: Conjure Cannon is base + 15 * level for every class");
        // Native TMSRVIN 0x4C3DE0 fixtures; transformed faces retain INT/40.
        check(skill_damage_preview::InstanceBase(10, 30, false, casters[0]) ==
                3 * 400 + 3 * 200 + 60 + 50 + 30,
            "A09-01: untransformed physical TK skill");
        check(skill_damage_preview::InstanceBase(20, 30, false, casters[0]) ==
                400 + 75 + 60 + 50 + 30 + 7,
            "A09-01: untransformed magic TK skill");
        check(skill_damage_preview::InstanceBase(20, 30, true, casters[0]) ==
                400 + 75 + 120 + 50 + 30 + 7,
            "A09-01: transformed magic TK skill");
        check(skill_damage_preview::InstanceBase(30, 30, false, casters[1]) == 10 + 100 + 60 + 30 + 50 &&
            skill_damage_preview::InstanceBase(30, 30, true, casters[2]) == 10 + 100 + 120 + 30 + 100,
            "A09-01: FM and BM magic skills");
        check(skill_damage_preview::InstanceBase(95, 30, false, casters[3]) == 900 &&
            skill_damage_preview::InstanceBase(80, 30, true, casters[3]) == 1200 + 600 + 60 + 50 + 30,
            "A09-01: HT skills");
        check(skill_damage_preview::InstanceBase(10, 30, false, {5, 1, 1, 1, 1, 1, 1}) == 0,
            "A09-01: unknown classes keep zero damage");
        check(skill_damage_preview::Amplify(1000, 300) == 5 * (1000 * 1300 / 100) / 4 &&
            skill_damage_preview::Amplify(1000, 44) == 5 * (1000 * 276 / 100) / 4,
            "A09-01: MagicAmp above 255 is not truncated");
        check(skill_damage_preview::Amplify(INT_MAX, 0xFFFFFFFFu) == INT_MAX &&
            skill_damage_preview::Amplify(0, 0xFFFFFFFFu) == 0,
            "A09-01: amplification saturates instead of overflowing");
        check(skill_damage_preview::MagicAmpDamage(300) == 600 &&
            skill_damage_preview::MagicAmpDamage(0xFFFFFFFFu) == INT_MAX,
            "A09-01: MagicAmp damage uses the full value");
    }

    // A09-10: 0x114 snapshot slots outside the catalog become empty slots.
    {
        STRUCT_MOB mob{};
        mob.Equip[0].sIndex = 1;
        mob.Equip[6].sIndex = MAX_ITEMLIST;
        mob.Equip[6].stEffect[0].cEffect = 9;
        mob.Equip[7].sIndex = -5;
        mob.Carry[3].sIndex = MAX_ITEMLIST - 1;
        mob.Carry[4].sIndex = 32000;
        const int equipCleared = catalog_item_slots::ClearOutOfCatalog(mob.Equip, MAX_ITEMLIST);
        const int carryCleared = catalog_item_slots::ClearOutOfCatalog(mob.Carry, MAX_ITEMLIST);
        check(equipCleared == 2 && carryCleared == 1, "A09-10: only out-of-catalog slots are cleared");
        check(mob.Equip[0].sIndex == 1 && mob.Equip[6].sIndex == 0 &&
            mob.Equip[6].stEffect[0].cEffect == 0 && mob.Equip[7].sIndex == 0 &&
            mob.Carry[3].sIndex == MAX_ITEMLIST - 1 && mob.Carry[4].sIndex == 0,
            "A09-10: valid slots stay unchanged; invalid slots become empty");
    }

    // A11-S1: `= {}` on 2D vectors yields zero instead of a null dereference.
    {
        TMVector2 vector(3.0f, 4.0f);
        vector = {};
        check(vector.x == 0.0f && vector.y == 0.0f, "A11-S1: TMVector2 empty assignment");
        IVector2 grid(5, 6);
        grid = {};
        check(grid.x == 0 && grid.y == 0, "A11-S1: IVector2 empty assignment");
        TMVector2 source(1.5f, 2.5f);
        vector = &source;
        IVector2 gridSource(7, 8);
        grid = &gridSource;
        check(vector.x == 1.5f && vector.y == 2.5f && grid.x == 7 && grid.y == 8,
            "A11-S1: pointer copy is unchanged");
    }

    // C-13: two transitions before one CleanUp delete both replaced scenes once.
    {
        CountedScene::liveCount = 0;
        CountedScene::destroyed = 0;
        RetiredScenes<CountedScene> retired;
        CountedScene::reenter = &retired;
        retired.Retire(nullptr);
        check(retired.Size() == 0, "C-13: no scene is not retired");
        auto* first = new CountedScene();
        auto* second = new CountedScene();
        retired.Retire(first);  // second transition retires the older previous scene
        CountedScene* previous = second;
        std::vector<CountedScene*> taken = retired.TakeAll();
        check(retired.Size() == 0 && taken.size() == 1, "C-13: the list is detached before deletion");
        retired.Retire(new CountedScene()); // a nested destructor finds this one
        delete previous;
        for (CountedScene* scene : taken)
            delete scene;
        check(CountedScene::liveCount == 0 && CountedScene::destroyed == 3,
            "C-13: each replaced scene is destroyed exactly once, including re-entry");
        auto* kept = new CountedScene();
        retired.Retire(kept);
        retired.Forget();
        check(retired.TakeAll().empty() && CountedScene::liveCount == 1,
            "C-13: shutdown forgets borrowed scenes for the root to delete");
        CountedScene::reenter = nullptr;
        delete kept;
    }

    // C-26: the IME binding restores the replaced context and destroys only its own.
    {
        FakeImeApi api;
        ime_context::Binding<int, int> binding;
        binding.Restore(api);
        check(api.associations.empty() && api.destroyed.empty(),
            "C-26: restore without install changes nothing");
        check(binding.Install(api, 42) && binding.Installed() && binding.Previous() == 7 &&
            api.associated == 100, "C-26: install associates the created context");
        check(binding.Install(api, 42) && api.creates == 1, "C-26: install is idempotent");
        binding.Restore(api);
        check(api.associated == 7 && api.destroyed.size() == 1 && api.destroyed[0] == 100 &&
            !binding.Installed(), "C-26: restore re-associates the previous context");
        binding.Restore(api);
        check(api.destroyed.size() == 1, "C-26: restore runs once");
        FakeImeApi failing;
        failing.failCreate = true;
        ime_context::Binding<int, int> none;
        check(!none.Install(failing, 42) && failing.associations.empty(),
            "C-26: a failed create leaves the window association unchanged");
    }

    {
        int data[64][32]{};
        check(cdata_integrity::Checksum(data) == 1024000u, "native cdata offsets checksum");
        data[12][7] = 17;
        check(cdata_integrity::Checksum(data) == 1024068u, "cdata includes off-diagonal words");
        data[12][7] = -1;
        check(cdata_integrity::Checksum(data) == 1023996u, "cdata uses x86 modular arithmetic");
        for (unsigned int fps : {15u, 30u, 60u, 144u, 500u}) {
            effect_emission::Clock clock;
            unsigned int count = 0;
            for (unsigned int frame = 0; frame <= fps; ++frame) count += clock.Steps(frame * 1000u / fps);
            check(count == 31, "emission count is independent of render FPS");
        }
        effect_emission::Clock clock;
        check(clock.Steps(100) == 1 && clock.Steps(5100) == 3, "stalled emission has bounded catch-up");
        check(clock.Steps(6000, false) == 0 && clock.Steps(9000, false) == 0 && clock.Steps(9100) == 1,
            "hidden emission has no backlog");
        check(clock.Steps(9000) == 0, "clock correction does not create a particle burst");
        missing_asset::Cache misses;
        check(misses.ShouldTry("Env/a.dds", 10), "new asset path is tried");
        misses.Missing("Env/a.dds", 10);
        check(!misses.ShouldTry("env\\A.dds", 5009) && misses.ShouldTry("ENV/a.dds", 5010),
            "missing asset cache normalizes paths and expires for repair");
        PendingMoveTarget target;
        target.Set(1, 2, 3, 0); target.Set(4, 5, 6, 1);
        check(!target.Due(1100, 100) && target.Due(1101, 100) && target.x == 4 && target.stop == 1,
            "latest move intent waits for native send cadence");
        check(target.Due(1000, 0xfffffff0u), "move queue handles tick wrap");
        target.Clear(); check(!target.Due(9000, 0), "cleared route cannot move after a scene change");
        check(equipment_appearance::Color(10, 0x11) == 116 && equipment_appearance::Legend(4, 10, 0x11) == 9,
            "dyed +10 compact visual matches full-item color and gem");
        check(equipment_appearance::Color(9, 116) == 116 && equipment_appearance::Legend(4, 9, 116) == 4,
            "ordinary dyed refinement preserves native presentation");
        check(mount_health::Maximum(2362) == mount_health::MaximumByType[2] &&
            mount_health::Maximum(3981) == 0 && mount_health::Maximum(2359) == 0,
            "mount maximum follows item identity, not mesh aliases");
        STRUCT_ITEM previous{};
        previous.sIndex = 2381;
        previous.stEffect[0].sValue = 15000;
        previous.stEffect[1].cEffect = 120;
        previous.stEffect[1].cValue = 10;
        previous.stEffect[2].cEffect = 30;
        auto next = previous;
        next.stEffect[0].sValue = 13840;
        check(mount_health::IsLiveHPUpdate(previous, next),
            "living Unicorn HP crossing 50 percent does not require model recreation");
        for (int field = 0; field < 5; ++field) {
            auto changed = next;
            if (field == 0) changed.stEffect[0].sValue = 0;
            if (field == 1) changed.stEffect[1].cEffect++;
            if (field == 2) changed.stEffect[1].cValue--;
            if (field == 3) changed.stEffect[2].cEffect--;
            if (field == 4) changed.sIndex++;
            check(!mount_health::IsLiveHPUpdate(previous, changed),
                "death, growth, longevity, food and identity changes require full projection");
        }
        auto dead = previous;
        dead.stEffect[0].sValue = 0;
        check(!mount_health::IsLiveHPUpdate(dead, next), "revival uses full projection");
        check(skill_damage_preview::FormulaSkill(0, 16) == 2 && skill_damage_preview::FormulaSkill(0, 3) == 13 &&
            skill_damage_preview::FormulaSkill(3, 95) == 95, "TK formula mapping preserves Huntress books");
    }

    return failures;
}
