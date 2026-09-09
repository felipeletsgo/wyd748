// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d5995 | Name: FUN_004d5995


undefined4 __thiscall FUN_004d5995(int param_1,undefined4 param_2)

{
  void *pvVar1;
  undefined4 uVar2;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0abd;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x220) < 0) {
    ExceptionList = &local_10;
    if (*(int *)(param_1 + 0x60) == 0) {
      ExceptionList = &local_10;
      pvVar1 = operator_new(0x358);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_30 = 0;
      }
      else {
        local_30 = FUN_004bd420(param_1 + 0x160,param_1 + 0x180,*(undefined4 *)(param_1 + 0x5c),
                                param_2,0,0,0);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x60) = local_30;
      if (*(int *)(param_1 + 0x60) != 0) {
        FUN_004be7b1();
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x24) = *(undefined4 *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x28) = *(undefined4 *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2c) = *(undefined4 *)(param_1 + 0x38);
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 0x50;
      }
    }
    if ((*(int *)(param_1 + 0x19c) == 10) && (*(int *)(param_1 + 0x60) != 0)) {
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2dc) = 0;
      uVar2 = (**(code **)(*DAT_0092e654 + 8))();
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e8) = uVar2;
    }
  }
  else {
    ExceptionList = &local_10;
    if (*(int *)(param_1 + 0x60) == 0) {
      ExceptionList = &local_10;
      pvVar1 = operator_new(0x358);
      local_8 = 1;
      if (pvVar1 == (void *)0x0) {
        local_34 = 0;
      }
      else {
        local_34 = FUN_004bd420(param_1 + 0x160,param_1 + 0x180,*(undefined4 *)(param_1 + 0x5c),0,0,
                                0,0);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x60) = local_34;
      if (*(int *)(param_1 + 0x60) != 0) {
        FUN_004be7b1();
        *(int *)(*(int *)(param_1 + 0x60) + 0x2f4) = param_1;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e4) = 0x50;
        *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x30) = 0;
      }
    }
    if ((*(int *)(param_1 + 0x19c) == 10) && (*(int *)(param_1 + 0x60) != 0)) {
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2dc) = 0;
      uVar2 = (**(code **)(*DAT_0092e654 + 8))();
      *(undefined4 *)(*(int *)(param_1 + 0x60) + 0x2e8) = uVar2;
    }
    if (*(int *)(param_1 + 0x21c) == 0) {
      pvVar1 = operator_new(0x358);
      local_8 = 2;
      if (pvVar1 == (void *)0x0) {
        local_38 = 0;
      }
      else {
        local_38 = FUN_004bd420(param_1 + 0x1fc,param_1 + 0x180,*(undefined4 *)(param_1 + 0x220),
                                param_2,0,0,0);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x21c) = local_38;
      if (*(int *)(param_1 + 0x21c) != 0) {
        FUN_004be7b1();
        *(undefined4 *)(*(int *)(param_1 + 0x21c) + 0x2e4) = 0x50;
        if ((*(int *)(param_1 + 0x5c) == 0x14) && (*(short *)(param_1 + 0x1fc) == 7)) {
          FUN_004c0746(4,*(undefined4 *)(param_1 + 0x5c));
        }
        else if (*(int *)(param_1 + 0x5c) == 0x14) {
          FUN_004c0746(3,*(undefined4 *)(param_1 + 0x5c));
        }
        else {
          FUN_004c0746(2,*(undefined4 *)(param_1 + 0x5c));
        }
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

