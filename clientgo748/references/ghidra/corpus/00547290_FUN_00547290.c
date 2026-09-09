// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00547290 | Name: FUN_00547290


undefined4 * __thiscall FUN_00547290(undefined4 *param_1,undefined4 param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a29f9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a5b84;
  param_1[9] = param_2;
  param_1[0x18] = 0;
  param_1[0x67] = 0x3f800000;
  param_1[0x68] = 0x3f800000;
  param_1[0x17] = 0x3c;
  _memset(param_1 + 0x58,0,0x20);
  _memset(param_1 + 0x60,0,0x10);
  *(short *)(param_1 + 0x58) = (short)param_2 + -0x1e7;
  ExceptionList = local_10;
  return param_1;
}

