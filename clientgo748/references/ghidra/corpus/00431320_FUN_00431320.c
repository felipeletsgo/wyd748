// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431320 | Name: FUN_00431320


undefined4 * __thiscall FUN_00431320(undefined4 *param_1,undefined4 param_2)

{
  *param_1 = &PTR_FUN_005a4218;
  _memset(param_1 + 1,0,0x104);
  param_1[0x42] = param_2;
  param_1[0x43] = 0;
  param_1[0x44] = 0;
  param_1[0x45] = 0;
  param_1[0x46] = 0;
  param_1[0x47] = 0;
  param_1[0x48] = 0;
  return param_1;
}

