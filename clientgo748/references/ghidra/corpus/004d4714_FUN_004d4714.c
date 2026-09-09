// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d4714 | Name: FUN_004d4714


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d4714(int *param_1)

{
  float fVar1;
  int iVar2;
  void *pvVar3;
  undefined4 uVar4;
  float10 fVar5;
  int *local_78;
  int *local_74;
  int *local_70;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0a1d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  local_14 = (float)((uint)(param_1[0x24] - iVar2) % (uint)param_1[0x25]) / (float)param_1[0x25];
  fVar1 = local_14 * _DAT_005a4148 + local_14 * _DAT_005a4148;
  if ((param_1[0x2c] == 1) && (param_1[7] != 0)) {
    param_1[0x29] = *(int *)(param_1[7] + 0x34);
    param_1[0x2a] = 0;
  }
  else {
    param_1[0x29] = (int)(fVar1 + fVar1);
  }
  if (param_1[0x2c] == 2) {
    local_14 = (float)(uint)(iVar2 - param_1[0x24]) / (float)param_1[0x26];
  }
  if (param_1[7] == 0) {
    if (param_1[0x27] == 0) {
      fVar5 = (float10)FUN_00493bf0(fVar1);
      param_1[0x1d] = (int)(float)(fVar5 * (float10)_DAT_005a3660 + (float10)(float)param_1[0x21]);
      fVar5 = (float10)FUN_00423b00(fVar1);
      param_1[0x1f] = (int)(float)(fVar5 * (float10)_DAT_005a3660 + (float10)(float)param_1[0x23]);
    }
    else {
      param_1[0x1d] = param_1[0x21];
      param_1[0x1f] = param_1[0x23];
    }
    param_1[0x1e] = param_1[0x22];
  }
  else if (param_1[0x27] == 0) {
    fVar5 = (float10)FUN_00493bf0(fVar1);
    param_1[0x1d] =
         (int)(float)(fVar5 * (float10)_DAT_005a3660 + (float10)*(float *)(param_1[7] + 0x28));
    fVar5 = (float10)FUN_00423b00(fVar1);
    param_1[0x1f] =
         (int)(float)(fVar5 * (float10)_DAT_005a3660 + (float10)*(float *)(param_1[7] + 0x2c));
    param_1[0x1e] = (int)(*(float *)(param_1[7] + 0x30) + _DAT_005a3660);
  }
  else {
    param_1[0x1d] = *(int *)(param_1[7] + 0x28);
    param_1[0x1f] = *(int *)(param_1[7] + 0x2c);
    param_1[0x1e] = *(int *)(param_1[7] + 0x30);
  }
  if ((param_1[0x20] != 0) && (FUN_004310a0(param_1 + 0x1d), param_1[0x2c] != 1)) {
    *(float *)(param_1[0x20] + 0x78) = *(float *)(param_1[0x20] + 0x78) + _DAT_005a4158;
  }
  if (param_1[0x28] == 1) {
    fVar5 = (float10)FUN_00423b00(fVar1);
    param_1[0x2e] = (int)(float)(fVar5 * (float10)_DAT_005a4214 + (float10)_DAT_005a4160);
  }
  if (param_1[0x2c] == 1) {
    iVar2 = _rand();
    iVar2 = iVar2 % 5;
    pvVar3 = operator_new(0x180);
    local_8 = 0;
    if (pvVar3 == (void *)0x0) {
      local_70 = (int *)0x0;
    }
    else {
      local_70 = (int *)FUN_004c8f70(0,800,(float)iVar2 * _DAT_005a439c + _DAT_005a41e8,
                                     (float)iVar2 * _DAT_005a4214 + _DAT_005a41e8,
                                     (float)iVar2 * _DAT_005a439c + _DAT_005a41e8,0x3a03126f,1,0x50)
      ;
    }
    local_8 = 0xffffffff;
    if (local_70 != (int *)0x0) {
      uVar4 = FUN_004310a0(param_1 + 0x1d);
      FUN_004310a0(uVar4);
      local_70[0x1b] = 1;
      local_70[0x40] = 0;
      local_70[0x52] = 1;
      local_70[0x50] = 0;
      (**(code **)(*local_70 + 0x3c))(0xff6688aa);
      FUN_0054ac09(local_70);
    }
  }
  else if (param_1[0x2c] == 2) {
    if (_DAT_005a3660 < local_14) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
    else {
      iVar2 = _rand();
      iVar2 = iVar2 % 0x14 + 5;
      pvVar3 = operator_new(0x180);
      local_8 = 1;
      if (pvVar3 == (void *)0x0) {
        local_74 = (int *)0x0;
      }
      else {
        local_74 = (int *)FUN_004c8f70(0,900,(float)iVar2 * _DAT_005a4158,
                                       (float)iVar2 * _DAT_005a4158,(float)iVar2 * _DAT_005a4158,
                                       0x3a03126f,1,0x50);
      }
      local_8 = 0xffffffff;
      if (local_74 != (int *)0x0) {
        uVar4 = FUN_004310a0(param_1 + 0x1d);
        FUN_004310a0(uVar4);
        local_74[0x1b] = 1;
        local_74[0x40] = 0;
        local_74[0x52] = 3;
        local_74[0x50] = 0x40000000;
        (**(code **)(*local_74 + 0x3c))(0xff6677cc);
        FUN_0054ac09(local_74);
      }
    }
  }
  else if (param_1[0x2c] == 3) {
    if (_DAT_005a3660 < local_14) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
    else {
      iVar2 = _rand();
      iVar2 = iVar2 % 0x14 + 3;
      pvVar3 = operator_new(0x180);
      local_8 = 2;
      if (pvVar3 == (void *)0x0) {
        local_78 = (int *)0x0;
      }
      else {
        local_78 = (int *)FUN_004c8f70(0,0x5dc,(float)iVar2 * _DAT_005a4158,
                                       (float)iVar2 * _DAT_005a4158,(float)iVar2 * _DAT_005a4158,
                                       0x3727c5ac,1,0x50);
      }
      local_8 = 0xffffffff;
      if (local_78 != (int *)0x0) {
        param_1[0x1e] = (int)((float)param_1[0x1e] - _DAT_005a415c);
        uVar4 = FUN_004310a0(param_1 + 0x1d);
        FUN_004310a0(uVar4);
        local_78[0x1b] = 1;
        local_78[0x40] = 0;
        local_78[0x52] = 3;
        local_78[0x4f] = 0x3f800000;
        local_78[0x50] = 0x40a00000;
        (**(code **)(*local_78 + 0x3c))(0xffff0000);
        FUN_0054ac09(local_78);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

