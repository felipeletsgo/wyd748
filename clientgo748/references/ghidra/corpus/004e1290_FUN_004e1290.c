// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e1290 | Name: FUN_004e1290


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004e1290(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  float10 fVar4;
  undefined4 in_stack_0000001c;
  undefined4 in_stack_00000020;
  undefined4 uVar5;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0e66;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4dc0;
  param_1[0x29] = 0;
  param_1[0x2a] = 0;
  param_1[0x2b] = 0;
  param_1[0x2d] = 0;
  param_1[0x28] = 0x2be;
  uVar1 = FUN_004310a0(&stack0x00000004);
  FUN_004310a0(uVar1);
  FUN_004310a0(&stack0x00000010);
  param_1[0x27] = in_stack_0000001c;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0xa0), iVar2 != 0)) {
    uVar5 = 0;
    uVar1 = 0;
    FUN_00429a6d(0xa0);
    FUN_0042ad2b(uVar1,uVar5);
  }
  FUN_00493db0(local_20,&stack0x00000004);
  fVar4 = (float10)FUN_004b3aa0();
  local_14 = (float)fVar4;
  iVar2 = __ftol();
  param_1[0x2c] = iVar2 * 300;
  if (param_1[0x2c] == 0) {
    param_1[0x2c] = 1;
  }
  if (5000 < (uint)param_1[0x2c]) {
    param_1[0x2c] = 5000;
  }
  param_1[0x2e] = 0;
  if (param_1[0x27] == 0) {
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 1;
    if (pvVar3 == (void *)0x0) {
      local_60 = 0;
    }
    else {
      local_60 = FUN_004df250(3,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2e] = local_60;
    if (param_1[0x2e] != 0) {
      (**(code **)(*(int *)param_1[0x2e] + 0x44))(0x5533);
    }
  }
  else if (param_1[0x27] == 1) {
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 2;
    if (pvVar3 == (void *)0x0) {
      local_64 = 0;
    }
    else {
      local_64 = FUN_004df250(5,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2e] = local_64;
    if (param_1[0x2e] != 0) {
      (**(code **)(*(int *)param_1[0x2e] + 0x44))(&DAT_00770000);
    }
  }
  else if (param_1[0x27] == 2) {
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 3;
    if (pvVar3 == (void *)0x0) {
      local_68 = 0;
    }
    else {
      local_68 = FUN_004df250(3,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2e] = local_68;
    if (param_1[0x2e] != 0) {
      (**(code **)(*(int *)param_1[0x2e] + 0x44))(0x3355);
    }
  }
  else if (param_1[0x27] == 3) {
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 4;
    if (pvVar3 == (void *)0x0) {
      local_6c = 0;
    }
    else {
      local_6c = FUN_004df250(3,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2e] = local_6c;
    if (param_1[0x2e] != 0) {
      (**(code **)(*(int *)param_1[0x2e] + 0x44))(0x553355);
    }
  }
  if (param_1[0x2e] != 0) {
    *(undefined4 *)(param_1[0x2e] + 0x6c) = 1;
  }
  param_1[7] = in_stack_00000020;
  if (param_1[0x2e] != 0) {
    FUN_0054ac09(param_1[0x2e]);
  }
  param_1[0x2f] = 0;
  param_1[0x30] = 0;
  param_1[0x31] = 0;
  if (param_1[0x27] == 2) {
    param_1[0x28] = 0xc;
  }
  if ((param_1[0x27] == 2) || (param_1[0x27] == 3)) {
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 5;
    if (pvVar3 == (void *)0x0) {
      local_70 = 0;
    }
    else {
      local_70 = FUN_004c8f70(0x38,0,0x3f800000,0x3f800000,0x3f800000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x2f] = local_70;
    if (param_1[0x2f] != 0) {
      *(undefined4 *)(param_1[0x2f] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2f] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x24);
      *(float *)(param_1[0x2f] + 0x78) = *(float *)(param_1[0x2f] + 0x78) - _DAT_005a34a0;
      if (param_1[0x27] == 2) {
        (**(code **)(*(int *)param_1[0x2f] + 0x3c))(0xff55eeff);
      }
      else {
        (**(code **)(*(int *)param_1[0x2f] + 0x3c))(0xffff55ee);
      }
      FUN_0054ac09(param_1[0x2f]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 6;
    if (pvVar3 == (void *)0x0) {
      local_74 = 0;
    }
    else {
      local_74 = FUN_004c8f70(2,0,0x3f000000,0x3f000000,0x3f000000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x30] = local_74;
    if (param_1[0x30] != 0) {
      *(undefined4 *)(param_1[0x30] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x30] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x24);
      *(float *)(param_1[0x30] + 0x78) = *(float *)(param_1[0x30] + 0x78) - _DAT_005a34a0;
      FUN_0054ac09(param_1[0x30]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 7;
    if (pvVar3 == (void *)0x0) {
      local_78 = 0;
    }
    else {
      local_78 = FUN_004c8f70(0x3c,0,0x40200000,0x40000000,0x40200000,0,1,0x50);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x31] = local_78;
    if (param_1[0x31] != 0) {
      *(undefined4 *)(param_1[0x31] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x31] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x24);
      *(float *)(param_1[0x31] + 0x78) = *(float *)(param_1[0x31] + 0x78) - _DAT_005a34a0;
      if (param_1[0x27] == 2) {
        (**(code **)(*(int *)param_1[0x31] + 0x3c))(0x33003344);
      }
      else {
        (**(code **)(*(int *)param_1[0x31] + 0x3c))(0x33440033);
      }
      FUN_0054ac09(param_1[0x31]);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

