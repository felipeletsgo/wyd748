// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a8e29 | Name: FUN_004a8e29


void __fastcall FUN_004a8e29(undefined4 *param_1)

{
  int iVar1;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a02fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4544;
  local_8 = 0;
  for (local_14 = 0; local_14 < 0x32; local_14 = local_14 + 1) {
    if (param_1[local_14 + 0x9bc3] != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1[local_14 + 0x9bc3]);
      param_1[local_14 + 0x9bc3] = 0;
    }
  }
  if (DAT_005ccf98 != 0) {
    iVar1 = FUN_00429a6d(0x66);
    if (iVar1 != 0) {
      FUN_00429a6d(0x66);
      iVar1 = FUN_0042afd0();
      if (iVar1 != 0) {
        FUN_00429a6d(0x66);
        FUN_0042aef6();
      }
    }
  }
  local_8 = 0xffffffff;
  FUN_00494c00();
  ExceptionList = local_10;
  return;
}

