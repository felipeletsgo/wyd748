// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f0c50 | Name: FUN_004f0c50


undefined4 * __fastcall FUN_004f0c50(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1609;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004f3a9b();
  local_8 = 0;
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a536c;
  param_1[0xba] = 0;
  param_1[0xbd] = 0x3ff33333;
  param_1[0xbe] = 0x3f800000;
  *(undefined1 *)(param_1 + 0xbf) = 0;
  *(undefined1 *)((int)param_1 + 0x2fd) = 0;
  ExceptionList = local_10;
  return param_1;
}

