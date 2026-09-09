// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004288a8 | Name: FUN_004288a8


void __fastcall FUN_004288a8(int param_1)

{
  if (*(int *)(param_1 + 4) != 0) {
    FUN_004292b0(3);
  }
  if (DAT_005b3688 != '\0') {
    DeleteFileA(&DAT_005b3688);
  }
  if (DAT_005b3788 != '\0') {
    DeleteFileA(&DAT_005b3788);
  }
  return;
}

