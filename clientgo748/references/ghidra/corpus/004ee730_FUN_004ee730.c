// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ee730 | Name: FUN_004ee730


undefined4 * __thiscall FUN_004ee730(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 in_stack_00000010;
  undefined4 uVar3;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1599;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a5268;
  param_1[0x22] = 1000;
  param_1[0x23] = 0;
  param_1[0x24] = 0;
  FUN_004310a0(&stack0x00000004);
  param_1[0x25] = in_stack_00000010;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 0;
  if (param_1[0x25] == 2) {
    FUN_004ee8cb(0x55ff);
  }
  else if (param_1[0x25] == 3) {
    FUN_004ee8cb(&DAT_00ff5500);
  }
  else {
    FUN_004ee8cb(0xffffffff);
  }
  if (DAT_005ccf98 != 0) {
    iVar2 = FUN_00429a6d(0x9f);
    if (iVar2 != 0) {
      uVar3 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x9f);
      FUN_0042ad2b(uVar1,uVar3);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

