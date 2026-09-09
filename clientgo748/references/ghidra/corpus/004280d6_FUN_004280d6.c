// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004280d6 | Name: FUN_004280d6


void __fastcall FUN_004280d6(int *param_1)

{
  param_1[0xa8e2] = 0;
  param_1[0xa8e3] = 0;
  if (param_1[0xa8f9] != 0) {
    (**(code **)(*param_1 + 0x14))();
    (**(code **)(*param_1 + 0x18))();
    (**(code **)(*(int *)param_1[0xa8f9] + 8))(param_1[0xa8f9]);
    (**(code **)(*(int *)param_1[0xa8f8] + 8))(param_1[0xa8f8]);
    param_1[0xa8f9] = 0;
    param_1[0xa8f8] = 0;
  }
  (**(code **)(*param_1 + 0x1c))();
  return;
}

