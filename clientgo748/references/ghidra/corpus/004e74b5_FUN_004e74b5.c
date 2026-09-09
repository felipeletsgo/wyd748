// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e74b5 | Name: FUN_004e74b5


void __fastcall FUN_004e74b5(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a11c1;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a5040;
  local_8 = 0;
  if (param_1[0x21] != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1[0x21]);
  }
  if (param_1[0x22] != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1[0x22]);
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

