// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040a189 | Name: FUN_0040a189


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0040a189(undefined4 *param_1,int param_2,undefined4 param_3,undefined4 param_4,float param_5,
            float param_6,float param_7)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059edc3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(0xfffffffe,param_3,param_4,0x433a0000,0x43340000,&DAT_01010101,4);
  local_8 = 0;
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a3ea4;
  uVar1 = FUN_00430f20(param_5,param_6);
  FUN_0040bd30(uVar1);
  uVar1 = __ftol();
  param_1[0xeb] = uVar1;
  *(undefined1 *)((int)param_1 + 0x369) = 0;
  param_1[0xde] = 0;
  param_1[0xdf] = 0;
  param_1[0xe0] = 0;
  param_1[0xdb] = 0;
  param_1[0xdc] = 0;
  param_1[0xdd] = 0;
  param_1[0xe3] = 0;
  param_1[0xe4] = 0;
  param_1[0xe8] = 0;
  param_1[0xe7] = 0;
  param_1[0xe6] = 0;
  param_1[0xe5] = param_1[0xe4];
  param_1[0xec] = 0;
  for (local_14 = 0; local_14 < 3; local_14 = local_14 + 1) {
    for (local_18 = 0; local_18 < 3; local_18 = local_18 + 1) {
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 1;
      if (pvVar2 == (void *)0x0) {
        local_68 = 0;
      }
      else {
        local_68 = FUN_00401466(0x15f,(param_5 + param_7) * (float)local_14,
                                (float)local_18 * param_6,param_5,param_6,0xffffffff,4);
      }
      local_8._0_1_ = 0;
      param_1[local_18 * 3 + local_14 + 0xc4] = local_68;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 2;
      if (pvVar2 == (void *)0x0) {
        local_6c = 0;
      }
      else {
        local_6c = FUN_00401466(0x15e,(param_5 + param_7) * (float)local_14,
                                (float)local_18 * param_6,param_5,param_6,0xffffffff,4);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[local_18 * 3 + local_14 + 0xbb] = local_6c;
      if (param_1[local_18 * 3 + local_14 + 0xc4] != 0) {
        FUN_0054ac09(param_1[local_18 * 3 + local_14 + 0xc4]);
      }
      if (param_1[local_18 * 3 + local_14 + 0xbb] != 0) {
        FUN_0054ac09(param_1[local_18 * 3 + local_14 + 0xbb]);
      }
      (**(code **)(*(int *)param_1[local_18 * 3 + local_14 + 0xbb] + 0x60))(0);
      (**(code **)(*(int *)param_1[local_18 * 3 + local_14 + 0xc4] + 0x60))(0);
      *(undefined1 *)((int)param_1 + local_18 * 3 + local_14 + 0x360) = 0;
    }
  }
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 3;
    if (pvVar2 == (void *)0x0) {
      local_70 = 0;
    }
    else {
      local_70 = FUN_00401466(0x163,(float)local_14 * _DAT_005a3970 + _DAT_005a3ea0,0xc2440000,
                              0x41600000,0x41c80000,0xffffffff,4);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[local_14 + 0xcd] = local_70;
    FUN_0054ac09(param_1[local_14 + 0xcd]);
  }
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 4;
    if (pvVar2 == (void *)0x0) {
      local_74 = 0;
    }
    else {
      local_74 = FUN_00401466(0x164,0x41f00000,0xc2480000,0x43200000,0x41e00000,0xffffffff,4);
    }
    param_1[0xd7] = local_74;
  }
  else {
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 5;
    if (pvVar2 == (void *)0x0) {
      local_78 = 0;
    }
    else {
      local_78 = FUN_00401466(0x164,0x41f00000,0xc2440000,0x43200000,0x41d80000,0xffffffff,4);
    }
    param_1[0xd7] = local_78;
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (param_1[0xd7] != 0) {
    FUN_0054ac09(param_1[0xd7]);
  }
  for (local_14 = 0; local_14 < 0x16; local_14 = local_14 + 1) {
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 6;
    if (pvVar2 == (void *)0x0) {
      local_7c = 0;
    }
    else {
      local_7c = FUN_00401466(param_2 + (char)(&DAT_005c3804)[local_14],0,(float)local_14 * param_6,
                              param_5,param_6,0xffffffff,4);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[local_14 + 0x79] = local_7c;
    FUN_0054ac09(param_1[local_14 + 0x79]);
  }
  for (local_14 = 0; local_14 < 0x16; local_14 = local_14 + 1) {
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 7;
    if (pvVar2 == (void *)0x0) {
      local_80 = 0;
    }
    else {
      local_80 = FUN_00401466(param_2 + (char)(&DAT_005c381c)[local_14],param_5 + param_7,
                              (float)local_14 * param_6,param_5,param_6,0xffffffff,4);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[local_14 + 0x8f] = local_80;
    FUN_0054ac09(param_1[local_14 + 0x8f]);
  }
  for (local_14 = 0; local_14 < 0x16; local_14 = local_14 + 1) {
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 8;
    if (pvVar2 == (void *)0x0) {
      local_84 = 0;
    }
    else {
      local_84 = FUN_00401466(param_2 + (char)(&DAT_005c3834)[local_14],
                              param_5 + param_7 + param_5 + param_7,(float)local_14 * param_6,
                              param_5,param_6,0xffffffff,4);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[local_14 + 0xa5] = local_84;
    FUN_0054ac09(param_1[local_14 + 0xa5]);
  }
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0xe1] = uVar1;
  param_1[0xe2] = 0;
  FUN_0040a9cc(1);
  ExceptionList = local_10;
  return param_1;
}

