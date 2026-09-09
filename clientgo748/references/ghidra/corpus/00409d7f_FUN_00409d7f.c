// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409d7f | Name: FUN_00409d7f


void __fastcall FUN_00409d7f(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059ed58;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a3e00;
  local_8 = 0;
  if (((param_1[0x79] == 2) && (*(int *)(DAT_0067cf38 + 0x28) != 0)) && (-1 < (int)param_1[0xcd])) {
    FUN_0040c1b0(*(int *)(DAT_0067cf38 + 0x28) + 0x3c,param_1 + 0x7b,param_1[0xcd]);
  }
  local_8 = 0xffffffff;
  FUN_0040151a();
  ExceptionList = local_10;
  return;
}

