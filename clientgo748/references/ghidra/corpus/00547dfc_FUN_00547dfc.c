// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00547dfc | Name: FUN_00547dfc


void __fastcall FUN_00547dfc(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_005a2a9d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a5c08;
  local_8 = 1;
  FUN_005903c2(param_1 + 0x7dd,0x180,2,FUN_004c92e9);
  local_8 = local_8 & 0xffffff00;
  FUN_005903c2(param_1 + 0x5d,0x180,0x14,FUN_004c92e9);
  local_8 = 0xffffffff;
  FUN_0053e26b();
  ExceptionList = local_10;
  return;
}

