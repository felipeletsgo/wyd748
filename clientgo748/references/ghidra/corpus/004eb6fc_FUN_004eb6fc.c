// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004eb6fc | Name: FUN_004eb6fc


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004eb6fc(int *param_1)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  void *pvVar6;
  float10 fVar7;
  undefined1 *puVar8;
  undefined4 uVar9;
  int *local_104;
  int *local_100;
  undefined1 local_88 [12];
  undefined1 local_7c [12];
  undefined1 local_70 [12];
  int *local_64;
  int *local_60;
  int local_5c;
  int local_58;
  int local_54;
  int local_50;
  undefined4 local_4c [2];
  undefined4 local_44;
  uint local_40;
  int local_3c;
  undefined1 local_38 [16];
  uint local_28;
  int local_20;
  undefined4 local_1c [2];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1420;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  uVar1 = _DAT_0092e670;
  if (_DAT_005a3534 <= (float)uVar2 - (float)param_1[0x20]) {
    if (param_1[7] != 0) {
      param_1[0x24] = *(int *)(param_1[7] + 0x28);
      param_1[0x26] = *(int *)(param_1[7] + 0x2c);
      param_1[0x25] = (int)(*(float *)(param_1[7] + 0x30) + _DAT_005a3660);
    }
    FUN_00493db0(local_1c,param_1 + 0x21);
    fVar7 = (float10)FUN_00493c30(local_1c[0],local_14);
    param_1[0x28] = (int)(float)(fVar7 - (float10)_DAT_005a4380);
    (**(code **)(*param_1 + 0x38))();
    if (param_1[0x27] == 1) {
      local_50 = FUN_004b14fd(0x2c4,1,180000);
      if (local_50 == 0) {
        ExceptionList = local_10;
        return 0;
      }
      (**(code **)(**(int **)(local_50 + 0x318) + 0x34))(*(undefined4 *)(local_50 + 0x318),local_38)
      ;
      (**(code **)(**(int **)(local_50 + 0x318) + 0x2c))
                (*(undefined4 *)(local_50 + 0x318),0,0,&local_20,0);
      local_40 = local_28 / 0x18;
      for (local_3c = 0; local_3c < (int)local_40; local_3c = local_3c + 1) {
        *(undefined4 *)(local_20 + 0xc + local_3c * 0x18) = 0xffffffff;
      }
      (**(code **)(**(int **)(local_50 + 0x318) + 0x30))(*(undefined4 *)(local_50 + 0x318));
      *(undefined4 *)(local_50 + 0x298) = 0x13;
      FUN_00493db0(local_4c,param_1 + 0x21);
      fVar7 = (float10)FUN_00493c30(local_4c[0],local_44);
      param_1[0x28] = (int)(float)(fVar7 - (float10)_DAT_005a4380);
    }
    param_1[0x29] = (int)((float)(uVar2 - param_1[0x20]) / (float)param_1[0x2a]);
    if (_DAT_005a3660 <= (float)param_1[0x29]) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
      uVar1 = _DAT_0092e670;
    }
    else {
      uVar3 = FUN_004c8c60(local_7c,param_1[0x29]);
      puVar8 = local_88;
      FUN_004c8c60(local_70,_DAT_005a3660 - (float)param_1[0x29]);
      uVar3 = FUN_00493d50(puVar8,uVar3);
      FUN_004310a0(uVar3);
      if (param_1[0x27] == 0) {
        if (param_1[0x2c] != 0) {
          FUN_004310a0(param_1 + 0x1d);
        }
        if (param_1[0x2d] != 0) {
          FUN_004310a0(param_1 + 0x1d);
        }
        if (param_1[0x2e] != 0) {
          FUN_004310a0(param_1 + 0x1d);
        }
        if (param_1[0x2b] != 0) {
          puVar4 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
          (**(code **)(*(int *)param_1[0x2b] + 0x3c))(*puVar4,puVar4[1]);
        }
      }
      else if (((param_1[0x27] != 1) || (param_1[0x2f] != 0)) ||
              ((float)param_1[0x29] <= _DAT_005a4214)) {
        if (param_1[0x27] == 4) {
          if (param_1[0x2c] != 0) {
            FUN_004310a0(param_1 + 0x1d);
          }
          if (param_1[0x2d] != 0) {
            FUN_004310a0(param_1 + 0x1d);
          }
          if (param_1[0x2e] != 0) {
            FUN_004310a0(param_1 + 0x1d);
          }
          if (param_1[0x2b] != 0) {
            puVar4 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
            (**(code **)(*(int *)param_1[0x2b] + 0x3c))(*puVar4,puVar4[1]);
          }
        }
        else if (param_1[0x27] == 6) {
          if (param_1[0x2c] != 0) {
            FUN_004310a0(param_1 + 0x1d);
          }
          if (param_1[0x2d] != 0) {
            FUN_004310a0(param_1 + 0x1d);
          }
          if (param_1[0x2e] != 0) {
            FUN_004310a0(param_1 + 0x1d);
          }
          if (param_1[0x2b] != 0) {
            puVar4 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
            (**(code **)(*(int *)param_1[0x2b] + 0x3c))(*puVar4,puVar4[1]);
          }
        }
      }
      else {
        param_1[0x2f] = 1;
        if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(0xa1), iVar5 != 0)) {
          uVar9 = 0;
          uVar3 = 0;
          FUN_00429a6d(0xa1);
          FUN_0042ad2b(uVar3,uVar9);
        }
      }
      if (param_1[0x27] == 2) {
        fVar7 = (float10)FUN_00423b00((float)param_1[0x29] * _DAT_005a4148);
        param_1[0x1e] = (int)(float)(fVar7 * (float10)_DAT_005a430c + (float10)(float)param_1[0x1e])
        ;
      }
      else if (param_1[0x27] == 3) {
        param_1[0x28] = (int)(((float)(uVar2 % 0xfa) * _DAT_005a4148) / _DAT_005a5134);
      }
      else if (param_1[0x27] == 6) {
        param_1[0x28] = (int)(((float)(uVar2 % 0xfa) * _DAT_005a4148) / _DAT_005a4ef8);
        fVar7 = (float10)FUN_00423b00((float)param_1[0x29] * _DAT_005a4148 * _DAT_005a3668);
        param_1[0x1d] = (int)(float)(fVar7 + (float10)(float)param_1[0x1d]);
        fVar7 = (float10)FUN_00423b00((float)param_1[0x29] * _DAT_005a4148 * _DAT_005a3668);
        param_1[0x1f] = (int)(float)(fVar7 + (float10)(float)param_1[0x1f]);
      }
      local_54 = 4;
      if (uVar2 - _DAT_0092e670 < 0x14) {
        local_54 = 1;
      }
      else if (uVar2 - _DAT_0092e670 < 0x1e) {
        local_54 = 2;
      }
      uVar1 = uVar2;
      if (DAT_0092e660 == 0) {
        for (local_58 = 0; local_58 < local_54; local_58 = local_58 + 1) {
          local_5c = _rand();
          local_5c = local_5c % 5;
          pvVar6 = operator_new(0x180);
          local_8 = 0;
          if (pvVar6 == (void *)0x0) {
            local_100 = (int *)0x0;
          }
          else {
            local_100 = (int *)FUN_004c8f70(0,1000,(float)local_5c * _DAT_005a4214 + _DAT_005a4154,
                                            (float)local_5c * _DAT_005a4214 + _DAT_005a4154,
                                            (float)local_5c * _DAT_005a4214 + _DAT_005a4154,
                                            0x3a83126f,1,0x50);
          }
          local_8 = 0xffffffff;
          local_60 = local_100;
          if (local_100 != (int *)0x0) {
            uVar3 = FUN_00430f50((float)local_5c * _DAT_005a45d4 + (float)param_1[0x1d],
                                 (float)local_5c * _DAT_005a45d4 +
                                 ((float)param_1[0x1e] - _DAT_005a4160),
                                 (float)local_5c * _DAT_005a45d4 + (float)param_1[0x1f]);
            FUN_004310a0(uVar3);
            local_60[0x1b] = 1;
            local_60[0x40] = 1;
            if (param_1[0x27] == 0) {
              (**(code **)(*local_60 + 0x3c))(0xffff7711);
            }
            else if (param_1[0x27] == 1) {
              (**(code **)(*local_60 + 0x3c))(0xff55eeff);
            }
            else if (param_1[0x27] == 2) {
              (**(code **)(*local_60 + 0x3c))(0xffffee55);
            }
            else if (param_1[0x27] == 3) {
              (**(code **)(*local_60 + 0x3c))(0xff555555);
            }
            else if (param_1[0x27] == 4) {
              (**(code **)(*local_60 + 0x3c))(0xff5555aa);
            }
            else if (param_1[0x27] == 5) {
              (**(code **)(*local_60 + 0x3c))(0xff6677ff);
            }
            FUN_0054ac09(local_60);
            if ((local_58 == 3) && ((param_1[0x27] == 0 || (param_1[0x27] == 2)))) {
              pvVar6 = operator_new(0x180);
              local_8 = 1;
              if (pvVar6 == (void *)0x0) {
                local_104 = (int *)0x0;
              }
              else {
                local_104 = (int *)FUN_004c8f70(0x3b,1000,
                                                (float)local_5c * _DAT_005a4214 + _DAT_005a4154,
                                                (float)local_5c * _DAT_005a4214 + _DAT_005a4154,
                                                (float)local_5c * _DAT_005a4214 + _DAT_005a4154,
                                                0x3a83126f,1,0x50);
              }
              local_8 = 0xffffffff;
              local_64 = local_104;
              if (local_104 != (int *)0x0) {
                uVar3 = FUN_00430f50((float)param_1[0x1d] - _DAT_005a45d4,
                                     (float)param_1[0x1e] - _DAT_005a4160,
                                     (float)param_1[0x1f] - _DAT_005a45d4);
                FUN_004310a0(uVar3);
                local_64[0x1b] = 0;
                if ((*(int *)(DAT_005ccf9c + 0x2a39c) == 1) ||
                   (*(int *)(DAT_005ccf9c + 0x2a3a0) == 1)) {
                  local_64[0x1b] = 1;
                }
                local_64[0x40] = 1;
                (**(code **)(*local_64 + 0x3c))(0xffffffff);
                FUN_0054ac09(local_64);
              }
            }
          }
        }
      }
    }
  }
  _DAT_0092e670 = uVar1;
  ExceptionList = local_10;
  return 1;
}

