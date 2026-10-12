#pragma once

namespace native_special_skill
{
// WYD.exe 7.48 FUN_004431e4 projects LearnedSkill bits 24..31 (Sephira and
// other special skills 96..103) into the skill-panel grid 571 as skill items
// 5096..5103. Modern UI mode 2, the only mode the native client runs, lays the
// eight slots out as four columns by two rows.
inline constexpr int SlotCount = 8;
inline constexpr int FirstBit = 24;
inline constexpr int FirstItem = 5096;
inline constexpr int Columns = 4;

struct Cell { int x; int y; };

constexpr unsigned int Bit(int slot) { return 1u << (FirstBit + slot); }
constexpr short ItemIndex(int slot) { return static_cast<short>(FirstItem + slot); }
constexpr Cell SlotCell(int slot) { return {slot % Columns, slot / Columns}; }
}
