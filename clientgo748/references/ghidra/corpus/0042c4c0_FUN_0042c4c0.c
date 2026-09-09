// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042c4c0 | Name: FUN_0042c4c0


void __fastcall FUN_0042c4c0(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059f07c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4190;
  local_8 = 0;
  FUN_0042cab5();
  DAT_005ccf9c = 0;
  local_8 = 0xffffffff;
  FUN_00425a3f();
  ExceptionList = local_10;
  return;
}

