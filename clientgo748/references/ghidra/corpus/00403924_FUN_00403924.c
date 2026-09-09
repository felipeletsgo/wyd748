// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403924 | Name: FUN_00403924


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_00403924(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            float param_5,float param_6,undefined4 param_7)

{
  void *pvVar1;
  undefined4 uVar2;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059e9eb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(0xffffffd3,param_3,param_4,param_5,param_6,0xaaffffff,4);
  local_8 = 0;
  *param_1 = &PTR_FUN_005a3708;
  param_1[0x10] = 9;
  if (DAT_005b892c == 2) {
    param_1[0x28] = 0xffffff4e;
    param_1[0x11] = 0x1606;
    FUN_0040116e(param_1[0x11],param_3,param_4,param_5,param_6);
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 1;
    if (pvVar1 == (void *)0x0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_00401d03(0xfffffffe,param_2,0xffffffff,0x41f00000,0x3f800000,
                              (param_5 - _DAT_005a365c) - _DAT_005a3704,param_6 - _DAT_005a365c,1,0,
                              1,1);
    }
    local_8._0_1_ = 0;
    param_1[0x79] = local_48;
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 2;
    if (pvVar1 == (void *)0x0) {
      local_4c = 0;
    }
    else {
      local_4c = FUN_00401d03(0xfffffffe,&DAT_005cced8,0xffffffff,0x42480000,0x41c00000,
                              (param_5 - _DAT_005a365c) - _DAT_005a3704,0x41600000,1,0,1,1);
    }
    local_8._0_1_ = 0;
    param_1[0x7a] = local_4c;
    pvVar1 = operator_new(0x1e4);
    local_8._0_1_ = 3;
    if (pvVar1 == (void *)0x0) {
      local_50 = 0;
    }
    else {
      local_50 = FUN_00401466(0xfffffefd,0xc1300000,0,0x41300000,param_6,0xaaffffff,4);
    }
    local_8._0_1_ = 0;
    param_1[0x7b] = local_50;
    pvVar1 = operator_new(0x1e4);
    local_8._0_1_ = 4;
    if (pvVar1 == (void *)0x0) {
      local_54 = 0;
    }
    else {
      local_54 = FUN_00401466(0xfffffefc,param_5,0,0x41300000,param_6,0xaaffffff,4);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x7c] = local_54;
    if (param_1[0x7b] != 0) {
      FUN_0054ac09(param_1[0x7b]);
    }
    if (param_1[0x7c] != 0) {
      FUN_0054ac09(param_1[0x7c]);
    }
  }
  else {
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 5;
    if (pvVar1 == (void *)0x0) {
      local_58 = 0;
    }
    else {
      local_58 = FUN_00401d03(0xfffffffe,param_2,0xffffffff,0x41f00000,0x3f800000,
                              (param_5 - _DAT_005a365c) - _DAT_005a3704,param_6 - _DAT_005a365c,1,0,
                              1,1);
    }
    local_8._0_1_ = 0;
    param_1[0x79] = local_58;
    pvVar1 = operator_new(0xe4c);
    local_8._0_1_ = 6;
    if (pvVar1 == (void *)0x0) {
      local_5c = 0;
    }
    else {
      local_5c = FUN_00401d03(0xfffffffe,&DAT_005ccedc,0xffffffff,0x42480000,0x41c00000,
                              (param_5 - _DAT_005a365c) - _DAT_005a3704,0x41600000,1,0,1,1);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x7a] = local_5c;
  }
  if (param_1[0x79] != 0) {
    FUN_0054ac09(param_1[0x79]);
  }
  param_1[0x7e] = param_7;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x7d] = uVar2;
  ExceptionList = local_10;
  return param_1;
}

