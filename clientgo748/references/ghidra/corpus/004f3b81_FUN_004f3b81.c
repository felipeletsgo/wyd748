// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f3b81 | Name: FUN_004f3b81


void __fastcall FUN_004f3b81(undefined4 *param_1)

{
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_005a16a9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a54f8;
  local_8 = 0;
  if ((*(int *)(DAT_0067cf38 + 0x24) == 30000) &&
     (param_1 == *(undefined4 **)(DAT_0067cf38 + 0x26e98))) {
    *(undefined4 *)(DAT_0067cf38 + 0x26e98) = 0;
  }
  if (param_1[0x6b] != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1[0x6b]);
  }
  param_1[0x6b] = 0;
  if (param_1[0x70] != 0) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1[0x70]);
  }
  param_1[0x70] = 0;
  if (param_1[0x8f] != 0) {
    if ((undefined4 *)param_1[0x8f] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x8f])(1);
    }
    param_1[0x8f] = 0;
  }
  for (local_14 = 0; local_14 < 0x1c; local_14 = local_14 + 1) {
    if (param_1[local_14 + 0x73] != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1[local_14 + 0x73]);
      param_1[local_14 + 0x73] = 0;
    }
  }
  DAT_0092e7c8 = DAT_0092e7c8 + -1;
  local_8 = 0xffffffff;
  FUN_0053e26b();
  ExceptionList = local_10;
  return;
}

