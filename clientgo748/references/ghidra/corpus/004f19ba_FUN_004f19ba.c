// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f19ba | Name: FUN_004f19ba


void __fastcall FUN_004f19ba(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a1679;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a5460;
  local_8 = 0;
  if (param_1[0xb8] != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1[0xb8],param_1,0);
  }
  local_8 = 0xffffffff;
  FUN_004f3b81();
  ExceptionList = local_10;
  return;
}

