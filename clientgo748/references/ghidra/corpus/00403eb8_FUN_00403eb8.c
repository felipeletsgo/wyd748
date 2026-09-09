// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403eb8 | Name: FUN_00403eb8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_00403eb8(undefined4 *param_1,char *param_2,char param_3,undefined4 param_4,undefined4 param_5)

{
  void *pvVar1;
  size_t sVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 *local_10c;
  undefined4 local_108;
  undefined4 *local_104;
  undefined4 local_100;
  undefined4 local_fc;
  undefined4 *local_f8;
  undefined4 local_f4;
  undefined4 local_f0;
  undefined4 *local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_cc;
  undefined4 local_c0;
  undefined4 local_b4;
  undefined4 local_a8;
  undefined4 local_9c;
  undefined4 local_98;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059eaca;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(0xfffffffe,param_4,param_5,0x43800000,0x43000000,&DAT_01010101,4);
  local_8 = 0;
  FUN_0040bad0();
  *param_1 = &PTR_FUN_005a379c;
  param_1[0x79] = &PTR_FUN_005a3798;
  param_1[0x7a] = 0;
  param_1[0x10] = 8;
  param_1[0x11] = 0x1209;
  param_1[0x7b] = 0xffffffff;
  FUN_0040116e(param_1[0x11],param_4,param_5,0x43680000,0x43000000);
  if (DAT_005b892c == 2) {
    pvVar1 = operator_new(0x1e4);
    local_8._0_1_ = 1;
    if (pvVar1 == (void *)0x0) {
      local_98 = 0;
    }
    else {
      local_98 = FUN_00401466(0xa4,0,0,0x43680000,0x42d60000,0x77777777,4);
    }
    param_1[0x7d] = local_98;
    param_1[0x77] = 1;
  }
  else {
    pvVar1 = operator_new(0x1e4);
    local_8._0_1_ = 2;
    if (pvVar1 == (void *)0x0) {
      local_9c = 0;
    }
    else {
      local_9c = FUN_00401466(9,0,0,0x43800000,0x42c20000,0x77777777,4);
    }
    param_1[0x7d] = local_9c;
  }
  local_8._0_1_ = 0;
  if (DAT_005b892c == 2) {
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 3;
    if (pvVar1 == (void *)0x0) {
      local_a8 = 0;
    }
    else {
      uVar7 = 0;
      uVar6 = 0;
      uVar5 = 1;
      uVar4 = 0xffffffff;
      uVar3 = 0x41400000;
      sVar2 = _strlen(param_2);
      local_a8 = FUN_00401d03(0xffffffff,param_2,0xffffffff,0x41200000,0x42100000,
                              (float)sVar2 * _DAT_005a3794,uVar3,uVar4,uVar5,uVar6,uVar7);
    }
    local_8._0_1_ = 0;
    param_1[0x7f] = local_a8;
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 4;
    if (pvVar1 == (void *)0x0) {
      local_b4 = 0;
    }
    else {
      uVar7 = 0;
      uVar6 = 0;
      uVar5 = 1;
      uVar4 = 0xffffffff;
      uVar3 = 0x41400000;
      sVar2 = _strlen(param_2);
      local_b4 = FUN_00401d03(0xffffffff,param_2,0xffffffff,0x41200000,0x42500000,
                              (float)sVar2 * _DAT_005a3794,uVar3,uVar4,uVar5,uVar6,uVar7);
    }
    local_8._0_1_ = 0;
    param_1[0x80] = local_b4;
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 5;
    if (pvVar1 == (void *)0x0) {
      local_c0 = 0;
    }
    else {
      uVar7 = 0;
      uVar6 = 0;
      uVar5 = 1;
      uVar4 = 0xffffffff;
      uVar3 = 0x41400000;
      sVar2 = _strlen(&DAT_00a3b7f8);
      local_c0 = FUN_00401d03(0xffffffff,&DAT_00a3b7f8,0xffffffff,0x42c00000,0x41100000,
                              (float)sVar2 * _DAT_005a3794,uVar3,uVar4,uVar5,uVar6,uVar7);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x81] = local_c0;
    if (param_1[0x81] != 0) {
      FUN_0054ac09(param_1[0x81]);
    }
  }
  else {
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 6;
    if (pvVar1 == (void *)0x0) {
      local_cc = 0;
    }
    else {
      uVar7 = 0;
      uVar6 = 0;
      uVar5 = 1;
      uVar4 = 0xffffffff;
      uVar3 = 0x41400000;
      sVar2 = _strlen(param_2);
      local_cc = FUN_00401d03(0xffffffff,param_2,0xffffffff,0x41a00000,0x41880000,
                              (float)sVar2 * _DAT_005a3794,uVar3,uVar4,uVar5,uVar6,uVar7);
    }
    local_8._0_1_ = 0;
    param_1[0x7f] = local_cc;
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 7;
    if (pvVar1 == (void *)0x0) {
      local_d8 = 0;
    }
    else {
      uVar7 = 0;
      uVar6 = 0;
      uVar5 = 1;
      uVar4 = 0xffffffff;
      uVar3 = 0x41400000;
      sVar2 = _strlen(param_2);
      local_d8 = FUN_00401d03(0xffffffff,param_2,0xffffffff,0x41a00000,0x42000000,
                              (float)sVar2 * _DAT_005a3794,uVar3,uVar4,uVar5,uVar6,uVar7);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x80] = local_d8;
  }
  if (param_1[0x7f] != 0) {
    FUN_0054ac09(param_1[0x7f]);
  }
  if (param_1[0x80] != 0) {
    FUN_0054ac09(param_1[0x80]);
  }
  (**(code **)(*(int *)param_1[0x7f] + 0x80))(&DAT_005ccee0,0);
  (**(code **)(*(int *)param_1[0x80] + 0x80))(&DAT_005ccee4,0);
  *(char *)(param_1 + 0x7c) = param_3;
  if (param_3 == '\0') {
    if (DAT_005b892c == 2) {
      pvVar1 = operator_new(0x1e4);
      local_8._0_1_ = 8;
      if (pvVar1 == (void *)0x0) {
        local_dc = 0;
      }
      else {
        local_dc = FUN_00401466(0xa5,0x41a00000,0x42980000,0x42b00000,0x41b80000,0x77777777,4);
      }
      local_8._0_1_ = 0;
      param_1[0x84] = local_dc;
      pvVar1 = operator_new(0x1e4);
      local_8._0_1_ = 9;
      if (pvVar1 == (void *)0x0) {
        local_e0 = 0;
      }
      else {
        local_e0 = FUN_00401466(0xa5,0x42f80000,0x42980000,0x42b00000,0x41b80000,0x77777777,4);
      }
      local_8._0_1_ = 0;
      param_1[0x85] = local_e0;
      pvVar1 = operator_new(0x208);
      local_8._0_1_ = 10;
      if (pvVar1 == (void *)0x0) {
        local_e4 = 0;
      }
      else {
        local_e4 = FUN_00402f01(0xfffffffe,0x41400000,0x429a0000,0x42ce0000,0x41a80000,0x77777777,1,
                                &DAT_00a3b878);
      }
      param_1[0x82] = local_e4;
    }
    else {
      pvVar1 = operator_new(0x208);
      local_8._0_1_ = 0xb;
      if (pvVar1 == (void *)0x0) {
        local_e8 = 0;
      }
      else {
        local_e8 = FUN_00402f01(0xe,0x41c80000,0x42600000,0x42ce0000,0x41a80000,0x77777777,1,
                                &DAT_005ccee8);
      }
      param_1[0x82] = local_e8;
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (param_1[0x82] != 0) {
      (**(code **)(*(int *)param_1[0x82] + 0x44))(1);
      if (param_1 == (undefined4 *)0x0) {
        local_ec = (undefined4 *)0x0;
      }
      else {
        local_ec = param_1 + 0x79;
      }
      (**(code **)(*(int *)param_1[0x82] + 0x50))(local_ec);
      FUN_0054ac09(param_1[0x82]);
    }
    if (DAT_005b892c == 2) {
      pvVar1 = operator_new(0x208);
      local_8._0_1_ = 0xc;
      if (pvVar1 == (void *)0x0) {
        local_f0 = 0;
      }
      else {
        local_f0 = FUN_00402f01(0xfffffffe,0x42ea0000,0x429a0000,0x42ce0000,0x41a80000,0x77777777,1,
                                &DAT_00a3b8f8);
      }
      param_1[0x83] = local_f0;
    }
    else {
      pvVar1 = operator_new(0x208);
      local_8._0_1_ = 0xd;
      if (pvVar1 == (void *)0x0) {
        local_f4 = 0;
      }
      else {
        local_f4 = FUN_00402f01(0xf,0x43000000,0x42600000,0x42ce0000,0x41a80000,0x77777777,1,
                                &DAT_005cceec);
      }
      param_1[0x83] = local_f4;
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (param_1[0x83] != 0) {
      (**(code **)(*(int *)param_1[0x83] + 0x44))(2);
      if (param_1 == (undefined4 *)0x0) {
        local_f8 = (undefined4 *)0x0;
      }
      else {
        local_f8 = param_1 + 0x79;
      }
      (**(code **)(*(int *)param_1[0x83] + 0x50))(local_f8);
      FUN_0054ac09(param_1[0x83]);
    }
    if (DAT_005b892c == 2) {
      if (param_1[0x84] != 0) {
        FUN_0054ac09(param_1[0x84]);
      }
      if (param_1[0x85] != 0) {
        FUN_0054ac09(param_1[0x85]);
      }
    }
  }
  else if (param_3 == '\x04') {
    if (DAT_005b892c == 2) {
      pvVar1 = operator_new(0x1e4);
      local_8._0_1_ = 0xe;
      if (pvVar1 == (void *)0x0) {
        local_fc = 0;
      }
      else {
        local_fc = FUN_00401466(0xa5,0x428c0000,0x42980000,0x42b00000,0x41b80000,0x77777777,4);
      }
      local_8._0_1_ = 0;
      param_1[0x84] = local_fc;
      pvVar1 = operator_new(0x208);
      local_8._0_1_ = 0xf;
      if (pvVar1 == (void *)0x0) {
        local_100 = 0;
      }
      else {
        local_100 = FUN_00402f01(0xfffffffe,0x428c0000,0x42980000,0x42b00000,0x41b80000,0x77777777,1
                                 ,&DAT_00a3b878);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x82] = local_100;
      if (param_1[0x82] != 0) {
        (**(code **)(*(int *)param_1[0x82] + 0x44))(1);
        if (param_1 == (undefined4 *)0x0) {
          local_104 = (undefined4 *)0x0;
        }
        else {
          local_104 = param_1 + 0x79;
        }
        (**(code **)(*(int *)param_1[0x82] + 0x50))(local_104);
        FUN_0054ac09(param_1[0x82]);
      }
      if (param_1[0x84] != 0) {
        FUN_0054ac09(param_1[0x84]);
      }
    }
    else {
      pvVar1 = operator_new(0x208);
      local_8._0_1_ = 0x10;
      if (pvVar1 == (void *)0x0) {
        local_108 = 0;
      }
      else {
        local_108 = FUN_00402f01(0xe,0x429e0000,0x42600000,0x42ce0000,0x41a80000,0x77777777,1,
                                 &DAT_005ccef0);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x82] = local_108;
      if (param_1[0x82] != 0) {
        (**(code **)(*(int *)param_1[0x82] + 0x44))(1);
        if (param_1 == (undefined4 *)0x0) {
          local_10c = (undefined4 *)0x0;
        }
        else {
          local_10c = param_1 + 0x79;
        }
        (**(code **)(*(int *)param_1[0x82] + 0x50))(local_10c);
        FUN_0054ac09(param_1[0x82]);
      }
    }
  }
  if (param_1[0x7d] != 0) {
    FUN_0054ac09(param_1[0x7d]);
  }
  ExceptionList = local_10;
  return param_1;
}

