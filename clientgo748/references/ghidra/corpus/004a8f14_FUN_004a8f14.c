// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a8f14 | Name: FUN_004a8f14


/* WARNING: Removing unreachable block (ram,0x004aab40) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004a8f14(int *param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_740;
  int local_73c;
  int local_738;
  int local_734;
  int local_730;
  int local_72c;
  int local_728;
  undefined4 local_720;
  int local_71c;
  undefined4 local_714;
  int local_710;
  int local_6d4;
  int local_6d0;
  int local_6cc;
  int local_63c;
  undefined1 local_634 [128];
  int local_5b4;
  int local_5b0;
  int local_5ac;
  int local_5a8;
  int local_5a4;
  uint local_5a0;
  float local_59c;
  float local_598;
  int local_594;
  int *local_590;
  int local_58c;
  int local_588;
  undefined1 local_584 [1024];
  int local_184;
  undefined4 auStack_180 [11];
  int local_154;
  int local_150;
  undefined1 local_14c [128];
  _SYSTEMTIME local_cc;
  undefined1 local_bc [128];
  undefined4 local_3c;
  undefined1 local_38 [4];
  undefined1 local_34 [4];
  undefined1 local_30 [4];
  undefined1 local_2c [4];
  undefined1 local_28 [4];
  undefined1 local_24 [4];
  undefined1 local_20 [4];
  undefined1 local_1c [4];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a03bc;
  local_10 = ExceptionList;
  if (DAT_005b892c == 2) {
    ExceptionList = &local_10;
    (**(code **)(*param_1 + 0x3c))(s_UI_SelServerScene2_txt_005b95dc);
  }
  else {
    ExceptionList = &local_10;
    (**(code **)(*param_1 + 0x3c))(s_UI_SelServerScene_txt_005b95f4);
  }
  _memset(&local_cc,0,0x10);
  GetLocalTime(&local_cc);
  param_1[0x9f4c] = (local_cc._10_4_ & 0xffff) % 3 + 1;
  (**(code **)(*DAT_005ccec0 + 0x60))(0);
  *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0;
  _memset(param_1 + 0x9b9e,0,0x28);
  _memset(param_1 + 0x9bb2,0,0x28);
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x120a);
  param_1[0x9b88] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x137);
  param_1[0x9b8a] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x138);
  param_1[0x9b8b] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x120d);
  param_1[0x9b8c] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x120e);
  param_1[0x9b8d] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x120f);
  param_1[0x9b8e] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1210);
  param_1[0x9b8f] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x120c);
  param_1[0x9b90] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1201);
  param_1[0x9b98] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1202);
  param_1[0x9b99] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1203);
  param_1[0x9b9a] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1200);
  param_1[0x9b91] = iVar1;
  if (DAT_005b892c == 2) {
    if (param_1[0x9b8e] != 0) {
      (**(code **)(*(int *)param_1[0x9b8e] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a4404,0x40800000);
    }
    if (param_1[0x9b8f] != 0) {
      (**(code **)(*(int *)param_1[0x9b8f] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a3c68,0x40800000);
    }
    DAT_005d04f4 = 0x8c;
    iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1603);
    param_1[0x9b95] = iVar1;
    iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1604);
    param_1[0x9b96] = iVar1;
    if (param_1[0x9b95] != 0) {
      *(float *)(param_1[0x9b95] + 0x50) = *(float *)(param_1[0x9b95] + 0x50) + _DAT_005a4104;
    }
    if (param_1[0x9b96] != 0) {
      *(float *)(param_1[0x9b96] + 0x4c) = *(float *)(param_1[0x9b96] + 0x4c) + _DAT_005a366c;
      *(float *)(param_1[0x9b96] + 0x50) = *(float *)(param_1[0x9b96] + 0x50) + _DAT_005a4104;
    }
    iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1605);
    param_1[0x9b97] = iVar1;
    iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1600);
    param_1[0x9b92] = iVar1;
    iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1601);
    param_1[0x9b93] = iVar1;
    iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1602);
    param_1[0x9b94] = iVar1;
    *(undefined4 *)(param_1[0x9b90] + 0x50) = 0x43610000;
    *(undefined4 *)(param_1[0x9b97] + 0x50) = 0x43610000;
    *(float *)(param_1[0x9b92] + 0x50) = *(float *)(param_1[0x9b92] + 0x50) + _DAT_005a4104;
    pvVar2 = operator_new(0x1e4);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_6cc = 0;
    }
    else {
      local_6cc = FUN_00401466(0xfffffe78,0x430f0000,0,0x42200000,0x42340000,0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0x9ba9] = local_6cc;
    local_588 = DAT_005d04f4 + -0x41;
    pvVar2 = operator_new(0x1e4);
    local_8 = 1;
    if (pvVar2 == (void *)0x0) {
      local_6d0 = 0;
    }
    else {
      local_6d0 = FUN_00401466(0xfffffe77,0x43370000,0,(float)local_588,0x42340000,0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0x9baa] = local_6d0;
    pvVar2 = operator_new(0x1e4);
    local_8 = 2;
    if (pvVar2 == (void *)0x0) {
      local_6d4 = 0;
    }
    else {
      local_6d4 = FUN_00401466(0xfffffe76,(float)local_588 + _DAT_005a45c4,0,0x42200000,0x42340000,
                               0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0x9bab] = local_6d4;
    for (local_58c = 0; local_58c < 3; local_58c = local_58c + 1) {
      FUN_0054ac09(param_1[local_58c + 0x9ba9]);
    }
    local_590 = (int *)(**(code **)(*(int *)param_1[10] + 0x48))(0x301);
    (**(code **)(*local_590 + 0x70))
              (local_590[0x13],(float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a3970);
    (**(code **)(*(int *)param_1[0x9b91] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9b91] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9b91] + 0x58) * _DAT_005a34a0);
    (**(code **)(*(int *)param_1[0x9b8d] + 0x70))
              ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
               *(float *)(param_1[0x9b8d] + 0x54) * _DAT_005a34a0,
               (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
               *(float *)(param_1[0x9b8d] + 0x58) * _DAT_005a34a0);
    if (*(int *)(DAT_005ccf9c + 0x2a504) == 0x640) {
      local_59c = DAT_005b490c;
      local_598 = DAT_005b4910;
      FUN_004012c6(_DAT_005a45c0 * DAT_005b490c,_DAT_005a3670 * DAT_005b4910);
      FUN_004012c6(_DAT_005a4534 * local_59c,_DAT_005a3670 * local_598);
      FUN_004012e5(_DAT_005a41e4 * local_59c,_DAT_005a41e4 * local_598);
      FUN_004012e5(_DAT_005a41e4 * local_59c,_DAT_005a41e4 * local_598);
    }
    else {
      local_594 = 0;
      if (*(int *)(DAT_005ccf9c + 0x2a504) == 0x400) {
        local_594 = 0x14;
      }
      else if (*(int *)(DAT_005ccf9c + 0x2a504) == 0x500) {
        local_594 = 0x28;
      }
      (**(code **)(*(int *)param_1[0x9b8a] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 *(float *)(param_1[0x9b8a] + 0x54),_DAT_005a3670 * DAT_005b4910 + (float)local_594)
      ;
      (**(code **)(*(int *)param_1[0x9b8b] + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0,
                 _DAT_005a3670 * DAT_005b4910 + (float)local_594);
    }
  }
  else {
    DAT_005d04f4 = 0x78;
  }
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1401);
  param_1[0x9f47] = iVar1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x1402);
  param_1[0x9f48] = iVar1;
  *(undefined4 *)(param_1[0x9f47] + 0x105c) = 1;
  *(undefined4 *)(param_1[0x9f48] + 0x105c) = 1;
  if (param_1[0x9b88] != 0) {
    for (local_5a4 = 0; local_5a4 < 0xb; local_5a4 = local_5a4 + 1) {
      if ((&DAT_009325f8)[local_5a4 * 0x2c0] == '\0') {
        param_1[0x9b9b] = local_5a4 + -1;
        break;
      }
    }
    local_5a0 = (local_cc._6_4_ & 0xffff) % 10;
    if (local_5a0 == 0) {
      local_5a0 = 10;
    }
    for (local_5a4 = 0; local_5a4 < 10; local_5a4 = local_5a4 + 1) {
      for (local_5ac = 1; local_5ac < 0xb; local_5ac = local_5ac + 1) {
        if ((&DAT_009325f8)[local_5ac * 0x40 + local_5a4 * 0x2c0] != '\0') {
          param_1[local_5a4 + 0x9b9e] = param_1[local_5a4 + 0x9b9e] + 1;
        }
      }
      if (param_1[local_5a4 + 0x9b9e] != 0) {
        local_5b0 = (int)((longlong)(ulonglong)(local_cc._6_4_ & 0xffff) %
                         (longlong)param_1[local_5a4 + 0x9b9e]);
        if (local_5b0 == 0) {
          local_5b0 = param_1[local_5a4 + 0x9b9e];
        }
        param_1[local_5a4 + 0x9b9e] = local_5b0;
      }
    }
    param_1[0x9b9c] = 0;
    for (local_5a8 = 0; local_5a8 < 0xc; local_5a8 = local_5a8 + 1) {
      if (*(int *)(&DAT_013b71b0 + local_5a8 * 4) < param_1[0x9b9c]) {
        local_710 = param_1[0x9b9c];
      }
      else {
        local_710 = *(int *)(&DAT_013b71b0 + local_5a8 * 4);
      }
      param_1[0x9b9c] = local_710;
      if (param_1[0x9b9b] < *(int *)(&DAT_013b71b0 + local_5a8 * 4)) {
        param_1[0x9b9b] = *(int *)(&DAT_013b71b0 + local_5a8 * 4);
      }
    }
    param_1[0x9b9c] = param_1[0x9b9c] + -1;
    if (DAT_00933eb8 != '\0') {
      param_1[0x9b9b] = 9;
    }
    for (local_5a4 = param_1[0x9b9b]; -1 < local_5a4; local_5a4 = local_5a4 + -1) {
      local_5b4 = *(int *)(&DAT_013b71b0 + local_5a4 * 4) + -1;
      if (((&DAT_009325f8)[local_5b4 * 0x2c0] != '\0') &&
         (*(int *)(&DAT_013b71b0 + local_5a4 * 4) != 0)) {
        if ((&DAT_013b714c)[local_5b4 * 9] == '\0') {
          FUN_0058f078(local_634,&DAT_00a36278,*(int *)(&DAT_013b71b0 + local_5a4 * 4));
        }
        else {
          FUN_0058f078(local_634,&DAT_013b714c + local_5b4 * 9);
        }
        if (DAT_005b892c == 2) {
          pvVar2 = operator_new(0xe50);
          local_8 = 3;
          if (pvVar2 == (void *)0x0) {
            local_714 = 0;
          }
          else {
            local_714 = FUN_00407203(local_634,0xffffffff,0,0,0x427c0000,0x41800000,0,0x77777777,1,0
                                    );
          }
          local_8 = 0xffffffff;
          auStack_180[local_5a4] = local_714;
          pvVar2 = operator_new(0x1e4);
          local_8 = 4;
          if (pvVar2 == (void *)0x0) {
            local_71c = 0;
          }
          else {
            local_71c = FUN_00401466(0xfffffe75,0x41880000,(float)(local_5a4 * 0x1a) + _DAT_005a45bc
                                     ,0x42da0000,0x41b00000,0xffffffff,4);
          }
          local_8 = 0xffffffff;
          param_1[local_5a4 + 0x9bb2] = local_71c;
          FUN_0054ac09(param_1[local_5a4 + 0x9bb2]);
          if (local_5a4 == 0) {
            *(undefined4 *)(param_1[0x9bb2] + 0x28) = 0;
          }
        }
        else {
          pvVar2 = operator_new(0xe50);
          local_8 = 5;
          if (pvVar2 == (void *)0x0) {
            local_720 = 0;
          }
          else {
            local_720 = FUN_00407203(local_634,0xffffffff,0,0,0x42c80000,0x41800000,0,0x77777777,1,1
                                    );
          }
          local_8 = 0xffffffff;
          auStack_180[local_5a4] = local_720;
        }
        FUN_00408d33(auStack_180[local_5a4]);
      }
    }
  }
  _DAT_005d04f8 = param_1[0x9b87] + 1;
  iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x120b);
  param_1[0x9b89] = iVar1;
  if (param_1[0x9b89] != 0) {
    (**(code **)(*(int *)param_1[0x9b89] + 0x60))(0);
  }
  *(undefined1 *)(DAT_013b71e8 + 0xef4) = 0xff;
  *(undefined1 *)(DAT_013b71e8 + 0xef5) = 0;
  _memset((void *)(DAT_013b71e8 + 4),0,0x2e8);
  _memset((void *)(DAT_013b71e8 + 0x2ec),0,0x400);
  _memset((void *)(DAT_013b71e8 + 0x6ec),0,0x2f4);
  for (local_154 = 0; local_154 < 4; local_154 = local_154 + 1) {
    *(undefined1 *)(DAT_013b71e8 + local_154 + 0x9d4) = 0xff;
  }
  if (DAT_013b71e4 != 0) {
    FUN_00423d7a();
  }
  if (*(int *)(DAT_013b71e8 + 0x1b090) == 1) {
    *(undefined4 *)(DAT_013b71e8 + 0x1b090) = 0;
    (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a38378,0,0);
    (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
  }
  for (local_184 = 0; local_184 < 0xb; local_184 = local_184 + 1) {
    *(undefined4 *)(local_38 + local_184 * 4 + -4) = 0xffffffff;
  }
  FUN_00557a41(&DAT_009325f8,local_584,0x400);
  FUN_005909d2(local_584,s__d__d__d__d__d__d__d__d__d__d_005b960c,&local_3c,local_38,local_34,
               local_30,local_2c,local_28,local_24,local_20,local_1c,local_18);
  if (param_1[0x9b88] != 0) {
    FUN_004ae5f0(0);
  }
  *(undefined1 *)(param_1 + 0x9f49) = 0;
  for (local_154 = 0; local_154 < 3; local_154 = local_154 + 1) {
    (**(code **)(*(int *)param_1[local_154 + 0x9b98] + 0x60))(0);
    if (param_1[local_154 + 0x9b98] != 0) {
      *(undefined4 *)(param_1[local_154 + 0x9b98] + 0x94) = 0;
    }
  }
  (**(code **)(*(int *)param_1[0x9b91] + 0x60))(0);
  *(undefined4 *)(param_1[0x9b91] + 0x94) = 0;
  if (DAT_005b892c == 2) {
    for (local_63c = 0; local_63c < param_1[0x9b9b]; local_63c = local_63c + 1) {
    }
    pvVar2 = operator_new(0x1e4);
    local_8 = 6;
    if (pvVar2 == (void *)0x0) {
      local_728 = 0;
    }
    else {
      local_728 = FUN_00401466(0xfffffe74,0x430e0000,0x42340000,
                               ((float)DAT_005d04f4 + _DAT_005a42fc) - _DAT_005a45b4,0x43530000,
                               0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0x9bac] = local_728;
    FUN_0054ac09(param_1[0x9bac]);
  }
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0x1e4);
    local_8 = 7;
    if (pvVar2 == (void *)0x0) {
      local_72c = 0;
    }
    else {
      local_72c = FUN_00401466(0xfffffe79,0,0,0x430f0000,0x43800000,0xffffffff,4);
    }
    local_8 = 0xffffffff;
    param_1[0x9ba8] = local_72c;
    FUN_0054ac09(param_1[0x9ba8]);
    *(float *)(param_1[0x9b88] + 0x50) = *(float *)(param_1[0x9b88] + 0x50) + _DAT_005a45b0;
    *(float *)(param_1[0x9b89] + 0x4c) = *(float *)(param_1[0x9b89] + 0x4c) + _DAT_005a4394;
    *(float *)(param_1[0x9b89] + 0x50) = *(float *)(param_1[0x9b89] + 0x50) + _DAT_005a45ac;
  }
  *(undefined4 *)(DAT_005ccf9c + 0x2a5e8) = 0;
  (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x50))(0);
  _memset(param_1 + 0x25,0,0x10000);
  if (param_1[0x9f4c] == 0) {
    FUN_0058f078(local_14c,s_env_Field2723_trn_005b962c);
    FUN_0058f078(local_bc,s_env_Field2723_dat_005b9640);
    DAT_005b8924 = 0xd00;
    DAT_005b8928 = 0xb00;
  }
  else if (param_1[0x9f4c] == 1) {
    FUN_0058f078(local_14c,s_env_Character_trn_005b9654);
    FUN_0058f078(local_bc,s_env_Character_dat_005b9668);
    DAT_005b8924 = 0x800;
    DAT_005b8928 = 0x800;
  }
  else if (param_1[0x9f4c] == 2) {
    FUN_0058f078(local_14c,s_env_Field0813_trn_005b967c);
    FUN_0058f078(local_bc,s_env_Field0813_dat_005b9690);
    DAT_005b8924 = 0x380;
    DAT_005b8928 = 0x600;
  }
  else if (param_1[0x9f4c] == 3) {
    FUN_0058f078(local_14c,s_env_Field0813_trn_005b96a4);
    FUN_0058f078(local_bc,s_env_Field0813_dat_005b96b8);
    DAT_005b8924 = 0x380;
    DAT_005b8928 = 0x600;
  }
  else if (param_1[0x9f4c] == 4) {
    FUN_0058f078(local_14c,s_env_Field2922_trn_005b96cc);
    FUN_0058f078(local_bc,s_env_Field2922_dat_005b96e0);
    DAT_005b8924 = 0xe00;
    DAT_005b8928 = 0xa80;
  }
  pvVar2 = operator_new(0x20a70);
  local_8 = 8;
  if (pvVar2 == (void *)0x0) {
    local_730 = 0;
  }
  else {
    local_730 = FUN_00532390();
  }
  local_8 = 0xffffffff;
  param_1[0x1a] = local_730;
  iVar1 = FUN_00533dd7(local_14c);
  if (iVar1 == 0) {
    if (param_1[0x15] == 0) {
      FUN_0049ed58(0xe,0,0,0,0);
    }
    param_1[0x15] = 1;
  }
  param_1[0x10] = param_1[0x1a];
  for (local_150 = 0; local_150 < 0x80; local_150 = local_150 + 1) {
    FUN_0058f220(param_1 + local_150 * 0x40 + 0x25,param_1[0x10] + 0xc0cc + local_150 * 0x80,0x80);
  }
  DAT_005b8924 = __ftol();
  DAT_005b8928 = __ftol();
  FUN_00534968();
  pvVar2 = operator_new(0x404c);
  local_8 = 9;
  if (pvVar2 == (void *)0x0) {
    local_734 = 0;
  }
  else {
    local_734 = FUN_00540ea3(param_1[0x10]);
  }
  local_8 = 0xffffffff;
  param_1[0x1c] = local_734;
  FUN_0054ac09(param_1[0x1c]);
  iVar1 = FUN_00541065(local_bc);
  if (iVar1 == 0) {
    FUN_00431159(s_DataFile_Not_Found_005b96f4);
    MessageBoxA(*(HWND *)(DAT_013b71e0 + 4),s_DataFile_Not_Found__005b9718,s_File_Lost_005b970c,0);
    PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
    uVar3 = 0;
  }
  else {
    FUN_0054ac09(param_1[0x1a]);
    if (param_1[0x9f4c] == 2) {
      FUN_0049add2(1);
    }
    pvVar2 = operator_new(0x16c);
    local_8 = 10;
    if (pvVar2 == (void *)0x0) {
      local_738 = 0;
    }
    else {
      local_738 = FUN_004eef50();
    }
    local_8 = 0xffffffff;
    param_1[0x12] = local_738;
    (**(code **)(*(int *)param_1[0x12] + 0x38))();
    FUN_0054ac09(param_1[0x12]);
    pvVar2 = operator_new(0x22a4);
    local_8 = 0xb;
    if (pvVar2 == (void *)0x0) {
      local_73c = 0;
    }
    else {
      local_73c = FUN_00547890();
    }
    local_8 = 0xffffffff;
    param_1[0x11] = local_73c;
    FUN_0054ac09(param_1[0x11]);
    _memset(&local_cc,0,0x10);
    GetLocalTime(&local_cc);
    DAT_005d04fc = local_cc._6_4_ & 3;
    FUN_00549bbf(DAT_005d04fc);
    DAT_005d04fc = DAT_005d04fc + 1 & 0x80000003;
    if ((int)DAT_005d04fc < 0) {
      DAT_005d04fc = (DAT_005d04fc - 1 | 0xfffffffc) + 1;
    }
    param_1[0x9bc1] = 0;
    pvVar2 = operator_new(0xd68);
    local_8 = 0xc;
    if (pvVar2 == (void *)0x0) {
      local_740 = 0;
    }
    else {
      local_740 = FUN_004de30e(0x3f800000);
    }
    local_8 = 0xffffffff;
    param_1[0x9bc1] = local_740;
    if (DAT_005d04fc == 2) {
      *(undefined4 *)(param_1[0x9bc1] + 0x28) = 1;
    }
    else {
      *(undefined4 *)(param_1[0x9bc1] + 0x28) = 0;
    }
    if (param_1[0xb] != 0) {
      FUN_0054ac09(param_1[0x9bc1]);
    }
    _memset(param_1 + 0x9bc3,0,200);
    uVar3 = FUN_00430f20(0x450408cd,0x45028000);
    FUN_0040bd30(uVar3);
    *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x28) = 0x4096cbe4;
    *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c) = 0x3e860a92;
    FUN_004ab9f7();
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x66), iVar1 != 0)) {
      FUN_00429a6d(0x66);
      iVar1 = FUN_0042afd0();
      if (iVar1 == 0) {
        uVar4 = 1;
        uVar3 = 0;
        FUN_00429a6d(0x66);
        FUN_0042ad2b(uVar3,uVar4);
      }
    }
    FUN_004ac55f();
    if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x65), iVar1 != 0)) {
      FUN_00429a6d(0x65);
      iVar1 = FUN_0042afd0();
      if (iVar1 != 0) {
        FUN_00429a6d(0x65);
        FUN_0042aef6();
      }
    }
    FUN_004a89a0();
    uVar3 = 1;
  }
  ExceptionList = local_10;
  return uVar3;
}

