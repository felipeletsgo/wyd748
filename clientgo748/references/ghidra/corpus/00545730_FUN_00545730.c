// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00545730 | Name: FUN_00545730


undefined4 * __fastcall FUN_00545730(undefined4 *param_1)

{
  FUN_0053e1d2();
  *param_1 = &PTR_FUN_005a5afc;
  param_1[9] = 2;
  param_1[0x71] = 0xffffffff;
  param_1[0x72] = 0xffffffff;
  param_1[0xd] = 0;
  param_1[0x5f] = 0;
  param_1[0x5e] = 0;
  param_1[0x73] = 1;
  return param_1;
}

