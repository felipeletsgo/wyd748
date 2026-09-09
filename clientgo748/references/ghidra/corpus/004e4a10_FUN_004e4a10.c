// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e4a10 | Name: FUN_004e4a10


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004e4a10(undefined4 *param_1,float param_2,float param_3,float param_4,int param_5)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  undefined4 uVar7;
  int *local_90;
  int *local_7c;
  uint local_38;
  undefined4 local_28;
  void *local_24 [6];
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a103f;
  local_24[5] = ExceptionList;
  ExceptionList = local_24 + 5;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4f08;
  param_1[0x23] = param_5;
  FUN_004310a0(&param_2);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 3000;
  local_24[0] = (void *)0xffff0000;
  local_24[1] = (void *)0xffaa00ff;
  local_24[2] = (void *)0xff00aa00;
  local_24[3] = (void *)0xffffeeff;
  local_24[4] = (void *)0xffffaa00;
  if (DAT_0092e660 == 0) {
    for (local_38 = 0; (int)local_38 < 0xf; local_38 = local_38 + 1) {
      iVar2 = _rand();
      pvVar3 = operator_new(0x180);
      local_8._0_1_ = 1;
      if (pvVar3 == (void *)0x0) {
        local_7c = (int *)0x0;
      }
      else {
        uVar6 = local_38 & 0x80000001;
        if ((int)uVar6 < 0) {
          uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
        }
        uVar4 = local_38 & 0x80000001;
        if ((int)uVar4 < 0) {
          uVar4 = (uVar4 - 1 | 0xfffffffe) + 1;
        }
        uVar5 = local_38 & 0x80000001;
        if ((int)uVar5 < 0) {
          uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
        }
        local_7c = (int *)FUN_004c8f70(0x38,local_38 * 300 + 2000,
                                       (float)(int)uVar5 * 0.1 + _DAT_005a4214,
                                       (float)(int)uVar4 * 0.1 + _DAT_005a4214,
                                       (float)(int)uVar6 * 0.1 + _DAT_005a4214,0,1,0x50);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_7c != (int *)0x0) {
        local_7c[0x4f] = (int)((float)((int)local_38 % 3) * _DAT_005a439c + _DAT_005a4214);
        local_7c[0x50] = -0x3f200000;
        local_7c[0x1b] = 1;
        local_7c[0x52] = 3;
        uVar1 = FUN_00430f50((float)(iVar2 % 5 + -3) * _DAT_005a4214 + param_2,
                             (float)(iVar2 % 5 + -3) * _DAT_005a4214 + param_3 + _DAT_005a4104,
                             (float)(iVar2 % 5 + -3) * _DAT_005a4214 + param_4);
        uVar1 = FUN_004310a0(uVar1);
        FUN_004310a0(uVar1);
        (**(code **)(*local_7c + 0x3c))(local_24[param_5]);
        FUN_0054ac09(local_7c);
      }
    }
  }
  local_28 = 0x34;
  if (param_5 == 1) {
    local_28 = 0x33;
  }
  if (((param_5 == 2) || (param_5 == 3)) || (param_5 == 4)) {
    local_28 = 0x7a;
  }
  pvVar3 = operator_new(0x180);
  local_8._0_1_ = 2;
  if (pvVar3 == (void *)0x0) {
    local_90 = (int *)0x0;
  }
  else {
    local_90 = (int *)FUN_004c8f70(local_28,4000,0x3fc00000,0x41200000,0x3fc00000,0,1,0x50);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (local_90 != (int *)0x0) {
    local_90[0x1b] = 1;
    local_90[0x45] = 0x4032b8c3;
    uVar1 = FUN_00430f50(param_2,param_3 + _DAT_005a365c,param_4);
    uVar1 = FUN_004310a0(uVar1);
    FUN_004310a0(uVar1);
    if (((param_5 == 2) || (param_5 == 3)) || (param_5 == 4)) {
      (**(code **)(*local_90 + 0x3c))(local_24[param_5]);
    }
    FUN_0054ac09(local_90);
  }
  if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x9f), iVar2 != 0)) {
    uVar7 = 0;
    uVar1 = 0;
    FUN_00429a6d(0x9f);
    FUN_0042ad2b(uVar1,uVar7);
  }
  ExceptionList = local_24[5];
  return param_1;
}

