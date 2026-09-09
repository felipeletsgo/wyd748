// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004015dd | Name: FUN_004015dd


void __thiscall FUN_004015dd(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x28) = param_2;
  if ((((*(int *)(param_1 + 0x28) == 0) && (DAT_0067cf38 != 0)) &&
      (*(int *)(DAT_0067cf38 + 0x28) != 0)) &&
     (param_1 == *(int *)(*(int *)(DAT_0067cf38 + 0x28) + 0x130))) {
    *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x28) + 0x130) = 0;
  }
  return;
}

