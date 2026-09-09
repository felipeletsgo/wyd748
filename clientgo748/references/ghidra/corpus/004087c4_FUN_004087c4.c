// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004087c4 | Name: FUN_004087c4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_004087c4(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,float param_7,float param_8,undefined4 param_9,
            undefined4 param_10,undefined4 param_11,undefined1 param_12,undefined1 param_13)

{
  void *pvVar1;
  bool bVar2;
  undefined4 *local_3c;
  undefined4 local_38;
  undefined4 *local_34;
  undefined4 local_30;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ecf5;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(param_2,param_5,param_6,param_7,param_8,param_9,param_10);
  local_8 = 0;
  FUN_0040bad0();
  *(undefined1 *)(param_1 + 0x7a) = param_12;
  *(undefined1 *)((int)param_1 + 0x1e9) = param_13;
  *(undefined2 *)((int)param_1 + 0x1ea) = 0;
  param_1[0x7c] = param_3;
  param_1[0x7d] = param_4;
  param_1[0x7e] = 0;
  param_1[0x7f] = 0xffffffff;
  param_1[0x82] = 0;
  param_1[0x83] = 0;
  param_1[0x84] = 0;
  *param_1 = &PTR_FUN_005a3cf0;
  param_1[0x79] = &PTR_FUN_005a3cec;
  if (100 < (int)param_1[0x7c]) {
    param_1[0x7c] = 100;
  }
  param_1[0xf] = param_11;
  param_1[0x10] = 6;
  param_1[0x7b] = 0;
  local_18 = 8;
  bVar2 = DAT_005b892c == 2;
  if (bVar2) {
    local_18 = 0xc;
  }
  if (*(char *)(param_1 + 0x7a) == '\x01') {
    pvVar1 = operator_new(0x94);
    local_8._0_1_ = 1;
    if (pvVar1 == (void *)0x0) {
      local_30 = 0;
    }
    else {
      local_30 = FUN_0040586f(0,0,0,0,(float)local_18,param_8 - _DAT_005a3660,0,0,0xffaaaaaa,bVar2);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x83] = local_30;
    if (param_1[0x83] != 0) {
      (**(code **)(*(int *)param_1[0x83] + 0x60))(1);
      (**(code **)(*(int *)param_1[0x83] + 0x44))(1);
      if (param_1 == (undefined4 *)0x0) {
        local_34 = (undefined4 *)0x0;
      }
      else {
        local_34 = param_1 + 0x79;
      }
      (**(code **)(*(int *)param_1[0x83] + 0x50))(local_34);
      FUN_0054ac09(param_1[0x83]);
    }
  }
  else if (*(char *)(param_1 + 0x7a) == '\x02') {
    pvVar1 = operator_new(0x94);
    local_8._0_1_ = 2;
    if (pvVar1 == (void *)0x0) {
      local_38 = 0;
    }
    else {
      local_38 = FUN_0040586f(0,0,(param_7 - (float)local_18) + _DAT_005a365c,0,(float)local_18,
                              param_8 - _DAT_005a3660,0,0,0xffaaaaaa,0);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x83] = local_38;
    if (param_1[0x83] != 0) {
      (**(code **)(*(int *)param_1[0x83] + 0x60))(1);
      (**(code **)(*(int *)param_1[0x83] + 0x44))(1);
      if (param_1 == (undefined4 *)0x0) {
        local_3c = (undefined4 *)0x0;
      }
      else {
        local_3c = param_1 + 0x79;
      }
      (**(code **)(*(int *)param_1[0x83] + 0x50))(local_3c);
      FUN_0054ac09(param_1[0x83]);
    }
  }
  if (*(char *)((int)param_1 + 0x1e9) == '\x01') {
    FUN_00408bed();
  }
  _memset(param_1 + 0x85,0,400);
  param_1[0x80] = param_1[0x15];
  param_1[0x81] = param_1[0x16];
  ExceptionList = local_10;
  return param_1;
}

