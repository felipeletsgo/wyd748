// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c80d7 | Name: FUN_004c80d7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004c80d7(int *param_1)

{
  float fVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 *puVar5;
  float10 fVar6;
  undefined1 *puVar7;
  int *piVar8;
  undefined1 *puVar9;
  int *local_114;
  int *local_10c;
  undefined1 local_dc [12];
  undefined1 local_d0 [12];
  undefined1 local_c4 [12];
  undefined1 local_b8 [12];
  undefined1 local_ac [24];
  void *local_94;
  int *local_90;
  void *local_80;
  int *local_7c;
  undefined1 local_78 [12];
  undefined1 local_6c [12];
  undefined1 local_60 [12];
  undefined1 local_54 [12];
  undefined1 local_48 [12];
  float local_3c;
  float local_38;
  int *local_34;
  int *local_30;
  int local_2c;
  undefined4 local_28;
  float local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a07d3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  local_20 = FUN_004b14fd(param_1[0x18],0,180000);
  if ((local_20 == 0) && (param_1[0x17] != 10000)) {
    uVar3 = 0;
  }
  else {
    (**(code **)(*param_1 + 0x38))();
    if (((param_1[0x17] == 0x97) ||
        (((param_1[0x17] == 0x98 || (param_1[0x17] == 0x99)) || (param_1[0x17] == 10000)))) ||
       ((param_1[0x17] == 0x2711 || (param_1[0x17] == 0x2712)))) {
      if (uVar2 < (uint)param_1[9]) {
        local_24 = 0.0;
      }
      else {
        local_24 = (float)(uVar2 - param_1[9]) / (float)param_1[0x19];
      }
    }
    else if (uVar2 < (uint)param_1[9]) {
      local_24 = 0.0;
    }
    else {
      local_24 = (float)(uVar2 - param_1[9]) / (float)param_1[0x19];
    }
    if (param_1[0x17] == 0x2712) {
      fVar6 = (float10)FUN_00423b00(local_24 * _DAT_005a3834);
      param_1[0x1f] = (int)(float)(((float10)_DAT_005a430c - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00493bf0(local_24 * _DAT_005a3834);
      param_1[0x20] = (int)(float)(((float10)_DAT_005a430c - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00423b00((local_24 - _DAT_005a34a0) * _DAT_005a3834);
      param_1[0x21] = (int)(float)(((float10)_DAT_005a430c - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00493bf0((local_24 - _DAT_005a34a0) * _DAT_005a3834);
      param_1[0x22] = (int)(float)(((float10)_DAT_005a430c - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00423b00((local_24 - _DAT_005a3660) * _DAT_005a3834);
      param_1[0x23] = (int)(float)(((float10)_DAT_005a430c - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00493bf0((local_24 - _DAT_005a3660) * _DAT_005a3834);
      param_1[0x24] = (int)(float)(((float10)_DAT_005a430c - (float10)local_24) * fVar6);
      if (_DAT_005a3660 < local_24) {
        FUN_004c64c3();
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
      uVar3 = 0;
    }
    else if (param_1[0x17] == 0x2713) {
      fVar6 = (float10)FUN_00423b00(local_24 * _DAT_005a3834);
      param_1[0x1f] = (int)(float)(((float10)_DAT_005a4744 - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00493bf0(local_24 * _DAT_005a3834);
      param_1[0x20] = (int)(float)(((float10)_DAT_005a4744 - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00423b00((local_24 - _DAT_005a34a0) * _DAT_005a3834);
      param_1[0x21] = (int)(float)(((float10)_DAT_005a4744 - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00493bf0((local_24 - _DAT_005a34a0) * _DAT_005a3834);
      param_1[0x22] = (int)(float)(((float10)_DAT_005a4744 - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00423b00((local_24 - _DAT_005a3660) * _DAT_005a3834);
      param_1[0x23] = (int)(float)(((float10)_DAT_005a4744 - (float10)local_24) * fVar6);
      fVar6 = (float10)FUN_00493bf0((local_24 - _DAT_005a3660) * _DAT_005a3834);
      param_1[0x24] = (int)(float)(((float10)_DAT_005a4744 - (float10)local_24) * fVar6);
      if (_DAT_005a3660 < local_24) {
        FUN_004c64c3();
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
      uVar3 = 0;
    }
    else {
      FUN_00493db0(&local_1c,param_1 + 10);
      fVar6 = (float10)FUN_00493c30(local_1c,local_14);
      param_1[0x14] = (int)(float)(fVar6 + (float10)_DAT_005a4380);
      fVar6 = (float10)FUN_00493c30(local_1c,local_18);
      param_1[0x15] = (int)(float)fVar6;
      if (param_1[0x17] == 0x99) {
        fVar1 = ((float)(uVar2 % 300) / _DAT_005a43c4) * _DAT_005a4148;
        param_1[0x14] = (int)(fVar1 + fVar1);
      }
      if (_DAT_005a3660 <= local_24) {
        FUN_004c64c3();
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
      else {
        uVar3 = FUN_004c8c60(local_6c,local_24);
        puVar9 = local_78;
        FUN_004c8c60(local_60,_DAT_005a3660 - local_24);
        uVar3 = FUN_00493d50(puVar9,uVar3);
        FUN_004310a0(uVar3);
        if ((param_1[0x17] == 0x98) && (param_1[0x13] == 2)) {
          fVar6 = (float10)FUN_00423b00(local_24 * _DAT_005a4148 * _DAT_005a3834);
          param_1[0x11] =
               (int)(float)(fVar6 * (float10)_DAT_005a4214 + (float10)(float)param_1[0x11]);
        }
        if (param_1[0x1b] != 0) {
          *(float *)(param_1[0x1b] + 0x8c) = (float)param_1[0x14] + _DAT_005a4148;
          FUN_004310a0(param_1 + 0x10);
          if (param_1[0x17] == 0x98) {
            *(int *)(param_1[0x1b] + 0xa4) = (int)(((ulonglong)uVar2 % 0x140) / 0x50) + 0x65;
          }
          else {
            *(int *)(param_1[0x1b] + 0xa4) = (int)(((ulonglong)uVar2 % 0x140) / 0x50) + 0xb;
          }
          (**(code **)(*(int *)param_1[0x1b] + 0x20))(uVar2);
        }
        local_28 = 0xffaaaaee;
        if ((param_1[0x17] == 0x97) && (param_1[0x13] == 1)) {
          local_28 = 0xffffaa00;
        }
        if ((param_1[0x17] == 0x97) && (param_1[0x13] == 2)) {
          local_28 = 0xff00aa00;
        }
        local_2c = _rand();
        local_2c = local_2c % 5;
        if (param_1[0x17] != 10000) {
          local_80 = operator_new(0x180);
          local_8 = 0;
          if (local_80 == (void *)0x0) {
            local_10c = (int *)0x0;
          }
          else {
            local_10c = (int *)FUN_004c8f70(0,1000,(float)local_2c * _DAT_005a4158 + _DAT_005a4158,
                                            (float)local_2c * _DAT_005a4158 + _DAT_005a4158,
                                            (float)local_2c * _DAT_005a4158 + _DAT_005a4158,
                                            0x3a83126f,1,0x50);
          }
          local_7c = local_10c;
          local_8 = 0xffffffff;
          local_30 = local_10c;
          if (local_10c != (int *)0x0) {
            uVar3 = FUN_00430f50(param_1[0x10],(float)param_1[0x11] - _DAT_005a34a0,param_1[0x12]);
            FUN_004310a0(uVar3);
            local_30[0x1b] = 1;
            local_30[0x40] = 1;
            (**(code **)(*local_30 + 0x3c))(local_28);
            FUN_0054ac09(local_30);
          }
        }
        if ((param_1[0x18] == 0x397) && (_DAT_005a4158 < local_24)) {
          iVar4 = _rand();
          local_3c = (float)(iVar4 % 5);
          local_38 = 0.3;
          local_94 = operator_new(0x180);
          local_8 = 1;
          if (local_94 == (void *)0x0) {
            local_114 = (int *)0x0;
          }
          else {
            local_114 = (int *)FUN_004c8f70(0xb,1000,_DAT_005a4740 * local_3c + local_38,
                                            _DAT_005a4740 * local_3c + local_38,
                                            _DAT_005a4740 * local_3c + local_38,0x3a83126f,1,0x50);
          }
          local_90 = local_114;
          local_8 = 0xffffffff;
          local_34 = local_114;
          if (local_114 != (int *)0x0) {
            (**(code **)(*local_114 + 0x3c))(0xffaa8855);
            local_34[0x1b] = 1;
            local_34[0x41] = 1;
            local_34[0x52] = 1;
            local_34[0x50] = 0x3f800000;
            uVar3 = FUN_00430f50(param_1[0x10],param_1[0x11],param_1[0x12]);
            uVar3 = FUN_004310a0(uVar3);
            FUN_004310a0(uVar3);
            FUN_0054ac09(local_34);
          }
        }
        if (((param_1[0x1a] != 0) && (_DAT_005a4158 < local_24)) &&
           (uVar2 < (uint)(param_1[9] + 2000 + param_1[0x19]))) {
          FUN_00430f50((float)param_1[10] - (float)param_1[0xd],
                       (float)param_1[0xb] - (float)param_1[0xe],
                       (float)param_1[0xc] - (float)param_1[0xf]);
          thunk_FUN_005611dd(local_54,local_54);
          uVar3 = 0x40400000;
          puVar9 = local_48;
          piVar8 = param_1 + 0xd;
          puVar7 = local_b8;
          FUN_004c8c60(local_ac,0x40000000);
          FUN_00493d50(puVar7,piVar8);
          FUN_00493e10(puVar9,uVar3);
          if (param_1[0x17] == 0x2711) {
            FUN_004310a0(param_1 + 10);
          }
          uVar3 = FUN_004c8c60(local_d0,local_24);
          puVar9 = local_dc;
          FUN_004c8c60(local_c4,_DAT_005a3660 - local_24);
          puVar5 = (undefined4 *)FUN_00493d50(puVar9,uVar3);
          FUN_004ccfed(param_1[0x10],param_1[0x11],param_1[0x12],*puVar5,puVar5[1],puVar5[2]);
        }
      }
      uVar3 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar3;
}

