// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ef840 | Name: FUN_004ef840


undefined4 * __fastcall FUN_004ef840(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a15e9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a530c;
  FUN_004ef8e0();
  param_1[0x30] = 0x41700000;
  ExceptionList = local_10;
  return param_1;
}

