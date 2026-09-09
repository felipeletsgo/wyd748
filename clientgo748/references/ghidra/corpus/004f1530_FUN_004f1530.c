// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f1530 | Name: FUN_004f1530


undefined4 * __thiscall FUN_004f1530(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_0053e1d2();
  *param_1 = &PTR_FUN_005a53e8;
  param_1[0x59] = 0;
  param_1[0x58] = param_3;
  param_1[0x5a] = param_2;
  param_1[0x5b] = param_2;
  return param_1;
}

