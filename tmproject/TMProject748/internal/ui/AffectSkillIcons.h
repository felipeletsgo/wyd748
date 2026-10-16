#pragma once

// Native 7.48 skill atlas mapping, shared by owner, target, party and C.C.
// See persistent-buff-visuals.md for the Critical Armor visual discriminator.
static constexpr int g_AffectSkillType[122] = {
    -1, 1, 41, 2, 124, 6, 89, 4, 9, 44, 51, 43, 84, 11, 13, 45,
    71, 5, 46, 76, 40, 77, 37, 54, 3, 53, 89, 74, 75, 102, 122, 86,
    100, 96, 123, 125, 15, 79, 87, 85, 96, 26, 121, 129, 99, 114, 130, 147, 140, 165
};
// Visual 24 is the server's Samaritan assignment, not native Holy Armor (49).
static_assert(g_AffectSkillType[24] == 3, "Samaritan must use its own skill icon");
static_assert(g_AffectSkillType[36] == 15, "Critical Armor must use its own skill icon");
