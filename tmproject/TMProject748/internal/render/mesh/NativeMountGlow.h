#pragma once

namespace native_mount_glow {
// FUN_004c3eec, native 7.48 table at 0x005baa04. Grade zero is gated out
// by RenderMesh; the table intentionally repeats the grade 6/7 texture.
constexpr int TextureIndex(int grade)
{
    constexpr int offsets[] = {10, 1, 2, 3, 4, 6, 7, 7, 8, 9, 10, 11, 12};
    return grade >= 0 && grade < 13 ? 233 + offsets[grade] : 121;
}
}
