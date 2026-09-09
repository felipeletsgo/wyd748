// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00408b8f | Name: FUN_00408b8f


void __fastcall FUN_00408b8f(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059ed08;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3cf0;
  param_1[0x79] = &PTR_FUN_005a3cec;
  local_8 = 0;
  FUN_004091ab();
  local_8 = 0xffffffff;
  FUN_0040151a();
  ExceptionList = local_10;
  return;
}

