// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004edd60 | Name: FUN_004edd60


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004edd60(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 in_stack_00000010;
  undefined4 uVar5;
  int *local_50;
  int local_4c;
  int local_48;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a152a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a51f0;
  param_1[0x23] = in_stack_00000010;
  FUN_004310a0(&stack0x00000004);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 1000;
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 1;
  if (pvVar2 == (void *)0x0) {
    local_48 = 0;
  }
  else {
    local_48 = FUN_004cb900(8,1000,(float)(int)param_1[0x23] * _DAT_005a430c + _DAT_005a45d4,
                            (float)(int)param_1[0x23] * _DAT_005a430c + _DAT_005a45d4,
                            (float)(int)param_1[0x23] * _DAT_005a430c + _DAT_005a45d4,0x3b449ba6,0);
  }
  local_8._0_1_ = 0;
  if (local_48 != 0) {
    *(undefined4 *)(local_48 + 0x6c) = 1;
    FUN_004310a0(param_1 + 0x1d);
    *(float *)(local_48 + 0x78) = *(float *)(local_48 + 0x78) - _DAT_005a414c;
    FUN_0054ac09(local_48);
  }
  pvVar2 = operator_new(0x180);
  local_8._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    local_4c = 0;
  }
  else {
    local_4c = FUN_004c8f70(0x21,1000,(float)(int)param_1[0x23] * _DAT_005a430c + _DAT_005a430c,
                            (float)(int)param_1[0x23] * _DAT_005a430c + _DAT_005a430c,
                            (float)(int)param_1[0x23] * _DAT_005a430c + _DAT_005a430c,0,9,0x6f);
  }
  local_8._0_1_ = 0;
  if (local_4c != 0) {
    *(undefined4 *)(local_4c + 0x6c) = 1;
    *(undefined4 *)(local_4c + 0x104) = 0;
    FUN_004310a0(param_1 + 0x1d);
    *(float *)(local_4c + 0x78) = *(float *)(local_4c + 0x78) - _DAT_005a34a0;
    FUN_0054ac09(local_4c);
  }
  pvVar2 = operator_new(0xc0);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_50 = (int *)0x0;
  }
  else {
    local_50 = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (local_50 != (int *)0x0) {
    (**(code **)(*local_50 + 0x44))(0x70704000);
    local_50[0x1b] = 1;
    puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_50 + 0x3c))(*puVar3,puVar3[1]);
    local_50[0x28] = 0x5dc;
    FUN_0054ac09(local_50);
  }
  local_14 = 0xa6;
  if (param_1[0x23] == 1) {
    local_14 = 0x9a;
  }
  if (DAT_005ccf98 != 0) {
    iVar4 = FUN_00429a6d(local_14);
    if (iVar4 != 0) {
      uVar5 = 0;
      uVar1 = 0;
      FUN_00429a6d(local_14);
      FUN_0042ad2b(uVar1,uVar5);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

