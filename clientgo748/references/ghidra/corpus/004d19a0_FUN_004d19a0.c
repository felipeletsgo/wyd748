// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d19a0 | Name: FUN_004d19a0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004d19a0(undefined4 *param_1,float param_2,float param_3,float param_4,int param_5)

{
  undefined4 uVar1;
  void *pvVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  uint uVar6;
  uint uVar7;
  float fVar8;
  float fVar9;
  int *local_100;
  int *local_fc;
  int *local_f8;
  int local_f4;
  int *local_e8;
  uint local_84;
  int local_80;
  int local_78 [10];
  int *local_50;
  int local_4c [14];
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0992;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a496c;
  param_1[0x23] = param_5;
  FUN_004310a0(&param_2);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  local_4c[4] = 0xfffffffe;
  local_4c[5] = 0;
  local_4c[6] = 0xffffffff;
  local_4c[7] = 0xffffffff;
  local_4c[0] = 0;
  local_4c[1] = 0xffffffff;
  local_4c[2] = 0;
  local_4c[3] = 0xffffffff;
  param_5 = param_5 % 3;
  local_4c[8] = 0xff558833;
  local_4c[9] = 0xff335511;
  local_4c[10] = 0xff666600;
  local_4c[0xb] = 0xff333300;
  local_4c[0xc] = 0xff665566;
  local_4c[0xd] = 0xff332833;
  local_78[0] = 0x36;
  local_78[1] = 0x7a;
  local_78[2] = 0x7a;
  local_78[3] = 0x37;
  local_78[4] = 0x38;
  local_78[5] = 0x38;
  if (DAT_0092e660 == 0) {
    if ((param_5 == 1) || (param_5 == 2)) {
      for (local_84 = 0; (int)local_84 < 8; local_84 = local_84 + 1) {
        pvVar2 = operator_new(0x180);
        local_8._0_1_ = 1;
        if (pvVar2 == (void *)0x0) {
          local_e8 = (int *)0x0;
        }
        else {
          uVar3 = local_84 & 0x80000001;
          if ((int)uVar3 < 0) {
            uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
          }
          uVar6 = local_84 & 0x80000001;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
          }
          uVar7 = local_84 & 0x80000001;
          if ((int)uVar7 < 0) {
            uVar7 = (uVar7 - 1 | 0xfffffffe) + 1;
          }
          local_e8 = (int *)FUN_004c8f70(0x38,local_84 * 300 + 2000,
                                         (float)(int)uVar7 * 0.1 + _DAT_005a4214,
                                         (float)(int)uVar6 * 0.1 + _DAT_005a4214,
                                         (float)(int)uVar3 * 0.1 + _DAT_005a4214,0,1,0x50);
        }
        local_8 = (uint)local_8._1_3_ << 8;
        if (local_e8 != (int *)0x0) {
          local_e8[0x4f] = 0x3f000000;
          local_e8[0x50] = 0x3fc00000;
          local_e8[0x1b] = 1;
          local_e8[0x52] = 3;
          iVar4 = _rand();
          fVar9 = (float)(iVar4 % 5 + -3) * _DAT_005a4158 + param_4;
          fVar8 = param_3 + _DAT_005a40ec;
          iVar4 = _rand();
          uVar1 = FUN_00430f50((float)(iVar4 % 5 + -3) * _DAT_005a4158 + param_2,fVar8,fVar9);
          uVar1 = FUN_004310a0(uVar1);
          FUN_004310a0(uVar1);
          if (param_5 == 1) {
            (**(code **)(*local_e8 + 0x3c))(0xffffff00);
          }
          else {
            (**(code **)(*local_e8 + 0x3c))(0xffffeeff);
          }
          FUN_0054ac09(local_e8);
        }
      }
    }
    for (local_80 = 0; local_80 < 4; local_80 = local_80 + 1) {
      pvVar2 = operator_new(0x180);
      local_8._0_1_ = 2;
      if (pvVar2 == (void *)0x0) {
        local_f4 = 0;
      }
      else {
        local_f4 = FUN_004c8f70(local_78[param_5 % 3],local_80 * 400 + 0x5dc,0x3f4ccccd,0x3f4ccccd,
                                0x3f4ccccd,0,1,0x50);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      local_78[local_80 + 6] = local_f4;
      if (local_78[local_80 + 6] != 0) {
        uVar1 = FUN_00430f50((float)local_4c[local_80 + 4] * _DAT_005a34a0 + (float)param_1[0x1d],
                             (float)param_1[0x1e] - _DAT_005a3660,
                             (float)local_4c[local_80] * _DAT_005a34a0 + (float)param_1[0x1f]);
        FUN_004310a0(uVar1);
        *(float *)(local_78[local_80 + 6] + 0xf0) = (float)local_80 * _DAT_005a41ec + _DAT_005a45e8;
        *(undefined4 *)(local_78[local_80 + 6] + 0x6c) = 1;
        *(undefined4 *)(local_78[local_80 + 6] + 0x100) = 1;
        (**(code **)(*(int *)local_78[local_80 + 6] + 0x3c))(local_4c[param_5 * 2 + 8]);
        FUN_0054ac09(local_78[local_80 + 6]);
      }
    }
  }
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_f8 = (int *)0x0;
  }
  else {
    local_f8 = (int *)FUN_004cb900(local_78[param_5 % 3 + 3],2000,0x3c23d70a,0x3c23d70a,0x3c23d70a,
                                   0x3b03126f,0);
  }
  local_8._0_1_ = 0;
  local_14 = local_f8;
  if (local_f8 != (int *)0x0) {
    local_f8[0x1b] = 1;
    FUN_004310a0(param_1 + 0x1d);
    local_14[0x1e] = (int)((float)local_14[0x1e] + _DAT_005a4214);
    (**(code **)(*local_14 + 0x3c))(local_4c[param_5 * 2 + 8]);
    FUN_0054ac09(local_14);
  }
  pvVar2 = operator_new(0x124);
  local_8._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    local_fc = (int *)0x0;
  }
  else {
    local_fc = (int *)FUN_004cb900(2,2000,0x40000000,0x40000000,0x40000000,0,0);
  }
  local_8._0_1_ = 0;
  if (local_fc != (int *)0x0) {
    local_fc[0x1b] = 1;
    FUN_004310a0(param_1 + 0x1d);
    local_fc[0x42] = 1;
    local_fc[0x1e] = (int)((float)local_fc[0x1e] + _DAT_005a40ec);
    (**(code **)(*local_fc + 0x3c))(0xff555555);
    FUN_0054ac09(local_fc);
  }
  pvVar2 = operator_new(0xc0);
  local_8._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    local_100 = (int *)0x0;
  }
  else {
    local_100 = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  local_50 = local_100;
  if (local_100 != (int *)0x0) {
    (**(code **)(*local_100 + 0x44))(local_4c[param_5 * 2 + 9]);
    local_50[0x1b] = 1;
    puVar5 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_50 + 0x3c))(*puVar5,puVar5[1]);
    local_50[0x28] = 3000;
    FUN_0054ac09(local_50);
  }
  param_1[0x21] = 1000;
  ExceptionList = local_10;
  return param_1;
}

