// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053aa20 | Name: FUN_0053aa20


undefined4 * __thiscall FUN_0053aa20(undefined4 *param_1,undefined1 param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a25f9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  FUN_00430f10();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a58bc;
  *(undefined1 *)(param_1 + 0x58) = 0;
  *(undefined1 *)((int)param_1 + 0x161) = param_2;
  param_1[0x5a] = 0;
  param_1[0x5b] = 0;
  param_1[0x59] = 0;
  param_1[0x11] = 0;
  ExceptionList = local_10;
  return param_1;
}

