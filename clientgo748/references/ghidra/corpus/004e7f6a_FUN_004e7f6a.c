// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e7f6a | Name: FUN_004e7f6a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e7f6a(int *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined4 *puVar4;
  void *pvVar5;
  int iVar6;
  float10 fVar7;
  undefined1 *puVar8;
  int local_c8;
  int local_c4;
  int local_c0;
  int *local_b8;
  undefined1 local_64 [12];
  undefined1 local_58 [12];
  undefined1 local_4c [12];
  undefined4 local_40;
  int local_3c;
  undefined4 local_38;
  float local_34;
  undefined4 local_30;
  int *local_2c;
  int local_28;
  int local_24;
  float local_20;
  undefined4 local_1c [2];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a126d;
  local_10 = ExceptionList;
  if (param_1[0x28] == 0) {
    uVar2 = 0;
  }
  else {
    ExceptionList = &local_10;
    uVar3 = (**(code **)(*DAT_0092e654 + 8))();
    if (param_1[0x29] == 0) {
      *(int *)(param_1[0x28] + 0xa4) = (int)(((ulonglong)uVar3 % 0xf0) / 0x28) + 0x14;
    }
    else if (param_1[0x29] == 1) {
      *(int *)(param_1[0x28] + 0xa4) = (int)(((ulonglong)uVar3 % 300) / 0x32) + 0x3d;
    }
    else if (param_1[0x29] == 2) {
      *(int *)(param_1[0x28] + 0xa4) = (int)(((ulonglong)uVar3 % 300) / 0x32) + 0x65;
    }
    else if (param_1[0x29] == 3) {
      *(undefined4 *)(param_1[0x28] + 0xa4) = 0x61;
    }
    else if (param_1[0x29] == 4) {
      *(undefined4 *)(param_1[0x28] + 0xa4) = 200;
    }
    else if (param_1[0x29] == 5) {
      *(int *)(param_1[0x28] + 0xa4) = (int)(((ulonglong)uVar3 % 300) / 0x32) + 0x65;
    }
    (**(code **)(*param_1 + 0x38))();
    param_1[0x2c] = (int)((float)(uVar3 - param_1[0x20]) / (float)param_1[0x2d]);
    if ((_DAT_005a40ec < (float)param_1[0x2c]) && (param_1[7] != 0)) {
      param_1[7] = 0;
    }
    if (param_1[7] != 0) {
      param_1[0x24] = *(int *)(param_1[7] + 0x28);
      param_1[0x26] = *(int *)(param_1[7] + 0x2c);
      param_1[0x25] = (int)(*(float *)(param_1[7] + 0x30) + _DAT_005a3660);
    }
    FUN_00493db0(local_1c,param_1 + 0x21);
    fVar7 = (float10)FUN_00493c30(local_1c[0],local_14);
    param_1[0x2a] = (int)(float)(fVar7 - (float10)_DAT_005a4380);
    if (_DAT_005a3660 <= (float)param_1[0x2c]) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
    else {
      uVar2 = FUN_004c8c60(local_58,param_1[0x2c]);
      puVar8 = local_64;
      FUN_004c8c60(local_4c,_DAT_005a3660 - (float)param_1[0x2c]);
      uVar2 = FUN_00493d50(puVar8,uVar2);
      FUN_004310a0(uVar2);
      if (param_1[0x29] == 3) {
        fVar7 = (float10)FUN_00423b00((float)param_1[0x2c] * _DAT_005a3834 * _DAT_005a4148);
        local_20 = (float)fVar7;
        param_1[0x1d] = (int)(local_20 + (float)param_1[0x1d]);
      }
      if (param_1[0x27] != 0) {
        puVar4 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
        (**(code **)(*(int *)param_1[0x27] + 0x3c))(*puVar4,puVar4[1]);
      }
      uVar1 = _DAT_0092e66c;
      if (0x1e < uVar3 - _DAT_0092e66c) {
        if (param_1[0x29] == 0) {
          for (local_24 = 0; local_24 < 3; local_24 = local_24 + 1) {
            local_28 = _rand();
            local_28 = local_28 % 5;
            pvVar5 = operator_new(0x180);
            local_8 = 0;
            if (pvVar5 == (void *)0x0) {
              local_b8 = (int *)0x0;
            }
            else {
              local_b8 = (int *)FUN_004c8f70(0,1000,(float)local_28 * _DAT_005a4158 + _DAT_005a40ec,
                                             (float)local_28 * _DAT_005a4158 + _DAT_005a40ec,
                                             (float)local_28 * _DAT_005a4158 + _DAT_005a40ec,
                                             0x3a83126f,1,0x50);
            }
            local_8 = 0xffffffff;
            local_2c = local_b8;
            if (local_b8 != (int *)0x0) {
              uVar2 = FUN_00430f50((float)local_28 * _DAT_005a4214 + (float)param_1[0x1d],
                                   (float)param_1[0x1e] - _DAT_005a3660,
                                   (float)local_28 * _DAT_005a4214 + (float)param_1[0x1f]);
              FUN_004310a0(uVar2);
              local_2c[0x1b] = 1;
              local_2c[0x40] = 1;
              (**(code **)(*local_2c + 0x3c))(param_1[0x2f]);
              FUN_0054ac09(local_2c);
            }
          }
        }
        else if ((param_1[0x29] != 3) && (param_1[0x29] != 4)) {
          local_40 = DAT_0067cf38;
          FUN_00430f50(param_1[0x1d],0,param_1[0x1f]);
          puVar4 = (undefined4 *)FUN_00430f20(local_38,local_30);
          iVar6 = FUN_0049c70f(*puVar4,puVar4[1]);
          local_34 = (float)iVar6 * _DAT_005a4214 + _DAT_005a34a0;
          local_3c = 0;
          if (param_1[0x29] == 2) {
            pvVar5 = operator_new(0xa0);
            local_8 = 1;
            if (pvVar5 == (void *)0x0) {
              local_c0 = 0;
            }
            else {
              local_c0 = FUN_004e2a30(local_38,local_34,local_30,6,0,0x4488,0x4488);
            }
            local_3c = local_c0;
          }
          else if (param_1[0x29] == 5) {
            pvVar5 = operator_new(0xa0);
            local_8 = 2;
            if (pvVar5 == (void *)0x0) {
              local_c4 = 0;
            }
            else {
              local_c4 = FUN_004e2a30(local_38,local_34,local_30,6,0,0x112211,0x112211);
            }
            local_3c = local_c4;
          }
          else {
            pvVar5 = operator_new(0xa0);
            local_8 = 3;
            if (pvVar5 == (void *)0x0) {
              local_c8 = 0;
            }
            else {
              local_c8 = FUN_004e2a30(local_38,local_34,local_30,3,0,0x44444444,0x22331100);
            }
            local_3c = local_c8;
          }
          local_8 = 0xffffffff;
          if (local_3c != 0) {
            FUN_0054ac09(local_3c);
          }
        }
        uVar1 = uVar3;
        if (param_1[0x28] != 0) {
          *(int *)(param_1[0x28] + 0x8c) = param_1[0x2a];
          FUN_004310a0(param_1 + 0x1d);
          (**(code **)(*(int *)param_1[0x28] + 0x20))(uVar3);
        }
      }
      _DAT_0092e66c = uVar1;
      if (((param_1[0x29] == 3) || (param_1[0x29] == 4)) && (param_1[0x28] != 0)) {
        *(float *)(param_1[0x28] + 0x8c) = (float)param_1[0x2a] + _DAT_005a4148;
        FUN_004310a0(param_1 + 0x1d);
        (**(code **)(*(int *)param_1[0x28] + 0x20))(uVar3);
      }
    }
    uVar2 = 1;
  }
  ExceptionList = local_10;
  return uVar2;
}

