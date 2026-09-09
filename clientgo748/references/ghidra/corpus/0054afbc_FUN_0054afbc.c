// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054afbc | Name: FUN_0054afbc


void __fastcall FUN_0054afbc(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a2b37;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a5d34;
  local_8 = 0;
  (**(code **)(*DAT_013b71e8 + 100))(param_1[0x5f]);
  local_8 = 0xffffffff;
  FUN_0053e26b();
  ExceptionList = local_10;
  return;
}

