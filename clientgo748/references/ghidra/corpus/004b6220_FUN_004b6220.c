// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6220 | Name: FUN_004b6220


void __fastcall FUN_004b6220(undefined4 param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a05d9;
  local_10 = ExceptionList;
  local_8 = 0;
  ExceptionList = &local_10;
  FUN_004b6310(param_1);
  local_8 = 0xffffffff;
  FUN_004b6310();
  ExceptionList = local_10;
  return;
}

