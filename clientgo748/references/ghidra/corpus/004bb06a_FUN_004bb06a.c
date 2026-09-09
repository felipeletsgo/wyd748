// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bb06a | Name: FUN_004bb06a


void __fastcall FUN_004bb06a(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_005a46a8;
  if (param_1[0xc6] != 0) {
    (**(code **)(*(int *)param_1[0xc6] + 8))(param_1[0xc6]);
    param_1[0xc6] = 0;
  }
  if (param_1[199] != 0) {
    (**(code **)(*(int *)param_1[199] + 8))(param_1[199]);
    param_1[199] = 0;
  }
  return;
}

