#include "../internal/application/AssetLoadValidation.h"
#include "../internal/application/IndexedPartyMessage.h"
#include "../internal/application/OwnedSoundBuffers.h"
#include "../internal/application/FieldChatSubmitPolicy.h"
#include "../internal/application/WrappedTexture.h"
#include "../internal/application/ShadeGrid.h"
#include "../internal/application/SkinFrameValidation.h"
#include "../internal/application/ScopedReentry.h"
#include "../internal/application/ActorLifetime.h"
#include "../internal/core/TreeAttachment.h"
#include "../internal/ui/RenderControlList.h"
#include "../internal/application/BoundedText.h"
#include "../internal/application/TextureRegionCopy.h"
#include "../internal/render/mesh/SkinMeshLoad.h"
#include <climits>
#include <filesystem>
#include <string>
#include <vector>

int RunAuditSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) { ++failures; std::fprintf(stderr, "FAIL audit safety: %s\n", message); }
    };
    {
        skin_mesh_load::Header header{0, 1, 0, 12, 4, 40, 3, 3};
        skin_mesh_load::Payload payload;
        constexpr std::uint64_t fileBytes = 32 + 40 * 68 + 36 + 6;
        check(skin_mesh_load::Plan(header, fileBytes, 40, payload) &&
            payload.names == 160 && payload.matrices == 2560 && payload.vertices == 36 && payload.indices == 6,
            "mesh palette boundary allocates exactly the validated payload");
        for (std::uint64_t length = 0; length < fileBytes; ++length)
            check(!skin_mesh_load::Plan(header, length, 40, payload) && !payload.vertices,
                "every truncated mesh extent is rejected before allocation");
        for (const auto palette : {41U, UINT_MAX})
        {
            auto invalid = header; invalid.palette = palette;
            check(!skin_mesh_load::Plan(invalid, UINT64_MAX, 40, payload), "palette cannot overrun the fixed bone-offset array");
        }
        for (const auto stride : {0U, 8U, 13U, UINT_MAX - 3})
        {
            auto invalid = header; invalid.stride = stride;
            check(!skin_mesh_load::Plan(invalid, UINT64_MAX, 40, payload), "invalid or overflowing vertex stride rejected");
        }
        for (const auto influence : {0U, 5U, UINT_MAX})
        {
            auto invalid = header; invalid.influences = influence;
            check(!skin_mesh_load::Plan(invalid, fileBytes, 40, payload), "invalid weighted shader influence rejected");
        }
        auto rigid = header; rigid.palette = 0; rigid.influences = 0;
        check(skin_mesh_load::Plan(rigid, 74, 40, payload) && !payload.names && !payload.matrices,
            "rigid mesh does not require palette allocations");
        auto overflow = header; overflow.vertices = UINT_MAX;
        check(!skin_mesh_load::Plan(overflow, UINT64_MAX, 40, payload), "vertex byte product cannot wrap the Direct3D UINT");
        overflow = header; overflow.indices = UINT_MAX;
        check(!skin_mesh_load::Plan(overflow, UINT64_MAX, 40, payload), "index byte product cannot wrap the Direct3D UINT");
        for (const auto count : {0U, 1U, 2U, 4U})
        {
            auto invalid = header; invalid.indices = count;
            check(!skin_mesh_load::Plan(invalid, fileBytes, 40, payload), "empty or incomplete triangle rejected");
        }
        float positions[9]{};
        check(skin_mesh_load::Positions(positions, rigid), "finite positions accepted");
        positions[8] = (std::numeric_limits<float>::quiet_NaN)();
        check(!skin_mesh_load::Positions(positions, rigid), "nonfinite last-vertex position rejected");
        std::uint16_t indices[]{0, 1, 2};
        check(skin_mesh_load::Indices(indices, rigid), "last valid vertex index accepted");
        indices[2] = 3;
        check(!skin_mesh_load::Indices(indices, rigid), "index beyond vertex buffer rejected");
        check(!skin_mesh_load::ReadExact(nullptr, nullptr, 0) &&
            !skin_mesh_load::ReadExact(stdin, nullptr, 1), "missing file or allocated destination rejected");

        struct Buffer
        {
            int* released;
            unsigned char data[16]{};
            int locks = 0, unlocks = 0;
            bool lockFails = false, nullData = false, unlockFails = false;
            long Lock(unsigned, unsigned, void** out, unsigned) { ++locks; *out = nullData ? nullptr : data; return lockFails ? -1L : 0L; }
            long Unlock() { ++unlocks; return unlockFails ? -1L : 0L; }
            void Release() { ++*released; }
        };
        for (int failure = 0; failure < 8; ++failure)
        {
            FILE* file = nullptr;
            if (tmpfile_s(&file) != 0) { check(false, "mesh upload fixture creation"); continue; }
            unsigned char source[16]{};
            std::fwrite(source, 1, failure == 6 ? 15 : 16, file);
            std::rewind(file);
            int released = 0;
            Buffer buffer{&released};
            buffer.lockFails = failure == 3;
            buffer.nullData = failure == 4;
            buffer.unlockFails = failure == 5;
            Buffer* outputBuffer = nullptr;
            const auto create = [&](Buffer** out) {
                if (failure != 1) *out = &buffer;
                return failure == 2 ? -1L : 0L;
            };
            int validations = 0;
            const bool success = skin_mesh_load::ReadBuffer(file, 16, outputBuffer, create,
                [&](void*) { ++validations; return failure != 7; });
            check(success == (failure == 0) && (outputBuffer != nullptr) == success,
                "creation, null pointer, lock, short read, validation and unlock failures never publish a buffer");
            check(buffer.locks == (failure == 1 || failure == 2 ? 0 : 1) &&
                buffer.unlocks == (failure >= 4 || failure == 0 ? 1 : 0),
                "only successful locks are balanced, including a null lock destination");
            check(released == (failure == 0 || failure == 1 ? 0 : 1), "failed temporary COM resource released exactly once");
            check(validations == (failure == 0 || failure == 5 || failure == 7 ? 1 : 0),
                "validation never reads failed locks or truncated payloads");
            if (outputBuffer) outputBuffer->Release();
            check(released == (failure == 1 ? 0 : 1), "published COM reference remains caller-owned");
            std::fclose(file);
        }
        const auto meshDirectory = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "client748" / "mesh";
        std::size_t meshCount = 0;
        check(std::filesystem::is_directory(meshDirectory), "runtime mesh corpus exists for the compatibility gate");
        if (std::filesystem::is_directory(meshDirectory))
            for (const auto& entry : std::filesystem::directory_iterator(meshDirectory))
            {
                if (entry.path().extension() != ".msh") continue;
                FILE* raw = nullptr;
                fopen_s(&raw, entry.path().string().c_str(), "rb");
                std::unique_ptr<FILE, decltype(&fclose)> input(raw, &fclose);
                skin_mesh_load::Header meshHeader{};
                skin_mesh_load::Payload meshPayload;
                const bool planned = skin_mesh_load::ReadExact(raw, &meshHeader, sizeof(meshHeader)) &&
                    skin_mesh_load::Plan(meshHeader, entry.file_size(), 40, meshPayload);
                check(planned, "shipped mesh header passes the production extent and palette checks");
                if (!planned) { std::fprintf(stderr, "Rejected mesh: %s\n", entry.path().string().c_str()); continue; }
                check(std::string("mesh\\").size() + entry.path().filename().string().size() < 64,
                    "shipped mesh path fits the cache key");
                std::vector<unsigned char> paletteData(meshPayload.matrices + meshPayload.names);
                std::vector<float> vertexData(meshPayload.vertices / sizeof(float));
                std::vector<std::uint16_t> indexData(meshPayload.indices / sizeof(std::uint16_t));
                const bool read = skin_mesh_load::ReadExact(raw, paletteData.data(), paletteData.size()) &&
                    skin_mesh_load::ReadExact(raw, vertexData.data(), meshPayload.vertices) &&
                    skin_mesh_load::ReadExact(raw, indexData.data(), meshPayload.indices);
                check(read && skin_mesh_load::Positions(vertexData.data(), meshHeader) &&
                    skin_mesh_load::Indices(indexData.data(), meshHeader),
                    "shipped mesh payload passes production read, position and index checks");
                ++meshCount;
            }
        check(meshCount > 0, "mesh corpus gate must not pass with an empty selection");
        std::printf("Skin mesh corpus: %zu files checked\n", meshCount);
    }
    char output[256]{};
    const char literal[] = "Skill_points_%n";
    bounded_text::Prepare(output, literal, sizeof(literal), false);
    check(!std::strcmp(output, "Skill points %n") && !std::strcmp(literal, "Skill_points_%n"), "text literals remain unchanged and percent signs are data");
    char tight[] = "1234567";
    bounded_text::Prepare(output, tight, sizeof(tight), true);
    check(!std::strcmp(output, "1,234,567") && !std::strcmp(tight, "1234567"), "comma expansion never mutates a tight caller buffer");
    for (const char* zero : {"0", "000", "0 1", "  "})
    {
        bounded_text::Prepare(output, zero, 256, true);
        check(!std::strcmp(output, zero), "zero and space-only text preserve the existing presentation");
    }
    bounded_text::Prepare(output, " 1234 567", 10, true);
    check(!std::strcmp(output, "  1,234,567"), "legacy numeric spaces become a prefix");
    bounded_text::Prepare(output, "_", 2, true);
    check(output[0] == 0, "single underscore clears the control");
    bounded_text::Prepare(output, nullptr, 0, true);
    check(output[0] == 0, "null text clears the control");
    struct TextGuard { char before = 'L'; char value[256]{}; char after = 'R'; } guarded;
    char unterminated[256];
    std::memset(unterminated, '9', sizeof(unterminated));
    bounded_text::Prepare(guarded.value, unterminated, sizeof(unterminated), true);
    check(std::strlen(guarded.value) <= 255 && guarded.before == 'L' && guarded.after == 'R', "long numeric text is bounded without integer conversion");
    check(guarded.value[std::strlen(guarded.value) - 1] != ',', "truncated grouping does not leave a dangling comma");
    char guilds[4][12]{};
    char guildName[12];
    std::memset(guildName, '%', sizeof(guildName));
    for (const int slot : {INT_MIN, -1, 4, INT_MAX})
        check(!bounded_text::CopyIndexedField(guilds, slot, guildName), "invalid guild slot cannot write the name table");
    for (const auto& guild : guilds) check(guild[0] == 0, "rejected guild slot leaves every name unchanged");
    for (int slot = 0; slot < 4; ++slot)
        check(bounded_text::CopyIndexedField(guilds, slot, guildName) && std::strlen(guilds[slot]) == 11, "nonterminated guild field is bounded and terminated");
    char leader[16]{};
    bounded_text::Copy(leader, "%n%s%08x", 9);
    check(!std::strcmp(leader, "%n%s%08x"), "party and summon names are never format templates");
    char whisper[128]{};
    bounded_text::Format(whisper, "[%s]> %s", std::string(15, 'n').c_str(), std::string(95, 'm').c_str());
    check(std::strlen(whisper) == 114, "maximum wire whisper plus sender fits the chat line");
    bounded_text::Format(whisper, "[%s] : %s> %s", std::string(15, 'n').c_str(), std::string(15, 'r').c_str(), std::string(95, 'm').c_str());
    check(std::strlen(whisper) == 127, "overlong combined whisper truncates instead of invoking CRT abort");

    for (const int bpp : {2, 4})
        for (const int resolution : {256, 512})
        {
            const int scale = resolution == 256 ? 2 : 1;
            const int srcWidth = 512, srcHeight = 512;
            const int srcPitch = srcWidth * bpp + 12;
            const int destPitch = resolution * bpp + 20;
            std::vector<unsigned char> src(static_cast<std::size_t>(srcPitch) * srcHeight, 0);
            for (std::size_t i = 0; i < src.size(); ++i) src[i] = static_cast<unsigned char>(i % 251);
            const auto srcBefore = src;
            std::vector<unsigned char> dest(static_cast<std::size_t>(destPitch) * resolution + 2, 0xCD);
            const auto plan = texture_region_copy::Build(resolution, resolution, srcWidth, srcHeight,
                resolution - 8, resolution - 6, 3, 2, INT_MAX, INT_MAX, scale);
            check(plan.valid && plan.width == 8 && plan.height == 6, "copy rectangle clips to actual destination dimensions");
            check(texture_region_copy::PitchesCover(bpp, srcWidth, srcPitch, resolution, destPitch), "padded pitches accepted");
            texture_region_copy::Copy(plan, bpp, src.data(), srcPitch, dest.data() + 1, destPitch);
            for (int row = 0; row < resolution; ++row)
                for (int column = 0; column < destPitch; ++column)
                {
                    unsigned char expected = 0xCD;
                    if (row >= plan.destY && row < plan.destY + plan.height &&
                        column >= plan.destX * bpp && column < (plan.destX + plan.width) * bpp)
                    {
                        const int sourceRow = scale * (row - plan.destY + plan.srcY);
                        const int pixelByte = column - plan.destX * bpp;
                        const int sourceColumn = (scale * (pixelByte / bpp) + plan.srcX) * bpp + pixelByte % bpp;
                        expected = src[static_cast<std::size_t>(sourceRow) * srcPitch + sourceColumn];
                    }
                    check(dest[1 + static_cast<std::size_t>(row) * destPitch + column] == expected, "copy preserves every pixel and row-padding sentinel");
                }
            check(dest.front() == 0xCD && dest.back() == 0xCD && src == srcBefore, "surface guard bytes and source remain unchanged");
            check(!texture_region_copy::PitchesCover(bpp, srcWidth, srcWidth * bpp - 1, resolution, destPitch), "short pitch rejected");
        }
    check(!texture_region_copy::DestinationCompatible(21, 128, 128, 21, 256, 256), "disk minimap cannot alias a dynamic destination");
    check(!texture_region_copy::DestinationCompatible(21, 256, 256, 23, 256, 256), "format change requires destination replacement");
    check(texture_region_copy::DestinationCompatible(23, 256, 256, 23, 256, 256), "matching dynamic texture is reusable");
    check(texture_region_copy::BytesPerPixel(0) == 0 && texture_region_copy::BytesPerPixel(0x31545844) == 0, "unknown and compressed formats rejected");
    for (const int coord : {-1, 128, INT_MAX})
        check(!texture_region_copy::Build(128, 128, 512, 512, coord, 0, 0, 0, 16, 16, 1).valid, "destination coordinate outside actual surface rejected");
    check(!texture_region_copy::Build(256, 256, 512, 512, 0, 0, 0, INT_MAX, 16, 16, 2).valid, "scaled source coordinate cannot overflow");
    const auto edge = texture_region_copy::Build(256, 256, 512, 512, 0, 0, 511, 255, 20, 20, 2);
    check(edge.valid && edge.width == 1 && edge.height == 1, "half-resolution sampling clips to source edges");
    struct Rect { int Pitch = 0; void* pBits = nullptr; };
    struct Surface
    {
        Rect rect;
        bool failLock = false, failUnlock = false;
        int locks = 0, unlocks = 0;
        long LockRect(Rect* out, const void*, unsigned long) { ++locks; *out = rect; return failLock ? -1L : 0L; }
        long UnlockRect() { ++unlocks; return failUnlock ? -1L : 0L; }
    };
    unsigned char pixels[16]{};
    const auto unitPlan = texture_region_copy::Build(1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1);
    for (int failure = 0; failure < 7; ++failure)
    {
        Surface src{{4, pixels}}, dest{{4, pixels + 8}};
        if (failure == 1) src.failLock = true;
        if (failure == 2) src.rect.pBits = nullptr;
        if (failure == 3) dest.failLock = true;
        if (failure == 4) dest.rect.pBits = nullptr;
        if (failure == 5) dest.rect.Pitch = 1;
        if (failure == 6) dest.failUnlock = true;
        check(texture_region_copy::CopyLocked<Rect>(src, dest, unitPlan, 4, 1, 1, 0) == (failure == 0), "copy reports injected lock/pointer/pitch/unlock failures");
        check(src.unlocks == (failure == 1 ? 0 : 1), "every successful source lock is balanced");
        check(dest.unlocks == (failure == 1 || failure == 2 || failure == 3 ? 0 : 1), "every successful destination lock is balanced");
    }
    struct Geometry { Geometry* m_pNextGeom = nullptr; };
    struct List { Geometry* pHeadGeom = nullptr; Geometry* pTailGeom = nullptr; };
    List list;
    Geometry head, middle, tail, replacement;
    check(render_control_list::Append(list, &head), "append empty list");
    check(render_control_list::Append(list, &middle), "append middle");
    check(render_control_list::Append(list, &tail), "append tail");
    check(!render_control_list::Append(list, &tail), "duplicate cannot create a cycle");
    check(render_control_list::Remove(list, &tail) && list.pTailGeom == &middle && !middle.m_pNextGeom, "tail unlinked before destruction");
    check(render_control_list::Append(list, &replacement) && middle.m_pNextGeom == &replacement, "append after tail deletion");
    check(render_control_list::Remove(list, &head) && list.pHeadGeom == &middle, "remove head");
    check(render_control_list::Remove(list, &middle) && list.pHeadGeom == &replacement, "remove middle");
    check(render_control_list::Remove(list, &replacement) && !list.pHeadGeom && !list.pTailGeom, "single removal clears both ends");
    for (int iteration = 0; iteration < 10000; ++iteration)
    {
        auto* icon = new Geometry;
        check(render_control_list::Append(list, icon) && render_control_list::Remove(list, icon), "repeated icon lifecycle");
        delete icon;
        check(!list.pHeadGeom && !list.pTailGeom, "no freed geometry remains linked");
    }
    struct Node { Node* m_pTop = nullptr; };
    Node root, child{&root}, grandchild{&child}, other;
    check(tree_attachment::CanAdopt(&root, &other), "independent child accepted");
    check(!tree_attachment::CanAdopt(&grandchild, &root), "ancestor adoption rejected");
    check(!tree_attachment::CanAdopt(&root, &root), "self adoption rejected");
    check(!tree_attachment::CanAdopt(&root, static_cast<Node*>(nullptr)), "null adoption rejected");
    root.m_pTop = &grandchild;
    check(!tree_attachment::CanAdopt(&child, &other), "existing parent cycle terminates safely");
    root.m_pTop = nullptr;
    struct OwnedNode
    {
        OwnedNode* m_pTop = nullptr;
        OwnedNode* m_pDown = nullptr;
        OwnedNode* m_pNextLink = nullptr;
        int* m_pOwner = nullptr;
    };
    int owner = 1, unrelatedOwner = 2;
    OwnedNode subtree, outsideSubtree, branch, leaf, sibling;
    subtree.m_pDown = &branch;
    subtree.m_pNextLink = &outsideSubtree;
    branch.m_pTop = &subtree;
    branch.m_pDown = &leaf;
    branch.m_pNextLink = &sibling;
    leaf.m_pTop = &branch;
    sibling.m_pTop = &subtree;
    branch.m_pOwner = leaf.m_pOwner = outsideSubtree.m_pOwner = &owner;
    sibling.m_pOwner = &unrelatedOwner;
    tree_attachment::ClearOwner(&subtree, &owner);
    check(!branch.m_pOwner && !leaf.m_pOwner, "actor removal clears owners through nested effect containers");
    check(sibling.m_pOwner == &unrelatedOwner && outsideSubtree.m_pOwner == &owner, "owner cleanup preserves other actors and stays in the subtree");
    leaf.m_pOwner = &owner;
    tree_attachment::ClearOwner(&subtree, &owner);
    check(!leaf.m_pOwner, "final deletion detaches an effect attached after initial retirement");
    tree_attachment::ClearOwner(&subtree, static_cast<int*>(nullptr));
    check(!sibling.m_pOwner && outsideSubtree.m_pOwner == &owner, "clear-all remains bounded to descendants");
    std::vector<OwnedNode> deep(10000);
    for (std::size_t i = 0; i < deep.size(); ++i)
    {
        deep[i].m_pOwner = &owner;
        if (i) { deep[i].m_pTop = &deep[i - 1]; deep[i - 1].m_pDown = &deep[i]; }
    }
    tree_attachment::ClearOwner(&deep[0], &owner);
    for (std::size_t i = 1; i < deep.size(); ++i)
        check(!deep[i].m_pOwner, "deep owner cleanup does not recurse on the call stack");
    check(deep[0].m_pOwner == &owner, "descendant traversal excludes its root");
    struct Actor {} localActor, remoteActor;
    struct Scene { int m_eSceneType; Actor* m_pMyHuman; } field{1, &localActor}, selection{2, &localActor};
    check(!actor_lifetime::MayDelayDelete(&localActor, &field, 1), "local field actor survives death retirement for resurrection and HUD");
    check(actor_lifetime::MayDelayDelete(&remoteActor, &field, 1), "remote actor retirement remains enabled");
    check(actor_lifetime::MayDelayDelete(&localActor, &selection, 1), "selection actor retirement remains enabled");
    bool active = false;
    {
        ScopedReentry first(active);
        ScopedReentry nested(active);
        check(static_cast<bool>(first) && !static_cast<bool>(nested) && active, "recursive commerce dispatch blocked");
    }
    check(!active, "commerce dispatch restored on return");
    const std::uint32_t bones[] = {UINT32_MAX, 0, 0, 1, 1, 2, 0, 2};
    check(skin_frames::Valid(bones, sizeof(bones), 4, 8, "mesh\\ch01", 10), "bounded duplicate bone IDs retain legacy alias semantics");
    check(!skin_frames::Valid(bones, sizeof(bones) - 1, 4, 8, "mesh\\ch01", 10), "truncated bone pair rejected");
    check(!skin_frames::Valid(bones, sizeof(bones), 4, 9, "mesh\\ch01", 10), "look part overflow rejected");
    const std::uint32_t outside[] = {0, 80};
    const std::uint32_t forward[] = {2, 1, 1, 2};
    check(!skin_frames::Valid(outside, sizeof(outside), 1, 1, "mesh", 5), "frame lookup overwrite rejected");
    check(!skin_frames::Valid(forward, sizeof(forward), 2, 1, "mesh", 5), "unreachable parent rejected");
    check(!skin_frames::Valid(bones, sizeof(bones), 4, 8, "unterminated", 4), "unterminated mesh prefix rejected");
    std::string parsedName;
    int parsedNumbers[3]{17, 18, 19};
    check(asset_load::ParseNamedIntegers("WALK 2 13 0", parsedName, parsedNumbers, 128), "animation sound row parsed");
    for (const auto& row : {std::string("WALK 1 2"), std::string("WALK 1 2 3 extra"),
        std::string("WALK 99999999999999999 2 3"), std::string(128, 'x') + " 1 2 3"})
        check(!asset_load::ParseNamedIntegers(row.c_str(), parsedName, parsedNumbers, 128), "malformed animation sound row rejected");
    int minimap[5]{};
    check(asset_load::ParseMinimap("2100 2100 1 0 0 NOOB", minimap, parsedName), "shipped minimap syntax accepted");
    check(!asset_load::ParseMinimap(("1 2 3 4 5 " + std::string(32, 'x')).c_str(), minimap, parsedName), "minimap label cannot overwrite adjacent entry");
    check(!asset_load::ParseMinimap("1 2 invalid 4 5 name", minimap, parsedName), "partial minimap row rejected");
    int id = -1, count = -1, parts = -1;
    std::string path;
    check(asset_load::ParseList("220 sound.wav 4", id, path, count, parts, true, false, 512, 256), "sound record");
    for (const char* line : {"0 sound.wav 1", "512 sound.wav 1", "x sound.wav 1", "2 sound.wav", "2 sound.wav 0", "2 sound.wav -1", "2 sound.wav 513", "2 sound.wav 999999999999", "2 sound.wav 1 extra"})
        check(!asset_load::ParseList(line, id, path, count, parts, true, false, 512, 256), "invalid sound record");
    check(!asset_load::ParseList("-1 mesh", id, path, count, parts, false, false, 3048, 64), "negative mesh index");
    check(!asset_load::ParseList("3048 mesh", id, path, count, parts, false, false, 3048, 64), "mesh index capacity");
    check(!asset_load::ParseList(("1 " + std::string(64, 'a')).c_str(), id, path, count, parts, false, false, 3048, 64), "long mesh name");
    check(asset_load::ParseList("0 186 8 mesh\\ch01", id, path, count, parts, false, true, 100, 116), "bone record");
    for (const char* line : {"100 1 1 mesh", "0 187 8 mesh", "0 -1 8 mesh", "0 1 9 mesh", "0 1 8", "0 1 8 mesh extra"})
        check(!asset_load::ParseList(line, id, path, count, parts, false, true, 100, 116), "invalid bone record");
    std::size_t bytes = 0;
    check(asset_load::AnimationBytes(2, 3, 3, bytes) && bytes == 384, "animation extent");
    check(!asset_load::AnimationBytes(2, 4, 3, bytes), "inconsistent frame stride");
    check(!asset_load::AnimationBytes(0, 3, 3, bytes), "zero ticks");
    check(!asset_load::AnimationBytes(2, 0, 0, bytes), "zero frames");
    check(!asset_load::AnimationBytes(UINT_MAX, UINT_MAX, 0, bytes), "animation overflow");
    check(!asset_load::AnimationBytes(1u << 30, 1u << 30, 0, bytes), "animation byte multiplication cannot wrap to zero");
    check(!asset_load::AnimationMapping(0, 0), "negative mapped type");
    check(asset_load::AnimationMapping(0, 100), "valid mapping");
    check(!asset_load::AnimationMapping(0, 156), "motion capacity");
    check(!asset_load::AnimationMapping(3, 100), "alternate type capacity");
    char source[260]{};
    check(asset_load::BoneSourcePath("mesh\\ed.dat", source, sizeof(source)) && !std::strcmp(source, "mesh\\ed.bon"), "BGM extension replacement");
    check(!asset_load::BoneSourcePath("", source, sizeof(source)), "empty BGM slot");
    check(!asset_load::BoneSourcePath("sound.wav", source, sizeof(source)), "unestablished BGM format");
    check(asset_load::BoneSourcePath("%s.dat", source, sizeof(source)) && !std::strcmp(source, "%s.bon"), "BGM path is data");

    static char table[2000][128]{};
    char payload[96]{1, 0}, text[128]{};
    for (const int code : {-32768, -1001, -1000, 999, 1000, 32767})
    {
        const auto value = static_cast<std::int16_t>(code);
        std::memcpy(payload + 2, &value, sizeof(value));
        const bool valid = code >= -1000 && code <= 999;
        check(party_message::Decode(payload, sizeof(payload), table, text) == valid, "indexed party boundaries");
        if (valid) check(text == std::to_string(code), "numeric fallback");
    }
    std::int16_t value = 0;
    std::memcpy(payload + 2, &value, sizeof(value));
    std::memset(table[1000], 'x', 128);
    check(party_message::Decode(payload, sizeof(payload), table, text) && std::strlen(text) == 127, "unterminated table entry");
    check(table[1000][127] == 'x' && table[1000][126] == 'x', "global table unchanged");
    check(!party_message::Decode(payload, 3, table, text), "short indexed payload");
    payload[0] = 0;
    check(!party_message::Decode(payload, sizeof(payload), table, text), "unsupported party envelope");

    struct Buffer { int& releases; void Release() { ++releases; } };
    int releases = 0;
    Buffer first{releases}, second{releases};
    { OwnedSoundBuffers<Buffer> buffers(4); buffers.data()[0] = &first; buffers.data()[2] = &second; }
    check(releases == 2, "partial sound construction releases exactly acquired references");
    { OwnedSoundBuffers<Buffer> buffers(1); buffers.data()[0] = &first; buffers.transfer(); }
    check(releases == 2, "successful sound ownership transfer is not released twice");

    FILE* file = nullptr;
    if (tmpfile_s(&file) == 0)
    {
        std::fprintf(file, "%s\n1 mesh\n", std::string(600, 'a').c_str());
        std::rewind(file);
        char line[512]{};
        check(asset_load::ReadLine(file, line) == asset_load::LineResult::Invalid, "overlong physical line rejected");
        check(asset_load::ReadLine(file, line) == asset_load::LineResult::Record && !std::strcmp(line, "1 mesh\n"), "parser progresses after overlong record");
        check(asset_load::ReadLine(file, line) == asset_load::LineResult::End, "EOF terminates parser");
        std::fclose(file);
    }
    else check(false, "temporary parser fixture could not be created");

    for (const int length : {0, 1, 15, 16, 31, 32, 127})
        check(chat_submit::FitsWhisperName(std::string(length, 'x').c_str()) == (length > 0 && length < 16), "whisper name boundaries before packet copy");
    check(!chat_submit::FitsWhisperName(nullptr), "null whisper name");
    for (const int capacity : {64, 512, 3000})
        for (const int index : {-1, 0, capacity - 1, capacity, INT_MAX})
            check(wrapped_texture::ValidIndex(index, capacity) == (index >= 0 && index < capacity), "texture index bounds");
    int textureLength = 0;
    for (const long length : {-1L, 0L, 1L, 4L, 21L, 22L, 88L, 128L, 129L, 64L * 1024 * 1024, LONG_MAX})
    {
        check(wrapped_texture::PayloadLength(length, false, textureLength) == (length >= 22 && length <= 64L * 1024 * 1024), "TGA extent boundaries");
        check(wrapped_texture::PayloadLength(length, true, textureLength) == (length >= 129 && length <= 64L * 1024 * 1024), "DDS extent boundaries");
    }
    for (const bool dds : {false, true})
    {
        FILE* textureFile = nullptr;
        if (tmpfile_s(&textureFile) != 0) { check(false, "texture fixture creation"); continue; }
        char encoded[129]{};
        encoded[85] = '2';
        const int fileLength = dds ? 129 : 22;
        std::fwrite(encoded, 1, fileLength, textureFile);
        std::fflush(textureFile);
        std::rewind(textureFile);
        const char footer[18] = "TRUEVISION-XFILE.";
        std::unique_ptr<char[]> decoded(wrapped_texture::Read(_dup(_fileno(textureFile)), dds, footer, textureLength));
        check(decoded != nullptr, "valid wrapped texture decode");
        if (decoded)
        {
            check(textureLength == (dds ? 128 : 18), "decoded payload extent");
            check(dds ? !std::memcmp(decoded.get(), "DDS", 3) && !std::memcmp(decoded.get() + 84, "DXT1", 4) : !std::memcmp(decoded.get() + textureLength, footer, 18), "established texture wrapper transformation");
        }
        // A descriptor can report a valid file length but still provide a short read.
        std::fseek(textureFile, fileLength - 1, SEEK_SET);
        std::unique_ptr<char[]> truncated(wrapped_texture::Read(_dup(_fileno(textureFile)), dds, footer, textureLength));
        check(truncated == nullptr, "short texture read rejected");
        std::fclose(textureFile);
    }
    check(shade_grid::VertexCount(70) == 5041 && shade_grid::IndexCount(70) == 29400, "shade allocation counts elements, not bytes");
    check(shade_grid::BoundedSize(INT_MAX) == 255 && shade_grid::VertexCount(INT_MAX) == 65536, "shade indices fit unsigned short");
    check(shade_grid::BoundedSize(INT_MIN) == 2, "shade lower grid bound");
    return failures;
}
