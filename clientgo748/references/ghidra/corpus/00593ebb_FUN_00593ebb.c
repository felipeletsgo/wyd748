// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593ebb | Name: FUN_00593ebb


void FUN_00593ebb(void)

{
  void *pvStack_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  puStack_c = &DAT_005aa0f8;
  puStack_10 = &LAB_00596b2c;
  pvStack_14 = ExceptionList;
  ExceptionList = &pvStack_14;
  if (PTR_FUN_005c9d64 != (undefined *)0x0) {
    local_8 = 1;
    ExceptionList = &pvStack_14;
    (*(code *)PTR_FUN_005c9d64)();
  }
  local_8 = 0xffffffff;
  FUN_00593e5a();
  return;
}

