// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ee190 | Name: FUN_004ee190


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004ee190(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 in_stack_00000010;
  undefined4 uVar5;
  int *local_98;
  int local_94;
  int local_90;
  int *local_8c;
  int *local_38 [10];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1578;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a522c;
  param_1[0x23] = in_stack_00000010;
  FUN_004310a0(&stack0x00000004);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 2000;
  local_38[0] = (int *)0xff77aaaa;
  local_38[1] = (int *)0xff7777aa;
  local_38[2] = (int *)0xff7777ff;
  local_38[3] = (int *)0xffffaa00;
  local_38[4] = (int *)0xffffaa00;
  local_38[5] = (int *)0xffffaa00;
  local_38[6] = (int *)0xff00ff77;
  local_38[7] = (int *)0xff00ffaa;
  local_38[8] = (int *)0xff00ffaa;
  pvVar2 = operator_new(0x180);
  local_8._0_1_ = 1;
  if (pvVar2 == (void *)0x0) {
    local_8c = (int *)0x0;
  }
  else {
    local_8c = (int *)FUN_004c8f70(0x2d,param_1[0x21],0x3f99999a,0x41000000,0x3f99999a,0,6,0x50);
  }
  local_8._0_1_ = 0;
  local_38[9] = local_8c;
  if (local_8c != (int *)0x0) {
    uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] - _DAT_005a3660,param_1[0x1f]);
    uVar1 = FUN_004310a0(uVar1);
    FUN_004310a0(uVar1);
    local_38[9][0x41] = 0;
    local_38[9][0x40] = 1;
    local_38[9][0x1b] = 1;
    (**(code **)(*local_38[9] + 0x3c))(local_38[param_1[0x23] * 3]);
  }
  pvVar2 = operator_new(0x180);
  local_8._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    local_90 = 0;
  }
  else {
    local_90 = FUN_004c8f70(0x2d,param_1[0x21],0x3f0ccccd,0x41000000,0x3f0ccccd,0,6,0x50);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (local_90 != 0) {
    uVar1 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] - _DAT_005a3660,param_1[0x1f]);
    uVar1 = FUN_004310a0(uVar1);
    FUN_004310a0(uVar1);
    *(undefined4 *)(local_90 + 0x104) = 0;
    *(undefined4 *)(local_90 + 0x6c) = 1;
    *(undefined4 *)(local_90 + 0x100) = 1;
    (**(code **)(*local_38[9] + 0x3c))(local_38[param_1[0x23] * 3 + 1]);
  }
  if (local_38[9] != (int *)0x0) {
    FUN_0054ac09(local_38[9]);
  }
  if (local_90 != 0) {
    FUN_0054ac09(local_90);
  }
  pvVar2 = operator_new(0x90);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_94 = 0;
  }
  else {
    local_94 = FUN_004ec9a0(param_1[0x1d],param_1[0x1e],param_1[0x1f],local_38[param_1[0x23] * 3],10
                            ,1,0);
  }
  local_8._0_1_ = 0;
  if (local_94 != 0) {
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
  if (local_98 != (int *)0x0) {
    (**(code **)(*local_98 + 0x44))(local_38[param_1[0x23] * 3 + 2]);
    local_98[0x1b] = 1;
    puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_98 + 0x3c))(*puVar3,puVar3[1]);
    local_98[0x28] = param_1[0x21];
    FUN_0054ac09(local_98);
  }
  if (DAT_005ccf98 != 0) {
    iVar4 = FUN_00429a6d(0x98);
    if (iVar4 != 0) {
      uVar5 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x98);
      FUN_0042ad2b(uVar1,uVar5);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

