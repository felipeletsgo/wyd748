// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e1d57 | Name: FUN_004e1d57


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e1d57(int *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  void *pvVar4;
  float10 fVar5;
  undefined1 *puVar6;
  int iVar7;
  int *local_c8;
  undefined1 local_8c [12];
  undefined1 local_80 [12];
  undefined1 local_74 [12];
  undefined1 local_68 [12];
  undefined1 local_5c [12];
  int *local_50;
  int local_4c;
  int local_48;
  undefined4 local_44 [2];
  undefined4 local_3c;
  uint local_38;
  int local_34;
  undefined1 local_30 [16];
  uint local_20;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0e91;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  local_48 = FUN_004b14fd(param_1[0x28],1,180000);
  if (local_48 == 0) {
    uVar2 = 0;
  }
  else {
    (**(code **)(**(int **)(local_48 + 0x318) + 0x34))(*(undefined4 *)(local_48 + 0x318),local_30);
    (**(code **)(**(int **)(local_48 + 0x318) + 0x2c))
              (*(undefined4 *)(local_48 + 0x318),0,0,&local_18,0);
    local_38 = local_20 / 0x18;
    local_14 = 0xffaaaaaa;
    if (param_1[0x27] == 1) {
      local_14 = 0xffff0000;
    }
    for (local_34 = 0; local_34 < (int)local_38; local_34 = local_34 + 1) {
      *(undefined4 *)(local_18 + 0xc + local_34 * 0x18) = local_14;
    }
    (**(code **)(**(int **)(local_48 + 0x318) + 0x30))(*(undefined4 *)(local_48 + 0x318));
    *(undefined4 *)(local_48 + 0x298) = 0x5b;
    (**(code **)(*param_1 + 0x38))();
    param_1[0x2b] = (int)((float)(uVar1 - param_1[0x20]) / (float)param_1[0x2c]);
    if ((_DAT_005a34a0 < (float)param_1[0x2b]) && (param_1[7] != 0)) {
      param_1[7] = 0;
    }
    if (param_1[7] != 0) {
      param_1[0x24] = *(int *)(param_1[7] + 0x28);
      param_1[0x26] = *(int *)(param_1[7] + 0x2c);
      param_1[0x25] = (int)(*(float *)(param_1[7] + 0x30) + _DAT_005a3660);
    }
    FUN_00493db0(local_44,param_1 + 0x21);
    fVar5 = (float10)FUN_00493c30(local_44[0],local_3c);
    param_1[0x29] = (int)(float)(fVar5 - (float10)_DAT_005a4380);
    if (param_1[0x27] == 2) {
      param_1[0x2a] = (int)(((float)(uVar1 % 1000) * _DAT_005a4290) / _DAT_005a43c0);
    }
    if (_DAT_005a3660 <= (float)param_1[0x2b]) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
    else {
      iVar7 = param_1[0x2b];
      puVar6 = local_80;
      uVar2 = FUN_004c8c60(local_68,0x40400000);
      FUN_00493d50(local_74,uVar2);
      uVar2 = FUN_004c8c60(puVar6,iVar7);
      puVar6 = local_8c;
      FUN_004c8c60(local_5c,_DAT_005a3660 - (float)param_1[0x2b]);
      uVar2 = FUN_00493d50(puVar6,uVar2);
      FUN_004310a0(uVar2);
      if ((param_1[0x27] == 2) || (param_1[0x27] == 3)) {
        FUN_004310a0(param_1 + 0x1d);
        FUN_004310a0(param_1 + 0x1d);
        FUN_004310a0(param_1 + 0x1d);
      }
      puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
      (**(code **)(*(int *)param_1[0x2e] + 0x3c))(*puVar3,puVar3[1]);
      if ((param_1[0x27] != 3) && (100 < uVar1 - param_1[0x2d])) {
        local_4c = _rand();
        local_4c = local_4c % 5;
        pvVar4 = operator_new(0x180);
        local_8 = 0;
        if (pvVar4 == (void *)0x0) {
          local_c8 = (int *)0x0;
        }
        else {
          local_c8 = (int *)FUN_004c8f70(0,1000,(float)local_4c * _DAT_005a4158 + _DAT_005a40ec,
                                         (float)local_4c * _DAT_005a4158 + _DAT_005a40ec,
                                         (float)local_4c * _DAT_005a4158 + _DAT_005a40ec,0x3a83126f,
                                         1,0x50);
        }
        local_8 = 0xffffffff;
        local_50 = local_c8;
        if (local_c8 != (int *)0x0) {
          uVar2 = FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] - _DAT_005a34a0,param_1[0x1f]);
          FUN_004310a0(uVar2);
          local_50[0x1b] = 1;
          local_50[0x40] = 1;
          if (param_1[0x27] == 0) {
            (**(code **)(*local_50 + 0x3c))(0xffaaffee);
          }
          else if (param_1[0x27] == 1) {
            (**(code **)(*local_50 + 0x3c))(0xffff9999);
          }
          FUN_0054ac09(local_50);
        }
        param_1[0x2d] = uVar1;
      }
    }
    uVar2 = 1;
  }
  ExceptionList = local_10;
  return uVar2;
}

