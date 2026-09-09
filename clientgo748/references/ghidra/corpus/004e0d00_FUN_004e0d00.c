// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e0d00 | Name: FUN_004e0d00


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004e0d00(undefined4 *param_1,float param_2,float param_3,float param_4,undefined4 param_5)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  float fVar8;
  float fVar9;
  undefined4 uVar10;
  int *local_88;
  int *local_78;
  int local_68;
  uint local_24;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0dfa;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4d84;
  param_1[0x23] = param_5;
  FUN_004310a0(&param_2);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 3000;
  local_18 = 0x10;
  if (_DAT_005a3670 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
    if (_DAT_005a4314 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
      if (*(float *)(DAT_005ccf9c + 0x2b040) < _DAT_005a40fc) {
        local_18 = 0xc;
      }
    }
    else {
      local_18 = 8;
    }
  }
  else {
    local_18 = 2;
  }
  if (DAT_0092e660 == 0) {
    pvVar2 = operator_new(0x90);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_68 = 0;
    }
    else {
      uVar1 = 1000;
      puVar3 = (undefined4 *)FUN_00430f50(0,0,0);
      local_68 = FUN_004d4de0(param_2,param_3,param_4,0,local_18,0x3dcccccd,0,1,0x38,0x3f800000,1,
                              *puVar3,puVar3[1],puVar3[2],uVar1);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (local_68 != 0) {
      FUN_0054ac09(local_68);
    }
  }
  local_18 = 5;
  if (_DAT_005a3670 <= *(float *)(DAT_005ccf9c + 0x2b040)) {
    if (*(float *)(DAT_005ccf9c + 0x2b040) < _DAT_005a4d80) {
      local_18 = 4;
    }
  }
  else {
    local_18 = 2;
  }
  if (DAT_0092e660 == 0) {
    for (local_24 = 0; (int)local_24 < local_18; local_24 = local_24 + 1) {
      pvVar2 = operator_new(0x180);
      local_8._0_1_ = 2;
      if (pvVar2 == (void *)0x0) {
        local_78 = (int *)0x0;
      }
      else {
        uVar4 = local_24 & 0x80000001;
        if ((int)uVar4 < 0) {
          uVar4 = (uVar4 - 1 | 0xfffffffe) + 1;
        }
        uVar6 = local_24 & 0x80000001;
        if ((int)uVar6 < 0) {
          uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
        }
        uVar7 = local_24 & 0x80000001;
        if ((int)uVar7 < 0) {
          uVar7 = (uVar7 - 1 | 0xfffffffe) + 1;
        }
        local_78 = (int *)FUN_004c8f70(0x38,local_24 * 300 + 2000,
                                       (float)(int)uVar7 * 0.3 + _DAT_005a40ec,
                                       (float)(int)uVar6 * 0.3 + _DAT_005a40ec,
                                       (float)(int)uVar4 * 0.3 + _DAT_005a40ec,0,1,0x50);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_78 != (int *)0x0) {
        local_78[0x4f] = 0x3f333333;
        local_78[0x1b] = 1;
        local_78[0x52] = 3;
        iVar5 = _rand();
        fVar9 = (float)(iVar5 % 5 + -3) * _DAT_005a40ec + param_4;
        iVar5 = _rand();
        fVar8 = (float)(iVar5 % 5 + -3) * _DAT_005a4214 + param_3;
        iVar5 = _rand();
        uVar1 = FUN_00430f50((float)(iVar5 % 5 + -3) * _DAT_005a40ec + param_2,fVar8,fVar9);
        uVar1 = FUN_004310a0(uVar1);
        FUN_004310a0(uVar1);
        (**(code **)(*local_78 + 0x3c))(0xffffffff);
        FUN_0054ac09(local_78);
      }
    }
  }
  pvVar2 = operator_new(0xc0);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_88 = (int *)0x0;
  }
  else {
    local_88 = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (local_88 != (int *)0x0) {
    (**(code **)(*local_88 + 0x44))(0x55555555);
    local_88[0x1b] = 1;
    puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_88 + 0x3c))(*puVar3,puVar3[1]);
    local_88[0x28] = 3000;
    FUN_0054ac09(local_88);
  }
  if (DAT_005ccf98 != 0) {
    iVar5 = FUN_00429a6d(4);
    if (iVar5 != 0) {
      uVar10 = 0;
      uVar1 = 0;
      FUN_00429a6d(4);
      FUN_0042ad2b(uVar1,uVar10);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

