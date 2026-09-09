// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00493e70 | Name: FUN_00493e70


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00493e70(undefined4 *param_1)

{
  undefined1 uVar1;
  void *pvVar2;
  int local_11c;
  undefined4 local_118;
  undefined4 local_10c;
  int local_100;
  int local_fc;
  undefined4 local_f8;
  undefined4 local_ec;
  undefined4 local_e0;
  undefined4 local_cc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  float local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ffc7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  param_1[9] = 0;
  param_1[10] = 0;
  param_1[0xf] = 0;
  param_1[0x10] = 0;
  param_1[0x1f] = 0;
  param_1[0x20] = 0;
  *param_1 = &PTR_FUN_005a443c;
  *(undefined2 *)(param_1 + 0x8026) = 0xffff;
  param_1[0x1e] = 0;
  param_1[0x8027] = 0;
  DAT_005b48e8 = 0x428c0000;
  DAT_005ccec4 = 0;
  param_1[0x802b] = 0;
  param_1[0x802a] = 0;
  param_1[0x8025] = 0;
  param_1[0x13] = 0;
  param_1[0x11] = 0;
  param_1[0x12] = 0;
  param_1[0xd] = 0;
  param_1[0xe] = 0;
  param_1[0x18] = 0;
  param_1[0x16] = 0;
  param_1[0x22] = 0;
  param_1[0x14] = 0;
  param_1[0x15] = 0;
  param_1[0x8028] = 0;
  param_1[0x8029] = 0;
  DAT_005bdeec = 2;
  param_1[0x17] = 0;
  param_1[0x9b85] = 0;
  param_1[0x9b86] = 0;
  for (local_14 = 0; local_14 < 2; local_14 = local_14 + 1) {
    param_1[local_14 + 0x1a] = 0;
    param_1[local_14 + 0x1c] = 0;
  }
  pvVar2 = operator_new(0x154);
  local_8._0_1_ = 1;
  if (pvVar2 == (void *)0x0) {
    local_98 = 0;
  }
  else {
    local_98 = FUN_0040c2cd(param_1);
  }
  local_8._0_1_ = 0;
  param_1[10] = local_98;
  pvVar2 = operator_new(0x24);
  local_8._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    local_9c = 0;
  }
  else {
    local_9c = FUN_0054a9e0(0);
  }
  local_8._0_1_ = 0;
  param_1[0xf] = local_9c;
  FUN_0054ac09(param_1[0xf]);
  pvVar2 = operator_new(0x24);
  local_8._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    local_a0 = 0;
  }
  else {
    local_a0 = FUN_0054a9e0(0);
  }
  local_8._0_1_ = 0;
  param_1[0xb] = local_a0;
  FUN_0054ac09(param_1[0xb]);
  pvVar2 = operator_new(0x24);
  local_8._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    local_a4 = 0;
  }
  else {
    local_a4 = FUN_0054a9e0(0);
  }
  local_8._0_1_ = 0;
  param_1[0xc] = local_a4;
  FUN_0054ac09(param_1[0xc]);
  pvVar2 = operator_new(0x24);
  local_8._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    local_a8 = 0;
  }
  else {
    local_a8 = FUN_0054a9e0(0);
  }
  local_8._0_1_ = 0;
  param_1[0x19] = local_a8;
  FUN_0054ac09(param_1[0x19]);
  pvVar2 = operator_new(0x24);
  local_8._0_1_ = 6;
  if (pvVar2 == (void *)0x0) {
    local_ac = 0;
  }
  else {
    local_ac = FUN_0054a9e0(0);
  }
  local_8._0_1_ = 0;
  param_1[0xd] = local_ac;
  FUN_0054ac09(param_1[0xd]);
  pvVar2 = operator_new(0xe4c);
  local_8._0_1_ = 7;
  if (pvVar2 == (void *)0x0) {
    local_b0 = 0;
  }
  else {
    local_b0 = FUN_00401d03(0xfffffffe,&DAT_005b8930,0xffffffff,0x42f00000,0x428c0000,0x44070000,
                            0x41a00000,1,0xaaff0000,1,1);
  }
  local_8._0_1_ = 0;
  param_1[0x22] = local_b0;
  *(undefined4 *)(param_1[0x22] + 0x3c) = 0;
  (**(code **)(*(int *)param_1[0x22] + 0x60))(0);
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0x1fc);
    local_8._0_1_ = 8;
    if (pvVar2 == (void *)0x0) {
      local_b4 = 0;
    }
    else {
      local_b4 = FUN_00403924(s_Message_Panel_005b893c,0x43160000,0x42a00000,0x43f00000,0x41c00000,
                              2000);
    }
    param_1[0x21] = local_b4;
  }
  else {
    pvVar2 = operator_new(0x1fc);
    local_8._0_1_ = 9;
    if (pvVar2 == (void *)0x0) {
      local_b8 = 0;
    }
    else {
      local_b8 = FUN_00403924(s_Message_Panel_005b894c,0x43160000,0x42a00000,0x43f00000,0x42200000,
                              2000);
    }
    param_1[0x21] = local_b8;
  }
  local_8._0_1_ = 0;
  pvVar2 = operator_new(0x218);
  local_8._0_1_ = 10;
  if (pvVar2 == (void *)0x0) {
    local_cc = 0;
  }
  else {
    local_cc = FUN_00403eb8(s_Message_Box_005b895c,0,
                            ((float)*(uint *)(DAT_005ccf9c + 0x2a504) / DAT_005b490c - _DAT_005a41e4
                            ) / _DAT_005a365c,
                            ((float)*(uint *)(DAT_005ccf9c + 0x2a508) / DAT_005b4910 - _DAT_005a4438
                            ) / _DAT_005a365c);
  }
  local_8._0_1_ = 0;
  param_1[0x23] = local_cc;
  pvVar2 = operator_new(0x218);
  local_8._0_1_ = 0xb;
  if (pvVar2 == (void *)0x0) {
    local_e0 = 0;
  }
  else {
    local_e0 = FUN_00403eb8(s_Message_Box_005b8968,4,
                            ((float)*(uint *)(DAT_005ccf9c + 0x2a504) / DAT_005b490c - _DAT_005a41e4
                            ) / _DAT_005a365c,
                            ((float)*(uint *)(DAT_005ccf9c + 0x2a508) / DAT_005b4910 - _DAT_005a4438
                            ) / _DAT_005a365c);
  }
  local_8._0_1_ = 0;
  uVar1 = (undefined1)local_8;
  local_8._0_1_ = 0;
  param_1[0x24] = local_e0;
  local_18 = 0.0;
  if (*(uint *)(DAT_005ccf9c + 0x2a504) < 800) {
    local_18 = 50.0;
  }
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0xe4c);
    local_8._0_1_ = 0xc;
    if (pvVar2 == (void *)0x0) {
      local_ec = 0;
    }
    else {
      local_ec = FUN_00401d03(0xfffffffe,s_1__2__3__4__5__6__7__8__9__005b8974,0xffffffff,0x41e00000
                              ,(float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a4314,
                              _DAT_005a43c4 + local_18,0x41800000,1,0x9955aa55,1,0);
    }
    param_1[0x17] = local_ec;
  }
  else {
    local_8._0_1_ = uVar1;
    pvVar2 = operator_new(0xe4c);
    local_8._0_1_ = 0xd;
    if (pvVar2 == (void *)0x0) {
      local_f8 = 0;
    }
    else {
      local_f8 = FUN_00401d03(0xfffffffe,s_1__2__3__4__5__6__7__8__9__005b89a0,0xffffffff,0x41e00000
                              ,(float)*(uint *)(DAT_005ccf9c + 0x2a508) / DAT_005b4910 -
                               _DAT_005a4314,_DAT_005a43c4 + local_18,0x41800000,1,0x9955aa55,1,0);
    }
    param_1[0x17] = local_f8;
  }
  local_8._0_1_ = 0;
  (**(code **)(*(int *)param_1[0x21] + 0x88))(0,1);
  (**(code **)(*(int *)param_1[0x23] + 0x60))(0);
  (**(code **)(*(int *)param_1[0x24] + 0x60))(0);
  if (param_1[10] == 0) {
    local_fc = 0;
  }
  else {
    local_fc = param_1[10] + 0x24;
  }
  (**(code **)(*(int *)param_1[0x23] + 0x50))(local_fc);
  if (param_1[10] == 0) {
    local_100 = 0;
  }
  else {
    local_100 = param_1[10] + 0x24;
  }
  (**(code **)(*(int *)param_1[0x24] + 0x50))(local_100);
  *(undefined4 *)(param_1[0x23] + 0x60) = 1;
  *(undefined4 *)(param_1[0x24] + 0x60) = 1;
  *(undefined4 *)(param_1[0x21] + 0x3c) = 0;
  *(undefined4 *)(param_1[10] + 0x134) = param_1[0x23];
  *(undefined4 *)(param_1[10] + 0x144) = param_1[0x24];
  (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x21]);
  (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x23]);
  (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x24]);
  (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x22]);
  (**(code **)(*(int *)param_1[0x17] + 0x60))(0);
  (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x17]);
  if (DAT_005b892c == 2) {
    pvVar2 = operator_new(0x208);
    local_8._0_1_ = 0xe;
    if (pvVar2 == (void *)0x0) {
      local_10c = 0;
    }
    else {
      local_10c = FUN_00402f01(0xfffffffe,0,(float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a4314
                               ,0x41800000,0x41400000,0xaa000077,0,&DAT_005b89cc);
    }
    param_1[0x18] = local_10c;
  }
  else {
    pvVar2 = operator_new(0x208);
    local_8._0_1_ = 0xf;
    if (pvVar2 == (void *)0x0) {
      local_118 = 0;
    }
    else {
      local_118 = FUN_00402f01(0xfffffffe,0,
                               (float)*(uint *)(DAT_005ccf9c + 0x2a508) / DAT_005b4910 -
                               _DAT_005a4314,0x41e00000,0x41800000,0xaa000077,0,&DAT_005b89d0);
    }
    param_1[0x18] = local_118;
  }
  local_8 = (uint)local_8._1_3_ << 8;
  (**(code **)(*(int *)param_1[0x18] + 0x44))(0x1214);
  (**(code **)(*(int *)param_1[0x18] + 0x60))(0);
  if (param_1[10] == 0) {
    local_11c = 0;
  }
  else {
    local_11c = param_1[10] + 0x24;
  }
  (**(code **)(*(int *)param_1[0x18] + 0x50))(local_11c);
  (**(code **)(*(int *)param_1[10] + 0x44))(param_1[0x18]);
  _memset(param_1 + 0x25,0,0x10000);
  _memset(param_1 + 0x4025,0,0x10000);
  FUN_0054f18b(param_1 + 0x25,0x100);
  ExceptionList = local_10;
  return param_1;
}

