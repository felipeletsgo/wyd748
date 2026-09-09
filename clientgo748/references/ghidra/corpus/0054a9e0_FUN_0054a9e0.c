// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054a9e0 | Name: FUN_0054a9e0


undefined4 * __thiscall FUN_0054a9e0(undefined4 *param_1,undefined4 param_2)

{
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = 0;
  param_1[4] = 0;
  *(undefined1 *)(param_1 + 5) = 0;
  param_1[6] = 0xffffffff;
  param_1[7] = 0;
  param_1[8] = param_2;
  *param_1 = &PTR_FUN_005a5cfc;
  return param_1;
}

