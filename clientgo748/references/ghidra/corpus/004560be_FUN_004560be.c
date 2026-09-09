// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004560be | Name: FUN_004560be


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004560be(int param_1,int param_2,int param_3)

{
  if (((((*(int *)(param_1 + 0x7c) == 0) || (*(int *)(*(int *)(param_1 + 0x7c) + 0x60) == 0)) ||
       (param_2 < 1)) ||
      ((param_3 < 1 ||
       (*(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4) <= param_2)))) ||
     (*(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8) <= param_3)) {
    if (*(int *)(DAT_005ccec0 + 0xa4) == 1) {
      *(undefined4 *)(DAT_005ccec0 + 0xa4) = 0;
    }
  }
  else if (((*(float *)(*(int *)(*(int *)(param_1 + 0x7c) + 0x60) + 0x2b8) < _DAT_005a4378) ||
           (_DAT_005a41ec < *(float *)(*(int *)(*(int *)(param_1 + 0x7c) + 0x60) + 700))) ||
          (_DAT_005a41ec < *(float *)(*(int *)(*(int *)(param_1 + 0x7c) + 0x60) + 0x2c0))) {
    if (*(int *)(DAT_005ccec0 + 0xa4) == 1) {
      *(undefined4 *)(DAT_005ccec0 + 0xa4) = 0;
    }
  }
  else {
    *(undefined4 *)(DAT_005ccec0 + 0xa4) = 1;
  }
  return;
}

