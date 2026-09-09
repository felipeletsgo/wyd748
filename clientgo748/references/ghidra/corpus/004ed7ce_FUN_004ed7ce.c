// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ed7ce | Name: FUN_004ed7ce


void __fastcall FUN_004ed7ce(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a14cd;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a51b0;
  local_8 = 0;
  if (param_1[0x25] != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1[0x25]);
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

