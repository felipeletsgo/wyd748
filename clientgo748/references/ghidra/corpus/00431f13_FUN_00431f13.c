// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431f13 | Name: FUN_00431f13


void __fastcall FUN_00431f13(undefined4 *param_1)

{
  int iVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059f0f7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a421c;
  local_8 = 0;
  *(undefined4 *)(DAT_005ccec0 + 0x28) = 1;
  if (DAT_005ccf98 != 0) {
    iVar1 = FUN_00429a6d(0x65);
    if (iVar1 != 0) {
      FUN_00429a6d(0x65);
      iVar1 = FUN_0042afd0();
      if (iVar1 != 0) {
        FUN_00429a6d(0x65);
        FUN_0042aef6();
      }
    }
  }
  local_8 = 0xffffffff;
  FUN_00494c00();
  ExceptionList = local_10;
  return;
}

