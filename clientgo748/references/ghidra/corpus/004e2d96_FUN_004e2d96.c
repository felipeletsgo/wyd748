// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e2d96 | Name: FUN_004e2d96


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e2d96(int *param_1)

{
  float fVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  void *pvVar5;
  undefined4 uVar6;
  int *local_a8;
  int *local_a4;
  int *local_a0;
  int *local_9c;
  int *local_98;
  int *local_94;
  int *local_90;
  int *local_8c;
  int *local_1c;
  uint local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0f71;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  fVar1 = (float)(uint)(iVar2 - param_1[0x20]) / (float)param_1[0x21];
  if (_DAT_005a45d4 <= fVar1) {
    if ((_DAT_005a34a0 < fVar1) && (param_1[7] != 0)) {
      param_1[7] = 0;
    }
    if (_DAT_005a3660 < fVar1) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
    else {
      local_18 = 100;
      if (param_1[0x26] == 1) {
        local_18 = 300;
      }
      if ((param_1[0x26] == 2) || (param_1[0x26] == 5)) {
        local_18 = 0xb4;
      }
      if ((param_1[0x26] == 3) || (param_1[0x26] == 6)) {
        local_18 = 10000;
      }
      if (param_1[0x26] == 7) {
        local_18 = 1000;
      }
      if (_DAT_005a3670 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
        if (_DAT_005a4314 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
          if (*(float *)(DAT_005ccf9c + 0x2b040) < _DAT_005a40fc) {
            local_18 = local_18 * 3;
          }
        }
        else {
          local_18 = local_18 * 5;
        }
      }
      else {
        local_18 = local_18 * 6;
      }
      if (local_18 < (uint)(iVar2 - param_1[0x22])) {
        if (param_1[7] != 0) {
          param_1[0x1d] = *(int *)(param_1[7] + 0x28);
          param_1[0x1f] = *(int *)(param_1[7] + 0x2c);
          param_1[0x1e] = *(int *)(param_1[7] + 0x30);
          if (param_1[0x27] != 0) {
            puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
            (**(code **)(*(int *)param_1[0x27] + 0x3c))(*puVar3,puVar3[1]);
          }
        }
        if (DAT_0092e660 == 0) {
          iVar4 = _rand();
          local_1c = (int *)0x0;
          if (param_1[0x26] == 0) {
            pvVar5 = operator_new(0x180);
            local_8 = 0;
            if (pvVar5 == (void *)0x0) {
              local_8c = (int *)0x0;
            }
            else {
              local_8c = (int *)FUN_004c8f70(0x21,1000,0x3f333333,0x3f333333,0x3f333333,0x3a83126f,9
                                             ,0x6f);
            }
            local_8 = 0xffffffff;
            local_1c = local_8c;
          }
          else if (param_1[0x26] == 1) {
            pvVar5 = operator_new(0x180);
            local_8 = 1;
            if (pvVar5 == (void *)0x0) {
              local_90 = (int *)0x0;
            }
            else {
              local_90 = (int *)FUN_004c8f70(0x21,1000,0x3f99999a,0x3f800000,0x3f99999a,0x3b03126f,9
                                             ,0x6f);
            }
            local_8 = 0xffffffff;
            local_1c = local_90;
          }
          else if (param_1[0x26] == 2) {
            pvVar5 = operator_new(0x180);
            local_8 = 2;
            if (pvVar5 == (void *)0x0) {
              local_94 = (int *)0x0;
            }
            else {
              local_94 = (int *)FUN_004c8f70(0x21,1000,0x3ecccccd,0x3ecccccd,0x3ecccccd,0x3a83126f,9
                                             ,0x6f);
            }
            local_8 = 0xffffffff;
            local_1c = local_94;
          }
          else if (param_1[0x26] == 3) {
            pvVar5 = operator_new(0x180);
            local_8 = 3;
            if (pvVar5 == (void *)0x0) {
              local_98 = (int *)0x0;
            }
            else {
              local_98 = (int *)FUN_004c8f70(0x21,1000,0x3f19999a,0x3f19999a,0x3f19999a,0x3a83126f,9
                                             ,0x6f);
            }
            local_8 = 0xffffffff;
            local_1c = local_98;
          }
          else if (param_1[0x26] == 4) {
            pvVar5 = operator_new(0x180);
            local_8 = 4;
            if (pvVar5 == (void *)0x0) {
              local_9c = (int *)0x0;
            }
            else {
              local_9c = (int *)FUN_004c8f70(0x47,1000,0x3f19999a,0x3f19999a,0x3f19999a,0x3a83126f,8
                                             ,0x6f);
            }
            local_8 = 0xffffffff;
            local_1c = local_9c;
          }
          else if (param_1[0x26] == 5) {
            pvVar5 = operator_new(0x180);
            local_8 = 5;
            if (pvVar5 == (void *)0x0) {
              local_a0 = (int *)0x0;
            }
            else {
              local_a0 = (int *)FUN_004c8f70(0x47,1000,0x3ecccccd,0x3ecccccd,0x3ecccccd,0x3a83126f,8
                                             ,0x6f);
            }
            local_8 = 0xffffffff;
            local_1c = local_a0;
          }
          else if (param_1[0x26] == 6) {
            pvVar5 = operator_new(0x180);
            local_8 = 6;
            if (pvVar5 == (void *)0x0) {
              local_a4 = (int *)0x0;
            }
            else {
              local_a4 = (int *)FUN_004c8f70(0x47,1000,0x3ecccccd,0x3ecccccd,0x3ecccccd,0x3a83126f,8
                                             ,0x6f);
            }
            local_8 = 0xffffffff;
            local_1c = local_a4;
          }
          else if (param_1[0x26] == 7) {
            pvVar5 = operator_new(0x180);
            local_8 = 7;
            if (pvVar5 == (void *)0x0) {
              local_a8 = (int *)0x0;
            }
            else {
              local_a8 = (int *)FUN_004c8f70(2,1000,0x3f4ccccd,0x3f4ccccd,0x3f4ccccd,0x3a83126f,1,1)
              ;
            }
            local_8 = 0xffffffff;
            local_1c = local_a8;
          }
          if (local_1c == (int *)0x0) {
            ExceptionList = local_10;
            return 0;
          }
          local_1c[0x1b] = 1;
          uVar6 = FUN_00430f50((float)(iVar4 % 5) * _DAT_005a45d4 + (float)param_1[0x1d],
                               param_1[0x1e],
                               (float)(iVar4 % 5) * _DAT_005a45d4 + (float)param_1[0x1f]);
          uVar6 = FUN_004310a0(uVar6);
          FUN_004310a0(uVar6);
          local_1c[0x52] = 1;
          if (param_1[0x26] == 0) {
            local_1c[0x50] = 0x40400000;
          }
          else if (param_1[0x26] == 1) {
            local_1c[0x50] = 0x40800000;
          }
          else if (param_1[0x26] == 2) {
            local_1c[0x50] = 0x40800000;
          }
          else if (param_1[0x26] == 3) {
            local_1c[0x50] = 0x3fd9999a;
          }
          else if (param_1[0x26] == 4) {
            local_1c[0x50] = 0x3fd9999a;
            (**(code **)(*local_1c + 0x3c))(0x3366);
          }
          else if (param_1[0x26] == 5) {
            local_1c[0x50] = 0x40800000;
            (**(code **)(*local_1c + 0x3c))(0x6688);
          }
          else if (param_1[0x26] == 6) {
            local_1c[0x50] = 0x3fd9999a;
            (**(code **)(*local_1c + 0x3c))(0x3366);
          }
          else if (param_1[0x26] == 7) {
            local_1c[0x50] = 0x3fd9999a;
            (**(code **)(*local_1c + 0x3c))(0xffffff);
          }
          (**(code **)(*local_1c + 0x3c))(param_1[0x23]);
          FUN_0054ac09(local_1c);
        }
        param_1[0x22] = iVar2;
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

