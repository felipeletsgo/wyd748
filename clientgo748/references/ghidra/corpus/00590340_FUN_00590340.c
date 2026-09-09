// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590340 | Name: FUN_00590340


void FUN_00590340(undefined4 param_1,undefined4 param_2,int param_3,code *param_4)

{
  int local_20;
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  puStack_c = &DAT_005a9fc0;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  local_8 = 0;
  ExceptionList = &local_14;
  for (local_20 = 0; local_20 < param_3; local_20 = local_20 + 1) {
    (*param_4)();
  }
  local_8 = 0xffffffff;
  FUN_005903aa();
  ExceptionList = local_14;
  return;
}

