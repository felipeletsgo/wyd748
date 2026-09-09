// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e422d | Name: FUN_004e422d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e422d(int *param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  uint uVar4;
  uint uVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  void *pvVar9;
  int iVar10;
  float fVar11;
  int local_cc;
  int *local_c0;
  int local_4c;
  undefined1 local_48 [16];
  uint local_38;
  uint local_30;
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1007;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar5 = (**(code **)(*DAT_0092e654 + 8))();
  if (_DAT_005a3534 <= (float)uVar5 - (float)param_1[0x20]) {
    local_14 = 0;
    if (param_1[0x27] < 2) {
      local_14 = FUN_004b14fd(param_1[0x2a],1,180000);
    }
    else {
      local_14 = FUN_004b14fd(param_1[0x2a],0,180000);
    }
    if (local_14 == 0) {
      uVar6 = 0;
    }
    else {
      (**(code **)(*param_1 + 0x38))();
      param_1[0x25] = (int)((float)(uVar5 - param_1[0x20]) / (float)param_1[0x21]);
      if (_DAT_005a3660 <= (float)param_1[0x25]) {
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
      else {
        local_20 = DAT_0067cf38;
        FUN_00430f20(param_1[0x1d],param_1[0x1f]);
        local_1c = FUN_0049c70f(local_28,local_24);
        param_1[0x1e] = (int)((float)local_1c * _DAT_005a4214);
        if (param_1[0x28] != 1) {
          if (param_1[0x27] < 2) {
            local_30 = 0x55113366;
            if ((float)_DAT_005a4f00 <= (float)param_1[0x25]) {
              FUN_00493bf0(((float)param_1[0x25] - _DAT_005a414c) * _DAT_005a4efc * _DAT_005a4148);
              iVar8 = __ftol();
              iVar10 = __ftol();
              iVar7 = __ftol();
              local_30 = __ftol();
              local_30 = iVar8 << 0x18 | iVar10 << 0x10 | iVar7 << 8 | local_30;
            }
            (**(code **)(**(int **)(local_14 + 0x318) + 0x34))
                      (*(undefined4 *)(local_14 + 0x318),local_48);
            (**(code **)(**(int **)(local_14 + 0x318) + 0x2c))
                      (*(undefined4 *)(local_14 + 0x318),0,0,&local_2c,0);
            for (local_4c = 0; local_4c < (int)(local_38 / 0x18); local_4c = local_4c + 1) {
              *(uint *)(local_2c + 0xc + local_4c * 0x18) = local_30;
            }
            (**(code **)(**(int **)(local_14 + 0x318) + 0x30))(*(undefined4 *)(local_14 + 0x318));
            *(undefined4 *)(local_14 + 0x298) = 0x13;
          }
          else {
            *(undefined4 *)(local_14 + 0x298) = 0x4ad;
          }
        }
        if ((param_1[0x27] < 2) || (8 < param_1[0x27])) {
          if ((float)param_1[0x25] < _DAT_005a4214) {
            *(float *)(local_14 + 0x330) = (float)param_1[0x25] * _DAT_005a3970 + _DAT_005a4158;
          }
        }
        else {
          if ((float)param_1[0x25] < _DAT_005a4214) {
            *(float *)(local_14 + 0x330) = (float)param_1[0x25] * _DAT_005a4394 + _DAT_005a4214;
          }
          if (_DAT_005a40f4 < (float)param_1[0x25]) {
            *(float *)(local_14 + 0x330) =
                 _DAT_005a4678 - ((float)param_1[0x25] - _DAT_005a40f4) * _DAT_005a4394;
          }
        }
        if (param_1[0x29] == 1) {
          *(float *)(local_14 + 0x330) = (float)param_1[0x25] * _DAT_005a40f0;
          *(float *)(local_14 + 0x32c) = (float)param_1[0x25] + (float)param_1[0x25];
        }
        uVar4 = _DAT_0092e668;
        if (100 < uVar5 - _DAT_0092e668) {
          iVar8 = _rand();
          iVar8 = iVar8 % 5;
          pvVar9 = operator_new(0x180);
          local_8 = 0;
          if (pvVar9 == (void *)0x0) {
            local_c0 = (int *)0x0;
          }
          else {
            local_c0 = (int *)FUN_004c8f70(0,0x5dc,(float)iVar8 * _DAT_005a40ec + _DAT_005a34a0,
                                           (float)iVar8 * _DAT_005a40ec + _DAT_005a34a0,
                                           (float)iVar8 * _DAT_005a40ec + _DAT_005a34a0,0x3a83126f,1
                                           ,0x50);
          }
          local_8 = 0xffffffff;
          uVar4 = uVar5;
          if (local_c0 != (int *)0x0) {
            iVar8 = _rand();
            fVar11 = (float)(iVar8 % 10 + -5) * _DAT_005a4158 + (float)param_1[0x1f];
            iVar8 = param_1[0x1e];
            iVar10 = _rand();
            uVar6 = FUN_00430f50((float)(iVar10 % 10 + -5) * _DAT_005a4158 + (float)param_1[0x1d],
                                 iVar8,fVar11);
            FUN_004310a0(uVar6);
            if ((param_1[0x27] < 2) || (8 < param_1[0x27])) {
              local_c0[0x1b] = 1;
              (**(code **)(*local_c0 + 0x3c))(0xffaaeeff);
            }
            else {
              local_c0[0x1b] = 0;
              (**(code **)(*local_c0 + 0x3c))(0xffaaaaaa);
            }
            local_c0[0x40] = 1;
            FUN_0054ac09(local_c0);
          }
        }
        _DAT_0092e668 = uVar4;
        local_18 = 0x3e4ccccd;
        if ((((param_1[0x2b] == 0) && (1 < param_1[0x27])) && (param_1[0x27] < 8)) &&
           (0.2 < (float)param_1[0x25])) {
          fVar2 = (float)param_1[0x1d];
          iVar8 = param_1[0x1e];
          fVar3 = (float)param_1[0x1f];
          fVar11 = (float)param_1[0x23];
          fVar1 = (float)param_1[0x24];
          pvVar9 = operator_new(0xb0);
          local_8 = 1;
          if (pvVar9 == (void *)0x0) {
            local_cc = 0;
          }
          else {
            local_cc = FUN_004e3ba0(fVar2 + fVar11,iVar8,fVar3 + fVar1,param_1[0x27] + 1,
                                    param_1[0x28],0);
          }
          local_8 = 0xffffffff;
          if (local_cc != 0) {
            FUN_0040bd30(param_1 + 0x23);
            FUN_0054ac09(local_cc);
          }
          param_1[0x2b] = 1;
        }
      }
      uVar6 = 1;
    }
  }
  else {
    uVar6 = 1;
  }
  ExceptionList = local_10;
  return uVar6;
}

