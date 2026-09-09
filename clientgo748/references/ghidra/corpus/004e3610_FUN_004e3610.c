// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e3610 | Name: FUN_004e3610


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004e3610(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 in_stack_00000010;
  undefined4 uVar5;
  int *local_98;
  int *local_94;
  int local_90;
  int local_8c;
  int local_4c;
  int aiStack_48 [4];
  int *local_38;
  int local_34 [9];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0fb8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4e74;
  param_1[0x23] = in_stack_00000010;
  FUN_004310a0(&stack0x00000004);
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
  for (local_4c = 0; local_4c < 4; local_4c = local_4c + 1) {
    pvVar2 = operator_new(0x180);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_8c = 0;
    }
    else {
      local_8c = FUN_004c8f70(0x3a,local_4c * 400 + 0x5dc,0x3f19999a,0x3f4ccccd,0x3f19999a,0,1,0x50)
      ;
    }
    local_8 = (uint)local_8._1_3_ << 8;
    aiStack_48[local_4c] = local_8c;
    if (aiStack_48[local_4c] != 0) {
      uVar1 = FUN_00430f50((float)local_34[local_4c + 4] * _DAT_005a34a0 + (float)param_1[0x1d],
                           param_1[0x1e],
                           (float)local_34[local_4c] * _DAT_005a34a0 + (float)param_1[0x1f]);
      FUN_004310a0(uVar1);
      *(float *)(aiStack_48[local_4c] + 0xf0) = (float)local_4c * _DAT_005a41ec + _DAT_005a45e8;
      *(undefined4 *)(aiStack_48[local_4c] + 0x6c) = 1;
      *(undefined4 *)(aiStack_48[local_4c] + 0x100) = 1;
      (**(code **)(*(int *)aiStack_48[local_4c] + 0x3c))(0xffffff00);
      FUN_0054ac09(aiStack_48[local_4c]);
    }
  }
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    local_90 = 0;
  }
  else {
    local_90 = FUN_004cb900(0x5d,2000,0x3c23d70a,0x3c23d70a,0x3c23d70a,0x3b03126f,0);
  }
  local_8._0_1_ = 0;
  local_34[8] = local_90;
  if (local_90 != 0) {
    *(undefined4 *)(local_90 + 0x6c) = 1;
    FUN_004310a0(param_1 + 0x1d);
    *(float *)(local_34[8] + 0x78) = *(float *)(local_34[8] + 0x78) + _DAT_005a4214;
    FUN_0054ac09(local_34[8]);
  }
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_94 = (int *)0x0;
  }
  else {
    local_94 = (int *)FUN_004cb900(2,2000,0x40000000,0x40000000,0x40000000,0x3b03126f,0);
  }
  local_8._0_1_ = 0;
  if (local_94 != (int *)0x0) {
    local_94[0x1b] = 1;
    FUN_004310a0(param_1 + 0x1d);
    local_94[0x42] = 1;
    local_94[0x1e] = (int)((float)local_94[0x1e] + _DAT_005a40ec);
    (**(code **)(*local_94 + 0x3c))(0xffaaaaaa);
    FUN_0054ac09(local_94);
  }
  pvVar2 = operator_new(0xc0);
  local_8._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    local_98 = (int *)0x0;
  }
  else {
    local_98 = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  local_38 = local_98;
  if (local_98 != (int *)0x0) {
    (**(code **)(*local_98 + 0x44))(0x55555555);
    local_38[0x1b] = 1;
    puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_38 + 0x3c))(*puVar3,puVar3[1]);
    local_38[0x28] = 3000;
    FUN_0054ac09(local_38);
  }
  param_1[0x21] = 1000;
  if ((DAT_005ccf98 != 0) && (iVar4 = FUN_00429a6d(0x9f), iVar4 != 0)) {
    uVar5 = 0;
    uVar1 = 0;
    FUN_00429a6d(0x9f);
    FUN_0042ad2b(uVar1,uVar5);
  }
  ExceptionList = local_10;
  return param_1;
}

