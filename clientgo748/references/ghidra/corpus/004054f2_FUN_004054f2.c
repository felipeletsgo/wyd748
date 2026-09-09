// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004054f2 | Name: FUN_004054f2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004054f2(int param_1)

{
  if (*(int *)(param_1 + 0x1ec) < *(int *)(param_1 + 0x1e8)) {
    *(undefined4 *)(param_1 + 0x1e8) = *(undefined4 *)(param_1 + 0x1ec);
  }
  if (*(int *)(param_1 + 0x1ec) < 1) {
    *(undefined4 *)(param_1 + 0x1ec) = 1;
  }
  if (*(int *)(param_1 + 0x1e4) == 1) {
    *(float *)(param_1 + 0x1f0) =
         ((*(float *)(param_1 + 0x54) - _DAT_005a3834) * (float)*(int *)(param_1 + 0x1e8)) /
         (float)*(int *)(param_1 + 0x1ec);
  }
  else if (*(int *)(param_1 + 0x1e4) == 2) {
    *(float *)(param_1 + 0x1f0) =
         ((float)*(int *)(param_1 + 0x1e8) * *(float *)(param_1 + 0x54)) /
         (float)*(int *)(param_1 + 0x1ec);
  }
  else {
    *(float *)(param_1 + 500) =
         ((float)*(int *)(param_1 + 0x1e8) * *(float *)(param_1 + 0x58)) /
         (float)*(int *)(param_1 + 0x1ec);
  }
  return;
}

