// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c372e | Name: FUN_004c372e


undefined4 * __thiscall FUN_004c372e(undefined4 *param_1,undefined4 param_2)

{
  *param_1 = &PTR_FUN_005a46dc;
  param_1[1] = 0;
  param_1[4] = 0;
  param_1[0x30] = param_2;
  param_1[0x34] = 0xffffffff;
  *(undefined2 *)((int)param_1 + 0xc6) = 0;
  *(undefined2 *)(param_1 + 0x31) = 0;
  param_1[0x32] = 0;
  param_1[3] = 0;
  param_1[0x33] = 0;
  return param_1;
}

