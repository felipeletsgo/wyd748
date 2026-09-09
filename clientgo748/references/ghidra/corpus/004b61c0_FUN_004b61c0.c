// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b61c0 | Name: FUN_004b61c0


undefined4 __fastcall FUN_004b61c0(undefined4 param_1)

{
  undefined1 local_18 [4];
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a05b9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004b6270(local_14);
  local_8 = 0;
  FUN_004b6270(local_18);
  ExceptionList = local_10;
  return param_1;
}

