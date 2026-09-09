// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004072f2 | Name: FUN_004072f2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004072f2(undefined4 *param_1,int param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,float param_7,undefined4 param_8,
            undefined4 param_9,undefined1 param_10,undefined1 param_11)

{
  void *pvVar1;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_1c;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ebe8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00407203(param_3,param_4,param_5,param_6,param_7,param_8,0,0x77777777,1,0);
  local_8._0_1_ = 0;
  local_8._1_3_ = 0;
  param_1[0x394] = 0;
  param_1[0x398] = 0;
  *(undefined1 *)((int)param_1 + 0xe65) = param_10;
  *param_1 = &PTR_FUN_005a3aa8;
  param_1[0x398] = param_9;
  *(undefined1 *)(param_1 + 0x399) = 1;
  param_1[0x395] = 0;
  param_1[0x396] = 0;
  param_1[0x397] = 0;
  *(undefined1 *)((int)param_1 + 0xe66) = param_11;
  local_14 = 0xffffffff;
  local_1c = 0xffffffff;
  if (*(char *)((int)param_1 + 0xe65) == '\x01') {
    pvVar1 = operator_new(0x1e4);
    local_8._0_1_ = 1;
    if (pvVar1 == (void *)0x0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_00401466(0x97,_DAT_005a3aa4 + 0.0,0x3f800000,0x41800000,0x41800000,0xffffffff,4
                             );
    }
    param_1[0x395] = local_44;
  }
  local_8._0_1_ = 0;
  if (*(char *)((int)param_1 + 0xe66) == '\x01') {
    pvVar1 = operator_new(0x1e4);
    local_8._0_1_ = 2;
    if (pvVar1 == (void *)0x0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_00401466(0x13c,_DAT_005a3aa0 + 0.0,0x3f800000,0x41800000,0x41800000,0xffffffff,
                              4);
    }
    param_1[0x396] = local_48;
  }
  local_8._0_1_ = 0;
  if (param_2 < -1) {
    local_14 = 0xff00ff99;
    local_1c = 0xff222222;
  }
  if (DAT_005b892c == 2) {
    pvVar1 = operator_new(0x374);
    local_8 = CONCAT31(local_8._1_3_,3);
    if (pvVar1 == (void *)0x0) {
      local_4c = 0;
    }
    else {
      local_4c = FUN_00405065(param_2,param_9,400,param_7 - _DAT_005a3a9c,0x40c00000,0x42200000,
                              0x40c00000,local_14,local_1c,1);
    }
    param_1[0x394] = local_4c;
  }
  else {
    pvVar1 = operator_new(0x374);
    local_8 = CONCAT31(local_8._1_3_,4);
    if (pvVar1 == (void *)0x0) {
      local_50 = 0;
    }
    else {
      local_50 = FUN_00405065(param_2,param_9,400,param_7 - _DAT_005a3a98,0x40c00000,0x42200000,
                              0x40c00000,local_14,local_1c,1);
    }
    param_1[0x394] = local_50;
  }
  ExceptionList = local_10;
  return param_1;
}

