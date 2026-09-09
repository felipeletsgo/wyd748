// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004052e3 | Name: FUN_004052e3


void __fastcall FUN_004052e3(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059eaf0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3838;
  local_8 = 0;
  FUN_00405376();
  if ((*(int *)(DAT_0067cf38 + 0x28) != 0) && (-1 < (int)param_1[0xd0])) {
    FUN_0040c1b0(*(int *)(DAT_0067cf38 + 0x28) + 0x3c,param_1 + 0x7e,param_1[0xd0]);
  }
  local_8 = 0xffffffff;
  FUN_0040151a();
  ExceptionList = local_10;
  return;
}

