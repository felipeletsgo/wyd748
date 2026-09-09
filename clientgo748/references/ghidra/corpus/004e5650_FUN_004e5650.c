// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e5650 | Name: FUN_004e5650


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004e5650(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            int param_5)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  int local_b8;
  int *local_b4;
  int *local_b0;
  int local_ac;
  int local_a8;
  int local_54;
  int aiStack_48 [4];
  int *local_38;
  int local_34 [9];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a10e6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8._0_1_ = 0;
  local_8._1_3_ = 0;
  *param_1 = &PTR_FUN_005a4fbc;
  param_1[0x23] = param_5;
  FUN_004310a0(&param_2);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  local_34[4] = 0xfffffffe;
  local_34[5] = 0;
  local_34[6] = 0xffffffff;
  local_34[7] = 0xffffffff;
  local_34[0] = 0;
  local_34[1] = 0xffffffff;
  local_34[2] = 0;
  local_34[3] = 0xffffffff;
  if (param_5 == 0) {
    for (local_54 = 0; local_54 < 4; local_54 = local_54 + 1) {
      pvVar2 = operator_new(0x180);
      local_8._0_1_ = 1;
      if (pvVar2 == (void *)0x0) {
        local_a8 = 0;
      }
      else {
        local_a8 = FUN_004c8f70(0x36,local_54 * 400 + 0x5dc,0x3f4ccccd,0x3f4ccccd,0x3f4ccccd,0,1,
                                0x50);
      }
      local_8._0_1_ = 0;
      aiStack_48[local_54] = local_a8;
      if (aiStack_48[local_54] != 0) {
        uVar1 = FUN_00430f50((float)local_34[local_54 + 4] * _DAT_005a34a0 + (float)param_1[0x1d],
                             (float)param_1[0x1e] - _DAT_005a3660,
                             (float)local_34[local_54] * _DAT_005a34a0 + (float)param_1[0x1f]);
        FUN_004310a0(uVar1);
        *(float *)(aiStack_48[local_54] + 0xf0) = (float)local_54 * _DAT_005a41ec + _DAT_005a45e8;
        *(undefined4 *)(aiStack_48[local_54] + 0x6c) = 1;
        *(undefined4 *)(aiStack_48[local_54] + 0x100) = 1;
        FUN_0054ac09(aiStack_48[local_54]);
      }
    }
    pvVar2 = operator_new(0x124);
    local_8._0_1_ = 2;
    if (pvVar2 == (void *)0x0) {
      local_ac = 0;
    }
    else {
      local_ac = FUN_004cb900(0x37,2000,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3b449ba6,0);
    }
    local_8._0_1_ = 0;
    if (local_ac != 0) {
      *(undefined4 *)(local_ac + 0x6c) = 1;
      FUN_004310a0(param_1 + 0x1d);
      *(float *)(local_ac + 0x78) = *(float *)(local_ac + 0x78) + _DAT_005a4214;
      FUN_0054ac09(local_ac);
    }
  }
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_b0 = (int *)0x0;
  }
  else {
    local_b0 = (int *)FUN_004cb900(2,2000,0x40000000,0x40000000,0x40000000,0x3b03126f,0);
  }
  local_8._0_1_ = 0;
  if (local_b0 != (int *)0x0) {
    local_b0[0x1b] = 1;
    FUN_004310a0(param_1 + 0x1d);
    local_b0[0x42] = 1;
    local_b0[0x1e] = (int)((float)local_b0[0x1e] + _DAT_005a40ec);
    FUN_0054ac09(local_b0);
  }
  pvVar2 = operator_new(0xc0);
  local_8._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    local_b4 = (int *)0x0;
  }
  else {
    local_b4 = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  local_38 = local_b4;
  if (local_b4 != (int *)0x0) {
    local_b4[0x1b] = 1;
    puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_38 + 0x3c))(*puVar3,puVar3[1]);
    local_38[0x28] = 3000;
    FUN_0054ac09(local_38);
  }
  local_34[8] = 0x9f;
  if (param_5 == 0) {
    (**(code **)(*local_b0 + 0x3c))(0xffaaaaaa);
    (**(code **)(*local_38 + 0x44))(0x55555555);
  }
  else if (param_5 == 1) {
    (**(code **)(*local_b0 + 0x3c))(0xffaa77ff);
    (**(code **)(*local_38 + 0x44))(0x55553388);
    local_34[8] = 0x9e;
  }
  else if (param_5 == 2) {
    (**(code **)(*local_b0 + 0x3c))(0xffaa0000);
    (**(code **)(*local_38 + 0x44))(0x55883333);
  }
  if ((param_5 == 0) || (param_5 == 2)) {
    pvVar2 = operator_new(0x90);
    local_8._0_1_ = 5;
    if (pvVar2 == (void *)0x0) {
      local_b8 = 0;
    }
    else {
      uVar1 = 1000;
      puVar3 = (undefined4 *)FUN_00430f50(0,0,0);
      local_b8 = FUN_004d4de0(param_2,param_3,param_4,0,10,0x3d4ccccd,0xffffffff,0,0x38,0x3f800000,1
                              ,*puVar3,puVar3[1],puVar3[2],uVar1);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (local_b8 != 0) {
      FUN_0054ac09(local_b8);
    }
  }
  param_1[0x21] = 1000;
  if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(local_34[8]), iVar4 != 0)) {
    uVar5 = 0;
    uVar1 = 0;
    FUN_00429a6d(local_34[8]);
    FUN_0042ad2b(uVar1,uVar5);
  }
  ExceptionList = local_10;
  return param_1;
}

