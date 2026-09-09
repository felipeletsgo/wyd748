// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040151a | Name: FUN_0040151a


void __fastcall FUN_0040151a(undefined4 *param_1)

{
  int iVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059e89c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a34ac;
  local_8 = 0;
  iVar1 = *(int *)(DAT_0067cf38 + 0x28);
  if ((iVar1 != 0) && (param_1 == *(undefined4 **)(iVar1 + 0x130))) {
    *(undefined4 *)(iVar1 + 0x130) = 0;
  }
  if ((iVar1 != 0) && (-1 < (int)param_1[0x6b])) {
    FUN_0040c1b0(iVar1 + 0x3c,param_1 + 0x19,param_1[0x6b]);
  }
  local_8 = 0xffffffff;
  FUN_00401152();
  ExceptionList = local_10;
  return;
}

