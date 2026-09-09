// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040586f | Name: FUN_0040586f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0040586f(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined4 param_9,undefined4 param_10,int param_11)

{
  float fVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  int local_34;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059eb96;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0040104c(param_4,param_5,param_6,param_7);
  local_8 = 0;
  FUN_0040bad0();
  param_1[0x1a] = param_8;
  param_1[0x1b] = param_2;
  param_1[0x1c] = param_3;
  *param_1 = &PTR_FUN_005a38d8;
  param_1[0x19] = &PTR_FUN_005a38d4;
  if (DAT_005b892c == 2) {
    local_14 = 1.0;
  }
  else {
    local_14 = DAT_005b4910;
  }
  param_1[0x20] = 0x41880000;
  param_1[0x10] = 0xb;
  if (param_1[0x1a] == 0) {
    param_1[0x1f] = param_6;
    local_34 = 0;
    if (param_11 != 0) {
      local_34 = -5;
    }
    if (DAT_005b892c == 2) {
      fVar1 = 10.0 - _DAT_005a365c;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 1;
      if (pvVar2 == (void *)0x0) {
        local_ac = 0;
      }
      else {
        local_ac = FUN_00401466(0x1a7,(float)local_34,0,0x41200000,param_1[0x20],0xaaaa00ff,4);
      }
      local_8._0_1_ = 0;
      param_1[0x22] = local_ac;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 2;
      if (pvVar2 == (void *)0x0) {
        local_b0 = 0;
      }
      else {
        local_b0 = FUN_00401466(0x1a8,(float)local_34,
                                (((float)param_1[0x16] - (float)param_1[0x20]) + _DAT_005a3664) /
                                local_14,0x41200000,param_1[0x20],0xaaaa00ff,4);
      }
      local_8._0_1_ = 0;
      param_1[0x23] = local_b0;
      uVar3 = __ftol();
      param_1[0x1e] = uVar3;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 3;
      if (pvVar2 == (void *)0x0) {
        local_b4 = 0;
      }
      else {
        local_b4 = FUN_00401466(0x1a9,(float)local_34 + _DAT_005a3660,0,fVar1,0x41880000,0xffffffff,
                                4);
      }
      local_8._0_1_ = 0;
      param_1[0x24] = local_b4;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 4;
      if (pvVar2 == (void *)0x0) {
        local_b8 = 0;
      }
      else {
        local_b8 = FUN_00401466(0x1aa,(float)local_34,0,0x41200000,param_7,0xffffffff,4);
      }
      param_1[0x21] = local_b8;
    }
    else {
      uVar3 = param_1[0x1f];
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 5;
      if (pvVar2 == (void *)0x0) {
        local_bc = 0;
      }
      else {
        local_bc = FUN_00401466(4,(float)local_34,0,(float)param_1[0x15] / local_14,param_1[0x20],
                                0xaaaa00ff,4);
      }
      local_8._0_1_ = 0;
      param_1[0x22] = local_bc;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 6;
      if (pvVar2 == (void *)0x0) {
        local_c0 = 0;
      }
      else {
        local_c0 = FUN_00401466(5,(float)local_34,
                                ((float)param_1[0x16] - (float)param_1[0x20]) / local_14,
                                (float)param_1[0x15] / local_14,param_1[0x20],0xaaaa00ff,4);
      }
      local_8._0_1_ = 0;
      param_1[0x23] = local_c0;
      uVar4 = __ftol();
      param_1[0x1e] = uVar4;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 7;
      if (pvVar2 == (void *)0x0) {
        local_c4 = 0;
      }
      else {
        local_c4 = FUN_00401466(0xfffffffe,(float)local_34,0,uVar3,uVar3,0,4);
      }
      local_8._0_1_ = 0;
      param_1[0x24] = local_c4;
      pvVar2 = operator_new(0x1e4);
      local_8._0_1_ = 8;
      if (pvVar2 == (void *)0x0) {
        local_c8 = 0;
      }
      else {
        local_c8 = FUN_00401466(0xfffffffe,(float)local_34,0,param_6,param_7,0x44aaaaaa,4);
      }
      param_1[0x21] = local_c8;
    }
    local_8 = (uint)local_8._1_3_ << 8;
    puVar5 = (undefined4 *)(**(code **)(*(int *)param_1[0x21] + 0x84))();
    *puVar5 = 4;
  }
  else {
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 9;
    if (pvVar2 == (void *)0x0) {
      local_cc = 0;
    }
    else {
      local_cc = FUN_00401466(0xfffffffe,0,0,param_1[0x20],param_1[0x16],0x44444444,4);
    }
    local_8._0_1_ = 0;
    param_1[0x22] = local_cc;
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 10;
    if (pvVar2 == (void *)0x0) {
      local_d0 = 0;
    }
    else {
      local_d0 = FUN_00401466(0xfffffffe,(float)param_1[0x15] - (float)param_1[0x20],0,param_1[0x20]
                              ,param_1[0x16],0x44444444,4);
    }
    local_8._0_1_ = 0;
    param_1[0x23] = local_d0;
    uVar3 = __ftol();
    param_1[0x1e] = uVar3;
    param_1[0x1f] = param_7;
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 0xb;
    if (pvVar2 == (void *)0x0) {
      local_d4 = 0;
    }
    else {
      local_d4 = FUN_00401466(0xfffffffe,0,0,param_1[0x1f],param_1[0x1f],0,4);
    }
    local_8._0_1_ = 0;
    param_1[0x24] = local_d4;
    pvVar2 = operator_new(0x1e4);
    local_8._0_1_ = 0xc;
    if (pvVar2 == (void *)0x0) {
      local_d8 = 0;
    }
    else {
      local_d8 = FUN_00401466(0xfffffffe,0,0,param_1[0x15],param_7,0x77777777,4);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x21] = local_d8;
    if (param_1[0x21] != 0) {
      iVar6 = (**(code **)(*(int *)param_1[0x21] + 0x84))();
      *(undefined4 *)(iVar6 + 0x40) = 0;
    }
  }
  if (param_1[0x22] != 0) {
    (**(code **)(*(int *)param_1[0x22] + 0x44))(0);
    FUN_0054ac09(param_1[0x22]);
  }
  if (param_1[0x23] != 0) {
    (**(code **)(*(int *)param_1[0x23] + 0x44))(1);
    FUN_0054ac09(param_1[0x23]);
  }
  if (param_1[0x24] != 0) {
    (**(code **)(*(int *)param_1[0x24] + 0x44))(2);
    FUN_0054ac09(param_1[0x24]);
  }
  if (param_1[0x21] != 0) {
    (**(code **)(*(int *)param_1[0x21] + 0x44))(3);
    FUN_0054ac09(param_1[0x21]);
  }
  FUN_004062a7();
  ExceptionList = local_10;
  return param_1;
}

