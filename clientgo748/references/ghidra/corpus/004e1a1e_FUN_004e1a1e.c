// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e1a1e | Name: FUN_004e1a1e


void __fastcall FUN_004e1a1e(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a0e79;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4dc0;
  local_8 = 0;
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x2f]);
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x30]);
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x31]);
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x2e]);
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

