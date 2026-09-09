// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f7ea6 | Name: FUN_004f7ea6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004f7ea6(undefined4 *param_1,undefined4 param_2)

{
  float fVar1;
  float fVar2;
  undefined1 uVar3;
  void *pvVar4;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a178d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0053e1d2();
  local_8 = 0;
  FUN_00421910(param_1 + 0xa4,8,0x30,FUN_00430f10);
  CSimpleArray<>();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_00433a40();
  FUN_0052fae0();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a557c;
  param_1[0x1ef] = param_2;
  DAT_0092eaf0 = DAT_0092eaf0 + 1;
  param_1[0x11d] = 0;
  param_1[0x11e] = 0;
  *(undefined2 *)(param_1 + 0x1ec) = 0;
  param_1[0x18] = 0;
  param_1[0x14c] = 0;
  param_1[0x1da] = 0;
  param_1[0x1db] = 0;
  param_1[0x106] = 0;
  param_1[0x107] = 0;
  param_1[0x108] = 0;
  param_1[0x109] = 0;
  param_1[0x10b] = 0;
  param_1[0x10d] = 0;
  param_1[0x10c] = 0;
  param_1[0x14d] = 0;
  param_1[0x14e] = 0;
  param_1[0x14f] = 0;
  param_1[0x157] = 0;
  param_1[0x158] = 0;
  param_1[0x152] = 0;
  param_1[0x150] = 0;
  param_1[0x151] = 0;
  param_1[0x66] = 0;
  param_1[0x10e] = 0;
  param_1[0x145] = 0;
  param_1[0x146] = 0;
  param_1[0x147] = 0;
  param_1[0x148] = 0;
  param_1[0x149] = 0;
  param_1[0x153] = 0;
  *(undefined2 *)(param_1 + 0x1ee) = 0;
  *(undefined1 *)((int)param_1 + 0x795) = 0;
  *(undefined2 *)(param_1 + 0x1a0) = 0;
  param_1[0x1d6] = 1;
  param_1[0x93] = 1;
  *(undefined2 *)(param_1 + 0x94) = 0;
  *(undefined2 *)((int)param_1 + 0x252) = 0;
  *(undefined1 *)((int)param_1 + 0x79b) = 0;
  *(undefined1 *)((int)param_1 + 0x782) = 0;
  *(undefined2 *)(param_1 + 0x68) = 0xffff;
  *(undefined2 *)((int)param_1 + 0x682) = 0;
  param_1[0x96] = 1;
  param_1[0x58] = 0;
  param_1[0x59] = 0;
  *(undefined1 *)(param_1 + 0x1dd) = 0;
  param_1[0x1e9] = 0x3f800000;
  param_1[0x67] = 0;
  *(undefined1 *)(param_1 + 0x1e7) = 0;
  *(undefined1 *)(param_1 + 0x91) = 0;
  *(undefined1 *)((int)param_1 + 0x79d) = 0;
  param_1[0x1e8] = 0;
  _memset((void *)((int)param_1 + 0x1a2),0,0x20);
  _memset((void *)((int)param_1 + 0x1c2),0,0x10);
  *(undefined2 *)((int)param_1 + 0x7aa) = 0;
  *(undefined1 *)(param_1 + 0x1eb) = 0;
  *(undefined1 *)((int)param_1 + 0x7ad) = 0;
  *(undefined2 *)((int)param_1 + 0x7ae) = 0;
  *(undefined2 *)((int)param_1 + 0x192) = 0;
  *(undefined1 *)((int)param_1 + 0x196) = 0x4b;
  *(undefined2 *)(param_1 + 0x65) = 0;
  *(undefined1 *)(param_1 + 0x1ea) = 0;
  *(undefined1 *)((int)param_1 + 0x246) = 0;
  *(undefined1 *)((int)param_1 + 0x249) = 0;
  *(undefined1 *)((int)param_1 + 0x247) = 0;
  *(undefined1 *)((int)param_1 + 0x7b2) = 0;
  param_1[0x8f] = 0;
  *(undefined2 *)(param_1 + 0x194) = 0;
  param_1[399] = 0;
  param_1[0x159] = 0;
  param_1[0x15a] = 0;
  param_1[0x191] = 0;
  param_1[0x193] = 0;
  *(undefined1 *)((int)param_1 + 0x7a9) = 1;
  param_1[0x179] = 0xbbffffff;
  _memset((void *)((int)param_1 + 0x652),0,0x1a);
  _memset(param_1 + 0x1c9,0,0x18);
  param_1[0x154] = 0;
  for (local_14 = 0; local_14 < 2; local_14 = local_14 + 1) {
    param_1[local_14 + 0x155] = 0;
  }
  for (local_14 = 0; local_14 < 7; local_14 = local_14 + 1) {
    param_1[local_14 + 0x15b] = 0;
    param_1[local_14 + 0x162] = 0;
  }
  for (local_14 = 0; local_14 < 7; local_14 = local_14 + 1) {
    param_1[local_14 + 0x169] = 0;
  }
  for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
    param_1[local_14 + 0x170] = 0;
  }
  for (local_14 = 0; local_14 < 5; local_14 = local_14 + 1) {
    param_1[local_14 + 0x174] = 0;
  }
  param_1[0x14a] = 0;
  param_1[0x14b] = 0;
  FUN_004f9334();
  param_1[0x106] = 0;
  param_1[0x107] = 0;
  param_1[0x109] = 0;
  param_1[0x108] = 0;
  param_1[0x10b] = 0;
  param_1[0x1f9] = 0;
  param_1[0x10d] = 0;
  param_1[0x10e] = 0;
  param_1[0x10a] = 0;
  param_1[0x1f9] = 0;
  param_1[0x10f] = 0;
  param_1[0x110] = 0;
  if (DAT_0067cf38 != 0) {
    pvVar4 = operator_new(0xe4c);
    local_8._0_1_ = 1;
    if (pvVar4 == (void *)0x0) {
      local_88 = 0;
    }
    else {
      local_88 = FUN_00401d03(0xffffffff,s__NoName_005bb707 + 1,0xffffffaa,0,0x44228000,0x43000000,
                              0x41800000,0,0x55aa0000,1,0);
    }
    local_8._0_1_ = 0;
    uVar3 = (undefined1)local_8;
    local_8._0_1_ = 0;
    param_1[0x106] = local_88;
    if (DAT_005b892c == 2) {
      pvVar4 = operator_new(0xe4c);
      local_8._0_1_ = 2;
      if (pvVar4 == (void *)0x0) {
        local_8c = 0;
      }
      else {
        local_8c = FUN_00401d03(0xffffffff,&DAT_0092eca0,0xffffffff,0,0x44228000,0x430f0000,
                                0x42480000,0,0xffffffff,1,0);
      }
      local_8._0_1_ = 0;
      param_1[0x109] = local_8c;
      pvVar4 = operator_new(0x1e4);
      local_8._0_1_ = 3;
      if (pvVar4 == (void *)0x0) {
        local_90 = 0;
      }
      else {
        local_90 = FUN_00401466(0x1be,0xc1200000,0x441ec000,0x430f0000,0x42480000,0x77777777,4);
      }
      param_1[0x108] = local_90;
      *(undefined4 *)(param_1[0x108] + 0x3c) = 0;
    }
    else {
      local_8._0_1_ = uVar3;
      pvVar4 = operator_new(0xe4c);
      local_8._0_1_ = 4;
      if (pvVar4 == (void *)0x0) {
        local_94 = 0;
      }
      else {
        local_94 = FUN_00401d03(9,&DAT_0092eca4,0xffffffff,0,0x44228000,0x430c0000,0x42200000,1,
                                0xffffffff,1,0);
      }
      param_1[0x109] = local_94;
    }
    local_8._0_1_ = 0;
    pvVar4 = operator_new(0xe4c);
    local_8._0_1_ = 5;
    if (pvVar4 == (void *)0x0) {
      local_98 = 0;
    }
    else {
      local_98 = FUN_00401d03(0xfffffffe,&DAT_0092eca8,0xffffffff,0,0x44228000,0x43800000,0x42800000
                              ,1,0x77000000,1,0);
    }
    local_8._0_1_ = 0;
    param_1[0x10b] = local_98;
    *(undefined4 *)(param_1[0x10b] + 0x70) = 1;
    pvVar4 = operator_new(0x1e4);
    local_8._0_1_ = 6;
    if (pvVar4 == (void *)0x0) {
      local_9c = 0;
    }
    else {
      local_9c = FUN_00401466(0xfffffffe,0,0,0x41200000,0x41200000,0x77777777,4);
    }
    local_8._0_1_ = 0;
    param_1[0x1f9] = local_9c;
    pvVar4 = operator_new(0x374);
    local_8._0_1_ = 7;
    if (pvVar4 == (void *)0x0) {
      local_a0 = 0;
    }
    else {
      local_a0 = FUN_00405065(0xfffffffe,0x1e,0x1e,0,0,0x42900000,0x40e00000,0xff9ef048,0xff333333,1
                             );
    }
    local_8._0_1_ = 0;
    param_1[0x10d] = local_a0;
    pvVar4 = operator_new(0x374);
    local_8._0_1_ = 8;
    if (pvVar4 == (void *)0x0) {
      local_a4 = 0;
    }
    else {
      local_a4 = FUN_00405065(0xfffffffe,0x1e,0x1e,0,0,0x42900000,0x40e00000,0xffffaa00,0xff333333,1
                             );
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x10e] = local_a4;
    (**(code **)(*(int *)param_1[0x10b] + 0x60))(0);
    *(undefined4 *)(param_1[0x106] + 0xd1c) = 0xfffffffe;
    (**(code **)(*(int *)param_1[0x106] + 0x60))(0);
    if (param_1[0x1f9] != 0) {
      (**(code **)(*(int *)param_1[0x1f9] + 0x60))(0);
    }
    (**(code **)(*(int *)param_1[0x109] + 0x60))(0);
    pvVar4 = operator_new(0xe4c);
    local_8._0_1_ = 9;
    if (pvVar4 == (void *)0x0) {
      local_a8 = 0;
    }
    else {
      local_a8 = FUN_00401d03(0xffffffff,&DAT_0092ecac,0xffffffaa,0,0x44228000,0x43000000,0x41800000
                              ,0,0x55aa0000,1,0);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x10a] = local_a8;
    *(undefined4 *)(param_1[0x10a] + 0xd1c) = 0xfffffffe;
    (**(code **)(*(int *)param_1[0x10a] + 0x60))(0);
    if (DAT_005b892c == 2) {
      (**(code **)(*(int *)param_1[0x108] + 0x60))(0);
    }
    (**(code **)(*(int *)param_1[0x10d] + 0x60))(0);
    (**(code **)(*(int *)param_1[0x10e] + 0x60))(0);
    fVar1 = _DAT_005a43c4 * DAT_005b490c;
    fVar2 = _DAT_005a5578 * DAT_005b4910;
    pvVar4 = operator_new(0x374);
    local_8._0_1_ = 10;
    if (pvVar4 == (void *)0x0) {
      local_ac = 0;
    }
    else {
      local_ac = FUN_00405065(0xfffffffe,0x1e,0x1e,fVar1,fVar2,0x43480000,0x41900000,0x80800000,
                              0x40333333,1);
    }
    local_8._0_1_ = 0;
    param_1[0x10f] = local_ac;
    (**(code **)(*(int *)param_1[0x10f] + 0x60))(0);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x10f]);
    pvVar4 = operator_new(0xe4c);
    local_8._0_1_ = 0xb;
    if (pvVar4 == (void *)0x0) {
      local_b0 = 0;
    }
    else {
      local_b0 = FUN_00401d03(0xffffffff,s_NoName_005bb710,0xffffffaa,0,0,0x43480000,0x41800000,0,
                              0x55aa0000,1,1);
    }
    local_8._0_1_ = 0;
    param_1[0x110] = local_b0;
    (**(code **)(*(int *)param_1[0x110] + 0x60))(0);
    FUN_0054ac09(param_1[0x110]);
    pvVar4 = operator_new(0xe4c);
    local_8._0_1_ = 0xc;
    if (pvVar4 == (void *)0x0) {
      local_b4 = 0;
    }
    else {
      local_b4 = FUN_00401d03(0xffffffff,&DAT_0092ecb0,0xffffffaa,0x430c0000,0,0x43000000,0x41800000
                              ,0,0x55aa0000,1,0);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x107] = local_b4;
    (**(code **)(*(int *)param_1[0x107] + 0x60))(0);
    FUN_0054ac09(param_1[0x107]);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x106]);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x1f9]);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x109]);
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x108]);
    }
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x10a]);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x10b]);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x10d]);
    (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x44))(param_1[0x10e]);
  }
  param_1[0x1f1] = 0;
  param_1[0x85] = 0;
  param_1[0x86] = 0;
  param_1[0x87] = 0;
  param_1[0x1dc] = 0;
  *(undefined1 *)(param_1 + 499) = 0;
  param_1[500] = 0;
  param_1[0x1f5] = 0;
  *(undefined1 *)((int)param_1 + 0x79a) = 0;
  param_1[0x1f6] = 0;
  param_1[0x1f7] = 0;
  *(undefined2 *)(param_1 + 0x1f8) = 0;
  *(undefined1 *)((int)param_1 + 0x7e2) = 0;
  param_1[0x1fa] = 0;
  param_1[0x1fb] = 0;
  param_1[0x1fc] = 0xffffffff;
  _memset(param_1 + 0x1fd,0,0x80);
  param_1[0x21d] = 0;
  param_1[0x21e] = 0;
  param_1[0x21f] = 0;
  param_1[0x221] = 0;
  param_1[0x222] = 0;
  param_1[0x223] = 0;
  param_1[0x224] = 0;
  ExceptionList = local_10;
  return param_1;
}

