// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052fc70 | Name: FUN_0052fc70


undefined4 * __fastcall FUN_0052fc70(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2499;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a5660;
  ExceptionList = local_10;
  return param_1;
}

