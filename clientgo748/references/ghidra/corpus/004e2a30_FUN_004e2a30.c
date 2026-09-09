// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e2a30 | Name: FUN_004e2a30


undefined4 * __thiscall FUN_004e2a30(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 in_stack_00000010;
  undefined4 in_stack_00000014;
  undefined4 in_stack_00000018;
  undefined4 in_stack_0000001c;
  undefined4 uVar5;
  undefined4 local_3c;
  undefined4 local_38;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0f0f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4e38;
  param_1[0x26] = in_stack_00000010;
  FUN_004310a0(&stack0x00000004);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 0x960;
  param_1[0x22] = 0;
  param_1[7] = in_stack_00000014;
  param_1[0x23] = in_stack_00000018;
  param_1[0x24] = in_stack_0000001c;
  if ((((param_1[0x26] == 2) || (param_1[0x26] == 3)) || (param_1[0x26] == 5)) ||
     (param_1[0x26] == 6)) {
    param_1[0x21] = 800;
    pvVar2 = operator_new(0xc0);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_38 = 0;
    }
    else {
      local_38 = FUN_004df250(2,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x27] = local_38;
    if (param_1[0x27] != 0) {
      (**(code **)(*(int *)param_1[0x27] + 0x44))(param_1[0x24]);
      *(undefined4 *)(param_1[0x27] + 0x6c) = 1;
      puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
      (**(code **)(*(int *)param_1[0x27] + 0x3c))(*puVar3,puVar3[1]);
      *(int *)(param_1[0x27] + 0xa0) = param_1[0x21] + 1000;
      FUN_0054ac09(param_1[0x27]);
    }
  }
  else if (param_1[0x26] == 7) {
    param_1[0x21] = 1000;
  }
  else {
    pvVar2 = operator_new(0xc0);
    local_8._0_1_ = 2;
    if (pvVar2 == (void *)0x0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_004df250(4,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x27] = local_3c;
    if (param_1[0x27] != 0) {
      (**(code **)(*(int *)param_1[0x27] + 0x44))(param_1[0x24]);
      *(undefined4 *)(param_1[0x27] + 0x6c) = 1;
      puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
      (**(code **)(*(int *)param_1[0x27] + 0x3c))(*puVar3,puVar3[1]);
      *(int *)(param_1[0x27] + 0xa0) = param_1[0x21] + 1000;
      FUN_0054ac09(param_1[0x27]);
    }
    if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x9b), iVar4 != 0)) {
      uVar5 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x9b);
      FUN_0042ad2b(uVar1,uVar5);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

