// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042a8a4 | Name: FUN_0042a8a4


void __fastcall FUN_0042a8a4(undefined4 *param_1)

{
  uint local_8;
  
  *param_1 = &PTR_FUN_005a4184;
  for (local_8 = 0; local_8 < (uint)param_1[4]; local_8 = local_8 + 1) {
    if (*(int *)(param_1[1] + local_8 * 4) != 0) {
      (**(code **)(**(int **)(param_1[1] + local_8 * 4) + 8))
                (*(undefined4 *)(param_1[1] + local_8 * 4));
      *(undefined4 *)(param_1[1] + local_8 * 4) = 0;
    }
  }
  if (param_1[1] != 0) {
    FUN_0058ea3a(param_1[1]);
    param_1[1] = 0;
  }
  if (param_1[3] != 0) {
    if (param_1[3] != 0) {
      FUN_0042c0b0(1);
    }
    param_1[3] = 0;
  }
  return;
}

