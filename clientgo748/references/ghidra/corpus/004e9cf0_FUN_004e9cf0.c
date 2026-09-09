// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e9cf0 | Name: FUN_004e9cf0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004e9cf0(undefined4 *param_1,float param_2,float param_3,float param_4,float param_5,
            float param_6,float param_7,undefined4 param_8,undefined4 param_9)

{
  int iVar1;
  undefined4 uVar2;
  void *pvVar3;
  float10 fVar4;
  undefined4 uVar5;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a132d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a50f8;
  param_1[0x28] = 0;
  param_1[0x29] = 0;
  param_1[0x2f] = 0;
  iVar1 = __ftol();
  if (((iVar1 == 0) && (iVar1 = __ftol(), iVar1 == 0)) && (iVar1 = __ftol(), iVar1 == 0)) {
    param_2 = param_5 + _DAT_005a40f0;
    param_4 = param_7 - _DAT_005a40f0;
    param_3 = param_6 + _DAT_005a4104;
  }
  uVar2 = FUN_004310a0(&param_2);
  FUN_004310a0(uVar2);
  FUN_004310a0(&param_5);
  param_1[0x27] = param_8;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar2;
  param_1[7] = param_9;
  param_1[0x2c] = 0;
  param_1[0x2d] = 0;
  param_1[0x2e] = 0;
  param_1[0x2b] = 0;
  FUN_00493db0(local_20,&param_2);
  fVar4 = (float10)FUN_004b3aa0();
  local_14 = (float)fVar4;
  if (param_1[0x27] == 0) {
    iVar1 = __ftol();
    param_1[0x2a] = iVar1 * 100;
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 1;
    if (pvVar3 == (void *)0x0) {
      local_88 = 0;
    }
    else {
      local_88 = FUN_004df250(3,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2b] = local_88;
    if (param_1[0x2b] != 0) {
      (**(code **)(*(int *)param_1[0x2b] + 0x44))(0x33881100);
      *(undefined4 *)(param_1[0x2b] + 0x6c) = 1;
      FUN_0054ac09(param_1[0x2b]);
    }
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0xa0), iVar1 != 0)) {
      uVar5 = 0;
      uVar2 = 0;
      FUN_00429a6d(0xa0);
      FUN_0042ad2b(uVar2,uVar5);
    }
  }
  else if (param_1[0x27] == 1) {
    DAT_005bb408 = (DAT_005bb408 + 1) % 5;
    iVar1 = __ftol();
    param_1[0x2a] = iVar1 * DAT_005bb408 * 0x1e;
  }
  else if (param_1[0x27] == 2) {
    iVar1 = __ftol();
    param_1[0x2a] = iVar1 * 100;
  }
  else if (param_1[0x27] == 3) {
    iVar1 = __ftol();
    param_1[0x2a] = iVar1 * 0x32;
  }
  else if (param_1[0x27] == 4) {
    iVar1 = __ftol();
    param_1[0x2a] = iVar1 * 100;
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 2;
    if (pvVar3 == (void *)0x0) {
      local_8c = 0;
    }
    else {
      local_8c = FUN_004df250(1,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2b] = local_8c;
    if (param_1[0x2b] != 0) {
      (**(code **)(*(int *)param_1[0x2b] + 0x44))(0xff5555aa);
      *(undefined4 *)(param_1[0x2b] + 0x6c) = 1;
      FUN_0054ac09(param_1[0x2b]);
    }
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0xa0), iVar1 != 0)) {
      uVar5 = 0;
      uVar2 = 0;
      FUN_00429a6d(0xa0);
      FUN_0042ad2b(uVar2,uVar5);
    }
  }
  else if (param_1[0x27] == 5) {
    iVar1 = __ftol();
    param_1[0x2a] = iVar1 * 100;
  }
  else if (param_1[0x27] == 6) {
    iVar1 = __ftol();
    param_1[0x2a] = iVar1 * 100;
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 3;
    if (pvVar3 == (void *)0x0) {
      local_90 = 0;
    }
    else {
      local_90 = FUN_004df250(1,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2b] = local_90;
    if (param_1[0x2b] != 0) {
      (**(code **)(*(int *)param_1[0x2b] + 0x44))(0xff5555aa);
      *(undefined4 *)(param_1[0x2b] + 0x6c) = 1;
      FUN_0054ac09(param_1[0x2b]);
    }
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0xa0), iVar1 != 0)) {
      uVar5 = 0;
      uVar2 = 0;
      FUN_00429a6d(0xa0);
      FUN_0042ad2b(uVar2,uVar5);
    }
  }
  if (param_1[0x2a] == 0) {
    param_1[0x2a] = 1;
  }
  if (5000 < (uint)param_1[0x2a]) {
    param_1[0x2a] = 5000;
  }
  if (param_1[0x27] == 0) {
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 4;
    if (pvVar3 == (void *)0x0) {
      local_94 = 0;
    }
    else {
      local_94 = FUN_004c8f70(0x38,0,0x3f800000,0x3f800000,0x3f800000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x2c] = local_94;
    if (param_1[0x2c] != 0) {
      *(undefined4 *)(param_1[0x2c] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2c] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x24);
      *(float *)(param_1[0x2c] + 0x78) = *(float *)(param_1[0x2c] + 0x78) - _DAT_005a34a0;
      (**(code **)(*(int *)param_1[0x2c] + 0x3c))(0xffff7711);
      FUN_0054ac09(param_1[0x2c]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 5;
    if (pvVar3 == (void *)0x0) {
      local_98 = 0;
    }
    else {
      local_98 = FUN_004c8f70(2,0,0x3f000000,0x3f000000,0x3f000000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x2d] = local_98;
    if (param_1[0x2d] != 0) {
      *(undefined4 *)(param_1[0x2d] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2d] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x24);
      *(float *)(param_1[0x2d] + 0x78) = *(float *)(param_1[0x2d] + 0x78) - _DAT_005a34a0;
      FUN_0054ac09(param_1[0x2d]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 6;
    if (pvVar3 == (void *)0x0) {
      local_9c = 0;
    }
    else {
      local_9c = FUN_004c8f70(0x3c,0,0x40200000,0x40000000,0x40200000,0,1,0x50);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2e] = local_9c;
    if (param_1[0x2e] != 0) {
      *(undefined4 *)(param_1[0x2e] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2e] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x24);
      *(float *)(param_1[0x2e] + 0x78) = *(float *)(param_1[0x2e] + 0x78) - _DAT_005a34a0;
      (**(code **)(*(int *)param_1[0x2e] + 0x3c))(0x33881100);
      FUN_0054ac09(param_1[0x2e]);
    }
  }
  else if (param_1[0x27] == 4) {
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 7;
    if (pvVar3 == (void *)0x0) {
      local_a0 = 0;
    }
    else {
      local_a0 = FUN_004c8f70(0x38,0,0x3f800000,0x3f800000,0x3f800000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x2c] = local_a0;
    if (param_1[0x2c] != 0) {
      *(undefined4 *)(param_1[0x2c] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2c] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x1d);
      *(float *)(param_1[0x2c] + 0x78) = *(float *)(param_1[0x2c] + 0x78) - _DAT_005a34a0;
      (**(code **)(*(int *)param_1[0x2c] + 0x3c))(0xff5555aa);
      FUN_0054ac09(param_1[0x2c]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 8;
    if (pvVar3 == (void *)0x0) {
      local_a4 = 0;
    }
    else {
      local_a4 = FUN_004c8f70(2,0,0x3f000000,0x3f000000,0x3f000000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x2d] = local_a4;
    if (param_1[0x2d] != 0) {
      *(undefined4 *)(param_1[0x2d] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2d] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x1d);
      *(float *)(param_1[0x2d] + 0x78) = *(float *)(param_1[0x2d] + 0x78) - _DAT_005a34a0;
      FUN_0054ac09(param_1[0x2d]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 9;
    if (pvVar3 == (void *)0x0) {
      local_a8 = 0;
    }
    else {
      local_a8 = FUN_004c8f70(0x3c,0,0x40200000,0x40000000,0x40200000,0,1,0x50);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2e] = local_a8;
    if (param_1[0x2e] != 0) {
      *(undefined4 *)(param_1[0x2e] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2e] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x1d);
      *(float *)(param_1[0x2e] + 0x78) = *(float *)(param_1[0x2e] + 0x78) - _DAT_005a34a0;
      (**(code **)(*(int *)param_1[0x2e] + 0x3c))(0xff5555aa);
      FUN_0054ac09(param_1[0x2e]);
    }
  }
  else if (param_1[0x27] == 6) {
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 10;
    if (pvVar3 == (void *)0x0) {
      local_ac = 0;
    }
    else {
      local_ac = FUN_004c8f70(0x38,0,0x40200000,0x40200000,0x40200000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x2c] = local_ac;
    if (param_1[0x2c] != 0) {
      *(undefined4 *)(param_1[0x2c] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2c] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x1d);
      *(float *)(param_1[0x2c] + 0x78) = *(float *)(param_1[0x2c] + 0x78) - _DAT_005a34a0;
      (**(code **)(*(int *)param_1[0x2c] + 0x3c))(0xff5555aa);
      FUN_0054ac09(param_1[0x2c]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 0xb;
    if (pvVar3 == (void *)0x0) {
      local_b0 = 0;
    }
    else {
      local_b0 = FUN_004c8f70(2,0,0x3fc00000,0x3fc00000,0x3fc00000,0,1,0x50);
    }
    local_8._0_1_ = 0;
    param_1[0x2d] = local_b0;
    if (param_1[0x2d] != 0) {
      *(undefined4 *)(param_1[0x2d] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2d] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x1d);
      *(float *)(param_1[0x2d] + 0x78) = *(float *)(param_1[0x2d] + 0x78) - _DAT_005a34a0;
      FUN_0054ac09(param_1[0x2d]);
    }
    pvVar3 = operator_new(0x180);
    local_8._0_1_ = 0xc;
    if (pvVar3 == (void *)0x0) {
      local_b4 = 0;
    }
    else {
      local_b4 = FUN_004c8f70(0x3c,0,0x40200000,0x40000000,0x40200000,0,1,0x50);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x2e] = local_b4;
    if (param_1[0x2e] != 0) {
      *(undefined4 *)(param_1[0x2e] + 0x6c) = 1;
      *(undefined4 *)(param_1[0x2e] + 0x104) = 0;
      FUN_004310a0(param_1 + 0x1d);
      *(float *)(param_1[0x2e] + 0x78) = *(float *)(param_1[0x2e] + 0x78) - _DAT_005a34a0;
      (**(code **)(*(int *)param_1[0x2e] + 0x3c))(0xff5555aa);
      FUN_0054ac09(param_1[0x2e]);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

