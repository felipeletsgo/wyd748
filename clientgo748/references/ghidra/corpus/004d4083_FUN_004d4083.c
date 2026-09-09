// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d4083 | Name: FUN_004d4083


void __fastcall FUN_004d4083(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a09f2;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a49e4;
  local_8 = 0;
  if (param_1[0x20] != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1[0x20]);
  }
  local_8 = 0xffffffff;
  FUN_004c8d6c();
  ExceptionList = local_10;
  return;
}

