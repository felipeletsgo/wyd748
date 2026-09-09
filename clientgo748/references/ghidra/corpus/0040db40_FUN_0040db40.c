// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040db40 | Name: FUN_0040db40


bool FUN_0040db40(int param_1)

{
  return *(int *)(&DAT_0092ff50 + param_1 * 0x60) == 1;
}

