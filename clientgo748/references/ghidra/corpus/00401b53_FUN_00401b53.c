// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401b53 | Name: FUN_00401b53


void __fastcall FUN_00401b53(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059e8c2;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3538;
  local_8 = 0;
  if ((*(int *)(DAT_0067cf38 + 0x28) != 0) && (-1 < (int)param_1[0x6b])) {
    FUN_0040c1b0(*(int *)(DAT_0067cf38 + 0x28) + 0x3c,param_1 + 0x19,param_1[0x6b]);
  }
  local_8 = 0xffffffff;
  FUN_00401152();
  ExceptionList = local_10;
  return;
}

