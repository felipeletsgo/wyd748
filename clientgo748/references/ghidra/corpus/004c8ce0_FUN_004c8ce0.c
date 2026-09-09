// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c8ce0 | Name: FUN_004c8ce0


undefined4 * __fastcall FUN_004c8ce0(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a07e9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_00430f10();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4748;
  param_1[0x1b] = 0;
  param_1[10] = 1;
  param_1[9] = 1;
  param_1[0x1c] = 0x3f800000;
  ExceptionList = local_10;
  return param_1;
}

