// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053cd40 | Name: FUN_0053cd40


undefined4 * __thiscall FUN_0053cd40(undefined4 *param_1,undefined4 param_2)

{
  FUN_0053e1d2();
  *param_1 = &PTR_FUN_005a5944;
  param_1[9] = param_2;
  param_1[0x18] = 0;
  param_1[0x17] = 0x3d;
  _memset(param_1 + 0x58,0,0x20);
  _memset(param_1 + 0x60,0,0x10);
  return param_1;
}

