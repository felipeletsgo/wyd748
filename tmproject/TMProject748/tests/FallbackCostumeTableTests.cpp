#include "../internal/render/mesh/FallbackCostumeTable.h"

#include <cstdio>
#include <cstring>
#include <initializer_list>

// Full data equivalence with the original TMSkinMesh::SetCostume chain is
// checked by .agents/research/fallback-costume-table.mjs --check. These tests
// pin the selection rule that replaced the chain and a sample of known rows.
int RunFallbackCostumeTableTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL fallback costume table: %s\n", message);
        }
    };
    constexpr int count = static_cast<int>(sizeof(fallback_costume::kCostumes) / sizeof(fallback_costume::kCostumes[0]));

    for (int i = 0; i < count; ++i) {
        const auto& costume = fallback_costume::kCostumes[i];
        for (int j = i + 1; j < count; ++j)
            check(costume.type != fallback_costume::kCostumes[j].type, "costume types are unique");
        check(fallback_costume::Find(costume.type) == &costume, "lookup returns the row");
        // Destination buffers in TMSkinMesh are 64 (texture) and 32 (mesh) bytes.
        check(costume.texture && std::strlen(costume.texture) < 64, "texture fits its buffer");
        check(!costume.firstPartTexture || std::strlen(costume.firstPartTexture) < 64, "part-1 texture fits");
        for (const char* mesh : costume.meshes)
            check(mesh && std::strlen(mesh) < 32, "mesh fits its buffer");
    }
    for (int type : {-1, 0, 1, 7, 161, 100000})
        check(fallback_costume::Find(type) == nullptr, "types outside the table delegate");

    // Parts 1..6 select their mesh and advance the cycle; other parts only
    // receive the texture. Part 1 uses the override when one exists.
    for (int i = 0; i < count; ++i) {
        const auto& costume = fallback_costume::kCostumes[i];
        for (int part = -3; part <= 9; ++part) {
            char texture[64] = "unchanged";
            char mesh[32] = "unchanged";
            int next = part;
            fallback_costume::Apply(costume, next, texture, mesh);
            const char* expectedTexture = part == 1 && costume.firstPartTexture
                ? costume.firstPartTexture : costume.texture;
            check(!std::strcmp(texture, expectedTexture), "texture selection");
            if (part >= 1 && part <= 6) {
                check(!std::strcmp(mesh, costume.meshes[part - 1]), "part mesh selection");
                check(next == part % 6 + 1, "part cycle advances and wraps");
            } else {
                check(!std::strcmp(mesh, "unchanged") && next == part, "other parts keep mesh and cycle");
            }
        }
    }

    // Known rows from the original chain.
    const auto* santa = fallback_costume::Find(8);
    check(santa && !std::strcmp(santa->texture, "mesh\\RedSanta.wyt") &&
        !std::strcmp(santa->meshes[0], "mesh\\ch020196.msh") &&
        !std::strcmp(santa->meshes[5], "mesh\\ch020696.msh") && !santa->firstPartTexture,
        "type 8 keeps the red Santa texture and its six meshes");
    const auto* split = fallback_costume::Find(123);
    check(split && !std::strcmp(split->texture, "mesh\\ch0103134.wys") &&
        split->firstPartTexture && !std::strcmp(split->firstPartTexture, "mesh\\ch0102134.wys") &&
        !std::strcmp(split->meshes[5], "mesh\\ch0106134.msh"),
        "type 123 keeps its distinct part-1 texture");
    return failures;
}
