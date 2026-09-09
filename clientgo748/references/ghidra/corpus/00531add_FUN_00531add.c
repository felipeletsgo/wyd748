// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00531add | Name: FUN_00531add


undefined4 __fastcall FUN_00531add(int param_1)

{
  void *pvVar1;
  int iVar2;
  int iVar3;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2564;
  local_10 = ExceptionList;
  if ((*(int *)(param_1 + 0x60) == 0) && (*(int *)(param_1 + 0x24) == 3)) {
    ExceptionList = &local_10;
    pvVar1 = operator_new(0x358);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_34 = 0;
    }
    else {
      local_34 = FUN_004bd420(param_1 + 0x160,param_1 + 0x180,*(undefined4 *)(param_1 + 0x5c),0,0,0,
                              0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x60) = local_34;
    if (*(int *)(param_1 + 0x60) == 0) {
      ExceptionList = local_10;
      return 1;
    }
    FUN_004be7b1();
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = 0x3fc00000;
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = 0x3fc00000;
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = 0x3fc00000;
    *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 0x50;
    iVar2 = (**(code **)(*DAT_0092e654 + 8))();
    iVar3 = _rand();
    *(int *)(*(int *)(param_1 + 0x60) + 0x2e8) = iVar2 + (iVar3 % 10) * -300;
  }
  else {
    ExceptionList = &local_10;
    if (*(int *)(param_1 + 0x24) == 5) {
      ExceptionList = &local_10;
      pvVar1 = operator_new(0x124);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_38 = 0;
      }
      else {
        local_38 = FUN_004cb900(0x5a,0,0x3f800000,0x3f800000,0x3f800000,0x3a03126f,0);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 400) = local_38;
      if (*(int *)(param_1 + 400) == 0) {
        ExceptionList = local_10;
        return 1;
      }
      *(undefined4 *)(*(int *)(param_1 + 400) + 0x6c) = 0;
      FUN_0054ac09(*(undefined4 *)(param_1 + 400));
    }
  }
  local_14 = 0;
  while( true ) {
    if (0 < local_14) {
      ExceptionList = local_10;
      return 1;
    }
    pvVar1 = operator_new(0x124);
    local_8 = 2;
    if (pvVar1 == (void *)0x0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_004cb900(10,0,0x3f000000,0x3f000000,0x3f000000,0x3a83126f,0);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x194 + local_14 * 4) = local_3c;
    *(undefined4 *)(*(int *)(param_1 + 0x194 + local_14 * 4) + 0x6c) = 1;
    if (*(int *)(param_1 + 0x194 + local_14 * 4) == 0) break;
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x194 + local_14 * 4));
    local_14 = local_14 + 1;
  }
  ExceptionList = local_10;
  return 1;
}

