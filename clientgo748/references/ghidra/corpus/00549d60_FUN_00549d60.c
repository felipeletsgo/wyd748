// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00549d60 | Name: FUN_00549d60


undefined4 * __thiscall FUN_00549d60(undefined4 *param_1,undefined4 param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2ab9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a5c80;
  FUN_00549dd6(param_2);
  ExceptionList = local_10;
  return param_1;
}

