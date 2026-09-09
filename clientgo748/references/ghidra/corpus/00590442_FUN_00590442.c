// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590442 | Name: FUN_00590442


void FUN_00590442(undefined4 param_1,undefined4 param_2,int param_3,code *param_4)

{
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  puStack_c = &DAT_005a9fe0;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  local_8 = 0;
  ExceptionList = &local_14;
  while( true ) {
    param_3 = param_3 + -1;
    if (param_3 < 0) break;
    (*param_4)();
  }
  ExceptionList = local_14;
  return;
}

