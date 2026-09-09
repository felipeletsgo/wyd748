// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593e5a | Name: FUN_00593e5a


void FUN_00593e5a(void)

{
  int iVar1;
  void *pvStack_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  puStack_c = &DAT_005aa0e0;
  puStack_10 = &LAB_00596b2c;
  pvStack_14 = ExceptionList;
  local_8 = 0;
  ExceptionList = &pvStack_14;
  iVar1 = FUN_00593d53();
  if (*(int *)(iVar1 + 0x60) != 0) {
    local_8 = 1;
    iVar1 = FUN_00593d53();
    (**(code **)(iVar1 + 0x60))();
  }
  local_8 = 0xffffffff;
  FUN_0059ad66();
  return;
}

